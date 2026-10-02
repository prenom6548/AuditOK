#!/usr/bin/env bash
set -euo pipefail

ASSUME_YES=0
TARGET_HOME="${HOME}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [--yes] [TARGET_HOME]

Delete Kylin Agent runtime/user cache directories so a fresh install path can be tested.

Directories removed:
  ~/.kylin-agent-runtime
  ~/.local/share/uv
  ~/.local/lib/python3.12
  ~/.local/bin
  ~/.cache/uv
  ~/.cache/pip
  ~/.cache/ms-playwright
  ~/.local/share/kylin-agent
  ~/.local/share/kylin-cua

Options:
  --yes, -y    Delete without interactive confirmation
  --help, -h   Show this help
EOF
}

log_info() {
    printf '[INFO] %s\n' "$1"
}

log_warn() {
    printf '[WARN] %s\n' "$1" >&2
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --yes|-y)
            ASSUME_YES=1
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            TARGET_HOME="$1"
            shift
            ;;
    esac
done

if [[ -z "${TARGET_HOME}" || "${TARGET_HOME}" == "/" ]]; then
    log_warn "Invalid target home: ${TARGET_HOME}"
    exit 2
fi

if [[ ! -d "${TARGET_HOME}" ]]; then
    log_warn "Target home does not exist: ${TARGET_HOME}"
    exit 2
fi

TARGETS=(
    "${TARGET_HOME}/.kylin-agent-runtime"
    "${TARGET_HOME}/.local/share/uv"
    "${TARGET_HOME}/.local/lib/python3.12"
    "${TARGET_HOME}/.local/bin"
    "${TARGET_HOME}/.cache/uv"
    "${TARGET_HOME}/.cache/pip"
    "${TARGET_HOME}/.cache/ms-playwright"
    "${TARGET_HOME}/.local/share/kylin-agent"
    "${TARGET_HOME}/.local/share/kylin-cua"
)

log_warn "This will permanently delete Kylin Agent runtime test state under: ${TARGET_HOME}"
for path in "${TARGETS[@]}"; do
    if [[ -e "${path}" || -L "${path}" ]]; then
        printf '  delete: %s\n' "${path}"
    else
        printf '  skip missing: %s\n' "${path}"
    fi
done

if [[ "${ASSUME_YES}" != "1" ]]; then
    printf '\nType DELETE to continue: '
    read -r answer
    if [[ "${answer}" != "DELETE" ]]; then
        log_info "Cancelled."
        exit 0
    fi
fi

for path in "${TARGETS[@]}"; do
    if [[ -e "${path}" || -L "${path}" ]]; then
        rm -rf -- "${path}"
        log_info "Deleted: ${path}"
    fi
done

log_info "Runtime test state cleaned."
