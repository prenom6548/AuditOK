# Observation kit — Phase 5

🇫🇷 [Version française (par défaut)](LISEZMOI.md) · 🇬🇧 English (this page)

This kit lets **anyone with an openKylin 3.0 virtual machine** check, on a real system, some of the
hypotheses the audit could only establish **in the code**. It complements
[`../04-protocole-dynamique.md`](../en/04-dynamic-protocol.md) (English version), which describes the
general method.

> **What this kit does: it observes and reads.** It records file permissions, the authorisation
> configuration declared by polkit and D-Bus, the update-repository configuration, and outbound network
> flows. **It executes no audited component, calls no privileged method, and contains no exploitation
> code.** It does not modify the system. Script comments and output are in French; this guide explains
> each one in English.

## This kit is the audit's Phase 5

The audit has no virtual machine available: its Phase 5 (validation on a real system) is therefore
**delivered as this kit**, run by the community. It is **updated with each wave** of the static analysis:
every new hypothesis that can be observed safely gets a check here, and the history below records what
changed.

| Version | Date | Content |
|---|---|---|
| 1 | 2026-10-01 | Hypotheses D1 to D15 (Phases 1 and 2, waves 1–2) |
| 2 | 2026-10-01 | Hypotheses D16 to D21 (end of Phase 2): `/opt/system` trust root, hardening of installed binaries, GRUB and Secure Boot, aptdaemon `SetEnviron`, `dnsmasq`, dpkg hooks and diversions; new script 60 |

## Who is this for?

Anyone in the community who already runs, or knows how to install, openKylin in a virtual machine. No
security expertise is required to **run** the kit; **interpreting** the results takes a bit more
familiarity — hence the option to send them back (see *What next?*).

## Golden rule: always in a disposable VM

- Run this kit **only inside a dedicated virtual machine**, never on your main computer.
- Take a **snapshot** of the VM before you start: you can roll back.
- For the network observation (script 40), put the VM on an **isolated network** ("host-only" or NAT with
  no useful route), so that no data leaves it towards real servers.

On Linux, **GNOME Boxes** or **Virtual Machine Manager** (QEMU/KVM, free software) are enough. The
official openKylin 3.0 install image is downloaded from the project's mirrors; verify its fingerprint
before installing.

## The scripts

| Script | Observes | Privileges |
|---|---|---|
| `00-collecte-environnement.sh` | System version, kernel hardening options, active LSMs, running services | user |
| `10-permissions-fichiers.sh` | Permissions of directories/sockets cited by the audit (writable? open?), including `/opt/system`; presence of `SetEnviron` in aptdaemon | user |
| `20-introspection-dbus.sh` | D-Bus services present, `context` value of their policies, exposed methods (without calling them) | user |
| `30-polkit-actions.sh` | Declared authorisation policy of the kaiming actions; is authentication requested? | user |
| `50-depots-et-maj.sh` | Repository transport, `Signed-By`, `Valid-Until`, APT options, OSTree `gpg-verify`, dpkg/apt hooks and diversions, installed versions | user |
| `60-binaires-et-amorcage.sh` | Protections of setuid binaries and root services (PIE, RELRO, canary, FORTIFY, CET), GRUB version, shim, Secure Boot, GRUB password, `dnsmasq` | user |
| `40-observation-reseau.sh` | Outbound flows (telemetry): destinations, ports, volume | **root**, isolated VM |
| `lancer-tout.sh` | Runs all the "user" scripts at once and prepares an archive | user |

## How to use it (5 steps)

1. **Install openKylin 3.0** in a VM, default configuration. Create a **regular user account** (no admin
   rights) for the observations.
2. **Take a snapshot** of the VM.
3. **Copy this folder** `phase5-kit/` into the VM.
4. Open a terminal **as the regular user** in this folder and run `bash lancer-tout.sh`.
   If a password prompt opens during script 30, **cancel it**: the fact that it opens is already the
   sought-after result ("authentication required").
5. *(Optional)* For telemetry, put the VM on an **isolated network**, install `tcpdump`, then run
   `sudo bash 40-observation-reseau.sh 120`. During the capture, use the system normally (open the control
   panel, run an update) to trigger any sends.

Results are written to `resultats/`, and `lancer-tout.sh` packs them into a `resultats-phase5.tar.gz`
archive.

## What each result settles

Hypotheses refer to the matrix in [`../en/04-dynamic-protocol.md`](../en/04-dynamic-protocol.md) § 2.

| Result file | Hypotheses | What is settled |
|---|---|---|
| `00-environnement.txt` | D15 | Real hardening of the installed kernel; presence of `kysec` |
| `10-permissions.txt` | D2, D12, D13, D16, D19 | Are kaiming's `rules.d` directories really writable? mode of the AI sockets? `/dev/fb*` at 0666? `test` account? |
| `20-dbus.txt` | D3, D14 | Are the root services really `context=default`? which methods are exposed? |
| `30-polkit.txt` | D1 | Do the kaiming actions run without an authentication prompt? |
| `50-depots.txt` | D10, D21 | APT repositories with no `Valid-Until`/`Signed-By`? OSTree remotes with `gpg-verify=false`? dpkg/apt diverted? versions lagging on security fixes? |
| `60-binaires-amorcage.txt` | D17, D18, D20 | setuid binaries and root services without protections? GRUB 2.12~rc1 without shim? Secure Boot enabled? `dnsmasq` listening? |
| `40-reseau.txt` | D6, D8, D9 | Which destinations does telemetry contact, and over which transport? |

Each hypothesis then becomes **CONFIRMED**, **REFUTED** or **UNDETERMINED** on a real system, to be
reported in `../02-findings.csv` (verification column) and `../07-verification.md`.

## What this kit does not do

- It does not confirm end-to-end **exploitation**: it establishes the observable **preconditions** (absence
  of control, open permissions, unverified transport). This is deliberate and sufficient to qualify the
  risk, without producing an offensive tool.
- It does not cover everything: the installer and the compositor (hypotheses D11, D12) require manual
  trials described in the protocol, not scripted here.

## What next?

Drop the `resultats-phase5.tar.gz` archive wherever the community can pick it up (for example attached to
an issue, or in a `resultats/` folder of the repository). The outputs contain only configuration
metadata; **review them** before publishing to make sure no personal information from your VM is in
there. Anyone comfortable with these topics can then turn these observations into verdicts in the report.
