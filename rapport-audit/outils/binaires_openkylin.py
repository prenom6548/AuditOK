#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Phase 1, vague 3 — acquisition et inventaire du contenu BINAIRE de composants sans source exploitable.

Usage : binaires_openkylin.py <nom-du-lot> <paquet-source> [<paquet-source> …]

Pour chaque paquet source, retrouve tous ses paquets binaires amd64/all dans l'archive openKylin huanghe
(composants main, pty, cross), les télécharge par la chaîne authentifiée de references_p2.py (InRelease vérifié
par gpgv → index Packages → .deb, SHA-256), les extrait par `dpkg-deb -x` (aucun script de mainteneur n'est
exécuté) et dresse l'inventaire : fichiers, nature (ELF, script, texte, autre), droits, scripts de mainteneur,
présence sur l'ISO 3.0. Rien n'est exécuté.

Sorties : $AUDIT_RAPPORT/donnees/phase1/vague3/<lot>-binaires.json et <lot>-provenance.tsv ;
extractions dans $AUDIT_WORK/ref-p2/deb/openkylin/.
"""
import os, sys
sys.path[:] = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
sys.path.append(os.path.dirname(os.path.abspath(__file__)))  # en dernier : pour importer references_p2 seulement
import csv, io, json, stat, subprocess, tarfile  # noqa: E402
import references_p2 as R  # noqa: E402

ISO = os.path.join(R.RAP, "donnees", "iso-3.0", "provenance-paquets-installes.csv")


def nature(p):
    with open(p, "rb") as fh:
        tete = fh.read(4096)
    if tete.startswith(b"\x7fELF"):
        return "elf"
    if tete.startswith(b"#!"):
        return "script"
    if b"\x00" not in tete:
        return "texte"
    return "autre"


def controle(deb):
    # dpkg-deb --ctrl-tarfile renvoie l'archive de contrôle décompressée (gzip, xz ou zstd), sans rien exécuter
    data = subprocess.run(["dpkg-deb", "--ctrl-tarfile", deb], capture_output=True, check=True).stdout
    out = {}
    with tarfile.open(fileobj=io.BytesIO(data)) as t:
        for m in t.getmembers():
            if m.isfile() and m.name.lstrip("./") != "md5sums":
                out[m.name.lstrip("./")] = t.extractfile(m).read().decode("utf-8", "replace")
    return out


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    lot, sources = sys.argv[1], set(sys.argv[2:])
    installes = {}
    if os.path.exists(ISO):
        for r in csv.reader(open(ISO, encoding="utf-8")):
            if r:
                installes[r[0]] = r[1]
    kr_ok = os.path.join(R.CORPUS, "archive", "openkylin-keyring", "keyrings", "openkylin-archive-keyring.gpg")
    sums = R.inrelease("openkylin", "huanghe", kr_ok)
    paquets = []
    for comp in ("main", "pty", "cross"):
        for st in R.parse_deb822(R.index("openkylin", "huanghe", sums, f"{comp}/binary-amd64/Packages")):
            src = st.get("Source", st["Package"]).split()[0]
            if src in sources:
                paquets.append((comp, st))
    res = {"lot": lot, "sources": sorted(sources), "methode": __doc__.strip().split("\n\n")[1], "paquets": {}}
    for comp, st in paquets:
        url = f"{R.MIR['openkylin']}/{st['Filename']}"
        deb = os.path.join(R.WORK, "deb", "openkylin", os.path.basename(st["Filename"]))
        if R.sha(R.get(url, deb)) != st["SHA256"]:
            raise SystemExit(f"EMPREINTE : {url}")
        R.PROV.append(["openkylin", f"huanghe/{comp}", st["Package"], st["Version"], url, st["SHA256"],
                       "SHA-256 conforme à l'index Packages signé"])
        d = deb[:-4]
        if not os.path.isdir(d):
            subprocess.run(["dpkg-deb", "-x", deb, d], check=True)  # extraction seule
        fichiers = []
        for r, _, fs in os.walk(d):
            for f in fs:
                p = os.path.join(r, f)
                if os.path.islink(p):
                    fichiers.append({"chemin": "/" + os.path.relpath(p, d), "nature": "lien", "cible": os.readlink(p)})
                    continue
                m = os.stat(p).st_mode
                fichiers.append({"chemin": "/" + os.path.relpath(p, d), "nature": nature(p), "taille": os.path.getsize(p),
                                 "mode": stat.filemode(m), "setuid_setgid": bool(m & 0o6000)})
        fichiers.sort(key=lambda x: x["chemin"])
        compte = {}
        for x in fichiers:
            compte[x["nature"]] = compte.get(x["nature"], 0) + 1
        res["paquets"][st["Package"]] = {
            "version": st["Version"], "composant": comp, "source": st.get("Source", st["Package"]),
            "installe_iso_3_0": st["Package"] in installes,
            "champs": {k: st.get(k, "") for k in ("Depends", "Pre-Depends", "Recommends", "Provides", "Replaces",
                                                   "Conflicts", "Breaks", "Priority", "Essential", "Section",
                                                   "Installed-Size", "Maintainer", "Description")},
            "extraction": d, "compte_par_nature": compte, "fichiers": fichiers, "controle": controle(deb)}
        R.log(st["Package"], st["Version"], comp, compte)
    sortie = os.path.join(R.RAP, "donnees", "phase1", "vague3")
    os.makedirs(sortie, exist_ok=True)
    json.dump(res, open(os.path.join(sortie, f"{lot}-binaires.json"), "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    with open(os.path.join(sortie, f"{lot}-provenance.tsv"), "w", encoding="utf-8") as fh:
        fh.write("archive\tsuite\tpaquet\tversion\turl\tsha256\tverification\n")
        for l in R.PROV:
            fh.write("\t".join(str(x) for x in l) + "\n")
    print(json.dumps({p: {"version": v["version"], "iso": v["installe_iso_3_0"], "natures": v["compte_par_nature"]}
                      for p, v in res["paquets"].items()}, ensure_ascii=False, indent=1))


if __name__ == "__main__":
    main()
