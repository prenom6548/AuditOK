#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Consolidation des Phases 1 et 2 — reproductible, lecture seule.

Régénère à l'identique, à partir des fiches de lot JSON versées dans ../donnees/ :
  - 02-findings.csv       tous les constats (Phase 1 vagues 1-3, Phase 2), triés par sévérité
  - 02-findings-apercu.csv  même contenu sans les colonnes longues (affichable sur GitHub)
  - 03-diffs-suspects.md  différentiel des forks amont (synthèse par fork, titres inexacts,
                          hunks ostree/ca-certificates, correctifs SUSPECT / À EXAMINER,
                          durcissement de la configuration noyau)
  - 05-donnees.md         flux réseau + collectes + croisement (gabarits/05-croisement.md)

Sources : $AUDIT_PHASE1 (défaut ../donnees/phase1 : V1-L*.json, V2-L*.json)
          $AUDIT_PHASE2 (défaut ../donnees/phase2 : P2-*.json)
Sortie  : $AUDIT_RAPPORT (défaut ..). N'imprime que des compteurs.
"""
import json, glob, csv, io, os, re, collections

HERE = os.path.dirname(os.path.abspath(__file__))
P1 = os.environ.get("AUDIT_PHASE1", os.path.join(HERE, "..", "donnees", "phase1"))
P2 = os.environ.get("AUDIT_PHASE2", os.path.join(HERE, "..", "donnees", "phase2"))
RAP = os.environ.get("AUDIT_RAPPORT", os.path.join(HERE, ".."))
GAB = os.path.join(HERE, "gabarits")
VER = os.environ.get("AUDIT_VERIF", os.path.join(HERE, "..", "donnees", "verification"))
SEV = {"critique": 0, "elevee": 1, "moyenne": 2, "faible": 3, "informationnelle": 4}
CLS = ["[SUSPECT]", "[À EXAMINER]", "[LÉGITIME]"]


def phase_of(lot):
    for p, tag in (("V1-", "P1-V1"), ("V2-", "P1-V2"), ("V3-", "P1-V3"), ("P2-", "P2")):
        if str(lot).startswith(p):
            return tag
    return "?"


def lot_key(lot):
    order = {"P1-V1": 0, "P1-V2": 1, "P1-V3": 2, "P2": 3}.get(phase_of(lot), 9)
    m = re.search(r"-L(\d+)-", str(lot))
    return (order, int(m.group(1)) if m else 0, str(lot))


def esc(s):
    return str(s or "").replace("|", "\\|").replace("\n", " ").strip()


def clip(s, n=600):
    s = str(s or "").replace("\r", "").replace("⏎", "\n")
    return s if len(s) <= n else s[:n] + " […]"


def load():
    files = glob.glob(os.path.join(P1, "V*-L*.json")) + glob.glob(os.path.join(P2, "P2-*.json"))
    docs = [json.load(open(f, encoding="utf-8")) for f in files]
    # Normalisation des libellés de classement (certaines fiches les écrivent sans accents).
    norm = {"[LEGITIME]": "[LÉGITIME]", "[A EXAMINER]": "[À EXAMINER]"}
    for d in docs:
        for coll in (d.get("patches", []), d.get("hunks", [])):
            for p in coll:
                p["classement"] = norm.get(p.get("classement"), p.get("classement"))
    # Normalisation des registres (quelques fiches écrivent « CONSTATE » sans accent).
    for d in docs:
        for f in d.get("findings", []):
            if str(f.get("registre", "")).strip().upper() == "CONSTATE":
                f["registre"] = "CONSTATÉ"
    docs.sort(key=lambda d: lot_key(d.get("lot", "")))
    return docs


# ------------------------------------------------------------------ 02
def load_verdicts():
    """Verdicts de la vérification contradictoire, indexés par « lot#indice »."""
    out = {}
    for f in glob.glob(os.path.join(VER, "G*-verdicts.json")):
        g = json.load(open(f, encoding="utf-8"))
        for v in g.get("verdicts", []):
            v = dict(v, groupe=g.get("groupe", os.path.basename(f)))
            # Normalisation : sévérité canonique + condition éventuelle écrite en texte libre.
            brut = str(v.get("severite_revisee") or v.get("severite_initiale") or "").strip()
            mot = (brut.lower().split() or [""])[0].strip("(),;")
            base = next((s for s in SEV if brut.lower().startswith(s) or (len(mot) >= 4 and s.startswith(mot))),
                        v.get("severite_initiale", ""))  # tolère les abréviations (« info »)
            v["severite_revisee"], v["condition"] = base, brut[len(base):].strip(" ()") if brut.lower().startswith(base) else ""
            out[v.get("id")] = v
    return out


def gen_csv(docs):
    keys = ["depot", "fichier", "ligne", "type", "severite", "registre",
            "extrait", "explication", "scenario", "contre_lecture"]
    ver = load_verdicts()
    rows = []
    for d in docs:
        lot = d.get("lot", "")
        for i, fi in enumerate(d.get("findings", [])):
            r = {k: fi.get(k, "") for k in keys}
            r["lot"], r["phase"] = lot, phase_of(lot)
            v = ver.get(f"{lot}#{i}")
            r["verification"] = ((v.get("verdict", "") + (f" — {v['condition']}" if v.get("condition") else "")) if v
                                 else ("non soumis" if r["severite"] in ("critique", "elevee") else "hors périmètre (< élevé)"))
            r["severite_verifiee"] = (v.get("severite_revisee") or r["severite"]) if v else r["severite"]
            r["justification_verification"] = v.get("justification", "") if v else ""
            rows.append(r)
    rows.sort(key=lambda r: (SEV.get(r["severite_verifiee"], 9), lot_key(r["lot"]), r["depot"], r["fichier"]))
    keys += ["verification", "severite_verifiee", "justification_verification"]
    cols = ["phase", "lot"] + keys
    with open(os.path.join(RAP, "02-findings.csv"), "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=cols, quoting=csv.QUOTE_ALL)
        w.writeheader()
        for r in rows:
            w.writerow(r)
    # Vue allégée : GitHub n'affiche en tableau que les CSV de 512 Ko au plus.
    vue = ["phase", "lot", "depot", "fichier", "ligne", "type", "severite", "severite_verifiee",
           "verification", "registre", "extrait", "explication"]
    # Explication tronquée (marque « […] ») juste assez pour rester sous 500 Ko ; le CSV complet reste intact.
    for limite in [None] + list(range(800, 99, -50)):
        buf = io.StringIO()
        w = csv.DictWriter(buf, fieldnames=vue, quoting=csv.QUOTE_ALL, extrasaction="ignore", lineterminator="\r\n")
        w.writeheader()
        for r in rows:
            e = r["explication"]
            w.writerow(dict(r, explication=e if limite is None or len(e) <= limite else e[:limite] + " […]"))
        if len(buf.getvalue().encode("utf-8")) < 500 * 1024:
            break
    with open(os.path.join(RAP, "02-findings-apercu.csv"), "w", newline="", encoding="utf-8") as fh:
        fh.write(buf.getvalue())
    print("02-findings.csv :", len(rows), "constats",
          dict(collections.Counter(r["severite"] for r in rows)),
          dict(collections.Counter(r["phase"] for r in rows)))


# ------------------------------------------------------------------ 05
def gen_donnees(docs):
    flux, coll = [], []
    for d in docs:
        lot = d.get("lot", "")
        for x in d.get("flux_reseau", []):
            flux.append(dict(x, _lot=lot))
        for x in d.get("collectes", []):
            coll.append(dict(x, _lot=lot))
    L = ["# 05 — Flux de données et croisement avec la politique de confidentialité", "",
         "> **Portée.** Agrégation des flux réseau et des collectes relevés par l'analyse statique",
         "> (Phase 1 vagues 1-3, Phase 2). Registres **[CONSTATÉ]**/**[PROBABLE]**. L'absence d'un flux",
         "> ne prouve pas son inexistence (couverture partielle, cf. `00-synthese.md`). Rien n'a été",
         "> exécuté : lu dans le code, non observé (confirmation : `04-protocole-dynamique.md`).", "",
         f"## 1. Flux réseau ({len(flux)})", "",
         "| Lot | Composant | Destination | Proto. | TLS vérifié | Déclencheur | Données | Classement | Preuve |",
         "|---|---|---|---|---|---|---|---|---|"]
    for x in flux:
        L.append("| " + " | ".join(esc(v) for v in (
            x["_lot"], x.get("composant"), x.get("destination"), x.get("protocole"),
            x.get("tls_verifie"), x.get("declencheur"), x.get("donnees"),
            x.get("classification"), x.get("fichier_ligne"))) + " |")
    L += ["", f"## 2. Collectes de données ({len(coll)})", "",
          "| Lot | Composant | Donnée | Stockage | Envoi | Preuve |", "|---|---|---|---|---|---|"]
    for x in coll:
        L.append("| " + " | ".join(esc(v) for v in (
            x["_lot"], x.get("composant"), x.get("donnee"), x.get("stockage"),
            x.get("envoi"), x.get("fichier_ligne"))) + " |")
    L.append("")
    gab = os.path.join(GAB, "05-croisement.md")
    if os.path.exists(gab):
        L.append(open(gab, encoding="utf-8").read())
    open(os.path.join(RAP, "05-donnees.md"), "w", encoding="utf-8").write("\n".join(L) + "\n")
    print("05-donnees.md :", len(flux), "flux,", len(coll), "collectes")


# ------------------------------------------------------------------ 03
def _finding_block(f):
    L = [f"### [{str(f.get('severite','')).upper()}] `{f.get('depot')}` — {esc(f.get('fichier'))} (l. {esc(f.get('ligne'))})", "",
         f"- **Type** : {f.get('type')} — **Registre** : {f.get('registre')}",
         f"- **Extrait** : `{esc(clip(f.get('extrait'), 300))}`",
         f"- **Constat** : {clip(f.get('explication'), 900)}"]
    if f.get("scenario"):
        L.append(f"- **Scénario** : {clip(f.get('scenario'), 700)}")
    if f.get("contre_lecture"):
        L.append(f"- **Contre-lecture** : {clip(f.get('contre_lecture'), 700)}")
    return L + [""]


def gen_diffs(docs):
    l10 = next((d for d in docs if "L10-" in d.get("lot", "")), {})
    l0 = next((d for d in docs if d.get("lot", "").startswith("V1-L0-")), {})
    p2 = [d for d in docs if phase_of(d.get("lot", "")) == "P2"]
    hunks = l10.get("hunks", [])
    L = ["# 03 — Différentiel des forks amont : correctifs et hunks suspects", "",
         "> **Portée.** Delta openKylin des forks de logiciels critiques (Phase 2), plus la chaîne de",
         "> confiance OSTree / ca-certificates différenciée en vague 1. Pour un paquet non natif, l'amont",
         "> `.orig` est identique à Debian/Ubuntu : le delta est dans `debian/` (série quilt, règles, scripts",
         "> de mainteneur). « Suspect » = écarte la posture de sécurité de l'amont ; **ne signifie pas",
         "> malveillant**. Registres [CONSTATÉ]/[PROBABLE]. Fiches complètes : `donnees/phase2/`.", "",
         "## 1. Synthèse par fork", "",
         "| Fork | Lot | Unités | SUSPECT | À EXAMINER | LÉGITIME | Titres inexacts | Constats ≥ moyen |",
         "|---|---|---|---|---|---|---|---|"]
    for dep in ("ostree", "ca-certificates"):
        hs = [h for h in hunks if h.get("depot") == dep]
        if hs:
            c = collections.Counter(h.get("classement") for h in hs)
            n = sum(1 for f in l10.get("findings", []) if f.get("depot") == dep and SEV.get(f.get("severite"), 9) <= 2)
            L.append(f"| {dep} | {l10.get('lot')} | {len(hs)} hunks | {c['[SUSPECT]']} | {c['[À EXAMINER]']} | {c['[LÉGITIME]']} | — | {n} |")
    titres = []
    for d in p2:
        ps = d.get("patches", [])
        c = collections.Counter(p.get("classement") for p in ps)
        bad = [p for p in ps if p.get("titre_fidele") is False]
        titres += [(d.get("lot"), p) for p in bad]
        n = sum(1 for f in d.get("findings", []) if SEV.get(f.get("severite"), 9) <= 2)
        L.append(f"| {esc(', '.join(d.get('composants', [])))} | {d.get('lot')} | {len(ps)} correctifs | "
                 f"{c['[SUSPECT]']} | {c['[À EXAMINER]']} | {c['[LÉGITIME]']} | {len(bad)} | {n} |")
    L += ["", f"## 2. Titres de correctifs inexacts ({len(titres)})", "",
          "Un correctif dont le **titre ne décrit pas le contenu** empêche le tri par titre : chaque",
          "correctif doit être lu. Ce motif est transversal à openKylin (voir aussi `policykit-1`).", "",
          "| Lot | Correctif | Titre déclaré | Contenu réel | Classement |", "|---|---|---|---|---|"]
    for lot, p in titres:
        L.append(f"| {lot} | {esc(p.get('nom'))} | {esc(clip(p.get('titre_declare'), 120))} | "
                 f"{esc(clip(p.get('contenu_reel') or p.get('resume'), 260))} | {p.get('classement')} |")
    trust = [f for f in l10.get("findings", []) if SEV.get(f.get("severite"), 9) <= 2]
    trust += [f for f in l0.get("findings", []) if f.get("depot") == "policykit-1"]
    for d in p2:
        trust += [f for f in d.get("findings", []) if SEV.get(f.get("severite"), 9) <= 2]
    trust.sort(key=lambda f: (SEV.get(f.get("severite"), 9), str(f.get("depot"))))
    L += ["", f"## 3. Constats ≥ moyen issus du différentiel ({len(trust)})", ""]
    for f in trust:
        L += _finding_block(f)
    return L, hunks, p2


def gen_diffs_details(L, hunks, p2):
    susp = [h for h in hunks if h.get("classement") == "[SUSPECT]"]
    L += [f"## 4. ostree / ca-certificates — hunks SUSPECT ({len(susp)})", ""]
    byfile = collections.OrderedDict()
    for h in susp:
        byfile.setdefault((h.get("depot"), h.get("fichier")), []).append(h)
    for (dep, fic), hs in byfile.items():
        L += [f"### `{dep}` — `{esc(fic)}`", ""]
        for h in hs:
            L += [f"**Lignes {esc(h.get('lignes'))}** — {clip(h.get('resume'), 500)}", ""]
            for lab, key in (("_Amont :_", "extrait_avant"), ("_openKylin :_", "extrait_apres")):
                v = clip(h.get(key), 500)
                if v.strip():
                    L += [lab, "```", v, "```"]
            L.append("")
    exam = [h for h in hunks if h.get("classement") == "[À EXAMINER]"]
    L += [f"## 5. ostree / ca-certificates — hunks À EXAMINER ({len(exam)})", "",
          "| Dépôt | Fichier | Lignes | Résumé |", "|---|---|---|---|"]
    for h in exam:
        L.append(f"| {esc(h.get('depot'))} | {esc(h.get('fichier'))} | {esc(h.get('lignes'))} | {esc(clip(h.get('resume'), 240))} |")
    L += ["", "## 6. Forks P2 — correctifs SUSPECT et À EXAMINER", ""]
    for d in p2:
        ps = [p for p in d.get("patches", []) if p.get("classement") in ("[SUSPECT]", "[À EXAMINER]")]
        if not ps:
            continue
        ps.sort(key=lambda p: CLS.index(p.get("classement")) if p.get("classement") in CLS else 9)
        L += [f"### {esc(', '.join(d.get('composants', [])))} ({d.get('lot')})", "",
              "| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |", "|---|---|---|---|---|"]
        for p in ps:
            fa = p.get("fichiers_amont") or []
            fa = ", ".join(fa) if isinstance(fa, list) else str(fa)
            L.append(f"| {p.get('classement')} | {esc(p.get('nom'))} | {'oui' if p.get('titre_fidele') else 'NON'} | "
                     f"{esc(clip(fa, 160))} | {esc(clip(p.get('contenu_reel') or p.get('resume'), 400))} |")
        L.append("")
    kconf = [(d.get("lot"), x) for d in p2 for x in d.get("durcissement_config", [])]
    if kconf:
        L += [f"## 7. Durcissement de la configuration noyau ({len(kconf)} options relevées)", "",
              "| Option | Valeur openKylin | Attendu (Debian/Ubuntu) | Écart | Preuve |", "|---|---|---|---|---|"]
        for lot, x in kconf:
            L.append(f"| {esc(x.get('option'))} | {esc(x.get('valeur'))} | {esc(x.get('attendu_debian'))} | "
                     f"{'**OUI**' if x.get('ecart') else 'non'} | {esc(x.get('fichier_ligne'))} |")
        L.append("")
    open(os.path.join(RAP, "03-diffs-suspects.md"), "w", encoding="utf-8").write("\n".join(L) + "\n")
    tot = collections.Counter(p.get("classement") for d in p2 for p in d.get("patches", []))
    print("03-diffs-suspects.md : forks P2 =", len(p2), "| correctifs", dict(tot),
          "| hunks ostree/ca SUSPECT", len(susp), "| options noyau", len(kconf))


if __name__ == "__main__":
    docs = load()
    print("lots chargés :", len(docs), dict(collections.Counter(phase_of(d.get("lot", "")) for d in docs)))
    gen_csv(docs)
    gen_donnees(docs)
    L, hunks, p2 = gen_diffs(docs)
    gen_diffs_details(L, hunks, p2)
