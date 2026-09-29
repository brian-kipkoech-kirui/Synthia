#include "synthia/finance/FinancialStream.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <random>
#include <regex>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

namespace synthia::finance {
namespace {
std::string base64(const std::string& input) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    unsigned int value = 0;
    int bits = -6;
    for (const unsigned char character : input) {
        value = (value << 8) | character;
        bits += 8;
        while (bits >= 0) {
            output.push_back(alphabet[(value >> bits) & 0x3f]);
            bits -= 6;
        }
    }
    if (bits > -6) output.push_back(alphabet[((value << 8) >> (bits + 8)) & 0x3f]);
    while (output.size() % 4) output.push_back('=');
    return output;
}

std::string websocket_accept(const std::string& key) {
    const auto material = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    const auto command = "printf '%s' '" + material + "' | openssl dgst -sha1 -binary | openssl base64 -A";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return {};
    std::array<char, 128> output{};
    const auto count = std::fread(output.data(), 1, output.size(), pipe);
    const auto status = pclose(pipe);
    if (status != 0) return {};
    return std::string(output.data(), count);
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string percent_encode(const std::string& input) {
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string output;
    for (const unsigned char character : input) {
        if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_' || character == '.' || character == '~') {
            output.push_back(static_cast<char>(character));
        } else {
            output.push_back('%');
            output.push_back(hex[character >> 4]);
            output.push_back(hex[character & 0x0f]);
        }
    }
    return output;
}

std::int64_t unix_milliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool write_all(int socket, const std::string& data, std::string& error) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        const auto result = ::send(socket, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (result <= 0) {
            error = "WebSocket transport write failed";
            return false;
        }
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

std::string json_string(const std::string& value) {
    std::ostringstream output;
    output << '"';
    for (const unsigned char character : value) {
        if (character == '\\' || character == '"') output << '\\' << static_cast<char>(character);
        else if (character < 0x20) output << ' ';
        else output << static_cast<char>(character);
    }
    output << '"';
    return output.str();
}

std::string endpoint_without_credentials(const std::string& endpoint) {
    const auto query = endpoint.find('?');
    return endpoint.substr(0, query);
}

}  // namespace

OpenSslWebSocketTransport::OpenSslWebSocketTransport() = default;
OpenSslWebSocketTransport::~OpenSslWebSocketTransport() { close(); }

bool OpenSslWebSocketTransport::connect(const std::string& endpoint, std::chrono::milliseconds timeout, std::string& error) {
    close();
    if (endpoint.compare(0, 6, "wss://") != 0 || endpoint.find_first_of("\r\n \t") != std::string::npos) {
        error = "WebSocket endpoint must use wss://";
        return false;
    }
    const auto authority_start = 6U;
    const auto authority_end = endpoint.find_first_of("/?", authority_start);
    const auto authority = endpoint.substr(authority_start, authority_end - authority_start);
    const auto colon = authority.rfind(':');
    const auto host = colon == std::string::npos ? authority : authority.substr(0, colon);
    const auto port = colon == std::string::npos ? "443" : authority.substr(colon + 1);
    if (host.empty() || !std::regex_match(authority, std::regex("[A-Za-z0-9.:-]+")) ||
        !std::regex_match(port, std::regex("[0-9]+"))) {
        error = "invalid WebSocket endpoint authority";
        return false;
    }

    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0) {
        error = "unable to create TLS transport socket pair";
        return false;
    }
    const pid_t process = fork();
    if (process < 0) {
        ::close(pair[0]);
        ::close(pair[1]);
        error = "unable to start OpenSSL TLS client";
        return false;
    }
    if (process == 0) {
        ::close(pair[0]);
        dup2(pair[1], STDIN_FILENO);
        dup2(pair[1], STDOUT_FILENO);
        const int null_output = open("/dev/null", O_WRONLY);
        if (null_output >= 0) dup2(null_output, STDERR_FILENO);
        if (pair[1] > STDERR_FILENO) ::close(pair[1]);
        if (null_output > STDERR_FILENO) ::close(null_output);
        execlp("openssl", "openssl", "s_client", "-quiet", "-connect", (host + ":" + port).c_str(),
               "-servername", host.c_str(), "-verify_hostname", host.c_str(), "-verify_return_error", static_cast<char*>(nullptr));
        _exit(127);
    }
    ::close(pair[1]);
    socket_ = pair[0];
    child_pid_ = process;
    input_.clear();

    std::random_device random;
    std::string nonce(16, '\0');
    for (auto& byte : nonce) byte = static_cast<char>(random() & 0xff);
    const auto key = base64(nonce);
    const auto query_path = endpoint.substr(authority_start + authority.size());
    const auto request = "GET " + (query_path.empty() ? "/" : query_path) + " HTTP/1.1\r\nHost: " + authority +
        "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " + key +
        "\r\nSec-WebSocket-Version: 13\r\n\r\n";
    if (!write_all(socket_, request, error)) {
        close();
        return false;
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::string response;
    while (response.find("\r\n\r\n") == std::string::npos && response.size() < 16384) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
        if (remaining.count() <= 0) break;
        pollfd descriptor{socket_, POLLIN, 0};
        const auto ready = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (ready <= 0) break;
        std::array<char, 4096> buffer{};
        const auto count = ::recv(socket_, buffer.data(), buffer.size(), 0);
        if (count <= 0) break;
        response.append(buffer.data(), static_cast<std::size_t>(count));
    }
    const auto normalized_response = lowercase(response);
    const auto expected_accept = lowercase(websocket_accept(key));
    if (response.find(" 101 ") == std::string::npos || normalized_response.find("upgrade: websocket") == std::string::npos ||
        normalized_response.find("sec-websocket-accept: " + expected_accept) == std::string::npos) {
        error = "WebSocket upgrade failed or timed out";
        close();
        return false;
    }
    const auto headers_end = response.find("\r\n\r\n") + 4;
    input_ = response.substr(headers_end);
    return true;
}

bool OpenSslWebSocketTransport::send_frame(unsigned char opcode, const std::string& payload, std::string& error) {
    if (socket_ < 0 || payload.size() > 65535) {
        error = "WebSocket is disconnected or frame is too large";
        return false;
    }
    std::random_device random;
    std::array<unsigned char, 4> mask{};
    for (auto& byte : mask) byte = static_cast<unsigned char>(random() & 0xff);
    std::string frame;
    frame.push_back(static_cast<char>(0x80 | opcode));
    if (payload.size() < 126) frame.push_back(static_cast<char>(0x80 | payload.size()));
    else {
        frame.push_back(static_cast<char>(0x80 | 126));
        frame.push_back(static_cast<char>((payload.size() >> 8) & 0xff));
        frame.push_back(static_cast<char>(payload.size() & 0xff));
    }
    for (const auto byte : mask) frame.push_back(static_cast<char>(byte));
    for (std::size_t index = 0; index < payload.size(); ++index) frame.push_back(static_cast<char>(static_cast<unsigned char>(payload[index]) ^ mask[index % mask.size()]));
    return write_all(socket_, frame, error);
}

bool OpenSslWebSocketTransport::send_text(const std::string& message, std::string& error) {
    return send_frame(0x1, message, error);
}

bool OpenSslWebSocketTransport::read_exact(std::size_t size, std::chrono::milliseconds timeout, std::string& output, std::string& error) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (input_.size() < size) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
        if (remaining.count() <= 0) {
            error = "timeout";
            return false;
        }
        pollfd descriptor{socket_, POLLIN, 0};
        const auto ready = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (ready == 0) {
            error = "timeout";
            return false;
        }
        if (ready < 0) {
            error = "WebSocket receive poll failed";
            return false;
        }
        std::array<char, 4096> buffer{};
        const auto count = ::recv(socket_, buffer.data(), buffer.size(), 0);
        if (count <= 0) {
            error = "WebSocket peer disconnected";
            return false;
        }
        input_.append(buffer.data(), static_cast<std::size_t>(count));
    }
    output.assign(input_.data(), size);
    input_.erase(0, size);
    return true;
}

WebSocketReceiveResult OpenSslWebSocketTransport::receive_text(std::chrono::milliseconds timeout, std::string& message, std::string& error) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        std::string header;
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
        if (!read_exact(2, remaining, header, error)) return error == "timeout" ? WebSocketReceiveResult::Timeout : WebSocketReceiveResult::Disconnected;
        const auto first = static_cast<unsigned char>(header[0]);
        const auto second = static_cast<unsigned char>(header[1]);
        const auto opcode = first & 0x0f;
        if ((first & 0x80) == 0 || (second & 0x80) != 0) {
            error = "fragmented or masked server WebSocket frame is unsupported";
            return WebSocketReceiveResult::Error;
        }
        std::uint64_t length = second & 0x7f;
        if (length == 126) {
            std::string extended;
            if (!read_exact(2, remaining, extended, error)) return error == "timeout" ? WebSocketReceiveResult::Timeout : WebSocketReceiveResult::Disconnected;
            length = (static_cast<unsigned char>(extended[0]) << 8) | static_cast<unsigned char>(extended[1]);
        } else if (length == 127) {
            std::string extended;
            if (!read_exact(8, remaining, extended, error)) return error == "timeout" ? WebSocketReceiveResult::Timeout : WebSocketReceiveResult::Disconnected;
            length = 0;
            for (const unsigned char byte : extended) length = (length << 8) | byte;
        }
        if (length > 1024 * 1024) {
            error = "WebSocket message exceeds 1 MiB limit";
            return WebSocketReceiveResult::Error;
        }
        std::string payload;
        if (!read_exact(static_cast<std::size_t>(length), remaining, payload, error)) return error == "timeout" ? WebSocketReceiveResult::Timeout : WebSocketReceiveResult::Disconnected;
        if (opcode == 0x8) return WebSocketReceiveResult::Disconnected;
        if (opcode == 0x9) {
            if (!send_frame(0xA, payload, error)) return WebSocketReceiveResult::Error;
            continue;
        }
        if (opcode == 0x1) {
            message = std::move(payload);
            return WebSocketReceiveResult::Message;
        }
        if (opcode != 0xA) {
            error = "unsupported WebSocket frame opcode";
            return WebSocketReceiveResult::Error;
        }
    }
}

void OpenSslWebSocketTransport::close() noexcept {
    if (socket_ >= 0) {
        ::shutdown(socket_, SHUT_RDWR);
        ::close(socket_);
        socket_ = -1;
    }
    if (child_pid_ > 0) {
        ::kill(child_pid_, SIGTERM);
        int status = 0;
        ::waitpid(child_pid_, &status, 0);
        child_pid_ = -1;
    }
    input_.clear();
}

FinnhubWebSocketSource::FinnhubWebSocketSource(FinnhubStreamConfig config, TransportFactory transport_factory)
    : config_(std::move(config)), transport_factory_(std::move(transport_factory)) {
    if (!transport_factory_) transport_factory_ = [] { return std::make_unique<OpenSslWebSocketTransport>(); };
}

knowledge::Source FinnhubWebSocketSource::source() const {
    return {"finnhub_websocket", "Finnhub WebSocket trades", endpoint_without_credentials(config_.endpoint), "market_stream"};
}

std::vector<FinancialRecord> FinnhubWebSocketSource::fetch() { return {}; }

std::string FinnhubWebSocketSource::authenticated_endpoint() const {
    const auto separator = config_.endpoint.find('?') == std::string::npos ? '?' : '&';
    return config_.endpoint + separator + "token=" + percent_encode(config_.api_key);
}

bool FinnhubWebSocketSource::run(const EventHandler& on_event, std::atomic_bool& stop_requested) {
    report_ = {};
    error_.clear();
    if (config_.api_key.empty()) {
        error_ = "SYNTHIA_FINANCE_API_KEY is required for Finnhub streaming";
        status_ = StreamConnectionStatus::Disconnected;
        return false;
    }
    if (config_.symbols.empty()) {
        error_ = "at least one Finnhub symbol is required";
        status_ = StreamConnectionStatus::Disconnected;
        return false;
    }
    static const std::regex symbol_pattern("[A-Za-z0-9._=^/-]+");
    for (const auto& symbol : config_.symbols) {
        if (!std::regex_match(symbol, symbol_pattern)) {
            error_ = "invalid subscription symbol: " + symbol;
            status_ = StreamConnectionStatus::Disconnected;
            return false;
        }
    }

    unsigned int attempts = 0;
    auto backoff = config_.initial_reconnect_delay;
    while (!stop_requested.load()) {
        status_ = attempts == 0 ? StreamConnectionStatus::Connecting : StreamConnectionStatus::Reconnecting;
        std::unique_ptr<WebSocketTransport> transport;
        std::string transport_error;
        bool connected = false;
        try {
            transport = transport_factory_();
            connected = transport && transport->connect(authenticated_endpoint(), config_.connect_timeout, transport_error);
        } catch (const std::exception& exception) {
            transport_error = std::string("WebSocket connection failed: ") + exception.what();
        } catch (...) {
            transport_error = "WebSocket connection failed";
        }
        if (!connected) {
            error_ = transport_error.empty() ? "unable to create WebSocket transport" : transport_error;
        } else {
            status_ = StreamConnectionStatus::Authenticated;
            bool subscribed = true;
            for (const auto& symbol : config_.symbols) {
                const auto subscription = "{\"type\":\"subscribe\",\"symbol\":" + json_string(symbol) + "}";
                bool sent = false;
                try {
                    sent = transport->send_text(subscription, transport_error);
                } catch (const std::exception& exception) {
                    transport_error = std::string("Finnhub subscription failed: ") + exception.what();
                } catch (...) {
                    transport_error = "Finnhub subscription failed";
                }
                if (!sent) {
                    subscribed = false;
                    break;
                }
            }
            if (subscribed) {
                status_ = StreamConnectionStatus::Subscribed;
                backoff = config_.initial_reconnect_delay;
                while (!stop_requested.load()) {
                    std::string message;
                    WebSocketReceiveResult receive = WebSocketReceiveResult::Error;
                    try {
                        receive = transport->receive_text(config_.receive_timeout, message, transport_error);
                    } catch (const std::exception& exception) {
                        transport_error = std::string("Finnhub receive failed: ") + exception.what();
                    } catch (...) {
                        transport_error = "Finnhub receive failed";
                    }
                    if (receive == WebSocketReceiveResult::Timeout) continue;
                    if (receive == WebSocketReceiveResult::Disconnected) {
                        error_ = transport_error.empty() ? "Finnhub WebSocket disconnected" : transport_error;
                        break;
                    }
                    if (receive == WebSocketReceiveResult::Error) {
                        error_ = transport_error.empty() ? "Finnhub WebSocket receive error" : transport_error;
                        break;
                    }
                    ++report_.messages;
                    std::vector<FinancialEvent> events;
                    bool normalized = false;
                    try {
                        normalized = normalize_message(message, events);
                    } catch (const std::exception& exception) {
                        error_ = std::string("malformed Finnhub message: ") + exception.what();
                    } catch (...) {
                        error_ = "malformed Finnhub message";
                    }
                    if (!normalized) {
                        ++report_.malformed_messages;
                        if (error_.find("authentication error") != std::string::npos) {
                            transport->close();
                            status_ = StreamConnectionStatus::Disconnected;
                            return false;
                        }
                        continue;
                    }
                    for (const auto& event : events) {
                        try {
                            on_event(event);
                        } catch (const std::exception& exception) {
                            error_ = std::string("financial event consumer failed: ") + exception.what();
                            break;
                        } catch (...) {
                            error_ = "financial event consumer failed";
                            break;
                        }
                        ++report_.events;
                    }
                }
            } else {
                error_ = transport_error.empty() ? "Finnhub subscription failed" : transport_error;
            }
        }
        if (transport) transport->close();
        if (stop_requested.load()) break;
        ++attempts;
        ++report_.reconnects;
        if (config_.maximum_reconnect_attempts != 0 && attempts > config_.maximum_reconnect_attempts) {
            status_ = StreamConnectionStatus::Disconnected;
            return false;
        }
        status_ = StreamConnectionStatus::Reconnecting;
        const auto end = std::chrono::steady_clock::now() + backoff;
        while (!stop_requested.load() && std::chrono::steady_clock::now() < end) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        backoff = std::min(backoff * 2, config_.maximum_reconnect_delay);
    }
    status_ = StreamConnectionStatus::Stopping;
    return true;
}

bool FinnhubWebSocketSource::normalize_message(const std::string& message, std::vector<FinancialEvent>& events) {
    if (message.find("\"type\":\"ping\"") != std::string::npos) return true;
    if (message.find("\"type\":\"error\"") != std::string::npos) {
        error_ = "Finnhub authentication error or provider rejection";
        return false;
    }
    if (message.find("\"type\":\"trade\"") == std::string::npos) return true;
    const std::regex object_pattern(R"(\{[^{}]*\})");
    const std::regex symbol_pattern(R"json("s"\s*:\s*"([^"]+)")json");
    const std::regex price_pattern(R"("p"\s*:\s*(-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)))");
    const std::regex timestamp_pattern(R"("t"\s*:\s*([0-9]+))");
    const std::regex volume_pattern(R"("v"\s*:\s*(-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)))");
    for (auto object = std::sregex_iterator(message.begin(), message.end(), object_pattern); object != std::sregex_iterator(); ++object) {
        const auto text = object->str();
        if (text.find("\"s\"") == std::string::npos) continue;
        std::smatch symbol_match, price_match, timestamp_match, volume_match;
        if (!std::regex_search(text, symbol_match, symbol_pattern) || !std::regex_search(text, price_match, price_pattern) ||
            !std::regex_search(text, timestamp_match, timestamp_pattern) || !std::regex_search(text, volume_match, volume_pattern)) return false;
        const auto symbol = symbol_match[1].str();
        if (std::find(config_.symbols.begin(), config_.symbols.end(), symbol) == config_.symbols.end()) continue;
        FinancialEvent event;
        event.type = FinancialEventType::Trade;
        event.instrument = {"US:FINNHUB:" + symbol, symbol, symbol, "US", "FINNHUB", "USD", "America/New_York", "", AssetClass::Equity, symbol};
        event.price = std::stod(price_match[1]);
        event.quantity = std::stod(volume_match[1]);
        event.source_timestamp = std::stoll(timestamp_match[1]);
        event.event_timestamp = event.source_timestamp / 1000;
        event.received_timestamp = unix_milliseconds() / 1000;
        event.source_id = source().id;
        event.source_uri = source().uri;
        event.quality = DataQuality::Verified;
        events.push_back(std::move(event));
    }
    return true;
}

StreamConnectionStatus FinnhubWebSocketSource::status() const { return status_; }
const StreamReport& FinnhubWebSocketSource::report() const { return report_; }
const std::string& FinnhubWebSocketSource::error() const { return error_; }

std::string stream_connection_status_name(StreamConnectionStatus status) {
    switch (status) {
        case StreamConnectionStatus::Connecting: return "CONNECTING";
        case StreamConnectionStatus::Authenticated: return "AUTHENTICATED";
        case StreamConnectionStatus::Subscribed: return "CONNECTED";
        case StreamConnectionStatus::Reconnecting: return "RECONNECTING";
        case StreamConnectionStatus::Stopping: return "STOPPING";
        default: return "DISCONNECTED";
    }
}

}  // namespace synthia::finance
