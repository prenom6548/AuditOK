# Lot B14-apps-kylin

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-weather`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native, origine_kylin=1) ; Vcs-Git https://github.com/ubuntukylin/kylin-weather.git ; forges Gitee/GitHub PRESENT (DIVERGENT)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 772 fichiers) ; Gitee non acquis (struct_gitee=null)
- **Langages / taille** : C++/Qt5 (qmake) ; fichiers .ts = traductions Qt ; Shell — 772 fichiers / 11,8 Mo ; ~104 fichiers C++/en-têtes hors tests (~14 200 lignes) ; 39 programmes de test sous tests/ — lignes de code propres mesurées : 28 688
- **Privilège** : utilisateur de session ; aucun service système, aucune politique D-Bus système ni polkit
- **Déclencheurs** : ouverture de session (autostart /etc/xdg/autostart, uniquement locales zh_CN*), activation D-Bus session org.kylin.weather, lancement manuel (menu)
- **Indices réseau** : Client HTTP en clair : global/corevar.cpp:37-40 http://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/ et heweather_simple_s6/ (chaînes constatées); Géolocalisation IP : global/corevar.cpp:28-32 http://whois.pconline.com.cn/, http://myip.ipip.net, http://restapi.amap.com/v5/; edu.kylinos.cn:8001 présent en commentaire (corevar.cpp:46-49); Aucun listener (champ listen vide)
- **Blobs** : Aucun blob binaire ; data/china-city-list.csv (434 Ko) ; images PNG
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant propre de taille raisonnable (~14 k lignes), lancé automatiquement en session, avec requêtes HTTP en clair, géolocalisation IP et liaison kysdk-datacollect (collecte de données).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (session) | `archive:data/org.kylin.weather.service` | Name=org.kylin.weather ; Exec=/opt/kaiming-tools/bin/kaiming run --command=/opt/apps/top.openkylin.kylin-weather/files/bin/kylin-weather top.openkylin.kylin-weather -- noshowmainwindow --dbus-service ; installé dans /usr/share/dbus-1/services (dbusService.path, branche non-kaiming de kylin-weather.pro) | CONSTATÉ |
| autostart | `archive:data/autostart/kylin-weather.desktop` | installé dans /etc/xdg/autostart (startdesktop présent dans INSTALLS) ; Exec=sh -c 'case ${LC_ALL:-${LC_MESSAGES:-$LANG}} in zh_CN*) exec /usr/bin/kylin-weather noshowmainwindow ;; esac' | CONSTATÉ |
| desktop-file | `archive:kylin-weather.desktop` | Exec=/usr/bin/kylin-weather ; X-UKUI-AutoRestart=true ; X-GNOME-Autostart-Delay=10 ; X-KDE-Wayland-Interfaces inclut org_kde_kwin_fake_input et zkde_screencast_unstable_v1 | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-weather.postinst` | glib-compile-schemas /usr/share/glib-2.0/schemas ; rm -f /etc/xdg/autostart/indicator-china-weather.desktop | CONSTATÉ |
| debian-rules / debian-control | `archive:debian/rules ; archive:debian/control` | qmake kylin-weather.pro, dh_missing --fail-missing ; paquet binaire unique kylin-weather ; Build-Depends libkysdk-datacollect-dev, libgeoip-dev | CONSTATÉ |
| cible construite | `archive:kylin-weather.pro` | TARGET=kylin-weather -> /usr/bin ; QT += network dbus ; PKGCONFIG kysdk-datacollect ; sources model/iplocation.cpp, model/network.cpp, model/dbus.cpp, kabase/buriedpoint.hpp | CONSTATÉ |

- [CONSTATÉ] Le .service D-Bus session installé hors build kaiming pointe vers /opt/kaiming-tools/bin/kaiming.
- [PROBABLE] kabase/buriedpoint.hpp (points de télémétrie) et PKGCONFIG kysdk-datacollect : appels de collecte à recenser en phase 1.
- [CONSTATÉ] Manifeste kaiming présent : kaiming/top.openkylin.kylin-weather.km.yaml.
- [NON COUVERT] Arbre Gitee non inventorié alors que forges=PRESENT/DIVERGENT.

### `kylin-ipmsg`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; Homepage https://github.com/ubuntukylin/kylin-ipmsg ; Vcs-Bzr lp://dev/kylin-ipmsg
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 384 fichiers) ; Gitee non acquis
- **Langages / taille** : C++/Qt5 (qmake) ; SQLite — 384 fichiers / 2,7 Mo ; ~89 fichiers C++/en-têtes hors tests (~20 000 lignes) ; src/network/tcp_module.cpp 48 Ko — lignes de code propres mesurées : 35 066
- **Privilège** : utilisateur de session, avec écoute réseau LAN sur ports fixes
- **Déclencheurs** : lancement manuel (aucune entrée autostart trouvée), réception de paquets UDP/TCP de pairs LAN une fois l'application lancée
- **Indices réseau** : Serveur TCP sur 39901 (adresse d'interface locale) : messages et transferts de fichiers (signaux sigTcpServerRecvFileSuccess); Socket UDP 39900 sur toutes les interfaces IPv4 (découverte de pairs PROBABLE); Protocole applicatif maison src/network/protocol_analysis.cpp
- **Blobs** : data/database/kylin-ipmsg.db (86 Ko, SQLite, installé) ; data/database/kylin-ipmsg.db.test (123 Ko, SQLite, tests)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre (~20 k lignes) installé par défaut qui ouvre un serveur TCP et un socket UDP accessibles depuis le LAN (messages et fichiers) : lecture exhaustive du code réseau, du protocole et du stockage SQLite.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| listener TCP | `archive:src/network/tcp_server.cpp` | l.47 this->listen(QHostAddress(ip), port) ; port TCP_LISTEN_PORT=39901 (src/global/utils/global_data.cpp:36,88) ; ip = adresse d'une interface locale (global_data.cpp:79) | CONSTATÉ |
| socket UDP | `archive:src/network/udp_socket.cpp` | l.51 bind(QHostAddress::AnyIPv4, UDP_LISTEN_PORT=39900, ReuseAddressHint) ; slot readyRead -> slotSocketRead | CONSTATÉ |
| desktop-file | `archive:data/kylin-ipmsg.desktop` | Exec=/usr/bin/kylin-ipmsg ; Categories=InstantMessaging ; X-KDE-Wayland-Interfaces inclut org_kde_kwin_fake_input | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | set -e ; glib-compile-schemas /usr/share/glib-2.0/schemas | CONSTATÉ |
| debian-rules / debian-control | `archive:debian/rules ; archive:debian/control` | dh $@ --parallel ; Section net ; Build-Depends libkysdk-datacollect-dev, libpeony-dev, libkysdk-diagnostics-dev | CONSTATÉ |
| fichiers installés | `archive:kylin-ipmsg.pro ; archive:src/src.pro` | target /usr/bin/kylin-ipmsg ; database.files=data/database/kylin-ipmsg.db -> /usr/share/kylin-ipmsg/data/database/ ; QT += network dbus sql | CONSTATÉ |

- [CONSTATÉ] kabase/buriedpoint.hpp présent et PKGCONFIG kysdk-datacollect dans src/src.pro.
- [NON COUVERT] Contenu de la base SQLite installée non inspecté.
- [CONSTATÉ] Manifeste kaiming (répertoire kaiming/).

### `kylin-miraclecast`

- **Nature** : fork-amont — **Amont** : MiracleCast (albfan/miraclecast, Wifi-Display/Miracast) d'après README.md (badges gitter/travis/coveralls albfan) ; version amont non visible (NEWS 'CHANGES WITH 1', debian/watch vide) ; historique openKylin 1.12.7-ok2~0721 (2022) -> 1.12.7-ok3~0622 ; ajouts Kylin visibles : plugin ukui-control-center plugin/projection, gstplayer-1.0 (castplayer), interface D-Bus org.freedesktop.miracle.wifi.ui, scripts res/*.sh
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, format 3.0 native : delta Kylin non séparable sans l'arbre amont)
- **Langages / taille** : C (démons), C++/Qt5 (gstplayer, plugin projection), Shell ; build cmake + qmake — 176 fichiers / 1,3 Mo ; ~60 fichiers C/C++ hors test (~36 300 lignes) — lignes de code propres mesurées : 38 827
- **Privilège** : root (service systemd système sans User=, scripts mainteneur) + session (miracle-agent, castplayer, plugin ukui-control-center)
- **Déclencheurs** : démarrage système (WantedBy=multi-user.target), activation D-Bus système via Alias dbus-org.freedesktop.miracle.wifi.service, ouverture de session (miracle-agent autostart, AutoRestart), mise à jour du paquet (--restart-after-upgrade)
- **Indices réseau** : Sockets DHCP AF_PACKET/AF_INET : src/dhcp/common.c:538,597 ; client.c:1265 ; ipv4ll.c:94,136 (serveur/client DHCP du groupe Wi-Fi P2P, PROBABLE); Socket unix de contrôle wpa_supplicant : src/shared/wpas.c:931,1041,1049; RTSP (src/shared/rtsp.c) et UIBC (src/uibc/) du protocole Miracast; Depends wpasupplicant (>= 2:2.9-1kylin4.8), network-manager
- **Blobs** : Aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Fork amont volumineux (~36 k lignes C) avec service root, politique D-Bus système ouvrant 15 méthodes à tous et code réseau Wi-Fi Direct/DHCP/RTSP : diff contre albfan/miraclecast puis L3 sur le delta Kylin (wifid-dbus, interface ui, agent, scripts res/*.sh).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit (système) | `archive:res/miracle-wifid.service.cmake` | Fichier configuré par res/CMakeLists.txt et installé dans systemdsystemunitdir : BusName=org.freedesktop.miracle.wifi ; ExecStart=/usr/bin/execstart.sh ; ExecStop=/usr/bin/recover_wpas.sh ; aucun User= (root) ; CapabilityBoundingSet commenté ; WantedBy=multi-user.target ; Alias=dbus-org.freedesktop.miracle.wifi.service | CONSTATÉ |
| systemd-unit (variante) | `archive:res/miracle-wifid.service.in` | ExecStart=@exec_path@/miracle-wifid --use-dev --log-level error --lazy-managed --config-methods display (variante autotools/meson, non utilisée par debian/rules cmake) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --with autoreconf ; cmake + qmake gstplayer-1.0 et plugin/projection ; override_dh_installsystemd : dh_installsystemd --restart-after-upgrade miracle-wifid.service | CONSTATÉ |
| dbus-policy (système) | `archive:res/org.freedesktop.miracle.conf` | Installée dans /etc/dbus-1/system.d ; policy user=root : own org.freedesktop.miracle et .wifi ; context=default : deny send_destination puis allow de 15 méthodes de org.freedesktop.miracle.wifi.ui (UsrDisconnect, ProjectionOp, HandShake, SetSinkName, SetSinkConfigMethods, QueryConnectedDevice, DeleteConnectedDevice, StartWpas, StopWpas, ResolutionUpdate, AgtStartR2Sess, QueryWpaStopped, SetManufacturer, QueryGroupInfo, userChosen) + Introspectable, Peer, ObjectManager, Properties.Get/GetAll | CONSTATÉ |
| autostart | `archive:res/miracle-agent.desktop` | Installé dans /etc/xdg/autostart (res/CMakeLists.txt) ; Exec=/usr/bin/miracle-agent ; X-UKUI-AutoRestart=true ; NoDisplay=true | CONSTATÉ |
| maintainer-script | `archive:debian/preinst ; archive:debian/postrm` | rm -rf /root/.config/miracast | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | mv /usr/share/projection/translations/libprojection.so vers /usr/lib/<multiarch>/ukui-control-center/libprojection.so ; kill -9 du PID de miracle-agent | CONSTATÉ |
| debian-install | `archive:debian/install` | gstplayer-1.0/castplayer -> /usr/bin ; libprojection.so -> /usr/share/projection/translations/ (bibliothèque déposée dans un répertoire de traductions) | CONSTATÉ |
| cibles construites | `archive:src/*/CMakeLists.txt ; archive:res/CMakeLists.txt` | miracled, miracle-wifid, miracle-dhcp, miracle-agent, miracle-sinkctl, miracle-wifictl, miracle-uibcctl -> bin ; uibc-viewer, checkhardware.sh, checkDaemonRunning.sh, checkFrontendRunning.sh, execstart.sh, recover_wpas.sh -> bin | CONSTATÉ |
| script ExecStart | `archive:res/execstart.sh ; archive:res/recover_wpas.sh` | Contenu non lu en phase 0.2 (scripts hors périmètre déclaratif) | NON COUVERT |

- [CONSTATÉ] Non installé par défaut par l'ISO 3.0 (installe_par_defaut_iso vide).
- [CONSTATÉ] Fiche meta origine_kylin=1 / format native alors que le README est celui de MiracleCast albfan.
- [CONSTATÉ] Description debian/control 'auto-generated package by debmake' ; Homepage placeholder.

### `kylin-miraclecast-frontend`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; patches internes 2021 (miracast-dialog-1.0) ; forges PRESENT (IDENTICAL-HEADS)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 40 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) — 40 fichiers / 86 Ko ; ~1 180 lignes C++ — lignes de code propres mesurées : 1 180
- **Privilège** : utilisateur de session
- **Déclencheurs** : invocation par la pile kylin-miraclecast (PROBABLE), lancement manuel
- **Indices réseau** : Aucun listener ; client D-Bus session (dbus/interfacedbusproxy.cpp:7, src/widget.cpp:67)
- **Blobs** : Aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petite IHM de session (~1,2 k lignes) sans service exposé ni réseau, non installée par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:miraclecast-frontend.desktop` | Exec=/usr/bin/miraclecast-frontend ; NoDisplay=true ; X-UKUI-AutoRestart=true ; non installé par le .pro (INSTALLS = target seul) | CONSTATÉ |
| maintainer-script | `archive:debian/miraclecast-frontend.postinst` | Supprime /etc/xdg/autostart/miraclecast-frontend.desktop ; nom de fichier ne correspondant pas au paquet binaire kylin-miraclecast-frontend | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | dh $@ ; paquet kylin-miraclecast-frontend ; description placeholder '<insert up to 60 chars description>' | CONSTATÉ |
| cible construite | `archive:miraclecast-frontend.pro` | TARGET=miraclecast-frontend -> /usr/bin ; QT += dbus x11extras | CONSTATÉ |
| patches | `archive:debian/patches/series` | 4 patches (dont 20210608_Add_autostart_scripts.patch) en format 3.0 native : non appliqués par dpkg-source | PROBABLE |

- [CONSTATÉ] Non installé par défaut par l'ISO.

### `kylin-camera`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; Homepage https://github.com/ubuntukylin/kylin-camera ; s'appuie sur libkylincamera (paquet kylincameralibs, sans source)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète de l'IHM) ; bibliothèque de capture libkylincamera sans source (kylincameralibs = squelette)
- **Langages / taille** : C++/Qt5 (qmake) — 235 fichiers / 26,7 Mo (dont 13,2 Mo binaire 'camera' et images de guide) ; 86 fichiers C++ src (~10 600 lignes) — lignes de code propres mesurées : 10 655
- **Privilège** : utilisateur de session ; accès caméra/micro
- **Déclencheurs** : lancement manuel, D-Bus session org.ukui.kylin_camera
- **Indices réseau** : Aucun indice réseau
- **Blobs** : camera (racine) : ELF 64-bit x86-64 PIE, with debug_info, not stripped, 13 198 496 o, BuildID 3054938f0a87… ; non référencé dans INSTALLS (non installé, PROBABLE)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — IHM de session à accès caméra/micro dont la logique de capture est déléguée à une bibliothèque sans source ; binaire ELF versionné à comparer au build.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:kylin-camera.desktop` | Exec=/usr/bin/kylin-camera ; Categories=Graphics | CONSTATÉ |
| cible construite | `archive:kylin_camera.pro` | TARGET=kylin-camera -> /usr/bin ; LIBS += -lkylincamera ; INCLUDEPATH /usr/include/kylincameralibs/ ; QT += multimedia dbus | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | qmake kylin_camera.pro ; Build-Depends kylincameralibs (>=1.0.0.1-0k0.23), libx264-dev, libfaac-dev, libgpac-dev ; Depends kylincameralibs, libqt5sql5-sqlite ; aucun script mainteneur | CONSTATÉ |
| dbus (session) | `archive:src/util/daemonipcdbus.cpp` | registerService("org.ukui.kylin_camera") sur le bus session (l.6,12) | CONSTATÉ |

- [CONSTATÉ] Dépendance de build et d'exécution vers kylincameralibs, dont l'arbre archive ne contient aucun code.
- [CONSTATÉ] Manifeste kaiming top.openkylin.kylin-camera.km.yaml.

### `kylin-screencap`

- **Nature** : absent-ou-vide — **Amont** : Homepage https://github.com/ukui/kylin-screencap (debian/control) ; forges Gitee et GitHub ABSENT
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun (squelette dh_make : debian/*.ex, rules 'dh $@') ; meta.source_coquille_vide=1
- **Langages / taille** : aucun — 19 fichiers / 26 Ko, uniquement debian/ — lignes de code propres mesurées : 0
- **Privilège** : inconnu (aucun fichier déclaratif réel)
- **Déclencheurs** : inconnus
- **Indices réseau** : aucun (pas de code)
- **Blobs** : Aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source alors que le binaire est installé par défaut : non auditable sans récupérer et analyser le .deb.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | '%: dh $@' (aucune cible de build) | CONSTATÉ |
| debian-control | `archive:debian/control` | paquet kylin-screencap, Architecture any, Suggests libcrystalhd3 \| firmware-crystalhd ; description d'enregistreur d'écran Qt5 | CONSTATÉ |
| modèles inactifs | `archive:debian/postinst.ex ; preinst.ex ; prerm.ex ; postrm.ex ; kylin-screencap.cron.d.ex` | Modèles dh_make (.ex) non utilisés par dh | CONSTATÉ |
| binaire installé | `(aucun fichier source)` | Le binaire kylin-screencap=1.0.0.1-ok0.21 installé par l'ISO n'a pas de source correspondante | NON COUVERT |

- [CONSTATÉ] Changelog unique 'Initial Release' (26 août 2026, huanghe).

### `kylin-screenshot`

- **Nature** : fork-amont — **Amont** : Flameshot (README.md : 'forked from flameshot' ; nom D-Bus org.dharkael.kylinscreenshot ; TAG_VERSION par défaut v0.6.0 dans kylin_screenshot.pro -> base Flameshot ~0.6, PROBABLE) ; Vcs https://github.com/ubuntukylin/kylin-screenshot ; sous-composant Kylin inputdevice_manager
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 654 fichiers, format native : delta non séparé) ; Gitee non acquis
- **Langages / taille** : C++/Qt5 (qmake) — 654 fichiers / 7,3 Mo ; kylin_screenshot/src 318 fichiers (~41 300 lignes dont ~1 350 third-party) ; inputdevice_manager 253 lignes — lignes de code propres mesurées : 41 494
- **Privilège** : root (inputdevice_manager activable sur le bus système) + utilisateur de session (kylin-screenshot)
- **Déclencheurs** : activation D-Bus système com.kylin.inputdevice.manager, activation D-Bus session org.dharkael.kylinscreenshot, raccourci clavier / lancement manuel
- **Indices réseau** : Upload Imgur : src/tools/imgur/imguruploader.cpp:113 https://api.imgur.com/3/image ; IMGUR_CLIENT_ID par défaut '313baf0c7b4d3ff' (src/imgur.pri); QLocalServer d'instance unique (src/third-party/singleapplication/singleapplication.cpp) : IPC locale; Aucun listener TCP/UDP
- **Blobs** : Aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Fork Flameshot volumineux (~41 k lignes) en session avec IPC D-Bus et upload réseau ; le sous-composant Kylin inputdevice_manager (service root, libinput/udev, ~250 lignes) doit être lu intégralement (L3).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (système) | `archive:inputdevice_manager/com.kylin.inputdevice.manager.service` | Installé dans /usr/share/dbus-1/system-services (inputdevice_manager.pro) ; Name=com.kylin.inputdevice.manager ; Exec=/usr/bin/inputdevice_manager ; User=root | CONSTATÉ |
| dbus-policy (système) | `archive:inputdevice_manager/com.kylin.inputdevice.manager.conf` | Installée dans /usr/share/dbus-1/system.d ; user=root : own ; context=default : allow send_interface=com.kylin.inputdevice.manager (sans send_destination), Introspectable, Properties | CONSTATÉ |
| interface D-Bus | `archive:inputdevice_manager/com.kylin.inputdevice.manager.xml` | Unique signal onMouseWheel((ii)) ; binaire lié à -linput -ludev | CONSTATÉ |
| dbus-service (session) | `archive:kylin_screenshot/dbus/package/org.dharkael.kylinscreenshot.service` | Name=org.dharkael.kylinscreenshot ; Exec=/usr/bin/kylin-screenshot (CONFIG+=packaging dans debian/rules) ; variante make : /usr/local/bin | CONSTATÉ |
| desktop-file | `archive:kylin_screenshot/docs/desktopEntry/package/kylin-screenshot.desktop` | Exec=kylin-screenshot gui -d 100 ; installé dans /usr/share/applications | CONSTATÉ |
| autostart (non installé) | `archive:kylin_screenshot/kylin-screenshot-start.desktop` | startdesktop.path=/etc/xdg/autostart défini mais startdesktop absent de INSTALLS (kylin_screenshot.pro:359-386) | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | hardening=+all ; CONFIG+=packaging CONFIG-=debug ; Rules-Requires-Root: no ; Suggests ca-certificates, openssl ; aucun script mainteneur | CONSTATÉ |
| fichiers installés | `archive:kylin_screenshot/kylin_screenshot.pro` | /usr/bin/kylin-screenshot, interfaces D-Bus, gschema, /etc/ukui/ukui-notification/default-app/kylin-screenshot.json, complétion bash | CONSTATÉ |

- [CONSTATÉ] kysdk-datacollect n'apparaît que dans tests/unit_test_default/unit_test_default.pro, mais libkysdk-datacollect-dev est en Build-Depends.

### `kylin-recorder`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; Homepage https://github.com/ubuntukylin/kylin-recorder
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 375 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) ; Python (tests) — 375 fichiers / 5,5 Mo ; 64 fichiers src (~12 400 lignes) ; 34 patches dans debian/patches — lignes de code propres mesurées : 14 907
- **Privilège** : utilisateur de session (accès micro)
- **Déclencheurs** : lancement manuel, D-Bus session (MPRIS)
- **Indices réseau** : Aucun indice réseau
- **Blobs** : Aucun (tests/test.mp3)
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Application de session sans service système ni réseau ; IPC limitée à MPRIS/instance unique ; triage de la gestion des fichiers audio et des appels kysdk-datacollect.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:kylin-recorder.desktop` | Exec=/usr/bin/kylin-recorder %u | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-recorder.postinst` | #!/bin/bash sans set -e ; glib-compile-schemas /usr/share/glib-2.0/schemas | CONSTATÉ |
| debian-rules / control / install | `archive:debian/rules ; archive:debian/control ; archive:debian/install` | dh $@ ; Build-Depends libkysdk-datacollect-dev, libmp3lame-dev, ffmpeg, libmpv-dev ; install du guide utilisateur | CONSTATÉ |
| cible construite | `archive:kylin-recorder.pro` | target /usr/bin ; QT += dbus sql multimedia ; PKGCONFIG kysdk-datacollect, peony | CONSTATÉ |
| dbus (session) | `archive:src/dbusadapter.cpp ; archive:src/mainwindow.cpp` | registerService org.mpris.MediaPlayer2.KylinRecorder (l.5) et org.ukui.kylin_recorder (l.228) | CONSTATÉ |

- [CONSTATÉ] debian/patches (34) en format 3.0 native : non appliqués par dpkg-source.
- [CONSTATÉ] kabase/buriedpoint.hpp présent.
- [CONSTATÉ] [correction critique] Lié à kysdk-datacollect : budget L1 maintenu, avec revue ciblée de tous les appels kdk_dia_* (critère « collecte »).

### `kylin-printer`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; Homepage https://github.com/ubuntukylin/kylin-printer
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 416 fichiers)
- **Langages / taille** : C++/Qt5 (CMake) ; Python ; C ; Shell — 416 fichiers / 6 Mo ; ~202 fichiers C/C++/Python hors tests (~26 400 lignes) — lignes de code propres mesurées : 35 421
- **Privilège** : root (service systemd système permanent, script RUN udev, scripts mainteneur) + session (applet, IHM)
- **Déclencheurs** : démarrage système (multi-user.target, Restart=always), branchement/débranchement USB de classe imprimante (udev 07/01), installation du paquet (postinst), ouverture de session (autostart UKUI), signaux D-Bus système com.redhat.PrinterSpooler PrinterAdded/PrinterRemoved (dbus/dbus.cpp:28-29)
- **Indices réseau** : api.kylinos.cn (common/base_info.cpp:104, ui/gsettings_monitor.h:91) : recherche de pilotes getprinterdrive/getprintertype; Adresse privée codée en dur 172.20.184.183:8888 (device_manager/device_information.cpp:190,557); Socket NETLINK_KOBJECT_UEVENT (device_manager/device_monitor.cpp:65-73), pas un listener réseau; Découverte réseau d'imprimantes (tests test_snmp, test_smb, test_networkdevicebrowse ; avahi) PROBABLE
- **Blobs** : tests/usb-creator-common_0.3.7kylin1_amd64.deb (75 Ko, archive .deb de test) ; doc/*.pdf
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre (~26 k lignes) avec service root permanent, règle udev RUN root, scripts mainteneur modifiant un fichier d'un autre paquet et chaîne réseau de recherche/installation de pilotes : lecture exhaustive des parties root/udev/installation, L2 sur l'IHM.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit (système) | `archive:data/kylin-printer-applet-dbus.service` | Installé dans /usr/lib/systemd/system ; ExecStart=/usr/share/kylin-printer/kylin-printer-applet-dbus ; Type=simple ; Restart=always ; Requires/After=cups.socket ; aucun User= (root) ; WantedBy=multi-user.target | CONSTATÉ |
| systemd-unit (utilisateur) | `archive:data/kylin-printer-applet.service` | Installé dans /usr/lib/systemd/user ; ExecStart=/usr/bin/kylin-printer-applet ; Restart=on-failure ; WantedBy=default.target | CONSTATÉ |
| udev | `archive:data/99-kylin-printer-applet.rules` | Installé dans /usr/lib/udev/rules.d ; ACTION==add SUBSYSTEM==usb ID_USB_INTERFACES==*:0701??:* RUN+=/usr/share/kylin-printer/kylin-printer-applet-start usbadd_%p ; ACTION==remove INTERFACE==7/1/* RUN+=... usbremove_%p | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | systemctl enable + start kylin-printer-applet-dbus.service ; ln -s /usr/share/kylin-printer/kylin-printer-applet.desktop /etc/xdg/autostart/ ; exécute /usr/share/kylin-printer/kylin-printer-applet-start (configure) ; pkill des anciens processus ; rm -rf /tmp/kylin-printer-applet-env ; sed commentant un bloc de /lib/udev/udev-add-printer (fichier d'un autre paquet) | CONSTATÉ |
| maintainer-script | `archive:debian/prerm` | rm autostart ; systemctl stop/disable kylin-printer-applet-dbus.service ; pkill kylin-printer-applet ; sed décommentant /lib/udev/udev-add-printer | CONSTATÉ |
| autostart (via postinst) | `archive:backend/data/kylin-printer-applet.desktop.in` | Exec=/usr/share/kylin-printer/kylin-printer-applet-once ; OnlyShowIn=UKUI ; NoDisplay=true | CONSTATÉ |
| desktop-file | `archive:kylin-printer.desktop.in` | Exec=/usr/bin/kylin-printer ; X-KDE-Wayland-Interfaces inclut org_kde_kwin_fake_input | CONSTATÉ |
| cibles construites | `archive:CMakeLists.txt` | kylin-printer et kylin-printer-applet -> /usr/bin ; kylin-printer-applet-dbus, kylin-printer-applet-once, kylin-printer-applet-start -> /usr/share/kylin-printer/ ; kylin-printer-applet-dbus lié à Qt Network/DBus ; json /etc/ukui/ukui-notification/default-app/ | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends libcups2, kylin-system-updater, avahi-daemon, libnss-mdns, libdriver-acquirer-widget, libboundscheck | CONSTATÉ |
| script RUN udev / postinst | `archive:data/kylin-printer-applet-start` | Contenu non lu en phase 0.2 | NON COUVERT |

- [CONSTATÉ] Le service 'kylin-printer-applet-dbus' ne réserve aucun nom D-Bus (aucun registerService dans dbus/*.cpp) : il écoute des signaux du bus système.
- [NON COUVERT] Gitee non acquis.

### `kylin-scanner`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ubuntukylin/kylin-scanner ; fiche meta = ppa-anything3.0 1.0.0-ok1 (3.0 quilt) mais arbre archive = 3.2.1-ok2.46 en 3.0 (native), version identique à l'ISO
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 289 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) — 289 fichiers / 8,9 Mo ; 114 fichiers src (~19 500 lignes) — lignes de code propres mesurées : 20 575
- **Privilège** : utilisateur de session
- **Déclencheurs** : lancement manuel, hotplug USB (socket netlink interne)
- **Indices réseau** : Découverte réseau de scanners : SNMP (src/device/snmpFinder.cpp), Avahi/zeroconf (src/device/avahi_qt, qt_zeroconf); api.kylinos.cn (src/device/base_info.cpp:35) ; ping kom.kylinos.cn (src/device/device_information.cpp:283); Installation de pilotes via src/device/ukui_apt.cpp; src/sendmail.cpp (envoi de courriel); Socket NETLINK_KOBJECT_UEVENT (src/usb.cpp:35-48), pas un listener
- **Blobs** : test/kylin-scanner (ELF x86-64 4 Mo, 'missing section headers', non installé PROBABLE) ; test/1.pdf ; images
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Application de session de taille moyenne avec accès réseau sortant (SNMP, Avahi, API Kylin), déclenchement d'installation de pilotes et traitement d'images/OCR : L3 ciblé sur src/device/* et ukui_apt, L2 ailleurs.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:data/kylin-scanner.desktop` | Exec=kylin-scanner %U ; Categories Scanning;OCR | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | qmake, hardening=+all ; Depends tesseract-ocr, kylin-system-updater, libboundscheck ; Build-Depends libsnmp-dev, libavahi-client-dev, libusb-1.0-0-dev, libkysdk-datacollect-dev ; aucun script mainteneur | CONSTATÉ |
| cible construite | `archive:kylin-scanner.pro` | target /usr/bin ; QT += network testlib ; QMAKE_CXXFLAGS --coverage, LIBS -lgcov ; SOURCES inclut test/mytest.cpp ; LIBS -lnetsnmp -lavahi-client -lsane -lusb-1.0 -ltesseract ; jsonfiles '/data/kylin-scanner.json' -> /etc/ukui/ukui-notification/default-app/ | CONSTATÉ |

- [CONSTATÉ] Écart fiche/arbre sur la version et le format (1.0.0-ok1 quilt vs 3.2.1-ok2.46 native).
- [CONSTATÉ] Code de test et instrumentation de couverture compilés dans le binaire livré.
- [CONSTATÉ] src/kabase/buriedpoint.hpp et PKGCONFIG kysdk-datacollect.

### `kylin-sane-virtual-backend`

- **Nature** : absent-ou-vide — **Amont** : inconnu ; Original-Maintainer liuyuhui@kylinos.cn ; forges Gitee et GitHub ABSENT
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : aucun (squelette dh_make) ; meta.source_coquille_vide=1
- **Langages / taille** : aucun — 19 fichiers / 26,6 Ko, uniquement debian/ — lignes de code propres mesurées : 0
- **Privilège** : inconnu ; un backend SANE est chargé par tout frontal SANE, y compris saned (PROBABLE)
- **Déclencheurs** : chargement par libsane (PROBABLE)
- **Indices réseau** : inconnus (pas de code)
- **Blobs** : Aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Squelette sans code alors que deux binaires (bibliothèque SANE + proxy) sont installés par défaut : non auditable sans les .deb.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 2 paquets binaires : kylin-sane-virtual-backend (bibliothèque backend SANE 'virtual scanners', communique avec l'application proxy) et kylin-sane-virtual-backend-app (application proxy 'scanner communication') | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' | CONSTATÉ |
| binaires installés | `(aucun fichier source)` | kylin-sane-virtual-backend et -app =1.0.0.1-0k0.2ok0.2 installés par l'ISO sans source | NON COUVERT |

- [CONSTATÉ] Changelog 'Initial Release' nile.bedrock (2 avril 2026) ; Maintainer 'Ubuntu Developers'.

### `kylin-music`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ubuntukylin/kylin-music ; composants tiers embarqués : wangfenjin/simple (tokenizer FTS5), cppjieba, SQLite 3.32.3 amalgamation (3rdparty/contrib/sqlite3), googletest-release-1.10.0.zip
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 557 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) ; C (SQLite) ; CMake — 557 fichiers / 23,8 Mo ; ~94 fichiers C++ propres (~23 200 lignes) + 3rdparty (sqlite3.c 8,1 Mo) ; 63 patches dans debian/patches — lignes de code propres mesurées : 23 230
- **Privilège** : utilisateur de session ; scripts mainteneur root
- **Déclencheurs** : lancement manuel, D-Bus session MPRIS, installation/suppression du paquet
- **Indices réseau** : QT += network déclaré (kylin-music.pro:1), usage non identifié; Aucun listener
- **Blobs** : 3rdparty/test/googletest-release-1.10.0.zip (1,1 Mo, ZIP) ; dictionnaires jieba
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Lecteur de session sans service exposé hors MPRIS ; triage du code propre, lecture des scripts mainteneur (boucle /home/*, lien /usr/lib/libsimple.so) et inventaire des versions tierces embarquées.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/kylin-music.preinst` | Écrit un horodatage dans /tmp/kylin-music.update | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-music.postinst` | glib-compile-schemas ; supprime puis crée le lien /usr/lib/libsimple.so -> /usr/lib/kylin-music/libsimple.so | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-music.postrm` | Lit /tmp/kylin-music.update (écart < 60 s = mise à jour) ; sinon pour chaque /home/* supprime ~/.config/.kylin_music_ver*_mymusic.db ; supprime le lien /usr/lib/libsimple.so | CONSTATÉ |
| debian-rules | `archive:debian/rules` | override_dh_auto_configure : cmake + make dans 3rdparty/build puis qmake | CONSTATÉ |
| cible construite | `archive:kylin-music.pro` | target /usr/bin ; simple.path=/usr/lib/kylin-music ; QT += network dbus sql ; PKGCONFIG taglib, kysdk-datacollect ; LIBS -lsqlite3 -lavformat | CONSTATÉ |
| desktop-file / dbus session | `archive:kylin-music.desktop ; archive:UI/dbusadapter.cpp` | Lancement de l'application ; registerService org.mpris.MediaPlayer2.KylinMusic (l.26) | CONSTATÉ |

- [CONSTATÉ] debian/patches (63) en format 3.0 native : non appliqués par dpkg-source.
- [CONSTATÉ] Version SQLite embarquée 3.32.3 (3rdparty/contrib/sqlite3/sqlite3.h).
- [CONSTATÉ] [correction critique] Lié à kysdk-datacollect : budget L1 maintenu, avec revue ciblée de tous les appels kdk_dia_* (critère « collecte »).

### `kylin-video`

- **Nature** : propre-kylin — **Amont** : KylinSoft ('Front-end for MPV') ; Homepage https://github.com/ukui/kylin-video ; fiche meta = ppa-anything3.0 3.1.3-ok1 (quilt) mais arbre = 3.1.4.2k1-ok26.39 (native), version identique à l'ISO
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 594 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) — 594 fichiers / 13,3 Mo ; 109 fichiers src (~24 200 lignes) — lignes de code propres mesurées : 31 240
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture d'un fichier média (association MIME), lancement manuel, D-Bus session
- **Indices réseau** : Aucun module Qt network ; lecture de flux possible via libmpv (PROBABLE)
- **Blobs** : Aucun
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Front-end MPV de session ; le décodage est délégué à libmpv/ffmpeg (hors lot) ; triage.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:kylin-video.desktop` | Exec=kylin-video %U ; MimeType audio/* et video/* très étendu (incl. application/x-shockwave-flash, application/vnd.rn-realvideo) | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | qmake, hardening=+all ; Depends libmpv1, ffmpegthumbnailer, mesa-vdpau-drivers, libcrystalhd3 [amd64 i386] ; aucun script mainteneur | CONSTATÉ |
| cible construite | `archive:src/src.pro` | TARGET=kylin-video -> /usr/bin ; QT += core gui dbus sql KGlobalAccel ; PKGCONFIG kysdk-datacollect | CONSTATÉ |
| dbus (session) | `archive:src/core/dbusadapter.cpp ; archive:src/mainwindow.cpp` | registerService org.mpris.MediaPlayer2.KylinVideo (l.63) et org.ukui.kylinvideo (l.419) | CONSTATÉ |

- [CONSTATÉ] Écart fiche/arbre sur version et format.
- [CONSTATÉ] [correction critique] Lié à kysdk-datacollect : budget L1 maintenu, avec revue ciblée de tous les appels kdk_dia_* (critère « collecte »).

### `kylin-photo-viewer`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ubuntukylin/kylin-photo-viewer ; tiers embarqués : QtApng avec libpng 1.6.37 et zlib 1.2.11 (codec/plugins/QtApng/src/3rdparty)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 850 fichiers)
- **Langages / taille** : C++/Qt5, QML, C (tiers), Shell — 850 fichiers / 68,5 Mo ; ~151 fichiers propres (~36 300 lignes hors 3rdparty/tests) — lignes de code propres mesurées : 38 292
- **Privilège** : utilisateur de session ; code exécuté dans le processus peony via le plugin
- **Déclencheurs** : ouverture d'une image (association MIME), menu contextuel peony (rotation), D-Bus session
- **Indices réseau** : Aucun indice réseau
- **Blobs** : model/kylin_photo_viewer_matting_model.onnx (25,9 Mo, modèle ONNX, installé dans /usr/share/kylin-photo-viewer)
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant volumineux (~36 k lignes) traitant des images non fiables via codecs propres et tiers embarqués, avec plugin injecté dans peony et modèle ONNX opaque.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/kylin-photo-viewer.install` | model -> /usr/share/kylin-photo-viewer ; usr/bin, usr/lib/*.so*, usr/lib/*/qt5/ (plugins d'images), cmake, include | CONSTATÉ |
| debian-install (plugin) | `archive:debian/peony-rotate-menu-plugin.install` | usr/lib/*/peony-extensions/ : plugin chargé dans le gestionnaire de fichiers peony | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-photo-viewer.postinst` | glib-compile-schemas | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | qmake ; dh_shlibdeps --ignore-missing-info ; Depends libonnxruntime, libkysdk-ocr, apng2gif, apngasm, peony-rotate-menu-plugin | CONSTATÉ |
| desktop-file | `archive:data/kylin-photo-viewer.desktop` | Exec=/usr/bin/kylin-photo-viewer %U ; MimeType images multiples | CONSTATÉ |
| cibles construites | `archive:kylin-photo-viewer.pro` | SUBDIRS src, codec, peony-rotate-menu-plugin, tests (photo-viewer-aiassistant-plugin commenté) | CONSTATÉ |
| dbus (session) | `archive:src/model/dbus.cpp` | registerService(Variable::PHOTO_VIEW_DBUS_SERVICENAME) (l.22) | CONSTATÉ |

- [CONSTATÉ] kabase/buriedpoint.hpp et PKGCONFIG kysdk-datacollect (src/src.pro).
- [CONSTATÉ] Paquet binaire peony-rotate-menu-plugin installé par défaut par l'ISO.

### `kylin-calculator`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native) ; Vcs https://github.com/ubuntukylin/kylin-calculator
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 512 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) ; Python (tests) — 512 fichiers / 2,3 Mo ; ~15 500 lignes C++ hors tests — lignes de code propres mesurées : 20 789
- **Privilège** : utilisateur de session
- **Déclencheurs** : lancement manuel
- **Indices réseau** : src/toolmodel.cpp:1079 http://www.safe.gov.cn/AppStructured/hlw/RMBQuery.do (taux de change, HTTP en clair) ; https://api.exchangerate-api.com en commentaire (l.1078)
- **Blobs** : Aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Application de session à faible privilège ; seul point notable : requête HTTP en clair de taux de change et analyse de sa réponse.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:kylin-calculator.desktop` | Exec=/usr/bin/kylin-calculator %u | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-calculator.postinst` | set -e ; glib-compile-schemas | CONSTATÉ |
| debian-rules | `archive:debian/rules` | override_dh_auto_configure : qmake kylin-calculator-intel-plug.pro && make && qmake kylin-calculator.pro && make | CONSTATÉ |
| cibles construites | `archive:kylin-calculator.pri ; archive:kylin-calculator-intel-plug.pro` | target /usr/bin ; lib.path=/opt/small-plugin/children/bin (plugin intel_plug) ; QT += network dbus ; PKGCONFIG kysdk-datacollect | CONSTATÉ |

- [CONSTATÉ] Manifeste kaiming présent ; fichiers .pro.user versionnés.
- [CONSTATÉ] [correction critique] Lié à kysdk-datacollect : budget L1 maintenu, avec revue ciblée de tous les appels kdk_dia_* (critère « collecte »).

### `kylin-calendar`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://gitee.com/openkylin/kylin-calendar ; dans_debian=1
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 241 fichiers)
- **Langages / taille** : C++/Qt5 (CMake) ; HTML/JSON — 241 fichiers / 10,4 Mo ; 35 fichiers C++ (~15 600 lignes) ; 41 patches dans debian/patches — lignes de code propres mesurées : 15 584
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session UKUI (autostart, AutoRestart), D-Bus session
- **Indices réseau** : Qt5 Network lié (CMakeLists.txt:22,136), usage non identifié; Aucun listener
- **Blobs** : Aucun (police fontawesome-webfont.ttf)
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session lancé automatiquement (autostart + AutoRestart), exposant deux services D-Bus session dont un remplaçable, avec module réseau lié et dépendance cron.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart | `archive:resources/calendar.desktop` | Installé dans /etc/xdg/autostart (CMakeLists.txt:144,155) ; Exec=/usr/bin/kylin-calendar -h %u ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; X-UKUI-Autostart-Phase=Application | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-calendar-plugin.postinst` | glib-compile-schemas ; exécute /usr/share/kylin-system-updater/kylin-reboot-required si présent ; nom ne correspondant à aucun paquet binaire (paquet = kylin-calendar) | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | cmake RelWithDebInfo, hardening=+all ; Depends cron ; description debmake auto-générée | CONSTATÉ |
| dbus (session) | `archive:lunarcalendarwidget/frmlunarcalendarwidget.cpp ; archive:lunarcalendarwidget/notificationsadaptor.cpp` | registerService org.ukui.kylin_calendar (l.122) ; org.ukui.calendar.Notifications avec ReplaceExistingService/AllowReplacement (notificationsadaptor.cpp:38,46) | CONSTATÉ |
| fichiers installés | `archive:CMakeLists.txt` | /usr/bin/kylin-calendar ; json /etc/ukui/ukui-notification/default-app/ ; html/ -> plugin-calendar/html ; Qt5::Network lié | CONSTATÉ |

- [PROBABLE] Termes 'cron'/'crontab' présents dans lunarcalendarwidget/*.cpp (schedulewidget, calendardatabase) : usage à qualifier.
- [CONSTATÉ] debian/patches (41) en format 3.0 native : non appliqués par dpkg-source.

### `kylin-font-viewer`

- **Nature** : propre-kylin — **Amont** : KylinSoft (format 3.0 native)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 114 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) — 114 fichiers / 1,4 Mo ; ~7 100 lignes — lignes de code propres mesurées : 7 664
- **Privilège** : utilisateur de session
- **Déclencheurs** : lancement manuel / ouverture de fichier de police
- **Indices réseau** : Aucun
- **Blobs** : Aucun
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Visionneuse de polices de session non installée par défaut ; seule surface : parsing de polices via freetype (hors lot).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:data/kylin-font-viewer.desktop` | Exec=/usr/bin/kylin-font-viewer %u | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-font-view.postinst` | glib-compile-schemas ; nom ne correspondant pas au paquet binaire kylin-font-viewer | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ --parallel ; override_dh_shlibdeps contient la seule ligne '--dpkg-shlibdeps-params=–ignore-missing-info' (dh_shlibdeps jamais appelé) | CONSTATÉ |
| cible construite | `archive:kylin-font-viewer.pro` | bin -> /usr/bin ; PKGCONFIG freetype2 fontconfig gsettings-qt | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO.
- [CONSTATÉ] kabase/buriedpoint.hpp présent.

### `kylin-burner`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ukui/kylin-burner ; héritage K3b visible (.desktop X-DocPath=k3b/index.html, MimeType application/x-k3b) ; xorriso 1.5.6 embarqué (libburn, libisofs, libjte) ; fiche meta = 3.0.11-ok1 (quilt) mais arbre = 3.2.0.1kylin1k32.01-ok45 (native) = version ISO
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 640 fichiers)
- **Langages / taille** : C++/Qt5 (CMake) ; C (xorriso) — 640 fichiers / 16,9 Mo ; ~196 fichiers propres (~52 200 lignes) + xorriso embarqué (~209 500 lignes) — lignes de code propres mesurées : 276 049
- **Privilège** : utilisateur de session (accès lecteurs optiques via udisks2) ; postinst root
- **Déclencheurs** : lancement manuel / insertion de disque (MimeType x-content/blank-*), installation du paquet
- **Indices réseau** : QLocalServer d'instance unique (local); Socket NETLINK_KOBJECT_UEVENT (src/model/monitorcdrom.cpp:122), pas un listener réseau
- **Blobs** : tests/test_data/test.dat (5 Ko de zéros)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant volumineux (~52 k lignes propres + xorriso embarqué) de session : L2 sur le code propre (périphériques, ISO/UDF ky-udf-image, ky-disc-info) et inventaire du delta xorriso.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/kylin-burner.postinst` | mkdir /var/log/kylin-burner (0755) ; touch os_audit.log ; chmod 0777 /var/log/kylin-burner/os_audit.log | CONSTATÉ |
| logrotate | `archive:debian/kylin-burner.logrotate` | /var/log/kylin-burner/*log daily rotate 7 ; postrotate invoke-rc.d --quiet kylin-burner restart (aucun script init kylin-burner dans debian/) | CONSTATÉ |
| desktop-file | `archive:kylin-burner.desktop ; archive:kylin-burner-kaiming.desktop` | Exec=/usr/bin/kylin-burner %U (kaiming : /opt/apps/top.openkylin.kylin-burner/files/bin/kylin-burner) | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | dh $@ ; Depends udisks2 ; Build-Depends libkysdk-datacollect-dev ; modèles dh_make *.ex résiduels | CONSTATÉ |
| cibles construites | `archive:CMakeLists.txt` | add_subdirectory lib/ky-burn-engine, ky-disc-info, ky-udf-image ; install kylin-burner (INSTALL_TARGETS_DEFAULT_ARGS), guide, .desktop, .mo, gschema | CONSTATÉ |
| IPC locale / dbus session | `archive:src/main.cpp` | QLocalServer::listen(LOCAL_SERVER_NAME) (l.65-71) ; D-Bus session com.kylin.burner.manager | CONSTATÉ |

- [CONSTATÉ] setup.sh à la racine exécute 'sudo apt update/install' (outil de développement, non installé).
- [CONSTATÉ] Écart fiche/arbre sur version et format.

### `kylin-user-guide`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ubuntukylin/kylin-user-guide ; format 3.0 (quilt), orig 'amont-inconnu' ; 20 patches
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 396 fichiers, patches quilt)
- **Langages / taille** : C++/Qt5 (CMake), QML, HTML/CSS/JavaScript (jQuery) — 396 fichiers / 105 Mo (médias, patches de 27 Mo) ; ~130 fichiers (~19 000 lignes C++/QML hors tests) — lignes de code propres mesurées : 15 814
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session (2 entrées autostart), ouverture d'URL help:/ghelp:/info:/man: (gestionnaire de schéma), D-Bus session
- **Indices réseau** : Aucun point de terminaison distant identifié ; QLocalServer d'instance unique (introduction/qtlocalpeer.cpp); Moteur QtWebKit rendant du HTML/JS local (src/kywebview.cpp, src/bridge.cpp)
- **Blobs** : introduction/media/01-openkylin.mp4 ; doc/kylin-backup0106.docx ; images
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session lancé automatiquement (2 autostart), gestionnaire de schémas d'URL, avec IPC D-Bus et moteur QtWebKit exécutant du JS local via un pont C++.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/kylin-user-guide.install` | src/data/kylin-user-guide-daemon.desktop -> /etc/xdg/autostart ; introduction/data/ukui-new-function-introduction.desktop -> /etc/xdg/autostart et /usr/share/applications ; src/web -> /usr/share/kylin-user-guide | CONSTATÉ |
| autostart | `archive:src/data/kylin-user-guide-daemon.desktop` | Exec=/usr/bin/user-guide-daemon ; NoDisplay=true | CONSTATÉ |
| autostart | `archive:introduction/data/ukui-new-function-introduction.desktop` | Exec=ukui-new-function-introduction %u ; NoDisplay=true | CONSTATÉ |
| desktop-file (gestionnaire d'URL) | `archive:src/data/kylin-user-guide.desktop` | Exec=/usr/bin/kylin-user-guide -A %u ; MimeType=x-scheme-handler/ghelp;help;info;man | CONSTATÉ |
| debian-install | `archive:debian/kylin-user-guide-common.install` | data -> /usr/share/kylin-user-guide/ ; traductions .qm | CONSTATÉ |
| cibles construites | `archive:CMakeLists.txt ; archive:src/CMakeLists.txt` | kylin-user-guide (Qt5WebKitWidgets), user-guide-daemon (src/daemon), ukui-new-function-introduction (introduction/) ; rules : lrelease | CONSTATÉ |
| dbus (session) | `archive:src/daemon/ipc/daemon_ipc_dbus.cpp` | registerService sur le bus session (l.58-65) | CONSTATÉ |

- [CONSTATÉ] Paquets binaires kylin-user-guide et kylin-user-guide-common installés par défaut.

### `kylin-theme-builder`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Homepage https://gitee.com/openkylin/kylin-theme-builder ; mainteneur 'kevin <kevin@unknown>'
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 370 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) ; Shell ; ressources — 370 fichiers / 37,7 Mo ; 67 fichiers C++ (~15 900 lignes) — lignes de code propres mesurées : 15 862
- **Privilège** : utilisateur de session (outil) ; les paquets générés exécutent des scripts root à l'installation
- **Déclencheurs** : lancement manuel, installation d'un paquet thème généré
- **Indices réseau** : Aucun
- **Blobs** : Images (resource/background/grub-background.png 16 Mo) ; resource/blacklist.txt 838 Ko
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Outil non installé par défaut, mais générateur de .deb dont les scripts gabarits agissent en root sur GRUB/initramfs/plymouth (chemin d'installation).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-file | `archive:kylin-theme-builder.desktop` | Exec=/usr/bin/kylin-theme-builder %u ; X-UKUI-AutoRestart=true | CONSTATÉ |
| gabarit maintainer-script | `archive:resource/debian/postinst` | Gabarit des paquets thème générés : update-grub ; update-initramfs -u | CONSTATÉ |
| gabarit maintainer-script | `archive:resource/debian/postrm` | update-grub ; update-initramfs -u ; update-alternatives --remove-all default.plymouth puis --install openkylin-logo.plymouth ; sed -i sur /usr/share/grub/themes/UKUI/theme.txt | CONSTATÉ |
| debian-rules | `archive:debian/rules ; archive:resource/debian/rules` | 'dh $@' (paquet et gabarit) | CONSTATÉ |
| desktop-file / script | `archive:resource/config/kylin-theme-builder-change.desktop ; archive:resource/config/kylin-theme-builder-change` | Exec=/usr/bin/kylin-theme-builder-change ; script shell POSIX (contenu non lu) | NON COUVERT |
| debian-control | `archive:debian/control` | Depends debhelper, meson, ninja-build, devscripts, build-essential, x11-apps | CONSTATÉ |
| cible construite | `archive:kylin-theme-builder.pro` | target /usr/bin ; INSTALLS target qm_files appdesktop ; PKGCONFIG kysdk-datacollect | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO.

### `ukui-clock`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Homepage https://github.com/ukui/ukui-clock ; cJSON et CJsonObject embarqués
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 259 fichiers)
- **Langages / taille** : C++/Qt5 (qmake) ; C (cJSON) — 259 fichiers / 14,4 Mo (polices) ; ~104 fichiers (~24 900 lignes) — lignes de code propres mesurées : 27 051
- **Privilège** : utilisateur de session
- **Déclencheurs** : lancement manuel, activation D-Bus session org.kylin.dbus.ukuiclock
- **Indices réseau** : QLocalServer d'instance unique (singleApplication.cpp) ; aucun réseau
- **Blobs** : Polices embarquées (siyuanheiti_CN_Medium.otf 8,4 Mo, HuaKangJinGangHei-Regular-2.ttf, DFPKingGothicGB-Semibold-2.ttf) ; ui_*.h générés versionnés
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Application de session exposant des méthodes D-Bus de manipulation d'alarmes et un .service D-Bus pointant vers kaiming ; pas de réseau ni de privilège.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-service (session) | `archive:debian/ukui-clock/usr/share/dbus-1/services/org.kylin.dbus.ukuiclock.service` | Fichier situé dans le répertoire de staging debian/ukui-clock/ mais installé par ukui-clock.pro:42-43 dans /usr/share/dbus-1/services ; Name=org.kylin.dbus.ukuiclock ; Exec=/opt/kaiming-tools/bin/kaiming run --command=/opt/apps/top.openkylin.ukui-clock/files/bin/ukui-clock top.openkylin.ukui-clock ; Timeout=1000 | CONSTATÉ |
| adaptateurs D-Bus | `archive:dbusdeleteclockbyidadaptor.cpp ; dbusupdateclockadaptor.cpp ; dbusselectclockbyidadaptor.cpp ; dbusnoticeshowagainadaptor.cpp ; dbusbaseclockadaptor.cpp` | Méthodes CRUD d'alarmes exposées sur le bus session (nom CLOCK_DBUS_SERVICE_NAME, ClockInterface.h:22) | CONSTATÉ |
| desktop-file | `archive:ukui-clock.desktop` | Exec=/usr/bin/ukui-clock | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' ; aucun script mainteneur | CONSTATÉ |

- [CONSTATÉ] Le .service D-Bus installé hors build kaiming pointe vers /opt/kaiming-tools/bin/kaiming.
- [CONSTATÉ] kabase/buriedpoint.hpp présent.

### `ukui-notebook`

- **Nature** : propre-kylin — **Amont** : KylinSoft ; Vcs https://github.com/ukui/ukui-sidebar.git ; dans_debian=1
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 240 fichiers)
- **Langages / taille** : C++/Qt5 (CMake) — 240 fichiers / 1,3 Mo ; 82 fichiers (~9 900 lignes) — lignes de code propres mesurées : 11 906
- **Privilège** : utilisateur de session ; postrm root
- **Déclencheurs** : activation D-Bus session org.ukui.note, lancement manuel, suppression du paquet
- **Indices réseau** : Aucun
- **Blobs** : Aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Petit composant de session exposant un serveur D-Bus activable, installant une politique D-Bus système permettant à tout utilisateur de posséder org.ukui.note, et dont le postrm agit dans tous les /home.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-policy (système) | `archive:data/org.ukui.note.conf` | Installée dans /etc/dbus-1/system.d (uNoteDbusServer/CMakeLists.txt:42) ; user=root : own org.ukui.note, send_interface org.ukui.note.interface ; context=default : allow own=org.ukui.note, allow send_destination=org.ukui.note | CONSTATÉ |
| dbus-service (session) | `archive:data/org.ukui.note.service` | Installé dans /usr/share/dbus-1/services ; Name=org.ukui.note ; Exec=/usr/bin/ukui-notebook-server ; serveur enregistré sur le bus session (uNoteDbusServer/main.cpp:32) ; méthode keywordMatch (data/org.ukui.note.xml) | CONSTATÉ |
| maintainer-script | `archive:debian/postrm` | Pour chaque entrée de /home : rm ~/.config/ukui/Clock_database.db et ~/.config/ukui/url.conf | CONSTATÉ |
| desktop-file | `archive:data/ukui-notebook.desktop.in` | Exec=/usr/bin/ukui-notebook | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; -Wall -pedantic | CONSTATÉ |

- [CONSTATÉ] Politique installée pour le bus système alors que le serveur s'enregistre sur le bus session.
- [CONSTATÉ] Le postrm supprime Clock_database.db, nom de base associé à ukui-clock.

### `kylin-time`

- **Nature** : propre-kylin — **Amont** : Réécriture Rust de GNU time par Kylin (Cargo.toml authors cite les auteurs GNU time) ; aucune forge (Gitee/GitHub ABSENT)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 18 fichiers ; 3.0 quilt, .pc/applied-patches vide)
- **Langages / taille** : Rust (cargo) — 18 fichiers / 40 Ko ; 5 fichiers .rs (784 lignes) — lignes de code propres mesurées : 788
- **Privilège** : utilisateur (outil en ligne de commande)
- **Déclencheurs** : invocation en ligne de commande
- **Indices réseau** : Au build : téléchargement des crates depuis le miroir Aliyun (pas de vendoring)
- **Blobs** : Aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Code minuscule (784 lignes) qui remplace l'utilitaire de base 'time' et dont la construction dépend d'un miroir de crates tiers non vendorisé : lecture exhaustive peu coûteuse et vérification de la chaîne d'approvisionnement.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Replaces: time ; Provides: time ; Conflicts: time ; paquet kylin-time | CONSTATÉ |
| debian-rules | `archive:debian/rules` | cargo build --release avec CARGO_HOME=debian/cargo-home ; install target/release/kylin-time -> /usr/bin/kylin-time ; cargo test ... \|\| true | CONSTATÉ |
| configuration de build | `archive:.cargo/config.toml` | source crates-io remplacée par sparse+https://mirrors.aliyun.com/crates.io-index/ ; [http] check-revoke = false ; git-fetch-with-cli = true | CONSTATÉ |
| manifeste | `archive:Cargo.toml ; archive:Cargo.lock` | Dépendances libc 0.2, nix 0.29 (Cargo.toml) ; Cargo.lock fige nix 0.26.4 (écart) ; aucune crate vendorisée | CONSTATÉ |

- [CONSTATÉ] Installé par défaut par l'ISO (kylin-time=1.9.0-ok1).

### `kylincameralibs`

- **Nature** : absent-ou-vide — **Amont** : KylinSoft (mainteneur binzemin@kylinos.cn) ; Homepage http://www.kylinos.cn ; forges ABSENT
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun (squelette dh_make) ; meta.source_coquille_vide=1
- **Langages / taille** : aucun — 19 fichiers / 25,7 Ko, uniquement debian/ — lignes de code propres mesurées : 0
- **Privilège** : bibliothèque chargée dans kylin-camera (session)
- **Déclencheurs** : lancement de kylin-camera
- **Indices réseau** : inconnus (pas de code)
- **Blobs** : Aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Squelette sans source pour une bibliothèque de capture installée par défaut et liée par kylin-camera : non auditable sans le binaire.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | paquet kylincameralibs, Multi-Arch foreign, 'kylin camera libraries for developing camera app' | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' | CONSTATÉ |
| binaire installé | `(aucun fichier source)` | kylincameralibs=1.0.0.1-1ok10 installé par l'ISO ; fournit libkylincamera liée par kylin-camera (-lkylincamera) | NON COUVERT |

- [CONSTATÉ] Changelog 'Initial Release' huanghe-proposed (21 août 2026).

### `wemeet`

- **Nature** : absent-ou-vide — **Amont** : Tencent Wemeet (Tencent Meeting), logiciel propriétaire ; Maintainer robertylu@tencent.com ; Homepage https://wemeet.qq.com ; Vendor: Wemeet ; forges ABSENT
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun (squelette dh_make) ; meta.source_coquille_vide=0 bien que l'arbre soit un squelette
- **Langages / taille** : aucun — 19 fichiers / 25,8 Ko, uniquement debian/ — lignes de code propres mesurées : 0
- **Privilège** : inconnu
- **Déclencheurs** : inconnus
- **Indices réseau** : Client de visioconférence (réseau par nature, PROBABLE) ; aucun code
- **Blobs** : Aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Squelette sans code pour un client propriétaire arm64 non installé par défaut ; non auditable en source.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | paquet wemeet, Architecture arm64, Section misc ; description de visioconférence | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' ; modèles *.ex inactifs | CONSTATÉ |

- [CONSTATÉ] Non installé par défaut par l'ISO.
- [CONSTATÉ] Incohérence : meta.source_coquille_vide=0 alors que l'arbre ne contient que des fichiers dh_make.

### `fcitx5-gb18030`

- **Nature** : propre-kylin — **Amont** : KylinSoft ('Copyright 2023 KylinSoft Co., Ltd.' dans src/gb18030.cpp) ; addon Fcitx5
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 18 fichiers)
- **Langages / taille** : C++ (CMake) — 18 fichiers / 34 Ko ; 443 lignes C++ — lignes de code propres mesurées : 443
- **Privilège** : session : chargé dans le processus fcitx5 et reçoit les frappes clavier
- **Déclencheurs** : sélection de la méthode d'entrée GB18030 (chargement à la demande)
- **Indices réseau** : Aucun
- **Blobs** : Aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Addon de méthode d'entrée minuscule (443 lignes) exécuté dans fcitx5 sur le flux des frappes : lecture exhaustive très peu coûteuse.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cible construite | `archive:src/CMakeLists.txt` | add_library(gb18030 SHARED) -> ${FCITX_INSTALL_LIBDIR}/fcitx5 ; gb18030.conf -> addon/ ; gb18030.conf -> inputmethod/ | CONSTATÉ |
| configuration addon | `archive:src/gb18030-addon.conf.in.in` | Category=InputMethod ; Library=libgb18030 ; Type=SharedLibrary ; OnDemand=True | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | 'dh $@' ; paquet fcitx5-gb18030 ; aucun script mainteneur ; debian/patches/series vide | CONSTATÉ |

- [CONSTATÉ] debian/copyright avec 'Source: <url://example.com>' (gabarit).

### `fcitx5-table-ukk`

- **Nature** : meta-ou-donnees — **Amont** : Dérivé de fcitx5-table-other (fcitx ; CMakeLists, po/fcitx5-table-other.pot, metainfo TableOther) : tables stuyghur, stkazakh, stkyrgiz ; mainteneur 'kevin <kevin@unknown>'
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (données + CMake, 145 fichiers)
- **Langages / taille** : données de tables, CMake — 145 fichiers / 318 Ko — lignes de code propres mesurées : 11
- **Privilège** : données chargées par fcitx5 (session)
- **Déclencheurs** : sélection de la table dans fcitx5
- **Indices réseau** : Aucun
- **Blobs** : tables/*.main.dict (données binaires ~9 Ko, versionnées) ; artefacts de build versionnés (tables/CMakeFiles, Makefile, cmake_install.cmake)
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Données de méthode d'entrée sans code exécutable propre ; vérifier la correspondance des sources .txt avec les .main.dict versionnés.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | 'dh $@' ; paquet fcitx5-table-ukk ; Homepage erronée (https://gitee.com/openkylin/kylin-theme-builder) ; aucun script mainteneur | CONSTATÉ |
| build des tables | `archive:tables/CMakeLists.txt` | TABLE_NAMES ug_CN/stuyghur, kz_CN/stkazakh, kg_CN/stkyrgiz (compilation libime) | CONSTATÉ |

- [CONSTATÉ] Artefacts de build CMake versionnés dans tables/.

### `fcitx5-qt`

- **Nature** : fork-amont — **Amont** : fcitx5-qt 5.1.9 (https://github.com/fcitx/fcitx5-qt ; CMakeLists project VERSION 5.1.9 ; Homepage debian/control) ; historique openKylin depuis 5.0.14-0ok0 (2022)
- **Installé par défaut (ISO 3.0)** : oui (4 binaire(s)) — **Code disponible** : archive (source complète, format 3.0 native : delta Kylin non séparé)
- **Langages / taille** : C++/Qt (CMake) — 191 fichiers / 781 Ko ; ~20 700 lignes (hors qt4) ; 50 liens symboliques internes qt6 -> qt5 — lignes de code propres mesurées : 17 206
- **Privilège** : session : code chargé dans chaque processus Qt utilisant le module d'entrée fcitx
- **Déclencheurs** : démarrage de toute application Qt5/Qt6 avec le contexte d'entrée fcitx (PROBABLE)
- **Indices réseau** : Aucun ; IPC D-Bus session vers fcitx5 (PROBABLE)
- **Blobs** : Aucun
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Fork amont de taille moyenne chargé dans toutes les applications Qt : isoler le delta Kylin par diff avec fcitx5-qt 5.1.9 puis lire intégralement ce delta.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/fcitx5-frontend-qt5.install ; archive:debian/fcitx5-frontend-qt6.install` | Plugins usr/lib/*/qt5\|qt6/plugins/platforminputcontexts/* (chargés dans toute application Qt) ; /usr/libexec/fcitx5-qt5\|6-gui-wrapper ; fcitx5-qt5\|6-immodule-probing ; org.fcitx.fcitx5-qt5-gui-wrapper.desktop | CONSTATÉ |
| debian-install | `archive:debian/libfcitx5-qt1.install ; libfcitx5-qt6-1.install ; libfcitx5-qt-data.install` | libFcitx5Qt5*.so.*, libFcitx5Qt6*.so.*, locales | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; -DCMAKE_BUILD_RPATH_USE_ORIGIN=ON -DENABLE_QT4=Off ; aucun script mainteneur | CONSTATÉ |

- [CONSTATÉ] Changelog : traitement spécifique du keycode 248 pour le mode clavier virtuel (modification Kylin).
- [CONSTATÉ] Paquets fcitx5-frontend-qt5, libfcitx5-qt1, libfcitx5-qt6-1, libfcitx5-qt-data installés par défaut.

### `fcitx5-gtk`

- **Nature** : fork-amont — **Amont** : fcitx5-gtk 5.1.3 (https://github.com/fcitx/fcitx5-gtk ; CMakeLists project VERSION 5.1.3) ; packaging hérité de Debian (Boyuan Yang, 2019-2020) ; openKylin depuis 5.0.12-1ok1
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète, format 3.0 native ; debian/patches/series vide)
- **Langages / taille** : C/C++ (CMake), GObject — 57 fichiers / 419 Ko ; ~12 400 lignes ; 13 liens symboliques internes — lignes de code propres mesurées : 9 877
- **Privilège** : session : code chargé dans chaque processus GTK2/GTK3
- **Déclencheurs** : démarrage de toute application GTK avec GTK_IM_MODULE=fcitx (PROBABLE)
- **Indices réseau** : Aucun ; client D-Bus session de fcitx5 (fcitx-gclient, PROBABLE)
- **Blobs** : Aucun
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Fork amont compact chargé dans toutes les applications GTK : isoler le delta Kylin par diff avec fcitx5-gtk 5.1.3 puis lire intégralement ce delta.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/fcitx5-frontend-gtk2.install ; archive:debian/fcitx5-frontend-gtk3.install` | usr/lib/*/gtk-2.0\|3.0/*/immodules/im-*.so (chargés dans toute application GTK2/3) ; fcitx5-gtk2\|3-immodule-probing | CONSTATÉ |
| debian-install | `archive:debian/libfcitx5gclient2.install ; gir1.2-fcitxg-1.0.install` | libFcitx5GClient .so.2 / .so.5.* ; typelib GObject-Introspection | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; -DENABLE_GTK4_IM_MODULE=OFF ; aucun script mainteneur | CONSTATÉ |

- [CONSTATÉ] Paquets fcitx5-frontend-gtk2/gtk3 et libfcitx5gclient2 installés par défaut.

### `ukui-globaltheme`

- **Nature** : meta-ou-donnees — **Amont** : KylinSoft ; Vcs https://gitee.com/openkylin/ukui-globaltheme
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (thèmes CSS/YAML + script de build)
- **Langages / taille** : CSS, YAML, Shell (dtConvert.sh), CMake — 42 fichiers / 6,8 Mo — lignes de code propres mesurées : 155
- **Privilège** : données ; postinst root
- **Déclencheurs** : installation du paquet, chargement des thèmes par qt5-styles-ukui (PROBABLE)
- **Indices réseau** : Aucun
- **Blobs** : Images preview.png
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Données de thème ; seuls le postinst et dtConvert.sh, qui génère des fichiers QML installés, méritent lecture.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/postinst` | #!/bin/bash sans set -e ; rm -rf /usr/share/config/globaltheme/token si présent | CONSTATÉ |
| debian-install | `archive:debian/ukui-globaltheme-*.install (5 fichiers)` | Thèmes -> /usr/share/config/globaltheme ; *.css et *.qml générés -> /usr/share/config/themeconfig/token ; gschema -> /usr/share/glib-2.0/schemas | CONSTATÉ |
| build | `archive:globaltheme/CMakeLists.txt` | chmod +x puis exécute dtConvert.sh (155 lignes) sur chaque .css à la configuration ; installe les .qml produits | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ --buildsystem cmake | CONSTATÉ |

- [CONSTATÉ] Les fichiers QML installés sont générés au build et absents de l'arbre.

### `openkylin-language-pack-fr`

- **Nature** : meta-ou-donnees — **Amont** : KylinSoft (Kevin Duan) ; outil tiers Merge-Qt-TS-files embarqué (tool/)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (outils Python/Qt ; aucun fichier de traduction présent)
- **Langages / taille** : Python, C++/Qt (outil), Makefile — 26 fichiers / 1,5 Mo — lignes de code propres mesurées : 1 298
- **Privilège** : root (postinst)
- **Déclencheurs** : installation du paquet
- **Indices réseau** : Aucun
- **Blobs** : tool/build-CombineTranslationFiles-unknown-Debug/CombineTranslationFiles (ELF 1,3 Mo, build Debug versionné, non installé)
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Paquet de traduction quasi vide (aucune traduction installée) dont le seul effet est la modification de /etc/locale.gen par le postinst.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/postinst` | set -e ; si 'fr_FR.UTF-8 UTF-8' absent : sed -i '$a fr_FR.UTF-8 UTF-8' /etc/locale.gen ; locale-gen | CONSTATÉ |
| debian-rules | `archive:debian/rules` | cible binary manuelle : make install-mo puis dh_installdeb … dh_builddeb | CONSTATÉ |
| build | `archive:Makefile` | install-mo crée seulement /usr/share/locale-langpack/fr/LC_MESSAGES/ (ligne install commentée) ; répertoire target/po absent de l'arbre | CONSTATÉ |
| debian-control | `archive:debian/control` | Architecture all ; Pre-Depends locales ; Description 'Translation update for language Chinese' | CONSTATÉ |

- [PROBABLE] Installé par défaut par l'ISO alors qu'il n'installe aucun fichier de traduction.

### `kylin-theme`

- **Nature** : meta-ou-donnees — **Amont** : KylinSoft ; Vcs https://gitee.com/openkylin/openkylin-theme.git ; forges Gitee/GitHub ABSENT
- **Installé par défaut (ISO 3.0)** : oui (6 binaire(s)) — **Code disponible** : archive (45 091 fichiers, essentiellement icônes/thèmes ; quelques scripts et un module C)
- **Langages / taille** : SVG/PNG, Shell, C (54 lignes), meson — 45 091 fichiers / 256,7 Mo ; code : grub.d (1 script), hook initramfs (1), ukuireload.c (54 lignes), scripts mainteneur (3) — lignes de code propres mesurées : 1 110
- **Privilège** : root (scripts mainteneur, grub.d, hook initramfs) ; module GTK chargé dans les applications GTK3 de session
- **Déclencheurs** : installation/suppression des paquets, update-grub, update-initramfs, démarrage d'applications GTK3 si le module est activé (GTK_MODULES via /etc/profile, PROBABLE)
- **Indices réseau** : Aucun
- **Blobs** : Polices GRUB .pf2 (unifont 2,4 Mo) ; icônes SVG/PNG
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Lecture exhaustive des ~10 fichiers de code (grub.d, hook initramfs, module GTK, scripts mainteneur) touchant la chaîne de démarrage ; les 45 000 fichiers de données sont seulement triés.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| grub.d | `archive:configrue/openkylin-grub-theme/06_openkylin_theme` | Installé dans /etc/grub.d (meson.build, paquet kylin-theme-configure) ; exécuté en root par update-grub : cp -r /usr/share/grub/themes/* /boot/grub/themes/ ; émet insmod gfxmenu, loadfont, set theme | CONSTATÉ |
| initramfs-hook | `archive:data/initramfs-tools/hooks/chinese` | Installé sous /usr/share/initramfs-tools/hooks ; copie DroidSansFallbackFull.ttf dans l'initrd si /sbin/plymouthd existe | CONSTATÉ |
| module GTK3 | `archive:highlight/ukuireload.c ; archive:highlight/meson.build` | shared_module ukuireload-gtk-module -> /usr/lib/<arch>-linux-gnu/gtk-3.0/modules (kylin-theme-configure.install) | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-theme.postinst` | update-alternatives default.plymouth/text.plymouth (openkylin-logo/openkylin-text) ; update-grub ; glib-compile-schemas ; update-initramfs -u ; update-alternatives x-cursor-theme ; rm -rf /usr/share/icons/ukui-icon-theme-HeYin et -lightseeking ; exécute /usr/share/kylin-system-updater/kylin-reboot-required | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-theme.prerm` | update-alternatives --remove (plymouth) ; branche dupliquée 'remove' contenant update-alternatives --remove x-cursor-theme et sed -i '/ukuireload-gtk-module/d' /etc/profile (inatteignable car 'remove' déjà traité, PROBABLE) | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-theme.postrm` | remove : update-grub ; update-initramfs -u | CONSTATÉ |
| debian-install | `archive:debian/kylin-*.install (5 fichiers)` | kylin-initiate-theme : usr/share/grub, initramfs-tools, plymouth ; kylin-theme-configure : etc/grub.d, glib-2.0, gtk-3.0/modules/*.so ; kylin-theme : icônes, fcitx-qimpanel, mime | CONSTATÉ |
| thèmes plymouth | `archive:data/plymouth/themes/` | Scripts plymouth non lus | NON COUVERT |

- [CONSTATÉ] 2 876 liens symboliques absolus vers /home/like/work/t/ubuntukylin-theme/… (pendants).
- [CONSTATÉ] 6 paquets binaires installés par défaut (kylin-gtk-theme, kylin-initiate-theme, kylin-sounds-theme, kylin-theme, kylin-theme-configure, openkylin-theme).

### `kylin-wallpapers`

- **Nature** : meta-ou-donnees — **Amont** : KylinSoft ; Vcs https://github.com/ubuntukylin/ubuntukylin-wallpapers.git ; forges ABSENT
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (images et XML)
- **Langages / taille** : images, XML — 47 fichiers / 373 Mo — lignes de code propres mesurées : 0
- **Privilège** : données ; postinst root
- **Déclencheurs** : installation du paquet
- **Indices réseau** : Aucun
- **Blobs** : usr/share/backgrounds/1-openkylin.pag (animation PAG, lue par libpag du paquet pag, PROBABLE) ; images jusqu'à 32 Mo
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Fonds d'écran et métadonnées XML sans code ; seul le fichier .pag constitue une entrée de parseur (audit côté pag).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/kylin-wallpapers.postinst` | configure : touch /usr/share/backgrounds/1-openkylin.jpg | CONSTATÉ |
| debian-install | `archive:debian/kylin-wallpapers.install ; archive:debian/kylin-wallpapers-yangtze.install` | usr/share/backgrounds/* (dont 1-openkylin.pag), gnome-background-properties, ukui-background-properties, gschema override 30_openkylin-wallpapers | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' | CONSTATÉ |

### `fonts-ukk`

- **Nature** : meta-ou-donnees — **Amont** : Polices tierces (métadonnées TrueType : Xinjiang R&D Center for Ethnic Minority Information Technology ; Uyghur Computer Science Association) ; mainteneur 'kevin <kevin@unknown>'
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (polices + configuration fontconfig)
- **Langages / taille** : TTF, XML fontconfig — 35 fichiers / 4,8 Mo ; 27 TTF dont 23 installés — lignes de code propres mesurées : 0
- **Privilège** : données ; postinst root
- **Déclencheurs** : installation du paquet
- **Indices réseau** : Aucun
- **Blobs** : Polices TrueType
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Polices et configuration fontconfig sans code.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/postinst` | #!/bin/bash ; fc-cache -f | CONSTATÉ |
| debian-install | `archive:debian/install` | 23 TTF -> /usr/share/fonts/ukk/ ; 65-ukk.conf -> /etc/fonts/conf.avail/ | CONSTATÉ |
| fontconfig | `archive:65-ukk.conf` | Alias <prefer> pour chaque famille UKK_* | CONSTATÉ |

### `gb-fonts`

- **Nature** : meta-ou-donnees — **Amont** : Polices « 国标 » GB18030-2022 (éditeur non indiqué dans debian/) ; mainteneur linyuxuan@kylinos.cn
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (polices)
- **Langages / taille** : TTF — 20 fichiers / 150,6 Mo ; 11 TTF — lignes de code propres mesurées : 0
- **Privilège** : données
- **Déclencheurs** : installation du paquet
- **Indices réseau** : Aucun
- **Blobs** : Polices TrueType (jusqu'à 44 Mo)
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Polices sans code ni script.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install | `archive:debian/gb-fonts.install` | 11 TTF -> /usr/share/fonts/gb/ | CONSTATÉ |
| debian-rules / control | `archive:debian/rules ; archive:debian/control` | 'dh $@' ; aucun script mainteneur | CONSTATÉ |

### `pag`

- **Nature** : fork-amont — **Amont** : Tencent libpag 4.5.12 (libpag/DEPS version 1.3.12) avec tgfx et libavc (github.com/libpag/*), Tencent ffavc ; DEPS liste aussi rttr, harfbuzz, lz4 ; delta Kylin d'après debian/changelog : fusion ffavc + libpag, support MP4, décodage vidéo de fond d'écran animé et rendu zero-copy (4.5.12-ok1 à ok7)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 6 148 fichiers, format 3.0 native : delta non séparé)
- **Langages / taille** : C++, C, assembleur, CMake — 6 148 fichiers / 85,6 Mo ; ~67 000 lignes dans libpag/src + ffavc/src, plus code tiers — lignes de code propres mesurées : 107 550
- **Privilège** : bibliothèque chargée par des processus de session (fond d'écran animé, PROBABLE)
- **Déclencheurs** : lecture de fichiers .pag/.mp4 (ex. usr/share/backgrounds/1-openkylin.pag de kylin-wallpapers)
- **Indices réseau** : libpag/third_party/tgfx/src/inspect/Socket.cpp:702-707 bind INADDR_ANY (inspecteur TGFX) compilé seulement si TGFX_USE_INSPECTOR, option activée uniquement en build Debug (tgfx/CMakeLists.txt:31-33) ; debian/rules force Release -> non compilé (PROBABLE)
- **Blobs** : Aucun blob exécutable détecté ; assets .pag
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Fork amont très volumineux de parseurs média installé par défaut : isoler le delta Kylin (MP4/zero-copy) par diff avec Tencent libpag 4.5.12 et ffavc, L3 sur ce delta, sans relecture du code tiers inchangé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | cmake Release, BUILD_SHARED_LIBS=ON, PAG_USE_FFAVC=ON ; -Wl,-z,now ; -include cstdint | CONSTATÉ |
| debian-install | `archive:debian/libpag.install ; libffavc.install ; libtgfx-dev.install` | libpag.so*, libffavc.so*, libtgfx.a et en-têtes ; aucun script mainteneur | CONSTATÉ |
| packaging tiers embarqué | `archive:libpag/third_party/tgfx/third_party/highway/debian/rules ; debian/control` | Répertoire debian de highway (non utilisé par le build principal, PROBABLE) | CONSTATÉ |

- [CONSTATÉ] Paquets libpag et libffavc installés par défaut ; présent sur Gitee seulement (GitHub ABSENT).

### `thorvg`

- **Nature** : fork-amont — **Amont** : ThorVG 1.1.0 (https://github.com/thorvg/thorvg ; debian/upstream/metadata ; meson.build version 1.1.0) ; packaging dérivé de Debian thorvg 1.0.6+dfsg-1 (debian/README.openkylin.md)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète ; 3.0 quilt ; .pc/applied-patches vide : aucun patch source)
- **Langages / taille** : C++ (meson) — 824 fichiers / 13,9 Mo ; ~232 000 lignes dans src/ — lignes de code propres mesurées : 267 756
- **Privilège** : bibliothèque (parseurs SVG, Lottie, PNG, JPG, WebP, polices)
- **Déclencheurs** : rendu de fichiers vectoriels/Lottie par les applications liées
- **Indices réseau** : Aucun
- **Blobs** : Ressources de test (test13.lot, images)
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Fork P2-B sans patch source (delta limité à debian/) : vérifier l'identité avec l'archive amont 1.1.0 et les options de build.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | meson -Dloaders=all -Dsavers=all -Dengines=cpu,gl -Dbindings=capi -Dtools=all -Dtests=true ; hardening=+all, -z relro -z now ; LTO désactivé | CONSTATÉ |
| debian-install | `archive:debian/libthorvg1.install ; thorvg-tools.install ; libthorvg-dev.install` | libthorvg-1.so.1* ; usr/bin (outils svg2png, lottie2gif) ; en-têtes et pkgconfig ; aucun script mainteneur | CONSTATÉ |

- [CONSTATÉ] libthorvg1 installé par défaut ; Gitee seulement.

### `ukui-touch-settings-plugin`

- **Nature** : propre-kylin — **Amont** : KylinSoft (Copyright Tianjin KYLIN) ; Homepage https://www.ukui.org/ ; format 3.0 (quilt), orig 'amont-inconnu' ; 67 patches appliqués (.pc/applied-patches)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 346 fichiers, patches appliqués)
- **Langages / taille** : C++/Qt5 (qmake) — 346 fichiers / 125,8 Mo (GIF) ; 82 fichiers C++ (~9 500 lignes) — lignes de code propres mesurées : 9 582
- **Privilège** : session : plugins chargés dans ukui-control-center ; preinst root
- **Déclencheurs** : ouverture des pages pavé tactile/écran tactile/stylet du centre de contrôle, installation du paquet (dpkg-divert)
- **Indices réseau** : Aucun
- **Blobs** : GIF d'animation (jusqu'à 9,5 Mo)
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Plugins de configuration de session sans service ni réseau ; vérifier le dpkg-divert remplaçant un plugin d'ukui-control-center et le contenu des 67 patches.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/ukui-touch-settings-plugin.preinst` | dpkg-divert --package ukui-touch-settings-plugin --rename --divert …/ukui-control-center/libtouchpad.so.old --add …/ukui-control-center/libtouchpad.so | CONSTATÉ |
| debian-install | `archive:debian/ukui-touch-settings-plugin.install ; archive:debian/ukui-stylus-settings-plugin.install` | usr/lib/*/ukui-control-center/libtouchpad*, libtouchscreen*, libstylus* (plugins chargés par ukui-control-center) ; données touchpad/touchscreen/stylus-settings | CONSTATÉ |
| cibles construites | `archive:ukui-touch-settings-plugin.pro ; archive:src/*/*.pro` | SUBDIRS touchpad-settings, stylus-settings, touchscreen-settings (TEMPLATE=lib, target.path=$$[QT_INSTALL_LIBS]/ukui-control-center) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' | CONSTATÉ |

- [CONSTATÉ] Répertoires src/config-monitor, src/gesture-widget, src/native-filter absents des SUBDIRS du .pro principal.

#### Constats transverses du lot B14-apps-kylin

- **[CONSTATÉ]** 4 paquets du lot ne contiennent qu'un squelette dh_make (modèles *.ex, rules '%: dh $@') et n'ont aucune forge : kylin-screencap, kylin-sane-virtual-backend, kylincameralibs, wemeet. 3 d'entre eux ont des binaires installés par défaut par l'ISO (kylin-screencap ; kylin-sane-virtual-backend et -app ; kylincameralibs). — *preuve* : archive:kylin-screencap/debian/rules, archive:kylincameralibs/debian/rules ('%:\n\tdh $@') ; archive:*/debian/postinst.ex ; fiche installe_par_defaut_iso
- **[CONSTATÉ]** 3 points d'entrée s'exécutent en root via le système : miracle-wifid.service (kylin-miraclecast, sans User=), kylin-printer-applet-dbus.service (kylin-printer, sans User=, Restart=always, activé par postinst) et le service D-Bus système com.kylin.inputdevice.manager (kylin-screenshot, User=root). S'y ajoute une règle udev RUN+= root (kylin-printer). — *preuve* : archive:kylin-miraclecast/res/miracle-wifid.service.cmake ; archive:kylin-printer/data/kylin-printer-applet-dbus.service ; archive:kylin-screenshot/inputdevice_manager/com.kylin.inputdevice.manager.service ('User=root') ; archive:kylin-printer/data/99-kylin-printer-applet.rules ('RUN+=/usr/share/kylin-printer/kylin-printer-applet-start usbadd_%p')
- **[CONSTATÉ]** 3 politiques D-Bus système du lot ouvrent des droits au contexte default : 15 méthodes org.freedesktop.miracle.wifi.ui (kylin-miraclecast), allow send_interface sans send_destination (kylin-screenshot/inputdevice_manager), allow own=org.ukui.note (ukui-notebook). — *preuve* : archive:kylin-miraclecast/res/org.freedesktop.miracle.conf ; archive:kylin-screenshot/inputdevice_manager/com.kylin.inputdevice.manager.conf ('<allow send_interface="com.kylin.inputdevice.manager"/>') ; archive:ukui-notebook/data/org.ukui.note.conf ('<policy context="default"> <allow own="org.ukui.note"/>')
- **[CONSTATÉ]** libkysdk-datacollect-dev figure dans les Build-Depends de 15 paquets du lot ; kysdk-datacollect est effectivement lié (PKGCONFIG/pkg_check_modules) dans 12 : kylin-weather, kylin-ipmsg, kylin-recorder, kylin-scanner, kylin-music, kylin-video, kylin-photo-viewer, kylin-calculator, kylin-calendar, kylin-burner, kylin-user-guide, kylin-theme-builder. — *preuve* : ex. archive:kylin-weather/kylin-weather.pro ('PKGCONFIG += ... kysdk-datacollect') ; archive:kylin-calendar/CMakeLists.txt:39 ('pkg_check_modules(KYSDK_DATACOLLECT REQUIRED kysdk-datacollect)')
- **[CONSTATÉ]** Points de terminaison distants codés en dur, dont plusieurs en HTTP clair : weather.ubuntukylin.com:8001, myip.ipip.net, restapi.amap.com, whois.pconline.com.cn (kylin-weather) ; www.safe.gov.cn (kylin-calculator) ; api.kylinos.cn et adresse privée 172.20.184.183:8888 (kylin-printer) ; api.kylinos.cn et kom.kylinos.cn (kylin-scanner) ; api.imgur.com (kylin-screenshot). — *preuve* : archive:kylin-weather/global/corevar.cpp:28-40 ; archive:kylin-calculator/src/toolmodel.cpp:1079 ; archive:kylin-printer/device_manager/device_information.cpp:190 ; archive:kylin-scanner/src/device/base_info.cpp:35 ; archive:kylin-screenshot/kylin_screenshot/src/tools/imgur/imguruploader.cpp:113
- **[CONSTATÉ]** Scripts mainteneur modifiant des fichiers d'autres paquets ou de la configuration système : kylin-printer (sed sur /lib/udev/udev-add-printer), kylin-miraclecast (mv libprojection.so dans le répertoire de plugins ukui-control-center), ukui-touch-settings-plugin (dpkg-divert de libtouchpad.so), kylin-theme (sed sur /etc/profile), openkylin-language-pack-fr (sed sur /etc/locale.gen). — *preuve* : archive:kylin-printer/debian/postinst ; archive:kylin-miraclecast/debian/postinst ; archive:ukui-touch-settings-plugin/debian/ukui-touch-settings-plugin.preinst ; archive:kylin-theme/debian/kylin-theme.prerm ; archive:openkylin-language-pack-fr/debian/postinst
- **[CONSTATÉ]** 2 scripts postrm parcourent tous les répertoires de /home et y suppriment des fichiers : kylin-music (~/.config/.kylin_music_ver*_mymusic.db) et ukui-notebook (~/.config/ukui/Clock_database.db, url.conf). — *preuve* : archive:kylin-music/debian/kylin-music.postrm ('for user in /home/*') ; archive:ukui-notebook/debian/postrm ('for file in $(ls /home)')
- **[CONSTATÉ]** 3 scripts mainteneur portent un nom ne correspondant à aucun paquet binaire déclaré, si bien que dh_installdeb ne les intégrerait pas (effet PROBABLE) : kylin-font-view.postinst (paquet kylin-font-viewer), kylin-calendar-plugin.postinst (paquet kylin-calendar), miraclecast-frontend.postinst (paquet kylin-miraclecast-frontend). — *preuve* : archive:kylin-font-viewer/debian/control:32 ('Package: kylin-font-viewer') vs debian/kylin-font-view.postinst ; archive:kylin-calendar/debian/control:29 ; archive:kylin-miraclecast-frontend/debian/control:18
- **[CONSTATÉ]** 4 paquets en format 3.0 (native) contiennent un répertoire debian/patches que dpkg-source n'applique pas : kylin-music (63), kylin-calendar (41), kylin-recorder (34), kylin-miraclecast-frontend (4). — *preuve* : archive:kylin-music/debian/source/format ('3.0 (native)') + debian/patches/series (63 entrées)
- **[CONSTATÉ]** Des binaires sont versionnés dans les arbres sources : ELF kylin-camera/camera (13,2 Mo, avec debug_info), kylin-scanner/test/kylin-scanner (4 Mo), openkylin-language-pack-fr/tool/build-CombineTranslationFiles-unknown-Debug/CombineTranslationFiles (1,3 Mo) ; un .deb dans kylin-printer/tests ; un modèle ONNX de 25,9 Mo installé par kylin-photo-viewer. — *preuve* : file archive:kylin-camera/camera -> 'ELF 64-bit LSB pie executable … with debug_info, not stripped' ; archive:kylin-photo-viewer/debian/kylin-photo-viewer.install ('model /usr/share/kylin-photo-viewer')
- **[CONSTATÉ]** 2 fichiers .service D-Bus session installés dans /usr/share/dbus-1/services pointent vers /opt/kaiming-tools/bin/kaiming, même dans les builds non kaiming : kylin-weather et ukui-clock. — *preuve* : archive:kylin-weather/data/org.kylin.weather.service ; archive:ukui-clock/debian/ukui-clock/usr/share/dbus-1/services/org.kylin.dbus.ukuiclock.service + ukui-clock.pro:42-43
- **[CONSTATÉ]** Écart entre la fiche et l'arbre pour kylin-scanner, kylin-video et kylin-burner : la fiche annonce ppa-anything3.0 en 3.0 (quilt) avec les versions 1.0.0-ok1, 3.1.3-ok1 et 3.0.11-ok1, alors que l'arbre archive est en 3.0 (native) et porte les versions installées par l'ISO (3.2.1-ok2.46, 3.1.4.2k1-ok26.39, 3.2.0.1kylin1k32.01-ok45). — *preuve* : archive:kylin-scanner/debian/changelog l.1 ('kylin-scanner (3.2.1-ok2.46) huanghe') ; archive:kylin-video/debian/source/format ('3.0 (native)')
- **[CONSTATÉ]** Entrées installées dans /etc/xdg/autostart par le lot : kylin-weather, kylin-miraclecast (miracle-agent), kylin-printer (lien créé par le postinst), kylin-calendar et kylin-user-guide (2 entrées). — *preuve* : archive:kylin-weather/kylin-weather.pro ('startdesktop.path = /etc/xdg/autostart/') ; archive:kylin-calendar/CMakeLists.txt:155 ; archive:kylin-user-guide/debian/kylin-user-guide.install
- **[CONSTATÉ]** Code chargé dans des processus tiers : modules d'entrée fcitx5-qt (platforminputcontexts) et fcitx5-gtk (immodules GTK2/3), addon fcitx5-gb18030, module GTK3 libukuireload-gtk-module (kylin-theme-configure), plugin peony-rotate-menu-plugin, plugins ukui-control-center (ukui-touch-settings-plugin, libprojection.so de kylin-miraclecast). — *preuve* : archive:fcitx5-qt/debian/fcitx5-frontend-qt5.install ; archive:fcitx5-gtk/debian/fcitx5-frontend-gtk3.install ; archive:kylin-theme/highlight/meson.build ; archive:kylin-photo-viewer/debian/peony-rotate-menu-plugin.install
- **[CONSTATÉ]** Composants tiers embarqués avec version visible : xorriso 1.5.6 (kylin-burner), SQLite 3.32.3 (kylin-music), libpng 1.6.37 et zlib 1.2.11 (kylin-photo-viewer, QtApng), libavc/tgfx/highway (pag), cJSON (ukui-clock). — *preuve* : archive:kylin-burner/lib/ky-burn-engine/lib/xorriso/xorriso/xorriso.h:75-77 ; archive:kylin-music/3rdparty/contrib/sqlite3/sqlite3.h ('#define SQLITE_VERSION "3.32.3"') ; archive:kylin-photo-viewer/codec/plugins/QtApng/src/3rdparty/libpng/src/png.h:294
- **[CONSTATÉ]** kylin-theme contient 2 876 liens symboliques absolus pendants vers un poste de développement (/home/like/work/t/ubuntukylin-theme/…). — *preuve* : find archive:kylin-theme -type l -lname '/home/*' \| wc -l -> 2876
- **[NON COUVERT]** Aucun des 38 paquets du lot n'a d'arbre Gitee inventorié (struct_gitee=null), bien que 30 d'entre eux soient présents sur Gitee : la comparaison archive/Gitee n'est pas faisable en phase 0.2. — *preuve* : Fiche B14-apps-kylin.json : chemin_gitee=null et struct_gitee=null pour toutes les entrées
