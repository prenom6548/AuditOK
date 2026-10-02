#!/usr/bin/env bash
set -euo pipefail

RUNTIME_SHARE_DIR="${RUNTIME_SHARE_DIR:-/usr/local/share/kylin-agent-runtime}"
RUNTIME_HOME_BASENAME="${RUNTIME_HOME_BASENAME:-.kylin-agent-runtime}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [TARGET_HOME]

Install packaged Kylin Agent runtime cache from:
  ${RUNTIME_SHARE_DIR}

into the target user's home directory.

Defaults:
  TARGET_HOME  current user's HOME, or SUDO_USER's home when run via sudo

Environment overrides:
  RUNTIME_SHARE_DIR   Runtime cache package root
  OVERWRITE_EXISTING  Set to 1 to overwrite existing files
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

rsync_copy() {
    local source="$1"
    local target="$2"

    [[ -d "${source}" ]] || return 0

    mkdir -p "${target}"
    if [[ "${OVERWRITE_EXISTING:-0}" == "1" ]]; then
        rsync -a --no-owner --no-group "${source}/" "${target}/"
    else
        rsync -a --ignore-existing --no-owner --no-group "${source}/" "${target}/"
    fi
}

version_lt() {
    local left="${1#v}"
    local right="${2#v}"
    [ "$left" != "$right" ] || return 1
    [ "$(printf '%s\n%s\n' "$left" "$right" | sort -V | head -n 1)" = "$left" ]
}

runtime_installed_version() {
    local runtime_dir="$1"
    if [[ -f "${runtime_dir}/version" ]]; then
        tr -d '[:space:]' < "${runtime_dir}/version" 2>/dev/null || true
    fi
}

deprecate_runtime_dir() {
    local runtime_dir="$1"
    local reason="$2"
    local suffix="$3"
    local backup_dir="${runtime_dir}.deprecated-${suffix}-$(date -u +%Y%m%d-%H%M%S)"
    log_warn "${reason}"
    log_warn "Moving it aside to ${backup_dir}."
    mv "${runtime_dir}" "${backup_dir}"
}

prepare_runtime_install_dir() {
    local hermes_home="$1"
    local runtime_dir="${hermes_home}/agent-runtime"
    local minimum_version="0.9.2"
    local installed_version=""

    if [[ -d "${runtime_dir}" ]]; then
        installed_version="$(runtime_installed_version "${runtime_dir}")"
        if [[ -z "${installed_version}" ]]; then
            deprecate_runtime_dir "${runtime_dir}" \
                "Existing runtime at ${runtime_dir} has no version file; deprecating it." \
                "unknown-version"
        elif version_lt "${installed_version}" "${minimum_version}"; then
            deprecate_runtime_dir "${runtime_dir}" \
                "Existing runtime at ${runtime_dir} is ${installed_version} (< ${minimum_version}); deprecating it." \
                "${installed_version}"
        fi
    fi
}

replace_paths() {
    local root="$1"
    local old_home="$2"
    local new_home="$3"

    [[ -n "${old_home}" && "${old_home}" != "${new_home}" && -d "${root}" ]] || return 0

    while IFS= read -r -d '' file; do
        if grep -Iq . "${file}" && grep -qF "${old_home}" "${file}"; then
            sed -i "s|${old_home}|${new_home}|g" "${file}"
        fi
    done < <(
        find "${root}" \
            \( -name '.git' \
               -o -name 'node_modules' \
               -o -name 'venv' \
               -o -path "*/${RUNTIME_HOME_BASENAME}/node" \
               -o -path '*/.local/share/uv/python' \
            \) -prune -o \
            -type f \( \
                -name '*.yaml' -o -name '*.yml' -o -name '*.json' -o -name '*.md' \
                -o -name '*.txt' -o -name '*.log' -o -name '*.sh' -o -name '*.py' \
                -o -name '.env' -o -name 'gateway.pid' -o -name 'npmrc' \
            \) -print0
    )
}

replace_symlink_targets() {
    local root="$1"
    local old_home="$2"
    local new_home="$3"

    [[ -n "${old_home}" && "${old_home}" != "${new_home}" && -d "${root}" ]] || return 0

    while IFS= read -r -d '' link_path; do
        local target
        target="$(readlink "${link_path}")"
        if [[ "${target}" == *"${old_home}"* ]]; then
            local new_target="${target//${old_home}/${new_home}}"
            ln -sfn "${new_target}" "${link_path}"
        fi
    done < <(find "${root}" -type l -print0)
}

remove_runtime_state() {
    local hermes_home="$1"

    [[ -d "${hermes_home}" ]] || return 0
    rm -rf "${hermes_home}/agent-runtime/venv"
    rm -rf "${hermes_home}/cron"
    rm -rf "${hermes_home}/sessions"
    rm -rf "${hermes_home}/logs"
    rm -rf "${hermes_home}/pairing"
    rm -rf "${hermes_home}/hooks"
    rm -rf "${hermes_home}/image_cache"
    rm -rf "${hermes_home}/audio_cache"
    rm -rf "${hermes_home}/memories"
    rm -rf "${hermes_home}/kylin-agent/message-details"
    rm -f "${hermes_home}/state.db"
    rm -f "${hermes_home}/response_store.db"
    rm -f "${hermes_home}/kanban.db"
    rm -f "${hermes_home}/desktop/sessions.json"
    rm -f "${hermes_home}/channel_directory.json"
    rm -f "${hermes_home}/cache-installed"
    rm -f "${hermes_home}/.tirith-install-failed"
    rm -f "${hermes_home}"/config.yaml.bak-*
    rm -f "${hermes_home}/.env"
    rm -f "${hermes_home}/config.yaml"
    rm -f "${hermes_home}/models.json"
    rm -f "${hermes_home}/SOUL.md"
    rm -f "${hermes_home}/gateway.pid"
    rm -f "${hermes_home}/gateway.lock"
    rm -f "${hermes_home}/gateway_state.json"
    rm -f "${hermes_home}/cron/.tick.lock"
    find "${hermes_home}" -maxdepth 2 \
        \( -name '*.lock' -o -name '*-shm' -o -name '*-wal' \) \
        -type f -delete 2>/dev/null || true
}

remove_nonportable_wrappers() {
    local target_home="$1"

    local runtime_home="${target_home}/${RUNTIME_HOME_BASENAME}"

    for wrapper in hermes agent-runtime kylin-runtime kylin-agent-runtime node npm npx python3.11; do
        local path="${target_home}/.local/bin/${wrapper}"
        [[ -e "${path}" || -L "${path}" ]] || continue

        local target=""
        if [[ -L "${path}" ]]; then
            target="$(readlink "${path}")"
        elif grep -Iq . "${path}"; then
            target="$(sed -n '1,20p' "${path}")"
        fi

        if [[ "${target}" == *"${runtime_home}"* ]]; then
            rm -f "${path}"
        fi
    done
}

fix_ownership() {
    local target_home="$1"
    local user="$2"
    local group="$3"

    if [[ "${EUID}" -eq 0 ]]; then
        chown -R "${user}:${group}" \
            "${target_home}/${RUNTIME_HOME_BASENAME}" \
            "${target_home}/.local/share/uv" \
            "${target_home}/.local/lib/python3.12" \
            "${target_home}/.local/bin" \
            "${target_home}/.local/share/kylin-agent" \
            "${target_home}/.local/share/kylin-cua" \
            "${target_home}/.cache/uv" \
            "${target_home}/.cache/pip" \
            "${target_home}/.cache/ms-playwright" 2>/dev/null || true
    fi
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

require_command rsync
require_command getent
require_command grep
require_command sed
require_command find

TARGET_USER="$(target_user)"
TARGET_GROUP="$(id -gn "${TARGET_USER}")"
TARGET_HOME="${1:-$(home_for_user "${TARGET_USER}")}"

if [[ -z "${TARGET_HOME}" ]]; then
    log_error "Cannot determine target home directory."
    exit 1
fi

if [[ ! -d "${RUNTIME_SHARE_DIR}" ]]; then
    log_warn "Runtime cache directory not found: ${RUNTIME_SHARE_DIR}"
    exit 0
fi

HERMES_HOME="${TARGET_HOME}/${RUNTIME_HOME_BASENAME}"
MARKER="${HERMES_HOME}/cache-installed"
if [[ -f "${MARKER}" ]]; then
    if [[ -x "${HERMES_HOME}/agent-runtime/venv/bin/hermes" ]]; then
        log_info "Runtime cache already installed: ${MARKER}"
        exit 0
    fi
    log_warn "Runtime cache marker exists but runtime venv is incomplete; refreshing cache from package."
fi

if [[ ! -d "${RUNTIME_SHARE_DIR}/local" && ! -d "${RUNTIME_SHARE_DIR}/cache" ]]; then
    log_warn "Runtime cache package has no local/cache payload: ${RUNTIME_SHARE_DIR}"
    exit 0
fi

if [[ -d "${RUNTIME_SHARE_DIR}/local" && ! -d "${RUNTIME_SHARE_DIR}/local/${RUNTIME_HOME_BASENAME}" ]]; then
    log_warn "Runtime cache package has no runtime home payload: ${RUNTIME_SHARE_DIR}/local/${RUNTIME_HOME_BASENAME}"
    exit 0
fi

OLD_HOME=""
if [[ -f "${RUNTIME_SHARE_DIR}/manifest" ]]; then
    OLD_HOME="$(sed -n 's/^SOURCE_HOME=//p' "${RUNTIME_SHARE_DIR}/manifest" | head -1)"
fi

log_info "Installing runtime cache into ${TARGET_HOME}"
mkdir -p "${HERMES_HOME}" "${TARGET_HOME}/.local" "${TARGET_HOME}/.cache"

CUA_CACHE_SOURCE="${RUNTIME_SHARE_DIR}/local/share/kylin-cua"
if [[ ! -d "${CUA_CACHE_SOURCE}" && -d "${RUNTIME_SHARE_DIR}/local/share/kylin-agent/kylin-cua" ]]; then
    CUA_CACHE_SOURCE="${RUNTIME_SHARE_DIR}/local/share/kylin-agent/kylin-cua"
fi

rsync_copy "${RUNTIME_SHARE_DIR}/local/${RUNTIME_HOME_BASENAME}" "${HERMES_HOME}"
rsync_copy "${RUNTIME_SHARE_DIR}/local/share/uv" "${TARGET_HOME}/.local/share/uv"
rsync_copy "${RUNTIME_SHARE_DIR}/local/lib/python3.12" "${TARGET_HOME}/.local/lib/python3.12"
rsync_copy "${RUNTIME_SHARE_DIR}/local/bin" "${TARGET_HOME}/.local/bin"
rsync_copy "${CUA_CACHE_SOURCE}" "${TARGET_HOME}/.local/share/kylin-cua"

rsync_copy "${RUNTIME_SHARE_DIR}/cache/uv" "${TARGET_HOME}/.cache/uv"
rsync_copy "${RUNTIME_SHARE_DIR}/cache/pip" "${TARGET_HOME}/.cache/pip"
rsync_copy "${RUNTIME_SHARE_DIR}/cache/ms-playwright" "${TARGET_HOME}/.cache/ms-playwright"

prepare_runtime_install_dir "${HERMES_HOME}"
remove_runtime_state "${HERMES_HOME}"
remove_nonportable_wrappers "${TARGET_HOME}"

if [[ -n "${OLD_HOME}" ]]; then
    old_runtime_home="${OLD_HOME}/${RUNTIME_HOME_BASENAME}"
    replace_paths "${HERMES_HOME}" "${old_runtime_home}" "${HERMES_HOME}"
    replace_paths "${TARGET_HOME}/.local/bin" "${old_runtime_home}" "${HERMES_HOME}"
    replace_paths "${TARGET_HOME}/.local/lib/python3.12" "${old_runtime_home}" "${HERMES_HOME}"
    replace_paths "${TARGET_HOME}/.local/share/kylin-agent" "${old_runtime_home}" "${HERMES_HOME}"
    replace_paths "${TARGET_HOME}/.local/share/kylin-cua" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${HERMES_HOME}" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/bin" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/uv" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/lib/python3.12" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/kylin-agent" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/kylin-cua" "${old_runtime_home}" "${HERMES_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.cache/uv" "${old_runtime_home}" "${HERMES_HOME}"

    replace_paths "${HERMES_HOME}" "${OLD_HOME}" "${TARGET_HOME}"
    replace_paths "${TARGET_HOME}/.local/bin" "${OLD_HOME}" "${TARGET_HOME}"
    replace_paths "${TARGET_HOME}/.local/lib/python3.12" "${OLD_HOME}" "${TARGET_HOME}"
    replace_paths "${TARGET_HOME}/.local/share/kylin-agent" "${OLD_HOME}" "${TARGET_HOME}"
    replace_paths "${TARGET_HOME}/.local/share/kylin-cua" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${HERMES_HOME}" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/bin" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/uv" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/lib/python3.12" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/kylin-agent" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.local/share/kylin-cua" "${OLD_HOME}" "${TARGET_HOME}"
    replace_symlink_targets "${TARGET_HOME}/.cache/uv" "${OLD_HOME}" "${TARGET_HOME}"
fi

fix_ownership "${TARGET_HOME}" "${TARGET_USER}" "${TARGET_GROUP}"

mkdir -p "${HERMES_HOME}"
cat > "${MARKER}" <<EOF
installed_at=$(date -Is)
source=${RUNTIME_SHARE_DIR}
source_home=${OLD_HOME}
target_home=${TARGET_HOME}
EOF

if [[ "${EUID}" -eq 0 ]]; then
    chown "${TARGET_USER}:${TARGET_GROUP}" "${MARKER}" 2>/dev/null || true
fi

log_info "Runtime cache installed: ${MARKER}"
