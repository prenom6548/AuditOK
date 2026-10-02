# Lot B08-kysdk-biometrie

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kba`

- **Nature** : propre-kylin — **Amont** : Kylin (en-têtes 'Copyright (C) 2022/2024 Tianjin KYLIN Information Technology Co., Ltd.'). debian/copyright non renseigné (Source: <url://example.com>). Mainteneur zhoukangyu@kylinos.cn ; changelog 1.0.0.1-ok9 (nile, 03/07/2025).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, format 3.0 quilt, patchs appliqués : .pc présent) ; pas d'arbre gitee acquis (forges : Gitee/GitHub PRESENT, DIVERGENT)
- **Langages / taille** : C 148 Ko (lib/ : tdes, sha256, sha512, pbkdf2_hmac, json, kba.c), C++ 23 Ko (kpr-backend Qt/DBus, TestSQA), en-têtes C 21 Ko, Python 7 Ko (example/password_recovery.py), qmake, JSON — 82 fichiers, 300 660 octets (arbre) ; source 57 818 octets ; cibles construites (CMakeLists.txt) : libkba.so (lib/), kpr-backend (kpr-backend/), kba-test (example/) — lignes de code propres mesurées : 6 166
- **Privilège** : root (kpr-backend activé par D-Bus système avec User=root) ; libkba.so chargée par les appelants
- **Déclencheurs** : activation D-Bus système à l'appel de org.kylin.KprBackend, installation/mise à jour du paquet (postinst configure)
- **Indices réseau** : aucun listener détecté (champ listen vide); seules URL trouvées : csrc.nist.gov en commentaire de lib/sha256.c et lib/sha512.c
- **Blobs** : aucun blob binaire (blob_count=0)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant propre de petite taille (~200 Ko de code) exposant un service D-Bus root et implémentant localement TDES/SHA/PBKDF2 pour une fonction liée à l'identité (questions de sécurité).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (activation D-Bus système ; classé 'systemd-unit' par l'extraction automatique) | `archive:kpr-backend/org.kylin.KprBackend.service` | [D-BUS Service] Name=org.kylin.KprBackend ; Exec=/usr/bin/kpr-backend ; User=root ; installé dans /usr/share/dbus-1/system-services/ (kpr-backend/CMakeLists.txt, debian/libkba.install) | CONSTATÉ |
| dbus-policy | `archive:kpr-backend/org.kylin.KprBackend.conf` | policy user=root : allow own=org.kylin.KprBackend ; context=default : allow send_destination=org.kylin.KprBackend pour send_interface org.kylin.KprBackend, org.freedesktop.DBus.Introspectable, org.freedesktop.DBus.Properties ; installé dans /etc/dbus-1/system.d/ | CONSTATÉ |
| liste-blanche D-Bus Kylin (.limit) | `archive:conf/org.kylin.KprBackend.limit` | [whitelist] key1..key11 : /usr/bin/kylin-os-installer, /usr/sbin/ukui-greeter, /usr/bin/ukui-screensaver-dialog, /usr/bin/ukui-control-center, polkit-ukui-authentication-agent-1 (7 architectures) ; [auth] RootOnly=false ; copié avec org.kylin.KprBackend.limit.verify (empreinte 40 hex) vers /etc/dbus-1/conf/ et /usr/share/dbus-1/conf/ (debian/libkba.install) | CONSTATÉ |
| maintainer-script postinst | `archive:debian/libkba.postinst` | configure : cp /usr/share/libkba/conf/question.json /etc/kba/question.json ; chmod 644 /etc/kba/question.json ; #DEBHELPER# présent | CONSTATÉ |
| maintainer-script prerm | `archive:debian/libkba.prerm` | aucune action (branches remove/upgrade/deconfigure/failed-upgrade vides) | CONSTATÉ |
| debian-install-lists | `archive:debian/libkba.install` | usr/bin/kpr-backend, usr/lib/libkba.so, usr/share/libkba/conf/*, politique D-Bus, fichiers .limit/.limit.verify, .mo ; debian/kba-test.install : usr/sbin/kba-test ; debian/libkba.dirs : etc/kba | CONSTATÉ |
| debian-control / rules | `archive:debian/control ; archive:debian/rules` | 3 paquets binaires (libkba, libkba-dev, kba-test) ; rules : msgfmt des .po puis 'dh $@' ; CMakeLists.txt racine force set(CMAKE_BUILD_TYPE Debug) (-O0 -g) | CONSTATÉ |

- [CONSTATÉ] lib/ contient des implémentations cryptographiques maison : tdes.c (50 Ko), sha256.c, sha512.c, pbkdf2_hmac.c, sha.c.
- [PROBABLE] kpr-backend gère des questions/réponses de sécurité (kpr-backend/src/securityquestionanswer.cpp, conf/question.json, example/password_recovery.py) : fonction de récupération de mot de passe.
- [CONSTATÉ] libkba est installé par défaut par l'ISO (libkba=1.0.0.1-ok9) ; kpr-backend et sa politique D-Bus font partie de ce paquet binaire.
- [CONSTATÉ] Restes de développement : TestSQA/TestSQA.pro.user (fichier Qt Creator 22 Ko), squelettes dh_make debian/*.ex, debian/copyright non renseigné.
- [NON COUVERT] Le mécanisme d'application des fichiers .limit/.limit.verify (dbus-daemon modifié par Kylin ?) n'est pas dans ce lot.

### `libboundscheck`

- **Nature** : fork-amont — **Amont** : libboundscheck / bounds_checking_function (securec) de Huawei, distribué par openEuler : en-têtes 'Copyright (c) Huawei Technologies Co., Ltd. 2014-2021', LICENSE MulanPSL v2. Version amont non indiquée (pas de debian/watch ni d'upstream/metadata) ; changelog Kylin 1.0.0-ok1 'Init' (21/10/2024) → 1.0.0-ok4 'Rebuild for huanghe' (25/03/2026).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, 2 patchs : 0001-changelog.patch, 0002-change-Cmakelist.patch ; .pc présent) ; pas de gitee acquis
- **Langages / taille** : C 164 Ko (src/*.c, *.inl), en-têtes C 80 Ko — 57 fichiers, 406 185 octets ; source 71 646 octets ; cible unique : bibliothèque partagée boundscheck (CMakeLists.txt : file(GLOB src/*.c)) — lignes de code propres mesurées : 6 463
- **Privilège** : aucun en propre (bibliothèque chargée dans les processus consommateurs)
- **Déclencheurs** : chargement par les programmes liés à libboundscheck.so
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Bibliothèque amont Huawei/openEuler reconditionnée ; il suffit de comparer l'arbre à l'amont et de lire les 2 patchs debian.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | un paquet binaire libboundscheck (Multi-Arch: same) ; Depends: g++, gcc ; Description générique non renseignée ('Your package description here.') ; pas de debian/copyright | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; export CC=gcc ; export LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all | CONSTATÉ |
| cible de construction | `archive:CMakeLists.txt` | add_library(boundscheck SHARED src/*.c) ; options -fstack-protector-all -D_FORTIFY_SOURCE=2 -O2 + nombreux -W ; installe include/securec.h et securectype.h dans include/boundscheck | CONSTATÉ |

- [CONSTATÉ] meta.origine_kylin=1 alors que les en-têtes de code attribuent le copyright à Huawei Technologies.
- [CONSTATÉ] Installé par défaut par l'ISO (libboundscheck=1.0.0-ok4).
- [NON COUVERT] Version exacte de l'amont openEuler correspondant à l'arbre non déterminée.

### `libkysdk-system`

- **Nature** : propre-kylin — **Amont** : Kylin (SDK système 'kysdk', mainteneur chenzhikai@kylinos.cn ; également présent dans Debian/Ubuntu selon meta). Composants tiers embarqués : JQLibrary (src/libkysdk-system-javascript-http/library/JQLibrary), bibliothèques JS/HTML clientes, jars Java de test.
- **Installé par défaut (ISO 3.0)** : oui (14 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, 51 patchs appliqués : .pc présent) ; pas de gitee acquis
- **Langages / taille** : C 1,90 Mo, JavaScript 1,08 Mo (html/ clientes), C++ 0,75 Mo, en-têtes 0,50 Mo, HTML 0,16 Mo, Java 0,14 Mo, CSS ; Python (src/location/location.py, src/python/kysdk.py) — 1 183 fichiers, 494 812 946 octets (dont debian/patches/0051-update-changelog.patch = 462 179 840 octets) ; source 135 033 660 octets ; 33 sous-répertoires construits (src/CMakeLists.txt) produisant ~30 bibliothèques libky*.so et les exécutables kySdkDbus, kySdkDbus-session, kysdk-net, systime, sdk-date, kysdk-logrotate, sdk-powermanager, httpServer, kysdk-dbus-websocket, kypackage_* (8), kylin-sysinfo — lignes de code propres mesurées : 140 434
- **Privilège** : root : kySdkDbus, kysdk-net, systime, kysdk-logrotate (systemd sans User=), location.py (activation D-Bus User=root) ; session utilisateur : kySdkDbus-session, sdk-date, sdk-powermanager, httpServer, kysdk-dbus-websocket ; outils kypackage_* invoqués par les appelants
- **Déclencheurs** : démarrage multi-user.target (kysdk-dbus, kysdk-systime, kysdk-logrotate activés par postinst ; kysdk-net si activé), activation D-Bus système (com.kysdk.location) et session (PowerManagerServer), ouverture de session graphique (autostart /etc/xdg/autostart), cron quotidien 09:30 (logrotate, si libkysdk-logrotate installé), installation/mise à jour de paquets (postinst ; outils kypackage_*)
- **Indices réseau** : QTcpServer LocalHost:8888 (httpServer, JQHttpServer) — CONSTATÉ src/libkysdk-system-javascript-http/kysdk/widget.cpp l.281; QWebSocketServer NonSecureMode LocalHost:12345 (kysdk-dbus-websocket) — CONSTATÉ src/libkysdk-system-javascript-websocket/main.cpp l.57-58; URL dans src/location/location.py (service root) : api.ip.sb, api.ipify.org, api.map.baidu.com, ifconfig.me, ipinfo.io; URL www.baidu.com dans src/search/libkysearch.c
- **Blobs** : 21 blobs : 20 .jar sous src/kysdk-java/ (dont src/kysdk-java/kysdk-java-ext/kysdk-java/target/kysdk-java-1.0.0.jar installé tel quel dans /usr/share/java par debian/libkysdk-system-java.install ; test.jar 5 Mo, dbus-java-3.3.2, jnr-*, jffi-1.3.9-native, asm-9.2, slf4j…) + 1 objet ELF src/tests/tests_packages/libkypackages.o. Le patch debian/patches/0051-update-changelog.patch (462 Mo) supprime 1 102 fichiers dont src/kdkocr/libs/{amd64,arm64,mips64el,sw64,loongarch64,i386}/* et src/kdkocr/models/{det,rec,cls} (bibliothèques Paddle précompilées et modèles) ; l'arbre patché utilise Tesseract (src/kdkocr/CMakeLists.txt).
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant volumineux (~1 200 fichiers) mais qui expose 5 noms D-Bus système root, un service de géolocalisation réseau, des serveurs HTTP/WebSocket locaux et des outils d'installation de paquets : ces sous-modules sont à lire en L3, le reste en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:src/kysdkdbus/kysdk-dbus.service` | Type=dbus ; BusName=com.kylin.kysdk.service ; ExecStart=/usr/bin/kySdkDbus ; pas de User= (root) ; Restart=always ; WantedBy=multi-user.target ; installé dans lib/systemd/system par debian/libkysdk-system-dbus.install | CONSTATÉ |
| dbus-policy | `archive:src/kysdkdbus/com.kylin.kysdk.service.conf` | policy user=root : own com.kylin.kysdk.service ; context=default : allow send_destination=com.kylin.kysdk.service sans restriction d'interface | CONSTATÉ |
| maintainer-script postinst | `archive:debian/libkysdk-system-dbus.postinst` | systemctl daemon-reload ; systemctl unmask kysdk-dbus.service ; systemctl enable kysdk-dbus.service ; systemctl restart kysdk-dbus.service ; pas de #DEBHELPER# | CONSTATÉ |
| autostart .desktop | `archive:src/kysdkdbus/kysdk-dbus-session.desktop` | Exec=/usr/bin/kySdkDbus-session ; NoDisplay=true ; installé dans /etc/xdg/autostart par debian/libkysdk-system-dbus.install | CONSTATÉ |
| systemd-unit | `archive:src/net/dbus/kysdk-net.service` | Type=dbus ; BusName=com.kylin.kysdk.Net ; ExecStart=/usr/bin/kysdk-net ; pas de User= (root) ; WantedBy=multi-user.target ; installé par debian/libkysdk-net.install ; postinst libkysdk-net le redémarre seulement s'il est actif (pas d'enable, pas de #DEBHELPER#) | CONSTATÉ |
| dbus-policy | `archive:src/net/dbus/com.kylin.kysdk.Net.conf` | user=root own com.kylin.kysdk.Net ; context=default allow send_destination=com.kylin.kysdk.Net sans restriction d'interface | CONSTATÉ |
| dbus-service (activation système) | `archive:src/location/com.kysdk.location.service` | Name=com.kysdk.location ; Exec=/usr/bin/python3 /usr/bin/location.py ; User=root ; installé dans usr/share/dbus-1/system-services par debian/libkysdk-location.install | CONSTATÉ |
| dbus-policy | `archive:src/location/com.kysdk.location.conf` | user=root own com.kysdk.location ; context=default allow send_destination=com.kysdk.location sans restriction d'interface | CONSTATÉ |
| systemd-unit (non installé) | `archive:src/location/dbus_location.service` | Description=KYLIN DBUS FOR POLICE ; Type=dbus ; BusName=com.kysdk.location ; ExecStart=/usr/bin/python3 /usr/bin/location.py ; absent de debian/libkysdk-location.install ; debian/libkysdk-location.postinst le redémarre s'il est actif | CONSTATÉ |
| systemd-unit | `archive:src/systemtime/kysdk-systime.service` | Type=dbus ; BusName=com.kylin.kysdk.TimeServer ; ExecStart=/usr/bin/systime ; After=lightdm.service ; pas de User= (root) ; WantedBy=multi-user.target | CONSTATÉ |
| dbus-policy | `archive:src/systemtime/com.kylin.kysdk.TimeServer.conf` | user=root own com.kylin.kysdk.TimeServer ; context=default allow send_destination sans restriction d'interface | CONSTATÉ |
| maintainer-script postinst | `archive:debian/libkysdk-systime.postinst` | si systemd-detect-virt != docker : daemon-reload, systemctl enable + restart kysdk-systime.service ; renommage de .mo ; pas de #DEBHELPER# | CONSTATÉ |
| autostart .desktop | `archive:src/systemtime/kysdk-systime-date.desktop` | Exec=/usr/bin/sdk-date ; installé dans /etc/xdg/autostart (debian/libkysdk-systime.install) | CONSTATÉ |
| systemd-unit | `archive:src/logrotate/kysdk-logrotate.service` | Type=dbus ; BusName=com.kysdk.base ; ExecStart=/usr/bin/kysdk-logrotate ; pas de User= (root) ; paquet libkysdk-logrotate (non installé par défaut par l'ISO) | CONSTATÉ |
| dbus-policy | `archive:src/logrotate/com.kysdk.base.conf` | user=root own com.kysdk.base ; context=default allow send_destination sans restriction d'interface | CONSTATÉ |
| maintainer-script postinst + cron | `archive:debian/libkysdk-logrotate.postinst ; archive:src/logrotate/logrotate.cron` | postinst : systemctl enable/restart kysdk-logrotate.service ; 'crontab /usr/lib/kysdk/kysdk-system/logrotate.cron' (exécuté en root, crontab(1) remplace la table de l'utilisateur appelant) ; cron : '30 9 * * * /usr/sbin/logrotate -f /etc/logrotate.d/kylog-rotate/*' | CONSTATÉ |
| dbus-service (session) | `archive:src/powermanagement/com.kylin.kysdk.PowerManagerServer.service` | Name=com.kylin.kysdk.PowerManagerServer ; Exec=/usr/bin/sdk-powermanager ; installé dans usr/share/dbus-1/services (bus de session) ; src/powermanagement/kysdk-powermanager.desktop n'est listé dans aucun .install | CONSTATÉ |
| listener TCP + autostart | `archive:src/libkysdk-system-javascript-http/kysdk/widget.cpp ; archive:src/libkysdk-system-javascript-http/conf/kysdkHttpServer.desktop` | l.277-282 : JQHttpServer::TcpServerManage(50).listen(QHostAddress::LocalHost, port) ; port lu dans /etc/kysdk/kysdk.conf (conf/kysdk.conf : port=8888) ; lancé par autostart /etc/xdg/autostart (Exec=/usr/bin/httpServer) ; paquet libkysdk-system-javascript-http non installé par défaut | CONSTATÉ |
| systemd-unit (non installé) | `archive:src/libkysdk-system-javascript-http/conf/kysdkHttpServer.service` | Type=simple ; User=root ; ExecStart=/usr/bin/httpServer ; WantedBy=multi-user.target ; absent de debian/libkysdk-system-javascript-http.install (seul le .desktop autostart est installé) | CONSTATÉ |
| listener WebSocket + autostart | `archive:src/libkysdk-system-javascript-websocket/main.cpp ; archive:src/libkysdk-system-javascript-websocket/kysdk-dbus-websocket.desktop` | l.45-58 : QWebSocketServer(NonSecureMode).listen(QHostAddress::LocalHost, port), port par défaut 12345 lu dans /etc/websocket/kysdk-dbus-websocket.conf ; autostart Exec=/usr/bin/kysdk-dbus-websocket ; paquet non installé par défaut | CONSTATÉ |
| polkit | `archive:src/accounts/org.kylin.kysdk.accounts.policy` | action org.kylin.kysdk.chpasswd : allow_any/allow_inactive/allow_active=auth_self ; paquet libkysdk-accounts (non installé par défaut) | CONSTATÉ |
| chemin d'installation de paquets | `archive:debian/libkysdk-package.install` | installe /usr/bin/kypackage_{inst,list,remove,unpack,configure,check,general,desktop}, /usr/bin/py-version-check.py, /etc/kylin-config/basic/libkysdk-package.yaml (installation: pkg_sign_check=true, privilege_runtime_check=true, whitelist_runtime_check=false, direct_install_whitelist…), listes bwlist/udeblist/unwlist dans /var/lib/kysdk/kysdk-system/kypackage/ ; libkysdk-package installé par défaut | CONSTATÉ |
| ld.so.conf.d | `archive:debian/libkysdk-systemcommon.install` | development-files/kysdk-system.conf → /etc/ld.so.conf.d | CONSTATÉ |
| maintainer-scripts postinst (bibliothèques) | `archive:debian/libkysdk-{accounts,disk,hardware,proc,realtime,sysinfo,net}.postinst` | systemctl restart kysdk-dbus.service si actif ; hardware/sysinfo : mv de fichiers .mo ; libkysdk-ocr.postinst vide (#DEBHELPER# seul) ; les 11 autres postinst n'ont pas de #DEBHELPER# | CONSTATÉ |
| debian-rules | `archive:debian/rules ; archive:CMakeLists.txt` | rules : 'dh $@' simple (debhelper-compat 12) ; CMakeLists.txt racine : set(CMAKE_BUILD_TYPE Debug), link_directories(/usr/lib/kysdk/kysdk-security) | CONSTATÉ |

- [CONSTATÉ] Paquets installés par défaut par l'ISO : disk, filesystem, hardware, location, net, ocr, package, powermanagement, proc, realtime, sysinfo, sysinfo-data, system-dbus, systime. Ne sont pas installés : javascript-http, javascript-websocket, logrotate, accounts, java, python.
- [CONSTATÉ] Les 5 politiques D-Bus système du paquet (com.kylin.kysdk.service, com.kylin.kysdk.Net, com.kysdk.location, com.kylin.kysdk.TimeServer, com.kysdk.base) sont des copies d'un même modèle (commentaire 'alsa service', ligne commentée com.jd.ccis) et autorisent send_destination pour context=default sans filtrer l'interface.
- [PROBABLE] location.py, lancé en root par activation D-Bus, interroge des services externes de géolocalisation par IP (déduit des URL présentes).
- [NON COUVERT] La conversion de kysdk-dbus/kysdk-net/systime au comportement debhelper (pas de #DEBHELPER# dans les postinst) n'a pas été vérifiée sur les .deb construits.
- [NON COUVERT] Contenu détaillé des 51 patchs quilt (hors liste des fichiers du patch 0051) non examiné.

### `libkysdk-applications`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitee.com/openkylin/libkysdk-applications.git ; debian/watch vers les releases Gitee openkylin ; présent dans Debian/Ubuntu)
- **Installé par défaut (ISO 3.0)** : oui (7 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, patchs appliqués : .pc présent) ; pas de gitee acquis
- **Langages / taille** : C++ 1,98 Mo, en-têtes 0,49 Mo, XML 0,13 Mo, 'TypeScript' 0,13 Mo (fichiers .ts de traduction Qt), qmake 90 Ko, C 2 Ko — 810 fichiers, 6 708 917 octets ; source 2 605 880 octets ; cibles (kysdk-application.pro) : kysdk-appconf2, kysdk-waylandhelper (+ kysdk-ukuiwindowhelper), kysdk-widgetutils, kysdk-qtwidgets, kysdk-kabase/kabase, kysdk-alm, kysdk-ukenv — lignes de code propres mesurées : 82 699
- **Privilège** : utilisateur (bibliothèques chargées dans les applications de session)
- **Déclencheurs** : chargement par les applications UKUI/Kylin liées à ces bibliothèques
- **Indices réseau** : aucun listener TCP ; QLocalServer seulement (socket Unix); URL blog.csdn.net en commentaire de kysdk-kabase/kabase/kylin_image_codec/image_load/image_load.cpp
- **Blobs** : aucun blob binaire ; images de test (kysdk-qtwidgets/test/testloopplaypage/*.jpg, ~1,8 Mo)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Bibliothèques de session volumineuses (~2,5 Mo de C++) majoritairement graphiques ; il faut cibler kabase (buried_point, application_access, dbus, system_information), alm/ukenv et l'IPC local.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install-lists | `archive:debian/libkysdk-*.install (15 fichiers)` | uniquement des bibliothèques usr/lib/*/libkysdk-{alm,appconf2,kabase,qtwidgets,ukenv,waylandhelper,ukuiwindowhelper,widgetutils}.so.* et fichiers -dev ; aucun service, aucune unité, aucun script mainteneur | CONSTATÉ |
| debian-rules | `archive:debian/rules` | QT_SELECT=5 ; DEB_BUILD_MAINT_OPTIONS=hardening=+all ; dh $@ | CONSTATÉ |
| socket local (single-instance) | `archive:kysdk-kabase/kabase/single_application/local_peer.cpp ; archive:kysdk-alm/src/singleapplication/localpeer.cpp` | new QLocalServer ; server->listen(socketName) (l.109-130 / l.108-129) : socket Unix local, pas TCP | CONSTATÉ |
| cible de construction | `archive:kysdk-kabase/kabase/kabase.pro` | compile buried_point.cpp/.hpp (埋点 = point de collecte d'événements), application_access.cpp, dbus.cpp, kylin_system/system_information.cpp | CONSTATÉ |

- [CONSTATÉ] Installés par défaut par l'ISO : libkysdk-alm, appconf2, kabase, qtwidgets, ukenv, waylandhelper, widgetutils.
- [PROBABLE] kabase/buried_point.cpp correspond probablement à un mécanisme de télémétrie / collecte d'événements applicatifs (sens du nom 埋点).
- [CONSTATÉ] kysdk-application.pro copie pre-commit dans .git/hooks en configuration debug uniquement (system(cp …)).

### `libkysdk-desktop`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitlab2.kylin.com/kylinos-src/libkysdk-desktop.git, forge interne ; changelog 3.0.1.0-ok0.3update1 'sync v11 3.0.1.0-0k1.4')
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, .pc présent) ; pas de gitee acquis
- **Langages / taille** : C++ 113 Ko, en-têtes 57 Ko, XML 41 Ko, qmake 17 Ko, Python 4 Ko (tests/convert_lcov2xml.py), Shell 2 Ko — 112 fichiers, 400 724 octets ; source 67 624 octets ; cibles (kysdk-desktop.pro) : kysdk-appmanager, kysdk-soundeffects, kysdk-notification, kysdk-shortcut (+ exécutable kdkshortcut), kysdk-thememanager, kysdk-watermark — lignes de code propres mesurées : 5 809
- **Privilège** : utilisateur (bibliothèques de session ; kdkshortcut en autostart de session)
- **Déclencheurs** : ouverture de session (kdkshortcut, si libkysdk-shortcut installé), chargement par les applications
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petites bibliothèques de session sans réseau ni privilège ; seul libkysdk-soundeffects est installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart .desktop | `archive:kysdk-shortcut/configs/kysdk-shortcut.desktop` | Exec=/usr/bin/kdkshortcut ; NoDisplay=true ; X-GNOME-Autostart-Phase=Initialization ; X-KDE-autostart-phase=1 ; installé dans etc/xdg/autostart par debian/libkysdk-shortcut.install | CONSTATÉ |
| gsettings schema | `archive:kysdk-shortcut/configs/com.kylin.kysdk.keybinding.gschema.xml` | installé dans usr/share/glib-2.0/schemas (debian/libkysdk-shortcut.install) | CONSTATÉ |
| ld.so.conf.d | `archive:debian/libkysdk-desktopcommon.install` | config/kysdk-desktop.conf → /etc/ld.so.conf.d/ | CONSTATÉ |
| debian-install-lists / rules | `archive:debian/*.install ; archive:debian/rules` | autres paquets : bibliothèques seulement ; rules : QT_SELECT=5, hardening=+all, dh $@ ; aucun script mainteneur | CONSTATÉ |

- [CONSTATÉ] Seul libkysdk-soundeffects est installé par défaut par l'ISO ; libkysdk-shortcut (autostart kdkshortcut) ne l'est pas.
- [PROBABLE] kdkshortcut (kysdk-shortcut/kdkshortcut/main.cpp, libkyshortcut/kglobalshortcut.cpp) gère des raccourcis clavier globaux en session.

### `ukui-clipboard`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitee.com/openkylin/ukui-clipboard.git) ; tiers embarqué : third-party/qtsingleapplication (Qt Solutions)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, .pc présent) ; pas de gitee acquis
- **Langages / taille** : C++ 106 Ko, QML 44 Ko, en-têtes 27 Ko, 'TypeScript' 160 Ko (fichiers .ts de traduction Qt), HTML 101 Ko (documentation qtsingleapplication), XML 12 Ko — 123 fichiers, 932 773 octets ; source 163 533 octets ; cibles CMake : ukui-clipboard, ukui-clipboard-tray (taskTray/), qtsingleapplication — lignes de code propres mesurées : 4 327
- **Privilège** : utilisateur (session UKUI)
- **Déclencheurs** : ouverture de session UKUI (2 autostarts, redémarrage automatique)
- **Indices réseau** : aucun listener TCP ; QLocalServer local seulement; aucune URL relevée hors tiers
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Application de session qui capture en continu le contenu du presse-papiers (historique, OCR optionnel via kysdk-coreai-vision) ; code propre modeste (~150 Ko).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart .desktop | `archive:ukui-clipboard.desktop` | Exec=/usr/bin/ukui-clipboard ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Delay=3 ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ (CMakeLists.txt l.143) | CONSTATÉ |
| autostart .desktop | `archive:taskTray/data/ukui-clipboard-tray.desktop` | Exec=/usr/bin/ukui-clipboard-tray ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ (taskTray/CMakeLists.txt l.71) | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | dh $@ ; un seul paquet binaire ukui-clipboard ; pas de .install ni de script mainteneur | CONSTATÉ |
| accès presse-papiers Wayland | `archive:taskTray/wlr-data-control-unstable-v1.xml` | protocole wlr-data-control utilisé par taskTray/waylandbackendThread.cpp (lecture du presse-papiers) | PROBABLE |
| socket local | `archive:third-party/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer (single-instance, socket Unix local) | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (ukui-clipboard=2.0.0.0-ok0.17).
- [CONSTATÉ] CMakeLists.txt recherche kysdk-coreai-vision (pkg_check_modules sans REQUIRED) pour l'OCR (ocrhandler.cpp).
- [NON COUVERT] Le stockage persistant de l'historique du presse-papiers (data.cpp, clipboarddata.cpp) n'a pas été vérifié.

### `ukui-search`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitee.com/openkylin/ukui-search.git ; présent dans Debian/Ubuntu). Tiers embarqués : perftools/ (sous-ensemble de gperftools/tcmalloc, en-têtes 'gperftools Contributors' → libukui-search-tcmalloc), libchinese-segmentation/cppjieba et pinyin4cpp, 3rd-parties/qtsingleapplication
- **Installé par défaut (ISO 3.0)** : oui (8 binaire(s)) — **Code disponible** : archive (source complète, 3.0 quilt, .pc présent) ; pas de gitee acquis
- **Langages / taille** : C++ 2,47 Mo, en-têtes 0,78 Mo, 'TypeScript' 1,2 Mo (.ts de traduction Qt), HTML 0,1 Mo, QML 13 Ko, Shell 13 Ko ; dictionnaires (~25 Mo dans libchinese-segmentation/dict et pinyin4cpp/dict) — 867 fichiers, 34 008 599 octets ; source 10 360 077 octets ; ~339 fichiers .cpp/.h/.qml propres ; sous-projets (CMakeLists.txt) : qtsingleapplication, perftools, libchinese-segmentation, libsearch, frontend, ukuisearch-systemdbus, search-ukcc-plugin, ukui-search-service, ukui-search-app-data-service, ukui-search-service-dir-manager — lignes de code propres mesurées : 93 326
- **Privilège** : root (module libinotify-config-service.so chargé par register-ukui-system-service, binaire hors lot, User=root) ; utilisateur (services d'indexation et frontend en session)
- **Déclencheurs** : ouverture de session UKUI (4 autostarts avec redémarrage automatique), activation D-Bus système (com.ukui.search.qt.systemdbus) et session, modifications du système de fichiers (indexation inotify), installation (glib-compile-schemas)
- **Indices réseau** : aucun listener TCP ; QLocalServer (3rd-parties/qtsingleapplication); URL de recherche web dans libsearch/websearch/web-search-plugin.cpp : baidu.com, www.so.com, www.sogou.com
- **Blobs** : aucun blob binaire ; gros fichiers de données (dictionnaires jieba/pinyin, jusqu'à 8,5 Mo)
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant volumineux qui indexe les fichiers et le contenu de l'utilisateur en session et expose un module D-Bus système root (inotify-config, à lire en L3) ; les tiers (gperftools, cppjieba) relèvent du triage.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (activation système ; classé 'systemd-unit' par l'extraction) | `archive:ukuisearch-systemdbus/conf/com.ukui.search.qt.systemdbus.service` | Name=com.ukui.search.qt.systemdbus ; Exec=/usr/bin/register-ukui-system-service /usr/libexec/ukui-service/system/libinotify-config-service.so ; User=root | CONSTATÉ |
| dbus-policy | `archive:ukuisearch-systemdbus/conf/com.ukui.search.qt.systemdbus.conf` | user=root own com.ukui.search.qt.systemdbus ; context=default allow send_destination pour send_interface com.ukui.search.interface, Introspectable, Properties | CONSTATÉ |
| module chargé en root | `archive:ukuisearch-systemdbus/CMakeLists.txt` | add_library(inotify-config-service MODULE inotify-config*.cpp) → /usr/libexec/ukui-service/system ; installe aussi com.ukui.search.qt.systemdbus.yaml dans /etc/kylin-config/basic/ (fichier absent de debian/ukui-search-systemdbus.install) | CONSTATÉ |
| dbus-service (session) | `archive:ukui-search-app-data-service/conf/com.ukui.search.appdb.service ; archive:ukui-search-service-dir-manager/conf/com.ukui.search.fileindex.service ; archive:frontend/search-app-widget-plugin/provider/org.ukui.appwidget.provider.search.service` | Exec=/usr/bin/ukui-search-app-data-service ; /usr/bin/ukui-search-service-dir-manager ; /usr/bin/ukui-search ; installés dans usr/share/dbus-1/services | CONSTATÉ |
| autostart .desktop | `archive:data/ukui-search.desktop ; data/ukui-search-service.desktop ; data/ukui-search-service-dir-manager.desktop ; data/ukui-search-app-data-service.desktop` | Exec respectifs /usr/bin/ukui-search, ukui-search-service, ukui-search-service-dir-manager, ukui-search-app-data-service ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; installés dans etc/xdg/autostart | CONSTATÉ |
| maintainer-script postinst | `archive:debian/ukui-search-service.postinst` | glib-compile-schemas /usr/share/glib-2.0/schemas/ ; pas de #DEBHELPER# | CONSTATÉ |
| plugin ukui-control-center | `archive:debian/ukui-search.install` | usr/lib/*/ukui-control-center/* (search-ukcc-plugin chargé dans le panneau de configuration) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake ; -DCMAKE_BUILD_TYPE=RelWithDebInfo | CONSTATÉ |

- [CONSTATÉ] Installés par défaut par l'ISO : ukui-search, ukui-search-service, ukui-search-systemdbus, libukui-search2, libukui-search-tcmalloc0, libchinese-segmentation1/-common, libukui-search-common.
- [CONSTATÉ] Le binaire register-ukui-system-service qui charge le module root n'appartient pas à ce lot.
- [NON COUVERT] Version de gperftools embarquée dans perftools/ non identifiée (en-têtes datés 2014-2025).

### `libkylin-chkname`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur mengyuan@kylinos.cn ; version 1.0.10kylin3uk1-0k3, changelog 'nile' 01/04/2025 ; historique 'jammy' 2022)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, autotools, 3.0 quilt, .pc présent) ; pas de gitee acquis
- **Langages / taille** : C 4 Ko (kylin-chkname.c), en-tête 1 Ko ; le reste est de l'outillage autotools généré (configure, ltmain.sh, aclocal.m4) — 79 fichiers, 1 547 237 octets ; source 366 080 octets ; cible : libkylin-chkname.so (Makefile.am) — lignes de code propres mesurées : 11 361
- **Privilège** : aucun en propre (bibliothèque chargée par les outils de création de comptes)
- **Déclencheurs** : appel par les programmes de gestion de comptes (validation des noms d'utilisateur)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Code minuscule (un fichier C de 4 Ko) lié à l'identité (validation des noms de comptes et liste reserved-names) : lecture exhaustive immédiate.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install-lists | `archive:debian/libkylin-chkname1.install` | usr/lib/*/lib*.so.* ; reserved-names → usr/share/kylin-chkname/ ; usr/share/locale | CONSTATÉ |
| debian-control / rules | `archive:debian/control ; archive:debian/rules` | paquets libkylin-chkname1 et libkylin-chkname-dev ('kylin username check') ; rules : dh $@ ; aucun script mainteneur actif (seulement des squelettes *.ex) | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (libkylin-chkname1).
- [CONSTATÉ] Les branches Gitee relevées dans la fiche ne contiennent pas openkylin/huanghe (nile, yangtze, debian/unstable, ubuntu/resolute).
- [NON COUVERT] Consommateurs de la bibliothèque (ukui-control-center, installateur…) non identifiés dans ce lot.

### `biometric-authentication`

- **Nature** : propre-kylin — **Amont** : Kylin (service unit Documentation=http://gogs.kylin.com/KylinBiometric/biometric-authentication ; format 3.0 native ; 0.9.63kord3~rc20-ok23). Dépend de libfprint-2 >= 1.90.1 (configure.ac) pour les pilotes community-multidevice.
- **Installé par défaut (ISO 3.0)** : oui (4 binaire(s)) — **Code disponible** : archive (source complète, natif, autotools) ; pas de gitee acquis
- **Langages / taille** : C 400 Ko, XML 144 Ko (coverage), en-têtes 102 Ko, C++ 61 Ko, qmake 6 Ko ; nombreux .po — 156 fichiers, 2 144 269 octets ; source 284 792 octets ; cibles : biometric-authenticationd, libbiometric.so, biometric-config-tool, biometric-auth-client, biometric-device-discover, pilotes ud650.so et 23 pilotes community-multidevice (*.so) + outils de découverte usb-* — lignes de code propres mesurées : 18 411
- **Privilège** : root (démon biometric-authenticationd ; pilotes .so chargés depuis /usr/lib/biometric-authentication/drivers dans ce démon)
- **Déclencheurs** : démarrage multi-user.target, appels D-Bus système org.ukui.Biometric (greeter, écran de verrouillage, polkit agent, PAM via uniauth), branchement de périphériques USB/série (outils de découverte), installation/suppression de pilotes (postinst/prerm)
- **Indices réseau** : aucun listener ni URL relevé dans ce paquet
- **Blobs** : 2 ELF précompilés : src/drivers/extra/ud650-extra.so.x86_64 (151 576 o) et ud650-extra.so.aarch64 (141 176 o), copiés selon `uname -m` (src/drivers/extra/Makefile.am) et installés par biometric-driver-ud650 dans /usr/lib/biometric-authentication/drivers/extra/
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Démon root D-Bus d'authentification biométrique (~560 Ko de C/C++) qui stocke des gabarits biométriques et charge des pilotes dynamiques : identité et service privilégié.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:data/biometric-authentication.service` | Type=dbus ; BusName=org.ukui.Biometric ; ExecStart=/usr/lib/biometric-authentication/biometric-authenticationd ; pas de User= (root) ; After=systemd-udevd.service, hwfp_init.service ; Restart=always ; WantedBy=multi-user.target | CONSTATÉ |
| init SysV | `archive:data/biometric-authentication` | script /etc/init.d (Makefile.am initddir) ; DAEMON=/usr/lib/biometric-authentication/biometric-authenticationd ; PIDFILE=/tmp/biometric-authentication.pid | CONSTATÉ |
| dbus-policy | `archive:data/org.ukui.Biometric.conf` | user=root own org.ukui.Biometric ; context=default allow send_destination=org.ukui.Biometric sans restriction d'interface (+ Introspectable, Properties) | CONSTATÉ |
| liste-blanche D-Bus Kylin (.limit) | `archive:data/org.ukui.Biometric.limit` | [whitelist] key1..key20 : ukui-greeter, ukui-screensaver-dialog, lightdm, ukui-control-center, ukui-screensaver-backend, polkit-ukui-authentication-agent-1 (5 arch.), uniauth-backend, bioauth, biometric-manager, ukui-biometric-manager, /usr/bin/d-feet, biometric-config-tool, biometric-auth-client, biometric-device-discover, /usr/sbin/box-manager, /usr/bin/hedronagent ; .limit.verify = empreinte 40 hex ; installés dans /etc/dbus-1/conf/ | CONSTATÉ |
| kylin-config | `archive:data/org.ukui.Biometric.yaml` | installé dans /etc/kylin-config/basic/ (data/Makefile.am) | CONSTATÉ |
| polkit | `archive:data/org.ukui.biometric.policy` | 10 actions org.ukui.biometric.{enroll,clean,modify}-{own,admin}-data, {enroll,clean}-{wechat,ukey}-data : *-own/wechat/ukey = auth_self (allow_active=auth_self_keep) ; *-admin = auth_admin (allow_active=auth_admin_keep) | CONSTATÉ |
| polkit (pkexec) | `archive:data/org.freedesktop.policykit.pkexec.biometric-authentication.policy` | action org.freedesktop.policykit.pkexec.biometric-config-tool : auth_admin / allow_active=auth_admin_keep ; exec.path=/usr/bin/biometric-config-tool | CONSTATÉ |
| maintainer-script postinst | `archive:debian/biometric-auth.postinst` | copie biometric-drivers.conf.template → /etc/biometric-auth/biometric-drivers.conf si absent ; deb-systemd-helper unmask puis enable si 'was-enabled' ; update-rc.d si /etc/init.d présent | CONSTATÉ |
| maintainer-script postinst | `archive:debian/biometric-driver-community-multidevice.postinst` | génère /etc/biometric-auth/key/community-multidevice-aes.key (dd /dev/urandom bs=32 \| base64 \| head -c 32 ; chmod 0600) ; biometric-config-tool add-driver -f pour 23 pilotes + set-key AESKey | CONSTATÉ |
| maintainer-script postrm | `archive:debian/biometric-auth.postrm` | purge : rm -rf /etc/biometric-auth/biometric-drivers.conf, /var/lib/biometric-auth/biometric.db, /etc/biometric_auth/extra.conf.d/, /home/*/.biometric_auth/UUID (utilisateurs listés par 'ls /home') ; remove : deb-systemd-helper mask | CONSTATÉ |
| maintainer-scripts pilotes | `archive:debian/biometric-driver-{ud650,a210,r301}.postinst/.prerm ; debian/biometric-driver-community-multidevice.prerm` | biometric-config-tool add-driver / remove-driver ; a210 : set-key Path /dev/ttyUSB0 ; r301 : set-key Path /dev/ttyS0 | CONSTATÉ |
| debian-rules | `archive:debian/rules` | autogen.sh ; --with-bio-db-dir=/var/lib/biometric-auth/ --with-bio-db-name=biometric.db --with-bio-config-dir=/etc/biometric-auth/ --with-bio-driver-dir=/usr/lib/biometric-authentication/drivers ; hardening=+all ; paquets -dbg séparés | CONSTATÉ |

- [CONSTATÉ] Installés par défaut par l'ISO : biometric-auth, biometric-driver-community-multidevice, biometric-utils, libbiometric0 ; biometric-driver-ud650 (porteur des blobs) n'est pas installé par défaut.
- [CONSTATÉ] debian/ contient des .install et scripts pour biometric-driver-a210 et biometric-driver-r301 sans paragraphe Package correspondant dans debian/control (11 paquets déclarés).
- [PROBABLE] Les pilotes chargés par le démon incluent ceux d'autres paquets du lot (fido, wechat) enregistrés via biometric-config-tool.
- [NON COUVERT] Code source des blobs ud650-extra.so absent de l'arbre.

### `ukui-biometric-auth`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (Vcs-Git https://github.com/ukui/ukui-biometric.git ; format 3.0 native ; 4.20.0.0-ok2.11update2)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, natif, CMake) ; pas de gitee acquis
- **Langages / taille** : C++ 568 Ko, 'TypeScript' 615 Ko (.ts de traduction Qt), XML 189 Ko (coverage), en-têtes 124 Ko, C 42 Ko (pam_biometric.c, logger.c, biotest.c), Qt UI 20 Ko, Shell 2 Ko (pam-biometric/utils/*) — 456 fichiers, 3 097 006 octets ; source 594 692 octets ; sous-projets (CMakeLists.txt) : bioauth, bioauth-bin, pam-biometric, polkit-agent, uniauth-backend, images — lignes de code propres mesurées : 22 588
- **Privilège** : root (uniauth-backend ; pam_biometric.so chargé par les processus PAM tels que lightdm, sudo, su, polkit ; scripts pkexec) + utilisateur (agent polkit de session, bioauth)
- **Déclencheurs** : toute authentification passant par la pile PAM gérée par pam-auth-update (Default: yes), activation D-Bus système org.ukui.UniauthBackend, ouverture de session UKUI/MATE (agent polkit), pkexec bioctl-helper / biodrvctl / biorestart, installation (pam-auth-update, modification de configuration)
- **Indices réseau** : aucun listener TCP relevé ; URL seulement en commentaires (stackoverflow.com, www.narf.ssji.net)
- **Blobs** : aucun blob binaire ; tests/kt-test-utils/cpp-stub/elfio.hpp (tiers, 153 Ko)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Module PAM activé par défaut, service D-Bus root, action pkexec allow_active=yes et agent polkit qui manipule des mots de passe : hook PAM/polkit et identité (~730 Ko de code hors tests et ressources).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| pam | `archive:pam-biometric/pam-configs/pam-biometric` | Name: Biometric Verification ; Default: yes ; Priority: 260 ; Auth-Type: Primary ; Auth: [success=end default=ignore] pam_biometric.so ; Conflicts: fprint fprintd ; installé dans /etc/pam-configs | CONSTATÉ |
| pam (module) | `archive:pam-biometric/pam_biometric.c` | add_library(pam_biometric SHARED) installé dans /lib/security (pam-biometric/CMakeLists.txt l.36-41) | CONSTATÉ |
| maintainer-script postinst | `archive:debian/libpam-biometric.postinst` | pam-auth-update --package ; crée ou modifie /etc/biometric-auth/ukui-biometric.conf (General UseFirstDevice=true, MaxFailedTimes=5, EnableAuth=true ; Functions EnableQRCode=true) ; copie org.ukui.UniauthBackend.yaml dans /etc/kylin-config/basic/ | CONSTATÉ |
| maintainer-script prerm | `archive:debian/libpam-biometric.prerm` | pam-auth-update --package --remove pam-biometric | CONSTATÉ |
| dbus-service (activation système) | `archive:uniauth-backend/org.ukui.UniauthBackend.service` | Name=org.ukui.UniauthBackend ; Exec=/usr/bin/uniauth-backend ; User=root | CONSTATÉ |
| dbus-policy | `archive:uniauth-backend/org.ukui.UniauthBackend.conf` | user=root own org.ukui.UniauthBackend ; context=default allow send_destination pour send_interface org.ukui.UniauthBackend, Introspectable, Properties | CONSTATÉ |
| polkit (pkexec) | `archive:pam-biometric/data/org.freedesktop.plicykit.pkexec.bioctl-helper.policy.in` | action org.freedesktop.policykit.pkexec.bioctl : allow_any=no, allow_inactive=no, allow_active=yes ; exec.path=/usr/bin/bioctl-helper (script bash de 201 lignes) | CONSTATÉ |
| polkit (pkexec) | `archive:pam-biometric/data/org.freedesktop.plicykit.pkexec.biodrvctl.policy.in` | action org.freedesktop.policykit.pkexec.biodrvctl : auth_admin / allow_active=auth_admin_keep ; exec.path=/usr/bin/biodrvctl (bash, 42 lignes) | CONSTATÉ |
| polkit (pkexec) | `archive:pam-biometric/data/org.freedesktop.plicykit.pkexec.biorestart.policy.in` | action org.freedesktop.policykit.pkexec.biorestart : auth_admin / allow_active=auth_admin_keep ; exec.path=/usr/bin/biorestart (bash, 8 lignes) | CONSTATÉ |
| autostart .desktop (agent polkit) | `archive:polkit-agent/data/polkit-ukui-authentication-agent-1.desktop.in` | Exec=/usr/lib/<arch>/ukui-polkit/polkit-ukui-authentication-agent-1 ; OnlyShowIn=UKUI;MATE ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg (debian/ukui-polkit.install) | CONSTATÉ |
| debian-install-lists | `archive:debian/libpam-biometric.install ; archive:debian/ukui-polkit.install` | libpam-biometric : /lib/security/*, /usr/bin/* (uniauth-backend, bioauth, bioctl, bioctl-helper, biodrvctl, biorestart), /etc/pam-configs/*, polkit actions, services D-Bus ; ukui-polkit : /etc/xdg/*, /usr/lib/*, /usr/share/applications/* | CONSTATÉ |

- [CONSTATÉ] Installés par défaut par l'ISO : libpam-biometric et ukui-polkit.
- [CONSTATÉ] Seule action polkit du lot dont allow_active vaut 'yes' : org.freedesktop.policykit.pkexec.bioctl (exécute /usr/bin/bioctl-helper en root sans authentification pour une session active).
- [PROBABLE] Priorité 260 dans pam-auth-update : le module est placé avant pam_unix (priorité 256) dans common-auth.
- [CONSTATÉ] Fichiers source des actions polkit nommés 'plicykit' (faute de frappe) ; les noms d'action restent org.freedesktop.policykit.pkexec.*.
- [PROBABLE] uniauth-backend/src contient dbusaccesscontrol.cpp, rsac.cpp, personalizeddata.cpp : contrôle d'accès et chiffrement implémentés dans le service (contenu non lu).

### `ukui-biometric-manager`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (Vcs-Git https://github.com/ukui/ukui-biometric-manager.git ; format 3.0 native ; 4.10.0.0-ok4.15update1)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, natif, qmake) ; pas de gitee acquis
- **Langages / taille** : C++ 782 Ko, 'TypeScript' 1,93 Mo (.ts de traduction Qt), Qt UI 195 Ko, en-têtes 174 Ko, qmake 25 Ko ; images PNG/GIF — 352 fichiers, 5 462 913 octets ; source 1 835 144 octets ; cibles (biometric-manager.pro) : biometric-manager (/usr/bin) et biometric-plugin (plugin ukui-control-center dans $$[QT_INSTALL_LIBS]/ukui-control-center) — lignes de code propres mesurées : 30 104
- **Privilège** : utilisateur (application et plugin chargé dans ukui-control-center) ; agit via les services root org.ukui.Biometric, org.ukui.UniauthBackend et org.kylin.KprBackend
- **Déclencheurs** : lancement manuel (menu) ou ouverture de la page biométrie de ukui-control-center
- **Indices réseau** : aucun listener TCP ; QLocalServer (biometric-manager/qt-solutions/qtsingleapplication); URL github.com dans biometric-manager/src/promptdialog.cpp
- **Blobs** : aucun blob binaire ; images de guide utilisateur (PNG jusqu'à 387 Ko)
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Interface de session volumineuse mais qui pilote l'enrôlement biométrique, le changement de mot de passe et les questions de sécurité via IPC vers des services root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| polkit | `archive:biometric-plugin/conf/org.ukui.biometric.manager.security.questions.policy` | action org.ukui.biometric.manager.security.questions : auth_self / allow_active=auth_self_keep ; installé dans /usr/share/polkit-1/actions/ | CONSTATÉ |
| maintainer-script postinst | `archive:debian/ukui-biometric-manager.postinst` | ln -sf /usr/bin/biometric-manager /usr/bin/ukui-biometric-manager | CONSTATÉ |
| desktop (menu, pas d'autostart) | `archive:biometric-manager/data/biometric-manager.desktop` | Exec=/usr/bin/biometric-manager ; Categories=Qt;System;Utility;Core ; installé dans /usr/share/applications/ | CONSTATÉ |
| plugin ukui-control-center | `archive:biometric-plugin/biometric-plugin.pro` | target.path=$$[QT_INSTALL_LIBS]/ukui-control-center ; sources changeuserpwd.cpp, securityquestionanswer.cpp, qrcodeenroll.cpp, binddialog.cpp, uniauthservice.cpp, pwdlessproxy.cpp | CONSTATÉ |
| debian-rules / install | `archive:debian/rules ; archive:debian/ukui-biometric-manager.install` | rules : make all mode=release, hardening=+all ; .install : biometric-manager/data/biometric-manager → /usr/share/kylin-user-guide/data/guide | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (ukui-biometric-manager=4.10.0.0-ok4.15update1).
- [CONSTATÉ] Fichier résiduel biometric-plugin/biometricswidget.cpp.orig présent dans l'arbre.
- [PROBABLE] Le plugin appelle les services D-Bus root du lot (noms de fichiers uniauthservice.cpp, biometricproxy.cpp, securityquestionanswer.cpp).

### `biometric-driver-fido`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur liuyang@kylinos.cn ; format 3.0 native ; 0.0.1-ok9, changelog nile 18/06/2024). S'appuie sur libfido2 et openssl (src/libfidoukey/Makefile).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, natif, Makefile) ; pas de gitee acquis (forges : IDENTICAL-HEADS)
- **Langages / taille** : C 49 Ko, en-têtes 12 Ko — 25 fichiers, 76 755 octets ; source 15 736 octets ; cibles : src/bio_drv_fidoukey.so (fidoukey_detect.c) et src/libfidoukey/libfidoukey.so (fidoukey_realization.c, fidoukey_internal.c) ; outil usb-bio_fidoukey commenté dans src/Makefile — lignes de code propres mesurées : 1 974
- **Privilège** : root (pilote .so chargé dans biometric-authenticationd)
- **Déclencheurs** : chargement par le démon biometric-authenticationd, branchement d'une clé FIDO/U2F USB, installation (enregistrement du pilote)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Petit pilote propre (~60 Ko) chargé en root qui réalise une authentification par clé FIDO via libfido2/openssl : confiance et identité.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script postinst | `archive:debian/biometric-driver-fido.postinst` | crée /etc/biometric-auth/biometric-drivers.conf si besoin ; configure : biometric-config-tool add-driver -f bio_drv_fidoukey /usr/lib/biometric-authentication/drivers/bio_drv_fidoukey.so | CONSTATÉ |
| maintainer-script preinst | `archive:debian/biometric-driver-fido.preinst` | mkdir -p /usr/lib/biometric-authentication/drivers/extra/fidoukey/ ; pas de #DEBHELPER# | CONSTATÉ |
| maintainer-script prerm | `archive:debian/biometric-driver-fido.prerm` | biometric-config-tool remove-driver -i bio_drv_fidoukey ; pas de #DEBHELPER# | CONSTATÉ |
| debian-install-lists | `archive:debian/biometric-driver-fido.install` | src/bio_drv_fidoukey.so → usr/lib/biometric-authentication/drivers/ ; src/libfidoukey/libfidoukey.so → drivers/extra/fidoukey/ ; .mo zh_CN | CONSTATÉ |
| debian-rules | `archive:debian/rules ; archive:src/Makefile` | override_dh_strip : 'Skip dh_strip stage to keep the symbol table' ; override_dh_shlibdeps vide ; compilation -g avec -Wl,-rpath=/usr/lib/biometric-authentication/drivers/extra/fidoukey,--disable-new-dtags | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (biometric-driver-fido=0.0.1-ok9).
- [CONSTATÉ] Les branches Gitee relevées dans la fiche ne contiennent pas openkylin/huanghe (openkylin/nile, openkylin/yangtze, packaging/openkylin/yangtze, pristine-tar, upstream).
- [PROBABLE] debian/control ne déclare en Build-Depends que debhelper (>= 9) sur la première ligne ; les dépendances de construction réelles (libfido2, openssl, libbiometric) ne sont pas confirmées.

### `biometric-driver-wechat`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur liuyang@kylinos.cn ; 3.0 native ; 1.0.0.1-0k0.5-ok6, changelog nile 08/08/2025) ; tiers embarqué : cJSON (src/libwechat/cJSON.c/.h)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, natif, Makefile) ; pas de gitee acquis
- **Langages / taille** : C 90 Ko (dont cJSON.c 69 Ko), C++ 53 Ko, en-têtes 25 Ko, XML 81 Ko (coverage), qmake 11 Ko (tests) — 53 fichiers, 322 018 octets ; source 57 264 octets ; cibles : src/wechat_driver.so (wechat_driver.c) et src/libwechat/libwechat.so (kywechat.c, ksb_http.c, wechat_realization.cpp, wechat_internal.cpp, cJSON.c) — lignes de code propres mesurées : 5 748
- **Privilège** : root (pilote .so chargé dans biometric-authenticationd) avec accès réseau sortant
- **Déclencheurs** : chargement par biometric-authenticationd, enrôlement / authentification par QR code WeChat (actions polkit org.ukui.biometric.enroll-wechat-data)
- **Indices réseau** : client HTTP libcurl (src/libwechat/ksb_http.c, Makefile : libcurl); URL dans src/libwechat/kywechat.h/.c : open.weixin.qq.com/connect/qrconnect, lp.open.weixin.qq.com/connect/l/qrconnect, api.weixin.qq.com/sns/oauth2/access_token, api.weixin.qq.com/sns/userinfo, www.baidu.com (kywechat.c l.225); constante TOKEN_URL_BASE (kywechat.h l.36) et 2 occurrences du mot 'secret' dans kywechat.h et kywechat.c (valeurs non reproduites)
- **Blobs** : aucun ELF ; images src/extra/network.jpg/png
- **Budget proposé** : L3 — 95 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Petit pilote (~100 Ko hors cJSON) chargé dans un démon root qui fait des requêtes réseau vers les API OAuth de WeChat pour authentifier des comptes : réseau, identité et privilège.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script postinst | `archive:debian/biometric-driver-wechat.postinst` | configure : biometric-config-tool add-driver -f wechat_driver /usr/lib/biometric-authentication/drivers/wechat_driver.so ; pas de #DEBHELPER# | CONSTATÉ |
| maintainer-script preinst / prerm | `archive:debian/biometric-driver-wechat.preinst ; archive:debian/biometric-driver-wechat.prerm` | preinst : mkdir -p /usr/lib/biometric-authentication/drivers/extra/wechat/ ; prerm : biometric-config-tool remove-driver -i wechat_driver | CONSTATÉ |
| debian-install-lists | `archive:debian/biometric-driver-wechat.install` | src/wechat_driver.so → drivers/ ; src/libwechat/libwechat.so → drivers/extra/wechat/ ; src/extra/network.jpg → /var/lib/biometric-auth | CONSTATÉ |
| debian-rules | `archive:debian/rules ; archive:src/libwechat/Makefile` | override_dh_strip (pas de strip) ; override_dh_shlibdeps vide ; libwechat liée à libcurl et opencv4 | CONSTATÉ |

- [CONSTATÉ] Installés par défaut par l'ISO : biometric-driver-wechat et biometric-driver-wechat-common.
- [CONSTATÉ] Les branches Gitee relevées dans la fiche ne contiennent pas openkylin/huanghe (openkylin/nile, nile-sp2, yangtze, upstream).
- [CONSTATÉ] Restes de développement dans l'arbre : .vscode/ (configurationCache.log, dryrun.log, targets.log, settings.json).
- [PROBABLE] Des identifiants d'application WeChat (appid/secret) seraient intégrés en constantes dans le code.

### `smart-interconnection-backend`

- **Nature** : absent-ou-vide — **Amont** : inconnu (LICENSE BSD-3-Clause 'Copyright (c) 2025, sacrex')
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement : dépôt quasi vide (LICENSE + README.md de 33 octets) ; aucune entrée archive ; branche master acquise (1 commit '8d1b754 add LICENSE.') ; têtes V1.0 et v1.0 citées par la fiche non acquises
- **Langages / taille** : aucun code — 2 fichiers, 1 554 octets (gitee) — lignes de code propres mesurées : 0
- **Privilège** : sans objet
- **Déclencheurs** : —
- **Indices réseau** : —
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source sur la branche acquise ; aucun paquet dans l'archive ni dans l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| aucun | `gitee:README.md` | README contient seulement le titre '# smart-interconnection-backend' ; aucun fichier déclaratif ni de construction | CONSTATÉ |
| branches non acquises | `gitee:(têtes V1.0, v1.0)` | contenu des branches V1.0/v1.0 non cloné en phase 0 | NON COUVERT |

- [CONSTATÉ] Absent de l'archive et non installé par l'ISO (installe_par_defaut_iso vide).

### `smart-interconnection-frontend`

- **Nature** : meta-ou-donnees — **Amont** : copie partielle de Xilinx FPGA_as_a_Service v1.2 (k8s-fpga-device-plugin ; main.go 'Portions Copyright 2018 Xilinx Inc.', LICENSE Apache-2.0) ; dépôt Gitee sous BSD-3-Clause 'Copyright (c) 2025, sacrex'
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche master, 2 commits : '8d1b754 add LICENSE.', '50a167f Feature: add k8s Fpga device plugin doc and readme') ; aucune entrée archive ; tête V1.0 non acquise
- **Langages / taille** : Go 2 259 octets (un fichier), Markdown/licences — 9 fichiers, 41 809 octets (gitee) — lignes de code propres mesurées : 84
- **Privilège** : sans objet (non empaqueté) ; un plugin de périphérique k8s s'exécute normalement en DaemonSet privilégié
- **Déclencheurs** : —
- **Indices réseau** : aucun listener relevé par l'extraction ; un plugin de périphérique k8s communique d'ordinaire via un socket gRPC Unix du kubelet
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Un seul fichier Go de 2 Ko copié de Xilinx, non empaqueté dans l'archive ni installé par l'ISO : simple triage et comparaison à l'amont.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| main Go | `gitee:FPGA_as_a_Service-v1.2/k8s-fpga-device-plugin/main.go` | seul fichier de code (plugin de périphérique Kubernetes pour FPGA, d'après README) ; aucun fichier déclaratif (unité, D-Bus, debian/) ni go.mod | CONSTATÉ |
| branche non acquise | `gitee:(tête V1.0)` | contenu de la branche V1.0 non cloné en phase 0 | NON COUVERT |

- [CONSTATÉ] Le dépôt ne contient aucun code 'frontend' au sens du nom ; uniquement un extrait de Xilinx FPGA_as_a_Service.
- [CONSTATÉ] Absent de l'archive et non installé par l'ISO.
- [NON COUVERT] [correction critique] Reclassé meta-ou-donnees (README et extrait Xilinx de 2 Ko) ; la mention d’un DaemonSet privilégié est NON COUVERT.

#### Constats transverses du lot B08-kysdk-biometrie

- **[CONSTATÉ]** 9 noms de bus D-Bus système du lot ne peuvent être détenus que par root (politique user=root own=…) : org.kylin.KprBackend, com.kylin.kysdk.service, com.kylin.kysdk.Net, com.kysdk.location, com.kylin.kysdk.TimeServer, com.kysdk.base, org.ukui.Biometric, org.ukui.UniauthBackend, com.ukui.search.qt.systemdbus. 8 d'entre eux sont fournis par des paquets installés par défaut par l'ISO ; com.kysdk.base (libkysdk-logrotate) ne l'est pas. — *preuve* : kba:kpr-backend/org.kylin.KprBackend.conf ; libkysdk-system:src/{kysdkdbus,net/dbus,location,systemtime,logrotate}/*.conf ; biometric-authentication:data/org.ukui.Biometric.conf ; ukui-biometric-auth:uniauth-backend/org.ukui.UniauthBackend.conf ; ukui-search:ukuisearch-systemdbus/conf/com.ukui.search.qt.systemdbus.conf
- **[CONSTATÉ]** 6 de ces 9 politiques autorisent tout utilisateur (context=default) à envoyer des messages au service sans filtrer l'interface : les 5 de libkysdk-system et org.ukui.Biometric. Les 3 autres (KprBackend, UniauthBackend, ukui-search) limitent l'envoi à l'interface du service plus Introspectable et Properties. — *preuve* : libkysdk-system:src/net/dbus/com.kylin.kysdk.Net.conf : '<policy context="default"> <allow send_destination="com.kylin.kysdk.Net" />' ; biometric-authentication:data/org.ukui.Biometric.conf : '<allow send_destination="org.ukui.Biometric"/>'
- **[CONSTATÉ]** Deux services installent une liste blanche Kylin d'exécutables appelants (*.limit) et un fichier *.limit.verify contenant une empreinte de 40 caractères hexadécimaux dans /etc/dbus-1/conf/. Le composant qui applique ces fichiers n'est pas dans ce lot. — *preuve* : kba:conf/org.kylin.KprBackend.limit ('[whitelist] key1 = /usr/bin/kylin-os-installer … [auth] RootOnly = false') ; biometric-authentication:data/org.ukui.Biometric.limit ('key15=/usr/bin/d-feet … key20=/usr/bin/hedronagent') ; data/Makefile.am 'dbuslimitdir = /etc/dbus-1/conf/'
- **[CONSTATÉ]** Activations à l'installation : des scripts mainteneur activent ou redémarrent des services root sans passer par debhelper. libkysdk-system-dbus : unmask + enable + restart kysdk-dbus ; libkysdk-systime : enable + restart ; libkysdk-logrotate : enable + restart + crontab ; biometric-auth : deb-systemd-helper enable si déjà activé. — *preuve* : libkysdk-system:debian/libkysdk-system-dbus.postinst 'systemctl enable kysdk-dbus.service' ; debian/libkysdk-logrotate.postinst 'crontab /usr/lib/kysdk/kysdk-system/logrotate.cron' ; biometric-authentication:debian/biometric-auth.postinst
- **[CONSTATÉ]** 17 scripts mainteneur du lot ne contiennent pas le jeton #DEBHELPER# : 11 postinst de libkysdk-system, debian/ukui-search-service.postinst, preinst et prerm de biometric-driver-fido, postinst, preinst et prerm de biometric-driver-wechat. — *preuve* : grep -L '#DEBHELPER#' sur libkysdk-system/debian/*.postinst, ukui-search/debian/ukui-search-service.postinst, biometric-driver-{fido,wechat}/debian/*.p*
- **[CONSTATÉ]** Les pilotes biométriques (fido, wechat, community-multidevice, ud650, a210, r301) sont des bibliothèques .so placées dans /usr/lib/biometric-authentication/drivers et enregistrées par 'biometric-config-tool add-driver -f' dans les postinst. biometric-authenticationd, qui s'exécute en root, est le chargeur prévu. — *preuve* : biometric-driver-wechat:debian/biometric-driver-wechat.postinst 'biometric-config-tool add-driver -f ${driver} /usr/lib/biometric-authentication/drivers/wechat_driver.so' ; biometric-authentication:debian/rules '--with-bio-driver-dir=/usr/lib/biometric-authentication/drivers'
- **[CONSTATÉ]** Seule action polkit du lot dont allow_active vaut 'yes' : org.freedesktop.policykit.pkexec.bioctl, qui exécute /usr/bin/bioctl-helper (bash, 201 lignes) sans authentification pour une session active. Toutes les autres actions du lot exigent auth_self ou auth_admin. — *preuve* : ukui-biometric-auth:pam-biometric/data/org.freedesktop.plicykit.pkexec.bioctl-helper.policy.in '<allow_active>yes</allow_active>' ; pam-biometric/CMakeLists.txt l.10 'set(SCRIPTS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/bin)'
- **[CONSTATÉ]** Un module PAM du lot est activé par défaut dans la pile d'authentification commune via pam-auth-update. — *preuve* : ukui-biometric-auth:pam-biometric/pam-configs/pam-biometric 'Default: yes / Priority: 260 / Auth-Type: Primary / [success=end default=ignore] pam_biometric.so' ; debian/libpam-biometric.postinst 'pam-auth-update --package'
- **[CONSTATÉ]** Les seuls listeners TCP du lot appartiennent à libkysdk-system et écoutent sur QHostAddress::LocalHost : httpServer sur le port 8888 et kysdk-dbus-websocket sur le port 12345 (NonSecureMode), tous deux lancés par autostart de session. Leurs paquets (libkysdk-system-javascript-http et -websocket) ne sont pas installés par défaut. Les autres indices 'listen' du lot sont des QLocalServer (sockets Unix locaux pour l'instance unique). — *preuve* : libkysdk-system:src/libkysdk-system-javascript-http/kysdk/widget.cpp l.281 'tcpServerManage.listen(QHostAddress::LocalHost, port)' ; src/libkysdk-system-javascript-websocket/main.cpp l.58 'server.listen(QHostAddress::LocalHost, port)'
- **[CONSTATÉ]** Deux composants installés par défaut contiennent des URL de services externes dans un contexte root : location.py (com.kysdk.location, User=root) avec api.ip.sb, api.ipify.org, api.map.baidu.com, ifconfig.me et ipinfo.io ; libwechat du pilote WeChat (chargé par le démon biométrique) avec api.weixin.qq.com et open.weixin.qq.com via libcurl. — *preuve* : libkysdk-system:src/location/location.py ; libkysdk-system:src/location/com.kysdk.location.service 'User=root' ; biometric-driver-wechat:src/libwechat/kywechat.h l.33-38 ; src/libwechat/Makefile 'PKG_CONFIG_LIBS = … libcurl'
- **[CONSTATÉ]** Binaires précompilés présents dans les arbres du lot : 2 ELF ud650-extra.so (x86_64 et aarch64) installés par biometric-driver-ud650, qui n'est pas installé par défaut ; 20 .jar et 1 .o dans libkysdk-system, dont target/kysdk-java-1.0.0.jar installé tel quel par libkysdk-system-java. Le patch debian 0051 de libkysdk-system (462 Mo) supprime 1 102 fichiers de bibliothèques Paddle précompilées et de modèles OCR présents dans le tarball orig. — *preuve* : biometric-authentication:src/drivers/extra/Makefile.am 'cp ud650-extra.so.${ARCH} ud650-extra.so' ; libkysdk-system:debian/libkysdk-system-java.install 'src/kysdk-java/kysdk-java-ext/kysdk-java/target/kysdk-java-1.0.0.jar usr/share/java' ; debian/patches/0051-update-changelog.patch (--- a/src/kdkocr/libs/amd64/… +++ /dev/null)
- **[CONSTATÉ]** Options de construction de débogage : kba et libkysdk-system forcent set(CMAKE_BUILD_TYPE Debug) dans le CMakeLists.txt racine ; biometric-driver-fido et biometric-driver-wechat désactivent dh_strip ('Skip dh_strip stage to keep the symbol table'). — *preuve* : kba:CMakeLists.txt 'set(CMAKE_BUILD_TYPE Debug)' ; libkysdk-system:CMakeLists.txt 'set(CMAKE_BUILD_TYPE Debug)' ; biometric-driver-{fido,wechat}:debian/rules 'override_dh_strip'
- **[CONSTATÉ]** Fichiers de configuration Kylin déposés dans /etc/kylin-config/basic/ par 4 paquets : libkysdk-package.yaml, com.ukui.search.qt.systemdbus.yaml, org.ukui.Biometric.yaml et org.ukui.UniauthBackend.yaml. — *preuve* : libkysdk-system:debian/libkysdk-package.install ; ukui-search:ukuisearch-systemdbus/CMakeLists.txt l.22 ; biometric-authentication:data/Makefile.am 'conf2dbusdir = /etc/kylin-config/basic/' ; ukui-biometric-auth:debian/libpam-biometric.postinst
- **[CONSTATÉ]** Pour 3 paquets installés par défaut, les branches Gitee relevées dans la fiche ne contiennent pas de tête openkylin/huanghe : libkylin-chkname, biometric-driver-fido et biometric-driver-wechat. — *preuve* : fiche B08-kysdk-biometrie.json, champ forges.gitee_heads (ex. biometric-driver-wechat : ['openkylin/nile','openkylin/nile-sp2','openkylin/yangtze','upstream'])
- **[CONSTATÉ]** Les 13 paquets du lot présents dans l'archive ont leur source complète. Les 2 dépôts smart-interconnection-* n'existent que sur Gitee et ne sont pas installés par l'ISO : l'un est vide, l'autre ne contient qu'un extrait Xilinx de 2 Ko. — *preuve* : fiche JSON : chemin_archive=null et installe_par_defaut_iso=[] pour smart-interconnection-backend/frontend ; gitee:smart-interconnection-backend (LICENSE, README.md)

