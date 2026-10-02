import json, subprocess, re, os, sys
r = json.load(open(sys.argv[1]))
BASE = "/home/user/openkylin-src/ref-p2/deb"
out = {}
for pk, files in sorted(r.items()):
    vals = []
    for f, x in files.items():
        p = BASE + "/" + pk + f
        t = subprocess.run(["objdump", "-d", "--no-show-raw-insn", "-j", ".text", p], capture_output=True, text=True, errors="replace").stdout
        ins = [l for l in t.splitlines() if re.match(r"^ +[0-9a-f]+:\t", l)]
        if len(ins) < 500: continue
        b = sum(1 for l in ins if "(%rbp)" in l)
        vals.append(round(100.0 * b / len(ins), 1))
    if vals:
        vals.sort()
        out[pk] = (len(vals), vals[len(vals)//2], sum(1 for v in vals if v >= 20))
for k, v in out.items():
    print(f"{k:70s} n={v[0]:3d} mediane_rbp%={v[1]:5.1f} n>=20%={v[2]}")
