# Logs & Observability

KylinBot emits structured logs, Prometheus metrics, and OpenTelemetry traces. All three are on by default in a release build.

## Logs

### Where they go

| Platform | Default destination |
|---|---|
| Linux (systemd) | journald — `journalctl --user -u kylinbot` |
| macOS (launchd) | `~/Library/Logs/KylinBot/kylinbot.log` (stdout), `kylinbot.err` (stderr) |
| Homebrew on macOS | `$HOMEBREW_PREFIX/var/log/kylinbot.log` |
| Windows | `%LOCALAPPDATA%\KylinBot\logs\kylinbot.log` |
| Docker | container stdout — `docker logs kylinbot` |
| Foreground (`kylinbot daemon` without service) | stderr |

### Levels

Set via the `RUST_LOG` env var. Examples:

```bash
RUST_LOG=kylinbot=info                       # default — high-signal events
RUST_LOG=kylinbot=debug                      # verbose — per tool call, per provider call
RUST_LOG=kylinbot::agent=trace               # very verbose — just the agent loop
RUST_LOG=warn,kylinbot::security=debug       # quiet except security subsystem
```

For persistent changes, put the value in your service unit:

```ini
# ~/.config/systemd/user/kylinbot.service.d/override.conf
[Service]
Environment=RUST_LOG=kylinbot=info,kylinbot::security=debug
```

### Format

JSON by default in service mode (easier for Loki/ELK ingestion). Pretty-print on a TTY (interactive `kylinbot daemon`).

Force one or the other:

```toml
[observability]
log_format = "json"        # or "pretty"
```

### What's logged at `info`

- Service lifecycle (start, stop, config reload)
- Channel connect / disconnect events
- Provider calls with latency and token counts
- Tool calls (name, outcome — success / blocked / denied) — *not* arguments or results, which are sensitive
- Approval requests, grants, denials

At `debug`:

- Full tool args and results (redacted — credentials stripped)
- Every streaming chunk
- Memory retrieval scores
- Security-policy evaluation details

At `trace`:

- Model context window construction
- Per-token streaming events
- All HTTP request/response headers (still credential-redacted)

### Credential redaction

The logger redacts known secret patterns (`sk-*`, `ghp_*`, `xox[baprs]-*`, `ya29.*`, `AIza*`, etc.) regardless of log level. Redaction happens at the logger layer — your log files and the journal never see them.

Audit this with:

```bash
grep -E 'sk-|ghp_|xox[baprs]' /path/to/log/file | head
# should return zero results in a normal run
```

If you see an unredacted secret, file an issue — the redaction list is in `crates/kylinbot-infra/src/redact.rs`.

## Metrics

Prometheus exposition on the gateway:

```
curl -s http://localhost:42617/metrics
```

Key metrics:

| Metric | Labels | What it measures |
|---|---|---|
| `kylinbot_provider_calls_total` | `provider`, `outcome` | Provider calls by outcome (ok / timeout / error) |
| `kylinbot_provider_latency_ms` | `provider` | Histogram of provider call latency |
| `kylinbot_tokens_total` | `provider`, `kind` (input/output) | Token counters |
| `kylinbot_tool_calls_total` | `tool`, `outcome` | Tool invocations by outcome |
| `kylinbot_tool_duration_ms` | `tool` | Tool-execution histogram |
| `kylinbot_channel_events_total` | `channel`, `direction` (inbound/outbound) | Message flow |
| `kylinbot_channel_errors_total` | `channel`, `kind` | Disconnects, rate limits, auth failures |
| `kylinbot_memory_searches_total` |  | Memory retrieval calls |
| `kylinbot_policy_blocks_total` | `policy`, `tool` | Security-policy denials |

A minimal Prometheus scrape config:

```yaml
scrape_configs:
  - job_name: kylinbot
    static_configs:
      - targets: ["localhost:42617"]
```

The [Grafana dashboard](https://github.com/kylinbot/kylinbot-templates/tree/master/grafana) (in the templates repo) visualises the above.

## Traces

OpenTelemetry over OTLP/HTTP. Off by default — enable in config:

```toml
[observability.otel]
enabled = true
endpoint = "http://localhost:4318"
service_name = "kylinbot"
```

Spans you'll see:

- `agent.loop` — one per inbound message, covers the whole turn
- `provider.chat` — a provider call, with attributes for model, token counts, and retry count
- `tool.invoke` — a tool call, with outcome and duration
- `security.validate` — policy check with decision
- `memory.search` — retrieval with hit count and max score

Pair with Jaeger or Tempo for distributed tracing if you run multiple KylinBot instances or are instrumenting downstream services.

## Receipts audit log

Separate from the general logs, tool receipts are written to:

```
<workspace>/receipts/<yyyy-mm-dd>.ndjson
```

One JSON line per tool invocation. Greppable, append-only, persistent across restarts. See [Tool receipts](../security/tool-receipts.md).

## Cost & token tracking

Every gateway-served turn (WebSocket chat, channel webhook, simple webhook) records its token usage and computed cost to:

```
<workspace>/state/costs.jsonl
```

One JSON line per LLM call, with input/output token counts and cost in USD. The accumulator behind `GET /api/cost` reads from the same source — call it any time for a session, daily, and monthly summary.

Per-model pricing comes from `[cost.prices]` in `config.toml`. Models without an entry record token counts with a zero cost; budget enforcement still works on the recorded counts. The WebSocket `done` frame echoes the same numbers so browser clients can show usage without a second round-trip:

```json
{
  "type": "done",
  "full_response": "...",
  "input_tokens": 142,
  "output_tokens": 87,
  "tokens_used": 229,
  "cost_usd": 0.000456,
  "model": "claude-sonnet-4-20250514",
  "provider": "anthropic"
}
```

Token fields are `null` when the upstream provider does not surface usage in streaming responses (most OpenAI-compatible providers do; Anthropic streaming reports usage too, but only via its native event format — non-OpenAI-shaped providers may not).

## Health endpoints

The gateway exposes three health views:

```bash
curl -s http://localhost:42617/health              # { "status": "ok", "version": "0.7.4" }
curl -s http://localhost:42617/health/channels     # per-channel status
curl -s http://localhost:42617/health/providers    # per-provider status + error rate
```

Point Uptime Kuma / your monitor at `/health` for a binary liveness check.

## Log retention

KylinBot doesn't rotate its own logs — the OS handles that:

- systemd-journald rotates by size and age (`/etc/systemd/journald.conf`)
- launchd / macOS ASL rotates daily by default
- Docker's default `json-file` driver rotates at 10 MB with 3 retained files; configure in daemon.json

For the receipts log, rotate manually or via logrotate if the file grows faster than you want to retain:

```
<workspace>/receipts/2026-04-25.ndjson
<workspace>/receipts/2026-04-26.ndjson
...
```

Day-sharded means you can drop old files individually.

## See also

- [Operations → Overview](./overview.md)
- [Troubleshooting](./troubleshooting.md)
- [Security → Tool receipts](../security/tool-receipts.md)
