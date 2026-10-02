# Lot B09-ukui-1

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-nm-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI : kylin-nm (Vcs-Git https://github.com/ukui/kylin-nm.git ; debian/watch : kylin-nm_*.orig.tar.gz). Paquet scindé du même amont que kylin-nm et libkylin-nm-base, version 4.24.0.0-ok0.2 (huanghe), 1 patch quilt (traduction russe)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, quilt appliqué 1/1) ; chemin_gitee null (dépôt Gitee PRESENT mais non acquis)
- **Langages / taille** : C++/Qt5 (qmake) ; 'TypeScript' = fichiers .ts de traduction Qt ; Shell marginal — 174 fichiers, 1,29 Mo ; ~74 fichiers de code hors tests (~15,4 k lignes) — lignes de code propres mesurées : 22 391
- **Privilège** : Session utilisateur (bibliothèques/greffons chargés par un processus hôte) ; aucun service root, aucun script mainteneur
- **Déclencheurs** : Chargement dynamique de /usr/lib/kylin-nm/lib{netconnect,wlanconnect}.so par un hôte non identifié dans l'arbre, Mise à jour du paquet (pas de maintainer script)
- **Indices réseau** : Aucun listener détecté (listen=[]); Client D-Bus système NetworkManager (configuration filaire/Wi-Fi, dont Wi-Fi entreprise : plugins/wlanconnect/hiddenwifi/entsecuritywidget.cpp)
- **Blobs** : Aucun (blob_count=0)
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session (~15 k lignes) sans privilège mais manipulant configuration réseau et secrets Wi-Fi via NetworkManager ; à auditer en différentiel avec plugins/ de kylin-nm (même amont).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet binaire kylin-nm-plugin ; Depends libkylin-nm-base, libkysdk-qtwidgets, network-manager (>=1.2.6) ; Rules-Requires-Root: no | CONSTATÉ |
| debian-rules | `archive:debian/rules` | qmake kylin-nm.pro ; hardening=+all ; dh_missing --fail-missing ; aucun fichier .install (installation via INSTALLS qmake) ; aucun script mainteneur dans debian/ | CONSTATÉ |
| cible-construite | `archive:kylin-nm.pro + archive:plugins/plugin.pro` | SUBDIRS = plugins/plugin.pro (netconnect, wlanconnect ; mobilehotspot commenté) + libnm-kylin (libnm-icon-kylin seul) ; greffons libnetconnect/libwlanconnect -> /usr/lib/kylin-nm ; libnm-icon-kylin -> /usr/lib | CONSTATÉ |
| code-non-construit | `archive:plugins/kylin-agent/kylinsecretagent.cpp` | Agent de secrets NetworkManager (53 Ko) présent dans l'arbre mais absent des SUBDIRS et des include .pri de netconnect/wlanconnect | PROBABLE |
| ipc-client | `archive:plugins/netconnect/netconnect.cpp ; archive:plugins/wlanconnect/wlanconnect.cpp` | Usage de QDBusConnection::systemBus (client D-Bus système, NetworkManager) | CONSTATÉ |

- [CONSTATÉ] Installé par défaut sur l'ISO 3.0 (kylin-nm-plugin=4.24.0.0-ok0.2).
- [NON COUVERT] Le consommateur de /usr/lib/kylin-nm/lib{netconnect,wlanconnect}.so n'est pas identifié dans le lot (kylin-nm installe ses greffons homonymes dans $$[QT_INSTALL_LIBS]/ukui-control-center).
- [PROBABLE] tests/kt-test-utils/cpp-stub/elfio.hpp (153 Ko) : utilitaire de test tiers, non construit.

### `libkylin-nm-base`

- **Nature** : propre-kylin — **Amont** : Kylin/UKUI : kylin-nm (Vcs-Git https://github.com/ukui/kylin-nm.git ; debian/watch kylin-nm_*.orig.tar.gz), version 4.24.0.0-ok0.2, 1 patch quilt (traduction mn_MN)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, quilt appliqué 1/1) + 1 binaire ELF précompilé versionné dans l'arbre ; Gitee non acquis
- **Langages / taille** : C++/Qt5 (qmake) ; .ts de traduction ; Shell (translate_generation.sh, fuzz.sh) — 207 fichiers, 4,46 Mo (dont blob ELF 3,2 Mo) ; ~47 fichiers de code hors tests/fuzz (~10,4 k lignes) — lignes de code propres mesurées : 25 890
- **Privilège** : Bibliothèques chargées en session utilisateur ; agent de secrets NetworkManager (mots de passe Wi-Fi/VPN via libsecret/libnma) ; aucun service root
- **Déclencheurs** : Chargement par les clients kylin-nm / kylin-nm-plugin, Enregistrement probable comme agent de secrets NetworkManager (d'après les noms kylinsecretagent/kylinagent)
- **Indices réseau** : Aucun listener détecté; Client D-Bus NetworkManager (libnm, KF5NetworkManagerQt)
- **Blobs** : 1 blob : kylin-keyring/kylin-keyring (ELF x86-64 ET_DYN, 3,2 Mo, magic 7f454c46) ; correspondance avec les sources et usage au build non vérifiés
- **Budget proposé** : L3 — 95 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — ~10 k lignes propres comprenant un agent de secrets NetworkManager (crypto/identité) et un binaire ELF précompilé à confronter aux sources.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet libkylin-nm-base ; Depends network-manager (>=1.22.10-1kylin29k3.6) ; Build-Depends libnma-dev, libsecret-1-dev, libcap-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | qmake kylin-nm.pro ; hardening=+all ; dh_missing --fail-missing ; aucun .install ni script mainteneur | CONSTATÉ |
| cible-construite | `archive:kylin-nm.pro` | SUBDIRS = libnm-kylin (libnm-base-kylin -> /usr/lib) + kylin-keyring/kylin-keyring.pro ; tests retirés (SUBDIRS -= tests) | CONSTATÉ |
| cible-construite | `archive:kylin-keyring/kylin-keyring.pro` | TEMPLATE=lib TARGET=kylin-keyring (kylinsecretagent.cpp, kylinagent.cpp, kylinpassworddialog.cpp) ; PKGCONFIG libnm libnma libsecret-1 libcap ; target.path=/usr/lib ; en-têtes -> /usr/include/kylin-nm/kylin-agent ; exécute translate_generation.sh au qmake (release) | CONSTATÉ |
| blob-binaire | `archive:kylin-keyring/kylin-keyring` | ELF 64 bits, x86-64 (e_machine 0x3e), e_type=3 ET_DYN, OSABI 03, 3 227 408 octets, exécutable (rwx), versionné à côté du .pro qui produit une cible du même nom | CONSTATÉ |

- [CONSTATÉ] Variante du même code d'agent présente dans kylin-nm-plugin/plugins/kylin-agent/ (kylinsecretagent.cpp 53 258 o vs 45 868 o ici).
- [NON COUVERT] Le blob ELF peut être un artefact de build oublié ou être embarqué dans le .deb ; non déterminé en phase 0.
- [CONSTATÉ] Répertoire fuzz/ et tests/ présents, non construits par défaut.

### `ukui-session-manager`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/ukui-session-manager), format 3.0 (native), 4.0.0.0-ok48
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, natif sans patches)
- **Langages / taille** : C++/Qt5 (CMake) ; Python (data/clean_all_jobs.py) ; Shell (data/ukui-session-wayland, 99ukui-environment) ; polkit JavaScript — 173 fichiers, 6,8 Mo (≈4,9 Mo de sons .wav et images) ; ~77 fichiers de code (~17 k lignes) — lignes de code propres mesurées : 24 222
- **Privilège** : Session utilisateur (ukui-session, ukuismserver, ukui-session-tools) ; également lancé comme greeter LightDM ('ukui-session simpleDesk', déclaré par ukui-screensaver) ; règles polkit système ; postinst root
- **Déclencheurs** : Ouverture de session X11 (xsessions/ukui.desktop) ou Wayland (ukui-session-wayland), Greeter LightDM via /usr/share/xgreeters/ukui-greeter.desktop de ukui-screensaver (Exec=ukui-session simpleDesk), Requêtes polkit d'hibernation (règle JS + pkla), postinst à l'installation/mise à jour
- **Indices réseau** : ICE/XSMP local (ukuismserver), transport TCP désactivé; Aucun autre listener détecté
- **Blobs** : Aucun exécutable ; ressources audio .wav (shutdown.wav 2 Mo, startup.wav 1,2 Mo…) et PNG
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — ~17 k lignes propres ; gestionnaire de session installant des règles polkit système (hibernation sans condition, pkla unix-user:*), un fichier Xsession.d, un script exécutable dans /etc et un postinst root modifiant un fichier du HOME.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 2 paquets : ukui-session-manager (Provides x-session-manager ; Recommends ukui-polkit, ukui-screensaver…) et ukui-session-wayland (Depends kwin-wayland) | CONSTATÉ |
| debian-install | `archive:debian/ukui-session-manager.install ; archive:debian/ukui-session-wayland.install` | etc/ukui/ukui-session/logout, etc/polkit-1/localauthority/50-local.d, etc/X11/Xsession.d, usr/share/xsessions, usr/bin/{ukui-session,ukui-session-tools,ukuismserver}, usr/lib/systemd/user, usr/share/polkit-1/{rules.d,actions} / usr/share/wayland-sessions, usr/bin/ukui-session-wayland | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-session-manager.postinst` | root : glib-compile-schemas ; sed -i sur /home/$SUDO_USER/.profile (supprime GDK_SCALE, QT_SCALE_FACTOR, QT_AUTO_SCREEN_SET_FACTOR) ; si NAME de /etc/os-release != openKylin : mv /usr/share/xsessions/ukui.desktop vers /etc/ukui/ukui-session/ | CONSTATÉ |
| polkit-rules | `archive:data/com.kylin.enable-hibernate.rules` | -> /usr/share/polkit-1/rules.d ; polkit.addRule retourne polkit.Result.YES pour org.freedesktop.upower.hibernate, login1.hibernate, login1.handle-hibernate-key, login1.hibernate-multiple-sessions, sans condition sur subject | CONSTATÉ |
| polkit-pkla | `archive:data/com.ubuntu.enable-hibernate.pkla` | -> /etc/polkit-1/localauthority/50-local.d ; Identity=unix-user:* ; ResultActive=yes pour upower.hibernate, login1.hibernate, handle-hibernate-key, org.freedesktop.login1, hibernate-multiple-sessions, hibernate-ignore-inhibit | CONSTATÉ |
| polkit-actions | `archive:data/org.gnome.SessionManager.policy` | 12 actions org.gnome.SessionManager.* (logout, reboot, poweroff, suspend, hibernate, switch-user, set-session-env, simulate-user-activity, start-app, start-module, stop-module, close-windows) ; allow_active=yes, allow_any/allow_inactive=auth_admin_keep | CONSTATÉ |
| systemd-user-target | `archive:data/ukui-session.target` | -> /usr/lib/systemd/user ; DefaultDependencies=no ; BindsTo= et Before=graphical-session.target | CONSTATÉ |
| session-desktop | `archive:data/ukui.desktop ; archive:data/ukui-wayland.desktop` | xsessions : Exec=ukui-session ; wayland-sessions : Exec=/usr/bin/ukui-session-wayland (X-LightDM-Session-Type=wayland) | CONSTATÉ |
| script-lanceur | `archive:data/ukui-session-wayland` | -> /usr/bin ; exporte GTK_USE_PORTAL=1, GLIB_FORCE_USE_PORTAL=1, GLIB_USE_KYLIN_PROCESS_MANAGER=1 ; exec [dbus-run-session] kwin_wayland --xwayland --drm --no-lockscreen --exit-with-session=/usr/bin/ukui-session | CONSTATÉ |
| xsession.d | `archive:data/99ukui-environment` | -> /etc/X11/Xsession.d (sourcé par Xsession) ; GTK_MODULES+=canberra-gtk-module ; GTK_OVERLAY_SCROLLING=0 si session ukui | CONSTATÉ |
| script-installé-etc | `archive:data/clean_all_jobs.py` | install(PROGRAMS) -> /etc/ukui/ukui-session/logout (script Python exécutable) ; contenu non lu | CONSTATÉ |
| desktop-file | `archive:data/ukui-session-tools.desktop` | Exec=/usr/bin/ukui-session-tools ; NoDisplay=true ; OnlyShowIn=UKUI | CONSTATÉ |
| listener-local | `archive:ukuismserver/ukuismserver.cpp` | SmsInitialize + IceListenForConnections (XSMP/ICE) précédé de _IceTransNoListen('tcp') avec onlyLocal=true : écoute locale uniquement | CONSTATÉ |
| cible-construite | `archive:CMakeLists.txt` | add_subdirectory ukui-session, tools, ukuismserver, data ; tests commentés ; mode 'simpleDesk' dans ukui-session/main.cpp | CONSTATÉ |
| profile (Xsession.d) [ajout critique] | `archive:ukui-session-manager/data/99ukui-environment` | Absent de la fiche et du recensement. Installe dans /etc/X11/Xsession.d (data/CMakeLists.txt:29 ; debian/ukui-session-manager.install:3). Pour les sessions ukui, exporte GTK_MODULES=canberra-gtk-module et GTK_OVERLAY_SCROLLING=0. | CONSTATÉ |

- [CONSTATÉ] La fiche classe data/com.kylin.enable-hibernate.rules en 'udev' : c'est une règle polkit JavaScript installée dans /usr/share/polkit-1/rules.d.
- [CONSTATÉ] Le postinst construit le chemin /home/$SUDO_USER/.profile ; SUDO_USER est vide hors exécution via sudo.
- [NON COUVERT] Le code qui vérifie les actions polkit org.gnome.SessionManager.* et le contenu de clean_all_jobs.py n'ont pas été lus.

### `ukui-settings-daemon`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/ukui-settings-daemon), 3.0 (native), 4.24.0.0-ok0.3 ; héritage MATE probable (Build-Depends libmate-desktop-dev, libmatekbd-dev, libmatemixer-dev ; traductions 'Maten asetuspalvelin'/'Mate-instellingenvoorziening' dans data/ukui-settings-daemon.desktop)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, natif)
- **Langages / taille** : C++/Qt5 (qmake) ; C (xsettings, locate-pointer) ; .ts/.po de traduction — 635 fichiers, 4,15 Mo ; ~359 fichiers de code (~62 k lignes) — lignes de code propres mesurées : 63 016
- **Privilège** : Démon de session utilisateur (ukui-settings-daemon + greffons) ; service D-Bus système root activable à la demande (authoritydbus) ; utilitaire save-param appelé par lightdm-session-setup.sh ; postinst root
- **Déclencheurs** : Autostart XDG phase Initialization (et simpledesk du greeter via ukui-screensaver), Activation D-Bus session org.ukui.SettingsDaemon, Activation D-Bus système com.kylin.ukui.SettingsDaemon par tout utilisateur (politique default), postinst
- **Indices réseau** : Client HTTP QNetworkAccessManager : plugins/gamma-manager/gamma-manager-location.cpp; URLs codées en dur (plugins/gamma-manager/gamma-manager-define.h) : http://ip-api.com/json/, https://location.services.mozilla.com/v1/geolocate?key=geoclue, http://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_simple_s6/?cityids=; NETLINK_CONNECTOR (proc events) dans authoritydbus root; QLocalServer local (kds)
- **Blobs** : Aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant volumineux (~62 k lignes) : L3 ciblé sur le service root authoritydbus (plugins/authority, ~3,9 k lignes), save-param (~0,9 k), le client réseau gamma-manager-location et le postinst ; greffons de session restants en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-settings-daemon (Depends ukui-polkit, imwheel, iio-sensor-proxy, xinput, dconf-cli, libevdev2…), ukui-settings-daemon-common, kylin-display-switch (transitionnel) | CONSTATÉ |
| debian-install | `archive:debian/ukui-settings-daemon.install ; archive:debian/ukui-settings-daemon-common.install` | /etc/ (arborescence entière issue du build), /usr/bin/, /usr/lib/*/ukui-settings-daemon/, /usr/share/dbus-1/, /opt/system/conf/usd/, /opt/system/resource/update-manifests/, /opt/system/conf/ukui-security-support/ ; common : glib-2.0, locale, ukui-settings-daemon, ukydisplayswitch | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-settings-daemon.postinst` | root : crée /etc/systemd/logind.conf.d/power-ignore.conf (HandlePowerKey=ignore) si absent ; test '[ -f"$touchpad_udev" ]' (sans espace, forme à un argument) puis rm -rf /etc/udev/hwdb.d/61-keyboard.hwdb ; copie com.kylin.ukui.SettingsDaemon.yaml vers /etc/kylin-config/basic/ ; crée /etc/dconf/profile/user + dconf update ; supprime libapp-proxy-service.so, son .ukui-settings-plugin et son schéma ; glib-compile-schemas | CONSTATÉ |
| dbus-system-service | `archive:plugins/authority/conf/com.kylin.ukui.SettingsDaemon.service` | -> /usr/share/dbus-1/system-services ; Name=com.kylin.ukui.SettingsDaemon ; Exec=/usr/bin/authoritydbus ; User=root | CONSTATÉ |
| dbus-policy | `archive:plugins/authority/conf/com.kylin.ukui.SettingsDaemon.conf` | -> /etc/dbus-1/system.d ; policy user=root own ; contexte default : send_destination autorisé pour com.kylin.ukui.SettingsDaemon.interface, Introspectable, Properties (tous utilisateurs) ; pas de fichier .limit | CONSTATÉ |
| dbus-session-service | `archive:data/org.ukui.SettingsDaemon.service` | -> /usr/share/dbus-1/services ; Name=org.ukui.SettingsDaemon ; Exec=/usr/bin/ukui-settings-daemon | CONSTATÉ |
| autostart | `archive:data/ukui-settings-daemon.desktop` | -> /etc/xdg/autostart (data/data.pri) ; Exec=/usr/bin/ukui-settings-daemon ; X-UKUI-Autostart-Phase=Initialization ; X-UKUI-AutoRestart=true ; OnlyShowIn=UKUI | CONSTATÉ |
| cible-construite | `archive:ukui-settings-daemon.pro` | SUBDIRS : auto-brightness, clipboard, gamma-manager, global-manager, housekeeping, keyboard, keybindings, media-keys, input-device-manager, mpris, sound, sharing, tablet-mode, xrandr, xrdb, xsettings, locate-pointer, kds (ukydisplayswitch -> /usr/bin), authority (authoritydbus -> /usr/bin), save-param (-> /usr/bin), daemon ; mouse, a11y-*, background, app-proxy-service, color, xinput non listés | CONSTATÉ |
| fichiers-sécurité | `archive:plugins/authority/authority.pro` | Installe security/SecurityConf.ini -> /opt/system/conf/usd/, 3 JSON -> /opt/system/conf/ukui-security-support/, ukui-settings-daemon.json -> /opt/system/resource/update-manifests/, yaml -> /usr/share/ukui-settings-daemon/ | CONSTATÉ |
| socket-noyau | `archive:plugins/authority/procaddserver.cpp` | socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR), nl_groups=CN_IDX_PROC, bind() : abonnement aux événements processus du noyau dans authoritydbus (root) ; pas un serveur réseau | CONSTATÉ |
| listener-local | `archive:plugins/kds/qtsingleapplication/qtlocalpeer.cpp` | QLocalServer (motif QtSingleApplication, instance unique) ; fichier non lu | PROBABLE |
| udev (declare, fichier absent) [ajout critique] | `archive:ukui-settings-daemon/plugins/mouse/mouse.pro` | Les lignes 46-52 declarent l'installation de 01-touchpad-state-onmouse.rules vers /lib/udev/rules.d et du script touchpad-state vers /usr/bin, mais ces deux fichiers sont absents de l'arbre : NON COUVERT. | NON COUVERT |

- [NON COUVERT] Le paquet installe '/etc/' entier issu du build ; inventaire exact des fichiers de /etc non énuméré.
- [NON COUVERT] Le greffon 'sharing' est compilé ; sa fonction (partage/accès distant) n'a pas été examinée.
- [CONSTATÉ] Le postinst désactive l'action logind sur bouton d'alimentation (HandlePowerKey=ignore) au niveau système.

### `ukui-control-center`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/ukui-control-center), 3.0 (native), 4.24.0.0-ok2.8
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, natif)
- **Langages / taille** : C++/Qt5 (qmake), QML, Qt UI ; Shell (skill kylinbot) ; .ts de traduction — 1 615 fichiers, 97,6 Mo (≈79 Mo d'images de guide utilisateur) ; ~590 fichiers de code (~113 k lignes) — lignes de code propres mesurées : 123 376
- **Privilège** : Application de session ; binaire setuid root changeuserpwd ; 2 services D-Bus système root (com.control.center.qt.systemdbus, org.ukui.groupmanager) chargés en .so par register-ukui-system-service ; PAM ; polkit ; postinst root
- **Déclencheurs** : Lancement utilisateur (desktop) ou activation D-Bus session (org.ukui.ukcc.session, org.ukui.ukcc.search), Activation D-Bus système par appel d'un client (filtrage .limit + polkit), Exécution de changeuserpwd (setuid) et de changeotheruserpwd via pkexec, Agent kylinbot exécutant les commandes du SKILL, Scripts mainteneur (preinst/postinst)
- **Indices réseau** : Aucun listener réseau ; netlink uevent (printer) et QLocalServer local; Client D-Bus système com.kylin.daq (plugins/system/about/about.cpp); Collecte : kdk_dia_upload_* dans libukcc/interface/ukcccommon.cpp (libkysdk-datacollect); Greffons system/vino (bureau à distance), network/proxy, network/vpn présents dans plugins/
- **Blobs** : Aucun binaire ; ~79 Mo d'images PNG de guide
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Très volumineux (~113 k lignes) : L3 exhaustif requis sur changeUserPwd (setuid), changeOtherUserPwd, checkUserPwdWithPAM, registeredQDbus et group-manager-server (root, ~4,9 k lignes), registeredSession (~10 k), scripts mainteneur et skill kylinbot ; greffons UI en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-control-center, libukcc3, libukcc-dev ; Build-Depends libkysdk-datacollect-dev, libpolkit-qt5-1-dev, libkysdk-* ; aucun Depends vers le fournisseur de register-ukui-system-service | CONSTATÉ |
| debian-install | `archive:debian/ukui-control-center.install` | etc/dbus-1/system.d/*, etc/pam.d/*, usr/bin/*, usr/libexec/*, usr/share/dbus-1/{services,system-services}/*, etc/dbus-1/conf/*, usr/share/dbus-1/conf/*, etc/kylin-config/basic/*, usr/share/polkit-1/actions/*, usr/lib/*/ukui-control-center/*.so, opt/kylinbot/skills/ukui-control-center/{,references,scripts}/* | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-control-center.postinst` | root : glib-compile-schemas ; chmod u+s /usr/bin/changeuserpwd ; force UKCC_SECURE_MODE_FOR_LOGINOPTION=1 dans /etc/environment ; exécute /usr/share/kylin-system-updater/kylin-reboot-required si présent ; systemctl enable ukui-group-manager.service commenté ; #DEBHELPER# présent 2 fois | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-control-center.preinst ; archive:debian/libukcc3.preinst` | dpkg-divert --package libukcc3 --rename de /usr/lib/<multiarch>/libukcc.so.1, .so.1.0, .so.1.0.0 vers *.old | CONSTATÉ |
| maintainer-script | `archive:debian/libukcc3.postinst` | Sans set -e : si /usr/sbin/kysec_get existe et /etc/kylin-build contient V10-SP1-hwe -> kysec_set -n exectl -v verified /usr/lib/x86_64-linux-gnu/libukcc.so.1.0.0.old ; puis dpkg-divert --remove des 3 chemins | CONSTATÉ |
| binaire-setuid | `archive:changeUserPwd/changeUserPwd.pro` | TARGET=changeuserpwd -> /usr/bin (482 lignes de code) ; rendu setuid root par le postinst | CONSTATÉ |
| pam | `archive:checkUserPwdWithPAM/conf/control-center` | -> /etc/pam.d/control-center (childCheckPwdWithPAM.pro) ; @include common-auth ; auth optional pam_gnome_keyring.so ; binaires checkUserPwd et childCheckpwdwithPAM -> /usr/bin | CONSTATÉ |
| dbus-system-service | `archive:registeredQDbus/conf/com.control.center.qt.systemdbus.service` | Name=com.control.center.qt.systemdbus ; Exec=/usr/bin/register-ukui-system-service /usr/libexec/ukui-services/system/liblaunchSysDbus.so ; User=root | CONSTATÉ |
| dbus-policy | `archive:registeredQDbus/conf/com.control.center.qt.systemdbus.conf` | -> /etc/dbus-1/system.d ; own root ; default : send_destination limité à com.control.center.interface, Introspectable, Properties | CONSTATÉ |
| polkit | `archive:registeredQDbus/conf/com.control.center.qt.systemdbus.policy` | 10 actions org.control.center.qt.systemdbus.action{,.ntp,.timezone,.autologin,.pid,.grub,.hostname,.packageinstall,.packageuninstall,.remote} ; auth_admin ou auth_admin_keep (timezone, grub, remote) | CONSTATÉ |
| dbus-limit | `archive:registeredQDbus/conf/com.control.center.qt.systemdbus.limit (+ .limit.verify)` | -> /etc/dbus-1/conf et /usr/share/dbus-1/conf ; [whitelist] ukui-control-center, hedron-client, /etc/hedron/policy-manager.py, d-feet, ukui-sidebar, ukui-upower, ukui-session-service, /opt/WST/eps/bin/secProtect, /opt/WST/eps/bin/eps-daemon, /opt/AQTJ/TIP/SFJB/bin/SetLoginStatusCtrl, /usr/sbin/kcm-daemon ; .verify = empreinte 40 hex | CONSTATÉ |
| dbus-system-service | `archive:group-manager-server/conf/org.ukui.groupmanager.service` | Name=org.ukui.groupmanager ; Exec=/usr/bin/register-ukui-system-service /usr/libexec/ukui-services/system/libgroupManagerServer.so ; User=root | CONSTATÉ |
| dbus-policy | `archive:group-manager-server/conf/org.ukui.groupmanager.conf` | -> /etc/dbus-1/system.d ; own root ; contexte default : <allow send_destination='org.ukui.groupmanager'/> sans restriction d'interface | CONSTATÉ |
| polkit | `archive:group-manager-server/conf/org.ukui.groupmanager.policy` | Action org.ukui.groupmanager.action.edit : auth_admin_keep (any/inactive/active) ; annotate org.freedesktop.policykit.exec.path=/usr/bin/changeotheruserpwd | CONSTATÉ |
| dbus-limit | `archive:group-manager-server/conf/org.ukui.groupmanager.limit` | [whitelist] /usr/bin/ukui-control-center, /usr/bin/d-feet, /usr/sbin/pwdless-daemon, /usr/bin/plukey-util | CONSTATÉ |
| systemd-unit-non-installée | `archive:group-manager-server/conf/ukui-group-manager.service` | User=root ; ExecStart=/usr/bin/group-manager-server ; WantedBy=multi-user.target ; absent des INSTALLS de group-manager-server.pro (TEMPLATE=lib) | CONSTATÉ |
| dbus-session-service | `archive:registeredSession/conf/org.ukui.ukcc.session.service ; archive:data/org.ukui.ukcc.search.service` | org.ukui.ukcc.session : Exec=/usr/bin/register-ukui-session-service /usr/libexec/ukui-services/session/libukccSession.so ; org.ukui.ukcc.search : Exec=/usr/bin/ukui-control-center --dbus | CONSTATÉ |
| desktop-file | `archive:shell/ukui-control-center.desktop` | Exec=/usr/bin/ukui-control-center %U ; OnlyShowIn=UKUI | CONSTATÉ |
| skill-agent-ia | `archive:data/ukui-control-center/SKILL.toml` | -> /opt/kylinbot/skills (shell.pro) ; manifeste (author susidian@kylinos.cn) prescrivant l'exécution shell de gdbus/gsettings/kscreen-doctor, dont com.kylin.daq.interface.SetUploadState (--system), com.kylin.RemoteDesktop SetPassword, com.control.center.qt.systemdbus setGrupPasswd, org.ukui.groupmanager (ajout/suppression d'utilisateurs) | CONSTATÉ |
| script-installé | `archive:data/ukui-control-center/scripts/ukcc_scripts.sh` | 244 lignes, installé dans /opt/kylinbot/skills/ukui-control-center/scripts ; contenu non lu | NON COUVERT |
| socket-noyau | `archive:plugins/devices/printer/usbthread.cpp` | socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT), SO_RCVBUFFORCE 16 Mo, bind : écoute des uevents hotplug ; pas un serveur réseau | CONSTATÉ |
| listener-local | `archive:shell/qtsingleapplication/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |
| pam [ajout critique] | `archive:ukui-control-center/checkUserPwdWithPAM/conf/control-center` | Absent de la fiche. Installe en /etc/pam.d/control-center (childCheckPwdWithPAM.pro:31 ; debian/ukui-control-center.install:2). Contenu : '@include common-auth' et 'auth optional pam_gnome_keyring.so'. | CONSTATÉ |
| maintainer-script [ajout critique] | `archive:ukui-control-center/debian/ukui-control-center.postinst` | Le postinst, execute en root, ecrit UKCC_SECURE_MODE_FOR_LOGINOPTION=1 dans /etc/environment, fichier global (l.10-18) ; ce point manque au recensement. Il execute aussi chmod u+s /usr/bin/changeuserpwd. Le jeton #DEBHELPER# y figure deux fois (l.21 et l.31). | CONSTATÉ |

- [CONSTATÉ] register-ukui-system-service et register-ukui-session-service ne sont ni construits par cet arbre ni déclarés dans debian/control.
- [CONSTATÉ] Les listes blanches .limit citent des chemins de produits tiers (/opt/WST/eps, /opt/AQTJ/TIP/SFJB, /etc/hedron) et l'outil de débogage d-feet.
- [NON COUVERT] Le mécanisme d'application des .limit (.verify) est externe à ce paquet.

### `ukui-screensaver`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/ukui-screensaver), 3.0 (quilt), 4.20.0.0-ok5.7 ; 51 patches quilt (dont 'sync codes from v11-2503')
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète ; .pc présent, 51/51 patches appliqués)
- **Langages / taille** : C++/Qt5 (CMake), C ; .ts de traduction ; Shell (scripts greeter) — 968 fichiers, 38 Mo (patches de changelog 9,2 Mo + 4,8 Mo, assets PNG) ; ~340 fichiers de code (~79 k lignes) — lignes de code propres mesurées : 90 524
- **Privilège** : Session utilisateur et session greeter LightDM (utilisateur lightdm) ; chaîne d'authentification PAM (déverrouillage et connexion simpleDesk) ; nom sur le bus système ; scripts mainteneur root
- **Déclencheurs** : Autostart XDG (Initialization), Démarrage du greeter LightDM via xgreeters (simpleDesk ou wlcom), Verrouillage/déverrouillage (ukui-screensaver-command, D-Bus session), Scripts mainteneur
- **Indices réseau** : Client HTTP météo : src/screensaver/weathermanager.cpp, 'http' + '://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/' + cityId; src/screensaver/networkwatcher.cpp (API QNetwork*); QLocalServer local
- **Blobs** : Aucun exécutable ; assets PNG (jusqu'à 0,6 Mo)
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — ~79 k lignes : L3 sur la chaîne d'authentification (src/lock-backend ~10 k lignes, lock-dialog/auth, config PAM ukui-screensaver-qt) et sur les fichiers greeter/LightDM ; économiseur d'écran et assets en L1.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet ukui-screensaver ; Depends ethtool… ; Replaces ukui-screensaver-common | CONSTATÉ |
| debian-install | `archive:debian/ukui-screensaver.install` | data/ukui-greeter/95-ukui-greeter.conf -> /usr/share/lightdm/lightdm.conf.d/ ; lightdm-session-setup.sh -> /usr/lib/lightdm/ ; ukui-greeter.desktop -> /usr/share/xgreeters/ ; kwin/ukui-screensaver/ukui-settings-daemon .desktop -> /usr/share/ukui/ukui-session-manager/simpledesk/ | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-screensaver.preinst` | set -e commenté ; dpkg-divert --package ukui-screensaver --rename de 95-ukui-greeter.conf, '/usr/lib/lightdm/lightdm-session-setup.sh ' (espace final dans la variable), /usr/share/xgreeters/ukui-greeter.desktop et des greffons ukcc libscreenlock.so/libscreensaver.so ; mkdir simpledesk | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-screensaver.postinst` | glib-compile-schemas ; dpkg-divert --remove des mêmes .old ; exécute /usr/share/kylin-system-updater/kylin-reboot-required si présent | CONSTATÉ |
| pam | `archive:data/ukui-screensaver-qt` | -> /etc/pam.d ; auth requisite pam_nologin.so ; auth sufficient pam_succeed_if.so user ingroup nopasswdlogin ; @include common-auth ; auth optional pam_gnome_keyring.so | CONSTATÉ |
| dbus-policy | `archive:data/org.ukui.screensaver.conf` | -> /etc/dbus-1/system.d ; contexte default : allow own=org.ukui.screensaver (tout utilisateur) ; policy user=root : send_destination org.ukui.screensaver (interfaces org.ukui.screensaver, Introspectable, Properties) ; nom enregistré sur le bus système par src/lock-backend/main.cpp:142 | CONSTATÉ |
| autostart | `archive:data/ukui-screensaver.desktop` | -> /etc/xdg/autostart ; Exec=/usr/bin/ukui-screensaver-backend %U ; X-UKUI-Autostart-Phase=Initialization ; X-UKUI-AutoRestart=true | CONSTATÉ |
| greeter-xgreeters | `archive:data/ukui-greeter/ukui-greeter.desktop ; archive:data/ukui-greeter/ukui-greeter-wlcom.desktop` | Exec=ukui-session simpleDesk ; wlcom : Exec=kylin-wlcom -s /usr/bin/start-greeter-session, X-LightDM-Session-Type=wayland | CONSTATÉ |
| lightdm-conf | `archive:data/ukui-greeter/95-ukui-greeter.conf ; archive:data/ukui-greeter/97-ukui-greeter-wlcom.conf` | [Seat:*] greeter-session=ukui-greeter, user-session=ukui ; 97 : greeter-session=ukui-greeter-wlcom, user-session=kylin-wlcom, enable-autologin-user-lock=false | CONSTATÉ |
| script-lanceur | `archive:data/ukui-greeter/start-greeter-session ; archive:data/ukui-greeter/lightdm-session-setup.sh` | start-greeter-session -> /usr/bin : exec /usr/bin/ukui-session simpleDesk ; lightdm-session-setup.sh -> /usr/lib/lightdm : hors Wayland et hors boot=casper, save-param -u $(whoami) | CONSTATÉ |
| simpledesk | `archive:data/desktop/*.desktop` | Composants du greeter simpleDesk : kwin_x11 --replace ; kglobalaccel5 ; ukui-screensaver-backend --lock-startup ; /usr/bin/ukui-settings-daemon | CONSTATÉ |
| cible-construite | `archive:src/CMakeLists.txt` | add_executable ukui-screensaver-dialog, ukui-screensaver-backend, ukui-screensaver-command (-> bin), ukui-screensaver-default ; greffons ukcc libscreenlock/libscreensaver ; src/lock-checkpasswd et src/lock-focushelper sans add_executable | CONSTATÉ |
| listener-local | `archive:src/QtSingleApplication/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |
| pam [ajout critique] | `archive:ukui-screensaver/data/ukui-screensaver-qt` | Absent de la fiche, mais cite dans les constats B09. Installe dans /etc/pam.d (data/CMakeLists.txt:5). Contient 'auth sufficient pam_succeed_if.so user ingroup nopasswdlogin' avant '@include common-auth'. | CONSTATÉ |

- [PROBABLE] Le paquet détourne (dpkg-divert) les fichiers LightDM/xgreeters de ukui-greeter : le greeter effectif serait 'ukui-session simpleDesk' + ukui-screensaver-backend --lock-startup.
- [CONSTATÉ] PAM : les membres du groupe nopasswdlogin sont authentifiés sans mot de passe (pam_succeed_if sufficient).
- [NON COUVERT] Aucune directive session-setup-script référençant lightdm-session-setup.sh n'a été trouvée dans le lot.

### `ukui-greeter`

- **Nature** : propre-kylin — **Amont** : UKUI (gitee openkylin/ukui-greeter ; GitHub EMPTY), 3.0 (native), 4.10.0.0-ok5.5update1 ; dernière entrée debian/changelog en distribution 'nile'
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, natif)
- **Langages / taille** : C++/Qt5 (qmake), C, Qt UI ; .ts de traduction — 381 fichiers, 3,4 Mo ; ~143 fichiers de code (~26 k lignes) — lignes de code propres mesurées : 35 341
- **Privilège** : Greeter exécuté par LightDM (utilisateur lightdm) ; démon D-Bus système root ukui-greeter-daemon (~390 lignes) ; authentification de connexion (PAM LightDM, biométrie)
- **Déclencheurs** : Démarrage de LightDM (greeter-session=ukui-greeter), sauf détournement par ukui-screensaver, Activation D-Bus système org.ukui.GreeterDaemon par tout utilisateur, postinst
- **Indices réseau** : Aucun listener détecté; Client D-Bus biométrique (ukui-greeter/BiometricAuth) probable
- **Blobs** : Aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — ~26 k lignes ; chemin d'authentification de connexion (LightDM/PAM/biométrie) et démon D-Bus système root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet ukui-greeter ; Depends ethtool, libpam-biometric (>=3.20.0.10), ukui-input-gather ; Suggests biometric-auth ; Provides lightdm-greeter | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-greeter.postinst` | Copie ukui-greeter.yaml et org.ukui.GreeterDaemon.yaml vers /etc/kylin-config/basic/ ; exécute /usr/share/kylin-system-updater/kylin-reboot-required si présent | CONSTATÉ |
| dbus-system-service | `archive:ukui-greeter-daemon/org.ukui.GreeterDaemon.service` | -> /usr/share/dbus-1/system-services ; Name=org.ukui.GreeterDaemon ; Exec=/usr/lib/ukui-greeter/ukui-greeter-daemon ; User=root (registerService systemBus dans serviceinterface.cpp) | CONSTATÉ |
| dbus-policy | `archive:ukui-greeter-daemon/org.ukui.GreeterDaemon.conf` | -> /etc/dbus-1/system.d ; own root ; default : send_destination pour interface org.ukui.GreeterDaemon, Introspectable, Properties (tous utilisateurs) | CONSTATÉ |
| dbus-limit-non-installé | `archive:ukui-greeter-daemon/org.ukui.GreeterDaemon.limit` | [whitelist] ukui-greeter, ukui-screensaver-dialog, lightdm, ukui-control-center, ukui-session-tools, ukui-session ; absent des INSTALLS (target configfile services kconf2) | CONSTATÉ |
| cible-construite | `archive:ukui-greeter/ukui-greeter.pro` | TARGET ukui-greeter -> /usr/sbin ; 95-ukui-greeter.conf -> /usr/share/lightdm/lightdm.conf.d ; display-switch/lightdm-session-setup.sh -> /usr/lib/lightdm ; ukui-greeter.conf -> /etc/lightdm ; ukui-greeter.desktop -> /usr/share/xgreeters | CONSTATÉ |
| lightdm-conf | `archive:ukui-greeter/95-ukui-greeter.conf` | [Seat:*] greeter-session=ukui-greeter ; user-session=ukui ; enable-autologin-user-lock=false ; greeter-setup-script commenté | CONSTATÉ |
| desktop-file | `archive:ukui-greeter/ukui-greeter.desktop` | xgreeters : Exec=ukui-greeter | CONSTATÉ |
| script | `archive:ukui-greeter/display-switch/lightdm-session-setup.sh` | save-param -u $(whoami) si save-param est présent | CONSTATÉ |

- [CONSTATÉ] Les chemins 95-ukui-greeter.conf, lightdm-session-setup.sh et xgreeters/ukui-greeter.desktop sont détournés par dpkg-divert par ukui-screensaver.
- [CONSTATÉ] Version 4.10 (série nile) alors que les autres composants UKUI du lot sont en 4.20–4.25.

### `ukui-panel`

- **Nature** : propre-kylin — **Amont** : UKUI (Vcs-Git https://gitee.com/openkylin/ukui-panel.git), 3.0 (quilt), 4.24.0.1-ok0.2 ; 170 patches quilt
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 170/170 patches appliqués)
- **Langages / taille** : C++/Qt5 + QML (CMake) ; .ts ; HTML (doc qtsingleapplication) — 593 fichiers, 2,97 Mo ; ~99 fichiers de code hors 3rd-parties/tests (~20 k lignes) — lignes de code propres mesurées : 24 664
- **Privilège** : Session utilisateur uniquement
- **Déclencheurs** : Autostart XDG phase Panel, Chargement du greffon ukcc par ukui-control-center
- **Indices réseau** : Aucun listener réseau ; QLocalServer local; Collecte d'usage via libkysdk-datacollect (kdk_dia_upload_default); panel/src/remote-config.cpp (nom de fichier ; contenu non lu)
- **Blobs** : Aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session (~20 k lignes) sans privilège ; L3 ciblé sur data-collector.cpp, remote-config.cpp et le widget ukui-ai-assistant.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-panel et ukui-panel-widgets | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake, RelWithDebInfo ; DEB_BUILD_MAINT_OPTIONS hardening commenté ; aucun script mainteneur | CONSTATÉ |
| debian-install | `archive:debian/ukui-panel.install ; archive:debian/ukui-panel-widgets.install` | /usr/bin/ukui-panel, /usr/lib/*/ukui-control-center (greffon), /etc/xdg/autostart/ukui-panel.desktop, gschema, /opt/system/conf/ukui-security-support/ukui-panel-v11.json ; widgets : /usr/share/ukui/widgets, qml org/ukui, 7 JSON ukui-security-support | CONSTATÉ |
| autostart | `archive:panel/ukui-panel.desktop` | Exec=ukui-panel ; X-UKUI-Autostart-Phase=Panel ; X-UKUI-AutoRestart=true ; OnlyShowIn=UKUI | CONSTATÉ |
| cible-construite | `archive:CMakeLists.txt ; archive:widgets/CMakeLists.txt` | panel (+ ukcc-plugin, qtsingleapplication) ; widgets : ukui-task-manager, ukui-menu-starter, ukui-panel-showDesktop, ukui-panel-widget-test, ukui-panel-calendar, ukui-panel-separator, ukui-panel-search, ukui-panel-taskView, ukui-panel-idm, ukui-ai-assistant | CONSTATÉ |
| collecte | `archive:panel/src/data-collector.cpp` | kdk_dia_data_init/kdk_dia_upload_default (événements adjust_panel_position_event, adjust_panel_size_event) ; aussi dans widgets/ukui-task-manager/ukui-task-manager.cpp | CONSTATÉ |
| listener-local | `archive:panel/3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |

- [PROBABLE] Widget ukui-panel-widget-test compilé (add_subdirectory) et livré dans ukui-panel-widgets.
- [NON COUVERT] Contenu de remote-config.cpp et du widget ukui-ai-assistant non examiné.

### `ukui-menu`

- **Nature** : propre-kylin — **Amont** : UKUI (Vcs-Git https://gitee.com/openkylin/ukui-menu.git ; GitHub EMPTY), 3.0 (quilt), 4.25.0.0-ok0.1 ; 1 patch quilt (traductions)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 1/1 appliqué)
- **Langages / taille** : C++/Qt5 + QML (CMake) ; .ts — 357 fichiers, 1,98 Mo ; ~162 fichiers de code hors tests (~24 k lignes) — lignes de code propres mesurées : 41 019
- **Privilège** : Session utilisateur uniquement
- **Déclencheurs** : Autostart XDG phase Application, Activation D-Bus session org.ukui.menu
- **Indices réseau** : Aucun listener réseau ; QLocalServer local; Collecte d'usage kdk_dia_upload (event-track.cpp)
- **Blobs** : Aucun
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Lanceur d'applications de session (~24 k lignes) avec IPC D-Bus session et collecte d'usage ; L3 limité à event-track.cpp et au chargement des extensions QML.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-menu, libukui-menu1, libukui-menu-dev ; Build-Depends libkysdk-datacollect-dev, libkysdk-package-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake RelWithDebInfo ; pas de hardening explicite ; aucun script mainteneur | CONSTATÉ |
| debian-install | `archive:debian/ukui-menu.install` | usr/bin/ukui-menu, etc/xdg/autostart/ukui-menu.desktop, usr/share/dbus-1/services/org.ukui.menu.service, gschema, opt/system/conf/ukui-security-support/ | CONSTATÉ |
| dbus-session-service | `archive:data/org.ukui.menu.service` | Name=org.ukui.menu ; Exec=/usr/bin/ukui-menu | CONSTATÉ |
| autostart | `archive:data/ukui-menu.desktop` | Exec=/usr/bin/ukui-menu %u ; X-UKUI-Autostart-Phase=Application ; X-UKUI-AutoRestart=true | CONSTATÉ |
| collecte | `archive:src/utils/event-track.cpp` | 3 appels kdk_dia_upload_* (libkysdk-datacollect) | CONSTATÉ |
| listener-local | `archive:3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |

- [CONSTATÉ] libukui-menu1 installe des extensions QML sous qt5/qml/org/ukui/menu/extension (point d'extension tiers).

### `peony`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/peony), 3.0 (quilt), 4.24.0.0-ok10 ; 191 patches quilt
- **Installé par défaut (ISO 3.0)** : oui (4 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 191/191 appliqués)
- **Langages / taille** : C++/Qt5 (qmake), QML, C (debian/upgrade-utils), Bash ; .ts — 1 425 fichiers, 105,8 Mo (≈20 Mo de patches de changelog, images de guide) ; ~688 fichiers de code (~187 k lignes) — lignes de code propres mesurées : 192 002
- **Privilège** : Session utilisateur ; postinst root (peony-utils) écrivant dans le HOME de tous les comptes
- **Déclencheurs** : Autostart XDG (peony-qt-desktop), Activation D-Bus session org.freedesktop.FileManager1, Ouverture de répertoire (MimeType inode/directory), postinst peony-utils à l'installation/mise à jour
- **Indices réseau** : Aucun listener réseau ; QLocalServer local; Accès réseau indirect via GVFS (gvfs-backends, gvfs-fuse en Depends de libpeony3); Collecte kdk_dia_upload (navigation-side-bar.cpp)
- **Blobs** : Aucun exécutable ; images PNG de guide (jusqu'à 4,4 Mo)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Très volumineux (~187 k lignes) : L3 sur debian/upgrade-utils (exécution root à l'installation, C + Bash) et libpeony-qt/file-operation ; reste du gestionnaire de fichiers en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets peony, peony-common, libpeony3 (Depends gvfs, gvfs-backends, gvfs-fuse, libblockdev-crypto3, dvd+rw-tools), libpeony-dev, peony-utils | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; USE_FEEDBACK_MENU_ACTION=1 ; compile 3 utilitaires hors qmake : meta-conf-parser (C++), meta-get et meta-set (C : metatree.c, metabuilder.c, crc32.c) | CONSTATÉ |
| maintainer-script | `archive:debian/peony-utils.postinst` | root : exécute update-desktop-grid | CONSTATÉ |
| script-root-installation | `archive:debian/upgrade-utils/desktop-grid/update-desktop-grid` | Bash 204 lignes -> /usr/bin ; pour chaque compte de getent passwd : sudo -u <user> meta-get…, rm -f puis crée $HOME/.peony-desktop-grid.conf, chown <user>, sudo -u <user> meta-conf-parser | CONSTATÉ |
| maintainer-script | `archive:debian/peony.postinst` | glib-compile-schemas | CONSTATÉ |
| debian-install | `archive:debian/peony-common.install ; archive:debian/peony-utils.install` | peony-desktop.desktop -> /etc/xdg/autostart ; org.ukui.freedesktop.FileManager1.service -> /usr/share/dbus-1/services ; org.ukui.peony.settings.json -> /opt/system/conf/ukui-security-support ; meta-get, meta-set, update-desktop-grid, meta-conf-parser -> /usr/bin | CONSTATÉ |
| dbus-session-service | `archive:peony-qt-desktop/org.ukui.freedesktop.FileManager1.service` | Name=org.freedesktop.FileManager1 ; Exec=/usr/bin/peony-qt-desktop -d | CONSTATÉ |
| autostart | `archive:data/peony-desktop.desktop` | Exec=/usr/bin/peony-qt-desktop -w -d ; OnlyShowIn=UKUI | CONSTATÉ |
| desktop-file | `archive:data/peony.desktop (+ peony-home/computer/trash)` | Exec=/usr/bin/peony %U ; MimeType=inode/directory | CONSTATÉ |
| cible-construite | `archive:peony-qt.pro` | SUBDIRS = src, libpeony-qt, peony-qt-desktop, stable (properties-window, windows) ; plugin et tests commentés | CONSTATÉ |
| collecte | `archive:src/control/navigation-side-bar.cpp ; archive:common.pri` | 1 appel kdk_dia_upload ; common.pri active la collecte si /usr/include/kysdk/diagnosetest/libkydatacollect.h existe | CONSTATÉ |
| listener-local | `archive:3rd-parties/SingleApplication/singleapplication_p.cpp ; archive:3rd-parties/qtsingleapplication/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichiers non lus | PROBABLE |

- [CONSTATÉ] Répertoire afl_fuzzing/ présent (cibles de fuzzing), non construit.
- [PROBABLE] Les utilitaires meta-get/meta-set réimplémentent le format de métadonnées GVFS (metatree.c) ; provenance amont non vérifiée.

### `peony-extensions`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/peony-extensions), 3.0 (quilt), 4.24.0.0-ok5 ; 298 patches quilt
- **Installé par défaut (ISO 3.0)** : oui (14 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 298/298 appliqués)
- **Langages / taille** : C++/Qt5 (qmake), QML, Qt UI, Shell ; .ts — 926 fichiers, 10,6 Mo (patches, PDF manuels IDM) ; ~176 fichiers de code (~33 k lignes) — lignes de code propres mesurées : 42 331
- **Privilège** : Greffons chargés dans peony (session) ; service D-Bus système root peony-samba-service ; service utilisateur IDM ; postinst root pilotant smbd/nmbd
- **Déclencheurs** : Chargement des greffons par peony, Activation D-Bus système org.ukui.samba.share.config (polkit auth_self_keep), Service systemd utilisateur IDM activé par postinst (xdg-desktop-autostart.target), postinst peony-share (état de smbd/nmbd)
- **Indices réseau** : Partage SMB : configuration usershares Samba et activation de smbd/nmbd par postinst si des partages existent; QLocalServer local (IDM 3rd-parties/qtlocalpeer); peony-send-to-device / peony-bluetooth : transfert vers périphériques (probable)
- **Blobs** : Aucun exécutable ; 2 PDF de manuel IDM (~1 Mo chacun), dictionnaire dpinyin.dict
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — ~33 k lignes : L3 sur peony-share (service root + app + menu, exposition SMB), scripts mainteneur et IDM (service utilisateur d'indexation, ~10 k lignes) ; autres greffons UI en L1.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 16 paquets binaires dont peony-share (Depends samba, samba-common-bin, acl), peony-intelligent-data-management-service, parchives, peony-bluetooth, peony-send-to-device… | CONSTATÉ |
| debian-install | `archive:debian/peony-share.install` | libpeony-share*.so, usr/share/polkit-1/actions, usr/libexec, usr/share/dbus-1/system.d/, usr/share/dbus-1/system-services/, usr/share/dbus-1/conf/, usr/share/peony-extensions/kylin-config/ | CONSTATÉ |
| dbus-system-service | `archive:peony-share/data/org.ukui.samba.share.config.service` | Name=org.ukui.samba.share.config ; User=root ; Exec=/usr/libexec/peony-samba-service (peony-share/service, ~630 lignes) | CONSTATÉ |
| dbus-policy | `archive:peony-share/data/org.ukui.samba.share.config.conf` | -> /usr/share/dbus-1/system.d ; policy user=root own ; contexte default : allow own=org.ukui.samba.share.config ET allow send_destination sans restriction d'interface (tous utilisateurs) | CONSTATÉ |
| polkit | `archive:peony-share/data/org.ukui.samba.share.config.policy` | Action org.ukui.samba.share.config.authorization : allow_inactive=no, allow_active=auth_self_keep, allow_any non défini | CONSTATÉ |
| dbus-limit | `archive:peony-share/data/org.ukui.samba.share.config.limit (+ .verify)` | [whitelist] /usr/bin/peony, /usr/bin/peony-qt-desktop ; copié vers /etc/dbus-1/conf par postinst | CONSTATÉ |
| maintainer-script | `archive:debian/peony-share.postinst` | #DEBHELPER# en tête ; si /var/lib/samba/usershares vide : systemctl stop+disable smbd et nmbd ; sinon enable+start smbd et nmbd ; daemon-reload ; copie yaml/limit/verify vers /etc/kylin-config/basic et /etc/dbus-1/conf | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | Sans préfixe de paquet : mkdir -p /var/lib/samba/usershares ; install -d -m 770 puis -m +t ; s'appliquerait au premier paquet (peony-share) qui a déjà son propre postinst | PROBABLE |
| systemd-user-unit | `archive:peony-intelligent-data-management-service/conf/peony-intelligent-data-management-service.service` | -> /usr/lib/systemd/user ; ExecStart=/usr/bin/peony-intelligent-data-management-service ; Restart=on-failure ; WantedBy=xdg-desktop-autostart.target | CONSTATÉ |
| dbus-session-service | `archive:peony-intelligent-data-management-service/conf/com.peony.idm.service` | Name=com.peony.idm.service ; Exec=/usr/bin/peony-intelligent-data-management-service ; SystemdService=peony-intelligent-data-management-service.service | CONSTATÉ |
| maintainer-script | `archive:debian/peony-intelligent-data-management-service.postinst` | deb-systemd-helper --user enable peony-intelligent-data-management-service.service | CONSTATÉ |
| polkit-non-construit | `archive:peony-admin/org.freedesktop.peony-admin.policy.in` | Actions pkexec peony/pluma/mate-terminal (auth_admin_keep, allow_gui) ; peony-admin absent des SUBDIRS de peony-extensions.pro | CONSTATÉ |
| cible-construite | `archive:peony-extensions.pro` | SUBDIRS : peony-share, peony-drive-rename, peony-send-to-device, peony-set-wallpaper, peony-bluetooth-plugin, peony-engrampa-menu-plugin, peony-extension-computer-view, peony-menu-plugin-mate-terminal, peony-intelligent-data-management-service, peony-intelligent-plugin, peony-spaceview-plugin, 2 widgets bureau, previewhub-hybrid, quick-look ; tests sautés (EXTENSIONS_SKIP_TEST=1) | CONSTATÉ |

- [CONSTATÉ] La politique D-Bus autorise tout utilisateur à posséder le nom système org.ukui.samba.share.config.
- [CONSTATÉ] peony-quick-look-plugin et peony-previewhub-hybrid-plugin sont construits mais non listés dans l'ISO 3.0.

### `kylin-nm`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/kylin-nm), 3.0 (quilt), 4.24.0.0-ok0.7 ; 121 patches quilt
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 121/121 appliqués)
- **Langages / taille** : C++/Qt5 (qmake), C (enhance-daemon, nma-resources.c généré), QML ; .ts — 1 225 fichiers, 13 Mo ; ~279 fichiers de code (~84 k lignes, dont src/backend/vpnsecrets/nma-resources.c 480 Ko généré) — lignes de code propres mesurées : 120 434
- **Privilège** : 3 démons root (kylin-nm-sysdbus, kylin-nm-netctrl, nm-enhance-optimization), dont 2 activés au démarrage par postinst ; applets de session kylin-nm/kylin-vpn ; greffons ukcc
- **Déclencheurs** : Démarrage système (multi-user.target) : kylin-nm-sysdbus, nm-enhance-optimization, Activation D-Bus système com.kylin.networkCtrol / com.kylin.network.qt.systemdbus, Autostart XDG des applets, Scripts mainteneur
- **Indices réseau** : PF_PACKET ARP (root); NETLINK_CONNECTOR (root); Gestion DNS/hosts (/etc/nm_enhance, /etc/resolv.dnsmasq.conf, /etc/hosts.d); Proxy par application (interface com.kylin.network.proxy); QLocalServer local
- **Blobs** : Aucun binaire ; nma-resources.c (480 Ko, ressources GResource générées)
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — ~84 k lignes au total : L3 exhaustif sur les 3 démons root (sys-dbus-register ~2,4 k, sys-dbus-netctrl ~2,1 k, enhance-daemon C ~2 k lignes), scripts mainteneur et secrets VPN (src/backend/vpnsecrets) ; interface Qt en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet kylin-nm ; Depends network-manager (>=1.2.6), ukui-control-center (>= 3.1.1+1217), libkysdk-* | CONSTATÉ |
| debian-install | `archive:debian/kylin-nm.install` | debian/kylin-nm.json -> /etc/ukui/ukui-notification/default-app/ ; sys-dbus-register/conf/kylin-nm-sysdbus.service -> /lib/systemd/system/ ; 2 yaml -> /usr/share/dbus-1/conf/.kylin-nm-yaml/ | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-nm.postinst` | rm /etc/NetworkManager/kylin_nm_ctrl.ini ; si configure : systemctl daemon-reload, enable+start kylin-nm-sysdbus.service, enable+restart nm-enhance-optimization.service ; cp .kylin-nm-yaml/* vers /etc/kylin-config/basic/ puis rm -rf ; pas de jeton #DEBHELPER# | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-nm.preinst` | dpkg-divert --package kylin-network-manager-enhance-daemon --rename de /etc/nm_enhance/nm_enhance_dns.conf ; pkill -f kylin-nm-sysdbus | CONSTATÉ |
| systemd-unit | `archive:sys-dbus-register/conf/kylin-nm-sysdbus.service` | Type=dbus BusName=com.kylin.network.qt.systemdbus ExecStart=/usr/bin/kylin-nm-sysdbus User=root Group=root Restart=on-failure WantedBy=multi-user.target ; installé dans /etc/systemd/system (qmake) et /lib/systemd/system (debian) | CONSTATÉ |
| dbus-system-service | `archive:sys-dbus-register/conf/com.kylin.network.qt.systemdbus.service` | Exec=/usr/bin/kylin-nm-sysdbus ; User=root | CONSTATÉ |
| dbus-policy | `archive:sys-dbus-register/conf/com.kylin.network.qt.systemdbus.conf` | own root ; default : send_destination interfaces com.kylin.network.interface, com.kylin.network.proxy, Introspectable, Properties, Peer ; .limit [whitelist] kylin-nm, ukui-control-center | CONSTATÉ |
| dbus-system-service | `archive:sys-dbus-netctrl/conf/com.kylin.networkCtrol.service` | Name=com.kylin.networkCtrol ; Exec=/usr/bin/kylin-nm-netctrl ; User=root | CONSTATÉ |
| dbus-policy | `archive:sys-dbus-netctrl/conf/com.kylin.networkCtrol.conf (+ .limit)` | own root ; default : send interface com.kylin.networkCtrol + Introspectable/Properties ; .limit [whitelist] 11 chemins (kylin-nm, hedron-client, NetworkManager, nm-connection-editor, kim-domain, kim-client, ukui-control-center, ukui-screensaver-dialog, ukui-greeter, ukui-sidebar, kcm-daemon) ; [rootcontrol] status=off | CONSTATÉ |
| systemd-unit | `archive:kylin-network-manager-enhance-daemon/conf/nm-enhance-optimization.service` | Type=dbus BusName=com.kylin.network.enhancement.optimization ExecStart=/usr/bin/nm-enhance-optimization User=root After=lightdm.service WantedBy=multi-user.target ; même fichier (format unité systemd) aussi installé dans /usr/share/dbus-1/system-services | CONSTATÉ |
| dbus-policy | `archive:kylin-network-manager-enhance-daemon/conf/com.kylin.network.enhancement.optimization.conf` | -> /etc/dbus-1/system.d ; contient <type>system</type> et <standard_session_servicedirs/> ; contexte default : allow user=root, allow own=com.kylin.network.enhancement.optimization, allow send_destination et receive_sender sans restriction ; .limit [whitelist] /usr/bin/kylin-nm | CONSTATÉ |
| fichiers-config-réseau | `archive:kylin-network-manager-enhance-daemon/kylin-network-manager-enhance-daemon.pro` | nm_enhance_dns.conf -> /etc/nm_enhance ; kylin.hosts (0 octet) -> /etc/hosts.d/ ; resolv.dnsmasq.conf (0 octet) -> /etc/ | CONSTATÉ |
| autostart | `archive:src/kylin-nm.desktop ; archive:src-vpn/kylin-vpn.desktop` | -> /etc/xdg/autostart ; Exec=/usr/bin/kylin-nm / /usr/bin/kylin-vpn ; X-UKUI-Autostart-Phase=Application ; AutoRestart | CONSTATÉ |
| socket-brut | `archive:sys-dbus-register/kyarping/kylinipv4arping.cpp` | socket(PF_PACKET, SOCK_DGRAM, 0), sll_protocol=ETH_P_ARP, bind() : émission/réception ARP dans le démon root kylin-nm-sysdbus | CONSTATÉ |
| socket-noyau | `archive:sys-dbus-register/proxyServer/proxyapphandler.cpp` | socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR) + bind (événements processus) pour le proxy applicatif | CONSTATÉ |
| cible-construite | `archive:kylin-nm.pro` | SUBDIRS : plugins (netconnect, wlanconnect, proxy, vpn, mobilehotspot -> $$[QT_INSTALL_LIBS]/ukui-control-center), src-vpn, src, sys-dbus-register, qml-network, sys-dbus-netctrl, kylin-network-manager-enhance-daemon | CONSTATÉ |
| listener-local | `archive:src/singleapplication/qt-local-peer.cpp ; archive:src-vpn/singleapplication/qt-local-peer.cpp` | QLocalServer (instance unique) ; fichiers non lus | PROBABLE |

- [CONSTATÉ] La liste 'mains' de la fiche est tronquée à 40 entrées : sys-dbus-register/main.cpp, sys-dbus-netctrl/main.cpp et kylin-network-manager-enhance-daemon/main.c existent mais n'y figurent pas.
- [CONSTATÉ] Le preinst divertit au nom d'un paquet kylin-network-manager-enhance-daemon non déclaré dans debian/control.
- [CONSTATÉ] La politique enhancement.optimization autorise tout utilisateur à posséder le nom système.

### `ukui-notification`

- **Nature** : propre-kylin — **Amont** : UKUI (Vcs-Git https://gitee.com/openkylin/ukui-notification.git), 3.0 (quilt), 4.24.0.1-ok0.1 ; 1 patch quilt
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 1/1 appliqué)
- **Langages / taille** : C++/Qt5 (CMake) ; .ts ; HTML (doc) — 200 fichiers, 0,9 Mo ; ~56 fichiers de code (~7,9 k lignes) — lignes de code propres mesurées : 9 531
- **Privilège** : Session utilisateur uniquement
- **Déclencheurs** : Autostart XDG, Activation D-Bus session org.freedesktop.Notifications par toute application de la session
- **Indices réseau** : Aucun listener réseau ; QLocalServer local; Serveur D-Bus de session recevant les notifications de toutes les applications
- **Blobs** : Aucun
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — ~7,9 k lignes ; service D-Bus de session recevant des données de toutes les applications, sans privilège root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-notification-server, libukui-notification1, -dev, -doc | CONSTATÉ |
| debian-install | `archive:debian/ukui-notification-server.install` | usr/bin/ukui-notification-server, /usr/share/dbus-1/services/org.freedesktop.Notifications.service, /etc/xdg/autostart/ukui-notification-server.desktop, /etc/ukui/ukui-notification/notification-allowlist.json, greffon ukcc libnotification-ukcc-plugin.so | CONSTATÉ |
| dbus-session-service | `archive:notification-server/data/org.freedesktop.Notifications.service` | Name=org.freedesktop.Notifications ; Exec=/usr/bin/ukui-notification-server | CONSTATÉ |
| autostart | `archive:notification-server/data/ukui-notification-server.desktop` | Exec=/usr/bin/ukui-notification-server %U ; X-UKUI-Autostart-Phase=Application ; AutoRestart | CONSTATÉ |
| cible-construite | `archive:notification-server/CMakeLists.txt` | install ukui-notification-server -> /usr/bin ; ukui-notification-v11.json -> /opt/system/conf/ukui-security-support/ ; aucun script mainteneur | CONSTATÉ |
| listener-local | `archive:3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |

- [CONSTATÉ] Même nom D-Bus de session (org.freedesktop.Notifications) déclaré par ukui-notification-daemon.

### `ukui-notification-daemon`

- **Nature** : propre-kylin — **Amont** : UKUI (https://github.com/ukui/ukui-notification-daemon), 3.0 (quilt), 4.0.0.0-ok0~0412 ; dernière entrée debian/changelog 'yangtze' ; forges IDENTICAL-HEADS
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète ; .pc présent sans patches appliqués, pas de debian/patches)
- **Langages / taille** : C++/Qt5 (qmake) — 46 fichiers, 0,27 Mo ; ~24 fichiers de code (~5,6 k lignes) — lignes de code propres mesurées : 5 576
- **Privilège** : Session utilisateur uniquement
- **Déclencheurs** : Activation D-Bus session org.freedesktop.Notifications (si installé)
- **Indices réseau** : Aucun listener détecté; Stockage local des notifications probable (src/sqlinfodata.cpp)
- **Blobs** : Aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Non installé par l'ISO 3.0, composant de session sans privilège (~5,6 k lignes), fonctionnellement remplacé par ukui-notification-server.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 1 paquet ukui-notification-daemon ; aucun .install ni script mainteneur | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; -Wall -pedantic ; dh_shlibdeps --ignore-missing-info | CONSTATÉ |
| cible-construite | `archive:ukui-notification-daemon.pro` | TARGET ukui-notifications -> $${PREFIX}/lib/ukui-notification-daemon ; file/org.ukui.freedesktop.Notification.service -> /usr/share/dbus-1/services ; 2 gschema | CONSTATÉ |
| dbus-session-service | `archive:file/org.ukui.freedesktop.Notification.service.in` | Name=org.freedesktop.Notifications ; Exec=$${PREFIX}/lib/ukui-notification-daemon/ukui-notifications | CONSTATÉ |

- [CONSTATÉ] installe_par_defaut_iso vide.
- [NON COUVERT] La génération du .service depuis le .in au build n'a pas été vérifiée.

### `kylin-wayland-compositor`

- **Nature** : propre-kylin — **Amont** : Kylin (gitee openkylin/kylin-wayland-compositor ; Vcs-Git vide), 1.3.1-ok36 (meson version 1.3.0) ; construit sur wlroots >=0.17.0 <0.18.0 (dependency, default_library=static, renderers=vulkan, backends=x11, xwayland=disabled, session=disabled) ; README : 'basé sur wlroots', fichiers d'autres projets sous leur licence (LICENSES : GPL-1.0+, LGPL-2.1+, MIT-CMU, MulanPSL-2.0)
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 18/18 patches appliqués, dont 'update-to-1.3.1-okNN' de 0,2 à 3 Mo)
- **Langages / taille** : C (meson) ; C++/Qt (wldialog, magnifier, sécurité GUI, exemples) ; QML — 731 fichiers, 16,5 Mo (≈10 Mo de patches) ; ~460 fichiers de code hors exemples/tests (~163 k lignes) — lignes de code propres mesurées : 174 138
- **Privilège** : Serveur d'affichage de session utilisateur (et du greeter wlcom) avec accès DRM/libinput ; aucune unité root ni script mainteneur
- **Déclencheurs** : Ouverture de session Wayland (wayland-sessions/kylin-wlcom.desktop, unité user kylin-wlcom.service), Greeter LightDM ukui-greeter-wlcom, Activation D-Bus session du portail
- **Indices réseau** : Sockets UNIX X11 locales (Xwayland); Capture d'écran via PipeWire (portail); Aucun listener TCP/UDP détecté
- **Blobs** : Aucun
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Base C volumineuse (~163 k lignes) : L3 sur src/security et src/plugin/security (~3 k lignes), portail (capture/pipewire), xwayland, gestion des entrées et règle udev ; rendu/effets en L2.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 5 paquets : kylin-wayland-compositor, -client, -tools, -examples, xdg-desktop-portal-wlcom | CONSTATÉ |
| debian-rules | `archive:debian/rules` | meson --with quilt ; -Dukui_theme=true --buildtype=debugoptimized ; aucun script mainteneur dans debian/ | CONSTATÉ |
| debian-install | `archive:debian/kylin-wayland-compositor.install ; archive:debian/xdg-desktop-portal-wlcom.install` | usr/bin/{kylin-wlcom,kylin-wlcom-wrapper,kylin-magnifier,wldialog}, usr/lib/udev/rules.d/, usr/share/wayland-sessions/, etc/kylin-wlcom/, usr/lib/systemd/user/kylin-wlcom* ; portail : /usr/libexec/xdg-desktop-portal-wlcom, unité user, dbus services, portals | CONSTATÉ |
| udev | `archive:data/udev-rules/90-framebuffer.rules` | -> /usr/lib/udev/rules.d ; SUBSYSTEM=='graphics', GROUP='video', MODE='0666' | CONSTATÉ |
| systemd-user-unit | `archive:data/kylin-wlcom.service ; archive:data/kylin-wlcom.target` | ExecStart=kylin-wlcom -s ukui-session ; Slice=session.slice ; PartOf=kylin-wlcom.target ; Before=graphical-session.target ; target Requires/BindsTo kylin-wlcom.service | CONSTATÉ |
| session-desktop | `archive:data/kylin-wlcom.desktop` | -> /usr/share/wayland-sessions ; Exec=kylin-wlcom-wrapper | CONSTATÉ |
| systemd-user-unit | `archive:portal/data/xdg-desktop-portal-wlcom.service` | Type=dbus BusName=org.freedesktop.impl.portal.desktop.wlcom ; ExecStart=/usr/libexec/xdg-desktop-portal-wlcom ; ConditionEnvironment=WAYLAND_DISPLAY ; Restart=on-failure | CONSTATÉ |
| dbus-session-service | `archive:portal/data/org.freedesktop.impl.portal.desktop.wlcom.service` | Exec=/usr/bin/xdg-desktop-portal-wlcom (chemin différent du binaire installé dans /usr/libexec) ; SystemdService=xdg-desktop-portal-wlcom.service | CONSTATÉ |
| listener-local | `archive:src/xwayland/sockets.c` | socket(AF_UNIX, SOCK_STREAM) ; unlink + bind + listen(fd,1) sur /tmp/.X11-unix/X%d et socket abstrait : sockets d'affichage X11 locales pour Xwayland | CONSTATÉ |
| cible-construite | `archive:meson.build` | subdir data, po, protocols, src, libkywc, examples, tools, portal ; dépendances libinput, libdrm, gbm, pipewire, libsystemd (sd-bus), librsvg, json-c | CONSTATÉ |
| greeter | `archive:../ukui-screensaver/data/ukui-greeter/ukui-greeter-wlcom.desktop (paquet ukui-screensaver)` | Greeter Wayland : Exec=kylin-wlcom -s /usr/bin/start-greeter-session | CONSTATÉ |
| udev (installe) [ajout critique] | `archive:kylin-wayland-compositor/data/udev-rules/90-framebuffer.rules` | Present dans la fiche, absent du recensement et des constats B09 (qui indiquent 'aucune unite root ni script mainteneur'). Contenu : SUBSYSTEM=="graphics", GROUP="video", MODE="0666". Installe dans /usr/lib/udev/rules.d (data/meson.build:3 ; debian/kylin-wayland-compositor.install:5). Le binaire kylin-wayland-compositor=1.3.1-ok36 est installe par l'ISO. | CONSTATÉ |

- [CONSTATÉ] subprojects/ vide : wlroots attendu comme dépendance système (version 0.17.x).
- [CONSTATÉ] Installé par défaut (kylin-wayland-compositor, -client, -tools) ; xdg-desktop-portal-wlcom absent de la liste ISO.
- [NON COUVERT] Les fichiers repris d'autres projets (mentionnés par le README) ne sont pas identifiés individuellement.

### `ukui-kwin`

- **Nature** : fork-amont — **Amont** : KDE KWin (debian/copyright : Upstream-Name kwin, Source git://anongit.kde.org/kwin ; README 'forked from kwin') ; base ≈ KWin/Plasma 5.18 (find_package KDecoration2 5.18.0, KF5 5.66) ; fork UKUI 1.0.2kylin6-ok1 (PROJECT_VERSION 1.0.0)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (arbre forké complet) ; aucun debian/patches, delta vs KWin non isolable (arbre amont non acquis en phase 0)
- **Langages / taille** : C++/Qt5/KF5 (CMake), Qt UI, QML, JavaScript — 1 584 fichiers, 11,3 Mo ; ~777 fichiers de code (~206 k lignes) — lignes de code propres mesurées : 257 653
- **Privilège** : Gestionnaire de fenêtres de session ; postinst root modifiant les fichiers de configuration dans /home/*
- **Déclencheurs** : Autostart XDG phase WindowManager (si installé), update-alternatives x-window-manager, postinst
- **Indices réseau** : Aucun listener détecté (listen=[])
- **Blobs** : Aucun
- **Budget proposé** : L1 — 10 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Fork KDE KWin (~206 k lignes) non installé par l'ISO 3.0, dernière version 2022 (yangtze) : triage des scripts mainteneur et des ajouts UKUI seulement, delta non isolable sans l'amont.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-kwin, ukui-kwin-common, -data, -dev, -x11, libukui-kwin4-effect-builtins1, libukui-kwineffects12, libukui-kwinglutils12, libukui-kwinxrenderutils12 ; aucun paquet *-wayland déclaré | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --with kf5,pkgkde_symbolshelper --buildsystem kf5 | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-kwin.postinst` | bash root : pour chaque entrée de ls /home : sed -i (library=, theme=) dans ~/.config/ukui-kwinrc, rm ~/.config/kglobalshortcutsrc, cp /usr/share/touchgesture/touchgesture.xml sur ~/.config/touchgesture/ ; systemctl stop/disable touchegg.service ; rm /usr/share/X11/touchscreen/01-kylin.conf et /etc/xdg/autostart/touchegg.desktop | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-kwin-x11.postinst ; archive:debian/ukui-kwin-x11.prerm` | update-alternatives x-window-manager /usr/bin/ukui-kwin_x11 priorité 50 ; retrait au remove | CONSTATÉ |
| maintainer-script-orphelin | `archive:debian/ukui-kwin-wayland.postinst` | setcap CAP_SYS_RESOURCE=+ep /usr/bin/kwin_wayland ; paquet ukui-kwin-wayland absent de debian/control | CONSTATÉ |
| autostart | `archive:ukui-kwin.desktop` | -> /etc/xdg/autostart ; Exec=ukui-kwin_x11 --replace ; X-UKUI-Autostart-Phase=WindowManager | CONSTATÉ |
| cible-construite | `archive:helpers/killer/CMakeLists.txt` | ukui_kwin_killer_helper -> LIBEXEC_INSTALL_DIR | CONSTATÉ |
| maintscript | `archive:debian/ukui-kwin-common.maintscript` | rm_conffile de 6 fichiers /etc/xdg/*.knsrc et org_ukui_kwin.categories | CONSTATÉ |
| desktop-file | `archive:effects/*, archive:kcmkwin/* (61+ fichiers)` | Métadonnées d'effets et modules KCM, non lus individuellement | NON COUVERT |

- [CONSTATÉ] installe_par_defaut_iso vide ; dernière entrée debian/changelog du 22/08/2022 (yangtze).
- [PROBABLE] Base KWin 5.18 déduite des versions minimales KDecoration2/KF5.

### `ukui-sidebar`

- **Nature** : propre-kylin — **Amont** : UKUI (Vcs-Git https://gitee.com/openkylin/ukui-sidebar.git), 3.0 (quilt), 4.25.0.0-ok0.1 ; 3 patches quilt
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète ; .pc : 3/3 appliqués)
- **Langages / taille** : C++/Qt5 + QML (CMake) ; .ts — 1 002 fichiers, 3,1 Mo ; ~185 fichiers de code hors tests/3rd-parties (~27,5 k lignes) — lignes de code propres mesurées : 55 453
- **Privilège** : Session utilisateur ; client autorisé (listes blanches .limit) de services root com.kylin.networkCtrol et com.control.center.qt.systemdbus
- **Déclencheurs** : Autostart XDG phase Application, Chargement des greffons ukui-shortcut
- **Indices réseau** : Aucun listener réseau ; QLocalServer local; Greffons wifi/vpn/bluetooth/connectivity (pilotage réseau via services système); Collecte kdk_dia_upload
- **Blobs** : Aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session (~27,5 k lignes) sans privilège, avec IPC vers des services root réseau et collecte d'usage ; L3 sur les greffons wifi/vpn/connectivity et event-track.cpp.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Paquets ukui-sidebar, qml-module-org-ukui-sidebar-items, libukui-shortcut1, libukui-shortcut-dev ; Build-Depends libkysdk-datacollect-dev | CONSTATÉ |
| debian-install | `archive:debian/ukui-sidebar.install ; archive:debian/libukui-shortcut1.install` | usr/bin/ukui-sidebar, etc/xdg/autostart/ukui-sidebar.desktop ; libukui-shortcut.so.*, usr/lib/*/ukui-shortcut-plugins, usr/share/ukui-sidebar/shortcuts ; aucun script mainteneur | CONSTATÉ |
| autostart | `archive:data/ukui-sidebar.desktop` | Exec=/usr/bin/ukui-sidebar %u ; X-UKUI-Autostart-Delay=3 ; Phase=Application ; AutoRestart | CONSTATÉ |
| cible-construite | `archive:ukui-shortcut/shortcuts/*/CMakeLists.txt` | Greffons de raccourcis : screenshot, dark-mode, multi-screen, wifi, eyecare-mode, connectivity, bluetooth, vpn, auto-rotation, notebook, status-change, night-mode, brightness… -> PLUGIN_INSTALL_DIRS | CONSTATÉ |
| collecte | `archive:src/utils/event-track.cpp` | 1 appel kdk_dia_upload (libkysdk-datacollect) | CONSTATÉ |
| listener-local | `archive:3rd-parties/qtsingleapplication/src/qtlocalpeer.cpp` | QLocalServer (instance unique) ; fichier non lu | PROBABLE |

- [CONSTATÉ] /usr/bin/ukui-sidebar est nommé dans les listes blanches .limit de kylin-nm (com.kylin.networkCtrol) et ukui-control-center (com.control.center.qt.systemdbus).

#### Constats transverses du lot B09-ukui-1

- **[CONSTATÉ]** 7 fichiers d'activation D-Bus système du lot déclarent User=root, plus 1 unité systemd root Type=dbus (nm-enhance-optimization) activée au démarrage. — *preuve* : archive/ukui-settings-daemon/plugins/authority/conf/com.kylin.ukui.SettingsDaemon.service ; archive/ukui-control-center/registeredQDbus/conf/com.control.center.qt.systemdbus.service ; archive/ukui-control-center/group-manager-server/conf/org.ukui.groupmanager.service ; archive/ukui-greeter/ukui-greeter-daemon/org.ukui.GreeterDaemon.service ; archive/peony-extensions/peony-share/data/org.ukui.samba.share.config.service ; archive/kylin-nm/sys-dbus-netctrl/conf/com.kylin.networkCtrol.service ; archive/kylin-nm/sys-dbus-register/conf/com.kylin.network.qt.systemdbus.service — 'User=root' ; archive/kylin-nm/kylin-network-manager-enhance-daemon/conf/nm-enhance-optimization.service
- **[CONSTATÉ]** 3 politiques D-Bus système du lot autorisent la possession du nom de service en contexte default (tout utilisateur). — *preuve* : archive/ukui-screensaver/data/org.ukui.screensaver.conf : '<policy context="default"><allow own="org.ukui.screensaver"/>' ; archive/peony-extensions/peony-share/data/org.ukui.samba.share.config.conf : '<allow own="org.ukui.samba.share.config"/>' (default) ; archive/kylin-nm/kylin-network-manager-enhance-daemon/conf/com.kylin.network.enhancement.optimization.conf : '<allow own="com.kylin.network.enhancement.optimization"/>' (default)
- **[CONSTATÉ]** Mécanisme Kylin de listes blanches D-Bus (.limit + .limit.verify à empreinte de 40 caractères hexadécimaux) installé dans /etc/dbus-1/conf et /usr/share/dbus-1/conf par ukui-control-center (2), kylin-nm (3) et peony-extensions (1) ; ukui-greeter en contient un non installé. — *preuve* : archive/ukui-control-center/registeredQDbus/registeredQDbus.pro : 'limit1.path = /etc/dbus-1/conf/' ; archive/kylin-nm/sys-dbus-netctrl/sys-dbus-netctrl.pro : 'limit.path = /etc/dbus-1/conf/' ; archive/peony-extensions/peony-share/peony-share.pro : 'daemonLimit.path = /usr/share/dbus-1/conf/' ; archive/ukui-greeter/ukui-greeter-daemon/ukui-greeter-daemon.pro : 'INSTALLS += target configfile services kconf2'
- **[CONSTATÉ]** 5 paquets du lot dépendent au build de libkysdk-datacollect-dev et appellent kdk_dia_upload_* (collecte d'usage). — *preuve* : debian/control de ukui-control-center, ukui-panel, ukui-menu, peony, ukui-sidebar : 'libkysdk-datacollect-dev' ; appels dans archive/ukui-control-center/libukcc/interface/ukcccommon.cpp, archive/ukui-panel/panel/src/data-collector.cpp ('kdk_dia_upload_default(... adjust_panel_position_event ...)'), archive/ukui-menu/src/utils/event-track.cpp, archive/ukui-sidebar/src/utils/event-track.cpp, archive/peony/src/control/navigation-side-bar.cpp
- **[CONSTATÉ]** 3 paquets ont des scripts mainteneur root qui modifient ou créent des fichiers dans les répertoires personnels des utilisateurs. — *preuve* : archive/ukui-session-manager/debian/ukui-session-manager.postinst : 'file=/home/$SUDO_USER/.profile … sed -i' ; archive/peony/debian/upgrade-utils/desktop-grid/update-desktop-grid (appelé par peony-utils.postinst) : 'config_file="$home_dir/.peony-desktop-grid.conf"' ; archive/ukui-kwin/debian/ukui-kwin.postinst : 'for file in `ls /home` … sed -i … rm'
- **[CONSTATÉ]** 3 postinst exécutent un binaire d'un autre paquet : /usr/share/kylin-system-updater/kylin-reboot-required. — *preuve* : archive/ukui-control-center/debian/ukui-control-center.postinst ; archive/ukui-screensaver/debian/ukui-screensaver.postinst ; archive/ukui-greeter/debian/ukui-greeter.postinst : 'if [ -x /usr/share/kylin-system-updater/kylin-reboot-required ]'
- **[CONSTATÉ]** Détournements dpkg-divert entre paquets : ukui-screensaver détourne les fichiers LightDM/xgreeters de ukui-greeter et les greffons ukcc ; ukui-control-center/libukcc3 détournent libukcc.so.1* ; kylin-nm détourne /etc/nm_enhance/nm_enhance_dns.conf. — *preuve* : archive/ukui-screensaver/debian/ukui-screensaver.preinst : 'dpkg-divert --package ukui-screensaver --rename --divert "$path_desktop".old' ; archive/ukui-control-center/debian/libukcc3.preinst ; archive/kylin-nm/debian/kylin-nm.preinst : 'dpkg-divert --package kylin-network-manager-enhance-daemon'
- **[CONSTATÉ]** 2 fichiers PAM installés par le lot ; celui de ukui-screensaver accepte sans mot de passe les membres du groupe nopasswdlogin. Un binaire est rendu setuid root par postinst. — *preuve* : archive/ukui-screensaver/data/ukui-screensaver-qt : 'auth sufficient pam_succeed_if.so user ingroup nopasswdlogin' ; archive/ukui-control-center/checkUserPwdWithPAM/conf/control-center -> /etc/pam.d ; archive/ukui-control-center/debian/ukui-control-center.postinst : 'chmod u+s /usr/bin/changeuserpwd'
- **[CONSTATÉ]** Aucun listener TCP/UDP détecté dans le lot ; écoutes locales seulement : QLocalServer (instance unique) dans 10 paquets, ICE local de ukuismserver (TCP désactivé), sockets X11 UNIX de wlcom ; sockets noyau netlink (ukui-settings-daemon, ukui-control-center, kylin-nm) et PF_PACKET ARP (kylin-nm, root). — *preuve* : archive/ukui-session-manager/ukuismserver/ukuismserver.cpp : '_IceTransNoListen("tcp")' ; archive/kylin-wayland-compositor/src/xwayland/sockets.c : '/tmp/.X11-unix/X%d' ; archive/kylin-nm/sys-dbus-register/kyarping/kylinipv4arping.cpp : 'socket(PF_PACKET, SOCK_DGRAM, 0)' ; archive/ukui-settings-daemon/plugins/authority/procaddserver.cpp : 'NETLINK_CONNECTOR'
- **[CONSTATÉ]** Clients HTTP en clair codés en dur vers weather.ubuntukylin.com:8001 et ip-api.com. — *preuve* : archive/ukui-settings-daemon/plugins/gamma-manager/gamma-manager-define.h : '#define IP_API_ADDRESS_BACKUP "http://ip-api.com/json/"', 'http://weather.ubuntukylin.com:8001/…' ; archive/ukui-screensaver/src/screensaver/weathermanager.cpp : 'weatherServerAddr = "://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/"'
- **[CONSTATÉ]** Tous les arbres au format quilt du lot ont leurs patches appliqués (.pc/applied-patches = series) ; les répertoires .pc contiennent des copies pré-patch à exclure des recherches. — *preuve* : archive/peony-extensions/.pc/applied-patches : 298/298 ; archive/peony/.pc : 191/191 ; archive/ukui-panel/.pc : 170/170 ; archive/kylin-nm/.pc : 121/121 ; archive/ukui-screensaver/.pc : 51/51 ; grep des DESTINATION de ukui-screensaver retournant aussi ./.pc/0011-149-update-changelog.patch/data/CMakeLists.txt
- **[CONSTATÉ]** Des fichiers de configuration de sécurité Kylin sont installés sous /opt/system/conf/ukui-security-support/ par 5 paquets. — *preuve* : archive/ukui-settings-daemon/plugins/authority/authority.pro ; archive/ukui-panel/debian/ukui-panel.install ('/opt/system/conf/ukui-security-support/ukui-panel-v11.json') ; archive/ukui-menu/debian/ukui-menu.install ; archive/peony/debian/peony-common.install ; archive/ukui-notification/notification-server/CMakeLists.txt
- **[CONSTATÉ]** kylin-nm-plugin, libkylin-nm-base et kylin-nm proviennent du même amont (kylin-nm) et partagent du code (agent de secrets, greffons netconnect/wlanconnect). — *preuve* : debian/control des 3 paquets : 'Vcs-Git: https://github.com/ukui/kylin-nm.git' ; debian/watch : 'kylin-nm_(\d\S+)\.orig\.tar\.gz'
- **[CONSTATÉ]** Un binaire ELF précompilé est versionné dans un arbre source du lot. — *preuve* : archive/libkylin-nm-base/kylin-keyring/kylin-keyring : en-tête '7f 45 4c 46 02 01 01 03 … 03 00 3e 00' (ELF64 x86-64 ET_DYN), 3 227 408 octets
- **[CONSTATÉ]** 2 paquets du lot ne sont pas installés par l'ISO 3.0 (ukui-notification-daemon, ukui-kwin) et ont leur dernière entrée de changelog en distribution 'yangtze' ; ukui-greeter est en 'nile' (4.10). — *preuve* : fiche B09-ukui-1.json : installe_par_defaut_iso [] ; archive/ukui-kwin/debian/changelog : 'ukui-kwin (1.0.2kylin6-ok1) yangtze' ; archive/ukui-notification-daemon/debian/changelog : '(4.0.0.0-ok0~0412) yangtze' ; archive/ukui-greeter/debian/changelog : '(4.10.0.0-ok5.5update1) nile'
- **[CONSTATÉ]** Aucun arbre Gitee n'a été acquis pour ce lot (chemin_gitee=null pour les 17 paquets) ; l'inventaire repose uniquement sur l'archive. — *preuve* : fiche B09-ukui-1.json : 'chemin_gitee': null, 'struct_gitee': null pour chaque entrée

