#!/usr/bin/env bash
# shellcheck disable=SC1091,SC2016

set -ex

# list of urls to match:
# - mobile.events.data.microsoft.com
# - vortex.data.microsoft.com

SEARCH="\.data\.microsoft\.com"
REPLACEMENT="s|//[^/]+\.data\.microsoft\.com|//0\.0\.0\.0|g"

exists() { type -t "$1" &> /dev/null; }

is_gnu_sed () {
  sed --version &> /dev/null
}

replace () {
  echo "${1}"
  if is_gnu_sed; then
    sed -i -E "${1}" "${2}"
  else
    sed -i '' -E "${1}" "${2}"
  fi
}

if ! exists gsed; then
  if is_gnu_sed; then
    function gsed() {
      sed -i -E "$@"
    }
  else
    function gsed() {
      sed -i '' -E "$@"
    }
  fi
fi

if is_gnu_sed; then
  replace_with_debug () {
    echo "found: ${2}"
    sed -i -E "${1}" "${2}"
  }
else
  replace_with_debug () {
    echo "found: ${2}"
    sed -i '' -E "${1}" "${2}"
  }
fi
export -f replace_with_debug

d1=$( date +%s )

arch=$(uname -m)

if [[ ${arch} == "x86_64" ]]; then
  ./node_modules/@vscode/ripgrep/bin/rg --no-ignore -l "${SEARCH}" . | xargs -I {} bash -c 'replace_with_debug "${1}" "{}"' _ "${REPLACEMENT}"
else
  grep -rl --exclude-dir=.git -E "${SEARCH}" . | xargs -I {} bash -c 'replace_with_debug "${1}" "{}"' _ "${REPLACEMENT}"
fi

d2=$( date +%s )

echo "close_telemetry: $((d2 - d1))s"
