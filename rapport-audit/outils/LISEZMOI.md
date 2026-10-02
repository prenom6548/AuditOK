# Outils de reproduction de la Phase 0

🇫🇷 Français (cette page) · 🇬🇧 [English version](LISEZMOI.en.md)

Variables : `AUDIT_WORK` (répertoire de travail : index, résultats intermédiaires) et `AUDIT_SRC` (corpus de sources
extrait, ≈ 8,4 Go). Prérequis : `python3`, `curl`, `git`, `gpgv`, `dpkg-dev` (`dpkg-source`, `dpkg-deb`).

| Étape | Script | Rôle |
|---|---|---|
| 1 | (manuel) | Télécharger les `InRelease` des dépôts par défaut, vérifier avec `gpgv` et la clé `09FF C10E A273 DD29 A986 B110 8B31 3CEA FF59 2D96` (PPA : `3EA5 5C18 3BD8 90B2 D91C 3A20 2636 AEDC 49E9 E2C8`), puis les index `Sources.gz` / `Packages.gz` (empreintes contrôlées contre l'`InRelease`). Voir `../donnees/apt/`. |
| 2 | `isoread.py <url-iso> <dossier>` | Lecture ISO 9660 par requêtes HTTP partielles : manifeste, `.disk/`, `.kylin-post-actions`. |
| 3 | `index2.py` | Index unifié sources ↔ binaires ↔ dépôts ↔ ISO. |
| 4 | `lsr.sh <nom>` | Présence et têtes de branches sur Gitee et GitHub (`git ls-remote`). |
| 5 | `master.py` | Table maîtresse (comparaison des `orig.tar` avec Debian/Ubuntu, origine Kylin, forges). |
| 6 | `candidates.py` | Liste des 368 candidats P1/P2/P3 (règles explicites, versions v1 → v3). |
| 7 | `acquire.py` | Téléchargement et extraction authentifiés (`dpkg-source -x`) des sources candidates. |
| 8 | `clone.sh <nom>` | Clone Gitee superficiel (branche `openkylin/huanghe` si présente). |
| 9 | `structure.py`, `codevol.py` | Extraction structurelle des points d'entrée et volume de code propre. |
| 10 | `surface.py` | Surface d'entrée installée (index `Contents-amd64` + listes de fichiers des `.deb`). |
| 11 | `digest.py` → workflow d'agents → `apply_critique.py` → `gen_md.py`, `gen_p2.py` | Fiches par lot, recensement par agents (lecture seule), corrections de la critique, génération Markdown. |
| 12 | `consolidation.py` | **Phases 1-2.** Régénère à l'identique `02-findings.csv` (et sa vue allégée `02-findings-apercu.csv`), `03-diffs-suspects.md` et `05-donnees.md` à partir des fiches `../donnees/phase1/V*-L*.json`, `../donnees/phase2/P2-*.json` et des verdicts `../donnees/verification/` (lecture seule). Variables : `AUDIT_PHASE1`, `AUDIT_PHASE2`, `AUDIT_VERIF`, `AUDIT_RAPPORT`. Texte d'analyse du croisement : `gabarits/05-croisement.md`. |
| 13 | `verification_rapport.py` | **Vérification contradictoire.** Génère `07-verification.md` à partir des verdicts ; consigne et générateur des groupes dans `../donnees/verification/` ; mesures `readelf`/`objdump` du groupe G9 dans `../donnees/verification/mesures-G9/`. |
| 14 | `references_p2.py` | **Phase 2, suite.** Récupère par chaîne authentifiée (InRelease `gpgv` → index → `.dsc` → fichiers, SHA-256 à chaque étape ; miroirs européens) les sources homologues Ubuntu, Debian et GNU des forks restants, puis compare fichier par fichier leur `orig` et leur `debian/` à ceux d'openKylin. Sorties : `../donnees/phase2/references/<paquet>.json` et `provenance.tsv`. Téléchargements dans `$AUDIT_WORK/ref-p2`. Consigne des agents : `../donnees/phase2/CONSIGNE-PHASE2-SUITE.md`. |
| 15 | `durcissement_binaires.py`, `binaires_openkylin.py` | **Contenu binaire.** Paquets binaires obtenus par chaîne signée et extraits sans exécution : mesure des protections de compilation (`readelf`) et inventaire des composants sans source (fichiers, droits, scripts de mainteneur, présence sur l'ISO). Sorties : `../donnees/phase2/durcissement-binaires.json`, `../donnees/phase1/vague3/`. |

L'empreinte de chaque arbre analysé figure dans `../donnees/empreintes-arbres-sources.tsv` : toute ré-analyse doit
d'abord vérifier qu'elle porte sur des arbres d'empreinte identique.
