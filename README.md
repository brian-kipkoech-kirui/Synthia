# SYNTHIA

SYNTHIA is a C++-first financial data, modeling, and intelligence platform. It ingests heterogeneous financial observations, preserves provenance, projects normalized data into a financial WorldState, and exposes evidence to a human research terminal. The deterministic synthetic-world runtime remains as a compatibility and simulation subsystem while financial analysis becomes the primary product direction.
Project status: Financial Data Fabric, delayed Yahoo polling boundary, deterministic ingestion replay, and financial terminal available. A WebSocket real-time provider and advanced analysis are not implemented yet.
## Vision

Observe markets, validate evidence, model uncertainty, analyze risk, and explain what the data supports.
The current `StaticFinancialDataSource` is deterministic test infrastructure, not a claim of live market data. Yahoo Finance chart HTTP polling is an optional delayed-data adapter. Future streaming exchange, economic, and licensed news adapters should implement `FinancialDataSource` independently; no provider is authoritative by architectural assumption.
SYNTHIA combines:

- financial data ingestion
The first external adapter is `YahooFinanceSource`, using Yahoo's chart endpoint through a bounded local `curl` process and intraday chart intervals. Data is labeled `delayed`, retains provider timestamps, and is not a WebSocket or exchange-real-time feed. `FileFinancialSource` provides an independent CSV adapter for licensed exports and deterministic fixtures. CSV rows use:
- source freshness and provenance
- financial WorldState
- future analysis, valuation, risk, and model laboratories

## Core concept
Run an explicit live snapshot request:
The current product boundary is a financial data fabric that can:

- ingest market observations through provider adapters
./build/synthia_finance --mode polling --symbols AAPL,MSFT --interval-seconds 60 --record ingestion_session.json
./build/synthia_finance --mode replay --replay ingestion_session.json
- distinguish historical, delayed, and provider-defined freshness
Configuration is documented in `.env.example`; environment variables are consumed by the CLI after they are exported by the caller. Yahoo does not require an API key. Network availability and Yahoo terms/rate limits apply. A recorded session contains ordered normalized observations and can be replayed by `FinancialIngestionSession` without network access.
- deduplicate and project records into knowledge::WorldState
Finance API endpoints exposed by the dashboard include `GET /api/finance/world` for current interpreted values and freshness, `GET /api/finance/markets` for accepted normalized records, `/instruments`, `/quotes`, `/history`, `/sources`, and `/ingestion`, plus `POST /api/finance/ingest` with form fields `symbol` and `period`. The Financial World panel supports explicit snapshots and browser-driven polling. The dashboard labels source, provider timestamp, observed time, received time, latency, currency, exchange, and quality where available.

The first financial-intelligence layer is a provider-neutral C++ ingestion boundary in `include/synthia/finance`. `FinancialDataSource` adapters normalize market, fundamental, macro, and news records into `FinancialDataFabric`. Each record carries canonical instrument or region metadata, observed/received timestamps, source URI, latency, and data quality. Invalid records are rejected, duplicate canonical keys are counted, and accepted records are projected into the existing provenance-aware `knowledge::WorldState`. Canonical trade, quote, price-update, and volume-update events update a compact current market interpretation while preserving ordered raw observations separately for replay.

See [docs/financial-data-fabric.md](docs/financial-data-fabric.md) for environment setup, replay, the finance API, freshness semantics, and provider limitations.
```text
SYNTHIA/
├── README.md
├── LICENSE
├── CONTRIBUTING.md
├── .gitignore
├── engine/
├── world/
├── simulation/
├── agents/
├── sensors/
├── scenarios/
├── datasets/
├── ai/
├── backend/
├── frontend/
├── tools/
├── tests/
├── docs/
└── .github/
```

## Architecture summary

```text
External financial sources
         |
    FinancialDataSource adapters
         |
     FinancialDataFabric
         |
      Financial WorldState
         |
 Data -> Analysis -> Models -> Risk -> Explanation
         |
    Financial Terminal
```

## Initial technical direction

SYNTHIA is built as a headless-first financial data platform with a lightweight browser terminal.

- Core data and world state: C++
- Ingestion: provider-neutral FinancialDataSource adapters
- Validation: FinancialDataFabric with deterministic keys and provenance
- Replay: normalized ingestion sessions without network access
- Browser: HTML/CSS/JavaScript visualization and interaction only
- Future boundaries: analysis, valuation, fixed income, portfolio, risk, models, and explanation

## Roadmap

1. Financial Data Fabric (v0.5.0)
2. Live multi-source ingestion and replay (v0.5.1)
3. Financial news and sentiment provenance (v0.5.2)
4. Quantitative features and indicators (v0.5.3)
5. Market regimes and model laboratory (v0.5.4-v0.5.5)
6. Valuation, portfolio, risk, scenarios, and explanation

## Documentation

See the docs directory for the technical design discussion:

- [docs/vision.md](docs/vision.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/world-model.md](docs/world-model.md)
- [docs/agents.md](docs/agents.md)
- [docs/sensors.md](docs/sensors.md)
- [docs/scenarios.md](docs/scenarios.md)
- [docs/roadmap.md](docs/roadmap.md)

## Status

Project status: Financial Data Fabric, delayed Yahoo fallback polling, Finnhub WebSocket adapter, deterministic ingestion replay, and financial terminal available. Advanced analysis is intentionally not implemented yet.

## Financial Data Fabric

The first financial-intelligence layer is now a provider-neutral C++ ingestion boundary in `include/synthia/finance`. `FinancialDataSource` adapters normalize market, fundamental, macro, and news records into `FinancialDataFabric`. Each record carries canonical instrument or region metadata, observed/received timestamps, source URI, latency, and data quality. Invalid records are rejected, duplicate canonical keys are counted, and accepted records are projected into the existing provenance-aware `knowledge::WorldState`.

The current `StaticFinancialDataSource` is deterministic test infrastructure, not a claim of live market data. Yahoo Finance chart HTTP polling is the delayed snapshot/fallback adapter; `FinnhubWebSocketSource` is the streaming adapter and normalizes provider messages before passing canonical events to the fabric. Future economic and licensed news adapters should implement `FinancialDataSource` independently; no provider is authoritative by architectural assumption.

### v0.5.1 live ingestion

`YahooFinanceSource` uses Yahoo's chart endpoint through a bounded local `curl` process. Its data is labeled `delayed` and retains provider timestamps; it is not equivalent to an exchange-grade real-time feed. `FinnhubWebSocketSource` provides one documented streaming provider and normalizes trades into SYNTHIA canonical events. `FileFinancialSource` provides an independent CSV adapter for licensed exports and deterministic fixtures. CSV rows use:

```text
symbol,country,exchange,currency,observed_at,open,high,low,close,volume
```

Run an explicit live snapshot request:

```sh
./build/synthia_finance --provider yahoo --symbol AAPL --period 1d --record ingestion_session.json
```

Yahoo snapshot mode does not require an API key. Network availability and Yahoo terms/rate limits apply. A recorded session contains ordered normalized observations and can be replayed by `FinancialIngestionSession` without network access.

Finnhub streaming requires runtime environment variables and OpenSSL `s_client`; the credential-free example is in `.env.example`:

```sh
export SYNTHIA_FINANCE_PROVIDER=finnhub
export SYNTHIA_FINANCE_MODE=streaming
export SYNTHIA_FINANCE_API_KEY="$FINNHUB_API_KEY"
export SYNTHIA_FINANCE_SYMBOLS=AAPL,MSFT
./build/synthia_finance --provider finnhub --mode streaming --record live_session.json
./build/synthia_finance --mode replay --replay live_session.json
./build/synthia_finance_live_test live_session.json
```

The last executable is a manual live integration test, excluded from mandatory CTest. It waits up to 30 seconds for a real event and confirms projection into `WorldState`.

Finance API endpoints exposed by the dashboard include `GET /api/finance/world` for current state/freshness, `GET /api/finance/markets` for normalized observations, plus `POST /api/finance/ingest` for Yahoo snapshots and `/api/finance/stream/start` and `/api/finance/stream/stop` for Finnhub streaming. The dashboard process reads the Finnhub key, endpoint, and symbols from its environment. The page shows connection status, source, symbol, price, bid/ask, volume, event timestamp, and freshness.

```text
FinancialDataSource adapters
    | normalized records + provenance
    v
FinancialDataFabric
    | validation / deduplication
    v
knowledge::WorldState
    | entities + observations + sources
    v
Web Observatory and future model laboratory
```

## Web Observatory

Build and launch the local observatory:

```sh
cmake -S . -B build
cmake --build build -j2
./build/synthia_dashboard
```

Open [http://localhost:8080](http://localhost:8080). The browser is a visualization and control layer; the C++ `World` remains authoritative for movement, environment, entities, events, and knowledge.

Available scenarios are `multi_agent_nav`, `resource_collection`, and `physics_survival`. Select a scenario and deterministic seed in the toolbar, then use Start, Pause, Step, or Reset. Polling speed changes browser refresh frequency only and does not alter simulation determinism.

The local HTTP API exposes:

- `GET /api/state` - complete serialized native world state
- `GET /api/agents` - native agent entities
- `GET /api/entities` - native non-agent entities
- `GET /api/events` - native world events
- `GET /api/knowledge` - knowledge entities, events, observations, relationships, and sources
- `POST /api/simulation/start`, `/pause`, `/step`, `/reset` - simulation controls
- `POST /api/scenario/load` - form fields `scenario` and `seed`

Architecture:

```text
World / Physics / Agents / Events / Knowledge (C++)
             |
         native JSON / HTTP
             |
         Web Observatory (HTML/CSS/JS)
```

The observatory has no simulation model of its own. Canvas positions, event feed, inspectors, and knowledge panels are rendered directly from API responses. The existing headless executable and deterministic test suite remain independent of the optional web process.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
