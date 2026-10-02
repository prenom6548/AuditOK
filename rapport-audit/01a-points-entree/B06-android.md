# Lot B06-android

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kmre`

- **Nature** : meta-ou-donnees — **Amont** : Kylin (KylinSoft). Métapaquet de l'environnement KMRE (Kylin Mobile Runtime Environment). Le README Gitee indique que la partie Android est construite depuis AOSP via le manifeste gitee.com/openkylin/platform_manifest.git, branche kmre-opensource-devel (BUILD_ID RP1A.201005.006).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette dh_make vide (19 fichiers, uniquement debian/, malgré meta.source_coquille_vide=0) + gitee README.md seul (1 fichier, 14 207 o). Aucun code.
- **Langages / taille** : aucun code (debian/ et Markdown uniquement) — archive : 19 fichiers / 26 199 o ; gitee : 1 fichier / 14 207 o — lignes de code propres mesurées : 0
- **Privilège** : Aucun binaire propre. Tire par dépendance un service D-Bus système root (kylin-kmre-daemon), docker.io et haveged.
- **Déclencheurs** : installation apt du métapaquet (non installé par défaut par l'ISO 3.0)
- **Indices réseau** : aucun dans l'arbre ; dépendance déclarée à docker.io (archive:debian/control)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Métapaquet sans code : vérifier la liste de dépendances et, si le .deb est acquis, l'absence de scripts mainteneur.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquet binaire kmre (amd64 et arm64) ; Depends: kylin-kmre-daemon, kylin-kmre-display-control, libkylin-kmre-emugl, kylin-kmre-image-data-x64 (arm64 : kylin-kmre-image-data), kylin-kmre-image-update-x64, kylin-kmre-manager, kylin-kmre-window, libkylin-kmre, docker.io, haveged, python3 ; Original-Maintainer: lixiang@kylinos.cn | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make : seule la règle '%: dh $@' ; aucune surcharge (md5 78f72f63bcffd66920908090d06b2d59, identique pour les 14 squelettes du lot) | CONSTATÉ |
| maintainer-script (modèles inactifs) | `archive:debian/postinst.ex, postrm.ex, preinst.ex, prerm.ex, kmre.cron.d.ex` | Modèles dh_make en .ex, non utilisés par debhelper ; postinst.ex ne contient que le squelette case/esac | CONSTATÉ |
| documentation de construction | `gitee:README.md` | Procédure : repo init AOSP (platform_manifest kmre-opensource-devel), make img → system.sfs, kylin-kmre-make-image (sudo make) → kmre3_*.tar copié en data/amd64/kmre-container-image.tar du paquet kylin-kmre-image-data ; liste les 10 dépôts Gitee hôtes KMRE | CONSTATÉ |
| contenu du .deb binaire publié | `(paquet binaire kmre non présent dans le corpus)` | Scripts mainteneur et fichiers réels du binaire kmre publié non examinés | NON COUVERT |

- [CONSTATÉ] meta.source_coquille_vide vaut 0 alors que l'arbre archive ne contient que les 19 fichiers du squelette dh_make (aucun fichier hors debian/).
- [PROBABLE] Le debian/control de l'archive porte des champs typiques d'un paquet binaire (Original-Maintainer, deux stanzas Package du même nom par architecture). Cela suggère un control reconstitué à partir des .deb publiés et non un source réellement construit.

### `kylin-kmre-daemon`

- **Nature** : propre-kylin — **Amont** : KylinSoft (copyright 'KylinSoft Co., Ltd. 2016-2024' ; mainteneur Machao <machao@kylinos.cn>). Composant tiers intégré : SimpleIni (src/simpleIni/).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k0.14) + gitee code (master, commit 31ae0d6, changelog 3.0.0.0-0k0.1). La version Gitee ne correspond pas à la version archive.
- **Langages / taille** : C++ (180 Ko), en-têtes C/C++ (205 Ko dont SimpleIni.h 130 Ko), C (68 Ko, FUSE), XML D-Bus, Shell (script kylin-kmre-load-image) — archive : 19 fichiers / 26 927 o ; gitee : 79 fichiers / 514 066 o, dont 47 fichiers de code pour 15 273 lignes — lignes de code propres mesurées : 15 273
- **Privilège** : root : service D-Bus système activable par tout utilisateur (policy default send_destination) et unité systemd activée au boot. Pilote docker et modprobe (indices grep). Binaires FUSE.
- **Déclencheurs** : boot (multi-user.target, activé par postinst), activation D-Bus système par n'importe quel client (cn.kylinos.Kmre), postinst configure (systemctl start), événements udev rfkill, connexion shell (profile.d)
- **Indices réseau** : aucun listen relevé par l'extraction automatique; méthode D-Bus GetContainerNetworkInformation (gitee:src/kmre-introspect.xml); gestion de conteneurs docker (indices grep sur /usr/bin/docker et docker.socket)
- **Blobs** : aucun
- **Budget proposé** : L3 — 95 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Service root propre Kylin d'environ 15 k lignes (SimpleIni tiers exclu), exposé sur le bus système, avec chemin de mise à jour (ComponentsUpgrade, LoadImage), docker, modprobe et FUSE.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `gitee:data/kylin-kmre-daemon.service` | Type=dbus ; BusName=cn.kylinos.Kmre ; ExecStart=/usr/bin/kylin-kmre-daemon ; pas de User= (donc root) ; StandardOutput/StandardError=null ; KillMode=process ; WantedBy=multi-user.target ; Alias=dbus-cn.kylinos.Kmre.service. Aucune directive de durcissement dans l'unité. Installé dans /lib/systemd/system (data/CMakeLists.txt) | CONSTATÉ |
| dbus-service (système) | `gitee:data/cn.kylinos.Kmre.service` | Name=cn.kylinos.Kmre ; Exec=/usr/bin/kylin-kmre-daemon ; User=root ; SystemdService=kylin-kmre-daemon.service ; installé dans share/dbus-1/system-services/ | CONSTATÉ |
| dbus-policy (système) | `gitee:data/cn.kylinos.Kmre.conf` | policy user=root : own, send_destination et receive_sender pour cn.kylinos.Kmre ; policy context=default : allow send_destination=cn.kylinos.Kmre (envoi ouvert à tous les utilisateurs) ; installé dans /etc/dbus-1/system.d/ | CONSTATÉ |
| interface D-Bus | `gitee:src/kmre-introspect.xml` | Interface cn.kylinos.Kmre, 19 méthodes : Prepare, StartContainer, StartContainerSilently, ChangeContainerRuntimeStatus, StopContainer, SetFocusOnContainer, Set/GetPropOfContainer, Set/GetDefaultPropOfContainer, GetContainerNetworkInformation, SetGlobalEnvironmentVariable, LoadImage, IsImageReady, GetAllContainersAndImages, SwitchImage, RemoveOneContainer, RemoveOneImage, ComponentsUpgrade ; 12 signaux | CONSTATÉ |
| systemd-service (non installé) | `gitee:data/kylin-kmre-load-image.service` | Type=oneshot ; ExecStart=/usr/bin/kylin-kmre-load-image ; ConditionPathExists=/usr/share/kmre/kmre-container-image.tar ; After=docker.service haveged.service ; WantedBy=multi-user.target. Son install() est commenté dans data/CMakeLists.txt, mais le script kylin-kmre-load-image est installé dans bin (src/CMakeLists.txt, PROGRAMS) | CONSTATÉ |
| maintainer-script postinst | `gitee:debian/kylin-kmre-daemon.postinst` | configure : systemctl enable kylin-kmre-daemon.service ; systemctl daemon-reload ; udevadm control --reload ; systemctl start kylin-kmre-daemon.service ; 'set -e' commenté | CONSTATÉ |
| maintainer-script postrm | `gitee:debian/kylin-kmre-daemon.postrm` | systemctl daemon-reload ; udevadm control --reload | CONSTATÉ |
| udev | `gitee:data/99-kmre.rules` | SUBSYSTEM=="rfkill", ENV{SYSTEMD_RFKILL}="1" ; installé dans /etc/udev/rules.d/ | CONSTATÉ |
| profile.d | `gitee:data/kmre-env.sh` | export ANDROID_ENV=1 ; installé dans /etc/profile.d/ (sourcé à chaque connexion shell) | CONSTATÉ |
| rsyslog / logrotate | `gitee:data/99-kmre-rsyslog.conf ; gitee:data/kmre-logrotate` | Filtre syslogtag commençant par 'KMRE_' vers /var/log/kmre.log ; rotation hebdomadaire, 4 rotations, maxsize 20M, copytruncate | CONSTATÉ |
| cibles construites | `gitee:CMakeLists.txt, gitee:src/CMakeLists.txt, gitee:src/fuse/fuse2/CMakeLists.txt, gitee:src/fuse/fuse3/CMakeLists.txt` | Exécutables kylin-kmre-daemon, kylin-kmre-fuse (libfuse2) et kylin-kmre-fuse3 installés dans bin ; bibliothèque statique fuse-common ; compilateur forcé g++-9. Aucun PERMISSIONS/SETUID dans les CMakeLists | CONSTATÉ |
| appels de programmes privilégiés (indices grep) | `gitee:src/container/container_manager.cpp, gitee:src/utils/load_module_task.cpp, gitee:src/kmre-daemon.cpp, gitee:src/kmre-server.cpp` | Constantes "/usr/bin/docker" et "/sbin/modprobe" ; noms de modules "kmre-binder" et "kmre-ashmem" ; vérification et démarrage de "docker.socket". Logique non lue (Phase 1) | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; control : Depends libssl3t64, libfuse2, libfuse3-3, libdbus-c++-1-0v5, libjsoncpp25, crudini, rsync ; Conflicts/Replaces kydroid-* ; rules 'dh $@' seul | CONSTATÉ |

- [CONSTATÉ] Le code Gitee (3.0.0.0-0k0.1, avril 2024) ne correspond pas à la version publiée dans l'archive (3.0.0.0-0k0.14). L'équivalence code/binaire n'est pas établie.
- [CONSTATÉ] L'archive control déclare libssl3t64 (>= 3.0.0) : usage d'OpenSSL dans le binaire publié.
- [CONSTATÉ] La grep de 'polkit', 'PolicyKit1' et 'pkexec' sur *.cpp, *.h et *.sh ne trouve aucune occurrence dans kylin-kmre-daemon. Le contrôle d'accès éventuel des appelants reste à examiner en Phase 1.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-kmre-manager`

- **Nature** : propre-kylin — **Amont** : KylinSoft (en-tête .pro 'KylinSoft Co., Ltd. 2016-2024', GPLv3 ; mainteneur xiehuijun@kylinos.cn)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k0.19) + gitee code (master, commit b5bc591, changelog 3.0.0.1). La version Gitee ne correspond pas à l'archive.
- **Langages / taille** : C++ (323 Ko), en-têtes (135 Ko), qmake, XML, Shell — archive : 19 fichiers / 26 749 o ; gitee : 153 fichiers / 517 831 o, dont 119 fichiers de code pour 15 636 lignes — lignes de code propres mesurées : 15 644
- **Privilège** : Utilisateur de session : services D-Bus de session. Socket UNIX en mode 0777 sous /var/lib/kmre/kmre-<uid>-<user>/sockets/.
- **Déclencheurs** : activation D-Bus de session (5 noms de bus), postinst configure
- **Indices réseau** : gitee:manager/camera/camera_service.cpp : serveur socket UNIX kmre_camera, chmod 0777, listen(…,1); pas de socket INET relevée par l'extraction
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Code propre Kylin d'environ 15,6 k lignes qui relaie vers le conteneur Android des périphériques et données de l'utilisateur (audio/enregistrement, caméra, GPS, capteurs, surveillance de fichiers) et expose une socket 0777 : critère collecte de données.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (session) | `gitee:data/cn.kylinos.Kmre.Manager.service` | Name=cn.kylinos.Kmre.Manager ; Exec=/usr/bin/kylin-kmre-manager ; SystemdService commenté ; installé dans /usr/share/dbus-1/services/ (manager/manager.pro) | CONSTATÉ |
| dbus-service (session) | `gitee:data/cn.kylinos.Kmre.Audio.service` | Name=cn.kylinos.Kmre.Audio ; Exec=/usr/bin/kylin-kmre-audio ; bus session (audio/audio.pro) | CONSTATÉ |
| dbus-service (session) | `gitee:data/cn.kylinos.Kmre.FileWatcher.service` | Name=cn.kylinos.Kmre.FileWatcher ; Exec=/usr/bin/kylin-kmre-filewatcher ; bus session (filewatcher/filewatcher.pro) | CONSTATÉ |
| dbus-service (session) | `gitee:data/com.kylin.Kmre.gps.service` | Name=com.kylin.Kmre.gpsserver ; Exec=/usr/bin/kylin-kmre-gps-server ; bus session (gps/gps.pro) | CONSTATÉ |
| dbus-service (session) | `gitee:data/com.kylin.Kmre.sensor.service` | Name=com.kylin.Kmre.sensor ; Exec=/usr/bin/kylin-kmre-sensor ; bus session (sensors/sensors.pro) | CONSTATÉ |
| maintainer-script postinst | `gitee:debian/kylin-kmre-manager.postinst` | Se ré-exécute sous bash ; systemctl daemon-reload ; configure : rm -f /etc/xdg/autostart/kylin-kmre-filewatcher.desktop | CONSTATÉ |
| autostart .desktop (non installé) | `gitee:data/kylin-kmre-filewatcher.desktop` | Exec=kylin-kmre-filewatcher ; NoDisplay=true ; X-GNOME-Autostart-Phase=Applications. Aucune règle d'installation trouvée par grep dans les .pro (le postinst supprime l'ancienne copie de /etc/xdg/autostart) | CONSTATÉ |
| script de session (non installé) | `gitee:data/99kmre-systemd-user` | systemctl --user import-environment DISPLAY XAUTHORITY ; dbus-update-activation-environment. Aucune règle d'installation trouvée par grep dans les .pro | CONSTATÉ |
| listener socket UNIX | `gitee:manager/camera/camera_service.cpp` | socket(AF_UNIX, SOCK_STREAM) ; chemin /var/lib/kmre/kmre-<uid>-<user>/sockets/kmre_camera ; unlink puis bind ; chmod(S_IRWXU\|S_IRWXG\|S_IRWXO) soit 0777 ; listen(listen_fd,1) (lignes 66-110) | CONSTATÉ |
| cibles construites | `gitee:kylin-kmre-manager.pro (+ */*.pro)` | SUBDIRS manager, audio, filewatcher, sensors, gps → /usr/bin/kylin-kmre-manager, kylin-kmre-audio, kylin-kmre-filewatcher, kylin-kmre-sensor, kylin-kmre-gps-server ; icônes dans /usr/share/kmre/icons/ | CONSTATÉ |
| debian/rules (gitee) | `gitee:debian/rules` | qmake via dh_auto_configure (QMAKE_OPTS DEFINES+=NO_DEBUG_ON_CONSOLE) ; dh_install --fail-missing. Variantes control.2004, control.2204 et control.wayland présentes | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; control : Depends libasound2t64, libspeexdsp1, libqt5dbus5, libdrm2, libxcb-dri3-0, kylin-kmre-display-control, libkylin-kmre-emugl, binfmt-support ; Description 'backend manager, audio server, input control, filewatcher' | CONSTATÉ |

- [CONSTATÉ] Gitee 3.0.0.1 (mai 2024) contre archive 3.0.0.0-0k0.19 : l'équivalence code/binaire n'est pas établie.
- [PROBABLE] audio/recordingworker.cpp, filewatcher/file-watcher.cpp et gps/ indiquent capture micro, surveillance de fichiers et localisation. Le périmètre exact des données transmises au conteneur reste à établir en Phase 1.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-kmre-modules-dkms`

- **Nature** : fork-amont — **Amont** : Noyau Linux : pilotes Android binder (drivers/android/binder.c, binder_alloc.c), ashmem (drivers/staging/android/ashmem.c) et virt_wifi (drivers/net/wireless/virt_wifi.c). Le Readme déclare 'kylin-kmre-modules-dkms is a fork of linux'. Version noyau amont non indiquée ; changelog Gitee : 'Support kernel version 6.8'.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 1.0.14, yangtze 2022) + gitee code (seule la branche kmre-kernel-modules-ubuntu-22.04 est acquise, commit 79750dd, changelog 3.0.0.0-0k0.2 ; les branches ubuntu-20.04 et ubuntu-24.04 ne sont pas présentes localement)
- **Langages / taille** : C (285 Ko), en-têtes (42 Ko), Makefile Kbuild — archive : 19 fichiers / 25 930 o ; gitee : 31 fichiers / 352 736 o, dont 9 fichiers de code pour 11 448 lignes (binder.c 196 924 o) — lignes de code propres mesurées : 11 448
- **Privilège** : Noyau (ring 0) : modules compilés et installés en root à l'installation, chargés par modprobe.
- **Déclencheurs** : postinst (dkms install pour chaque noyau 5.x/6.x présent), AUTOINSTALL dkms à chaque nouveau noyau, modprobe par kylin-kmre-daemon (PROBABLE)
- **Indices réseau** : module virt_wifi (interface Wi-Fi virtuelle au-dessus d'une interface Ethernet existante) ; aucun listen en espace utilisateur
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Code noyau (hook noyau) de 11,4 k lignes ; audit par diff contre la version Linux amont dont il dérive (à identifier), puis lecture intégrale du delta.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dkms (paquet) | `gitee:debian/dkms` | PACKAGE_NAME=kylin-kmre-modules-dkms 1.0 ; MAKE ashmem, binder et virtwifi ; modules kmre-binder, kmre-ashmem, kmre-virtwifi → /updates/dkms ; AUTOINSTALL=yes ; REMAKE_INITRD=false | CONSTATÉ |
| dkms.conf | `gitee:binder/dkms.conf ; gitee:ashmem/dkms.conf ; gitee:virtwifi/dkms.conf` | Un module par répertoire (kmre-binder, kmre-ashmem, kmre-virtwifi), AUTOINSTALL=yes, DEST_MODULE_LOCATION=/updates/dkms | CONSTATÉ |
| Kbuild | `gitee:binder/Makefile ; gitee:ashmem/Makefile ; gitee:virtwifi/Makefile` | obj-m kmre-binder.o (binder.o, binder_alloc.o, -DCONFIG_ANDROID -DCONFIG_ANDROID_BINDER_IPC), kmre-ashmem.o, kmre-virtwifi.o ; -Wno-int-conversion et -Wno-error=implicit-int ; thunk-extern si CONFIG_RETHUNK | CONSTATÉ |
| maintainer-script postinst | `gitee:debian/postinst` | Pour chaque /lib/modules/5.* et 6.* qui a un build/ : dkms install kylin-kmre-modules-dkms/1.0 -k <kv> (erreurs ignorées) | CONSTATÉ |
| maintainer-script prerm | `gitee:debian/prerm` | dkms remove kylin-kmre-modules-dkms/1.0 --all | CONSTATÉ |
| debian/install | `gitee:debian/install` | ashmem, binder et virtwifi copiés dans usr/src/kylin-kmre-modules-dkms-1.0 ; rules : dh --with dkms, toutes les étapes auto_* neutralisées | CONSTATÉ |
| chargement des modules | `gitee (kylin-kmre-daemon):src/utils/load_module_task.cpp` | Le daemon root référence /sbin/modprobe et les noms kmre-binder et kmre-ashmem (indice grep, paquet voisin) | PROBABLE |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; Architecture: all ; Depends dkms (>= 2.1.0.0) ; Conflicts binder-dkms, kydroid-modules-dkms ; Maintainer root <root@unknown> | CONSTATÉ |
| kernel-module [ajout critique] | `gitee:kylin-kmre-modules-dkms/debian/dkms` | Configuration DKMS effective (dh $@ --with dkms), absente de la fiche, qui ne liste que les dkms.conf des sous-repertoires. PACKAGE_NAME=kylin-kmre-modules-dkms ; modules kmre-binder, kmre-ashmem et kmre-virtwifi installes dans /updates/dkms ; AUTOINSTALL="yes". | CONSTATÉ |

- [CONSTATÉ] Le README du dépôt kmre indique que ce composant n'est requis que sous Ubuntu. L'archive openKylin publie toutefois une version 1.0.14 (2022) absente du changelog Gitee (3.0.0.0-0k0.1 et 0k0.2 seulement).
- [CONSTATÉ] La version amont exacte du noyau dont proviennent binder, ashmem et virt_wifi n'est pas indiquée (pas de debian/watch ni de debian/upstream).
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-kmre-window`

- **Nature** : propre-kylin — **Amont** : KylinSoft (en-tête .pro 'KylinSoft Co., Ltd. 2016-2024', GPLv3 ; mainteneur xiehuijun@kylinos.cn). Tiers intégré : qtsingleapplication (3 copies) et code protobuf généré (KmreCore.pb.*).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k1.12) + gitee code (master, commit 53f2f85, changelog 3.0.0.0-0k0.2). La version Gitee ne correspond pas à l'archive.
- **Langages / taille** : C++ (2,7 Mo dont ~32 k lignes protobuf générées), en-têtes (1,1 Mo), TypeScript (.ts de traduction Qt, 293 Ko), C, qmake, Shell ; ~8 Mo d'images (guides utilisateur) — archive : 19 fichiers / 27 178 o ; gitee : 835 fichiers / 12 268 766 o, dont 523 fichiers de code pour 110 745 lignes (window 67,7 k ; settings-ui 14,5 k ; startapp 3,6 k ; gps 3,3 k ; sensor 1,9 k ; settings-daemon 0,9 k) — lignes de code propres mesurées : 111 143
- **Privilège** : Mixte : kylin-kmre-settings-daemon tourne en root, activé sur le bus système et appelable par tous (policy default). Les autres binaires tournent en session utilisateur. Élévation via pkexec pour rm_kmre.sh.
- **Déclencheurs** : ouverture de session (autostart startapp, délai 8 s), activation D-Bus système cn.kylinos.Kmre.Pref par tout client, activation D-Bus de session cn.kylinos.Kmre.Window, action utilisateur de désinstallation (pkexec)
- **Indices réseau** : QLocalServer (qtsingleapplication) dans sensor/, settings-ui/, gps/; dépendances libqt5network5 et libqt5webenginewidgets5 (archive control) ; pas de serveur INET relevé
- **Blobs** : aucun binaire exécutable ; ~8 Mo d'images PNG/GIF (guides data/v10, data/v10pro ; settings-ui/res)
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant volumineux (110 k lignes) : L3 sur settings-daemon (root, bus système), sur le chemin pkexec et rm_kmre.sh et sur startapp ; L2 sur window et le protocole protobuf ; triage du reste de l'interface.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (système, root) | `gitee:settings-daemon/cn.kylinos.Kmre.Pref.service` | Name=cn.kylinos.Kmre.Pref ; Exec=/usr/bin/kylin-kmre-settings-daemon ; User=root ; pas de SystemdService (activation directe par dbus-daemon-launch-helper) ; installé dans /usr/share/dbus-1/system-services/ | CONSTATÉ |
| dbus-policy (système) | `gitee:settings-daemon/cn.kylinos.Kmre.Pref.conf` | policy user=root : own, send_destination et receive_sender cn.kylinos.Kmre.Pref ; policy context=default : allow send_destination=cn.kylinos.Kmre.Pref (tous les utilisateurs) ; installé dans /etc/dbus-1/system.d/ | CONSTATÉ |
| dbus-service (session) | `gitee:data/cn.kylinos.Kmre.Window.service` | Name=cn.kylinos.Kmre.Window ; Exec=/usr/bin/kylin-kmre-window ; installé dans /usr/share/dbus-1/services/ | CONSTATÉ |
| autostart .desktop | `gitee:startapp/startapp.desktop` | Exec=/usr/bin/startapp start_kmre_silently ; X-GNOME-Autostart-Delay=8 ; NoDisplay=true ; installé dans /etc/xdg/autostart/ (startapp.pro, inst1) | CONSTATÉ |
| élévation pkexec | `gitee:settings-ui/advancedwidget.cpp (l.183-190)` | command << "/bin/bash" << "/usr/share/kmre/rm_kmre.sh" ; mUninstallProcess->start("/bin/pkexec", command). Aucun fichier .policy polkit dans le dépôt. Script data/rm_kmre.sh installé dans /usr/share/kmre/ (settings-ui.pro) ; son contenu n'a pas été lu | CONSTATÉ |
| listener QLocalServer | `gitee:settings-ui/qtsingleapplication/qtlocalpeer.cpp (et copies sensor/, gps/)` | QLocalServer de qtsingleapplication (instance unique) ; fichier de verrou dans QDir::tempPath() | CONSTATÉ |
| fichier de configuration | `gitee:gps/data/gps.conf` | Contenu : une seule chaîne de type base64 ('MjU1OGVjZGRmZDE2') ; installé dans /usr/share/kmre/ (gps.pro) ; schéma org.kylin.gps.gschema.xml installé dans glib-2.0/schemas | CONSTATÉ |
| cibles construites | `gitee:kylin-kmre-window.pro (+ */*.pro)` | SUBDIRS window, startapp, settings-daemon, settings-ui, sensor, gps → /usr/bin/kylin-kmre-window, startapp, kylin-kmre-settings-daemon, kylin-kmre-settings, kylin-kmre-sensorwindow, kylin-kmre-gps ; données games.json, warning.json, feature_config.ini et app.json dans /usr/share/kmre/ | CONSTATÉ |
| debian/rules (gitee) | `gitee:debian/rules` | qmake ; dh_install --fail-missing ; pas de scripts mainteneur dans debian/ Gitee | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; control : Depends libqt5webenginewidgets5, libqt5network5, libqapt3, libsystemd0, libprotobuf32t64, libavcodec60, libsdl2, binfmt-support ; Conflicts/Replaces kydroid-update-installer, kylin-kydroid-updates, startapp | CONSTATÉ |

- [CONSTATÉ] Gitee 3.0.0.0-0k0.2 (juillet 2024) contre archive 3.0.0.0-0k1.12 : l'équivalence code/binaire n'est pas établie.
- [PROBABLE] La chaîne de gps/data/gps.conf ressemble à une clé ou un identifiant encodé en base64, installé en clair. Sa nature reste à confirmer.
- [CONSTATÉ] window.pro référence ../data/dynamic_size_app_list.xml, absent de data/ dans le dépôt Gitee.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-kmre-display-control`

- **Nature** : fork-amont — **Amont** : Le Readme déclare 'a fork of qemu'. Le code dérive de l'emugl de l'émulateur Android/AOSP (shared/emugl/common, scripts/gen-emugl-entries.py ; debian/copyright cite 'The Android Open Source Project' 2007-2015) avec des ajouts KylinSoft. Version amont non indiquée.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k0.4) + gitee code (master, commit 0ed6544, changelog 3.0.0.0-0k0.1)
- **Langages / taille** : C++ (101 Ko), en-têtes (96 Ko), Python (générateur, 17,7 Ko), C (15,7 Ko), Shell — archive : 19 fichiers / 26 883 o ; gitee : 100 fichiers / 262 589 o, dont 74 fichiers de code pour 7 778 lignes — lignes de code propres mesurées : 7 778
- **Privilège** : Bibliothèque chargée par les processus de session (kylin-kmre-manager, kylin-kmre-window) ; socket UNIX 0777 dans /var/run/user/<uid>/kmre.
- **Déclencheurs** : chargement par kylin-kmre-manager et kylin-kmre-window (dépendance déclarée)
- **Indices réseau** : gitee:host/libs/libDisplayControl/kmre_socket.c : serveur socket UNIX, chmod 0777
- **Blobs** : aucun
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Bibliothèque de session de 7,8 k lignes exposant une socket IPC vers le conteneur Android (décodeur de flux) ; prioriser libDisplayControl, kmre_socket.c et displayControl_dec.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| listener socket UNIX | `gitee:host/libs/libDisplayControl/kmre_socket.c` | server_listen() : socket(AF_UNIX, SOCK_STREAM) ; chemin construit sous /var/run/user/<uid>/kmre (name_unix_path) ; unlink si présent ; bind ; chmod(S_IRWXU\|S_IRWXG\|S_IRWXO) soit 0777 ; listen(fd, num_conn) (lignes 25-80) | CONSTATÉ |
| cibles construites | `gitee:CMakeLists.txt (+ host/, shared/, system/)` | Bibliothèque partagée DisplayControl installée dans CMAKE_INSTALL_LIBDIR ; statiques displayControl_dec et emugl_common ; exécutable display_control (non installé : pas de install()) | CONSTATÉ |
| debian install | `gitee:debian/kylin-kmre-display-control.install` | En-têtes display_control_api.h, DisplayControlDecoder.h et display_control_api_functions.h vers /usr/include/DisplayControl/ | CONSTATÉ |
| debian/rules (gitee) | `gitee:debian/rules` | dh + cmake ; -DCMAKE_INSTALL_LIBDIR=/usr/lib/$(DEB_HOST_MULTIARCH) ; pas de scripts mainteneur | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; Depends libc6, libgcc-s1, libstdc++6 ; Provides/Replaces display-control-host | CONSTATÉ |

- [CONSTATÉ] Gitee 3.0.0.0-0k0.1 contre archive 3.0.0.0-0k0.4 : l'équivalence code/binaire n'est pas établie.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-kmre-emugl`

- **Nature** : fork-amont — **Amont** : Émulateur Android (AOSP platform/external/qemu, sous-arbres android-emu et android-emugl), ANGLE (Google) et astc-codec (Google). Le Readme déclare 'a fork of qemu'. Version amont non indiquée (android-emu/android/version.h présent, non lu).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k0.11) + gitee code (master, commit 8c9e1c9, changelog 3.0.0.0-0k0.4)
- **Langages / taille** : C++ (19 Mo), en-têtes (10 Mo), C (1,6 Mo), Qt UI, Protobuf, Python, JSON — archive : 19 fichiers / 26 682 o ; gitee : 3 599 fichiers / 42 023 630 o (android-emu : 829 fichiers C/C++, dont 105 référencés par android-emu-base ; android-emugl : 388 ; astc-codec : 30) — lignes de code propres mesurées : 782 667
- **Privilège** : Bibliothèques (libOpenglRender, libshadertranslator) chargées en session utilisateur par kylin-kmre-manager et kylin-kmre-window ; décodent les flux GLES venant du conteneur Android.
- **Déclencheurs** : chargement par les processus de session KMRE
- **Indices réseau** : 9 indices relevés (WifiForwardServer, QWebSocketServer, OffworldPipe, utils/sockets.c listen) ; seul utils/sockets.c est référencé par le build (PROBABLE : les autres ne sont pas compilés)
- **Blobs** : 1 fichier de test de 14 o (android-emu/android/emulation/CpuAccelerator_unittest.dat) ; modèles 3D et images (model.obj, *.png, *.jpg) non exécutables
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Fork volumineux (42 Mo, 3 599 fichiers) ; limiter l'audit aux sources effectivement compilées (décodeurs GLESv1/v2, renderControl_dec, libOpenglRender, android-emu-base) et au delta Kylin, faute de version amont identifiée.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cibles construites | `gitee:CMakeLists.txt ; gitee:android-emugl/CMakeLists.txt` | Seul add_subdirectory(android-emugl) au niveau racine ; android-emugl inclut host, shared, android-emu-base, astc-codec et angle ; exécutables renderer et emugen construits mais non installés | CONSTATÉ |
| bibliothèques installées | `gitee:android-emugl/host/libs/libOpenglRender/CMakeLists.txt (l.57) ; gitee:android-emugl/angle/CMakeLists.txt (l.233)` | Seules 2 cibles install() : OpenglRender et shadertranslator (LIBRARY → CMAKE_INSTALL_LIBDIR), si NEED_INSTALL | CONSTATÉ |
| indices listen | `gitee:android-emu/android/network/WifiForwardServer.cpp, skin/qt/websockets/*, offworld/OffworldPipe.cpp, utils/sockets.c` | Seul utils/sockets.c est référencé par android-emugl/android-emu-base/CMakeLists.txt (l.93) ; aucune référence CMake aux autres fichiers trouvée par grep | CONSTATÉ |
| debian/rules (gitee) | `gitee:debian/rules` | dh + cmake ; -DCMAKE_INSTALL_LIBDIR=/usr/lib/$(DEB_HOST_MULTIARCH) ; pas de scripts mainteneur | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; paquet binaire libkylin-kmre-emugl ; Depends libdrm2, libpciaccess0, libxcb-dri3-0, libegl1-mesa, libgles2-mesa | CONSTATÉ |

- [CONSTATÉ] Gitee 3.0.0.0-0k0.4 contre archive 3.0.0.0-0k0.11 : l'équivalence code/binaire n'est pas établie.
- [PROBABLE] La majorité de l'arbre android-emu (829 fichiers C/C++) n'est pas référencée par le build CMake (105 entrées dans android-emu-base).
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `libkylin-kmre`

- **Nature** : propre-kylin — **Amont** : KylinSoft (Makefile '(c) lixiang 2019-2021 <lixiang@kylinos.cn>' ; mainteneur xiehuijun@kylinos.cn)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive squelette vide (19 fichiers dh_make, version 3.0.0.0-0k0.5) + gitee code (master, commit f34fa85, changelog 3.0.0.1)
- **Langages / taille** : C++ (40,7 Ko), Protobuf (KmreCore.proto), en-têtes — archive : 19 fichiers / 26 302 o ; gitee : 14 fichiers / 63 522 o, dont 3 fichiers de code pour 1 378 lignes — lignes de code propres mesurées : 1 709
- **Privilège** : Bibliothèque cliente chargée par les programmes appelants (session utilisateur) ; pas de service propre.
- **Déclencheurs** : chargement dynamique par d'autres programmes (-ldl dans le Makefile)
- **Indices réseau** : aucun listen relevé ; client socket (kmre_socket.cc) vers le conteneur, d'après les noms de fichiers
- **Blobs** : aucun
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Bibliothèque IPC de session de 3 fichiers (1,4 k lignes) : une lecture intégrale coûte moins d'une heure.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cible construite | `gitee:Makefile` | protoc KmreCore.proto ; g++ -fPIC -shared main.cc kmre_socket.cc KmreCore.pb.cc -fpermissive -g -o libkmre.so -ldl | CONSTATÉ |
| debian install | `gitee:debian/libkylin-kmre.install` | libkmre.so → usr/lib/ ; rules 'dh $@ --parallel', NO_PKG_MANGLE=1 ; pas de scripts mainteneur | CONSTATÉ |
| debian (archive) | `archive:debian/control, archive:debian/rules` | Squelette dh_make ; Description 'kmre socket library API for other programs' ; Depends libprotobuf32t64 ; Replaces libkydroidrequest | CONSTATÉ |

- [CONSTATÉ] Gitee 3.0.0.1 contre archive 3.0.0.0-0k0.5 : l'équivalence code/binaire n'est pas établie.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `peony-vfs-kylin-kmre`

- **Nature** : absent-ou-vide — **Amont** : Plugin VFS du gestionnaire de fichiers Peony (UKUI) pour KMRE. Homepage déclarée : https://github.com/ukui/peony-vfs-kylin-kmre ; Original-Maintainer 'Kylin Team <team+kylin@tracker.debian.org>'.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive squelette dh_make vide (19 fichiers) ; chemin_gitee null ; forges Gitee et GitHub EMPTY
- **Langages / taille** : aucun code — archive : 19 fichiers / 27 006 o — lignes de code propres mesurées : 0
- **Privilège** : Plugin chargé dans le processus Peony de l'utilisateur (PROBABLE, d'après la description).
- **Déclencheurs** : chargement par le gestionnaire de fichiers Peony (PROBABLE)
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible (archive squelette, forges vides) ; seule une analyse du binaire publié serait possible.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquet binaire peony-vfs-kylin-kmre (amd64, arm64) ; Depends libglib2.0-0t64, libqt5core5a, libqt5dbus5, libqt5gui5 ; Conflicts/Replaces peony-vfs-kydroid ; description de type 'metapackage' recopiée | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make ('dh $@' seul) | CONSTATÉ |
| contenu du .deb binaire publié | `(non présent)` | Plugin .so Peony et éventuels scripts mainteneur non examinés | NON COUVERT |

- [CONSTATÉ] Le dépôt GitHub ukui/peony-vfs-kylin-kmre est déclaré en Homepage mais est relevé EMPTY par l'inventaire des forges.

### `kylin-kmre-image-update-x64`

- **Nature** : absent-ou-vide — **Amont** : KylinSoft (Original-Maintainer jianglinxuan <jianglinxuan@kylinos.cn>). Fichiers de mise à jour de l'image Android KMRE ; Homepage factice https://launchpad.dev/.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive squelette dh_make vide (19 fichiers, malgré meta.source_coquille_vide=0) ; chemin_gitee null ; forges ABSENT
- **Langages / taille** : aucun code — archive : 19 fichiers / 26 243 o — lignes de code propres mesurées : 0
- **Privilège** : Inconnu. Les fichiers d'image sont probablement consommés par kylin-kmre-daemon (root), via la méthode D-Bus ComponentsUpgrade ou LoadImage.
- **Déclencheurs** : installation ou mise à jour du paquet ; consommation probable par kylin-kmre-daemon (PROBABLE)
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun dans l'arbre ; le binaire publié contient vraisemblablement des images système précompilées (PROBABLE)
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucune source : paquet de données binaires (image Android) construit hors archive.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquet binaire amd64, Section: kernel ; Depends kylin-kmre-image-data-x64 (>= 3.0-250506.10) ; Provides kylin-kmre-image-update ; Description 'Kmre image update files' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make ('dh $@' seul) | CONSTATÉ |
| contenu du .deb binaire publié | `(non présent)` | Fichiers de mise à jour d'image et scripts mainteneur éventuels non examinés | NON COUVERT |

- [CONSTATÉ] Version 3.0-250506.10+250506.11 ; le binaire publié n'est pas reproductible à partir du source de l'archive.

### `kare`

- **Nature** : absent-ou-vide — **Amont** : KylinSoft (Original-Maintainer zhanglin <zhanglin@kylinos.cn>). Description : 'Solution for kylinos to run apps in compatible mode'.
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive squelette dh_make (19 fichiers, version 3.0.0.0-0k4.50, nile.bedrock, 04/06/2026) et gitee squelette dh_make (19 fichiers, branche openkylin/nile, commit b51083f 'Import Upstream version 2.4.0.0', changelog 2.4.0.0-0k0.13). Les branches upstream, pristine-tar et packaging/openkylin/nile ne sont pas présentes localement.
- **Langages / taille** : aucun code — archive : 19 fichiers / 25 281 o ; gitee : 19 fichiers / 25 724 o — lignes de code propres mesurées : 0
- **Privilège** : Inconnu. Les dépendances Gitee (docker.io, bubblewrap, debootstrap) font envisager des opérations de conteneur ou de chroot nécessitant root (PROBABLE).
- **Déclencheurs** : installé par défaut par l'ISO officielle 3.0 (kare=3.0.0.0-0k4.50), autres déclencheurs inconnus (pas de code)
- **Indices réseau** : dépendance curl dans gitee:debian/control (PROBABLE : téléchargements réseau)
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Aucun code source (squelettes des deux côtés) alors que le paquet est installé par défaut : seule une analyse du .deb binaire est possible ; à prioriser en acquisition.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control (archive) | `archive:debian/control` | Architecture: any ; Depends ${shlibs:Depends}, ${misc:Depends} ; Recommends flatpak ; Maintainer 'Ubuntu Developers' | CONSTATÉ |
| debian-control (gitee) | `gitee:debian/control` | Deux stanzas kare (arm64, amd64) ; Depends docker.io \| docker-ce, bubblewrap, debootstrap, libqt5dbus5, curl, python3-pydbus ; Recommends flatpak | CONSTATÉ |
| debian-rules | `archive:debian/rules ; gitee:debian/rules` | Squelette dh_make ('dh $@' seul) des deux côtés | CONSTATÉ |
| maintainer-script (modèles inactifs) | `archive:debian/postinst.ex, preinst.ex, prerm.ex, postrm.ex, kare.cron.d.ex` | Modèles dh_make non actifs | CONSTATÉ |
| contenu du .deb binaire installé par l'ISO | `(binaire kare=3.0.0.0-0k4.50 non présent dans le corpus)` | Binaires, services et scripts mainteneur réellement installés non examinés | NON COUVERT |

- [CONSTATÉ] Seul paquet du lot installé par défaut par l'ISO 3.0, et sans aucun code source disponible.
- [CONSTATÉ] Le control Gitee (2.4.0.0) déclare des dépendances (docker.io\|docker-ce, bubblewrap, debootstrap, curl, python3-pydbus) absentes du control archive (3.0.0.0-0k4.50, générique ${shlibs:Depends}).

### `kylin-kmre-image-data-x64`

- **Nature** : absent-ou-vide — **Amont** : KylinSoft (Original-Maintainer jianglinxuan <jianglinxuan@kylinos.cn>). D'après gitee:kmre/README.md, ce paquet contient l'image conteneur Android kmre-container-image.tar (system.sfs AOSP construit via platform_manifest kmre-opensource-devel) et data/kmre.conf.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun dans le corpus : archive squelette dh_make vide (19 fichiers) ; chemin_gitee null alors que les forges indiquent Gitee et GitHub PRESENT (IDENTICAL-HEADS, master), dépôt non acquis
- **Langages / taille** : aucun code — archive : 19 fichiers / 26 216 o — lignes de code propres mesurées : 0
- **Privilège** : Image chargée dans docker (root) par kylin-kmre-daemon ou kylin-kmre-load-image ; le conteneur Android s'exécute sous docker.
- **Déclencheurs** : installation du paquet ; chargement docker par le daemon KMRE
- **Indices réseau** : aucun dans l'arbre
- **Blobs** : aucun dans l'arbre ; le binaire publié contient une image conteneur Android précompilée (CONSTATÉ via gitee:kmre/README.md)
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucune source dans le corpus. Le contenu est une image Android binaire. L'audit relève de l'acquisition du dépôt Gitee et de l'analyse de l'image, pas de la lecture de code.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquet binaire amd64, Section: kernel ; Conflicts/Replaces kydroid-image-data(-x64) ; Provides kylin-kmre-image-data ; Description 'kmre image data for x86_64' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make ('dh $@' seul) | CONSTATÉ |
| chargement de l'image | `gitee (kylin-kmre-daemon):data/kylin-kmre-load-image.service ; src/kylin-kmre-load-image` | ConditionPathExists=/usr/share/kmre/kmre-container-image.tar ; le script exécute /usr/bin/docker load -i /usr/share/kmre/kmre-container-image.tar (grep) | CONSTATÉ |
| contenu du .deb binaire publié | `(non présent ; dépôt Gitee non acquis)` | Image conteneur, kmre.conf et scripts mainteneur non examinés | NON COUVERT |

- [CONSTATÉ] Les forges déclarent le dépôt Gitee PRESENT mais il n'a pas été acquis (chemin_gitee null) : écart d'acquisition à combler.
- [CONSTATÉ] Le README kmre indique que l'image Android est construite depuis AOSP (Android 11, BUILD_ID RP1A.201005.006) avec des modifications Kylin non incluses dans le corpus.

### `kare-v10sp1-base-x86-64`

- **Nature** : absent-ou-vide — **Amont** : KylinSoft (Original-Maintainer zhanglin <zhanglin@kylinos.cn>). Même description que kare ; le nom évoque une base système Kylin V10 SP1 x86-64 pour kare.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive squelette dh_make vide (19 fichiers, version 3.0.0.0-0k0.29, nile.bedrock, 04/02/2026) ; chemin_gitee null ; forges ABSENT
- **Langages / taille** : aucun code — archive : 19 fichiers / 25 992 o — lignes de code propres mesurées : 0
- **Privilège** : Inconnu (pas de code). Probablement un environnement de base consommé par kare (PROBABLE).
- **Déclencheurs** : installation du paquet (non installé par défaut par l'ISO)
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun dans l'arbre ; le binaire publié contient vraisemblablement une base système précompilée (PROBABLE, d'après le nom)
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucune source ni forge : non auditable par lecture de code.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Architecture: any ; Depends ${shlibs:Depends}, ${misc:Depends} ; Recommends flatpak ; Description 'Solution for kylinos to run apps in compatible mode.' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make ('dh $@' seul) | CONSTATÉ |
| contenu du .deb binaire publié | `(non présent)` | Contenu (base système ou runtime) et scripts mainteneur non examinés | NON COUVERT |

- [CONSTATÉ] Le paquet est publié en 2026 sous la distribution 'nile.bedrock', comme kare.

### `android-platform-system-core`

- **Nature** : fork-amont — **Amont** : AOSP platform/system/core, tag android-8.1.0_r23 (version 1:8.1.0+r23-ok2 ; debian/watch vers android.googlesource.com/platform/system/core ; Homepage idem). Empaquetage dérivé du paquet Debian/Ubuntu 'android-platform-system-core' (README.source signé Hans-Christoph Steiner et Kai-Chung Yan ; 13 patches de type Debian).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète AOSP + debian/ : 1 446 fichiers) ; pas de chemin Gitee dans le corpus (Vcs-Git déclaré : gitee.com/openkylin/android-platform-system-core.git ; forges PRESENT)
- **Langages / taille** : C++ (5,2 Mo), en-têtes (1,6 Mo), C (945 Ko), assembleur (122 Ko), Python (93 Ko), Shell — archive : 1 446 fichiers / 16 011 007 o (dont 7,1 Mo de blobs de test) ; 17 makefiles debian/*.mk qui référencent environ 160 sources compilées — lignes de code propres mesurées : 250 465
- **Privilège** : Outils utilisateur (adb, fastboot, mkbootimg, utilitaires sparse) sans service ni setuid déclaré ; adb démarre un serveur sur l'interface loopback.
- **Déclencheurs** : exécution manuelle par l'utilisateur (non installé par défaut par l'ISO)
- **Indices réseau** : adb (hôte) : bind loopback + listen(s,4) dans adb/sysdeps/posix/network.cpp (compilé); 16 indices au total ; ceux de init/, lmkd/, adbd_auth, jdwp_service et libnetutils relèvent de code Android cible, non référencé par debian/*.mk (PROBABLE : non compilé)
- **Blobs** : 14 blobs de test (7,1 Mo) : ELF ARM/x86/x86_64/aarch64 sous libbacktrace/testdata/ (dont libart.so de 5,5 Mo) et zip/apk sous libziparchive/testdata/ ; aucun n'est référencé par les debian/*.install
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Fork AOSP (niveau B) empaqueté à la manière de Debian : revue de la liste des 13 patches, des debian/*.mk et comparaison avec le paquet Debian/Ubuntu 1:8.1.0+r23-x ; pas de service privilégié.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | Build via debian/*.mk : adb, libadb, libbacktrace, libbase, libcutils, liblog, libsparse, libutils, libziparchive, libcrypto_utils, simg2img, img2simg, append2simg, simg2simg, fastboot, libnativebridge, libnativeloader ; dh --with bash-completion ; dh_shlibdeps -l/usr/lib/<multiarch>/android | CONSTATÉ |
| debian-control | `archive:debian/control` | 31 paquets binaires (adb, fastboot, mkbootimg, android-sdk-libsparse-utils, android-lib*, transitional) ; Maintainer 'Openkylin Developers' ; Uploaders Luo Yaoming ; Build-Depends-Arch android-libboringssl-dev, libusb-1.0-0-dev | CONSTATÉ |
| debian install | `archive:debian/adb.install, adb.links, fastboot.install, fastboot.links, mkbootimg.install, android-sdk-libsparse-utils.install, android-lib*.install (29 fichiers)` | adb et fastboot dans /usr/lib/android-sdk/platform-tools, liens dans /usr/bin ; mkbootimg et simg_dump/simg2img/img2simg/simg2simg/append2simg dans /usr/bin ; bibliothèques dans /usr/lib/<multiarch>/android | CONSTATÉ |
| patches | `archive:debian/patches/series` | 13 patches : move-log-file-to-proper-dir, Added-missing-headers, Direct-include-fs_config-header, libusb-header-path, stdatomic, Nonnull, ucontext, Vector-cast, use-Python-3-for-mkbootimg, major-minor-moved-to-sysmacros, drop-libext4_utils, throw-exception-on-unknown-os, ENODATA-BSD | CONSTATÉ |
| listener réseau (adb server hôte) | `archive:adb/sysdeps/posix/network.cpp (compilé via debian/libadb.mk)` | bind sur loopback_addr4/loopback_addr6 puis listen(s, 4) (lignes 98-113) ; adb_listeners.cpp et transport_local.cpp également compilés dans libadb | CONSTATÉ |
| listener socket (libcutils) | `archive:libcutils/socket_local_server_unix.c, socket_inaddr_any_server_unix.c (compilés via debian/libcutils.mk)` | Fonctions de bibliothèque de création de serveurs socket ; usage par les binaires installés non déterminé | CONSTATÉ |
| maintainer-scripts | `archive:debian/` | Aucun fichier preinst, postinst, prerm, postrm, systemd, udev ni D-Bus dans debian/ | CONSTATÉ |
| autopkgtest | `archive:debian/tests/control` | Test-Command: adb help && ls -l libcutils.so et en-têtes | CONSTATÉ |

- [CONSTATÉ] Le debian/changelog ne contient que 2 entrées openKylin (ok1, ok2, 08/09/2022) ; l'historique Debian amont a été retiré.
- [PROBABLE] Base AOSP 8.1.0_r23 (2018) : des correctifs de sécurité amont postérieurs peuvent être absents, à vérifier en Phase 1 par comparaison de versions.
- [CONSTATÉ] 59 liens symboliques internes (clang-format, en-têtes liblog include_vndk), sans cible hors de l'arbre.

### `liblol`

- **Nature** : absent-ou-vide — **Amont** : AOSC-Dev liblol (Homepage https://github.com/AOSC-Dev/liblol ; Maintainer du binaire Miao Wang <shankerwangmiao@gmail.com>). Couche de compatibilité 'old world' pour loongarch64. Kylin n'est pas l'auteur amont, malgré meta.origine_kylin=1.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive squelette dh_make vide (19 fichiers, version 0.1.5-1, malgré meta.source_coquille_vide=0) ; chemin_gitee null ; forges ABSENT
- **Langages / taille** : aucun code — archive : 19 fichiers / 25 674 o — lignes de code propres mesurées : 0
- **Privilège** : Inconnu (pas de code). Couche de compatibilité ABI chargée par des applications ; la dépendance recommandée liblol-dkms implique un module noyau.
- **Déclencheurs** : installation sur loong64 uniquement
- **Indices réseau** : aucun (pas de code)
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucune source dans le corpus. Le code amont AOSC-Dev/liblol devrait être acquis et comparé au binaire loong64.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquet binaire loong64 seulement, Section otherosfs ; Depends libstdc++6, libc6 (>= 2.36), libc6 (<< 2.39.1~) ; Recommends liblol-dkms ; Conflicts glibc ; Breaks libcrypt1 (>= 1:4.4.36.1~) ; Multi-Arch foreign | CONSTATÉ |
| debian-rules | `archive:debian/rules` | Squelette dh_make ('dh $@' seul) | CONSTATÉ |
| contenu du .deb binaire publié | `(non présent)` | Bibliothèques de compatibilité et scripts mainteneur non examinés ; liblol-dkms (module noyau recommandé) absent du lot | NON COUVERT |

- [CONSTATÉ] La métadonnée origine_kylin=1 contredit le control (Homepage AOSC-Dev, mainteneur externe).

#### Constats transverses du lot B06-android

- **[CONSTATÉ]** Les 14 paquets du lot hors android-platform-system-core n'ont dans l'archive qu'un squelette dh_make de 19 fichiers (debian/ uniquement), avec un debian/rules identique. Cela inclut kmre, kylin-kmre-image-update-x64 et liblol, dont meta.source_coquille_vide vaut pourtant 0. — *preuve* : md5sum archive/{kmre,kylin-kmre-*,libkylin-kmre,peony-vfs-kylin-kmre,kare,kare-v10sp1-base-x86-64,liblol}/debian/rules = 78f72f63bcffd66920908090d06b2d59 ; contenu : '%:\n\tdh $@'
- **[CONSTATÉ]** Les debian/control des squelettes archive déclarent des dépendances de bibliothèques déjà résolues (ex. 'libssl3t64 (>= 3.0.0)', 'libqt5core5a (>= 5.15.1)') et des champs Original-Maintainer, alors que Build-Depends se limite à debhelper-compat. Les binaires publiés ne sont pas reproductibles depuis l'archive. — *preuve* : archive/kylin-kmre-daemon/debian/control : 'Build-Depends: debhelper-compat (= 12)' et 'Depends: libc6 (>= 2.38), libdbus-c++-1-0v5, …, libssl3t64 (>= 3.0.0)'
- **[CONSTATÉ]** Pour les 7 paquets dont le code est sur Gitee, aucune version Gitee ne correspond à la version archive : daemon 0k0.1 contre 0k0.14, manager 3.0.0.1 contre 0k0.19, window 0k0.2 contre 0k1.12, display-control 0k0.1 contre 0k0.4, emugl 0k0.4 contre 0k0.11, libkylin-kmre 3.0.0.1 contre 0k0.5, dkms 3.0.0.0-0k0.2 contre 1.0.14. — *preuve* : head -1 gitee/<pkg>/debian/changelog vs archive/<pkg>/debian/changelog (ex. 'kylin-kmre-daemon (3.0.0.0-0k0.1) v101' vs 'kylin-kmre-daemon (3.0.0.0-0k0.14) nile')
- **[CONSTATÉ]** 2 services D-Bus système du lot s'exécutent en root et autorisent l'envoi de messages par tout utilisateur (policy context=default allow send_destination) : cn.kylinos.Kmre (kylin-kmre-daemon) et cn.kylinos.Kmre.Pref (kylin-kmre-settings-daemon de kylin-kmre-window). — *preuve* : gitee:kylin-kmre-daemon/data/cn.kylinos.Kmre.conf et gitee:kylin-kmre-window/settings-daemon/cn.kylinos.Kmre.Pref.conf : '<policy context="default"><allow send_destination="cn.kylinos.Kmre[.Pref]" />' ; fichiers .service : 'User=root'
- **[CONSTATÉ]** Aucun fichier de politique polkit (.policy, .pkla, .rules polkit) n'existe dans les dépôts Gitee du lot. La seule référence relevée est un appel /bin/pkexec /bin/bash /usr/share/kmre/rm_kmre.sh dans l'interface de réglages. — *preuve* : find gitee/kylin-kmre-* -name '*.policy' → vide ; gitee:kylin-kmre-window/settings-ui/advancedwidget.cpp:190 'mUninstallProcess->start("/bin/pkexec", command)'
- **[CONSTATÉ]** 2 composants créent un serveur socket UNIX puis appliquent chmod 0777 (S_IRWXU\|S_IRWXG\|S_IRWXO) au fichier socket. — *preuve* : gitee:kylin-kmre-manager/manager/camera/camera_service.cpp:106 et gitee:kylin-kmre-display-control/host/libs/libDisplayControl/kmre_socket.c:76 'chmod(…sun_path, S_IRWXU \| S_IRWXG \| S_IRWXO)'
- **[CONSTATÉ]** Seul kare est installé par défaut par l'ISO 3.0, et il n'a de code ni dans l'archive ni sur Gitee (squelettes dh_make des deux côtés). Les composants KMRE (kmre, kylin-kmre-*) ne sont pas installés par défaut. — *preuve* : fiche B06-android.json : installe_par_defaut_iso = ['kare=3.0.0.0-0k4.50'] pour kare et [] pour les 14 autres ; gitee/kare : 19 fichiers debian/ (commit b51083f)
- **[CONSTATÉ]** La chaîne KMRE dépend de docker : le métapaquet kmre dépend de docker.io, et le daemon root référence /usr/bin/docker et docker.socket ainsi qu'un script de chargement d'image docker. — *preuve* : archive:kmre/debian/control 'Depends: … docker.io, haveged, python3' ; gitee:kylin-kmre-daemon/src/kylin-kmre-load-image:36 '/usr/bin/docker load -i /usr/share/kmre/kmre-container-image.tar'
- **[CONSTATÉ]** Le lot installe 3 modules noyau hors arbre (kmre-binder, kmre-ashmem, kmre-virtwifi) via DKMS avec AUTOINSTALL=yes. Le postinst les compile pour chaque noyau 5.x/6.x présent. — *preuve* : gitee:kylin-kmre-modules-dkms/debian/dkms 'AUTOINSTALL="yes"' ; debian/postinst 'dkms install ${NAME}/${VERSION} -k ${kv}'
- **[CONSTATÉ]** Écarts d'acquisition : kylin-kmre-image-data-x64 est indiqué Gitee PRESENT mais n'a pas été acquis (chemin_gitee null). Pour kylin-kmre-modules-dkms, seule 1 des 3 branches Gitee est présente localement. Pour kare, seule la branche openkylin/nile est présente (upstream et pristine-tar absents). — *preuve* : fiche JSON forges.gitee='PRESENT' et chemin_gitee=null ; git -C gitee/kylin-kmre-modules-dkms branch -a → kmre-kernel-modules-ubuntu-22.04 seule ; git -C gitee/kare show-ref → refs/heads/openkylin/nile seule
- **[CONSTATÉ]** Le lot comporte des points d'entrée de démarrage de session ou de système : unité systemd kylin-kmre-daemon (WantedBy=multi-user.target, activée et démarrée par postinst), autostart XDG startapp (délai 8 s) et /etc/profile.d/kmre-env.sh. — *preuve* : gitee:kylin-kmre-daemon/debian/kylin-kmre-daemon.postinst 'systemctl enable kylin-kmre-daemon.service' ; gitee:kylin-kmre-window/startapp/startapp.pro 'inst1.path = /etc/xdg/autostart/' ; gitee:kylin-kmre-daemon/data/CMakeLists.txt 'DESTINATION ${DIR_ETC}/profile.d/'

