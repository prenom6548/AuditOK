#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Phase 2 (suite) — sources de référence des forks P2 restants : acquisition authentifiée + comparaison.

Lecture seule sur le corpus audité (../../sources-openkylin/). Aucun code téléchargé n'est exécuté :
les archives sont seulement vérifiées (gpgv, SHA-256) puis décompressées.

Chaîne de confiance, pour chaque fichier téléchargé :
  InRelease (gpgv + trousseau)  →  index Sources/Packages (SHA-256 listé dans l'InRelease)
  →  .dsc (SHA-256 listé dans l'index)  →  orig / debian.tar (SHA-256 listés dans le .dsc)
Trousseaux : openKylin = keyrings/openkylin-archive-keyring.gpg du paquet source openkylin-keyring du corpus ;
Ubuntu = /usr/share/keyrings/ubuntu-archive-keyring.gpg ; Debian = paquet debian-archive-keyring obtenu
par la chaîne Ubuntu ; archives GNU = debian/upstream/signing-key.asc du paquet de référence vérifié.

Miroirs (européens) : mirrors.ircam.fr (Ubuntu), ftp.fr.debian.org (Debian), mirror.cyberbits.eu (GNU) ;
pré-versions GNU : alpha.gnu.org (non reprises par les miroirs ; la signature amont fait foi).

Variables : AUDIT_WORK (défaut /home/user/openkylin-src) — téléchargements dans $AUDIT_WORK/ref-p2 ;
            AUDIT_RAPPORT (défaut ..) — comparaisons dans donnees/phase2/references/.
"""
import os, sys
# Les scripts voisins ne doivent pas masquer de modules standard (ex. ancien struct.py) : on retire ce dossier du chemin.
sys.path[:] = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
import gzip, hashlib, json, lzma, re, subprocess, tarfile, tempfile, time, urllib.error, urllib.request  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
RAP = os.environ.get("AUDIT_RAPPORT", os.path.join(HERE, ".."))
CORPUS = os.path.join(RAP, "..", "sources-openkylin")
WORK = os.path.join(os.environ.get("AUDIT_WORK", "/home/user/openkylin-src"), "ref-p2")
OUT = os.path.join(RAP, "donnees", "phase2", "references")

MIR = {
    "openkylin": "http://archive.build.openkylin.top/openkylin",
    "ubuntu": "https://mirrors.ircam.fr/pub/ubuntu/archive",
    "debian": "http://ftp.fr.debian.org/debian",
    "gnu": "https://mirror.cyberbits.eu/gnu",
}
SUITES = {
    "ubuntu": ["resolute", "resolute-updates", "resolute-security", "questing", "noble", "noble-updates",
               "noble-security"],
    "debian": ["sid", "trixie"],
}
COMPONENTS = {"ubuntu": ["main", "universe"], "debian": ["main"]}
PAQUETS = ["glibc", "grub2", "grub2-signed", "dpkg", "aptdaemon", "network-manager", "wpa", "apport"]
GNU = {"glibc": ("glibc", "glibc-2.43.tar.xz"), "grub2": ("grub", "grub-2.12.tar.xz")}
# Pré-versions GNU (non reprises par les miroirs) : seule la signature amont fait foi, pas l'hôte.
GNU_PRE = {"grub2": ["https://alpha.gnu.org/gnu/grub/grub-2.12~rc1.tar.xz"]}

PROV = []  # lignes de provenance


def log(*a):
    print(*a, file=sys.stderr, flush=True)


def get(url, dest=None):
    if dest and os.path.exists(dest):
        return open(dest, "rb").read()
    req = urllib.request.Request(url, headers={"User-Agent": "audit-openkylin-ref/1.0"})
    for essai in range(7):
        try:
            data = urllib.request.urlopen(req, timeout=120).read()
            break
        except Exception as e:  # réseau ou serveur saturé : 7 essais, attente progressive 2 s → 64 s
            if essai == 6 or (isinstance(e, urllib.error.HTTPError) and e.code == 404):
                raise
            log("  nouvel essai dans", 2 ** (essai + 1), "s :", url, e)
            time.sleep(2 ** (essai + 1))
    if dest:
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        open(dest, "wb").write(data)
    return data


def sha(b):
    return hashlib.sha256(b).hexdigest()


def gpgv(keyring, sigfile, datafile=None):
    cmd = ["gpgv", "--keyring", keyring, sigfile] + ([datafile] if datafile else [])
    r = subprocess.run(cmd, capture_output=True, text=True)
    m = re.search(r"using \S+ key (\S+)", r.stderr)
    return r.returncode == 0, (m.group(1) if m else "?")


def keyring_from_asc(asc):
    d = tempfile.mkdtemp(dir=WORK)
    kr = os.path.join(d, "k.gpg")
    subprocess.run(["gpg", "--batch", "--quiet", "--dearmor", "-o", kr, asc], check=True)
    return kr


def parse_deb822(text):
    for bloc in re.split(r"\n\s*\n", text):
        st, cle = {}, None
        for l in bloc.split("\n"):
            if not l.strip():
                continue
            if l[0] in " \t" and cle:
                st[cle] += "\n" + l.strip()
            elif ":" in l:
                cle, v = l.split(":", 1)
                st[cle] = v.strip()
        if st:
            yield st


def checksums(field):
    out = {}
    for l in (field or "").split("\n"):
        p = l.split()
        if len(p) == 3:
            out[p[2]] = (p[0], int(p[1]))
    return out


def inrelease(arch, suite, keyring):
    base = f"{MIR[arch]}/dists/{suite}"
    dest = os.path.join(WORK, "dists", arch, suite, "InRelease")
    get(base + "/InRelease", dest)
    ok, cle = gpgv(keyring, dest)
    if not ok:
        raise SystemExit(f"SIGNATURE INVALIDE : {arch} {suite}")
    txt = open(dest, encoding="utf-8", errors="replace").read()
    corps = txt.split("\n\n", 1)[1].split("-----BEGIN PGP SIGNATURE")[0]
    st = next(parse_deb822(corps))
    PROV.append([arch, suite, "InRelease", "", base + "/InRelease", sha(open(dest, "rb").read()),
                 f"gpgv OK clé {cle} ; Date {st.get('Date', '?')}"])
    return checksums(st.get("SHA256"))


def index(arch, suite, sums, chemin):
    for ext, dec in ((".xz", lzma.decompress), (".gz", gzip.decompress)):
        if chemin + ext in sums:
            h, _ = sums[chemin + ext]
            dest = os.path.join(WORK, "dists", arch, suite, chemin + ext)
            data = get(f"{MIR[arch]}/dists/{suite}/{chemin}{ext}", dest)
            if sha(data) != h:
                raise SystemExit(f"EMPREINTE INDEX : {arch} {suite} {chemin}{ext}")
            return dec(data).decode("utf-8", "replace")
    return ""


def telecharger_source(arch, suite, st):
    pk, ver, rep = st["Package"], st["Version"], st["Directory"]
    d = os.path.join(WORK, arch, f"{pk}_{ver.split(':')[-1]}")
    fichiers = checksums(st.get("Checksums-Sha256"))
    dsc = next(f for f in fichiers if f.endswith(".dsc"))
    for f, (h, _) in sorted(fichiers.items(), key=lambda x: not x[0].endswith(".dsc")):
        url = f"{MIR[arch]}/{rep}/{f}"
        data = get(url, os.path.join(d, f))
        if sha(data) != h:
            raise SystemExit(f"EMPREINTE : {url}")
        PROV.append([arch, suite, pk, ver, url, h, "SHA-256 conforme à l'index Sources signé"])
    # le .dsc liste à son tour les fichiers : contrôle croisé
    dst = next(parse_deb822(open(os.path.join(d, dsc), encoding="utf-8", errors="replace").read()
                            .split("-----BEGIN PGP SIGNATURE")[0].split("\n\n", 1)[-1]))
    for f, (h, _) in checksums(dst.get("Checksums-Sha256")).items():
        if sha(open(os.path.join(d, f), "rb").read()) != h:
            raise SystemExit(f"EMPREINTE .dsc : {d}/{f}")
    return d, fichiers


_CACHE = {}


def extraire_cache(archive, dest):
    h = sha(open(archive, "rb").read())
    if h not in _CACHE:
        _CACHE[h] = extraire(archive, dest)
    return _CACHE[h]


def extraire(archive, dest):
    """Décompresse une archive tar dans dest, sans le premier composant de chemin ; renvoie {chemin: sha256}."""
    os.makedirs(dest, exist_ok=True)
    out = {}
    with tarfile.open(archive) as t:
        for m in t.getmembers():
            if not m.isfile():
                continue
            p = m.name.split("/", 1)[1] if "/" in m.name else m.name
            p = os.path.normpath(p)
            if p.startswith("..") or os.path.isabs(p):  # chemin piégé : ignoré et signalé
                log("  chemin refusé dans", archive, ":", m.name)
                continue
            data = t.extractfile(m).read()
            out[p] = sha(data)
            cible = os.path.join(dest, p)
            os.makedirs(os.path.dirname(cible), exist_ok=True)
            open(cible, "wb").write(data)
    return out


def arbre(racine):
    out = {}
    for r, _, fs in os.walk(racine):
        for f in fs:
            p = os.path.join(r, f)
            if os.path.isfile(p) and not os.path.islink(p):
                out[os.path.relpath(p, racine)] = sha(open(p, "rb").read())
    return out


def comparer(a, b, plafond=400):
    """a = openKylin, b = référence ; {chemin: sha256}."""
    com = set(a) & set(b)
    mod = sorted(p for p in com if a[p] != b[p])
    aj, ab = sorted(set(a) - set(b)), sorted(set(b) - set(a))
    return {"identiques": len(com) - len(mod), "modifies": len(mod), "ajoutes_openkylin": len(aj),
            "absents_openkylin": len(ab), "liste_modifies": mod[:plafond],
            "liste_ajoutes_openkylin": aj[:plafond], "liste_absents_openkylin": ab[:plafond]}


def series(d):
    p = os.path.join(d, "patches", "series")
    if not os.path.exists(p):
        return []
    return [l.split()[0] for l in open(p, encoding="utf-8", errors="replace") if l.strip() and not l.startswith("#")]


def main():
    os.makedirs(WORK, exist_ok=True)
    os.makedirs(OUT, exist_ok=True)
    kr_ok = os.path.join(CORPUS, "archive", "openkylin-keyring", "keyrings", "openkylin-archive-keyring.gpg")
    kr_ub = "/usr/share/keyrings/ubuntu-archive-keyring.gpg"

    # 1. openKylin : index signé → .dsc → orig
    sums = inrelease("openkylin", "huanghe", kr_ok)
    ok = {st["Package"]: st for st in parse_deb822(index("openkylin", "huanghe", sums, "main/source/Sources"))
          if st.get("Package") in PAQUETS}
    ok_dirs = {pk: telecharger_source("openkylin", "huanghe", ok[pk]) for pk in PAQUETS}

    # 2. Ubuntu, puis trousseau Debian via la chaîne Ubuntu
    refs = {pk: [] for pk in PAQUETS}
    for suite in SUITES["ubuntu"]:
        try:
            s = inrelease("ubuntu", suite, kr_ub)
        except urllib.error.HTTPError as e:  # suite en fin de vie, retirée des miroirs
            log("  suite indisponible :", suite, e.code)
            PROV.append(["ubuntu", suite, "InRelease", "", "", "", f"indisponible (HTTP {e.code}) : non utilisée"])
            continue
        for comp in COMPONENTS["ubuntu"]:
            for st in parse_deb822(index("ubuntu", suite, s, f"{comp}/source/Sources")):
                if st.get("Package") in PAQUETS:
                    refs[st["Package"]].append(("ubuntu", suite, st))
            if suite == "resolute" and comp == "universe":
                for st in parse_deb822(index("ubuntu", suite, s, "universe/binary-amd64/Packages")):
                    if st.get("Package") == "debian-archive-keyring":
                        url = f"{MIR['ubuntu']}/{st['Filename']}"
                        deb = os.path.join(WORK, "ubuntu", os.path.basename(st["Filename"]))
                        if sha(get(url, deb)) != st["SHA256"]:
                            raise SystemExit("EMPREINTE debian-archive-keyring")
                        PROV.append(["ubuntu", suite, "debian-archive-keyring", st["Version"], url, st["SHA256"],
                                     "SHA-256 conforme à l'index Packages signé"])
                        kdir = os.path.join(WORK, "debian-archive-keyring")
                        subprocess.run(["dpkg-deb", "-x", deb, kdir], check=True)
    kr_deb = next(os.path.join(r, f) for r, _, fs in os.walk(os.path.join(WORK, "debian-archive-keyring"))
                  for f in fs if f in ("debian-archive-keyring.gpg", "debian-archive-keyring.pgp"))
    for suite in SUITES["debian"]:
        s = inrelease("debian", suite, kr_deb)
        for st in parse_deb822(index("debian", suite, s, "main/source/Sources")):
            if st.get("Package") in PAQUETS:
                refs[st["Package"]].append(("debian", suite, st))

    # 3. Téléchargement des références (une version = une fois) et comparaisons
    resultat = {}
    for pk in PAQUETS:
        okd, okf = ok_dirs[pk]
        ok_orig = [f for f in okf if ".orig." in f and not f.endswith(".asc")]
        ok_orig_tree = extraire(os.path.join(okd, ok_orig[0]), os.path.join(okd, "orig")) if ok_orig else {}
        ok_debian = arbre(os.path.join(CORPUS, "archive", pk, "debian"))
        ok_series = series(os.path.join(CORPUS, "archive", pk, "debian"))
        vus, comps = set(), []
        for arch, suite, st in refs[pk]:
            cle = (arch, st["Version"])
            if cle in vus:
                continue
            vus.add(cle)
            d, fs = telecharger_source(arch, suite, st)
            orig = [f for f in fs if ".orig." in f and not f.endswith(".asc")]
            deb = [f for f in fs if ".debian.tar" in f]
            c = {"archive": arch, "suite": suite, "version": st["Version"]}
            if orig and ok_orig:
                h_ref = fs[orig[0]][0]
                c["orig"] = {"fichier": orig[0], "sha256": h_ref, "identique_octet": h_ref == okf[ok_orig[0]][0]}
                if not c["orig"]["identique_octet"]:
                    c["orig"]["contenu"] = comparer(ok_orig_tree, extraire_cache(os.path.join(d, orig[0]),
                                                                                 os.path.join(d, "orig")))
            natif = [f for f in fs if re.search(r"\.tar\.(xz|gz|bz2)$", f) and ".orig." not in f
                     and ".debian.tar" not in f]
            if deb or natif:
                ref_deb_dir = os.path.join(d, "deb")
                if deb:
                    ref_debian = extraire(os.path.join(d, deb[0]), ref_deb_dir)
                else:  # source native : on ne garde que son debian/
                    tout = extraire(os.path.join(d, natif[0]), os.path.join(d, "natif"))
                    ref_debian = {p[len("debian/"):]: h for p, h in tout.items() if p.startswith("debian/")}
                    ref_deb_dir = os.path.join(d, "natif", "debian")
                    c["reference_native"] = natif[0]
                    if ok_orig_tree:  # orig openKylin comparé à l'arbre natif, hors debian/
                        hors = lambda t: {p: h for p, h in t.items() if not p.startswith("debian/")}
                        c["orig_vs_natif_hors_debian"] = comparer(hors(ok_orig_tree), hors(tout))
                c["debian"] = comparer(ok_debian, ref_debian)
                rs = series(ref_deb_dir)
                okp = {p: ok_debian.get("patches/" + p) for p in ok_series}
                rfp = {p: ref_debian.get("patches/" + p) for p in rs}
                c["serie"] = {
                    "openkylin": len(ok_series), "reference": len(rs),
                    "identiques_octet": sorted(p for p in okp if p in rfp and okp[p] == rfp[p]),
                    "meme_nom_contenu_different": sorted(p for p in okp if p in rfp and okp[p] != rfp[p]),
                    "propres_openkylin": [p for p in ok_series if p not in rfp],
                    "retires_par_openkylin": [p for p in rs if p not in okp],
                }
            comps.append(c)
        # référence la plus proche : maximum de fichiers debian/ identiques
        proche = max((c for c in comps if "debian" in c), key=lambda c: c["debian"]["identiques"], default=None)
        r = {"paquet": pk, "version_openkylin": ok[pk]["Version"],
             "orig_openkylin": {"fichier": ok_orig[0], "sha256": okf[ok_orig[0]][0]} if ok_orig else None,
             "reference_la_plus_proche": f"{proche['archive']} {proche['suite']} {proche['version']}" if proche else None,
             "comparaisons": comps}
        # 4. archives GNU signées (glibc, grub2) : clé prise dans le paquet de référence vérifié
        if pk in GNU and proche:
            proj, tb = GNU[pk]
            gd = os.path.join(WORK, "gnu")
            for f in (tb, tb + ".sig"):
                get(f"{MIR['gnu']}/{proj}/{f}", os.path.join(gd, f))
            asc = os.path.join(WORK, proche["archive"], f"{pk}_{proche['version'].split(':')[-1]}", "deb",
                               "upstream", "signing-key.asc")
            okg, cle = gpgv(keyring_from_asc(asc), os.path.join(gd, tb + ".sig"), os.path.join(gd, tb)) \
                if os.path.exists(asc) else (False, "clé absente")
            h = sha(open(os.path.join(gd, tb), "rb").read())
            PROV.append(["gnu", "", proj, tb, f"{MIR['gnu']}/{proj}/{tb}", h,
                         f"signature amont {'OK' if okg else 'NON VÉRIFIÉE'} clé {cle} (trousseau : {proche['archive']} {proche['version']})"])
            r["amont_gnu"] = {"archive": tb, "sha256": h, "signature_valide": okg, "cle": cle,
                              "identique_octet_orig_openkylin": h == (r["orig_openkylin"] or {}).get("sha256"),
                              "contenu": comparer(ok_orig_tree, extraire(os.path.join(gd, tb),
                                                                         os.path.join(gd, pk + "-src")))}
            autres = []
            for url in GNU_PRE.get(pk, []):
                f = os.path.join(gd, os.path.basename(url))
                get(url, f)
                get(url + ".sig", f + ".sig")
                okp, clep = gpgv(keyring_from_asc(asc), f + ".sig", f) if os.path.exists(asc) else (False, "clé absente")
                hp = sha(open(f, "rb").read())
                PROV.append(["gnu", "pré-version", proj, os.path.basename(url), url, hp,
                             f"signature amont {'OK' if okp else 'NON VÉRIFIÉE'} clé {clep}"])
                autres.append({"archive": os.path.basename(url), "sha256": hp, "signature_valide": okp, "cle": clep,
                               "identique_octet_orig_openkylin": hp == (r["orig_openkylin"] or {}).get("sha256")})
            if autres:
                r["amont_gnu_pre_versions"] = autres
        # 5. signature amont jointe au .dsc openKylin (apport)
        asc_ok = [f for f in okf if f.endswith(".orig.tar.xz.asc") or f.endswith(".orig.tar.gz.asc")]
        if asc_ok and proche:
            asc = os.path.join(WORK, proche["archive"], f"{pk}_{proche['version'].split(':')[-1]}", "deb",
                               "upstream", "signing-key.asc")
            if os.path.exists(asc):
                okg, cle = gpgv(keyring_from_asc(asc), os.path.join(okd, asc_ok[0]), os.path.join(okd, ok_orig[0]))
                r["signature_amont_orig_openkylin"] = {"valide": okg, "cle": cle,
                                                       "trousseau": f"{proche['archive']} {proche['version']}"}
        resultat[pk] = r
        json.dump(r, open(os.path.join(OUT, f"{pk}.json"), "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        log(pk, "→ référence la plus proche :", r["reference_la_plus_proche"])

    with open(os.path.join(OUT, "provenance.tsv"), "w", encoding="utf-8") as fh:
        fh.write("archive\tsuite\tpaquet\tversion\turl\tsha256\tverification\n")
        for l in PROV:
            fh.write("\t".join(str(x) for x in l) + "\n")
    print(json.dumps({pk: {"proche": r["reference_la_plus_proche"],
                           "orig_identique": [c.get("orig", {}).get("identique_octet") for c in r["comparaisons"]],
                           "propres_openkylin": [len(c.get("serie", {}).get("propres_openkylin", []))
                                                 for c in r["comparaisons"]]}
                      for pk, r in resultat.items()}, ensure_ascii=False, indent=1))


if __name__ == "__main__":
    main()
