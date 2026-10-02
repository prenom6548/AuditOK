#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
#
# SPDX-License-Identifier: GPL-1.0-or-later
#
# Integration test for wlcctrl. Runs against the live Wayland session but only
# touches windows/workspaces it creates itself, and restores cursor position,
# active workspace and workspace count on exit.
#
# Usage: test-wlcctrl.sh [--quick] [--no-input] [--keep] [-h]
#   --quick     skip keyboard/mouse injection and workspace mutation
#   --no-input  skip keyboard/mouse injection only
#   --keep      keep the temporary directory for inspection
#
# Environment:
#   WLCCTRL_BIN  path to the wlcctrl binary (auto-detected by default)
#
# NOTE: keyboard/mouse injection steers the real cursor and focused window, so
# avoid using the machine while those groups run.

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
W=${WLCCTRL_BIN:-$ROOT/build/tools/wlcctrl/wlcctrl}

RUN_INPUT=1
RUN_WSMUT=1
KEEP=0

usage() {
    cat <<'USAGE'
Usage: test-wlcctrl.sh [--quick] [--no-input] [--keep] [-h]
  --quick     skip keyboard/mouse injection and workspace mutation
  --no-input  skip keyboard/mouse injection only
  --keep      keep the temporary directory for inspection

Environment:
  WLCCTRL_BIN  path to the wlcctrl binary (auto-detected by default)
USAGE
}

for arg in "$@"; do
    case "$arg" in
    --quick) RUN_INPUT=0; RUN_WSMUT=0 ;;
    --no-input) RUN_INPUT=0 ;;
    --keep) KEEP=1 ;;
    -h | --help) usage; exit 0 ;;
    *)
        echo "unknown option: $arg (try --help)" >&2
        exit 2
        ;;
    esac
done

if [ ! -x "$W" ]; then
    echo "wlcctrl binary not found at $W (set WLCCTRL_BIN)" >&2
    exit 2
fi

# ---------------------------------------------------------------- output ----
if [ -t 1 ]; then
    C_RED=$(printf '\033[31m'); C_GRN=$(printf '\033[32m')
    C_YLW=$(printf '\033[33m'); C_RST=$(printf '\033[0m')
else
    C_RED=""; C_GRN=""; C_YLW=""; C_RST=""
fi

pass=0; fail=0; skip=0
ok()   { pass=$((pass + 1)); printf '  %sPASS%s %s\n' "$C_GRN" "$C_RST" "$1"; }
bad()  { fail=$((fail + 1)); printf '  %sFAIL%s %s\n' "$C_RED" "$C_RST" "$1"; }
skp()  { skip=$((skip + 1)); printf '  %sSKIP%s %s\n' "$C_YLW" "$C_RST" "$1"; }
group() { printf '\n== %s ==\n' "$1"; }

assert_rc() { # name expected actual
    if [ "$2" -eq "$3" ]; then ok "$1"; else bad "$1 (rc=$3, want $2)"; fi
}
assert_nonzero() { # name actual
    if [ "$2" -ne 0 ]; then ok "$1"; else bad "$1 (rc=0, want non-zero)"; fi
}
assert_eq() { # name expected actual
    if [ "$2" = "$3" ]; then ok "$1"; else bad "$1 (want '$2', got '$3')"; fi
}

poll_until() { # timeout_seconds check_cmd...
    local t=$1; shift
    local i=0
    while [ "$i" -lt $((t * 2)) ]; do
        if "$@" >/dev/null 2>&1; then return 0; fi
        sleep 0.5; i=$((i + 1))
    done
    return 1
}
assert_eventually() { # name timeout check_cmd...
    local name=$1 t=$2; shift 2
    if poll_until "$t" "$@"; then ok "$name"; else bad "$name (timeout ${t}s)"; fi
}

# ------------------------------------------------------------- helpers ------
uuid_of() { "$W" --search "$1" 2>/dev/null | awk 'NR==1{print $3}'; }
wait_window() { # fragment [timeout]
    local frag=$1 t=${2:-20} i=0
    while [ "$i" -lt "$t" ]; do
        [ -n "$(uuid_of "$frag")" ] && return 0
        sleep 0.5; i=$((i + 1))
    done
    return 1
}
status_is() { "$W" --getwindowstatus "$1" 2>/dev/null | grep -q "$2: $3"; }
geom_pos() {
    "$W" --getwindowgeometry "$1" 2>/dev/null |
        sed -n 's/^geometry: (\([0-9]*\), \([0-9]*\)).*/\1 \2/p'
}
pos_is() { [ "$(geom_pos "$1")" = "$2 $3" ]; }
geom_size() { "$W" --getwindowgeometry "$1" 2>/dev/null | awk '{print $(NF-2), $NF}'; }
size_is() { [ "$(geom_size "$1")" = "$2 $3" ]; }
ws_count() { "$W" --workspace 2>/dev/null | grep -c '^workspace "'; }
ws_uuid_at() { # position
    "$W" --workspace 2>/dev/null |
        awk -v p="$1" '/^workspace "/{u=$2} /position:/{if($2==p) print u}' | tr -d '"'
}
ws_active() {
    "$W" --workspace 2>/dev/null |
        awk '/^workspace "/{u=$2} /activated: true/{print u}' | tr -d '"'
}
mouse_pos() {
    "$W" --getmouselocation 2>/dev/null |
        sed -n 's/.*position : \([0-9]*\), \([0-9]*\).*/\1 \2/p'
}
is_png() { head -c 8 "$1" 2>/dev/null | grep -q PNG; }
appid_of() { "$W" --getwindowname "$1" 2>/dev/null | sed -n 's/^name : //p'; }
# condition helpers re-evaluate on every poll (pass to assert_eventually)
count_is() { [ "$(ws_count)" -eq "$1" ]; }
active_is() { [ "$(ws_active)" = "$1" ]; }
active_window_is() {
    [ "$("$W" --getactivewindow 2>/dev/null | awk '{print $3}')" = "$1" ]
}
is_normal() {
    ! status_is "$1" minimized yes && ! status_is "$1" maximized yes &&
        ! status_is "$1" fullscreen yes
}
# the compositor occasionally drops a single minimize request right after a
# geometry/activate change; retry a few times before declaring failure
minimize_until() {
    local i=0
    while [ "$i" -lt 5 ]; do
        "$W" --windowminimize "$1" >/dev/null 2>&1
        if poll_until 5 status_is "$1" minimized yes; then return 0; fi
        "$W" --windowactivate "$1" >/dev/null 2>&1
        sleep 1; i=$((i + 1))
    done
    status_is "$1" minimized yes
}
# click the entry of the test window (needs cx/cy from the geometry probe)
click_entry() {
    [ -n "$cx" ] || return 1
    "$W" --mousemove "$cx,$cy" >/dev/null 2>&1; sleep 0.3
    "$W" --mousebutton 1 >/dev/null 2>&1; sleep 1
}
entry_text() { cat "$TMP/entry_A.txt" 2>/dev/null; }
# activation right after a minimize/restore cycle is occasionally slow to
# propagate; re-issue the request a few times before declaring failure
activate_until() {
    local i=0
    while [ "$i" -lt 4 ]; do
        "$W" --windowactivate "$1" >/dev/null 2>&1
        if poll_until 5 active_window_is "$1"; then return 0; fi
        sleep 1; i=$((i + 1))
    done
    active_window_is "$1"
}
# maximizing requires an active window; re-activate and retry together
maximize_until() {
    local i=0
    while [ "$i" -lt 5 ]; do
        activate_until "$1" >/dev/null 2>&1
        "$W" --windowmaximize "$1" >/dev/null 2>&1
        if poll_until 5 status_is "$1" maximized yes; then return 0; fi
        sleep 1; i=$((i + 1))
    done
    status_is "$1" maximized yes
}
fullscreen_until() {
    local i=0
    while [ "$i" -lt 5 ]; do
        "$W" --windowfullscreen "$1" >/dev/null 2>&1
        if poll_until 5 status_is "$1" fullscreen yes; then return 0; fi
        sleep 1; i=$((i + 1))
    done
    status_is "$1" fullscreen yes
}
# drive a window back to its normal state; compositor state changes take ~1s to
# settle, so allow a few windowmap rounds with waits in between
normalize_window() {
    local i=0
    while [ "$i" -lt 4 ]; do
        is_normal "$1" && return 0
        "$W" --windowmap "$1" >/dev/null 2>&1
        sleep 1; i=$((i + 1))
    done
    is_normal "$1"
}

# ---------------------------------------------------------------- setup -----
TMP=$(mktemp -d /tmp/wlcctrl-test-XXXXXX)
RUN="wlct$$"
CURSOR=""
WS_BEFORE=""
WS_ACTIVE=""

cleanup() {
    for frag in "$RUN-A" "$RUN-B"; do
        u=$(uuid_of "$frag") && [ -n "$u" ] && "$W" --windowclose "$u" >/dev/null 2>&1
    done
    pkill -f "$TMP/win_$RUN.py" 2>/dev/null
    if [ -n "$WS_BEFORE" ]; then
        n=0
        while [ "$(ws_count)" -gt "$WS_BEFORE" ] && [ "$n" -lt 20 ]; do
            u=$(ws_uuid_at "$WS_BEFORE")
            [ -z "$u" ] && break
            "$W" --workspaceremove "$u" >/dev/null 2>&1
            sleep 0.3; n=$((n + 1))
        done
    fi
    [ -n "$WS_ACTIVE" ] && "$W" --workspaceactive "$WS_ACTIVE" >/dev/null 2>&1
    [ -n "$CURSOR" ] && "$W" --mousemove "$(echo "$CURSOR" | tr ' ' ',')" >/dev/null 2>&1
    [ "$KEEP" -eq 1 ] || rm -rf "$TMP"
}
trap cleanup EXIT

echo "wlcctrl integration test (binary: $W)"
if ! "$W" --outputs >/dev/null 2>&1; then
    echo "cannot connect to a Wayland session; run this inside one" >&2
    exit 2
fi

HAVE_GTK=0
python3 -c "import gi; gi.require_version('Gtk','3.0')" 2>/dev/null && HAVE_GTK=1

CURSOR=$(mouse_pos)
WS_BEFORE=$(ws_count)
WS_ACTIVE=$(ws_active)
echo "session state: workspaces=$WS_BEFORE cursor=$CURSOR gtk=$HAVE_GTK"

cat > "$TMP/win_$RUN.py" <<'EOF'
import sys
import gi

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

outfile, title = sys.argv[1], sys.argv[2]
win = Gtk.Window(title=title)
win.set_default_size(420, 320)
entry = Gtk.Entry()
entry.set_text('initial')


def on_changed(e):
    with open(outfile, 'w') as f:
        f.write(e.get_text())


entry.connect('changed', on_changed)
win.add(entry)
win.connect('destroy', Gtk.main_quit)
win.show_all()
win.present()
Gtk.main()
EOF

# ------------------------------------------------- group: read-only ---------
group "parse / errors / read-only queries"

"$W" >/dev/null 2>&1; assert_rc "no args prints usage" 0 $?
"$W" --help >/dev/null 2>&1; assert_rc "--help" 0 $?
"$W" -v 2>/dev/null | grep -q "wlcctrl v"; assert_rc "-v prints version" 0 $?
"$W" --definitely-not-real >/dev/null 2>&1; assert_nonzero "unknown option" $?
"$W" --windowclose >/dev/null 2>&1; assert_nonzero "missing required argument" $?
"$W" --sleep abc >/dev/null 2>&1; assert_nonzero "--sleep with non-integer" $?
"$W" --sleep 10 >/dev/null 2>&1; assert_rc "--sleep 10" 0 $?
"$W" --exec true >/dev/null 2>&1; assert_rc "--exec true" 0 $?
"$W" -l >/dev/null 2>&1; assert_rc "-l list toplevels" 0 $?
"$W" --outputs >/dev/null 2>&1; assert_rc "--outputs" 0 $?
"$W" --workspace >/dev/null 2>&1; assert_rc "--workspace list" 0 $?
"$W" --getactivewindow >/dev/null 2>&1; assert_rc "--getactivewindow" 0 $?
"$W" --search '.*' >/dev/null 2>&1; assert_rc "--search .*" 0 $?
"$W" --search '[' >/dev/null 2>&1; assert_nonzero "--search invalid regex" $?

OUT=$("$W" --outputs 2>/dev/null | awk '/^output "/{print $2; exit}' | tr -d '"')
"$W" --getdisplaygeometry "$OUT" >/dev/null 2>&1; assert_rc "getdisplaygeometry by uuid" 0 $?
"$W" --getdisplaygeometry bad-uuid >/dev/null 2>&1
assert_nonzero "getdisplaygeometry bad uuid" $?

"$W" --fullscreencapture --path "$TMP/full.png" >/dev/null 2>&1
assert_rc "fullscreencapture" 0 $?
is_png "$TMP/full.png"; assert_rc "fullscreencapture wrote a PNG" 0 $?
"$W" --setarea 0,0,64,64 --path "$TMP/area.png" >/dev/null 2>&1
assert_rc "setarea" 0 $?
is_png "$TMP/area.png"; assert_rc "setarea wrote a PNG" 0 $?
"$W" --setarea 1,2 --path "$TMP/bad.png" >/dev/null 2>&1
assert_nonzero "setarea with malformed area" $?
"$W" --getpixelcolor 5,5 2>/dev/null | grep -q 'R:'
assert_rc "getpixelcolor" 0 $?
"$W" --getpixelcolor abc >/dev/null 2>&1; assert_nonzero "getpixelcolor bad point" $?

# ------------------------------------------------- group: windows -----------
if [ "$HAVE_GTK" -eq 1 ]; then
    group "window commands (self-created GTK windows)"

    "$W" --exec "python3 $TMP/win_$RUN.py $TMP/entry_A.txt $RUN-A" >/dev/null 2>&1
    assert_rc "launch window A via --exec" 0 $?
    wait_window "$RUN-A"; assert_rc "window A appeared" 0 $?
    A=$(uuid_of "$RUN-A")

    "$W" --getwindowname "$A" >/dev/null 2>&1; assert_rc "getwindowname by uuid" 0 $?
    "$W" --getwindowname no-such >/dev/null 2>&1; assert_nonzero "getwindowname bad query" $?

    "$W" --windowmove "$A" -x 120 -y 80 >/dev/null 2>&1
    assert_eventually "windowmove to (120,80)" 5 pos_is "$A" 120 80
    "$W" --windowsize "$A" -w 500 -h 300 >/dev/null 2>&1
    if ! poll_until 5 size_is "$A" 500 300; then
        "$W" --windowsize "$A" -w 500 -h 300 >/dev/null 2>&1
    fi
    assert_eventually "windowsize to 500x300" 6 size_is "$A" 500 300
    sleep 0.8
    "$W" --windowmove "$A" -x 50 >/dev/null 2>&1
    assert_nonzero "windowmove missing -y" $?

    if activate_until "$A"; then ok "windowactivate"; else bad "windowactivate"; fi
    sleep 0.8

    if minimize_until "$A"; then
        ok "windowminimize"
    else
        bad "windowminimize"
    fi
    sleep 0.8
    normalize_window "$A"
    assert_eventually "windowmap restores minimized" 6 status_is "$A" minimized no
    sleep 0.8

    if maximize_until "$A"; then
        ok "windowmaximize (active window)"
    else
        bad "windowmaximize (active window)"
    fi
    sleep 0.8
    normalize_window "$A"
    assert_eventually "windowmap restores maximized" 6 status_is "$A" maximized no
    sleep 0.8

    if fullscreen_until "$A"; then
        ok "windowfullscreen"
    else
        bad "windowfullscreen"
    fi
    sleep 0.8
    normalize_window "$A"
    assert_eventually "windowmap restores fullscreen" 6 status_is "$A" fullscreen no
    normalize_window "$A"; sleep 0.8

    WS2=$(ws_uuid_at 1)
    if [ -n "$WS2" ]; then
        "$W" --windowenter "$A:$WS2" >/dev/null 2>&1; assert_rc "windowenter" 0 $?
        "$W" --windowleave "$A:$WS2" >/dev/null 2>&1; assert_rc "windowleave" 0 $?
    else
        skp "windowenter/leave (needs >=2 workspaces)"
    fi
    "$W" --windowenter "$A" >/dev/null 2>&1; assert_nonzero "windowenter without colon" $?

    "$W" --windowcapture "$RUN-A" --path "$TMP/win.png" >/dev/null 2>&1
    assert_rc "windowcapture by name fragment" 0 $?
    is_png "$TMP/win.png"; assert_rc "windowcapture wrote a PNG" 0 $?

    group "fuzzy window resolution"
    "$W" --exec "python3 $TMP/win_$RUN.py $TMP/entry_B.txt $RUN-B" >/dev/null 2>&1
    wait_window "$RUN-B"; assert_rc "window B appeared" 0 $?
    B=$(uuid_of "$RUN-B")

    "$W" --getwindowgeometry "$RUN-A" >/dev/null 2>&1
    assert_rc "resolve unique fragment" 0 $?
    "$W" --getwindowstatus "$RUN" >/dev/null 2>&1
    assert_nonzero "ambiguous fragment rejected" $?
    "$W" --getwindowstatus zz-no-match-zz >/dev/null 2>&1
    assert_nonzero "no-match rejected" $?
    "$W" --windowclose "$RUN-B" >/dev/null 2>&1; assert_rc "windowclose by name" 0 $?
    sleep 1
    assert_eq "window B gone" "" "$(uuid_of "$RUN-B")"

    cx=""; cy=""
    if [ "$RUN_INPUT" -eq 1 ]; then
        group "keyboard injection (content verified)"
        normalize_window "$A"
        if activate_until "$A"; then
            ok "focus window for input"
        else
            bad "focus window for input"
        fi
        sleep 0.8
        geo=( $("$W" --getwindowgeometry "$A" 2>/dev/null |
            sed -n 's/^geometry: (\([0-9]*\), \([0-9]*\)) \([0-9]*\) x \([0-9]*\).*/\1 \2 \3 \4/p') )
        if [ "${#geo[@]}" -eq 4 ]; then
            cx=$((geo[0] + geo[2] / 2)); cy=$((geo[1] + geo[3] / 2))
        else
            skp "geometry query before click"
        fi

        click_entry
        "$W" --key ctrl+a >/dev/null 2>&1; "$W" --key BackSpace >/dev/null 2>&1; sleep 0.6
        "$W" --keystring 'hi' >/dev/null 2>&1; sleep 0.8
        if [ "$(entry_text)" != "hi" ]; then
            click_entry
            "$W" --key ctrl+a >/dev/null 2>&1; "$W" --key BackSpace >/dev/null 2>&1; sleep 0.6
            "$W" --keystring 'hi' >/dev/null 2>&1; sleep 0.8
        fi
        assert_eq "keystring types 'hi'" "hi" "$(entry_text)"

        "$W" --key ctrl+a >/dev/null 2>&1; "$W" --key BackSpace >/dev/null 2>&1; sleep 0.8
        assert_eq "ctrl+a + BackSpace clears" "" "$(entry_text)"

        "$W" --key shift_l+a >/dev/null 2>&1; sleep 0.8
        if [ "$(entry_text)" != "A" ]; then
            click_entry
            "$W" --key shift_l+a >/dev/null 2>&1; sleep 0.8
        fi
        assert_eq "combo shift_l+a types 'A'" "A" "$(entry_text)"

        "$W" --key nosuchkey_zz >/dev/null 2>&1; assert_nonzero "bogus key exits non-zero" $?

        group "pointer injection"
        if [ -n "$cx" ]; then
            "$W" --mousemove "$cx,$cy" >/dev/null 2>&1; sleep 0.5
            mp=$(mouse_pos); mx=${mp% *}; my=${mp#* }
            dx=$((mx - cx)); [ "$dx" -lt 0 ] && dx=$(( -dx ))
            dy=$((my - cy)); [ "$dy" -lt 0 ] && dy=$(( -dy ))
            if [ "$dx" -le 20 ] && [ "$dy" -le 20 ]; then
                ok "mousemove lands near target ($mx,$my vs $cx,$cy)"
            else
                bad "mousemove off target ($mx,$my vs $cx,$cy)"
            fi
        else
            skp "mousemove target check (no geometry)"
        fi
        "$W" --mousebutton 1 >/dev/null 2>&1; assert_rc "mousebutton 1" 0 $?
        "$W" --mousepress 1 >/dev/null 2>&1; assert_rc "mousepress 1" 0 $?
        "$W" --mouserelease 1 >/dev/null 2>&1; assert_rc "mouserelease 1" 0 $?
        "$W" --scroll -y 2 >/dev/null 2>&1; assert_rc "scroll -y 2" 0 $?
        "$W" --mousebutton 8 >/dev/null 2>&1; assert_nonzero "mousebutton out of range" $?
        "$W" --mousemove abc >/dev/null 2>&1; assert_nonzero "mousemove malformed" $?
    else
        skp "keyboard/pointer injection (--quick/--no-input)"
    fi

    group "windowkill"
    APPID=$(appid_of "$A")
    if [ -n "$APPID" ]; then
        "$W" --windowkill "$APPID" >/dev/null 2>&1; assert_rc "windowkill by app_id" 0 $?
        sleep 1.5
        assert_eq "window A gone after kill" "" "$(uuid_of "$RUN-A")"
    else
        skp "windowkill (could not read app_id)"
    fi
else
    group "window commands"
    skp "window/fuzzy/input groups (python3-gtk3 not available)"
fi

# ------------------------------------------------- group: workspaces --------
group "workspace commands"
if [ "$RUN_WSMUT" -eq 1 ]; then
    "$W" --workspaceadd >/dev/null 2>&1; assert_rc "workspaceadd (default 1)" 0 $?
    assert_eventually "count +1" 12 count_is "$((WS_BEFORE + 1))"
    "$W" --workspaceadd 2 >/dev/null 2>&1; assert_rc "workspaceadd 2" 0 $?
    assert_eventually "count +3 total" 12 count_is "$((WS_BEFORE + 3))"
    "$W" --workspaceadd abc >/dev/null 2>&1; assert_nonzero "workspaceadd non-integer" $?
    "$W" --workspaceadd 99 >/dev/null 2>&1; assert_nonzero "workspaceadd over cap" $?

    NEWWS=$(ws_uuid_at "$WS_BEFORE")
    if [ -n "$NEWWS" ]; then
        "$W" --workspaceactive "$NEWWS" >/dev/null 2>&1
        if ! poll_until 6 active_is "$NEWWS"; then
            "$W" --workspaceactive "$NEWWS" >/dev/null 2>&1
        fi
        assert_eventually "workspaceactive switches" 8 active_is "$NEWWS"
    fi

    # leave the added workspaces before removing them
    [ -n "$WS_ACTIVE" ] && "$W" --workspaceactive "$WS_ACTIVE" >/dev/null 2>&1
    sleep 1
    n=0
    while [ "$(ws_count)" -gt "$WS_BEFORE" ] && [ "$n" -lt 20 ]; do
        u=$(ws_uuid_at "$WS_BEFORE")
        [ -z "$u" ] && break
        "$W" --workspaceremove "$u" >/dev/null 2>&1
        sleep 0.4; n=$((n + 1))
    done
    assert_eventually "count restored" 8 count_is "$WS_BEFORE"
    "$W" --workspaceremove bad-uuid >/dev/null 2>&1
    assert_nonzero "workspaceremove bad uuid" $?
else
    skp "workspace mutation (--quick)"
fi

# ------------------------------------------------------------- summary ------
printf '\n== summary ==\n%d passed, %d failed, %d skipped\n' "$pass" "$fail" "$skip"
[ "$fail" -eq 0 ]
