# Lot B01-IA-coeur

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-ai-sdk`

- **Nature** : propre-kylin — **Amont** : Kylin (dépôt Gitee openkylin/kylin-ai-sdk dont debian/control déclare Source: libkylin-ai-base ; Maintainer guopengfei@kylinos.cn ; miroir GitHub IDENTICAL-HEADS selon la fiche)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche openkylin/nile, version 2.0.0.0-ok1.1, format 3.0 native) ; pas de chemin archive propre, mais contenu quasi identique à archive:libkylin-ai-base (diff -rq : seuls debian/changelog, debian/source/format et src/services/speechprocessorproxy.cpp diffèrent)
- **Langages / taille** : C 535 Ko (fichiers *glue.c), en-têtes C/C++ 158 Ko, C++ 146 Ko, XML 4 Ko — 81 fichiers / 892 375 octets ; 68 fichiers de code hors debian/ — lignes de code propres mesurées : 23 502
- **Privilège** : Bibliothèque chargée dans le processus de l'application appelante (utilisateur) ; aucun service, aucun script mainteneur
- **Déclencheurs** : chargement par une application liée à libkylin-ai-base.so.2
- **Indices réseau** : aucun indice listen (fiche) ; clients unix:path=/tmp/org.openkylin.aisdk et unix:path=/tmp/org.openkylin.aisdk.aibusiness.<euid> (src/services/serverproxy.cpp, datamanagementprocessorproxy.cpp)
- **Blobs** : aucun
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Doublon Gitee de libkylin-ai-base (non installé par l'ISO) : seul le delta speechprocessorproxy.cpp + changelog est à examiner, le reste est couvert par l'entrée libkylin-ai-base.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `gitee:debian/control` | Source: libkylin-ai-base ; binaires libkylin-ai-base2 (Depends kylin-ai-runtime, tesseract-ocr-eng, tesseract-ocr-chi-sim) et libkylin-ai-base-dev ; Homepage gitee.com/openkylin/libkylin-ai-base | CONSTATÉ |
| debian-rules | `gitee:debian/rules` | dh $@ sans surcharge (comme l'archive) | CONSTATÉ |
| debian-install | `gitee:debian/libkylin-ai-base2.install` | usr/lib/*/lib*.so.* ; /usr/share/glib-2.0/schemas/ (identique à archive:libkylin-ai-base) | CONSTATÉ |
| debian-install | `gitee:debian/libkylin-ai-base-dev.install` | usr/include/* ; usr/lib/*/lib*.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `gitee:CMakeLists.txt` | add_library(kylin-ai-base SHARED) ; install pkg-configs/kylin-ai-base.pc et data/org.openkylin.aisdk.gschema.xml -> /usr/share/glib-2.0/schemas/ | CONSTATÉ |
| client-ipc | `gitee:src/services/serverproxy.cpp` | client GDBus pair-à-pair vers unix:path=/tmp/org.openkylin.aisdk (fichier identique à l'archive selon diff -rq) | CONSTATÉ |

- [CONSTATÉ] Le dépôt Gitee kylin-ai-sdk porte le paquet source libkylin-ai-base en version 2.0.0.0-ok1.1 (natif) alors que l'archive contient 2.0.0.0-ok1.0 en 3.0 (quilt) avec 36 patchs appliqués (.pc présent).
- [CONSTATÉ] Aucun binaire de ce source n'est listé dans installe_par_defaut_iso.
- [PROBABLE] Les fichiers src/services/*glue.c (535 Ko) sont vraisemblablement générés par gdbus-codegen (nommage), ce qui réduit le code écrit à la main.

### `libkylin-ai-system-private`

- **Nature** : propre-kylin — **Amont** : Kylin/openKylin (gitee.com/openkylin/libkylin-ai-system-private selon debian/upstream/metadata ; paquet présent dans Debian et Ubuntu, orig IDENTIQUE selon la fiche ; Uploaders zhangpan@kylinos.cn, jianfengli@ubuntukylin.com)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète 3.0 quilt : en-têtes ai-engine + définitions de protocoles ai-proto, aucun .cpp) ; gitee : README.md/README.en.md seulement (branche master)
- **Langages / taille** : Protobuf 78 Ko, XML (introspection D-Bus) 25 Ko, en-têtes C++ 25 Ko, CMake 7 Ko — 48 fichiers / 178 542 octets (archive) ; 2 fichiers / 2 301 octets (gitee) — lignes de code propres mesurées : 3 803
- **Privilège** : Aucun (paquet de développement : en-têtes et fichiers de protocole, pas de binaire exécutable)
- **Déclencheurs** : dépendance de construction (Build-Depends de libkysdk-ai-private)
- **Indices réseau** : définitions d'interfaces IPC : protocols/.../gdbus/*.xml et grpc/vector-db/*.proto (pas de code exécutable)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Uniquement en-têtes et définitions de protocoles, non installé par l'ISO ; utile comme référence de surface IPC, à comparer par diff avec libkylin-ai-engine-dev et libkylin-ai-proto.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | binaire unique libkylin-ai-system-private-dev (Section libdevel, en-têtes) ; Vcs-Git gitee openkylin | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ sans surcharge | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_subdirectory(ai-engine) et add_subdirectory(ai-proto) ; ai-proto/CMakeLists.txt : install(DIRECTORY protocols/usr/ DESTINATION /usr) | CONSTATÉ |
| watch/upstream | `archive:debian/watch` | version=5, source gitee.com/openkylin/libkylin-ai-system-private/releases ; debian/upstream/metadata pointe le même dépôt | CONSTATÉ |

- [CONSTATÉ] Variante divergente des paquets séparés : 7 en-têtes ai-engine diffèrent de libkylin-ai-engine-dev, et 10 fichiers de protocole diffèrent de libkylin-ai-proto (advaiknowledgebaseservice.xml absent ici).
- [CONSTATÉ] Gitee ne contient que des README (pas de code) alors que l'archive contient l'arbre complet.
- [CONSTATÉ] Aucun binaire listé dans installe_par_defaut_iso.

### `libkysdk-ai-private`

- **Nature** : propre-kylin — **Amont** : Kylin/openKylin (gitee.com/openkylin/libkysdk-ai-private ; paquet présent dans Debian et Ubuntu, orig IDENTIQUE selon la fiche ; Uploaders zhangpan@kylinos.cn, jianfengli@ubuntukylin.com)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète 3.0 quilt, .pc présent sans patch appliqué) ; gitee : README seulement (branche master)
- **Langages / taille** : C++ 91 Ko, en-têtes 41 Ko — 46 fichiers / 179 841 octets ; 24 fichiers de code — lignes de code propres mesurées : 3 898
- **Privilège** : Bibliothèques chargées dans le processus appelant (utilisateur) ; aucun service ni script mainteneur
- **Déclencheurs** : chargement par une application liée à libkyai-assistant.so.1 / libkyai-config.so.1 / libkyai-data-management-client.so.1
- **Indices réseau** : clients GDBus pair-à-pair vers les sockets de kylin-ai-runtime et de kyai-data-management-service (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Bibliothèques clientes non installées par l'ISO, copies divergentes de libkyai-assistant/libkyai-config/libkyai-data-management-client : audit par diff contre ces paquets.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | binaires libkysdk-ai-private0 et libkysdk-ai-private-dev ; Build-Depends libkylin-ai-system-private-dev, libglib2.0-dev, libjson-glib-dev | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-ai-private0.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-ai-private-dev.install` | usr/include/* ; usr/lib/*/lib*.so ; usr/share/pkgconfig/* | CONSTATÉ |
| lintian-overrides | `archive:debian/libkysdk-ai-private0.lintian-overrides` | package-name-doesnt-match-sonames libkyai-assistant1 libkyai-config1 libkyai-data-management-client1 (le paquet agrège 3 bibliothèques) | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_subdirectory(ai-config, ai-assistant, libkyai-data-management-client) ; chaque sous-projet installe une lib partagée + include/ + usr/ -> /usr | CONSTATÉ |
| client-ipc | `archive:ai-config/configserviceproxy.cpp` | client unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/config.sock ; ai-assistant/serverproxy.cpp -> assistant.sock ; libkyai-data-management-client/src/datamanagementserviceproxy.cpp -> unix:abstract=/tmp/.kylin-ai-business-unix/<euid>/DataManagement.sock | CONSTATÉ |

- [CONSTATÉ] ai-assistant et ai-config diffèrent de libkyai-assistant et libkyai-config (assistantserviceproxy.cpp, osassistant.cpp, configserviceproxy.cpp...) ; libkyai-data-management-client diffère de l'archive libkyai-data-management-client (5 fichiers src/ différents, dataManagementDBusSyncFuncs.h absent).
- [CONSTATÉ] Gitee ne contient que des README.

### `kylin-ai-runtime`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native ; changelog huanghe 1.1.0.1-ok0.21 du 24/07/2026)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 516 fichiers) ; Gitee et GitHub présents (DIVERGENT) mais non acquis (chemin_gitee null)
- **Langages / taille** : C++ 1,55 Mo, en-têtes 627 Ko, JSON 132 Ko, Python 96 Ko, XML 4 Ko ; « other » 39,8 Mo = bases SQLite et vector.db — 516 fichiers / 42 173 402 octets ; ~273 fichiers de code hors third-party (2,1 Mo) — lignes de code propres mesurées : 53 653
- **Privilège** : Utilisateur de session (autostart XDG) ; aucun composant root ; unité systemd --user présente mais non installée
- **Déclencheurs** : ouverture de session graphique (autostart XDG, redémarrage automatique X-UKUI-AutoRestart), connexions clientes sur /tmp/.kylin-ai-runtime-unix/<uid>/*.sock, événements d'alimentation via bus système (powereventmonitor)
- **Indices réseau** : champ listen de la fiche vide, mais 8 serveurs GDBus pair-à-pair sur sockets Unix constatés (abstractserver.cpp); libcurl + OpenSSL + libsodium liés (appels vers moteurs cloud via plugins : PROBABLE); client gRPC vers kylin-ai-vector-engine et TritonClient httpclient (CMakeLists.txt)
- **Blobs** : configs/usr/share/kylin-ai-runtime/intent-recognition/intent.db (5,6 Mo SQLite) ; knowledge-base/knowledge_base.db (4 Ko SQLite) + -shm/-wal ; knowledge-base/vector.db (33,6 Mo, non classé blob par l'extracteur) ; tests/16k.pcm ; tous installés sous /usr/share sauf tests
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Composant volumineux (~273 fichiers, 2,1 Mo) installé par l'ISO, démarré à chaque session, exposant 8 serveurs IPC, exécutant des commandes déclarées et du Python embarqué, avec accès réseau cloud : points d'entrée serveurs, assistant/intents, APIs et modelconfig en priorité.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| autostart-xdg | `archive:configs/etc/xdg/autostart/kylin-ai-runtime.desktop` | Exec=/bin/bash -c "env LD_LIBRARY_PATH=/usr/lib/kylin-ai/depends kylin-ai-runtime" ; NoDisplay=true ; X-UKUI-AutoRestart=true ; installé par install(DIRECTORY configs/etc/ DESTINATION /etc) | CONSTATÉ |
| systemd-user-unit | `archive:configs/lib/systemd/user/kylin-ai-runtime.service` | ExecStart=/usr/bin/kylin-ai-runtime ; Restart=always ; WantedBy=default.target ; aucune directive d'installation trouvée (CMake n'installe que configs/etc et configs/usr ; pas de debian/*.install ni postinst) | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_executable(kylin-ai-runtime) -> /usr/bin ; liens libcurl, OpenSSL crypto, libsodium, pulse, grpc++, TritonClient httpclient, pybind11::embed, kysdk-vector-engine-client, kyai-knowledgebase-client | CONSTATÉ |
| listener-ipc | `archive:src/server/abstractserver.cpp` | g_dbus_server_new_sync("unix:path="+filePath_, G_DBUS_SERVER_FLAGS_NONE, guid, GDBusAuthObserver=nullptr) ; chmod(/tmp/.kylin-ai-runtime-unix/<uid>/, 0777) (UNIX_PATH_MODE) | CONSTATÉ |
| listener-ipc | `archive:src/server/*server.cpp` | 8 sockets /tmp/.kylin-ai-runtime-unix/<uid>/ : config.sock, assistant.sock, core-speech.sock, core-vision.sock, core-textembedding.sock, core-imageembedding.sock, genai-nlp.sock, genai-vision.sock | CONSTATÉ |
| config-declarative | `archive:configs/etc/kylin-ai/APIs/cmd.json` | 154 entrées "cmd" contenant des lignes de commande shell (ex. ukui-control-center -m userinfo &, gsettings set org.ukui.SettingsDaemon...) installées dans /etc/kylin-ai/APIs/ | CONSTATÉ |
| config-declarative | `archive:configs/etc/kylin-ai/intents/scripts/` | slot_extractor.py, slot_pattern_rules.py installés sous /etc/kylin-ai/intents/scripts ; le binaire embarque l'interpréteur (pybind11::embed) | CONSTATÉ |
| config-declarative | `archive:configs/etc/kylin-ai/engines/ai-engines/` | profils de moteurs baichuan, baidu, custom, deepseek, freetrial, ondevice, qwen, sensetime, xunfei (prompts) ; aucune URL dans configs/etc/kylin-ai | CONSTATÉ |
| gschema | `archive:configs/usr/share/glib-2.0/schemas/com.kyin.aisdk.model-config.gschema.xml` | clés enabled/priority/nlp/vision/speech | CONSTATÉ |
| debian-control | `archive:debian/control` | binaire kylin-ai-runtime (Section admin) ; Build-Depends libssl-dev, libsodium-dev, libcurl4-openssl-dev, libgrpc++-dev, pybind11-dev, kytensor-client ; debian/rules dh $@ | CONSTATÉ |
| client-bus-systeme | `archive:src/lifecycle/powereventmonitor.cpp` | g_bus_get_sync(G_BUS_TYPE_SYSTEM) (client du bus système, l.40) | CONSTATÉ |

- [NON COUVERT] Le dépôt Gitee (branches openkylin/huanghe, upstream-onnxruntime...) existe selon la fiche mais n'a pas été acquis : la comparaison archive/Gitee n'est pas faite.
- [PROBABLE] Le répertoire /usr/lib/kylin-ai/depends ajouté à LD_LIBRARY_PATH par l'autostart n'est fourni par aucun paquet de ce lot.
- [CONSTATÉ] Code tiers embarqué : third-party/concurrentqueue (moodycamel).
- [CONSTATÉ] L'extracteur automatique ne détecte pas g_dbus_server_new_sync comme indice d'écoute.

### `kylin-ai-subsystem`

- **Nature** : meta-ou-donnees — **Amont** : Kylin (Maintainer wangzhijie@kylinos.cn ; métapaquet)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive debian/ + README.md (6 fichiers) ; gitee identique (diff -r : seul .git diffère)
- **Langages / taille** : aucun code (fichiers debian) — 6 fichiers / 5 336 octets (archive = gitee) — lignes de code propres mesurées : 0
- **Privilège** : Aucun (pas de script mainteneur ni fichier installé)
- **Déclencheurs** : installation du paquet (tire les dépendances du sous-système IA)
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Métapaquet de 6 fichiers installé par l'ISO ; seule la liste de dépendances (périmètre IA réellement installé) est à exploiter.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | métapaquet kylin-ai-subsystem : Depends ~30 paquets dont kyai-data-management-service, kylin-ai-document-qa-service, kylin-ai-knowledge-base-service, kylin-ai-document-service, kylin-ai-vector-engine, kylin-ai-runtime, kylin-ai-engine-plugins, kylin-ai-subsystem-modelconfig, et hors lot kylin-ai-python-env, kylin-ai-abstract-models, kytensor-client/server/python, onnxruntime-backend, llm-backend, kytensor-llm | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ | CONSTATÉ |
| debian-control | `gitee:debian/control` | identique à l'archive | CONSTATÉ |

- [CONSTATÉ] Les dépendances hors lot (kytensor-*, llm-backend, onnxruntime-backend, kylin-ai-python-env, kylin-ai-abstract-models, libkysdk-coreai-*, libkysdk-genai-*) étendent la surface IA installée et doivent figurer dans d'autres lots.

### `kylin-ai-subsystem-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer zhaokexin@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 40 Ko, en-têtes 14 Ko, qmake 2 Ko ; « TypeScript » 120 Ko = fichiers de traduction Qt Linguist translations/*.ts — 34 fichiers / 195 893 octets ; 9 fichiers de code (54 Ko) — lignes de code propres mesurées : 1 680
- **Privilège** : Plugin chargé dans ukui-control-center (session utilisateur) ; opérations APT déléguées au backend QApt (worker privilégié hors lot, via polkit)
- **Déclencheurs** : ouverture du module dans le centre de contrôle UKUI, action utilisateur d'installation/désinstallation/mise à jour
- **Indices réseau** : pas d'accès réseau direct relevé ; téléchargement de paquets via APT/QApt (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Petit composant propre (9 fichiers) installé par l'ISO, situé sur un chemin d'installation/mise à jour de paquets via QApt.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | kylin-ai-subsystem-plugin : « download, installation, uninstallation, and update functions of subsystems » ; Build-Depends libukcc-dev, libqapt-dev ; libkylin-ai-subsystem-plugin-dev | CONSTATÉ |
| debian-install | `archive:debian/kylin-ai-subsystem-plugin.install` | usr/lib/*/ukui-control-center/libkylinaisubsystemplugin.so ; traductions ; /usr/share/glib-2.0/schemas/ | CONSTATÉ |
| debian-install | `archive:debian/libkylin-ai-subsystem-plugin-dev.install` | usr/include/kylin-ai-subsystem-plugin/* ; usr/share/pkgconfig/* | CONSTATÉ |
| qmake-cible | `archive:ai-subsystem-plugin.pro` | TEMPLATE=lib plugin ; QT += widgets dbus xml ; LIBS -lukcc -lQApt -lgsettings-qt ; target.path=$$[QT_INSTALL_LIBS]/ukui-control-center ; schéma org.kylin-ai-subsystem-plugin (install-progress, uninstall-progress, last-detect-install-time) | CONSTATÉ |
| gestion-paquets | `archive:subsystemwidget.cpp` | #include <QApt/Transaction> (l.21) ; initialisation du backend QApt (l.539-548) | CONSTATÉ |

- [CONSTATÉ] Le .pro lit /etc/os-release de la machine de construction (KYLIN_RELEASE_ID) pour définir SYSTEM_VERSION_2403.
- [NON COUVERT] Gitee présent (branches openkylin/huanghe...) mais non acquis.

### `kylin-ai-subsystem-modelconfig`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer dongshuo@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 554 Ko, en-têtes 116 Ko, qmake 7 Ko, Shell 0,5 Ko ; « TypeScript » 1,04 Mo = traductions Qt .ts — 146 fichiers / 1 930 792 octets ; 94 fichiers de code (671 Ko) — lignes de code propres mesurées : 18 687
- **Privilège** : Plugin chargé par kylin-ai-subsystem-plugin dans ukui-control-center (session) ; demande d'autorisation polkit auth_admin ; le programme exécuté après autorisation n'a pas été identifié
- **Déclencheurs** : ouverture de la page de configuration des modèles, édition de modèles cloud/privés (contrôle polkit), partage de puissance de calcul (gsettings)
- **Indices réseau** : computingpowersharing/computepowerinterface.cpp : QNetworkAccessManager; updatenotificationwidget.cpp : curl_easy_init; gschema local-ip / verification-code : partage de calcul entre machines (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant de session de taille moyenne (94 fichiers) installé par l'ISO, avec polkit, réseau (partage de calcul entre IP, curl) et gestion de modèles : cibler computingpowersharing/, modeldownload, widgets polkit et configmanager.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| polkit | `archive:data/org.kylin-ai-subsystem-modelconfig.policy` | action org.kylin-ai-subsystem-modelconfig.policy ; allow_any=auth_admin, allow_inactive=auth_admin, allow_active=auth_admin ; « Authorization is required to edit model information » ; installée dans /usr/share/polkit-1/actions (.pro) | CONSTATÉ |
| qmake-cible | `archive:kylin-ai-subsystem-modelconfig.pro` | TEMPLATE=lib ; target.path=$$[QT_INSTALL_LIBS]/kylin-ai-subsystem-plugin ; QT += network ; LIBS -lpolkit-qt5-core-1 -lcurl ; PKGCONFIG libqapt kyai-config ; INSTALLS target qmfile polkitflie schemas | CONSTATÉ |
| gschema | `archive:data/org.kylin-ai-subsystem-modelconfig.gschema.xml` | clés enable-compute-power-sharing, compute-power-sharing-role, local-ip, enable-request-compute-power, enable-shared-compute-power, verification-code, enable-domain-control | CONSTATÉ |
| debian-control | `archive:debian/control` | binaire unique ; Depends libkyai-config0, libkysdk-ai-common ; aucun script mainteneur ; debian/rules dh $@ | CONSTATÉ |
| usage-polkit | `archive:cloudmodelconfigwidget.cpp` | référence à l'action org.kylin-ai-subsystem-modelconfig.policy (l.964) ; idem privatemodelconfigwidget.cpp l.2193 | CONSTATÉ |
| script | `archive:running.sh` | script shell de premier niveau absent des INSTALLS ; contenu non lu | NON COUVERT |

- [CONSTATÉ] La forte part « TypeScript » de la fiche correspond à des traductions Qt (.ts) et non à du code.
- [NON COUVERT] Gitee présent mais non acquis.

### `kylin-ai-business-framework-service`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer wangyan@kylinos.cn ; format 3.0 quilt, orig « amont-inconnu » ; Homepage www.ukui.org ; changelog nile 1.0.0.0-ok0.11 du 30/08/2024)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète avec 20 patchs quilt appliqués, .pc présent) + 204 roues Python précompilées ; Gitee présent mais non acquis ; GitHub EMPTY
- **Langages / taille** : C++ 130 Ko, C 113 Ko (glue), en-têtes 66 Ko, Python 13 Ko ; « other » 488,7 Mo (roues .whl et patchs) — 297 fichiers / 489 040 272 octets ; ~56 fichiers de code propres (307 Ko) — lignes de code propres mesurées : 9 213
- **Privilège** : postinst en root (création venv + pip install) ; service utilisateur de session (systemd --user)
- **Déclencheurs** : installation/mise à jour du paquet (postinst), démarrage de session (default.target utilisateur), connexions sur /tmp/org.openkylin.aisdk.aibusiness.<euid>
- **Indices réseau** : serveur GDBus pair-à-pair unix:path sous /tmp (champ listen de la fiche vide); pip3 install sans --no-index dans le postinst : résolution de dépendances potentiellement réseau (PROBABLE)
- **Blobs** : 204 roues .whl (102 amd64 + 102 arm64, 160 Mo) : numpy 1.26.4, transformers 4.40.2/4.42.3, onnxruntime 1.17.3/1.18.1, grpcio, kubernetes 29/30, chromadb 0.5.0/0.5.3, lxml, pillow, tokenizers, uvloop... ; debian/patches/0019 (244 Mo) et 0001-feat-Python.patch (83 Mo)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Code propre de taille raisonnable (~56 fichiers) avec chemin d'installation root (venv + pip) et serveur IPC ; les roues tierces relèvent d'une vérification d'empreintes contre PyPI plutôt que d'une lecture.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:configs/lib/systemd/user/kylin-ai-business-framework-service.service` | ExecStart=/bin/bash -c 'source /usr/share/kylin-ai-business-framework/python-env/bin/activate && /usr/bin/kylin-ai-business-framework-service' ; Restart=always ; After=login@.service ; WantedBy=default.target ; installé dans /usr/lib/systemd/user (CMakeLists.txt l.86) | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-ai-business-framework-service.postinst` | deb-systemd-helper --user enable ... ; python3 -m venv /usr/share/kylin-ai-business-framework/python-env ; pip3 install /usr/share/kylin-ai-business-framework/python-depends/{amd64\|arm64}/* selon uname -m (exécuté en root par dpkg) | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_executable(kylin-ai-business-framework-service) ; install src/utils/python -> ${FRAMEWORK_PATH}/utils ; install python-depends/ -> ${FRAMEWORK_PATH}/python-depends | CONSTATÉ |
| listener-ipc | `archive:src/server.cpp` | g_dbus_server_new_sync(unix:path=/tmp/org.openkylin.aisdk.aibusiness.<euid>) (l.24-25, l.92) | CONSTATÉ |
| debian-control | `archive:debian/control` | Architecture amd64 arm64 ; Depends onnxruntime, python3-venv, python3-pip ; debian/rules dh $@ | CONSTATÉ |
| maintainer-script [ajout critique] | `archive:kylin-ai-business-framework-service/debian/kylin-ai-business-framework-service.postinst` | Detail manquant au recensement. Le postinst (root) cree le venv et lance 'pip3 install /usr/share/kylin-ai-business-framework/python-depends/<amd64\|arm64>/*' sans --no-index ni --require-hashes. Il execute aussi deb-systemd-helper --user enable. | CONSTATÉ |

- [CONSTATÉ] Aucun binaire de ce source n'est listé dans installe_par_defaut_iso (dernier changelog : nile, 2024).
- [CONSTATÉ] Les versions des roues diffèrent entre amd64 et arm64 (ex. transformers 4.40.2 vs 4.42.3, onnxruntime 1.17.3 vs 1.18.1).
- [NON COUVERT] Gitee présent mais non acquis.

### `libkyai-business-framework`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer hezhe@kylinos.cn ; Homepage gitlab2.kylin.com/kylinos-src/kylin-ai-business-framework ; embarque SQLiteCpp)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; aucune forge publique (Gitee et GitHub ABSENT)
- **Langages / taille** : C++ 195 Ko, en-têtes 120 Ko, C 51 Ko (test glue), XML, JSON — 114 fichiers / 8 806 806 octets (dont une image de test de 8,4 Mo) ; 93 fichiers de code (366 Ko) — lignes de code propres mesurées : 10 027
- **Privilège** : Bibliothèque chargée par les services utilisateur kyai-data-management-service, kylin-ai-document-qa-service et kylin-ai-knowledge-base-service (pkg_check_modules kyai-business-framework)
- **Déclencheurs** : démarrage des services consommateurs (systemd --user)
- **Indices réseau** : serveur GDBus sur socket Unix abstraite (espace de noms réseau, pas de permission fichier) : unix:abstract=/tmp/.kylin-ai-business-unix/<euid>/<service>.sock
- **Blobs** : test/sqlitecpp/test.db (24 Ko SQLite) ; test/embedding/20240709181353.jpg (8,4 Mo)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Socle serveur IPC commun à trois services installés par l'ISO, taille raisonnable (93 fichiers dont SQLiteCpp tiers à vérifier par diff).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkyai-business-framework (« AI场景服务框架 ») et -dev ; Build-Depends libsqlite3-dev, libkylin-coreai-embedding-dev, libkysdk-vector-engine-client-dev ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkyai-business-framework.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkyai-business-framework-dev.install` | usr/include/* ; usr/lib/*/lib*.so ; usr/share/pkgconfig/*.pc | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kyai-business-framework SHARED) ; demo/ et test/ en add_subdirectory optionnels | CONSTATÉ |
| listener-ipc | `archive:src/server/businessServer.cpp` | g_dbus_server_new_sync(SERVER_ADDRESS_PRE + euid + "/" + serviceName + ".sock", ..., GDBusAuthObserver=nullptr) ; g_dbus_server_start ; signal new-connection (l.35-55) | CONSTATÉ |
| constante-ipc | `archive:src/common/constant.cpp` | SERVER_ADDRESS_PRE = "unix:abstract=/tmp/.kylin-ai-business-unix/" (l.21) | CONSTATÉ |

- [CONSTATÉ] Homepage sur un GitLab interne (gitlab2.kylin.com) ; aucun dépôt public recensé dans la fiche.
- [CONSTATÉ] Code tiers SQLiteCpp embarqué (src/sqlitecpp, include/kyai-business-framework/SQLiteCpp), version non relevée.

### `kylin-ai-cloud-model`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Original-Maintainer hezhe@kylinos.cn ; Homepage code.kylinos.cn/plugins/gitiles/desktop/kylin-ai-cloud-model)
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive debian/ seulement (squelette dh_make, 19 fichiers dont modèles .ex) ; aucune forge
- **Langages / taille** : aucun code — 19 fichiers / 26 757 octets — lignes de code propres mesurées : 0
- **Privilège** : Indéterminé (contenu binaire inconnu) ; aucun script actif dans la source
- **Déclencheurs** : installation des binaires ISO (contenu non observé)
- **Indices réseau** : nom « cloud model » : configuration de modèles distants (PROBABLE, non vérifié)
- **Blobs** : aucun dans la source
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Squelette dh_make sans contenu alors que 3 binaires sont installés par l'ISO : acquérir et désassembler les .deb pour connaître leur contenu.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 4 binaires ai-kylin-qwen-plus-cloud-model, ai-kylin-qwen-max-cloud-model, ai-kylin-deepseek-r1-cloud-model, ai-kylin-deepseek-v3-cloud-model (« 麒麟AI云端试用模型 ») ; Maintainer Ubuntu Developers | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ sans surcharge ; aucun fichier source à installer | CONSTATÉ |
| maintainer-script | `archive:debian/postinst.ex` | modèles dh_make inactifs (postinst.ex, postrm.ex, preinst.ex, prerm.ex, cron.d.ex) | CONSTATÉ |
| binaire-iso | `aucun (paquets binaires de l'ISO)` | contenu de ai-kylin-deepseek-r1/deepseek-v3/qwen-plus-cloud-model 1.0.0.0-ok0.1 non dérivable de la source | NON COUVERT |

- [CONSTATÉ] dl/build/kylin-ai-cloud-model ne contient que le .tar.xz natif correspondant à ce squelette.
- [CONSTATÉ] Changelog « Initial Release » 1.0.0.0-ok0.1 (nile.bedrock, 23/06/2026).

### `kylin-ai-engine-plugins`

- **Nature** : meta-ou-donnees — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; métapaquet)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive debian/ + README (modèle Gitee) ; gitee identique (diff -rq : seul .git diffère)
- **Langages / taille** : aucun code — 7 fichiers / 5 314 octets (archive = gitee) — lignes de code propres mesurées : 0
- **Privilège** : Aucun
- **Déclencheurs** : installation (tire 8 plugins moteurs hors lot)
- **Indices réseau** : les plugins tirés (baidu, xunfei, qwen, deepseek, freetrial...) correspondent à des fournisseurs cloud (PROBABLE, hors lot)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Métapaquet de 7 fichiers installé par l'ISO ; seule la liste des plugins moteurs à couvrir ailleurs est utile.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | métapaquet : Depends libkylin-baidu-ai-engine-plugin, libkylin-xunfei-ai-engine-plugin, libkylin-qwen-ai-engine-plugin, libkylin-custom-ai-engine-plugin, libkylin-deepseek-ai-engine-plugin, libkylin-freetrial-ai-engine-plugin, libkylin-ondevice-traditional-ai-engine-plugin, libkylin-ondevice-ai-engine-plugin | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ | CONSTATÉ |
| debian-control | `gitee:debian/control` | identique à l'archive | CONSTATÉ |

- [CONSTATÉ] Les 8 plugins moteurs (code d'accès aux API cloud) ne sont pas dans ce lot.

### `libkylin-ai-engine-dev`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète : en-têtes seulement) ; aucune forge
- **Langages / taille** : en-têtes C++ 25 Ko, CMake 0,6 Ko — 21 fichiers / 67 757 octets ; 11 en-têtes — lignes de code propres mesurées : 808
- **Privilège** : Aucun (en-têtes de développement)
- **Déclencheurs** : dépendance de construction de kylin-ai-runtime
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 11 en-têtes (25 Ko) définissant l'ABI des plugins moteurs ; lecture complète triviale, utile pour le runtime.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ai-engine-dev (Section libdevel) : en-têtes de l'interface des plugins moteurs ; debian/rules dh $@ | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(KylinAiEngine INTERFACE) ; install include/kylin-ai -> include ; fichiers cmake de config | CONSTATÉ |

- [CONSTATÉ] Le CMakeLists.txt reprend un gabarit tiers (DESCRIPTION « header-only c++ library ... International System of Units », HOMEPAGE_URL github.com/bernedom/SI).
- [CONSTATÉ] Non installé par l'ISO (paquet -dev).

### `libkylin-ai-base`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; Homepage gitee.com/openkylin/libkylin-ai-base ; format 3.0 quilt, orig « amont-inconnu »)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète, 36 patchs quilt appliqués, .pc présent) ; dépôt Gitee du même source acquis sous gitee/kylin-ai-sdk ; Gitee libkylin-ai-base non acquis
- **Langages / taille** : C 535 Ko (glue gdbus), en-têtes 158 Ko, C++ 145 Ko ; « other » 997 Ko = patchs — 118 fichiers / 1 839 037 octets ; 68 fichiers de code (838 Ko) — lignes de code propres mesurées : 23 472
- **Privilège** : Bibliothèque chargée dans le processus appelant (utilisateur)
- **Déclencheurs** : chargement par une application liée à libkylin-ai-base.so.2
- **Indices réseau** : clients GDBus pair-à-pair vers sockets /tmp (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Bibliothèque cliente non installée par l'ISO, majoritairement du code de liaison généré ; triage des proxys et de l'OCR, avec revue de la série de 36 patchs.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ai-base2 (Depends kylin-ai-runtime, tesseract-ocr-eng, tesseract-ocr-chi-sim) et libkylin-ai-base-dev ; Build-Depends libssl-dev, libtesseract-dev, libleptonica-dev ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkylin-ai-base2.install` | usr/lib/*/lib*.so.* ; /usr/share/glib-2.0/schemas/ | CONSTATÉ |
| debian-install | `archive:debian/libkylin-ai-base-dev.install` | usr/include/* ; usr/lib/*/lib*.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kylin-ai-base SHARED) ; install data/org.openkylin.aisdk.gschema.xml (clés enabled, deploy-policy, policy-model-map, model-config) | CONSTATÉ |
| client-ipc | `archive:src/services/serverproxy.cpp` | client unix:path=/tmp/org.openkylin.aisdk ; datamanagementprocessorproxy.cpp : unix:path=/tmp/org.openkylin.aisdk.aibusiness.<euid> | CONSTATÉ |

- [CONSTATÉ] La série debian/patches (36 entrées) inclut de gros patchs (0016/0019-nlp-sessionId.patch ~80-100 Ko) ; arbre archive livré patchs appliqués.
- [PROBABLE] Le serveur à l'adresse unix:path=/tmp/org.openkylin.aisdk n'est fourni par aucun paquet de ce lot (le runtime écoute sous /tmp/.kylin-ai-runtime-unix/).
- [CONSTATÉ] Aucun binaire listé dans installe_par_defaut_iso.

### `libkylin-ai-proto`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète : définitions de protocoles + fonctions CMake de génération) ; aucune forge
- **Langages / taille** : Protobuf 79 Ko, XML introspection D-Bus 47 Ko, CMake 10 Ko — 29 fichiers / 144 505 octets ; 11 XML gdbus + 9 .proto — lignes de code propres mesurées : 3 011
- **Privilège** : Aucun (fichiers de définition)
- **Déclencheurs** : dépendance de construction (runtime, business-framework, services, clients)
- **Indices réseau** : définit la surface IPC GDBus et gRPC du sous-système (pas de code exécutable)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Données de définition non installées par l'ISO mais servant de cartographie exhaustive des méthodes IPC exposées par les serveurs du lot.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ai-proto-dev : « Unified protocol files for communication between AI service processes, including D-Bus and gRPC » ; debian/rules dh $@ | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(KylinAiProto INTERFACE) ; install(DIRECTORY protocols/usr/ DESTINATION /usr) -> /usr/share/kylin-ai/protocols/{gdbus,grpc} | CONSTATÉ |
| definition-ipc | `archive:protocols/usr/share/kylin-ai/protocols/gdbus/` | advaiknowledgebaseservice, assistantservice, configservice, coreimageembeddingservice, corespeechservice, coretextembeddingservice, corevisionservice, datamanagementservice, genaivisionservice, gennlpservice, knowledgebase (.xml) ; grpc/document-service/document.proto ; grpc/vector-db/*.proto (milvus, common, schema, plan...) | CONSTATÉ |

- [CONSTATÉ] Une copie divergente de ces protocoles existe dans libkylin-ai-system-private/ai-proto.

### `libkylin-ai-default-key`

- **Nature** : absent-ou-vide — **Amont** : Kylin (Maintainer Luoyaoming@kylinos.cn, Original-Maintainer zhaokexin@kylinos.cn ; composant « pty »)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive debian/ seulement (squelette dh_make avec modèles .ex) ; aucune forge
- **Langages / taille** : aucun code — 19 fichiers / 27 134 octets — lignes de code propres mesurées : 0
- **Privilège** : Indéterminé (bibliothèque binaire non observée)
- **Déclencheurs** : chargement par un consommateur non identifié
- **Indices réseau** : aucun observable
- **Blobs** : aucun dans la source
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; le binaire (dont le nom évoque une clé par défaut) doit être acquis et analysé en binaire.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 4 strophes du même binaire libkylin-ai-default-key (riscv64, arm64, loong64, amd64) avec Depends libc6, libgcc-s1, libjsoncpp25, libstdc++6 : métadonnées de binaires compilés absents de la source ; Description « Kylin AI Default Key » | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ sans surcharge | CONSTATÉ |
| maintainer-script | `archive:debian/postinst.ex` | modèles dh_make inactifs (.ex) | CONSTATÉ |
| binaire | `aucun` | bibliothèque compilée correspondante non disponible en source | NON COUVERT |

- [NON COUVERT] Le nom « Default Key » suggère une donnée de clé ou d'identifiant (catégorie crypto/confiance) ; non vérifiable sans le binaire.
- [CONSTATÉ] Aucun binaire listé dans installe_par_defaut_iso.

### `kylin-aiassistant`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer zhangzhaohong@kylinos.cn ; format 3.0 native ; changelog huanghe 5.0.0.0-0k1.33 du 07/09/2026)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 1 312 fichiers) ; Gitee et GitHub présents (DIVERGENT) mais non acquis
- **Langages / taille** : C++ 1,29 Mo, QML 2,08 Mo, JavaScript 1,04 Mo (bundle web_kbot/dist), en-têtes 234 Ko, « TypeScript » 3,1 Mo (traductions Qt .ts + 53 .ts/85 .tsx de web_kbot/src), XML 533 Ko ; « other » 111 Mo (images, SVG, GIF) — 1 312 fichiers / 120 066 926 octets ; ~401 fichiers de code (4,5 Mo) hors dist — lignes de code propres mesurées : 126 283
- **Privilège** : Application de session utilisateur (activation D-Bus de session, lancement menu) ; scripts mainteneur root (postinst, postrm parcourant /home/*)
- **Déclencheurs** : activation D-Bus de session org.ukui.kylin_aiassistant / org.ukui.voice_assistant, lancement depuis le menu ou raccourci (schéma media-keys), menu contextuel du gestionnaire de fichiers Peony (plugin), désinstallation/purge (postrm)
- **Indices réseau** : QLocalSocket (32 occurrences) vers la passerelle kylin-bot : ~/.kylinbot/gateway.sock ou $XDG_RUNTIME_DIR/kylin-bot/gateway.sock (unixsocketwebsocket.cpp, webapibridge.cpp, scheduled/kybotcronclient.cpp); QNetworkAccessManager (7 occurrences); QWebEngine + QWebChannel (contenu web embarqué); registerService sur le bus de session (dbus/kadbusoperator.cpp, vadbusoperator.cpp); indice listen de la fiche (web_kbot/dist/assets/index.js « .listen( ») = écouteur d'historique React Router / événements Tauri, pas un serveur réseau
- **Blobs** : aucun blob binaire classé ; bundle JS préconstruit web_kbot/dist/assets/index.js embarqué tel quel ; GIF/SVG volumineux (voice_logo.gif 5,2 Mo x3)
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Très gros composant de session (~401 fichiers de code) installé par l'ISO, avec IPC D-Bus/socket, réseau, WebEngine, collecte de données (kdk_dia) et interfaces Wayland d'injection d'entrées/capture d'écran : cibler dbus/, webapibridge, unixsocketwebsocket, scheduled/, textselect, collecte, plugin Peony et cohérence dist/src.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| dbus-session-activation | `archive:kylin-aiassistant/debdata/org.ukui.kylin_aiassistant.service` | Name=org.ukui.kylin_aiassistant ; Exec=/usr/bin/kylin-aiassistant ; installé dans /usr/share/dbus-1/services (variante data/ pour BUILD_KAIMING : Exec=/opt/kaiming/bin/kylin-aiassistant) | CONSTATÉ |
| dbus-session-activation | `archive:voice-assistant/data/org.ukui.voice_assistant.service` | Name=org.ukui.voice_assistant ; Exec=/usr/bin/voice-assistant ; installé dans /usr/share/dbus-1/services | CONSTATÉ |
| desktop-file | `archive:kylin-aiassistant/kylin-aiassistant.desktop` | Exec=/usr/bin/kylin-aiassistant ; X-UKUI-AutoRestart=true ; X-KDE-Wayland-Interfaces=org_kde_plasma_window_management, org_kde_kwin_keystate, org_kde_kwin_fake_input, zkde_screencast_unstable_v1 | CONSTATÉ |
| autostart-xdg | `archive:kylin-aiassistant/kylin-aiassistant-autostart.desktop` | Exec=/usr/bin/kylin-aiassistant --silence ; NON installé (lignes autostart commentées dans kylin-aiassistant.pro, « IK8E9A 临时方案 ») ; maintscript rm_conffile /etc/xdg/autostart/kylin-aiassistant-autostart.desktop | CONSTATÉ |
| desktop-file | `archive:voice-assistant/data/voice-assistant.desktop` | Exec=/opt/kaiming-tools/bin/kaiming run --command=/usr/bin/voice-assistant cn.kylin.kylin-aiassistant -- ; NoDisplay=true | CONSTATÉ |
| polkit | `archive:kylin-aiassistant/data/org.kylin.aiassistant.policy` | action org.kylin.aiassistant.action, auth_admin partout ; listé seulement dans DISTFILES, absent des INSTALLS et non référencé dans le code C++ | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-aiassistant.postinst` | glib-compile-schemas /usr/share/glib-2.0/schemas ; rm -f /etc/xdg/autostart/kylin-aiassistant-autostart.desktop | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-aiassistant.postrm` | sur remove et purge : boucle for user in /home/* et rm $user/.config/kylin-aiassistant/settings.ini | CONSTATÉ |
| debian-rules | `archive:debian/rules` | override_dh_auto_build : exige web_kbot/dist/index.html préconstruit (pas de npm build) ; dh_shlibdeps --ignore-missing-info | CONSTATÉ |
| qmake-cible | `archive:kylin-aiassistant.pro` | SUBDIRS kylin-aiassistant, voice-assistant, peony-menu-plugin-kylin-aiassistant (x86_64/arm64) -> $$[QT_INSTALL_LIBS]/peony-extensions ; kylin-aiassistant.pro : QT webengine webchannel websockets network ; INSTALLS target qmfile icon desktopentry plugin_schema (media-keys) doctemplate guide service notification (/etc/ukui/ukui-notification/default-app/) | CONSTATÉ |
| ressource-embarquee | `archive:kylin-aiassistant/res/webkbot.qrc` | embarque ../web_kbot/dist/index.html, qwebchannel.js, assets/index.js (bundle minifié) dans le binaire | CONSTATÉ |
| debian-control | `archive:debian/control` | Depends kylin-bot, ffmpeg, pandoc, poppler-utils, libqt5webengine5, libkyai-assistant0, libkyai-config0 ; Build-Depends libkysdk-datacollect-dev, libpeony-dev, libxtst-dev ; champ « Version: 3.0.0.0-0k3.46 » dans la strophe source | CONSTATÉ |

- [CONSTATÉ] Collecte de données : Build-Depends libkysdk-datacollect-dev et 23 occurrences de kdk_dia dans le code C++/pro.
- [CONSTATÉ] Le bundle web_kbot/dist n'est pas reconstruit à partir de web_kbot/src lors du build Debian ; la correspondance dist/src est à vérifier.
- [CONSTATÉ] Dépendance d'exécution kylin-bot (passerelle gateway.sock) hors lot.
- [NON COUVERT] Gitee/GitHub présents (nombreuses branches dont develop-for-6.0, research/barge-in-qt-aec) mais non acquis.

### `kylin-ai-data-management-service`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer wangzhijie@kylinos.cn ; Homepage gitlab2.kylin.com/kylinos-src/kylin-ai-data-management-service ; embarque l'extension SQLite recover)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 200 Ko, C 119 Ko (sqlite3recover.c, dbdata.c), en-têtes 47 Ko, JSON — 80 fichiers / 465 907 octets ; 66 fichiers de code (367 Ko) — lignes de code propres mesurées : 10 695
- **Privilège** : Service utilisateur de session (systemd --user) ; postinst root limité à l'activation utilisateur
- **Déclencheurs** : démarrage de session (default.target utilisateur), requêtes IPC des clients (libkyai-data-management-client)
- **Indices réseau** : serveur GDBus sur socket Unix abstraite (via framework); clients gRPC (document-service, vector-engine) et NLP
- **Blobs** : aucun (test/20250626.jpg, test/hand.docx : fichiers de test)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Service propre de taille raisonnable installé par l'ISO qui indexe le contenu de fichiers utilisateur (handlers AddTextFiles/AddImageFiles, tâches tag/summary/embedding) : collecte de données ; sqlite3recover.c amont à vérifier par diff.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:conf/kyai-data-management-service.service` | ExecStart=/usr/bin/kyai-data-management-service -s DataManagement ; Restart=always ; WantedBy=default.target ; installé dans /usr/lib/systemd/user (CMakeLists.txt l.65) | CONSTATÉ |
| maintainer-script | `archive:debian/kyai-data-management-service.postinst` | deb-systemd-helper --user enable kyai-data-management-service.service | CONSTATÉ |
| config-declarative | `archive:conf/DataManagement.json` | service_name DataManagement ; extract_tag_nlp_model_name llm_Qwen-2.5-3b_1.0 (OnDevice) ; ai_index_task_type [1,2,3] ; installé dans /usr/share/kylin-ai/kyai-business-framework | CONSTATÉ |
| debian-control | `archive:debian/control` | binaire kyai-data-management-service (amd64 arm64) ; Rules-Requires-Root: no ; Build-Depends libkyai-business-framework-dev, libkylin-ai-document-service-dev, libkysdk-vector-engine-client-dev, libkysdk-genai-nlp-dev ; debian/rules dh $@ | CONSTATÉ |
| listener-ipc | `archive:src/main.cpp` | serveur via libkyai-business-framework (BusinessServer) : adresse attendue unix:abstract=/tmp/.kylin-ai-business-unix/<euid>/DataManagement.sock (déduite de constant.cpp + service_name) | PROBABLE |

- [CONSTATÉ] src/sqliteRecover/sqlite3recover.c et dbdata.c portent l'en-tête de domaine public SQLite (code amont embarqué).
- [CONSTATÉ] Nom du paquet binaire (kyai-data-management-service) différent du nom du source.

### `libkyai-data-management-client`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer wangzhijie@kylinos.cn ; Homepage gitlab2.kylin.com/kylinos-src/kylin-ai-data-management-client)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 182 Ko, en-têtes 35 Ko — 46 fichiers / 262 027 octets ; 29 fichiers de code (dont nombreux tests) — lignes de code propres mesurées : 5 108
- **Privilège** : Bibliothèque chargée dans le processus appelant (utilisateur)
- **Déclencheurs** : chargement par une application cliente du service DataManagement
- **Indices réseau** : client GDBus sur socket Unix abstraite (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Bibliothèque cliente à faible privilège (installée par l'ISO) ; l'effort porte sur le serveur kyai-data-management-service.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkyai-data-management-client et -dev ; Rules-Requires-Root: no ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkyai-data-management-client.install` | usr/lib/*/*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkyai-data-management-client-dev.install` | usr/include/* ; usr/lib/*/*.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kyai-data-management-client SHARED) ; test/ optionnel | CONSTATÉ |
| client-ipc | `archive:src/datamanagementserviceproxy.cpp` | client unix:abstract=/tmp/.kylin-ai-business-unix/<euid>/DataManagement.sock (l.93) | CONSTATÉ |

- [CONSTATÉ] Une copie divergente existe dans libkysdk-ai-private/libkyai-data-management-client.

### `kylin-ai-document-service`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer mayuan@kylinos.cn ; Homepage gitlab2.kylin.com/kylinos-src/kylin-ai-document-service)
- **Installé par défaut (ISO 3.0)** : oui (2 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; Gitee présent mais non acquis
- **Langages / taille** : Python 109 Ko, C++ 16 Ko, en-têtes 8 Ko, Protobuf ; « other » 3,6 Mo (PDF, journaux, docx) — 59 fichiers / 3 695 105 octets ; 37 fichiers de code (133 Ko) — lignes de code propres mesurées : 3 458
- **Privilège** : Service utilisateur (systemd --user) démarré à la demande (pas de WantedBy, pas de postinst)
- **Déclencheurs** : démarrage par un client via le gestionnaire systemd utilisateur (PROBABLE, sd_bus dans src/client.cpp), requêtes gRPC de parsing de documents
- **Indices réseau** : serveur gRPC Python sur 2 sockets Unix /tmp avec add_insecure_port (champ listen de la fiche vide)
- **Blobs** : aucun binaire classé ; python/embedding.pdf (1,3 Mo), python/pdftest.pdf, python/data/金融知识库.pdf, python/data/麒麟软件工会经费报销单.docx, python/logs/parse_service.log (982 Ko), parse_client.log (358 Ko)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Composant propre compact (37 fichiers) installé par l'ISO, exposant un serveur gRPC qui analyse des documents utilisateur (PDF, Office, OFD, HTML, XML, Excel).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:configs/lib/systemd/user/kylin-ai-document-service.service` | ExecStart=/bin/bash -c 'source /usr/share/kylin-ai-python-env/python-env/bin/activate && python3 /usr/share/kylin-ai-document-service/python/server.py' ; Restart=always ; After=login@.service ; aucune section [Install] | CONSTATÉ |
| listener-ipc | `archive:python/server.py` | grpc.server(ThreadPoolExecutor(max_workers=10)) ; add_insecure_port unix:///tmp/document_service_sync_<uid>.sock et unix:///tmp/document_service_async_<uid>.sock (l.202-220) | CONSTATÉ |
| debian-install | `archive:debian/kylin-ai-document-service.install` | usr/lib/systemd/user/* ; usr/share/kylin-ai-document-service/python | CONSTATÉ |
| debian-install | `archive:debian/libkylin-ai-document-service.install` | usr/lib/*/lib*.so.* (client C++) ; -dev : usr/include/*, *.so, pkgconfig | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kylin-ai-document-service SHARED) ; install(DIRECTORY python/ DESTINATION /usr/share/kylin-ai-document-service/python) ; install configs/lib/systemd/user | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-ai-document-service Depends kylin-ai-python-env, kylin-ai-parser-extension (hors lot) ; Build-Depends libgrpc++-dev, libsystemd-dev | CONSTATÉ |
| client-systemd | `archive:src/client.cpp` | sd_bus vers org.freedesktop.systemd1 (checkServiceActive) ; clients gRPC vers unix:///tmp/document_service_sync_*.sock | CONSTATÉ |

- [CONSTATÉ] install(DIRECTORY python/ ...) installe tout le répertoire python/, y compris logs/ (journaux d'exécution) et data/ (documents d'exemple) sous /usr/share/kylin-ai-document-service/python.
- [CONSTATÉ] Code gRPC généré inclus (document_pb2.py, document_pb2_grpc.py).

### `kylin-ai-document-qa-service`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer wangyan@kylinos.cn ; Homepage gitlab2.kylin.com/kylinos-src/kylin-ai-document-qa-service.git)
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 360 Ko, en-têtes 79 Ko, XML 15 Ko, Python 2 Ko, Shell 2 Ko — 116 fichiers / 548 963 octets ; 74 fichiers de code (444 Ko) — lignes de code propres mesurées : 10 973
- **Privilège** : Deux services utilisateur de session (systemd --user) ; postinst root limité à l'activation utilisateur
- **Déclencheurs** : démarrage de session (default.target utilisateur ; seul document-qa est activé par postinst), requêtes IPC (runtime, libkysdk-advanced-ai, clients knowledgebase)
- **Indices réseau** : serveurs GDBus sur sockets Unix abstraites (via framework); clients gRPC vers vector-engine et document-service ; appels NLP (kysdk-genai-nlp)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Deux services propres installés par l'ISO (74 fichiers) qui ingèrent des documents utilisateur dans une base de connaissances avec Python embarqué et IPC : collecte de données ; tests exclus.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:document-qa-service/conf/kylin-ai-document-qa-service.service` | ExecStart=/usr/bin/kylin-ai-document-qa-service -s KnowledgeBase ; Restart=always ; WantedBy=default.target | CONSTATÉ |
| systemd-user-unit | `archive:knowledge-base-service/conf/kylin-ai-knowledgebase-service.service` | ExecStart=/bin/bash -c 'source /usr/share/kylin-ai-python-env/python-env/bin/activate && /usr/bin/kylin-ai-knowledge-base-service -s knowledgebaseservice' ; Restart=always ; WantedBy=default.target | CONSTATÉ |
| maintainer-script | `archive:debian/kylin-ai-document-qa-service.postinst` | deb-systemd-helper --user enable kylin-ai-document-qa-service.service (aucun postinst pour kylin-ai-knowledge-base-service) | CONSTATÉ |
| debian-install | `archive:debian/kylin-ai-knowledge-base-service.install` | knowledgebaseservice.json, unité utilisateur, prompts/*, /usr/bin/kylin-ai-knowledge-base-service | CONSTATÉ |
| debian-install | `archive:debian/kylin-ai-document-qa-service.install` | KnowledgeBase.json, unité utilisateur, /usr/bin/kylin-ai-document-qa-service ; lib/-dev : libkyai-knowledgebase-client | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_subdirectory document-qa-service, document-qa-client, knowledge-base-service (defaultknowledgebase non construit) ; knowledge-base-service lie Python3 Development, Xapian, Qt5Core, libcrypto, kyai-business-framework, kysdk-vector-engine-client | CONSTATÉ |
| config-declarative | `archive:knowledge-base-service/conf/knowledgebaseservice.json` | service_name KnowledgeBaseService ; chemins de prompts ; retrieval HYBRID ; KnowledgeBase.json : service_name Knowledgebase | CONSTATÉ |
| listener-ipc | `archive:knowledge-base-service/server.cpp` | serveurs via libkyai-business-framework : unix:abstract=/tmp/.kylin-ai-business-unix/<euid>/KnowledgeBaseService.sock et .../Knowledgebase.sock (déduit des service_name) | PROBABLE |

- [CONSTATÉ] L'unité kylin-ai-knowledgebase-service porte WantedBy=default.target mais aucun script ne l'active (compat debhelper 12).
- [CONSTATÉ] Le sous-répertoire defaultknowledgebase/ (main.cpp, service.cpp) n'est pas inclus dans la construction.

### `kylin-ai-vector-engine`

- **Nature** : fork-amont — **Amont** : Milvus Lite (github.com/milvus-io/milvus-lite, Homepage du control ; README « A lightweight vector database fork from Milvus Lite ») ; version amont non indiquée ; tiers embarqués : milvus internal/core, knowhere v2.3.1, folly, arrow, milvus-storage, simdjson 3.1.7
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète 3.0 native incluant thirdparty/) ; Gitee présent mais non acquis
- **Langages / taille** : en-têtes 9,2 Mo, C++ 7,8 Mo, Python 1,6 Mo, JSON 642 Ko, CMake 203 Ko, Protobuf 85 Ko, Shell 74 Ko — 2 582 fichiers / 23 136 640 octets ; delta propre src/ = 66 fichiers / 549 Ko ; thirdparty ~2 477 fichiers — lignes de code propres mesurées : 539 853
- **Privilège** : Service utilisateur de session (systemd --user) ; postinst/postrm root (activation utilisateur ; suppression de bases dans /home/* à la purge)
- **Déclencheurs** : démarrage de session (default.target utilisateur), requêtes gRPC sur /tmp/kylin-ai-vector-engine-<euid>.sock, purge du paquet (postrm)
- **Indices réseau** : serveur gRPC sur socket Unix /tmp, identifiants InsecureServerCredentials (src/server.cpp l.51); indices bind/listen dans thirdparty/folly et faiss contrib/rpc.py (code tiers, non construits comme serveurs dans src/ : PROBABLE)
- **Blobs** : aucun ; milvus_lite_log.png ; lien symbolique interne thirdparty/.../hnswlib/python_bindings/setup.py -> ../setup.py
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Fork volumineux : lecture complète du delta Kylin src/ (66 fichiers : serveur gRPC, stockage SQLCipher) et vérification par diff de thirdparty/ contre les versions amont milvus/knowhere/folly/simdjson.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-user-unit | `archive:conf/kylin-ai-vector-engine.service` | ExecStart=/usr/bin/kylin-ai-vector-engine INFO /usr/share/kylin-ai-vector-engine/vector_engine.json ; Restart=always ; WantedBy=default.target ; installé dans /usr/lib/systemd/user | CONSTATÉ |
| maintainer-script | `archive:debian/postinst` | deb-systemd-helper --user enable kylin-ai-vector-engine.service | CONSTATÉ |
| maintainer-script | `archive:debian/postrm` | sur purge : for i in $(ls /home) ; rm /home/$i/.local/share/kylin-ai-vector-engine/kylin-ai-vector-engine.db | CONSTATÉ |
| listener-ipc | `archive:src/server.cpp` | grpc::ServerBuilder ; AddListeningPort(unix:/tmp/kylin-ai-vector-engine-<euid>.sock, InsecureServerCredentials()) ; SetMaxReceiveMessageSize 256 Mio ; MilvusServiceImpl | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_subdirectory thirdparty/folly, knowhere, simdjson, milvus core (log, config, common, query, exec, index, segcore, bitset), src ; install binaire /usr/bin, unité, conf/vector_engine.json (max_count 400000, release_time 300) | CONSTATÉ |
| debian-control | `archive:debian/control` | Architecture amd64 arm64 ; Build-Depends libgrpc++-dev, libsqlcipher-dev, libsqlitecpp-dev, libboost-all-dev, libantlr4-runtime-dev... ; Section x11 ; debian/rules dh $@ | CONSTATÉ |
| chiffrement | `archive:src/milvus_proxy.cpp` | chaîne littérale "PRAGMA key='kylin-ai-vector-engine'" (l.54 et l.131) ; src/storage.cpp l.71 : PRAGMA key='{}' avec key_ | CONSTATÉ |

- [CONSTATÉ] La version précise de Milvus Lite forkée n'est visible ni dans debian/ ni dans le README ; seuls knowhere v2.3.1 (CMakeLists.txt l.119) et simdjson 3.1.7 sont identifiés.
- [CONSTATÉ] Installé par l'ISO (kylin-ai-vector-engine=1.2.0.1-ok0.11).

### `libkysdk-vector-engine-client`

- **Nature** : fork-amont — **Amont** : milvus-sdk-cpp (github.com/milvus-io/milvus-sdk-cpp, README « modify from milvus-sdk-cpp » ; en-têtes LF AI & Data, Apache-2.0) ; version amont non indiquée
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète 3.0 native) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 205 Ko, en-têtes 93 Ko — 70 fichiers / 318 337 octets ; 55 fichiers de code — lignes de code propres mesurées : 8 207
- **Privilège** : Bibliothèque chargée par runtime, data-management, document-qa (utilisateur)
- **Déclencheurs** : chargement par les services/applications consommateurs
- **Indices réseau** : client gRPC vers socket Unix /tmp (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Fork client de taille modeste installé par l'ISO touchant au chiffrement SQLCipher : lire les ajouts Kylin (tools/, ConnectParam, encrypt) et diff contre milvus-sdk-cpp pour le reste.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-vector-engine-client et -dev (amd64 arm64) ; Build-Depends libgrpc++-dev, libsqlcipher-dev, libsqlitecpp-dev ; Homepage gitlab2.kylin.com ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-vector-engine-client.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-vector-engine-client-dev.install` | usr/include/* ; usr/lib/*/lib*.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kysdk-vector-engine-client SHARED) ; install usr/ -> /usr ; demo/ optionnel | CONSTATÉ |
| client-ipc | `archive:src/types/ConnectParam.cpp` | URI par défaut unix:/tmp/kylin-ai-vector-engine-<euid>.sock (l.25) | CONSTATÉ |
| chiffrement | `archive:src/tools/DatabaseTools.cpp` | SELECT sqlcipher_export('encrypted') (outil de conversion de base chiffrée) ; demo/encrypt/main.cpp | CONSTATÉ |

- [NON COUVERT] Gitee présent mais non acquis ; version amont de milvus-sdk-cpp à établir.

### `kysdk-ai-common`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native ; fiche : présent dans Debian et Ubuntu)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee présent mais non acquis
- **Langages / taille** : en-têtes 10 Ko, C++ 3 Ko — 18 fichiers / 56 274 octets ; 5 fichiers de code — lignes de code propres mesurées : 304
- **Privilège** : Bibliothèque utilitaire (codes d'erreur, macros) chargée par les composants IA
- **Déclencheurs** : chargement par les composants consommateurs
- **Indices réseau** : aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 5 fichiers de code (12 Ko) installés par l'ISO, sans IPC ni réseau : lecture complète triviale.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-ai-common et libkysdk-ai-common-dev (Depends dpkg) ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-ai-common.install` | usr/lib/*/*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-ai-common-dev.install` | usr/include/kylin-ai/* ; *.so ; usr/share/*/cmake/*.cmake ; usr/share/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(KysdkAiCommon SHARED) ; install include/kylin-ai et usr/ -> /usr | CONSTATÉ |

- [NON COUVERT] Gitee présent mais non acquis.

### `libkysdk-mcp`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native) ; embarque cpp-httplib 0.19.0 (src/common/httplib.h) et nlohmann/json 3.11.3 (src/common/json.hpp)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; aucune forge
- **Langages / taille** : C++ 2,27 Mo (dont json.hpp 920 Ko, tests et exemples), en-têtes 553 Ko (dont httplib.h 351 Ko), Shell 6 Ko, CMake, YAML — 120 fichiers / 3 009 946 octets ; 80 fichiers de code — lignes de code propres mesurées : 77 928
- **Privilège** : Bibliothèque ; privilège et exposition déterminés par le programme qui instancie le serveur MCP (aucun consommateur dans ce lot)
- **Déclencheurs** : instanciation d'un serveur/client MCP par une application tierce
- **Indices réseau** : serveur HTTP/HTTPS/SSE (cpp-httplib) sur hôte/port configurables (src/net/netio.cpp); transport IPC par tubes (src/ipc/pipe_communication.cpp); indices bind() dans tests/server/*_gtest.cpp (tests uniquement)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Bibliothèque propre installée par l'ISO implémentant un serveur réseau HTTP(S)/SSE et la crypto TLS : lecture complète de src/ hors json.hpp/httplib.h (à vérifier par diff contre les versions amont), tests et exemples en triage.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-mcp (libkysdk-mcp-client.so.* et libkysdk-mcp-server.so.*) et -dev ; Build-Depends libssl-dev ; Rules-Requires-Root: no | CONSTATÉ |
| debian-rules | `archive:debian/rules` | hardening=+all ; LDFLAGS -z relro -z now ; dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-mcp.install` | usr/lib/*/libkysdk-mcp-client.so.* ; usr/lib/*/libkysdk-mcp-server.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-mcp-dev.install` | usr/include/kylin-ai/mcp ; .so ; pkgconfig kysdk-mcp-client.pc et kysdk-mcp-server.pc | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kysdk_mcp_client SHARED) et add_library(kysdk_mcp_server SHARED) ; find_package(OpenSSL) ; exemples OFF par défaut ; suites de tests ON | CONSTATÉ |
| listener-reseau | `archive:src/net/netio.cpp` | httplib::Server (l.324) ; server_->listen(config.host, config.port) (l.623) ; endpoints configurables + SSE | CONSTATÉ |

- [CONSTATÉ] Le dépôt contient AGENTS.md (consignes pour un agent de génération de code « Codex ») et un répertoire openspec/.
- [CONSTATÉ] Aucune forge publique recensée.

### `libkysdk-advanced-ai`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) + gitee (branche openkylin/huanghe) identiques (diff -rq : seul .git diffère)
- **Langages / taille** : C++ 196 Ko, en-têtes 82 Ko ; « other » 143 Ko dont 中文接口文档.md (132 Ko) — 57 fichiers / 420 472 octets ; 45 fichiers de code — lignes de code propres mesurées : 8 006
- **Privilège** : Bibliothèque chargée dans le processus appelant (utilisateur)
- **Déclencheurs** : chargement par une application (ex. kylin-aiassistant, Build-Depends libkysdk-advanced-ai-dev)
- **Indices réseau** : client GDBus sur socket Unix abstraite vers kylin-ai-knowledge-base-service (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Bibliothèque cliente installée par l'ISO, sans service propre ; l'effort porte sur le serveur knowledge-base.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-advanced-ai0 et -dev (amd64 arm64) : « KnowledgeBase functionality » ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-advanced-ai0.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-advanced-ai-dev.install` | usr/include/* ; *.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kysdk-advanced-ai SHARED) ; tests/ optionnel ; install usr/ -> /usr | CONSTATÉ |
| client-ipc | `archive:src/knowledgebaseserver.cpp` | adresse unix:abstract=/tmp/.kylin-ai-business-unix/<uid>/KnowledgeBaseService.sock (l.4-6) ; usage client | CONSTATÉ |
| debian-control | `gitee:debian/control` | identique à l'archive | CONSTATÉ |

- [CONSTATÉ] Archive et Gitee strictement identiques hors métadonnées git.

### `libkyai-assistant`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 39 Ko, en-têtes 13 Ko ; « other » 553 Ko dont test/1.png (505 Ko) — 26 fichiers / 604 873 octets ; 12 fichiers de code — lignes de code propres mesurées : 1 719
- **Privilège** : Bibliothèque chargée dans le processus appelant (kylin-aiassistant notamment)
- **Déclencheurs** : chargement par kylin-aiassistant et autres clients
- **Indices réseau** : client GDBus pair-à-pair vers kylin-ai-runtime (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Petite bibliothèque cliente (12 fichiers) installée par l'ISO, sans service ni réseau.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkyai-assistant0 et -dev ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkyai-assistant0.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkyai-assistant-dev.install` | usr/include/* ; *.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kyai-assistant SHARED) ; test/ optionnel | CONSTATÉ |
| client-ipc | `archive:serverproxy.cpp` | client unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/assistant.sock (l.21-22) | CONSTATÉ |

- [CONSTATÉ] Copie divergente dans libkysdk-ai-private/ai-assistant.

### `libkyai-config`

- **Nature** : propre-kylin — **Amont** : Kylin (Maintainer guopengfei@kylinos.cn ; format 3.0 native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee présent mais non acquis
- **Langages / taille** : C++ 47 Ko, en-têtes 8 Ko — 17 fichiers / 97 938 octets ; 5 fichiers de code — lignes de code propres mesurées : 1 571
- **Privilège** : Bibliothèque chargée par kylin-ai-subsystem-modelconfig, kylin-aiassistant (utilisateur)
- **Déclencheurs** : chargement par les clients de configuration de modèles
- **Indices réseau** : client GDBus pair-à-pair vers kylin-ai-runtime (aucun listener)
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 5 fichiers de code installés par l'ISO véhiculant la configuration des modèles (potentiellement des identifiants d'API) : lecture complète peu coûteuse.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkyai-config0 et -dev : « model configuration functionality » ; debian/rules dh $@ | CONSTATÉ |
| debian-install | `archive:debian/libkyai-config0.install` | usr/lib/*/lib*.so.* | CONSTATÉ |
| debian-install | `archive:debian/libkyai-config-dev.install` | usr/include/* ; *.so ; usr/share/pkgconfig/* | CONSTATÉ |
| cmake-cible | `archive:CMakeLists.txt` | add_library(kyai-config SHARED) ; tests/ optionnel | CONSTATÉ |
| client-ipc | `archive:configserviceproxy.cpp` | client unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/config.sock (l.28-29) | CONSTATÉ |

- [NON COUVERT] Le contenu exact des configurations transmises (clés d'API éventuelles) n'a pas été examiné en phase 0.
- [CONSTATÉ] Copie divergente dans libkysdk-ai-private/ai-config.

### `kylin-cua`

- **Nature** : absent-ou-vide — **Amont** : inconnu (aucune métadonnée source dans la fiche)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun (chemin_archive et chemin_gitee null ; forges ABSENT ; find -iname '*kylin-cua*' sous $AUDIT_SRC : aucun résultat)
- **Langages / taille** : inconnu — 0 fichier disponible — lignes de code propres mesurées : 0
- **Privilège** : Indéterminé
- **Déclencheurs** : indéterminés (binaire installé par défaut par l'ISO)
- **Indices réseau** : indéterminés
- **Blobs** : indéterminé
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Binaire installé par défaut sans aucune source disponible : acquérir le .deb et l'analyser en binaire avant toute estimation.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| aucun | `aucun` | aucune source ni paquet source dans archive/, gitee/ ou dl/ | CONSTATÉ |
| binaire-iso | `aucun (binaire kylin-cua=1.0.0 de l'ISO)` | unités, services D-Bus, scripts mainteneur et privilèges du binaire installé non observables | NON COUVERT |

- [CONSTATÉ] Paquet présent dans installe_par_defaut_iso (kylin-cua=1.0.0) sans source correspondante dans le corpus : angle mort de couverture.

#### Constats transverses du lot B01-IA-coeur

- **[CONSTATÉ]** Aucun service système root dans le lot : toutes les unités systemd sont des unités utilisateur (installées dans /usr/lib/systemd/user ou fournies sous configs/lib/systemd/user) : kylin-ai-runtime (non installée), kylin-ai-business-framework-service, kyai-data-management-service, kylin-ai-document-service, kylin-ai-document-qa-service, kylin-ai-knowledgebase-service, kylin-ai-vector-engine. — *preuve* : archive:kylin-ai-data-management-service/CMakeLists.txt l.65 « install(FILES conf/kyai-data-management-service.service DESTINATION /usr/lib/systemd/user) » ; idem kylin-ai-vector-engine/CMakeLists.txt, kylin-ai-document-qa-service/*/CMakeLists.txt, kylin-ai-document-service/CMakeLists.txt l.48, kylin-ai-business-framework-service/CMakeLists.txt l.86
- **[CONSTATÉ]** 4 scripts postinst activent des unités utilisateur pour tous les utilisateurs via deb-systemd-helper --user enable. — *preuve* : archive:kylin-ai-business-framework-service/debian/kylin-ai-business-framework-service.postinst, kylin-ai-data-management-service/debian/kyai-data-management-service.postinst, kylin-ai-document-qa-service/debian/kylin-ai-document-qa-service.postinst, kylin-ai-vector-engine/debian/postinst : « deb-systemd-helper --user enable <unité>.service »
- **[CONSTATÉ]** L'IPC du sous-système passe par des serveurs pair-à-pair sur sockets Unix sous /tmp plutôt que par le bus de session : GDBusServer (unix:path et unix:abstract) avec GDBusAuthObserver nul, et gRPC avec identifiants non sécurisés. — *preuve* : archive:kylin-ai-runtime/src/server/abstractserver.cpp l.44-50 g_dbus_server_new_sync("unix:path="..., nullptr) ; libkyai-business-framework/src/common/constant.cpp l.21 « unix:abstract=/tmp/.kylin-ai-business-unix/ » + businessServer.cpp l.42 ; kylin-ai-business-framework-service/src/server.cpp l.24-25 ; kylin-ai-vector-engine/src/server.cpp l.51 InsecureServerCredentials ; kylin-ai-document-service/python/server.py l.219-220 add_insecure_port
- **[CONSTATÉ]** Le champ listen de l'extraction automatique est vide pour 4 paquets où un serveur est constaté : l'extracteur ne reconnaît ni g_dbus_server_new_sync ni le gRPC Python (grpc.server/add_insecure_port). — *preuve* : fiche B01 : listen [] pour kylin-ai-runtime, libkyai-business-framework, kylin-ai-business-framework-service, kylin-ai-document-service ; serveurs lus dans les fichiers cités ci-dessus
- **[CONSTATÉ]** Deux scripts postrm (exécutés en root) parcourent /home/* et suppriment des fichiers dans les répertoires personnels. — *preuve* : archive:kylin-aiassistant/debian/kylin-aiassistant.postrm « for user in /home/* ... rm $user$path$filename » (~/.config/kylin-aiassistant/settings.ini, remove et purge) ; kylin-ai-vector-engine/debian/postrm « for i in $(ls /home) ... rm /home/$i/$DB_FILE » (purge)
- **[CONSTATÉ]** Deux actions polkit dans le lot, toutes deux en auth_admin ; seule celle de kylin-ai-subsystem-modelconfig est installée. — *preuve* : archive:kylin-ai-subsystem-modelconfig/data/org.kylin-ai-subsystem-modelconfig.policy (polkitflie.path = /usr/share/polkit-1/actions dans le .pro) ; archive:kylin-aiassistant/kylin-aiassistant/data/org.kylin.aiassistant.policy listé uniquement dans DISTFILES
- **[CONSTATÉ]** La langue « TypeScript » attribuée par l'extracteur correspond en grande partie à des fichiers de traduction Qt Linguist (.ts), ce qui surestime le volume de code. — *preuve* : fiche : plus gros fichiers de kylin-ai-subsystem-plugin = translations/mn.ts, bo_CN.ts... ; kylin-ai-subsystem-modelconfig = translations/.../Internationalization_*.ts
- **[CONSTATÉ]** Trois paquets sans code source exploitable alors que des binaires associés figurent sur l'ISO ou en dépendent : kylin-ai-cloud-model (squelette dh_make, 3 binaires sur l'ISO), libkylin-ai-default-key (squelette dh_make avec métadonnées de binaires compilés), kylin-cua (aucune source, binaire kylin-cua=1.0.0 sur l'ISO). — *preuve* : archive:kylin-ai-cloud-model/debian/postinst.ex, manpage.1.ex... ; archive:libkylin-ai-default-key/debian/control (4 strophes Architecture riscv64/arm64/loong64/amd64 avec Depends libc6, libjsoncpp25) ; fiche kylin-cua : chemin_archive null, chemin_gitee null, installe_par_defaut_iso [kylin-cua=1.0.0]
- **[CONSTATÉ]** Code tiers embarqué dans plusieurs paquets propres, à vérifier par diff contre les versions amont plutôt qu'à lire. — *preuve* : kylin-ai-vector-engine/thirdparty/{milvus,folly,arrow,simdjson (SIMDJSON_VERSION 3.1.7),milvus-storage} + KNOWHERE_VERSION v2.3.1 ; libkysdk-mcp/src/common/httplib.h (CPPHTTPLIB_VERSION 0.19.0) et json.hpp (3.11.3) ; libkyai-business-framework/src/sqlitecpp ; kylin-ai-data-management-service/src/sqliteRecover/sqlite3recover.c ; kylin-ai-runtime/third-party/concurrentqueue ; kylin-ai-business-framework-service/python-depends (204 .whl)
- **[CONSTATÉ]** Plusieurs paquets déclarent une Homepage sur des forges internes Kylin (gitlab2.kylin.com, code.kylinos.cn). — *preuve* : debian/control de libkyai-business-framework, kylin-ai-data-management-service, libkyai-data-management-client, kylin-ai-document-service, kylin-ai-document-qa-service, libkysdk-vector-engine-client (gitlab2.kylin.com/kylinos-src/...) ; kylin-ai-cloud-model (code.kylinos.cn/plugins/gitiles/desktop/kylin-ai-cloud-model)
- **[CONSTATÉ]** Le lot contient des copies divergentes du même code : libkysdk-ai-private agrège des variantes de libkyai-assistant, libkyai-config et libkyai-data-management-client ; libkylin-ai-system-private agrège des variantes de libkylin-ai-engine-dev et libkylin-ai-proto ; gitee/kylin-ai-sdk porte le source libkylin-ai-base. — *preuve* : diff -rq archive:libkysdk-ai-private/ai-config archive:libkyai-config (configserviceproxy.cpp differ) ; diff -rq libkylin-ai-system-private/ai-engine/include libkylin-ai-engine-dev/include (7 fichiers differ) ; gitee:kylin-ai-sdk/debian/control « Source: libkylin-ai-base »
- **[CONSTATÉ]** 15 paquets du lot ont un dépôt Gitee recensé PRESENT mais non acquis (chemin_gitee null), empêchant la comparaison archive/Gitee. — *preuve* : fiche B01 : forges.gitee PRESENT et chemin_gitee null pour kylin-ai-runtime, kylin-ai-subsystem-plugin, kylin-ai-subsystem-modelconfig, kylin-ai-business-framework-service, libkylin-ai-base, kylin-aiassistant, kylin-ai-data-management-service, libkyai-data-management-client, kylin-ai-document-service, kylin-ai-document-qa-service, kylin-ai-vector-engine, libkysdk-vector-engine-client, kysdk-ai-common, libkyai-assistant, libkyai-config
- **[CONSTATÉ]** 20 des 28 entrées du lot ont au moins un binaire installé par défaut par l'ISO 3.0 ; 8 n'en ont aucun (kylin-ai-sdk, libkylin-ai-system-private, libkysdk-ai-private, kylin-ai-business-framework-service, libkylin-ai-engine-dev, libkylin-ai-base, libkylin-ai-proto, libkylin-ai-default-key). — *preuve* : fiche B01 : champ installe_par_defaut_iso

