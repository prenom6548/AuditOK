# openKylin sources acquired for the audit

🇫🇷 [Version française (par défaut)](LISEZMOI.md) · 🇬🇧 English (this page)

> **Licence: THIRD-PARTY code and artefacts.** All the content of this folder keeps **its original
> licence** (openKylin, and upstream Debian/Ubuntu/various free-software projects; sometimes
> proprietary). It is **not** covered by the rest of the repository's AGPLv3: the audit relicenses
> nothing. See [`../LICENCES.en.md`](../LICENCES.en.md) (§ 2) and, for each package, its
> `debian/copyright`.

Corpus used by the audit (`../rapport-audit/`), included in this repository at the lead auditor's request.
Surveys of 2026-10-01 (UTC). No file was modified: this is a byte-for-byte copy of the analysed trees,
minus the exclusions listed in [`EXCLUSIONS.tsv`](EXCLUSIONS.tsv).

| Folder | Content | Provenance and authenticity |
|---|---|---|
| `archive/<package>/` | 311 source packages extracted with `dpkg-source -x` (quilt patches applied); for non-native P2 forks, `debian/` only | `http://archive.build.openkylin.top/openkylin/` (`huanghe` suite) and the `anything3.0` PPA; files verified by SHA-256 against the `Sources` index whose signed `InRelease` was verified |
| `gitee/<repo>/` | 40 shallow clones (1 commit) | `https://gitee.com/openkylin/<repo>`; branch and commit in [`gitee/COMMITS-GITEE.tsv`](gitee/COMMITS-GITEE.tsv); `.git` folders removed |
| `dsc/` | 312 `.dsc` files of the acquired source packages | same as `archive/` |
| `index-apt/` | `InRelease`, `Sources.gz`, `Packages.gz`, `Contents-amd64.gz` of the default repositories and of the `archive.openkylin.top` archive | Signatures verifiable with the keys in `../rapport-audit/donnees/apt/` |
| `binaires-sans-source/` | 42 `.deb` packages of components with no usable source (≤ 100 MB) | URL and SHA-256 in `../rapport-audit/donnees/binaires-sans-source/provenance.tsv` |

## Checking that a tree is the one that was audited

The fingerprint of every tree is in `../rapport-audit/donnees/empreintes-arbres-sources.tsv` (column 3).
From this folder:

```sh
find archive/<package> -path '*/.git' -prune -o -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum
```

Expected exceptions: the two archive trees stripped of a file larger than 100 MB
(`kylin-ai-business-framework-service`, `libkysdk-system`) have a different fingerprint here; the missing
files can be retrieved from the package's `debian.tar` (URL and SHA-256 in the `Sources` index) and their
SHA-256 is listed in `EXCLUSIONS.tsv`.

## Exclusions

- `.git` folders (Git cannot version a `.git` path) — including a `.git` folder present **inside the
  `ukui-voice-wing` source package itself**.
- Files larger than 100 MB (GitHub's limit): two `debian/patches` patches titled "update changelog"
  (234 MB and 441 MB) and two binary packages (`kylin-software-center`, `com.iflytek.iflyime`).
- Downloaded `orig.tar.*` / `debian.tar.*` archives: not duplicated (content extracted into `archive/`,
  re-downloadable identically via the signed indexes).

## Technical note

Several trees carry their own `.gitattributes` (including `filter=lfs` rules). Files were versioned as raw
bytes, with no conversion or LFS. On extraction, disable the filters to get the same bytes, for example:
`GIT_LFS_SKIP_SMUDGE=1 git clone …`, then
`printf '* -text -eol -filter\n' > .git/info/attributes && git checkout -f`.
