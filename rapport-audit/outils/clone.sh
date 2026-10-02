#!/bin/bash
S=${AUDIT_WORK:-travail}
n="$1"; dst=${AUDIT_SRC:-sources}/gitee/$n
[ -d "$dst/.git" ] && { echo "$n ALREADY"; exit 0; }
export GIT_TERMINAL_PROMPT=0
heads=$(awk '{print $2}' $S/lsr/$n.gitee | grep '^refs/heads/' | sed 's#refs/heads/##')
br=""
for c in openkylin/huanghe huanghe packaging/openkylin/huanghe openkylin/nile master main upstream; do echo "$heads" | grep -qx "$c" && { br=$c; break; }; done
args=(--depth 1 --single-branch --no-tags)
[ -n "$br" ] && args+=(-b "$br")
for i in 1 2 3; do
  timeout 1800 git clone -q "${args[@]}" https://gitee.com/openkylin/$n.git "$dst" 2>$S/clone_err_$n.txt && break
  rm -rf "$dst"; sleep $((i*5))
done
if [ -d "$dst/.git" ]; then echo "$n OK branch=${br:-HEAD} commit=$(git -C $dst rev-parse HEAD) size=$(du -sm $dst | cut -f1)MB"; else echo "$n FAIL $(tail -1 $S/clone_err_$n.txt)"; fi
