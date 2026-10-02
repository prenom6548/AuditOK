# Lot B05-installateur-ostree-P3

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-os-installer`

- **Nature** : propre-kylin — **Amont** : Kylin (paquet natif 3.0 (native), mainteneur Kylin Developers <kylin@kylinos.cn>, version 0.2.2-0k1.16 huanghe ; aucun Vcs déclaré)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 828 fichiers) ; Gitee et GitHub PRESENT (13 branches dont openkylin/huanghe, relation DIVERGENT) non acquis en phase 0
- **Langages / taille** : C++/Qt (~31 kLoC .cpp, 281 fichiers .cpp/.h, 14 plugins + Installer_main) ; Shell/Python (66 scripts sous scripts/ et data/, dont preinstall-scan.py) ; 'TypeScript' de l'extracteur = fichiers de traduction Qt .ts (XML) — 828 fichiers ; 44,6 Mo dans l'arbre (majorité SVG/JPG/PNG, dont 2 cartes de fuseaux de 6,8 Mo) ; taille_source 19,6 Mo — lignes de code propres mesurées : 40 883
- **Privilège** : root : service systemd sans User= dans l'environnement live, scripts lancés via sudo, scripts in_chroot exécutés dans le chroot du système cible ; unités OEM root au premier démarrage du système installé
- **Déclencheurs** : démarrage live avec boot=casper (kylin-os-installer.service, graphical.target), session lightdm start-kylin-os-installer (fichier 96-kylin-os-installer.conf), lancement manuel via kylin-os-installer.desktop, premier démarrage OEM (oem-config.target, oem-name.service, delete-oem-user.service), paramètres noyau lus dans /proc/cmdline
- **Indices réseau** : QLocalServer signalé uniquement dans language/qt_*.ts : contexte de traduction Qt (location ../src/network/socket/qlocalserver.cpp), pas un serveur; grep QLocalServer\|QTcpServer\|QUdpSocket\|listen(\|bind( sur src/, scripts/, data/ : aucun résultat; plugin KNetConnect lié à KF5 NetworkManagerQt (client NetworkManager); clients D-Bus système (PluginService/dbus/uniauthservice.cpp, KUserRegister/userregisterframe.cpp -> org.freedesktop.UPower); URLs APT http:// dans scripts/in_chroot/96-resetup-sourceslist
- **Blobs** : 1 blob : archive zip de 885 o (src/plugins/KUserRegister/res/svg/公用-右箭头_slices.zip) ; aucun ELF
- **Budget proposé** : L2 — 55 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre volumineux (~31 kLoC C++ + 66 scripts) exécuté en root qui fixe la configuration de sécurité du système installé (sudoers, sources APT, services) : lecture exhaustive de scripts/ et data/ et des plugins KUserRegister, KPartition, KNetConnect, PluginService, KInstallAPP, triage du reste de l'UI.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:debian/kylin-os-installer.kylin-os-installer.service` | ConditionKernelCommandLine=boot=casper ; Type=oneshot ; Restart=on-failure ; ExecStart=/usr/share/kylin-os-installer/data/start-kylin-os-installer ; pas de User= (root) ; WantedBy=graphical.target ; debian/rules : override_dh_installsystemd 'dh_installsystemd --name kylin-os-installer' | CONSTATÉ |
| systemd-service | `archive:data/oem-name.service` | Type=oneshot ; ExecStart=/usr/share/kylin-os-installer/data/oem-name ; WantedBy=multi-user.target ; copié dans /lib/systemd/system puis '/bin/systemctl enable oem-name.service' par scripts/in_chroot/99setup-oem-service l.115-117 | CONSTATÉ |
| systemd-service | `archive:data/delete-oem-user.service` | Type=notify ; ExecStart=/usr/sbin/delete-oem-user.sh ; After=lightdm.service ; WantedBy=multi-user.target ; copié puis 'systemctl disable' par 99setup-oem-service l.123-125 ; 'systemctl start delete-oem-user.service' dans scripts/oem.sh l.444 | CONSTATÉ |
| systemd-service+target | `archive:data/oem-config.service ; archive:data/oem-config.target` | ExecStart=/bin/sh -ec ... exec /usr/bin/kylin-os-installer-oem $debug $automatic, options lues dans /proc/cmdline (debug-oem-config, ubiquity/frontend=, automatic-oem-config) ; TTYPath=/dev/tty1 ; WantedBy=oem-config.target ; target AllowIsolate=yes, Wants=oem-config.service network-manager.service getty-static.service systemd-user-sessions.service ; texte quasi identique aux unités oem-config d'ubiquity ; mécanisme d'installation de ces unités non identifié (absent de debian/rules) | CONSTATÉ |
| session-lightdm | `archive:data/96-kylin-os-installer.conf ; archive:data/start-kylin-os-installer.desktop` | [Seat:*] user-session=start-kylin-os-installer ; session Exec=kylin-wlcom -s kylin-os-installer-wrapper ; start-kylin-os-installer supprime ce fichier lightdm sur CPU kirin 990/9a0 et PANGU M900 | CONSTATÉ |
| desktop-file | `archive:kylin-os-installer.desktop` | Exec=/usr/bin/kylin-os-installer-live (script utilisant sudo umount, sudo xmodmap) | CONSTATÉ |
| desktop-file | `archive:data/oem-qc.desktop ; archive:data/oem-unlock.desktop` | oem-qc : Exec=sudo mate-terminal -x /var/log/installer/oem-qc, Terminal=true ; oem-unlock : Exec=/var/log/installer/oem-unlock (exécutables placés sous /var/log/installer) | CONSTATÉ |
| debian-install | `archive:debian/kylin-os-installer.install` | /usr/bin : kylin-os-installer, kylin-os-installer-{dm,live,oem,oem-automatic,wrapper} ; scripts/, data/, language/ -> /usr/share/kylin-os-installer ; ky-installer.cfg -> /var/log/installer et /usr/share/kylin-os-installer | CONSTATÉ |
| sudoers (écrit à l'installation) | `archive:scripts/in_chroot/21setup-user ; archive:scripts/oem.sh` | 21setup-user l.24 : echo 'oem ALL=(ALL) NOPASSWD: ALL' >/etc/sudoers.d/oem ; oem.sh l.371 : '${userName} ALL=(ALL) NOPASSWD: ALL' >/etc/sudoers.d/${userName} ; oem.sh l.359-360 supprime /etc/sudoers.d/oem (conditions d'exécution non lues) | CONSTATÉ |
| sources-apt (chemin d'installation) | `archive:scripts/in_chroot/96-resetup-sourceslist ; archive:scripts/z.sh` | URLs http://archive.kylinos.cn/kylin/KYLIN-ALL, http://archive.kylinos.cn/kylin/partner, http://archive.launchpad.dev/kylin/, http://archive.launchpad.dev/kylin-desktop/ ; z.sh l.768 écrit 'deb [trusted=yes] file:///opt/apps-third/ ./' dans etc/apt/sources.list.d/kylin-apps-third.list du sysroot | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends cryptsetup, parted, lvm2, rsync, os-prober, ntfs-3g, xfsprogs... ; Build-Depends libkba-dev/libkba, libkylin-chkname-dev, libkylin-nm-base, kylin-nm-plugin, libkf5networkmanagerqt-dev, libkysdk-* | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_installsystemd uniquement ; hardening commenté | CONSTATÉ |
| sudoers (ecrit par script) [ajout critique] | `archive:kylin-os-installer/scripts/after/21setup-auditmode` | Si /proc/cmdline contient auditmode ou ple-mode (scripts/utils:653-661), le script cree un compte 'test' avec un mot de passe fixe (valeur masquee), active l'autologin et ecrit 'test ALL=(ALL) NOPASSWD: ALL' dans <nouvelle racine>/etc/sudoers.d/test (l.15-20). Absent de la fiche (categorie sudoers vide) et du constat B05, qui ne cite que le mode OEM. | CONSTATÉ |

- [CONSTATÉ] La chaîne 'Kylin123.' est passée sur stdin à 'sudo -S umount /media/oem/*' (data/kylin-os-installer-oem l.31, data/kylin-os-installer-oem-automatic l.11).
- [CONSTATÉ] debian/patches/series (patch 'tangzhicheng', 1930 lignes, en-tête 1.1.1.1-0k0.3 v101) est présent alors que debian/source/format = 3.0 (native).
- [PROBABLE] En format natif, dpkg-source n'applique pas la série quilt : le patch n'aurait pas d'effet au build.
- [CONSTATÉ] scripts/oem.sh.orig (copie de sauvegarde) est installé avec le répertoire scripts/ (debian/kylin-os-installer.install copie le répertoire entier).
- [NON COUVERT] Les différences entre l'archive et la branche Gitee openkylin/huanghe n'ont pas été établies (Gitee non acquis).

### `kylin-fix-boot-grub`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs/Homepage https://gitlab2.kylin.com/kylinos-src/kylin-fix-boot-grub.git ; debian/watch pointe vers gitlab2.kylin.com) ; version 3.0.1.1-0k0.21, distribution de tête du changelog 'v11-2603-preview'
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 58 fichiers) ; Gitee PRESENT (branche openkylin/nile-sp2 seulement) non acquis
- **Langages / taille** : C++/Qt (~1,06 kLoC .cpp, 21 fichiers src/), Shell (fix-bash/ : 3 scripts, 196 lignes ; data/kvWayland : 2 scripts), qmake — 58 fichiers ; 652 Ko (dont image 525 Ko) ; taille_source 557 Ko — lignes de code propres mesurées : 1 466
- **Privilège** : root (service systemd sans User=, lancement via sudo depuis le .desktop) ; agit sur GRUB / chaîne de démarrage
- **Déclencheurs** : graphical.target si le service est activé, mode recovery via générateur (si installé, actuellement commenté), lancement manuel 'sudo kylin-fix-boot-grub', session lightdm wayland dédiée
- **Indices réseau** : aucun indice d'écoute réseau signalé ni relevé
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Composant propre de petite taille (~1,1 kLoC C++ + ~200 lignes shell) exécuté en root et modifiant le chargeur de démarrage.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:data/system/kylin-fix-boot-grub.service` | Type=oneshot ; Restart=on-failure ; ExecStart=/usr/bin/start-kylin-fix-boot-grub ; pas de User= (root) ; Before=lightdm.service ; WantedBy=graphical.target ; installé dans /lib/systemd/system par kylin-fix-boot-grub.pro (system_fix_service) | CONSTATÉ |
| systemd-target | `archive:data/system/kylin-fix-boot-grub.target` | Wants/After=kylin-fix-boot-grub.service ; AllowIsolate=yes ; absent des INSTALLS du .pro | CONSTATÉ |
| systemd-generator | `archive:data/system-generators/kylin-fix-boot-grub` | si 'recovery' dans /proc/cmdline : ln -sf /lib/systemd/system/kylin-fix-boot-grub.target $earlydir/default.target ; installation dans /lib/systemd/system-generators commentée dans le .pro | CONSTATÉ |
| maintainer-script | `archive:debian/preinst` | systemctl disable kylin-fix-boot-grub.service \|\| : (commentaire 'it is now a static service') | CONSTATÉ |
| desktop-file | `archive:data/kylin-fix-boot-grub.desktop` | Exec=sudo /usr/bin/kylin-fix-boot-grub ; NoDisplay=true ; X-UKUI-AutoRestart=true | CONSTATÉ |
| session-lightdm/wayland | `archive:data/kvWayland/wayland-kylin-fix-boot-grub.desktop ; archive:data/kvWayland/99-w-wayland-fix-boot-grub.conf` | Exec=/usr/bin/w-wayland-fix-boot-grub, X-LightDM-Session-Type=wayland ; [Seat:*] user-session=wayland-kylin-fix-boot-grub ; installés sous /usr/share/kylin-fix-boot-grub/data/kvwayland/ | CONSTATÉ |
| build (qmake INSTALLS) | `archive:kylin-fix-boot-grub.pro` | binaire -> /usr/bin ; fix-boot-bash, fix-grub-real -> /usr/share/kylin-fix-boot-grub/fix-bash ; start-kylin-fix-boot-grub -> /usr/bin ; service -> /lib/systemd/system ; .desktop -> /usr/share/applications ; pas de debian/*.install | CONSTATÉ |
| debian-rules/control | `archive:debian/rules ; archive:debian/control` | hardening=+all ; dh $@ ; Rules-Requires-Root: no ; Description 'When system grub is damaged, use the live system to repair grub' | CONSTATÉ |

- [PROBABLE] Le service est installé par qmake avec une section [Install] ; avec debhelper compat 12, dh_installsystemd peut générer des snippets d'activation, alors que preinst le désactive.
- [CONSTATÉ] La distribution de tête du changelog est 'v11-2603-preview' (lignée Kylin V11) pour un paquet installé par l'ISO openKylin 3.0.

### `ukui-iso-customer`

- **Nature** : propre-kylin — **Amont** : openKylin UKUI SIG (Gitee openkylin/ukui-iso-customer, branche master) ; mainteneur DEBIAN/control wenyuanbo@kylinos.cn ; paquet iso-customizer 2.8.0
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (27 fichiers : source Python + artefacts PyInstaller et paquet .deb déplié) ; aucune entrée archive
- **Langages / taille** : Python/Tkinter (iso_customizer.py, 4148 lignes, 46 appels subprocess/os.system/Popen), Shell (install.sh 197 lignes), spec PyInstaller — 27 fichiers ; 31,5 Mo dont 20,5 Mo de blobs — lignes de code propres mesurées : 4 345
- **Privilège** : root via pkexec (aucun fichier .policy dans le dépôt)
- **Déclencheurs** : lancement manuel depuis le menu (pkexec)
- **Indices réseau** : aucun listener; install.sh télécharge via apt et pip3 (pip3 install pyinstaller)
- **Blobs** : 8 blobs (20,5 Mo) : 2 ELF PyInstaller de 9,76 Mo identiques octet à octet (dist/iso-customizer et iso-customizer-deb/usr/bin/iso-customizer, sha256 b41b2207...) ; build/iso-customizer/base_library.zip ; 5 .pyc cpython-38 ; plus PYZ-00.pyz et iso-customizer.pkg
- **Budget proposé** : L3 — 95 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Outil propre (~4,1 kLoC Python en un seul fichier) exécuté en root via pkexec qui reconstruit des ISO d'installation (injection de .deb, paramètres d'installation sans surveillance) ; ajouter le contrôle de correspondance entre l'ELF commité et la source.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `gitee:iso-customizer/iso-customizer-deb/usr/share/applications/iso-customizer.desktop` | Exec=sh -c 'pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY /usr/bin/iso-customizer' | CONSTATÉ |
| maintainer-script | `gitee:iso-customizer/iso-customizer-deb/DEBIAN/postinst` | case "" in configure) ... : la chaîne testée est la chaîne vide littérale ; gtk-update-icon-cache, update-desktop-database | CONSTATÉ |
| debian-control | `gitee:iso-customizer/iso-customizer-deb/DEBIAN/control` | Package iso-customizer 2.8.0, Architecture all ; Depends python3-tk, genisoimage, squashfs-tools, policykit-1, imagemagick, binfmt-support, qemu-user-static | CONSTATÉ |
| build (PyInstaller) | `gitee:iso-customizer/iso-customizer.spec` | Analysis(['iso_customizer.py']) ; EXE upx=True, console=False | CONSTATÉ |
| script d'installation | `gitee:iso-customizer/install.sh` | exige root ; apt install/apt-get install ; pip3 install pyinstaller ; génère le .desktop pkexec ; dpkg-deb --build ; dpkg -i | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive openKylin et non installé par défaut par l'ISO 3.0 (installe_par_defaut_iso vide).
- [NON COUVERT] La correspondance entre l'ELF PyInstaller distribué et iso_customizer.py n'a pas été vérifiée.

### `kylin-usb-creator`

- **Nature** : propre-kylin — **Amont** : Kylin (Homepage https://github.com/ubuntukylin/kylin-usb-creator) ; 1.2.0.3kylin-ok6.13k0.21update2 huanghe
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 380 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt (UI UIControl ~4,7 kLoC .cpp ; helper D-Bus registeredQDbus 882 lignes ; 45 fichiers .cpp/.h hors tests), tests C++, qmake ; 'TypeScript' = traductions .ts — 380 fichiers ; 3,77 Mo — lignes de code propres mesurées : 17 965
- **Privilège** : helper D-Bus système en root activé à la demande ; UI en session utilisateur
- **Déclencheurs** : activation D-Bus par un appel à com.kylinusbcreator.systemdbus, lancement manuel de l'UI
- **Indices réseau** : QLocalServer dans UIControl/src/include/qtlocalpeer.cpp l.103-131 : server->listen(socketName), socket local QtSingleApplication (instance unique), pas d'écoute réseau; SDK de collecte de données : kdk_dia_upload_program_startup/exit et kdk_dia_upload_default(node, 'start-usrcreator-capture', 'startMake') dans UIControl/src/mainwindow.cpp l.68, 197, 338, 617-619
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant propre de taille modérée (~5,5 kLoC hors tests) avec helper D-Bus root ouvert au contexte default, écriture sur périphériques bloc et appels au SDK de collecte de données.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (système) | `archive:registeredQDbus/conf/com.kylinusbcreator.systemdbus.service` | Name=com.kylinusbcreator.systemdbus ; Exec=/opt/apps/top.openkylin.kylin-usb-creator/files/bin/kylin-usb-creator-sysdbus ; User=root | CONSTATÉ |
| dbus-policy | `archive:registeredQDbus/conf/com.kylinusbcreator.systemdbus.conf` | policy user=root : own com.kylinusbcreator.systemdbus ; context=default : allow send_destination=com.kylinusbcreator.systemdbus send_interface=com.kylinusbcreator.interface (+ Introspectable, Properties) | CONSTATÉ |
| polkit | `archive:registeredQDbus/conf/com.kylinusbcreator.systemdbus.policy` | action com.kylinusbcreator.systemdbus.authoritycheck : allow_any=auth_admin, allow_inactive=auth_admin, allow_active=auth_admin_keep | CONSTATÉ |
| build (qmake INSTALLS) | `archive:registeredQDbus/registeredQDbus.pro` | TARGET kylin-usb-creator-sysdbus (QT core dbus, -lpolkit-qt5-core-1) -> /opt/apps/top.openkylin.kylin-usb-creator/files/bin ; .service -> /usr/share/dbus-1/system-services ; .conf -> /etc/dbus-1/system.d ; .policy -> /usr/share/polkit-1/actions | CONSTATÉ |
| debian-links | `archive:debian/kylin-usb-creator.links` | /usr/bin/kylin-usb-creator et /usr/bin/kylin-usb-creator-sysdbus -> binaires sous /opt/apps/top.openkylin.kylin-usb-creator/files/bin | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-usb-creator.postinst` | glib-compile-schemas /usr/share/glib-2.0/schemas | CONSTATÉ |
| desktop-file | `archive:UIControl/kylin-usb-creator.desktop` | Exec=/opt/apps/top.openkylin.kylin-usb-creator/files/bin/kylin-usb-creator %u | CONSTATÉ |
| manifeste kaiming | `archive:kaiming/top.openkylin.kylin-usb-creator.km.yaml` | permission filesystem '~' ; kylin.polkit: true ; kylin.depend-apps : /usr/sbin/mkfs.fat, /usr/bin/mount, /usr/bin/umount, /usr/bin/md5sum, service-support-mini | CONSTATÉ |
| debian-control/rules | `archive:debian/control ; archive:debian/rules` | Build-Depends libkysdk-datacollect-dev, libpolkit-qt5-1-dev, libukui-log4qt-dev ; hardening=+all | CONSTATÉ |

- [CONSTATÉ] La version du manifeste kaiming (1.2.0.3-ok6.13k0.63) diffère de celle de debian/changelog (1.2.0.3kylin-ok6.13k0.21update2).
- [NON COUVERT] Le contenu Gitee/GitHub (DIVERGENT) n'a pas été comparé à l'archive.

### `yhkylin-backup-tools`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur liubuquan <liubuquan@kylinos.cn>, Homepage http://kylinos.cn) ; 4.2.0.1-ok1 huanghe ('rebuild for openkylin3.0')
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète + artefacts de build : ELF, arbre de staging debian/yhkylin-backup-tools/, debian/.debhelper/ dbgsym) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt (backup-daemon ~9,6 kLoC, UI kybackup ~14,6 kLoC ; 168 fichiers .cpp/.h dans backup-daemon/, kybackup/, common/), Shell (hooks/scripts initramfs), qmake — 482 fichiers ; 97,4 Mo dont 88,2 Mo de blobs ELF ; taille_source 54,2 Mo — lignes de code propres mesurées : 34 230
- **Privilège** : root : daemon D-Bus système activé à la demande, scripts initramfs au démarrage, maintainer scripts ; UI kybackup en session utilisateur
- **Déclencheurs** : activation D-Bus de com.kylin.backup, update-initramfs (hook) puis démarrage (init-bottom), installation/suppression du paquet, lancement manuel de kybackup
- **Indices réseau** : QLocalServer dans kybackup/qtsingleapplication/qtlocalpeer.cpp l.160 : socket local 'qtsingleapp-*' (instance unique), pas d'écoute réseau; SDK de collecte de données (kdk_dia_* / libkydatacollect) appelé dans 7 fichiers de kybackup/ (main.cpp, maindialog.cpp, leftsiderbarwidget.cpp, module/*.cpp)
- **Blobs** : 9 ELF (88,2 Mo) : kybackup/kybackup (30 Mo), backup-daemon/backup-daemon (15,4 Mo), 5 .debug sous debian/.debhelper/yhkylin-backup-tools/dbgsym-root, debian/yhkylin-backup-tools/usr/bin/{kybackup,backup-daemon} ; lien symbolique vers /home/user/桌面/桌面/yhkylin-backup-tools/... (chemin de la machine de build)
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre volumineux (~24 kLoC C++) : lecture exhaustive du daemon D-Bus root (backup-daemon/, common/) et des scripts initramfs, UI kybackup limitée aux zones à risque (appels D-Bus, collecte de données), plus comparaison ELF commités et source.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (système) | `archive:backup-daemon/data/com.kylin.backup.service` | Name=com.kylin.backup ; Exec=/usr/bin/backup-daemon ; User=root | CONSTATÉ |
| dbus-policy | `archive:backup-daemon/data/com.kylin.backup.conf` | policy user=root : own com.kylin.backup ; context=default : <allow send_interface=com.kylin.backup.manager/> sans send_destination, + Introspectable/Properties vers com.kylin.backup | CONSTATÉ |
| polkit | `archive:backup-daemon/data/com.kylin.backup.policy` | action com.kylin.backup.manager : allow_inactive=auth_admin ; deux éléments <allow_active> (auth_admin puis auth_admin_keep) ; pas d'allow_any | CONSTATÉ |
| liste blanche D-Bus | `archive:backup-daemon/data/com.kylin.backup.limit ; archive:backup-daemon/data/com.kylin.backup.limit.verify` | [whitelist] kylin-unattended-upgrade-shutdown, ukui-control-center, kylin-system-updater, kybackup, kybackupcli ; .verify = 00090eb5f48c1644207ef53fa056f681346bb83b ; installés dans /etc/dbus-1/conf/ et /usr/share/dbus-1/conf/ | CONSTATÉ |
| configuration | `archive:backup-daemon/data/com.kylin.backup.yaml ; archive:backup-daemon/data/userdata_backup_or_restore.conf` | yaml -> /etc/kylin-config/basic/ (audit false, whitelist, backupstatus, backuppointnummax -1) ; /etc/userdata_backup_or_restore.conf liste /home/*/.config/kglobalshortcutsrc et /home/*/.config/dconf/user | CONSTATÉ |
| initramfs-hook | `archive:backup-daemon/data/initramfs-tools/hooks/kybackup-hooks` | copy_exec si présents : /etc/.bootinfo, /etc/fstab -> /etc/fstab-backup, rsync, bc, backup-auto-factory, backup-auto-efi, backup-auto, mount_fstab_efi | CONSTATÉ |
| initramfs-script | `archive:backup-daemon/data/initramfs-tools/scripts/local-bottom/kybackup` | installé dans scripts/init-bottom ; 535 lignes ; PREREQ=security_set ; 'exit 0' inconditionnel après l'en-tête (commentaire : fonction déplacée dans yhkylin-systembackup-tools) | CONSTATÉ |
| initramfs-conf | `archive:backup-daemon/data/initramfs-tools/conf-hooks.d/kybackup` | FRAMEBUFFER=y | CONSTATÉ |
| maintainer-script | `archive:debian/yhkylin-backup-tools.postinst ; archive:debian/yhkylin-backup-tools.postrm` | postinst : rm scripts/local-bottom/kybackup, killall kybackup et backup-daemon, rm -rf ancien guide, update-initramfs -u ; postrm : update-initramfs -u | CONSTATÉ |
| desktop-file | `archive:kybackup/yhkylin-backup-tools.desktop ; archive:backup-daemon/backup-daemon.desktop` | Exec=kybackup %U ; Exec=/usr/bin/backup-daemon (NoDisplay=true) | CONSTATÉ |
| debian-install/rules | `archive:debian/yhkylin-backup-tools.install ; archive:debian/rules` | kybackup et backup-daemon -> /usr/bin ; conf/service/policy/initramfs/limit/yaml ; rules : 'dh $@' et cible 'dh_override_auto_build: qmake; make' (nom de cible non standard) | CONSTATÉ |

- [CONSTATÉ] Les fichiers déclaratifs du staging debian/yhkylin-backup-tools/ (conf, service, policy, hook et script initramfs, DEBIAN/postinst) sont identiques aux sources.
- [CONSTATÉ] yhkylin-rockbackup-tools fournit le même nom de bus com.kylin.backup et les mêmes chemins /usr/bin/kybackup et /usr/bin/backup-daemon ; aucun champ Conflicts/Replaces dans l'un ou l'autre debian/control.
- [CONSTATÉ] La valeur de .limit.verify ne correspond ni au SHA-1 (b17c16a5...) ni au MD5 (1384b3f7...) du fichier .limit ; le mécanisme de vérification n'a pas été lu.

### `yhkylin-rockbackup-tools`

- **Nature** : propre-kylin — **Amont** : Kylin (liubuquan <liubuquan@kylinos.cn>) ; 4.1.0.1-ok20, distribution 'nile' ('去除kysec依赖') ; variante de yhkylin-backup-tools
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète + artefacts de build) ; Gitee PRESENT (master, openkylin/nile-sp2) non acquis
- **Langages / taille** : C++/Qt (backup-daemon ~9,5 kLoC, kybackup ~13,8 kLoC ; 164 fichiers .cpp/.h), Shell (initramfs), qmake — 407 fichiers ; 75,1 Mo dont 67,6 Mo de blobs ELF ; taille_source 34,8 Mo — lignes de code propres mesurées : 31 144
- **Privilège** : root (daemon D-Bus système, initramfs, maintainer scripts)
- **Déclencheurs** : activation D-Bus de com.kylin.backup, update-initramfs puis démarrage, installation du paquet
- **Indices réseau** : QLocalServer dans kybackup/qtsingleapplication/qtlocalpeer.cpp l.156 : socket local QtSingleApplication; SDK de collecte de données (kdk_dia_*) dans 6 fichiers de kybackup/
- **Blobs** : 7 ELF (67,6 Mo) : kybackup/kybackup, backup-daemon/backup-daemon, 3 .debug sous debian/.debhelper/yhkylin-backup-tools/ (nom du paquet frère), debian/yhkylin-rockbackup-tools/usr/bin/{kybackup,backup-daemon}
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Service D-Bus système root com.kylin.backup, hooks initramfs : différentiel contre yhkylin-backup-tools (même nom de bus). — (avant correction : L1/10%/e2)

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service + dbus-policy + polkit | `archive:backup-daemon/data/com.kylin.backup.service ; archive:backup-daemon/data/com.kylin.backup.conf ; archive:backup-daemon/data/com.kylin.backup.policy` | identiques octet à octet à ceux de yhkylin-backup-tools : Name=com.kylin.backup, Exec=/usr/bin/backup-daemon, User=root ; context=default allow send_interface=com.kylin.backup.manager sans send_destination ; polkit com.kylin.backup.manager auth_admin/auth_admin_keep | CONSTATÉ |
| liste blanche D-Bus / config | `archive:backup-daemon/data/com.kylin.backup.limit ; archive:backup-daemon/data/com.kylin.backup.yaml` | .limit et .limit.verify identiques à yhkylin-backup-tools ; .yaml différent (sans backupstatus ni backuppointnummax) | CONSTATÉ |
| initramfs | `archive:backup-daemon/data/initramfs-tools/hooks/kybackup-hooks ; archive:backup-daemon/data/initramfs-tools/scripts/local-bottom/kybackup ; archive:backup-daemon/data/initramfs-tools/conf-hooks.d/kybackup` | identiques à yhkylin-backup-tools ; script installé dans scripts/init-bottom | CONSTATÉ |
| maintainer-script | `archive:debian/yhkylin-rockbackup-tools.postinst ; archive:debian/yhkylin-rockbackup-tools.postrm` | même texte que yhkylin-backup-tools : killall kybackup/backup-daemon, update-initramfs -u | CONSTATÉ |
| debian-install | `archive:debian/yhkylin-rockbackup-tools.install` | mêmes destinations que yhkylin-backup-tools (/usr/bin, /etc/dbus-1/system.d, polkit, initramfs, /etc/kylin-config/basic) ; guide depuis data/kybackup/* | CONSTATÉ |
| desktop-file | `archive:kybackup/yhkylin-backup-tools.desktop ; archive:backup-daemon/backup-daemon.desktop` | backup-daemon.desktop identique ; yhkylin-backup-tools.desktop différent de la version backup-tools | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0 (installe_par_defaut_iso vide) ; changelog de tête en distribution 'nile'.
- [CONSTATÉ] debian/.debhelper/ contient un sous-arbre 'yhkylin-backup-tools' : artefacts issus du build du paquet frère.

### `yhkylin-systembackup-tools`

- **Nature** : propre-kylin — **Amont** : Kylin (yuchengyong <yuchengyong@kylinos.cn>) ; 1.0.0.0-ok10 huanghe ('rebuild for openkylin3.0')
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète + 2 ELF + arbre de staging debian/ divergent des sources) ; Gitee PRESENT non acquis
- **Langages / taille** : C++/Qt (systembackup-daemon + common ~18,8 kLoC, 74 fichiers .cpp/.h hors tests, dont myprocess/OstreeInterface.cpp), Shell (12 scripts dans systembackup-daemon/data, ~293 Ko dont backup-grub 99 Ko et systembackup-inrock 63 Ko ; initramfs 687 + 98 lignes), qmake, nombreux tests — 334 fichiers ; 31,7 Mo dont 27,6 Mo de blobs ; taille_source 6,5 Mo — lignes de code propres mesurées : 31 607
- **Privilège** : root : daemon D-Bus système, scripts shell /usr/bin appelés par le daemon et depuis l'initramfs (GRUB, restauration, ostree), maintainer scripts
- **Déclencheurs** : activation D-Bus de com.kylin.systembackup, démarrage (init-bottom) selon fichiers drapeaux de restauration, hooks update-manifests du mécanisme de mise à jour, installation/suppression (update-initramfs)
- **Indices réseau** : aucun indice d'écoute signalé (listen vide); interaction ostree via systembackup-daemon/myprocess/OstreeInterface.cpp (non lu)
- **Blobs** : 2 ELF : systembackup-daemon/systembackup-daemon (27 Mo) et debian/yhkylin-systembackup-tools/usr/bin/systembackup-daemon (533 Ko)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Daemon propre root (~18,8 kLoC C++) avec ~290 Ko de scripts shell root et scripts initramfs exécutés au démarrage : chemin de restauration et de mise à jour du système.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (système) | `archive:systembackup-daemon/data/com.kylin.systembackup.service` | Name=com.kylin.systembackup ; User=root ; Exec=/bin/sh -c '[ -x /usr/bin/update-launcher ] && exec /usr/bin/update-launcher /usr/bin/systembackup-daemon \|\| exec /usr/bin/systembackup-daemon' | CONSTATÉ |
| dbus-policy | `archive:systembackup-daemon/data/com.kylin.systembackup.conf` | policy user=root : own com.kylin.systembackup ; context=default : <allow send_interface=com.kylin.systembackup.manager/> sans send_destination, + Introspectable/Properties | CONSTATÉ |
| polkit | `archive:systembackup-daemon/data/com.kylin.systembackup.policy` | action com.kylin.systembackup.manager : allow_inactive=auth_admin ; deux <allow_active> (auth_admin, auth_admin_keep) | CONSTATÉ |
| liste blanche D-Bus / config | `archive:systembackup-daemon/data/com.kylin.systembackup.limit ; archive:systembackup-daemon/data/com.kylin.systembackup.yaml` | whitelist + key6=/var/opt/update-module/current/usr/share/kylin-system-updater/kylin-system-updater ; .limit.verify = même valeur 00090eb5... que yhkylin-backup-tools alors que le contenu du .limit diffère | CONSTATÉ |
| initramfs-hook | `archive:systembackup-daemon/data/initramfs-tools/hooks/systemkybackup-hooks` | copy_exec : rsync, bc, mksquashfs, bash, mkdir, ls, mount, sync, reboot, grub-install, backup-auto*, mount_fstab_efi, backup-factory, backup-grub, backup-deploy, backup-undeploy, backup-setdefault, /etc/.bootinfo, /etc/fstab | CONSTATÉ |
| initramfs-script | `archive:systembackup-daemon/data/initramfs-tools/scripts/local-bottom/systemkybackup` | installé dans scripts/init-bottom ; 687 lignes ; restauration au démarrage pilotée par les fichiers drapeaux ${rootmnt}/data/.backup/update-roolback, kare-roolback et /data/.backup/roolback.txt (en-tête lu) | CONSTATÉ |
| debian-install | `archive:debian/yhkylin-systembackup-tools.install` | /usr/bin : systembackup-daemon, systembackup-inrock, backup-factory, backup-grub, backup-deploy, backup-undeploy, backup-setdefault ; manifest -> /usr/share/update-manifests/ ; .desktop (dont etc/xdg/autostart) commentés | CONSTATÉ |
| hook de mise à jour | `archive:systembackup-daemon/data/yhkylin-systembackup-tools.manifest.json` | post_hooks : command 'pkill -f', value '/usr/bin/systembackup-daemon' | CONSTATÉ |
| maintainer-script | `archive:debian/yhkylin-systembackup-tools.postinst ; archive:debian/yhkylin-systembackup-tools.postrm` | postinst : killall systembackup-daemon, update-initramfs -u ; postrm : update-initramfs -u | CONSTATÉ |
| desktop-file | `archive:systembackup-daemon/yhkylin-systembackup-tools.desktop ; archive:systembackup-daemon/systembackup-daemon.desktop` | Exec=/usr/bin/systembackup-daemon, NoDisplay=true ; non installés (lignes commentées dans .install) | CONSTATÉ |

- [CONSTATÉ] Le staging debian/yhkylin-systembackup-tools/ diverge des sources : .service avec Exec=/opt/update-module/current/usr/bin/systembackup-daemon, hook initramfs sans 3 copy_exec, script init-bottom avec 391 lignes de diff.
- [CONSTATÉ] Le hook initramfs copie backup-auto, backup-auto-efi, backup-auto-factory et mount_fstab_efi depuis /usr/bin alors que .install ne les installe pas (présents seulement dans systembackup-daemon/data/).
- [NON COUVERT] La provenance de /usr/bin/update-launcher, référencé par le .service, n'a pas été établie ; il n'est pas fourni par ce paquet.

### `ostree-maintain-daemon`

- **Nature** : absent-ou-vide — **Amont** : inconnu (Homepage '<insert the upstream URL>', copyright dh_make non rempli ; Gitee et GitHub ABSENT)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive squelette vide (dh_make, 20 fichiers debian/ uniquement dont modèles *.ex) ; aucun code
- **Langages / taille** : aucun — 20 fichiers ; 30 Ko ; taille_source 8,2 Ko (source_coquille_vide=1) — lignes de code propres mesurées : 0
- **Privilège** : inconnu : le binaire installé n'est pas reconstructible depuis l'arbre
- **Déclencheurs** : inconnus (binaire non analysé)
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source : le paquet installé par défaut ne peut pas être produit à partir de ce squelette dh_make.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package ostree-maintain-daemon, Architecture any ; Description 'immutableOS ostree maintain mode develop' ; Build-Depends debhelper-compat (= 12) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | uniquement '%: dh $@' ; aucun fichier .install ni source à construire | CONSTATÉ |
| modèles inactifs | `archive:debian/postinst.ex ; archive:debian/ostree-maintain-daemon.cron.d.ex` | modèles dh_make .ex, non utilisés par debhelper | CONSTATÉ |
| binaire installé | `(paquet .deb 2.0.0.0-0k2.18update1.5 de l'ISO)` | contenu, services et privilèges du binaire installé non disponibles dans le corpus source | NON COUVERT |

- [CONSTATÉ] Installé par défaut par l'ISO 3.0 ; changelog unique 'Initial Release' du 27 Aug 2026 (huanghe) ; README signé 'kylin <kylin@unknown>'.
- [PROBABLE] Le nom suggère un service système ; seule une analyse du .deb binaire (reconditionnement) permettrait de l'inventorier.

### `ostree-coredata-protect`

- **Nature** : absent-ou-vide — **Amont** : inconnu (Homepage non renseigné, copyright dh_make non rempli ; Gitee et GitHub ABSENT)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive squelette vide (dh_make, 20 fichiers debian/ uniquement) ; aucun code
- **Langages / taille** : aucun — 20 fichiers ; 30 Ko ; taille_source 8,2 Ko (source_coquille_vide=1) — lignes de code propres mesurées : 0
- **Privilège** : inconnu (binaire non analysé)
- **Déclencheurs** : inconnus
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible pour un paquet installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package ostree-coredata-protect, Architecture any ; mainteneur wudan <wudan@kylinos.cn> ; Description 'Content featuring merged data，etc，var,boot，merge data / data handle status' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | uniquement '%: dh $@' ; aucun fichier à installer | CONSTATÉ |
| modèles inactifs | `archive:debian/postinst.ex ; archive:debian/ostree-coredata-protect.cron.d.ex` | modèles dh_make .ex non actifs | CONSTATÉ |
| binaire installé | `(paquet .deb 2.0.0.0-0k2.23.6.0 de l'ISO)` | contenu du binaire installé non disponible | NON COUVERT |

- [CONSTATÉ] Version installée par l'ISO (2.0.0.0-0k2.23.6.0) différente de la version de l'archive (2.0.0.0-0k2.23.7.0, changelog du 21 Sep 2026, postérieur à l'ISO du 18/09/2026).
- [PROBABLE] La description (données fusionnées etc/var/boot) indique une intervention sur les données système ostree ; non vérifiable sans le binaire.

### `ostree-extra-utils`

- **Nature** : absent-ou-vide — **Amont** : inconnu (Maintainer 'Ubuntu Developers', Original-Maintainer zhangtingting <zhangtingting@kylinos.cn> ; Gitee et GitHub ABSENT)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive squelette vide (dh_make, 19 fichiers debian/ uniquement) ; aucun code
- **Langages / taille** : aucun — 19 fichiers ; 26 Ko ; taille_source 7,4 Ko (source_coquille_vide=1) — lignes de code propres mesurées : 0
- **Privilège** : inconnu (binaire non analysé)
- **Déclencheurs** : inconnus
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible pour un paquet installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package ostree-extra-utils, Architecture any ; Description 'OSTREE Extended Attribute Function Tool ... (factory reset, unified configuration of environment variables)' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | uniquement '%: dh $@' ; aucun fichier à installer | CONSTATÉ |
| binaire installé | `(paquet .deb 2.0.0.0-0k1.20update1 de l'ISO)` | contenu du binaire installé non disponible | NON COUVERT |

- [CONSTATÉ] Changelog unique 'Initial Release' en distribution 'nile.bedrock' (11 Jun 2026) pour un paquet installé par l'ISO 3.0.
- [PROBABLE] La description (restauration d'usine) recoupe kylin-ostree-extra-utils (même mainteneuse) : successeur sans source publiée.

### `kylin-ostree-extra-utils`

- **Nature** : propre-kylin — **Amont** : Kylin (zhangtingting <zhangtingting@kylinos.cn>) ; format 3.0 (quilt) dont tout le code est introduit par debian/patches (init.patch, 0002-1-CI.patch, 0003-4-I9TJ0F-2-grub.patch) ; 1.0.0-ok4 nile
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète minuscule ; patchs appliqués, répertoire .pc présent) ; Gitee/GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : Python (factory-restore/kylin-ostree-restore.py, 2,5 Ko), Shell (kylin-ostree-restore-plymouth.sh, ostree-update-initramfs.sh) — 17 fichiers ; 25 Ko — lignes de code propres mesurées : 136
- **Privilège** : root (service systemd User=root au démarrage)
- **Déclencheurs** : démarrage multi-user.target, avant lightdm
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Trois scripts propres (~140 lignes) exécutés en root au démarrage pour la restauration d'usine ostree et GRUB.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:conf/ostree-extra-restore.service` | Type=oneshot ; ExecStart=/usr/sbin/kylin-ostree-restore-plymouth.sh ; User=root ; KillMode=process ; Before=lightdm.service ; WantedBy=multi-user.target | CONSTATÉ |
| debian-install | `archive:debian/install` | service -> /lib/systemd/system/ ; kylin-ostree-restore.py, kylin-ostree-restore-plymouth.sh, ostree-update-initramfs.sh -> /usr/sbin/ | CONSTATÉ |
| debian-control/rules | `archive:debian/control ; archive:debian/rules` | Depends python3, grub2-common ; rules 'dh $@' (activation du service par dh_installsystemd, compat 12) | CONSTATÉ |
| activation systemd | `archive:debian/rules` | snippets d'activation générés par debhelper pour le service présent dans /lib/systemd/system | PROBABLE |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0 ; en-têtes de patch laissés au modèle dh_make ('TODO: Put a short summary').

### `openkylin-keyring`

- **Nature** : meta-ou-donnees — **Amont** : openKylin Infrastructure (paquet natif 2022.05.12-ok3, distribution de tête 'nile')
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète : 2 trousseaux OpenPGP + packaging)
- **Langages / taille** : aucun code (données OpenPGP, Makefile debian/rules) — 13 fichiers ; 7,3 Ko — lignes de code propres mesurées : 0
- **Privilège** : fichiers de confiance système (APT global via /etc/apt/trusted.gpg.d) ; postinst udeb en root
- **Déclencheurs** : installation du paquet, chaque opération APT (vérification des Release), vérification de signature de .deb par le composant utilisant /usr/share/keyrings/deb-sign/
- **Indices réseau** : aucun
- **Blobs** : 2 trousseaux OpenPGP binaires (2290 o et 2262 o)
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Ancre de confiance APT installée par défaut, quelques fichiers : vérifier empreintes, chemins d'installation et consommateur de la clé deb-sign.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/openkylin-keyring.install` | keyrings/openkylin-archive-keyring.gpg -> /etc/apt/trusted.gpg.d/ et /usr/share/keyrings/ ; keyrings/openkylin.gpg -> /usr/share/keyrings/deb-sign/ | CONSTATÉ |
| debian-install (udeb) | `archive:debian/openkylin-keyring-udeb.install` | openkylin-archive-keyring.gpg -> /usr/share/keyrings/ | CONSTATÉ |
| maintainer-script | `archive:debian/openkylin-keyring-udeb.postinst` | ln -s openkylin-archive-keyring.gpg /usr/share/keyrings/archive.gpg si absent | CONSTATÉ |
| debian-rules | `archive:debian/rules` | build : gpg --homedir .gnupg --import keyrings/* ; checkkeyrings ; override_dh_installdeb supprime DEBIAN/conffiles | CONSTATÉ |
| ancre de confiance | `archive:keyrings/openkylin-archive-keyring.gpg` | RSA 4096, empreinte 09FFC10EA273DD29A986B1108B313CEAFF592D96, UID 'openKylin Archive Signing Key <release-team@openkylin.top>', créée 2022-05-12, expiration +3650 j ; sous-clé 6FF39A7001F06CDC5E75E25A8DE01E7B922F0973 | CONSTATÉ |
| ancre de confiance (signature deb) | `archive:keyrings/openkylin.gpg` | RSA 4096, empreinte ED6B59E1119FE97EC326743E5CFF40DB55781542, UID 'Kylin Deb Signing Key <release@kylinos.cn>', créée 2026-01-09, aucun sous-paquet d'expiration relevé ; sous-clé 788058919141374E522F55A6B616154ECF289AA3 | CONSTATÉ |

- [CONSTATÉ] md5sums.txt donne SHA1/SHA256 seulement pour openkylin-archive-keyring.gpg (valeurs concordantes avec le fichier) ; openkylin.gpg n'y figure pas.
- [CONSTATÉ] Changelog ok2 (30 Mar 2026) : '【dpkg改造】增加特权标记工具' (modification de dpkg, outil de marquage de privilège).
- [PROBABLE] La clé deb-sign est utilisée par une modification de dpkg hors de ce lot.

### `ubuntukylin-keyring`

- **Nature** : meta-ou-donnees — **Amont** : Ubuntu Kylin (paquet Ubuntu ubuntukylin-keyring 2014.04.10, dans_ubuntu=1) reconditionné ('Build for openKylin')
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète : 1 trousseau + packaging) ; Gitee/GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : aucun code (données OpenPGP, debian/rules manuel) — 11 fichiers ; 8,2 Ko — lignes de code propres mesurées : 0
- **Privilège** : root (maintainer scripts) ; confiance APT globale via lien dans /etc/apt/trusted.gpg.d
- **Déclencheurs** : installation/suppression du paquet, opérations APT
- **Indices réseau** : aucun
- **Blobs** : 1 trousseau OpenPGP binaire (2251 o) ; md5sums.txt signé PGP (MD5 9417500f... concordant)
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Ancre de confiance APT de 11 fichiers (non installée par défaut), vérification immédiate.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | rules manuel : installe keyrings/ubuntukylin-archive-keyring.gpg dans /usr/share/keyrings, postinst/postrm dans DEBIAN, chown -R root.root, dpkg --build ; construction de l'udeb commentée | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | si configure et dpkg --compare-versions "$2" lt 2014.04.10 : ln -s /usr/share/keyrings/ubuntukylin-archive-keyring.gpg /etc/apt/trusted.gpg.d/ubuntukylin-archive-keyring.gpg | CONSTATÉ |
| maintainer-script | `archive:debian/postrm` | remove/purge : supprime le lien /etc/apt/trusted.gpg.d/ubuntukylin-archive-keyring.gpg | CONSTATÉ |
| maintainer-script (udeb) | `archive:debian/ubuntukylin-keyring-udeb.postinst` | lien archive.gpg ; udeb non construit (rules commenté) | CONSTATÉ |
| ancre de confiance | `archive:keyrings/ubuntukylin-archive-keyring.gpg` | RSA 4096, empreinte 6CE35A4EBAB676094476BE7CD259B7555E1D3C58, UID 'Ubuntu Kylin Archive Automatic Signing Key <council@ubuntukylin.com>', créée 2014-04-09 | CONSTATÉ |

- [CONSTATÉ] Les métadonnées (version 2014.04.10build1-ok4, format 3.0 (native)) ne correspondent pas à l'arbre : debian/changelog en tête 2014.04.10-ok1 (yangtze) et debian/source/format 3.0 (quilt) avec répertoire .pc.
- [CONSTATÉ] Non installé par défaut ; dépôt d'origine ppa-anything3.0.

### `openkylin-archive-anything`

- **Nature** : meta-ou-donnees — **Amont** : openKylin Infrastructure (2023.02.06-ok4 nile, format 3.0 (quilt) avec 2 patchs modifiant openkylin-anything.list)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète : source APT + clé) ; Gitee/GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : aucun code (données) — 16 fichiers ; 6,5 Ko — lignes de code propres mesurées : 0
- **Privilège** : configuration APT système (root à l'installation)
- **Déclencheurs** : installation du paquet, apt update / apt install
- **Indices réseau** : source APT en HTTP vers ppa.build.openkylin.top (client)
- **Blobs** : 1 trousseau OpenPGP binaire (1153 o) + clé ASCII
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Paquet installé par défaut qui ajoute un dépôt APT tiers et une clé de confiance globale : quelques fichiers.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/openkylin-archive-anything.install` | keyrings/openkylin-archive-anything-keyring.gpg -> /etc/apt/trusted.gpg.d/ et /usr/share/keyrings/ ; openkylin-anything.list -> /etc/apt/sources.list.d/ | CONSTATÉ |
| source APT | `archive:openkylin-anything.list` | deb http://ppa.build.openkylin.top/kylinsoft/anything2.0/openkylin/ nile main (sans option signed-by) | CONSTATÉ |
| conffiles | `archive:debian/openkylin-archive-anything.conffiles` | /etc/apt/sources.list.d/openkylin-anything.list (recopié par override_dh_installdeb) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | regenerate-key-fragments (gpg --import gpgkeys/* puis --export) ; checkkeyrings | CONSTATÉ |
| ancre de confiance | `archive:keyrings/openkylin-archive-anything-keyring.gpg` | RSA 4096, empreinte 3EA55C183BD890B2D91C3A202636AEDC49E9E2C8, UID 'openKylin Build Service PPA for kylinsoft developers', créée 2022-06-29 ; même clé que gpgkeys/kylinsoft-ppa.asc | CONSTATÉ |

- [CONSTATÉ] README : dépôt 'anything' reconstruit à partir des sources Ubuntu 20.04 ; suite 'nile' référencée dans un paquet installé par l'ISO 3.0.
- [CONSTATÉ] md5sums.txt (SHA1 94643c70..., SHA256 8c64937a...) concordant avec le trousseau.

### `openkylin-default-settings`

- **Nature** : meta-ou-donnees — **Amont** : openKylin Release SIG (natif 1:3.0.2 huanghe ; Vcs https://gitee.com/openkylin/openkylin-default-settings.git)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 19 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : aucun code applicatif (configurations, JS de préférences Firefox, shell profile.d) — 19 fichiers ; 20 Ko — lignes de code propres mesurées : 0
- **Privilège** : root (maintainer scripts, fichiers /etc système)
- **Déclencheurs** : installation/mise à jour du paquet, chaque shell de connexion (profile.d), démarrage (sysctl), lancement de Firefox (autoconfig), opérations APT
- **Indices réseau** : sources APT en HTTP (client) avec Signed-By; dépendances tirant des services réseau/collecte : avahi-daemon, kylin-daq (à inventorier dans leurs lots)
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Métapaquet installé par défaut définissant sources APT, sysctl, profile.d et autoconfig Firefox : 19 petits fichiers.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/openkylin-default-settings.install` | gschema override ; /etc/profile.d/Z99-zip-gbk.sh ; firefox -> /usr/lib/ et /opt/apps/net.mozilla.firefox ; /etc/apt/sources.list.d/openkylin.sources et openkylin-software.sources ; lightdm 96-ukui-greeter.conf ; /etc/sysctl.d/10-openkylin-sysctl.conf ; fontconfig | CONSTATÉ |
| maintainer-script | `archive:debian/openkylin-default-settings.postinst` | configure : ln -sf 64-openkylin-prefer-cjk.conf ; si md5 de /etc/apt/sources.list = fb7f2a0e9a06692bd34eb45c6df10a89 : sed -i '/yangtze-proposed/d' ; mv distribution.ini Firefox -> .backup | CONSTATÉ |
| maintainer-script | `archive:debian/openkylin-default-settings.postrm` | rm lien fontconfig ; restaure distribution.ini.ubuntu-defaults | CONSTATÉ |
| sources APT | `archive:debian/openkylin.sources ; archive:debian/openkylin-software.sources` | http://archive.build.openkylin.top/openkylin/ suites huanghe, huanghe-security, huanghe-updates (main cross pty) ; http://software.openkylin.top/openkylin/ huanghe (main all) ; Signed-By /usr/share/keyrings/openkylin-archive-keyring.gpg | CONSTATÉ |
| sysctl | `archive:debian/10-openkylin-sysctl.conf` | fs.inotify.max_user_instances = 1024 | CONSTATÉ |
| profile.d | `archive:debian/Z99-zip-gbk.sh` | si LANGUAGE zh_CN* : export UNZIP='-O GBK', ZIPINFO='-O GBK' | CONSTATÉ |
| Firefox autoconfig/politiques | `archive:debian/firefox/browser/defaults/preferences/all-openkylin.js ; archive:debian/firefox/openkylin.cfg ; archive:debian/distribution/policies.json` | general.config.filename=openkylin.cfg, general.config.obscure_value=0 ; openkylin.cfg ne contient que des commentaires ; policies.json verrouille 2 préférences | CONSTATÉ |
| session lightdm | `archive:debian/96-ukui-greeter.conf` | [Seat:*] user-session=kylin-wlcom | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends avahi-daemon, kylin-daq, kylin-os-manager, systemd-resolved, gnome-keyring, openkylin-archive-anything, openkylin-keyring, python3-cups... | CONSTATÉ |

- [CONSTATÉ] Dépendance directe vers kylin-daq et kylin-os-manager (composants à auditer dans leurs lots respectifs).

### `ubuntukylin-default-settings`

- **Nature** : meta-ou-donnees — **Amont** : Ubuntu Kylin (Vcs https://github.com/UbuntuKylin/ubuntukylin-default-settings.git, dans_ubuntu=1) reconditionné ('Build for openKylin')
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 20 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : aucun code applicatif (configurations, shell) — 20 fichiers ; 30 Ko ; taille_source 811 Ko — lignes de code propres mesurées : 0
- **Privilège** : root (maintainer scripts modifiant /etc/os-release et /etc/apt)
- **Déclencheurs** : installation/suppression du paquet, lancement de Firefox, opérations APT
- **Indices réseau** : source APT HTTP vers archive.ubuntukylin.com:10006 (suite focal, client)
- **Blobs** : aucun (1 PNG)
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Paquet de configuration de 20 fichiers modifiant sources APT, os-release et Firefox (non installé par défaut).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/ubuntukylin-default-settings.install` | gschema override ; /etc/profile.d/Z99-zip-gbk.sh ; /etc/apport/native-origins.d/ ; firefox -> /usr/lib/ ; /etc/apt/sources.list.d/ubuntukylin.list ; /usr/share/ubiquity/pixmaps/ | CONSTATÉ |
| maintainer-script | `archive:debian/ubuntukylin-default-settings.preinst` | install/upgrade : dpkg-divert ubuntu_installed.png ; sed -i sur /etc/os-release (NAME, PRETTY_NAME) ; rm incomplete-language-support-gnome.note | CONSTATÉ |
| maintainer-script | `archive:debian/ubuntukylin-default-settings.postinst` | configure : supprime de /etc/apt/sources.list.d/*.list tout fichier (hors ubuntukylin.list) dont le md5 vaut 47d621d5a840d525a1663c12b2ab9e50 ; mv distribution.ini Firefox | CONSTATÉ |
| maintainer-script | `archive:debian/ubuntukylin-default-settings.postrm` | remove/purge : retire la diversion, sed /etc/os-release, restaure distribution.ini | CONSTATÉ |
| source APT | `archive:debian/ubuntukylin.list` | deb http://archive.ubuntukylin.com:10006/ubuntukylin focal main | CONSTATÉ |
| Firefox autoconfig | `archive:debian/firefox/browser/defaults/preferences/all-ubuntukylin.js ; archive:debian/firefox/ubuntukylin.cfg` | general.config.filename=ubuntukylin.cfg ; defaultPref browser.startup.homepage = 'data:text/plain,browser.startup.homepage=http://123.sogou.com/?11458' | CONSTATÉ |
| hook de construction | `archive:hooks/chroot` | sed sur /etc/default/grub (nom de distribution) ; non listé dans .install, destiné à un outil de build | PROBABLE |

- [CONSTATÉ] Les métadonnées (version 26.04-ok1, format 3.0 (native)) ne correspondent pas à l'arbre : debian/changelog en tête 20.04.2-ok1 (yangtze) et debian/source/format 3.0 (quilt).
- [CONSTATÉ] Non installé par défaut ; dépôt d'origine ppa-anything3.0.

### `base-files`

- **Nature** : fork-amont — **Amont** : Debian/Ubuntu base-files (lignée 11 -> 13 -> 13.1 -> 14 dans le changelog ; éléments motd-news issus d'Ubuntu) ; 14-ok3 huanghe, format natif
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, format natif : delta Kylin non séparable sans référence amont) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C (locale-check.c, ~2 Ko), Shell, données (licences, fichiers /etc) — 55 fichiers ; 268 Ko — lignes de code propres mesurées : 82
- **Privilège** : root (paquet Essential, postinst, timer/service systemd)
- **Déclencheurs** : installation/mise à jour, timer motd-news deux fois par jour + 1 min après le démarrage, chaque shell de connexion (profile.d)
- **Indices réseau** : motd-news : récupération HTTPS depuis https://motd.openkylin.top par /etc/update-motd.d/50-motd-news ; ENABLED=0 par défaut ; ce script n'est pas fourni par le paquet (motd/ ne contient que 00-header et 10-help-text)
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (fichiers de configuration et scripts) — effort 1/5 — Paquet Essential root de petite taille (postinst, profile.d sourcé par tous les shells, timer systemd réseau) ; établir aussi le delta vs base-files Ubuntu.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-timer | `archive:debian/motd-news.timer` | OnCalendar=00,12:00:00 ; RandomizedDelaySec=12h ; OnStartupSec=1min ; Persistent=true ; WantedBy=timers.target ; rules : dh_installsystemd -pbase-files motd-news.timer | CONSTATÉ |
| systemd-service | `archive:debian/motd-news.service` | Type=oneshot ; ExecStart=/etc/update-motd.d/50-motd-news --force ; After=network-online.target ; pas de User= (root) | CONSTATÉ |
| configuration | `archive:debian/motd-news` | -> /etc/default/motd-news : ENABLED=0 ; URLS='https://motd.openkylin.top' ; WAIT=5 | CONSTATÉ |
| profile.d | `archive:profile.d/01-locale-fix.sh` | eval $(/usr/bin/locale-check C.UTF-8) (binaire compilé depuis locale-check.c, installé dans /usr/bin) | CONSTATÉ |
| maintainer-script | `archive:debian/postinst.in` | lien /etc/dpkg/origins/default -> openkylin ; première installation : /root/.profile, /root/.bashrc, /etc/profile, /etc/networks depuis /usr/share/base-files ; crée /mnt /srv /opt /media /var/mail... | CONSTATÉ |
| debian-rules | `archive:debian/rules` | installe etc/*, motd/* -> /etc/update-motd.d (755), origins/*, os-release -> /usr/lib/os-release ; chmod 1777/2775 et 700 sur root ; compile locale-check | CONSTATÉ |
| identité de distribution | `archive:etc/os-release ; archive:etc/lsb-release ; archive:origins/openkylin` | ID=openkylin, VERSION='3.0 (huanghe)', PRODUCT_FEATURES=3 ; Vendor-URL http://www.openkylin.top/ | CONSTATÉ |
| debian-control | `archive:debian/control` | Essential: yes ; Priority required ; Pre-Depends awk ; paquet udeb lsb-release-udeb | CONSTATÉ |
| systemd-timer (reseau) [ajout critique] | `archive:base-files/debian/motd-news.timer ; debian/motd-news.service ; debian/motd-news` | Timer OnCalendar=00,12:00:00 et OnStartupSec=1min, qui lance '/etc/update-motd.d/50-motd-news --force' ; ce script est absent de l'arbre (NON COUVERT). /etc/default/motd-news contient ENABLED=0 et URLS="https://motd.openkylin.top". Le recensement (net=1) ne cite pas cette URL. | NON COUVERT |

- [CONSTATÉ] base-files n'apparaît pas dans community/permissions/gitee.json alors que la forge Gitee est notée PRESENT.

### `lsb`

- **Nature** : fork-amont — **Amont** : Debian lsb 11.6 (changelog 11.6-ok1 nile 'Sync upstream version') + 1 patch openKylin (0001-Fix-I8HPTC.patch)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, format 3.0 (quilt), patch appliqué) ; Gitee/GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : Python (lsb_release.py, initdutils.py), Shell (init-functions, install_initd...) — 31 fichiers ; 90 Ko — lignes de code propres mesurées : 946
- **Privilège** : aucun (paquet vide)
- **Déclencheurs** : aucun
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Le seul paquet installé (lsb-base) est un paquet de transition vide ; contrôle du contenu du .deb et de l'unique patch.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | un seul paquet binaire : lsb-base (Section oldlibs, 'transitional package', 'This is an empty package') ; Depends sysvinit-utils (>= 3.05-4~) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' ; aucun .install/.postinst | CONSTATÉ |
| patch | `archive:debian/patches/0001-Fix-I8HPTC.patch` | modifie get_os_release() dans lsb_release.py (casse 'openKylin') | CONSTATÉ |

- [PROBABLE] Le patch porte sur lsb_release.py, qui n'est pas installé par le paquet binaire produit.
- [CONSTATÉ] Distribution de tête du changelog 'nile' pour un paquet (lsb-base) installé par l'ISO 3.0.

### `casper`

- **Nature** : fork-amont — **Amont** : Ubuntu casper 1.445 (format natif ; entrées openKylin 1.498-ok1 puis 1.445-ok9 à 1.445-ok11.6, distribution nile ; modifications 99wayland/kylin-wlcom, BUILD_SYSTEM)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 96 fichiers ; delta Kylin non séparé en patchs) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : Shell (scripts initramfs : casper, casper-helpers, 42 scripts casper-bottom, bin/*), C (casper-md5check ~24 Ko) — 96 fichiers ; 228 Ko — lignes de code propres mesurées : 905
- **Privilège** : root (initramfs de l'ISO live, session live, service d'arrêt)
- **Déclencheurs** : démarrage live boot=casper (initramfs casper-premount/casper-bottom), update-initramfs (postinst), arrêt du système live (final.target)
- **Indices réseau** : aucun listener signalé; scripts casper-bottom 41apt_cdrom, 57pollinate, 58server_network et Depends cifs-utils (tag protocol::smb) : contenu non lu
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (fichiers de configuration et scripts) — effort 2/5 — Fork natif d'Ubuntu casper 1.445 exécuté en root dans l'initramfs de l'ISO : diff contre casper 1.445 d'Ubuntu puis lecture des scripts modifiés et de 25adduser, 44pk_allow_ubuntu et 99wayland.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:debian/casper.service` | ExecStart=/sbin/casper-stop ; DefaultDependencies=no ; StandardInput=tty-force ; WantedBy=final.target ; rules : dh --with systemd, dh_systemd_start --no-start | CONSTATÉ |
| maintainer-script | `archive:debian/casper.postinst` | update-initramfs -u | CONSTATÉ |
| debian-install | `archive:debian/casper.install ; archive:debian/casper.dirs` | casper-getty, casper-login, casper-new-uuid, casper-snapshot, casper-stop -> /sbin ; scripts/, hooks/, conf-hooks.d -> /usr/share/initramfs-tools ; casper-md5check -> /usr/lib/casper ; casper.conf -> /etc | CONSTATÉ |
| debian-install | `archive:debian/ubiquity-casper.install` | casper-a11y-enable, casper-reconfigure -> /usr/bin ; ubiquity-hooks/* -> /usr/lib/ubiquity/target-config | CONSTATÉ |
| script initramfs (classé systemd-unit par l'extracteur) | `archive:scripts/casper-bottom/25disable_cdrom.mount` | script shell : ln -s /dev/null /root/lib/systemd/system/cdrom.mount | CONSTATÉ |
| sudoers (session live) | `archive:scripts/casper-bottom/25adduser` | l.76-77 : echo '${USERNAME}  ALL=(ALL) NOPASSWD: ALL' > /root/etc/sudoers.d/casper | CONSTATÉ |
| polkit (session live) | `archive:scripts/casper-bottom/44pk_allow_ubuntu ; archive:scripts/casper-bottom/32disable_hibernation` | écrit /var/lib/polkit-1/localauthority/10-vendor.d/10-live-cd.pkla (ResultAny=no, ResultActive=yes) ; disable-hibernate.pkla (ResultActive=no) | CONSTATÉ |
| configuration | `archive:casper.conf ; archive:conf-hooks.d/casper` | USERNAME='ubuntu', HOST='ubuntu', BUILD_SYSTEM='Ubuntu' (réécrit par le Makefile via lsb_release) ; FRAMEBUFFER=y | CONSTATÉ |
| initramfs-hook | `archive:hooks/casper` | contenu non lu | NON COUVERT |

- [CONSTATÉ] Le changelog contient une entrée 1.498-ok1 antérieure aux entrées 1.445-ok9 et suivantes (numérotation de version décroissante).

### `live-build`

- **Nature** : fork-amont — **Amont** : Debian live-build 3.0~a57 (Debian Live Project, Daniel Baumann ; orig IDENTIQUE à Debian/Ubuntu) avec patchs Ubuntu (ubuntu-*.patch) et 35 patchs openKylin 0101-0135 ; 3.0~a57-ok24 nile
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète + 135 patchs quilt, ~8,8 k lignes de patch) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : Shell (scripts lb_*), HTML (frontend CGI) — 428 fichiers ; 1,5 Mo — lignes de code propres mesurées : 3 023
- **Privilège** : root pendant la construction d'ISO ; cron root si live-build-cgi installé
- **Déclencheurs** : exécution manuelle de lb, cron chaque minute (live-build-cgi)
- **Indices réseau** : frontend CGI web (live-build-cgi, nécessite httpd); git clone et sendmail dans le cron; téléchargements depuis les miroirs pendant la construction
- **Blobs** : aucun ; 6 liens symboliques absolus vers /usr/lib/syslinux/*
- **Budget proposé** : L2 — 50 % (fichiers de configuration et scripts) — effort 2/5 — Outil de construction d'ISO (chaîne d'approvisionnement) non installé par défaut : lecture des 35 patchs openKylin 0101-0135, amont Debian identique.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cron | `archive:frontends/cgi/live-build-cgi.crontab` | */1 * * * * root /etc/cron.hourly/live-build-cgi | CONSTATÉ |
| cron (script) | `archive:frontends/cgi/live-build-cgi.cron` | si _WEBBUILD=enabled dans /etc/default/live-build-cgi : traite ${_TEMPDIR}/*.build, git clone ${_LB_CONFIG}, lb config/lb build, envoi via /usr/sbin/sendmail, umount | CONSTATÉ |
| debian-control | `archive:debian/control` | paquets live-build et live-build-cgi (Recommends apache2 \| httpd, cron) ; live-build Depends debootstrap, cpio, initramfs-tools, dpkg-dev, apt-utils | CONSTATÉ |
| debian-rules | `archive:debian/rules` | chmod +x share/hooks/100-build-wubildr.binary ; déplace usr/share/live/build/cgi dans live-build-cgi ; liens vers /usr/lib/syslinux et /usr/lib/ISOLINUX | CONSTATÉ |
| apt-hook (construction) | `archive:debian/patches/0116-build-apt-etc-apt-live-build-sources.list-sources.li.patch` | écrit chroot/etc/apt/apt.conf.d/99-live-build (Dir::Etc::sourcelist live-build/sources.list) et les sources du chroot de construction | CONSTATÉ |
| patchs openKylin | `archive:debian/patches/series` | 135 entrées ; patchs ostree (0105, 0115, 0129-0132), kylin-v11/kare (0125, 0126), debootstrap (0119), dpkg force-confnew (0121), firmware (0123) | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut ; changelog ok24 (nile) référence 'V11-2602-RC5' (lignée Kylin V11).

### `debootstrap`

- **Nature** : fork-amont — **Amont** : Debian debootstrap 1.0.134 (salsa.debian.org/installer-team/debootstrap) ; 1.0.134-ok3 nile ('Build for openKylin nile-sp2-rc2'), format natif
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 46 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : Shell (debootstrap, functions, scripts/*) — 46 fichiers ; 236 Ko — lignes de code propres mesurées : 0
- **Privilège** : root à l'exécution (outil d'amorçage de chroot)
- **Déclencheurs** : invocation manuelle ou par un outil de construction/installation
- **Indices réseau** : téléchargement HTTP des .deb depuis un miroir (client, wget)
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Fork Debian quasi inchangé : diff contre debootstrap 1.0.134 et contrôle des scripts de suite openKylin.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | debootstrap (Depends wget ; Recommends gnupg, ${keyring}) et debootstrap-udeb | CONSTATÉ |
| debian-rules | `archive:debian/rules` | KEYRING selon dpkg-vendor : ubuntu-keyring / tanglu / kali / debian-archive-keyring ; make install DESTDIR | CONSTATÉ |
| scripts de suite | `archive:scripts/nile ; archive:scripts/yangtze` | liens symboliques vers scripts/gutsy : default_mirror http://archive.ubuntu.com/ubuntu ou http://ports.ubuntu.com/ubuntu-ports, keyring /usr/share/keyrings/ubuntu-archive-keyring.gpg ; aucun script 'huanghe' | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO 3.0 ; distribution de tête du changelog 'nile'.

### `ubiquity`

- **Nature** : fork-amont — **Amont** : Ubuntu ubiquity 24.04.5 (git.launchpad.net/ubiquity) ; une seule entrée openKylin 24.04.5-ok1 nile 'Build for openKylin' ; format 3.0 (quilt) sans debian/patches (.pc/applied-patches vide)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 4328 fichiers, dont 33 composants debian-installer sous d-i/source)
- **Langages / taille** : Python (~1,25 Mo), C (composants d-i ~424 Ko), Shell, Qt UI, XML — 4328 fichiers ; 57,4 Mo ; taille_source 10,5 Mo — lignes de code propres mesurées : 57 828
- **Privilège** : root (installateur et oem-config)
- **Déclencheurs** : démarrage live graphical.target, premier démarrage OEM (oem-config.target), lancement via polkit/pkexec
- **Indices réseau** : d-i/source/netcfg/wpa_ctrl.c l.110 : bind() d'un socket AF_UNIX local /tmp/wpa_ctrl_<pid>-<n> (client du contrôle wpa_supplicant), pas d'écoute réseau; composants d-i réseau embarqués : netcfg, choose-mirror, apt-setup, preseed (network-preseed)
- **Blobs** : aucun ELF ; fichiers Release/InRelease Ubuntu groovy embarqués (d-i/source/apt-setup/release-files/, ~267 Ko chacun)
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Fork Ubuntu non installé par défaut, sans patch openKylin identifiable : vérification du diff vs ubiquity 24.04.5.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:debian/ubiquity.ubiquity.service` | ExecStart=/usr/share/ubiquity/start-ubiquity-dm ; Conflicts=getty@tty1.service ; WantedBy=graphical.target ; root | CONSTATÉ |
| systemd-service+target | `archive:data/oem-config.service ; archive:data/oem-config.target` | exec oem-config-firstboot $debug $automatic (options /proc/cmdline) ; TTYPath=/dev/tty1 ; installés dans /usr/lib/oem-config/ (debian/oem-config.install) | CONSTATÉ |
| polkit | `archive:data/com.ubuntu.ubiquity.policy.in` | actions com.ubuntu.ubiquity.ubiquity (exec.path /usr/bin/ubiquity) et com.ubuntu.ubiquity.oem-config-prepare (/usr/sbin/oem-config-prepare) : auth_admin pour any/inactive/active | CONSTATÉ |
| systemd-service (d-i) | `archive:d-i/source/console-setup/debian/console-setup-linux.console-setup.service ; .keyboard-setup.service ; .setvtrgb.service` | unités console-setup embarquées (non lues en détail) | NON COUVERT |
| udev | `archive:d-i/source/hw-detect/net-hotplug.rules ; archive:d-i/source/console-setup/init/90-console-setup.rules` | SUBSYSTEM=='net' RUN+='/etc/hotplug.d/net/hw-detect.hotplug' ; vtconsole/vc RUN+='/etc/console-setup/cached_setup_*.sh' | CONSTATÉ |
| maintainer-script | `archive:debian/ubiquity.postinst ; archive:debian/oem-config.postinst` | ubiquity.postinst : update-rc.d -f ubiquity remove si /etc/rc2.d/S20ubiquity ; oem-config.postinst : confmodule seul ; 41 scripts mainteneur au total dont 32 sous d-i/source (non lus) | CONSTATÉ |
| autostart / sources APT (tests) | `archive:autopilot/ubiquity-autopilot-runner/custom-installation/iso-override/etc/xdg/autostart/autopilot.desktop ; .../etc/apt/sources.list.d/all.list` | Exec=/usr/local/bin/run-autopilot.sh -R ; deb http://archive.ubuntu.com/ubuntu/ RELEASE main restricted universe multiverse (infrastructure de test autopilot) | CONSTATÉ |
| debian-control | `archive:debian/control` | 11 paquets binaires : ubiquity, ubiquity-frontend-{gtk,gtk-panel,kde,debconf}, oem-config, oem-config-{gtk,kde,debconf,remaster}, ubiquity-ubuntu-artwork | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut ; les unités oem-config de kylin-os-installer reprennent la structure de celles d'ubiquity.

### `openkylin-docker-images`

- **Nature** : propre-kylin — **Amont** : openKylin (Gitee openkylin/openkylin-docker-images, branche master) : recettes Dockerfile empaquetant des logiciels tiers
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (244 fichiers) ; rootfs de l'image de base absent du dépôt
- **Langages / taille** : Dockerfile (67), YAML, Shell, Python (validation ROS), Go/C (tests distroless) — 244 fichiers ; 3,0 Mo — lignes de code propres mesurées : 1 375
- **Privilège** : aucun sur le système installé ; root dans les conteneurs pendant la construction
- **Déclencheurs** : construction/publication d'images (pipeline CI non visible)
- **Indices réseau** : téléchargements au build (curl, pip, git); ports exposés par les images serveurs (nginx, mariadb, postgres, etcd, kafka...) non inventoriés
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 2/5 — Recettes de conteneurs hors ISO : triage de la chaîne d'approvisionnement (image de base, téléchargements exécutés sans vérification).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| build (image de base) | `gitee:Base/openkylin/Dockerfile` | FROM scratch ; ADD openKylin-docker-rootfs.$TARGETARCH.tar.xz / (tarball absent du dépôt) ; sed TMOUT=300 -> TMOUT=0 dans /etc/bash.bashrc ; CMD /bin/bash | CONSTATÉ |
| build (images applicatives) | `gitee:AI/, BigData/, Cloud/, Database/, Distroless/, HPC/, Others/, Storage/` | 67 fichiers Dockerfile* (FROM ${BASE}, openkylin/openkylin:2.0, ROS...) | CONSTATÉ |
| téléchargement exécuté au build | `gitee:AI/hermes-agent/2026.7.7.2/2.0/Dockerfile (et 5 autres)` | 9 occurrences 'curl ... \| sh' dans 6 Dockerfiles (https://sh.rustup.rs, https://astral.sh/uv/install.sh) : picoclaw, atomcode, codewhale, vllm-cpu, hermes-agent (x2), kserve storage-initializer | CONSTATÉ |
| faux positif extracteur | `gitee:Others/ros/lyrical/3.0/Dockerfile.desktop ; gitee:Others/ros/jazzy/2.0/Dockerfile.desktop` | classés desktop-file mais ce sont des Dockerfile (variante ROS desktop) | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive et non installé par l'ISO ; README : tag d'image de base publié '2.0'.
- [NON COUVERT] La provenance et le contenu du rootfs openKylin-docker-rootfs.*.tar.xz n'ont pas été établis.

### `release-management`

- **Nature** : meta-ou-donnees — **Amont** : openKylin Release SIG (Gitee openkylin/release-management, master ; GitHub DIVERGENT)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (29 fichiers : documentation Markdown et images)
- **Langages / taille** : aucun code (Markdown, PNG) — 29 fichiers ; 509 Ko — lignes de code propres mesurées : 0
- **Privilège** : aucun
- **Déclencheurs** : aucun
- **Indices réseau** : aucun
- **Blobs** : aucun (PNG de documentation)
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Documentation sans code ; utile seulement pour cadrer le périmètre 3.0 et le processus de publication.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| documentation | `gitee:openKylin-3.0/openKylin-3.0版本发行说明.md ; gitee:openKylin-3.0/子文档/` | notes de version 3.0, listes de correctifs, nouvelles fonctions, matériel supporté (ARM, LoongArch, RISC-V, x86) ; aucun point d'entrée exécutable | CONSTATÉ |
| documentation | `gitee:openKylin版本发行规范.md ; gitee:Release SIG Maintainer增选规范.md ; gitee:openKylin-2.0/系统核心组件包选型基线.md` | règles de publication, cooptation des mainteneurs, base de sélection des composants | CONSTATÉ |

### `server-package-list`

- **Nature** : meta-ou-donnees — **Amont** : openKylin (Gitee server-package-list, branche main)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (13 fichiers)
- **Langages / taille** : Shell (scripts/render-profile.sh, 172 lignes), listes de paquets, Markdown — 13 fichiers ; 27 Ko — lignes de code propres mesurées : 172
- **Privilège** : aucun
- **Déclencheurs** : exécution manuelle pour produire les listes d'images
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Listes de paquets et un script de rendu de 172 lignes, sans privilège : utile pour le périmètre de l'image serveur.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| build (rendu de profil) | `gitee:scripts/render-profile.sh` | génère la liste de paquets d'un profil d'image ; grep curl\|wget\|http\|sudo\|eval : aucun résultat | CONSTATÉ |
| données de build | `gitee:package-lists/profiles.conf ; gitee:package-lists/*.list ; gitee:generated/*.packages` | profils server-qemu-riscv64.img, normal-qemu-riscv64.img, server-amd64.iso composés de base/server/normal/amd64/iso-live/qemu-riscv64 | CONSTATÉ |

### `community`

- **Nature** : meta-ou-donnees — **Amont** : openKylin (Gitee openkylin/community, master + nombreuses branches openkylin-cibot_* ; GitHub DIVERGENT)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (531 fichiers : gouvernance, SIG, droits des dépôts, outillage Fortify)
- **Langages / taille** : JSON, XML, YAML, Python (2 scripts Fortify), documents — 531 fichiers ; 33 Mo (permissions/gitee.json 12,9 Mo ; règles Fortify core_cpp_2022_2.xml 11 Mo) — lignes de code propres mesurées : 157
- **Privilège** : aucun sur le système ; les droits décrits gouvernent l'écriture dans les dépôts sources
- **Déclencheurs** : régénération par le bot openkylin-cibot (branches openkylin-cibot_*)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (fichiers de configuration et scripts) — effort 1/5 — Données de gouvernance sans code exécuté sur le système : à exploiter comme référentiel des comptes ayant droit d'écriture.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| droits des dépôts (identité/confiance) | `gitee:permissions/gitee.json ; gitee:permissions/README` | droits pull/push/admin pour 4687 dépôts, 'générés automatiquement par cibot à partir des informations sig et ppm' ; pour les 21 dépôts du lot présents, chaque compte push est aussi admin | CONSTATÉ |
| métadonnées | `gitee:packages/*.yaml ; gitee:sig/*/sig.yaml` | 27 fiches de paquets et 162 entrées sig/ | CONSTATÉ |
| outillage sécurité | `gitee:sig/SecurityGovernance/可信开发/Fortify扫描脚本/` | xml_issuecount.py, rule/rule_vc.py, fortify_sca.x, xml-count.x, règles et modèles XML | CONSTATÉ |

- [CONSTATÉ] ostree-maintain-daemon, ostree-coredata-protect, ostree-extra-utils, ubuntukylin-keyring et base-files sont absents de permissions/gitee.json.

#### Constats transverses du lot B05-installateur-ostree-P3

- **[CONSTATÉ]** 3 noms de bus D-Bus système du lot sont activés en root : com.kylinusbcreator.systemdbus (kylin-usb-creator), com.kylin.backup (fourni à l'identique par yhkylin-backup-tools et yhkylin-rockbackup-tools) et com.kylin.systembackup (yhkylin-systembackup-tools). — *preuve* : archive kylin-usb-creator:registeredQDbus/conf/com.kylinusbcreator.systemdbus.service 'User=root' ; archive yhkylin-backup-tools:backup-daemon/data/com.kylin.backup.service 'Exec=/usr/bin/backup-daemon / User=root' ; archive yhkylin-systembackup-tools:systembackup-daemon/data/com.kylin.systembackup.service 'User=root'
- **[CONSTATÉ]** Dans les politiques D-Bus des 3 paquets yhkylin-*, le contexte default autorise send_interface=...manager sans attribut send_destination. — *preuve* : backup-daemon/data/com.kylin.backup.conf : '<policy context="default"> <allow send_interface="com.kylin.backup.manager"/>' ; systembackup-daemon/data/com.kylin.systembackup.conf : '<allow send_interface="com.kylin.systembackup.manager"/>'
- **[CONSTATÉ]** Les 3 politiques polkit yhkylin-* déclarent deux éléments <allow_active> (auth_admin puis auth_admin_keep) pour la même action. — *preuve* : backup-daemon/data/com.kylin.backup.policy et systembackup-daemon/data/com.kylin.systembackup.policy : '<allow_active>auth_admin</allow_active> <allow_active>auth_admin_keep</allow_active>'
- **[CONSTATÉ]** 3 paquets installés par défaut par l'ISO 3.0 (ostree-maintain-daemon, ostree-coredata-protect, ostree-extra-utils) n'ont dans l'archive qu'un squelette dh_make sans code ; aucune forge Gitee/GitHub. — *preuve* : archive ostree-*/debian/rules : '%: dh $@' seul ; fichiers debian/*.ex ; fiche JSON : source_coquille_vide=1, forges gitee/github ABSENT, installe_par_defaut_iso non vide
- **[CONSTATÉ]** Des sources contiennent des artefacts de build ELF commités : yhkylin-backup-tools (9 ELF, 88,2 Mo), yhkylin-rockbackup-tools (7 ELF, 67,6 Mo), yhkylin-systembackup-tools (2 ELF, 27,6 Mo), ukui-iso-customer (2 ELF PyInstaller identiques de 9,76 Mo). — *preuve* : fiche JSON struct_*.blobs (signature 7f454c46) : kybackup/kybackup, backup-daemon/backup-daemon, systembackup-daemon/systembackup-daemon, iso-customizer/dist/iso-customizer ; cmp dist/iso-customizer = iso-customizer-deb/usr/bin/iso-customizer
- **[CONSTATÉ]** Les sources APT installées par défaut par le lot utilisent http:// ; openkylin-anything.list n'a pas d'option signed-by et sa clé est placée dans /etc/apt/trusted.gpg.d. — *preuve* : openkylin-default-settings:debian/openkylin.sources 'URIs: http://archive.build.openkylin.top/openkylin/ ... Signed-By:' ; openkylin-archive-anything:openkylin-anything.list 'deb http://ppa.build.openkylin.top/kylinsoft/anything2.0/openkylin/ nile main' ; debian/openkylin-archive-anything.install 'keyrings/... etc/apt/trusted.gpg.d/'
- **[CONSTATÉ]** Deux clés de confiance APT sont installées dans /etc/apt/trusted.gpg.d (confiance globale) par des paquets installés par défaut ; une troisième (ubuntukylin) l'est via un lien créé en postinst. — *preuve* : openkylin-keyring:debian/openkylin-keyring.install 'openkylin-archive-keyring.gpg etc/apt/trusted.gpg.d/' ; openkylin-archive-anything:debian/openkylin-archive-anything.install ; ubuntukylin-keyring:debian/postinst 'ln -s /usr/share/keyrings/ubuntukylin-archive-keyring.gpg /etc/apt/trusted.gpg.d/...'
- **[CONSTATÉ]** Des scripts du lot écrivent des règles sudoers NOPASSWD : installateur (mode OEM) et casper (session live). — *preuve* : kylin-os-installer:scripts/in_chroot/21setup-user l.24 'oem ALL=(ALL) NOPASSWD: ALL' ; scripts/oem.sh l.371 '${userName} ALL=(ALL) NOPASSWD: ALL' ; casper:scripts/casper-bottom/25adduser l.77 '${USERNAME}  ALL=(ALL) NOPASSWD: ALL'
- **[CONSTATÉ]** Plusieurs paquets du lot installés par l'ISO 3.0 ont une entrée de tête de debian/changelog dans une autre distribution que huanghe. — *preuve* : kylin-fix-boot-grub 3.0.1.1-0k0.21 'v11-2603-preview' ; casper 1.445-ok11.6 'nile' ; debootstrap 1.0.134-ok3 'nile' ; lsb 11.6-ok1 'nile' ; openkylin-keyring 2022.05.12-ok3 'nile' ; openkylin-archive-anything 2023.02.06-ok4 'nile' ; ostree-extra-utils 'nile.bedrock'
- **[CONSTATÉ]** Pour ostree-coredata-protect, la version installée par l'ISO diffère de la version de l'archive. — *preuve* : fiche JSON : installe_par_defaut_iso 'ostree-coredata-protect=2.0.0.0-0k2.23.6.0' vs meta.version '2.0.0.0-0k2.23.7.0' (debian/changelog daté du 21 Sep 2026)
- **[CONSTATÉ]** Pour ubuntukylin-keyring et ubuntukylin-default-settings, les métadonnées de l'archive (format natif, versions 2014.04.10build1-ok4 et 26.04-ok1) ne correspondent pas à l'arbre extrait. — *preuve* : debian/source/format '3.0 (quilt)' + répertoire .pc ; debian/changelog en tête 'ubuntukylin-keyring (2014.04.10-ok1) yangtze' et 'ubuntukylin-default-settings (20.04.2-ok1) yangtze'
- **[CONSTATÉ]** 4 paquets du lot régénèrent l'initramfs depuis leurs scripts mainteneur ; les 3 paquets yhkylin-* installent hooks initramfs et scripts init-bottom exécutés au démarrage. — *preuve* : debian/yhkylin-{backup,rockbackup,systembackup}-tools.postinst et .postrm 'update-initramfs -u' ; casper:debian/casper.postinst 'update-initramfs -u' ; .install : '.../initramfs-tools/scripts/local-bottom/... usr/share/initramfs-tools/scripts/init-bottom'
- **[CONSTATÉ]** Le SDK de collecte de données kysdk-datacollect (kdk_dia_*) est appelé par kylin-usb-creator et par l'UI kybackup de yhkylin-backup-tools et yhkylin-rockbackup-tools ; pas d'appel relevé dans yhkylin-systembackup-tools, kylin-os-installer ni kylin-fix-boot-grub. — *preuve* : kylin-usb-creator:UIControl/src/mainwindow.cpp l.618 'kdk_dia_upload_default(node, "start-usrcreator-capture", "startMake")' ; yhkylin-backup-tools:kybackup/main.cpp (grep kdk_dia_\|libkydatacollect)
- **[CONSTATÉ]** Aucun indice d'écoute réseau du lot n'est un serveur réseau : les QLocalServer sont des sockets locaux QtSingleApplication ou des chaînes de traduction, et le seul bind() est un client AF_UNIX wpa_ctrl. — *preuve* : kylin-usb-creator:UIControl/src/include/qtlocalpeer.cpp l.121 'server->listen(socketName)' ; kylin-os-installer:language/qt_zh_CN.ts l.2764 '<name>QLocalServer</name>' ; ubiquity:d-i/source/netcfg/wpa_ctrl.c l.110 bind() sur sun_path '/tmp/wpa_ctrl_%d-%d'
- **[CONSTATÉ]** Dans community/permissions/gitee.json, pour les 21 dépôts du lot présents, le nombre de comptes ayant push est égal au nombre de comptes admin. — *preuve* : gitee community:permissions/gitee.json (généré par cibot) : kylin-usb-creator push=50 admin=50, yhkylin-backup-tools 50/50, kylin-os-installer 17/17, openkylin-keyring 12/12

