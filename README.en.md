# Independent security audit of openKylin 3.0 "huanghe"

🇫🇷 [Version française (par défaut)](README.md) · 🇬🇧 English (this page)

> **Independent, neutral and reproducible audit.** This repository is **not** affiliated with
> openKylin, KylinSoft, or any of their partners. It reflects no institutional position: it records
> **verifiable facts** and their limits.

**Status: audit in progress.** Phases 0, 1 (waves 1–3) and 2 (18 critical forks, source-less binaries, binary
hardening) are complete; the 83 critical and high findings went through an adversarial verification; Phase 5 takes the form of a
read-only [**observation kit**](rapport-audit/phase5-kit/LISEZMOI.en.md), kept up to date with each wave,
that anyone can run in a virtual machine. See
[`rapport-audit/en/00-synthesis.md`](rapport-audit/en/00-synthesis.md).

> **Language note.** French is the primary language of this repository. The key documents are translated
> into English (links on each French page). The large generated reports and the raw data
> (`02-findings.csv`, `03-diffs-suspects.md`, `05-donnees.md`, the entry-point census, the JSON evidence
> sheets) are **still in French**; file paths, line numbers and code excerpts they contain are
> language-independent. See the [terminology](#terminology) below to read them.

---

## Why this repository

openKylin is a GNU/Linux distribution derived from Debian/Ubuntu, with the UKUI desktop, an immutable-system
option (OSTree), an Android compatibility layer (KMRE), an in-house AI SDK and many components developed
by the Kylin ecosystem. Its attack surface is large, partly specific to it, and some of the code has no
public upstream equivalent.

This audit pursues one simple goal: **let anyone form a well-founded opinion**, from evidence rather than
from reputation or rumour. We publish our findings, **but also** our tools, our fingerprints and the exact
scope of what we examined — and above all of what we did **not** examine.

## Analytical stance

Three principles, applied without exception:

1. **Neither presumption of guilt nor naivety.** We look for facts, not for a pre-decided conclusion.
2. **We never conclude "safe" from an absence of evidence.** The absence of a finding is not proof of
   harmlessness: it first reflects our coverage, which we quantify (which files, what percentage of the
   volume, which paths were not explored).
3. **Every finding is traced**: repository + file + line number + excerpt. No exceptions.

Every statement carries one of three **registers**:

| Register (French original) | Meaning |
|---|---|
| **[ESTABLISHED]** (`[CONSTATÉ]`) | Read in the code; proof cited (file: line). |
| **[PROBABLE]** | Strong indication, not confirmed; what is missing is stated. |
| **[NOT COVERED]** (`[NON COUVERT]`) | Out of reach of the analysis performed; supports no conclusion. |

For every finding of medium severity or above we give **both** a realistic exploitation scenario **and** a
plausible benign counter-reading — an honesty test that protects the reader and the vendor alike.

## Terminology

The French data files use these terms:

| French | English |
|---|---|
| critique / élevée / moyenne / faible / informationnelle | critical / high / medium / low / informational |
| CONFIRMÉ / NUANCÉ / RÉFUTÉ / INDÉTERMINÉ (verification verdicts) | CONFIRMED / QUALIFIED (holds, narrower) / REFUTED / UNDETERMINED |
| constat, scénario, contre-lecture | finding, scenario, benign counter-reading |
| fichier, ligne, extrait | file, line, excerpt |
| LÉGITIME / À EXAMINER / SUSPECT (patch triage) | LEGITIMATE / TO REVIEW / SUSPECT ("suspect" ≠ malicious) |
| CONSTATÉ / PROBABLE / NON COUVERT (registers) | ESTABLISHED (proof: file and line) / PROBABLE (strong indication, not confirmed) / NOT COVERED |
| vague, lot, dépôt | wave, batch, repository or package |
| phase `P1-V1`, `P1-V2`, `P1-V3`, `P2` | Phase 1 waves 1 to 3 (Kylin-specific components), Phase 2 (upstream forks) |
| coquille vide | empty shell (source package containing no code) |
| angles morts, durcissement, chaîne signée | blind spots, hardening, signed chain (InRelease checked with `gpgv`, then index, then package, SHA-256 at each step) |

**Columns of `02-findings.csv`** (and of its lighter view `02-findings-apercu.csv`):

| Column | Meaning |
|---|---|
| `phase`, `lot` | phase and batch that produced the finding |
| `depot`, `fichier`, `ligne`, `extrait` | package, file, line and short excerpt (the proof) |
| `type` | `privileges`, `confiance` (trust, traceability), `durcissement` (hardening), `telemetrie` (telemetry), `reseau` (network), `crypto`, `persistance` (persistence, remote control), `obfuscation` |
| `severite`, `registre` | initial severity and register |
| `explication`, `scenario`, `contre_lecture` | explanation, realistic scenario, benign counter-reading |
| `verification` | adversarial verdict, or `hors périmètre (< élevé)` = out of scope (findings below high were not submitted) |
| `severite_verifiee`, `justification_verification` | severity after verification, and its justification |

**Keys of the JSON batch sheets** (`rapport-audit/donnees/phase1/`, `phase2/`): `findings` (findings), `composants`
(components), `empreintes` (fingerprints), `flux_reseau` (network flows), `collectes` (data collection),
`couverture` (coverage), `non_couvert` (not covered), `synthese` (summary), `reference` (verified reference
sources), `hypotheses_phase5` (observations for the Phase 5 kit), `incidents` (methodological incidents).
For patches: `titre_declare` (declared title), `contenu_reel` (actual content), `titre_fidele` (title faithful to
content), `classement` (triage), `fichiers_amont` (upstream files touched).

## How to read this audit

In the recommended order:

1. [`rapport-audit/en/00-synthesis.md`](rapport-audit/en/00-synthesis.md) — verdict, structuring findings,
   and the **explicit list of limitations**.
2. [`rapport-audit/01-inventaire.md`](rapport-audit/01-inventaire.md) *(French)* — method, real APT
   landscape, inventory of source packages, forges, and the **coverage matrix**.
3. [`rapport-audit/en/06-blind-spots.md`](rapport-audit/en/06-blind-spots.md) — what the audit
   **cannot** settle (source-less binaries, empty-source packages, components without a public repository).
4. [`rapport-audit/02-findings.csv`](rapport-audit/02-findings.csv) *(French fields)* — 704 traceable
   findings (file: line, scenario, counter-reading, verification verdict). To browse it on GitHub as a
   searchable table: [`02-findings-apercu.csv`](rapport-audit/02-findings-apercu.csv) (same findings, without
   the long columns, explanation sometimes shortened — GitHub only renders CSV files of 512 KB or less as
   tables).
5. [`rapport-audit/07-verification.md`](rapport-audit/07-verification.md) *(French)* — adversarial
   verification of the critical and high findings.

## Repository layout

```
.
├── LICENSE              # AGPL-3.0 (full text) — applies to OUR work
├── LICENCES.md / .en.md # Licence scope — read before any reuse
├── README.md / .en.md   # Entry page (French / English)
│
├── rapport-audit/       # Our original work (AGPL-3.0-or-later)
│   ├── en/                      # English translations of the key reports
│   ├── 00-synthese.md           # Synthesis and verdict (French)
│   ├── 01-inventaire.md         # Inventory + coverage matrix (French)
│   ├── 01a-points-entree*       # Entry-point census, by batch (French)
│   ├── 02-findings.csv          # 704 findings (French fields)
│   ├── 02-findings-apercu.csv   # Same, lighter: browsable as a table on GitHub
│   ├── 03-diffs-suspects.md     # Upstream fork differential, Phase 2 (French)
│   ├── 04-protocole-dynamique.md# Phase 5 method, reference for the kit (French)
│   ├── 05-donnees.md            # Network flows and data collection (French)
│   ├── 06-angles-morts.md       # Blind spots and limits (French)
│   ├── 07-verification.md       # Adversarial verification (French)
│   ├── phase5-kit/              # Phase 5: read-only observation kit for a VM (FR / EN guide)
│   ├── donnees/                 # Generated analysis data + redistributed openKylin
│   │                            #   artefacts (signed APT indexes, ISO manifests)
│   └── outils/                  # Reproduction scripts (strictly read-only)
│
└── sources-openkylin/   # THIRD-PARTY code and artefacts — original licences (see LICENCES)
    ├── archive/                 # Extracted source packages, SHA-256-verified
    ├── gitee/                   # Shallow clones of openKylin repositories
    ├── dsc/                     # .dsc files of the source packages
    ├── index-apt/               # Signed repository indexes (verifiable)
    ├── binaires-sans-source/    # .deb packages with no usable source
    ├── LISEZMOI.md              # Provenance and authenticity of the corpus
    └── EXCLUSIONS.tsv           # Excluded files (size limits) + their SHA-256
```

## Reproducibility and verification

Reproducibility is at the heart of the approach. Two concrete guarantees:

- **Every analysed tree has a fingerprint.** They are in
  `rapport-audit/donnees/empreintes-arbres-sources.tsv`. Any re-analysis must first check that it covers
  identical bytes:

  ```sh
  cd sources-openkylin
  find archive/<package> -path '*/.git' -prune -o -type f -print0 \
    | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum
  ```

- **The distribution trust chain is verifiable.** The APT indexes included (`InRelease`, `Sources.gz`…)
  are signed; their signatures can be checked with `gpgv` and the keys documented in
  `rapport-audit/donnees/apt/`. Source archives were authenticated by SHA-256 against these signed indexes.

The reproduction tools (read-only, parameterised by the `AUDIT_WORK` and `AUDIT_SRC` variables) are
described in [`rapport-audit/outils/LISEZMOI.en.md`](rapport-audit/outils/LISEZMOI.en.md).

> **Technical note.** Some trees ship their own `filter=lfs` rules. Files are versioned as raw bytes. When
> extracting, neutralise the filters (`GIT_LFS_SKIP_SMUDGE=1`, then
> `printf '* -text -eol -filter\n' > .git/info/attributes && git checkout -f`) to get the same bytes.

## Limitations of the audit (please read)

An honest audit states its limits as clearly as its findings. The main ones, as of now:

- **Partial volume.** Only a fraction of the thousands of source packages is examined in depth; the rest
  is covered by inventory only. Real coverage is quantified, component by component, in the inventory.
- **Non-auditable binaries.** Many components ship without usable source (package skeletons, proprietary
  binaries, AI models, the Android image): they can only be examined through inventory and metadata.
- **Code ↔ shipped-binary equality not demonstrated statically.** We neither compile nor execute the
  corpus. The exact correspondence between an installed binary and its source belongs to the dynamic phase:
  the Phase 5 kit, to be run in a VM.
- **Static analysis, strictly read-only.** No corpus file was modified, compiled or executed; no network
  access was made from the analysis. Confirmation on a real system goes through the Phase 5 kit.

The exhaustive, up-to-date list is maintained in
[`rapport-audit/en/00-synthesis.md`](rapport-audit/en/00-synthesis.md) (§ Limitations) and
[`rapport-audit/en/06-blind-spots.md`](rapport-audit/en/06-blind-spots.md).

## Responsible use

This repository is meant to **help**: security-minded users, administrators, researchers, and the vendor
itself. In that spirit:

- The findings are the result of an **independent static analysis**; unless explicitly marked
  [ESTABLISHED] with proof, several remain to be confirmed dynamically.
- This work provides **no ready-made exploitation tool**: it describes risk chains for defensive and
  verification purposes.
- Anyone who confirms an exploitable weakness is free to inform openKylin through its reporting channels.
  The author of this audit does not carry out that coordination themselves: it is **open to anyone in the
  community who wishes to take it on**.

## Licence

- **Our work** (reports, tools, generated analysis data): **AGPL-3.0-or-later** — full text in
  [`LICENSE`](LICENSE).
- **Third-party code and artefacts** under `sources-openkylin/` keep **their original licences** (GPL, LGPL,
  Apache, MIT, BSD… and sometimes proprietary). **We relicense nothing.**

The precise scope — and why these two natures coexist in one repository (mere aggregation within the
meaning of the AGPLv3) — is detailed in [`LICENCES.en.md`](LICENCES.en.md). **Read it before any reuse**,
especially before redistributing a third-party component.

## Contributing

Contributions that **strengthen rigour** are welcome: correcting a misreading, confirming or refuting a
finding from evidence, extending coverage, independent reproduction. Every contribution must respect the
stance above: traced facts (file: line), explicit registers, and no "safe" conclusion drawn from an absence
of evidence. Translations of the French reports and data are also very welcome.
