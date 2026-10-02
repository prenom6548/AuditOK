import os, re, subprocess, sys, json
BASE = "/home/user/openkylin-src/ref-p2/deb"
def run(args, p):
    return subprocess.run(["readelf", "-W"] + args + [p], capture_output=True, text=True, errors="replace").stdout
def props(p):
    h = run(["-h"], p); l = run(["-l"], p); d = run(["-d"], p); s = run(["--dyn-syms"], p); n = run(["-n"], p)
    typ = re.search(r"^\s*Type:\s+(\S+)", h, re.M).group(1)
    interp = bool(re.search(r"^\s*INTERP\s", l, re.M))
    relro = bool(re.search(r"^\s*GNU_RELRO\s", l, re.M))
    flags = " ".join(m.group(0) for m in re.finditer(r"\((FLAGS|FLAGS_1|BIND_NOW)\).*", d))
    now = ("BIND_NOW" in flags) or bool(re.search(r"FLAGS_1\)\s+Flags:.*\bNOW\b", flags))
    und = set()
    for line in s.splitlines():
        f = line.split()
        if len(f) >= 8 and f[6] == "UND":
            und.add(f[7].split("@")[0])
    ssp = "__stack_chk_fail" in und or "__stack_chk_fail_local" in und
    chk = sorted(x for x in und if x.startswith("__") and x.endswith("_chk"))
    feat = re.search(r"x86 feature:\s*([^,\n]*(?:,\s*(?:IBT|SHSTK))*)", n)
    feat = feat.group(1) if feat else ""
    ibt = "IBT" in feat; shstk = "SHSTK" in feat
    st = os.stat(p).st_mode
    return dict(type=typ, interp=interp, pie=(typ=="DYN" and interp), static=(not interp and typ=="EXEC"),
                relro=relro, now=now, ssp=ssp, chk=chk, ibt=ibt, shstk=shstk, mode=oct(st & 0o7777))
def walk(root):
    out = {}
    for r, _, fs in os.walk(root):
        for f in fs:
            p = os.path.join(r, f)
            if os.path.islink(p) or not os.path.isfile(p): continue
            with open(p, "rb") as fh:
                if fh.read(4) != b"\x7fELF": continue
            out["/" + os.path.relpath(p, root)] = props(p)
    return out
if __name__ == "__main__":
    res = {}
    for arch in ("openkylin", "ubuntu"):
        for e in sorted(os.listdir(os.path.join(BASE, arch))):
            d = os.path.join(BASE, arch, e)
            if os.path.isdir(d):
                res[f"{arch}/{e}"] = walk(d)
    json.dump(res, open(sys.argv[1], "w"), indent=0)
