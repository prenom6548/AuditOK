#!/bin/bash
# usage: lsr.sh <name>  -> writes $S/lsr/<name>.gitee / .github (refs or ERR)
S=${AUDIT_WORK:-travail}
n="$1"
export GIT_TERMINAL_PROMPT=0 GIT_ASKPASS=/bin/false
for host in gitee github; do
  out="$S/lsr/$n.$host"
  [ -s "$out" ] && continue
  url="https://$host.com/openkylin/$n.git"
  for i in 1 2 3; do
    r=$(timeout 60 git ls-remote --heads --tags "$url" 2>&1); rc=$?
    if [ $rc -eq 0 ]; then echo "$r" > "$out"; [ -z "$r" ] && echo "EMPTY" > "$out"; break; fi
    if echo "$r" | grep -qiE "not found|could not read Username|Authentication failed|repository .* not found|terminal prompts disabled|403|404"; then echo "ABSENT $(echo "$r"|head -1)" > "$out"; break; fi
    sleep $((i*3))
    [ $i -eq 3 ] && echo "NETERR $(echo "$r"|head -1)" > "$out"
  done
done
