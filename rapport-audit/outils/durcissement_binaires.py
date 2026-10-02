#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Phase 2, vérification — durcissement réel des binaires distribués (lecture seule, rien n'est exécuté).

Question : la politique de durcissement de dpkg-buildflags ne s'applique pas sur openKylin (module
Dpkg::Vendor::openKylin jamais chargé) ; les binaires distribués sont-ils pour autant non durcis, ou le
compilateur applique-t-il ses protections par défaut ?

Méthode : paquets binaires téléchargés par la chaîne authentifiée de references_p2.py (InRelease gpgv →
Packages → .deb, SHA-256), extraits par `dpkg-deb -x` (aucun script de mainteneur n'est exécuté), puis
chaque fichier ELF est lu par `readelf` : PIE, RELRO, BIND_NOW, protecteur de pile (__stack_chk_fail),
FORTIFY (fonctions __*_chk), CET (propriété x86 IBT/SHSTK). Comparaison avec les mêmes paquets Ubuntu.
Pour dpkg-wrapper et ostree-extra-utils (binaires sans source correspondante), relevé de la liste des
fichiers, des scripts de mainteneur et des petits fichiers de configuration (texte, sans exécution).

Sortie : $AUDIT_RAPPORT/donnees/phase2/durcissement-binaires.json
"""
import os, sys
sys.path[:] = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
sys.path.append(os.path.dirname(os.path.abspath(__file__)))  # en dernier : pour importer references_p2 seulement
import json, re, subprocess, tarfile, io  # noqa: E402
import references_p2 as R  # noqa: E402

COMPARES = ["dpkg", "sudo", "passwd", "openssh-server", "mount", "pkexec", "polkitd", "util-linux", "login",
            "coreutils", "bash", "systemd", "openssl", "curl", "dbus-daemon", "network-manager", "wpasupplicant",
            "xserver-xorg-core", "libc-bin"]
OPENKYLIN_SEULS = ["kylin-daq", "libkysdk-system", "ukui-session-manager", "biometric-auth", "kylin-nm",
                   "kylin-os-manager", "kylin-wayland-compositor", "peony", "ukui-control-center", "ukui-panel",
                   "ukui-screensaver", "ukui-settings-daemon", "kylin-ai-runtime", "lightdm"]
INSPECTER = ["dpkg-wrapper", "ostree-extra-utils"]
OUT = os.path.join(R.RAP, "donnees", "phase2", "durcissement-binaires.json")


def packages(arch, suite, keyring, comps):
    sums = R.inrelease(arch, suite, keyring)
    out = {}
    for comp in comps:
        for st in R.parse_deb822(R.index(arch, suite, sums, f"{comp}/binary-amd64/Packages")):
            out[st["Package"]] = st
    return out


def telecharger_deb(arch, suite, st):
    url = f"{R.MIR[arch]}/{st['Filename']}"
    dest = os.path.join(R.WORK, "deb", arch, os.path.basename(st["Filename"]))
    if R.sha(R.get(url, dest)) != st["SHA256"]:
        raise SystemExit(f"EMPREINTE : {url}")
    R.PROV.append([arch, suite, st["Package"], st["Version"], url, st["SHA256"], "SHA-256 conforme à l'index Packages signé"])
    d = dest[:-4]
    if not os.path.isdir(d):
        subprocess.run(["dpkg-deb", "-x", dest, d], check=True)  # extraction seule
    return dest, d


def elf_props(path):
    r = subprocess.run(["readelf", "-W", "-h", "-l", "-d", "--dyn-syms", "-n", path],
                       capture_output=True, text=True, errors="replace")
    t = r.stdout
    typ = re.search(r"Type:\s+(\w+)", t)
    typ = typ.group(1) if typ else "?"
    und = set(m.group(1) for m in re.finditer(r"\sUND\s+([^\s@]+)", t))
    interp = "INTERP" in t
    return {
        "type": typ,
        "executable": interp,
        "pie": typ == "DYN" and (interp or "PIE" in t),
        "relro": "GNU_RELRO" in t,
        "bind_now": bool(re.search(r"\(BIND_NOW\)|\(FLAGS\).*BIND_NOW|\(FLAGS_1\).*\bNOW\b", t)),
        "ssp": "__stack_chk_fail" in und,
        "fortify": sorted(s for s in und if re.fullmatch(r"__\w+_chk", s) and s != "__stack_chk_fail")[:8],
        "cet": bool(re.search(r"x86 feature:.*IBT.*SHSTK|x86 feature:.*SHSTK.*IBT", t)),
    }


def analyser(racine):
    res = {}
    for r, _, fs in os.walk(racine):
        for f in fs:
            p = os.path.join(r, f)
            if os.path.islink(p) or not os.path.isfile(p):
                continue
            with open(p, "rb") as fh:
                if fh.read(4) != b"\x7fELF":
                    continue
            props = elf_props(p)
            props["setuid_setgid"] = bool(os.stat(p).st_mode & 0o6000)
            res["/" + os.path.relpath(p, racine)] = props
    return res


def bilan(elfs):
    ex = {k: v for k, v in elfs.items() if v["executable"]}
    tous = list(elfs.values())
    n = lambda xs, k: sum(1 for x in xs if (x[k] if not isinstance(x[k], list) else bool(x[k])))
    return {"elf": len(tous), "executables": len(ex), "setuid_setgid": n(tous, "setuid_setgid"),
            "pie_executables": n(list(ex.values()), "pie"), "relro": n(tous, "relro"), "bind_now": n(tous, "bind_now"),
            "ssp": n(tous, "ssp"), "fortify": n(tous, "fortify"), "cet": n(tous, "cet")}


def controle(deb):
    """Scripts de mainteneur et fichier conffiles du .deb (texte), sans exécution."""
    out = {}
    # dpkg-deb --ctrl-tarfile renvoie l'archive de contrôle décompressée (gzip, xz ou zstd), sans rien exécuter
    data = subprocess.run(["dpkg-deb", "--ctrl-tarfile", deb], capture_output=True, check=True).stdout
    with tarfile.open(fileobj=io.BytesIO(data)) as t:
        for m in t.getmembers():
            if m.isfile():
                out[m.name.lstrip("./")] = t.extractfile(m).read().decode("utf-8", "replace")
    return out


def main():
    kr_ok = os.path.join(R.CORPUS, "archive", "openkylin-keyring", "keyrings", "openkylin-archive-keyring.gpg")
    ok = packages("openkylin", "huanghe", kr_ok, ["main"])
    ub = {}
    for suite in ("resolute", "resolute-updates", "resolute-security"):  # la dernière version l'emporte
        try:
            ub.update(packages("ubuntu", suite, "/usr/share/keyrings/ubuntu-archive-keyring.gpg", ["main"]))
        except Exception as e:
            R.log("  suite Ubuntu indisponible", suite, e)
    resultat = {"methode": __doc__.strip().split("\n\n")[1], "compares": {}, "openkylin_seuls": {}, "inspections": {}}
    for pk in COMPARES:
        lig = {}
        for arch, idx, suite in (("openkylin", ok, "huanghe"), ("ubuntu", ub, "resolute")):
            if pk in idx:
                deb, d = telecharger_deb(arch, suite, idx[pk])
                elfs = analyser(d)
                lig[arch] = {"version": idx[pk]["Version"], "bilan": bilan(elfs), "detail": elfs}
        resultat["compares"][pk] = lig
        R.log(pk, {a: v["bilan"] for a, v in lig.items()})
    for pk in OPENKYLIN_SEULS:
        if pk in ok:
            deb, d = telecharger_deb("openkylin", "huanghe", ok[pk])
            elfs = analyser(d)
            resultat["openkylin_seuls"][pk] = {"version": ok[pk]["Version"], "bilan": bilan(elfs), "detail": elfs}
            R.log(pk, resultat["openkylin_seuls"][pk]["bilan"])
    for pk in INSPECTER:
        if pk in ok:
            deb, d = telecharger_deb("openkylin", "huanghe", ok[pk])
            fichiers = sorted("/" + os.path.relpath(os.path.join(r, f), d) for r, _, fs in os.walk(d) for f in fs)
            petits = {}
            for f in fichiers:
                p = os.path.join(d, f.lstrip("/"))
                if os.path.isfile(p) and not os.path.islink(p) and os.path.getsize(p) <= 4096:
                    b = open(p, "rb").read()
                    if b"\x00" not in b:
                        petits[f] = b.decode("utf-8", "replace")
            resultat["inspections"][pk] = {
                "version": ok[pk]["Version"], "champs": {k: ok[pk].get(k, "") for k in
                                                         ("Depends", "Pre-Depends", "Provides", "Replaces", "Conflicts",
                                                          "Breaks", "Priority", "Essential", "Source")},
                "fichiers": fichiers, "fichiers_texte_courts": petits, "controle": controle(deb),
                "elf": {k: v for k, v in analyser(d).items()}}
    def total(lignes):
        t = {}
        for b in lignes:
            for k, v in b.items():
                t[k] = t.get(k, 0) + v
        return t
    resultat["synthese"] = {
        "openkylin_compares": total(l["openkylin"]["bilan"] for l in resultat["compares"].values() if "openkylin" in l),
        "ubuntu_compares": total(l["ubuntu"]["bilan"] for l in resultat["compares"].values() if "ubuntu" in l),
        "openkylin_seuls": total(v["bilan"] for v in resultat["openkylin_seuls"].values()),
        "setuid_setgid_openkylin": {f"{pk}:{f}": {k: v[k] for k in ("pie", "relro", "bind_now", "ssp", "fortify", "cet")}
                                    for pk, l in resultat["compares"].items() if "openkylin" in l
                                    for f, v in l["openkylin"]["detail"].items() if v["setuid_setgid"]},
    }
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    json.dump(resultat, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    with open(os.path.join(os.path.dirname(OUT), "durcissement-binaires-provenance.tsv"), "w", encoding="utf-8") as fh:
        fh.write("archive\tsuite\tpaquet\tversion\turl\tsha256\tverification\n")
        for l in R.PROV:
            fh.write("\t".join(str(x) for x in l) + "\n")
    print(json.dumps(resultat["synthese"], ensure_ascii=False, indent=1))


if __name__ == "__main__":
    main()
