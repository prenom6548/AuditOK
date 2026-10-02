# 00 — Synthesis of the openKylin 3.0 "huanghe" audit

🇫🇷 [Version française (par défaut)](../00-synthese.md) · 🇬🇧 English (this page)

> **Status: Phases 0, 1 (waves 1–3 and deep-dives) and 2 (18 fork batches and 2 cross-cutting batches) are complete; every critical
> and high finding went through an adversarial verification; Phase 5 is delivered as an observation kit,
> kept up to date as the audit progresses.** Surveys of 2026-10-01 (UTC); wave 3 of 2026-10-02.
> Registers: **[ESTABLISHED]** (proof: file : line — `[CONSTATÉ]` in the French originals), **[PROBABLE]**
> (strong indication, not confirmed), **[NOT COVERED]** (`[NON COUVERT]`, out of reach).
> Principle: **we never conclude "safe" from an absence of evidence.**
>
> Links marked *(French)* point to documents not yet translated. Terminology:
> [`../../README.en.md`](../../README.en.md#terminology).

## 1. Overall verdict

| Question | Answer as of now |
|---|---|
| Backdoor / unsolicited remote code? | **No backdoor [ESTABLISHED]** in the code and binaries read. However, the app store installed by default opens a **remote management channel controlled by the vendor**, active without any device enrolment (installs limited to signed packages, uninstalls unrestricted) (§ 3.3). Not excluded for the unread volume and the opaque components (§ 6). |
| Real security weaknesses? | **Yes [ESTABLISHED]**: 704 findings; after verification, **9 critical** and **56 high** (§ 3). |
| Collection beyond what is declared? | **Gaps [ESTABLISHED]** in qualification, transport and consent (§ 4, `05-donnees.md` *(French)*). |
| Critical upstream forks (OpenSSH, systemd, OpenSSL 3, sudo, APT, crypto, glibc, GRUB, dpkg…)? | **No backdoor and no tampered upstream code [ESTABLISHED]**: the upstream code checked is authentic (glibc identical to the signed GNU archive, GRUB identical to a signed GNU pre-release). But **security-fix lag** (glibc, GRUB, NetworkManager, apport) and **binaries shipped without the standard compiler protections** (§ 3.3, § 3.4). |

**Overall reading.** The dominant pattern, established and confirmed by the adversarial verification, is not a
remote backdoor but **defective privilege engineering in Kylin-specific components**: many system services
exposed to any local user without effective caller checks, over-broad authorisation policies, and
unvalidated input in privileged code. The allowlist mechanism meant to restrict these services (`.limit`) is
**not enforced by default**. On top of this: **telemetry with hardware identifiers** sent over
unauthenticated channels, an **OSTree update chain with no authenticity verification**, and **poor
traceability** of the packaging (42 patches whose title has no relation to their content).

The end of Phase 2 adds two **cross-cutting** weaknesses, with no sign of malicious intent: **shipped binaries
largely stripped of the standard compiler protections** (PIE, FORTIFY, `BIND_NOW`), including setuid binaries
and root services, and a **lag in security fixes** on core components (glibc, a GRUB boot loader left at a
pre-release, NetworkManager).

Wave 3 (app store, Kylin ID identity, local password reset) confirms the dominant pattern: two new **critical
local privilege escalations** in the password-reset service (`kpr-backend`, source published and matching the
shipped binary). In two components **shipped without source** (app store, Kylin ID), it establishes identity
connections without certificate verification and a **remote management channel** in the app store, targeted
by hardware identifiers and active by default. The additional UKUI batches (24 components installed by default)
confirm the pattern: root system services for networking, Bluetooth and file sharing without effective caller
checks. Conversely, the biometric PAM module does not let anyone through on failure, and one authentication
service does check its callers.

## 2. Progress

| Phase | Status | Deliverables |
|---|---|---|
| 0 — Inventory, entry points, matrix | Complete | [`01-inventaire.md`](../01-inventaire.md) *(French)*, [`01a-points-entree-detail.md`](../01a-points-entree-detail.md) *(French)*, [`06-angles-morts.md`](../06-angles-morts.md) *(French)* |
| 1 — Static P1: wave 1 (11 batches) + 4 deep-dives | Complete | [`02-findings.csv`](../02-findings.csv) *(French fields)*, [`donnees/phase1/`](../donnees/phase1/) |
| 1 — Static P1: wave 2 (4 batches: UKUI session and shell, identity/telemetry, cloud AI) | Complete | same |
| 1 — Static P1: wave 3 (7 batches: app store, Kylin ID, password reset — binary content for the first two, which have no source — then 24 UKUI components in 4 batches) | Complete | same, [`donnees/phase1/vague3/`](../donnees/phase1/vague3/) |
| 2 — Differential of P2 forks (18 fork batches + 2 cross-cutting batches: source-less binaries, binary hardening) | Complete; verified Ubuntu/Debian/GNU reference sources | [`03-diffs-suspects.md`](../03-diffs-suspects.md) *(French)*, [`donnees/phase2/`](../donnees/phase2/) |
| 3 — Data flows | Waves 1–2 consolidated | [`05-donnees.md`](../05-donnees.md) *(French)* |
| Adversarial verification (83 findings ≥ high) | Complete | [`07-verification.md`](../07-verification.md) *(French)*, [`donnees/verification/`](../donnees/verification/) |
| 4 — Build chain | Elements surveyed | — |
| 5 — Observation kit (VM) | Delivered as a kit, updated with each wave; running it is left to the community | [`phase5-kit/`](../phase5-kit/LISEZMOI.en.md), method: [`04-dynamic-protocol.md`](04-dynamic-protocol.md) |

## 3. Results

**704 findings**: 226 wave 1 (+ deep-dives), 106 wave 2, 118 wave 3, 254 Phase 2. Severities **after
verification**: **9 critical · 56 high · 160 medium · 189 low · 290 informational**.

### 3.1 Adversarial verification (83 critical/high findings)

Each finding was re-read by a sceptical auditor tasked with refuting it: **57 CONFIRMED, 23 QUALIFIED,
3 UNDETERMINED, 0 REFUTED** (French originals: CONFIRMÉS, NUANCÉS, INDÉTERMINÉS, RÉFUTÉ). The qualifications
mostly **lowered** severities (narrower conditions: authentication still required, architecture not
installed, different default path); in two cases the re-read on the contrary **refuted benign
counter-readings** (service really enabled, code really reachable). The second verification wave (groups G8
and G9, 8 end-of-Phase-2 findings) **confirmed** the binary-hardening regression and brought the GRUB-related
findings down to "medium": on 3.0 amd64, GRUB boots without shim and nothing is signed, so there is no Secure
Boot to bypass (the flaws remain present). The third (groups G10 to G15, 15 wave-3 findings) confirmed 12,
qualified 2 (including the identity service open to all, brought down to "medium" as neither root takeover nor
token theft is possible) and left 1 undetermined (capabilities of the system monitor, to be observed on a real
system). Detail: [`07-verification.md`](../07-verification.md) *(French)*.

### 3.2 Critical findings (9, all CONFIRMED) — local privilege escalation

Described here by category, for defensive purposes; exact references in `02-findings.csv`.

| Component (installed by default) | Category | Batch |
|---|---|---|
| `kaiming` — polkit policy (2 actions) | authorisation granted without authentication to any user | V1-L0 |
| `kaiming` — application deployment | unauthenticated content processed with privileges | V1-L14 |
| `kylin-process-manager` | privileged service, unvalidated input, no caller check | V1-L4 |
| `ukui-service-manager` | privileged service with no caller check | V1-L4 |
| `kytensor` (AI inference server) | privileged service with an unauthenticated local API | V1-L8 |
| `biometric-authentication` | memory-handling defect in a privileged service open to all | V2-L3 |
| `libkba` (`kpr-backend`) — local password reset | privileged service with no caller check: changing another account's password, root included | V3-L3 |
| `libkba` (`kpr-backend`) | unvalidated input passed to a command run with privileges | V3-L3 |

### 3.3 High-severity themes (56)

- **System services open to any user** (`context="default"`, no polkit and no effective UID check):
  kylin-process-manager, kysdk (conf2, location), kylin-system-updater, ukui-settings-daemon,
  ukui-control-center, kylin-os-manager, the kaiming proxy. `.limit` allowlist not enforced.
- **Telemetry and transport**: three channels send hardware identifiers (serial, MAC, `machine-id`,
  activation key) — `kylin-daq` and `service-support` over HTTPS **without certificate verification**, the
  updater over **cleartext HTTP**; AI speech engines (Baidu, iFlytek) without certificate verification.
- **Trust chain**: OSTree remotes over HTTP with `gpg-verify=false` (integrity without authenticity); APT
  repositories without `Valid-Until` or `Signed-By` (medium).
- **Graphical stack**: Wayland compositor with no access control on privileged protocols (capture, input,
  exclusive keyboard grab).
- **AI agents**: `kylin-bot` configuration shipped without safeguards; AI services with weakly protected
  sockets.
- **polkit (fork)**: a patch titled "update changelog" that modifies the authorisation engine
  [ESTABLISHED].
- **Hardening of shipped binaries** [CONFIRMED]: on 18 core packages compared with the same Ubuntu packages,
  **16 PIE executables out of 234** against 234/234, FORTIFY 14/262 against 226/262, `BIND_NOW` 127/262 against
  251/262; setuid binaries (`passwd`, `chsh`, `su`, `mount`…) and root services (`dbus-daemon`, `polkitd`,
  `pkexec`) affected; none of the 52 openKylin-specific executables measured is PIE; signature of unoptimised
  compilation in 20 packages out of 32. Measured with `readelf` on the official packages, without execution.
- **App store** (installed by default, no published source): remote management channel **active by default**,
  without device enrolment. "Strategies" downloaded from the vendor's server, targeted by hardware identifiers,
  can trigger silent installs (limited to signed packages from the repositories) and unrestricted silent
  uninstalls. Hardware identifiers (MAC, serial numbers, activation code) attached to every request, absent from
  the "app store" section of the privacy policy.
- **Kylin ID identity** (installed by default, no source): connections to the identity service **without
  certificate verification** (credentials, tokens, SMS codes, hardware serial).
- **Password reset** (`kpr-backend`): open D-Bus policy, `.limit` allowlist ineffective with the shipped `dbus`,
  fixed encryption key making the token forgeable.
- **UKUI system services** (installed by default): network management (`kylin-nm`), Bluetooth and Samba file
  sharing (`peony-share`) exposed to any local user without effective caller checks; for Bluetooth, a privileged
  command built from the name of a received file (conditional on Bluetooth being enabled and the transfer being
  accepted); system monitor given privileged capabilities at installation (undetermined).

### 3.4 Upstream forks (Phase 2)

| Result | Forks |
|---|---|
| **No weakening observed** in the openKylin delta | OpenSSH (Ubuntu base, no Kylin patch), systemd, OpenSSL 3.5, sudo, sudo-common, APT, gnutls/nss/libgcrypt/gnupg2/glib-networking/curl, pam/shadow/lightdm (low/medium findings only); glibc (upstream code identical to the signed GNU archive, 86 patches identical in substance to Ubuntu's), dpkg (Ubuntu source taken as is), wpa, network-manager (no own code) |
| **Notable gaps** | polkit (engine modified under a misleading title); kernel: amd64 aligned with Ubuntu except the Downfall mitigation disabled, **loong64** with hardening largely removed (not the installed architecture); Hygon hardware patches; **GRUB**: the "2.12" archive is the **2.12~rc1** pre-release (GNU-signed) renamed, missing 108 upstream commits including the NTFS fixes CVE-2023-4692/4693, a Hygon TPCM verifier added under a "changelog" title, boot **without shim** on the ISO; **dpkg**: the `dpkg-buildflags` hardening policy is never loaded (misnamed module) |
| **Security-fix lag** | glibc (at least 11 CVEs fixed since by Ubuntu or Debian), GRUB (22 CVEs from 2025), NetworkManager (CVE-2026-19685, fixed by Ubuntu 10 days before the surveys), apport (CVE-2025-5054 and -5467; package not installed by default) |
| **Not auditable here** | `coreutils 9.4-ok4` (installed base commands) **with no source in any signed index**; `libssl1.1` (end-of-life OpenSSL 1.1.1) installed; upstream archives of curl and gnutls differ from Ubuntu's; `ostree-extra-utils` (**installed by default**: root dpkg hook, `/opt/system` added to the library, Python and systemd paths) and `dpkg-wrapper` (diverts `dpkg` and `apt`): signed binaries whose published source is an **empty shell** |

**Traceability**: **42 patches** across 11 batches carry a title unrelated to their content; their content,
read in full, is mostly benign (disabled tests, announced CVE fixes), with the notable exception of polkit.
In addition, Ubuntu patches were **silently folded** into the upstream archive (NetworkManager, aptdaemon), and
an **84 MB memory dump** from a developer workstation was left in aptdaemon's source archive (environment
variables; no secret detected).

## 4. Cross-check against the privacy policy

The **nature** of the system collection matches the declared categories. Gaps [ESTABLISHED]:
(1) collection described as "non-identifying" (l.176) while stable hardware identifiers are transmitted;
(2) transmission promised as "safe and controlled" (l.154) while several channels are unauthenticated or in
cleartext; (3) collections not enumerated (memory dumps, security-policy logs, contact details, device
identifiers, hardware identifiers attached by the app store to every request); (4) consent not established for system channels, **present but not informed** for the AI
assistant. Detail: [`05-donnees.md`](../05-donnees.md) § 3 *(French)*.

## 5. Real coverage

Very uneven and openly so: small, sensitive trees read **in full**; large trees read **on their privileged
surface** (root services, IPC, network, telemetry) with a low overall percentage.

| Coverage | Trees |
|---|---|
| **100 %** | kylin-daq, libkysdk-diagnosetest, kylin-wget, ukui-input-gather, ukui-service-manager (98 %), ca-certificates, keyrings; P2 patches: OpenSSH, systemd, OpenSSL 3, sudo, coreutils, polkit, pam/shadow/lightdm, crypto, glibc, grub2 (133/133 settled, upstream-archive gaps 191/191), dpkg, network-manager, wpa, aptdaemon, apport; contents of the `ostree-extra-utils` and `dpkg-wrapper` binaries; text files and scripts of the app store and kylin-id (executables characterised) |
| **25–90 %** (lines) | kba (`kpr-backend`) 83 %, kylin-system-updater 88 %, ukui-clipboard 44 %, ostree 41 % (upstream delta 100 %), biometric-authentication 38 %, kylin-ai-runtime 31 %, libkysdk-base 27 %, kaiming 24 % |
| **< 15 %** (targeted privileged surface) | wave-3 UKUI components (24 trees: system services read in full, session interfaces by targeted search, 2–8 % of lines), Wayland compositor 11 % (all exposed protocols read), kylin-bot 7 %, kytensor 7 %, libkysdk-system 7 %, kylin-os-manager 5 %, UKUI session/shell 0–6 % |

A defect located outside the zones read would not be detected. Per-batch detail: evidence sheets in
`donnees/phase1/`, `donnees/phase2/` (key `couverture`, i.e. coverage).

## 6. Limitations of the audit (explicit list)

- **Volume**: ~90 components analysed out of 368 selected (3,238 sources in the default archive); the
  `anything3.0` PPA (37,252 sources) and `software.openkylin.top` (3,132 binaries) covered by inventory only.
- **Source-less components audited on the binary** (wave 3: app store, kylin-id; Phase 2: ostree-extra-utils,
  dpkg-wrapper): text files and scripts read in full, executables characterised (`readelf`, strings, targeted
  disassembly) but **not fully decompiled**.
- **Source-less binaries**: 76 components classed X (36 installed by default); plus `coreutils 9.4-ok4` and
  the AI plug-ins ("freetrial" engine, default key) whose real destination is unknown.
- **Upstream archives** absent for part of the P2 forks: delta outside `debian/` cannot be verified for curl,
  gnutls, kernel 7.0, Triton, llama.cpp; **verified** for glibc, grub2, dpkg, network-manager, wpa, aptdaemon and
  apport thanks to signed Ubuntu, Debian and GNU references (`outils/references_p2.py`).
- **Binary hardening**: measured on a sample of 32 packages; the configuration of openKylin's gcc-15 compiler
  (source outside the corpus) is not established.
- **Static only**: no compilation, execution or network access from the analysis. Findings are confirmed
  **in the code**, not **on a running system**; 2 UNDETERMINED verdicts and several reachability conditions
  are deferred to Phase 5 (real permissions, umask, browser).
- **`.limit` allowlist**: the component that would enforce it (patched dbus/kysdk) is absent from the
  corpus; the audit treats it as not enforced by default, which is verified on the shipped configuration.
- **Code ↔ distributed-binary equality** not demonstrated statically (spot fingerprints only).
- **Architectures**: amd64; the loong64 gaps noted concern the distributed source, not the ISO.
- **Verification**: one adversarial verifier per group, no redundant vote; only findings ≥ high.
