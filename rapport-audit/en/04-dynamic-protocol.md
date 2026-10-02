# 04 — Dynamic validation protocol (Phase 5)

🇫🇷 [Version française (par défaut)](../04-protocole-dynamique.md) · 🇬🇧 English (this page)

> **Nature of this document.** It describes **observation and verification procedures** meant to
> **confirm or refute**, in an isolated environment, the hypotheses drawn from the static analysis
> (Phases 1–2). It **provides no exploitation recipe**: every test measures a state (does an
> authentication prompt appear? does a flow go out in cleartext? is a file writable?); it "arms" nothing.
> Goal: move findings from the **[PROBABLE]** register to **[CONFIRMED]** or **[REFUTED]**, with
> timestamped proof.
>
> **Status: Phase 5 is delivered as an observation kit**, [`../phase5-kit/`](../phase5-kit/LISEZMOI.en.md),
> updated with each wave of the audit. This protocol is its methodological reference. Running it is left
> to the community, always in a dedicated virtual machine.

## 0. Ethical and safety principles

1. **Confirm, don't exploit.** We observe the system's behaviour; we do not develop a reusable offensive
   tool. Escalation tests stop at the **proof of absent control** (e.g. "the root method answers an
   unprivileged caller"), not at actual takeover.
2. **Strict isolation.** Everything runs **in a disposable VM**, on a network **with no real Internet
   access**. No corpus binary is executed on the host. No real data (hardware identifiers, telemetry)
   leaves the test bench.
3. **Responsible use.** This protocol observes and verifies; it provides no exploitation procedure. Any
   confirmation is to be handled for defensive purposes (see `README.en.md` § Responsible use).
4. **Reproducibility.** A fixed base image (ISO fingerprint noted), snapshots, timestamped logging; one
   test equals one clean snapshot.
5. **Free tools preferred** (consistent with the audit's approach): virtualisation **QEMU/KVM + libvirt**;
   capture **tcpdump/Wireshark**; TLS inspection proxy **mitmproxy**; lying resolution **dnsmasq/unbound**;
   audit **auditd**, `busctl`/`dbus-monitor`, `strace`. Avoid proprietary hypervisors.

## 1. Test bench

### 1.1 Topology
```
[ Analysis host ]                          (receives only logs + captures)
   └─ QEMU/KVM
       ├─ "target" VM: openKylin 3.0 huanghe (amd64), installed from the audited ISO
       └─ "network sandbox" VM (or namespace): the target's only network gateway
            ├─ dnsmasq: resolves ALL domains to the sandbox (sinkhole)
            ├─ mitmproxy: terminates TLS and logs the content (tests VerifyNone)
            ├─ lying HTTP/HTTPS server: answers telemetry / OSTree / repository APIs
            └─ tcpdump: pcap capture of the whole segment
```
The target has **no** route to the Internet: its only neighbour is the network sandbox, which
**captures and simulates**. We observe what the target *attempts* to send and *accepts* to receive,
without exfiltrating anything.

### 1.2 Images and accounts
- **Base image**: standard install from the ISO (ISO fingerprint noted); `base` snapshot.
- **Immutable variant**: OSTree install, if available; `base-ostree` snapshot.
- **Accounts**: `audituser` (standard user, not a sudoer) for every local-escalation test; root access is
  used only to *survey* the state (permissions, services), never as the starting point of an escalation
  test.

### 1.3 Instrumentation enabled before each test
- `auditd` with rules on `execve`, `ptrace`, mounts, writes into `/usr/share/polkit-1`, `/etc/polkit-1`,
  `/opt`, `/var/lib/dpkg/plugins`.
- `dbus-monitor --system` and `busctl` for IPC introspection and tracing.
- `tcpdump` on the network-sandbox side; `mitmproxy` as a transparent proxy.
- `journalctl -f` and a fixed NTP timestamp (no network sync).

## 2. Matrix of hypotheses to settle

Each row refers to `02-findings.csv` (columns `lot`, `fichier` — batch, file). Expected result phrased as
an **observable**, not as a gained access.

| # | Finding (batch) | Static severity | Hypothesis to settle | Decisive observable |
|---|---|---|---|---|
| D1 | kaiming polkit (L0) | critical | `pkexec` of the `org.kylin.kaiming.*` actions runs **without a prompt** for a standard user | `pkcheck`/`pkaction` + absence of an authentication prompt |
| D2 | kaiming rules.d (L0) | high | `/opt/kaiming/share/polkit-1/rules.d` and `/var/opt/kaiming/share/*` **writable** by a user | `stat`/`getfacl` on an installed system |
| D3 | kylin-process-manager `LockGSettingsKey` (L4) | critical | the root D-Bus method is **callable with no check** by `audituser` | `busctl` introspection + `dbus-monitor` trace of a **harmless** call |
| D4 | ukui-service-manager `launchServices` (L4) | critical | the root slot accepts an **arbitrary plug-in path** with no UID check | introspection; test with an **inert** diagnostic `.so` (logs its own loading, does nothing else) |
| D5 | kytensor root service (L8) | critical | the local API (127.0.0.1:8000/8001) answers **without authentication** | **harmless** `/v2/health` request + check of the service's `User=` |
| D6 | kylin-daq TLS (L1) | high | HTTPS upload with **unverified certificate** (`VerifyNone`) | `mitmproxy` intercepts the flow **without** a trusted certificate installed |
| D7 | kylin-daq reconfig (L1) | high | the **server response** reconfigures the daemon (persisted) | lying sandbox response → diff of `/var/cache/kylin-daq/kylinDaqConf.json` |
| D8 | updater DataAcquisition (L5) | high | **cleartext HTTP** channel to `archive1.kylinos.cn:32294` | cleartext pcap after a simulated update |
| D9 | libkysdk-location (L3) | high | request to `api.map.baidu.com` **in cleartext HTTP** with a hardcoded key | pcap + check whether the service is still called in 3.0 |
| D10 | OSTree gpg-verify=false (L10) | high | an **unsigned** commit served by the sandbox is **accepted** | trace of the `pull` against the lying remote (without deploying malicious content) |
| D11 | installer auditmode (L9) | high | `auditmode`/`ple-mode` (and `noauditmode`, `auditmode=0`) create a **persistent** NOPASSWD `test` account | presence of `/etc/sudoers.d/test`, autologin; search for `kylin-audit-mode` |
| D12 | compositor `/dev/fb*` 0666 + privileged protocols (L9) | high | screen capture / virtual input **with no access control** | effective udev rule + test with an unauthorised Wayland client (capture of a **test screen**) |
| D13 | kylin-ai-runtime sockets (L8) | high/medium | **effective** mode of the sockets in a `0777` directory; **default** cloud upload | `stat` of the sockets + capture of the freetrial cloud flow on first use |
| D14 | kysdk IPC `context=default` + allowlists (L2/L3/L4) | medium/high | are the **`.limit` allowlists** actually enforced? | call to an info method from `audituser`; presence/effect of the patched dbus |
| D15 | kernel hardening (Phase 2) | per diff | hardening options **actually** absent on the installed kernel | `/boot/config-*`, `/sys/kernel/security`, `sysctl` |
| D16 | ostree-extra-utils, `/opt/system` (P2) | high | `/opt/system` and its subfolders (libraries, Python, `PATH`, systemd units) **writable** by a non-root user, directly or through kaiming | `stat` of the folders; search for non-root or writable entries (script 10) |
| D17 | binary hardening (P2) | high | **installed** setuid binaries and root services without PIE, `BIND_NOW`, canary, FORTIFY or CET | `file` and `readelf` on installed binaries (script 60) |
| D18 | grub2 2.12~rc1 (P2) | high | installed GRUB = 2.12~rc1, **without shim**; Secure Boot state; factory-restore entry without GRUB password | `dpkg-query`, `modinfo.sh`, EFI variables, reading `grub.cfg` (script 60) |
| D19 | aptdaemon `SetEnviron` (P2) | to be confirmed | the unchecked root D-Bus method is **absent** from the installed version | read the installed `aptdaemon/core.py`, without waking the service (script 10) |
| D20 | network-manager → dnsmasq (P2) | medium | the `dnsmasq` **daemon** is active and listening at boot | `systemctl is-enabled/is-active`, `ss -lntu` (script 60) |
| D21 | dpkg and security fixes (P2) | medium | root `pre-invoke` hook active; `dpkg`/`apt` diverted; installed versions lagging on security fixes | `/etc/dpkg/dpkg.cfg.d`, `dpkg-divert --list`, `dpkg-query` (script 50) |

## 3. Detailed procedures by cluster

> Result convention for each test: **[CONFIRMED]** (observable present), **[REFUTED]** (compensating
> control established — e.g. `kysec` actually blocks it), **[UNDETERMINED]** (not reproducible on the
> bench). The **proof** (capture, log, timestamp) is always recorded.

### 3.1 Local escalation via polkit — `kaiming` (D1, D2)
1. From `audituser`: `pkaction --action-id org.kylin.kaiming.mount --verbose` → record
   `implicit any/inactive/active`.
2. Observe the presence/absence of an authentication **prompt** via
   `pkcheck --action-id … --process $$` (without launching the actual privileged action).
3. Record the permissions of `/opt/kaiming/share/polkit-1/rules.d`, `/var/opt/kaiming/share/*`,
   `/usr/share/polkit-1/actions/org.kylin.kaiming.policy` (`stat`, `getfacl`).
4. Check whether `kysec` imposes a compensating restriction (`kysec-status`, attributes on `/usr/bin/mount`).
5. **Decisive observable**: implicit `yes` + no prompt = D1 **[CONFIRMED]**; `rules.d` directory writable
   by the user = D2 **[CONFIRMED]**.

### 3.2 Root D-Bus IPC with no control (D3, D4, D14)
1. `busctl --system list` then `busctl introspect` of the relevant root services (ProcessManagerDaemon,
   `org.ukui.Service*`, `com.kylin.systemupgrade`, kysdk dispatchers).
2. Record the installed policy (`/usr/share/dbus-1/system.d/*.conf`): effective `context`.
3. From `audituser`, call a **read-only / harmless** method (e.g. an info method) and note whether the
   call **succeeds** with no check. Do **not** call the dangerous primitives (injection, `.so` loading,
   mounting); for D4, limit yourself to loading an **inert diagnostic plug-in** that logs its own loading.
4. For D14: check whether a patched `dbus-daemon`/kysec module enforces the `.limit` files (otherwise the
   `context=default` policy really governs access). Record `/etc/dbus-1/conf/*.limit` and whether it is
   honoured.

### 3.3 Root AI service `kytensor` (D5)
1. `systemctl show kytensor.service -p User` → confirm it runs as root.
2. `ss -ltnp` → confirm local listening on 8000/8001.
3. **Harmless** `GET /v2/health` request from `audituser` → the absence of authentication is the
   observable. **Do not** submit a model with an executable backend.

### 3.4 Telemetry and TLS (D6, D7, D8, D9)
1. On **first boot**: check whether a **consent** step (OOBE) is presented.
2. Capture the `kylin-daq` flows: confirm the destination
   (`datareporting.openkylin.top:30005`), and — via `mitmproxy` with no trusted certificate installed in
   the target — confirm that the flow is **still intercepted** (⇒ `VerifyNone`, D6 **[CONFIRMED]**).
3. Record the headers (`X-Uid`, `X-Service-Serial-No`, `X-Os-Serial-Number`…) and the actual message
   **content**; compare against the privacy policy (`05-donnees.md`).
4. D7: have the sandbox answer with a modified `bo` object → check the **persistent merge** into
   `kylinDaqConf.json`.
5. D8: trigger a simulated update → capture the **cleartext HTTP** to `archive1.kylinos.cn`.
6. D9: observe whether `libkysdk-location` still emits to `api.map.baidu.com` (code comment: "unused since
   3.0.1.0" — to be settled).
7. Settle the effect of `QSsl::TlsV1SslV3` (is negotiation really bounded to TLS 1.0?) via the observed
   TLS handshake.

### 3.5 OSTree / CA trust chain (D10)
1. Record `/etc/ostree/remotes.d/*.conf` on the immutable target: `gpg-verify`, URL.
2. Point the remote at the sandbox and serve an **unsigned test OSTree repository** (benign content):
   observe whether `ostree pull` / the updater **accepts** it with no signature error (D10 **[CONFIRMED]**
   if accepted). **Do not** deploy a malicious payload; the observable is acceptance of the unauthenticated
   commit.
3. Record the content and effect of `/opt/system/resource/ca-certificates` (CA added outside debconf).

### 3.6 Installer / compositor (D11, D12)
1. D11: install while successively passing `auditmode`, `ple-mode`, `noauditmode`, `auditmode=0` on the
   kernel command line; after install, check `/etc/sudoers.d/test`, autologin, and the **presence** of the
   `kylin-audit-mode` package (absent from the corpus); note whether the account persists.
2. Read the ISO's **GRUB/isolinux** entries (mounted read-only) to see whether a menu activates these
   modes (L9 blind spot).
3. D12: record the effective udev rule on `/dev/fb*` (`stat`); with a **test Wayland client**, attempt a
   capture of a **test screen** from another session/user → the observable is the absence of refusal.
   Also record the `security-context` state and the `screencopy`, `virtual-keyboard`, `data-control`
   protocols.

### 3.7 AI agents (D13)
1. `stat` the sockets under `/tmp/.kylin-ai-runtime-unix/<uid>/` → **effective** mode (the directory is
   `0777`; is the socket too?).
2. On first use of the assistant: capture the flow → confirm the **default** upload to the *freetrial*
   cloud service, the endpoint, the TLS, and the presence/absence of **consent**.
3. Confirm that the "hardcoded" RSA keys/passwords do decrypt the stored secrets (observation; values
   **masked** in the deliverables).

### 3.8 Kernel hardening (D15, per Phase 2)
1. Record `/boot/config-$(uname -r)`: state of the options flagged by the `P2-linux` batch (LOCKDOWN,
   MODULE_SIG, BPF_UNPRIV, STACKPROTECTOR, LEGACY_TIOCSTI, STRICT_DEVMEM…).
2. `sysctl -a` and `/sys/kernel/security/`: active LSMs, `lockdown`.
3. Compare against a reference Debian/Ubuntu kernel (expected values).

## 4. "Code ↔ distributed binary" verification

Compilation is **forbidden**; we therefore proceed by **concordance**, not reconstruction:
- fingerprints of installed binaries vs `.deb` packages vs source trees (already partly done in Phase 0);
- `strings`/`readelf`/symbols to find debug paths, versions and expected strings;
- for **source-less** components (AI plug-ins, etc.), the opacity is **accepted** and documented as a
  blind spot — Phase 5 does not lift this limitation.

## 5. Phase 5 deliverables

- `donnees/phase5/`: `pcap` captures, `mitmproxy` exports, timestamped `stat`/`busctl`/`journalctl`
  records, one JSON result sheet per test (`{id, hypothesis, result, proof, timestamp}`).
- Update of `02-findings.csv`: every tested finding receives a **dynamic register**
  (CONFIRMED/REFUTED/UNDETERMINED); settled `[PROBABLE]` entries switch accordingly.
- Update of `00-synthese.md`: consolidated verdict after the static ↔ dynamic confrontation.

## 6. Limitations of Phase 5 (explicit)

- **Virtual hardware**: the VM's DMI/serial numbers/MAC differ from a real machine; the telemetry observed
  may be **partial** or degraded (some hardware queries fail in a VM).
- **Simulated servers**: the network sandbox mimics the APIs; it does not reproduce the **real** behaviour
  of openKylin's servers (responses, server-side activation conditions).
- **Scope**: amd64 and the default configuration; the variants (arm64/loong64, Server, Embedded, AgentOS)
  and unusual install paths are not covered.
- **Opaque components**: source-less plug-ins/binaries remain unverifiable beyond their observable network
  behaviour.
- **Activation conditions**: a conditional behaviour (date, geography, remote server, specific hardware)
  may not trigger on the bench — absence of observation is not proof of absence (the audit's guiding
  principle).
