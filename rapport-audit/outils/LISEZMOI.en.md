# Phase 0 reproduction tools

🇫🇷 [Version française (par défaut)](LISEZMOI.md) · 🇬🇧 English (this page)

Variables: `AUDIT_WORK` (working directory: index, intermediate results) and `AUDIT_SRC` (extracted source
corpus, ≈ 8.4 GB). Prerequisites: `python3`, `curl`, `git`, `gpgv`, `dpkg-dev` (`dpkg-source`, `dpkg-deb`).

| Step | Script | Role |
|---|---|---|
| 1 | (manual) | Download the default repositories' `InRelease` files, verify with `gpgv` and the key `09FF C10E A273 DD29 A986 B110 8B31 3CEA FF59 2D96` (PPA: `3EA5 5C18 3BD8 90B2 D91C 3A20 2636 AEDC 49E9 E2C8`), then the `Sources.gz` / `Packages.gz` indexes (fingerprints checked against the `InRelease`). See `../donnees/apt/`. |
| 2 | `isoread.py <iso-url> <folder>` | ISO 9660 reading via partial HTTP requests: manifest, `.disk/`, `.kylin-post-actions`. |
| 3 | `index2.py` | Unified index of sources ↔ binaries ↔ repositories ↔ ISO. |
| 4 | `lsr.sh <name>` | Presence and branch heads on Gitee and GitHub (`git ls-remote`). |
| 5 | `master.py` | Master table (comparison of `orig.tar` against Debian/Ubuntu, Kylin origin, forges). |
| 6 | `candidates.py` | List of the 368 P1/P2/P3 candidates (explicit rules, versions v1 → v3). |
| 7 | `acquire.py` | Authenticated download and extraction (`dpkg-source -x`) of the candidate sources. |
| 8 | `clone.sh <name>` | Shallow Gitee clone (`openkylin/huanghe` branch if present). |
| 9 | `structure.py`, `codevol.py` | Structural extraction of entry points and own-code volume. |
| 10 | `surface.py` | Installed entry surface (`Contents-amd64` index + `.deb` file listings). |
| 11 | `digest.py` → agent workflow → `apply_critique.py` → `gen_md.py`, `gen_p2.py` | Per-batch sheets, read-only agent census, critique corrections, Markdown generation. |
| 12 | `consolidation.py` | **Phases 1–2.** Regenerates `02-findings.csv` (and its lighter view `02-findings-apercu.csv`), `03-diffs-suspects.md` and `05-donnees.md` identically from the `../donnees/phase1/V*-L*.json`, `../donnees/phase2/P2-*.json` sheets and the `../donnees/verification/` verdicts (read-only). Variables: `AUDIT_PHASE1`, `AUDIT_PHASE2`, `AUDIT_VERIF`, `AUDIT_RAPPORT`. Cross-check analysis text: `gabarits/05-croisement.md`. |
| 13 | `verification_rapport.py` | **Adversarial verification.** Generates `07-verification.md` from the verdicts; the brief and the group generator are in `../donnees/verification/`; the G9 `readelf`/`objdump` measurements are in `../donnees/verification/mesures-G9/`. |
| 14 | `references_p2.py` | **Phase 2, continued.** Fetches through an authenticated chain (InRelease `gpgv` → index → `.dsc` → files, SHA-256 at each step; European mirrors) the matching Ubuntu, Debian and GNU sources of the remaining forks, then compares their `orig` and `debian/` file by file with openKylin's. Outputs: `../donnees/phase2/references/<package>.json` and `provenance.tsv`. Downloads go to `$AUDIT_WORK/ref-p2`. Agent instructions: `../donnees/phase2/CONSIGNE-PHASE2-SUITE.md`. |
| 15 | `durcissement_binaires.py`, `binaires_openkylin.py` | **Binary content.** Binary packages obtained through the signed chain and extracted without execution: measurement of compiler protections (`readelf`) and inventory of source-less components (files, permissions, maintainer scripts, presence on the ISO). Outputs: `../donnees/phase2/durcissement-binaires.json`, `../donnees/phase1/vague3/`. |

The fingerprint of every analysed tree is in `../donnees/empreintes-arbres-sources.tsv`: any re-analysis
must first check that it covers trees with an identical fingerprint.
