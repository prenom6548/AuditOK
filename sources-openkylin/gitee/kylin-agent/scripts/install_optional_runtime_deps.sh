#!/usr/bin/env bash
set -euo pipefail

HERMES_HOME="${HERMES_HOME:-${AGENT_RUNTIME_HOME:-$HOME/.kylin-agent-runtime}}"
if [ -n "${HERMES_INSTALL_DIR:-}" ]; then
    INSTALL_DIR="$HERMES_INSTALL_DIR"
elif [ -d "$HERMES_HOME/agent-runtime" ]; then
    INSTALL_DIR="$HERMES_HOME/agent-runtime"
elif [ -d "/usr/local/lib/agent-runtime" ]; then
    INSTALL_DIR="/usr/local/lib/agent-runtime"
else
    INSTALL_DIR="$HERMES_HOME/agent-runtime"
fi
TIMEOUT_SECONDS="${OPTIONAL_DEPS_TIMEOUT:-600}"
INSTALL_BROWSER=true
INSTALL_TUI=true
INSTALL_PLAYWRIGHT=true

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Install optional Kylin Agent runtime dependencies that the desktop-managed
runtime installer may skip or time out:
  - browser-tool Node.js dependencies from the runtime root package.json
  - Playwright Chromium for browser automation
  - terminal TUI dependencies from ui-tui/package.json

Options:
  --dir PATH          Runtime source directory
                      default: \$HERMES_INSTALL_DIR, \$HERMES_HOME/agent-runtime,
                      or /usr/local/lib/agent-runtime when present
  --hermes-home PATH  Runtime data directory
                      default: \$HERMES_HOME, \$AGENT_RUNTIME_HOME, or ~/.kylin-agent-runtime
  --timeout SECONDS   Per-command timeout, default: ${TIMEOUT_SECONDS}
  --browser-only      Install browser-tool dependencies only
  --tui-only          Install terminal TUI dependencies only
  --skip-browser      Skip browser-tool dependencies and Playwright
  --skip-playwright   Skip Playwright Chromium only
  --skip-tui          Skip terminal TUI dependencies
  -h, --help          Show this help

Environment:
  OPTIONAL_DEPS_TIMEOUT  Default timeout in seconds
  NPM_CONFIG_REGISTRY    Optional npm registry mirror
EOF
}

log_info() {
    printf '[INFO] %s\n' "$1"
}

log_warn() {
    printf '[WARN] %s\n' "$1" >&2
}

log_error() {
    printf '[ERROR] %s\n' "$1" >&2
}

run_with_timeout() {
    local timeout_seconds="$1"
    shift

    case "$timeout_seconds" in
        ''|*[!0-9]*) timeout_seconds=600 ;;
    esac

    if command -v timeout >/dev/null 2>&1; then
        if timeout --foreground -k 10 1 true >/dev/null 2>&1; then
            timeout --foreground -k 10 "$timeout_seconds" "$@"
        else
            timeout "$timeout_seconds" "$@"
        fi
        return $?
    fi
    if command -v gtimeout >/dev/null 2>&1; then
        if gtimeout --foreground -k 10 1 true >/dev/null 2>&1; then
            gtimeout --foreground -k 10 "$timeout_seconds" "$@"
        else
            gtimeout "$timeout_seconds" "$@"
        fi
        return $?
    fi

    set -m
    ( "$@" ) &
    local cmd_pid=$!
    set +m

    local waited=0
    local rc=0
    while [ "$waited" -lt "$timeout_seconds" ]; do
        if ! kill -0 "$cmd_pid" 2>/dev/null; then
            wait "$cmd_pid" 2>/dev/null || rc=$?
            return "$rc"
        fi
        sleep 1
        waited=$((waited + 1))
    done

    kill -TERM "-$cmd_pid" 2>/dev/null || kill -TERM "$cmd_pid" 2>/dev/null || true
    sleep 2
    kill -KILL "-$cmd_pid" 2>/dev/null || kill -KILL "$cmd_pid" 2>/dev/null || true
    wait "$cmd_pid" 2>/dev/null || true
    return 124
}

find_system_browser() {
    local candidate
    for candidate in \
        google-chrome-stable google-chrome chromium chromium-browser microsoft-edge \
        firefox firefox-esr; do
        if command -v "$candidate" >/dev/null 2>&1; then
            command -v "$candidate"
            return 0
        fi
    done
    return 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        --hermes-home)
            HERMES_HOME="$2"
            if [ -z "${HERMES_INSTALL_DIR:-}" ]; then
                INSTALL_DIR="$HERMES_HOME/agent-runtime"
            fi
            shift 2
            ;;
        --timeout)
            TIMEOUT_SECONDS="$2"
            shift 2
            ;;
        --browser-only)
            INSTALL_BROWSER=true
            INSTALL_PLAYWRIGHT=true
            INSTALL_TUI=false
            shift
            ;;
        --tui-only)
            INSTALL_BROWSER=false
            INSTALL_PLAYWRIGHT=false
            INSTALL_TUI=true
            shift
            ;;
        --skip-browser)
            INSTALL_BROWSER=false
            INSTALL_PLAYWRIGHT=false
            shift
            ;;
        --skip-playwright)
            INSTALL_PLAYWRIGHT=false
            shift
            ;;
        --skip-tui)
            INSTALL_TUI=false
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

if ! command -v npm >/dev/null 2>&1; then
    log_error "npm not found. Install Node.js/npm first, or run agent_runtime_install.sh --ensure node."
    exit 1
fi

if [ ! -d "$INSTALL_DIR" ]; then
    log_error "Runtime directory not found: $INSTALL_DIR"
    exit 1
fi

export HERMES_HOME
export AGENT_RUNTIME_HOME="$HERMES_HOME"

failures=0

if [ "$INSTALL_BROWSER" = true ]; then
    if [ -f "$INSTALL_DIR/package.json" ]; then
        log_info "Installing browser-tool Node.js dependencies..."
        if run_with_timeout "$TIMEOUT_SECONDS" npm --prefix "$INSTALL_DIR" install; then
            log_info "Browser-tool Node.js dependencies installed"
        else
            log_warn "Browser-tool Node.js dependency install failed or timed out"
            failures=$((failures + 1))
        fi
    else
        log_warn "Skipping browser-tool Node.js dependencies: $INSTALL_DIR/package.json not found"
    fi
fi

if [ "$INSTALL_PLAYWRIGHT" = true ]; then
    if find_system_browser >/dev/null 2>&1; then
        log_info "System browser found; Playwright bundled Chromium is optional."
    fi

    if [ -f "$INSTALL_DIR/package.json" ]; then
        log_info "Installing Playwright Chromium..."
        if run_with_timeout "$TIMEOUT_SECONDS" \
            bash -c 'cd "$1" && npx playwright install chromium' _ "$INSTALL_DIR"; then
            log_info "Playwright Chromium installed"
        else
            log_warn "Playwright Chromium install failed or timed out"
            failures=$((failures + 1))
        fi
    else
        log_warn "Skipping Playwright Chromium: $INSTALL_DIR/package.json not found"
    fi
fi

if [ "$INSTALL_TUI" = true ]; then
    if [ -f "$INSTALL_DIR/ui-tui/package.json" ]; then
        log_info "Installing terminal TUI dependencies..."
        if run_with_timeout "$TIMEOUT_SECONDS" npm --prefix "$INSTALL_DIR/ui-tui" install; then
            log_info "Terminal TUI dependencies installed"
        else
            log_warn "Terminal TUI dependency install failed or timed out"
            failures=$((failures + 1))
        fi
    else
        log_warn "Skipping terminal TUI dependencies: $INSTALL_DIR/ui-tui/package.json not found"
    fi
fi

if [ "$failures" -gt 0 ]; then
    log_error "Optional dependency install completed with ${failures} failure(s)."
    exit 1
fi

log_info "Optional runtime dependencies installed successfully."
