cli-about = The fastest, smallest AI assistant.
cli-no-command-provided = No command provided.
cli-try-onboard = Try `kylinbot onboard` to initialize your workspace.

cli-onboard-about = Initialize your workspace and configuration
cli-agent-about = Start the AI agent loop
cli-gateway-about = Manage the gateway server (webhooks, websockets)
cli-acp-about = Start the ACP server (JSON-RPC 2.0 over stdio)
cli-daemon-about = Start the long-running autonomous daemon
cli-service-about = Manage OS service lifecycle (launchd/systemd user service)
cli-doctor-about = Run diagnostics for daemon/scheduler/channel freshness
cli-status-about = Show system status (full details)
cli-estop-about = Engage, inspect, and resume emergency-stop states
cli-cron-about = Configure and manage scheduled tasks
cli-models-about = Manage provider model catalogs
cli-providers-about = List supported AI providers
cli-channel-about = Manage communication channels
cli-integrations-about = Browse 50+ integrations
cli-skills-about = Manage skills (user-defined capabilities)
cli-sop-about = Manage standard operating procedures (SOPs)
cli-migrate-about = Migrate data from other agent runtimes
cli-auth-about = Manage provider subscription authentication profiles
cli-hardware-about = Discover and introspect USB hardware
cli-peripheral-about = Manage hardware peripherals
cli-memory-about = Manage agent memory entries
cli-config-about = Manage KylinBot configuration
cli-update-about = Check for and apply KylinBot updates
cli-self-test-about = Run diagnostic self-tests
cli-completions-about = Generate shell completion scripts
cli-desktop-about = Launch the KylinBot companion desktop app

cli-config-schema-about = Dump the full configuration JSON Schema to stdout
cli-config-list-about = List all config properties with current values
cli-config-get-about = Get a config property value
cli-config-set-about = Set a config property (secret fields auto-prompt for masked input)
cli-config-init-about = Initialize unconfigured sections with defaults (enabled=false)
cli-config-migrate-about = Migrate config.toml to the current schema version on disk (preserves comments)

cli-service-install-about = Install daemon service unit for auto-start and restart
cli-service-start-about = Start daemon service
cli-service-stop-about = Stop daemon service
cli-service-restart-about = Restart daemon service to apply latest config
cli-service-status-about = Check daemon service status
cli-service-uninstall-about = Uninstall daemon service unit
cli-service-logs-about = Tail daemon service logs

cli-channel-list-about = List all configured channels
cli-channel-start-about = Start all configured channels
cli-channel-doctor-about = Run health checks for configured channels
cli-channel-add-about = Add a new channel configuration
cli-channel-remove-about = Remove a channel configuration
cli-channel-send-about = Send a one-off message to a configured channel
cli-wechat-pairing-required = 🔐 WeChat pairing required. One-time bind code: {$code}
cli-wechat-send-bind-command = Send `{$command} <code>` from your WeChat.
cli-wechat-qr-login = 📱 WeChat QR Login ({$attempt}/{$max})
cli-wechat-scan-to-connect = Scan with WeChat to connect.
cli-wechat-qr-url = QR URL: {$url}
cli-wechat-qr-expired-giving-up = WeChat QR code expired {$max} times, giving up.
cli-wechat-qr-fetch-failed = Failed to fetch WeChat QR code.
cli-wechat-qr-fetch-status-failed = WeChat QR code fetch failed ({$status}): {$body}
cli-wechat-missing-response-field = Missing {$field} in WeChat response.
cli-wechat-scanned-confirm = 👀 Scanned! Confirm on your phone...
cli-wechat-qr-expired-refreshing = ⏳ QR code expired, refreshing...
cli-wechat-login-confirmed-missing-field = Login confirmed but {$field} missing.
cli-wechat-connected = ✅ WeChat connected!
cli-wechat-bound-success = ✅ WeChat account bound successfully. You can talk to KylinBot now.
cli-wechat-invalid-bind-code = ❌ Invalid bind code. Please try again.

cli-skills-list-about = List all installed skills
cli-skills-audit-about = Audit a skill source directory or installed skill name
cli-skills-install-about = Install a new skill from a URL or local path
cli-skills-remove-about = Remove an installed skill
cli-skills-test-about = Run TEST.sh validation for a skill (or all skills)

cli-cron-list-about = List all scheduled tasks
cli-cron-add-about = Add a new recurring scheduled task
cli-cron-add-at-about = Add a one-shot task that fires at a specific UTC timestamp
cli-cron-add-every-about = Add a task that repeats at a fixed interval
cli-cron-once-about = Add a one-shot task that fires after a delay from now
cli-cron-remove-about = Remove a scheduled task
cli-cron-update-about = Update one or more fields of an existing scheduled task
cli-cron-pause-about = Pause a scheduled task
cli-cron-resume-about = Resume a paused task

cli-auth-login-about = Login with OAuth (OpenAI Codex or Gemini)
cli-auth-refresh-about = Refresh OpenAI Codex access token using refresh token
cli-auth-logout-about = Remove auth profile
cli-auth-use-about = Set active profile for a provider
cli-auth-list-about = List auth profiles
cli-auth-status-about = Show auth status with active profile and token expiry info

cli-memory-list-about = List memory entries with optional filters
cli-memory-get-about = Get a specific memory entry by key
cli-memory-stats-about = Show memory backend statistics and health
cli-memory-clear-about = Clear memories by category, by key, or clear all

cli-estop-status-about = Print current estop status
cli-estop-resume-about = Resume from an engaged estop level

cli-models-refresh-about = Refresh and cache provider models
cli-models-list-about = List cached models for a provider
cli-models-set-about = Set the default model in config
cli-models-status-about = Show current model configuration and cache status

cli-doctor-models-about = Probe model catalogs across providers and report availability
cli-doctor-traces-about = Query runtime trace events (tool diagnostics and model replies)

cli-hardware-discover-about = Enumerate USB devices and show known boards
cli-hardware-introspect-about = Introspect a device by its serial or device path
cli-hardware-info-about = Get chip info via USB using probe-rs over ST-Link

cli-peripheral-list-about = List configured peripherals
cli-peripheral-add-about = Add a peripheral by board type and transport path
cli-peripheral-flash-about = Flash KylinBot firmware to an Arduino board

cli-sop-list-about = List loaded SOPs
cli-sop-validate-about = Validate SOP definitions
cli-sop-show-about = Show details of an SOP

cli-migrate-openclaw-about = Import memory from an OpenClaw workspace into this KylinBot workspace

cli-agent-long-about =
    Start the AI agent loop.

    Launches an interactive chat session with the configured AI provider. Use --message for single-shot queries without entering interactive mode.

    Examples:
      kylinbot agent                              # interactive session
      kylinbot agent -m "Summarize today's logs"  # single message
      kylinbot agent -p anthropic --model claude-sonnet-4-20250514
      kylinbot agent --peripheral nucleo-f401re:/dev/ttyACM0

cli-gateway-long-about =
    Manage the gateway server (webhooks, websockets).

    Start, restart, or inspect the HTTP/WebSocket gateway that accepts incoming webhook events and WebSocket connections.

    Examples:
      kylinbot gateway start              # start gateway
      kylinbot gateway restart            # restart gateway
      kylinbot gateway get-paircode       # show pairing code

cli-acp-long-about =
    Start the ACP server (JSON-RPC 2.0 over stdio).

    Launches a JSON-RPC 2.0 server on stdin/stdout for IDE and tool integration. Supports session management and streaming agent responses as notifications.

    Methods: initialize, session/new, session/prompt, session/stop.

    Examples:
      kylinbot acp                        # start ACP server
      kylinbot acp --max-sessions 5       # limit concurrent sessions

cli-daemon-long-about =
    Start the long-running autonomous daemon.

    Launches the full KylinBot runtime: gateway server, all configured channels (Telegram, Discord, Slack, etc.), heartbeat monitor, and the cron scheduler. This is the recommended way to run KylinBot in production or as an always-on assistant.

    Use 'kylinbot service install' to register the daemon as an OS service (systemd/launchd) for auto-start on boot.

    Examples:
      kylinbot daemon                   # use config defaults
      kylinbot daemon -p 9090           # gateway on port 9090
      kylinbot daemon --host 127.0.0.1  # localhost only

cli-cron-long-about =
    Configure and manage scheduled tasks.

    Schedule recurring, one-shot, or interval-based tasks using cron expressions, RFC 3339 timestamps, durations, or fixed intervals.

    Cron expressions use the standard 5-field format: 'min hour day month weekday'. Timezones default to UTC; override with --tz and an IANA timezone name.

    Examples:
      kylinbot cron list
      kylinbot cron add '0 9 * * 1-5' 'Good morning' --tz America/New_York --agent
      kylinbot cron add '*/30 * * * *' 'Check system health' --agent
      kylinbot cron add '*/5 * * * *' 'echo ok'
      kylinbot cron add-at 2025-01-15T14:00:00Z 'Send reminder' --agent
      kylinbot cron add-every 60000 'Ping heartbeat'
      kylinbot cron once 30m 'Run backup in 30 minutes' --agent
      kylinbot cron pause TASK_ID
      kylinbot cron update TASK_ID --expression '0 8 * * *' --tz Europe/London

cli-channel-long-about =
    Manage communication channels.

    Add, remove, list, send, and health-check channels that connect KylinBot to messaging platforms. Supported channel types: telegram, discord, slack, whatsapp, matrix, imessage, email.

    Examples:
      kylinbot channel list
      kylinbot channel doctor
      kylinbot channel add telegram '{ "{" }"bot_token":"...","name":"my-bot"{ "}" }'
      kylinbot channel remove my-bot
      kylinbot channel bind-telegram kylinbot_user
      kylinbot channel send 'Alert!' --channel-id telegram --recipient 123456789

cli-hardware-long-about =
    Discover and introspect USB hardware.

    Enumerate connected USB devices, identify known development boards (STM32 Nucleo, Arduino, ESP32), and retrieve chip information via probe-rs / ST-Link.

    Examples:
      kylinbot hardware discover
      kylinbot hardware introspect /dev/ttyACM0
      kylinbot hardware info --chip STM32F401RETx

cli-peripheral-long-about =
    Manage hardware peripherals.

    Add, list, flash, and configure hardware boards that expose tools to the agent (GPIO, sensors, actuators). Supported boards: nucleo-f401re, rpi-gpio, esp32, arduino-uno.

    Examples:
      kylinbot peripheral list
      kylinbot peripheral add nucleo-f401re /dev/ttyACM0
      kylinbot peripheral add rpi-gpio native
      kylinbot peripheral flash --port /dev/cu.usbmodem12345
      kylinbot peripheral flash-nucleo

cli-memory-long-about =
    Manage agent memory entries.

    List, inspect, and clear memory entries stored by the agent. Supports filtering by category and session, pagination, and batch clearing with confirmation.

    Examples:
      kylinbot memory stats
      kylinbot memory list
      kylinbot memory list --category core --limit 10
      kylinbot memory get KEY
      kylinbot memory clear --category conversation --yes

cli-config-long-about =
    Manage KylinBot configuration.

    View, set, or initialize config properties by dotted path. Use 'schema' to dump the full JSON Schema for the config file.

    Properties are addressed by dotted path (e.g. channels.matrix.mention-only).
    Secret fields (API keys, tokens) automatically use masked input.
    Enum fields offer interactive selection when value is omitted.

    Examples:
      kylinbot config list                                  # list all properties
      kylinbot config list --secrets                        # list only secrets
      kylinbot config list --filter channels.matrix         # filter by prefix
      kylinbot config get channels.matrix.mention-only      # get a value
      kylinbot config set channels.matrix.mention-only true # set a value
      kylinbot config set channels.matrix.access-token      # secret: masked input
      kylinbot config set channels.matrix.stream-mode       # enum: interactive select
      kylinbot config init channels.matrix                  # init section with defaults
      kylinbot config schema                                # print JSON Schema to stdout
      kylinbot config schema > schema.json

    Property path tab completion is included automatically in `kylinbot completions <shell>`.

cli-update-long-about =
    Check for and apply KylinBot updates.

    By default, downloads and installs the latest release with a 6-phase pipeline: preflight, download, backup, validate, swap, and smoke test. Automatic rollback on failure.

    Use --check to only check for updates without installing.
    Use --force to skip the confirmation prompt.
    Use --version to target a specific release instead of latest.

    Examples:
      kylinbot update                      # download and install latest
      kylinbot update --check              # check only, don't install
      kylinbot update --force              # install without confirmation
      kylinbot update --version 0.6.0      # install specific version

cli-self-test-long-about =
    Run diagnostic self-tests to verify the KylinBot installation.

    By default, runs the full test suite including network checks (gateway health, memory round-trip). Use --quick to skip network checks for faster offline validation.

    Examples:
      kylinbot self-test             # full suite
      kylinbot self-test --quick     # quick checks only (no network)

cli-completions-long-about =
    Generate shell completion scripts for `kylinbot`.

    The script is printed to stdout so it can be sourced directly:

    Examples:
      source <(kylinbot completions bash)
      kylinbot completions zsh > ~/.zfunc/_kylinbot
      kylinbot completions fish > ~/.config/fish/completions/kylinbot.fish

cli-desktop-long-about =
    Launch the KylinBot companion desktop app.

    The companion app is a lightweight menu bar / system tray application that connects to the same gateway as the CLI. It provides quick access to the dashboard, status monitoring, and device pairing.

    Use --install to download the pre-built companion app for your platform.

    Examples:
      kylinbot desktop              # launch the companion app
      kylinbot desktop --install    # download and install it

# Channel-side reply emitted when chat dispatch refuses because the
# gateway has no model configured. Used by the gateway crate channel
# webhook handlers (WhatsApp, Linq, WATI, Nextcloud Talk).
channel-needs-onboarding-reply = This agent isn't fully set up yet. The operator needs to complete onboarding before I can reply.

## SkillHub gateway API messages
skillhub-installed-successfully = Skill installed successfully
skillhub-uninstalled-successfully = Skill uninstalled successfully
skillhub-install-task-failed = Install task failed
skillhub-uninstall-task-failed = Uninstall task failed
skillhub-upgrade-task-failed = Upgrade task failed
skillhub-query-param-required = Query parameter '{$param}' is required
skillhub-field-required = Field '{$field}' is required
skillhub-invalid-skill-slug = Invalid skill slug
skillhub-search-failed = Search failed: {$error}
skillhub-showcase-failed = Showcase failed: {$error}
skillhub-upgrade-failed = Upgrade failed: {$error}
skillhub-detail-failed = Detail failed: {$error}
skillhub-check-complete = Check complete
skillhub-upgrade-complete = Upgrade complete
