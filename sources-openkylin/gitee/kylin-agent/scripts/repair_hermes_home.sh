#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<EOF
Usage: $(basename "$0") [HERMES_HOME] [OLD_HOME] [NEW_HOME]

Repair a copied Hermes home so it can be used by the current target user.

Defaults:
  HERMES_HOME  \$HOME/.kylin-agent-runtime
  OLD_HOME     auto-detected from common Hermes files
  NEW_HOME     target user's home directory

Examples:
  $(basename "$0")
  $(basename "$0") /home/hank/.kylin-agent-runtime /home/ok /home/hank
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

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        log_error "Missing required command: $1"
        exit 1
    fi
}

shell_quote() {
    printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

target_user() {
    if [[ "${EUID}" -eq 0 && -n "${SUDO_USER:-}" && "${SUDO_USER}" != "root" ]]; then
        printf '%s\n' "${SUDO_USER}"
        return
    fi
    id -un
}

home_for_user() {
    local user="$1"
    getent passwd "${user}" | cut -d: -f6
}

detect_old_home() {
    local hermes_home="$1"
    local detected=""
    detected="$(
        {
            [[ -f "${hermes_home}/gateway.pid" ]] && printf '%s\n' "${hermes_home}/gateway.pid"
            [[ -f "${hermes_home}/memories/USER.md" ]] && printf '%s\n' "${hermes_home}/memories/USER.md"
            [[ -f "${hermes_home}/config.yaml" ]] && printf '%s\n' "${hermes_home}/config.yaml"
            [[ -f "${hermes_home}/models.json" ]] && printf '%s\n' "${hermes_home}/models.json"
        } | xargs -r grep -Eho '/home/[^/"[:space:]]+' 2>/dev/null | head -1
    )"

    if [[ -n "${detected}" ]]; then
        printf '%s\n' "${detected}"
    fi
}

replace_paths() {
    local hermes_home="$1"
    local old_home="$2"
    local new_home="$3"

    [[ -n "${old_home}" && "${old_home}" != "${new_home}" ]] || return 0

    log_info "Replacing ${old_home} with ${new_home} in portable text files..."
    while IFS= read -r -d '' file; do
        if grep -Iq . "${file}" && grep -qF "${old_home}" "${file}"; then
            sed -i "s|${old_home}|${new_home}|g" "${file}"
        fi
    done < <(
        find "${hermes_home}" \
            \( -path "${hermes_home}/hermes-agent/.git" \
               -o -path "${hermes_home}/agent-runtime/.git" \
               -o -path "${hermes_home}/hermes-agent/node_modules" \
               -o -path "${hermes_home}/agent-runtime/node_modules" \
               -o -path "${hermes_home}/hermes-agent/venv" \
               -o -path "${hermes_home}/agent-runtime/venv" \
               -o -path "${hermes_home}/node" \
            \) -prune -o \
            -type f \( \
                -name '*.yaml' -o -name '*.yml' -o -name '*.json' -o -name '*.md' \
                -o -name '*.txt' -o -name '*.log' -o -name '*.sh' -o -name '*.py' \
                -o -name '.env' -o -name 'gateway.pid' \
            \) -print0
    )
}

remove_runtime_state() {
    local hermes_home="$1"

    log_info "Removing stale runtime state..."
    rm -f "${hermes_home}/gateway.pid"
    rm -f "${hermes_home}/auth.lock"
    rm -f "${hermes_home}/response_store.db-shm" "${hermes_home}/response_store.db-wal"
    rm -f "${hermes_home}/state.db-shm" "${hermes_home}/state.db-wal"

    if [[ -d "${hermes_home}/hermes-agent/venv" ]]; then
        log_info "Removing non-portable Python venv: ${hermes_home}/hermes-agent/venv"
        rm -rf "${hermes_home}/hermes-agent/venv"
    fi
    if [[ -d "${hermes_home}/agent-runtime/venv" ]]; then
        log_info "Removing non-portable Python venv: ${hermes_home}/agent-runtime/venv"
        rm -rf "${hermes_home}/agent-runtime/venv"
    fi
}

fix_ownership() {
    local hermes_home="$1"
    local user="$2"
    local group="$3"

    log_info "Ensuring owner: ${user}:${group}"
    if [[ "${EUID}" -eq 0 ]]; then
        chown -R "${user}:${group}" "${hermes_home}"
    elif [[ "${user}" == "$(id -un)" ]] &&
         ! find "${hermes_home}" \( ! -user "${user}" -o ! -group "${group}" \) -print -quit | grep -q .; then
        log_info "Ownership already matches ${user}:${group}"
    else
        sudo chown -R "${user}:${group}" "${hermes_home}"
    fi
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

require_command find
require_command grep
require_command sed
require_command getent

TARGET_USER="$(target_user)"
TARGET_GROUP="$(id -gn "${TARGET_USER}")"
TARGET_HOME="$(home_for_user "${TARGET_USER}")"
if [[ -z "${TARGET_HOME}" ]]; then
    log_error "Cannot determine home directory for user: ${TARGET_USER}"
    exit 1
fi

HERMES_HOME="${1:-${TARGET_HOME}/.kylin-agent-runtime}"
OLD_HOME="${2:-}"
NEW_HOME="${3:-${TARGET_HOME}}"

if [[ ! -d "${HERMES_HOME}" ]]; then
    log_error "Hermes home not found: ${HERMES_HOME}"
    exit 1
fi

if [[ -z "${OLD_HOME}" ]]; then
    OLD_HOME="$(detect_old_home "${HERMES_HOME}" || true)"
fi

if [[ -z "${OLD_HOME}" ]]; then
    log_warn "Old home path was not detected; path replacement skipped."
else
    replace_paths "${HERMES_HOME}" "${OLD_HOME}" "${NEW_HOME}"
fi

remove_runtime_state "${HERMES_HOME}"
fix_ownership "${HERMES_HOME}" "${TARGET_USER}" "${TARGET_GROUP}"

log_info "Repair complete: ${HERMES_HOME}"
log_info "Start KylinAgent once to let the runtime installer recreate missing venv/dependencies."
