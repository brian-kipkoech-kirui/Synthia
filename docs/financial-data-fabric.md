# Financial Data Fabric v0.5.1

## Architecture

`FinancialDataSource` keeps snapshot providers source-compatible and adds an optional delivery-mode declaration plus an optional canonical event batch. Yahoo Finance remains the bounded delayed HTTP polling/snapshot fallback. `FinnhubWebSocketSource` is the single streaming provider and converts its documented trade messages into canonical `FinancialEvent`s before they reach the fabric. Provider parsing and transport live in `FinancialStream.cpp`, not the core financial model.

Finnhub WebSocket endpoint and trade/subscription message format: [Finnhub WebSocket API documentation](https://finnhub.io/docs/api/websocket-trades).

```text
Yahoo chart API / Finnhub WebSocket / CSV fixture
                  |
       FinancialDataSource
        snapshot / polling / streaming
                  |
          FinancialDataFabric
     validation / deterministic dedup
             /             \
   observation history    latest market state
             |                   |
       session replay       knowledge::WorldState
                                   |
                       GET /api/finance/world
                                   |
                         Web Observatory
```

Canonical market events currently include trade, quote, price update, and volume update. Each accepted event becomes a `MarketRecord` with instrument identity and provenance. Validation rejects malformed symbols, non-finite or negative values, crossed quotes, invalid timestamps, and missing provider provenance. Duplicate event keys are ignored. Older late arrivals remain in observation history but do not roll the current instrument entity backward.

The raw accepted input sequence is kept separately from the deduplication map and the per-instrument current snapshot. Financial sessions serialize events in input order, including event and receive timestamps, provider/source URI, latency, quality, instrument metadata, prices, quote values, and quantities. Replay uses that stored order and does not contact an external provider.

## Configuration

Use `.env.example` as a reference and export variables in the launching process; SYNTHIA does not load `.env` files automatically. For Finnhub, set `SYNTHIA_FINANCE_PROVIDER=finnhub`, `SYNTHIA_FINANCE_MODE=streaming`, `SYNTHIA_FINANCE_API_KEY`, and `SYNTHIA_FINANCE_SYMBOLS`. `SYNTHIA_FINANCE_ENDPOINT` optionally overrides the secure endpoint. The key is only appended to the TLS WebSocket endpoint at runtime and is never included in source metadata or logs. Yahoo fallback does not require an API key.

```sh
./build/synthia_finance --mode snapshot --symbols AAPL,MSFT --period 1d --interval 1m --record session.json
./build/synthia_finance --mode polling --symbols AAPL,MSFT --interval-seconds 60 --record session.json
SYNTHIA_FINANCE_PROVIDER=finnhub SYNTHIA_FINANCE_MODE=streaming SYNTHIA_FINANCE_API_KEY="$FINNHUB_API_KEY" SYNTHIA_FINANCE_SYMBOLS=AAPL,MSFT ./build/synthia_finance --provider finnhub --mode streaming --record session.json
./build/synthia_finance --mode replay --replay session.json
./build/synthia_finance_live_test live_session.json
```

Yahoo polling requests are bounded to 15 seconds and retry after a bounded delay when a request fails. Send SIGINT or SIGTERM to stop CLI polling or streaming and save the requested session. `synthia_finance_live_test` is manual-only, waits up to 30 seconds for an actual provider event, and is not registered with CTest. The production TLS transport uses the installed OpenSSL `s_client`; the transport itself is injectable for offline tests. Provider permissions, coverage, terms, and rate limits apply. Tests, CSV ingestion, and replay do not need network access.

## Dashboard

The existing dashboard exposes:

- `GET /api/finance/world`: current per-symbol last price, bid, ask, volume, spread, mid-price, price change, event/receive times, latency, source, quality, and freshness/connection status.
- `GET /api/finance/markets`: accepted normalized market records with instrument and provenance fields.
- `GET /api/finance/ingestion`: last request report and provider error, if any.
- `POST /api/finance/ingest`: form fields `symbol` and `period` for an explicit Yahoo snapshot.
- `POST /api/finance/stream/start` and `/api/finance/stream/stop`: connect/disconnect the Finnhub stream using the dashboard process environment.

The Financial World panel supports Yahoo snapshots/polling and explicit Finnhub connect/disconnect. It never contacts a provider until requested. `LIVE`, `STALE`, and `DISCONNECTED` labels use receive-time freshness thresholds. Yahoo chart polling may be delayed and is not equivalent to an exchange-grade real-time feed.

## Limitations and next step

Offline mock tests cover connection/authentication setup, subscriptions, normalized trades, malformed messages, duplicates, reconnect, provider failure, and shutdown. A real Finnhub market event was not verified in this environment because no runtime API key is configured. Run the manual integration executable with a Finnhub key and verify it reports `world_state_updated=yes`; the dashboard can use the same environment configuration. Current derived values are limited to spread, mid-price, price change, and realized volume; there is no quantitative research or execution system.