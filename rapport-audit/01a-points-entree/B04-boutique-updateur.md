# Lot B04-boutique-updateur

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-software-center`

- **Nature** : propre-kylin — **Amont** : Kylin / Ubuntu Kylin Team (ubuntu-kylin-software-center, Homepage/Vcs https://github.com/ubuntukylin/ubuntu-kylin-software-center d'après gitee:debian/control). Le code gitee est la version 4.5.77.1 (gitee:debian/changelog, Source: ubuntu-kylin-software-center) alors que l'ISO installe 5.0.9.1-ok4.77, dont la source n'est disponible nulle part dans le corpus.
- **Installé par défaut (ISO 3.0)** : oui (4 binaire(s)) — **Code disponible** : archive : squelette dh_make vide (19 fichiers, meta.source_coquille_vide=1). Gitee : code présent (branche master, clone d'1 commit du 2024-03-25, version 4.5.77.1, qui diverge de la 5.0.9.1 livrée). Les branches ubuntukylin/noble et ubuntukylin/resolute n'ont pas été clonées.
- **Langages / taille** : archive : aucun code. gitee : Python (~2,17 Mo, 81 fichiers .py, ~41 000 lignes), Qt UI (.ui), 1 fichier C++ (utils/notify.cpp), shell (lanceur ubuntu-kylin-software-center), setup.py (pybuild). — archive : 19 fichiers / 27 Ko. gitee : 5 111 fichiers / 108,7 Mo, dont data/uksc.db (15 Mo), 140 .pyc et des images. — lignes de code propres mesurées : 41 200
- **Privilège** : D'après gitee : démon D-Bus système en root (com.kylin.softwarecenter, User=root) et client GUI dans la session utilisateur. Pour la version livrée 5.0.9.1 : NON COUVERT.
- **Déclencheurs** : boot (multi-user.target, si l'unité système est activée) [gitee], activation D-Bus système com.kylin.softwarecenter [gitee], ouverture de session (unité user, default.target) [gitee], lancement manuel ou handler .desktop, postinst (installation/mise à jour du paquet) [gitee], plugins livrés synchrodata/expand : déclencheurs NON COUVERT
- **Indices réseau** : archive:debian/control : descriptions des plugins 'regularly visits the server to obtain data updates' et 'access the server to get data' (CONSTATÉ); gitee (grep d'URL) : http://service.ubuntukylin.com:8001/uksc/, http://archive.kylinos.cn/kylin/kydroid/3/x86/, http://archive.ubuntukylin.com:10006/ubuntukylin, http://screenshots.ubuntu.com/ (PROBABLE); gitee:debian/control Build-Depends python3-requests, python3-httplib2, python3-dnspython, python3-certifi, python3-pycryptodome (CONSTATÉ); QLocalServer local, pas réseau (CONSTATÉ)
- **Blobs** : archive : aucun. gitee : 142 blobs (18,7 Mo) : data/uksc.db (SQLite 15 Mo, installé dans /usr/share/kylin-software-center/data/), 140 .pyc (cpython-38 et cpython-312), fonts-liberation2_2.1.5-3_all.deb à la racine. On trouve aussi une clé OpenPGP RSA 4096 bits (backend/aptdaemon/keyrings/ubuntukylin-archive-keyring.gpg) et un fichier vide backend/service/password.txt, installé puis passé en chmod 777.
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — La source de la version livrée (5.0.9.1 et ses 3 plugins) est absente. Le seul code disponible (gitee, 4.5.77.1) n'a pas de correspondance établie avec les binaires. En option, on peut lire ce code gitee comme proxy (démon root kylin_software_center_daemon/, 5 fichiers).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 4 paquets binaires Architecture any : kylin-software-center, -plugin-synchrodata ('regularly visits the server to obtain data updates'), -plugin-expand ('Software store access the server to get data'), -plugin-preprocessing. Homepage http://gitlab2.kylin.com/kylin-desktop/kylin-software-center | CONSTATÉ |
| debian-rules | `archive:debian/rules` | seulement '%: dh $@' ; aucun code à construire (fichiers *.ex = modèles dh_make inactifs) | CONSTATÉ |
| systemd-service | `gitee:backend/aptdaemon/conf/softwarecenter-daemon.service` | Type=dbus ; BusName=com.kylin.softwarecenter ; ExecStart=/usr/bin/kylin-software-center-daemon ; User=root ; Restart=always ; WantedBy=multi-user.target ; Alias=dbus-com.kylin.softwarecenter.service ; installé dans /lib/systemd/system par setup.py | CONSTATÉ |
| dbus-service | `gitee:backend/aptdaemon/conf/com.kylin.softwarecenter.service` | bus système : Name=com.kylin.softwarecenter ; Exec=/usr/bin/kylin-software-center-daemon ; User=root (installé dans share/dbus-1/system-services) | CONSTATÉ |
| dbus-policy | `gitee:backend/aptdaemon/conf/com.kylin.softwarecenter.conf` | policy user=root own=com.kylin.softwarecenter ; policy context=default allow send_destination=com.kylin.softwarecenter (interfaces com.kylin.softwarecenter, Introspectable, Properties) ; installé dans /etc/dbus-1/system.d | CONSTATÉ |
| polkit | `gitee:backend/aptdaemon/conf/com.kylin.softwarecenter.policy` | action com.kylin.softwarecenter.action : allow_any=auth_admin, allow_inactive=auth_admin, allow_active=auth_admin_keep | CONSTATÉ |
| systemd-user-service | `gitee:kylin-software-center.service` | installé dans lib/systemd/user (setup.py) : ExecStart=/usr/bin/python3 /usr/share/kylin-software-center/backend/remote/remoter_daemon.py ; Restart=always ; WantedBy=default.target. Le répertoire backend/remote/ est absent de l'arbre. | CONSTATÉ |
| dbus-session-service | `gitee:backend/utildaemon/conf/com.kylin.utiliface.service` | installé dans share/dbus-1/services : Name=com.kylin.utiliface ; Exec=/usr/share/kylin-software-center/utils/utiliface_dbus. L'arbre contient utils/utiliface_dbus.py (avec extension). | CONSTATÉ |
| dbus-service+policy+systemd | `gitee:backend/run_app/com.kylin.softwarecenter.runapp.service ; gitee:backend/run_app/com.kylin.softwarecenter.runapp.conf ; gitee:backend/run_app/softwarecenter-runapp-daemon.service` | Name=com.kylin.softwarecenter.runapp, Exec=python3 start_run_dbus.py ; la policy autorise own au user 'kylin' et l'envoi au contexte default. Ces fichiers ne figurent pas dans les data_files de setup.py, donc probablement non installés. | PROBABLE |
| maintainer-script | `gitee:debian/postinst` | supprime /home/*/.cache/uksc/uksc.db*, kylin-software-center.conf et /etc/xdg/autostart/kylin-software-center-autostart.desktop ; chmod 777 /usr/share/kylin-software-center/backend/service/password.txt et /usr/share/kylin-software-center/kylin-software-center.py ; chmod 775 .../kylin_software_center_daemon/start_systemdbus.py ; chmod 774 kylin-software-center.conf | CONSTATÉ |
| maintainer-script | `gitee:debian/prerm ; gitee:debian/postrm` | exit 0 (lignes pkill commentées) | CONSTATÉ |
| debian-links | `gitee:debian/links` | /usr/bin/kylin-software-center-daemon -> /usr/lib/python3/dist-packages/kylin_software_center_daemon/start_systemdbus.py ; /usr/bin/kylin-software-center-runapp -> .../backend/run_app/start_run_dbus.py | CONSTATÉ |
| build/setup.py | `gitee:setup.py` | data_files installés : /etc/dbus-1/system.d, share/dbus-1/system-services, share/polkit-1/actions, /etc/rsyslog.d/58-kylin-software-center.conf, lib/systemd/user, /lib/systemd/system, share/dbus-1/services, data/uksc.db, backend/service/*.txt. La ligne d'installation de l'autostart est commentée. | CONSTATÉ |
| desktop-file/autostart | `gitee:kylin-software-center.desktop ; gitee:kylin-software-center-autostart.desktop` | Exec=/usr/bin/ubuntu-kylin-software-center %u ; l'autostart (Exec=... -quiet) n'est pas installé (ligne commentée dans setup.py) | CONSTATÉ |
| listener-local | `gitee:kylin-software-center.py` | QLocalServer().listen(serverName) : socket local servant à garantir une instance unique, pas réseau (l. 50-52) | CONSTATÉ |
| binaire-livre | `(paquets ISO kylin-software-center 5.0.9.1 et plugins synchrodata/expand/preprocessing)` | les points d'entrée réels de la version installée sont inconnus faute de source | NON COUVERT |

- [CONSTATÉ] Archive et gitee n'ont pas la même structure : l'archive est un squelette dh_make vide (debian/ seul), alors que gitee contient un arbre Python complet portant un autre nom de source (ubuntu-kylin-software-center) et une autre version (4.5.77.1).
- [CONSTATÉ] Installé par défaut par l'ISO : kylin-software-center plus 3 plugins (expand, preprocessing, synchrodata), sans source correspondante.
- [CONSTATÉ] /usr/bin/kylin-software-center et /usr/bin/kylin-software-center-plugin-synchrodata figurent dans la liste blanche d'appelants de kylin-system-updater (archive:kylin-system-updater/backend/data/com.kylin.systemupgrade.limit, key9 et key13).
- [PROBABLE] Le contenu réel des plugins synchrodata/expand (accès serveur, selon debian/control) relève probablement de la collecte ou de la synchronisation de données. Comportement à confirmer sur binaire.
- [CONSTATÉ] [correction critique] Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.

### `kylin-installer`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur wanghailiang@kylinos.cn). Intègre probablement QtSingleApplication (qt/qtlocalpeer.*, qtlockedfile*, qtsingleapplication.*, composant Qt Solutions).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 3.0 native). Gitee : forge PRESENT/DIVERGENT mais non acquise dans le corpus local.
- **Langages / taille** : C++/Qt5 (430 Ko .cpp + 85 Ko .h), qmake (18 Ko), Python et Shell pour les tests uniquement. Le libellé 'TypeScript' de la fiche (430 Ko) correspond aux traductions Qt .ts. — 201 fichiers / 1,85 Mo ; 97 fichiers de code, ~15 300 lignes, dont 22 fichiers de tests — lignes de code propres mesurées : 15 277
- **Privilège** : Application de la session utilisateur. Élévation possible via polkit : pkexec /usr/bin/rm autorisé sans authentification (org.kylin.uninstaller.action=yes). Les installations passent probablement par le démon root com.kylin.systemupgrade.
- **Déclencheurs** : ouverture d'un fichier .deb, .apk, .kaiming ou .ok (MimeType), lancement manuel kylin-installer / kylin-uninstaller, pkexec /usr/bin/rm (action org.kylin.uninstaller.action)
- **Indices réseau** : aucun listener réseau ; QLocalServer local (CONSTATÉ); seule URL trouvée par grep : http://www.kylinos.cn (vendor polkit) (CONSTATÉ)
- **Blobs** : 1 blob : tests/fileInfo/app.web.xuexi_1.0.0_all.deb (26 Ko, fixture de test, non installé d'après les .pro). Images PNG du guide utilisateur. debian/kylin-installer-coverage.xml (146 Ko, artefact de couverture).
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre de taille raisonnable (~15 k lignes) installé par défaut, situé sur le chemin d'installation de paquets, avec une action polkit 'yes' sur /usr/bin/rm.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| build-qmake | `archive:kre.pro` | TEMPLATE subdirs : qt/kylin-installer.pro et uninstall_qt/kylin-uninstaller.pro | CONSTATÉ |
| build-qmake | `archive:qt/kylin-installer.pro` | TARGET kylin-installer -> /usr/bin ; INSTALLS target, translation, desktop, guideCN, guideEN, imageformats. Aucune installation de polkit. | CONSTATÉ |
| build-qmake | `archive:uninstall_qt/kylin-uninstaller.pro` | TARGET kylin-uninstaller -> /usr/bin ; INSTALLS polkit (org.kylin.uninstaller.policy -> /usr/share/polkit-1/actions/) et config/irregularPackage.list -> /usr/share/kylin-installer/ | CONSTATÉ |
| polkit | `archive:uninstall_qt/org.kylin.uninstaller.policy` | action org.kylin.uninstaller.action : allow_any=yes, allow_inactive=yes, allow_active=yes ; org.freedesktop.policykit.exec.path=/usr/bin/rm ; exec.allow_gui=true | CONSTATÉ |
| polkit | `archive:qt/org.kylin.installer.policy` | action org.kylin.installer.action : auth_admin (any, inactive, active). Le fichier n'est référencé par aucun INSTALLS de qt/kylin-installer.pro. | CONSTATÉ |
| desktop-file | `archive:qt/kylin-installer.desktop` | Exec=/usr/bin/kylin-installer %U ; MimeType=application/vnd.debian.binary-package;application/vnd.android.package-archive;application/x-kaiming;application/x-ok | CONSTATÉ |
| debian-control | `archive:debian/control` | Package kylin-installer (Architecture any) ; Depends libxml2, kylin-system-updater ; Build-Depends libpolkit-qt5-1-dev, libqapt-dev, libssl-dev, libkysdk-package-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; DEB_BUILD_MAINT_OPTIONS hardening=+all ; dh_shlibdeps --ignore-missing-info. Aucun script mainteneur dans debian/. | CONSTATÉ |
| listener-local | `archive:qt/qtlocalpeer.cpp` | QLocalServer::listen(socketName 'qtsingleapp-<prefix>-<uid>') (l. 103-126) : socket local d'instance unique, pas réseau | CONSTATÉ |
| ipc-dbus-client | `archive:debian/control + archive:kylin-system-updater/backend/data/com.kylin.systemupgrade.limit` | Dépend de kylin-system-updater ; /usr/bin/kylin-installer et /usr/bin/kylin-uninstaller figurent dans la liste blanche d'appelants du démon root com.kylin.systemupgrade | PROBABLE |

- [CONSTATÉ] Installé par défaut par l'ISO (kylin-installer=2.0.2.1-ok4.16).
- [PROBABLE] L'action cn.kylin.installer.action (auth_admin_keep), définie dans kylin-system-updater, est probablement celle qu'utilise l'installeur, puisque org.kylin.installer.policy n'est pas installé.
- [NON COUVERT] Le dépôt Gitee (branches openkylin/huanghe, nile, yangtze...) n'est pas présent dans le corpus local, donc aucune comparaison archive/gitee n'a pu être faite.

### `kmre-apk-installer`

- **Nature** : absent-ou-vide — **Amont** : Aucun identifiable. Kylin d'après les mainteneurs (luzhiping@kylinos.cn, lixiang@kylinos.cn) ; changelog '2.4.1.3 yangtze, Build for openKylin' (2022).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : l'archive est un squelette dh_make (meta.source_coquille_vide=1) et les forges Gitee et GitHub sont ABSENT
- **Langages / taille** : aucun — 19 fichiers / 26 Ko (modèles dh_make) — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (pas de code). D'après debian/control, il s'agit d'une application Qt de session.
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : Depends libqt5network5 et libssl1.1 dans debian/control : usage réseau probable (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source dans l'archive ni sur les forges : seul un squelette dh_make est présent.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Architecture amd64 ; Depends libqt5network5, libqt5dbus5, libssl1.1 (>= 1.1.1), aapt, android-libaapt, android-libandroidfw, android-libziparchive... ; 'A qt window to install apk packages' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul ; postinst.ex, prerm.ex, postrm.ex, preinst.ex et cron.d.ex sont des modèles inactifs | CONSTATÉ |
| binaire | `(binaire kmre-apk-installer)` | points d'entrée réels inconnus | NON COUVERT |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.
- [CONSTATÉ] Le paquet déclare une dépendance sur libssl1.1, absente des séries récentes.

### `kylin-software-properties`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Original-Maintainer kylin@kylinos.cn)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun. L'archive est un squelette dh_make (1.0.1.1-ok1.24). Gitee contient aussi un squelette dh_make (branche openkylin/nile, version 0.0.1-136k2, 1 commit 'Import Upstream version 0.0.1'), seul l'en-tête debian/control y est enrichi.
- **Langages / taille** : aucun — archive : 19 fichiers / 26 Ko ; gitee : 19 fichiers / 27,8 Ko — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT. Les dépendances polkit-qt et D-Bus et le nom '-service' suggèrent un composant privilégié (PROBABLE).
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : Depends libqt5network5 dans gitee:debian/control (PROBABLE); gestionnaire de sources APT ('soft source manager') : il manipule probablement les dépôts APT (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source, ni dans l'archive ni sur Gitee (deux squelettes dh_make).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package kylin-software-properties, Architecture any, 'kylin soft source manager' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul | CONSTATÉ |
| debian-control | `gitee:debian/control` | 4 strophes (riscv64, arm64, loong64, amd64). Depends libpolkit-qt5-1-1, libqt5dbus5, libqt5network5, libdbus-glib-1-2, libglib2.0-0t64, nvme-cli | CONSTATÉ |
| debian-rules | `gitee:debian/rules` | '%: dh $@' seul | CONSTATÉ |
| service-binaire | `archive:kylin-system-updater/backend/data/com.kylin.systemupgrade.limit` | /usr/bin/kylin-software-properties-service figure dans la liste blanche (key5) du démon root de mise à jour. Ce binaire n'est fourni par aucune source du corpus. | PROBABLE |

- [CONSTATÉ] Installé par défaut par l'ISO (kylin-software-properties=1.0.1.1-ok1.24) sans aucune source disponible.
- [CONSTATÉ] Les versions divergent : archive 1.0.1.1-ok1.24 (huanghe, 2026-08-27), gitee 0.0.1-136k2 (nile, 2024-08-29).

### `dpkg-wrapper`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Original-Maintainer sunzhen1@kylinos.cn ; Maintainer affiché : Ubuntu Developers)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : squelette dh_make dans l'archive (alors que meta.source_coquille_vide=0) ; forges ABSENT
- **Langages / taille** : aucun — 19 fichiers / 25,7 Ko (modèles dh_make) — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT. Le nom et la description indiquent un emballage de dpkg, qui s'exécute normalement en root (PROBABLE).
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : —
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; seul un squelette dh_make est présent.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package dpkg-wrapper, Architecture all ; Description 'a dpkg wrapper. hook some essential options and make it accessible.' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul ; preinst.ex, postinst.ex, prerm.ex, postrm.ex sont des modèles inactifs | CONSTATÉ |
| binaire | `(binaire dpkg-wrapper)` | points d'entrée réels inconnus | NON COUVERT |

- [CONSTATÉ] Le drapeau automatique meta.source_coquille_vide vaut 0 alors que l'arbre ne contient que le squelette dh_make (fichiers *.ex, rules 'dh $@').
- [CONSTATÉ] Non installé par défaut par l'ISO 3.0. Changelog 'nile.bedrock-proposed' du 2026-06-25.

### `aptss`

- **Nature** : fork-amont — **Amont** : Spark Store / GXDE-OS : aptss 4.8.0 (debian/changelog '同步星火4.8.0' ; Homepage https://gitee.com/GXDE-OS/aptss/ ; README : 'part of Spark APP Store https://gitee.com/spark-store-project/spark-store'). ss-apt-fast dérive probablement d'apt-fast (URL www.mattparnell.com dans ss-apt-fast). debian/watch et debian/upstream/metadata sont des modèles vides.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, paquet natif). Forge Gitee PRESENT mais non acquise localement.
- **Langages / taille** : Shell/Bash (~3 350 lignes réparties sur une quinzaine de scripts, la plupart sans extension), Amber (.amber), configuration APT, fichiers de traduction .transhell — 50 fichiers / 168 Ko — lignes de code propres mesurées : 489
- **Privilège** : root pour les scripts mainteneur et le trigger dpkg. Commandes CLI aptss, ssinstall, ssaudit : élévation non déterminée. La policy polkit store.spark-app.ssinstall.policy est référencée mais n'est pas fournie par ce paquet.
- **Déclencheurs** : installation, mise à jour et suppression du paquet (preinst, postinst, prerm), trigger dpkg sur /opt/apps (exécute spark-dstore-patch en root), CLI utilisateur aptss / ssinstall, handler d'URL spk:// (xdg-mime vers spark-store.desktop, fichier non fourni)
- **Indices réseau** : Depends wget, aria2, curl, gnupg (CONSTATÉ); grep d'URL : https://d.spark-app.store/, https://amber-ce-resource.spark-app.store/store/depends/spark-store-console-in-container_latest_all.deb (PROBABLE); clé APT de confiance ajoutée par postinst : /etc/apt/trusted.gpg.d/spark-store.gpg (CONSTATÉ)
- **Blobs** : aucun binaire. Clé publique OpenPGP src/opt/durapps/spark-store/bin/spark-store.asc, installée comme clé APT de confiance.
- **Budget proposé** : L3 — 95 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Code tiers réduit (~3 350 lignes de shell) qui modifie la confiance APT (clé dans trusted.gpg.d, sources dédiées) et s'exécute en root via postinst et trigger.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/aptss.postinst` | configure : dpkg --add-architecture i386 (x86_64) ; rm /etc/apt/sources.list.d/sparkstore.list ; ln -sf vers /usr/bin/ssaudit, /usr/local/bin/ssaudit, /usr/bin/spark-dstore-patch, /usr/local/bin/spark-dstore-patch, /usr/bin/aptss ; ln -s /opt/durapps/spark-store/bin/auto-install-policy/store.spark-app.ssinstall.policy /usr/share/polkit-1/actions/ (cible absente de l'arbre) ; gpg --dearmor spark-store.asc puis copie vers /etc/apt/trusted.gpg.d/spark-store.gpg ; xdg-mime default spark-store.desktop x-scheme-handler/spk. triggered : exécute spark-dstore-patch. | CONSTATÉ |
| dpkg-trigger | `archive:debian/aptss.triggers` | interest-noawait /opt/apps (déclenche postinst 'triggered', qui lance spark-dstore-patch) | CONSTATÉ |
| maintainer-script | `archive:debian/aptss.preinst` | définit network-check (curl -I www.baidu.com) sans l'appeler (ligne commentée) ; affiche '不再检测网络' | CONSTATÉ |
| maintainer-script | `archive:debian/aptss.prerm` | remove/purge : unlink des liens ; rm -rf /etc/aptss/ /var/lib/aptss/ ; rm /etc/xdg/autostart/spark-update-notifier.desktop ; rm -rf /home/*/.config/spark-union/spark-store ; rm de la policy polkit ssinstall ; rm /etc/apt/trusted.gpg.d/spark-store.gpg ; apt-key del '9D9A A859 F750 24B1 A1EC E16E 0E41 D354 A29A 440C' | CONSTATÉ |
| debian-install | `archive:debian/install` | 'src/* /' : installe /opt/durapps/spark-store/bin/*, /usr/share/aptss/ et /usr/share/bash-completion/completions/aptss | CONSTATÉ |
| apt-config | `archive:src/opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf` | Dir::Etc::SourceParts '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/' ; Dir::State::lists '/var/lib/aptss/lists/' ; Dir::Cache '/var/lib/aptss/' ; Debug::RunScripts true ; APT::Get::Fix-Broken true | CONSTATÉ |
| apt-sources-dir | `archive:src/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/.keep` | répertoire de sources APT dédié, vide dans l'arbre ; son contenu est fourni à l'exécution | CONSTATÉ |
| debian-control | `archive:debian/control` | Architecture all ; Depends wget, aria2, curl, gnupg ; Provides et Conflicts spark-store, spark-store-console | CONSTATÉ |
| scripts-cli | `archive:src/opt/durapps/spark-store/bin/{aptss,ssinstall,ssaudit,spark-dstore-patch,store-helper/pass-auth.sh,store-helper/uninstaller,update-upgrade/*.sh}` | scripts exécutables installés ; mode d'élévation et déclencheurs (dont l'autostart spark-update-notifier) non lus | NON COUVERT |

- [CONSTATÉ] meta.origine_kylin=1, mais le mainteneur (shenmo@spark-app.store) et l'Homepage (GXDE-OS) désignent un tiers non Kylin.
- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.
- [CONSTATÉ] La copyright déclare 'src/DEBIAN/control', un fichier absent de l'arbre.

### `kylin-web-app-engine`

- **Nature** : absent-ou-vide — **Amont** : Kylin (mainteneur liuyaxian@kylinos.cn)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : squelette dh_make dans l'archive (meta.source_coquille_vide=1) ; forges ABSENT
- **Langages / taille** : aucun — 19 fichiers / 26 Ko (modèles dh_make) — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : la description 'engine of launching web applications' suggère un contenu web (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; seul un squelette dh_make est présent.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Package kylin-web-app-engine, Section libs, Architecture any ; 'an engine of launching web applications.' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul ; scripts *.ex inactifs | CONSTATÉ |
| binaire | `(binaire kylin-web-app-engine)` | points d'entrée réels inconnus | NON COUVERT |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.

### `kaiming`

- **Nature** : propre-kylin — **Amont** : Kylin, SIG KaiMing (Vcs-Git https://gitee.com/openkylin/kaiming.git). Intègre du code tiers : squashfuse (ok/engine/squashfuse/, 216 Ko, PROBABLE) et, selon NOTICE, des composants libdeflate, curl, nlohmann-json, zlib, elfutils, ostree et util.
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 3.0 native). Le dépôt gitee openkylin/kaiming n'est pas acquis localement.
- **Langages / taille** : C++ (~2,0 Mo), C (297 Ko, dont dbus/gdbus/kmsysteminterface.c de 293 Ko, probablement généré par gdbus-codegen), en-têtes C/C++ (450 Ko), Shell (143 Ko), CMake, XML D-Bus, Python (tests) — 453 fichiers / 3,8 Mo ; 399 fichiers de code, ~94 800 lignes, dont 170 fichiers de tests — lignes de code propres mesurées : 94 897
- **Privilège** : root : kaiming-system-dbus (unité systemd), kaiming-systemproxy-dbus (D-Bus User=root), et pkexec de kaiming, setfattr, mount et umount sans authentification (polkit yes). Utilisateur : km-ses-dbusproxy, kaiming-sessionproxy-dbus, km-cli, km-run, ok-engine.
- **Déclencheurs** : boot (graphical.target, unité activée par postinst), activation D-Bus système (org.kylin.kaiming, org.kaiming.proxy) et session, ouverture de session utilisateur (km-ses-dbusproxy, default.target), pkexec (4 actions polkit), ouverture de paquets .ok ou .kaiming (MimeType déclaré par kylin-installer), générateur d'environnement systemd (60-kaiming), shell de login (/etc/profile.d) et Xsession, postinst (adduser, scripts, enable/start)
- **Indices réseau** : remote ostree https://archive2.kylinos.cn/KM/repo-v11 avec gpg-verify=false (shells/kaiming_version.sh) (CONSTATÉ); Build-Depends libcurl4-gnutls-dev, libostree-dev (CONSTATÉ); grep d'URL : IP privées 172.30.60.17, 172.30.60.202, 10.41.118.234 dans app/remoteModify/KMRemoteModify.cpp et les tests ; https://archive2.kylinos.cn/KM/OPENKYLIN/repo (PROBABLE); socket Unix local, pas réseau (CONSTATÉ)
- **Blobs** : aucun binaire. debian/kaiming-coverage.xml (619 Ko, artefact de couverture). À la construction, shells/bundle_dependencies.sh copie les bibliothèques dépendantes dans /opt/kaiming-tools/lib (debian/rules) : des binaires tiers sont donc embarqués dans le .deb (PROBABLE).
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant volumineux (~95 k lignes) installé par défaut, avec plusieurs services root, des actions polkit 'yes', un déploiement d'applications et une bibliothèque d'interception. On cible les services D-Bus, les chemins pkexec, le déploiement et l'installation, ok-engine/squashfuse et vfsredirect.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:data/org.kylin.kaiming.service` | Type=dbus ; BusName=org.kylin.kaiming ; ExecStart=/opt/kaiming-tools/bin/kaiming-system-dbus ; pas de User= (donc root) ; Restart=always ; WantedBy=graphical.target. Installé dans /opt/system/lib/systemd/system puis lié dans /lib/systemd/system par install_files.sh. | CONSTATÉ |
| dbus-policy | `archive:data/org.kylin.kaiming.conf` | /etc/dbus-1/system.d : policy user=root own=org.kylin.kaiming ; policy context=default allow send_destination=org.kylin.kaiming (interfaces org.kylin.kaiming.interface, Introspectable, Properties) | CONSTATÉ |
| dbus-service | `archive:data/org.kaiming.proxy.system.service` | bus système : Name=org.kaiming.proxy ; Exec=/opt/kaiming-tools/bin/kaiming-systemproxy-dbus ; User=root | CONSTATÉ |
| dbus-policy | `archive:data/org.kaiming.proxy.system.conf` | policy user=root own=org.kaiming.proxy ; context default allow send_destination=org.kaiming.proxy, interface org.kaiming.proxy.system | CONSTATÉ |
| dbus-session | `archive:data/org.kaiming.proxy.session.service ; archive:data/org.kaiming.proxy.session.conf` | bus session : Name=org.kaiming.proxy, Exec=/opt/kaiming-tools/bin/kaiming-sessionproxy-dbus ; la policy (installée dans /etc/dbus-1/session.d) ouvre l'envoi au contexte default | CONSTATÉ |
| systemd-user-service | `archive:data/km-ses-dbusproxy.service` | ExecStart=/opt/kaiming-tools/bin/km-ses-dbusproxy ; ConditionUser=!root et !lightdm ; ExecCondition uid >= 1000 ; WantedBy=default.target. Activé pour tous les utilisateurs via un lien dans /etc/systemd/user/default.target.wants créé par postinst. | CONSTATÉ |
| polkit | `archive:data/org.kylin.kaiming.policy` | 4 actions en allow_any/allow_inactive/allow_active=yes avec allow_gui=true : org.kylin.kaiming.install (exec.path /opt/kaiming-tools/bin/kaiming), org.kylin.kaiming.unsetkysec (/usr/bin/setfattr), org.kylin.kaiming.mount (/usr/bin/mount), org.kylin.kaiming.umount (/usr/bin/umount) | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming.postinst` | adduser --system --group _kaiming ; mkdir /opt/kaiming/... ; exécute /opt/kaiming-tools/bin/kaiming_version.sh et install_files.sh ; killall kaiming-system-dbus, kaiming-systemproxy-dbus, kaiming-sessionproxy-dbus, km-ses-dbusproxy ; deb-systemd-helper unmask, systemctl enable puis deb-systemd-invoke start org.kylin.kaiming.service ; lien utilisateur global km-ses-dbusproxy.service | CONSTATÉ |
| maintainer-script(appelé) | `archive:shells/kaiming_version.sh` | exécuté par postinst : réécrit /opt/kaiming/repo/config avec [remote "kaiming-repo"] gpg-verify=false, gpg-verify-summary=false, url=https://archive2.kylinos.cn/KM/repo-v11 | CONSTATÉ |
| maintainer-script(appelé) | `archive:shells/install_files.sh` | exécuté par postinst : mkdir /var/log/kaiming puis chmod o+rwx ; liens symboliques vers /lib/systemd/system, /lib/systemd/user, /usr/share/dbus-1/{system-services,services}, /usr/share/polkit-1/actions, /usr/share/mime, /usr/bin/{kaiming,km-cli,km-run,kaiming-system-dbus,kaiming-sessionproxy-dbus,kaiming-systemproxy-dbus} | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming.postrm` | stop et disable org.kylin.kaiming.service ; suppression des liens symboliques créés par install_files.sh | CONSTATÉ |
| build-cmake | `archive:CMakeLists.txt` | exécutables km-cli, km-run, kaiming-system-dbus, kaiming-sessionproxy-dbus, kaiming-systemproxy-dbus -> /opt/kaiming-tools/bin ; sous-répertoires dbus, ok (ok-engine), dbusproxy (km-ses-dbusproxy) et tools (libvfspreload) ; ETC_PATH=/etc, LIB_PATH=/opt/system/lib, USR_SHARE_PATH=/opt/system/resource | CONSTATÉ |
| systemd-env-generator | `archive:data/60-kaiming` | installé (USE_SOURCE_PERMISSIONS) dans /etc/systemd/system-environment-generators : ajoute /opt/kaiming/share à XDG_DATA_DIRS | CONSTATÉ |
| profile.d/Xsession | `archive:data/kaiming.sh ; archive:data/20kaiming ; archive:data/60-kaimingenv.conf` | /etc/profile.d/kaiming.sh (XDG_DATA_DIRS, PATH+=/opt/kaiming/bin) ; /etc/X11/Xsession.d/20kaiming ; /etc/ostree/ostree.conf.d/60-kaimingenv.conf (PATH=/var/opt/kaiming/bin:$PATH) | CONSTATÉ |
| bibliotheque-preload | `archive:tools/CMakeLists.txt` | bibliothèque partagée vfspreload (vfsredirect.c, 'intercepter et rediriger les appels au système de fichiers du conteneur') installée dans /opt/kaiming-tools/lib | CONSTATÉ |
| listener-local | `archive:common/KMUnixSocketServer.cpp` | socket(AF_UNIX, SOCK_STREAM, 0) ; bind(m_serverfd, sun_path=m_socketPath) ; listen(m_serverfd, 1) (l. 73-93) : serveur de socket Unix local. Chemin du socket et appelants NON COUVERT. | CONSTATÉ |
| triggers-internes | `archive:shells/desktop-database.trigger ; gtk-icon-cache.trigger ; mime-database.trigger` | installés dans /opt/kaiming-tools/share/kaiming/shells ; sourcent /etc/profile.d/kaiming.sh puis lancent update-desktop-database etc. | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (kaiming=1.0.0-ok1k0.96).
- [CONSTATÉ] Le debian/changelog (ok1k0.96) indique que le déploiement d'applications réécrit les champs User/Group/ExecStart/ExecStop des services systemd système des applications, le niveau user étant désormais exclu : « 系统级服务改写逻辑保持不变 ».
- [CONSTATÉ] Les unités et policies sont installées hors des chemins standards (/opt/system/...) puis liées vers /lib, /usr/share et /usr/bin par un script postinst : ces liens ne sont pas suivis par dpkg.
- [CONSTATÉ] debian/control déclare Rules-Requires-Root: no et DEB_BUILD_OPTIONS=nocheck (les tests ne sont pas exécutés à la construction).

### `kaiming-box`

- **Nature** : absent-ou-vide — **Amont** : Probablement bubblewrap renommé : debian/control reprend mot pour mot la description de bubblewrap ('utility for unprivileged chroot and namespace manipulation'), avec Homepage https://github.com/containers/kaiming-box et Original-Maintainer 'Utopia Maintenance Team' (mainteneurs Debian de bubblewrap).
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : squelette dh_make dans l'archive (alors que meta.source_coquille_vide=0) ; forges ABSENT
- **Langages / taille** : aucun — 19 fichiers / 27,6 Ko (modèles dh_make) — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT. D'après la description : namespaces non privilégiés, avec option setuid root.
- **Déclencheurs** : NON COUVERT (probablement invoqué par kaiming pour lancer des applications en bac à sable)
- **Indices réseau** : —
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; l'amont probable est bubblewrap mais la version et le delta Kylin sont inconnus.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 2 strophes (arm64, amd64) ; Depends libc6 (>= 2.38), libcap2, libselinux1 ; description : 'system administrators can make the kaiming-box executable setuid root' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul ; scripts *.ex inactifs | CONSTATÉ |
| binaire | `(binaire kaiming-box)` | bit setuid et points d'entrée réels inconnus | NON COUVERT |

- [CONSTATÉ] Le drapeau automatique meta.source_coquille_vide vaut 0 alors que l'arbre ne contient que le squelette dh_make.
- [CONSTATÉ] Non installé par défaut par l'ISO 3.0. Changelog '1.0.1 nile' du 2024-05-24.

### `kaiming-builder`

- **Nature** : propre-kylin — **Amont** : Kylin, SIG KaiMing (Vcs-Git https://gitee.com/openkylin/kaiming.git)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 3.0 native). Gitee non acquis localement.
- **Langages / taille** : C++ (463 Ko) + en-têtes (149 Ko), Shell (8,6 Ko), YAML, CMake — 130 fichiers / 766 Ko ; 107 fichiers de code, ~22 000 lignes, sans tests — lignes de code propres mesurées : 21 995
- **Privilège** : CLI de l'utilisateur, exécutable en root via pkexec sans authentification (polkit 'yes' sur /opt/kaiming-tools/bin/kaiming-builder)
- **Déclencheurs** : CLI kaiming-builder, kaiming-buildpackage, kch, pkexec /opt/kaiming-tools/bin/kaiming-builder, kaiming-builder-gui (PROBABLE)
- **Indices réseau** : fichiers sources KMBuilderSourceGit.cpp, KMBuilderSourceArchive.cpp, KMBuilderSourceFile.cpp : récupération de sources distantes probable (noms de fichiers uniquement) (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre de taille raisonnable (~22 k lignes) dont l'exécutable est autorisé en root sans authentification par polkit, et qui exécute des scripts et commandes de construction. Il n'est pas installé par défaut : le coordinateur peut abaisser la priorité.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| polkit | `archive:data/cn.kylin.kaiming-builder.policy` | action cn.kylin.kaiming-builder.polkit : allow_any=yes, allow_inactive=yes, allow_active=yes ; exec.path=/opt/kaiming-tools/bin/kaiming-builder | CONSTATÉ |
| build-cmake | `archive:CMakeLists.txt` | kaiming-builder -> /opt/kaiming-tools/bin ; policy -> ${USR_SHARE_PATH}/polkit-1/actions ; kaiming-buildpackage, kch et build_post_hook.sh -> /opt/kaiming-tools/bin ; template.yaml -> /opt/kaiming-tools/config ; complétion -> ${ETC_PATH}/bash_completion.d | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming-builder.postinst` | si /ostree/pkgs/system-ovl.conf est absent : liens /usr/bin/kaiming-builder, kaiming-buildpackage, kch ; lien /usr/share/polkit-1/actions/cn.kylin.kaiming-builder.policy -> /opt/system/resource/... ; lien de complétion bash et .mo. Sinon, liens vers /opt/system/bin. | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming-builder.postrm` | suppression des liens symboliques | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends kaiming (>= 1.0.0-ok1k0.16), dpkg-dev ; Build-Depends libssl-dev, libyaml-cpp-dev, nlohmann-json3-dev, patchelf | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh_auto_install puis ./shells/bundle_dependencies.sh (copie des dépendances dans /opt/kaiming-tools/lib) ; dh_shlibdeps -l/opt/kaiming-tools/lib | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.
- [CONSTATÉ] Le changelog porte la distribution 'v11-2603-preview' et non 'huanghe'.

### `kaiming-builder-gui`

- **Nature** : propre-kylin — **Amont** : Kylin, SIG KaiMing (Vcs-Git https://gitee.com/openkylin/kaiming.git)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 3.0 native). Gitee non acquis localement.
- **Langages / taille** : C++/Qt5 (384 Ko) + en-têtes (71 Ko), qmake, Shell. Le libellé 'TypeScript' (107 Ko) correspond à qt_zh_CN.ts (traduction Qt). — 124 fichiers / 2,85 Mo, dont resources/images/bg.png (2,18 Mo) ; 96 fichiers de code, ~14 100 lignes — lignes de code propres mesurées : 14 130
- **Privilège** : GUI de la session utilisateur (pas de service ni de polkit propre). Invoque probablement kaiming-builder (PROBABLE).
- **Déclencheurs** : lancement manuel (.desktop)
- **Indices réseau** : seules des URL d'exemple dans l'interface (https://github.com/user/repo.git, https://..../dmeo.sh) (CONSTATÉ)
- **Blobs** : aucun binaire exécutable ; images PNG et .qm
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Application de faible privilège, sans service ni polkit propre. Triage limité à l'appel de kaiming-builder.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| build-qmake | `archive:kaiming-builder-gui.pro` | target.path=/opt/kaiming-tools/bin ; data.path=/opt/kaiming-tools/share/kaiming-builder-gui. Variante conditionnelle : /opt/apps/top.openkylin.kaiming-builder-gui/files/{bin,share}. | CONSTATÉ |
| desktop-file | `archive:data/kaiming-builder-gui.desktop ; archive:kaiming-builder-gui.desktop` | Exec=/opt/kaiming-tools/bin/kaiming-builder-gui (resp. Exec=kaiming-builder-gui) ; application | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming-builder-gui.postinst` | liens /usr/bin/kaiming-builder-gui et /usr/share/applications/kaiming-builder-gui.desktop (ou /opt/system/... en mode V11) | CONSTATÉ |
| maintainer-script | `archive:debian/kaiming-builder-gui.postrm` | suppression des liens | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends kaiming, kaiming-builder | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.

### `kylin-system-updater`

- **Nature** : propre-kylin — **Amont** : Kylin (Kylin Development Team cp@kylinos.cn). Le sous-répertoire unattended-upgrades/ dérive probablement de unattended-upgrades Debian/Ubuntu (Documentation=man:unattended-upgrade(8), configuration reprenant le modèle 50unattended-upgrades).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) + gitee (branche openkylin/huanghe, même version 3.0.0.0-ok56). D'après diff -rq, gitee est identique à l'archive à l'exception de 34 fichiers .po et d'un .gitignore.
- **Langages / taille** : Python (915 Ko : backend ~12 800 lignes, backend-immutable ~8 150 lignes), Shell (43 Ko, surtout des tests), JSON, configuration APT et systemd — archive : 168 fichiers / 1,9 Mo ; gitee : 203 fichiers / 4,1 Mo (traductions en plus). 84 fichiers de code, ~22 100 lignes. — lignes de code propres mesurées : 22 178
- **Privilège** : root : démon D-Bus système com.kylin.systemupgrade, com.kylin.UpgradeStrategies (User=root) et kylin-unattended-upgrade-shutdown. La partie session se limite au script de notification /usr/bin/notify.
- **Déclencheurs** : boot (multi-user.target, 2 unités activées par postinst), activation et appels D-Bus système (ukcc, kylin-installer, kylin-update-frontend...), arrêt du système (unattended-upgrade-shutdown --wait-for-signal), autostart de session (notify.desktop), configuration APT (apt.conf.d), post_hooks du manifeste de mise à jour (immutable), postinst (DataMigration.py, quitStrategiesService.py)
- **Indices réseau** : téléchargements APT via aptdaemon (Depends aptdaemon) ; python3-launchpadlib recommandé (PROBABLE); upload_upgrade_log = True dans backend/data/system-updater-defaults.conf (CONSTATÉ); grep d'URL : http(s)://archive.kylinos-intranet.cn/kylin-update-manager-server/main.php et get_file.php dans backend/SystemUpdater/Core/DataAcquisition.py ; archive1.kylinos.cn ; changelogs.ubuntu.com (PROBABLE); méthode APT ftp déclarée (30kylin-system-updater) (CONSTATÉ); aucun listener réseau détecté (CONSTATÉ)
- **Blobs** : 2 bases SQLite (backend/data/kylin-system-updater.db et backend-immutable/data/kylin-system-updater.db, 40 Ko chacune), installées dans /usr/share/kylin-system-updater/ puis copiées dans /var/cache/kylin-system-updater/ par postinst
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant propre installé par défaut, qui porte la chaîne de mise à jour système en root (2 services D-Bus root, accès ouvert au contexte default, contrôle polkit, configuration APT, upload de journaux). Environ 21 k lignes Python réparties en deux variantes à comparer.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:backend/data/kylin-system-updater.service` | Type=dbus ; BusName=com.kylin.systemupgrade ; ExecStart=/usr/share/kylin-system-updater/kylin-system-updater ; pas de User= (root) ; Restart=always ; WantedBy=multi-user.target ; activé par postinst (systemctl enable) | CONSTATÉ |
| dbus-policy | `archive:backend/data/com.kylin.systemupgrade.conf` | policy user=root own=com.kylin.systemupgrade ; policy context=default allow send_destination=com.kylin.systemupgrade (interfaces com.kylin.systemupgrade.interface, Introspectable, Properties) | CONSTATÉ |
| dbus-service | `archive:backend/data/com.kylin.UpgradeStrategies.service` | bus système : Name=com.kylin.UpgradeStrategies ; Exec=/usr/share/kylin-system-updater/kylin-upgrade-strategies ; User=root | CONSTATÉ |
| dbus-policy | `archive:backend/data/com.kylin.UpgradeStrategies.conf` | policy user=root own=com.kylin.UpgradeStrategies ; policy context=default allow send_destination (interface com.kylin.UpgradeStrategies.interface) | CONSTATÉ |
| polkit | `archive:backend/data/cn.kylinos.KylinSystemUpdater.policy` | actions cn.kylin.installer.action, cn.kylin.uninstaller.action et cn.kylinos.KylinSystemUpdater.action en auth_admin_keep ; cn.kylin.software.center.action en auth_admin | CONSTATÉ |
| polkit | `archive:backend-immutable/data/cn.kylinos.KylinSystemUpdater.policy` | mêmes actions avec cn.kylin.software.center.action en auth_admin_keep, plus 4 actions *.self.action en auth_self_keep (installer, uninstaller, KylinSystemUpdater, software.center) | CONSTATÉ |
| polkit | `archive:backend/data/cn.kylinos.UpgradeStrategies.policy` | action com.kylin.UpgradeStrategies.action en auth_admin_keep (identique en backend-immutable) | CONSTATÉ |
| systemd-service | `archive:backend-immutable/data/kylin-system-updater.service` | ExecStart=/bin/sh -c : exécute /opt/update-module/current/usr/share/kylin-system-updater/kylin-system-updater s'il est exécutable, sinon /usr/share/... ; Environment LD_LIBRARY_PATH et GI_TYPELIB_PATH=/opt/update-module/current/usr/lib/aarch64-linux-gnu | CONSTATÉ |
| dbus-service | `archive:backend-immutable/data/com.kylin.UpgradeStrategies.service` | Exec=/bin/sh -c avec préférence pour /opt/update-module/current/.../kylin-upgrade-strategies ; User=root | CONSTATÉ |
| systemd-service | `archive:unattended-upgrades/kylin-unattended-upgrades.service` | ExecStart=/usr/bin/kylin-unattended-upgrade-shutdown --wait-for-signal ; Type=idle ; root ; WantedBy=multi-user.target ; activé par postinst | CONSTATÉ |
| autostart | `archive:unattended-upgrades/notify.desktop` | /etc/xdg/autostart : Exec=/usr/bin/notify (script Python de 71 lignes) ; NoDisplay | CONSTATÉ |
| apt-config | `archive:unattended-upgrades/kylin-unattended-upgrades` | /etc/apt/apt.conf.d : Unattended-Upgrade::Allowed-Origins 'Kylin:10.1' et ':default' ; DPkg::Options '--force-confnew' ; OnlyOnACPower 'True' ; Debug 'True' ; RetryTimes 3 | CONSTATÉ |
| apt-config | `archive:backend/data/30kylin-system-updater` | /etc/apt/apt.conf.d : Dir::Bin::Methods::ftp 'ftp' | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-system-updater.postinst` | mkdir /var/cache/kylin-system-updater et copie de kylin-system-updater.db ; systemctl enable kylin-system-updater.service et kylin-unattended-upgrades.service ; chmod +x /usr/bin/kylin-unattended-upgrade* ; 5 exécutions de /usr/share/kylin-system-updater/SystemUpdater/Core/DataMigration.py | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-system-updater-immutable.postinst` | même contenu, plus rm /var/lib/unattended-upgrades/unattended-upgrade.conf et exécution de SystemUpdater/Core/quitStrategiesService.py | CONSTATÉ |
| debian-install | `archive:debian/kylin-system-updater.install ; archive:debian/kylin-system-updater-immutable.install` | /usr/share/kylin-system-updater/ ; /usr/bin/report-updater-bug, kylin-unattended-upgrade, kylin-unattended-upgrade-shutdown, notify ; /etc/apt/apt.conf.d ; /etc/dbus-1/system.d ; /usr/share/dbus-1/system-services ; polkit ; /etc/logrotate.d ; /etc/kylin-version/. Variante immutable : /usr/share/update-manifests/hooks/set-netctl.py et /etc/kylin-system-updater/{update.env,settings-default.json} | CONSTATÉ |
| update-hook | `archive:backend-immutable/data/kylin-system-updater.manifest.json` | post_hooks : '/usr/sbin/kyexectl -s -r /opt/update-module/versions', 'python3 /usr/share/update-manifests/hooks/set-netctl.py', 'pkill -f .../kylin-upgrade-strategies' (2 chemins) | CONSTATÉ |
| dbus-acl | `archive:backend/data/com.kylin.systemupgrade.limit ; archive:backend-immutable/data/com.kylin.UpgradeStrategies.limit (+ .limit.verify)` | section [whitelist] listant les exécutables appelants autorisés (13 à 19 entrées, dont kylin-background-upgrade, kylin-installer, kylin-uninstaller, kylin-software-properties-service, kylin-software-center, kylin-unattended-upgrade, NotifySend). Le fichier .verify contient une empreinte hexadécimale. Ces fichiers ne sont pas installés par les .install (ligne commentée) : installation et usage NON COUVERT. | CONSTATÉ |
| config | `archive:backend/data/system-updater-defaults.conf` | installé dans /var/lib/kylin-system-updater/ : upload_upgrade_log = True ; upload_installer_log = False ; install_authority = True | CONSTATÉ |
| build | `archive:Makefile` | cd backend && ./setup.py build ; cd backend-immutable && ./setup.py build | CONSTATÉ |
| gitee-miroir | `gitee:(arbre complet)` | mêmes unités, policies et scripts que l'archive ; seuls diffèrent 17 .po supplémentaires dans backend/po et 17 dans backend-immutable/po | CONSTATÉ |
| apt-hook (apt.conf.d) [ajout critique] | `archive:kylin-system-updater/unattended-upgrades/kylin-unattended-upgrades ; archive:kylin-system-updater/backend/data/30kylin-system-updater` | Absents de la fiche (aucune categorie apt-hook). Le recensement ne dit que 'apt.conf.d'. Les deux fichiers sont installes dans /etc/apt/apt.conf.d/ (debian/kylin-system-updater.install:5 et 37). Le premier fixe DPkg::Options "--force-confnew" pour toutes les operations APT (l.55-56) et Unattended-Upgrade::Allowed-Origins "Kylin:10.1". Le second fixe Dir::Bin::Methods::ftp "ftp". | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (kylin-system-updater=3.0.0.0-ok56).
- [PROBABLE] Les fichiers backend/SystemUpdater/Core/DataAcquisition.py et la clé upload_upgrade_log évoquent une collecte de données vers un serveur Kylin. Contenu non lu en phase 0.
- [CONSTATÉ] La variante immutable exécute de préférence le code situé sous /opt/update-module/current (mise à jour indépendante du paquet).
- [CONSTATÉ] backend-immutable/data/inhibit-sleep.conf (AllowSuspend=no, AllowHibernation=no) est présent mais n'est pas listé dans les .install.

### `kylin-update-frontend`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur hechengyuan@kylinos.cn). Code tiers de test : immutable/tests/kt-test-utils/cpp-stub/elfio.hpp (ELFIO, PROBABLE).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) + gitee (branche openkylin/huanghe, version 2.0.0.0-ok36.8). D'après diff -rq, gitee est identique sauf pour les traductions (.po/.ts) et un .gitignore.
- **Langages / taille** : C++/Qt5 (712 Ko) + en-têtes (167 Ko), C (7 Ko, notification initial), qmake, CMake, Shell, Python (kylin_release_id_2209.py). Le libellé 'TypeScript' (1,8 Mo) correspond aux traductions Qt .ts. — archive : 373 fichiers / 3,3 Mo ; gitee : 436 fichiers / 4,7 Mo. 149 fichiers de code, ~27 300 lignes, dont 10 de tests. — lignes de code propres mesurées : 27 280
- **Privilège** : Session utilisateur : plugin chargé dans ukui-control-center, notifieurs en autostart, unité systemd user. Les scripts mainteneur s'exécutent en root (dpkg-divert, copie de bibliothèque, kysec_set).
- **Déclencheurs** : ouverture de ukui-control-center (plugin libupgrade.so), autostart de session (4 à 5 .desktop), unité systemd user graphical-session.target (NotifySend --WaitSignalNotify), installation et mise à jour du paquet (preinst, postinst)
- **Indices réseau** : aucun listener réseau détecté (CONSTATÉ); grep d'URL : https://archive1.kylinos.cn (PROBABLE); client D-Bus du démon root com.kylin.systemupgrade : Depends kylin-system-updater ; NotifySend et kylin-updateresult-notify-new figurent dans la liste blanche .limit de l'updater (PROBABLE)
- **Blobs** : 1 ELF précompilé : initial/kylin-updatefinish-notify-new (39 Ko, x86-64, non strippé, avec debug_info), au chemin même installé dans /usr/bin par debian/kylin-update-frontend.install. Il est probablement écrasé par la construction CMake (RUNTIME_OUTPUT_DIRECTORY).
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant de session utilisateur assez volumineux (~27 k lignes) qui dialogue en IPC avec le démon root de mise à jour. Zones ciblées : appels D-Bus, scripts mainteneur, plugin ukcc, notifieurs.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/kylin-update-frontend.install` | initial/plugin/libupgrade.so -> /usr/lib/upgrade/ ; initial/kylin-updatefinish-notify-new -> /usr/bin ; initial/notification/kylin-updatefinish-notify1.desktop -> /etc/xdg/autostart ; config_file -> /usr/share/ukui-control-center/upgrade/ | CONSTATÉ |
| debian-install | `archive:debian/kylin-update-frontend-immutable.install` | libupgrade.so -> /usr/lib/upgrade2.0/ ; /usr/bin/kylin-updateresult-notify-new, NotifySend, NotifySendStart, UpgradeRebootNotify, kylin_release_id_2209.py ; 4 .desktop en autostart ; kylin-stepinstall-notify.service -> /usr/lib/systemd/user | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-update-frontend.preinst ; archive:debian/kylin-update-frontend-immutable.preinst` | dpkg-divert --package kylin-system-updater --rename pour /usr/bin/kylin-updatefinish-notify, /usr/share/ukui-control-center/upgrade/kylin-need-logout.conf et kylin-need-reboot.conf | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-update-frontend.postinst` | rm puis cp de /usr/lib/upgrade/libupgrade.so vers /usr/lib/<multiarch>/ukui-control-center/ et .../ukui-control-center/V2.0/ (fichiers non suivis par dpkg) | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-update-frontend-immutable.postinst` | ln -s /usr/lib/upgrade2.0/libupgrade.so dans /usr/lib/<multiarch>/ukui-control-center/ ; si /usr/sbin/kysec_get existe : kysec_set -n exectl -v verified sur ce fichier | CONSTATÉ |
| systemd-user-service | `archive:immutable/NotifySend/Config/kylin-stepinstall-notify.service` | ExecStart=/usr/bin/NotifySend --WaitSignalNotify ; After=systemd-user-sessions.service ; WantedBy=graphical-session.target | CONSTATÉ |
| autostart | `archive:immutable/NotifySend/Config/kylin-reboot-installnotify.desktop ; kylin-stepinstall-notify.desktop ; immutable/notification/kylin-updateresult-notify.desktop ; initial\|immutable/notification/kylin-updatefinish-notify1.desktop` | Exec respectifs : /usr/bin/NotifySend --DesktopNotify ; /usr/bin/NotifySendStart ; kylin-updateresult-notify-new ; /usr/bin/kylin-updatefinish-notify-new | CONSTATÉ |
| desktop-file | `archive:initial/plugin/src/kylin-update-frontend.desktop` | Exec=ukui-control-center --upgrade ; NoDisplay=true. Le plugin libupgrade.so est chargé par ukui-control-center. | CONSTATÉ |
| systemd-service+timer | `archive:immutable/upgradenote/UpgradeRebootNotify.service ; .timer` | ExecStart=/usr/bin/RebootAttention ; OnCalendar=*-*-* 17:00 ; Persistent=true. Ces fichiers et le binaire RebootAttention sont absents des .install, donc probablement non installés. | PROBABLE |
| build | `archive:Makefile ; archive:initial/Makefile ; archive:immutable/Makefile ; archive:initial/notification/CMakeLists.txt` | qmake pour les plugins, NotifySend, RebootAttention et kylin-updateresult-notify ; cmake pour la notification initiale. add_executable(kylin-updatefinish-notify-new) avec RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../, c'est-à-dire initial/, chemin installé par le .install. | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (kylin-update-frontend=2.0.0.0-ok36.8).
- [CONSTATÉ] Archive et gitee sont alignés : seules les traductions diffèrent.
- [CONSTATÉ] Le postinst de la variante immutable marque libupgrade.so comme 'verified' dans le contrôle d'exécution kysec.

### `kylin-background-upgrade`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Original-Maintainer kylin@kylinos.cn ; Maintainer 'kylin <kylin@unknown>')
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : squelette dh_make dans l'archive (meta.source_coquille_vide=1) ; forges ABSENT
- **Langages / taille** : aucun — 19 fichiers / 27 Ko (modèles dh_make) — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT. Probablement un composant de session qui pilote le démon root de mise à jour, par analogie avec kylin-update-notify (PROBABLE).
- **Déclencheurs** : NON COUVERT (probablement des timers systemd user et un autostart, comme kylin-update-notify) (PROBABLE)
- **Indices réseau** : —
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; seul un squelette dh_make est présent.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 4 strophes (riscv64, amd64, loong64, arm64) ; Depends kylin-system-updater ; Breaks et Replaces kylin-update-notify, kylin-update-notify-immutable | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul ; scripts *.ex inactifs | CONSTATÉ |
| binaire | `(binaire /usr/bin/kylin-background-upgrade)` | figure dans la liste blanche (key1) de com.kylin.systemupgrade.limit de kylin-system-updater ; points d'entrée réels inconnus | NON COUVERT |

- [NON COUVERT] La source kylin-update-notify (1.3.2.1-ok17) construit un binaire TARGET = kylin-background-upgrade avec des unités kylin-background-upgrade-*. Elle constitue probablement une version antérieure du même logiciel (1.3.2.10 ici). [reclassé NON COUVERT : analogie sans indice dans un fichier]
- [CONSTATÉ] Non installé par défaut par l'ISO 3.0.

### `kylin-update-notify`

- **Nature** : propre-kylin — **Amont** : Kylin (kylin@kylinos.cn). Format 3.0 (quilt) sans tarball orig (meta : amont-inconnu). Les patches debian/patches/0002-fix.patch (296 Ko) créent l'arbre complet kylin-update-notify/ et kylin-update-notify-immutable/ ; 0001 supprime 30 fichiers ; 0003 modifie updatehandle.cpp et updatewidget.cpp.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète ; l'arbre semble déjà dans l'état patché, PROBABLE). Gitee PRESENT mais non acquis localement.
- **Langages / taille** : C++/Qt5 (144 Ko) + en-têtes (27 Ko), Python (checkUpdate.py), qmake. Le libellé 'TypeScript' correspond aux traductions .ts. — 98 fichiers / 707 Ko, dont 427 Ko de patches ; 28 fichiers de code, ~5 300 lignes (deux variantes quasi identiques) — lignes de code propres mesurées : 5 309
- **Privilège** : Session utilisateur : unités systemd user activées globalement et autostart. Les scripts mainteneur s'exécutent en root.
- **Déclencheurs** : timers systemd user (silent : 30 min puis toutes les 2 h ; manul : toutes les 5 min 10 s, ou 10 min en immutable), autostart de session UKUI, postinst (systemctl --global enable)
- **Indices réseau** : aucun listener réseau (CONSTATÉ); vérification des mises à jour probablement déléguée au démon kylin-system-updater (Depends ; /usr/bin/kylin-background-upgrade dans sa liste blanche) (PROBABLE); Build-Depends libqapt-dev (CONSTATÉ)
- **Blobs** : 2 checkUpdate.pyc de 897 octets (magic 03f30d0a, Python 2.7), non installés ; .qm et PNG
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Petit composant de session (~5 k lignes en deux variantes quasi identiques) qui déclenche périodiquement la chaîne de mise à jour via IPC.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/kylin-update-notify.install` | /usr/bin/kylin-background-upgrade ; /usr/share/kylin-update-notify/checkUpdate.py ; 4 unités user (silent et manul, .service et .timer) -> /usr/lib/systemd/user/ ; 2 .desktop -> /etc/xdg/autostart ; gschema ; template.conf -> /var/lib/kylin-background-upgrade/ | CONSTATÉ |
| debian-install | `archive:debian/kylin-update-notify-immutable.install` | même contenu sans les unités silent | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-update-notify.postinst` | systemctl --user daemon-reload ; systemctl --global enable kylin-background-upgrade-silent.timer et kylin-background-upgrade-manul.timer | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-update-notify-immutable.postinst` | systemctl --global enable kylin-background-upgrade-manul.timer | CONSTATÉ |
| systemd-user-timer | `archive:kylin-update-notify/data/kylin-background-upgrade-silent.service ; .timer` | ExecStart=/usr/bin/kylin-background-upgrade ; Type=forking ; TimeoutStartSec=infinity ; timer OnStartupSec=30min, OnUnitInactiveSec=2h ; WantedBy=default.target | CONSTATÉ |
| systemd-user-timer | `archive:kylin-update-notify/data/kylin-background-upgrade-manul.service ; .timer (+ variante immutable)` | ExecStart=/usr/bin/kylin-background-upgrade --check-upgrade ; timer OnStartupSec et OnUnitInactiveSec=5min 10s (immutable : 10min) | CONSTATÉ |
| autostart | `archive:kylin-update-notify/data/kylin-background-upgrade.desktop` | Exec=/usr/bin/kylin-background-upgrade --check-immediately ; X-UKUI-AutoRestart=true ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Phase=Application | CONSTATÉ |
| autostart | `archive:kylin-update-notify/kylin-update-notify.desktop` | Exec=/usr/bin/kylin-update-notify --check-upgrade, binaire qu'aucun .install ne fournit (TARGET qmake = kylin-background-upgrade) | CONSTATÉ |
| debian-imbrique | `archive:kylin-update-notify-immutable/debian/{control,rules,install,postinst,postrm}` | sous-arbre debian/ imbriqué que le debian/ racine n'utilise pas ; son postinst active aussi silent.timer | CONSTATÉ |
| build-qmake | `archive:kylin-update-notify.pro` | SUBDIRS kylin-update-notify et kylin-update-notify-immutable ; TARGET = kylin-background-upgrade dans les deux .pro | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0. Conflicts et Breaks avec kylin-background-upgrade.
- [CONSTATÉ] Format quilt sans orig : le delta réel par rapport à un amont est inconnu, et les patches recréent la quasi-totalité de l'arbre.

### `kylin-update-desktop-config`

- **Nature** : meta-ou-donnees — **Amont** : openKylin Release SIG (release@lists.openkylin.top). Format 3.0 (quilt) sans orig (meta : amont-inconnu), 21 patches.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (données complètes ; .pc/ présent, patches appliqués). Gitee non acquis localement.
- **Langages / taille** : JSON (88 Ko), YAML, Makefile — 41 fichiers (hors .pc) / 692 Ko, dont 610 Ko de patches — lignes de code propres mesurées : 0
- **Privilège** : Données lues par le démon root de mise à jour (PROBABLE). Le postinst s'exécute en root.
- **Déclencheurs** : installation du paquet (postinst), lecture par kylin-system-updater lors d'une mise à jour (PROBABLE)
- **Indices réseau** : —
- **Blobs** : icon/kylin-update-desktop-system.png uniquement
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Données de configuration sans code. Triage des listes JSON (paquets imposés ou retirés, auto-mise à jour) pour alimenter l'audit de kylin-system-updater.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/install` | config/ et icon/ -> /usr/share/kylin-update-desktop-config/ | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | si /usr/share/ukui-control-center/upgrade/need-reboot.conf existe : y ajoute 'kylin-update-desktop-system' | CONSTATÉ |
| build | `archive:Makefile` | cp ./template/${ARCH}/* ./config/ (ARCH = dpkg-architecture -qDEB_HOST_ARCH) | CONSTATÉ |
| donnees-mise-a-jour | `archive:template/amd64/kylin-update-desktop-system.json (+ arm64, loong64, riscv64)` | package kylin-update-desktop-system, version 3171.F260001 (amd64) ; install_list de 272 paquets ; remove_white_list de 5 ; silent_install_list et silent_upgrade_list vides ; update_self_upgrade = kylin-system-updater, kylin-update-frontend, aptdaemon | CONSTATÉ |
| debian-patches | `archive:debian/patches/series` | 21 patches : versions et ID de release jusqu'à 20260918, liste blanche, loong64 | CONSTATÉ |
| consommateur | `(kylin-system-updater)` | ces JSON pilotent les listes d'installation et de mise à jour du démon root | PROBABLE |

- [CONSTATÉ] Le config/kylin-update-desktop-system.json de l'arbre (version 33797.0001, install_list de 349, upgrade_list de 2251, update_self_upgrade incluant kylin-background-upgrade) diffère des templates. Il est remplacé à la construction par le Makefile.
- [CONSTATÉ] Non installé par défaut par l'ISO 3.0. Section non-free/admin.

### `revisions-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (Kylin Desktop Team), paquet binaire kylin-revision-manager. Embarque des paquets tiers précompilés : docker.io 20.10.16-ok1, containerd 1.5.9-ok6, runc 1.1.0-ok1, iptables 1.8.4-ok2, bubblewrap 0.4.0-ok3, debootstrap 1.0.118-ok4, kare 2.0-0ok2update3, libkysdk-* 2.3.1.0. Le script scripts/dpkg-repack est probablement l'outil Debian dpkg-repack.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète du code propre) + 24 .deb précompilés sans source. Gitee PRESENT mais non acquis localement.
- **Langages / taille** : Python (152 Ko, 14 .py), C++/Qt5 (154 Ko + 44 Ko d'en-têtes), Shell (scripts init d'initramfs, plugins PluginManager), YAML, qmake — 214 fichiers / 116 Mo, dont 115 Mo de .deb ; ~11 000 lignes de code propre — lignes de code propres mesurées : 11 082
- **Privilège** : root : démon D-Bus système RevisionsManagerDaemon et oneshot RevisionsManagerSupplement au boot, avant lightdm. Frontend Qt Revision dans la session utilisateur.
- **Déclencheurs** : boot (multi-user.target, avant lightdm ; 2 unités activées par postinst), appels D-Bus système com.kylin.RevisionsManager, autostart de session (Revision result), initramfs de mise à niveau ou de retour arrière (PROBABLE), postinst
- **Indices réseau** : Depends python3-requests ; After=NetworkManager.service (CONSTATÉ); grep d'URL : http://archive.build.openkylin.top/openkylin/, http://archive.kylinos.cn/kylin/KYLIN-ALL, http://archive.kylinos.cn/kylin/partner (PROBABLE); QLocalServer local (CONSTATÉ)
- **Blobs** : 24 .deb précompilés (114,96 Mo) dans daemon/data/depends/{x86_64,aarch64}/ : docker.io (33,2 Mo amd64), containerd (28,1 Mo), runc (4,2 Mo), iptables, libip4tc2, libip6tc2, libxtables12, bubblewrap, debootstrap, kare, libkysdk-qtwidgets, libkysdk-waylandhelper. Installés dans /usr/share/revisions-manager/data/depends.
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Code propre de taille modérée (~11 k lignes) qui porte un service root de migration d'OS (déplacement de /etc, des applications et de la sécurité), avec initramfs et scripts plugins. Les 24 .deb tiers ne sont pas auditables en source (X).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service | `archive:daemon/data/RevisionsManager.service` | Type=dbus ; BusName=com.kylin.RevisionsManager ; ExecStart=/usr/share/revisions-manager/RevisionsManagerDaemon ; pas de User= (root) ; Before=lightdm.service ; After=NetworkManager.service ; Restart=always ; WantedBy=multi-user.target | CONSTATÉ |
| systemd-service | `archive:daemon/data/RevisionsManagerSupplement.service` | Type=oneshot ; ExecStart=/usr/share/revisions-manager/RevisionsManagerSupplement ; Before=lightdm.service ; WantedBy=multi-user.target | CONSTATÉ |
| dbus-policy | `archive:daemon/data/com.kylin.RevisionsManager.conf` | /etc/dbus-1/system.d : policy user=root own=com.kylin.RevisionsManager ; policy context=default allow send_destination (interfaces com.kylin.RevisionsManager.interface, Introspectable, Properties) | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | si /usr/bin/dpkg-repack est absent : cp /usr/share/revisions-manager/RevisionsManager/scripts/dpkg-repack /usr/bin/ ; systemctl enable RevisionsManager.service et RevisionsManagerSupplement.service ; deb-systemd-invoke start ou restart RevisionsManager.service | CONSTATÉ |
| maintainer-script | `archive:debian/preinst ; archive:debian/prerm ; archive:debian/postrm` | py3clean -p kylin-revision-manager ; rm -rf /var/cache/RevisionsManager ; stop du service ; daemon-reload | CONSTATÉ |
| debian-install | `archive:debian/install` | *.service -> /lib/systemd/system ; RevisionsManager.conf -> /etc/RevisionsManager/ ; conf D-Bus -> /etc/dbus-1/system.d ; plymouth -> /usr/share/plymouth/themes/openkylin-logo/ ; RevisionsManagerResult.desktop -> /etc/xdg/autostart ; depends/, initramfs-back/, initramfs-upgrade/ et .etcPolicy.yaml -> /usr/share/revisions-manager/data/ ; daemon/RevisionsManager* -> /usr/share/revisions-manager/ ; Revision -> /usr/bin | CONSTATÉ |
| initramfs | `archive:daemon/data/initramfs-back/init ; archive:daemon/data/initramfs-upgrade/init (+ functions)` | scripts init d'initramfs installés comme données dans /usr/share/revisions-manager/data/. Mode d'intégration au démarrage NON COUVERT. | CONSTATÉ |
| autostart | `archive:daemon/data/RevisionsManagerResult.desktop` | /etc/xdg/autostart : Exec=/usr/bin/Revision result | CONSTATÉ |
| desktop-file | `archive:daemon/data/RevisionsManager.desktop` | Exec=/usr/bin/Revision | CONSTATÉ |
| config | `archive:daemon/data/RevisionsManager.conf` | [switch] moveEtc=True, moveApp=True, convertApp=False, moveSecutiry=True ; logLevel=DEBUG ; backPeriod=15 | CONSTATÉ |
| plugins-scripts | `archive:daemon/RevisionsManager/PluginManager/{conf.yaml,modules/modules-enabled/part-a.yaml,part-b.yaml,script/part-a/*.sh,script/part-b/main.sh}` | plugins de scripts shell exécutés par le démon (contenu NON COUVERT) | CONSTATÉ |
| listener-local | `archive:frontend/single/qtlocalpeer.cpp` | QLocalServer::listen(socketName 'qtsingleapp-...') (l. 103-126) : socket local d'instance unique | CONSTATÉ |
| build | `archive:Makefile ; archive:frontend/upgrade.pro` | qmake frontend : TARGET Revision ; le démon Python est installé tel quel | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0. Dernier changelog : 1.0.3.0-1ok3 nile du 2023-12-12.
- [CONSTATÉ] Le postinst copie dpkg-repack dans /usr/bin hors du suivi dpkg.

#### Constats transverses du lot B04-boutique-updateur

- **[CONSTATÉ]** 6 noms de bus D-Bus système du lot sont possédés par root et ouverts en envoi (send_destination) au contexte default : com.kylin.systemupgrade, com.kylin.UpgradeStrategies, org.kylin.kaiming, org.kaiming.proxy, com.kylin.RevisionsManager et com.kylin.softwarecenter (gitee). — *preuve* : archive:kylin-system-updater/backend/data/com.kylin.systemupgrade.conf et com.kylin.UpgradeStrategies.conf ; archive:kaiming/data/org.kylin.kaiming.conf et org.kaiming.proxy.system.conf ; archive:revisions-manager/daemon/data/com.kylin.RevisionsManager.conf ; gitee:kylin-software-center/backend/aptdaemon/conf/com.kylin.softwarecenter.conf. Extrait commun : '<policy user="root"><allow own="..."/>' puis '<policy context="default"><allow send_destination="..."'
- **[CONSTATÉ]** 6 actions polkit du lot ont allow_any, allow_inactive et allow_active à 'yes', avec une annotation org.freedesktop.policykit.exec.path : /usr/bin/rm, /opt/kaiming-tools/bin/kaiming, /usr/bin/setfattr, /usr/bin/mount, /usr/bin/umount et /opt/kaiming-tools/bin/kaiming-builder. — *preuve* : archive:kylin-installer/uninstall_qt/org.kylin.uninstaller.policy ('<allow_active>yes</allow_active>' + exec.path /usr/bin/rm) ; archive:kaiming/data/org.kylin.kaiming.policy (actions install, unsetkysec, mount, umount) ; archive:kaiming-builder/data/cn.kylin.kaiming-builder.policy
- **[CONSTATÉ]** 7 sources du lot n'ont dans l'archive qu'un squelette dh_make (debian/*.ex, rules '%: dh $@') et aucun code : kylin-software-center, kmre-apk-installer, kylin-software-properties, dpkg-wrapper, kylin-web-app-engine, kaiming-box, kylin-background-upgrade. Deux sont installées par défaut par l'ISO (kylin-software-center avec 3 plugins, et kylin-software-properties) sans aucune source correspondante. — *preuve* : archive:<paquet>/debian/rules = '%:\n\tdh $@' ; archive:<paquet>/debian/{postinst.ex,prerm.ex,manpage.xml.ex} ; installe_par_defaut_iso de la fiche
- **[CONSTATÉ]** Le drapeau automatique meta.source_coquille_vide vaut 0 pour dpkg-wrapper et kaiming-box alors que leur arbre ne contient que le squelette dh_make : l'indicateur de la fiche n'est pas fiable pour ces deux paquets. — *preuve* : archive:dpkg-wrapper/debian/ et archive:kaiming-box/debian/ (19 fichiers chacun : README, *.ex, rules 'dh $@', source/format '3.0 (native)')
- **[CONSTATÉ]** Côté Gitee : kylin-system-updater et kylin-update-frontend sont identiques à l'archive hors traductions. Gitee kylin-software-center porte une autre source et une autre version (ubuntu-kylin-software-center 4.5.77.1 contre 5.0.9.1 livrée). Gitee kylin-software-properties est lui aussi un squelette (0.0.1-136k2). Les clones Gitee sont superficiels (1 commit, 1 branche). — *preuve* : diff -rq archive/kylin-system-updater gitee/kylin-system-updater -> uniquement 'Only in ... po: *.po' et .gitignore ; gitee:kylin-software-center/debian/changelog 'ubuntu-kylin-software-center (4.5.77.1) jammy' ; gitee:kylin-software-properties/debian/changelog 'kylin-software-properties (0.0.1-136k2) nile'
- **[CONSTATÉ]** Des scripts mainteneur ou des scripts qu'ils appellent modifient la configuration de confiance : aptss ajoute une clé APT dans /etc/apt/trusted.gpg.d ; kaiming écrit un remote ostree avec gpg-verify=false ; kylin-update-frontend-immutable marque libupgrade.so 'verified' dans kysec. — *preuve* : archive:aptss/debian/aptss.postinst 'cp -f /tmp/spark-store-install/spark-store.asc.gpg /etc/apt/trusted.gpg.d/spark-store.gpg' ; archive:kaiming/shells/kaiming_version.sh 'gpg-verify=false' (appelé par debian/kaiming.postinst) ; archive:kylin-update-frontend/debian/kylin-update-frontend-immutable.postinst 'kysec_set -n exectl -v verified $file_path'
- **[CONSTATÉ]** Des binaires précompilés présents dans les arbres sources sont installés tels quels : un ELF x86-64 dans /usr/bin pour kylin-update-frontend, et 24 paquets .deb (docker.io, containerd, runc, iptables...) dans /usr/share/revisions-manager/data/depends. — *preuve* : archive:kylin-update-frontend/debian/kylin-update-frontend.install 'initial/kylin-updatefinish-notify-new /usr/bin' ; archive:revisions-manager/debian/install 'daemon/data/depends /usr/share/revisions-manager/data/'
- **[CONSTATÉ]** Des scripts mainteneur créent hors du suivi dpkg des fichiers ou liens dans des répertoires système : liens kaiming vers /lib/systemd/system, /usr/share/polkit-1/actions et /usr/bin ; copie de libupgrade.so dans ukui-control-center ; copie de dpkg-repack dans /usr/bin ; liens aptss vers /usr/bin et /usr/local/bin ; polkit kaiming-builder. — *preuve* : archive:kaiming/shells/install_files.sh 'ln -s "$lib_path/systemd/system/org.kylin.kaiming.service" ...' ; archive:kylin-update-frontend/debian/kylin-update-frontend.postinst 'cp /usr/lib/upgrade/libupgrade.so ...' ; archive:revisions-manager/debian/postinst 'cp .../scripts/dpkg-repack /usr/bin/' ; archive:aptss/debian/aptss.postinst 'ln -sf /opt/durapps/spark-store/bin/aptss /usr/bin/aptss'
- **[CONSTATÉ]** Des unités systemd sont activées explicitement par postinst : kylin-system-updater.service, kylin-unattended-upgrades.service, org.kylin.kaiming.service, RevisionsManager.service et RevisionsManagerSupplement.service (système, root) ; km-ses-dbusproxy.service et les timers kylin-background-upgrade-silent/manul (utilisateur, activation globale). — *preuve* : archive:kylin-system-updater/debian/kylin-system-updater.postinst 'systemctl enable kylin-system-updater.service' ; archive:kaiming/debian/kaiming.postinst "systemctl enable 'org.kylin.kaiming.service'" ; archive:revisions-manager/debian/postinst 'systemctl enable RevisionsManager.service' ; archive:kylin-update-notify/debian/kylin-update-notify.postinst 'systemctl --global enable kylin-background-upgrade-silent.timer'
- **[CONSTATÉ]** Le démon de mise à jour embarque une liste blanche d'exécutables appelants qui cite 7 binaires d'autres paquets du lot (kylin-background-upgrade, kylin-installer, kylin-uninstaller, kylin-software-properties-service, kylin-software-center, kylin-software-center-plugin-synchrodata, kylin-unattended-upgrade). L'installation et l'usage de ce fichier .limit ne sont pas couverts. — *preuve* : archive:kylin-system-updater/backend/data/com.kylin.systemupgrade.limit '[whitelist] key1 = /usr/bin/kylin-background-upgrade ... key13 = /usr/bin/kylin-software-center-plugin-synchrodata' ; debian/kylin-system-updater.install '#backend/data/com.kylin.systemupgrade.limit /etc/dbus-1/conf/' (commenté)
- **[CONSTATÉ]** 6 des 17 sources du lot sont installées par défaut par l'ISO 3.0 : kylin-software-center, kylin-installer, kylin-software-properties, kaiming, kylin-system-updater et kylin-update-frontend. Les 11 autres ne le sont pas. — *preuve* : fiche B04-boutique-updateur.json, champ installe_par_defaut_iso (ex. 'kylin-system-updater=3.0.0.0-ok56', 'kaiming=1.0.0-ok1k0.96') ; tableau vide pour les autres paquets
- **[CONSTATÉ]** Les seuls listeners détectés dans le lot sont locaux : QLocalServer (instance unique) dans kylin-installer, revisions-manager et kylin-software-center (gitee), et un socket Unix AF_UNIX dans kaiming. Aucun socket TCP/UDP en écoute n'a été relevé par l'extraction automatique. — *preuve* : archive:kylin-installer/qt/qtlocalpeer.cpp l.103 'server = new QLocalServer(this);' ; archive:kaiming/common/KMUnixSocketServer.cpp l.73 'socket(AF_UNIX, SOCK_STREAM, 0)' ; champ listen de la fiche

