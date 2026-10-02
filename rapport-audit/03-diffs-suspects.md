# 03 — Différentiel des forks amont : correctifs et hunks suspects

> **Portée.** Delta openKylin des forks de logiciels critiques (Phase 2), plus la chaîne de
> confiance OSTree / ca-certificates différenciée en vague 1. Pour un paquet non natif, l'amont
> `.orig` est identique à Debian/Ubuntu : le delta est dans `debian/` (série quilt, règles, scripts
> de mainteneur). « Suspect » = écarte la posture de sécurité de l'amont ; **ne signifie pas
> malveillant**. Registres [CONSTATÉ]/[PROBABLE]. Fiches complètes : `donnees/phase2/`.

## 1. Synthèse par fork

| Fork | Lot | Unités | SUSPECT | À EXAMINER | LÉGITIME | Titres inexacts | Constats ≥ moyen |
|---|---|---|---|---|---|---|---|
| ostree | V1-L10-confiance-ostree-ca | 473 hunks | 42 | 95 | 336 | — | 6 |
| ca-certificates | V1-L10-confiance-ostree-ca | 11 hunks | 2 | 7 | 2 | — | 1 |
| apt | P2-apt | 6 correctifs | 0 | 2 | 4 | 0 | 2 |
| pam, shadow, lightdm | P2-auth-login | 67 correctifs | 4 | 4 | 59 | 4 | 2 |
| ostree-extra-utils, dpkg-wrapper | P2-binaires-sans-source | 0 correctifs | 0 | 0 | 0 | 0 | 5 |
| coreutils | P2-coreutils | 12 correctifs | 0 | 5 | 7 | 5 | 1 |
| gnutls28, nss, libgcrypt20, gnupg2, glib-networking, curl | P2-crypto-libs | 7 correctifs | 0 | 2 | 5 | 1 | 1 |
| dpkg, dpkg-wrapper | P2-dpkg | 1 correctifs | 1 | 0 | 0 | 1 | 4 |
| bash, biometric-auth, coreutils, curl, dbus-daemon, dpkg, kylin-ai-runtime, kylin-daq, kylin-nm, kylin-os-manager, kylin-wayland-compositor, libc-bin, libkysdk-system, lightdm, login, mount, network-manager, openssh-server, openssl, passwd, peony, pkexec, polkitd, sudo, systemd, ukui-control-center, ukui-panel, ukui-screensaver, ukui-session-manager, ukui-settings-daemon, util-linux, wpasupplicant, xserver-xorg-core | P2-durcissement-binaires | 0 correctifs | 0 | 0 | 0 | 0 | 2 |
| glibc | P2-glibc | 86 correctifs | 0 | 2 | 84 | 0 | 1 |
| grub2, grub2-signed | P2-grub2 | 133 correctifs | 10 | 5 | 118 | 10 | 3 |
| grub2 | P2-grub2-amont | 0 correctifs | 0 | 0 | 0 | 0 | 2 |
| coreutils (binaire 9.4-ok4) | P2-index-sources | 0 correctifs | 0 | 0 | 0 | 0 | 1 |
| linux | P2-linux | 8 correctifs | 3 | 2 | 3 | 5 | 9 |
| openssh | P2-openssh | 35 correctifs | 0 | 10 | 25 | 0 | 1 |
| openssl, openssl1 | P2-openssl | 59 correctifs | 7 | 2 | 50 | 7 | 1 |
| aptdaemon, apport | P2-outils-ubuntu | 7 correctifs | 0 | 3 | 4 | 2 | 3 |
| policykit-1 | P2-policykit | 2 correctifs | 1 | 0 | 1 | 1 | 5 |
| network-manager, wpa | P2-reseau | 33 correctifs | 0 | 2 | 31 | 1 | 2 |
| sudo | P2-sudo | 8 correctifs | 0 | 0 | 8 | 0 | 0 |
| sudo-common | P2-sudo-common | 0 correctifs | 0 | 0 | 0 | 0 | 0 |
| systemd | P2-systemd | 39 correctifs | 1 | 8 | 30 | 5 | 0 |

## 2. Titres de correctifs inexacts (42)

Un correctif dont le **titre ne décrit pas le contenu** empêche le tri par titre : chaque
correctif doit être lu. Ce motif est transversal à openKylin (voir aussi `policykit-1`).

| Lot | Correctif | Titre déclaré | Contenu réel | Classement |
|---|---|---|---|---|
| P2-auth-login | 0009-15-update-changelog.patch | !15 update changelog | 1673 insertions sur 28 fichiers C: integration UKUI/Wayland, PAM_BINARY_PROMPT (biometrie/OTP), secondes-login (autologin-user-lock), detection materielle (dmidecode/Kirin/Pangu/HW-cloud), popen ps\|grep root, retrait du garde allow_greeter, defaut greeter-hide […] | [SUSPECT] |
| P2-auth-login | 0011-17-update-changelog.patch | !17 update changelog | corrections C (cast SESSION(), int p=0) dans seat.c/session.c | [SUSPECT] |
| P2-auth-login | 0013-23-update-changelog.patch | !23 update changelog | modifie les PAM lightdm/lightdm-autologin: ajoute user_readenv=1 et 'auth sufficient pam_succeed_if user ingroup nopasswdlogin'; retire doublon SIGPIPE | [SUSPECT] |
| P2-auth-login | 0014-24-update-change-log.patch | !24 update change log | 183 insertions dans src/dm-tool.c: resolution du chemin de seat DisplayManager via logind (GetSession/ListSessions) | [SUSPECT] |
| P2-coreutils | 0008-.patch | 同步上游 (« synchronisation avec l'amont ») — 武丹 <wudan@kylinos.cn>, 17/04/2026 18:40 | m4/gnulib-comp.m4 : retire tests/test-readutmp.c de gl_FILE_LIST (-1 ligne). | [À EXAMINER] |
| P2-coreutils | 0009-.patch | 同步上游 — 武丹 <wudan@kylinos.cn>, 17/04/2026 18:56 | gnulib-tests/Makefile.in (fichier généré) : retire test-readutmp de TESTS et check_PROGRAMS ainsi que sa règle d'édition de liens. Supprime en outre 148 règles .log de tests, les règles génériques .test.log et la cible distdir (+2/-1061). | [À EXAMINER] |
| P2-coreutils | 0010-.patch | 同步上游 — 武丹 <wudan@kylinos.cn>, 17/04/2026 19:15 | gnulib-tests/Makefile.in : retire les dernières références à test_readutmp (SOURCES, OBJECTS, DEPENDENCIES, LDADD, .Po, DIST_SOURCES, EXTRA_DIST, distclean). | [À EXAMINER] |
| P2-coreutils | 0011-.patch | 同步上游 — 武丹 <wudan@kylinos.cn>, 17/04/2026 19:23 | gnulib-tests/gnulib.mk : commente le module readutmp-tests (TESTS, check_PROGRAMS, LDADD, EXTRA_DIST). | [À EXAMINER] |
| P2-coreutils | 0012-.patch | 同步上游 — 武丹 <wudan@kylinos.cn>, 17/04/2026 19:39 | gnulib-tests/gnulib.mk : retire test-localtime_r et test-localtime_r-mt de TESTS et check_PROGRAMS. tests/local.mk : retire tests/tail/inotify-dir-recreate.sh. | [À EXAMINER] |
| P2-crypto-libs | 0003-Description-Backport-of-status-request-revoked.c-fro.patch | Description: Backport of status-request-revoked.c from groovy (Origin: upstream, Applied-Upstream ed208fe55f31...) | Ajoute 'skip_if_no_datefudge' au debut de tests/testpkcs11.sh (2 lignes). Aucun fichier status-request-revoked.c, aucun code de bibliotheque. | [À EXAMINER] |
| P2-dpkg | feat_openKylin.patch | Subject: <short summary of the patch> (modèle dpkg-source --commit non rempli ; description = entrée changelog 1.23.7-ok […] | Crée scripts/Dpkg/Vendor/openKylin.pm (544 lignes) : classe fournisseur openKylin héritant de Default ; crochets archive-keyrings (trousseau Debian), builtin-build-depends, extend-patch-header (Bug-Debian), update-buildprofiles (noudeb) et update-buildflags av […] | [SUSPECT] |
| P2-grub2 | 0094-changelog-2.12-ok1.4.patch | changelog: 2.12-ok1.4 | Ajoute un module linuxefi (ancien chargeur Fedora/Ubuntu 2.02, 383 lignes : vérification de la signature du noyau par shim_lock puis RELECTURE du fichier après vérification), grub_efi_allocate_pages_max, et un détournement de la commande linux i386 vers linuxe […] | [SUSPECT] |
| P2-grub2 | 0095-changelog-2.12-ok1.5.patch | changelog: 2.12-ok1.5 | (1) Annule exactement 0094 ; (2) remplace la logique de 10_linux/20_linux_xen/grub-mkconfig_lib par une variante Kylin plus ancienne : version_find_latest/dpkg au lieu du tri, suppression de .old et GRUB_TOP_LEVEL, noyaux *.efi.signed, ajout automatique de res […] | [SUSPECT] |
| P2-grub2 | 0096-changelog-2.12-ok1.8.patch | changelog: 2.12-ok1.8 | grub-mkconfig_lib.in : en chroot avec /ostree, utilise /usr/sbin/grub-ostree-mkrelpath pour les chemins relatifs. Désactivé ensuite par 0132. | [SUSPECT] |
| P2-grub2 | 0097-changelog-2.12-ok1.9.patch | changelog:2.12-ok1.9 | Nouveau module tpcm_hygon (x86_64-efi) : vérificateur GRUB (grub_verifier_register) qui transmet chaque fichier ouvert, la ligne de commande noyau et chaque commande GRUB au protocole micrologiciel Hygon C2P (TPCM) ; ouverture par défaut si protocole absent ou […] | [SUSPECT] |
| P2-grub2 | 0100-changelog-2.12-ok1.12.patch | changelog:2.12-ok1.12 | Première version du chargeur LoongArch « back-compatibility » (ELF + EFI, relocator, BPI) avec modifications de kern/efi/mm.c touchant aussi les autres architectures (GRUB_EFI_MAX_ALLOCATION_ADDRESS). Intégralement annulé par 0101 (vérifié), remplacé par 0103. | [SUSPECT] |
| P2-grub2 | 0102-changelog-2.12-ok1.13.patch | changelog: 2.12-ok1.13 | grub-install.c : plate-forme par défaut loongarch64-efi, fichier BOOTLOONGARCH64.EFI, recherche des images dans le répertoire -signed pour loongarch64-efi. | [SUSPECT] |
| P2-grub2 | 0130-25-changelog-2.12-ok1.16.patch | !25 changelog:2.12-ok1.16 Merge pull request !25 from niubix/openkylin/nile-sp2 | 00_header.in : insère des marqueurs « ###grub-password begin/end » (utilisés par debian/grub-password pour superusers/password_pbkdf2) ; sur aarch64 seulement, en-tête réduit (grubenv, initrdfail, défaut, timeout, recordfail, load_video) et le reste de l'en-tê […] | [SUSPECT] |
| P2-grub2 | 0131-26-changelog-2.12-ok1.17.patch | !26 changelog:2.12-ok1.17 Merge pull request !26 from niubix/openkylin/nile-sp2 | Traductions uniquement : supprime po/ar.po (ajouté par 0105), met à jour po/kk.po et po/vi.po ; marque « fuzzy » les traductions vietnamiennes dont les spécificateurs printf étaient incohérents (état final vérifié : aucune entrée non-fuzzy incohérente). | [À EXAMINER] |
| P2-grub2 | 0132-27-changelog-2.12-ok1.19.patch | !27 changelog:2.12-ok1.19 Merge pull request !27 from niubix/openkylin/nile-sp2 | grub-mkconfig_lib.in : commente la branche ostree introduite par 0096 (retour à grub-mkrelpath). | [SUSPECT] |
| P2-grub2 | 0133-30-changelog-2.12-ok1.20.patch | !30 changelog:2.12-ok1.20 Merge pull request !30 from niubix/openkylin/nile-sp2 | 30_uefi-firmware.in : « exit 0 » en tête — supprime l'entrée de menu « UEFI Firmware Settings ». | [SUSPECT] |
| P2-linux | 0001-update-packaging-config.patch | update packaging config | Crée debian.master/ (annotations de 9 342 lignes compactée sans notes, control.stub.in, rules.d/{amd64,arm64,loong64,riscv64}.mk avec do_skip_checks=true et uefi_signed=true, vars.*, reconstruct, modprobe.d) ET modifie du code noyau : binder (txn_security_ctx  […] | [SUSPECT] |
| P2-linux | 0002-build-build-linux-7.0.0-2.patch | build: build linux 7.0.0-2 | ≈252 000 lignes, 522 fichiers : support complet SoC Phytium (bus PIO, GPIO, I2C, I3C, SPI/QSPI, MMC, UART, CAN, MAC, DMA, PWM, hwmon, IPMI BT/KCS, snoop, devfreq, PMU DDR, HDA/I2S/codecs, USB, TCM SE), pilote GPU Glenfly « arise » (≈285 fichiers), pilote SCSI  […] | [À EXAMINER] |
| P2-linux | 0004-fix-fix-loongarch-and-riscv64-build-error.patch | fix: fix loongarch and riscv64 build error | Makefile (mrproper ne supprime plus debian/), dépendances de build loong64, perf (cible rust loongarch, unwind-libdw) ; MAIS aussi arch/loongarch/kernel/extern.c : 4 EXPORT_SYMBOL_GPL de primitives de signal, et ptrace.c/signal.c : gpr_get, gpr_set, read_user, […] | [À EXAMINER] |
| P2-linux | 0007-build-loongarch-ext4.patch | build: 更新loongarch编译包依赖; 修复集成系统无ext4格式问题 (mise à jour des dépendances de build loongarch ; correction de l'absence du fo […] | Remplace la configuration loong64 par un .config complet importé d'un environnement Kylin « ky11 » (CC_VERSION_TEXT) : 5 843 réglages loong64 modifiés. EXT4_FS=y ajouté, mais aussi désactivation de lockdown, MODULE_SIG, IMA/EVM, trousseaux, FORTIFY, HARDENED_U […] | [SUSPECT] |
| P2-linux | 0008-build-new-version-7.0.0-3.patch | build: new version 7.0.0-3 (corps : 1. support des fonctions de sécurité Hygon ; 2. correctif son D3000M) | ≈15 500 lignes : pilotes Hygon CCP (SM2/SM3/SM4, sm4-xts/gcm), ccp-mdev (/dev/ccp_share 0666, /dev/ccpN), hct.ko (/dev/hct_noiommu, /dev/hct_mig 0666, serveur vsock invité 12345), PSP Hygon (/dev/hygon_psp_config, verrou partagé avec l'espace utilisateur dans  […] | [SUSPECT] |
| P2-openssl | openssl/update-changelog-information.patch | update changelog information | Corrige un débordement d'entier dans buf2hexstr (CVE-2026-31789) et remplace OPENSSL_free par X509_free dans dane_match_cert (CVE-2026-28387). | [SUSPECT] |
| P2-openssl | openssl/update-changelog-information-1.patch | update changelog information | CVE-2026-28388 (delta CRL sans crl_number), CVE-2026-28389 (CMS DH/ECDH via X509_ALGOR_get0), CVE-2026-28390 (OAEP label CMS), CVE-2026-31790 (RSASVE vérifie le retour) + certificats/CRL et tests. | [SUSPECT] |
| P2-openssl | openssl/update-changelog-info.patch | update changelog info | 15 correctifs CVE (changelog ok6) : ASN.1 (a_mbstr, tasn_dec), CMP genm, CMS enc/env/pwri, CRMF, PKCS12 PBMAC1 (clé >= 20 octets), PKCS7 (suppression de la désactivation du rejet implicite RSA, UAF PKCS7_verify), AES-GCM-SIV/OCB/SIV, DH (q du pair), QUIC PATH_ […] | [SUSPECT] |
| P2-openssl | openssl/add-fix-cve-infos-of-changelog.patch | add fix cve infos of changelog | Limite de connexions QUIC en attente (256, nouvelle API SSL_VALUE_QUIC_MAX_PENDING_CONNS) ; vérification du tag à Final pour AES-GCM-SIV, OCB et ChaCha20-Poly1305 (EVP_Cipher). | [SUSPECT] |
| P2-openssl | openssl/update-changelog-information-2.patch | update changelog information | 8 correctifs CVE (changelog ok8) : CMP format string ("%s"), cache extraCerts, protectionAlg ; CMS kari (taille du tampon d'unwrap) ; QUIC (refcount qrx, paquets ACK seuls) ; DTLS (file limitée à 16, copie du seul enregistrement) ; t1_lib check_cert_usable ave […] | [SUSPECT] |
| P2-openssl | openssl1/4-update-version-number-and-commit-to-trigger-a-new-build.patch | !4 update version number and commit to trigger a new build job | Correctif de code CVE-2023-0465 dans check_policy() (x509_vfy.c) : boucle depuis i=0 et indicateur cbcalled. | [SUSPECT] |
| P2-openssl | openssl1/add-loong64-support.patch | add loong64 support | Cibles linux64-loong64/debian-loong64, plus CHANGES/NEWS CVE-2023-0465, doc CVE-2023-0466 (X509_VERIFY_PARAM_add0_policy), certificats et tests de politique. | [SUSPECT] |
| P2-outils-ubuntu | 0001-update-new-version.patch | update new version | Réapplique 6 correctifs Ubuntu non intégrés à l'orig (test-with-timeout, fix-some-deprecated-code, gdk-gtk-versions.py, drop-function-not-in-3.11, setup-py-import-distutils-extra, pep8-to-pycodestyle) et annule 4 modifications Kylin de l'orig : suppression de  […] | [À EXAMINER] |
| P2-outils-ubuntu | 0005-1-openkylin.patch | !1 判断操作系统时增加对openkylin的判断 Merge pull request !1 (traduction : « ajouter openkylin lors de la détermination du système d' […] | Remplace la condition de détection de dpkg par une expression toujours vraie ("debian" or "openkylin" in ids) et supprime 3 tests (test_add_gdb_info_damaged, test_add_gdb_info_exe_missing, test_ui_update_view). | [À EXAMINER] |
| P2-policykit | 12-update-changelog.patch | !12 update changelog | Ne modifie aucun changelog. Injecte 145 lignes de C dans 4 fichiers du coeur : (1) polkitagenthelper-pam.c accepte PAM_NEW_AUTHTOK_REQD apres pam_acct_mgmt ; (2) polkitbackendactionpool.c ajoute collect_directories_from_xdg_data_dirs (repertoires actions via X […] | [SUSPECT] |
| P2-reseau | network-manager:Translated-using-Weblate-Arabic.patch | Translated using Weblate (Arabic) | Crée po/ar.po (14 574 l., 90,9 %) ET po/ko_KP.po (13 176 l., gabarit entièrement non traduit, en-tête zh_CN). | [À EXAMINER] |
| P2-systemd | Translated-using-Weblate-Chinese-Traditional.patch | Translated using Weblate (Chinese (Traditional)) | Touche 20 fichiers po (19 langues autres que zh_TW) : retire 668 drapeaux « #, fuzzy », réécrit les en-têtes (Last-Translator KevinDuan, Weblate openKylin), modifie 12 msgstr. Active des traductions inexactes de descriptions polkit. Aucun msgid modifié, aucune […] | [À EXAMINER] |
| P2-systemd | Added-translation-using-Weblate-khb_CN.patch | Added translation using Weblate (傣仂文 (khb_CN)) | Crée po/khb_CN.po ET po/ms.po, po/th.po, po/vi.po (gabarits vides). | [À EXAMINER] |
| P2-systemd | Added-translation-using-Weblate-ko_KP-1.patch | Added translation using Weblate (朝鲜语 (ko_KP)) | ANNULE les 264 traductions ko_KP (retour au gabarit vide, blob 90656e7 -> f4bf15f). | [À EXAMINER] |
| P2-systemd | 63-fix-install-systemd-pam-config-to-etc-pam-configs-to-m.patch | !63 fix: install systemd pam config to /etc/pam-configs to match pam 1.7.0-ok3 | Uniquement po/ko_KP.po : 102 msgstr coréens (polkit, pam_systemd_home). Aucun fichier PAM, aucun code. | [SUSPECT] |
| P2-systemd | 64-rmove-arm-some-test-for-compile-loongarch-add-pie-for-.patch | !64 rmove arm some test for compile, loongarch add pie for fix MemoryDenyW… | Commente 11 tests unitaires dans 4 meson.build, pour toutes les architectures ; ne contient pas la partie PIE loongarch (dans debian/rules). | [À EXAMINER] |

## 3. Constats ≥ moyen issus du différentiel (55)

### [ELEVEE] `aptdaemon` — archive/aptdaemon/debian/changelog (l. 7-41)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `aptdaemon (1.1.1+bzr982-ok6) nile; * Update new version. -- 25 Apr 2024 16:46:28 / ... aptdaemon (1.1.1+bzr982-ok1) yangtze; * Sync kylin-desktop -- 14 Aug 2022`
- **Constat** : Le correctif qui retire SetEnviron (0001-update-new-version.patch, en-tête Date: Thu, 25 Apr 2024 16:46:58) est apparu en ok6 (2024-04-25). Les membres de l'orig.tar.xz sont datés 2022-08-14 11:57 (= ok1 'Sync kylin-desktop'), et ok4 indique seulement 'Update orig.tar format'. Les versions ok1 à ok5 (yangtze, openKylin 1.x, 2022-08 → 2024-01) ont donc très probablement été construites depuis cet orig SANS le correctif, c'est-à-dire avec la méthode D-Bus root non authentifiée SetEnviron, les protections de suppression des paquets essentiels désactivées et allow_downgrade=True. Ce constat porte sur des versions antérieures à 3.0 (la 3.0 livre ok7, neutralisée).
- **Scénario** : Sur un poste openKylin 1.x équipé d'aptdaemon ok1–ok5 : tout utilisateur local appelle SetEnviron sur le bus système puis déclenche une opération aptd ; les processus root enfants héritent de l'environnement contrôlé (élévation locale). Aucun code d'exploitation n'est fourni.
- **Contre-lecture** : Les debian/ et binaires ok1–ok5 ne sont pas disponibles : ces versions ont pu porter un autre correctif retirant SetEnviron, ou un orig différent. Indice manquant : debian/patches et paquets binaires ok1–ok5 (archives openKylin yangtze).

### [ELEVEE] `chaîne de construction openKylin (18 paquets comparés à Ubuntu)` — donnees/phase2/durcissement-binaires.json (clés compares, openkylin_seuls, synthese) (l. —)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `exécutables PIE : openKylin 16/234, Ubuntu 234/234 ; BIND_NOW : 127/262 contre 251/262 ; FORTIFY : 14/262 contre 226/262 ; CET : 218/262 contre 262/262 ; composants propres openKylin : 0/52 exécutables PIE`
- **Constat** : Régression systémique du durcissement des binaires distribués, mesurée sur 18 paquets fondamentaux comparés aux mêmes paquets Ubuntu (resolute) et sur 14 composants propres à openKylin. Concerne notamment des services root ou exposés : sshd (ni PIE ni CET ; openssh-server n'est toutefois pas installé par défaut), dbus-daemon du bus système (ni PIE, ni protecteur de pile, ni FORTIFY, ni CET), polkitd et pkexec, NetworkManager, wpa_supplicant, systemd, Xorg, curl. Seuls sudo et libc-bin, dont le système de construction amont force ces options, gardent le PIE. Cause : un défaut de dpkg est établi dans le code (P2-dpkg) — la politique de durcissement de dpkg-buildflags n'est jamais chargée sur openKylin (module Dpkg::Vendor::openKylin introuvable par Dpkg::Vendor, orig scripts/Dpkg/Vendor.pm:172-201 ; origins/openkylin sans Parent) ; l'absence générale de PIE indique en outre que le compilat […]
- **Scénario** : Toute vulnérabilité mémoire découverte dans l'un de ces composants (par un attaquant local pour dbus-daemon, polkitd ou pkexec ; à distance pour sshd lorsqu'il est installé) est nettement plus simple à exploiter de manière fiable sur openKylin que sur Ubuntu, les défenses en profondeur standard étant absentes.
- **Contre-lecture** : Il ne s'agit pas d'une porte dérobée ni d'une faille en soi ; l'échantillon (32 paquets) ne couvre pas toute la distribution, et le protecteur de pile reste majoritairement présent (défaut du compilateur). Aucun indice d'intention : la cause la plus plausible est une erreur de configuration de la chaîne de construction (nom de vendor), non détectée faute de tests.

### [ELEVEE] `grub2` — debian/patches/series (l. 1-133)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `Aucun correctif ntfs/CVE dans la série. orig : configure.ac:37 « AC_INIT([GRUB],[2.12~rc1],…) » ; grub-core/fs/ntfs.c:388 « grub_memcpy (dest, pa + u32at (pa, 0x14) + ofs, len); » sans contrôle de bornes ; debian/build-efi-images:129 « ntfs ».`
- **Constat** : L'archive amont livrée sous le nom grub2_2.12.orig.tar.xz est en réalité GRUB 2.12~rc1 (juillet 2023). Les correctifs NTFS CVE-2023-4692 (écriture hors limites, $ATTRIBUTE_LIST) et CVE-2023-4693 (lecture hors limites d'attribut résident), présents dans l'orig 2.12 d'Ubuntu (ntfs.c : first_attr_off/res_attr_data_len, contrôles pa_end) et portés en correctifs par Ubuntu sur sa base rc1 (changelog Ubuntu : d/p/ntfs-cve-fixes/*), sont absents de l'orig openKylin et aucun correctif debian/ ne les apporte. Le module ntfs est embarqué dans les trois images EFI monolithiques destinées à la signature (gcd, grub, grubnet). Portée : génération SBAT réelle < 4. Même fait que le constat « élevée » de P2-grub2-amont (orig/grub-core/fs/ntfs.c), qui détaille l'écart amont ; le présent constat porte sur l'angle debian/ : aucun correctif compensatoire dans la série et ntfs embarqué dans les images à signe […]
- **Scénario** : Un attaquant disposant d'un accès physique (clé USB) ou des droits root sur une machine en Secure Boot présente à GRUB un système de fichiers NTFS forgé (partition ou image) ; l'écriture hors limites dans le tas de GRUB peut permettre l'exécution de code dans le chargeur signé, donc le contournement de la chaîne Secure Boot (persistance pré-OS). Exploitabilité non démontrée ici (PROBABLE).
- **Contre-lecture** : Sur l'ISO 3.0, GRUB est lancé directement (pas de shim) et la valeur réelle dépend de l'activation de Secure Boot avec une clé openKylin/Kylin en db ; sans Secure Boot, un attaquant physique a d'autres moyens. GRUB ne lit un NTFS que s'il le parcourt (search, ls, os-prober/entrées Windows, chainloader).

### [ELEVEE] `grub2` — debian/patches/series (l. 1-133)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `Série openKylin sans aucun « cve-2025-jan/* » ; référence ubuntu/grub2_2.12-1ubuntu7.3/deb/patches/series:116+ (71 correctifs cve-2025-jan, p. ex. fs-ufs-Fix-a-heap-OOB-write.patch) ; changelog Ubuntu 7.3:15-55 (CVE-2024-45774…CVE-2025-1125).`
- **Constat** : Les 21 CVE GRUB publiées en février 2025 (CVE-2024-45774 à 45783, CVE-2025-0622, 0624, 0677, 0678, 0684, 0685, 0686, 0689, 0690, 1118, 1125 : débordements dans ufs/hfs/hfs+/squash4/jfs/ext4/jpeg, UAF, net OOB write, désactivation de modules sous lockdown) sont corrigées par Ubuntu noble-updates 2.12-1ubuntu7.3 (mars 2025, SBAT grub,5). openKylin 2.12-ok1.21 (avril 2026) n'en contient aucune : ni correctif debian/ ni présence dans l'orig rc1 (vérifié : aucun « grub_is_lockdown » dans grub-core/fs/ hors hfs.c). La référence la plus proche (noble 2.12-1ubuntu7, avril 2024) est antérieure à ces CVE, ce n'est donc pas un retrait mais une absence de suivi de sécurité. Le debian/changelog openKylin ne mentionne aucune CVE.
- **Scénario** : Comme ci-dessus : système de fichiers ou image forgée (p. ex. squash4, ufs, hfs+, jpeg de thème) lu par GRUB, ou réponse réseau forgée en amorçage réseau (CVE-2025-0624, écriture hors limites dans la recherche de configuration réseau) — corruption mémoire dans GRUB, potentiellement contournement Secure Boot. PROBABLE (non démontré).
- **Contre-lecture** : Une partie de ces CVE concerne des modules non embarqués dans les images signées et non chargeables sous Secure Boot ; sans Secure Boot, l'impact est limité à un attaquant déjà privilégié ou physique. openKylin peut corriger hors de ce dépôt (non observé dans les sources auditées).

### [ELEVEE] `grub2` — orig/grub-core/fs/ntfs.c (l. 225-245, 386-388, 575, 811, 1159-1168)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `233: grub_set_unaligned32 ((char *) pa + 0x10, … (sans contrôle pa_end) \| 386-388: if (ofs + len > u32at (pa, 0x10)) … grub_memcpy (dest, pa + u32at (pa, 0x14) + ofs, len); \| 1167: *label = get_utf8 (pa, len);`
- **Constat** : Le pilote NTFS de l'orig (2.12~rc1) ne contient pas les 6 correctifs amont du 2023-10-03 présents dans GNU 2.12 et chez Ubuntu/Debian : « Fix an OOB write when parsing the $ATTRIBUTE_LIST attribute for the $MFT file » (CVE-2023-4692, écriture hors limites dans le tas, lignes 225-245 : aucune vérification pa_end avant grub_set_unaligned32), « Fix an OOB read when reading data from the resident $DATA attribute » (CVE-2023-4693, lignes 386-388 : seules ofs+len sont bornées, ni la taille ni le décalage de l'attribut résident par rapport au tampon MFT), et trois lectures hors limites (étiquette de volume l.1159-1168, bitmap d'index résident l.811, entrées de répertoire list_file l.575 sans end_pos). Aucun des 133 correctifs de debian/patches/series ne touche grub-core/fs/ntfs.c (vérifié par extraction des en-têtes « +++ » et recherche « ntfs » : seules des traductions et tests/ntfs_test.in).  […]
- **Scénario** : Un attaquant ayant un accès physique (clé USB contenant un volume NTFS forgé) ou déjà root (écriture d'une partition/d'une image NTFS forgée) fait analyser ce volume par GRUB au démarrage — sondage automatique des périphériques par « search », entrée Windows générée par os-prober, ou accès explicite — et déclenche le débordement de tas CVE-2023-4692 pour exécuter du code dans GRUB avant le noyau : contournement de la chaîne Secure Boot (bootkit persistant, désactivation du lockdown) ; CVE-2023-4693 permet de faire fuiter le contenu mémoire (p. ex. variables UEFI) vers le pager GRUB.
- **Contre-lecture** : Exploitation conditionnée à un accès physique ou à des privilèges root préalables, et sans intérêt si Secure Boot est désactivé (root peut alors remplacer GRUB). La faille vient du code amont rc1 authentique, pas d'un ajout openKylin ; toutes les distributions ayant livré 2.12~rc1 avant octobre 2023 étaient exposées (Debian 2.12~rc1-11, Ubuntu 2.12~rc1-10ubuntu4 corrigées le 02/10/2023, d'après le changelog Ubuntu de référence l.271-334). Le risque concret dépend aussi de la politique SBAT du shim utilisé (non audité ici).

### [ELEVEE] `linux` — debian/patches/0007-build-loongarch-ext4.patch (l. 9442)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+CONFIG_SECURITY_LOCKDOWN_LSM policy<{'amd64': 'y', 'arm64': 'y', 'loong64': 'n', 'riscv64': 'y'}> ; l.6376-6378 : -CONFIG_MODULE_SIG_SHA256 {'loong64':'y'} / +CONFIG_MODULE_SIG {'loong64':'n'}`
- **Constat** : Le patch 0007 (titre : « mise à jour des dépendances loongarch ; correction de l'absence d'ext4 ») réécrit 5 843 réglages loong64 de l'annotation (2 276 ajouts, 3 506 modifications, 61 retraits ; aucune autre arch. touchée). Il supprime pour loong64 la frontière root→noyau : SECURITY_LOCKDOWN_LSM y→n (l.9442), MODULE_SIG=n (l.6378, MODULE_SIG_SHA256 retiré l.6376), IMA y→n (l.4481), EVM y→n (l.3052), INTEGRITY_SIGNATURE n (l.4728), SECONDARY_TRUSTED_KEYRING/SYSTEM_BLACKLIST_KEYRING/TRUSTED_KEYS n (l.9416/11449/11746), STRICT_DEVMEM=n avec DEVMEM=y (l.11384/2136) ; MODULE_FORCE_LOAD=y existait déjà (0001:4618). La config importée porte CONFIG_CC_VERSION_TEXT="gcc (GCC) 12.3.1 (kylin 12.3.1-62.p03.ky11)" (l.1185) : .config complet d'un autre produit Kylin (ky11), non dérivé des annotations Ubuntu. Pourtant debian.master/rules.d/loong64.mk déclare uefi_signed = true (0001:10002).
- **Scénario** : Sur une machine loong64, tout processus root (y compris après une élévation locale ou via un paquet tiers malveillant) peut charger un module noyau non signé (insmod, voire --force), ou écrire la RAM physique via /dev/mem : rootkit persistant indétectable par IMA, contournement de la chaîne Secure Boot que suggère uefi_signed=true.
- **Contre-lecture** : Les plateformes Loongson n'ont souvent ni infrastructure de signature de modules ni Secure Boot opérationnel ; l'équipe a pu reprendre tel quel le .config Kylin V11 fonctionnel pour corriger rapidement l'absence d'ext4 (EXT4_FS loong64 ajouté l.3070) sans viser la sécurité. Aucun indice d'intention malveillante.

### [ELEVEE] `openssl1` — debian/patches/series (l. 33-39)

- **Type** : crypto — **Registre** : PROBABLE
- **Extrait** : `CVE-2020-1967-1.patch ... CVE-2020-1967-4.patch / 4-update-version-number-and-commit-to-trigger-a-new-build.patch / add-loong64-support.patch / add-sw64-support.patch`
- **Constat** : libssl1.1 1.1.1f-ok10 (OpenSSL 1.1.1f, mars 2020, fin de vie amont en septembre 2023) est installé dans l'ISO à côté de libssl3t64 (iso-3.0/casper--filesystem.manifest). Dans debian/patches, les seuls correctifs de sécurité postérieurs à 1.1.1f sont CVE-2020-1967 (series:33-36) et CVE-2023-0465/0466 (code dans 4-update-version...patch:14-40, tests et doc dans add-loong64-support.patch:22-246). Le changelog (debian/changelog:21,27,37) ne mentionne pas d'autre CVE. Aucune trace de correctif pour les vulnérabilités 1.1.1 publiées entre 1.1.1g et 1.1.1w puis après la fin de vie. Exemples, liste indicative établie de mémoire, à confirmer dans une base CVE : CVE-2020-1971 (EDIPARTYNAME), CVE-2021-3449 (renégociation), CVE-2021-3711 (débordement en déchiffrement SM2), CVE-2021-3712, CVE-2022-0778 (boucle infinie BN_mod_sqrt sur certificat), CVE-2022-4450, CVE-2023-0215, CVE-2023-0286 (confusion […]
- **Scénario** : Une application qui utilise encore libssl.so.1.1 (c'est la raison de ce paquet : « 为兼容openssl1.x的生态 », changelog:15) parse un certificat, une CRL ou un message TLS fourni par un pair distant malveillant. Les exemples CVE-2022-0778 (déni de service sans authentification au parsing du certificat) et CVE-2023-0286 (lecture mémoire ou plantage lors d'une vérification de CRL) se déclenchent à distance. CVE-2021-3711 donne un débordement de tas si l'application déchiffre du SM2.
- **Contre-lecture** : Le tarball .orig d'openssl1 est repaqueté (renommé openssl1_1.1.1f.orig.tar.gz, sha256 05deef8a…, 9 970 480 octets selon le .dsc) et ne figure pas dans le corpus. Des correctifs ont pu y être intégrés directement, hors quilt. Rien ne permet de le vérifier, et la présence de CVE-2023-0465 sous forme de patch plaide plutôt contre cette hypothèse. Par ailleurs, l'exposition réelle dépend des binaires de l'ISO liés à libssl.so.1.1 : faute de données de dépendances inverses dans le corpus, ils n'ont pas été identifiés.

### [ELEVEE] `ostree` — debian/openkylin.conf (+ debian/openkylin-kaiming.conf, debian/ostree.install:6-7) (l. 1-3)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `[remote "openkylin"] url=http://ostree.openkylin.top/openkylin/ ; gpg-verify=false`
- **Constat** : Remotes systeme installes dans /etc/ostree/remotes.d : transport HTTP en clair et verification GPG des commits desactivee ; aucune verification sign/ed25519 non plus. Le code de verification de libostree n'est PAS retire (ostree-repo-pull.c conserve les branches gpg_verify/signapi, seuls les messages sont traduits) mais n'est jamais active pour ces remotes. Cote client kylin-system-updater (BackendOstreeNext.py:219-255,311-375) appelle pull_with_options sans option gpg ni timestamp-check. Seule l'integrite par adressage de contenu (SHA-256) est garantie, pas l'authenticite.
- **Scénario** : Attaquant en position reseau (DNS/miroir/CDN) repondant aux requetes HTTP du remote avec un commit forge ; le client telecharge et deploie sans verif de signature. Au deploiement, les scripts post-deploy du nouvel arbre sont executes en root (ostree-sysroot-deploy.c:3564).
- **Contre-lecture** : L'authenticite pourrait venir d'un canal amont authentifie fournissant l'ID de commit (liste push/override-commit-ids) ; ce canal n'a pas ete audite et ne couvre pas la resolution directe de ref depuis le serveur. Pratique courante mais a risque chez d'autres distributions OSTree qui, elles, signent (GPG/ed25519).

### [ELEVEE] `ostree` — debian/update-kysec (post-deploy) + ostree-repo-checkout.c:1162 (l. 2-38)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `ostree --subpath=/var/lib/dpkg/plugins checkout $checkout_commit /tmp/plugins ; cp -f /tmp/plugins/spro.so /ostree/pkgs/ovl-<commit>/var-ovl/var-upper/lib/dpkg/plugins/`
- **Constat** : Hook post-deploy execute en root. Il extrait vers le chemin FIXE /tmp/plugins (repertoire monde-inscriptible ; le service updater n'a pas PrivateTmp) puis copie spro.so et ksaf_label.so vers la zone var du nouveau deploiement, sans set -e, sans controle d'erreur ni de proprietaire. Si /tmp/plugins preexiste, 'ostree checkout' echoue (mkdirat EEXIST) et le script copie alors le contenu depose par l'occupant de /tmp/plugins.
- **Scénario** : Un utilisateur local cree /tmp/plugins/spro.so avant une mise a jour ; le hook copie ce .so dans /var/lib/dpkg/plugins du prochain deploiement ; au redemarrage le composant root qui charge ces plugins (dpkg modifie/kysec) execute le code.
- **Contre-lecture** : Le developpeur suppose /tmp vide / espace /tmp prive ; si les plugins ne sont charges qu'apres verification de signature kysec, l'impact se reduit a une alteration d'integrite. Le chargeur de /var/lib/dpkg/plugins/*.so n'est pas dans ce lot (non couvert).

### [ELEVEE] `ostree-extra-utils` — pool/main/o/ostree-extra-utils/ostree-extra-utils_2.0.0.0-0k1.20update1.tar.xz (source publiée) vs ostree-extra-utils_2.0.0.0-0k1.20update1_amd64.deb (binaire) (l. tar.xz:tous / deb:/usr/sbin/unified-conf,/usr/bin/ostree-pkgs-guard,...)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `source publiée = 19 fichiers, tous sous debian/, squelette dh_make (rules: '%: dh $@', aucun .install, scripts seulement en .ex) ; binaire = ELF /usr/sbin/unified-conf + ostree-pkgs-guard + kylin-ostree-restore.py + unified-conf + 9 conffiles. Aucune source du contenu réel.`
- **Constat** : Paquet signé INSTALLÉ PAR DÉFAUT (manifeste ISO l.1560, provenance-paquets-installes.csv l.1561) dont la source homologue (sha256 a4eb018f…, conforme au .dsc et à Sources signé) ne contient aucun code : c'est une coquille dh_make (sources-coquilles-vides.csv:ostree-extra-utils, 10 fichiers .ex). Le contenu réellement livré — un ELF exécuté en initramfs et un hook root exécuté avant chaque dpkg — n'est auditable qu'au binaire. Un paquet plus ancien et DISTINCT, kylin-ostree-extra-utils 1.0.0-ok4, publie 3 scripts voisins mais à contenu divergent (kylin-ostree-restore.py et ostree-update-initramfs.sh diffèrent, diff -u : en-tête de copyright et 6 lignes mount --bind ajoutées) et n'a jamais produit ce nom de paquet binaire.
- **Scénario** : Le code réellement exécuté en root (hook dpkg, générateur d'environnement systemd, ELF initramfs, restauration d'usine) peut diverger de tout source publié sans qu'aucun tiers ne puisse le détecter par revue ; une modification malveillante y serait invisible.
- **Contre-lecture** : Omission d'empaquetage plausible : l'équipe a conservé un gabarit dh_make comme source nominale et n'a pas versionné les fichiers réels dans ce dépôt (le dépôt kylin-ostree-extra-utils contient une partie du code). Non-conformité DFSG/traçabilité, pas nécessairement dissimulation. Vérifiable en Phase 5 en exigeant le dépôt source gitlab2.kylin.com/kylinos-src/ostree-extra-utils (copyright:4).

### [ELEVEE] `ostree-extra-utils` — etc/ld.so.conf.d/zzz-ostree.conf ; usr/lib/python3/dist-packages/opt_path.pth ; etc/ostree/ostree.conf.d/99-ostree.conf ; etc/systemd/system-environment-generators/99-ostree ; etc/X11/Xsession.d/70opt-path ; etc/ostree/compat-paths.conf (l. zzz-ostree:1 ; opt_path.pth:1 ; 99-ostree.conf:2-8 ; 99-ostree:3 ; 70opt-path:1-2)

- **Type** : confiance — **Registre** : PROBABLE
- **Extrait** : `zzz-ostree.conf:1 '/opt/system/lib/shared-libs' ; opt_path.pth:1 '/opt/system/lib/python3/dist-packages' ; 99-ostree.conf: PATH=$PATH:/opt/system/bin, QML2_IMPORT_PATH, PKG_CONFIG_PATH, SYSTEMD_UNIT_PATH=/opt/system/lib/systemd/system:...user ; 99-ostree: XDG_DATA_DIRS+=/opt/system/resource`
- **Constat** : Ce paquet fait de /opt/system une RACINE DE CONFIANCE à l'échelle du système : (1) ldconfig ajoute /opt/system/lib/shared-libs au chemin des bibliothèques partagées (toute *.so y est chargée par tout processus, root compris) ; (2) opt_path.pth ajoute /opt/system/lib/python3/dist-packages au sys.path de CHAQUE python3 ; (3) PATH gagne /opt/system/bin, SYSTEMD_UNIT_PATH /opt/system/lib/systemd/{system,user} (unités propagées au gestionnaire systemd via unified-conf, cf. finding dédié), XDG_DATA_DIRS /opt/system/resource, XDG_CONFIG_DIRS /opt/system/conf/xdg. Le risque dépend ENTIÈREMENT du propriétaire et du mode de /opt/system et de ses sous-dossiers. postinst:128-130 crée /opt/system/bin en root:root 0755, mais /opt/system lui-même est créé/bind-monté ailleurs (ostree fork ostree-sysroot-deploy.c:3648 'chroot … mkdir -p … /opt/system' puis bind_mount:3276) et alimenté par kaiming/kare. O […]
- **Scénario** : Un utilisateur local sans privilège dépose (directement si /opt/system/lib/shared-libs est inscriptible, ou via l'action pkexec kaiming sans mot de passe) une bibliothèque ou un module python ; au prochain démarrage/login/exécution d'un processus ou script root, le code est chargé en root — élévation locale persistante.
- **Contre-lecture** : Si /opt/system et tous ses sous-dossiers sont strictement root:root 0755 sur un système installé et ne peuvent être peuplés que par des flux root authentifiés, l'extension est une simple adaptation de recherche de chemins pour le système immuable, sans élévation. L'inscriptibilité réelle n'est pas observable sur les seuls manifestes ISO : à établir en Phase 5.

### [ELEVEE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 30-120 (polkitbackendactionpool.c, polkitbackendduktapeauthority.c))

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `collect_directories_from_xdg_data_dirs(): pour chaque entree de $XDG_DATA_DIRS prefixee par /usr/ ou /opt/, ajoute <dir>/polkit-1/actions ; append_rules_dirs_from_xdg_data_dirs(): idem pour /usr/, /opt/, /lib/, /var/ -> <dir>/polkit-1/rules.d`
- **Constat** : Le fork openKylin de polkit (127-ok3) elargit, au-dela des chemins compiles, les repertoires d'ou polkit charge les ACTIONS et surtout les REGLES JavaScript (rules.d) : toute entree de XDG_DATA_DIRS sous /usr, /opt, /lib ou /var est prise en compte. Le titre du patch ("update changelog") ne reflete pas son contenu (145 lignes de code C dans le moteur d'autorisation). polkit s'execute en root (polkitd).
- **Scénario** : Combine avec le generateur d'environnement systeme de kaiming (etc/systemd/system-environment-generators/60-kaiming, installe par defaut) qui injecte /opt/kaiming/share dans XDG_DATA_DIRS pour TOUS les services, et data/60-kaimingenv.conf qui ajoute /var/opt/kaiming/share/* : polkit lit alors des regles d'autorisation .rules depuis des emplacements sous /opt et /var. Si l'un de ces repertoires rules.d est accessible en ecriture a un utilisateur (ou le devient via les actions kaiming ci-dessus, ou via un paquet kaiming installe par l'utilisateur), celui-ci peut deposer une regle JavaScript accordant toute autorisation polkit -> root. A verifier en Phase 5 : droits reels de /opt/kaiming/share/ […]
- **Contre-lecture** : Objectif : permettre aux applications empaquetees kaiming d'installer leurs propres actions/regles polkit hors de /usr. Legitime en soi, mais deplace la frontiere de confiance de polkit vers /opt et /var, dont les permissions doivent etre irreprochables ; le patch ne verifie ni le proprietaire ni les permissions des repertoires ajoutes.

### [ELEVEE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 3, 10-14)

- **Type** : obfuscation — **Registre** : CONSTATÉ
- **Extrait** : `Subject: !12 update changelog ... src/polkitagent/polkitagenthelper-pam.c \|2 +-; polkitbackendactionpool.c \|61 ++; polkitbackendduktapeauthority.c \|79 ++; pkexec.c \|5 ++  (145 insertions, 2 deletions)`
- **Constat** : Le patch porte un titre/Subject << update changelog >> mais ne touche AUCUN fichier changelog : il injecte 145 lignes de C dans 4 fichiers du coeur de polkit (agent PAM, pool d'actions, moteur de regles duktape, pkexec). Titre deliberement trompeur masquant des modifications de mecanismes d'authentification et d'autorisation. L'entree changelog 127-ok3 dit seulement << 同步v11实现代码 >> (sync code v11).
- **Scénario** : Un relecteur survolant la serie quilt ou le changelog classe ce patch comme cosmetique et n'inspecte pas son contenu, laissant passer en revue un affaiblissement d'auth/autorisation.
- **Contre-lecture** : Pratique git courante ou le titre d'une PR mergee (!12) ecrase le sujet du commit ; le contenu reel serait un simple oubli de renommage de PR plutot qu'une dissimulation intentionnelle.

### [ELEVEE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 20-25)

- **Type** : privileges — **Registre** : CONSTATÉ
- **Extrait** : `- if (rc != PAM_SUCCESS) + if (rc != PAM_SUCCESS && rc != PAM_NEW_AUTHTOK_REQD)  (apres pam_acct_mgmt dans polkitagenthelper-pam.c)`
- **Constat** : polkit-agent-helper-1 (binaire exec en root : SUID 4755 sur non-systemd, socket-active sinon) valide le mot de passe via pam_authenticate puis la validite du compte via pam_acct_mgmt. L'amont rejette (goto error) tout code != PAM_SUCCESS. Le patch accepte en plus PAM_NEW_AUTHTOK_REQD (mot de passe expire / devant etre change) SANS declencher pam_chauthtok : un compte au mot de passe expire est traite comme pleinement authentifie pour autoriser des actions polkit privilegiees.
- **Scénario** : Un administrateur force l'expiration du mot de passe d'un compte (chage -d 0 / passwd -e) pour bloquer l'acces en attendant un reset ; l'utilisateur, connaissant encore l'ancien mot de passe, s'authentifie malgre tout aupres de polkit (pkexec, operations privilegiees) sans jamais changer son mot de passe, contournant la politique d'expiration.
- **Contre-lecture** : Amelioration d'ergonomie : eviter qu'un dialogue polkit echoue cryptiquement quand le mot de passe est expire ; l'utilisateur doit toujours connaitre le mot de passe courant, donc ce n'est pas un bypass d'authentification complet mais un assouplissement de la gestion de compte.

### [ELEVEE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 123-206 (notamment 160-166, 188, 215))

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `append_rules_dirs_from_xdg_data_dirs(): prefix /usr/ \|\| /opt/ \|\| /lib/ \|\| /var/ ; rules_path = g_build_filename(trimmed, "polkit-1", "rules.d") ; appele dans polkit_backend_common_js_authority_constructed()`
- **Constat** : L'amont charge les regles JS d'autorisation (executees par duktape pour decider YES/NO) depuis une liste FIXE root-controlee (/etc, /run, /usr/local/share, /usr/share .../rules.d). Le patch ajoute tout composant de $XDG_DATA_DIRS prefixe par /usr/,/opt/,/lib/,/var/ + /polkit-1/rules.d, SANS verification de proprietaire ni de permissions du repertoire. Le prefixe /var/ autorise p.ex. /var/tmp (monde-inscriptible, sticky) => /var/tmp/polkit-1/rules.d. Filtre rules.d plus large que celui des actions.
- **Scénario** : Si $XDG_DATA_DIRS de polkitd peut etre influence (demarrage hors unit systemd durcie, drop-in/EnvironmentFile mal configure, superviseur non standard heritant l'environnement), un utilisateur local depose /var/tmp/polkit-1/rules.d/00.rules contenant polkit.addRule(()=>polkit.Result.YES) et obtient l'autorisation de toute action => root. En deploiement systemd par defaut, XDG_DATA_DIRS n'est pas herite (repli /usr/local/share:/usr/share) donc non directement exploitable : le code est CONSTATÉ, la voie d'exploitation reste conditionnelle (PROBABLE).
- **Contre-lecture** : Intention de permettre a des paquets installes sous /opt ou des prefixes alternatifs de livrer leurs regles ; en pratique polkitd systemd tourne avec un environnement nettoye, donc la surface reelle est limitee tant que XDG_DATA_DIRS reste maitrise.

### [ELEVEE] `shadow (paquet binaire passwd)` — /usr/bin/passwd, chsh, chfn, gpasswd (setuid root) ; chage, expiry (setgid shadow) (passwd_4.17.4-ok1_amd64.deb) (l. en-tête ELF et section dynamique)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `readelf : « Type: EXEC » (non PIE) ; aucune entrée (FLAGS) BIND_NOW ; aucun symbole __stack_chk_fail ni __*_chk ; aucune propriété x86 IBT/SHSTK — modes -rwsr-xr-x root:root (4 binaires) et -rwxr-sr-x root:shadow (chage, expiry)`
- **Constat** : Les 6 binaires privilégiés du paquet passwd (4 setuid root : passwd, chsh, chfn, gpasswd ; 2 setgid shadow : chage, expiry) d'openKylin (1:4.17.4-ok1) sont compilés sans AUCUNE des protections standard : ni exécutable à adresse aléatoire (PIE), ni RELRO complet (BIND_NOW), ni protecteur de pile, ni FORTIFY, ni CET. Les mêmes binaires Ubuntu (1:4.17.4-2ubuntu3, même source shadow 4.17.4) les ont toutes. Plus largement, 9 des 10 binaires setuid/setgid relevés ne sont pas PIE (seul sudo.ws l'est). Relevé : donnees/phase2/durcissement-binaires.json (outil outils/durcissement_binaires.py, paquets obtenus par chaîne signée, lecture readelf sans exécution). Correction après vérification contradictoire (G9) : chage et expiry sont setgid shadow et non setuid root ; pkexec et polkit-agent-helper-1, rendus setuid par leurs scripts postinst, sont eux aussi non PIE.
- **Scénario** : Un utilisateur local qui trouverait un défaut de corruption mémoire dans l'un de ces binaires (analyse de /etc/login.defs, d'arguments ou de variables d'environnement) l'exploiterait bien plus facilement qu'ailleurs : adresse du programme fixe, pas de canari de pile, pas de vérification FORTIFY, table GOT inscriptible, pas de contrôle de flux CET. Le gain obtenu serait root (passwd, chsh, chfn, gpasswd) ou l'accès au groupe shadow (chage, expiry).
- **Contre-lecture** : Aucune faille connue n'est établie dans shadow 4.17.4 : il s'agit de l'absence de défenses en profondeur, pas d'une vulnérabilité exploitable en soi. L'ASLR reste active pour la pile, le tas et les bibliothèques.

### [MOYENNE] `apport` — ref-p2/openkylin/apport_2.28.1-ok3/orig/data/apport (l. 647, 769, 1124-1127)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `create_directory(apport.fileutils.report_dir, 0o1777) / if _check_global_pid_and_forward(options): return 0 / gid = pwd.getpwnam("whoopsie").pw_gid ... except (OSError, KeyError): os.fchown(fd, report_owner.uid, report_owner.gid)`
- **Constat** : openKylin 2.28.1-ok3 est basé sur la version de sortie 2.28.1-0ubuntu2 (orig identique, sha256 b9a83445…, signature amont valide) et ne contient aucun des correctifs publiés depuis par Ubuntu dans noble-security jusqu'à 2.28.1-0ubuntu3.8 (42 fichiers de correctifs absents). Correctifs de sécurité manquants : CVE-2025-5054 (course PID lors du transfert de vidages vers les conteneurs → lecture du vidage d'un programme SUID ; 10 correctifs de 0ubuntu3.6 : apport-move-consistency_checks-call-further-up, apport-do-not-override-options.pid, apport-open-proc-pid-as-early-as-possible, fileutils-respect-proc_pid_fd-in-get_core_path, apport-use-opened-proc-pid-everywhere, apport-do-consistency-check-before-forwarding-crashes, apport-require-dump-mode-to-be-specified, apport-determine-report-owner-by-dump_mode, apport-do-not-forward-crash-for-dump_mode-2, apport-support-pidfd-F-parameter-from-kerne […]
- **Scénario** : Un administrateur installe depuis le dépôt openKylin apport et apport-core-dump-handler (apport devient gestionnaire core_pattern). Un utilisateur local provoque le plantage d'un programme SUID et exploite la course sur /proc/<pid> pour obtenir un vidage contenant des données sensibles (CVE-2025-5054) ; ou un membre du même groupe primaire lit les rapports .crash d'un autre utilisateur (CVE-2025-5467). Aucun code d'exploitation n'est fourni.
- **Contre-lecture** : Le paquet apport n'est PAS installé dans l'ISO 3.0 (seuls python3-apport et python3-problem-report : rapport-audit/donnees/iso-3.0/casper--filesystem.manifest:1652, 1696) ; systemd-coredump est le gestionnaire installé (:1796). Installer apport seul satisfait 'apport-core-dump-handler | systemd-coredump' par systemd-coredump, chemin qui retourne avant le transfert vulnérable (data/apport:760-761). Les correctifs sont dans data/apport (paquet apport) sauf fileutils.py. Exposition conditionnée à une installation volontaire.

### [MOYENNE] `apt (configuration ISO)` — apt-pkg/contrib/gpgv.cc (l. 398-417)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `auto Parts = GetListOfFilesInDir(_config->FindDir("Dir::Etc::TrustedParts"), ...); for (auto &Part : Parts) maybeAddKeyring(Part);`
- **Constat** : Sans Signed-By, APT fait confiance à TOUTES les clés de /etc/apt/trusted.gpg.d pour TOUS les dépôts ; apt-private/private-update.cc:119-135 se contente d'une Notice ou d'un Audit. Or l'ISO installe openkylin-archive-anything (iso-3.0/casper--filesystem.manifest), qui dépose dans trusted.gpg.d la clé « openKylin Build Service PPA for kylinsoft developers » (3EA5 5C18 ... 49E9 E2C8, donnees/apt/verification-signatures-et-empreintes.txt), à côté de la clé d'archive 09FF C10E ... FF59 2D96. La clé d'un service de build PPA peut donc signer un InRelease accepté pour l'archive principale ou de sécurité.
- **Scénario** : Compromission de la clé du service de build PPA (clé de signature automatique, a priori moins protégée), puis service d'un InRelease forgé pour huanghe-security via MITM HTTP ou miroir. APT l'accepte et installe des paquets arbitraires en root.
- **Contre-lecture** : Modèle de confiance APT historique (Debian/Ubuntu), clés appartenant au même éditeur, PPA effectivement utilisée par l'ISO (provenance-paquets-installes.csv). Le correctif relève des sources et du trousseau (Signed-By), hors de l'arbre apt.

### [MOYENNE] `apt (dépôts openKylin)` — rapport-audit/donnees/apt/archive.build.openkylin.top_huanghe-security_InRelease (l. 1-20)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `Date: Mon, 17 Aug 2026 18:20:38 UTC (aucun champ Valid-Until)`
- **Constat** : Aucun des 6 InRelease relevés (huanghe, huanghe-security, huanghe-updates, archive.openkylin.top, software, ppa anything3.0) ne porte Valid-Until. APT n'applique l'expiration que si le champ existe ou si Acquire::Max-ValidTime est configuré (apt-pkg/deb/debmetaindex.cc:607-657), et le paquet apt ne livre aucun Max-ValidTime. APT bloque seulement le retour à un InRelease plus ancien que le dernier connu (apt-pkg/acquire-item.cc:1873-1885). Il ne détecte pas un gel. huanghe-updates est daté du 15/10/2024.
- **Scénario** : Un attaquant en position réseau (sources en HTTP) rejoue indéfiniment le dernier InRelease signé valide et bloque en silence les correctifs de sécurité. Il ne peut pas injecter de paquet, mais garde le parc exploitable après la publication d'une CVE.
- **Contre-lecture** : Pratique identique à celle des archives principales Ubuntu (pas de Valid-Until). Le risque se limite au gel, sans exécution de code. Le côté dépôt est hors de l'arbre apt.

### [MOYENNE] `aptdaemon` — ref-p2/openkylin/aptdaemon_1.1.1+bzr982-ok7/orig/aptdaemon/core.py (l. 2011-2021)

- **Type** : privileges — **Registre** : CONSTATÉ
- **Extrait** : `@dbus.service.method(APTDAEMON_DBUS_INTERFACE, in_signature="ss", out_signature="b") def SetEnviron(self , key , value): ... os.environ[key] = value / return True`
- **Constat** : L'archive amont (orig) livrée par openKylin n'est pas l'orig Ubuntu : elle contient une méthode D-Bus SetEnviron ajoutée par Kylin dans le démon root aptd, sans sender_keyword ni contrôle policykit (aucun appel à _check_auth/policykit1), atteignable par tout utilisateur local (ref-p2/openkylin/aptdaemon_1.1.1+bzr982-ok7/orig/data/org.debian.apt.conf:10-14, policy context="default" allow send_destination="org.debian.apt"). Elle écrit une variable d'environnement arbitraire dans le processus root, héritée par les processus enfants (dpkg, scripts de mainteneur, debconf). Dans la version auditée ok7, cette méthode est RETIRÉE par archive/aptdaemon/debian/patches/0001-update-new-version.patch:164-177 (format 3.0 (quilt), série appliquée à l'extraction) : l'arbre source ok7 après application de la série est identique octet pour octet à Ubuntu noble 0ubuntu44 patché (283 fichiers, hors fichier  […]
- **Scénario** : Si l'orig est construit sans appliquer la série quilt (dpkg-source --skip-patches, dépôt git en mode patches-non-appliqués, ou version future abandonnant 0001), aptd réexpose SetEnviron : un utilisateur local sans privilège appelle la méthode sur le bus système pour placer dans l'environnement du démon root une variable influençant les processus root lancés lors de la transaction suivante (élévation locale vers root). Aucun code d'exploitation n'est fourni.
- **Contre-lecture** : Fonction conçue pour transmettre DISPLAY/XAUTHORITY de la session graphique à aptd (dialogues debconf/conffile), cf. kylin-system-updater:456-457 ; openKylin l'a explicitement retirée en ok6/ok7, et l'appel côté client échoue proprement (UpdateManagerDbus.py:124-130). Le paquet binaire construit depuis le source ok7 ne devrait pas la contenir (non vérifié sur binaire).

### [MOYENNE] `ca-certificates` — sbin/update-ca-certificates (l. 30, 54-56, 124, 171-182)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `COMPATCERTSDIR=/opt/system/resource/ca-certificates ... if [ -d "$COMPATCERTSDIR" ]; then find -L "$COMPATCERTSDIR" -type f -name '*.crt' \| ... add ; find -L ... -name '*.cer' \| ... add; fi`
- **Constat** : Ajout Kylin realisant le « 支持保存国密证书方案 » du changelog : tout fichier *.crt ET *.cer trouve recursivement (find -L, liens suivis) sous /opt/system/resource/ca-certificates est ajoute au magasin de confiance TLS systeme (liens dans /etc/ssl/certs + concatenation dans ca-certificates.crt). Ces CA sont HORS controle debconf (/etc/ca-certificates.conf ne peut pas les desactiver) et hors du repertoire admin standard /usr/local/share/ca-certificates. C'est un mecanisme (nouveau repertoire de CA), AUCUNE CA n'est livree dedans par ce paquet.
- **Scénario** : Tout composant ecrivant dans /opt/system/resource/ca-certificates (p. ex. kaiming qui rsync des arbres vers /opt/system/resource/*, ou un paquet applicatif) ajoute une CA racine de confiance systeme a la prochaine execution du trigger update-ca-certificates, sans trace debconf et sans possibilite de desactivation par la liste standard.
- **Contre-lecture** : Objectif declare legitime : permettre l'installation de certificats SM2/GuoMi par l'outillage Kylin sur systeme immuable (ou /usr/local n'est pas le bon emplacement). Le repertoire etant vide a l'installation, aucune CA supplementaire n'est de confiance par defaut sur l'ISO.

### [MOYENNE] `coreutils` — rapport-audit/donnees/iso-3.0/casper--filesystem.manifest (l. 56)

- **Type** : confiance — **Registre** : PROBABLE
- **Extrait** : `coreutils	9.4-ok4`
- **Constat** : Les outils réellement exécutés en root par les scripts de mainteneur, les services et les administrateurs (ls, cp, mv, rm, chmod, chown, install, split, sort...) viennent du binaire coreutils 9.4-ok4. Aucune source publiée ne produit ce binaire : l'arbre audité est en 9.5-1ok3.4, et son changelog ne contient AUCUNE entrée 9.4-ok* (lignée Kylin 9.4 absente, changelog:37-43 passe de 9.5-1ubuntu4.1 à 9.5-1ok1). Ce delta ne permet donc ni de vérifier le contenu de ces binaires ni de dire quels correctifs de sécurité ils portent. Exemple concret : CVE-2024-0684 (débordement de tas dans split --line-bytes) touche la 9.4 ; Ubuntu l'a corrigée par un patch dédié (debian/changelog:258-262, 9.4-2ubuntu*). Rien ne permet de savoir si 9.4-ok4 l'inclut. Le binaire installé ne correspond pas à la source auditée : c'est déjà relevé en Phase 0/1 (01-inventaire.md:321, 06-angles-morts.md:120). Ce constat […]
- **Scénario** : Un script système ou un utilisateur appelle split --line-bytes sur une entrée contrôlée par un attaquant (fichier journal, données téléchargées, par exemple dans une tâche de traitement exécutée en root). Si 9.4-ok4 ne contient pas le correctif CVE-2024-0684, le débordement de tas peut provoquer un plantage, voire une exécution de code dans le contexte de l'appelant. Plus largement, toute modification non publiée de ces binaires critiques, comme un traitement particulier des attributs étendus ou de kysec par cp, mv ou install, échappe à l'audit.
- **Contre-lecture** : Lecture bénigne très plausible : incohérence d'empaquetage. openKylin a importé la 9.5 d'Ubuntu, déjà renommée gnu-coreutils, sans importer la source coreutils-from qui fournit le paquet « coreutils » chez Ubuntu. Le dernier binaire « coreutils » du pool, 9.4-ok4, issu de la lignée openKylin précédente, reste alors le fournisseur des commandes de base. 9.4-ok4 dérive probablement de Debian ou Ubuntu 9.4, qui intègrent le correctif CVE-2024-0684. À confirmer en récupérant le .deb et la source 9.4-ok4 : le contenu réel de ce binaire n'est pas établi.

### [MOYENNE] `coreutils` — sources-openkylin/index-apt/archive.build.openkylin.top/build_huanghe_main_amd64_Packages.gz (décompressé) (l. 14170-14172)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `Package: coreutils / Architecture: amd64 / Version: 9.4-ok4 (Filename: pool/main/c/coreutils/coreutils_9.4-ok4_amd64.deb) ; manifeste ISO casper--filesystem.manifest:56 'coreutils 9.4-ok4'`
- **Constat** : Le binaire coreutils 9.4-ok4, installé par défaut (manifeste ISO l.56) et qui fournit les commandes de base (ls, cp, mv, rm, chmod, chown, install…), n'a AUCUNE source correspondante : la chaîne '9.4-ok4' apparaît 0 fois dans les 10 index Sources signés (archive.build.openkylin.top main/pty/cross et -security, archive.openkylin.top main/pty/cross, PPA anything3.0). Le seul source 'coreutils' publié est 9.5-1ok3.4 (build_huanghe_main_Sources.gz l.7230), qui ne produit que gnu-coreutils (commandes préfixées 'gnu'). Les binaires les plus exécutés du système, souvent en root, sont donc inauditables statiquement ; leurs correctifs de sécurité et leur traitement des attributs kysec ne sont pas vérifiables. Confirme et renforce le constat PROBABLE du lot P2-coreutils.
- **Scénario** : Un binaire de base sans source publiée pourrait diverger de GNU coreutils (comportement modifié, code ajouté, correctif CVE manquant) sans qu'aucun tiers ne puisse le vérifier ; sa compromission serait invisible à toute revue de code.
- **Contre-lecture** : Reliquat d'empaquetage plausible : version 9.4 importée d'Ubuntu (coreutils 9.4-ok4 ~ Ubuntu 9.4-3ubuntu*) avant la migration vers gnu-coreutils 9.5, conservée pour les commandes non préfixées ; l'absence de source serait alors une omission de publication, non une dissimulation. Vérifiable en Phase 5 en comparant le .deb 9.4-ok4 à coreutils 9.4 d'Ubuntu (empreintes, symboles).

### [MOYENNE] `curl` — debian/libcurl3t64-gnutls.install ; debian/libcurl3t64-gnutls.symbols ; debian/rules ; debian/ (absence de patches/) (l. install:1 ; symbols:1,3 ; rules:63-74)

- **Type** : confiance — **Registre** : PROBABLE
- **Extrait** : `usr/lib/*/libcurl-gnutls.so.4* \| libcurl-gnutls.so.4 libcurl3t64-gnutls #MINVER# \| CURL_GNUTLS_3@CURL_GNUTLS_3 7.16.2 \| (aucun repertoire debian/patches)`
- **Constat** : L'arbre curl n'a AUCUN repertoire debian/patches (42 fichiers reguliers, aucun patch). Pourtant l'empaquetage attend des artefacts que l'amont curl 8.18.0 ne produit pas tel quel : une bibliotheque renommee libcurl-gnutls.so.4 et un noeud de version de symboles CURL_GNUTLS_3. Dans Debian, ces deux points viennent des patches de la serie Debian (renommage de la variante GnuTLS et compatibilite des symboles). debian/rules (configure-curl, lignes 63-74) copie l'arbre et lance autoreconf + configure --with-gnutls sans aucune etape de renommage. Ces modifications sont donc tres probablement pre-appliquees dans curl_8.18.0.orig.tar.gz (dsc:30, sha256 bfaeb060...926b), dont l'empreinte DIFFERE de celle d'Ubuntu resolute 8.18.0-1ubuntu2 (donnees/forks-p2-delta-preliminaire.csv:12, EMPREINTE-DIFFERENTE). Consequence : pour curl, le delta openKylin n'est PAS entierement dans debian/ ; une partie ( […]
- **Scénario** : Une modification de la verification TLS (par exemple dans lib/vtls/ ou dans la gestion de CURLOPT_SSL_VERIFYPEER) inseree dans l'orig serait invisible a toute revue de debian/ et a l'outillage quilt, tout en etant livree dans libcurl, utilise par de nombreux programmes du systeme (mises a jour, telechargements). Ce qui manque pour conclure : l'orig openKylin, a comparer a curl-8.18.0.tar.gz signe par l'amont (cle 27ED EAF2 2F3A BCEB 50DB 9A12 5CC9 08FD B71E 12C2 presente dans debian/upstream/signing-key.asc) et a l'orig Ubuntu.
- **Contre-lecture** : Flux de travail openKylin courant : le depot gitee stocke l'arbre source complet avec les patches Debian/Ubuntu deja appliques, puis l'orig est regenere depuis git. L'ecart d'empreinte serait alors exactement la serie Debian/Ubuntu (renommage libcurl-gnutls, symboles CURL_GNUTLS_3, correctifs de pages de manuel), sans code ajoute. Les symboles et fichiers .install observes sont strictement ceux de Debian.

### [MOYENNE] `dpkg` — debian/patches/feat_openKylin.patch (l. 28, 50)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+++ b/scripts/Dpkg/Vendor/openKylin.pm ... +package Dpkg::Vendor::openKylin;  \|\| orig scripts/Dpkg/Vendor.pm:181-182 : push @names, join q{}, map { ucfirst } @vendor_parts; push @names, join q{}, map { ucfirst lc } @vendor_parts;`
- **Constat** : Le module fournisseur ajouté par le seul correctif openKylin s'appelle openKylin.pm (o minuscule). Or Dpkg::Vendor::get_vendor_object() de 1.23.7 (orig scripts/Dpkg/Vendor.pm:178-194) ne tente que Dpkg::Vendor::OpenKylin et Dpkg::Vendor::Openkylin : le module livré par libdpkg-perl (build_Contents-amd64.gz l. 1791743) n'est jamais chargé sur un système de fichiers sensible à la casse. Le fichier d'origine de base-files 14-ok3 (archive/base-files/origins/openkylin:1-3, lien default posé par debian/postinst.in:43-45) déclare « Vendor: openKylin » SANS champ Parent : la résolution retombe sur Dpkg::Vendor::Default (Vendor.pm:196-201), dont le crochet update-buildflags n'ajoute rien (Default.pm:204-205). Sur un openKylin 3.0 installé (dpkg, dpkg-dev, libdpkg-perl 1.23.7-ok3 et base-files sont dans casper--filesystem.manifest l. 110, 111, 528), dpkg-buildflags et debhelper ne fournissent donc […]
- **Scénario** : Un éditeur ou un administrateur construit des paquets .deb sur openKylin 3.0 (dpkg-buildpackage/debhelper) : les exécutables produits, y compris des services root, ne reçoivent aucun des drapeaux de durcissement de la distribution ; si le compilateur ne les active pas par défaut, un débordement de pile ou de tampon y devient exploitable sans canari, sans FORTIFY et avec une GOT réinscriptible.
- **Contre-lecture** : Erreur de nommage involontaire (changelog:21 « adapt to openKylin.pm ») plutôt qu'intention ; l'effet réel dépend des défauts de gcc-15 15.2.0-16ok8 (source absente du corpus : un GCC à défauts Ubuntu active SSP, FORTIFY, relro, now, PIE et CET indépendamment de dpkg-buildflags) et de DEB_VENDOR ou d'une autre origine dans l'environnement de construction.

### [MOYENNE] `dpkg` — debian/rules (l. 13, 17-22, 52)

- **Type** : durcissement — **Registre** : PROBABLE
- **Extrait** : `export DEB_BUILD_MAINT_OPTIONS = hardening=+all ... $(CURDIR)/build-aux/run-script scripts/dpkg-buildflags.pl ... $(shell $(dpkg_buildflags) --export=configure)`
- **Constat** : dpkg (Essential, exécuté en root, analyse des archives .deb : ar, tar, décompresseurs) est configuré avec les drapeaux du dpkg-buildflags de l'arbre, qui charge les modules Dpkg::Vendor de l'arbre et lit l'origine de l'hôte de construction (build-aux/run-script:15-16 ne redéfinit pas DPKG_ORIGINS_DIR). Sur un hôte openKylin (origine openKylin sans Parent), hardening=+all ne porte sur aucune fonctionnalité, Default n'en définissant pas : dpkg 1.23.7-ok3 serait compilé sans les drapeaux de durcissement de dpkg-buildflags, seuls restant les défauts du compilateur, les avertissements WFLAGS et -Wl,--as-needed. Indice indirect : l'entrée ok3 ajoute -Wl,--as-needed au niveau système (buildflags.conf:1), ce que le GCC d'Ubuntu fait par défaut, ce qui suggère un compilateur sans les défauts Ubuntu, donc peut-être aussi sans SSP/FORTIFY/now par défaut. Manque pour confirmer : le binaire dpkg_1.23 […]
- **Scénario** : Un paquet .deb piégé (dépôt tiers ou fichier installé par dpkg -i) exploite un défaut mémoire du code d'extraction de dpkg ; sans canari de pile, FORTIFY ni RELRO complet, la prise de contrôle du processus root est facilitée.
- **Contre-lecture** : Le parc de construction peut définir DEB_VENDOR, utiliser une autre origine ou un dpkg-dev plus ancien ; gcc-15 d'openKylin peut activer ces protections par défaut. Le .dsc de dpkg-wrapper (3.0 native avec révision, signé après dpkg 1.23.7-ok3) aurait été refusé par dpkg-source 1.23.7 sous le fournisseur Default (orig V3/Native.pm:95-100) : l'environnement qui l'a produit ne retombait donc pas sur Default, ou utilisait un dpkg-dev antérieur à 1.23.

### [MOYENNE] `dpkg-wrapper` — pool/main/d/dpkg-wrapper/dpkg-wrapper_1.0.0.0-0k2.1ok1.tar.xz (source publiée) vs dpkg-wrapper_1.0.0.0-0k2.1ok1_all.deb (binaire) (l. tar.xz:tous / deb:/usr/bin/{dpkg_wrapper,apt_wrapper,wrapper_common})

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `source publiée (sha256 30dbea54…, conforme .dsc:17 et Sources signé) = 19 fichiers, tous sous debian/, aucune ligne de code (rules 'dh $@', scripts .ex uniquement) ; binaire = 3 scripts bash (dpkg_wrapper 17127o, apt_wrapper 14655o, wrapper_common 16249o).`
- **Constat** : Paquet signé, PRÉSENT dans l'archive par défaut (huanghe/main, Packages.gz: Filename pool/main/d/dpkg-wrapper/…, Depends 'dpkg, libkysdk-conf2-tools' tous deux présents dans l'index) donc INSTALLABLE via apt, mais ABSENT de l'ISO 3.0 (hors manifeste). Sa source homologue est une coquille dh_make sans code (sources-coquilles-vides.csv:dpkg-wrapper). Le code réellement exécuté — qui détourne /usr/bin/dpkg, apt, apt-get vers des scripts — n'a aucune source publiée. Exposition : l'installation remplace les gestionnaires de paquets du système par des wrappers non auditables au source ; corrobore le constat moyen déjà porté par P2-dpkg.json.
- **Scénario** : Un administrateur installe dpkg-wrapper depuis le dépôt par défaut : dpkg/apt/apt-get sont remplacés par des scripts dont le source n'est pas publié ; une modification de leur logique (filtrage, contournement de vérifications) ne serait pas détectable par revue de source.
- **Contre-lecture** : Omission de publication : le code vit probablement sur une forge interne. Non-conformité traçabilité. Exposition réduite car hors ISO (installation explicite requise).

### [MOYENNE] `dpkg-wrapper` — usr/bin/wrapper_common (deb) ; configuration libkysdk-package (libkysdk-conf2-tools:kconf2) (l. wrapper_common:185-193,195-204,282-313,415-507 ; libkysdk-package.yaml:7-15,102-105)

- **Type** : confiance — **Registre** : PROBABLE
- **Extrait** : `get_whitelist_runtime_check: kconf2 get libkysdk-package.installation whitelist_runtime_check ; get_privilege_runtime_check: … privilege_runtime_check ; is_in_white_list: kconf2 list-keys libkysdk-package.direct_install_whitelist ; is_in_privilege_list: kconf2 get libkysdk-package.bare_metal pkg-lis […]`
- **Constat** : Les wrappers lisent leur politique dans la configuration kconf2 (paquet libkysdk-conf2-tools, dépendance déclarée) : commutateurs libkysdk-package.installation.{whitelist_runtime_check(défaut false), privilege_runtime_check(défaut true), pkg_sign_check(défaut true)}, listes direct_install_whitelist et bare_metal.pkg-list. Si privilege_runtime_check/whitelist_runtime_check sont mis à false, les contrôles runtime (correspondance de tag runtime_version, vérification gpg du tag bare_metal) sont sautés — seul kylinsigntool -v reste. Or V1-L2 (élevée) établit que la méthode D-Bus root 'save' de conf2 est ouverte à tout utilisateur et que /home/<user>/.config/kylin-config/user.db est fusionnée et parfois chmod 0666 : la politique d'installation privilégiée des wrappers repose donc sur une configuration potentiellement modifiable par un utilisateur non privilégié. De plus privilege_deb_check ext […]
- **Scénario** : Un utilisateur capable de modifier les clés kconf2 libkysdk-package (via la faille conf2 V1-L2) met privilege_runtime_check=false, affaiblissant la vérification de tag des paquets privilégiés ; ou exploite le chemin /tmp/sdk prévisible (lien symbolique pré-créé) pour détourner une écriture/lecture root pendant l'extraction. Nécessite que la vérification kylinsigntool soit elle-même contournable pour un gain réel.
- **Contre-lecture** : kylinsigntool -v reste exigé inconditionnellement avant tout contrôle runtime, donc la signature麒麟 demeure la vraie barrière ; les commutateurs ne font qu'ajuster des vérifications supplémentaires. Les chemins /tmp sont nettoyés (rm -rf) avant usage. À vérifier en Phase 5 : droits réels des clés kconf2 et comportement de /tmp/sdk sous course.

### [MOYENNE] `dpkg-wrapper` — debian/control (l. 8-14)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `Package: dpkg-wrapper \| Architecture: all \| Depends: ${shlibs:Depends}, ${misc:Depends} \| Description: a dpkg wrapper. hook some essential options and make it accessible.`
- **Constat** : La source publiée (dpkg-wrapper_1.0.0.0-0k2.1ok1.tar.xz, SHA256 30dbea54… = .dsc:17 et Sources archive.openkylin.top l. 10890) contient 19 fichiers, tous sous debian/, et aucune ligne de code : squelette dh_make (rules:17-18 « %: dh $@ », aucun .install, scripts de mainteneur uniquement en .ex ignorés par debhelper, dpkg-wrapper-docs.docs visant un paquet inexistant, copyright:3-8 non rempli). Construite, elle donnerait un paquet arch:all ne contenant que /usr/share/doc/dpkg-wrapper/{copyright,README.Debian,changelog}, sans champ Depends (substvars vides) et sans script de mainteneur. Le binaire publié dans huanghe/main (archive.openkylin.top huanghe_main_amd64_Packages.gz l. 23544-23558 ; archive.build l. 21763) : dpkg-wrapper_1.0.0.0-0k2.1ok1_all.deb, SHA256 058bb16b…, 12 872 octets, Installed-Size: 63 (l. 23552), Depends: dpkg, libkysdk-conf2-tools (l. 23553) ; il installe /usr/bin/dp […]
- **Scénario** : Un administrateur ou une édition dérivée installe dpkg-wrapper : des scripts root non auditables s'interposent devant dpkg et apt et peuvent modifier les options transmises (forçages, contournements), journaliser ou exécuter d'autres actions à chaque opération de paquet, sans source publiée permettant de vérifier ou de reproduire le binaire.
- **Contre-lecture** : Le code réel vit probablement dans un dépôt interne Kylin (Original-Maintainer sunzhen@kylinos.cn, version de lignée « 0k2 ») et n'a pas été poussé dans l'archive source ; la fonction attendue (intégration du mode ostree immuable, cf. libkysdk-system) est légitime ; le paquet n'est pas installé par défaut.

### [MOYENNE] `glibc` — debian/changelog (l. 1-5)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `glibc (2.43-ok2) huanghe; urgency=medium    * openkylin huanghe更新glibc 2.43   -- wangweiran <wangweiran@kylinos.cn>  Fri, 05 Jun 2026 16:42:33 +0800`
- **Constat** : Retard de correctifs de sécurité (pas un retrait). 2.43-ok2 est calqué sur ubuntu 2.43-2ubuntu2 (31/03/2026). Depuis, Ubuntu resolute-security a publié 2.43-2ubuntu2.3 (22/07/2026) et 2.43-2ubuntu2.4 (03/09/2026) : 17 correctifs, 11 CVE (CVE-2026-4046, -5435, -5450, -5928, -6238, -19499, -19542, -6368, -6791, -77117, -80489 ; changelog ubuntu 2.43-2ubuntu2.4 l. 1-77). Debian sid 2.43-3 à 2.43-6 en corrige 2 autres via la branche stable amont : CVE-2026-8674 (assertion du résolveur DNS, domaine de recherche long) et CVE-2026-18374 (débordement de tas, suffixe ,ccs= de fopen). Absence vérifiée dans la source openKylin : les 17 correctifs Ubuntu s'appliquent encore (code non corrigé) sur l'arbre orig+série reconstitué en mémoire ; libio/fileops.c:342-343 et resolv/resolv_conf.c:284,316 de l'orig ne sont touchés par aucun correctif (debian/patches/series, 86 entrées, aucune CVE-2026-*). L'in […]
- **Scénario** : Une application d'openKylin traite des données non fiables avec une fonction concernée : conversion iconv SHIFT_JISX0213/EUC_JISX0213/IBM1390/1399 (blocage ou abort, déni de service sur un service de conversion de documents) ; scanf « %mc » (débordement d'un octet sur le tas) ; wordexp avec tilde long ou WRDE_APPEND ; strfmon ; tdelete ; ns_sprintrr. Ou bien une configuration DHCP pousse un domaine de recherche très long dans resolv.conf et fait avorter les processus qui résolvent des noms (CVE-2026-8674). Selon le cas : déni de service, corruption mémoire, voire exécution de code si un programme privilégié est exposé.
- **Contre-lecture** : Décalage normal d'une dérivée : ok2 (05/06/2026) était à jour au moment de son import, et les avis sont postérieurs. Plusieurs CVE touchent des fonctions dépréciées ou peu utilisées (ns_sprintrr, strfmon, wordexp), et l'exploitation exige un usage précis de l'API avec des entrées contrôlées. Une mise à jour 2.43-ok3 a pu paraître après l'instantané de l'archive. L'exploitabilité n'a pas été évaluée (aucune exécution).

### [MOYENNE] `grub2` — debian/patches/secure-boot/efi-use-peimage-shim.patch (l. 952-967)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `GRUB_MOD_INIT (peimage) { … exit_orig = grub_efi_system_table->boot_services->exit; grub_efi_system_table->boot_services->exit = exit_hook; } — restauré uniquement dans GRUB_MOD_FINI.`
- **Constat** : openKylin embarque une version antérieure du chargeur peimage d'Ubuntu : le crochet sur BootServices->Exit est posé à l'initialisation du module et n'est retiré qu'au déchargement. C'est le schéma corrigé par Ubuntu 2.12-1ubuntu7 sous CVE-2024-2312 (changelog Ubuntu:7-11 : « Use-after-free in peimage module », SBAT grub.peimage,2, garde dans build-efi-images). openKylin a en outre supprimé cette garde (debian/build-efi-images, lignes « if ! grep -q ^grub.peimage $sbat_csv » d'Ubuntu:37-40 absentes) et n'a pas d'entrée grub.peimage dans sbat.openkylin.csv.in. Atténuation constatée : « peimage » est retiré de la liste des modules des images signées (build-efi-images, cf. Ubuntu:140), mais peimage.mod est livré (ISO SHA256SUMS:222).
- **Scénario** : Si peimage est chargé (image non signée ou future réintroduction dans l'image signée), après sortie de GRUB vers le micrologiciel le pointeur Exit de la table système désigne de la mémoire libérée ; une image ultérieure appelant Exit exécute du code contrôlable → use-after-free pouvant contourner Secure Boot. Révocation ciblée impossible faute d'entrée SBAT grub.peimage.
- **Contre-lecture** : Sous Secure Boot avec vérificateur shim_lock, insmod d'un module externe est refusé et peimage n'est pas dans les images signées : le code vulnérable n'est pas atteignable dans la configuration signée telle que construite. Version simplement non mise à jour plutôt que modification délibérée.

### [MOYENNE] `grub2` — orig/configure.ac (l. 37)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `AC_INIT([GRUB],[2.12~rc1],[bug-grub@gnu.org])  \| orig/ChangeLog:3 « Release 2.12~rc1 » \| racine de l'archive grub2_2.12.orig.tar.xz : « grub-2.12~rc1/ » (mtime 2023-07-10)`
- **Constat** : L'archive amont distribuée par openKylin sous le nom grub2_2.12.orig.tar.xz (sha256 7a60c08b…cd650, conforme à l'index Sources signé) n'est PAS GRUB 2.12 mais la pré-version GRUB 2.12~rc1 du 2023-07-10 : AC_INIT 2.12~rc1 (configure.ac:37, configure, docs/version*.texi, stamp-*), ChangeLog commençant par « Release 2.12~rc1 » et identique octet pour octet à la fin du ChangeLog GNU 2.12 à partir de cette ligne (cmp), NEWS sans section « New in 2.12 », répertoire racine « grub-2.12~rc1/ ». Les 187 fichiers modifiés et 4 absents par rapport à l'archive GNU 2.12 signée (clé BE5C2320…2166), qui est l'orig d'Ubuntu noble et de Debian (même sha256 f3c97391…faa), correspondent exactement, et en sens inverse, aux 108 commits amont publiés entre 2.12~rc1 et 2.12 (ChangeLog GNU lignes 1-1496) : aucun hunk n'est étranger à ces commits. Ce remplacement n'est déclaré nulle part : debian/changelog:164-16 […]
- **Scénario** : Un gestionnaire de vulnérabilités (scanner OVAL/SBOM, suivi CVE) compare la version annoncée « 2.12 » aux versions corrigées amont : CVE-2023-4692 et CVE-2023-4693 (corrigées dans 2.12) sont alors marquées « non affecté » alors que le code livré est vulnérable (voir constat ntfs.c) ; de même pour les autres correctifs rc1→2.12. Le défaut de traçabilité masque donc une exposition réelle du chargeur d'amorçage signé.
- **Contre-lecture** : Calendrier compatible avec une erreur d'empaquetage : openKylin a commencé (2.12-ok1, 02/10/2023) à partir de la lignée Debian/Ubuntu 2.12~rc1-x (debian/grub-common.maintscript.in:2 cite 2.12~rc1-5~, et debian/patches/upstream/unmerged-usr-shebang.patch ne s'applique qu'à rc1), en retirant le suffixe ~rc1 par anticipation, sans réimporter l'archive finale ensuite. Aucun indice de modification intentionnelle : tous les écarts sont du code amont GNU authentique (PROBABLE pour l'identité octet à octet avec grub-2.12~rc1.tar.xz, non vérifiable hors ligne).

### [MOYENNE] `libkysdk-system` — src/packages/libkypackages.c (l. 10573-10655, 11091-11160, 4546-4556)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `gpg --import /usr/share/keyrings/deb-sign/* ; gpg --verify "sig" "plain" ; kdk_verify_signature(uri,dir,typebm,...) pour les .deb marques bare_metal:yes`
- **Constat** : Consommateur identifie de la cle deb-sign (reponse a la question du lot) : libkysdk-system verifie, pour les paquets portant la marque _compat 'bare_metal: yes' (paquets 'privilegies'), une signature detachee (bare_metal_sig) en important /usr/share/keyrings/deb-sign/*.gpg (la 'Kylin Deb Signing Key') dans un trousseau gpg et en verifiant un SHA512 du .deb signe. Chaine de confiance SEPAREE d'APT, specifique a l'installation de paquets 'bare metal' privilegies. 'debsig-verify' est seulement une dependance/recommandation d'APT (dpkg.cfg), non active ici. Lot a auditer en detail dans son propre perimetre.
- **Scénario** : Verification reposant sur des commandes shell (system/popen avec gpg --import ... *) et des fichiers temporaires /tmp/kysdk-package-* a chemins previsibles ; une defaillance de la verif ferait echouer la marque privilegiee. Analyse complete hors perimetre de ce lot (confiance ostree/CA).
- **Contre-lecture** : Mecanisme de signature maison coherent (marquer certains .deb comme privilegies et verifier leur signature Kylin avant installation en mode special) ; la cle deb-sign n'est volontairement PAS une ancre APT globale.

### [MOYENNE] `libkysdk-system` — src/packages/tools/libkypackage_general.c (l. 222-226, 250-253, 327-334, 449)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `strcmp(real_path, "/usr/bin/dpkg_wrapper") != 0 && strcmp(real_path, "/usr/bin/dpkg") != 0 ... WRAPPER_TAG="dpkg_wrapper" ... return "/usr/bin/dpkg.orig";`
- **Constat** : Corroboration de la fonction de dpkg-wrapper (fichier d'un autre lot, lu partiellement : l. 1-60 et 180-461). kypackage_general (paquet libkysdk-package, installé par l'ISO : manifest l. 903 ; Contents l. 47947 ; debian/libkysdk-package.install:11), exécuté en root, n'accepte qu'un parent dont l'exécutable, /usr/bin/dpkg_wrapper ou /usr/bin/apt_wrapper mais aussi /usr/bin/dpkg, /usr/bin/apt ou /usr/bin/apt-get, contient la balise WRAPPER_TAG=dpkg_wrapper/apt_wrapper ; il lance ensuite /usr/bin/dpkg.orig, apt.orig ou apt-get.orig dans un « ns_mode » ostree (libostreemode.so, l. 46, 355). Cette conception suppose qu'une fois dpkg-wrapper installé, /usr/bin/dpkg, apt et apt-get soient les scripts enveloppes, et que les vrais binaires soient détournés en *.orig, ce que seuls les scripts de mainteneur du binaire dpkg-wrapper (sans source, absents du corpus) peuvent faire (dpkg-divert). La des […]
- **Scénario** : Si dpkg-wrapper est installé, chaque invocation root de dpkg ou apt passe d'abord par un script non audité qui remplace le binaire Essential dpkg ; une erreur ou une altération de ce script compromet toutes les installations suivantes (persistance, modification silencieuse des options).
- **Contre-lecture** : Mécanisme plausible d'isolation des gestionnaires de paquets pour le système immuable ostree, hors ISO standard ; le contrôle de balise, de propriétaire root et d'absence d'écriture groupe/autres (l. 228-239) limite l'usurpation de l'appelant.

### [MOYENNE] `lightdm` — debian/patches/0013-23-update-changelog.patch (l. 15-17 (data/pam/lightdm), 28-30 (data/pam/lightdm-autologin))

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `session required pam_env.so readenv=1 user_readenv=1 envfile=/etc/default/locale`
- **Constat** : Sous un titre 'update changelog', le patch ajoute user_readenv=1 a pam_env dans les piles PAM lightdm ET lightdm-autologin, reactivant la lecture du fichier ~/.pam_environment controle par l'utilisateur. Dans PAM 1.7 cette fonction est DEPRECIEE mais toujours fonctionnelle (le fork pam retire meme l'avertissement via pam_env-remove-deprecation-notice-for-user_readenv.patch). La base Debian (05_debianize-pam-files.patch) n'avait que readenv=1.
- **Scénario** : Un utilisateur place dans ~/.pam_environment des variables (p.ex. manipulant l'environnement) lues par pam_env lors de pam_open_session execute en contexte root par session-child avant le drop de privileges; selon les modules/scripts root executes ensuite (session-setup-script), l'environnement injecte pourrait influencer leur comportement.
- **Contre-lecture** : pam_env n'accepte que des paires NAME=VALUE simples et filtre; le fichier appartient a l'utilisateur et la session tourne sous son UID, donc pas d'elevation directe. Ubuntu applique deja user_readenv=1 a sshd; il s'agit d'un choix de compatibilite plus que d'une porte. Exploitabilite reelle PROBABLE, a confirmer selon les scripts root declenches.

### [MOYENNE] `lightdm` — debian/patches/0009-15-update-changelog.patch (l. src/session.c ~969 (session_real_run))

- **Type** : privileges — **Registre** : CONSTATÉ
- **Extrait** : `if (priv->config /*&& session_config_get_allow_greeter (priv->config)*/)  { ... ouvre la socket greeter in-session }`
- **Constat** : Le garde session_config_get_allow_greeter() est mis en commentaire: la socket du greeter in-session (run-dir/<user>/) est desormais ouverte pour TOUTES les sessions, pas seulement celles dont le .desktop declare X-LightDM-Allow-Greeter. Modification livree sous un titre 'update changelog'.
- **Scénario** : Tout processus de la session utilisateur peut dialoguer avec lightdm via le protocole greeter in-session (demarrage de session, deverrouillage), la ou l'amont restreignait ce canal aux sessions explicitement autorisees; surface d'attaque IPC elargie au sein de la session.
- **Contre-lecture** : La socket est creee dans le repertoire run de l'utilisateur (possede/limite a cet UID) et le canal ne confere pas de droits hors de la session de l'utilisateur; necessaire au deverrouillage par ukui-screensaver. Impact confine au perimetre de l'utilisateur.

### [MOYENNE] `linux` — debian/patches/0007-build-loongarch-ext4.patch (l. 3294)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+CONFIG_FORTIFY_SOURCE                          policy<{'amd64': 'y', 'arm64': 'y', 'loong64': 'n'}>`
- **Constat** : Pour loong64, 0007 retire les protections mémoire présentes dans 0001 ou chez Ubuntu : FORTIFY_SOURCE n (l.3294), HARDENED_USERCOPY n (l.3718), SLAB_FREELIST_HARDENED n et SLAB_FREELIST_RANDOM n (l.10133-10134), INIT_ON_ALLOC_DEFAULT_ON n (l.4581), LIST_HARDENED n (l.5552), ZERO_CALL_USED_REGS n (l.13400), RANDOMIZE_KSTACK_OFFSET_DEFAULT n (l.8505), SCHED_STACK_END_CHECK n (l.9256), BUG_ON_DATA_CORRUPTION n (l.1054), KFENCE n (l.5247), BPF_JIT_ALWAYS_ON n (l.899), SECURITY_DMESG_RESTRICT n (l.9437), DEBUG_FS_ALLOW_ALL y (l.2046), MAGIC_SYSRQ_DEFAULT_ENABLE 0x1 (l.5671). Amélioration en sens inverse : BPF_UNPRIV_DEFAULT_OFF n→y (l.907), RANDOMIZE_BASE y.
- **Scénario** : Une vulnérabilité mémoire noyau (débordement de tas ou de pile, use-after-free, fuite d'adresse via dmesg) devient nettement plus simple à exploiter de façon fiable par un utilisateur local loong64 pour obtenir root ; aucune vulnérabilité nouvelle n'est créée par ce seul réglage.
- **Contre-lecture** : Valeurs proches du loongson64_defconfig et des réglages par défaut de l'amont ; le choix vise probablement la performance ou la compatibilité de pilotes hors arbre plutôt qu'un affaiblissement délibéré.

### [MOYENNE] `linux` — debian/patches/0007-build-loongarch-ext4.patch (l. 5515)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+CONFIG_LEGACY_TIOCSTI                          policy<{'amd64': 'n', 'arm64': 'n', 'loong64': 'y', 'riscv64': 'n'}>`
- **Constat** : LEGACY_TIOCSTI vaut n pour amd64/arm64/riscv64 (déjà en 0001:4101), mais 0007 le passe à y pour loong64. L'affirmation d'un autre lot (« LEGACY_TIOCSTI=n ») n'est donc exacte que pour 3 architectures sur 4. LEGACY_PTYS=y et LDISC_AUTOLOAD=y y sont aussi activés pour loong64 (l.5512, l.5331).
- **Scénario** : Sur loong64, un processus moins privilégié partageant le terminal de contrôle d'une session privilégiée (par exemple après su/sudo sans pty dédié) peut injecter des caractères dans l'entrée du shell privilégié et lui faire exécuter des commandes.
- **Contre-lecture** : Valeur par défaut amont historique, utile à quelques outils anciens ; sudo moderne avec use_pty atténue le risque.

### [MOYENNE] `linux` — debian/patches/0007-build-loongarch-ext4.patch (l. 5614)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+CONFIG_LSM policy<{... 'loong64': '"landlock,lockdown,yama,loadpin,safesetid,ipe,bpf"' ...}> ; l.2118 DEFAULT_SECURITY_APPARMOR loong64 n`
- **Constat** : Sur loong64, AppArmor et SELinux sont compilés (0001:6916/6926) mais absents de la liste LSM par défaut (déjà absents en 0001:4148). DEFAULT_SECURITY_DAC=y y figure depuis 0001:1806 ; 0007 ajoute DEFAULT_SECURITY_APPARMOR=n pour loong64 (l.2118), désactive LANDLOCK, LOADPIN, SAFESETID et IPE (l.9439-9447) bien qu'ils figurent dans la liste, et laisse BPF_LSM non défini. Seuls capability et Yama restent actifs. amd64/arm64/riscv64 ont la liste Ubuntu "landlock,lockdown,yama,integrity,apparmor".
- **Scénario** : Sur loong64, les profils AppArmor livrés par la distribution (navigateurs, services réseau) ne sont pas appliqués, sauf si l'administrateur ajoute lsm= à la ligne de commande : un service compromis n'est plus confiné.
- **Contre-lecture** : La liste correspond au défaut amont pour DEFAULT_SECURITY_DAC ; openKylin peut s'appuyer sur un autre mécanisme (kysec) sur ce matériel, mais il est absent de ce delta.

### [MOYENNE] `linux` — debian/patches/0001-update-packaging-config.patch (l. 4511)

- **Type** : durcissement — **Registre** : PROBABLE
- **Extrait** : `+CONFIG_MITIGATION_GDS                           policy<{'amd64': 'n'}>`
- **Constat** : Les annotations amd64 désactivent explicitement CONFIG_MITIGATION_GDS (Gather Data Sampling / « Downfall », CVE-2022-40982), dont le défaut amont est y. Dans l'amont récent, cette option à n fixe la mitigation GDS par défaut à « off », ce qui désactive aussi la mitigation microcode via MSR sur les processeurs Intel concernés. Cette sémantique est connue de l'auditeur mais n'a pu être vérifiée : l'arbre 7.0 est absent.
- **Scénario** : Sur un poste Intel concerné (Core de 6e à 11e génération, Xeon Skylake à Ice Lake) démarré sans gather_data_sampling=force, un utilisateur local ou une VM invitée peut inférer des données de registres vectoriels (clés AES, données d'autres processus ou du noyau).
- **Contre-lecture** : Choix de performance documenté par l'amont (la mitigation coûte sur AVX) ; la valeur a pu être héritée d'une ancienne option GDS_FORCE_MITIGATION=n renommée. Le parc openKylin vise aussi des processeurs non Intel (Hygon, Zhaoxin, Phytium, Loongson).

### [MOYENNE] `linux` — debian/patches/0001-update-packaging-config.patch (l. 9960)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `+do_skip_checks = true   (amd64.mk ; idem arm64.mk l.9988, loong64.mk l.10007, riscv64.mk l.10035)`
- **Constat** : Les quatre fichiers debian.master/rules.d/<arch>.mk posent do_skip_checks = true. Le contrôle « annotations --check » de la .config finale est donc sauté (debian/rules.d/2-binary-arch.mk:43-46), de même que les contrôles de debian/rules:116-118. L'annotation est par ailleurs compactée : seule la section « Annotations without notes » subsiste (0001:287), sans aucune note de justification. Des options critiques n'y sont fixées pour aucune arch. (MODULE_SIG pour amd64/arm64/riscv64, MODULE_SIG_FORCE/ALL, LOCK_DOWN_*, STRICT_KERNEL_RWX, IO_STRICT_DEVMEM, STACKPROTECTOR_STRONG hors loong64) et dépendent des défauts Kconfig. Les nouvelles options de 0008 en « default y » (TCG_HYGON, TCM_HYGON, HYGON_GM, HYGON_PSP2CPU_CMD, TDM_DEV_HYGON, TDM_KERNEL_GUARD ; 0008:2488-3007) entrent donc dans la build sans revue d'annotation.
- **Scénario** : Un rebasage ou un patch qui change un défaut Kconfig (ou ajoute un pilote en default y) peut désactiver silencieusement un durcissement ou activer une interface sensible dans les noyaux livrés, sans faire échouer la construction.
- **Contre-lecture** : Le saut des contrôles est courant pendant la mise au point d'un noyau de développement (7.0) multi-arch ; il peut être temporaire.

### [MOYENNE] `linux` — debian/patches/0008-build-new-version-7.0.0-3.patch (l. 10166-10168)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `+	.name = "ccp_share", +	.fops = &ccp_share_fops, +	.mode = 0666,`
- **Constat** : Sur processeur Hygon, ccp-mdev est compilé dans ccp (ccp-objs, 0008 Makefile) et crée /dev/ccp_share accessible à tous (0666). Tout utilisateur peut d'abord mapper un tampon noyau global partagé entre tous les processus, dont le premier appelant choisit la taille (kzalloc, l.10133 ; remap_pfn_range, l.10148). Il peut aussi, par ioctl MCCP_SHARE_OP_DMA_MAP (l.10009), épingler durablement ses pages (pin_user_pages_fast, FOLL_LONGTERM, sans contrôle RLIMIT_MEMLOCK, l.9563) et les mapper à une IOVA de son choix, sans contrôle de plage, dans le domaine IOMMU partagé par les CCP passés en mode utilisateur. /dev/ccpN (mmap sans contrôle de l'index de BAR ni de la longueur, l.9385-9388) n'a pas de mode explicite. Le changelog annonce « drop mdev support », mais ce code reste compilé.
- **Scénario** : Un utilisateur local non privilégié sur poste ou serveur Hygon peut épuiser la mémoire par épinglage illimité (déni de service). Il peut aussi corrompre l'état de coordination partagé ou préempter des IOVA utilisées par un service crypto privilégié qui pilote le CCP en mode utilisateur : interférence, voire détournement des DMA vers ses propres pages (fuite de données traitées).
- **Contre-lecture** : Code d'origine fournisseur (Hygon, openEuler) prévu pour une bibliothèque crypto multi-processus non root. Le chemin n'existe que si l'IOMMU est actif et le CCP présent. Aucune lecture de mémoire noyau arbitraire n'a été établie.

### [MOYENNE] `linux` — debian/patches/0008-build-new-version-7.0.0-3.patch (l. 11887-11893)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `+static int mmap_psp(...) +	page = virt_to_phys((void *)psp_misc->data_pg_aligned) >> PAGE_SHIFT; +	if (remap_pfn_range(vma, vma->vm_start, page, (vma->vm_end - vma->vm_start),`
- **Constat** : /dev/hygon_psp_config (l.12014, sans .mode, donc root par défaut sauf règle udev) mappe vm_end-vm_start octets à partir d'une seule page allouée, sans contrôle de taille ni de vm_pgoff : un mmap de plusieurs pages expose la mémoire physique noyau adjacente en lecture et écriture. La même page, partagée avec l'espace utilisateur, contient le verrou mb_mutex que le noyau utilise ensuite pour sérialiser les commandes SEV/CSV (sev-dev.c modifié, 0008:15234+). L'ioctl HYGON_PSP_MUTEX_ENABLE attend ce verrou en boucle sans délai (timeout 0, l.11952).
- **Scénario** : Un processus root ou un groupe autorisé par udev obtient lecture et écriture de la mémoire noyau, ce qui contourne le verrouillage (lockdown) actif sur amd64. Un processus détenteur de la page peut aussi bloquer indéfiniment les commandes PSP/SEV (déni de service).
- **Contre-lecture** : Interface réservée à root, destinée aux outils Hygon de gestion du PSP ; présente uniquement sur matériel Hygon.

### [MOYENNE] `linux` — debian/patches/0001-update-packaging-config.patch (l. 10146-10154)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `+	wiphy->flags \|= WIPHY_FLAG_NETNS_OK; ... +	wiphy_net_set(common_wiphy, current->nsproxy->net_ns);`
- **Constat** : Sous le titre « update packaging config », le pilote virt_wifi est modifié : la wiphy unique et globale (common_wiphy) devient déplaçable entre espaces de noms réseau. À chaque création de lien, elle est rattachée de force au netns de l'appelant par wiphy_net_set(), sans passer par cfg80211_switch_netns() qui maintient la cohérence des netdevs. VIRT_WIFI=m sur amd64/arm64/riscv64 (0007:13130), USER_NS=y.
- **Scénario** : Si les espaces de noms utilisateur non privilégiés sont autorisés et le module chargeable automatiquement, un utilisateur local peut créer un netns, y ajouter un lien virt_wifi et y aspirer la wiphy partagée. Il perturbe ainsi le Wi-Fi émulé d'autres conteneurs (par exemple le conteneur Android Kmre) et crée un état cfg80211 incohérent, source potentielle de bogues noyau.
- **Contre-lecture** : Modification fonctionnelle pour fournir un Wi-Fi émulé au conteneur Android dans son netns ; virt_wifi n'est pas chargé par défaut et sa création exige CAP_NET_ADMIN dans le netns.

### [MOYENNE] `network-manager` — ref-p2/openkylin/network-manager_1.54.3-ok5/orig/src/core/supplicant/nm-supplicant-config.c (l. 1586-1600)

- **Type** : durcissement — **Registre** : CONSTATÉ
- **Extrait** : `/* CA path */ path = nm_setting_802_1x_get_ca_path(setting); path = ca_path_override ?: path; if (path) { if (!add_string_val(self, path, "ca_path", FALSE, NULL, error)) ... /* Phase2 CA path */ ... "ca_path2"`
- **Constat** : Correctif de sécurité CVE-2026-19685 ABSENT. La référence Ubuntu resolute-updates 1.54.3-2ubuntu3.1 (deb/patches/CVE-2026-19685.patch, changelog 21/09/2026 « SECURITY UPDATE: Improper certificate validation ») refuse ca-path et phase2-ca-path pour les connexions 802.1X privées. Le fichier openKylin est identique octet pour octet à l'orig Ubuntu SANS correctif (cmp), et aucun patch de la série openKylin ne le touche. Pour une connexion privée, NM transmet les fichiers 802.1X au supplicant sous forme de blobs (lus avec les droits de l'utilisateur) ; un répertoire ne pouvant être un blob, ca-path est passé en chemin brut à wpa_supplicant (root) qui, sans ca-cert, fait confiance à ce seul répertoire et déclare le serveur vérifié. Portée : contournement du confinement des fichiers des profils privés (lecture par root d'un répertoire choisi par l'utilisateur) et ancre de confiance EAP choisie  […]
- **Scénario** : Sur un poste multi-utilisateur, un utilisateur local sans privilège (settings.modify.own : allow_active=yes sans authentification) crée un profil 802.1X privé PEAP/TTLS dont ca-path désigne un répertoire qu'il ne peut pas lire lui-même ou qu'il contrôle ; wpa_supplicant (root) charge les certificats de ce répertoire et signale le serveur comme vérifié, ce qui contourne la règle « blobs uniquement » des connexions privées.
- **Contre-lecture** : Le correctif amont/Ubuntu (21/09/2026) est postérieur à la dernière version openKylin 1.54.3-ok5 (16/09/2026) : simple retard de rétroportage, non un retrait délibéré. Impact limité aux profils privés de l'utilisateur lui-même ; pas d'exploitation à distance.

### [MOYENNE] `network-manager` — debian/control (l. 65-66)

- **Type** : reseau — **Registre** : PROBABLE
- **Extrait** : `Depends: ... adduser, +         dnsmasq, +         dnsmasq-base,  Recommends: ppp, -            dnsmasq-base,`
- **Constat** : Écart openKylin (changelog:3 « 把 dnsmasq 加入 Depends » pour réparer le point d'accès mobile IK8IEU) : dnsmasq-base passe de Recommends à Depends ET le paquet complet « dnsmasq » (démon système, et non le seul binaire utilisé par NM pour les connexions partagées) devient une dépendance stricte. L'ISO 3.0 installe dnsmasq 2.92-ok1 (casper--filesystem.manifest). Sous Debian/Ubuntu, le paquet dnsmasq installe et active un service dnsmasq.service écoutant le port 53 (DNS, DHCP désactivé par défaut). NM n'a besoin que de /usr/sbin/dnsmasq (rules:31 -Ddnsmasq=/usr/sbin/dnsmasq), fourni par dnsmasq-base.
- **Scénario** : Sur chaque poste, un résolveur DNS supplémentaire démarre au boot ; s'il se lie aux interfaces réseau (comportement par défaut de dnsmasq sans bind-interfaces, restreint aux sous-réseaux locaux par --local-service dans l'empaquetage Debian), il devient un point d'entrée réseau exposé au LAN (requêtes, sondage de cache, exposition aux vulnérabilités de dnsmasq), et peut entrer en conflit avec l'instance dnsmasq lancée par NM pour le point d'accès.
- **Contre-lecture** : Si le paquet dnsmasq d'openKylin ne démarre pas le service par défaut, ou si celui-ci échoue faute de port 53 libre (systemd-resolved installé sur l'ISO), aucune exposition supplémentaire. Ce qui manque pour conclure : l'empaquetage dnsmasq 2.92-ok1 (service, /etc/default/dnsmasq, script d'aide) n'est pas dans le corpus.

### [MOYENNE] `openssh` — auth-pam.c (l. 915-923)

- **Type** : privileges — **Registre** : PROBABLE
- **Extrait** : `case PAM_ERROR_MSG: case PAM_TEXT_INFO: *num = 0; free(*info); *info = msg; msg = NULL; ctxt->pam_done = SshPamAgain; sshbuf_free(buffer); return (0);  \|\|  monitor.c:1224-1227: if (ret == 0 && num == 0) sshpam_authok = sshpam_ctxt;  \|\|  monitor.c:1299: int r = sshpam_authok != NULL && sshpam_authok  […]`
- **Constat** : Patch Ubuntu (PR openssh-portable #452, non fusionne en amont) auth-pam-Immediately-report-interactive-instructions-to-c.patch (l.55-61). En amont, un message PAM_TEXT_INFO/PAM_ERROR_MSG etait accumule (boucle continue). Desormais sshpam_query() renvoie 0 avec num=0 des le premier message informatif. Or le moniteur privilegie (monitor.c:1224-1227, code amont inchange) interprete ret==0 && num==0 comme 'PAM a reussi' et arme sshpam_authok ; mm_answer_pam_free_ctx (monitor.c:1299, entree MON_AUTHDECIDE monitor.c:199) renvoie alors 1 = authentifie. Le flux honnete (query -> respond, respond remet sshpam_authok a NULL monitor.c:1264) n'est pas affecte et un client distant seul ne provoque qu'une desynchronisation fatale de SA connexion (auth2.c:338 appelle auth2_challenge_stop, le moniteur attend ensuite MONITOR_REQ_PAM_ACCOUNT que le fils honnete n'envoie pas). Le risque porte sur la garant […]
- **Scénario** : Conditions cumulatives : openssh-server installe (non present sur l'ISO), KbdInteractiveAuthentication yes (non par defaut, sshd_config:86) et UsePAM yes, pile PAM 'auth' emettant un message PAM_TEXT_INFO/PAM_ERROR_MSG avant l'invite (ex. modules 2FA/OTP, pam_faillock, module biometrique openKylin s'il figure dans common-auth - a verifier lot PAM). Un attaquant ayant obtenu une execution de code dans sshd-auth avant authentification (2e etape d'une chaine d'exploitation, p. ex. bogue memoire preauth) envoie au moniteur PAM_INIT_CTX, PAM_QUERY (recoit le message informatif => sshpam_authok arme) puis PAM_FREE_CTX sans PAM_RESPOND : le moniteur conclut 'authentifie' et ouvre une session pour l […]
- **Contre-lecture** : Patch public d'un developpeur Canonical visant l'ergonomie (affichage immediat des instructions PAM, RFC 4256), porte par Ubuntu depuis 2023 ; aucun lien avec Kylin. Exige une compromission prealable du processus sandboxe et une configuration non par defaut ; sans cela, aucun contournement n'est atteignable par un client distant. La faiblesse resulte d'une interaction non anticipee avec une heuristique du moniteur, pas d'une intention.

### [MOYENNE] `ostree` — src/libostree/ostree-gpg-verifier.c (l. 515-549)

- **Type** : crypto — **Registre** : CONSTATÉ
- **Extrait** : `list=g_slist_append(list,"/usr/share/ostree/trusted.gpg.d/"); ..."/opt/system/resource/keyrings/"); ..."/usr/share/keyrings/"); ..."/etc/apt/trusted.gpg.d/");`
- **Constat** : Modification dans le CODE : le trousseau 'global' d'OSTree (amont : seulement /usr/share/ostree/trusted.gpg.d) inclut desormais toutes les cles *.gpg de /usr/share/keyrings/, /etc/apt/trusted.gpg.d/ (cles APT) et /opt/system/resource/keyrings/. OSTREE_GPG_HOME devient additif. La separation signature-APT / signature-commits-OSTree est perdue. Inoperant tant que gpg-verify=false sur les remotes livres. Parcours non recursif : la cle deb-sign (sous-repertoire) n'est pas incluse.
- **Scénario** : Si gpg-verify=true etait active sans trousseau dedie, tout detenteur d'une cle de /etc/apt/trusted.gpg.d ou /usr/share/keyrings (cle PPA, cle de depot tiers ajoute par l'utilisateur) pourrait signer un commit systeme accepte ; un paquet deposant une cle dans /opt/system/resource/keyrings ajoute une ancre de confiance silencieuse.
- **Contre-lecture** : Choix d'ingenierie plausible : reutiliser la cle d'archive openKylin deja distribuee pour signer aussi les commits OSTree sans nouveau paquet de trousseau. Aucune cle n'est ajoutee par ce lot dans /opt/system/resource/keyrings.

### [MOYENNE] `ostree` — src/libostree/ostree-sysroot-deploy.c (l. 3564-3568, 3967-3990)

- **Type** : persistance — **Registre** : CONSTATÉ
- **Extrait** : `sprintf(command,"run-parts --verbose %s >> /var/log/ostree.log 2>&1",dir_path); system(command); // full_sysroot_deployment_directory + /etc/ostree/scripts/post-deploy/`
- **Constat** : A chaque nouveau deploiement, libostree execute en root, dans un pool de threads, les scripts run-parts du repertoire <nouveau deploiement>/etc/ostree/scripts/post-deploy/ (contenu issu du commit telecharge) et /etc/ostree/scripts/post-undeploy/ ; les erreurs sont ignorees. Mecanisme d'execution de code au moment de la mise a jour, dependant de l'authenticite du commit (cf. finding openkylin.conf).
- **Scénario** : Un commit forge (remote HTTP non verifie) ou une alteration de l'arbre deploye place un script dans etc/ostree/scripts/post-deploy/ ; il s'execute en root au deploiement, avant meme le redemarrage.
- **Contre-lecture** : Mecanisme de hooks legitime (equivalent des triggers dpkg pour un systeme immuable) ; sans danger si les commits sont authentifies et si les scripts proviennent uniquement de paquets Kylin de confiance.

### [MOYENNE] `ostree` — debian/self-update/ostree (+ Makefile-ostree.am:142 rpath) (l. 1-13)

- **Type** : persistance — **Registre** : CONSTATÉ
- **Extrait** : `CUSTOM_PATH="/opt/update-module/current/usr/bin/" ; if [ -x "$CUSTOM_PATH/ostree" ]; then exec "$CUSTOM_PATH/ostree" "$@"; else exec /usr/bin/ostree "$@"; fi`
- **Constat** : Wrapper installe en /usr/sbin/ostree (prioritaire dans le PATH root) qui execute le binaire ostree de /opt/update-module/current/ s'il existe ; de plus /usr/bin/ostree est lie avec RPATH vers /opt/update-module/current/usr/lib/<arch>. /opt/update-module est un emplacement d'auto-mise a jour (kylin-system-updater self_upgrade) hors gestion dpkg et hors systeme immuable OSTree.
- **Scénario** : Quiconque peut ecrire dans /opt/update-module/current remplace le binaire/bibliotheque ostree utilise par toutes les operations root de deploiement ; le canal qui alimente /opt/update-module (self-update) devient un chemin de code root a part entiere.
- **Contre-lecture** : Mecanisme assume de mise a jour 'hors-bande' des outils de mise a jour eux-memes sur systeme immuable ; benin si /opt/update-module n'est alimente que par un canal authentifie et n'est inscriptible que par root. Canal self-update non couvert dans ce lot.

### [MOYENNE] `ostree` — src/libostree/ostree-sepolicy.c (l. 405-412)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `//if (enabled)   if(0)     { ... selinux_set_policy_root ... selabel_open ... }`
- **Constat** : Le chargement de la politique SELinux du deploiement est desactive en dur (if(0)). Combine avec la mise en commentaire de sysroot_finalize_selinux_policy et de merge_configuration_from (ostree-sysroot-deploy.c:4886-4892), OSTree n'applique plus d'etiquetage SELinux aux arbres deployes.
- **Scénario** : Sur un systeme ou SELinux serait le mecanisme de confinement, l'absence d'etiquetage a la mise a jour affaiblit l'isolation ; sans objet si openKylin n'utilise pas SELinux en enforcing (kysec/AppArmor a la place).
- **Contre-lecture** : openKylin utilise kysec et non SELinux ; desactiver le code SELinux d'OSTree est alors neutre et evite des erreurs de deploiement. Plausible et coherent avec le reste du fork.

### [MOYENNE] `ostree-extra-utils` — usr/bin/ostree-pkgs-guard (deb) ; etc/dpkg/dpkg.cfg.d/ostree-guard (deb) (l. ostree-guard:1 ; ostree-pkgs-guard:11,17-23,34,128-158,165-177)

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `ostree-guard:1 'pre-invoke "/usr/bin/ostree-pkgs-guard"' ; guard: detect_operation() lit $DPKG_HOOK_ACTION ; is_unshare_mount_ns() -> return 1 (ne bloque pas) ; should_execute : ! live_build && ! chroot && ! livecd && is_normal_mode -> bloque install/remove/... (exit 1)`
- **Constat** : Hook dpkg s'exécutant en root AVANT chaque invocation de dpkg (dpkg can_invoke_hooks exige root sauf --force-not-root ; orig/src/main/main.c:353-367,399-416). Rôle : INTERDIRE install/remove/purge/unpack/configure en « mode normal » (système immuable), pour forcer le passage par les wrappers/kare. Lectures : /proc/cmdline, variables d'env LIVE_BUILD/LB_BUILD_IMAGE, et le mode via gdbus com.kylin.MaintainMode.GetBootMode. Il journalise dans /var/log/ostree.log l'horodatage, le hostname, l'opération et TOUS les arguments ($*), localement (aucun envoi réseau, aucun exec de contenu non fixe). Contournable par conception : dans un espace de noms de montage (unshare -m) is_unshare_mount_ns renvoie 0 et should_execute_operation retourne 1 => l'opération N'EST PAS bloquée ; de même si le service D-Bus MaintainMode est injoignable (boot_mode != normal). Le hook ne durcit donc pas : il peut être n […]
- **Scénario** : Un utilisateur convaincu que le système immuable empêche les modifications de paquets contourne le garde-fou via unshare -m (ou en rendant le bus MaintainMode indisponible) et exécute dpkg ; la garantie d'immuabilité est illusoire. Inversement, le hook lui-même n'ouvre pas de privilège (chemins /usr/bin fixes, root:root).
- **Contre-lecture** : Comportement voulu : le modèle immuable repose sur des couches overlay par espace de noms ; autoriser unshare est délibéré (les wrappers s'y appuient). Le hook est une commodité de politique, pas une frontière de sécurité ; le vrai contrôle est ailleurs (overlay en lecture seule). À confirmer en Phase 5 : tester si un utilisateur non privilégié peut, via unshare, modifier durablement le déploiement actif.

### [MOYENNE] `policykit-1` — debian/patches/12-update-changelog.patch (l. polkitagenthelper-pam.c l.235)

- **Type** : privileges — **Registre** : CONSTATÉ
- **Extrait** : `- if (rc != PAM_SUCCESS)  +if (rc != PAM_SUCCESS && rc != PAM_NEW_AUTHTOK_REQD)`
- **Constat** : L'agent d'authentification polkit (PAM) accepte desormais le code PAM_NEW_AUTHTOK_REQD (mot de passe expire / changement requis) comme un succes de gestion de compte. Un compte dont le mot de passe a expire peut donc franchir une authentification polkit.
- **Scénario** : Un compte expire (mot de passe a changer, ou desactive par expiration) peut neanmoins s'authentifier aupres de polkit pour des actions auth_admin/auth_self, alors que la politique du systeme exige un compte valide.
- **Contre-lecture** : Choix d'ergonomie pour ne pas bloquer un utilisateur au premier login ; mais pam_acct_mgmt renvoie NEW_AUTHTOK_REQD aussi pour des comptes dont l'acces devrait etre restreint.

### [MOYENNE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 37-103 (notamment 60-62, 64, 95))

- **Type** : confiance — **Registre** : CONSTATÉ
- **Extrait** : `collect_directories_from_xdg_data_dirs(): prefix /usr/ \|\| /opt/ ; actions_path = g_build_filename(trimmed,"polkit-1","actions") ; pool_new utilise directories_collected au lieu de la liste fixe`
- **Constat** : Meme mecanisme pour les fichiers .policy (definitions d'actions et autorisations par defaut allow_any/active/inactive). L'amont utilise une liste fixe (/etc, /run, /usr/local/share, PACKAGE_DATA_DIR). Le patch ajoute les repertoires /usr/* et /opt/* issus de $XDG_DATA_DIRS, sans controle de proprietaire/permissions. Filtre plus restreint que rules.d (pas de /var ni /lib) et portee moindre (.policy ne peut pas executer de JS arbitraire, seulement fixer des defauts d'autorisation).
- **Scénario** : Un .policy depose dans un repertoire /opt accessible en ecriture non-root (ou via XDG_DATA_DIRS influencable) declare <allow_active>yes</allow_active> pour une action sensible, relachant le prompt d'authentification pour cette action.
- **Contre-lecture** : Extension d'integration pour applications installees hors prefixe standard ; impact limite car les .policy ne font que definir des defauts et restent subordonnes aux regles JS.

### [MOYENNE] `policykit-1` — debian/patches/12-update-changelog.patch (l. 224-232)

- **Type** : privileges — **Registre** : CONSTATÉ
- **Extrait** : `//pkexec environnement ajoute support fcitx + g_ptr_array_add(saved_env, "XMODIFIERS"); + "@im=fcitx"; + "GTK_IM_MODULE"; + "xim";`
- **Constat** : pkexec construit l'environnement du processus privilegie a partir d'une liste curee MINIMALE (modele de securite cle post-CVE-2021-4034). Le patch y ajoute XMODIFIERS=@im=fcitx et GTK_IM_MODULE=xim a valeurs CONSTANTES (non fournies par l'appelant). Un programme GTK execute via pkexec chargera donc un module de methode d'entree et se couplera a la pile IM/XIM.
- **Scénario** : Une application GTK privilegiee lancee par pkexec avec allow_gui charge le module IM et dialogue avec le daemon fcitx de l'utilisateur non privilegie ; une methode d'entree pouvant lire/injecter des frappes devient une surface d'attaque sur le processus privilegie.
- **Contre-lecture** : Besoin legitime de saisie CJK (fcitx) dans les boites de dialogue pkexec ; valeurs codees en dur donc non detournables vers un module arbitraire, et DISPLAY/XAUTHORITY ne sont transmis que si allow_gui, bornant le risque aux actions GUI explicitement autorisees.

## 4. ostree / ca-certificates — hunks SUSPECT (44)

### `ostree` — `Makefile-ostree.am`

**Lignes avant 137-142 / après 137-146** — RPATH du binaire /usr/bin/ostree vers /opt/update-module/current/usr/lib/<arch>-linux-gnu/ : bibliothèques chargées en priorité depuis un répertoire de mise à jour hors paquet (exécution/persistance root).

_openKylin :_
```
# 自动检测架构并设置路径 
 ARCH := $(shell uname -m) 
 ostree_LDFLAGS = -Wl,-rpath=/opt/update-module/current/usr/lib/$(ARCH)-linux-gnu/
```

### `ostree` — `debian/kylin.conf`

**Lignes nouveau fichier, après 1-3** — Remote 'kylin' url=http://ostree.kylinos.cn/repo/kylin gpg-verify=false (fichier présent mais non installé par ostree.install).

_openKylin :_
```
[remote "kylin"] 
 url=http://ostree.kylinos.cn/repo/kylin 
 gpg-verify=false
```

### `ostree` — `debian/openkylin-kaiming.conf`

**Lignes nouveau fichier, après 1-3** — Remote système 'openkylin-kaiming' url=http://ostree.openkylin.top/openkylin-kaiming/ gpg-verify=false (installé dans /etc/ostree/remotes.d).

_openKylin :_
```
[remote "openkylin-kaiming"] 
 url=http://ostree.openkylin.top/openkylin-kaiming/ 
 gpg-verify=false
```

### `ostree` — `debian/openkylin.conf`

**Lignes nouveau fichier, après 1-3** — Remote système 'openkylin' url=http://ostree.openkylin.top/openkylin/ gpg-verify=false (installé dans /etc/ostree/remotes.d) : canal de mise à jour système en HTTP sans vérification de signature.

_openKylin :_
```
[remote "openkylin"] 
 url=http://ostree.openkylin.top/openkylin/ 
 gpg-verify=false
```

### `ostree` — `debian/ostree-boot.postinst`

**Lignes avant 8-11 / après 8-15** — Crée /etc/ostree/scripts/post-deploy, post-undeploy et /opt/system/resource/update-manifests (répertoires de hooks exécutés en root par libostree).

_openKylin :_
```
mkdir -p /etc/ostree/scripts/post-deploy/ 
 mkdir -p /etc/ostree/scripts/post-undeploy/ 
 mkdir -p /opt/system/resource/update-manifests/
```

### `ostree` — `debian/ostree-update-kernel.py`

**Lignes nouveau fichier, après 1-169** — Script Python root (hook noyau) : déplace /boot/vmlinuz-<ver> vers /boot/ostree/<os>-<bootcsum>/ et réécrit l'entrée BLS du déploiement démarré (linux, initrd, linuxTotal, initrdTotal).

_openKylin :_
```
#!/usr/bin/python3 
 # -- coding: UTF-8 -- 
 import gi
```

### `ostree` — `debian/ostree.install`

**Lignes avant 3-5 / après 3-21** — Installe les remotes openkylin/openkylin-kaiming (http, gpg-verify=false), remotes-conf-dir.conf, hooks noyau zz-ostree-update, /usr/bin/ostree-update-kernel.py, wrapper /usr/sbin/ostree, hook post-deploy update-kysec, manifestes.

_openKylin :_
```
debian/openkylin.conf etc/ostree/remotes.d/ 
 debian/openkylin-kaiming.conf etc/ostree/remotes.d/ 
 debian/ostree-aux.conf etc/ostree/ 
 debian/ostree-aux.conf etc/ostree/
```

### `ostree` — `debian/remotes-conf-dir.conf`

**Lignes nouveau fichier, après 1-11** — Déclare [Level-1] dir=/opt/kylin-software-properties/ostree-conf/ : si ce répertoire est non vide, libostree y lit les remotes à la place de /etc/ostree/remotes.d (cf. ostree-repo.c set_property).

_openKylin :_
```
# "count" in "General" represents the count of directories listed. 
 # Please do not modify "Level-1". The later directory has higher level. 
 # The example for next level is: 
 # [Level-2]
```

### `ostree` — `debian/self-update/ostree`

**Lignes nouveau fichier, après 1-13** — Wrapper /usr/sbin/ostree : exec /opt/update-module/current/usr/bin/ostree s'il est exécutable (prioritaire dans le PATH root), sinon /usr/bin/ostree.

_openKylin :_
```
#!/bin/bash 
 # 自定义路径 
 CUSTOM_PATH="/opt/update-module/current/usr/bin/"
```

### `ostree` — `debian/update-kysec`

**Lignes nouveau fichier, après 1-40** — Hook post-deploy root : ostree checkout du sous-répertoire /var/lib/dpkg/plugins vers le chemin fixe /tmp/plugins puis cp de spro.so et ksaf_label.so vers la var-upper du nouveau déploiement, sans vérification d'erreur ni de propriétaire.

_openKylin :_
```
#!/bin/bash 
 pending_commit=$(ostree admin status | grep "pending" | awk '{print $2}') 
 checkout_commit=$(ostree admin status | grep "pending" | cut -d'.' -f1 | awk '{print $2}')
```

### `ostree` — `debian/zz-ostree-update`

**Lignes nouveau fichier, après 1-25** — Hook noyau /etc/kernel/postinst.d|postrm.d : hors chroot, exige /sysroot/ostree monté puis exécute python3 /usr/bin/ostree-update-kernel.py update en root.

_openKylin :_
```
#!/bin/sh 
 if ischroot ; then 
 echo "ostree install kernel in chroot"
```

### `ostree` — `src/libostree/ostree-bootloader-grub2.c`

**Lignes avant 166-171 / après 182-987** — Bloc Kylin : sous-menus multi-noyaux, titres (override /etc/ostree-release.override via get_os_version), menus de restauration usine/système, lecture APP_SCENE dans /etc/LICENSE ou /etc/.kylin-osinfo, détection matérielle via /usr/sbin/dmidecode -t 2 / -t 1 (sortie complète journalisée : g_debug→syslog et syslog LOG_INFO), popen('file -b ...'), system('dd ... | zcat > ...'), écriture de /boot/efi/EFI/BOOT/OSLoader.cfg (code GreatWall non appelé).

_openKylin :_
```
static const char * 
 _ostree_update_vmlinuz_sub(gchar *output_str, const char *title, const char *quoted_uuid, 
 const char *linuxTotal, const char *initrdTotal) 
 {
```

**Lignes avant 204-228 / après 1067-1137** — Construction des entrées : titres réécrits ('openkylin v3.0 <version>'), sprintf dans des tampons de pile de 100 octets avec des chaînes issues du titre BLS/du commit (débordement possible), menuentry --unrestricted conservé (comme l'amont).

_Amont :_
```
kernel = ostree_bootconfig_parser_get (config, "linux"); 
 g_string_append_printf (
```
_openKylin :_
```
const char *linuxTotal; 
 const char *initrdTotal; 
 char *version_part = NULL; 
 if (grub2_title_override && *grub2_title_override != '\0'
```

**Lignes avant 231-262 / après 1140-1204** — Ligne linux : ajout inconditionnel de GRUB_CMDLINE_LINUX_DEFAULT et GRUB_CMDLINE_LINUX_SECURITY (g_string_append sur NULL possible), video=efifb:off si dmidecode identifie un X1 Carbon, 'factory-restore' ou valeur upgrade.

_Amont :_
```
return glnx_throw (error, "No \"linux\" key in bootloader config");
```
_openKylin :_
```
{ 
 g_free(uuid); 
 g_free(quoted_title); 
 g_free(quoted_uuid);
```

### `ostree` — `src/libostree/ostree-fetcher-curl.c`

**Lignes avant 290-296 / après 296-357** — Réseau : nouvelle gestion des annulations (retrait des poignées curl annulées) et décompte des octets ; pas de changement TLS.

_openKylin :_
```
static void 
 fetcher_unaccount_session_bytes (FetcherRequest *req) 
 { 
 OstreeFetcher *fetcher = req->fetcher;
```

**Lignes avant 323-331 / après 384-396** — Réseau : décompte des octets en cas d'échec / reprise (fetcher_unaccount_session_bytes).

_Amont :_
```
g_task_return_error (task, g_steal_pointer (&req->caught_write_error));
```
_openKylin :_
```
{ 
 fetcher_unaccount_session_bytes (req); 
 g_task_return_error (task, g_steal_pointer (&req->caught_write_error)); 
 }
```

**Lignes avant 366-374 / après 431-456** — Réseau : messages d'erreur HTTP 403/404 personnalisés et décompte des octets.

_Amont :_
```
g_autofree char *response_msg 
 = g_strdup_printf ("Server returned HTTP %lu", response); 
 g_task_return_new_error (task, G_IO_ERROR, giocode, "%s", response_msg);
```
_openKylin :_
```
g_autofree char *response_msg = NULL; 
 GError *error = NULL; 
 if (response == 403) 
 {
```

**Lignes avant 377-382 / après 459-465** — Réseau : décompte des octets en cas d'échec / reprise (fetcher_unaccount_session_bytes).

_openKylin :_
```
fetcher_unaccount_session_bytes (req);
```

**Lignes avant 396-405 / après 479-490** — Réseau : décompte des octets en cas d'échec / reprise (fetcher_unaccount_session_bytes).

_openKylin :_
```
fetcher_unaccount_session_bytes (req); 
 fetcher_unaccount_session_bytes (req);
```

**Lignes avant 434-439 / après 519-525** — Réseau : vérification des annulations à chaque événement/minuterie curl.

_openKylin :_
```
check_cancelled_requests (fetcher);
```

**Lignes avant 463-468 / après 549-555** — Réseau : vérification des annulations à chaque événement/minuterie curl.

_openKylin :_
```
check_cancelled_requests (fetcher);
```

**Lignes avant 556-562 / après 643-665** — Réseau : limitation de débit (CURLOPT_MAX_RECV_SPEED_LARGE, BUFFERSIZE 2048) si max-speed-limit-kilobyte > 0 ; aucune option TLS modifiée.

_openKylin :_
```
//基于curl_running的动态限速逻辑 
 OstreeFetcher *fetcher=req->fetcher; 
 if(fetcher->opt_max_speed_limit>0) 
 {
```

**Lignes avant 951-956 / après 1060-1072** — Réseau : limitation de débit (CURLOPT_MAX_RECV_SPEED_LARGE, BUFFERSIZE 2048) si max-speed-limit-kilobyte > 0 ; aucune option TLS modifiée.

_openKylin :_
```
if(req->fetcher->opt_max_speed_limit>0) 
 { 
 rc = curl_easy_setopt (req->easy, CURLOPT_BUFFERSIZE, 2048L); 
 g_assert_cmpint (rc, ==, CURLM_OK);
```

**Lignes avant 960-966 / après 1076-1082** — Réseau : décompte des octets en cas d'échec / reprise (fetcher_unaccount_session_bytes).

_openKylin :_
```
req->current_size_at_easy_start = req->current_size;
```

### `ostree` — `src/libostree/ostree-gpg-verifier.c`

**Lignes avant 515-532 / après 515-549** — Trousseaux GPG « globaux » : ajoute /opt/system/resource/keyrings/, /usr/share/keyrings/ et /etc/apt/trusted.gpg.d/ à /usr/share/ostree/trusted.gpg.d/ ; OSTREE_GPG_HOME devient additif. Toute clé APT .gpg devient ancre de confiance pour les commits/summary OSTree des remotes sans trousseau propre.

_Amont :_
```
if (global_keyring_path == NULL) 
 global_keyring_path = DATADIR "/ostree/trusted.gpg.d/"; 
 if (g_file_test (global_keyring_path, G_FILE_TEST_IS_DIR)) 
 g_autoptr (GFile) global_keyring_dir = g_file_new_for_path (global
```
_openKylin :_
```
GSList *list = NULL; 
 if (global_keyring_path != NULL) 
 list = g_slist_append (list, global_keyring_path);
```

### `ostree` — `src/libostree/ostree-gpg-verify-result.c`

**Lignes avant 639-676 / après 641-680** — Sortie de vérification GPG : l'identité du signataire (nom/e-mail, date, ID de clé) n'est plus affichée, seulement 'server-signed | Client has valid signature' ; détails en g_debug.

_Amont :_
```
g_string_append_printf (output_buffer, "Signature made %s using %s key ID %s\n", 
 formatted_date_time, pubkey_algo, key_id); 
 if (line_prefix != NULL) 
 g_string_append (output_buffer, line_prefix);
```
_openKylin :_
```
g_debug("Signature made %s using %s key ID %s\n", formatted_date_time, pubkey_algo, key_id); 
 g_string_append_printf (output_buffer, "server-signed "); 
 // if (line_prefix != NULL) 
 //   g_string_append (output_buffer
```

**Lignes avant 768-775 / après 772-803** — Messages d'erreur GPG remplacés par des libellés génériques traduits (perte du détail clé/signataire dans l'erreur) ; codes d'erreur conservés, retour FALSE inchangé.

_Amont :_
```
g_strchomp (buffer->str); 
 g_set_error_literal (error, OSTREE_GPG_ERROR, code, buffer->str);
```
_openKylin :_
```
// g_strchomp (buffer->str); 
 // g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("GPG : Signature verification failed")); 
 switch (code) 
 {
```

### `ostree` — `src/libostree/ostree-gpg-verify-result.h`

**Lignes avant 165-171 / après 165-171** — Constante opaque : OSTREE_GPG_ERROR_NO_SIGNATURE passe de 0 à 48 (décale tout l'enum OstreeGpgError) — rupture d'ABI publique.

_Amont :_
```
OSTREE_GPG_ERROR_NO_SIGNATURE = 0,
```
_openKylin :_
```
OSTREE_GPG_ERROR_NO_SIGNATURE = 48,
```

### `ostree` — `src/libostree/ostree-repo-pull-verify.c`

**Lignes avant 298-305 / après 298-305** — Suppression du préfixe 'Commit <checksum>' des erreurs de vérification GPG (gestion d'erreur de sécurité : diagnostic appauvri, logique inchangée).

_Amont :_
```
const char *error_prefix = glnx_strjoina ("Commit ", checksum); 
 GLNX_AUTO_PREFIX_ERROR (error_prefix, error);
```
_openKylin :_
```
// const char *error_prefix = glnx_strjoina ("Commit ", checksum); 
 // GLNX_AUTO_PREFIX_ERROR (error_prefix, error);
```

### `ostree` — `src/libostree/ostree-repo-pull.c`

**Lignes avant 161-166 / après 189-865** — Calcul de taille avant mise à jour : décompression xz (liblzma) de download_and_install_sizeInfo.txt.xz extrait du commit vers /ostree/repo/tmp/<commit>.file, analyse 'checksum:install|download' (fgets 256, strtoul sur all_size[1] potentiellement NULL), parcours de /ostree/repo/objects, estimation du temps, écriture de métadonnées détachées (install-size, download-size, deploy-estimate-time) ; lit /media/OfflineSource/*/deltas.

_openKylin :_
```
static gchar* find_deltas_dir_with_content(const gchar *base_path, GError **error); 
 static gboolean get_static_delta_sizes (OstreeRepo *self, const gchar *delta_path, guint64 *total_size, guint64 *total_usize, 
 guint6
```

**Lignes avant 3656-3662 / après 4644-4663** — Mode COMMIT_AND_SIZE_ONLY : sous-chemin forcé /usr/etc/ostree/remotes.d/download_and_install_sizeInfo.txt.xz, ou bascule en commit-only si /media/OfflineSource/*/deltas non vide (comportement conditionné par un support amovible).

_Amont :_
```
pull_data->is_commit_only = (flags & OSTREE_REPO_PULL_FLAGS_COMMIT_ONLY) > 0;
```
_openKylin :_
```
pull_data->is_commit_only = (flags & OSTREE_REPO_PULL_FLAGS_COMMIT_ONLY && !((flags & OSTREE_REPO_PULL_FLAGS_COMMIT_AND_SIZE_ONLY) > 0)) > 0; 
 pull_data->is_commit_and_size_only = (flags & OSTREE_REPO_PULL_FLAGS_COMMIT_
```

**Lignes avant 4770-4775 / après 5786-5864** — Après transaction : écriture de métadonnées détachées de taille dans le dépôt local (y compris à partir de 'download-size' du commit ; g_variant_get_string sur valeur possiblement NULL).

_openKylin :_
```
// 处理离线增量更新的元数据写入 
 if (pull_data->is_commit_and_size_only && deltas_dir) 
 { 
 g_debug(" [offline-deltas-update] Writing detached metadata for offline delta update");
```

### `ostree` — `src/libostree/ostree-repo.c`

**Lignes avant 164-169 / après 164-171** — Constante /etc/ostree/remotes-conf-dir.conf.

_openKylin :_
```
#define REMOTES_CONF_DIR_CONF "/etc/ostree/remotes-conf-dir.conf"
```

**Lignes avant 1195-1201 / après 1211-1241** — Propriété remotes-config-dir (propriété de construction, appelée pour tout OstreeRepo) : si /etc/ostree/remotes-conf-dir.conf désigne un répertoire non vide (/opt/kylin-software-properties/ostree-conf/), il REMPLACE le répertoire des remotes (/etc/ostree/remotes.d) — source de confiance des URL/gpg-verify déplacée hors de /etc ; fuite mémoire g_strdup_printf.

_Amont :_
```
self->remotes_config_dir = g_value_dup_string (value);
```
_openKylin :_
```
gboolean find_flag = FALSE; 
 g_autoptr (GKeyFile) dir_config = g_key_file_new (); 
 if (g_key_file_load_from_file (dir_config, REMOTES_CONF_DIR_CONF, G_KEY_FILE_NONE, NULL)) 
 {
```

### `ostree` — `src/libostree/ostree-sepolicy.c`

**Lignes avant 405-411 / après 405-412** — if (enabled) remplacé par if(0) : la politique SELinux du déploiement n'est jamais chargée (pas d'étiquetage SELinux des déploiements).

_Amont :_
```
if (enabled)
```
_openKylin :_
```
//if (enabled) 
 if(0)
```

### `ostree` — `src/libostree/ostree-sysroot-deploy.c`

**Lignes avant 48-53 / après 61-100** — Constantes : /etc/ostree/ostree-aux.conf, répertoires de hooks /etc/ostree/scripts/post-deploy|post-undeploy, /var/opt/system/resource/update-manifests/ ; structures de tâches de fond.

_openKylin :_
```
#define SECONDARY_CONF          "/etc/ostree/ostree-aux.conf" 
 #define GROUP_NAME_DEPLOYMENT   "DEPLOYMENT" 
 #define MAX_NUM                 "max_num"
```

**Lignes avant 2426-2435 / après 2691-2767** — get_release_version : lit <racine>/ostree/deploy/<os>/deploy/<csum>.<serial>/etc/ostree-release avec racines {'/', '/tmp/', '/target/'} (repli sur /tmp, accessible en écriture à tous) ; get_os_version concatène des jetons par strcat dans un tampon calloc(1024) sans borne (débordement de tas possible) ; memset(sizeof(pointeur)).

_Amont :_
```
return ostree_sysroot_write_deployments_with_options (self, new_deployments, &opts, cancellable,
```
_openKylin :_
```
return ostree_sysroot_write_deployments_with_options (self, NULL, new_deployments, &opts, cancellable, 
 char *get_os_version(char *release_version) 
 {
```

**Lignes avant 2466-2471 / après 2798-2804** — get_release_version : lit <racine>/ostree/deploy/<os>/deploy/<csum>.<serial>/etc/ostree-release avec racines {'/', '/tmp/', '/target/'} (repli sur /tmp, accessible en écriture à tous) ; get_os_version concatène des jetons par strcat dans un tampon calloc(1024) sans borne (débordement de tas possible) ; memset(sizeof(pointeur)).

_openKylin :_
```
ostree_dployment_set_osversion(deployment);
```

**Lignes avant 2836-2841 / après 3273-3996** — Bloc Kylin exécuté en root à chaque déploiement : bind mounts (/dev,/sys,/proc,/boot), overlay sur etc du nouveau déploiement, copie de /boot/config-*, fork+chroot+execve mkinitramfs, system('run-parts --verbose <deploy>/etc/ostree/scripts/post-deploy/ >> /var/log/ostree.log') — scripts issus du NOUVEAU déploiement (contenu du commit) exécutés hors chroot —, run-parts /etc/ostree/scripts/post-undeploy, reconstruction DKMS via system('chroot … kare run dkmsContainer dkms build -m %s -v %s …') ave […]

_openKylin :_
```
static int bind_mount(const char *deployment_path, const char *targets[], int target_count) 
 { 
 char target[256]; 
 for (int i = 0; i < target_count; i++)
```

**Lignes avant 3028-3063 / après 4192-4302** — Fin de write_deployments : exécution en pool de threads de la régénération initramfs/DKMS et des scripts post-deploy/post-undeploy + process_manifest ; erreurs des tâches ignorées ; suppression des déploiements excédentaires ; ajout de deploy_datetime dans l'origin.

_Amont :_
```
g_autofree char *msg 
 = g_strdup_printf ("%s; bootconfig swap: %s; bootversion: %s, deployment count change: %i", 
 (bootloader_is_atomic ? "Transaction complete" : "Bootloader updated"), 
 requires_new_bootversion ? "y
```
_openKylin :_
```
if(progress) update_progress_deploy(progress, 83, 100, "Processing...", FALSE); 
 write_detached_metadata_deployment_timestamp(self, repo, revision, "update-initramfs", error); 
 int old_deployments_len = self->deploymen
```

**Lignes avant 3518-3530 / après 4883-4895** — Fusion 3 voies de /etc (merge_configuration_from) et finalisation de la politique SELinux commentées : la configuration locale /etc n'est plus fusionnée par OSTree (remplacée par le mécanisme overlay Kylin).

_Amont :_
```
if (!merge_configuration_from (self, merge_deployment, deployment, deployment_dfd, 
 cancellable, error)) 
 return FALSE; 
 if (!sysroot_finalize_selinux_policy (deployment_dfd, error))
```
_openKylin :_
```
// if (!merge_configuration_from (self, merge_deployment, deployment, deployment_dfd, 
 //                                 cancellable, error)) 
 //    return FALSE; 
 //if (!sysroot_finalize_selinux_policy (deployment_d
```

### `ostree` — `src/ostree/ot-admin-builtin-status.c`

**Lignes avant 138-150 / après 143-200** — ostree admin status : une erreur de vérification GPG n'est plus fatale (affichée puis ignorée, retour TRUE) ; erreurs non-GPG avalées silencieusement.

_Amont :_
```
g_propagate_error (error, g_steal_pointer (&local_error)); 
 return glnx_prefix_error (error, "Deployment %i", 
 ostree_deployment_get_index (deployment));
```
_openKylin :_
```
g_debug ("    GPG: No signatures found\n"); 
 // GPG 验证过程中发生了错误 
 switch (local_error->code) 
 {
```

### `ca-certificates` — `sbin/update-ca-certificates`

**Lignes avant 27-32 / après 27-33** — Nouvelle variable COMPATCERTSDIR=/opt/system/resource/ca-certificates (« compatible certificates path »).

_openKylin :_
```
COMPATCERTSDIR=/opt/system/resource/ca-certificates #compatible certificates path
```

**Lignes avant 162-167 / après 166-184** — Ajout au magasin de confiance système (/etc/ssl/certs + ca-certificates.crt) de TOUT fichier *.crt et *.cer trouvé récursivement (find -L, liens suivis) sous /opt/system/resource/ca-certificates, hors contrôle debconf (/etc/ca-certificates.conf ne peut pas les désactiver).

_openKylin :_
```
fi 
 #Code added:handle certificates under "/opt/system/resource/ca-certificates" 
 if [ -d "$COMPATCERTSDIR" ]
```

## 5. ostree / ca-certificates — hunks À EXAMINER (102)

| Dépôt | Fichier | Lignes | Résumé |
|---|---|---|---|
| ostree | Makefile-boot.am | avant 43-48 / après 43-49 | Ajout de l'unité systemd ostree-boot-bind.service à l'installation (bind mount /sysroot/boot sur /boot en root). |
| ostree | debian/changelog | avant 1-1369 / après 1-391 | Changelog remplacé par 51 entrées Kylin (2022.7-ok1.0 → 2024.5-1ok22update5) ; n'y figurent ni l'élargissement des trousseaux GPG, ni la désactivation SELinux, ni l'exécution de scripts post-deploy des déploiements. |
| ostree | debian/libostree-1-1.symbols | avant 48-53 / après 48-54 | Nouveau symbole public non préfixé commit_download_size_info (API Kylin exportée). |
| ostree | debian/ostree-boot.install | avant 6-11 / après 6-12 | Installe ostree-boot-bind.service. |
| ostree | debian/ostree-update-aux.json | nouveau fichier, après 1-9 | Manifeste listant /etc/ostree/ostree-aux.conf comme fichier forcé lors des mises à jour (process_manifest). |
| ostree | debian/ostree.postinst | nouveau fichier, après 1-13 | postinst : chmod 755 des hooks /etc/kernel/postinst.d\|postrm.d/zz-ostree-update. |
| ostree | debian/rules | avant 26-31 / après 26-32 | --with-gcr : gestion via GCR du code PIN des certificats client TLS (interaction utilisateur) ; pas d'affaiblissement TLS constaté. |
| ostree | debian/rules | avant 94-96 / après 101-107 | override_dh_auto_test vide : la suite de tests (y compris tests GPG/pull) n'est plus exécutée au build. |
| ostree | debian/source/format | avant 1-1 / après 1-1 | Format 3.0 (native) : les modifications Kylin ne sont plus isolées en patchs (delta reconstitué uniquement par diff). |
| ostree | po/zh_CN.po | nouveau fichier, après 1-771 | Traduction zh_CN : 3 incohérences de spécificateurs de format (ex. 'Failed to open directory:%s\n' traduit avec 2 %s) — comportement indéfini dans g_printerr en locale zh_CN. |
| ostree | src/boot/grub2/grub2-15_ostree | avant 55-60 / après 74-97 | Ajout de resume=<swap> et de l'argument noyau krm-upgrade si /boot/krm-upgrade existe (paramètres de ligne de commande noyau). |
| ostree | src/boot/grub2/grub2-15_ostree | avant 62-65 / après 99-104 | Export de GRUB_CMDLINE_LINUX_SECURITY (paramètres de sécurité noyau ajoutés aux entrées ostree). |
| ostree | src/boot/ostree-boot-bind.service | nouveau fichier, après 1-10 | Nouvelle unité root : [ -d /boot/loader ] \|\| mount --bind /sysroot/boot /boot (WantedBy=local-fs.target). |
| ostree | src/boot/ostree-tmpfiles.conf | avant 14-19 / après 14-21 | tmpfiles : d /run/media 0755 et 'D /var/tmp 1777 root root' (vidage de /var/tmp au démarrage, changement de sémantique de persistance). |
| ostree | src/libostree/ostree-bootloader-grub2.c | avant 67-72 / après 79-88 | Constantes : /etc/ostree-release.override et stades RC/Release. |
| ostree | src/libostree/ostree-bootloader-grub2.c | avant 175-185 / après 991-1013 | Variables d'environnement GRUB_CMDLINE_LINUX_DEFAULT/SECURITY/UPGRADE, GRUB_RESTORE_DISABLED, titre forcé ; pas de load_video sur ARM. |
| ostree | src/libostree/ostree-bootloader-grub2.c | avant 267-276 / après 1209-1291 | Ajout des sous-menus 'Historical Version', 'System Restore', 'Factory Restore' (paramètres noyau restore/factory-restore). |
| ostree | src/libostree/ostree-cmdprivate.c | nouveau fichier, après 1-60 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree-cmdprivate.h | nouveau fichier, après 1-44 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree-core.c | avant 1851-1870 / après 1851-1919 | _ostree_parse_delta_name assoupli : accepte des noms de delta 'from-to-<ref>' (segments supplémentaires) ; validation d'entrée modifiée (les checksums restent validés). |
| ostree | src/libostree/ostree-deployment.h | avant 59-64 / après 59-72 | Exporte en API publique get_os_version()/get_release_version() (fonctions à tampons fixes, cf. sysroot-deploy.c). |
| ostree | src/libostree/ostree-repo-checkout.c | avant 180-186 / après 180-242 | Checkout : recopie atime/mtime de l'objet du dépôt sur le fichier extrait (best effort). |
| ostree | src/libostree/ostree-repo-checkout.c | avant 296-301 / après 352-360 | Checkout : recopie atime/mtime de l'objet du dépôt sur le fichier extrait (best effort). |
| ostree | src/libostree/ostree-repo-checkout.c | avant 385-390 / après 444-452 | Checkout : recopie atime/mtime de l'objet du dépôt sur le fichier extrait (best effort). |
| ostree | src/libostree/ostree-repo-checkout.c | avant 991-996 / après 1053-1059 | Mode full-sync : suppression récursive dans la destination de tout élément absent du dirtree. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1078-1084 / après 1141-1160 | Mode UNION_FILES : suppression (rm -rf, sans suivre les liens) d'un non-répertoire existant à la place d'un répertoire. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1125-1130 / après 1201-1209 | Mode full-sync : suppression récursive dans la destination de tout élément absent du dirtree. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1135-1140 / après 1214-1228 | Mode full-sync : suppression récursive dans la destination de tout élément absent du dirtree. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1169-1174 / après 1257-1265 | Mode full-sync : suppression récursive dans la destination de tout élément absent du dirtree. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1182-1187 / après 1273-1310 | Mode full-sync : suppression récursive dans la destination de tout élément absent du dirtree. |
| ostree | src/libostree/ostree-repo-checkout.c | avant 1215-1227 / après 1338-1350 | Plus de forçage de mtime OSTREE_TIMESTAMP sur les répertoires extraits. |
| ostree | src/libostree/ostree-repo-commit.c | avant 232-240 / après 253-263 | Horodatages : les objets bare conservent mtime/atime de la source au lieu de OSTREE_TIMESTAMP (reproductibilité ; pas d'impact de confiance). |
| ostree | src/libostree/ostree-repo-commit.c | avant 282-297 / après 306-343 | Horodatages : les objets bare conservent mtime/atime de la source au lieu de OSTREE_TIMESTAMP (reproductibilité ; pas d'impact de confiance). |
| ostree | src/libostree/ostree-repo-commit.c | avant 861-867 / après 907-934 | Horodatages : les objets bare conservent mtime/atime de la source au lieu de OSTREE_TIMESTAMP (reproductibilité ; pas d'impact de confiance). |
| ostree | src/libostree/ostree-repo-commit.c | avant 1157-1163 / après 1224-1230 | Horodatages : les objets bare conservent mtime/atime de la source au lieu de OSTREE_TIMESTAMP (reproductibilité ; pas d'impact de confiance). |
| ostree | src/libostree/ostree-repo-commit.c | avant 3553-3559 / après 3793-3800 | Gestion du descripteur : fd ouvert transféré au flux (close_fd=TRUE) pour l'exécution en thread. |
| ostree | src/libostree/ostree-repo-commit.c | avant 3791-3801 / après 4079-4156 | Commit : conversion des whiteouts overlayfs (char 0:0) en fichiers .ostree-wh.* (fchown/fchmod sur fichier temporaire). |
| ostree | src/libostree/ostree-repo-commit.c | avant 3863-3872 / après 4221-4260 | Commit : conversion des whiteouts overlayfs (char 0:0) en fichiers .ostree-wh.* (fchown/fchmod sur fichier temporaire). |
| ostree | src/libostree/ostree-repo-pull.c | avant 61-70 / après 68-88 | Constantes : seuil low-speed abaissé à 100 o/s, LOCAL_REPO=/ostree/repo/objects, DELTAS_BASE_DIR=/media/OfflineSource/ (support amovible). |
| ostree | src/libostree/ostree-repo-pull.c | avant 173-178 / après 872-878 | update_progress : nouvel indicateur tmp-bytes-transferred ; arrêt de la boucle de progression sur erreur (is_terminated). |
| ostree | src/libostree/ostree-repo-pull.c | avant 195-206 / après 895-917 | update_progress : nouvel indicateur tmp-bytes-transferred ; arrêt de la boucle de progression sur erreur (is_terminated). |
| ostree | src/libostree/ostree-repo-pull.c | avant 665-670 / après 1376-1384 | Mode COMMIT_AND_SIZE_ONLY : mémorise le checksum du fichier de tailles rencontré dans le dirtree. |
| ostree | src/libostree/ostree-repo-pull.c | avant 1446-1463 / après 2158-2225 | Fonctions publiques non préfixées ostree_get_system_arch / ostree_commit_get_arch (arch déduite de ostree.ref-binding) ; fuite de g_variant_get_child_value. |
| ostree | src/libostree/ostree-repo-pull.c | avant 1596-1601 / après 2358-2378 | Contrôle d'architecture du commit (option arch-verify, désactivée par défaut, non utilisée par kylin-system-updater). |
| ostree | src/libostree/ostree-repo-pull.c | avant 3411-3416 / après 4195-4399 | Lecture de /media/OfflineSource/<x>/deltas et des superblocs de delta statique (signé ou non) pour calculer des tailles — information uniquement. |
| ostree | src/libostree/ostree-repo-pull.c | avant 3585-3590 / après 4570-4576 | Contrôle d'architecture du commit (option arch-verify, désactivée par défaut, non utilisée par kylin-system-updater). |
| ostree | src/libostree/ostree-repo-pull.c | avant 4679-4684 / après 5687-5697 | Reprise de progression : relit /ostree/repo/tmp/<commit>.file et le tmpdir de staging pour estimer les octets déjà téléchargés. |
| ostree | src/libostree/ostree-repo-pull.c | avant 4686-4709 / après 5699-5725 | Boucle principale interrompue si is_terminated. |
| ostree | src/libostree/ostree-repo-static-delta-core.c | avant 86-92 / après 86-92 | ostree_repo_list_static_delta_names : suffixe '-<ref>' ajouté aux noms de delta (parcours des parents de chaque ref) ; return FALSE sans libérer 'out' en cas d'erreur. |
| ostree | src/libostree/ostree-repo-static-delta-core.c | avant 152-163 / après 152-189 | ostree_repo_list_static_delta_names : suffixe '-<ref>' ajouté aux noms de delta (parcours des parents de chaque ref) ; return FALSE sans libérer 'out' en cas d'erreur. |
| ostree | src/libostree/ostree-repo.h | avant 799-804 / après 823-829 | Champ full_sync_repo_data ajouté au milieu de OstreeRepoCheckoutAtOptions (rupture d'ABI de la structure publique). |
| ostree | src/libostree/ostree-repo.h | avant 1110-1115 / après 1135-1141 | Nouveau drapeau public OSTREE_REPO_PULL_FLAGS_COMMIT_AND_SIZE_ONLY (1<<5). |
| ostree | src/libostree/ostree-soup-form.c | nouveau fichier, après 1-140 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree-soup-uri.c | nouveau fichier, après 1-1483 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree-soup-uri.h | nouveau fichier, après 1-147 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree-sysroot-cleanup.c | avant 290-296 / après 308-403 | Nettoyage : suppression récursive des répertoires ostree/pkgs/ovl-* ne correspondant à aucun déploiement (correspondance par sous-chaîne g_strstr_len). |
| ostree | src/libostree/ostree-sysroot-cleanup.c | avant 546-566 / après 655-691 | Nettoyage : erreurs détaillées remplacées par des messages génériques (g_set_error sur erreur déjà positionnée) ; ajout du nettoyage des pkgs overlay. |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 104-109 / après 151-295 | Réutilisation de noyaux dans /boot par nom puis par empreinte MD5 (hardlink vers n'importe quel fichier régulier de /boot/ostree/*-*/ de même MD5). |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 180-185 / après 366-436 | Journal d'horodatages de déploiement écrit en métadonnées détachées du commit ; update_progress_deploy fait g_usleep(700 ms) à chaque étape. |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 1012-1018 / après 1264-1275 | Message d'erreur générique remplaçant l'erreur d'initialisation SELinux. |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 1935-1944 / après 2186-2206 | Installation du noyau : tentative de réutilisation (hardlink) avant copie. |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 2104-2125 / après 2366-2390 | Titre BLS = os_version lue depuis etc/ostree-release du déploiement (au lieu de PRETTY_NAME + version + osname:index). |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 2473-2479 / après 2806-2816 | Erreurs précises écrasées par des messages génériques (g_set_error sur une erreur déjà positionnée : perte du diagnostic). |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 2492-2507 / après 2829-2944 | Erreurs précises (sync, bootloader) écrasées par des messages génériques ; suppression automatique des déploiements excédentaires (max_num de /etc/ostree/ostree-aux.conf, défaut 5) avec boucle 'guint i >= 0' toujours vraie (dépassement d'in […] |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 2900-2906 / après 4060-4067 | revision = csum de new_deployment (déréférence possible de NULL si new_deployments vide). |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 3226-3264 / après 4468-4524 | Initialisation du déploiement : erreurs détaillées remplacées par des messages génériques ; horodatage checkout-tree en métadonnées détachées. |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 3488-3498 / après 4748-4863 | Vérification d'architecture du déploiement par lecture de l'en-tête ELF de usr/bin/ostree (refus si différente). |
| ostree | src/libostree/ostree-sysroot-deploy.c | avant 3554-3574 / après 4919-4950 | Erreurs détaillées (dont échec du scellement immuable) remplacées par des messages génériques. |
| ostree | src/libostree/ostree-sysroot.c | avant 175-183 / après 178-255 | libostree installe à l'initialisation de tout OstreeSysroot un handler qui envoie TOUS les messages debug/message du domaine OSTree vers syslog (LOG_USER) — y compris la sortie dmidecode et les métadonnées de commit journalisées en g_debug. |
| ostree | src/libostree/ostree-sysroot.c | avant 190-195 / après 262-270 | setlocale(LC_ALL, "") appelé dans le constructeur de la bibliothèque (effet de bord sur le processus hôte) ; init gettext et logging. |
| ostree | src/libostree/ostree-sysroot.c | avant 2034-2045 / après 2110-2135 | Hotfix/unlock : le déploiement cloné est écrit avec RETAIN au lieu de NOT_DEFAULT (devient le déploiement par défaut) ; clés 'deploy'/'clone_deploy' ajoutées à l'origin. |
| ostree | src/libostree/ostree-sysroot.c | avant 2230-2235 / après 2320-2330 | Hotfix/unlock : le déploiement cloné est écrit avec RETAIN au lieu de NOT_DEFAULT (devient le déploiement par défaut) ; clés 'deploy'/'clone_deploy' ajoutées à l'origin. |
| ostree | src/libostree/ostree-sysroot.h | avant 121-127 / après 121-127 | API publique modifiée : paramètre OstreeAsyncProgress inséré dans ostree_sysroot_deploy_tree*, simple_write_deployment, write_deployments_with_options, prepare_cleanup (rupture d'ABI) ; nouvelles fonctions publiques non préfixées update_pro […] |
| ostree | src/libostree/ostree-sysroot.h | avant 131-136 / après 131-147 | API publique modifiée : paramètre OstreeAsyncProgress inséré dans ostree_sysroot_deploy_tree*, simple_write_deployment, write_deployments_with_options, prepare_cleanup (rupture d'ABI) ; nouvelles fonctions publiques non préfixées update_pro […] |
| ostree | src/libostree/ostree-sysroot.h | avant 167-173 / après 178-184 | API publique modifiée : paramètre OstreeAsyncProgress inséré dans ostree_sysroot_deploy_tree*, simple_write_deployment, write_deployments_with_options, prepare_cleanup (rupture d'ABI) ; nouvelles fonctions publiques non préfixées update_pro […] |
| ostree | src/libostree/ostree-sysroot.h | avant 189-202 / après 200-213 | API publique modifiée : paramètre OstreeAsyncProgress inséré dans ostree_sysroot_deploy_tree*, simple_write_deployment, write_deployments_with_options, prepare_cleanup (rupture d'ABI) ; nouvelles fonctions publiques non préfixées update_pro […] |
| ostree | src/libostree/ostree-sysroot.h | avant 262-268 / après 273-279 | API publique modifiée : paramètre OstreeAsyncProgress inséré dans ostree_sysroot_deploy_tree*, simple_write_deployment, write_deployments_with_options, prepare_cleanup (rupture d'ABI) ; nouvelles fonctions publiques non préfixées update_pro […] |
| ostree | src/libostree/ostree-tls-cert-interaction.h | nouveau fichier, après 1-43 | Fichier hérité d'une version antérieure d'OSTree (2022.x), absent de l'amont 2024.5 et non référencé par Makefile*.am (code mort non compilé). |
| ostree | src/libostree/ostree.h | avant 42-47 / après 42-48 | Définit la macro _() (gettext) dans l'en-tête public ostree.h (pollution d'espace de noms des consommateurs). |
| ostree | src/libotutil/ot-fs-utils.c | avant 86-92 / après 86-106 | ot_ensure_unlinked_at : supprime aussi un répertoire vide (AT_REMOVEDIR) en cas d'EISDIR. |
| ostree | src/ostree/ot-admin-builtin-deploy.c | avant 92-124 / après 119-160 | Barre de progression console + main loop ; is_cmdline_invoked=TRUE (désactive l'écriture d'horodatages en métadonnées détachées pour la CLI). |
| ostree | src/ostree/ot-builtin-checkout.c | avant 35-40 / après 35-41 | Option --full-sync (union + suppression des fichiers absents du commit dans la destination). |
| ostree | src/ostree/ot-builtin-checkout.c | avant 80-85 / après 81-88 | Option --full-sync (union + suppression des fichiers absents du commit dans la destination). |
| ostree | src/ostree/ot-builtin-checkout.c | avant 139-145 / après 142-148 | Option --full-sync (union + suppression des fichiers absents du commit dans la destination). |
| ostree | src/ostree/ot-builtin-checkout.c | avant 149-154 / après 152-160 | Option --full-sync (union + suppression des fichiers absents du commit dans la destination). |
| ostree | src/ostree/ot-builtin-checkout.c | avant 207-212 / après 213-219 | Option --full-sync (union + suppression des fichiers absents du commit dans la destination). |
| ostree | src/ostree/ot-builtin-pull.c | avant 29-35 / après 29-37 | Option --arch-verify (contrôle d'architecture du commit, désactivé par défaut). |
| ostree | src/ostree/ot-builtin-pull.c | avant 85-90 / après 90-97 | Option --arch-verify (contrôle d'architecture du commit, désactivé par défaut). |
| ostree | src/ostree/ot-builtin-pull.c | avant 370-375 / après 390-397 | Option --arch-verify (contrôle d'architecture du commit, désactivé par défaut). |
| ostree | src/ostree/ot-builtin-trivial-httpd.c | nouveau fichier, après 1-42 | Ancien builtin trivial-httpd (exec ostree-trivial-httpd), non référencé par le build (code mort). |
| ostree | src/ostree/ot-main.c | avant 131-146 / après 132-182 | Toutes les traces g_debug de la CLI ostree sont envoyées à syslog (handler installé sans --verbose) ; priorité non initialisée si niveau inconnu (indice de tableau indéfini). |
| ostree | src/ostree/ot-main.c | avant 532-539 / après 572-579 | Toutes les traces g_debug de la CLI ostree sont envoyées à syslog (handler installé sans --verbose) ; priorité non initialisée si niveau inconnu (indice de tableau indéfini). |
| ostree | src/ostree/ot-main.c | avant 675-681 / après 715-721 | Libellé de sortie GPG ('found N signatures.') — l'identité du signataire n'est plus détaillée (cf. gpg-verify-result.c). |
| ostree | src/switchroot/ostree-remount.c | avant 79-84 / après 91-120 | Au démarrage (root) : bind mount /sysroot/boot → /boot si /boot/loader absent. |
| ostree | src/switchroot/ostree-remount.c | avant 249-253 / après 285-291 | Au démarrage (root) : bind mount /sysroot/boot → /boot si /boot/loader absent. |
| ca-certificates | debian/changelog | avant 1-1440 / après 1-23 | Changelog Debian remplacé par 3 entrées Kylin ; 20240203-ok3 « 支持保存国密证书方案 » (support de l'enregistrement de certificats GuoMi/SM2) — implémenté par le répertoire de compatibilité de update-ca-certificates. |
| ca-certificates | mozilla/certdata.txt | avant 5652-5660 / après 5652-5658 | Suppression des seules lignes de commentaire '# Issuer:' et '# Subject:' (O=TAIWAN-CA, C=TW) des CA TWCA ; objets certificat/trust (CKA_*) inchangés — les CA TWCA restent installées et de confiance. |
| ca-certificates | mozilla/certdata.txt | avant 5750-5758 / après 5748-5754 | Suppression des seules lignes de commentaire '# Issuer:' et '# Subject:' (O=TAIWAN-CA, C=TW) des CA TWCA ; objets certificat/trust (CKA_*) inchangés — les CA TWCA restent installées et de confiance. |
| ca-certificates | mozilla/certdata.txt | avant 7391-7399 / après 7387-7393 | Suppression des seules lignes de commentaire '# Issuer:' et '# Subject:' (O=TAIWAN-CA, C=TW) des CA TWCA ; objets certificat/trust (CKA_*) inchangés — les CA TWCA restent installées et de confiance. |
| ca-certificates | mozilla/certdata.txt | avant 7516-7524 / après 7510-7516 | Suppression des seules lignes de commentaire '# Issuer:' et '# Subject:' (O=TAIWAN-CA, C=TW) des CA TWCA ; objets certificat/trust (CKA_*) inchangés — les CA TWCA restent installées et de confiance. |
| ca-certificates | sbin/update-ca-certificates | avant 50-55 / après 51-59 | Option --compatcertsdir. |
| ca-certificates | sbin/update-ca-certificates | avant 117-123 / après 121-127 | --fresh : purge aussi les liens pointant vers COMPATCERTSDIR. |

## 6. Forks P2 — correctifs SUSPECT et À EXAMINER

### apt (P2-apt)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | bloc réintroduction apt-key (cmdline/apt-key.in, cmdline/CMakeLists.txt:14-20,59, debian/apt.install:10, debian/control:73) | oui | cmdline/apt-key.in, cmdline/CMakeLists.txt | Script apt-key d'apt 2.8.x réinjecté tel quel (diff vide avec /usr/bin/apt-key 2.8.3, entités vendor exceptées). net-update désactivé par défaut. add/update écrivent dans /etc/apt/trusted.gpg, ignoré par APT 3.1.16. Le même bloc restaure apt-extracttemplates (code 2.x amont, mkstemp) dans apt-utils. |
| [À EXAMINER] | bloc compat gcc9 (methods/gpgv.cc:398-408 identifié ; autres non localisés) | oui | methods/gpgv.cc | Aucun std::ranges ne reste dans l'arbre. Le seul bloc réécrit repéré dans le code de vérification est sémantiquement équivalent. Incohérence : CMakeLists.txt:67 impose toujours CMAKE_CXX_STANDARD 23, que gcc 9 ne prend pas en charge. |

### pam, shadow, lightdm (P2-auth-login)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | 0009-15-update-changelog.patch | NON | src/seat.c, src/lightdm.c, src/session-child.c, src/session.c, common/configuration.c, src/vt.c, src/wayland-session.c, src/greeter.c | 1673 insertions sur 28 fichiers C: integration UKUI/Wayland, PAM_BINARY_PROMPT (biometrie/OTP), secondes-login (autologin-user-lock), detection materielle (dmidecode/Kirin/Pangu/HW-cloud), popen ps\|grep root, retrait du garde allow_greeter, defaut greeter-hide-users=false, journalisation syslog |
| [SUSPECT] | 0011-17-update-changelog.patch | NON | src/seat.c, src/session.c | corrections C (cast SESSION(), int p=0) dans seat.c/session.c |
| [SUSPECT] | 0013-23-update-changelog.patch | NON | data/pam/lightdm, data/pam/lightdm-autologin, src/session-child.c | modifie les PAM lightdm/lightdm-autologin: ajoute user_readenv=1 et 'auth sufficient pam_succeed_if user ingroup nopasswdlogin'; retire doublon SIGPIPE |
| [SUSPECT] | 0014-24-update-change-log.patch | NON | src/dm-tool.c | 183 insertions dans src/dm-tool.c: resolution du chemin de seat DisplayManager via logind (GetSession/ListSessions) |
| [À EXAMINER] | pam_env-remove-deprecation-notice-for-user_readenv.patch | oui | modules/pam_env/pam_env.c | retire l'avertissement de depreciation user_readenv (fonction conservee) |
| [À EXAMINER] | ubuntu-enable-listusers.patch | oui | data/lightdm.conf | greeter-hide-users=false, greeter-show-manual-login=false dans data/lightdm.conf |
| [À EXAMINER] | 0010-16-fix-compile-errors.patch | oui | src/logger.h | correction macro syslog_debug dans logger.h |
| [À EXAMINER] | 0012-Xauthority-cookie-ukui-wlcom.patch | oui | src/session-child.c | x_authority_write REPLACE -> SET dans session-child.c |

### coreutils (P2-coreutils)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | 0008-.patch | NON | m4/gnulib-comp.m4 | m4/gnulib-comp.m4 : retire tests/test-readutmp.c de gl_FILE_LIST (-1 ligne). |
| [À EXAMINER] | 0009-.patch | NON | gnulib-tests/Makefile.in | gnulib-tests/Makefile.in (fichier généré) : retire test-readutmp de TESTS et check_PROGRAMS ainsi que sa règle d'édition de liens. Supprime en outre 148 règles .log de tests, les règles génériques .test.log et la cible distdir (+2/-1061). |
| [À EXAMINER] | 0010-.patch | NON | gnulib-tests/Makefile.in | gnulib-tests/Makefile.in : retire les dernières références à test_readutmp (SOURCES, OBJECTS, DEPENDENCIES, LDADD, .Po, DIST_SOURCES, EXTRA_DIST, distclean). |
| [À EXAMINER] | 0011-.patch | NON | gnulib-tests/gnulib.mk | gnulib-tests/gnulib.mk : commente le module readutmp-tests (TESTS, check_PROGRAMS, LDADD, EXTRA_DIST). |
| [À EXAMINER] | 0012-.patch | NON | gnulib-tests/gnulib.mk, tests/local.mk | gnulib-tests/gnulib.mk : retire test-localtime_r et test-localtime_r-mt de TESTS et check_PROGRAMS. tests/local.mk : retire tests/tail/inotify-dir-recreate.sh. |

### gnutls28, nss, libgcrypt20, gnupg2, glib-networking, curl (P2-crypto-libs)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | 0003-Description-Backport-of-status-request-revoked.c-fro.patch | NON | tests/testpkcs11.sh | Ajoute 'skip_if_no_datefudge' au debut de tests/testpkcs11.sh (2 lignes). Aucun fichier status-request-revoked.c, aucun code de bibliotheque. |
| [À EXAMINER] | 2.4.9-ok1.1.patch | oui | README, agent/agent.h, agent/cache.c, agent/divert-tpm2.c, agent/gpg-agent.c, agent/trustlist.c, autogen.sh, common/Makefile.am, common/compliance.c, common/com […] | Consolidation en un seul patch (3 695 lignes, 63 fichiers) de la serie Debian/FreePG 2.4.9 : (1) keyboxd non utilise par defaut (README, common/homedir.c) ; (2) cache de l'agent par minuteries + threads de surveillance (retroportage amont 2.5) ; (3) PR_SET_DUMPABLE=0 pour gpg-agent et scdaemon ; (4) suppression de l'avertissement --supervised, --supervised pour keyboxd, unites systemd --user (sock […] |

### dpkg, dpkg-wrapper (P2-dpkg)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | feat_openKylin.patch | NON | scripts/Dpkg/Vendor/openKylin.pm (nouveau), scripts/Makefile.am, scripts/Makefile.in | Crée scripts/Dpkg/Vendor/openKylin.pm (544 lignes) : classe fournisseur openKylin héritant de Default ; crochets archive-keyrings (trousseau Debian), builtin-build-depends, extend-patch-header (Bug-Debian), update-buildprofiles (noudeb) et update-buildflags avec une politique complète de compilation : -g -O2, LTO sur amd64/arm64/ppc64el/s390x sauf lto-disabled-list (blocs dupliqués), SSP strong, F […] |

### glibc (P2-glibc)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | debian/patches/any/local-revert-bz13979.diff | oui | include/features.h | include/features.h : retire l'avertissement « _FORTIFY_SOURCE requires compiling with optimization » (la fortification reste inactive sans -O, mais silencieusement). — Écart avec ubuntu 2.43-2ubuntu2 : DE FORME uniquement (en-tête gbp pq (From/Date/Subject) ajouté; auteur par défaut « Ubuntu Developers »; lignes diff --git/index ajoutées; numéros de lignes des hunks recalculés; diffstat régénéré). […] |
| [À EXAMINER] | debian/patches/any/local-ldconfig-ignore-ld.so.diff | oui | elf/ldconfig.c | ldconfig ne modifie pas les liens symboliques existants du chargeur dynamique (liste ld_sonames). — Écart avec ubuntu 2.43-2ubuntu2 : DE FORME uniquement (en-tête gbp pq (From/Date/Subject) ajouté; auteur par défaut « Ubuntu Developers »; empreintes index raccourcies; numéros de lignes des hunks recalculés; étiquettes de fonction des hunks ajoutées/modifiées; diffstat régénéré). Effet sur l'arbre  […] |

### grub2, grub2-signed (P2-grub2)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | secure-boot/efi-use-peimage-shim.patch | oui | grub-core/Makefile.core.def, grub-core/loader/efi/peimage.c, include/grub/efi/peimage.h | Chargeur PE interne (peimage) — version ANTÉRIEURE au correctif CVE-2024-2312 d'Ubuntu 2.12-1ubuntu7 : GRUB_MOD_INIT remplace globalement boot_services->exit par exit_hook (lignes 952-961) et ne le restaure qu'au GRUB_MOD_FINI ; crée de nouveaux handles d'image ; pas de contrôle « expected EFI application » sur le sous-système PE. Module non embarqué dans les images signées (build-efi-images), mai […] |
| [SUSPECT] | 0094-changelog-2.12-ok1.4.patch | NON | grub-core/Makefile.core.def, grub-core/kern/efi/mm.c, grub-core/loader/i386/efi/linux.c, grub-core/loader/i386/linux.c, include/grub/efi/efi.h | Ajoute un module linuxefi (ancien chargeur Fedora/Ubuntu 2.02, 383 lignes : vérification de la signature du noyau par shim_lock puis RELECTURE du fichier après vérification), grub_efi_allocate_pages_max, et un détournement de la commande linux i386 vers linuxefi sous Secure Boot. Intégralement annulé par 0095 (vérifié : lignes ajoutées = lignes retirées par 0095). |
| [SUSPECT] | 0095-changelog-2.12-ok1.5.patch | NON | grub-core/Makefile.core.def, grub-core/kern/efi/mm.c, grub-core/loader/i386/efi/linux.c, grub-core/loader/i386/linux.c, include/grub/efi/efi.h, util/grub-mkconf […] | (1) Annule exactement 0094 ; (2) remplace la logique de 10_linux/20_linux_xen/grub-mkconfig_lib par une variante Kylin plus ancienne : version_find_latest/dpkg au lieu du tri, suppression de .old et GRUB_TOP_LEVEL, noyaux *.efi.signed, ajout automatique de resume= (swap de /etc/fstab), entrées init-sysvinit/upstart, sous-menu « Backup and Restore » --unrestricted (si /etc/.bootinfo et kybackup) do […] |
| [SUSPECT] | 0096-changelog-2.12-ok1.8.patch | NON | util/grub-mkconfig_lib.in | grub-mkconfig_lib.in : en chroot avec /ostree, utilise /usr/sbin/grub-ostree-mkrelpath pour les chemins relatifs. Désactivé ensuite par 0132. |
| [SUSPECT] | 0097-changelog-2.12-ok1.9.patch | NON | grub-core/Makefile.core.def, grub-core/commands/efi/tpcm_hygon.c, grub-core/commands/tpcm_hygon.c, include/grub/efi/tpcm_hygon.h, include/grub/err.h, util/grub- […] | Nouveau module tpcm_hygon (x86_64-efi) : vérificateur GRUB (grub_verifier_register) qui transmet chaque fichier ouvert, la ligne de commande noyau et chaque commande GRUB au protocole micrologiciel Hygon C2P (TPCM) ; ouverture par défaut si protocole absent ou désactivé ; boucle hlt si le TPCM refuse ; ajouté de force aux images construites par grub-install et grub-mkstandalone ; nouveau code d'er […] |
| [SUSPECT] | 0100-changelog-2.12-ok1.12.patch | NON | configure.ac, grub-core/Makefile.core.def, grub-core/kern/efi/mm.c, grub-core/lib/loongarch64/relocator.c, grub-core/lib/loongarch64/relocator_asm.S, grub-core/ […] | Première version du chargeur LoongArch « back-compatibility » (ELF + EFI, relocator, BPI) avec modifications de kern/efi/mm.c touchant aussi les autres architectures (GRUB_EFI_MAX_ALLOCATION_ADDRESS). Intégralement annulé par 0101 (vérifié), remplacé par 0103. |
| [SUSPECT] | 0102-changelog-2.12-ok1.13.patch | NON | util/grub-install.c | grub-install.c : plate-forme par défaut loongarch64-efi, fichier BOOTLOONGARCH64.EFI, recherche des images dans le répertoire -signed pour loongarch64-efi. |
| [SUSPECT] | 0130-25-changelog-2.12-ok1.16.patch | NON | util/grub.d/00_header.in | 00_header.in : insère des marqueurs « ###grub-password begin/end » (utilisés par debian/grub-password pour superusers/password_pbkdf2) ; sur aarch64 seulement, en-tête réduit (grubenv, initrdfail, défaut, timeout, recordfail, load_video) et le reste de l'en-tête standard est sauté. Correspond au bug ICL0QJ (mot de passe GRUB non effectif). |
| [SUSPECT] | 0132-27-changelog-2.12-ok1.19.patch | NON | util/grub-mkconfig_lib.in | grub-mkconfig_lib.in : commente la branche ostree introduite par 0096 (retour à grub-mkrelpath). |
| [SUSPECT] | 0133-30-changelog-2.12-ok1.20.patch | NON | util/grub.d/30_uefi-firmware.in | 30_uefi-firmware.in : « exit 0 » en tête — supprime l'entrée de menu « UEFI Firmware Settings ». |
| [À EXAMINER] | secure-boot/loader-framework.patch | oui | grub-core/kern/efi/efi.c, grub-core/loader/efi/chainloader.c, grub-core/loader/efi/linux.c, include/grub/efi/efi.h | Enveloppes grub_efi_load_image/start_image/unload_image et enregistrement d'un chargeur de remplacement. Différence avec Ubuntu : les pointeurs de fonctions de grub_efi_loader_t n'ont pas l'attribut __grub_efi_api (convention d'appel EFI/ms_abi). Si un protocole chargeur de shim était enregistré, l'appel se ferait avec une mauvaise convention (défaut fonctionnel, pas de contournement identifié). |
| [À EXAMINER] | fix-build-error.patch | oui | tests/btrfs_test.in, tests/exfat_test.in, tests/f2fs_test.in, tests/grub_cmd_cryptomount.in, tests/hfs_test.in, tests/hfsplus_test.in, tests/iso9660_test.in, te […] | 17 scripts tests/*.in : « exit 99 » (erreur dure) remplacé par « exit 77 » (test ignoré) quand non-root ou outil absent ; dans grub_cmd_cryptomount.in, les erreurs dures de _testcase deviennent aussi des « skip » ; suppression dans iso9660_test.in des tests de régression « CE loops » et « early CE ». Aucun code livré modifié. |
| [À EXAMINER] | 0092-13-fix-build-error-for-amd64.patch | oui | tests/ext234_test.in, tests/fat_test.in, tests/minixfs_test.in | tests ext234/fat/minixfs : exit 99 -> exit 77. Tests uniquement. |
| [À EXAMINER] | 0103-24-LoongArch-Add-back-compatibility-for-linux-kernel.patch | oui | configure.ac, grub-core/Makefile.core.def, grub-core/kern/efi/mm.c, grub-core/lib/loongarch64/relocator.c, grub-core/lib/loongarch64/relocator_asm.S, grub-core/ […] | Remplace sur loongarch64 le chargeur EFI générique (loader/efi/linux.c) par un chargeur Loongson : noyaux PE démarrés via LoadImage du micrologiciel, noyaux ELF chargés directement (relocator) ; mode BPI construisant la ligne de commande sans grub_create_loader_cmdline ; tas GRUB porté à 256 Mio ; GRUB_EFI_MAX_USABLE_ADDRESS dynamique. |
| [À EXAMINER] | 0131-26-changelog-2.12-ok1.17.patch | NON | po/ar.po, po/kk.po, po/vi.po | Traductions uniquement : supprime po/ar.po (ajouté par 0105), met à jour po/kk.po et po/vi.po ; marque « fuzzy » les traductions vietnamiennes dont les spécificateurs printf étaient incohérents (état final vérifié : aucune entrée non-fuzzy incohérente). |

### linux (P2-linux)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | 0001-update-packaging-config.patch | NON | .gitignore, drivers/android/binder.c, drivers/net/wireless/virtual/virt_wifi.c, drivers/staging/Kconfig, drivers/staging/Makefile, drivers/staging/android/*, de […] | Crée debian.master/ (annotations de 9 342 lignes compactée sans notes, control.stub.in, rules.d/{amd64,arm64,loong64,riscv64}.mk avec do_skip_checks=true et uefi_signed=true, vars.*, reconstruct, modprobe.d) ET modifie du code noyau : binder (txn_security_ctx forcé à false, debug_mask=0), virt_wifi (wiphy globale déplaçable entre netns, wiphy_net_set au newlink, traces retirées), réintroduction du […] |
| [SUSPECT] | 0007-build-loongarch-ext4.patch | NON | Makefile, debian.master/config/annotations, debian.master/control.stub.in, tools/perf/Makefile.config | Remplace la configuration loong64 par un .config complet importé d'un environnement Kylin « ky11 » (CC_VERSION_TEXT) : 5 843 réglages loong64 modifiés. EXT4_FS=y ajouté, mais aussi désactivation de lockdown, MODULE_SIG, IMA/EVM, trousseaux, FORTIFY, HARDENED_USERCOPY, SLAB_FREELIST_*, INIT_ON_ALLOC, KFENCE, LANDLOCK, STRICT_DEVMEM, et activation de LEGACY_TIOCSTI ; BPF_UNPRIV_DEFAULT_OFF et RANDOM […] |
| [SUSPECT] | 0008-build-new-version-7.0.0-3.patch | NON | arch/x86/crypto/*cis*, arch/x86/include/asm/cpufeature*.h, arch/x86/kernel/cpu/hygon.c, arch/x86/kvm/{cpuid.c,reverse_cpuid.h}, crypto/testmgr.*, drivers/char/t […] | ≈15 500 lignes : pilotes Hygon CCP (SM2/SM3/SM4, sm4-xts/gcm), ccp-mdev (/dev/ccp_share 0666, /dev/ccpN), hct.ko (/dev/hct_noiommu, /dev/hct_mig 0666, serveur vsock invité 12345), PSP Hygon (/dev/hygon_psp_config, verrou partagé avec l'espace utilisateur dans sev-dev.c), CSV (/dev/sev → csv_fops, import de certificat HGSC, ring buffer), TDM (/dev/tdm, tdm-kernel-guard via kprobe→kallsyms_lookup_na […] |
| [À EXAMINER] | 0002-build-build-linux-7.0.0-2.patch | NON | drivers/gpu/drm/arise/**, drivers/scsi/leapraid/**, drivers/*/phytium*, sound/soc/phytium/**, kernel/time/clockevents.c, kernel/time/tick-broadcast.c, mm/swap_c […] | ≈252 000 lignes, 522 fichiers : support complet SoC Phytium (bus PIO, GPIO, I2C, I3C, SPI/QSPI, MMC, UART, CAN, MAC, DMA, PWM, hwmon, IPMI BT/KCS, snoop, devfreq, PMU DDR, HDA/I2S/codecs, USB, TCM SE), pilote GPU Glenfly « arise » (≈285 fichiers), pilote SCSI LeapRAID, et retouches du cœur : clockevents/tick-broadcast (correctif amont next_event_forced), mm/swap_cgroup (correctif de déréférencemen […] |
| [À EXAMINER] | 0004-fix-fix-loongarch-and-riscv64-build-error.patch | NON | Makefile, arch/loongarch/kernel/{Makefile,extern.c,ptrace.c,signal.c}, tools/perf/Makefile.config, tools/perf/arch/loongarch/util/Build, debian.master/control.s […] | Makefile (mrproper ne supprime plus debian/), dépendances de build loong64, perf (cible rust loongarch, unwind-libdw) ; MAIS aussi arch/loongarch/kernel/extern.c : 4 EXPORT_SYMBOL_GPL de primitives de signal, et ptrace.c/signal.c : gpr_get, gpr_set, read_user, write_user et setup_rt_frame rendues non statiques. |

### openssh (P2-openssh)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | gssapi.patch | oui | Makefile.in, README.md, auth.c, auth2-gss.c, auth2-methods.c, auth2.c, clientloop.c, configure.ac, gss-genr.c, gss-serv-krb5.c, gss-serv.c, kex-names.c, kex.c,  […] | Echange de cles GSSAPI (RFC 4462 + draft curdle) : nouveaux kexgssc.c/kexgsss.c, cle d'hote 'null' (ssh-null.c), methode userauth gssapi-keyex, requetes moniteur GSSSIGN/GSSUPCREDS, renouvellement de credentials, options GSSAPIKeyExchange/KexAlgorithms/ClientIdentity/ServerIdentity/TrustDns/StoreCredentialsOnRekey ; inclut le correctif Ubuntu CVE-2026-3497 (ssh_packet_disconnect, init de variables […] |
| [À EXAMINER] | restore-tcp-wrappers.patch | oui | configure.ac, sshd-session.c, sshd.8 | Option configure --with-tcp-wrappers ; controle hosts_access() dans sshd-session.c avant authentification ; doc sshd.8. |
| [À EXAMINER] | selinux-role.patch | oui | auth.h, auth2.c, monitor.c, monitor.h, monitor_wrap.c, monitor_wrap.h, openbsd-compat/port-linux.c, openbsd-compat/port-linux.h, platform.c, platform.h, session […] | Analyse 'user/role[:style]' dans auth2.c, transmission du role au moniteur (MONITOR_REQ_AUTHROLE, mm_inform_authserv a 3 champs), contexte SELinux par role pour exec et pty. |
| [À EXAMINER] | user-group-modes.patch | oui | auth-rhosts.c, auth.c, misc.c, misc.h, readconf.c, ssh.1, ssh_config.5 | Nouvelle fonction secure_permissions() remplacant les tests (mode & 022) dans auth-rhosts.c, auth.c, misc.c safe_path, readconf.c : group-writable accepte si le groupe ne compte que le proprietaire ; doc. |
| [À EXAMINER] | debian-config.patch | oui | readconf.c, regress/test-exec.sh, ssh.1, ssh_config, ssh_config.5, sshd_config, sshd_config.5 | Client : ForwardX11Trusted=1, Include ssh_config.d, SendEnv, HashKnownHosts yes, GSSAPIAuthentication yes. Serveur : Include sshd_config.d (bloc de commentaire Ubuntu), KbdInteractiveAuthentication no, UsePAM yes, X11Forwarding yes, PrintMotd no, AcceptEnv LANG LC_* COLORTERM NO_COLOR, sftp /usr/lib/openssh ; tests UsePAM yes ; docs. |
| [À EXAMINER] | restore-authorized_keys2.patch | oui | sshd_config | Commente 'AuthorizedKeysFile .ssh/authorized_keys' dans sshd_config : authorized_keys2 relu. |
| [À EXAMINER] | systemd-socket-activation.patch | oui | configure.ac, sshd.c, sshd-session.c, sshd-auth.c | sshd.c : recuperation LISTEN_FDS/LISTEN_PID, sockets AF_INET/AF_INET6/AF_VSOCK passes par systemd, conservation a la re-execution, option -N transmise a sshd-session (ignoree par sshd-auth), decalage des fds de re-exec ; configure.ac SYSTEMD_SOCKET_ACTIVATION. |
| [À EXAMINER] | auth-pam-Immediately-report-interactive-instructions-to-c.patch | oui | auth-pam.c | sshpam_query() renvoie immediatement (num=0, info=message, pam_done=SshPamAgain) sur PAM_ERROR_MSG/PAM_TEXT_INFO au lieu d'accumuler ; sshpam_respond() renvoie KbdintResultAgain dans cet etat ; pam_done remis a SshPamNone a chaque query. |
| [À EXAMINER] | sshconnect2-Write-kbd-interactive-service-info-and-instru.patch | oui | sshconnect2.c | Client : nom/instruction kbd-int et info de changement de mot de passe affiches via notify_start()/writemsg() (octets bruts) au lieu de logit() (assaini strnvis). |
| [À EXAMINER] | sshd-socket-generator.patch | oui | sshd-socket-generator.c(nouveau), Makefile.in, sshd_config | Nouveau generateur systemd (sshd-socket-generator.c, 314 l.) lisant sshd_config et ecrivant ssh.socket.d/addresses.conf si Port/ListenAddress non defaut ; cible Makefile.in et installation dans systemd_system_generator_dir ; commentaire sshd_config. |

### openssl, openssl1 (P2-openssl)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | openssl/update-changelog-information.patch | NON | crypto/o_str.c, crypto/x509/x509_vfy.c | Corrige un débordement d'entier dans buf2hexstr (CVE-2026-31789) et remplace OPENSSL_free par X509_free dans dane_match_cert (CVE-2026-28387). |
| [SUSPECT] | openssl/update-changelog-information-1.patch | NON | crypto/cms/cms_dh.c, crypto/cms/cms_ec.c, crypto/cms/cms_rsa.c, crypto/x509/x509_vfy.c, providers/implementations/kem/rsa_kem.c, test/certs/cve-2026-28388-*.pem […] | CVE-2026-28388 (delta CRL sans crl_number), CVE-2026-28389 (CMS DH/ECDH via X509_ALGOR_get0), CVE-2026-28390 (OAEP label CMS), CVE-2026-31790 (RSASVE vérifie le retour) + certificats/CRL et tests. |
| [SUSPECT] | openssl/update-changelog-info.patch | NON | crypto/asn1/*, crypto/cmp/cmp_genm.c, crypto/cms/*, crypto/crmf/crmf_lib.c, crypto/pkcs12/p12_mutl.c, crypto/pkcs7/*, providers/implementations/ciphers/*, provi […] | 15 correctifs CVE (changelog ok6) : ASN.1 (a_mbstr, tasn_dec), CMP genm, CMS enc/env/pwri, CRMF, PKCS12 PBMAC1 (clé >= 20 octets), PKCS7 (suppression de la désactivation du rejet implicite RSA, UAF PKCS7_verify), AES-GCM-SIV/OCB/SIV, DH (q du pair), QUIC PATH_CHALLENGE ; tests. |
| [SUSPECT] | openssl/add-fix-cve-infos-of-changelog.patch | NON | include/openssl/ssl.h.in, include/internal/quic_port.h, ssl/quic/quic_impl.c, ssl/quic/quic_port.c, ssl/quic/quic_port_local.h, providers/implementations/cipher […] | Limite de connexions QUIC en attente (256, nouvelle API SSL_VALUE_QUIC_MAX_PENDING_CONNS) ; vérification du tag à Final pour AES-GCM-SIV, OCB et ChaCha20-Poly1305 (EVP_Cipher). |
| [SUSPECT] | openssl/update-changelog-information-2.patch | NON | crypto/cmp/cmp_protect.c, crypto/cmp/cmp_vfy.c, crypto/cms/cms_kari.c, ssl/quic/*, include/internal/quic_*.h, ssl/record/methods/dtls_meth.c, ssl/record/methods […] | 8 correctifs CVE (changelog ok8) : CMP format string ("%s"), cache extraCerts, protectionAlg ; CMS kari (taille du tampon d'unwrap) ; QUIC (refcount qrx, paquets ACK seuls) ; DTLS (file limitée à 16, copie du seul enregistrement) ; t1_lib check_cert_usable avec RPK ; tests. |
| [SUSPECT] | openssl1/4-update-version-number-and-commit-to-trigger-a-new-build.patch | NON | crypto/x509/x509_vfy.c | Correctif de code CVE-2023-0465 dans check_policy() (x509_vfy.c) : boucle depuis i=0 et indicateur cbcalled. |
| [SUSPECT] | openssl1/add-loong64-support.patch | NON | Configurations/10-main.conf, Configurations/20-debian.conf, CHANGES, NEWS, doc/man3/X509_VERIFY_PARAM_set_flags.pod, test/certs/*, test/recipes/25-test_verify.t | Cibles linux64-loong64/debian-loong64, plus CHANGES/NEWS CVE-2023-0465, doc CVE-2023-0466 (X509_VERIFY_PARAM_add0_policy), certificats et tests de politique. |
| [À EXAMINER] | openssl/default-configuration-read-dropins-and-crypto-config.patch | oui | apps/openssl.cnf | Ajoute deux .include dans apps/openssl.cnf (/var/lib/crypto-config/profiles/current/openssl.conf.d et /etc/ssl/openssl.conf.d). |
| [À EXAMINER] | openssl1/add-sw64-support.patch | oui | Configurations/00-base-templates.conf, Configurations/10-main.conf, Configurations/20-debian.conf, config, apps/vms_term_sock.c, crypto/bn/*, crypto/build.info, […] | Cibles sw_64 et asm Alpha CRYPTOGAMS porté (mont, ghash, sha1, cpuid) ; C gardé par __sw_64. |

### aptdaemon, apport (P2-outils-ubuntu)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | 0001-update-new-version.patch | NON | aptdaemon/client.py, aptdaemon/core.py, aptdaemon/errors.py, aptdaemon/gtk3widgets.py, aptdaemon/progress.py, aptdaemon/test.py, aptdaemon/worker/aptworker.py,  […] | Réapplique 6 correctifs Ubuntu non intégrés à l'orig (test-with-timeout, fix-some-deprecated-code, gdk-gtk-versions.py, drop-function-not-in-3.11, setup-py-import-distutils-extra, pep8-to-pycodestyle) et annule 4 modifications Kylin de l'orig : suppression de la méthode D-Bus root non authentifiée SetEnviron (core.py), retrait de SIGHUP ignoré (progress.py), réactivation des refus de suppression d […] |
| [À EXAMINER] | 0005-1-openkylin.patch | NON | apport/packaging_impl/__init__.py, tests/integration/test_report.py, tests/integration/test_ui_cli.py | Remplace la condition de détection de dpkg par une expression toujours vraie ("debian" or "openkylin" in ids) et supprime 3 tests (test_add_gdb_info_damaged, test_add_gdb_info_exe_missing, test_ui_update_view). |
| [À EXAMINER] | 0006-2-test_signal_crashes.py-T-test_modify_after_start.patch | oui | tests/integration/test_signal_crashes.py | Supprime le seul test test_modify_after_start (51 lignes), qui vérifie qu'apport ignore un exécutable modifié après le démarrage du processus. |

### policykit-1 (P2-policykit)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | 12-update-changelog.patch | NON | src/polkitagent/polkitagenthelper-pam.c, src/polkitbackend/polkitbackendactionpool.c, src/polkitbackend/polkitbackendduktapeauthority.c, src/programs/pkexec.c | Ne modifie aucun changelog. Injecte 145 lignes de C dans 4 fichiers du coeur : (1) polkitagenthelper-pam.c accepte PAM_NEW_AUTHTOK_REQD apres pam_acct_mgmt ; (2) polkitbackendactionpool.c ajoute collect_directories_from_xdg_data_dirs (repertoires actions via XDG_DATA_DIRS, prefixes /usr,/opt) + modif LANGUAGE dans _localize ; (3) polkitbackendduktapeauthority.c ajoute append_rules_dirs_from_xdg_da […] |

### network-manager, wpa (P2-reseau)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [À EXAMINER] | network-manager:Translated-using-Weblate-Arabic.patch | NON | po/ar.po, po/ko_KP.po | Crée po/ar.po (14 574 l., 90,9 %) ET po/ko_KP.po (13 176 l., gabarit entièrement non traduit, en-tête zh_CN). |
| [À EXAMINER] | network-manager:CVE-2026-19685.patch (Ubuntu resolute-updates 1.54.3-2ubuntu3.1) | oui | src/core/supplicant/nm-supplicant-config.c, src/core/supplicant/tests/test-supplicant-config.c | ABSENT d'openKylin (ni série, ni orig) : nm-supplicant-config.c identique à l'orig Ubuntu non corrigé. |

### systemd (P2-systemd)

| Classement | Correctif | Titre fidèle | Fichiers amont | Contenu réel |
|---|---|---|---|---|
| [SUSPECT] | 63-fix-install-systemd-pam-config-to-etc-pam-configs-to-m.patch | NON | po/ko_KP.po | Uniquement po/ko_KP.po : 102 msgstr coréens (polkit, pam_systemd_home). Aucun fichier PAM, aucun code. |
| [À EXAMINER] | lp2077538/login-Add-XDG_SESSION_EXTRA_DEVICE_ACCESS-variable-for-ad.patch | oui | src/core/manager.c, src/libsystemd/libsystemd.sym, src/libsystemd/sd-login/sd-login.c, src/login/logind-dbus.c, src/login/logind-dbus.h, src/login/logind-sessio […] | Ajoute un attribut de session extra_device_access (booléen) : lu par pam_systemd depuis l'environnement, transmis via varlink CreateSession, persistant dans l'état de session, exposé en D-Bus (propriété ExtraDeviceAccess) et via sd_session_has_extra_device_access ; D-Bus CreateSession force false. |
| [À EXAMINER] | lp2077538/udev-Grant-sessions-access-to-devices-tagged-with-xaccess.patch | oui | rules.d/73-seat-late.rules.in, src/login/logind-seat.c, src/login/logind-session.c, src/shared/acl-util.c, src/shared/acl-util.h, src/udev/udev-builtin-uaccess. […] | Le builtin udev uaccess accorde des ACL à un ENSEMBLE d'UID (utilisateur actif du siège + sessions extra_device_access) ; devnode_acl prend un Set ; logind déclenche des événements change sur les périphériques xaccess au début/fin de session ; règle 73-seat-late étendue à TAG xaccess. |
| [À EXAMINER] | lp2077538/xaccess-Rework-from-boolean-into-a-list-of-tags.patch | oui | src/libsystemd/libsystemd.sym, src/libsystemd/sd-json/json-util.h, src/libsystemd/sd-login/sd-login.c, src/login/logind-dbus.c, src/login/logind-dbus.h, src/log […] | Transforme extra_device_access en liste d'identifiants (XDG_SESSION_EXTRA_DEVICE_ACCESS séparé par « : ») donnant accès aux périphériques étiquetés xaccess-<id> ; varlink accepte un tableau de chaînes ; sérialisation dans l'état de session par fprintf sans échappement (l.248) ; propriété D-Bus devient « as ». |
| [À EXAMINER] | tmpfiles-remove-duplicate-run-lock-definition.patch | oui | tmpfiles.d/legacy.conf.in | Supprime « d /run/lock 0755 root root - » de legacy.conf.in ; la définition debian.conf « d /run/lock 1777 » devient seule effective. |
| [À EXAMINER] | Translated-using-Weblate-Chinese-Traditional.patch | NON | po/be.po, po/be@latin.po, po/bg.po, po/da.po, po/el.po, po/eu.po, po/gl.po, po/he.po, po/hi.po, po/hr.po, po/hu.po, po/lt.po, po/nl.po, po/pa.po, po/ro.po, po/s […] | Touche 20 fichiers po (19 langues autres que zh_TW) : retire 668 drapeaux « #, fuzzy », réécrit les en-têtes (Last-Translator KevinDuan, Weblate openKylin), modifie 12 msgstr. Active des traductions inexactes de descriptions polkit. Aucun msgid modifié, aucune entrée c-format concernée. |
| [À EXAMINER] | Added-translation-using-Weblate-khb_CN.patch | NON | po/khb_CN.po, po/ms.po, po/th.po, po/vi.po | Crée po/khb_CN.po ET po/ms.po, po/th.po, po/vi.po (gabarits vides). |
| [À EXAMINER] | Added-translation-using-Weblate-ko_KP-1.patch | NON | po/ko_KP.po | ANNULE les 264 traductions ko_KP (retour au gabarit vide, blob 90656e7 -> f4bf15f). |
| [À EXAMINER] | 64-rmove-arm-some-test-for-compile-loongarch-add-pie-for-.patch | NON | src/libsystemd/meson.build, src/network/meson.build, src/test/meson.build, src/udev/meson.build | Commente 11 tests unitaires dans 4 meson.build, pour toutes les architectures ; ne contient pas la partie PIE loongarch (dans debian/rules). |

## 7. Durcissement de la configuration noyau (55 options relevées)

| Option | Valeur openKylin | Attendu (Debian/Ubuntu) | Écart | Preuve |
|---|---|---|---|---|
| CONFIG_SECURITY_LOCKDOWN_LSM | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 passé de y à n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9442 (annotations reconstituée l.8390) |
| CONFIG_SECURITY_LOCKDOWN_LSM_EARLY | amd64=y, arm64=y, loong64=-, riscv64=y ; loong64 '-' (indéfini) depuis 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9443 (annotations reconstituée l.8391) |
| CONFIG_LOCK_DOWN_KERNEL_FORCE_NONE | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) ; LOCK_DOWN_KERNEL_FORCE_INTEGRITY/CONFIDENTIALITY et LOCK_DOWN_IN_EFI_SECURE_BOOT également absents : mode de verrouillage effectif inconnu | y + LOCK_DOWN_IN_EFI_SECURE_BOOT=y (verrouillage automatique sous Secure Boot chez Debian/Ubuntu) | non | non_couvert |
| CONFIG_MODULE_SIG | loong64=n ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; amd64/arm64/riscv64 : PROBABLE y via « select MODULE_SIG if MODULES » de SECURITY_LOCKDOWN_LSM (Kconfig amont non vérifiable) ; loong64 : MODULE_SIG_SHA256=y retiré et MODULE_SIG=n posé par 0007:6376-6378 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:6378 (annotations reconstituée l.5572) |
| CONFIG_MODULE_SIG_FORCE | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) | n (Debian/Ubuntu ; application via lockdown sous Secure Boot) | non | non_couvert |
| CONFIG_MODULE_SIG_ALL | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) ; défaut Kconfig amont y si MODULE_SIG (PROBABLE) | y | non | non_couvert |
| CONFIG_IMA | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:4481 (annotations reconstituée l.3963) |
| CONFIG_EVM | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 ; INTEGRITY_SIGNATURE loong64 n (0007:4728) | y (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:3052 (annotations reconstituée l.2777) |
| CONFIG_BPF_UNPRIV_DEFAULT_OFF | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64 n→y par 0007 (amélioration) ; autres arch. : défaut Kconfig amont y (PROBABLE) | y | non | debian/patches/0007-build-loongarch-ext4.patch:907 (annotations reconstituée l.909) |
| CONFIG_BPF_JIT_ALWAYS_ON | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:899 (annotations reconstituée l.902) |
| CONFIG_STACKPROTECTOR_STRONG | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; amd64/arm64/riscv64 : défaut Kconfig y (PROBABLE) | y | non | debian/patches/0007-build-loongarch-ext4.patch:11338 (annotations reconstituée l.9866) |
| CONFIG_FORTIFY_SOURCE | amd64=y, arm64=y, loong64=n ; absent pour riscv64 (défaut Kconfig, non vérifiable) ; loong64 y→n par 0007 ; riscv64 absent | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:3294 (annotations reconstituée l.2995) |
| CONFIG_RANDOMIZE_BASE | arm64=y, loong64=y, riscv64=y ; absent pour amd64 (défaut Kconfig, non vérifiable) ; amd64 absent (défaut x86 y, PROBABLE) ; loong64 : était désactivé en 0001 via CMDLINE="nokaslr" (0001:1341), corrigé par 0007 | y | non | debian/patches/0007-build-loongarch-ext4.patch:8502 (annotations reconstituée l.7501) |
| CONFIG_RANDOMIZE_KSTACK_OFFSET | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) | y | non | debian/patches/0007-build-loongarch-ext4.patch:8504 (annotations reconstituée l.7503) |
| CONFIG_RANDOMIZE_KSTACK_OFFSET_DEFAULT | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:8505 (annotations reconstituée l.7504) |
| CONFIG_HARDENED_USERCOPY | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:3718 (annotations reconstituée l.3391) |
| CONFIG_INIT_ON_ALLOC_DEFAULT_ON | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:4581 (annotations reconstituée l.4055) |
| CONFIG_INIT_ON_FREE_DEFAULT_ON | loong64=n ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) | n | non | debian/patches/0007-build-loongarch-ext4.patch:4582 (annotations reconstituée l.4056) |
| CONFIG_SLAB_FREELIST_HARDENED | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:10133 (annotations reconstituée l.8872) |
| CONFIG_SLAB_FREELIST_RANDOM | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:10134 (annotations reconstituée l.8873) |
| CONFIG_STRICT_KERNEL_RWX | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) ; généralement non désactivable (ARCH_HAS_STRICT_KERNEL_RWX) ; STRICT_MODULE_RWX également absent | y | non | non_couvert |
| CONFIG_STRICT_DEVMEM | loong64=n ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64=n alors que DEVMEM=y (0007:2136) et lockdown absent | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:11384 (annotations reconstituée l.9912) |
| CONFIG_IO_STRICT_DEVMEM | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) ; absent : sans valeur explicite, défaut Kconfig amont n (PROBABLE) — écart possible non confirmable | y (Ubuntu) | non | non_couvert |
| CONFIG_LEGACY_TIOCSTI | amd64=n, arm64=n, loong64=y, riscv64=n ; l'affirmation « =n » d'un autre lot n'est vraie que pour amd64/arm64/riscv64 ; loong64=y (0007) | n | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5515 (annotations reconstituée l.4901) |
| CONFIG_DEVMEM | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) | y | non | debian/patches/0007-build-loongarch-ext4.patch:2136 (annotations reconstituée l.2010) |
| CONFIG_DEVKMEM | non_couvert (absent des annotations pour toutes les arch. ; valeur = défaut Kconfig/select, non vérifiable sans arbre amont) | sans objet (supprimé en amont depuis 5.13) | non | non_couvert |
| CONFIG_KEXEC | amd64=y, arm64=y, loong64=y, riscv64=y | y | non | debian/patches/0001-update-packaging-config.patch:3870 (annotations reconstituée l.4653) |
| CONFIG_KEXEC_SIG | amd64=y, arm64=y ; absent pour loong64/riscv64 (défaut Kconfig, non vérifiable) ; KEXEC_BZIMAGE_VERIFY_SIG amd64=y ; loong64 KEXEC_FILE=y sans KEXEC_SIG | y (amd64/arm64) | non | debian/patches/0001-update-packaging-config.patch:3876 (annotations reconstituée l.4661) |
| CONFIG_HIBERNATION | amd64=y, loong64=y, riscv64=y ; absent pour arm64 (défaut Kconfig, non vérifiable) ; arm64 absent | y | non | debian/patches/0001-update-packaging-config.patch:2853 (annotations reconstituée l.3507) |
| CONFIG_PROC_KCORE | amd64=y, arm64=y, loong64=y, riscv64=y | y | non | debian/patches/0001-update-packaging-config.patch:5980 (annotations reconstituée l.7268) |
| CONFIG_DEBUG_FS | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64 DEBUG_FS_ALLOW_ALL=y (0007:2046) | y | non | debian/patches/0007-build-loongarch-ext4.patch:2045 (annotations reconstituée l.1930) |
| CONFIG_KPROBES | loong64=n ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64=n (plus restrictif) ; amd64/arm64/riscv64 absents mais BPF_KPROBE_OVERRIDE=y implique KPROBES=y (PROBABLE) | y | non | debian/patches/0007-build-loongarch-ext4.patch:5257 (annotations reconstituée l.4719) |
| CONFIG_SECURITY_SELINUX | amd64=y, arm64=y, loong64=y, riscv64=y | y | non | debian/patches/0001-update-packaging-config.patch:6926 (annotations reconstituée l.8396) |
| CONFIG_SECURITY_APPARMOR | amd64=y, arm64=y, loong64=y, riscv64=y ; compilé partout mais absent de la liste LSM loong64 | y | non | debian/patches/0001-update-packaging-config.patch:6916 (annotations reconstituée l.8378) |
| CONFIG_SECURITY_YAMA | amd64=y, arm64=y, loong64=y, riscv64=y | y | non | debian/patches/0001-update-packaging-config.patch:6932 (annotations reconstituée l.8408) |
| CONFIG_LSM | amd64="landlock,lockdown,yama,integrity,apparmor", arm64="landlock,lockdown,yama,integrity,apparmor", loong64="landlock,lockdown,yama,loadpin,safesetid,ipe,bpf", riscv64="landlock,lockdown,yama,integrity,apparmor" ; loong64 sans apparmor ni integrity ; landlock/loadpin/safesetid/ipe listés mais =n, BPF_LSM absent pour loong64 | "landlock,lockdown,yama,integrity,apparmor" (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5614 (annotations reconstituée l.4985) |
| CONFIG_DEFAULT_SECURITY_APPARMOR | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n, DEFAULT_SECURITY_DAC=y | y (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:2118 (annotations reconstituée l.1994) |
| CONFIG_SECURITY_LANDLOCK | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9440 (annotations reconstituée l.8388) |
| CONFIG_USER_NS | amd64=y, arm64=y, loong64=y, riscv64=y | y | non | debian/patches/0001-update-packaging-config.patch:9051 (annotations reconstituée l.10845) |
| CONFIG_SECURITY_DMESG_RESTRICT | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9437 (annotations reconstituée l.8385) |
| CONFIG_LIST_HARDENED | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5552 (annotations reconstituée l.4930) |
| CONFIG_ZERO_CALL_USED_REGS | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64=n | y (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:13400 (annotations reconstituée l.11536) |
| CONFIG_SCHED_STACK_END_CHECK | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9256 (annotations reconstituée l.8230) |
| CONFIG_BUG_ON_DATA_CORRUPTION | loong64=n ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64 y→n par 0007 | y (Ubuntu) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:1054 (annotations reconstituée l.1016) |
| CONFIG_KFENCE | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5247 (annotations reconstituée l.4712) |
| CONFIG_MITIGATION_GDS | amd64=n ; absent pour arm64/loong64/riscv64 (défaut Kconfig, non vérifiable) ; amd64=n explicite depuis 0001 : mitigation Downfall/GDS désactivée par défaut (sémantique amont, PROBABLE) | y (défaut amont) | **OUI** | debian/patches/0001-update-packaging-config.patch:4511 (annotations reconstituée l.5441) |
| CONFIG_X86_KERNEL_IBT | amd64=n ; absent pour arm64/loong64/riscv64 (défaut Kconfig, non vérifiable) ; amd64=n explicite depuis 0001 | y | **OUI** | debian/patches/0001-update-packaging-config.patch:9514 (annotations reconstituée l.11410) |
| CONFIG_SECONDARY_TRUSTED_KEYRING | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:9416 (annotations reconstituée l.8373) |
| CONFIG_SYSTEM_BLACKLIST_KEYRING | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:11449 (annotations reconstituée l.9978) |
| CONFIG_TRUSTED_KEYS | amd64=y, arm64=y, loong64=n, riscv64=y ; loong64 y→n par 0007 | y | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:11746 (annotations reconstituée l.10335) |
| CONFIG_MAGIC_SYSRQ_DEFAULT_ENABLE | amd64=0x01b6, arm64=0x01b6, loong64=0x1, riscv64=0x01b6 ; loong64=0x1 (toutes fonctions SysRq) | 0x01b6 | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5671 (annotations reconstituée l.5036) |
| CONFIG_LDISC_AUTOLOAD | amd64=n, arm64=n, loong64=y, riscv64=n ; loong64=y | n (valeur amd64/arm64/riscv64 du même fichier) | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5331 (annotations reconstituée l.4773) |
| CONFIG_LEGACY_PTYS | amd64=n, arm64=n, loong64=y, riscv64=n ; loong64=y | n | **OUI** | debian/patches/0007-build-loongarch-ext4.patch:5512 (annotations reconstituée l.4899) |
| CONFIG_MODULE_FORCE_LOAD | loong64=y ; absent pour amd64/arm64/riscv64 (défaut Kconfig, non vérifiable) ; loong64=y depuis 0001 (chargement forcé ignorant vermagic) | n | **OUI** | debian/patches/0001-update-packaging-config.patch:4618 (annotations reconstituée l.5570) |
| CONFIG_CMDLINE | arm64="console=ttyAMA0", loong64="" ; absent pour amd64/riscv64 (défaut Kconfig, non vérifiable) ; loong64 "nokaslr" en 0001 (0001:1341) remplacé par "" en 0007 | "" / non défini | non | debian/patches/0007-build-loongarch-ext4.patch:1400 (annotations reconstituée l.1324) |

