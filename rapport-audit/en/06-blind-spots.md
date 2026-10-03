# 06 — Audit blind spots

🇫🇷 [Version française (par défaut)](../06-angles-morts.md) · 🇬🇧 English (this page)

> **Status**: document produced at the end of Phase 0 (inventory), **before** any semantic reading of the
> code. It will be completed at each phase. Surveys of 2026-10-01 (UTC).
> Registers: **[ESTABLISHED]** = proof (file, signed index, server response); **[PROBABLE]** = strong
> indication, not confirmed; **[NOT COVERED]** = out of the audit's reach at this stage.

A blind spot is not an accusation: it is an area where the audit **cannot** establish what the distributed
software does. The absence of a finding in these areas never means "safe".

---

## 1. Distributed components whose source package contains no code

**[ESTABLISHED]** 49 source packages from `archive.build.openkylin.top` (`huanghe` suite) contain **no file
outside `debian/`**; 48 of them contain only the unfilled `dh_make` skeleton (10 `*.ex` files), while their
binaries weigh from 100 KB to 4.7 GB. The distributed binary therefore cannot have been built from the
published source. Full list:
[`../donnees/sources-coquilles-vides.csv`](../donnees/sources-coquilles-vides.csv).

**Honesty check**: `binutils-mingw-w64` appears on this list but falls under a legitimate Debian pattern
(built from the `binutils-source` package); it is treated as a **false positive** of the automated
detection.

**[ESTABLISHED]** 22 other source packages with no code at all have binaries under 100 KB; some are
legitimate metapackages (`meta-kde`, `xfce4`, `rust-defaults`…), but others do ship executable files or
libraries with no source: `libkylin-ai-default-key`, `dpkg-wrapper`, `kaiming-box`, `kmre`,
`liblol`/`liblol-dkms`, `app-compat-dkms`, `kylin-kmre-image-update-x64`, `kylin-ai-cloud-model` (which
produces the three `ai-kylin-*-cloud-model` packages installed by default), `kylin-freetrial-ai-engine-plugin`,
`libime-cpis`, `wemeet`. The 100 KB threshold above therefore understates the phenomenon; the up-to-date
list is maintained by the census agents (see [`01a-points-entree-detail.md`](../01a-points-entree-detail.md)
*(French)*, nature "absent-or-empty").

### 1.1 Installed by default by the official Desktop 3.0 ISO (build 20260918, x86_64) — 17 packages

| Source package | Declared role (metadata) | Code elsewhere? |
|---|---|---|
| `kylin-software-center` | App store ("*Software store access the server to get data*") | Gitee `master` (5,111 files) carries **a different product** (`ubuntu-kylin-software-center 4.5.77.1`) than the shipped version (5.0.9.1): the distributed binary's code is not published; **binary content audited in wave 3** (`donnees/phase1/V3-L1-boutique.json`) |
| `kylin-sso-client` | Provides `kylin-id`, `kylin-cloud-sync` ("*UI of cloud-sync*") | **No public repository**; **binary content audited in wave 3** (`donnees/phase1/V3-L2-kylin-id.json`) |
| `kydoctor` | `kydoctor-server` (depends on `libcurl4`, `sqlcipher`, `libsystemd0`) — no description | **No public repository** |
| `openkylin-token-center` | Not described | **No public repository** |
| `kylin-software-properties` | APT sources manager | Gitee: `debian/` only |
| `ostree-maintain-daemon`, `ostree-coredata-protect`, `ostree-extra-utils` | Daemons/tools of the immutable OSTree system | **No public repository** |
| `llm-backend`, `onnxruntime-backend`, `onnxruntime` | AI inference engines | No public repository (upstream `onnxruntime` is public, but the binary is not rebuilt here) |
| `kylin-freetrial-nlp-engine` | "Free trial" NLP engine | Gitee: `upstream` branch |
| `kylin-screencap`, `kylincameralibs`, `kylin-sane-virtual-backend` | Screen capture, camera, virtual scanner | **No public repository** |
| `kare` | Application compatibility environment | Gitee: `openkylin/nile` branch (1 MB) |
| `wps-office`, `kylin-fonts`, `openkylin-fonts` | Proprietary office suite, fonts | N/A / none |

### 1.2 Not installed by default but distributed — high-stakes examples
`kylin-code` (IDE, 356 MB of binaries; code present on Gitee, see §7), `kylin-mfa` (multi-factor
authentication, no repository), `kylin-driver-manager-service`, `kylin-background-upgrade`,
`kylin-web-app-engine`, `kmre-apk-installer` and the whole Android stack `kylin-kmre-*` (daemon, manager,
window, emugl, display-control, DKMS modules, 1.5 GB Android system image).

---

## 2. `software.openkylin.top` repository: binaries only

- **[ESTABLISHED]** The repository is enabled by default (ISO's `/.kylin-post-actions`, signed with the
  archive key) but publishes **no `Sources` index**: 3,132 binary packages (`main` and `all` components), 0
  sources.
- **[ESTABLISHED]** 5 packages from it are in the default install: `net.mozilla.firefox 146.0.1` (the
  browser actually installed), `kylin-cua 1.0.0` ("computer use" agent), `com.iflytek.iflyime` (iFlytek
  input method), `app.web.openkylincommunity`, `fonts-urw-base35`.
- **[ESTABLISHED]** Notable content (not installed by default, available via the store): `audit-kylin`
  (description: "classified information verification" tool, offline and online analysis), `kguard`
  ("*Kylin lightweight HIDS*"), `kguard-net-ai` ("*AI-powered network traffic detection engine*"),
  third-party security agents (`cn.com.topsec.edragent`, `cn.com.topsec.topnac`, `com.360.epp`,
  `cn.huorong.esm`, `cn.com.sangfor.atrust`), "cloud" AI models (`ai-kylin-deepseek-*-cloud-model`,
  `ai-kylin-qwen-plus-cloud-model`), AI "skills" (`ai-kylin-skill-*`).
- **[ESTABLISHED]** The `Packages` index has non-conformant fields (`Installed-Size: 1.9 MB`), a sign of
  publishing tooling outside the standard Debian chain.
- **[NOT COVERED]** The behaviour of these 3,132 binaries is out of reach of the static audit.

## 3. Kylin-origin components with no public repository at all

**[ESTABLISHED]** 74 Kylin-origin source packages published in `archive.build` have no repository on either
Gitee or GitHub (absent or empty), including **35 installed by default** — notably `kydoctor`,
`kylin-sso-client`, `kylin-bot`, `kylin-wget`, `openkylin-token-center`, `libkysdk-mcp`,
`libkyai-business-framework`, `kylin-ai-cloud-model`, `ostree-*`, `ukui-service-manager`, `kytensor`,
`llm-backend`. For those whose source package does contain code — e.g. `kylin-wget` (85 files,
Rust/Python), `libkysdk-mcp` (120 files, C++), `kylin-bot` (1,629 files, Rust/TypeScript/JavaScript) — the
archive remains the only reference; development history is inaccessible.
List: `gitee`/`github` columns of
[`../donnees/inventaire-paquets-sources.csv`](../donnees/inventaire-paquets-sources.csv).

## 4. Non-public forges referenced by metadata

- **[ESTABLISHED]** 6 source packages declare a `Vcs-Git` on `gitlab2.kylin.com` (internal forge, not
  publicly reachable): `kylin-ai-abstract-models`, `kylin-ondevice-vision-engine`, `kylin-fix-boot-grub`,
  `libkysdk-desktop`, `libkysdk-diagnosetest` (which produces `libkysdk-datacollect`, installed by
  default), `ukui-kwin-effects`.
- **[ESTABLISHED]** 193 binary entries declare a `Homepage` on `gitlab2.kylin.com` (25 sources, including
  `kylin-process-manager`, `kylin-software-center`, `kylin-connectivity`, `libkysdk-base`,
  `libkysdk-system`) and 20 on `gogs.kylin.com` (biometrics).
- **[PROBABLE]** The reference development of these components happens on an internal forge; Gitee would
  be a partial mirror of it. The equality "Gitee = internal forge = binary" cannot be demonstrated from
  the outside.

## 5. Precompiled binaries present in the source trees ("blobs")

**[ESTABLISHED]** The structural extraction of P1 components lists 604 binary files (ELF, archives, `.deb`
packages, `.vsix`, `.whl`, `.onnx`/`.gguf` models, SQLite databases), totalling 1.40 GB. Examples not
rebuilt from the sources present:

| Tree | File | Nature |
|---|---|---|
| `kylin-connectivity` (archive) | `3rdparty/tools/amd/anti-control-server`, `anti-control-client` | ELF x86-64 (12 MB / 10 MB) |
| `kylin-connectivity` (archive) | `3rdparty/tools/{amd,arm}/kylin-screencast.AppImage` | ELF/AppImage (81–86 MB) |
| `yhkylin-systembackup-tools` (archive, **installed by default**) | `systembackup-daemon/systembackup-daemon` | ELF (26 MB) |
| `yhkylin-backup-tools`, `yhkylin-rockbackup-tools` | `kybackup/kybackup`, `backup-daemon/backup-daemon` | ELF (15–29 MB) |
| `revisions-manager` (archive) | `daemon/data/depends/*/docker.io_*.deb`, `containerd_*.deb` | embedded `.deb` packages |
| `kylin-ai-business-framework-service` | `python-depends/*/numpy-*.whl`, `transformers-*.whl` | precompiled Python wheels |
| `kylin-camera` | `camera` | ELF (12.9 MB) |
| `extensions-repo` (Gitee, kylin-code) | 66 `.vsix` files (Java, Gradle, Maven, Python, remote-dev…) | packaged extensions |
| `kylin-software-center` (Gitee) | `data/uksc.db` | SQLite database (14.7 MB) |

Full detail: `blobs` field of
[`../donnees/points-entree-extraction-structurelle.json`](../donnees/points-entree-extraction-structurelle.json).
**[NOT COVERED]** The content of these files cannot be audited at the source level; only
`file`/`strings`/`readelf` will be possible (Phase 1-D).

## 6. Gap between the official ISO and the published repositories

- **[ESTABLISHED]** Desktop 3.0 x86_64 ISO of 2026-09-18: 1,920 installed packages; 1,651 come from
  `archive.build/main`, **143 from the `anything3.0` PPA only** (separate key "*openKylin Build Service PPA
  for kylinsoft developers*", `Origin: LP-PPA-kylinsoft-anything3.0`), 13 from `pty`, 5 from
  `software.openkylin.top`.
- **[ESTABLISHED]** 18 ISO binaries are no longer indexed at their exact version; about a dozen remain in
  the `pool`, **5 have been removed (HTTP 404)** including `openkylin-token-center 1.0.0-ok1.7` and
  `ostree-coredata-protect 2.0.0.0-0k2.23.6.0`, and `kylin-update-notify` appears in **no** index at all.
- **[ESTABLISHED]** For 76 installed packages, no source at the exact version is published; notable
  examples: `policykit-1 124-ok6.1` (the published source is at 127 and no longer produces this binary),
  `coreutils 9.4-ok4`, `libsqlcipher1 4.5.6-ok1`.
- **[ESTABLISHED]** In total, **76 candidate components are classed X** (not auditable at the source
  level), of which **36 installed by default**.
- **[ESTABLISHED]** `/.kylin-post-actions` installs NVIDIA drivers via `dpkg -i /opt/nvidia/nvidia-*.deb`
  (outside APT signature verification); these `.deb` files reside in the ISO's filesystem.
- **[NOT COVERED]** `filesystem.squashfs` (3.2 GB) was not extracted: the static audit does not verify
  that installed files match the published packages byte for byte (to be addressed in Phase 4/5).
- **[NOT COVERED]** Variants not examined: arm64, loong64, "AgentOS", Server, Embedded, WSL ISOs.
- **[ESTABLISHED]** Only **MD5** fingerprints are published (`www.openkylin.top/md5.html`), with no file
  name or build date while three Desktop x86 3.0 ISOs coexist; no detached signature on two mirrors.

## 7. Forges: incomplete enumeration and divergences

- **[ESTABLISHED]** The Gitee organisation `openkylin` announces **4,770 repositories**; anonymous
  enumeration is blocked beyond 400 (redirects to login) and the organisation API returns HTTP 500.
  Repositories were therefore verified **by name** (3,404 names tested): 3,139 Gitee repositories
  identified out of 4,770 announced (≈ 66 %); about 1,630 repositories, not tied to a distributed package,
  were not identified.
- **[ESTABLISHED]** Of 2,778 repositories present on both Gitee and GitHub, **1,693 have diverging branch
  heads**; 156 are empty on GitHub, 190 absent. Whether this is a lagging mirror or genuinely different
  content is not yet established.
- **[ESTABLISHED]** A third forge is used: `gitcode.com/openkylin` (Vcs-Git of the `linux 7.0.0-3.0ok1`
  kernel and of `openhitls`).
- **[ESTABLISHED]** The `libkylin-ai-system-private` and `libkysdk-ai-private` libraries have a Gitee
  repository reduced to a template README; **their code is however published in the PPA** (source package
  with C++ and gRPC/D-Bus protocols).
- **[ESTABLISHED]** A competing archive `archive.openkylin.top` publishes a different `huanghe` suite
  (dated 2026-08-28, 3,042 sources) from the one actually configured (`archive.build.openkylin.top`,
  2026-09-30, 3,238 sources).

## 8. Upstream forks (P2): limitations at this stage

- **[ESTABLISHED]** 334 packages are in **native** format (`3.0 (native)`): the boundary between upstream
  code and openKylin modifications is erased; the differential audit will require rebuilding the upstream
  tree.
- **[ESTABLISHED]** 322 packages have an `orig.tar` with the **same name but a different fingerprint** from
  Debian/Ubuntu (among the installed ones: `glibc`, `curl`, `gnutls28`, `grub2`, `dbus`, `linux`, `nettle`,
  `libseccomp`, `krb5`, `policykit-1`, `sqlite3`, `python-cryptography`, `tzdata`). This may simply be a
  repackaging; to be settled in Phase 2.
- **[NOT COVERED]** In Phase 0, only the `debian/` parts of the P2 forks were acquired (the full upstream
  trees will be in Phase 2).

## 9. Proprietary third-party components (P1-X)

**[NOT COVERED]** `wps-office`, `sogoupinyin`, `crossover`, `cpis`/`libime-cpis`, `arise`/NVIDIA drivers,
cuDNN/CUDA, Intel/AMD microcode, `linux-firmware`, AI models (`kylin-qwen2.5-3b-*`,
`kylin-*-model-onnx`, `kylin-datamanagement-models`, 3.3 GB), Android image
`kylin-kmre-image-data-x64` (1.5 GB), KARE bases (1.7 GB). Their behaviour can only be established through
binary and dynamic analysis (Phase 5).

## 10. Methodological limitations of Phase 0

- "Kylin-origin" classification by heuristics (names, `kylinos.cn` maintainers, native format absent from
  Debian/Ubuntu): false positives/negatives are possible; the list was supplemented by cross-checking
  against the ISO (v3, 368 candidates).
- Entry-point detection by path patterns and content signatures, with no execution; network-listening
  indications are lexical matches to be confirmed.
- Only the **amd64** architecture was indexed; arm64, loong64, riscv64 and i386 are not covered.
- Sources of truth used: signed APT indexes (key `09FF C10E A273 DD29 A986 B110 8B31 3CEA FF59 2D96`), ISO
  read via partial requests, forges consulted read-only. None of these sources alone proves the equality
  "published code = executed binary".

## Blind spots revealed by Phases 1–2 and wave 3 (surveys of 2026-10-01 and 2026-10-02)

| Blind spot | Scope | Source / proof |
|---|---|---|
| **`coreutils 9.4-ok4`**: binary installed by default (base commands `ls`, `cp`, `rm`, `chmod`…) **with no source in any of the 10 signed `Sources` indexes**; only 9.5 (`gnu-coreutils`, prefixed) is published and audited | the system's most frequently run binaries are not statically auditable | `donnees/phase2/P2-index-sources.json`; ISO manifest l.56 |
| **`libssl1.1`** (end-of-life OpenSSL 1.1.1f) installed; the repackaged upstream archive is absent from the corpus | post-2020 CVE fixes cannot be verified by name | `donnees/phase2/P2-openssl.json` |
| **Upstream archives** absent or different from Ubuntu's: curl, gnutls28 (`.orig` ≠ Ubuntu), kernel 7.0, Triton (kytensor), llama.cpp (kytensor-llm) | the openKylin delta outside `debian/` cannot be verified | `donnees/phase2/`, `donnees/phase1/V1-L13-*` |
| **Source-less AI plug-ins**: `libkylin-ai-default-key`, `kylin-freetrial-nlp-engine`, `llm-backend`, `onnxruntime-backend` | real destination, TLS and consent of the default free-trial cloud model | `donnees/phase1/V1-L8-*`, `V2-L4-*` |
| **`.limit` allowlist**: the component that would enforce it (patched dbus/kysdk) is absent from the corpus; the shipped configuration disables it (`audit: false`) | real access control of system services | `donnees/verification/G2-*`, `G5-*` |
| **Third-party D-Bus services invoked**, not audited: `org.ukui.UniauthBackend`, `com.kylin.kim.backend`, `com.kylin.RemoteDesktop` | lock-screen authentication, remote access | `donnees/phase1/V2-L1-*` |
| **External binaries**: `kylinsigntool` (`.deb` signature verification), `kylin-os-manager-remote-user` (remote assistance), the `ud650-extra.so` blob | trust chains and remote access | `donnees/phase1/V1-L5-*`, `V2-L3-*` |
| **Execution conditions** not settleable statically: real permissions of kaiming's `rules.d`, umask of the AI sockets, Firefox 146's "Local Network Access" protections, factory stations in `ple-mode` | reachability of several findings | `07-verification.md` (UNDETERMINED), `04-protocole-dynamique.md` |
| **`ostree-extra-utils`** (installed by default) and **`dpkg-wrapper`**: published source reduced to a `dh_make` shell, generated after the fact (10 versions on the binary side, a single one on the source side); `unified-conf` ELF characterised without source | real content and history not traceable to source | `donnees/phase2/P2-binaires-sans-source.json` |
| **Real permissions of `/opt/system`** and `/var/opt/system`, now added to the library, Python, `PATH` and systemd paths; exposure of `kare` (published source = `debian/` only) | writable by a non-root user = root code execution | Phase 5 kit (D16), `donnees/verification/G9-*` |
| **openKylin's `gcc-15` compiler** and build logs (outside the corpus) | cause of the missing PIE and of the unoptimised compilation | `donnees/phase2/P2-durcissement-binaires.json`, `donnees/verification/mesures-G9/` |
| **GRUB EFI images shipped** and signing infrastructure (`grub2-signed` downloads unpublished images); `os-prober` → `grub-mount` path (same NTFS driver, as root) | real exposure to the GRUB CVEs | `donnees/phase2/P2-grub2*.json`, `donnees/verification/G8-*`, kit (D18) |
| **aptdaemon ok1 to ok5** (openKylin 1.x): `debian/` and binaries unavailable | past exposure to the `SetEnviron` method | `donnees/phase2/P2-outils-ubuntu.json` |
| **`kylin-system-updater` `GetBackendStatus`** (open to all) rewrites `LANG`/`LANGUAGE` of the root daemon: lead found during verification G8, not investigated | real effect on the root daemon | `donnees/verification/G8-*`, kit (script 20, introspection) |
| **Compiled executables of the app store and kylin-id**: characterised (`readelf`, strings, targeted disassembly), not fully decompiled | behaviour outside the paths examined | `donnees/phase1/V3-L1-*`, `V3-L2-*` |
| **App-store "strategies" server** (`api.kylinos.cn`): content actually pushed, targeting criteria, frequency | actual use of the remote management channel | `donnees/verification/G10-*` |
| **Server-side identity service** (`id.kylinos.cn`): SMS rate limiting, token issuance | actual exposure of kylin-id | `donnees/verification/G11-*` |
| **Installed system bus**: actual `dbus` configuration and absence of any component enforcing `.limit` at run time | real reachability of `kpr-backend` and the other open services | `donnees/verification/G12-*` |
| **UKUI session interfaces** (batches V3-L4 to V3-L7): system services read in full, session interfaces examined by targeted search (2 to 8 % of lines read) | the absence of network flows rests on this triage, not on an exhaustive reading | `donnees/phase1/V3-L4-*` to `V3-L7-*` |
| **Upstream Bluetooth stack** (`bluez`, `obexd`: only `debian/` in the corpus): possible filtering of received file names, pairing requirement | real reachability of the Bluetooth finding | `donnees/verification/G14-*` |
| **Installed state of the system monitor** (binary capabilities, `/usr` mount options) | real severity of the finding left undetermined | `donnees/verification/G13-*` |
| **KMRE and kylin-code**: empty archive sources; the versions published on Gitee all differ from the distributed versions (usually older). Only `kare` (application runtime environment) is installed by default; the other KMRE packages, including the kernel module, and kylin-code are not. KMRE binary packages inventoried (signed chain, no execution), not analysed | not audited | § 1, `donnees/phase1/vague3/V3-L8-kmre-binaires.json` |

**Resolved in wave 3**: `org.kylinID.service` (kylin-id), previously listed as not audited, has been audited, as
has `org.kylin.KprBackend` (kba) (`donnees/phase1/V3-L2-*`, `V3-L3-*`) — the app store and kylin-id, lacking source,
on their binary content. 24 UKUI components installed by default were also audited (batches V3-L4 to V3-L7).

**Resolved in Phase 2**: `sudo-common 1.2ok1` (carries `/etc/sudoers` and sudo's PAM stack), absent from the
initial corpus, was acquired through the signed chain and fully audited
(`donnees/phase2/P2-sudo-common.json`). The upstream archives of glibc, grub2, dpkg, network-manager, wpa,
aptdaemon and apport were compared with signed Ubuntu, Debian and GNU references (`outils/references_p2.py`,
`donnees/phase2/references/`).
