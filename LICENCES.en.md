# Licence scope

🇫🇷 [Version française (par défaut)](LICENCES.md) · 🇬🇧 English (this page)

This repository combines **two very different kinds of content**, which are not under the same licence.
This distinction is essential, legally and ethically: **we do not relicense anyone else's code.**

In copyright terms, this repository is a **mere aggregation** (within the meaning of Section 5 of the
AGPLv3, "mere aggregation"): our original work, placed under AGPLv3+, is distributed **alongside**
third-party works that each keep their own licence. The AGPLv3 applies to our work; it **does not extend**
to the aggregated third-party works.

---

## 1. Our original work — AGPL-3.0-or-later

**SPDX-License-Identifier: `AGPL-3.0-or-later`**

Everything **produced by this audit** is placed under the GNU Affero General Public License, version 3 or
later. The full text is in the [`LICENSE`](LICENSE) file.

Covered:

| Path | Nature |
|---|---|
| `rapport-audit/*.md`, `rapport-audit/en/*.md` | Reports, synthesis, inventory, blind spots, protocol |
| `rapport-audit/*.csv` | Finding tables produced by the audit |
| `rapport-audit/outils/**`, `rapport-audit/phase5-kit/**` | Reproduction scripts and observation kit written for the audit |
| `rapport-audit/donnees/**` (*see the reservation below*) | Generated analysis data: fingerprints, censuses, unified indexes, surveys |
| `README.md`, `README.en.md`, `LICENCES.md`, `LICENCES.en.md` | Repository documentation |

> **Reservation on `rapport-audit/donnees/`.** This folder mostly contains our analysis output
> (AGPLv3+), **but also** some **artefacts published by openKylin** and redistributed as-is for
> verification: signed APT indexes (`InRelease`, `Sources.gz`, `Packages.gz`, `Contents-*`), ISO manifests
> and metadata. These artefacts belong to their publisher (openKylin / KylinSoft); they are included so
> that anyone can **re-verify** the signatures and fingerprints, not as original work of the audit. They
> fall under category 2 below.

Copyright © 2026 the contributors of this audit.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero
General Public License as published by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. It is distributed in the hope that it will be useful, but **WITHOUT
ANY WARRANTY**; see the licence for details.

---

## 2. Third-party code and artefacts — original licences, **unmodified**

**`SPDX-License-Identifier`: does not apply uniformly — see each component.**

The [`sources-openkylin/`](sources-openkylin/) folder contains third-party **source code** (openKylin, and
upstream Debian / Ubuntu / various free-software projects) as well as **binary packages** and
**repository artefacts**. This content:

- **was not written by the audit**;
- is included **byte for byte**, without any modification (a copy of the analysed trees, see
  [`sources-openkylin/LISEZMOI.md`](sources-openkylin/LISEZMOI.md) *(French)* and
  [`sources-openkylin/EXCLUSIONS.tsv`](sources-openkylin/EXCLUSIONS.tsv));
- **fully keeps its original licence(s)**: our choice of the AGPLv3 for our work **does not relicense
  anything** and **cannot relicense anything** here.

Each source package carries its own licence, usually in its `debian/copyright` file and/or file headers.
Depending on the component you will find in particular: **GPL-2.0**, **GPL-3.0**, **LGPL-2.1**,
**LGPL-3.0**, **Apache-2.0**, **MIT**, **BSD** (2- and 3-clause), **MPL**, **public domain**, and, for
some components, **proprietary licences**.

> **Why include this code?** For **reproducibility**: a neutral audit must allow a third party to check
> each finding on **exactly** the same bytes that were analysed. The fingerprints of all trees are in
> `rapport-audit/donnees/empreintes-arbres-sources.tsv`.

### 2 bis. Proprietary / source-less binaries

The `sources-openkylin/binaires-sans-source/` sub-folder contains `.deb` packages **with no usable
source** (`dh_make` skeletons, or proprietary third-party components: for example WPS Office, Sogou,
iFlytek, CrossOver, drivers, microcodes, AI models). They are redistributed **as published** by the
official openKylin repositories, with their provenance and SHA-256
(`rapport-audit/donnees/binaires-sans-source/provenance.tsv`).

**These components remain subject to the licence agreement of their respective publisher, which may be
restrictive.** Their presence here implies **neither** approval **nor** the granting of any right by the
audit: check the applicable licence before any use or redistribution.

---

## 3. When in doubt

- A file is under `sources-openkylin/` → the **component's original licence** (category 2). Look for its
  `debian/copyright`.
- A file is under `rapport-audit/` → **AGPL-3.0-or-later** (category 1), except the redistributed
  openKylin artefacts flagged above.

If a file mistakenly carried no clear indication, **category 2 prevails out of caution** for any content
derived from a third party: when in doubt, we never claim someone else's work as our own.
