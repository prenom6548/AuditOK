# Lot B10-ukui-2

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `ukui-power-manager`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (Vcs-Git github.com/ukui/ukui-power-manager), version 4.24.0.0-ok1.3, format 3.0 (quilt) avec 6 patches. Fichiers data/gpm-prefs.ui et data/gpm-statistics.ui (préfixe gpm) : héritage GNOME/MATE Power Manager probable. Le patch 0001 supprime un sous-arbre 3rd/tlp (scripts TLP amont).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, patches quilt appliqués : 3rd/ absent de l'arbre). Pas de chemin Gitee (forges Gitee et GitHub PRESENT, relation DIVERGENT).
- **Langages / taille** : C++ (523 Ko) + en-têtes (194 Ko), Qt UI, qmake, YAML. 'TypeScript' = fichiers de traduction Qt .ts — 303 fichiers, 1,98 Mo décompressés (archive source 371 Ko). Environ 85 fichiers de code hors tests — lignes de code propres mesurées : 21 573
- **Privilège** : root : ukui-powermanagement-service (activation D-Bus système User=root, lié à -lukui-log4qt -lpci -ludev -linput). Utilisateur de session : ukui-powermanagement, ukui-upower, ukui-power-manager-tray. Plugin libpower.so chargé dans ukui-control-center
- **Déclencheurs** : activation D-Bus système à la demande (org.ukui.powermanagement), ouverture de session UKUI (3 autostart XDG), activation D-Bus de session (org.ukui.upower), installation/suppression du paquet (preinst/postinst/postrm)
- **Indices réseau** : aucun listener réseau détecté par l'extraction (listen vide)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Service D-Bus système root ouvert à tous les appelants (avec liste blanche par exécutable), environ 85 fichiers de code C++ (523 Ko) : lecture exhaustive de registeredQDbus/ et PowerManagementDaemon/, puis tray et plugin.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-system-service | `archive:registeredQDbus/conf/org.ukui.powermanagement.service` | Name=org.ukui.powermanagement ; Exec=/usr/bin/ukui-powermanagement-service ; User=root ; installé dans /usr/share/dbus-1/system-services/ (registeredQDbus/registeredQDbus.pro inst1) | CONSTATÉ |
| dbus-policy | `archive:registeredQDbus/conf/org.ukui.powermanagement.conf` | policy user=root own=org.ukui.powermanagement ; context=default : send_destination autorisé sur org.ukui.powermanagement.interface + Introspectable + Properties ; installé dans /etc/dbus-1/system.d/ (inst2) | CONSTATÉ |
| config-controle-acces | `archive:registeredQDbus/conf/org.ukui.powermanagement.yaml` | liste blanche d'exécutables appelants (ukui-powermanagement, ukui-power-manager-tray, ukui-settings-daemon, ukui-control-center, powersaver, /usr/bin/gdbus, ukui-sidebar) ; audit _default false ; installée dans /usr/share/ukui/ukui-power-manager/ puis copiée par le postinst | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-power-manager.preinst` | dpkg-divert --rename de /usr/lib/<multiarch>/ukui-control-center/libpower.so vers libpower.so.old | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-power-manager.postinst` | copie org.ukui.powermanagement.yaml dans /etc/kylin-config/basic/ ; glib-compile-schemas | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-power-manager.postrm` | remove\|purge : dpkg-divert --remove de libpower.so | CONSTATÉ |
| autostart | `archive:PowerManagementDaemon/resources/ukui-powermanagement-daemon.desktop` | Exec=ukui-powermanagement ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ (PowerManagementDaemon.pro) | CONSTATÉ |
| autostart | `archive:ukui-upower/resources/ukui-upower.desktop` | Exec=ukui-upower ; OnlyShowIn=UKUI ; installé dans /etc/xdg/autostart/ (ukui-upower.pro) | CONSTATÉ |
| autostart | `archive:ukui-power-manager-tray/resources/ukui-power-manager-tray.desktop` | Exec=ukui-power-manager-tray ; installé dans /etc/xdg/autostart/ (ukui-power-manager-tray.pro) | CONSTATÉ |
| dbus-session-service | `archive:ukui-upower/resources/org.ukui.upower.service` | Name=org.ukui.upower ; Exec=/usr/bin/ukui-upower ; installé dans /usr/share/dbus-1/services/ (bus de session) | CONSTATÉ |
| fichiers-config-systeme | `archive:ukui-upower/ukui-upower.pro` | installe resources/10-monitor.conf dans /usr/share/X11/xorg.conf.d/ et power-manager.conf dans /etc/ukui/power ; registeredQDbus.pro installe upm-hardware-global.conf et 8 fichiers fabricants dans /opt/system/resource/ukui/ukui-power-manager/ | CONSTATÉ |
| plugin | `archive:power/power.pro` | libpower.so (plugin ukui-control-center) installé dans $$[QT_INSTALL_LIBS]/ukui-control-center | CONSTATÉ |
| systemd-unit (installation non trouvée) | `archive:data/ukui-power-policy.service` | ExecStart=/usr/bin/ukui-power-policy ; Type=dbus ; BusName=ukui.power.policy ; pas de User= (root) ; WantedBy=multi-user.target ; aucune référence d'installation dans les .pro/.install, aucune source de ukui-power-policy trouvée par grep | PROBABLE |
| dbus-policy (installation non trouvée) | `archive:data/ukui.power.policy.conf` | policy user=root own=ukui.power.policy ; context=default send_destination=ukui.power.policy sans restriction d'interface ; non référencé à l'installation | PROBABLE |
| dbus-session-service (installation non trouvée) | `archive:data/org.ukui.PowerManager.service.in` | Name=org.ukui.PowerManager ; Exec=@servicedir@/ukui-power-manager ; data/ et ukui-power-manager/ absents des SUBDIRS de powermanagement.pro | PROBABLE |
| udev (installation non trouvée) | `archive:ukui-upower/resources/99-upower-wake-up-usb-device.rules` | ATTR{power/wakeup}=enabled pour les USB *Mouse/*Keyboard (copie identique dans data/) ; aucune référence d'installation trouvée | PROBABLE |
| debian-rules | `archive:debian/rules` | dh $@ ; DEB_BUILD_MAINT_OPTIONS=hardening=+all ; QT_SELECT=qt5 | CONSTATÉ |
| debian-install-lists | `archive:debian/ukui-power-manager.install` | fichier vide : installation pilotée par les INSTALLS qmake | CONSTATÉ |
| systemd-service + dbus-policy + udev (installation non trouvee) [ajout critique] | `archive:ukui-power-manager/data/ukui-power-policy.service ; data/ukui.power.policy.conf ; data/99-upower-wake-up-usb-device.rules ; ukui-upower/resources/99-upower-wake-up-usb-device.rules` | Presents dans la fiche, absents du recensement. ukui-power-policy.service : Type=dbus, BusName=ukui.power.policy, sans User=, WantedBy=multi-user.target. La politique autorise send_destination depuis le contexte default. Aucune regle d'installation trouvee dans les .pro ni dans debian/ : PROBABLEMENT non installes. Le postinst copie aussi org.ukui.powermanagement.yaml dans /etc/kylin-config/basic/. | CONSTATÉ |

- [CONSTATÉ] debian/patches/0001-chore-update-version-4.24.0.0-ok1.1.patch (542 Ko) supprime 30 fichiers 3rd/tlp/* et modifie registeredQDbus/sysdbusregister.cpp. Le code construit correspond à l'arbre patché.
- [NON COUVERT] La liste blanche YAML identifie les appelants par chemin d'exécutable et inclut /usr/bin/gdbus. La façon dont le code l'applique n'est pas vérifiée en Phase 0.2.
- [PROBABLE] ukui-power-manager/src/main.cpp et tests/ ne figurent pas dans SUBDIRS de powermanagement.pro : ils ne sont probablement pas construits.

### `ukui-media`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (github.com/ukui/ukui-media), 4.10.0.0-0ok3.8, format 3.0 (native). Héritage MATE probable : arborescence autotools (Makefile.am, AUTHORS) et Description de l'unité « Mate media control leds daemon ».
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète). debian/patches/series liste 63 patches (dont 0057-57.patch de 2,9 Mo) alors que le format est natif, sans répertoire .pc. Pas de chemin Gitee.
- **Langages / taille** : C++ (879 Ko) + en-têtes (189 Ko), Shell (15 Ko), qmake. 'XML' = rapport de couverture debian, 'TypeScript' = traductions Qt .ts — 368 fichiers, 8,49 Mo (sons .ogg et patches compris). Environ 53 fichiers de code hors tests — lignes de code propres mesurées : 31 793
- **Privilège** : root : ukui-media-control-led (unité systemd système) et soundCardAdaptation.sh (RUN udev). Utilisateur de session : ukui-volume-control-applet-qt, sound-theme-player, ukui-login-sound. Plugin libaudio.so chargé dans ukui-control-center
- **Déclencheurs** : démarrage (multi-user.target), événement udev change sur une carte son, ouverture de session UKUI (autostart), installation du paquet
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local, aucune écoute IP
- **Blobs** : aucun exécutable binaire ; sons .ogg (startup, shutdown, weakup) et rapport debian/ukui-media-coverage.xml (806 Ko)
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant volumineux (879 Ko de C++) dont deux parties s'exécutent en root (ukui-media-control-led, script udev) : lecture exhaustive de ces deux parties et des zones à risque du reste.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:data/ukui-media-control-mute-led.service` | ExecStart=/usr/bin/ukui-media-control-led ; pas de User= (root) ; Restart=always ; After=lightdm.service ; WantedBy=multi-user.target ; installé dans /lib/systemd/system par debian/ukui-media.install | CONSTATÉ |
| udev | `archive:configuration/99-ukui-media.rules` | SUBSYSTEM=sound, ACTION=change, KERNEL=card*, hors USB : RUN+=/usr/share/ukui-media/scripts/soundCardAdaptation.sh (exécuté en root par udev) ; PULSE_PROFILE_SET/PULSE_VOLUME_IGNORE_DB par VID/PID USB ; installé dans /usr/lib/udev/rules.d | CONSTATÉ |
| script-root (contenu) | `archive:scripts/soundCardAdaptation.sh` | 318 lignes, invoqué par la règle udev ; contenu non lu (hors périmètre 0.2) | NON COUVERT |
| maintainer-script | `archive:debian/ukui-media.preinst` | dpkg-divert --rename de .../ukui-control-center/libaudio.so vers libaudio.so.old | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-media.postinst` | dpkg-divert --rename --remove "$path".old ; glib-compile-schemas | CONSTATÉ |
| maintainer-script (orphelin) | `archive:debian/ukui-media-server.postinst` | configure : systemctl enable ukui-media-server.service. debian/control ne déclare aucun paquet binaire ukui-media-server (seulement ukui-media et ukui-media-common) : le script n'est probablement pas utilisé | PROBABLE |
| autostart | `archive:data/ukui-volume-control-applet.desktop` | Exec=ukui-volume-control-applet-qt ; OnlyShowIn=UKUI ; AutoRestart ; installé dans /etc/xdg/autostart par ukui-media-common.install | CONSTATÉ |
| autostart | `archive:data/sound-theme-player.desktop` | Exec=sound-theme-player ; OnlyShowIn=UKUI ; installé dans /etc/xdg/autostart par ukui-media.install | CONSTATÉ |
| config-pulseaudio | `archive:debian/ukui-media.install` | configuration/kylin-profiles/* vers /usr/share/pulseaudio/alsa-mixer/profile-sets, kylin-paths/* vers .../paths ; scripts detection_output_mode.sh et soundCardAdaptation.sh vers /usr/share/ukui-media/scripts | CONSTATÉ |
| socket-local | `archive:ukui-volume-control-applet-qt/QtSingleApplication/qtlocalpeer.cpp` | new QLocalServer l.103, server->listen(socketName) l.121 : instance unique (Qt Solutions), socket local, aucune écoute IP | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; hardening=+all ; CXXFLAGS=-Wno-error ; dh_shlibdeps --ignore-missing-info | CONSTATÉ |

- [PROBABLE] Format 3.0 (native) avec 63 patches dans debian/patches/series : dpkg-source ne les applique pas aux paquets natifs, on ne sait donc pas si l'arbre les intègre déjà. À lever avant la Phase 1.
- [PROBABLE] Activation de ukui-media-control-mute-led.service par dh_installsystemd (debhelper-compat 12, unité livrée dans /lib/systemd/system) probable.
- [CONSTATÉ] ukui-volume-control (même lot) détourne aussi libaudio.so, avec --package ukui-media.

### `ukui-bluetooth`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (mainteneur dengtuo@kylinos.cn), 4.20.0.0-ok0.8, format 3.0 (native), 0 patch, Vcs-Git vide
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee (forges PRESENT, DIVERGENT).
- **Langages / taille** : C++ (637 Ko) + en-têtes (177 Ko), QML (17 Ko), qmake. 'TypeScript' = traductions .ts — 209 fichiers, 1,51 Mo. Environ 67 fichiers de code — lignes de code propres mesurées : 27 166
- **Privilège** : root : bluetoothService (systemd système, nom com.ukui.bluetooth sur le bus système). Utilisateur de session : ukui-bluetooth (UI, transfert de fichiers). Plugins ukcc et QML
- **Déclencheurs** : démarrage (multi-user.target), postinst : enable + start, ouverture de session (autostart), événements BlueZ (appairage, transfert OBEX)
- **Indices réseau** : aucun listener IP détecté; surface radio Bluetooth via BlueZ (appairage, transfert de fichiers OBEX : ukui-bluetooth/popwidget/bluetoothfiletransferwidget.cpp) : PROBABLE
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Service système root exposé sur le bus système à tous les utilisateurs et traitant des événements radio Bluetooth/OBEX, environ 67 fichiers de code (637 Ko de C++).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:data/ukui-bluetooth.service` | ExecStart=/usr/bin/bluetoothService ; Type=simple ; pas de User= (root) ; Environment=DBUS_SESSION_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket ; Restart=always ; WantedBy=multi-user.target ; installé dans /lib/systemd/system par debian/ukui-bluetooth.install | CONSTATÉ |
| dbus-policy | `archive:data/com.ukui.bluetooth.conf` | policy user=root own=com.ukui.bluetooth ; context=default : send_destination com.ukui.bluetooth (interface com.ukui.bluetooth) + Introspectable + Properties ; installé dans /etc/dbus-1/system.d/ | CONSTATÉ |
| dbus-policy | `archive:data/org.bluez.obex.conf` | policy user=root own=org.bluez.obex sur le bus système ; context=default : send_destination org.bluez.obex (interface org.bluez.obex) ; installé dans /etc/dbus-1/system.d/ | CONSTATÉ |
| dbus-policy | `archive:data/org.bluez.Agent1.conf` | policy context=default : allow own=org.bluez.Agent1 (tout utilisateur peut posséder ce nom sur le bus système) et send_destination org.bluez.Agent1 ; installé dans /etc/dbus-1/system.d/ par ukui-bluetooth/ukui-bluetooth.pro (inst1, CONF_INSTALL_DIR) | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-bluetooth.preinst` | set -e commenté ; dpkg-divert de libukcc-bluetooth.so ; systemctl stop ukui-bluetooth.service ; crée /etc/bluetooth/ukui-bluetooth.conf (switch=true) si la version précédente contient 1.0.0-1 ou 1.0.3 | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-bluetooth.postinst` | glib-compile-schemas ; exécute /usr/share/kylin-system-updater/kylin-reboot-required s'il existe ; sed activeConnection=true→false dans /etc/bluetooth/ukui-bluetooth.conf ; systemctl enable puis start ukui-bluetooth.service | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-bluetooth.prerm` | remove\|upgrade\|deconfigure : dpkg-divert --remove de libukcc-bluetooth.so | CONSTATÉ |
| autostart | `archive:data/ukui-bluetooth.desktop` | Exec=/usr/bin/ukui-bluetooth ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ | CONSTATÉ |
| autostart (installation non trouvée) | `archive:data/ukui-bluetooth-service.desktop` | Exec=/usr/bin/profileDaemon ; ni référence d'installation ni cible profileDaemon trouvées dans les .pro/.pri/.install | PROBABLE |
| plugins | `archive:ukui-bluetooth.pro` | SUBDIRS ukui-bluetooth (UI), ukui-shortcut-bluetooth (module QML), ukcc-bluetooth (plugin ukui-control-center), service (bluetoothService vers /usr/bin) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | gabarit dh_make : dh $@ ; DEB_BUILD_MAINT_OPTIONS=hardening=+all commenté | CONSTATÉ |

- [CONSTATÉ] org.bluez.Agent1.conf autorise le contexte default à posséder le nom org.bluez.Agent1 sur le bus système.
- [CONSTATÉ] L'unité root redirige DBUS_SESSION_BUS_ADDRESS vers le socket du bus système.
- [CONSTATÉ] Durcissement dpkg-buildflags commenté dans debian/rules (compat 10).

### `ukui-system-monitor`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (github.com/ukui/ukui-system-monitor), 4.24.0.0-ok1.3, format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (907 Ko) + en-têtes (257 Ko), C (11 Ko), Shell (3 Ko), qmake. 'TypeScript' = traductions .ts (1,4 Mo) — 311 fichiers, 4,15 Mo. Environ 92 fichiers de code hors tests — lignes de code propres mesurées : 37 856
- **Privilège** : binaire GUI lancé par l'utilisateur, doté par le postinst des capacités fichier cap_net_raw et cap_dac_read_search, ou à défaut setuid root
- **Déclencheurs** : lancement manuel (menu applications), installation du paquet (postinst setcap/setuid)
- **Indices réseau** : cap_net_raw accordée au binaire : sockets bruts (PROBABLE : mesure du débit réseau par processus); QLocalServer QtSingleApplication (socket local)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Tout le code du binaire GUI s'exécute avec des privilèges (cap_dac_read_search et cap_net_raw, ou setuid root en repli) : lecture exhaustive des environ 92 fichiers de code.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script (privilèges fichier) | `archive:debian/ukui-system-monitor.postinst` | configure : setcap cap_net_raw,cap_dac_read_search+ep /usr/bin/ukui-system-monitor puis chmod u-s. Si setcap échoue ou est absent : chmod u+s (setuid root). set -e n'est placé qu'après ce bloc | CONSTATÉ |
| desktop | `archive:ukui-system-monitor.desktop` | Exec=/usr/bin/ukui-system-monitor ; NoDisplay=false ; installé dans /usr/share/applications/ (src/src.pro) | CONSTATÉ |
| config | `archive:ukui-system-monitor.conf` | section [CUSTOMPROC] (correspondance de noms de processus) installée dans /usr/share/ukui/ (src/src.pro config) | CONSTATÉ |
| socket-local | `archive:src/QtSingleApplication/qtlocalpeer.cpp` | QLocalServer listen l.121 (instance unique), aucune écoute IP | CONSTATÉ |
| debian-install-lists | `archive:debian/ukui-system-monitor.install` | guide, icônes, traductions ; ukui-system-monitor.json dans /etc/ukui/ukui-notification/default-app/ | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; hardening=+all | CONSTATÉ |

- [CONSTATÉ] Le repli setuid root dépend de la présence et du succès de setcap sur le système cible.
- [CONSTATÉ] tests/ embarque un cpp-stub (elfio.hpp 153 Ko) exclu de SUBDIRS (SUBDIRS -= tests).

### `ukui-interface`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (github.com/ukui/ukui-interface), 4.10.0.0-ok4, format 3.0 (native). Embarque une copie de Log4Qt (src/log4qt/log4qt, LOG4QT_VERSION_STR 0.3.0) enveloppée par des fichiers Kylin (ukui-log4qt, ukui-logconfigurator, ukui-logrolling).
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (542 Ko) + en-têtes (394 Ko), C (10 Ko, src/common), autotools + qmake. 'XML' = rapport de couverture — 152 fichiers, 1,39 Mo — lignes de code propres mesurées : 29 300
- **Privilège** : bibliothèques sans processus propre, qui héritent du privilège de l'hôte (ex. ukui-powermanagement-service, root, lie -lukui-log4qt)
- **Déclencheurs** : chargement par les processus liés, configuration GSettings org.ukui.log4qt
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Bibliothèques de journalisation et de lecture INI liées par des services root : lecture exhaustive des fichiers Kylin (ukui-log*, src/common/*.c) et diff de la copie Log4Qt avec l'amont 0.3.0.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install-lists | `archive:debian/libukui-log4qt1.install` | usr/lib/libukui-log4qt.so.* + schéma /usr/share/glib-2.0/schemas/org.ukui.log4qt.gschema.xml | CONSTATÉ |
| debian-install-lists | `archive:debian/libukui-common0.install` | libukui-com4c.so.*, libukui-com4cxx.so.* (src/common : kylin-com4c, kylin-ini) | CONSTATÉ |
| maintainer-script | `archive:debian/libukui-log4qt1.postinst` | glib-compile-schemas | CONSTATÉ |
| debian-rules | `archive:debian/rules` | autogen.sh puis configure --includedir=/usr/include/ukuisdk --bindir=/usr/libexec ; build qmake de src/log4qt avec flags explicites (-fstack-protector-strong, _FORTIFY_SOURCE=2, -z relro -z now) ; hardening=+all | CONSTATÉ |
| bibliotheque-liee | `archive:src/log4qt/ukui-log4qt.pro` | target.path=/usr/lib ; installe le schéma gsettings et l'en-tête dans /usr/include | CONSTATÉ |

- [CONSTATÉ] Le répertoire afltests/ (harnais AFL pour log4qt et common) est présent.
- [CONSTATÉ] debian/control déclare aussi libukui-log4qt0 (Architecture all) sans fichier .install.

### `ukui-framework`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (gitee.com/openkylin/ukui-framework), 4.24.0.0-ok0.3, format 3.0 (native), absent de Debian et d'Ubuntu. Embarque QtSingleApplication (Qt Solutions) dans 3rd-parties/.
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (298 Ko) + en-têtes (160 Ko), QML (8 Ko), CMake. 'HTML' = documentation QtSingleApplication — 231 fichiers, 750 Ko. Environ 51 fichiers de code — lignes de code propres mesurées : 15 444
- **Privilège** : utilisateur de session (bus de session)
- **Déclencheurs** : ouverture de session (autostart), activation D-Bus de session
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Service D-Bus de session qui agrège des réglages (périphériques audio, Bluetooth, écran, alimentation, UI) plus un service de sauvegarde de configuration. Environ 51 fichiers de code, sans privilège root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-session-service | `archive:data/org.ukui.Framework.service` | Name=org.ukui.Framework ; Exec=/usr/bin/ukui-framework-dbus ; installé dans /usr/share/dbus-1/services/ (debian/ukui-framework-dbus.install) | CONSTATÉ |
| autostart | `archive:data/ukui-framework-dbus.desktop` | Exec=ukui-framework-dbus ; NoDisplay=true ; X-UKUI-Autostart-Notify=true ; installé dans /etc/xdg/autostart | CONSTATÉ |
| dbus-session-service | `archive:backup-service/data/org.ukui.ConfigBackupService.service` | Name=org.ukui.ConfigBackupService ; Exec=/usr/bin/ukui-config-backup-service ; paquet ukui-config-backup-service, absent de la liste ISO ; modèle ukui-backup-config-template.json installé dans /usr/share/ukui/config-backup-service | CONSTATÉ |
| socket-local | `archive:3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer listen l.85, aucune écoute IP | CONSTATÉ |
| build | `archive:CMakeLists.txt` | add_subdirectory(dbus), add_subdirectory(backup-service) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ minimal (pas d'option hardening explicite) | CONSTATÉ |

- [CONSTATÉ] Le paquet binaire ukui-framework (Architecture all) n'a pas de fichier .install.

### `ukui-quick`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (gitee.com/openkylin/ukui-quick), 4.25.0.0-ok0.2, format 3.0 (quilt), 1 patch
- **Installé par défaut (ISO 3.0)** : oui (6 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (2,6 Mo, autotests volumineux compris) + en-têtes (508 Ko), QML (204 Ko), CMake, Shell, JSON — 523 fichiers, 4,32 Mo — lignes de code propres mesurées : 99 039
- **Privilège** : bibliothèques et plugins QML chargés dans les processus de session (panel, menu, widgets : PROBABLE)
- **Déclencheurs** : chargement QML/bibliothèque par les composants UKUI
- **Indices réseau** : socket AF_UNIX SOCK_DGRAM (heartbeat), local
- **Blobs** : aucun (images PNG de documentation)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Bibliothèques QML volumineuses (environ 2,6 Mo de C++ tests compris) chargées dans les processus de session, sans service ni privilège : viser l'IPC (heartbeat), le chargement Lottie/décompression et les modèles window-thumbnail/MPRIS.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install-lists | `archive:debian/libukui-quick-core1.install` | bibliothèques core, framework et platform (3 paquets lib*.so.*) ; modules QML org/ukui/quick/items, org/ukui/quick/platform, org/ukui/windowThumbnail (9 fichiers .install au total) | CONSTATÉ |
| socket-local | `archive:platform/ukui/heartbeat/keep-alive-client.cpp` | socket(AF_UNIX, SOCK_DGRAM) l.227 ; bind sur l'adresse client l.247 ; QSocketNotifier Read : client heartbeat en datagramme Unix local, aucune écoute IP | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake ; CMAKE_BUILD_TYPE=RelWithDebInfo ; hardening commenté | CONSTATÉ |
| build | `archive:CMakeLists.txt` | sous-projets core, platform/ukui/lottie, items, platform, framework, modules, cmake-extend ; BUILD_TEST OFF par défaut | CONSTATÉ |

- [PROBABLE] items/autotest/lottie-blocking-unzip-helper.cpp suggère une décompression d'archives Lottie à l'exécution.

### `ukui-window-switch`

- **Nature** : fork-amont — **Amont** : Kylin/UKUI (gitee.com/openkylin/ukui-window-switch), 4.25.0.0-ok0.1, format 3.0 (quilt), 0 patch. windowswitchers/thumbnail_grid provient de KWin/KDE (metadata.desktop : X-KDE-PluginInfo-Author=Chris Holland, X-KDE-ServiceTypes=KWin/WindowSwitcher).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (439 Ko) + en-têtes (90 Ko), QML (175 Ko), JavaScript (10 Ko), CMake. 'HTML' = documentation QtSingleApplication — 194 fichiers, 983 Ko — lignes de code propres mesurées : 21 831
- **Privilège** : utilisateur de session ; le plugin d'effet s'exécute dans le processus du compositeur KWin
- **Déclencheurs** : ouverture de session (autostart, délai 3 s), raccourci ou appel D-Bus de la vue multitâche
- **Indices réseau** : QLocalServer (QtSingleApplication modifiée) : socket local
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session avec IPC D-Bus et un plugin chargé dans le compositeur, environ 440 Ko de C++ : viser les services D-Bus, le plugin KWin et le lanceur.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart | `archive:windowsview/ukui-window-switch.desktop` | Exec=/usr/bin/ukui-window-switch ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Delay=3 ; AutoRestart ; installé dans /etc/xdg/autostart/ (windowsview/CMakeLists.txt l.18) | CONSTATÉ |
| lanceur-shell | `archive:windowsview/ukui-window-switch` | script bash de 15 lignes installé dans /usr/bin : si ps aux trouve kylin-wlcom, lance /usr/bin/ukui-window-switch-wlcom, sinon dbus-send org.kde.KWin /MultitaskView show | CONSTATÉ |
| plugin-compositeur | `archive:windowsview/ukui-window-switch-kwineffect/CMakeLists.txt` | plugin installé dans ${QT_INSTALL_PLUGINS}/kwin/effects/plugins/ (chargé dans le processus KWin), avec dbusservice.cpp | CONSTATÉ |
| binaire-session | `archive:windowsview/ukui-window-switch-wlcom/CMakeLists.txt` | ukui-window-switch-wlcom installé dans /usr/bin ; dbusservice.cpp ; QtSingleApplication modifiée (qtlocalpeer.cpp : QLocalServer l.180, listen l.241) | CONSTATÉ |
| kwin-tabbox | `archive:windowswitchers/CMakeLists.txt` | installe thumbnail_grid dans ${KDE_INSTALL_DATADIR}/kwin/tabbox et metadata.desktop renommé kwin4_window_switcher_thumbnail_grid.desktop | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | chmod a+x /usr/bin/ukui-window-switch | CONSTATÉ |
| debian-rules | `archive:debian/rules` | gabarit dh_make, hardening commenté | CONSTATÉ |

- [CONSTATÉ] La copie de qtlocalpeer.cpp dans ukui-window-switch-wlcom est plus longue que les autres copies du lot (listen l.241 au lieu de l.85-121) : elle a été modifiée localement.
- [CONSTATÉ] [correction critique] Reclassé fork-amont (partiel) : code dérivé de KWin (switcher thumbnail_grid), KDE ; version amont à établir en Phase 2.

### `xdg-desktop-portal-ukui`

- **Nature** : propre-kylin — **Amont** : Kylin (Yue Lan, lanyue@kylinos.cn), 0.4.0-ok7, format 3.0 (native), absent de Debian et d'Ubuntu. Entrée initiale du changelog : 0.1.0 du 16/03/2023. debian/watch ne contient que version=3.
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (314 Ko) + en-têtes (48 Ko), CMake. 'XML' = couverture, 'TypeScript' = traductions — 73 fichiers, 814 Ko — lignes de code propres mesurées : 9 683
- **Privilège** : utilisateur de session, activé par xdg-desktop-portal pour le compte des applications (y compris sandboxées)
- **Déclencheurs** : appel D-Bus de portail par une application (sélection de fichier, impression, choix d'application, notification, inhibition), variables Xsession forçant l'usage du portail par GTK/GLib
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Le backend de portail est une frontière de confiance entre les applications (éventuellement sandboxées) et la session (accès aux fichiers, impression). Taille réduite : 73 fichiers, 314 Ko de C++.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-session-service | `archive:data/org.freedesktop.impl.portal.desktop.ukui.service.in` | Name=org.freedesktop.impl.portal.desktop.ukui ; Exec=${LIBEXEC_PATH}/xdg-desktop-portal-ukui ; installé dans /usr/share/dbus-1/services | CONSTATÉ |
| portal | `archive:data/ukui.portal` | Interfaces FileChooser, Inhibit, Notification, AppChooser, Print ; UseIn=UKUI ; installé dans /usr/share/xdg-desktop-portal/portals | CONSTATÉ |
| xsession.d | `archive:data/50xdp-ukui` | installé dans /etc/X11/Xsession.d. Si XDG_SESSION_DESKTOP vaut ukui ou kylin-wlcom : export GTK_USE_PORTAL=1, GLIB_FORCE_USE_PORTAL=1, GLIB_USE_KYLIN_PROCESS_MANAGER=1 | CONSTATÉ |
| desktop | `archive:data/org.freedesktop.impl.portal.desktop.ukui.desktop.in` | Exec=${LIBEXEC_PATH}/xdg-desktop-portal-ukui ; NoDisplay=true ; installé dans /usr/share/applications | CONSTATÉ |
| build | `archive:CMakeLists.txt` | définition de compilation PLUGIN_INSTALL_FILES=/usr/lib/<arch>/peony-extensions/libpeony-computer-view-plugin.so (référence à un plugin peony) ; binaire installé dans libexec | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; XDG_UKUI_SKIP_TEST=1 | CONSTATÉ |

- [CONSTATÉ] forges.gitee_heads ne contient aucune branche openkylin/huanghe (seulement nile et nile-sp2) alors que le paquet est sur l'ISO huanghe ; l'entrée en tête du changelog est ciblée nile.
- [PROBABLE] L'usage de PLUGIN_INSTALL_FILES (chargement dynamique d'un plugin peony) dans le code n'est pas vérifié.

### `qt5-ukui-platformtheme`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (gitee.com/openkylin/qt5-ukui-platformtheme), 4.24.0.0-ok0.6, format 3.0 (quilt), 57 patches. Des fichiers de ukui-qqc2-style/org.ukui.style/private et ukui-qml-style-helper/private mentionnent KDE : dérivation probable de qqc2-desktop-style.
- **Installé par défaut (ISO 3.0)** : oui (5 binaire(s)) — **Code disponible** : archive (source complète + 57 patches, dont 0048 de 2,5 Mo). Pas de chemin Gitee.
- **Langages / taille** : C++ (2,78 Mo) + en-têtes (587 Ko), QML (708 Ko), Shell, YAML, CMake. 'TypeScript' = traductions — 666 fichiers, 8,93 Mo — lignes de code propres mesurées : 109 057
- **Privilège** : plugins chargés dans les processus Qt5/QML de la session qui utilisent le thème ukui, avec le privilège de chaque processus hôte
- **Déclencheurs** : démarrage de toute application Qt5/QML de la session, ouverture d'un dialogue de fichiers Qt
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Plugins chargés dans toutes les applications Qt5, volumineux (2,8 Mo de C++) : viser la lecture de configuration (gsettings/yaml), le dialogue de fichiers et les chargements dynamiques.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| plugin-qt | `archive:debian/qt5-ukui-platformtheme.install` | usr/lib/*/qt5/plugins/platformthemes/*.so (plugin de thème de plateforme Qt5, contient qt5-ukui-filedialog/kyfiledialog.cpp de 180 Ko) | CONSTATÉ |
| plugin-qt | `archive:debian/qt5-styles-ukui.install` | usr/lib/*/qt5/plugins/styles/*.so | CONSTATÉ |
| module-qml | `archive:debian/qml-module-org-ukui-qqc2desktopstyle.install` | installé dans usr/lib/*/qt5/qml/QtQuick/Controls.2/* (espace de noms QML de Qt lui-même) | CONSTATÉ |
| config | `archive:debian/libqt5-ukui-style1.install` | /etc/kylin-config/basic/org.ukui.style.yaml, schéma org.ukui.style.gschema.xml, lib*.so.* | CONSTATÉ |
| build | `archive:CMakeLists.txt` | sous-projets ukui-styles, libqt5-ukui-style, qt5-ukui-filedialog, qt5-ukui-platformtheme, ukui-qqc2-style, ukui-qml-style-helper ; icon-engines/ (usvg) absent des add_subdirectory | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --with pkgkde_symbolshelper ; PLATFORMTHEME_SKIP_TEST=1 ; hardening=+all | CONSTATÉ |

- [CONSTATÉ] Il y a deux arborescences de tests, test/ et tests/, plus un Doxyfile.in de 108 Ko.
- [NON COUVERT] Le chargement du plugin par des processus lancés en root (pkexec) dépend de l'environnement QT_QPA_PLATFORMTHEME et n'est pas vérifié.

### `kylin-virtual-keyboard`

- **Nature** : propre-kylin — **Amont** : Kylin (hanteng@kylinos.cn), 4.24.0.0-ok0.3, format 3.0 (quilt), 32 patches (traductions Weblate et changelog). Frontal de clavier virtuel pour Fcitx5 (Fcitx5CompilerSettings).
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (194 Ko) + en-têtes (102 Ko), QML (94 Ko), JavaScript (7 Ko), CMake — 282 fichiers, 2,38 Mo (dont une image PNG de 1,6 Mo) — lignes de code propres mesurées : 10 583
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session (autostart), interaction avec Fcitx5 par D-Bus (PROBABLE)
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session qui manipule la saisie clavier (IPC avec Fcitx5), taille modeste (194 Ko de C++) : viser les chemins IPC et la génération de touches.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart | `archive:data/kylin-virtual-keyboard.desktop.in` | Exec=@CMAKE_INSTALL_FULL_BINDIR@/org.ukui.virtualkeyboard.start.sh ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ (data/CMakeLists.txt) | CONSTATÉ |
| lanceur-shell | `archive:data/org.ukui.virtualkeyboard.start.sh.in` | exec kylin-virtual-keyboard ; hors session UKUI Wayland, force QT_QPA_PLATFORM=xcb ; installé dans bindir (install PROGRAMS) | CONSTATÉ |
| dbus-session-service (installation non trouvée) | `archive:data/org.fcitx.Fcitx5.VirtualKeyboard.service` | Name=org.fcitx.Fcitx5.VirtualKeyboard ; Exec=/usr/bin/kylin-virtual-keyboard ; aucune règle install() correspondante dans les CMakeLists | PROBABLE |
| maintainer-script | `archive:debian/kylin-virtual-keyboard.postinst` | glib-compile-schemas | CONSTATÉ |
| socket-local | `archive:third-party/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer listen l.89, aucune écoute IP | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ minimal (pas de hardening explicite) | CONSTATÉ |

- [CONSTATÉ] test/CMakeLists.txt installe runTests dans bindir si ENABLE_TEST est actif ; l'option n'est pas positionnée dans debian/rules.

### `youker-assistant`

- **Nature** : propre-kylin — **Amont** : Ubuntu Kylin/Kylin (github.com/ubuntukylin/youker-assistant ; debian/watch pointe vers les releases GitHub), 4.10.0.0-ok2.10, format 3.0 (quilt), 19 patches. Application kylin-assistant.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (616 Ko) + en-têtes (153 Ko), Python (380 Ko : démons D-Bus), qmake. 'TypeScript' = traductions — 286 fichiers, 5,95 Mo. Environ 102 fichiers de code dont environ 30 modules Python — lignes de code propres mesurées : 31 416
- **Privilège** : root : démon Python sur le bus système (User=root). Utilisateur : démon de session Python et GUI Qt
- **Déclencheurs** : activation D-Bus système ou de session à la demande, lancement manuel de l'interface
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local; kyasDbus/systemdaemon/networkadaptorthread.py : collecte d'informations sur les cartes réseau (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Démon Python root activable par tout utilisateur via le bus système, qui collecte des informations matérielles (environ 30 modules : CPU, disques, caméra, batterie, réseau, ventilateurs). Non installé par défaut sur l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-system-service | `archive:kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.service` | Name=com.kylin.assistant.systemdaemon ; Exec=/usr/bin/kylin-assistant-systemdaemon.py ; User=root ; installé dans /usr/share/dbus-1/system-services/ (kyasDbus/kyasDbus.pro inst3) | CONSTATÉ |
| dbus-policy | `archive:kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.conf` | policy user=root own ; context=default : send_destination com.kylin.assistant.systemdaemon (interface homonyme) + Introspectable + Properties ; installé dans /etc/dbus-1/system.d/ (inst2) | CONSTATÉ |
| polkit | `archive:kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.policy` | action com.kylin.assistant.systemdaemon.action : allow_any, allow_inactive et allow_active = auth_admin ; installé dans /usr/share/polkit-1/actions/ (inst4) | CONSTATÉ |
| dbus-session-service | `archive:kyasDbus/sessiondaemon/conf/com.kylin.assistant.sessiondaemon.service` | Name=com.kylin.assistant.sessiondaemon ; Exec=/usr/bin/kylin-assistant-sessiondaemon.py ; installé dans /usr/share/dbus-1/services/ (inst5) | CONSTATÉ |
| liens | `archive:debian/links` | /usr/lib/python3/dist-packages/kylin-assistant-daemon/systemdaemon/kasystemdbusmain.py vers /usr/bin/kylin-assistant-systemdaemon.py ; même chose pour sessiondaemon (code Python installé par inst1) | CONSTATÉ |
| maintainer-script (nom non apparié) | `archive:debian/kylin-assistant.postinst` | exécute /usr/share/kylin-system-updater/kylin-reboot-required s'il existe. Le paquet binaire s'appelle youker-assistant : dh_installdeb ignore probablement ce fichier | PROBABLE |
| desktop | `archive:kylin-assistant.desktop` | Exec=/usr/bin/kylin-assistant %u | CONSTATÉ |
| socket-local | `archive:src/qtsingleapplication/qtlocalpeer.cpp` | QLocalServer listen l.121 | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+bindnow ; DEFINES+=NO_DEBUG_ON_CONSOLE ; dh_missing --fail-missing | CONSTATÉ |

- [CONSTATÉ] Le paquet est absent de installe_par_defaut_iso.
- [NON COUVERT] L'appel effectif à l'action polkit dans le code Python n'est pas vérifié en Phase 0.2.

### `ukui-app-widget`

- **Nature** : propre-kylin — **Amont** : Kylin (gitee.com/openkylin/ukui-app-widget), 4.1.0.0-ok0.2, format 3.0 (quilt), 0 patch. Aucune branche huanghe dans gitee_heads.
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (116 Ko) + en-têtes (52 Ko), QML (23 Ko), qmake — 118 fichiers, 300 Ko — lignes de code propres mesurées : 5 840
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session (autostart, si le paquet manager est installé), activation D-Bus de session
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — IPC D-Bus de session entre fournisseurs et consommateurs de widgets, petit volume (116 Ko de C++) ; seules les bibliothèques sont sur l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-session-service | `archive:ukui-appwidget-manager/data/org.ukui.appwidget.manager.service` | Name=org.ukui.appwidget.manager ; Exec=/usr/bin/ukui-appwidget-manager ; installé dans /usr/share/dbus-1/services par le paquet ukui-appwidget-manager (absent de l'ISO) | CONSTATÉ |
| autostart | `archive:ukui-appwidget-manager/data/ukui-appwidget-manager.desktop` | Exec=/usr/bin/ukui-appwidget-manager ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Phase=Initialization ; installé dans etc/xdg/autostart (debian/ukui-appwidget-manager.install) | CONSTATÉ |
| dbus-session-service (démo) | `archive:ukui-appwidget-test/appwidgetprovider-clock-demo/appwidgetprovider-clock-service/provider/org.ukui.appwidget.provider.democlock.service` | Exec=/usr/bin/appwidgetprovider-clock-service ; même chose pour demosearch ; paquet ukui-appwidget-test (absent de l'ISO) | CONSTATÉ |
| bibliotheques | `archive:debian/libukui-appwidget-manager0.install` | libukui-appwidget-manager.so.* et libukui-appwidget-provider.so.* (installées par l'ISO) | CONSTATÉ |
| socket-local | `archive:ukui-appwidget-manager/QtSingleApplication/qtlocalpeer.cpp` | QLocalServer listen l.121 | CONSTATÉ |

- [CONSTATÉ] L'ISO n'installe que libukui-appwidget-manager0 et libukui-appwidget-provider0, pas le démon ukui-appwidget-manager.

### `ukui-file-metadata`

- **Nature** : fork-amont — **Amont** : Kylin (gitee.com/openkylin/ukui-file-meta-data), 4.25.0.0-ok0.1, format 3.0 (quilt), 0 patch. Une partie du cœur dérive de KDE KFileMetaData (en-têtes SPDX Vishesh Handa 2012/2014 et Alexander Stippich 2018 dans src/).
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (615 Ko) + en-têtes (88 Ko), CMake. 'other' (36,9 Mo) = échantillons de test (images, documents) — 248 fichiers, 37,7 Mo (dont test.DNG de 22,7 Mo) — lignes de code propres mesurées : 19 478
- **Privilège** : bibliothèque et plugins chargés dans le processus hôte (indexeur de recherche, gestionnaire de fichiers : PROBABLE)
- **Déclencheurs** : extraction de métadonnées ou de miniatures sur des fichiers rencontrés (contenu non fiable)
- **Indices réseau** : aucun
- **Blobs** : 6 blobs de test : autotests/samplefiles/test.AppImage (ELF, 208 Ko) et 5 archives ZIP de test (moins de 210 octets). Échantillons volumineux (DNG 22,7 Mo, GIF 3,7 Mo, TIFF/BMP/XPM)
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Bibliothèque de parsing de formats non fiables (bureautique, PDF, OFD/UOF, ZIP, médias) : lecture exhaustive des extracteurs et du lecteur ZIP, triage du reste.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| plugins | `archive:debian/libukui-file-metadata-bin.install` | usr/lib/*/ukui-file-metadata/plugins/libukuifilemetadata_*.so : extracteurs office, office2007, ofd, uof, pdf, ffmpeg, taglib, image, png, text, bookmarks (src/extractors/) | CONSTATÉ |
| bibliotheque | `archive:debian/libukui-file-metadata1.install` | libukui-file-metadata.so.* (src/ : zip-reader.cpp, ocr-utils.cpp, thumbnail.cpp, extractor-manager.cpp) | CONSTATÉ |
| build | `archive:CMakeLists.txt` | add_subdirectory(src) ; autotests/ et tests/ seulement si BUILD_TESTING ; dépendance Poppler | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake ; RelWithDebInfo ; pas d'option hardening explicite | CONSTATÉ |

- [CONSTATÉ] L'arbre source contient un exécutable ELF (autotests/samplefiles/test.AppImage) utilisé comme échantillon de test.
- [CONSTATÉ] [correction critique] Reclassé fork-amont (partiel) : code dérivé de KFileMetaData, KDE ; version amont à établir en Phase 2.

### `ukui-sni`

- **Nature** : fork-amont — **Amont** : Kylin (gitee.com/openkylin/ukui-sni), 4.24.0.0-ok0.1, format 3.0 (quilt), 0 patch. xembed-sni-proxy et status-notifier-watcher dérivent de KDE plasma-workspace (SPDX David Edmundson 2015/2017, Konrad Materka 2019, Martin Graesslin 2012-2013).
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (248 Ko) + en-têtes (56 Ko), QML (44 Ko), CMake. 'HTML' = documentation QtSingleApplication — 163 fichiers, 694 Ko — lignes de code propres mesurées : 10 680
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session (phase Initialization), enregistrement d'éléments SNI par toute application via le bus de session, fenêtres d'icônes X11 XEmbed
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Services de session qui reçoivent des données de toute application (StatusNotifierWatcher D-Bus, XEmbed X11), environ 250 Ko de C++ dont une partie dérivée de KDE.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart | `archive:status-notifier-watcher/ukui-sni-watcher.desktop` | Exec=ukui-sni-watcher ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Phase=Initialization ; installé dans /etc/xdg/autostart | CONSTATÉ |
| autostart | `archive:xembed-sni-proxy/xembed-sni-proxy.desktop` | Exec=xembed-sni-proxy ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Phase=Initialization ; installé dans /etc/xdg/autostart | CONSTATÉ |
| debian-install-lists | `archive:debian/ukui-sni-watcher.install` | etc/xdg/autostart/* et usr/bin/* | CONSTATÉ |
| widget-qml | `archive:debian/ukui-widget-system-tray.install` | /usr/lib/*/qt5/qml/org/ukui/systemTray/* et /usr/share/ukui/widgets/org.ukui.systemTray/* | CONSTATÉ |
| socket-local | `archive:3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer listen l.89 | CONSTATÉ |

- [CONSTATÉ] debian/copyright contient la ligne gabarit « This software is Copyright (c) 2023 by X. Ample. ».
- [CONSTATÉ] [correction critique] Reclassé fork-amont (partiel) : code dérivé de plasma-workspace (xembed-sni-proxy), KDE ; version amont à établir en Phase 2.

### `ukui-ui-config-service`

- **Nature** : propre-kylin — **Amont** : Kylin (xibowen@kylinos.cn), 4.20.0.0-ok0.3, format 3.0 (quilt), 1 patch, absent de Debian et d'Ubuntu
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (83 Ko) + en-têtes (29 Ko), CMake — 19 fichiers, 117 Ko — lignes de code propres mesurées : 2 738
- **Privilège** : utilisateur de session
- **Déclencheurs** : activation D-Bus de session
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petit service D-Bus de session (19 fichiers) qui lit la configuration de thème, non installé par défaut sur l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-session-service | `archive:data/com.kylin.ThemeConfig.service.in` | Name=com.kylin.ThemeConfig ; Exec=${LIBEXEC_PATH}/ukui-ui-config-service ; installé dans /usr/share/dbus-1/services | CONSTATÉ |
| desktop | `archive:data/com.kylin.ThemeConfig.desktop.in` | Exec=${LIBEXEC_PATH}/ukui-ui-config-service ; NoDisplay=true ; installé dans /usr/share/applications | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; dh_clean supprime les fichiers générés | CONSTATÉ |

- [CONSTATÉ] Le paquet est absent de installe_par_defaut_iso.

### `ukui-shell-integration`

- **Nature** : propre-kylin — **Amont** : Kylin (gitee.com/openkylin/ukui-shell-integration), 1.0.0-ok5, format 3.0 (quilt), 16 patches. Protocoles Wayland ukui-shell, xdg-shell, xdg-decoration-unstable-v1, xdg-dialog-v1.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète). Pas de chemin Gitee (forges en IDENTICAL-HEADS, branche nile uniquement).
- **Langages / taille** : C++ (65 Ko) + en-têtes (9 Ko). XML = protocoles Wayland — 53 fichiers, 321 Ko — lignes de code propres mesurées : 2 241
- **Privilège** : plugin chargé dans les applications Qt sous Wayland (privilège de l'hôte)
- **Déclencheurs** : démarrage d'une application Qt en session Wayland UKUI
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Plugin client Wayland à faible privilège (65 Ko de C++), non installé par défaut sur l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| plugin-qt-wayland | `archive:CMakeLists.txt` | QT_PLUGINS_INSTALL_DIR=/usr/lib/<arch>/qt5/plugins/ ; add_subdirectory(src) (plugin d'intégration shell, outil src/tools/ukuiwaylandscanner.cpp) ; examples/ seulement en build Debug | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ minimal | CONSTATÉ |

- [CONSTATÉ] Le paquet est absent de installe_par_defaut_iso.

### `ukui-air-gesture`

- **Nature** : propre-kylin — **Amont** : Kylin (韩美霖, hanmeilin@kylinos.cn), 4.24.0.0-ok1.13, format 3.0 (native). Aucune forge (Gitee et GitHub ABSENT). Absent de Debian et d'Ubuntu.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source Kylin complète). Les dépendances Python (mediapipe 0.10.14, jax 0.10.1, jaxlib 0.10.1, ImageHash 4.3.1, Pillow 10.4.0, dbus-next 0.2.3) sont téléchargées dans un venv pendant l'installation du build et ne figurent pas dans l'arbre.
- **Langages / taille** : C++ (76 Ko) + en-têtes (26 Ko), Python (63 Ko), Shell (4 Ko), CMake. 'other' (23 Mo) = GIF/PNG et modèle — 67 fichiers, 23,5 Mo. Environ 20 fichiers de code — lignes de code propres mesurées : 4 993
- **Privilège** : utilisateur de session (accès caméra) ; root via pkexec sans authentification pour la session active (ensure-vcam) ; /dev/uinput en mode 0666 pour tous les utilisateurs
- **Déclencheurs** : ouverture de session (autostart), pkexec ensure-vcam (création de caméra virtuelle), installation du paquet (udevadm trigger)
- **Indices réseau** : téléchargement au build depuis pypi.tuna.tsinghua.edu.cn avec --trusted-host; caméra virtuelle partagée (app/python/virtual_camera.py) : PROBABLE
- **Blobs** : app/python/hand_landmarker.task (archive ZIP de 7,8 Mo, modèle MediaPipe) non signalé par l'extraction automatique. Venv de roues binaires PyPI produit au build, absent de l'arbre
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Code Kylin réduit (environ 20 fichiers) mais élévation pkexec allow_active=yes, /dev/uinput en 0666 et capture caméra ; le contenu du venv PyPI (binaires) relève du niveau X.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| polkit | `archive:org.ukui.airgesture.ensure-vcam.policy` | action org.ukui.airgesture.ensure-vcam : allow_any=no, allow_inactive=no, allow_active=yes ; exec.path=/usr/libexec/ukui-air-gesture/ensure-vcam ; exec.allow_gui=true ; installé dans /usr/share/polkit-1/actions | CONSTATÉ |
| script-root (contenu) | `archive:ensure-vcam.sh` | installé sous le nom /usr/libexec/ukui-air-gesture/ensure-vcam (install PROGRAMS), exécutable en root via pkexec ; contenu non lu | NON COUVERT |
| udev | `archive:99-ukui-air-gesture-uinput.rules` | KERNEL=="uinput", MODE="0666" ; installé dans /lib/udev/rules.d | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | configure : udevadm control --reload-rules puis udevadm trigger --action=add --sysname-match=uinput (application immédiate des permissions de /dev/uinput) | CONSTATÉ |
| autostart | `archive:app/org.ukui.air.gesture.desktop` | Exec=/usr/bin/ukui-air-gesture ; OnlyShowIn=UKUI ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart (app/CMakeLists.txt) | CONSTATÉ |
| python-runtime | `archive:app/CMakeLists.txt` | python/*.py installés dans /usr/share/ukui-air-gesture/python ; AIR_GESTURE_VENV_PYTHON=/usr/libexec/ukui-air-gesture/venv/bin/python exécute main.py ; modèle hand_landmarker.task dans /usr/share/ukui-air-gesture/models | CONSTATÉ |
| build-reseau | `archive:app/CMakeLists.txt` | install(CODE) : python -m venv puis pip install --no-cache-dir -i https://pypi.tuna.tsinghua.edu.cn/simple --trusted-host pypi.tuna.tsinghua.edu.cn -r requirements.txt | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh_dwz, dh_strip et dh_shlibdeps --exclude=venv (le venv est inclus dans le paquet) | CONSTATÉ |
| plugin | `archive:plugin/CMakeLists.txt` | ukui-air-gesture-plugin (lié à ukcc) installé dans ${LIBDIR}/accessibility-plugin | CONSTATÉ |

- [CONSTATÉ] Le paquet est absent de installe_par_defaut_iso et n'a aucune forge publique : l'archive est la seule source.
- [PROBABLE] Le contenu binaire empaqueté (venv) dépend de l'état du miroir PyPI au moment du build ; la reproductibilité n'est pas garantie par l'arbre source.

### `ukui-previewhub`

- **Nature** : propre-kylin — **Amont** : Kylin (UKUI Team, team@kylinos.cn), 4.24.0.0-ok5, format 3.0 (quilt), 2 patches. Gitee présent, GitHub ABSENT. Embarque pdf.js 2.0.550 (build 76337fdc) via core/third_party/qpdflib.
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (110 Ko) + en-têtes (46 Ko), JavaScript (2,56 Mo, pdf.js embarqué), CSS, HTML, CMake — 524 fichiers, 5,2 Mo — lignes de code propres mesurées : 4 393
- **Privilège** : bibliothèque dans le processus hôte (gestionnaire de fichiers ou recherche : PROBABLE) ; python3 http.server lancé avec les droits de l'utilisateur
- **Déclencheurs** : prévisualisation d'un fichier (bureautique via onlyoffice, PDF, image, audio, HTML, texte)
- **Indices réseau** : python3 -m http.server -b localhost 9124 (core/previewpage/onlyoffice.cpp l.66)
- **Blobs** : aucun binaire exécutable (cmaps .bcmap de pdf.js)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Code propre modeste (110 Ko de C++) qui lance un serveur HTTP local et rend des fichiers non fiables (HTML, PDF) ; pdf.js embarqué traité par sa version (2.0.550).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| listener-tcp-local | `archive:core/previewpage/onlyoffice.cpp` | l.64-72 : QProcess python3 -m http.server -d <path> -b localhost 9124, lancé en startDetached (serveur HTTP TCP local qui sert un répertoire) | CONSTATÉ |
| bibliotheque | `archive:debian/libukui-previewhub1.install` | libukui-previewhub.so.1* et traductions | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem=cmake ; hardening=+all ; RelWithDebInfo | CONSTATÉ |
| conversion-externe | `archive:core/libreoffice-runtime.cpp` | fichier dédié à l'exécution de LibreOffice (conversion de documents) ; contenu non lu | PROBABLE |

- [CONSTATÉ] Le paquet est absent de installe_par_defaut_iso.
- [CONSTATÉ] pdf.js est embarqué en version 2.0.550 (core/third_party/qpdflib/pdfview/pdf.js l.126).

### `ukui-i18n-management`

- **Nature** : propre-kylin — **Amont** : Kylin, 1.0.0-ok2, format 3.0 (native). Aucune forge (Gitee et GitHub ABSENT). debian/control indique le mainteneur gabarit « Maintainer <maintainer@example.com> ».
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète : 2 scripts)
- **Langages / taille** : Shell (15 Ko), Python (5 Ko) — 16 fichiers, 36 Ko — lignes de code propres mesurées : 623
- **Privilège** : root (service oneshot systemd et script dispatcher NetworkManager)
- **Déclencheurs** : démarrage après network-online.target, NetworkManager connectivity-change à l'état full, installation du paquet (enable + start)
- **Indices réseau** : After=network-online.target ; dépendance iputils-ping (test de connectivité : PROBABLE); déclenchement par l'état de connectivité NetworkManager
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Service root déclenché par des événements réseau, deux scripts (environ 20 Ko) : lecture intégrale peu coûteuse.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:lib/systemd/system/ukui-i18n-management.service` | Type=oneshot ; ExecStart=/usr/lib/ukui-i18n-management/check-packages.sh ; pas de User= (root) ; After/Wants=network-online.target ; ConditionPathExists=/etc/ukui-i18n-management/packages.conf ; RemainAfterExit=yes ; WantedBy=multi-user.target | CONSTATÉ |
| nm-dispatcher | `archive:etc/NetworkManager/dispatcher.d/99-ukui-i18n-management` | exécuté en root par NetworkManager. Sur connectivity-change avec CONNECTIVITY_STATE=full : sleep 2 puis /bin/systemctl restart ukui-i18n-management.service ; journalisation via logger | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | configure : deb-systemd-helper unmask, update-state et enable, puis deb-systemd-invoke start (repli systemctl) | CONSTATÉ |
| maintainer-script | `archive:debian/prerm` | remove\|upgrade\|deconfigure : disable puis mask du service | CONSTATÉ |
| maintainer-script | `archive:debian/postrm` | purge et unmask du service, ou daemon-reload | CONSTATÉ |
| debian-rules | `archive:debian/rules` | override_dh_auto_install : packages.conf (644), check-packages.sh et send-notification.py (755) dans /usr/lib/ukui-i18n-management, unité systemd, dispatcher (755) | CONSTATÉ |
| script-root (contenu) | `archive:usr/lib/ukui-i18n-management/check-packages.sh` | ExecStart du service root (15 Ko) ; send-notification.py associé ; contenu non lu | NON COUVERT |
| debian-control | `archive:debian/control` | Depends : systemd, python3-dbus, python3-gi, iputils-ping \| inetutils-ping ; Recommends : network-manager | CONSTATÉ |

- [PROBABLE] La description du paquet prévoit l'affichage d'une notification de bureau depuis ce service root (send-notification.py) ; le mécanisme de passage vers la session utilisateur n'est pas lu.
- [CONSTATÉ] Pas de forge publique : l'archive est la seule source.

### `qt5-gesture-extensions`

- **Nature** : propre-kylin — **Amont** : Kylin (wangweinan@kylinos.cn), 4.0.1.0-ok0.3, format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète)
- **Langages / taille** : C++ (9 Ko) + en-têtes (9 Ko), qmake — 26 fichiers, 58 Ko — lignes de code propres mesurées : 585
- **Privilège** : bibliothèque (privilège de l'hôte) ; postinst exécuté en root
- **Déclencheurs** : installation du paquet, ouverture de session X (Xsession.d)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Très petite bibliothèque de gestes (5 fichiers de code), à faible privilège ; seul le postinst agit sur d'autres paquets.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/libqt5-gesture-extensions1.postinst` | #!/bin/bash, sans set -e : deb-systemd-helper stop et disable touchegg.service, daemon-reload ; rm /usr/share/X11/touchscreen/01-kylin.conf et /etc/xdg/autostart/touchegg.desktop (fichiers d'autres paquets) | CONSTATÉ |
| xsession.d | `archive:etc/Xsession.d/101qt-dist-threshold` | installé dans /etc/X11/Xsession.d : export QT_DBL_TAP_DIST=30 et QT_DBL_CLICK_DIST=30 | CONSTATÉ |
| bibliotheque | `archive:qt5-gesture-extensions.pro` | TEMPLATE=lib ; target.path=$$[QT_INSTALL_LIBS] ; en-têtes dans /usr/include/qt5-gesture-extensions | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ minimal | CONSTATÉ |

- [CONSTATÉ] Le postinst désactive touchegg.service et supprime des fichiers appartenant à d'autres paquets.
- [CONSTATÉ] Le paquet binaire qt5-gesture-extensions (Architecture all) n'a pas de fichier .install.

### `ukui-volume-control`

- **Nature** : propre-kylin — **Amont** : Kylin (gitee.com/openkylin/ukui-volume-control ; GitHub ABSENT). D'après meta : 4.20.0.1-ok1, format 3.0 (quilt), dépôt ppa-anything3.0, orig IDENTIQUE à Debian/Ubuntu.
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). L'arbre correspond à 4.20.0.0-ok3.25 (en-tête du changelog, format 3.0 native), c'est-à-dire à la version de l'ISO, et non à la version meta 4.20.0.1-ok1.
- **Langages / taille** : C++ (1,04 Mo) + en-têtes (344 Ko), QML (34 Ko), CMake. 'TypeScript' = traductions — 313 fichiers, 1,70 Mo — lignes de code propres mesurées : 42 743
- **Privilège** : utilisateur de session (unité systemd --user) ; plugin dans ukui-control-center
- **Déclencheurs** : démarrage de graphical-session (systemd --user), appels D-Bus de session vers le backend (backend/DbusServer.cpp : PROBABLE)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Démon audio de session volumineux (1 Mo de C++ : PulseaudioManager, DbusServer), sans privilège root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:data/ukui-volume-control.service` | ExecStart=/usr/bin/ukui-audio-service ; Type=simple ; After=pulseaudio.service ; PartOf=graphical-session.target ; WantedBy=default.target ; installé dans /usr/lib/systemd/user/ (CMakeLists.txt l.19) | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-volume-control.preinst` | dpkg-divert --package ukui-media (nom d'un autre paquet) --rename de .../ukui-control-center/libaudio.so | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-volume-control.postinst` | glib-compile-schemas | CONSTATÉ |
| autostart | `archive:data/ukui-volume-control.desktop` | installé dans /etc/xdg/autostart (debian/ukui-volume-control.install) ; ligne Exec commentée (#Exec=ukui-audio-service) | CONSTATÉ |
| debian-install-lists | `archive:debian/ukui-volume-control.install` | usr/bin, usr/lib/systemd/user, plugin ukui-control-center, widget et module QML org.ukui.shortcut.audio | CONSTATÉ |
| debian-rules | `archive:debian/rules` | cmake ; WITH_LIBUDEV=ON ; dh_installinit --no-start ; hardening=+all | CONSTATÉ |

- [CONSTATÉ] L'en-tête du changelog (4.20.0.0-ok3.25) et debian/source/format (3.0 native) divergent de meta.version (4.20.0.1-ok1) et de meta.format (3.0 quilt).
- [CONSTATÉ] ukui-media et ukui-volume-control, tous deux installés par l'ISO, détournent tous deux libaudio.so.

### `ukui-desktop-environment`

- **Nature** : meta-ou-donnees — **Amont** : Kylin/UKUI (gitee.com/openkylin/ukui-desktop-environment), 4.20.0.0-ok3.1, métapaquets
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (debian/ seulement : métapaquets, aucun code)
- **Langages / taille** : aucun code (debian/control, debian/rules, gschema override) — 8 fichiers, 46 Ko — lignes de code propres mesurées : 0
- **Privilège** : aucun (pas de binaire)
- **Déclencheurs** : installation (tire les dépendances)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Métapaquet sans code : seuls debian/control et rules sont à vérifier, pour le graphe de dépendances.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 3 métapaquets. ukui-desktop-environment-core dépend de peony, ukui-control-center, ukui-polkit, ukui-session-manager, ukui-settings-daemon, ukui-service-manager… ; ukui-desktop-environment dépend de kylin-nm, kylin-virtual-keyboard, ukui-media, ukui-power-manager, ukui-screensaver, kylin-device-daemon… | CONSTATÉ |
| debian-rules | `archive:debian/rules` | override_dh_install : installe debian/ukui-ubuntu.gschema.override uniquement si dpkg-vendor --is Ubuntu | CONSTATÉ |

- [CONSTATÉ] ukui-desktop-environment-extras est déclaré dans control mais absent de la liste ISO.

### `kylin-ki18n`

- **Nature** : propre-kylin — **Amont** : Kylin (liweinan@kylinos.cn). Réimplémentation (backend Rust ki18n_rs + façade C++) de l'API/ABI KDE KI18n 5.92.0 (libKF5I18n.so.5). Données ki18n-data copiées de l'amont KDE ki18n (debian/README.source). Aucune forge (Gitee et GitHub ABSENT) ; debian/watch pointe vers gitee.com/openkylin/kylin-ki18n/rs.
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source Rust/C++ complète). Les 166 crates de Cargo.lock (source registry, avec checksums) ne sont pas incluses ; .cargo/config.toml redirige crates.io vers mirrors.aliyun.com.
- **Langages / taille** : Rust (612 Ko), C++ (219 Ko) + en-têtes (158 Ko), Python (68 Ko, scripts de génération), JavaScript (111 Ko), JSON (939 Ko), CMake + Cargo — 315 fichiers, 13,5 Mo — lignes de code propres mesurées : 34 737
- **Privilège** : bibliothèque chargée par toutes les applications KF5 (y compris celles lancées en root), sans processus propre
- **Déclencheurs** : chargement par une application KF5, évaluation des scripts de transcription LC_SCRIPTS via ktranscript (PROBABLE)
- **Indices réseau** : téléchargement de 166 crates au build depuis mirrors.aliyun.com (aucun vendoring)
- **Blobs** : data/spatial_index_entries.bin (1,1 Mo, données binaires ; scripts de génération présents dans tools/localedata-gen et scripts/)
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Bibliothèque qui remplace l'ABI KDE KI18n dans toutes les applications KF5, avec un moteur JS pour la transcription ; environ 830 Ko de Rust/C++ ; les crates tierces ne sont pas dans l'arbre.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| bibliotheque | `archive:debian/libkf5i18n5.install` | usr/lib/*/libKF5I18n.so.5* et plugin usr/lib/*/qt5/plugins/kf5/ktranscript.so | CONSTATÉ |
| donnees-scripts | `archive:debian/libkf5i18n-data.install` | catalogues ki18n5.mo et répertoires usr/share/locale/*/LC_SCRIPTS/ki18n5/ (scripts de transcription) ; catégories qlogging | CONSTATÉ |
| bibliotheque | `archive:debian/libkf5i18nlocaledata5.install` | libKF5I18nLocaleData.so.5* ; module QML org/kde/i18n/localeData | CONSTATÉ |
| debian-rules | `archive:debian/rules` | CARGO_HOME=debian/cargo-home ; SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt ; cargo build --release puis cmake --build ; dh_auto_test : cargo test et ctest ; hardening=+all | CONSTATÉ |
| build-reseau | `archive:.cargo/config.toml` | [source.crates-io] replace-with=aliyun ; registry=sparse+https://mirrors.aliyun.com/crates.io-index/ ; pas de répertoire vendor/ | CONSTATÉ |
| dependances | `archive:Cargo.toml` | boa_engine 0.16 (moteur JavaScript), regex, quick-xml, serde_json, chrono =0.4.38, sys-locale, etc. | CONSTATÉ |

- [CONSTATÉ] L'en-tête du changelog (5.92.0-ok3.7) diffère de meta.version et de la version ISO (5.92.0-ok4).
- [CONSTATÉ] Les noms de paquets binaires (libkf5i18n5, libkf5i18n-data, libkf5i18n-dev…) sont identiques à ceux du source KDE ki18n de Debian.

### `time-shutdown`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI (github.com/ukui/time-shutdown, dépôt GitHub EMPTY), 4.10.0.0-ok0.17update1, format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (90 Ko) + en-têtes (27 Ko), qmake. 'TypeScript' = traductions — 62 fichiers, 403 Ko — lignes de code propres mesurées : 3 677
- **Privilège** : utilisateur de session
- **Déclencheurs** : ouverture de session (démon en autostart), planification d'arrêt par l'utilisateur
- **Indices réseau** : QLocalServer (QtSingleApplication) : socket local
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petite application de session (90 Ko de C++) sans privilège propre ; l'arrêt passe par les services système.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart | `archive:time-shutdown-daemon/resources/time-shutdown-daemon.desktop` | Exec=time-shutdown-daemon ; NoDisplay=true ; X-UKUI-AutoRestart=true ; installé dans /etc/xdg/autostart/ (time-shutdown-daemon.pro) | CONSTATÉ |
| binaires | `archive:time-shutdown.pro` | SUBDIRS time-shutdown (GUI) et time-shutdown-daemon, tous deux installés dans /usr/bin ; schéma gsettings ; guide utilisateur | CONSTATÉ |
| socket-local | `archive:time-shutdown/qtlocalpeer.cpp` | QLocalServer listen l.121 | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; CFLAGS -Wall -pedantic | CONSTATÉ |

- [NON COUVERT] Le mécanisme d'arrêt (logind ou session manager) n'est pas identifié en Phase 0.2.

### `peony-print-pictures`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git github.com/ukui/peony-extensions), 1.0.2.0-ok7, format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète). Pas de chemin Gitee.
- **Langages / taille** : C++ (218 Ko) + en-têtes (52 Ko), CMake. Shell et Python = scripts de traduction exécutés au build — 68 fichiers, 522 Ko — lignes de code propres mesurées : 7 290
- **Privilège** : utilisateur de session (plugin dans peony, application d'impression)
- **Déclencheurs** : action d'impression d'images depuis peony, lancement de kylin-photo-printer
- **Indices réseau** : aucun
- **Blobs** : aucun (tests/testfile.jpg comme échantillon)
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Extension et application de session à faible privilège (218 Ko de C++) ; à noter : décodage d'images et interfaces Wayland privilégiées demandées dans le .desktop.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| plugin-peony | `archive:peony-print-pictures/CMakeLists.txt` | install(TARGETS) dans ${QT_INSTALL_LIBS}/peony-extensions (chargé dans le gestionnaire de fichiers peony) | CONSTATÉ |
| application | `archive:kylin-photo-printer/CMakeLists.txt` | kylin-photo-printer installé dans /usr/bin ; schéma org.kylin.photo.printer.gschema.xml | CONSTATÉ |
| desktop | `archive:kylin-photo-printer.desktop.in` | Exec=/usr/bin/kylin-photo-printer ; X-KDE-Wayland-Interfaces=org_kde_plasma_window_management, org_kde_plasma_activation_feedback, org_kde_kwin_keystate, org_kde_kwin_fake_input, zkde_screencast_unstable_v1 | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | glib-compile-schemas | CONSTATÉ |
| build | `archive:CMakeLists.txt` | 3rdparty/kylinimagecodec (chargement et conversion d'images) ; execute_process python3 copy_translations.py, bash update_translations.sh et apply_translations.sh au configure | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; hardening=+all | CONSTATÉ |

- [CONSTATÉ] Le fichier .desktop demande les interfaces Wayland org_kde_kwin_fake_input (injection d'entrées) et zkde_screencast_unstable_v1 (capture d'écran).

#### Constats transverses du lot B10-ukui-2

- **[CONSTATÉ]** Le lot compte 26 paquets, tous avec un chemin archive et sans chemin Gitee (struct_gitee null). Tous sauf ukui-desktop-environment (métapaquet) contiennent du code source Kylin ; aucun n'est une coquille vide. — *preuve* : digest/B10-ukui-2.json : chemin_gitee null et meta.source_coquille_vide 0 pour les 26 entrées
- **[CONSTATÉ]** Six points d'entrée s'exécutent en root par déclaration : ukui-powermanagement-service (D-Bus système, User=root), kylin-assistant-systemdaemon.py (D-Bus système, User=root), bluetoothService, ukui-media-control-led et ukui-i18n-management (unités systemd sans User=), plus le script udev soundCardAdaptation.sh. — *preuve* : ukui-power-manager/registeredQDbus/conf/org.ukui.powermanagement.service « User=root » ; youker-assistant/kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.service « User=root » ; ukui-bluetooth/data/ukui-bluetooth.service « ExecStart=/usr/bin/bluetoothService » ; ukui-media/data/ukui-media-control-mute-led.service ; ukui-i18n-management/lib/systemd/system/ukui-i18n-management.service ; ukui-media/configuration/99-ukui-media.rules « RUN+= »
- **[CONSTATÉ]** Cinq politiques D-Bus système installées autorisent le contexte default à appeler le service (send_destination). org.bluez.Agent1.conf autorise en outre le contexte default à posséder le nom (allow own). — *preuve* : ukui-power-manager/registeredQDbus/conf/org.ukui.powermanagement.conf ; ukui-bluetooth/data/com.ukui.bluetooth.conf, org.bluez.obex.conf, org.bluez.Agent1.conf « <policy context="default"> <allow own="org.bluez.Agent1"/> » ; youker-assistant/kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.conf
- **[CONSTATÉ]** Deux paquets accordent des privilèges à l'installation : ukui-system-monitor pose des capacités fichier (setcap cap_net_raw,cap_dac_read_search+ep) avec repli setuid root ; ukui-air-gesture rend /dev/uinput accessible à tous (MODE 0666) et déclare une action polkit allow_active=yes pour un script exécuté par pkexec. — *preuve* : ukui-system-monitor/debian/ukui-system-monitor.postinst « chmod u+s /usr/bin/ukui-system-monitor » ; ukui-air-gesture/99-ukui-air-gesture-uinput.rules « KERNEL=="uinput", MODE="0666" » ; ukui-air-gesture/org.ukui.airgesture.ensure-vcam.policy « <allow_active>yes</allow_active> »
- **[CONSTATÉ]** Deux paquets téléchargent des dépendances absentes de l'arbre source pendant le build : ukui-air-gesture (pip depuis pypi.tuna.tsinghua.edu.cn avec --trusted-host, venv empaqueté) et kylin-ki18n (166 crates via le miroir mirrors.aliyun.com, sans vendoring). — *preuve* : ukui-air-gesture/app/CMakeLists.txt « --trusted-host pypi.tuna.tsinghua.edu.cn » ; kylin-ki18n/.cargo/config.toml « registry = "sparse+https://mirrors.aliyun.com/crates.io-index/" »
- **[CONSTATÉ]** Quatre paquets détournent (dpkg-divert) un plugin de ukui-control-center. ukui-media et ukui-volume-control détournent le même chemin libaudio.so, tous deux sous --package ukui-media. — *preuve* : ukui-power-manager/debian/ukui-power-manager.preinst (libpower.so) ; ukui-media/debian/ukui-media.preinst (libaudio.so) ; ukui-bluetooth/debian/ukui-bluetooth.preinst (libukcc-bluetooth.so) ; ukui-volume-control/debian/ukui-volume-control.preinst « dpkg-divert --package ukui-media »
- **[CONSTATÉ]** Neuf paquets embarquent chacun une copie de QtSingleApplication (Qt Solutions) avec un QLocalServer (socket local). La copie de ukui-window-switch-wlcom est modifiée. Seul listener TCP du lot : python3 http.server sur localhost:9124 dans ukui-previewhub. — *preuve* : qtlocalpeer.cpp « server->listen(socketName) » dans ukui-media, ukui-system-monitor, ukui-framework, ukui-window-switch, kylin-virtual-keyboard, youker-assistant, ukui-app-widget, ukui-sni, time-shutdown ; ukui-previewhub/core/previewpage/onlyoffice.cpp l.66 « -m http.server … -b localhost 9124 »
- **[CONSTATÉ]** Des paquets classés propre-kylin contiennent du code dérivé de KDE : ukui-sni (xembed-sni-proxy, plasma-workspace), ukui-file-metadata (KFileMetaData), ukui-window-switch (switcher KWin thumbnail_grid). kylin-ki18n réimplémente l'ABI KDE KI18n 5.92.0 sous les mêmes noms de paquets binaires que Debian. — *preuve* : ukui-sni/xembed-sni-proxy/sni-proxy.cpp « SPDX-FileCopyrightText: 2015 David Edmundson » ; ukui-file-metadata/src/*.cpp « Vishesh Handa » ; ukui-window-switch/windowswitchers/thumbnail_grid/metadata.desktop « X-KDE-PluginInfo-Author=Chris Holland » ; kylin-ki18n/debian/libkf5i18n5.install
- **[CONSTATÉ]** Cinq paquets du lot sont absents de la liste des paquets installés par défaut sur l'ISO 3.0 : youker-assistant, ukui-ui-config-service, ukui-shell-integration, ukui-air-gesture, ukui-previewhub. — *preuve* : digest/B10-ukui-2.json : installe_par_defaut_iso = [] pour ces 5 entrées
- **[CONSTATÉ]** Trois paquets n'ont aucune forge publique (Gitee et GitHub ABSENT) : ukui-air-gesture, ukui-i18n-management, kylin-ki18n. L'archive est leur seule source. — *preuve* : digest/B10-ukui-2.json : forges.gitee=ABSENT, forges.github=ABSENT
- **[CONSTATÉ]** L'arbre extrait diverge des métadonnées pour deux paquets : ukui-volume-control (changelog 4.20.0.0-ok3.25 native, contre meta 4.20.0.1-ok1 quilt) et kylin-ki18n (changelog 5.92.0-ok3.7, contre meta et ISO 5.92.0-ok4). — *preuve* : ukui-volume-control/debian/changelog l.1 « (4.20.0.0-ok3.25) huanghe » ; kylin-ki18n/debian/changelog l.1 « (5.92.0-ok3.7) huanghe »
- **[PROBABLE]** Deux scripts mainteneur ne correspondent à aucun paquet binaire déclaré, et sont donc probablement inertes : ukui-media-server.postinst (systemctl enable ukui-media-server.service) et kylin-assistant.postinst (le paquet s'appelle youker-assistant). — *preuve* : ukui-media/debian/control (Package: ukui-media, ukui-media-common) ; youker-assistant/debian/control (Package: youker-assistant)
- **[PROBABLE]** ukui-media est au format 3.0 (native) alors que debian/patches/series liste 63 patches, sans répertoire .pc : l'état d'application de ces patches dans l'arbre est indéterminé. — *preuve* : ukui-media/debian/source/format « 3.0 (native) » ; ukui-media/debian/patches/series (63 lignes)
- **[CONSTATÉ]** Deux fichiers Xsession.d du lot modifient l'environnement de toutes les sessions X : 50xdp-ukui (GTK_USE_PORTAL=1, GLIB_FORCE_USE_PORTAL=1, GLIB_USE_KYLIN_PROCESS_MANAGER=1) et 101qt-dist-threshold. — *preuve* : xdg-desktop-portal-ukui/data/50xdp-ukui ; qt5-gesture-extensions/etc/Xsession.d/101qt-dist-threshold

