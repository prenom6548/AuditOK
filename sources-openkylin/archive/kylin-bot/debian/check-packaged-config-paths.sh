#!/bin/sh
# Fail the Debian build if packaged KylinBot config embeds a developer home path.
# Use ~/.kylinbot/... or omit keys so runtime defaults apply.

set -eu

TEMPLATE_DIR="${1:-debian/kylinbot}"

if [ ! -d "$TEMPLATE_DIR" ]; then
	echo "check-packaged-config-paths: missing template dir: $TEMPLATE_DIR" >&2
	exit 1
fi

# TOML path values like: key = "/home/someuser/.kylinbot/..."
# SKILL.md examples mentioning /home/ are documentation only and are not scanned.
violations=$(grep -RInE '= *"/home/[^/"]+/' "$TEMPLATE_DIR" --include='*.toml' 2>/dev/null || true)

if [ -n "$violations" ]; then
	echo "error: packaged config must not hardcode /home/<user>/ paths." >&2
	echo "       Use ~/.kylinbot/... or relative paths under the config dir." >&2
	echo "$violations" >&2
	exit 1
fi

echo "check-packaged-config-paths: ok ($TEMPLATE_DIR)"
