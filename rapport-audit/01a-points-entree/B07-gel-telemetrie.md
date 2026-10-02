# Lot B07-gel-telemetrie

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-process-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (Homepage https://gitlab2.kylin.com/kylinos-src/kylin-process-manager, format 3.0 native) ; daemon/src/cpulimit_fork.c dérivé de l'outil cpulimit (commentaire l.36) — PROBABLE
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 262 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) mais non acquis
- **Langages / taille** : C++/Qt5 (977 Ko), C (cpulimit_fork), XML D-Bus, JSON de règles — 262 fichiers, 1,97 Mo ; ~36,3 k lignes de code dont autotests 10,6 k ; démons root daemon/ 6,7 k + daemon1/ 0,3 k ; core/ 11,7 k ; control/ 3,6 k — lignes de code propres mesurées : 36 341
- **Privilège** : root : kylin-process-manager-daemon, kylin-process-resource-manager-daemon, thawer (oneshot), cleaner/cpulimit_fork (helpers) ; utilisateur : kylin-process-manager (unité systemd --user, bus session)
- **Déclencheurs** : boot : multi-user.target (deux démons système), activation D-Bus système com.kylin.ProcessManagerDaemon / com.kylin.ProcessResourceManagerDaemon, arrêt/reboot/halt : kylin-process-manager-thawer.service, session graphique : activation D-Bus session com.kylin.ProcessManager, dpkg : postinst (copie de configuration dans /etc)
- **Indices réseau** : aucun indice d'écoute (champ listen vide); liaison à libkysdk-datacollect, client D-Bus de com.kylin.daq (voir libkysdk-diagnosetest) — PROBABLE canal de télémétrie
- **Blobs** : aucun (blob_count=0) ; debian/kylin-process-manager-coverage.xml 443 Ko = rapport de couverture XML
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant propre Kylin d'environ 25,6 k lignes hors tests, avec deux démons système root exposés à tout appelant D-Bus et pilotant cgroups/freezer, plus un lien vers la bibliothèque de collecte de données.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:kylin-process-manager/daemon/configs/kylin-process-manager-daemon.service` | Type=dbus ; BusName=com.kylin.ProcessManagerDaemon ; ExecStart=/usr/bin/kylin-process-manager-daemon ; aucun User= (root) ; After=display-manager.service ; WantedBy=multi-user.target ; installé /lib/systemd/system (daemon/CMakeLists.txt l.86) | CONSTATÉ |
| dbus-activation (système) | `archive:kylin-process-manager/daemon/configs/com.kylin.ProcessManagerDaemon.service` | Name=com.kylin.ProcessManagerDaemon ; Exec=/usr/bin/kylin-process-manager-daemon ; User=root ; SystemdService=kylin-process-manager-daemon.service | CONSTATÉ |
| dbus-policy | `archive:kylin-process-manager/daemon/configs/com.kylin.ProcessManagerDaemon.conf` | policy user=root own=com.kylin.ProcessManagerDaemon ; policy context=default : allow send_destination=com.kylin.ProcessManagerDaemon + receive_sender, sans restriction d'interface ; installé /usr/share/dbus-1/system.d | CONSTATÉ |
| kylin-config whitelist (yaml) | `archive:kylin-process-manager/daemon/configs/com.kylin.ProcessManagerDaemon.yaml` | installé /etc/kylin-config/basic ; audit=false ; whitelist de 7 appelants (kylin-process-manager, ukui-search-service, ukui-powermanagement-service, kylin-process-manager-cleaner, kylin-process-manager-thawer, ukui-system-monitor, ukui-session) | CONSTATÉ |
| systemd-service oneshot (arrêt) | `archive:kylin-process-manager/daemon/configs/kylin-process-manager-thawer.service` | Type=oneshot ; ExecStart=/usr/bin/kylin-process-manager-thawer (script bash daemon/configs/kylin-process-manager-thawer installé par install(FILES)) ; DefaultDependencies=no ; WantedBy=shutdown.target, reboot.target, halt.target ; root | CONSTATÉ |
| helper root | `archive:kylin-process-manager/daemon/CMakeLists.txt` | l.88 install /usr/bin/kylin-process-manager-cleaner (script bash, argument CGROUP_PATH) ; l.91-93 add_executable(cpulimit_fork) installé /usr/bin ; listés dans debian/kylin-process-manager-daemon.install | CONSTATÉ |
| systemd-service + dbus-activation + policy (système, daemon1) | `archive:kylin-process-manager/daemon1/configs/kylin-process-resource-manager-daemon.service` | Type=dbus BusName=com.kylin.ProcessResourceManagerDaemon ; ExecStart=/usr/bin/kylin-process-resource-manager-daemon ; aucun User= ; WantedBy=multi-user.target ; activation daemon1/configs/com.kylin.ProcessResourceManagerDaemon.service User=root ; policy .conf context=default allow send_destination ; paquet binaire kylin-process-resource-manager-daemon | CONSTATÉ |
| systemd-service (user) + dbus-activation (session) | `archive:kylin-process-manager/configs/kylin-process-manager.service` | unité user (/usr/lib/systemd/user) ; Type=dbus BusName=com.kylin.ProcessManager ; ExecStart=/usr/bin/kylin-process-manager ; Slice=session.slice ; Restart=always ; PartOf=graphical-session.target ; pas de section [Install] ; activation session configs/com.kylin.ProcessManager.service (SystemdService=kylin-process-manager.service) | CONSTATÉ |
| maintainer-script postinst | `archive:kylin-process-manager/debian/kylin-process-manager.postinst` | mkdir -p /etc/kylin-process-manager/policy_rules.d ; cp (\|\| true) des JSON de /usr/share/kylin-process-manager vers /etc/kylin-process-manager ; aucune commande systemctl | CONSTATÉ |
| rsyslog / logrotate | `archive:kylin-process-manager/configs/40-kylin-process-manager.conf` | /etc/rsyslog.d : redirection des 3 programmes vers /var/log/kylin-process-manager.log ; configs/kylin-process-manager -> /etc/logrotate.d (su root root, daily, rotate 3, copytruncate) | CONSTATÉ |
| debian-rules / build | `archive:kylin-process-manager/CMakeLists.txt` | debian/rules = 'dh $@' ; CMake : pkg_check_modules(DATACOLLECT kysdk-datacollect) l.31, lié à la cible kylin-process-manager l.270-283 ; add_subdirectory(daemon, daemon1) ; Build-Depends libkysdk-datacollect-dev, libcgroup-dev, libproc2-dev | CONSTATÉ |

- [CONSTATÉ] Le paquet binaire kylin-process-resource-manager-daemon est défini dans debian/control mais absent de la liste installe_par_defaut_iso (seuls kylin-process-manager et kylin-process-manager-daemon y figurent).
- [NON COUVERT] Forges Gitee/GitHub PRESENT avec relation DIVERGENT ; aucun arbre Gitee acquis (chemin_gitee null), les différences ne sont pas inventoriées.
- [NON COUVERT] Le contenu des scripts thawer/cleaner exécutés en root n'a été lu qu'en en-tête (3 lignes) ; leur logique relève de la Phase 1.

### `kylin-status-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (format 3.0 native, mainteneur liling@kylinos.cn) ; QtSingleApplication (Qt Solutions) incorporé
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 50 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt5 (qmake) — 50 fichiers, 226 Ko ; 28 fichiers de code, ~3,5 k lignes — lignes de code propres mesurées : 3 534
- **Privilège** : utilisateur (session graphique, bus session)
- **Déclencheurs** : activation D-Bus session com.kylin.statusmanager.interface, autostart UKUI phase Initialization (si le .desktop est installé)
- **Indices réseau** : QLocalServer d'instance unique (local, non réseau); aucun indice réseau TCP/UDP
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petit composant de session (~3,5 k lignes) exposant un service D-Bus session et consommant les événements d'entrée de ukui-input-gather.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-activation (session) | `archive:kylin-status-manager/data/com.kylin.statusmanager.interface.service` | Name=com.kylin.statusmanager.interface ; Exec=/usr/bin/kylin-status-manager ; pas de User= ; installé /usr/share/dbus-1/services/ (debian/install) ; enregistrement sur sessionBus (DbusManager/dbusservice.cpp l.42-44) | CONSTATÉ |
| autostart .desktop | `archive:kylin-status-manager/data/kylin-status-manager.desktop` | Exec=/usr/bin/kylin-status-manager ; NoDisplay=true ; OnlyShowIn=UKUI ; X-UKUI-Autostart-Phase=Initialization ; X-UKUI-AutoRestart=true ; fichier non référencé par debian/install ni kylin-status-manager.pro (grep) : mode d'installation non établi | PROBABLE |
| debian-install | `archive:kylin-status-manager/debian/install` | première ligne '#!/bin/bash' (fichier mode 644, non exécutable) puis une seule entrée data/com.kylin.statusmanager.interface.service -> /usr/share/dbus-1/services/ | CONSTATÉ |
| debian-control / rules | `archive:kylin-status-manager/debian/control` | Depends ukui-input-gather (>=4.1.0.0-ok0.3) ; Build-Depends libukuiinputgatherclient-dev, libkysdk-sysinfo-dev, libqt5sensors5-dev, iio-sensor-proxy ; debian/rules = gabarit dh_make 'dh $@' | CONSTATÉ |
| cible de build | `archive:kylin-status-manager/kylin-status-manager.pro` | TARGET kylin-status-manager -> /usr/bin ; QT core dbus xml sensors concurrent ; LIBS -lukui-log4qt -lukuiinputgatherclient ; DBUS_ADAPTORS com.kylin.statusmanager.interface.xml | CONSTATÉ |
| socket locale (instance unique) | `archive:kylin-status-manager/QtSingleApplication/qtlocalpeer.cpp` | l.103-126 : QLocalServer::listen(socketName) suffixé par l'uid, fichier de verrou dans QDir::tempPath() ; mécanisme d'instance unique, pas d'écoute réseau | CONSTATÉ |
| client D-Bus système autorisé | `archive:ukui-input-gather/ukui-input-gather/installation/org.ukui.libinput.proxy.limit` | /usr/bin/kylin-status-manager est le seul appelant listé en whitelist de org.ukui.libinput.proxy (key1) | CONSTATÉ |

- [CONSTATÉ] data/conf.ini associe des valeurs DMI (sys_vendor, product_family) à des types de machine (SLATE, LAPTOP...) ; fichiers de configuration lus partiellement.
- [CONSTATÉ] QtSingleApplication/qtlockedfile_win.cpp (code Windows) est présent dans l'arbre.
- [NON COUVERT] Arbre Gitee (branches openkylin/huanghe, packaging/openkylin/huanghe) non acquis.

### `ukui-input-gather`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitee.com/openkylin/ukui-input-gather.git, format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 49 fichiers) ; Gitee PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt5 (qmake), libinput/libevdev/libudev — 49 fichiers, 132 Ko ; 23 fichiers de code, ~2,4 k lignes — lignes de code propres mesurées : 2 365
- **Privilège** : root (groupe input) pour le démon ; privilège du processus appelant pour libukuiinputgatherclient
- **Déclencheurs** : boot : multi-user.target (unité activée par postinst), activation D-Bus système org.ukui.libinput.proxy, connexion d'un client sur la socket unix abstraite
- **Indices réseau** : socket AF_UNIX abstraite locale (pas de TCP/UDP); QLocalServer WorldAccessOption présent mais appel commenté
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Démon root de petite taille (~2,4 k lignes) qui lit les événements libinput de tous les périphériques d'entrée et les redistribue par socket locale et D-Bus système.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:ukui-input-gather/ukui-input-gather/installation/ukui-input-gather.service` | Type=simple ; ExecStart=/usr/bin/ukui-input-gather ; Group=input ; aucun User= (commentaire : 'This service is a root permission daemon') ; Restart=on-failure ; WantedBy=multi-user.target ; installé /lib/systemd/system | CONSTATÉ |
| maintainer-script postinst | `archive:ukui-input-gather/debian/ukui-input-gather.postinst` | deb-systemd-helper enable ukui-input-gather.service | CONSTATÉ |
| dbus-activation (système) | `archive:ukui-input-gather/ukui-input-gather/installation/org.ukui.libinput.proxy.service` | Name=org.ukui.libinput.proxy ; Exec=/usr/bin/ukui-input-gather ; User=root ; pas de SystemdService= ; enregistré sur systemBus (src/libinputdbusserver.cpp l.44-47) | CONSTATÉ |
| dbus-policy | `archive:ukui-input-gather/ukui-input-gather/installation/org.ukui.libinput.proxy.conf` | root own ; context=default : allow send_destination=org.ukui.libinput.proxy pour interfaces org.ukui.libinput.proxy, Introspectable, Properties | CONSTATÉ |
| dbus limit / kylin-config | `archive:ukui-input-gather/ukui-input-gather/installation/org.ukui.libinput.proxy.limit` | installé dans /etc/dbus-1/conf et /usr/share/dbus-1/conf avec .limit.verify (empreinte hexadécimale de 40 caractères) ; whitelist key1=/usr/bin/kylin-status-manager ; org.ukui.libinput.proxy.yaml -> /etc/kylin-config/basic (audit=false) | CONSTATÉ |
| socket unix (écoute) | `archive:ukui-input-gather/ukui-input-gather/src/socketserver.cpp` | l.61-86 : socket(AF_UNIX, SOCK_STREAM), bind sur SOCKET_NAME (socketserver.h l.30 : octet nul + '/InputGatherServer', longueur fixée à 10 octets = espace abstrait), listen(backlog 10) ; activé par main.cpp l.38 SocketServer::self()->run() | CONSTATÉ |
| QLocalServer (inactif) | `archive:ukui-input-gather/ukui-input-gather/src/localserver.cpp` | l.54-67 : QLocalServer avec WorldAccessOption, nom 'InputGatherServer' ; appel LocalServer::self()->runServer() commenté dans main.cpp l.39 | CONSTATÉ |
| bibliothèque cliente | `archive:ukui-input-gather/debian/libukuiinputgatherclient1.install` | /usr/lib/*.so.* (libukuiinputgatherclient, chargée dans les processus clients) ; -dev : /usr/include/*, /usr/lib/*.so | CONSTATÉ |
| debian-rules / build | `archive:ukui-input-gather/ukui-input-gather/ukui-input-gather.pro` | LIBS -linput -ludev -levdev ; QT network dbus ; INSTALLS service systemd, conf/service D-Bus, limit1/limit2, yaml ; debian/rules hardening=+all, dh_shlibdeps --ignore-missing-info | CONSTATÉ |

- [CONSTATÉ] Le binaire de test ukui-input-gather-test et le dossier test/ existent dans l'arbre ; leur installation n'apparaît dans aucun fichier debian/*.install.
- [NON COUVERT] Arbre Gitee (branches dont qt6, upstream-backup) non acquis.

### `app-compat-dkms`

- **Nature** : absent-ou-vide — **Amont** : inconnu (mainteneur wangkanglong@loongson.cn ; description : compatibilité syscall/ucontext entre ABI)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive = squelette dh_make (19 fichiers debian/, gabarits *.ex) ; forges Gitee/GitHub ABSENT
- **Langages / taille** : aucun (fichiers debian uniquement) — 19 fichiers, 25,7 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (un module noyau s'exécuterait en ring 0 ; aucun artefact disponible)
- **Déclencheurs** : NON COUVERT : aucun script mainteneur actif
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — L'arbre ne contient aucun code de module ni fichier DKMS ; il n'y a rien à auditer.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:app-compat-dkms/debian/control` | Package app-compat-dkms, Section kernel, Architecture any ; Description 'app-compat kmod ... compatibility support syscall and ucontext in different ABIs' ; aucune dépendance dkms | CONSTATÉ |
| debian-rules | `archive:app-compat-dkms/debian/rules` | gabarit dh_make : 'dh $@' seulement | CONSTATÉ |
| dkms.conf / Kbuild / modprobe | `archive:app-compat-dkms/` | aucun dkms.conf, Kbuild, Makefile ni source de module dans l'arbre ; postinst.ex/preinst.ex/prerm.ex/postrm.ex sont des gabarits inactifs | CONSTATÉ |

- [CONSTATÉ] meta.source_coquille_vide=0 alors que l'arbre n'est qu'un squelette dh_make (changelog 'Initial Release', 24/08/2026, huanghe).
- [CONSTATÉ] Le paquet n'est pas installé par défaut par l'ISO 3.0.

### `ukui-service-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur lanyue@kylinos.cn ; changelog : synchronisation depuis une branche Gerrit interne)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète, 77 fichiers) ; forges Gitee/GitHub ABSENT
- **Langages / taille** : C++/Qt5 (qmake), Bash (helpers) — 77 fichiers, 73 Ko ; 29 fichiers de code, ~1,2 k lignes — lignes de code propres mesurées : 1 154
- **Privilège** : root (ukui-system-service-manager, ukui-system-service) ; utilisateur (variantes session)
- **Déclencheurs** : boot : graphical.target (unité système), activation D-Bus système et session org.ukui.serviceManager, appel registerServiceByPlugin (chargement de plugin à partir d'un chemin passé en argument), session : graphical-session.target (unité user activée par postinst)
- **Indices réseau** : aucun indice réseau (champ listen vide)
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Service système root de petite taille (~1,2 k lignes) qui charge des plugins enregistrés via D-Bus, avec une politique D-Bus qui autorise own au contexte default.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:ukui-service-manager/data/ukui-system-service-manager.service` | Type=dbus ; BusName=org.ukui.serviceManager ; ExecStart=/usr/bin/ukui-system-service-manager ; aucun User= (root) ; Restart=on-failure ; WantedBy=graphical.target ; installé /usr/lib/systemd/system (ukui-system-service-manager.pro l.40) | CONSTATÉ |
| dbus-activation (système) | `archive:ukui-service-manager/data/system/org.ukui.serviceManager.service` | Name=org.ukui.serviceManager ; User=root ; Exec=/usr/bin/ukui-system-service-manager ; pas de SystemdService= | CONSTATÉ |
| dbus-policy | `archive:ukui-service-manager/data/org.ukui.serviceManager.conf` | installé /usr/share/dbus-1/system.d ; policy user=root own org.ukui.serviceManager, .Register, .Launcher, own_prefix org.ukui.Service ; policy context=default contient aussi allow own sur org.ukui.serviceManager, .Register, .Launcher et own_prefix='org.ukui.Service', plus send_destination(_prefix) sans restriction d'interface | CONSTATÉ |
| systemd-service (user) + dbus-activation (session) | `archive:ukui-service-manager/data/ukui-session-service-manager.service` | unité user /usr/lib/systemd/user ; Type=dbus BusName=org.ukui.serviceManager ; ExecStart=/usr/bin/ukui-session-service-manager ; WantedBy=graphical-session.target ; activation data/session/org.ukui.serviceManager.service | CONSTATÉ |
| maintainer-script postinst | `archive:ukui-service-manager/debian/ukui-service-manager.postinst` | deb-systemd-helper --user enable ukui-session-service-manager.service ; preinst/prerm/postrm = gabarits sans action hors #DEBHELPER# | CONSTATÉ |
| helper d'enregistrement de plugin | `archive:ukui-service-manager/src/ukui-system-service-manager/register-ukui-system-service` | installé /usr/bin ; dbus-send --system --dest=org.ukui.serviceManager ... registerServiceByPlugin string:$1 array:string:... ; équivalent bus session : src/ukui-session-service-manager/register-ukui-session-service | CONSTATÉ |
| lanceurs de services | `archive:ukui-service-manager/src/src.pro` | cibles ukui-system-service, ukui-system-service-manager, ukui-session-service, ukui-session-service-manager (-> /usr/bin) ; libukuiservice -> /usr/lib ; ukuiserviceinterface -> QT_INSTALL_LIBS + en-têtes /usr/include/ukui-service-interface | CONSTATÉ |
| dbus-policy (démo) | `archive:ukui-service-manager/service-plugin-demo/org.ukui.service.demo.conf` | installé /usr/share/dbus-1/system.d (service-plugin-demo.pro l.34) ; root own_prefix=org.ukui.service ; default send_destination_prefix=org.ukui.service ; inclus vraisemblablement dans le .deb via le motif usr/share/dbus-1/* de debian/ukui-service-manager.install (bibliothèque démo /usr/local/lib exclue par debian/not-installed) | PROBABLE |
| debian-rules | `archive:ukui-service-manager/debian/rules` | dh $@ --with-systemd | CONSTATÉ |
| dbus-policy (contenu) [ajout critique] | `archive:ukui-service-manager/data/org.ukui.serviceManager.conf` | Present dans la fiche, absent du recensement et des constats B07. Le contexte default autorise 'allow own' sur le bus systeme pour org.ukui.serviceManager (l.13), org.ukui.serviceManager.Register (l.31) et org.ukui.serviceManager.Launcher (l.49), et 'own_prefix' sur org.ukui.Service (l.67). Installe dans /usr/share/dbus-1/system.d (ukui-system-service-manager.pro:31-32). Activation User=root (data/system/org.ukui.serviceManager.service). Paquet installe par l'ISO (0.1.0-ok4~0813). | CONSTATÉ |

- [CONSTATÉ] debian/control conserve les descriptions auto-générées par debmake et un Homepage gabarit.
- [CONSTATÉ] Aucune forge publique (Gitee/GitHub ABSENT).

### `kylin-daq`

- **Nature** : propre-kylin — **Amont** : Kylin (format 3.0 native, mainteneur kylin@kylinos.cn, uploader pengcheng@kylinos.cn)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 68 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt5 (qmake ; QtNetwork, QtDBus, QtSql ; libcrypto) — 68 fichiers, 469 Ko ; 48 fichiers de code, ~10,6 k lignes — lignes de code propres mesurées : 10 573
- **Privilège** : root (kylin-daq-daemon, User=root)
- **Déclencheurs** : boot : multi-user.target (service activé par postinst), activation D-Bus système com.kylin.daq par tout appelant, minuteries internes (PollingInterval / UploadInterval de la configuration), dpkg : postinst redémarre le service
- **Indices réseau** : URL de remontée https://datareporting.openkylin.top:30005/ (conf/kylinDaqConf.json l.39, conf/defaultConf.json l.133); src/TransmitModule.cpp/.h utilisent QNetworkAccessManager/QSsl* (grep) : client HTTPS — PROBABLE; QLocalServer d'instance unique (local)
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Démon root de télémétrie (~10,6 k lignes) qui collecte des informations système et utilisateur, les met en cache en SQL et les transmet en HTTPS ; il est joignable par D-Bus depuis tout le système.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:kylin-daq/data/kylin-daq.service` | Type=dbus ; BusName=com.kylin.daq ; ExecStart=/usr/sbin/kylin-daq-daemon ; User=root ; Restart=always ; After=lightdm.service ; WantedBy=multi-user.target ; installé /usr/lib/systemd/system (kylin-daq.pro) | CONSTATÉ |
| systemd-timer | `archive:kylin-daq/data/kylin-daq.timer` | OnStartupSec=5minutes ; Unit=kylin-daq.service ; WantedBy=multi-user.target ; absent des INSTALLS de kylin-daq.pro ; désactivé par postinst | CONSTATÉ |
| dbus-activation (système) | `archive:kylin-daq/data/com.kylin.daq.service` | Name=com.kylin.daq ; SystemdService=kylin-daq.service ; User=root ; installé /usr/share/dbus-1/system-services | CONSTATÉ |
| dbus-policy | `archive:kylin-daq/data/com.kylin.daq.conf` | installé /etc/dbus-1/system.d ; root own/send/receive ; context=default : allow send_destination=com.kylin.daq, sans restriction d'interface | CONSTATÉ |
| dbus limit | `archive:kylin-daq/data/com.kylin.daq.limit` | installé /etc/dbus-1/conf et /usr/share/dbus-1/conf avec .limit.verify ; whitelist de 48 chemins (kylin-system-updater, ukui-control-center, kylin-os-manager, kylin-process-manager, kguard-desktop/research y compris /var/opt/appdata/kguard/bin, applications /opt/apps/top.openkylin.*) ; [envwhitelist] env1=LD_LIBRARY_PATH | CONSTATÉ |
| maintainer-script postinst | `archive:kylin-daq/debian/postinst` | systemctl daemon-reload ; systemctl enable kylin-daq.service ; systemctl disable kylin-daq.timer ; killall /usr/sbin/kylin-daq-daemon ; systemctl restart kylin-daq.service (chaque commande \|\| true) | CONSTATÉ |
| configuration de collecte/transmission | `archive:kylin-daq/conf/kylinDaqConf.json` | installé /var/cache/kylin-daq ; TransmitModule.UploadRealm ['https://datareporting.openkylin.top:30005/'] ; UploadState=1 ; UploadInterval=3600 s ; PollingInterval Basic/Online/CommonInfo=86400 s ; CacheModule StorageDuration=2592000 s ; DBusModule.ValidPackageName ['kylin-system-updater'] ; conf/defaultConf.json (embarqué via conf.qrc) contient la même URL | CONSTATÉ |
| socket locale (instance unique) | `archive:kylin-daq/src/SingleApplication.cpp` | l.69-80 : QLocalServer::listen(m_serverName) puis removeServer et nouvelle écoute en cas d'échec ; utilisé par src/main.cpp l.63 | CONSTATÉ |
| cible de build / install | `archive:kylin-daq/kylin-daq.pro` | TARGET kylin-daq-daemon -> /usr/sbin ; QT network dbus sql ; LIBS -ldl -lcrypto ; DEFINES QT_NO_DEBUG_OUTPUT ; INSTALLS : unité systemd, activation/policy D-Bus, data/com.kylin.daq.xml (/usr/share/dbus-1/interfaces), limit, yaml data/com.kylin.daq.yaml ; debian/install vide ; debian/rules hardening=+all | CONSTATÉ |
| systemd-timer (non installe) + fichier declare absent [ajout critique] | `archive:kylin-daq/data/kylin-daq.timer ; archive:kylin-daq/kylin-daq.pro` | Le timer (OnStartupSec=5minutes, Unit=kylin-daq.service) figure dans la fiche mais pas dans le recensement. Il n'est pas installe par kylin-daq.pro, et debian/postinst execute 'systemctl disable kylin-daq.timer'. kylin-daq.pro installe aussi data/com.kylin.daq.yaml vers /etc/kylin-config/basic/, mais ce fichier est absent de l'arbre (NON COUVERT). | NON COUVERT |

- [CONSTATÉ] debian/control décrit le paquet comme 'kylin-daq collect user information, analyze and process basic data'.
- [CONSTATÉ] data/com.kylin.daq.yaml est référencé dans les INSTALLS de kylin-daq.pro mais absent de l'arbre source.
- [CONSTATÉ] Le changelog 1.2.9.2-ok2.2 (26/08/2026) traite un bogue '数据埋点' (point de collecte DATA5) ; ok2.1 indique une fusion de la branche yhkylin/v11-2603.
- [NON COUVERT] Le dépôt Gitee (branches openkylin/huanghe...) n'a pas été acquis ; la divergence avec l'archive n'est pas mesurée.

### `libkysdk-diagnosetest`

- **Nature** : propre-kylin — **Amont** : Kylin (Vcs-Git https://gitlab2.kylin.com/kylinos-src/libkysdk-diagnosetest.git, interne ; format 3.0 quilt)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète ; patch 0001 appliqué, répertoire .pc présent) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C (bibliothèque), CMake ; tests C++/Shell — 31 fichiers, 228 Ko ; ~1,4 k lignes de code hors .pc (libkydatacollect.c 28,7 Ko) — lignes de code propres mesurées : 1 390
- **Privilège** : privilège du processus qui charge la bibliothèque
- **Déclencheurs** : chargement dynamique par les applications liées (ex. kylin-process-manager)
- **Indices réseau** : aucun accès réseau direct relevé ; IPC D-Bus système vers com.kylin.daq
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petite bibliothèque C (~1,4 k lignes) de collecte de données d'usage, cliente du service de télémétrie root com.kylin.daq.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-install-lists | `archive:libkysdk-diagnosetest/debian/libkysdk-datacollect.install` | usr/lib/*/libkysdk-datacollect.so.* ; -dev : kysdk-datacollect.pc, /usr/include/kysdk/diagnosetest/libkydatacollect.h, .so ; debian/libkysdk-diagnosetest.install vide | CONSTATÉ |
| debian-control | `archive:libkysdk-diagnosetest/debian/control` | paquets libkysdk-diagnosetest, -dev, -doc, libkysdk-datacollect ('used to collect user operation data, for optimize applications function') ; Build-Depends libcjson-dev, libdbus-1-dev, libssl-dev, libkysdk-log-dev | CONSTATÉ |
| cible de build | `archive:libkysdk-diagnosetest/kysdk-datacollect/src/CMakeLists.txt` | add_library(kysdk-datacollect SHARED libkydatacollect.c) SOVERSION 1 ; liens dbus-1, libcjson, openssl, kysdk-log | CONSTATÉ |
| IPC D-Bus sortant | `archive:libkysdk-diagnosetest/kysdk-datacollect/src/libkydatacollect.c` | l.49-55 : chaînes 'com.kylin.daq', 'com.kylin.daq.interface', '/org/freedesktop/activation' (grep) : client du service root kylin-daq | CONSTATÉ |
| debian-rules | `archive:libkysdk-diagnosetest/debian/rules` | hardening=+all ; 'dh $@' ; options spécifiques sw64 | CONSTATÉ |

- [CONSTATÉ] Le patch debian/patches/0001-v11-3.0.1.0-0k0.8-update-changelog.patch (42 Ko) modifie libkydatacollect.c, libkydatacollect.h et CMakeLists.txt malgré son nom.
- [CONSTATÉ] Seul le paquet binaire libkysdk-datacollect est installé par l'ISO.

### `libkysdk-base`

- **Nature** : propre-kylin — **Amont** : Kylin (format 3.0 quilt, mainteneur chenzhikai@kylinos.cn) ; bibliothèques tierces incorporées : nlohmann/json (src/diagnostics/nlohmann), cpp_stub (src/tests/cpp_stub)
- **Installé par défaut (ISO 3.0)** : oui (8 binaire(s)) — **Code disponible** : archive (source complète ; patches 0001-.patch et 0002-V11.patch appliqués, .pc présent) ; Gitee PRESENT (non acquis), GitHub EMPTY
- **Langages / taille** : C, C++, Python 3 (services conf2 et outils), CMake — 293 fichiers, 2,3 Mo ; ~47 k lignes hors .pc, dont tests 7,5 k et nlohmann ; code propre hors tests et vendored ~17 k (conf2 9,4 k, log 2,1 k, utils 2 k, config 1,2 k, diagnostics 0,7 k) — lignes de code propres mesurées : 47 316
- **Privilège** : root (conf2-system.py, conf2-sync-system, conf2-compile) ; utilisateur (conf2-session.py, conf2-sync-session) ; bibliothèques au privilège de l'appelant
- **Déclencheurs** : boot : multi-user.target (kysdk-conf2 et conf2-compile activés par postinst ; SyncConfig selon debhelper), conf2-compile avant dbus.service, ouverture de session (2 autostart XDG), dpkg : postinst (systemctl enable, lien symbolique)
- **Indices réseau** : aucun indice d'écoute (champ listen vide); IPC D-Bus vers com.kylin.daq depuis libkydiagnostics (point de collecte '埋点')
- **Blobs** : aucun dans l'arbre ; debian/patches/0001-.patch (231 Ko) contient des données binaires (suppression de src/conf2/service/verify/{amd64,arm64}/calc_verify)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Socle SDK installé en 8 paquets binaires, avec deux services D-Bus système root (dont un script Python), un oneshot avant dbus, un ajout ld.so et un module de télémétrie ; ~17 k lignes propres hors tests et code tiers.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:libkysdk-base/src/conf2/service/kysdk-conf2.service` | Type=dbus ; BusName=com.kylin.kysdk.conf2 ; ExecStart=/usr/share/kysdk/kysdk-base/conf2-system.py ; aucun User= (root) ; Restart=always ; Before=systemd-logind.service ; WantedBy=multi-user.target ; installé usr/lib/systemd/system | CONSTATÉ |
| dbus-policy | `archive:libkysdk-base/src/conf2/service/kysdk-conf2.conf` | installé etc/dbus-1/system.d ; root own ; context=default : allow send_destination=com.kylin.kysdk.conf2, sans restriction d'interface | CONSTATÉ |
| systemd-service (système) | `archive:libkysdk-base/src/conf2/sync-config/com.kylin.kysdk.SyncConfig.service` | Type=dbus ; BusName=com.kylin.kysdk.SyncConfig ; ExecStart=/usr/bin/conf2-sync-system ; aucun User= ; WantedBy=multi-user.target ; policy com.kylin.kysdk.SyncConfig.conf : context=default allow send_destination | CONSTATÉ |
| systemd-service oneshot (pré-D-Bus) | `archive:libkysdk-base/src/conf2/configs/conf2-compile.service` | Type=oneshot ; ExecStart=/usr/bin/conf2-compile ; DefaultDependencies=no ; Before=dbus.service ; WantedBy=multi-user.target ; root | CONSTATÉ |
| autostart XDG | `archive:libkysdk-base/src/conf2/service/kysdk-conf2.desktop` | -> /etc/xdg/autostart ; Exec=/usr/share/kysdk/kysdk-base/conf2-session.py ; X-UKUI-Autostart-Phase=Initialization ; second autostart src/conf2/sync-config/com.kylin.kysdk.SyncConfig.desktop Exec=/usr/bin/conf2-sync-session | CONSTATÉ |
| maintainer-script postinst | `archive:libkysdk-base/debian/libkysdk-conf2.postinst` | sans shebang ni set -e ; supprime conditionnellement /etc/systemd/system/multi-user.target.wants/conf2-update-db.service ; systemctl enable conf2-compile.service ; systemctl enable kysdk-conf2.service | CONSTATÉ |
| maintainer-script postinst | `archive:libkysdk-base/debian/libkysdk-conf2-tools.postinst` | ln -s /usr/share/kysdk/kysdk-base/kconf2-editor /usr/bin/kconf2-editor | CONSTATÉ |
| ld.so.conf.d | `archive:libkysdk-base/development-files/kysdk-base.conf` | installé /etc/ld.so.conf.d par libkysdk-basecommon (debian/libkysdk-basecommon.install) ; contenu : /usr/lib/kysdk/kysdk-base | CONSTATÉ |
| cron (non installé) | `archive:libkysdk-base/src/log/logrotate.cron` | '00 0 * * * /usr/bin/logrotate -s /tmp/kylog-rotate-tmp /etc/kysdk/kysdk-base/kylog-rotate-default' ; ligne d'installation commentée dans debian/libkysdk-log.install ; kylog-rotate-default installé /etc/kysdk/kysdk-base | CONSTATÉ |
| outils conf2 | `archive:libkysdk-base/debian/libkysdk-conf2-tools.install` | src/conf2/tools/health-check, gschema_xml2yaml, ini2yaml (scripts Python) -> usr/bin ; kconf2-editor (Python) -> /usr/share/kysdk/kysdk-base ; kconf2-completion.sh -> etc/bash_completion.d ; usr/bin/kconf2 (compilé depuis kconf2-tool.c) | CONSTATÉ |
| IPC D-Bus sortant (télémétrie) | `archive:libkysdk-base/src/diagnostics/buriedpoint.cpp` | l.49-51 : chaînes 'com.kylin.daq' et 'com.kylin.daq.interface' (grep) ; compilé dans libkydiagnostics.so (paquet libkysdk-diagnostics) | CONSTATÉ |
| debian-rules | `archive:libkysdk-base/debian/rules` | gabarit dh_make 'dh $@' ; 17 listes debian/*.install | CONSTATÉ |
| dbus-policy / systemd-service (contenu) [ajout critique] | `archive:libkysdk-base/src/conf2/service/kysdk-conf2.conf ; src/conf2/sync-config/com.kylin.kysdk.SyncConfig.conf ; src/conf2/service/kysdk-conf2.service ; src/conf2/sync-config/com.kylin.kysdk.SyncConfig.service` | Le champ decl ne mentionne pas les appels D-Bus systeme. Deux services root Type=dbus sans User= (com.kylin.kysdk.conf2, ExecStart=/usr/share/kysdk/kysdk-base/conf2-system.py ; com.kylin.kysdk.SyncConfig, ExecStart=/usr/bin/conf2-sync-system) acceptent send_destination depuis le contexte default. kysdk-conf2.service s'execute avant systemd-logind (Before=). | CONSTATÉ |
| cron / maintainer-script [ajout critique] | `archive:libkysdk-base/src/log/logrotate.cron ; debian/libkysdk-conf2.postinst ; debian/libkysdk-conf2-tools.postinst` | Le cron (00 0 * * * /usr/bin/logrotate -s /tmp/kylog-rotate-tmp ...) figure dans la fiche mais pas dans le recensement. Son installation est commentee (debian/libkysdk-log.install:4), il est donc non installe. Les deux postinst n'ont ni shebang ni #DEBHELPER#. libkysdk-conf2-tools.postinst cree le lien /usr/bin/kconf2-editor. | CONSTATÉ |

- [CONSTATÉ] Le patch 0001-.patch supprime notamment com.kylin.kysdk.conf2.limit/.limit.verify, conf2-server.py, conf2-session-server.py et les binaires calc_verify ; 0002-V11.patch modifie yamlhelper.cpp, conf2Utils.py, sync-dbus-server.c et kconf2-tool.c.
- [CONSTATÉ] La description du paquet mentionne '调试与埋点' (débogage et points de collecte).
- [PROBABLE] Aucun fichier d'activation D-Bus (system-services) n'est fourni pour com.kylin.kysdk.conf2 et SyncConfig : le démarrage passe uniquement par systemd.

### `kydoctor`

- **Nature** : reconditionnement-binaire — **Amont** : Kylin (Original-Maintainer liaoxianfu@kylinos.cn) ; composant 'pty' ; code non publié (forges ABSENT)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; debian/control porte des Depends résolus par architecture, ce qui indique des binaires produits hors de cet arbre
- **Langages / taille** : aucun dans l'arbre ; dépendances Qt5, libcurl, sqlcipher (C++/Qt — PROBABLE) — 19 fichiers, 28 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (le nom 'server' et la dépendance libsystemd0 suggèrent un service — PROBABLE)
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : dépendances libcurl4 et libqt5network5 : client réseau — PROBABLE
- **Blobs** : aucun dans l'arbre (binaires distribués hors source)
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible ; seuls des binaires installés par l'ISO pourraient être analysés, ce qui sort de la Phase 0.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:kydoctor/debian/control` | kydoctor-server défini 4 fois (loong64, arm64, riscv64, amd64), Depends libcurl4, libqt5network5, libqt5dbus5, libsqlcipher1, sqlcipher, sqlite3, libsystemd0, libdbus-1-3, libkf5windowsystem5 ; kydoctor-common (all) ; descriptions gabarits '<insert ...>' | CONSTATÉ |
| debian-rules | `archive:kydoctor/debian/rules` | 'dh $@' seulement ; scripts mainteneur uniquement en gabarits *.ex | CONSTATÉ |
| services / D-Bus / systemd des binaires | `archive:kydoctor/` | absents de l'arbre ; les points d'entrée réels de kydoctor-server ne sont pas observables | NON COUVERT |

- [CONSTATÉ] Les paquets kydoctor-common et kydoctor-server sont installés par défaut par l'ISO 3.0 (1.0.0.0-0k0.59).
- [NON COUVERT] La dépendance sqlcipher (base SQLite chiffrée) suggère un stockage local de diagnostics chiffré. [reclassé NON COUVERT : analogie sans indice dans un fichier]

### `kylin-os-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (format 3.0 quilt, mainteneur jishengjie@kylinos.cn ; 400 patches debian appliqués, .pc présent)
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète, arbre patché) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt5 (CMake), Python 3 (kyasDbus), Bash (core-dump), fichiers Qt .ts — 1 846 fichiers, 116 Mo (dont debian/patches ~100 Mo) ; ~82 k lignes de code hors .pc (plugins 41,5 k, daemons 15,3 k, kyasDbus 10,7 k, treasure-chest 9,2 k) — lignes de code propres mesurées : 78 583
- **Privilège** : root : kylin-os-manager-daemon, kylin-os-manager-driver-acquirer, kylin-core-dump-monitor.sh, kylin-assistant-systemdaemon.py ; utilisateur : application, session-service, komd-config, driver-acquirer (autostart), sessiondaemon Python
- **Déclencheurs** : boot : graphical.target / default.target (3 services root activés par postinst), activation D-Bus système (com.kylin-os-manager.startup, com.kylin.assistant.systemdaemon) et session, ouverture de session (3 autostart XDG), dpkg : postinst (restart des services, modification des bureaux de /home/*), polkit auth_admin pour l'action systemdaemon
- **Indices réseau** : URLs relevées : kom.kylinos.cn/d/ (driver-acquirer/webdriverinstallwidget.cpp), gitee.com/oauth/ (plugins/service-support/UI/uiproblemfeedback.cpp), wx.kylinos.cn/zsk, www.kylinos.cn, www.openkylin.top; outils réseau du 'treasure-chest' (tcpdump, telnet, arping, iperf, traceroute); outil d'assistance à distance kylin-os-manager-remote-user référencé par postinst ; aucune cible de build correspondante trouvée par grep hors .pc — PROBABLE binaire externe; champ listen vide
- **Blobs** : plugins/service-support/data/service_support_faq.db (SQLite, 1,66 Mo) installé par kylin-os-manager-data ; patches debian volumineux (0110-92.patch 18,4 Mo, 0197 14 Mo, 0181 12,6 Mo)
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant volumineux (~82 k lignes) avec 4 démons root (dont un Python), un polkit, des scripts root de gestion des core dumps, de l'assistance à distance et des remontées réseau ; il faut cibler les démons, kyasDbus, service-support et postinst.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-service (système) | `archive:kylin-os-manager/daemons/service/data/com.kylin-os-manager.service` | ExecStart=/usr/bin/kylin-os-manager-daemon ; User=root Group=root ; Requires/After=lightdm.service ; WantedBy=graphical.target ; installé lib/systemd/system ; activé et redémarré par postinst | CONSTATÉ |
| dbus-activation + policy (système) | `archive:kylin-os-manager/daemons/service/data/com.kylin-os-manager.conf` | root own com.kylin-os-manager ; context=default send_destination pour les interfaces tool.tool, systemmonitor.systemmonitor, driverinfo.tool, com.KylinOsManager.JunkClean, com.KylinOsManager.ToolBox ; com.kylin-os-manager.startup.conf idem pour com.KylinOsManager.Startup ; activation daemons/service/data/com.kylin.os.manager.service (Name=com.kylin-os-manager.startup, User=root, SystemdService=com.kylin-os-manager.service) | CONSTATÉ |
| systemd-service (système) | `archive:kylin-os-manager/daemons/kylin-os-manager-driver-acquirer/data/com.kylin-os-manager-driver-acquirer.service` | ExecStart=/usr/bin/kylin-os-manager-driver-acquirer ; User=root ; WantedBy=default.target ; installé etc/systemd/system ; policy .conf : default send_destination interface driveracquirer.tool ; le même binaire est aussi lancé en session via autostart kylin-os-manager-driver-acquirer.desktop | CONSTATÉ |
| systemd-service (système) | `archive:kylin-os-manager/daemons/kylin-core-dump-monitor/kylin-core-dump-monitor.service` | ExecStart=/usr/share/kylin-os-manager/kylin-core-dump-monitor/kylin-core-dump-monitor.sh ; User=root ; WantedBy=graphical.target ; activé par postinst ; scripts kylin-core-dump-handler.sh (197 l.) et kylin-core-dump-check.sh installés ; kylin-core-dump-trailblazer.service commenté dans debian/kylin-os-manager.install | CONSTATÉ |
| dbus-activation + policy + polkit (système, Python) | `archive:kylin-os-manager/kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.service` | Name=com.kylin.assistant.systemdaemon ; Exec=/usr/bin/kylin-assistant-systemdaemon.py (lien vers /usr/lib/python3/dist-packages/kylin-assistant-daemon/systemdaemon/kasystemdbusmain.py, debian/kylin-os-manager.links) ; User=root ; policy : default send_destination interface com.kylin.assistant.systemdaemon ; polkit com.kylin.assistant.systemdaemon.policy action com.kylin.assistant.systemdaemon.action allow_any/inactive/active=auth_admin | CONSTATÉ |
| dbus-activation (session, Python) | `archive:kylin-os-manager/kyasDbus/sessiondaemon/conf/com.kylin.assistant.sessiondaemon.service` | Name=com.kylin.assistant.sessiondaemon ; Exec=/usr/bin/kylin-assistant-sessiondaemon.py ; /usr/share/dbus-1/services | CONSTATÉ |
| autostart XDG (session) | `archive:kylin-os-manager/daemons/sessionservice/sessionservice-config.desktop` | Exec=/usr/lib/kylin-os-manager/bin/kylin-os-manager-session-service ; X-UKUI-AutoRestart ; aussi daemons/config/komd-config.desktop (Exec=/usr/lib/kylin-os-manager/bin/komd-config) et kylin-os-manager-driver-acquirer.desktop -> /etc/xdg/autostart | CONSTATÉ |
| dbus limit | `archive:kylin-os-manager/daemons/service/data/com.kylin-os-manager.limit` | whitelist de 16 chemins dont /opt/KylinPvt/bin/..., /usr/bin/hedron-client, /usr/bin/launchSysDbus et key16=/home/kylin/Gyingjian.py ; com.kylin.assistant.systemdaemon.limit : 15 chemins dont key15=/home/kylin/Gyingjian.py et [envwhitelist] env1=LD_LIBRARY_PATH ; protégés par kysec_set readonly dans postinst | CONSTATÉ |
| maintainer-script postinst | `archive:kylin-os-manager/debian/kylin-os-manager.postinst` | chmod 755 des 2 mains Python ; ldconfig ; glib-compile-schemas ; kysec_set ; systemctl enable/restart com.kylin-os-manager, driver-acquirer, kylin-core-dump-monitor ; ln -sf /usr/lib/kylin-os-manager/bin/kylin-os-manager-remote-user -> /usr/share/kylin-os-manager/komt-rcu/komt-rcu ('远程协助被控工具') ; mv /usr/share/kylin-os-manager/kylin-service-support -> /usr/bin ; boucle sur /home/*/ (Desktop/桌面) : rm, cp et chown de raccourcis .desktop | CONSTATÉ |
| maintainer-script postrm | `archive:kylin-os-manager/debian/kylin-os-manager.postrm` | rm /usr/bin/kylin-service-support | CONSTATÉ |
| ld.so.conf.d | `archive:kylin-os-manager/data/config/kylin-os-manager.conf` | installé /etc/ld.so.conf.d (CMakeLists.txt) : /usr/libs/kysdk/applications/, /usr/lib/kysdk/kysdk-base/, /usr/lib/kysdk/kysdk-system/, /usr/lib/kysdk/applications/, /usr/lib/kylin-os-manager/common/ | CONSTATÉ |
| application / outils | `archive:kylin-os-manager/debian/kylin-os-manager.install` | usr/lib/kylin-os-manager/bin/* (motif global) ; plugins/*.so ; usr/share/kylin-os-manager/script/*.py ; kyasDbus/* -> dist-packages ; service-support-mini lié dans /opt/kylin-os-manager/tools ; data/kylin-service-support (script bash 320 o) | CONSTATÉ |
| debian-rules / build | `archive:kylin-os-manager/CMakeLists.txt` | debian/rules 'dh $@ --parallel' ; sous-répertoires 3rdparty, kom, src, plugins, daemons, treasure-chest (wrappers tcpdump, telnet, arping, iperf, traceroute, file-smash, source-update, system-cleanup) ; CXX_FLAGS -Wl,-z,now | CONSTATÉ |

- [CONSTATÉ] L'arbre est l'état patché (401 entrées dans .pc pour 400 patches de debian/patches/series) ; les patches font au total environ 100 Mo.
- [CONSTATÉ] Les whitelists D-Bus contiennent un chemin dans un répertoire personnel (/home/kylin/Gyingjian.py) et des chemins /opt/KylinPvt/bin non fournis par ce paquet.
- [CONSTATÉ] Le fichier ld.so.conf.d ajoute un chemin non standard /usr/libs/kysdk/applications/.
- [NON COUVERT] Le contenu des scripts root kylin-core-dump-*.sh et des démons Python n'a pas été lu (hors périmètre de la Phase 0).

### `kylin-driver-manager-service`

- **Nature** : absent-ou-vide — **Amont** : Kylin / Ubuntu Kylin (Original-Maintainer fanyuchen@kylinos.cn ; Homepage https://github.com/ubuntukylin/kylin-driver-manager-service) ; composant 'pty'
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; forges ABSENT selon la fiche (le Homepage GitHub n'a pas été vérifié)
- **Langages / taille** : aucun dans l'arbre ; dépendances Qt5/GLib/libusb/libudev (C++/Qt — PROBABLE) — 19 fichiers, 28,5 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (dépendance dmidecode et le terme 'service' suggèrent un service root — PROBABLE)
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : dépendance libqt5network5 — PROBABLE
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible dans l'archive ; paquet non installé par défaut par l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:kylin-driver-manager-service/debian/control` | paquet défini pour amd64, riscv64, loong64, arm64 ; Depends dmidecode, libusb-1.0-0, libudev1, libqt5dbus5, libqt5network5, libglib2.0-0t64 ; Description 'Service for kylin-os-manager ... get device and driver info' | CONSTATÉ |
| debian-rules | `archive:kylin-driver-manager-service/debian/rules` | 'dh $@' ; scripts mainteneur en gabarits *.ex seulement | CONSTATÉ |
| service / D-Bus | `archive:kylin-driver-manager-service/` | aucune unité ni politique dans l'arbre ; points d'entrée réels inconnus | NON COUVERT |

- [CONSTATÉ] Le paquet n'est pas dans installe_par_defaut_iso ; changelog 'Initial Release' 25/09/2024 signé 'unknown <mao@unknown>'.
- [NON COUVERT] Le Homepage GitHub ubuntukylin pourrait héberger un code source public ; cette piste n'a pas été vérifiée.

### `kylin-sso-client`

- **Nature** : absent-ou-vide — **Amont** : Ubuntu Kylin Team (Original-Maintainer) ; Homepage https://www.kylinos.cn ; code non publié
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; forges ABSENT
- **Langages / taille** : aucun dans l'arbre — 19 fichiers, 26,5 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : fonctions déclarées de connexion unifiée et de synchronisation cloud : client réseau — PROBABLE
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source alors que le binaire kylin-id (identité, synchronisation cloud) est installé par l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:kylin-sso-client/debian/control` | paquets libsync-plugin ('plugins of kylin-cloud-sync'), kylin-sso-client ('UI of cloud-sync'), kylin-id ('union login dialog of kylin-id'), kylin-cloud-sync ('executor of cloud-sync') ; Depends génériques ${shlibs:Depends} | CONSTATÉ |
| debian-rules | `archive:kylin-sso-client/debian/rules` | 'dh $@' ; gabarits *.ex seulement | CONSTATÉ |
| services / D-Bus / identifiants | `archive:kylin-sso-client/` | absents de l'arbre ; mécanismes d'authentification et de synchronisation cloud non observables | NON COUVERT |

- [CONSTATÉ] Le binaire kylin-id=2.2.11.2-0k1.18 est installé par défaut par l'ISO 3.0.
- [CONSTATÉ] Les whitelists D-Bus de kylin-os-manager (com.kylin.assistant.systemdaemon.limit) listent /usr/bin/kylin-id et /usr/bin/kylinid-dbus.

### `kylin-mfa`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Original-Maintainer 付焕章 <fuhuanzhang@unknown>) ; code non publié
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; forges ABSENT
- **Langages / taille** : aucun dans l'arbre — 19 fichiers, 25,7 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (une MFA Linux s'intègre typiquement à PAM, donc en root — PROBABLE)
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : aucun observable
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; paquet non installé par défaut par l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:kylin-mfa/debian/control` | Package kylin-mfa, Section admin ; Description 'multi-factor authentication tool for Linux ... password and a second factor, such as a smart card or a one-time password token' | CONSTATÉ |
| debian-rules | `archive:kylin-mfa/debian/rules` | 'dh $@' ; gabarits *.ex seulement | CONSTATÉ |
| PAM / polkit / services | `archive:kylin-mfa/` | aucun module PAM ni configuration d'authentification dans l'arbre | NON COUVERT |

- [CONSTATÉ] Changelog 1.0.0.0-ok1.39 'Initial Release' daté du 31/07/2026 (huanghe).

### `openkylin-token-center`

- **Nature** : absent-ou-vide — **Amont** : openKylin Token Team (Original-Maintainer token-team@openkylin.top) ; code non publié
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; forges ABSENT
- **Langages / taille** : aucun dans l'arbre (Description : client Qt 6 natif) — 19 fichiers, 26,4 Ko ; 0 fichier de code — lignes de code propres mesurées : 0
- **Privilège** : NON COUVERT (client de bureau, a priori utilisateur — PROBABLE)
- **Déclencheurs** : NON COUVERT
- **Indices réseau** : connexion déclarée aux API openKylinID et Token (debian/control)
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source pour un client d'identité et de vérification d'installation installé par l'ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:openkylin-token-center/debian/control` | Description 'Native Qt 6 desktop client for openKylinID authentication, installation verification, Token applications, rewards, credentials and account activity. It connects to the published openKylinID and Token service APIs.' ; Conflicts/Replaces openkylin-token-checkin, openkylin-token-installer | CONSTATÉ |
| debian-rules | `archive:openkylin-token-center/debian/rules` | 'dh $@' ; gabarits *.ex seulement | CONSTATÉ |
| autostart / services | `archive:openkylin-token-center/` | absents de l'arbre | NON COUVERT |

- [CONSTATÉ] L'ISO installe openkylin-token-center=1.0.0-ok1.7 alors que l'archive contient 1.0.0-ok1.9 (changelog 18/09/2026, date de l'ISO).
- [CONSTATÉ] [correction critique] Écart de version : archive 1.0.0-ok1.9, ISO 1.0.0-ok1.7 (retirée du pool, HTTP 404).

### `kylin-wget`

- **Nature** : propre-kylin — **Amont** : aucun code amont : réécriture Rust revendiquant la compatibilité GNU Wget (numéro de version 1.21.4 repris de GNU Wget) ; crate 'wget-rust' 0.1.0
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète Rust ; format 3.0 quilt sans série de patches, .pc/applied-patches vide) ; forges ABSENT
- **Langages / taille** : Rust (42 fichiers .rs, ~10,5 k lignes) ; Python (tests, 174 Ko) ; Shell — 85 fichiers, 787 Ko ; Cargo.lock = 259 crates, sans répertoire vendor — lignes de code propres mesurées : 15 232
- **Privilège** : privilège de l'appelant (utilisateur, ou root lorsque /usr/bin/wget est appelé par des scripts système)
- **Déclencheurs** : toute invocation de /usr/bin/wget (diversion du wget GNU), dpkg preinst/postrm (diversion)
- **Indices réseau** : client HTTP/HTTPS/FTP (reqwest/rustls, ftp.rs); build : récupération des crates depuis mirrors.aliyun.com avec check-revoke=false
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Remplace /usr/bin/wget pour tout le système (chemin de téléchargement utilisé par des scripts d'installation) et implémente la validation TLS/certificats ; ~10,5 k lignes Rust propres, plus 259 crates à recenser.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script preinst | `archive:kylin-wget/debian/preinst` | install\|upgrade\|reinstall : dpkg-divert --add [--rename] --package kylin-wget /usr/bin/wget (\|\| true) | CONSTATÉ |
| maintainer-script postrm | `archive:kylin-wget/debian/postrm` | remove\|purge : dpkg-divert --remove --package kylin-wget /usr/bin/wget ; prerm et postinst = exit 0 | CONSTATÉ |
| debian-rules | `archive:kylin-wget/debian/rules` | override_dh_auto_build : cd rust && CARGO_HOME=debian/cargo-home cargo build --release --locked ; override_dh_auto_install : install -m 0755 rust/target/release/wget -> /usr/bin/wget | CONSTATÉ |
| debian-control | `archive:kylin-wget/debian/control` | Provides: wget ; Conflicts: wget ; Replaces: wget ; Depends ca-certificates, dpkg ; Build-Depends cargo, rustc | CONSTATÉ |
| configuration Cargo (chaîne de build) | `archive:kylin-wget/.cargo/config.toml` | [source.crates-io] replace-with='aliyun' -> sparse+https://mirrors.aliyun.com/crates.io-index/ ; [http] check-revoke=false ; [net] git-fetch-with-cli=true | CONSTATÉ |
| cible de build | `archive:kylin-wget/rust/Cargo.toml` | [[bin]] name='wget' ; reqwest 0.12.4 (rustls-tls, http2, cookies), rustls 0.22.4 (ring), tokio-rustls 0.25, x509-parser 0.16, cookie_store, publicsuffix ; modules rust/src/tls_ca.rs, tls_peer.rs, cert_check.rs, hsts.rs, netrc.rs, ftp.rs, wgetrc.rs | CONSTATÉ |
| serveurs de test (non installés) | `archive:kylin-wget/tests/server.py` | socketserver (tests/server.py, tests/proxy.py) et .listen( (tests/ftp_server.py) ; non installés par debian/rules (seuls wget et README.md) | CONSTATÉ |

- [CONSTATÉ] Le paquet est installé par défaut par l'ISO (kylin-wget=1.21.4-ok4) et dévie le binaire /usr/bin/wget.
- [NON COUVERT] Les documents de conception en chinois (重写计划书.md, docs/测试方案.md) décrivent la réécriture ; non lus.
- [CONSTATÉ] meta.origine_kylin=1 et orig_vs_debian_ubuntu='amont-inconnu' ; aucun lien de code avec GNU Wget n'a été constaté (pas de C).

### `kylin-bot`

- **Nature** : fork-amont — **Amont** : ZeroClaw (https://github.com/zeroclaw-labs/zeroclaw) renommé en KylinBot : scripts/rename-zeroclaw-to-kylinbot.sh, crate crates/zeroclaw-skillhub (repository zeroclaw-labs/zeroclaw), authors=['theonlyhennygod'] dans Cargo.toml ; version de base workspace 0.7.5 — PROBABLE
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 1 629 fichiers, format 3.0 native) ; forges ABSENT
- **Langages / taille** : Rust (~338 k lignes dans src/ et crates/ hors firmware), TypeScript/JavaScript (web, apps/tauri), Python, Shell — 1 629 fichiers, 58 Mo ; Cargo.lock = 1 244 crates, sans vendor ; 17 crates de workspace — lignes de code propres mesurées : 424 828
- **Privilège** : utilisateur de session (démon lancé par autostart et par postinst via su pour chaque session active)
- **Déclencheurs** : ouverture de session (autostart XDG, redémarrage automatique UKUI), dpkg postinst : démarrage immédiat dans chaque session graphique active, planificateur cron interne ([cron] enabled=true), canaux de messagerie/gateway (socket unix ~/.kylinbot/gateway.sock)
- **Indices réseau** : gateway : socket unix par défaut, TCP 127.0.0.1:42617 désactivé (listen_tcp=false); fournisseurs LLM / transcription (api.groq.com, api.firecrawl.dev, OpenAI) présents dans la configuration, désactivés par défaut; http_request/web_fetch/browser activés avec allowed_domains=['*']; build : 1 244 crates via le miroir mirrors.tuna.tsinghua.edu.cn; indices .listen(/http.server dans 11 fichiers (dont firmware et tests)
- **Blobs** : aucun blob ELF (blob_count=0) ; gros fichiers médias (voice_logo.gif 5,2 Mo x2, PNG/SVG 2,4 Mo) et docs/book/mermaid.min.js 2,7 Mo
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Fork très volumineux (~338 k lignes Rust, 1 244 crates) d'un agent IA de session qui, selon la configuration empaquetée, exécute des commandes sans restriction et accède au réseau ; il faut cibler le delta Kylin, les outils shell/fichiers, la gateway, les canaux et la configuration.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart XDG (session) | `archive:kylin-bot/debian/kylin-bot-autostart.desktop` | installé /etc/xdg/autostart (debian/rules) ; Exec=/usr/bin/kylin-bot daemon ; X-GNOME-Autostart-enabled=true ; X-UKUI-AutoRestart=true ; commentaire 'gateway, channels and scheduler' | CONSTATÉ |
| maintainer-script postinst | `archive:kylin-bot/debian/postinst` | configure : pour chaque UID de 'loginctl list-sessions' (hors root) ayant /run/user/UID, su - user -c 'nohup /usr/bin/kylin-bot daemon &' avec DISPLAY, XDG_RUNTIME_DIR, DBUS_SESSION_BUS_ADDRESS | CONSTATÉ |
| maintainer-script prerm | `archive:kylin-bot/debian/prerm` | remove\|upgrade\|deconfigure : pkill -f '/usr/bin/kylin-bot daemon' | CONSTATÉ |
| debian-rules | `archive:kylin-bot/debian/rules` | cp debian/cargo-config.toml -> CARGO_HOME ; cargo build --release --locked ; install target/release/kylinbot -> /usr/bin/kylin-bot ; cp -a debian/kylinbot/. -> /usr/share/kylinbot ; dh_shlibdeps sauté ; dh_auto_test neutralisé | CONSTATÉ |
| configuration Cargo (chaîne de build) | `archive:kylin-bot/debian/cargo-config.toml` | [source.crates-io] replace-with='mirror' -> sparse+https://mirrors.tuna.tsinghua.edu.cn/crates.io-index/ ; rustflags -C embed-bitcode=no | CONSTATÉ |
| configuration par défaut empaquetée | `archive:kylin-bot/debian/kylinbot/.kylinbot/config.toml` | -> /usr/share/kylinbot/.kylinbot/config.toml ; [autonomy] level='full', workspace_only=false, allowed_commands=['*'], block_high_risk_commands=false, allowed_roots=['~'] ; [gateway] host=127.0.0.1 port=42617 listen_tcp=false unix_socket='~/.kylinbot/gateway.sock' require_pairing=true allow_public_bind=false ; [browser]/[http_request]/[web_fetch] enabled=true allowed_domains=['*'] ; [skills] allow_scripts=true ; [cron] enabled=true | CONSTATÉ |
| skills empaquetées | `archive:kylin-bot/debian/kylinbot/.kylinbot/workspace/skills` | 12 répertoires de skills (audio, bluetooth, keyboard, mouse, network-manager, power, panel...) ; SOUL.md et IDENTITY.md -> /usr/share/kylinbot | CONSTATÉ |
| systemd-service (non empaqueté) | `archive:kylin-bot/scripts/kylinbot.service` | User=pi ; ExecStart=/home/pi/kylinbot/kylinbot gateway --host 0.0.0.0 --port 8080 ; EnvironmentFile=/home/pi/kylinbot/.env ; non installé par debian/rules | CONSTATÉ |
| règle udev (non empaquetée) | `archive:kylin-bot/scripts/99-act-led.rules` | SUBSYSTEM=='leds', KERNEL=='ACT' : RUN+= chgrp gpio / chmod g+w sur brightness et trigger ; non installée par debian/rules | CONSTATÉ |
| écoutes réseau (code) | `archive:kylin-bot/crates/kylinbot-channels/src/orchestrator/mod.rs` | indices '.listen(' dans crates/kylinbot-channels (acp_channel.rs, orchestrator/mod.rs), crates/kylinbot-tools (escalate.rs, ask_user.rs), 'http.server' dans crates/kylinbot-config/src/policy.rs ; non confirmés par lecture | PROBABLE |
| systemd-unit + udev (non installes) [ajout critique] | `archive:kylin-bot/scripts/kylinbot.service ; archive:kylin-bot/scripts/99-act-led.rules` | Presents dans la fiche, absents du recensement. kylinbot.service : User=pi, ExecStart=/home/pi/kylinbot/kylinbot gateway --host 0.0.0.0 --port 8080, WantedBy=multi-user.target. 99-act-led.rules : RUN+=chgrp gpio / chmod g+w sur /sys/.../leds/ACT. Aucun des deux n'est installe par debian/rules (override_dh_auto_install n'installe que le binaire, /usr/share/kylinbot et l'autostart). | CONSTATÉ |

- [CONSTATÉ] Deux fichiers JSON cron-<uuid> (62 Ko chacun) à la racine contiennent un historique de conversation ({'version':1,'history':[{'role':'system',...}]}) ; artefacts d'exécution non installés.
- [CONSTATÉ] L'arbre contient des répertoires d'outillage d'assistants IA (.claude/skills, .gemini, AGENTS.md, CLAUDE.md) ainsi que deploy-k8s, Dockerfiles et firmware ESP32/Pico non utilisés par debian/rules.
- [CONSTATÉ] NOTICE et Cargo.toml désignent https://github.com/kylinbot/kylinbot comme 'seul dépôt officiel' (URL réécrite par le script de renommage).
- [NON COUVERT] La version et le commit exacts de ZeroClaw ayant servi de base ne sont pas établis ; le delta Kylin n'est pas isolé.
- [CONSTATÉ] [correction critique] Amont non nommé ; debian/copyright déclare « Upstream-Name: kylin-aiassistant », incohérent avec le contenu (Rust/TypeScript) ; amont à établir depuis Cargo.toml en Phase 1.

### `kylin-botshell`

- **Nature** : propre-kylin — **Amont** : Kylin (dépôt Gitee kylin-botshell, branche main, commit f546651)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (aucun paquet source dans l'archive ; pas de répertoire debian/)
- **Langages / taille** : Markdown (SKILL.md), Shell (3 scripts) — 13 fichiers, 34 Ko ; 3 scripts shell (kom.sh, screenshot.sh, system-monitor.sh) — lignes de code propres mesurées : 265
- **Privilège** : privilège de l'agent appelant (session utilisateur) ; aucun sudo/pkexec relevé par grep
- **Déclencheurs** : invocation par un agent IA (skills)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petit corpus de skills (3 scripts shell et 7 SKILL.md) sans empaquetage ni service ; il suffit de tout lire rapidement.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| skills d'agent | `gitee:kylin-botshell/skills/system-control/kylin-os-manager-control/scripts/kom.sh` | script appelé par un agent pour piloter kylin-os-manager ; aussi screenshot.sh et system-monitor.sh ; les SKILL.md décrivent des appels dbus-send --session / qdbus (MPRIS org.mpris.MediaPlayer2.KylinMusic) | CONSTATÉ |
| empaquetage | `gitee:kylin-botshell/` | aucun fichier debian/, unité systemd, D-Bus, polkit ou autostart ; cli/ et docs/ ne contiennent que .gitkeep | CONSTATÉ |

- [CONSTATÉ] Ce dépôt n'est ni dans l'archive ni dans la liste ISO ; les branches Gitee develop-for-v11-2603, oemccai/v11-2603-preview, etc. n'ont pas été examinées (seule main est extraite).

### `platform-security-patrol`

- **Nature** : propre-kylin — **Amont** : Kylin / openKylin Platform Security Team (changelog signé suchangzheng@kylinos.cn ; Homepage https://gitee.com/openkylin/platform-security-patrol) ; cJSON incorporé (common/cJSON.c)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) + gitee (identique : diff -rq ne montre que .git et .gitignore ; HEAD df8d42d = changelog 0.1.4)
- **Langages / taille** : C (CMake), JSON (définitions de capacités), Shell — 72 fichiers (archive) / 73 (gitee), 596 Ko ; 48 fichiers de code, ~16,6 k lignes dont cJSON.c 80 Ko — lignes de code propres mesurées : 16 620
- **Privilège** : utilisateur appelant ; documentation orientée sudo pour l'installation de 'hag' ; privilège requis par les collecteurs firmware/CPU non établi
- **Déclencheurs** : invocation manuelle de plat-sec-patrol, chargement dlopen des plugins de /usr/lib/plat-sec-patrol/plugins
- **Indices réseau** : aucune écoute ; URL de téléchargement d'un binaire externe 'hag' (download.sourcefind.cn:65024) dans le JSON de capacités, le README et scripts/hygon/install_hag.sh
- **Blobs** : aucun
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Outil CLI propre (~13,5 k lignes hors cJSON) non installé par défaut, sans service, mais comportant un exécuteur de commandes (src/core/executor.c), un chargement de plugins et un guide de téléchargement de binaire tiers.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:platform-security-patrol/debian/rules` | hardening=+all ; cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=OFF ; crée /var/log/plat-sec-patrol (755) ; vérifie la présence des plugins environment/ et capability/ dans /usr/lib/plat-sec-patrol/plugins ; dh_makeshlibs exclut les plugins dlopen | CONSTATÉ |
| maintainer-scripts | `archive:platform-security-patrol/debian/postinst` | postinst : chmod 755 /var/log/plat-sec-patrol ; prerm : ldconfig ; postrm purge : rm -rf /var/log/plat-sec-patrol | CONSTATÉ |
| cibles de build | `archive:platform-security-patrol/CMakeLists.txt` | add_executable(plat-sec-patrol) -> bin ; add_library(plat_common SHARED) ; plugins cpu_collector, firmware_collector, os_collector, capability/hygon ; en-têtes include/ installés | CONSTATÉ |
| définitions de capacités (commandes) | `archive:platform-security-patrol/plugins/capability/hygon/hygon_capabilities.json` | outil 'hag' : check_command='hag' ; install_guide='sudo wget -c https://download.sourcefind.cn:65024/directlink/8/c86-devkit/previous-release/bin/hag -O /usr/local/bin/hag && sudo chmod +x ...' | CONSTATÉ |
| script d'installation (non empaqueté) | `archive:platform-security-patrol/scripts/hygon/install_hag.sh` | l.14 : wget -c https://download.sourcefind.cn:65024/.../bin/hag -O $INSTALL_PATH/bin/hag ; non installé par CMake | CONSTATÉ |
| service / D-Bus / polkit | `archive:platform-security-patrol/` | aucun : outil en ligne de commande uniquement | CONSTATÉ |

- [CONSTATÉ] meta.origine_kylin=0 alors que le changelog est signé @kylinos.cn et que Homepage pointe vers gitee.com/openkylin ; format source 1.0 (pas de debian/source/format).
- [CONSTATÉ] Les arbres archive et Gitee sont identiques hors .git et .gitignore.

### `kylin-device-daemon`

- **Nature** : propre-kylin — **Amont** : Kylin / UKUI (Vcs https://github.com/ukui/kylin-device-daemon ; debian/watch vers les releases GitHub) ; QtSingleApplication incorporé (3rd-parties/)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 99 fichiers) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++/Qt5 (CMake), UDisks2/GLib — 99 fichiers, 1,17 Mo ; 65 fichiers de code, ~15,2 k lignes (mainwindow.cpp 198 Ko) — lignes de code propres mesurées : 15 238
- **Privilège** : utilisateur (session graphique) ; opérations de montage et de réparation déléguées à UDisks2
- **Déclencheurs** : ouverture de session UKUI (autostart), insertion de périphériques amovibles (événements UDisks2)
- **Indices réseau** : Qt5::Network lié ; QLocalServer d'instance unique ; aucune écoute TCP relevée
- **Blobs** : aucun ; debian/kylin-device-daemon-coverage.xml 307 Ko (rapport de couverture)
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Composant de session de taille moyenne (~15 k lignes) qui traite des supports amovibles non fiables via UDisks2 et lance des processus externes.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart XDG (session) | `archive:kylin-device-daemon/data/kylin-device-daemon.desktop` | installé /etc/xdg/autostart (CMakeLists.txt) ; Exec=kylin-device-daemon ; X-UKUI-AutoRestart=true ; OnlyShowIn=UKUI ; NoDisplay=true ; 'auto-mount tool' | CONSTATÉ |
| cible de build / install | `archive:kylin-device-daemon/CMakeLists.txt` | add_executable(kylin-device-daemon) -> bin ; Qt5 Widgets DBus Network X11Extras ; /etc/kylin-device-daemon/kylin-device-daemon.ini ; kylin-device-daemon.lock -> /usr/share/ukui/ukui-panel ; gschema org.kylin-device-daemon.autoload ; cible test-repair-dialog (non installée) | CONSTATÉ |
| debian-rules | `archive:kylin-device-daemon/debian/rules` | dh --buildsystem cmake ; hardening=+all ; -D_FORTIFY_SOURCE=2 ; dh_shlibdeps --ignore-missing-info ; aucun script mainteneur | CONSTATÉ |
| IPC D-Bus sortant | `archive:kylin-device-daemon/src/` | destinations relevées par grep : org.freedesktop.UDisks2 (Block, Filesystem), com.kylin.burner.manager, org.freedesktop.Notifications, com.ukuiflashdisk.hotel ; QProcess utilisé dans datacdrom.cpp, qclickwidget.cpp, fdclickwidget.cpp, mainwindow.cpp | CONSTATÉ |
| socket locale (instance unique) | `archive:kylin-device-daemon/3rd-parties/QtSingleApplication/qtlocalpeer.cpp` | QLocalServer de QtSingleApplication (indice listen) ; non lu | PROBABLE |

- [CONSTATÉ] debian/ukui-panel/ contient un résidu d'arbre de paquet construit (DEBIAN/control ukui-panel 1.0.0-24.5 amd64, DEBIAN/conffiles listant /etc/xdg/autostart/kylin-device-daemon.desktop, un schéma gsettings).
- [NON COUVERT] Arbre Gitee/GitHub (DIVERGENT) non acquis.

### `kylin-connectivity`

- **Nature** : propre-kylin — **Amont** : Kylin (format 3.0 quilt, mainteneur huheng@kylinos.cn) ; code tiers incorporé : dsoftbus OpenHarmony/openEuler (3rdparty/dsoftbus/dsoftbus_build ; sync.sh mentionne openeuler/src/dsoftbus_standard) avec mbedtls, libcoap, cJSON ; fineftp-server + asio (3rdparty/ftpserver) ; 'simple' (tokenizer SQLite FTS5) + cppjieba + sqlite3 (3rdparty/plugins-simple) ; scrcpy-server.jar ; sndcpy
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 66 patches appliqués, .pc présent) + 7 binaires précompilés ; Gitee PRESENT (branches upstream, upstream-softbus, kaiming) non acquis ; GitHub EMPTY
- **Langages / taille** : C++/Qt5 (CMake) pour le code propre ; C/C++ pour le code tiers (dsoftbus, mbedtls, libcoap, asio, sqlite) ; Shell, Python, Perl — 3 981 fichiers, 377 Mo ; ~867 k lignes hors .pc, dont code propre ~41 k (projection 11,8 k, ui 7,7 k, connectmanage 6,6 k, filetransfer 4,4 k) et code tiers ~826 k (dsoftbus 376 k, plugins-simple 296 k, ftpserver 155 k) — lignes de code propres mesurées : 41 305
- **Privilège** : root (kylin-connectivity-manager, activation D-Bus système) ; utilisateur (application, serveurs TCP, outils anti-control et screencast) ; privilège de connectivity_softbus_server non établi
- **Déclencheurs** : lancement de l'application (desktop, URL %u), activation D-Bus système com.kylin.connectivity.manager, connexions réseau entrantes sur 27188/27181/27192, dpkg postinst (liens symboliques dans /usr/bin et /lib)
- **Indices réseau** : TCP Any:27188, Any:27181, AnyIPv4:27192 (confirmés); socket /tmp/anti_control_server.socket; dsoftbus (découverte CoAP, sockets TCP/UDP, netlink); serveur FTP fineftp ; client FTP qftp avec listener actif; sndcpy : adb + port 27182 ; scrcpy-server.jar (Android)
- **Blobs** : 7 blobs (200,8 Mo) : 3rdparty/tools/arm/kylin-screencast.AppImage (88,5 Mo ELF), 3rdparty/tools/amd/kylin-screencast.AppImage (83,3 Mo ELF), amd/anti-control-server (12,6 Mo ELF), amd/anti-control-client (10,3 Mo ELF), arm/anti-control-server (3,3 Mo ELF), arm/anti-control-client (2,7 Mo ELF), 3rdparty/scrcpy-server.jar (40 Ko ZIP) ; tous installés dans /opt/connectivity/tools ou /usr/share/kylin-connectivity
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant très volumineux (41 k lignes propres et 826 k lignes de code tiers) qui écoute en TCP sur toutes les interfaces, expose un service D-Bus root et installe 6 exécutables ELF précompilés non auditables sans analyse binaire.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-activation (système) | `archive:kylin-connectivity/kylinconnectivitymanager/com.kylin.connectivity.manager.service` | Name=com.kylin.connectivity.manager ; Exec=/usr/bin/kylin-connectivity-manager ; User=root ; installé /usr/share/dbus-1/system-services (kylinconnectivitymanager/CMakeLists.txt l.29) | CONSTATÉ |
| dbus-policy + kylin-config | `archive:kylin-connectivity/kylinconnectivitymanager/com.kylin.connectivity.manager.conf` | installé /etc/dbus-1/system.d ; root own ; default send_destination interface com.kylin.connectivity.manager + Introspectable + Properties ; com.kylin.connectivity.manager.yaml -> /etc/kylin-config/basic (whitelist /usr/bin/kylin-connectivity) ; fichiers .limit/.limit.verify présents mais absents des install() relevés | CONSTATÉ |
| maintainer-script postinst | `archive:kylin-connectivity/debian/postinst` | ln -sf vers /usr/bin de /opt/connectivity/tools/kylin-screencast.AppImage, anti-control-server, anti-control-client, /opt/connectivity/connectivity_softbus_server et _client ; création si absent du lien /lib/$(uname -i)-linux-gnu/libz.so -> libz.so.1 | CONSTATÉ |
| installation de binaires / bibliothèques | `archive:kylin-connectivity/CMakeLists.txt` | l.113-124 : output/lib/dsoftbus/ -> /opt/connectivity/dsoftbus/ ; connectivity_softbus_server/client -> /opt/connectivity/ ; 3rdparty/tools/{arm,amd}/ -> /opt/connectivity/tools (USE_SOURCE_PERMISSIONS) ; libsimple.so -> /usr/lib/ ; sndcpy (script bash) et scrcpy-server.jar -> /usr/share/kylin-connectivity/ ; config/phoneServerConfig.ini -> $ENV{HOME}/.config/kylin-connectivity/ (HOME de l'environnement de build) | CONSTATÉ |
| écoutes TCP | `archive:kylin-connectivity/connectmanage/connectionservice.cpp` | l.34 listen(QHostAddress::Any, 27188) ; l.70 listen(QHostAddress::Any, 27181) (heartbeat) ; pcscreenmanage/pcscreenmanage.cpp l.20 listen(QHostAddress::AnyIPv4, 27192) ; projection/device/usbdevice/usbserver.cpp l.388 listen(LocalHost, port) | CONSTATÉ |
| socket locale | `archive:kylin-connectivity/pcscreenmanage/control/control.cpp` | l.20 QLocalServer::listen('/tmp/anti_control_server.socket') | CONSTATÉ |
| serveurs tiers | `archive:kylin-connectivity/3rdparty/ftpserver/fineftp-server/src/server_impl.cpp` | serveur FTP asio (acceptor.listen l.74) ; dsoftbus : bind TCP/UDP/CoAP/netlink (nstackx, softbus_tcp_socket.c, coap_app.c) ; connectivity_softbus_server construit depuis 3rdparty/dsoftbus/server.cpp ; usage effectif dans l'application non confirmé | PROBABLE |
| desktop | `archive:kylin-connectivity/kylin-connectivity.desktop` | Exec=/usr/bin/kylin-connectivity %u ; X-KDE-Wayland-Interfaces inclut org_kde_kwin_fake_input et zkde_screencast_unstable_v1 | CONSTATÉ |
| debian-rules | `archive:kylin-connectivity/debian/rules` | dh --parallel ; override_dh_strip contient seulement '--exclude=item' et override_dh_shlibdeps seulement '--dpkg-shlibdeps-params=–ignore-missing-info' (sans commande dh_*) : dh_strip et dh_shlibdeps ne sont vraisemblablement pas exécutés | PROBABLE |

- [CONSTATÉ] Paquet non installé par défaut par l'ISO 3.0.
- [CONSTATÉ] debian/patches/0004-87.patch fait 75,9 Mo ; 3rdparty/dsoftbus/dsoftbus_build contient build.log, cp_release.sh et sync.sh (rsync vers un hôte 172.17.146.14).
- [CONSTATÉ] Les binaires anti-control-* et kylin-screencast.AppImage n'ont aucun source dans l'arbre ; leur audit relève d'une analyse binaire (X).
- [CONSTATÉ] task.sh à la racine duplique les liens symboliques du postinst.

#### Constats transverses du lot B07-gel-telemetrie

- **[CONSTATÉ]** 12 noms de bus D-Bus système du lot sont servis par des processus root (User=root dans le fichier d'activation, ou unité systemd sans User=), et leur politique D-Bus autorise send_destination au contexte default. — *preuve* : kylin-process-manager/daemon/configs/com.kylin.ProcessManagerDaemon.service et daemon1/configs/com.kylin.ProcessResourceManagerDaemon.service (User=root) ; ukui-input-gather/.../org.ukui.libinput.proxy.service (User=root) ; ukui-service-manager/data/system/org.ukui.serviceManager.service (User=root) ; kylin-daq/data/com.kylin.daq.service (User=root) ; libkysdk-base/src/conf2/service/kysdk-conf2.service et sync-config/com.kylin.kysdk.SyncConfig.service (sans User=) ; kylin-os-manager/daemons/service/data/com.kylin.os.manager.service (User=root, com.kylin-os-manager et .startup), .../com.kylin-os-manager-driver-acquirer.service (User=root), kyasDbus/systemdaemon/conf/com.kylin.assistant.systemdaemon.service (User=root) ; kylin-connectivity/kylinconnectivitymanager/com.kylin.connectivity.manager.service (User=root) ; chaque .conf : <policy context="default"><allow send_destination=...>
- **[CONSTATÉ]** Le service root com.kylin.daq (kylin-daq) est la cible D-Bus des bibliothèques de collecte libkysdk-datacollect et libkysdk-diagnostics. kylin-process-manager se lie à kysdk-datacollect, et la configuration installée de kylin-daq définit l'URL de remontée https://datareporting.openkylin.top:30005/. — *preuve* : libkysdk-diagnosetest/kysdk-datacollect/src/libkydatacollect.c l.49-51 'com.kylin.daq' ; libkysdk-base/src/diagnostics/buriedpoint.cpp l.49-51 'com.kylin.daq' ; kylin-process-manager/CMakeLists.txt l.31 pkg_check_modules(DATACOLLECT kysdk-datacollect) ; kylin-daq/conf/kylinDaqConf.json l.39 "UploadRealm": ["https://datareporting.openkylin.top:30005/"]
- **[CONSTATÉ]** 6 paquets source du lot n'ont aucun code (squelette dh_make). 3 d'entre eux fournissent néanmoins des binaires installés par défaut par l'ISO : kydoctor-common, kydoctor-server, kylin-id et openkylin-token-center. — *preuve* : archive:{app-compat-dkms,kydoctor,kylin-driver-manager-service,kylin-sso-client,kylin-mfa,openkylin-token-center}/debian/ : 19 fichiers chacun, rules = 'dh $@', gabarits *.ex ; fiche JSON installe_par_defaut_iso : kydoctor-common/kydoctor-server=1.0.0.0-0k0.59, kylin-id=2.2.11.2-0k1.18, openkylin-token-center=1.0.0-ok1.7
- **[CONSTATÉ]** Le mécanisme Kylin de contrôle d'appelants D-Bus (.limit + .limit.verify dans /etc/dbus-1/conf et /usr/share/dbus-1/conf, yaml whitelist dans /etc/kylin-config/basic) est utilisé par 5 paquets du lot. Deux fichiers .limit déclarent [envwhitelist] env1=LD_LIBRARY_PATH. — *preuve* : ukui-input-gather/ukui-input-gather/ukui-input-gather.pro (limit1/limit2/conf) ; kylin-daq/kylin-daq.pro (limit, limit2, yaml) et data/com.kylin.daq.limit '[envwhitelist] env1=LD_LIBRARY_PATH' ; kylin-os-manager/debian/kylin-os-manager.install (# dbus安全管控) et daemons/service/data/com.kylin.assistant.systemdaemon.limit '[envwhitelist] env1=LD_LIBRARY_PATH' ; kylin-process-manager/daemon/CMakeLists.txt l.83 (yaml) ; kylin-connectivity/kylinconnectivitymanager/CMakeLists.txt l.30 (yaml)
- **[CONSTATÉ]** Les deux paquets Rust du lot (kylin-wget et kylin-bot), tous deux installés par l'ISO, se construisent avec 'cargo build --release --locked' sans répertoire vendor. Les crates sont téléchargées depuis des miroirs crates.io chinois ; kylin-wget désactive aussi la vérification de révocation. — *preuve* : kylin-wget/.cargo/config.toml : registry = "sparse+https://mirrors.aliyun.com/crates.io-index/", [http] check-revoke = false ; kylin-bot/debian/cargo-config.toml : registry = "sparse+https://mirrors.tuna.tsinghua.edu.cn/crates.io-index/" ; aucun répertoire vendor/ ; Cargo.lock : 259 et 1 244 crates
- **[CONSTATÉ]** Deux paquets du lot ajoutent des chemins au chargeur dynamique via /etc/ld.so.conf.d. — *preuve* : libkysdk-base/development-files/kysdk-base.conf (/usr/lib/kysdk/kysdk-base) via debian/libkysdk-basecommon.install ; kylin-os-manager/data/config/kylin-os-manager.conf (5 chemins dont /usr/libs/kysdk/applications/) via CMakeLists.txt install(... DESTINATION /etc/ld.so.conf.d/)
- **[CONSTATÉ]** 7 entrées d'autostart XDG (/etc/xdg/autostart) sont installées par 4 paquets du lot. — *preuve* : libkysdk-base/debian/libkysdk-conf2.install (kysdk-conf2.desktop, com.kylin.kysdk.SyncConfig.desktop) ; kylin-os-manager/debian/kylin-os-manager.install (sessionservice-config.desktop, komd-config.desktop, kylin-os-manager-driver-acquirer.desktop) ; kylin-bot/debian/rules (kylin-bot-autostart.desktop) ; kylin-device-daemon/CMakeLists.txt (kylin-device-daemon.desktop)
- **[CONSTATÉ]** Deux scripts postinst du lot agissent dans les comptes utilisateurs : kylin-os-manager parcourt /home/*/ (rm, cp et chown de raccourcis), et kylin-bot démarre son démon via 'su - <user>' dans chaque session active. — *preuve* : kylin-os-manager/debian/kylin-os-manager.postinst : 'for home_dir in /home/*/; do ... chown "${user_name}":"${user_name}" "${dest_file}"' ; kylin-bot/debian/postinst : 'su - "$user" -c "... nohup $BIN daemon >/dev/null 2>&1 &"'
- **[CONSTATÉ]** Le lot contient 8 blobs binaires : 6 exécutables ELF et 1 JAR précompilés dans kylin-connectivity (installés sous /opt/connectivity et liés dans /usr/bin par postinst), ainsi qu'une base SQLite dans kylin-os-manager. — *preuve* : fiche JSON struct_archive.blobs (magic 7f454c46 / 504b0304 / 53514c69) ; kylin-connectivity/CMakeLists.txt l.120-122 install(DIRECTORY 3rdparty/tools/{arm,amd}/ DESTINATION /opt/connectivity/tools) ; kylin-connectivity/debian/postinst 'ln -sf /opt/connectivity/tools/anti-control-server /usr/bin/anti-control-server'
- **[CONSTATÉ]** 9 paquets du lot ont un dépôt Gitee marqué PRESENT, mais aucun arbre Gitee n'a été acquis pour eux. La comparaison archive/Gitee n'a été faite que pour platform-security-patrol (identique hors .git). — *preuve* : fiche JSON : forges.gitee=PRESENT et chemin_gitee=null pour kylin-process-manager, kylin-status-manager, ukui-input-gather, kylin-daq, libkysdk-diagnosetest, libkysdk-base, kylin-os-manager, kylin-device-daemon, kylin-connectivity ; diff -rq archive/platform-security-patrol gitee/platform-security-patrol -> seulement .git et .gitignore

