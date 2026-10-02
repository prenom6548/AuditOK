#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

PACKAGE_NAME="${PACKAGE_NAME:-kylin-cua}"
PACKAGE_VERSION="${PACKAGE_VERSION:-1.0.0}"
MAINTAINER="${MAINTAINER:-Kylin Agent Team <lihanhua@yhkylin.com>}"
DESCRIPTION="${DESCRIPTION:-Kylin CUA runtime package}"
SOURCE_DIR="${SOURCE_DIR:-${SCRIPT_DIR}}"
INSTALL_DIR="${INSTALL_DIR:-/usr/local/share/kylin-cua}"
USER_INSTALLER="${USER_INSTALLER:-/usr/local/sbin/kylin-cua-install-user}"
YDOTOOL_SOURCE_DIR="${YDOTOOL_SOURCE_DIR:-${HOME}/ydotool}"
BUILD_ROOT="${BUILD_ROOT:-${ROOT_DIR}/build-kylin-cua-package}"
DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
ARCH="${ARCH:-$(dpkg --print-architecture)}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [SOURCE_DIR]

Build a Debian package from SOURCE_DIR and install its contents under:
  ${INSTALL_DIR}

During package installation, the post-install script tries to copy the files to:
  ~/.local/share/kylin-cua

After selecting the target desktop user, it also runs:
  SOURCE_DIR/scripts/ydotool_install.sh
using the ydotool source embedded in the deb to install and start the
ydotoold service for that user without downloading it during installation.

If they already exist, the following skill directories also receive
SOURCE_DIR/skill/kylin-cua as a refreshed kylin-cua skill:
  ~/.openclaw/skills
  ~/.hermes/skills
  ~/.kylin-agent-runtime/skills
  ~/.kylinbot/workspace/skills

The target user is selected from KYLIN_CUA_TARGET_USER, SUDO_USER,
PKEXEC_UID/SUDO_UID, the active login user, or a single regular local user.
If no user can be selected safely, run this after installing the package:
  sudo ${USER_INSTALLER} USERNAME

Defaults:
  SOURCE_DIR=${SOURCE_DIR} (directory containing this script)
  output=${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb

Environment overrides:
  SOURCE_DIR               Source kylin-cua directory
  PACKAGE_NAME             Debian package name
  PACKAGE_VERSION          Debian package version
  MAINTAINER               Debian package maintainer
  DESCRIPTION              Debian package description
  INSTALL_DIR              System-wide payload directory
  USER_INSTALLER           Installed per-user copy helper
  YDOTOOL_SOURCE_DIR       Local ydotool source embedded in the deb
  BUILD_ROOT               Temporary package root
  DIST_DIR                 Output directory
  ARCH                     Debian package architecture
  KYLIN_CUA_TARGET_USER    Target user used by the deb postinst script
EOF
}

log_info() {
    printf '[INFO] %s\n' "$1"
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

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if [[ $# -ge 1 ]]; then
    SOURCE_DIR="$1"
fi
if [[ $# -gt 1 ]]; then
    usage >&2
    exit 1
fi

if [[ ! -d "${SOURCE_DIR}" ]]; then
    log_error "Source directory not found: ${SOURCE_DIR}"
    exit 1
fi
if [[ ! -f "${SCRIPT_DIR}/ydotool_install.sh" ]]; then
    log_error "ydotool installer not found: ${SCRIPT_DIR}/ydotool_install.sh"
    exit 1
fi
if [[ ! -f "${YDOTOOL_SOURCE_DIR}/CMakeLists.txt" ]]; then
    log_error "ydotool source not found: ${YDOTOOL_SOURCE_DIR}"
    exit 1
fi

require_command dpkg
require_command dpkg-deb
require_command rsync

case "${BUILD_ROOT}" in
    /*) ;;
    *) BUILD_ROOT="${ROOT_DIR}/${BUILD_ROOT}" ;;
esac
case "${DIST_DIR}" in
    /*) ;;
    *) DIST_DIR="${ROOT_DIR}/${DIST_DIR}" ;;
esac

if [[ -z "${BUILD_ROOT}" || "${BUILD_ROOT}" == "/" ||
      -z "${DIST_DIR}" || "${DIST_DIR}" == "/" ]]; then
    log_error "Refusing to use an unsafe build or output directory."
    exit 1
fi

PKG_DIR="${BUILD_ROOT}/${PACKAGE_NAME}"
PAYLOAD_DIR="${PKG_DIR}${INSTALL_DIR}"
INSTALLER_PATH="${PKG_DIR}${USER_INSTALLER}"
YDOTOOL_PAYLOAD_DIR="${PAYLOAD_DIR}/vendor/ydotool"
DEB_FILE="${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb"

log_info "Cleaning package workspace: ${PKG_DIR}"
rm -rf -- "${PKG_DIR}"
mkdir -p "${PAYLOAD_DIR}" "$(dirname "${INSTALLER_PATH}")" \
    "${PKG_DIR}/DEBIAN" "${DIST_DIR}"

log_info "Copying ${SOURCE_DIR} to ${INSTALL_DIR}"
rsync -a --delete "${SOURCE_DIR}/" "${PAYLOAD_DIR}/"

log_info "Embedding offline ydotool source from ${YDOTOOL_SOURCE_DIR}"
mkdir -p "${PAYLOAD_DIR}/scripts" "${YDOTOOL_PAYLOAD_DIR}"
install -m 0755 "${SCRIPT_DIR}/ydotool_install.sh" \
    "${PAYLOAD_DIR}/scripts/ydotool_install.sh"
rsync -a --delete --exclude='.git/' --exclude='build/' \
    "${YDOTOOL_SOURCE_DIR}/" "${YDOTOOL_PAYLOAD_DIR}/"

cat > "${INSTALLER_PATH}" <<EOF
#!/usr/bin/env bash
set -euo pipefail

SOURCE_DIR="${INSTALL_DIR}"
REQUESTED_USER="\${1:-}"

user_from_uid() {
    getent passwd "\$1" | awk -F: 'NR == 1 { print \$1 }'
}

is_regular_user() {
    local user="\$1"
    local entry uid home shell uid_min
    entry="\$(getent passwd "\${user}" || true)"
    [[ -n "\${entry}" ]] || return 1
    IFS=: read -r _ _ uid _ _ home shell <<<"\${entry}"
    uid_min="\$(awk '\$1 == "UID_MIN" { print \$2; exit }' /etc/login.defs 2>/dev/null || true)"
    uid_min="\${uid_min:-1000}"
    [[ "\${uid}" =~ ^[0-9]+\$ ]] || return 1
    (( uid >= uid_min )) || return 1
    [[ "\${home}" == /* && "\${home}" != "/" ]] || return 1
    [[ "\${shell}" != */nologin && "\${shell}" != */false ]] || return 1
}

detect_target_user() {
    local candidate=""
    local -a regular_users=()

    if [[ -n "\${KYLIN_CUA_TARGET_USER:-}" ]]; then
        printf '%s\n' "\${KYLIN_CUA_TARGET_USER}"
        return 0
    fi
    if [[ -n "\${SUDO_USER:-}" && "\${SUDO_USER}" != "root" ]]; then
        printf '%s\n' "\${SUDO_USER}"
        return 0
    fi
    if [[ -n "\${PKEXEC_UID:-}" ]]; then
        candidate="\$(user_from_uid "\${PKEXEC_UID}")"
        if [[ -n "\${candidate}" && "\${candidate}" != "root" ]]; then
            printf '%s\n' "\${candidate}"
            return 0
        fi
    fi
    if [[ -n "\${SUDO_UID:-}" ]]; then
        candidate="\$(user_from_uid "\${SUDO_UID}")"
        if [[ -n "\${candidate}" && "\${candidate}" != "root" ]]; then
            printf '%s\n' "\${candidate}"
            return 0
        fi
    fi

    while IFS= read -r candidate; do
        [[ -n "\${candidate}" ]] || continue
        if is_regular_user "\${candidate}"; then
            printf '%s\n' "\${candidate}"
            return 0
        fi
    done < <(who 2>/dev/null | awk '{print \$1}' | awk '!seen[\$0]++')

    while IFS=: read -r candidate _ _ _ _ _ _; do
        if is_regular_user "\${candidate}"; then
            regular_users+=("\${candidate}")
        fi
    done < /etc/passwd
    if (( \${#regular_users[@]} == 1 )); then
        printf '%s\n' "\${regular_users[0]}"
        return 0
    fi
    return 1
}

if [[ ! -d "\${SOURCE_DIR}" ]]; then
    printf '[ERROR] Packaged source directory not found: %s\n' "\${SOURCE_DIR}" >&2
    exit 1
fi

TARGET_USER="\${REQUESTED_USER}"
if [[ -z "\${TARGET_USER}" ]]; then
    TARGET_USER="\$(detect_target_user || true)"
fi
if [[ -z "\${TARGET_USER}" ]]; then
    printf '[WARN] Cannot determine the target desktop user.\n' >&2
    printf '[WARN] Run: sudo ${USER_INSTALLER} USERNAME\n' >&2
    exit 0
fi

PASSWD_ENTRY="\$(getent passwd "\${TARGET_USER}" || true)"
if [[ -z "\${PASSWD_ENTRY}" ]]; then
    printf '[ERROR] User does not exist: %s\n' "\${TARGET_USER}" >&2
    exit 1
fi
IFS=: read -r _ _ TARGET_UID TARGET_GID _ TARGET_HOME _ <<<"\${PASSWD_ENTRY}"
if [[ "\${TARGET_HOME}" != /* || "\${TARGET_HOME}" == "/" ]]; then
    printf '[ERROR] Unsafe home directory for %s: %s\n' "\${TARGET_USER}" "\${TARGET_HOME}" >&2
    exit 1
fi

TARGET_DIR="\${TARGET_HOME}/.local/share/kylin-cua"
install -d -m 0755 -o "\${TARGET_UID}" -g "\${TARGET_GID}" \
    "\${TARGET_HOME}/.local" "\${TARGET_HOME}/.local/share" "\${TARGET_DIR}"
rsync -a --chown="\${TARGET_UID}:\${TARGET_GID}" \
    "\${SOURCE_DIR}/" "\${TARGET_DIR}/"

printf '[INFO] Installed Kylin CUA for %s at %s\n' "\${TARGET_USER}" "\${TARGET_DIR}"

SKILL_SOURCE_DIR="\${SOURCE_DIR}/skill/kylin-cua"
if [[ -d "\${SKILL_SOURCE_DIR}" ]]; then
    SKILL_DIRS=(
        "\${TARGET_HOME}/.openclaw/skills"
        "\${TARGET_HOME}/.hermes/skills"
        "\${TARGET_HOME}/.kylin-agent-runtime/skills"
        "\${TARGET_HOME}/.kylinbot/workspace/skills"
    )
    for SKILL_DIR in "\${SKILL_DIRS[@]}"; do
        [[ -d "\${SKILL_DIR}" ]] || continue
        SKILL_TARGET="\${SKILL_DIR}/kylin-cua"
        rm -rf -- "\${SKILL_TARGET}"
        rsync -a --chown="\${TARGET_UID}:\${TARGET_GID}" \
            "\${SKILL_SOURCE_DIR}/" "\${SKILL_TARGET}/"
        printf '[INFO] Installed Kylin CUA skill for %s at %s\n' \
            "\${TARGET_USER}" "\${SKILL_TARGET}"
    done
else
    printf '[WARN] Packaged skill directory not found: %s\n' \
        "\${SKILL_SOURCE_DIR}" >&2
fi

YDOTOOL_INSTALLER="\${SOURCE_DIR}/scripts/ydotool_install.sh"
printf '[INFO] Installing ydotool for %s with %s\n' \
    "\${TARGET_USER}" "\${YDOTOOL_INSTALLER}"
bash "\${YDOTOOL_INSTALLER}" \
    "\${SOURCE_DIR}/vendor/ydotool" "\${TARGET_USER}"
EOF

cat > "${PKG_DIR}/DEBIAN/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Depends: bash, coreutils, libc-bin, rsync, mawk | gawk, git, make, cmake, scdoc, grim, wl-clipboard, build-essential, systemd, python3-dev, python3-pip, xclip, xdotool, wmctrl, x11-utils, x11-xserver-utils, xdg-utils, libglib2.0-bin, libgtk-3-bin, scrot, python3-tk
Description: ${DESCRIPTION}
 This package installs Kylin CUA under ${INSTALL_DIR} and copies it into
 the selected user's ~/.local/share/kylin-cua during package configuration.
 It also refreshes the kylin-cua skill in supported existing skill directories.
 The package builds its embedded ydotool source and starts ydotoold for the
 selected desktop user without downloading ydotool during package installation.
EOF

cat > "${PKG_DIR}/DEBIAN/postinst" <<EOF
#!/usr/bin/env bash
set -e

if [[ "\${1:-}" == "configure" ]]; then
    "${USER_INSTALLER}"
fi
exit 0
EOF

chmod 0644 "${PKG_DIR}/DEBIAN/control"
chmod 0755 "${PKG_DIR}/DEBIAN/postinst" "${INSTALLER_PATH}"
# Debian's control directory and package-created parent directories require
# conventional modes. The payload itself is excluded so all source modes are
# retained exactly (including group-write and executable bits).
find "${PKG_DIR}" -type d \
    ! -path "${PAYLOAD_DIR}" \
    ! -path "${PAYLOAD_DIR}/*" \
    -exec chmod 0755 {} +
chmod --reference="${SOURCE_DIR}" "${PAYLOAD_DIR}"

log_info "Building Debian package"
rm -f -- "${DEB_FILE}"
dpkg-deb --root-owner-group --build "${PKG_DIR}" "${DEB_FILE}"

log_info "Package created: ${DEB_FILE}"
log_info "Install with: sudo dpkg -i \"${DEB_FILE}\""
log_info "Explicit target user: sudo env KYLIN_CUA_TARGET_USER=USERNAME dpkg -i \"${DEB_FILE}\""
