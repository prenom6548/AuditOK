#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Génère 07-verification.md à partir des verdicts de la vérification contradictoire.

Entrées : fiches de lot (via consolidation.load) et ../donnees/verification/G*-verdicts.json.
Sortie  : $AUDIT_RAPPORT/07-verification.md (défaut ..). Lecture seule ; n'imprime que des compteurs.
"""
import collections, os
import consolidation as C

ORDRE = ["RÉFUTÉ", "NUANCÉ", "INDÉTERMINÉ", "CONFIRMÉ"]


def main():
    docs = C.load()
    ver = C.load_verdicts()
    soumis = {f"{d.get('lot')}#{i}": fi for d in docs for i, fi in enumerate(d.get("findings", []))
              if fi.get("severite") in ("critique", "elevee")}
    non_verifies = sorted(k for k in soumis if k not in ver)
    tot = collections.Counter(v.get("verdict") for v in ver.values())
    groupes = collections.OrderedDict()
    for v in sorted(ver.values(), key=lambda v: (v["groupe"], v.get("id", ""))):
        groupes.setdefault(v["groupe"], []).append(v)

    L = ["# 07 — Vérification contradictoire des constats critiques et élevés", "",
         "> **Méthode.** Chaque constat de sévérité critique ou élevée a été relu par un **auditeur sceptique",
         "> indépendant** dont la mission était de le **réfuter** : reproduire la preuve (fichier:ligne), chercher",
         "> un contrôle compensatoire, une condition d'atteignabilité non remplie, une erreur de lecture ou une",
         "> sévérité surévaluée — et, symétriquement, une sous-évaluation. Lecture seule stricte, sans exécution.",
         "> Consigne : `donnees/verification/CONSIGNE-VERIF.md` ; verdicts bruts : `donnees/verification/`.", "",
         "| Verdict | Sens |", "|---|---|",
         "| **CONFIRMÉ** | le fait et sa portée résistent à la lecture adverse |",
         "| **NUANCÉ** | le fait tient, mais la sévérité ou les conditions changent |",
         "| **RÉFUTÉ** | constat faux, non atteignable ou entièrement compensé |",
         "| **INDÉTERMINÉ** | non tranchable en lecture seule — renvoyé à la Phase 5 |", "",
         "## 1. Bilan", "",
         f"- Constats soumis : **{len(soumis)}** ; verdicts rendus : **{len(ver)}**.",
         "- " + " · ".join(f"**{k}** : {tot.get(k, 0)}" for k in ORDRE),
         "- La sévérité retenue dans `02-findings.csv` est la **sévérité vérifiée** (colonne `severite_verifiee`).", ""]
    if non_verifies:
        L += [f"- Constats non encore vérifiés ({len(non_verifies)}) : " + ", ".join(non_verifies), ""]
    L += ["| Groupe | Constats | " + " | ".join(ORDRE) + " |", "|---|---|" + "---|" * len(ORDRE)]
    for g, vs in groupes.items():
        c = collections.Counter(v.get("verdict") for v in vs)
        L.append(f"| {g} | {len(vs)} | " + " | ".join(str(c.get(k, 0)) for k in ORDRE) + " |")
    L.append("")

    L += ["## 2. Constats nuancés, réfutés ou indéterminés", "",
          "Ce sont les corrections apportées par la vérification : elles priment sur la fiche d'origine.", "",
          "| Verdict | Constat | Sévérité initiale → vérifiée | Justification | Preuve |", "|---|---|---|---|---|"]
    for v in sorted(ver.values(), key=lambda v: (ORDRE.index(v.get("verdict")) if v.get("verdict") in ORDRE else 9, v.get("id", ""))):
        if v.get("verdict") == "CONFIRMÉ":
            continue
        L.append(f"| {v.get('verdict')} | `{v.get('id')}` {C.esc(v.get('depot'))} — {C.esc(v.get('fichier'))} | "
                 f"{v.get('severite_initiale')} → {v.get('severite_revisee') or v.get('severite_initiale')}{(' (' + v['condition'] + ')') if v.get('condition') else ''} | "
                 f"{C.esc(C.clip(v.get('justification'), 500))} | {C.esc(C.clip(v.get('preuve'), 200))} |")
    L += ["", "## 3. Constats confirmés", "",
          "| Constat | Sévérité | Justification de la confirmation |", "|---|---|---|"]
    for v in sorted(ver.values(), key=lambda v: (C.SEV.get(v.get("severite_initiale"), 9), v.get("id", ""))):
        if v.get("verdict") != "CONFIRMÉ":
            continue
        L.append(f"| `{v.get('id')}` {C.esc(v.get('depot'))} — {C.esc(v.get('fichier'))} | {v.get('severite_initiale')} | "
                 f"{C.esc(C.clip(v.get('justification'), 400))} |")
    L += ["", "## 4. Limites de la vérification", "",
          "- Vérification **statique** : elle confirme la lecture du code, pas l'exploitabilité sur un système en",
          "  fonctionnement. Les verdicts INDÉTERMINÉ et les conditions d'atteignabilité relèvent de la Phase 5",
          "  (`04-protocole-dynamique.md`).",
          "- Un vérificateur par groupe : la vérification est **adverse** mais **non redondante** (pas de vote).",
          "- Seuls les constats critiques et élevés ont été soumis ; les autres gardent leur sévérité d'origine.", ""]
    open(os.path.join(C.RAP, "07-verification.md"), "w", encoding="utf-8").write("\n".join(L) + "\n")
    print("07-verification.md :", len(ver), "verdicts sur", len(soumis), "soumis", dict(tot),
          "| non vérifiés :", len(non_verifies))


if __name__ == "__main__":
    main()
