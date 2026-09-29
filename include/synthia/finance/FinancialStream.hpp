#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "synthia/finance/FinancialDataSource.hpp"

namespace synthia::finance {

enum class StreamConnectionStatus { Disconnected, Connecting, Authenticated, Subscribed, Reconnecting, Stopping };
enum class WebSocketReceiveResult { Message, Timeout, Disconnected, Error };

class WebSocketTransport {
public:
    virtual ~WebSocketTransport() = default;
    virtual bool connect(const std::string& endpoint, std::chrono::milliseconds timeout, std::string& error) = 0;
    virtual bool send_text(const std::string& message, std::string& error) = 0;
    virtual WebSocketReceiveResult receive_text(std::chrono::milliseconds timeout, std::string& message, std::string& error) = 0;
    virtual void close() noexcept = 0;
};

class OpenSslWebSocketTransport final : public WebSocketTransport {
public:
    OpenSslWebSocketTransport();
    ~OpenSslWebSocketTransport() override;

    bool connect(const std::string& endpoint, std::chrono::milliseconds timeout, std::string& error) override;
    bool send_text(const std::string& message, std::string& error) override;
    WebSocketReceiveResult receive_text(std::chrono::milliseconds timeout, std::string& message, std::string& error) override;
    void close() noexcept override;

private:
    bool send_frame(unsigned char opcode, const std::string& payload, std::string& error);
    bool read_exact(std::size_t size, std::chrono::milliseconds timeout, std::string& output, std::string& error);

    int socket_ = -1;
    int child_pid_ = -1;
    std::string input_;
};

struct FinnhubStreamConfig {
    std::string endpoint = "wss://ws.finnhub.io";
    std::string api_key;
    std::vector<std::string> symbols;
    std::chrono::milliseconds connect_timeout{10000};
    std::chrono::milliseconds receive_timeout{1000};
    std::chrono::milliseconds initial_reconnect_delay{250};
    std::chrono::milliseconds maximum_reconnect_delay{10000};
    unsigned int maximum_reconnect_attempts = 0;
};

struct StreamReport {
    std::size_t messages = 0;
    std::size_t events = 0;
    std::size_t malformed_messages = 0;
    std::size_t reconnects = 0;
};

class FinnhubWebSocketSource final : public FinancialDataSource {
public:
    using TransportFactory = std::function<std::unique_ptr<WebSocketTransport>()>;
    using EventHandler = std::function<void(const FinancialEvent&)>;

    explicit FinnhubWebSocketSource(FinnhubStreamConfig config, TransportFactory transport_factory = {});

    knowledge::Source source() const override;
    std::vector<FinancialRecord> fetch() override;
    DataDeliveryMode delivery_mode() const override { return DataDeliveryMode::Streaming; }
    bool run(const EventHandler& on_event, std::atomic_bool& stop_requested);
    StreamConnectionStatus status() const;
    const StreamReport& report() const;
    const std::string& error() const;

private:
    bool normalize_message(const std::string& message, std::vector<FinancialEvent>& events);
    std::string authenticated_endpoint() const;

    FinnhubStreamConfig config_;
    TransportFactory transport_factory_;
    StreamConnectionStatus status_ = StreamConnectionStatus::Disconnected;
    StreamReport report_;
    std::string error_;
};

std::string stream_connection_status_name(StreamConnectionStatus status);

}  // namespace synthia::finance