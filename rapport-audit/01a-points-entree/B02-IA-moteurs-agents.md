# Lot B02-IA-moteurs-agents

> Partie de l'annexe [`01a-points-entree-detail.md`](../01a-points-entree-detail.md). Registres : CONSTATÉ / PROBABLE / NON COUVERT.


### `kylin-deepseek-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (mainteneur liqi@kylinos.cn) ; aucun amont tiers ; format 3.0 (quilt) sans debian/patches (.pc sans patch appliqué)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète, 2 fichiers C++ propres) ; Gitee PRESENT mais arbre non acquis en phase 0
- **Langages / taille** : C++ (3,3 Ko dont test), en-têtes C++ (1 Ko), CMake — 15 fichiers / 48,7 Ko (LICENSE 35,8 Ko) ; code propre hors test : 2 fichiers / 2,1 Ko — lignes de code propres mesurées : 134
- **Privilège** : Aucun privilège propre : module chargé dans le processus hôte du moteur IA Kylin (KylinAiEngine, hors lot) ; privilège de l'hôte non déterminé
- **Déclencheurs** : Chargement dynamique par l'hôte KylinAiEngine lors d'une requête IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin (listen vide) ; réseau délégué à libkylin-deepseek-nlp-engine (api.deepseek.com)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ propres (2,1 Ko) reliant l'hôte IA à un moteur à accès réseau authentifié ; lecture exhaustive en moins d'une heure.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Binaire unique libkylin-deepseek-ai-engine-plugin (libs, Multi-Arch same) ; B-D libkylin-ai-engine-dev(>=1.2.0.0-ok0.4), libkylin-deepseek-nlp-engine-dev(>=1.1.0.1-ok0.8) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' sans override ; aucun script mainteneur dans debian/ | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(deepseek-engine-plugin AI_ENGINE deepseekengine.h deepseekengine.cpp) ; target_link_libraries deepseek-nlp-engine ; tests seulement si ENABLE_TEST | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO 3.0 : libkylin-deepseek-ai-engine-plugin=1.1.0.1-ok0.4 ; changelog de tête huanghe (fanhaobo, 16/07/2026, « build for huanghe »).
- [NON COUVERT] Comparaison archive/Gitee non réalisée (arbre Gitee non acquis).

### `kylin-deepseek-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (liqi@kylinos.cn) ; embarque la bibliothèque tierce cpr (C++ Requests, surcouche libcurl) dans third-party/cpr avec CMakeLists réécrit, version amont non indiquée, compilée STATIC
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 105 Ko, en-têtes 96 Ko (majoritairement cpr), CMake — 95 fichiers / 260 Ko ; code propre hors test : 8 fichiers / 37 Ko ; third-party/cpr : 75 fichiers — lignes de code propres mesurées : 1 316
- **Privilège** : Bibliothèque partagée exécutée dans le processus hôte (via le plugin deepseek) ; aucun service ni script mainteneur
- **Déclencheurs** : Requête NLP transmise par kylin-deepseek-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Client HTTPS sortant (cpr/libcurl) vers api.deepseek.com; Aucun listener (champ listen vide)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Code propre 37 Ko effectuant des appels réseau authentifiés par clé API ; cpr vendorisé identique dans 11 paquets du lot, à auditer une seule fois (hors de ce pourcentage).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 2 binaires : libkylin-deepseek-nlp-engine (libs) et -dev ; B-D libjsoncpp-dev, libcurl4-openssl-dev, libkylin-ai-engine-dev(>=1.2.0.0-0k0.7), libkysdk-ai-common-dev(>=1.1.0.1-ok0.4) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps --dpkg-shlibdeps-params=-xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-deepseek-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : usr/lib/*/lib*.so, usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(deepseek-nlp-engine SHARED deepseeknlpengine_p.cpp, deepseeknlpengine.cpp, servererror.cpp, util.cpp) ; liens cpr (add_subdirectory third-party) + jsoncpp ; en-tête vers include/kylin-ai/plugins/ai-engines ; SOVERSION 1 | CONSTATÉ |
| client-reseau | `archive:deepseeknlpengine_p.cpp` | Chaînes https://api.deepseek.com et https://platform.deepseek.com relevées par grep (logique non lue) | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-deepseek-nlp-engine=1.1.0.1-ok0.12.
- [CONSTATÉ] third-party/cpr/cpr/session.cpp identique (md5 aca9e8bc…) aux 10 autres copies du lot.

### `kylin-qwen-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (linyuxuan@kylinos.cn) ; aucun amont tiers ; format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ (4,3 Ko dont test), en-têtes (1 Ko), CMake — 13 fichiers / 47,6 Ko ; code propre : 2 fichiers / 2,1 Ko — lignes de code propres mesurées : 167
- **Privilège** : Aucun privilège propre : module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; réseau délégué à libkylin-qwen-nlp-engine (dashscope.aliyuncs.com)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ (2,1 Ko) de pont vers un moteur cloud ; lecture exhaustive triviale.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Binaire libkylin-qwen-ai-engine-plugin (libs, Multi-Arch same) ; B-D libkylin-ai-engine-dev(>=1.2.0.0-ok0.4), libkylin-qwen-nlp-engine-dev(>=1.1.0.1-ok0.8) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' sans override ; aucun script mainteneur | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(qwen-engine-plugin AI_ENGINE qwenengine.h qwenengine.cpp) ; lié à qwen-nlp-engine | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-qwen-ai-engine-plugin=1.1.0.2-ok0.5 ; changelog de tête huanghe 16/07/2026.

### `kylin-qwen-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (linyuxuan) ; embarque cpr (third-party/cpr, STATIC, version non indiquée)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 122 Ko, en-têtes 97 Ko (majoritairement cpr), CMake — 95 fichiers / 278 Ko ; code propre hors test : 8 fichiers / 38 Ko — lignes de code propres mesurées : 1 780
- **Privilège** : Bibliothèque partagée dans le processus hôte ; aucun service
- **Déclencheurs** : Requête NLP via kylin-qwen-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers dashscope.aliyuncs.com (Alibaba); Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Code propre 38 Ko avec appels réseau authentifiés ; cpr mutualisé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-qwen-nlp-engine + -dev ; B-D libjsoncpp-dev, libkylin-ai-engine-dev(>=1.2.0.0-0k0.7), libkysdk-ai-common-dev, libcurl4-openssl-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul (pas d'override -xlibcurl4, contrairement aux autres moteurs cloud) | CONSTATÉ |
| debian-install | `archive:debian/libkylin-qwen-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(qwen-nlp-engine SHARED qwennlpengine_p.cpp, qwennlpengine.cpp, servererror.cpp, util.cpp) ; liens cpr + jsoncpp | CONSTATÉ |
| client-reseau | `archive:qwennlpengine_p.cpp` | Chaîne https://dashscope.aliyuncs.com relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-qwen-nlp-engine=1.1.0.1-ok0.12.

### `kylin-sensetime-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (zhenshaofeng@kylinos.cn) ; format 3.0 (native)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ (4,3 Ko dont test), en-têtes, CMake — 14 fichiers / 51,9 Ko ; code propre : 2 fichiers / 2,1 Ko — lignes de code propres mesurées : 168
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; délégué à libkylin-sensetime-nlp-engine (api.sensenova.cn)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ (2,1 Ko) ; non installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-sensetime-ai-engine-plugin ; Depends libkylin-sensetime-nlp-engine ; Homepage gitee.com/openkylin/kylin-sensetime-ai-engine-plugin | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(sensetime-engine-plugin AI_ENGINE sensetimeengine.*) ; include_directories(third-party/cpr/include) alors qu'aucun répertoire third-party n'existe dans l'arbre | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO 3.0 (installe_par_defaut_iso vide).

### `kylin-sensetime-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (zhenshaofeng) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C/C++ (en-têtes 97 Ko, C++ 93 Ko, majoritairement cpr), CMake — 99 fichiers / 260 Ko ; code propre hors test : 10 fichiers / 30,7 Ko — lignes de code propres mesurées : 1 023
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête NLP via le plugin sensetime (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers api.sensenova.cn; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Code propre 31 Ko combinant réseau et signature JWT (identité) ; cpr mutualisé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-sensetime-nlp-engine + -dev ; B-D libjsoncpp-dev, libcurl4-openssl-dev, libssl-dev, libjwt-dev, libkylin-ai-engine-dev(>=1.1.0.0-0k0.2) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-sensetime-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(sensetime-nlp-engine SHARED … token.cpp …) ; liens cpr, jsoncpp, jwt (génération de jetons JWT) | CONSTATÉ |
| client-reseau | `archive:sensetimenlpengine_p.cpp` | Chaîne https://api.sensenova.cn relevée par grep | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO.

### `kylin-baidu-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei@kylinos.cn) ; format native
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee et GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 81 Ko (dont cpr et tests), en-têtes 90 Ko, CMake ; 754 Ko de PCM de test — 91 fichiers / 991 Ko (test/16k_10.pcm 754 Ko) ; code propre : 2 fichiers / 2,9 Ko ; third-party/cpr 76 fichiers non compilés — lignes de code propres mesurées : 413
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; délégué aux moteurs Baidu NLP/speech/vision
- **Blobs** : aucun binaire exécutable ; test/16k_10.pcm (audio brut 754 Ko)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ propres (2,9 Ko) ; le reste est cpr non compilé et des tests.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-baidu-ai-engine-plugin ; Depends libkylin-baidu-speech-engine(>=1.0.0.2-ok0.7) ; B-D libkylin-baidu-nlp-engine-dev, libkylin-baidu-vision-engine-dev, libcurl4-openssl-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(baidu-engine-plugin AI_ENGINE baiduengine.*) ; liens jsoncpp, baidu-nlp-engine, baidu-speech-engine, baidu-vision-engine ; third-party/cpr seulement en include_directories (pas d'add_subdirectory) | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-baidu-ai-engine-plugin=1.0.0.2-ok0.6.
- [CONSTATÉ] Fichiers de test contenant des identifiants factices (« fakeApiKey », « fakeSecretKey »).

### `kylin-baidu-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 105 Ko, en-têtes 111 Ko (dont cpr), CMake — 99 fichiers / 285 Ko ; code propre src/ : 12 fichiers / 53 Ko — lignes de code propres mesurées : 1 601
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête NLP via kylin-baidu-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers aip.baidubce.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Code propre 53 Ko avec réseau et gestion de jetons OAuth ; cpr mutualisé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-baidu-nlp-engine + -dev ; B-D libssl-dev, libcurl4-openssl-dev, libjsoncpp-dev, libkysdk-ai-common-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-baidu-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(baidu-nlp-engine SHARED src/baidunlpengine_p.cpp, token.cpp, logger.cpp, util.cpp …) ; liens cpr, jsoncpp | CONSTATÉ |
| client-reseau | `archive:src/baidunlpengine_p.cpp` | Chaînes https://aip.baidubce.com (et cloud.baidu.com) relevées par grep ; token.cpp (obtention de jeton) non lu | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-baidu-nlp-engine=1.1.0.0-ok0.10.

### `kylin-baidu-speech-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; embarque libhv v1.3.3 (third-party/libhv-v1.3.3, ~30 sources compilées directement dans la .so) et cpr (STATIC) ; copie libhv modifiée (http/httpdef.h : type MIME AUDIO_BASIC ajouté par rapport à la copie xunfei)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 1,43 Mo, C 784 Ko, en-têtes 587 Ko (majoritairement libhv), CMake, Shell/Python (libhv) — 468 fichiers / 3,76 Mo ; code propre src/ : 12 fichiers / 82 Ko — lignes de code propres mesurées : 2 922
- **Privilège** : Bibliothèque partagée dans le processus hôte (plugin Baidu)
- **Déclencheurs** : Requête ASR/TTS via kylin-baidu-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Clients HTTPS et WebSocket TLS sortants vers *.baidu.com; Primitives serveur libhv compilées (hloop/hsocket) ; listener Kylin non établi (NON COUVERT)
- **Blobs** : aucun binaire exécutable ; third-party/libhv-v1.3.3/cert/cacert.pem (224 Ko), PNG de doc libhv, test/16k.pcm
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 82 Ko de code propre réseau + ~30 sources libhv compilées à comparer à libhv v1.3.3 amont (copie modifiée).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Binaire unique libkylin-baidu-speech-engine (pas de -dev) ; B-D libssl-dev, libcurl4-openssl-dev, libjsoncpp-dev, libkysdk-ai-common-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 ; aucun .install (tout DESTDIR va dans le binaire unique) | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(baidu-speech-engine SHARED) incluant libhv base/hsocket.c, event/hloop.c, ssl/openssl.c+hssl.c, http/client/{AsyncHttpClient,HttpClient,WebSocketClient}.cpp, http_parser.c, websocket_parser.c… + src/* ; -DWITH_OPENSSL ; liens cpr, jsoncpp, OpenSSL | CONSTATÉ |
| indice-ecoute (bibliothèque) | `archive:third-party/libhv-v1.3.3/event/hloop.c` | l.1019 bind() dans la branche 'if (side == HIO_SERVER_SIDE)' (≈15 lignes lues) : capacité serveur générique de libhv compilée dans la .so ; base/hsocket.c bind() idem ; usage effectif par le code Kylin non vérifié | CONSTATÉ |
| client-reseau | `archive:src/baiduspeechengine_p.cpp` | Chaînes wss://vop.baidu.com, https://vop.baidu.com, https://tsn.baidu.com, https://openapi.baidu.com relevées par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-baidu-speech-engine=1.0.0.2-ok0.10.
- [CONSTATÉ] test/test_baidu_speech_engine.cpp contient une clé de configuration de test (valeur « secretKey » placeholder).

### `kylin-baidu-vision-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : en-têtes 114 Ko, C++ 107 Ko (dont cpr), CMake — 103 fichiers / 289 Ko ; code propre src/ : 16 fichiers / 62 Ko — lignes de code propres mesurées : 1 701
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête vision via le plugin Baidu (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers aip.baidubce.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 62 Ko de code propre : décodage d'images (FreeImage) + envoi réseau authentifié.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-baidu-vision-engine + -dev ; B-D libfreeimage-dev, libssl-dev, libcurl4-openssl-dev, libjsoncpp-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-baidu-vision-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(baidu-vision-engine SHARED … imageloader.cpp, baiduresponseprocessor.cpp, token.cpp …) ; liens cpr, jsoncpp, freeimage, OpenSSL | CONSTATÉ |
| client-reseau | `archive:src/baiduvisionengine_p.cpp` | Chaîne https://aip.baidubce.com relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-baidu-vision-engine=1.0.0.2-ok0.10.

### `kylin-xunfei-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; format native
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 81 Ko (dont cpr et tests), en-têtes 90 Ko, CMake — 91 fichiers / 990 Ko (test/16k_10.pcm 754 Ko) ; code propre : 2 fichiers / 2,9 Ko ; third-party/cpr non compilé — lignes de code propres mesurées : 418
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; délégué aux moteurs iFlytek
- **Blobs** : test/16k_10.pcm (audio brut)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ propres ; reste = tests et cpr inutilisé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-xunfei-ai-engine-plugin ; Depends libkylin-xunfei-speech-engine(>=1.1.0.1-ok0.7) ; B-D xunfei vision/nlp -dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(xunfei-engine-plugin AI_ENGINE xunfeiengine.*) ; liens jsoncpp, xunfei-speech/vision/nlp-engine | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-xunfei-ai-engine-plugin=1.1.0.1-ok0.6.

### `kylin-xunfei-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; pas de bibliothèque vendorisée (libcurl système)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 65 Ko, en-têtes 23 Ko, CMake — 23 fichiers / 136 Ko ; code propre src/ : 12 fichiers / 68 Ko — lignes de code propres mesurées : 2 086
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête NLP via le plugin iFlytek (PROBABLE)
- **Indices réseau** : Client WebSocket TLS sortant vers spark-api.xf-yun.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 68 Ko de code propre réseau (signature de requêtes, WebSocket via libcurl).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-xunfei-nlp-engine + -dev ; B-D libssl-dev, libcurl4-openssl-dev, libjsoncpp-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-xunfei-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(xunfei-nlp-engine SHARED src/*) ; find_package(CURL) ; liens curl, jsoncpp | CONSTATÉ |
| client-reseau | `archive:src/xunfeinlpengine_p.cpp` | Chaîne wss://spark-api.xf-yun.com relevée par grep ; l.35-50 : URL de sites d'actualité en commentaire (exemples de réponse) | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-xunfei-nlp-engine=1.1.0.1-ok0.9.

### `kylin-xunfei-speech-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; embarque libhv v1.3.3 (sources compilées dans la .so) ; pas de cpr
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 1,37 Mo, C 784 Ko, en-têtes 497 Ko (majoritairement libhv), CMake — 389 fichiers / 3,58 Mo ; code propre src/ : 10 fichiers / 93 Ko — lignes de code propres mesurées : 2 964
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête ASR/TTS via le plugin iFlytek (PROBABLE)
- **Indices réseau** : Clients WebSocket TLS sortants vers *.xfyun.cn; Primitives serveur libhv compilées ; listener Kylin non établi (NON COUVERT)
- **Blobs** : aucun binaire exécutable ; cacert.pem et PNG de doc libhv, test/16k.pcm
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 93 Ko de code propre réseau + sous-ensemble libhv compilé à comparer à l'amont v1.3.3.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Binaire unique libkylin-xunfei-speech-engine ; B-D libssl-dev, libjsoncpp-dev, libkysdk-ai-common-dev (pas de libcurl) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 ; aucun .install | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(xunfei-speech-engine SHARED) incluant libhv (hsocket.c, hloop.c, ssl/openssl.c, WebSocketClient.cpp, http_parser.c…) + src/* ; -DWITH_OPENSSL ; target_link_libraries jsoncpp seul | CONSTATÉ |
| indice-ecoute (bibliothèque) | `archive:third-party/libhv-v1.3.3/event/hloop.c` | bind() côté HIO_SERVER_SIDE (copie libhv identique au constat baidu, hors http/httpdef.h) ; usage par le code Kylin non vérifié | CONSTATÉ |
| client-reseau | `archive:src/xunfeispeechengine_p.cpp` | Chaînes wss://iat-api.xfyun.cn, wss://rtasr.xfyun.cn, wss://tts-api.xfyun.cn relevées par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-xunfei-speech-engine=1.1.0.1-ok0.8.

### `kylin-xunfei-vision-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : en-têtes 113 Ko, C++ 101 Ko (dont cpr), CMake — 101 fichiers / 282 Ko ; code propre src/ : 14 fichiers / 54 Ko — lignes de code propres mesurées : 1 426
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête vision via le plugin iFlytek (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers spark-api.cn-huabei-1.xf-yun.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 54 Ko de code propre : décodage d'images + réseau authentifié.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-xunfei-vision-engine + -dev ; B-D libfreeimage-dev, libssl-dev, libcurl4-openssl-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-xunfei-vision-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(xunfei-vision-engine SHARED … imageloader.cpp, token.cpp …) ; liens cpr, jsoncpp, freeimage | CONSTATÉ |
| client-reseau | `archive:src/xunfeivisionengine_p.cpp` | Chaîne https://spark-api.cn-huabei-1.xf-yun.com relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-xunfei-vision-engine=1.1.0.1-ok0.10.

### `kylin-baichuan-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (linyuxuan) ; format 3.0 (quilt) sans patches
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ (4,3 Ko dont test), en-têtes, CMake — 15 fichiers / 49,8 Ko ; code propre : 2 fichiers / 2,1 Ko — lignes de code propres mesurées : 168
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; délégué à libkylin-baichuan-nlp-engine
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ ; non installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-baichuan-ai-engine-plugin ; B-D libkylin-baichuan-nlp-engine-dev(>=1.1.0.0-0k0.3) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(baichuan-engine-plugin AI_ENGINE baichuanengine.*) ; lié à baichuan-nlp-engine | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO.

### `kylin-baichuan-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (linyuxuan) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : en-têtes 96 Ko, C++ 88 Ko (dont cpr), CMake — 97 fichiers / 249 Ko ; code propre : 8 fichiers / 24 Ko — lignes de code propres mesurées : 842
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête NLP via le plugin baichuan (PROBABLE)
- **Indices réseau** : Client HTTPS sortant vers api.baichuan-ai.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 24 Ko de code propre réseau ; cpr mutualisé ; non installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-baichuan-nlp-engine + -dev ; B-D libcurl4-openssl-dev, libjsoncpp-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-baichuan-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(baichuan-nlp-engine SHARED …) ; liens cpr, jsoncpp | CONSTATÉ |
| client-reseau | `archive:baichuannlpengine_p.cpp` | Chaîne https://api.baichuan-ai.com relevée par grep | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO.

### `kylin-custom-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (liqi) ; format 3.0 (quilt) sans patches
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ (4,7 Ko dont test), en-têtes, CMake — 14 fichiers / 48,7 Ko ; code propre : 2 fichiers / 2,1 Ko — lignes de code propres mesurées : 182
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; délégué à libkylin-custom-nlp-engine (URL configurable)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ de pont vers un moteur à URL arbitraire configurable.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-custom-ai-engine-plugin ; B-D libkylin-custom-nlp-engine-dev(>=1.1.0.1-ok0.8) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(custom-engine-plugin AI_ENGINE customengine.*) ; lié à custom-nlp-engine | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-custom-ai-engine-plugin=1.1.0.1-ok0.3update1 ; entrée de tête du changelog en distribution « nile » (10/06/2026), pas huanghe.

### `kylin-custom-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (liqi) ; embarque cpr (STATIC)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 110 Ko, en-têtes 97 Ko (dont cpr), CMake — 95 fichiers / 266 Ko ; code propre hors test : 8 fichiers / 42 Ko — lignes de code propres mesurées : 1 453
- **Privilège** : Bibliothèque partagée dans le processus hôte
- **Déclencheurs** : Requête NLP via le plugin custom (PROBABLE)
- **Indices réseau** : Client HTTP(S) sortant vers une URL fournie par configuration (apiUrl) ; schéma http:// présent dans les exemples de test; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 42 Ko de code propre ; cible réseau arbitraire configurable (http ou https).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-custom-nlp-engine + -dev ; B-D libcurl4-openssl-dev, libjsoncpp-dev, libkylin-ai-engine-dev(>=1.2.0.0-0k0.7) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; override_dh_shlibdeps -xlibcurl4 | CONSTATÉ |
| debian-install | `archive:debian/libkylin-custom-nlp-engine.install` | usr/lib/*/lib*.so.* ; -dev : lib*.so + usr/include/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(custom-nlp-engine SHARED customnlpengine_p.cpp …) ; liens cpr, jsoncpp | CONSTATÉ |
| client-reseau (URL configurable) | `archive:test_custom_nlp_engine.cpp` | Configurations de test avec champ apiUrl : http://10.41.116.100:11434/api/chat, http://10.41.160.151:11434/api/chat (adresses privées, port Ollama), dashscope.aliyuncs.com, api.lkeap.cloud.tencent.com, http://www.baidu.com | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-custom-nlp-engine=1.1.0.1-ok0.9.
- [CONSTATÉ] test_custom_nlp_engine.cpp:31 contient une valeur apiKey au format « sk- » + 32 caractères hexadécimaux (valeur masquée, validité non vérifiée) ; fichier compilé seulement si ENABLE_TEST.

### `kylin-freetrial-ai-engine-plugin`

- **Nature** : absent-ou-vide — **Amont** : Inconnu : aucun code ; Original-Maintainer linyuxuan@kylinos.cn dans debian/control
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (19 fichiers debian/, gabarits .ex) ; Gitee = README.md + README.en.md uniquement (branche upstream, commit 5691cb4 « Initial commit »)
- **Langages / taille** : aucun (fichiers texte debian/) — archive 19 fichiers / 27,7 Ko ; gitee 2 fichiers / 2,3 Ko — lignes de code propres mesurées : 0
- **Privilège** : Indéterminable : le binaire installé par l'ISO n'est pas reconstructible depuis l'arbre source
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE, binaire non examiné)
- **Indices réseau** : NON COUVERT (aucun code)
- **Blobs** : aucun dans l'arbre ; binaire de l'ISO hors arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source dans l'archive ni sur Gitee ; seule une analyse du binaire ISO serait possible (hors périmètre source).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 4 stances 'Package: libkylin-freetrial-ai-engine-plugin' (riscv64, loong64, amd64, arm64) avec Depends résolus (libc6 >= 2.27/2.38/2.2.5/2.17, libkylin-freetrial-nlp-engine >= 1.1.0.0) ; Maintainer kylin <kylin@unknown> ; descriptions gabarit | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul (aucune source à construire) | CONSTATÉ |
| maintainer-script (gabarit inactif) | `archive:debian/postinst.ex` | Gabarits dh_make postinst.ex, preinst.ex, prerm.ex, postrm.ex, *.cron.d.ex : suffixe .ex, non utilisés par debhelper | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-freetrial-ai-engine-plugin=1.2.0.0-ok0.1 ; changelog unique « Initial Release » nile, 25/08/2025, kylin@unknown.
- [PROBABLE] La forme du debian/control (stances par architecture, Depends résolus, Original-Maintainer) correspond à des champs extraits de .deb déjà construits.

### `kylin-freetrial-nlp-engine`

- **Nature** : absent-ou-vide — **Amont** : Inconnu : aucun code ; Original-Maintainer liqi@kylinos.cn
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; Gitee = README.md + README.en.md (branche upstream, commit 0f72987 « Initial commit »)
- **Langages / taille** : aucun — archive 19 fichiers / 26,6 Ko ; gitee 2 fichiers / 2,3 Ko — lignes de code propres mesurées : 0
- **Privilège** : Indéterminable (binaire ISO non reconstructible)
- **Déclencheurs** : Inconnu
- **Indices réseau** : NON COUVERT (aucun code)
- **Blobs** : aucun dans l'arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source disponible.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Source Maintainer « Ubuntu Developers » ; 2 binaires libkylin-freetrial-nlp-engine et -dev (Architecture any, ${shlibs:Depends}) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul | CONSTATÉ |
| maintainer-script (gabarit inactif) | `archive:debian/postinst.ex` | Gabarits .ex (postinst, preinst, prerm, postrm, cron.d) inactifs | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-freetrial-nlp-engine=1.1.0.0-ok0.5 ; changelog unique « Initial Release » distribution « nile.bedrock » (09/06/2026).

### `kylin-ondevice-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (pangyi@kylinos.cn) ; Vcs-Git gitee.com/openkylin/kylin-ondevice-ai-engine-plugin
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT, têtes ai/v11-2603-preview, devel/26w) non acquis
- **Langages / taille** : C++ 2,4 Ko, en-têtes 1,6 Ko, CMake — 10 fichiers / 15,7 Ko ; code propre : 2 fichiers / 4 Ko — lignes de code propres mesurées : 95
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun dans le plugin ; moteurs on-device clients de kytensor en loopback
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers C++ (4 Ko) : fabrique des moteurs locaux.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-ai-engine-plugin (amd64 arm64) ; Depends libkylin-ondevice-nlp/vision/speech-engine ; Rules-Requires-Root: no | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(ondevice-engine-plugin AI_ENGINE ondeviceengine.*) ; liens kylin-ondevice-nlp/vision/speech-engine ; en-tête installé dans include/ | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-ai-engine-plugin=1.2.0.0-ok0.8.

### `kylin-ondevice-nlp-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (pangyi) ; Vcs-Git gitee.com/openkylin/kylin-ondevice-nlp-engine
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 61 Ko, en-têtes 11 Ko, XML 26 Ko (rapport de couverture), CMake — 19 fichiers / 150 Ko ; code propre hors test : 8 fichiers / 60 Ko (client/llmtritonclient.cpp 44 Ko) — lignes de code propres mesurées : 1 728
- **Privilège** : Bibliothèque dans le processus hôte ; client du service root kytensor
- **Déclencheurs** : Requête NLP locale via kylin-ondevice-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Client gRPC vers localhost:8001 (kytensor); Aucun listener
- **Blobs** : aucun ; debian/kylin-ondevice-nlp-engine-coverage.xml (rapport de couverture livré dans debian/)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 60 Ko de code propre transmettant des données utilisateur au service root kytensor.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-nlp-engine (amd64 arm64) ; Depends kylin-ai-abstract-models (>=1.2.0.0-0k0.4), kytensor-client ; B-D libgrpc++-dev, libprotobuf-dev, kytensor-client, libglib2.0-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' (commentaires dh_make) ; aucun .install, binaire unique | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kylin-ondevice-nlp-engine SHARED ondevicenlpengine.cpp, client/llmtritonclient.cpp, utils/*) ; liens httpclient, grpcclient (TritonClient), fmt, glib | CONSTATÉ |
| client-IPC (loopback) | `archive:client/llmtritonclient.cpp` | Chaîne "localhost:8001" (port gRPC Triton/kytensor) relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-nlp-engine=1.2.0.0-ok0.13.

### `kylin-ondevice-speech-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (pangyi ; dernière entrée wangkeran 26/08/2026)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 148 Ko, en-têtes 39 Ko, Python 21 Ko (clients de test), CMake — 44 fichiers / 253 Ko ; code propre src/ : 18 fichiers / 109 Ko — lignes de code propres mesurées : 5 639
- **Privilège** : Bibliothèque dans le processus hôte ; client du service root kytensor
- **Déclencheurs** : Requête ASR/TTS locale via le plugin on-device (PROBABLE)
- **Indices réseau** : Client gRPC vers localhost:8001; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 109 Ko de code propre traitant l'audio utilisateur et le transmettant à kytensor (root).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-speech-engine (amd64 arm64) ; Depends kylin-ai-abstract-models, kytensor-client ; B-D uuid-dev, libgrpc++-dev, libssl-dev, libprotobuf-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul ; aucun .install | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kylin-ondevice-speech-engine SHARED ondevicespeechengine*.cpp, asrstreaming, asrnonstreaming, mellotts, voiceactivitydetection, punctuation, tritonmodelclient, utils) ; liens httpclient, TritonClient::grpcclient, jsoncpp, uuid, OpenSSL | CONSTATÉ |
| client-IPC (loopback) | `archive:src/tritonmodelclient.cpp` | Chaîne "localhost:8001" relevée par grep dans src/ | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-speech-engine=1.2.0.0-ok0.3.

### `kylin-ondevice-vision-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (suhaotian@kylinos.cn) ; Vcs-Git gitlab2.kylin.com/kylinos-src/kylin-ondevice-vision-engine (forge interne)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 106 Ko, en-têtes 25 Ko, XML 15 Ko (couverture), CMake — 36 fichiers / 158 Ko ; code propre : 23 fichiers / 128 Ko — lignes de code propres mesurées : 3 474
- **Privilège** : Bibliothèque dans le processus hôte ; client du service root kytensor
- **Déclencheurs** : Requête OCR/segmentation/détourage via le plugin on-device (PROBABLE)
- **Indices réseau** : Client HTTP vers localhost:8000; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 128 Ko de code propre : décodage d'images (OpenCV, base64) et envoi à kytensor (root).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-vision-engine (amd64 arm64) ; Depends kylin-ai-abstract-models, kytensor-client ; B-D libopencv-dev, nlohmann-json3-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kylin-ondevice-vision-engine SHARED ondeviceocr/seg/portraitmatting/imagebinaryzation/visioncapability engine) + sous-bibliothèques ocr_triton_client, matting_triton_client, image_binaryzation, seg_triton_client ; OpenCV ; httpclient ; cible de test référence ondeviceddcpengine.cpp absent de l'arbre | CONSTATÉ |
| client-IPC (loopback) | `archive:ocr_triton_client/client.cpp` | Chaîne "localhost:8000" (port HTTP Triton/kytensor) relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-vision-engine=1.2.0.0-ok0.9.

### `kylin-ondevice-embedding-engine`

- **Nature** : propre-kylin — **Amont** : Kylin (wangzhijie@kylinos.cn) ; Homepage gitlab2.kylin.com
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 54 Ko, en-têtes 8,6 Ko, CMake ; image JPEG de test 8,4 Mo — 21 fichiers / 8,48 Mo (tests/微信图片_20240709181353.jpg 8,37 Mo) ; code propre : 6 fichiers / 52 Ko — lignes de code propres mesurées : 1 628
- **Privilège** : Bibliothèque dans le processus hôte ; client du service root kytensor
- **Déclencheurs** : Requête d'embedding via kylin-ondevice-traditional-ai-engine-plugin (PROBABLE)
- **Indices réseau** : Client gRPC vers localhost:8001; Aucun listener
- **Blobs** : aucun binaire exécutable ; JPEG de test 8,37 Mo
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 52 Ko de code propre (images/texte vers kytensor).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-embedding-engine + -dev (amd64 arm64) ; Replaces/Breaks text/image-embedding-engine (<< 1.1.0.0) ; Depends kytensor-client, kylin-ai-abstract-models | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| debian-install | `archive:debian/libkylin-ondevice-embedding-engine.install` | usr/lib/*/lib*.so.* ; -dev : usr/include/* + lib*.so | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kylin-ondevice-embedding-engine SHARED ondeviceembeddingengine.cpp, src/embeddingclient.cpp, src/utils.cpp) ; liens uuid, OpenCV, TritonClient::grpcclient, nlohmann_json | CONSTATÉ |
| client-IPC (loopback) | `archive:src/embeddingclient.cpp` | Chaîne "localhost:8001" relevée par grep | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-embedding-engine=1.2.0.0-ok0.7.

### `kylin-ondevice-traditional-ai-engine-plugin`

- **Nature** : propre-kylin — **Amont** : Kylin (wangzhijie) ; Homepage gitlab2.kylin.com
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 3 Ko, en-têtes 1,9 Ko, CMake — 14 fichiers / 13,9 Ko ; code propre : 4 fichiers / 4,5 Ko — lignes de code propres mesurées : 147
- **Privilège** : Module chargé par l'hôte KylinAiEngine (hors lot)
- **Déclencheurs** : Chargement par l'hôte du moteur IA (PROBABLE)
- **Indices réseau** : Aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 4 fichiers (4,5 Ko).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-ondevice-traditional-ai-engine-plugin (amd64 arm64) ; Depends libkylin-ondevice-embedding-engine (>= 1.2.0.0-ok0.6update1) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' (commentaires dh_make) | CONSTATÉ |
| plugin-moteur-IA (cible CMake) | `archive:CMakeLists.txt` | kylin_ai_add_engine_plugin(kylin-traditional-engine-plugin AI_ENGINE embeddingengineprivate.*, embeddingplugin.*) ; lié à kylin-ondevice-embedding-engine | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-ondevice-traditional-ai-engine-plugin=1.2.0.0-ok0.4.

### `kylin-ai-python-env`

- **Nature** : reconditionnement-binaire — **Amont** : Roues PyPI téléchargées au build : index https://mirrors.aliyun.com/pypi/simple/, extra-index https://download.pytorch.org/whl/cpu, liens https://csukuangfj.github.io/kaldifeat/cpu-cn.html ; 68 paquets épinglés (torch 2.8.0+cpu, transformers 4.46.1, tritonclient 2.49.0, PyMuPDF 1.24.11, lxml 5.3.1, grpcio 1.66.1, requests 2.32.3…)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive : scripts de construction + listes requirements (le contenu installé n'est pas dans l'arbre) ; Gitee PRESENT non acquis
- **Langages / taille** : Shell 3,5 Ko ; Python 280 Ko (2 copies de pip/_vendor/rich/_emoji_codes.py) — 12 fichiers / 293 Ko — lignes de code propres mesurées : 94
- **Privilège** : Aucun exécutable propre ; l'environnement est activé par le service root kytensor.service (ExecStart 'source /usr/share/kylin-ai-python-env/python-env/bin/activate')
- **Déclencheurs** : Build du paquet (téléchargement réseau), Démarrage de kytensor.service (activation du venv)
- **Indices réseau** : Téléchargement au build depuis mirrors.aliyun.com, download.pytorch.org, csukuangfj.github.io; Aucun listener propre
- **Blobs** : Aucun dans l'arbre ; ~68 roues binaires (dont torch) intégrées au .deb au moment du build
- **Budget proposé** : L3 — 100 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Scripts propres (3,5 Ko) d'un chemin d'installation utilisé par un service root ; la provenance et les CVE des 68 roues relèvent d'une analyse SCA séparée.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `archive:debian/rules` | override_dh_auto_configure: ./scripts/configure.sh ; override_dh_auto_install: ./scripts/install.sh ; dh_shlibdeps, dh_dwz, dh_strip, dh_auto_build neutralisés (exit 0) | CONSTATÉ |
| script de build (téléchargement) | `archive:scripts/configure.sh` | python3 -m venv debian/python-env ; pip install --no-deps --no-cache-dir -i https://mirrors.aliyun.com/pypi/simple/ --extra-index-url https://download.pytorch.org/whl/cpu -f https://csukuangfj.github.io/kaldifeat/cpu-cn.html -r <arch>/py3.12/requirements.txt ; aucune option --require-hashes | CONSTATÉ |
| script de build (installation) | `archive:scripts/install.sh` | Copie le venv vers /usr/share/kylin-ai-python-env/python-env ; réécrit activate*, shebangs (#!/usr/share/kylin-ai-python-env/python-env/bin/python3) et .pth ; remplace pip/_vendor/rich/_emoji_codes.py par la copie de l'arbre (commentaire : suppression de mots sensibles) | CONSTATÉ |
| debian-install | `archive:debian/install` | kylin-ai-python-env usr/share | CONSTATÉ |
| liste de dépendances | `archive:amd64/py3.12/requirements.txt` | 68 lignes '==' ; arm64/py3.12/requirements.txt diffère sur 5 versions (filelock, protobuf, tokenizers, torchaudio, torchvision) | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-ai-python-env (arm64 amd64) ; Depends python3.12 ; B-D python3-venv, python3-all-dev | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : kylin-ai-python-env=1.2.0.0-ok0.6.
- [PROBABLE] Le contenu binaire du .deb dépend de l'état des index au moment du build (non reproductible depuis l'arbre).

### `libkysdk-genai-nlp`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; code D-Bus généré depuis libkylin-ai-proto-dev (gennlpservice, hors lot)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 113 Ko, en-têtes 77 Ko, CMake — 42 fichiers / 201 Ko ; code propre (src+include) : ≈150 Ko ; examples/chatbot.cpp 13,8 Ko — lignes de code propres mesurées : 5 306
- **Privilège** : Bibliothèque chargée dans l'application cliente (session utilisateur) ; serveur (runtime IA) hors lot
- **Déclencheurs** : Appel de l'API SDK par une application
- **Indices réseau** : Socket UNIX /tmp/.kylin-ai-runtime-unix/<uid>/genai-nlp.sock (client); Aucun listener TCP
- **Blobs** : aucun
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — SDK de session (≈150 Ko) ; priorité au chemin IPC (socket dans /tmp, sérialisation des messages).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-genai-nlp0 + -dev ; B-D libkylin-ai-proto-dev (>= 1.0.0.0-ok1.3), libkysdk-ai-common-dev, libssl-dev, libglib2.0-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-genai-nlp0.install` | usr/lib/*/lib*.so.* ; -dev : usr/include/*, lib*.so, usr/share/pkgconfig/* | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kysdk-genai-nlp SHARED src/* + code gdbus généré (kylin_ai_generate_gdbus_proto_code gennlpservice)) ; BUILD_EXAMPLES ON (chatbot construit, sans règle install) ; install(DIRECTORY usr/ DESTINATION /usr) | CONSTATÉ |
| IPC D-Bus pair-à-pair | `archive:src/genainlpserver.cpp` | l.28 adresse "unix:path=/tmp/.kylin-ai-runtime-unix/" + getuid() + "/genai-nlp.sock" ; l.59 g_dbus_connection_new_for_address_sync(…, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT) ; src/genainlpserviceproxy.cpp:32 interface com.kylin.AiRuntime.GenAiNlp | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkysdk-genai-nlp0=1.1.0.1-ok1.6.

### `libkysdk-genai-vision`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; gdbus généré depuis libkylin-ai-proto-dev (genaivisionservice)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 55 Ko, en-têtes 39 Ko, CMake — 32 fichiers / 139 Ko ; code propre : 18 fichiers / 86 Ko — lignes de code propres mesurées : 2 713
- **Privilège** : Bibliothèque dans l'application cliente
- **Déclencheurs** : Appel de l'API SDK
- **Indices réseau** : Socket UNIX client /tmp/.kylin-ai-runtime-unix/<uid>/genai-vision.sock
- **Blobs** : aucun
- **Budget proposé** : L2 — 45 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — SDK de session 86 Ko ; IPC vers runtime IA.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-genai-vision0 + -dev ; B-D libkylin-ai-proto-dev, libssl-dev, libglib2.0-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-genai-vision0.install` | usr/lib/*/lib*.so.* ; -dev : include, .so, pkgconfig | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kysdk-genai-vision SHARED imageconfig, _imageresult, _genaiimagesession, genaivisionserviceproxy, protocolversion, image, logger) ; liens jsoncpp, gio, OpenSSL | CONSTATÉ |
| IPC D-Bus pair-à-pair | `archive:src/genaivisionserviceproxy.cpp` | l.32 unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/genai-vision.sock ; l.34 interface com.kylin.AiRuntime.GenAiVision ; l.148 g_dbus_connection_new_for_address_sync | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkysdk-genai-vision0=1.1.0.1-ok1.5.

### `libkysdk-coreai-vision`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; gdbus généré (corevisionservice)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee ABSENT, GitHub ABSENT
- **Langages / taille** : C++ 150 Ko, en-têtes 89 Ko, CMake ; images de test ≈11,5 Mo — 71 fichiers / 11,8 Mo (15 images de test, dont tests/test1 = PNG sans extension) ; code propre : 38 fichiers / 184 Ko — lignes de code propres mesurées : 6 482
- **Privilège** : Bibliothèque dans l'application cliente
- **Déclencheurs** : Appel de l'API SDK (OCR, segmentation)
- **Indices réseau** : Socket UNIX client /tmp/.kylin-ai-runtime-unix/<uid>/core-vision.sock
- **Blobs** : aucun binaire exécutable ; images de test
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 184 Ko de code propre ; IPC et traitement de résultats d'images.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-coreai-vision0 + -dev ; B-D libkylin-ai-proto-dev (>= 1.0.0.0-ok1.3), libssl-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-coreai-vision0.install` | usr/lib/*/lib*.so.* ; -dev : include, .so, pkgconfig | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kysdk-coreai-vision SHARED src/* + gdbus corevisionservice) ; liens jsoncpp, gio, OpenSSL | CONSTATÉ |
| IPC D-Bus pair-à-pair | `archive:src/corevisionserviceproxy.cpp` | l.40 unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/core-vision.sock ; l.42 interface com.kylin.AiRuntime.CoreVision ; l.361 g_dbus_connection_new_for_address_sync | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkysdk-coreai-vision0=1.1.0.1-ok1.3 ; entrée de tête du changelog en distribution « nile ».

### `libkysdk-coreai-speech`

- **Nature** : propre-kylin — **Amont** : Kylin (guopengfei) ; gdbus généré (corespeechservice) ; 3rdparty/catch2 (tests)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 1,08 Mo (dont catch2 ≈950 Ko), en-têtes 72 Ko, CMake ; PCM de test — 66 fichiers / 2,49 Mo ; code propre : 33 fichiers / 143 Ko — lignes de code propres mesurées : 5 860
- **Privilège** : Bibliothèque dans l'application cliente
- **Déclencheurs** : Appel de l'API SDK (ASR/TTS)
- **Indices réseau** : Socket UNIX client /tmp/.kylin-ai-runtime-unix/<uid>/core-speech.sock
- **Blobs** : aucun binaire exécutable ; fichiers PCM de test
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 143 Ko de code propre ; flux audio vers runtime IA via IPC.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkysdk-coreai-speech0 + -dev ; B-D libkylin-ai-proto-dev (>= 1.0.0.0-0k0.9), libssl-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |
| debian-install | `archive:debian/libkysdk-coreai-speech0.install` | usr/lib/*/lib*.so.* ; -dev : include, .so, pkgconfig | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kysdk-coreai-speech SHARED src/* + gdbus corespeechservice) ; liens jsoncpp, gio, OpenSSL ; tests (catch2) si ENABLE_TEST | CONSTATÉ |
| IPC D-Bus pair-à-pair | `archive:src/coreaispeechserver.cpp` | l.8 unix:path=/tmp/.kylin-ai-runtime-unix/<uid>/core-speech.sock ; l.27 g_dbus_connection_new_for_address_sync ; src/coreaispeechserviceproxy.cpp:37 interface com.kylin.AiRuntime.CoreSpeech | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkysdk-coreai-speech0=1.1.0.1-ok1.5 ; utilisé en B-D par ukui-voice-wing.

### `kylin-coreai-embedding`

- **Nature** : propre-kylin — **Amont** : Kylin (wangzhijie) ; Homepage gitlab2.kylin.com ; gdbus généré (coretextembeddingservice, coreimageembeddingservice)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 230 Ko (dont tests 128 Ko), en-têtes 66 Ko, CMake — 49 fichiers / 8,71 Mo (image de test 8,37 Mo) ; code propre hors test : 27 fichiers / 147 Ko — lignes de code propres mesurées : 7 800
- **Privilège** : Bibliothèque dans l'application cliente
- **Déclencheurs** : Appel de l'API SDK d'embedding
- **Indices réseau** : Sockets UNIX clients sous /tmp/.kylin-ai-runtime-unix/
- **Blobs** : aucun binaire exécutable ; JPEG/DOCX de test
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 147 Ko de code propre ; IPC vers runtime IA (fichiers utilisateur à indexer).

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | libkylin-coreai-embedding (Section libdevel) + -dev ; B-D libkylin-ai-proto-dev, libkysdk-ai-common-dev, nlohmann-json3-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' (commentaires dh_make) | CONSTATÉ |
| debian-install | `archive:debian/libkylin-coreai-embedding.install` | usr/lib/*/lib*.so.* ; -dev : include, .so, pkgconfig | CONSTATÉ |
| cible CMake | `archive:CMakeLists.txt` | add_library(kysdk-coreai-embedding SHARED file(GLOB src/*.cpp, textEmbedding, imageEmbedding, server, log)) ; liens gio, nlohmann_json | CONSTATÉ |
| IPC D-Bus pair-à-pair | `archive:src/server/server.cpp` | l.74 g_dbus_connection_new_for_address_sync ; textembeddingserver.cpp:25 / imageembeddingserver.cpp:25 adresses unix:path=/tmp/.kylin-ai-runtime-unix/… ; *processorproxy.cpp idem | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : libkylin-coreai-embedding=1.2.0.0-ok0.5.

### `kylin-ai-parser-extension`

- **Nature** : propre-kylin — **Amont** : Kylin (wangyan@kylinos.cn) ; Homepage gitlab2.kylin.com
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) + gitee (branche openkylin/huanghe) : arbres identiques (diff -r : seul .git diffère)
- **Langages / taille** : C++ 1,8 Ko, en-têtes 1 Ko, CMake — 10 fichiers / 10,5 Ko ; code propre : 2 fichiers / 2,9 Ko — lignes de code propres mesurées : 81
- **Privilège** : Module chargé dans le processus Python consommateur (kylin-ai-document-service, hors lot) ; privilège de ce dernier NON COUVERT
- **Déclencheurs** : Import Python par kylin-ai-document-service (PROBABLE)
- **Indices réseau** : Aucun
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — 2 fichiers (2,9 Ko) exposant l'analyse de documents (libukui-file-metadata, hors lot) à Python.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | kylin-ai-parser-extension ; B-D pybind11-dev, python3-dev, libukui-file-metadata-dev, qtbase5-dev ; description : utilisé par kylin-ai-document-service (hors lot) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' (commentaires dh_make) | CONSTATÉ |
| module Python (cible CMake) | `archive:CMakeLists.txt` | add_library(parser_extension MODULE parserextension.cpp) pybind11 + Qt5Core + ukui-file-metadata ; installé dans ${CMAKE_INSTALL_LIBDIR}/kylin-ai-parser-extention/ (orthographe « extention ») | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : kylin-ai-parser-extension=1.2.0.0-ok0.4.

### `kylin-asrassistant`

- **Nature** : propre-kylin — **Amont** : Kylin (huheng@kylinos.cn) ; description : basé sur le modèle open source PaddleSpeech (Baidu) côté serveur
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee et GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : C++ 77 Ko, en-têtes 8 Ko, TypeScript 12 Ko (fichiers .ts de traduction Qt), Python 0,3 Ko, CMake ; 54 PNG — 97 fichiers / 1,16 Mo ; code propre : 21 fichiers / 86 Ko — lignes de code propres mesurées : 2 409
- **Privilège** : Session utilisateur (démarrage automatique), accès micro
- **Déclencheurs** : Ouverture de session (autostart XDG), Appel D-Bus session com.kylinos.asr.assistant.service
- **Indices réseau** : Client WebSocket non chiffré vers 124.126.103.57:59999 (configuration); Service D-Bus session
- **Blobs** : aucun binaire exécutable ; 54 PNG d'animation
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 86 Ko de code propre : capture audio, envoi réseau, collecte de données (kysdk-datacollect), service D-Bus ; non installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-autostart | `archive:kylin-asrassistant.desktop` | Installé dans /etc/xdg/autostart/ (CMakeLists.txt racine ; installation dans /usr/share/applications commentée) ; Exec=env QT_PLATFORM=xcb /usr/bin/kylin-asrassistant -s%u ; X-KDE-Wayland-Interfaces=…org_kde_kwin_fake_input, zkde_screencast_unstable_v1 | CONSTATÉ |
| dbus-session | `archive:src/dbusservice/asrdbusservice.cpp` | l.8-10 QDBusConnection::sessionBus().registerService("com.kylinos.asr.assistant.service") + registerObject (export des slots) | CONSTATÉ |
| configuration réseau | `archive:config/speechrecognitionusr.conf` | Installé dans /usr/share/kylin-asrassistant/ ; contenu : ws://124.126.103.57:59999/paddlespeech/asr/streaming (schéma ws:// sans TLS, adresse IP littérale) | CONSTATÉ |
| cibles CMake | `archive:src/CMakeLists.txt` | Exécutable kylin-asrassistant → /usr/bin ; liens Qt5 Widgets/Network/DBus, kysdk-alm, kysdk-datacollect, kysdk-waylandhelper ; bibliothèques speechrecognitionclient (Qt5::WebSockets) et speechinput (Qt5::Multimedia) → /usr/lib/ | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-asrassistant ; B-D libkysdk-datacollect-dev, libqt5websockets5-dev, qtmultimedia5-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' ; debian/kylin-asrassistant.install : icônes ; debian/source/options : include-binaries | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO ; entrée de tête du changelog en distribution « yangtze » (23/11/2023).

### `llm-backend`

- **Nature** : absent-ou-vide — **Amont** : Inconnu ; Maintainer binaire kylin <binzemin@kylinos.cn> ; Depends kytensor-llm (llama.cpp)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; Gitee EMPTY
- **Langages / taille** : aucun — 19 fichiers / 26 Ko (debian/ seulement) — lignes de code propres mesurées : 0
- **Privilège** : Indéterminable ; si backend Triton, chargé par kytensor.service (root) (PROBABLE)
- **Déclencheurs** : Installé comme dépendance de kytensor-client (archive:kytensor/debian/control)
- **Indices réseau** : NON COUVERT (aucun code)
- **Blobs** : aucun dans l'arbre ; binaire de l'ISO hors arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; contenu du binaire ISO non reconstructible.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Source Maintainer root <root@unknown> ; 2 stances llm-backend (amd64, arm64) ; Depends libc6 (>= 2.38), libgcc-s1, libstdc++6 (>= 12), kytensor-llm (>= 1.0.0) ; Breaks/Replaces kytensor-llama ; descriptions gabarit | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul | CONSTATÉ |
| maintainer-script (gabarit inactif) | `archive:debian/postinst.ex` | Gabarits .ex inactifs | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : llm-backend=1.0.1-0ok3 ; changelog unique « Initial Release » nile, 25/08/2025, root@unknown.

### `kytensor`

- **Nature** : fork-amont — **Amont** : NVIDIA Triton Inference Server (README : « 基于 NVIDIA Triton Inference Server 构建 ») ; CMakeLists.txt TRITON_VERSION "2.49.0" mais server/TRITON_VERSION = 2.50.0dev ; version paquet 2.49.0.6-0ok11 ; format native : delta Kylin non isolable sans l'amont
- **Installé par défaut (ISO 3.0)** : oui (3 binaire(s)) — **Code disponible** : archive (source complète réorganisée : server, core, client, backend, common, framework_backends {python_backend, identity_backend}, third_party {libevent, libevhtp, dlpack}) ; Gitee ABSENT, GitHub ABSENT
- **Langages / taille** : C++ 3,7 Mo, en-têtes 2,1 Mo, C 2 Mo, Python 1,3 Mo, Protobuf 125 Ko, Java 97 Ko — 1024 fichiers / 14,5 Mo (dont PDF de licence NVIDIA 3 Mo) — lignes de code propres mesurées : 178 354
- **Privilège** : root (unité système sans User=), démarrée au boot ; charge des backends natifs et Python depuis les dépôts de modèles
- **Déclencheurs** : Démarrage système (WantedBy=default.target), postinst kytensor-server (enable + start), Restart=always
- **Indices réseau** : Serveur HTTP (127.0.0.1:8000), gRPC (127.0.0.1:8001) et métriques (127.0.0.1:8002) (PROBABLE pour les ports); Clients : moteurs on-device du lot (localhost:8000/8001)
- **Blobs** : 4 modèles de test (saved_model.pb 959 o, 3 model.onnx 331 o) ; PDF de licence NVIDIA
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Service root avec écoute HTTP/gRPC et exécution de backends (Python, natifs) ; base volumineuse (1024 fichiers) dont le delta Kylin doit d'abord être isolé par diff avec Triton r24.08.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| systemd-unit | `archive:server/kytensor.service` | ExecStart=/bin/bash -c 'source /usr/share/kylin-ai-python-env/python-env/bin/activate && /usr/bin/kytensor --model-repository=/usr/share/kylin-ai/model-repository --model-repository=/opt/appdata/kylin-ai/model-repository --model-control-mode=explicit --http-address=127.0.0.1 --grpc-address=127.0.0.1' ; aucun User= (root) ; Restart=always ; RestartSec=5 ; WantedBy=default.target ; installé dans /usr/lib/systemd/system/ (CMakeLists.txt:272) | CONSTATÉ |
| maintainer-script | `archive:debian/kytensor-server.postinst` | systemctl daemon-reload ; systemctl enable kytensor.service ; systemctl start kytensor.service (sans set -e, sans test de $1, sans #DEBHELPER#) | CONSTATÉ |
| maintainer-script | `archive:debian/kytensor-client.postinst` | sed -i sur /usr/include/triton/client/grpc_service.grpc.pb.h (réécriture de 2 #include) ; jeton '# DEBHELPER#' (avec espace) | CONSTATÉ |
| maintainer-script | `archive:debian/kytensor-python.postinst` | chmod +x /opt/tritonserver/backends/python/triton_python_backend_stub ; jeton '# DEBHELPER#' (avec espace) | CONSTATÉ |
| ecoute-reseau | `archive:server/src/command_line_parser.cc` | l.1795-1798 : metrics_address = http_address si non fourni ; command_line_parser.h:193-194/211-212 défauts http 0.0.0.0:8000, metrics port 8002 ; l'unité impose 127.0.0.1 → HTTP 127.0.0.1:8000, métriques 127.0.0.1:8002, gRPC 127.0.0.1 (port 8001 par défaut amont) | PROBABLE |
| debian-rules | `archive:debian/rules` | dh_auto_configure -DTRITON_ENABLE_SERVER/CLIENT/CC_HTTP/CC_GRPC/PYTHON_HTTP/PYTHON_GRPC/PYTHON_BACKEND/ENSEMBLE/TESTS=ON, GPU OFF ; dh_shlibdeps --ignore-missing-info | CONSTATÉ |
| debian-install | `archive:debian/kytensor-server.install` | /usr/bin/kytensor, libtritonserver.so, libproto-library.so, en-têtes triton, /usr/lib/systemd/system/kytensor.service ; kytensor-python.install : /opt/tritonserver/backends/python/{libtriton_python.so, triton_python_backend_stub, triton_python_backend_utils.py} ; kytensor-client-wheel.install : /opt/appdata/kylin-ai/kytensor-client/wheel/ ; libtriton-onnxruntime.install sans paquet déclaré dans control | CONSTATÉ |
| debian-control | `archive:debian/control` | 4 binaires (kytensor-server, kytensor-client, kytensor-client-wheel, kytensor-python), descriptions dh_make non remplies ; kytensor-client Depends onnxruntime-backend, kytensor-python, llm-backend ; kytensor-client-wheel Depends libpython3.8-dev | CONSTATÉ |
| desktop (non installé) | `archive:server/tritonserver.desktop` | Exec=bash -c "source …/python-env/bin/activate && tritonserver … --http-address=127.0.0.1 --grpc-address=127.0.0.1 --log-file=$HOME/.log/tritonserver.log" ; X-UKUI-Autostart-Phase=Application ; aucune règle d'installation trouvée (CMakeLists, debian/*.install) | CONSTATÉ |
| maintainer-script [ajout critique] | `archive:kytensor/debian/kytensor-client.postinst ; debian/kytensor-python.postinst` | Le recensement ne cite que le postinst de kytensor-server. kytensor-client.postinst modifie par sed /usr/include/triton/client/grpc_service.grpc.pb.h. kytensor-python.postinst fait chmod +x /opt/tritonserver/backends/python/triton_python_backend_stub. Le jeton est mal ecrit ('# DEBHELPER#'). | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : kytensor-client, kytensor-python, kytensor-server =2.49.0.6-0ok11.
- [PROBABLE] CMakeLists.txt utilise FetchContent avec SOURCE_DIR locaux (core, third_party) ; server/CMakeLists.txt et client/CMakeLists.txt contiennent des GIT_REPOSITORY ${TRITON_REPO_ORGANIZATION}, probablement inutilisés dans ce mode de build.

### `kytensor-llm`

- **Nature** : fork-amont — **Amont** : llama.cpp (ggerganov, Homepage https://github.com/ggerganov/llama.cpp) ; révision amont non indiquée (BUILD_NUMBER calculé via git) ; changelog : ajout loongarch/riscv64, Qwen3/Qwen3MoE, SSE 4.2
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète llama.cpp + ggml + gguf-py) ; Gitee EMPTY
- **Langages / taille** : C++ 7,4 Mo, C 1,6 Mo, en-têtes 1,6 Mo, Python 1,2 Mo, Shell, JSON, HTML — 1178 fichiers / 68,5 Mo (dont 49 Mo de vocabulaires GGUF de test dans models/) — lignes de code propres mesurées : 311 335
- **Privilège** : Bibliothèques sans privilège propre ; chargées par llm-backend sous kytensor.service (root) (PROBABLE)
- **Déclencheurs** : Chargement par llm-backend / kytensor (PROBABLE), Exécution manuelle des binaires du paquet -dev
- **Indices réseau** : llama-server (paquet -dev, non installé par l'ISO) : écoute HTTP configurable, défaut port 8080; Aucun réseau dans les bibliothèques installées (LLAMA_CURL=OFF)
- **Blobs** : 17 blobs : 16 vocabulaires models/ggml-vocab-*.gguf (49 Mo) + gradle-wrapper.jar (exemple Android)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Fork volumineux ; priorité au diff vs llama.cpp amont et au chargeur GGUF (fichiers modèles) exécuté potentiellement dans un service root.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | kytensor-llm (bibliothèques) et kytensor-llm-dev ; Replaces/Breaks/Conflicts llamacpp ; B-D git, cmake, ocl-icd-libopencl1 | CONSTATÉ |
| debian-rules | `archive:debian/rules` | cmake -DGGML_BACKEND_DL=ON (amd64) / GGML_NATIVE=ON + -mtune=native (autres arch.) ; -DLLAMA_CURL=OFF ; -DGGML_CUDA=OFF ; tests désactivés | CONSTATÉ |
| debian-install | `archive:debian/kytensor-llm.install` | usr/lib/*/libggml.so, libggml-base.so, libllama.so, libcommon.so, libggml-*.so ; -dev : usr/bin/* (tous les exemples construits), en-têtes, cmake, pkgconfig | CONSTATÉ |
| ecoute-reseau (paquet -dev) | `archive:examples/server/server.cpp` | l.4465 svr->bind_to_port(params.hostname, params.port) ; l.4475 listen_after_bind() ; port par défaut 8080 (common/common.h:361) ; llama-server construit (LLAMA_BUILD_SERVER) et livré via usr/bin/* dans kytensor-llm-dev | CONSTATÉ |
| indice-ecoute | `archive:ggml/src/ggml-rpc/ggml-rpc.cpp` | bind() du serveur RPC ggml ; GGML_RPC OFF par défaut (ggml/CMakeLists.txt:187), non activé dans debian/rules | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : kytensor-llm=1.0.0-ok0.7~1 (bibliothèques seulement) ; changelog de tête « No-change rebuild » root@luoyaoming 17/08/2026.

### `onnxruntime-backend`

- **Nature** : absent-ou-vide — **Amont** : Inconnu ; Maintainer binaire binzemin@kylinos.cn ; Depends libonnxruntime (>= 1.20.1)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : aucun : archive = squelette dh_make (meta.source_coquille_vide=1) ; Gitee EMPTY
- **Langages / taille** : aucun — 19 fichiers / 26,4 Ko (debian/ seulement) — lignes de code propres mesurées : 0
- **Privilège** : Indéterminable ; backend ONNX Runtime de Triton chargé par kytensor.service (root) (PROBABLE)
- **Déclencheurs** : Installé comme dépendance de kytensor-client
- **Indices réseau** : NON COUVERT (aucun code)
- **Blobs** : aucun dans l'arbre ; binaire de l'ISO hors arbre
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Aucun code source ; le backend onnxruntime n'existe pas non plus dans kytensor/framework_backends.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | Source Maintainer root <root@unknown> ; 2 stances onnxruntime-backend (arm64, amd64) ; Depends libc6 (>= 2.38), libgcc-s1, libonnxruntime (>= 1.20.1), libstdc++6 (>= 12) ; Breaks/Replaces libtriton-onnxruntime | CONSTATÉ |
| debian-rules | `archive:debian/rules` | '%: dh $@' seul | CONSTATÉ |
| maintainer-script (gabarit inactif) | `archive:debian/postinst.ex` | Gabarits .ex inactifs | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : onnxruntime-backend=1.0.0-0ok3 ; changelog unique « Initial Release » nile 25/08/2025.

### `ukui-menu-extensions`

- **Nature** : propre-kylin — **Amont** : Projet UKUI (Kylin), aussi présent dans Debian/Ubuntu (meta) ; format 3.0 (quilt) sans debian/patches ; debian/watch : releases gitee.com/openkylin/ukui-menu-extensions
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 134 Ko, TypeScript 78 Ko (traductions .ts), QML 47 Ko, en-têtes 17 Ko, Shell 5 Ko — 92 fichiers / 345 Ko ; code propre hors tests : 29 fichiers / 118 Ko — lignes de code propres mesurées : 6 365
- **Privilège** : Plugins chargés dans ukui-menu (session utilisateur)
- **Déclencheurs** : Chargement par ukui-menu à l'ouverture du menu (PROBABLE)
- **Indices réseau** : ernie-bot : client HTTPS vers aip.baidubce.com; Aucun listener
- **Blobs** : aucun
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — Plugins de session ; recent-file (installé) lié à kysdk-datacollect ; ernie-bot réseau mais non installé.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | 2 binaires : ukui-menu-ernie-bot, ukui-menu-recent-file ; B-D libkysdk-datacollect-dev, libkysdk-systime-dev, libukui-menu-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh --buildsystem cmake ; -DCMAKE_BUILD_TYPE=RelWithDebInfo | CONSTATÉ |
| plugin ukui-menu | `archive:debian/ukui-menu-recent-file.install` | usr/lib/*/ukui-menu/extensions/librecent-file.so ; ukui-menu-ernie-bot.install : libernie-bot.so (même répertoire) | CONSTATÉ |
| cible CMake | `archive:recent-file/CMakeLists.txt` | PC_PKGS glib-2.0 gio-unix-2.0 kysdk-datacollect kysdk-systime (l.33) liés à l'extension | CONSTATÉ |
| client-reseau | `archive:ernie-bot/src/chat.cpp` | Chaînes https://aip.baidubce.com/oauth/2.0/token, …/wenxinworkshop/chat/completions, …/chat/eb-instant relevées par grep ; Qt Network (ernie-bot/CMakeLists.txt) | CONSTATÉ |

- [CONSTATÉ] ISO : seul ukui-menu-recent-file=4.24.0.0-ok0.1 est installé (ernie-bot non installé).

### `ukui-intelligent-data-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (Yue Lan) ; debian/watch présent
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (DIVERGENT) non acquis
- **Langages / taille** : C++ 283 Ko, QML 112 Ko, en-têtes 60 Ko, TypeScript 22 Ko (traductions), qmake, Qt UI — 109 fichiers / 4,53 Mo (ui/data/nospaceindicator.png 3,6 Mo) ; code propre : 58 fichiers / 429 Ko — lignes de code propres mesurées : 13 593
- **Privilège** : Session utilisateur (démarrage automatique, redémarrage auto)
- **Déclencheurs** : Ouverture de session UKUI (autostart), Lancement manuel
- **Indices réseau** : Socket local QtRemoteObjects (local:ukui-idm-<user><display>); Aucun listener TCP
- **Blobs** : aucun binaire exécutable ; PNG 3,6 Mo
- **Budget proposé** : L2 — 40 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 429 Ko de code de session avec IPC locale et indexation de fichiers (Qt Sql) ; non installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| desktop-autostart | `archive:data/ukui-idm-backend.desktop` | Installé dans /etc/xdg/autostart (CMakeLists.txt:154) ; Exec=/usr/bin/ukui-intelligent-data-manager ; X-UKUI-AutoRestart=true ; OnlyShowIn=UKUI ; NoDisplay=true | CONSTATÉ |
| desktop | `archive:data/ukui-intelligent-data-manager.desktop` | Exec=/usr/bin/ukui-intelligent-data-manager -s → /usr/share/applications (CMakeLists.txt:153) | CONSTATÉ |
| IPC locale (QtRemoteObjects) | `archive:file/fileinfomanager.cpp` | l.71-121 QRemoteObjectHost, URL "local:ukui-idm-" + $USER + display, enableRemoting | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh_auto_configure -DSHOULD_SKIP_TEST=true -DBUILD_WITH_UKUI_SHELL=true | CONSTATÉ |
| debian-control | `archive:debian/control` | ukui-intelligent-data-manager ; Depends libukui-file-metadata-bin ; B-D libpeony-dev, libukui-search-dev, libqt5remoteobjects5-dev, libkysdk-filesystem-dev | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO ; entrée de tête du changelog en distribution « nile ».

### `kylin-ai-abstract-models`

- **Nature** : propre-kylin — **Amont** : Kylin (pangyi) ; Vcs-Git gitlab2.kylin.com ; backend ppocr dérivé d'un exemple Triton (TutorialOcrBackendConfig.cmake.in, en-tête de licence BSD NVIDIA dans ppocr/CMakeLists.txt)
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (source complète) ; Gitee PRESENT non acquis
- **Langages / taille** : C++ 79 Ko, Python 10 Ko, en-têtes 5,5 Ko, configurations pbtxt/YAML/JSON — 40 fichiers / 135 Ko ; code propre : 12 fichiers / 95 Ko — lignes de code propres mesurées : 2 613
- **Privilège** : Code (backends .so, model.py) chargé par kytensor.service, donc root (PROBABLE) ; postinst exécuté en root
- **Déclencheurs** : postinst (configure), Chargement explicite de modèles par kytensor (--model-control-mode=explicit) (PROBABLE)
- **Indices réseau** : Aucun
- **Blobs** : aucun (poids des modèles fournis par les paquets kylin-*-model, hors lot)
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 95 Ko de code propre exécuté dans le service root kytensor (backends natifs + Python) ; postinst créant le dépôt modifiable.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `archive:debian/postinst` | mkdir -p /opt/appdata/kylin-ai/model-repository et /usr/share/kylin-ai/model-repository si absents ; chmod/chown commentés ; set -e ; exit 0 | CONSTATÉ |
| cible CMake (dépôt de modèles) | `archive:CMakeLists.txt` | Installe dans /usr/share/kylin-ai/model-repository : ocr_ppocr, model_bank, embd/ensemble/tokenizer gte-base et cn-clip | CONSTATÉ |
| backend Triton natif | `archive:model_bank/CMakeLists.txt` | l.26 add_library(triton-model_bank-backend SHARED modelbank.cpp utils.cpp) ; l.64-84 copie .so + config.pbtxt + default_model.yaml + modeldevicebinding.json dans model_repos/model_bank/1 ; ppocr : backend "ocr" (ppocr/src/config.pbtxt) | CONSTATÉ |
| modèle Python Triton | `archive:gte-base-uint8-onnx/tokenizer_gte-base_uint8-text/1/model.py` | config.pbtxt backend: "python" (idem cn-clip tokenizer) ; exécuté par triton_python_backend_stub sous kytensor (PROBABLE) | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-ai-abstract-models (amd64 arm64) ; Depends kylin-gte-base-model, kylin-cn-clip-model, kylin-ai-python-env, kylin-paddle-ocr-model, kylin-portrait-matting-model, kylin-universal-matting-model, kylin-sam-model, kylin-speech-asr-model, kylin-speech-tts-model | CONSTATÉ |
| debian-rules | `archive:debian/rules` | dh $@ ; dh_shlibdeps --ignore-missing-info | CONSTATÉ |

- [CONSTATÉ] Installé par l'ISO : kylin-ai-abstract-models=1.2.0.0-0k0.21.

### `kylin-agent`

- **Nature** : propre-kylin — **Amont** : Kylin ; version CMake 0.9.6 (runtime 0.9.9, CUA 0.9.7) ; pilote agent-runtime (fork Hermes) et computer-use-agent
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche master, HEAD 462e725) ; absent de l'archive et de l'ISO ; pas de répertoire debian/
- **Langages / taille** : C++ 367 Ko, Shell 410 Ko, Python 76 Ko, en-têtes 71 Ko — 159 fichiers / 3,03 Mo (images res/ 1,85 Mo) — lignes de code propres mesurées : 25 760
- **Privilège** : Application de session ; scripts installés demandant sudo et installant un démon root (ydotoold, injection d'entrées)
- **Déclencheurs** : Lancement utilisateur, Exécution manuelle des scripts d'installation
- **Indices réseau** : Sockets locaux ~/.cache/kylin-agent*.socket; Socket ydotool (gatewayservice.cpp:1110); Clients cloud ASR/TTS (WebSocket) ; téléchargements astral.sh, nodejs.org, gitee.com
- **Blobs** : aucun binaire exécutable ; GIF/PNG
- **Budget proposé** : L2 — 50 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — Composant de session avec IPC locale, réseau, scripts d'installation privilégiés (≈10 800 lignes de shell) ; hors ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cible CMake | `gitee:CMakeLists.txt` | Exécutable kylin-agent → /usr/bin ; install(FILES packaging/kylin-agent.desktop) alors que packaging/ est absent de l'arbre ; scripts agent_runtime_install.sh, kylin_agent_ctl.py, ydotool_install.sh, seed_runtime_skills.sh, repair_hermes_home.sh, install_runtime_cache.sh → /usr/local/share/kylin-agent/scripts (PROGRAMS) | CONSTATÉ |
| socket-local | `gitee:src/services/controlservice.cpp` | l.17-40 QLocalServer ~/.cache/kylin-agent-control.socket, setSocketOptions(UserAccessOption), suppression de socket périmé après sonde | CONSTATÉ |
| socket-local | `gitee:src/app.cpp` | l.105 m_socketName ~/.cache/kylin-agent.socket ; l.223-225 QLocalServer::listen sans option de permission explicite | CONSTATÉ |
| script d'installation (root) | `gitee:scripts/ydotool_install.sh` | sudo -v ; écrit /etc/systemd/system/ydotoold.service (0644) : ExecStart=/usr/local/bin/ydotoold --socket-path … --socket-own UID:GID, aucun User= (root), WantedBy=multi-user.target ; systemctl enable --now ydotoold | CONSTATÉ |
| script d'installation (réseau) | `gitee:scripts/agent_runtime_install.sh` | l.46 REPO_URL_HTTPS = https://<utilisateur>:<secret>@gitee.com/openkylin/agent-runtime.git (identifiants intégrés, valeur masquée) ; télécharge https://astral.sh/uv/install.sh et Node.js depuis nodejs.org/dist | CONSTATÉ |
| clients ASR/TTS | `gitee:py/` | baidu_realtime_asr.py, iflytek_*.py, qwen*_realtime_asr.py, qwen_tts_realtime.py, tencent_*.py, volcengine_realtime_asr.py ; py/requirements.txt websocket-client, sounddevice | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive huanghe et de l'ISO 3.0.
- [CONSTATÉ] scripts/agent_runtime_install_bak.sh:47 contient aussi une URL Git avec identifiants intégrés (valeur masquée).

### `kylin-agent-armor`

- **Nature** : absent-ou-vide — **Amont** : Kylin (description : plugin de sécurité pour kylin-agent)
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : aucun : gitee seulement, ReadMe.md (227 octets), commit ecb3bf1 « 首次创建 »
- **Langages / taille** : aucun — 1 fichier / 227 octets — lignes de code propres mesurées : 0
- **Privilège** : Sans objet
- **Déclencheurs** : Sans objet
- **Indices réseau** : Aucun
- **Blobs** : aucun
- **Budget proposé** : X — 0 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Dépôt sans code.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| documentation | `gitee:ReadMe.md` | Décrit un plugin d'analyse et de blocage des nœuds d'exécution de kylin-agent ; aucun code | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive et de l'ISO.

### `agent-runtime`

- **Nature** : fork-amont — **Amont** : Hermes Agent (Nous Research, licence MIT « Copyright (c) 2025 Nous Research ») renommé kylin-agent-runtime 1.0.1 (pyproject.toml, fichier version) ; README : « built on Hermes » ; version Hermes de base non indiquée
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche master, HEAD 983ce8f ; têtes memory, skill) ; absent de l'archive et de l'ISO
- **Langages / taille** : Python 35 Mo (dont tests 16,7 Mo), JSON 15,7 Mo, TypeScript 3,4 Mo, YAML, JavaScript, Shell, CSS — 3615 fichiers / 91,7 Mo ; Python hors tests ≈15 Mo (agent 2,8 Mo, gateway 3,6 Mo, tools 2,6 Mo, kylin_agent_runtime_cli 3,8 Mo, plugins 1,5 Mo) — lignes de code propres mesurées : 981 090
- **Privilège** : Processus utilisateur ; exécution d'outils (terminal, code) par l'agent (PROBABLE d'après tools/terminal_tool.py, tools/code_execution_tool.py)
- **Déclencheurs** : Lancement par kylin-agent ou CLI, Planificateur cron/scheduler.py (PROBABLE), Passerelle gateway/run.py (PROBABLE)
- **Indices réseau** : Serveur web local 127.0.0.1:9119; Callbacks OAuth locaux; Adaptateurs de messagerie (Teams, WhatsApp, Feishu) (PROBABLE); Clients API LLM multiples
- **Blobs** : 2 archives skills/.curator_backups/*/skills.tar.gz (~2,1 Mo chacune) ; models/Qwen3-Embedding-0.6B (tokenizer/vocab, download_model.sh) ; fichiers d'état versionnés (.kylin-chat-approval.lock, .install_method)
- **Budget proposé** : L2 — 30 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 5/5 — Fork très volumineux (≈15 Mo de Python hors tests) avec serveurs locaux, exécution d'outils et réseau ; priorité au delta Kylin vs Hermes et aux points d'écoute.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| manifeste Python | `gitee:pyproject.toml` | name kylin-agent-runtime 1.0.1 ; authors Nous Research ; scripts kylin-agent-runtime, hermes, hermes-agent, hermes-acp ; dépendances épinglées == (openai 2.24.0, cryptography 45.0.7, PyJWT 2.12.1…) | CONSTATÉ |
| systemd --user (déprécié) | `gitee:plugins/kanban/systemd/hermes-kanban-dispatcher.service` | ExecStart=/usr/bin/env hermes kanban daemon --force --interval 60 ; WantedBy=default.target ; marqué DEPRECATED ; mécanisme d'installation non identifié | CONSTATÉ |
| ecoute-reseau | `gitee:kylin_agent_runtime_cli/web_server.py` | l.4514 host par défaut "127.0.0.1" ; l.4582 uvicorn.run(app, host=host, port=port) ; port documenté 9119 (l.8) ; contrôles d'hôte en cas de bind 0.0.0.0 (l.191-194) | CONSTATÉ |
| ecoute-reseau (callback OAuth) | `gitee:kylin_agent_runtime_cli/auth.py` | l.2305-2309 HTTPServer((host, port)) ; redirections 127.0.0.1 (xAI, Spotify :43827) | CONSTATÉ |
| ecoute-reseau (autres) | `gitee:plugins/platforms/teams/adapter.py` | FastAPI ; plugins/google_meet/node/server.py websockets.serve ; scripts/whatsapp-bridge/bridge.js .listen ; tools/mcp_oauth.py, agent/google_oauth.py http.server (non lus) | PROBABLE |
| script d'installation | `gitee:setup-kylin-agent-runtime.sh` | curl https://astral.sh/uv/install.sh ; uv/pip install -e ".[all]" | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive et de l'ISO ; cloné par kylin-agent/scripts/agent_runtime_install.sh.

### `computer-use-agent`

- **Nature** : propre-kylin — **Amont** : Kylin (Kylin-CUA, version 0.9.7 ; skill 0.9.8) ; LICENSE MIT « Copyright (c) 2025 Nous Research » ; architecture agent/message_manager + controller/registry évoquant un projet amont non identifié
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche master, HEAD 48686ad) ; absent de l'archive et de l'ISO
- **Langages / taille** : Python 915 Ko (src/agent/service.py 354 Ko), JSON — 47 fichiers / 977 Ko — lignes de code propres mesurées : 22 207
- **Privilège** : Session utilisateur ; capture d'écran et injection clavier/souris
- **Déclencheurs** : Appel par kylin-agent/agent-runtime via la skill kylin-cua (PROBABLE), Lancement manuel
- **Indices réseau** : Serveur HTTP local 127.0.0.1 port éphémère; Client LLM 127.0.0.1:18080 (configurable); oss2 / ddgs (PROBABLE)
- **Blobs** : aucun ; fichiers .DS_Store versionnés
- **Budget proposé** : L2 — 60 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 4/5 — 915 Ko de Python contrôlant le bureau (entrées, écran) avec serveur local ; hors ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| entrée CLI | `gitee:run/kylin-cua.py` | Point d'entrée Python (main) ; lanceur gitee:run/kylin-cua-launch (bash) déduisant DISPLAY/WAYLAND_DISPLAY/DBUS_SESSION_BUS_ADDRESS via loginctl et /tmp/.X11-unix | CONSTATÉ |
| configuration | `gitee:run/config.json` | Rôles brain/actor/memory/planner ; base_url http://127.0.0.1:18080/v1 ; api_key = valeur substitutive non secrète | CONSTATÉ |
| ecoute-reseau | `gitee:src/utils/status_window.py` | l.2708 ThreadingHTTPServer(("127.0.0.1", 0), Handler) port éphémère ; l.2711 URL http://127.0.0.1:<port>/ | CONSTATÉ |
| dépendances | `gitee:requirements.txt` | pyautogui, pynput, pyperclip (clavier/souris/presse-papiers), PySide6, Pillow, oss2 (Alibaba OSS), ddgs, openai, langchain | CONSTATÉ |
| déclaration de skill | `gitee:skill/kylin-cua/SKILL.md` | Skill pour agent amont (kylin-agent/agent-runtime) déclenchant l'automatisation GUI | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive et de l'ISO.
- [PROBABLE] Origine amont exacte (licence attribuée à Nous Research) non établie.

### `kylin-voice-input`

- **Nature** : propre-kylin — **Amont** : Kylin (UKUI) ; même historique que ukui-voice-wing
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement : arbre identique à gitee:ukui-voice-wing (même HEAD a97ce4d, branche openkylin/huanghe ; diff -r vide hors .git)
- **Langages / taille** : C++ 88 Ko, en-têtes 23 Ko, XML 18 Ko, TypeScript 7 Ko (traductions), QML 4 Ko, Shell 1,4 Ko — 56 fichiers / 161 Ko — lignes de code propres mesurées : 3 825
- **Privilège** : Identique à ukui-voice-wing (session utilisateur, addon fcitx5)
- **Déclencheurs** : Identiques à ukui-voice-wing
- **Indices réseau** : Identiques à ukui-voice-wing
- **Blobs** : aucun
- **Budget proposé** : L1 — 5 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Doublon exact de gitee:ukui-voice-wing ; audit mutualisé avec ce paquet.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| maintainer-script | `gitee:debian/ukui-voice-wing.postinst` | Identique à ukui-voice-wing (glib-compile-schemas) | CONSTATÉ |
| desktop-autostart | `gitee:data/org.ukui.voice-wing.desktop.in` | Identique à ukui-voice-wing (autostart XDG) | CONSTATÉ |

- [CONSTATÉ] Nom de dépôt différent mais contenu identique à ukui-voice-wing.

### `ukui-voice-wing`

- **Nature** : propre-kylin — **Amont** : UKUI SIG (Kylin) ; Vcs-Git gitee.com/openkylin/ukui-voice-wing
- **Installé par défaut (ISO 3.0)** : oui (1 binaire(s)) — **Code disponible** : archive (format 3.0 quilt, 3 patches) + gitee (branche openkylin/huanghe, HEAD a97ce4d, format native) : arbre source identique hors debian/patches et debian/source/format ; patches 0002 et 0003 déjà présents dans l'arbre (application inverse à blanc OK), 0001 non réversible
- **Langages / taille** : C++ 88 Ko, en-têtes 23 Ko, XML 18 Ko (protocole Wayland), QML 4 Ko, Shell 1,4 Ko — archive 60 fichiers / 297 Ko (debian/patches/0002-fix.patch 126 Ko) ; code propre : 25 fichiers / 111 Ko — lignes de code propres mesurées : 3 825
- **Privilège** : Session utilisateur (processus autostart + module chargé dans fcitx5) ; capture micro (PulseAudio)
- **Déclencheurs** : Ouverture de session (autostart), Démarrage de fcitx5 (OnDemand=False), Raccourci global (KGlobalAccel)
- **Indices réseau** : Aucun listener ; reconnaissance vocale via libkysdk-coreai-speech (socket /tmp/.kylin-ai-runtime-unix/<uid>/core-speech.sock) (PROBABLE)
- **Blobs** : aucun
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — 111 Ko de code propre : capture audio, addon de méthode de saisie injectant du texte, autostart ; installé par défaut.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-control | `archive:debian/control` | ukui-voice-wing ; Depends fcitx5, libkf5globalaccel-bin ; B-D libkysdk-coreai-speech-dev, libpulse-dev, libfcitx5core-dev, fcitx5-modules-dev, libkysdk-waylandhelper-dev | CONSTATÉ |
| debian-rules | `archive:debian/rules` | DEB_BUILD_MAINT_OPTIONS=hardening=+all ; dh --buildsystem=cmake | CONSTATÉ |
| maintainer-script | `archive:debian/ukui-voice-wing.postinst` | Si configure : glib-compile-schemas /usr/share/glib-2.0/schemas/ ; #DEBHELPER# ; exit 0 | CONSTATÉ |
| desktop-autostart | `archive:data/org.ukui.voice-wing.desktop.in` | Installé dans share/applications et ${CMAKE_INSTALL_PREFIX}/../etc/xdg/autostart (data/CMakeLists.txt) ; X-UKUI-AutoRestart=true ; NoDisplay=true | CONSTATÉ |
| addon fcitx5 | `archive:src/fcitx5/voice-wing.conf.in` | Type=SharedLibrary, Library=lib…-fcitx5, OnDemand=False, dépendances core+dbus ; module installé dans ${LIBDIR}/fcitx5/ (src/fcitx5/CMakeLists.txt) | CONSTATÉ |
| schéma GSettings | `archive:data/org.ukui.voice-wing.gschema.xml` | Installé dans share/glib-2.0/schemas | CONSTATÉ |
| clients D-Bus | `archive:src/` | Noms org.ukui.kylin_aiassistant, org.ukui.kylin_pearl, org.ukui.volume.control, org.ukui.style relevés par grep | PROBABLE |

- [CONSTATÉ] Installé par l'ISO : ukui-voice-wing=4.24.0.0-ok0.1k0.3 ; changelog de tête hanteng 07/09/2026.

### `kylin-ai-model-manager`

- **Nature** : propre-kylin — **Amont** : Kylin (zouxiaoyi@kylinos.cn) ; format 3.0 (quilt) avec 5 patches appliqués (.pc/applied-patches) ; debian/source/include-binaries
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee et GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : C++ 17 Ko, en-têtes 4 Ko, Python 1,5 Ko, Bash 0,6 Ko — 29 fichiers / 75 Ko ; code propre : 6 fichiers / 22 Ko — lignes de code propres mesurées : 675
- **Privilège** : Interface utilisateur ; élévation root via pkexec pour /usr/bin/download-in-venv
- **Déclencheurs** : Lancement de l'application, pkexec download-in-venv
- **Indices réseau** : Téléchargement de modèles (ModelScope, PROBABLE) et pip install modelscope (PyPI)
- **Blobs** : aucun binaire exécutable ; icône PNG
- **Budget proposé** : L3 — 90 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 2/5 — 22 Ko de code propre comprenant un chemin d'élévation polkit/pkexec et des téléchargements réseau.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| polkit | `archive:data/com.kylin.ai.model.manager.policy` | action com.kylin.ai.model.manager.download-model : allow_any=yes (commentaire « 允许所有用户执行 »), allow_inactive=auth_admin_keep, allow_active=auth_admin_keep ; org.freedesktop.policykit.exec.path=/usr/bin/download-in-venv ; installée dans share/polkit-1/actions (CMakeLists.txt:44) | CONSTATÉ |
| cible pkexec | `archive:download-in-venv` | Installé dans bin/ (CMakeLists.txt:41) ; l.1-15 lues : si $1 == /usr/bin/download.py → python3 -m venv $HOME/.cache/pythonvenv/modelenv, activation, pip install modelscope si absent | CONSTATÉ |
| script | `archive:download.py` | Installé dans bin/ (CMakeLists.txt:40) ; modelsmd5 → share/kylin-ai/ondevice/ | CONSTATÉ |
| desktop | `archive:data/kylin-ai-model-manager.desktop` | Exec=/usr/bin/kylin-ai-model-manager ; Categories Internet;Download | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-ai-model-manager ; Depends python3, python3-pip, python3-venv ; Replaces/Conflicts kylin-model-management-tools (<< 0.0.0.1-ok7) | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO ; dernière entrée changelog nile (19/08/2024).

### `kylin-model-management-tools`

- **Nature** : propre-kylin — **Amont** : Kylin (zouxiaoyi) ; format 3.0 (quilt) avec 7 patches appliqués
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : archive (source complète) ; Gitee/GitHub PRESENT (IDENTICAL-HEADS) non acquis
- **Langages / taille** : C++ 17 Ko, en-têtes 4 Ko, Python 1,5 Ko — 28 fichiers / 95 Ko ; code propre : 6 fichiers / 22 Ko — lignes de code propres mesurées : 672
- **Privilège** : Aucun (paquet de transition sans fichier installé)
- **Déclencheurs** : Aucun
- **Indices réseau** : Aucun
- **Blobs** : aucun
- **Budget proposé** : L1 — 15 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 1/5 — Paquet de transition ; vérifier seulement l'absence de fichiers installés.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| cible CMake | `archive:CMakeLists.txt` | l.25 add_executable et l.40-45 install() (download.py, download-in-venv, modelsmd5, desktop, policy) tous commentés → aucun fichier installé par CMake | CONSTATÉ |
| debian-control | `archive:debian/control` | kylin-model-management-tools ; Depends kylin-ai-model-manager (paquet de transition, patch 0007 « virtual package depends on kylin-ai-model-manager ») | CONSTATÉ |
| polkit (non installé) | `archive:data/com.kylin.ai.model.management.policy` | action com.kylin.ai.model.management.download-model : allow_any=yes, allow_inactive/active=auth_admin_keep, exec.path=/usr/bin/download-in-venv ; règle d'installation commentée | CONSTATÉ |
| debian-rules | `archive:debian/rules` | 'dh $@' seul | CONSTATÉ |

- [CONSTATÉ] Non installé par l'ISO ; dernière entrée changelog nile (12/06/2024).

### `model-management-tool`

- **Nature** : propre-kylin — **Amont** : Kylin (Zhao Kexin) ; application Tauri (Rust + React/TypeScript), identifiant org.openkylin.model-management-tool 1.0.1
- **Installé par défaut (ISO 3.0)** : non — **Code disponible** : gitee seulement (branche master, HEAD ee89598) ; absent de l'archive et de l'ISO
- **Langages / taille** : Rust 139 Ko, TypeScript 114 Ko, JSON 433 Ko (lockfiles, schémas générés), CSS 29 Ko, Shell 7 Ko — 61 fichiers / 1,28 Mo — lignes de code propres mesurées : 7 408
- **Privilège** : Application de session utilisateur
- **Déclencheurs** : Lancement utilisateur
- **Indices réseau** : Client HTTPS (reqwest/rustls) vers dépôts de modèles cloud (PROBABLE, cloud.rs non lu); Téléchargements npm/crates au build
- **Blobs** : aucun ; icônes PNG
- **Budget proposé** : L2 — 35 % (code propre (hors code tiers embarqué, traductions Qt et code généré)) — effort 3/5 — Application de session avec téléchargements réseau et import de fichiers ; hors ISO.

| Type | Fichier | Détail | Registre |
|---|---|---|---|
| debian-rules | `gitee:debian/rules` | PATH préfixé par $HOME/.cargo/bin et nvm ; npm ci \|\| npm install ; npm run build ; cargo build --release --locked --features custom-protocol ; installe /usr/bin/model-management-tool, desktop et icônes | CONSTATÉ |
| debian-control | `gitee:debian/control` | model-management-tool ; B-D webkit2gtk, libsoup, libdbus-1-dev ; Rust/Node hors Build-Depends (commentaire) | CONSTATÉ |
| configuration Tauri | `gitee:src-tauri/tauri.conf.json` | identifier org.openkylin.model-management-tool ; app.security.csp = null | CONSTATÉ |
| permissions Tauri | `gitee:src-tauri/capabilities/default.json` | core:default + window allow-minimize/toggle-maximize/close/destroy/start-dragging | CONSTATÉ |
| manifeste Rust | `gitee:src-tauri/Cargo.toml` | tauri 2, tauri-plugin-single-instance, reqwest 0.12 (rustls-tls, stream, http2), sha2, tokio, gtk 0.18 ; modules src/cloud.rs, packager.rs, device.rs | CONSTATÉ |
| manifeste npm | `gitee:package.json` | react 18, @tauri-apps/api 2 et 1.6 ; scripts vite --host 127.0.0.1 | CONSTATÉ |

- [CONSTATÉ] Absent de l'archive et de l'ISO ; changelog unique distribution « v11-2603-preview » (28/08/2026).

#### Constats transverses du lot B02-IA-moteurs-agents

- **[CONSTATÉ]** Un seul service système dans le lot : kytensor.service, sans directive User= (exécution root), activé et démarré inconditionnellement par le postinst ; kytensor-server est installé par l'ISO. — *preuve* : archive:kytensor/server/kytensor.service (ExecStart=/bin/bash -c 'source /usr/share/kylin-ai-python-env/python-env/bin/activate && /usr/bin/kytensor … --http-address=127.0.0.1 --grpc-address=127.0.0.1', Restart=always) ; archive:kytensor/debian/kytensor-server.postinst (systemctl enable kytensor.service ; systemctl start kytensor.service)
- **[CONSTATÉ]** Le service root kytensor charge les dépôts /usr/share/kylin-ai/model-repository et /opt/appdata/kylin-ai/model-repository ; kylin-ai-abstract-models crée ces répertoires sans chmod/chown explicite et y installe des backends Triton natifs (.so) et des modèles à backend Python (model.py). — *preuve* : archive:kytensor/server/kytensor.service (--model-repository=…) ; archive:kylin-ai-abstract-models/debian/postinst (mkdir -p, chmod/chown commentés) ; archive:kylin-ai-abstract-models/model_bank/CMakeLists.txt:64-84 ; …/tokenizer_gte-base_uint8-text/config.pbtxt (backend: "python")
- **[CONSTATÉ]** L'environnement Python activé par le service root kytensor est construit au build de kylin-ai-python-env par pip depuis des index tiers, 68 paquets épinglés mais sans vérification de hachage. — *preuve* : archive:kylin-ai-python-env/scripts/configure.sh (pip install --no-deps -i https://mirrors.aliyun.com/pypi/simple/ --extra-index-url https://download.pytorch.org/whl/cpu -f https://csukuangfj.github.io/kaldifeat/cpu-cn.html, pas de --require-hashes)
- **[CONSTATÉ]** La bibliothèque cpr est vendorisée à l'identique dans 11 paquets du lot et compilée en STATIC ; dans 9 d'entre eux elle est liée au moteur, dans les 2 plugins Baidu/iFlytek elle n'est qu'en include_directories. — *preuve* : md5 aca9e8bc… identique pour third-party/cpr/cpr/session.cpp dans kylin-{deepseek,qwen,sensetime,baichuan,custom,baidu,xunfei-vision,baidu-vision}-nlp/vision-engine, kylin-baidu-speech-engine, kylin-baidu-ai-engine-plugin, kylin-xunfei-ai-engine-plugin ; archive:kylin-deepseek-nlp-engine/third-party/cpr/CMakeLists.txt:35 add_library(cpr STATIC …)
- **[CONSTATÉ]** libhv v1.3.3 est vendorisée dans les 2 moteurs vocaux et ~30 de ses sources (dont event/hloop.c, base/hsocket.c, ssl/openssl.c, WebSocketClient) sont compilées directement dans les .so ; les deux copies divergent. — *preuve* : archive:kylin-baidu-speech-engine/CMakeLists.txt et archive:kylin-xunfei-speech-engine/CMakeLists.txt (add_library SHARED third-party/libhv-v1.3.3/…) ; diff http/httpdef.h : XX(AUDIO_BASIC, audio/basic, basic) présent seulement dans la copie baidu
- **[CONSTATÉ]** 10 moteurs cloud et l'extension ernie-bot contiennent des points de terminaison de services d'IA tiers en dur (DeepSeek, Alibaba DashScope, SenseNova, Baichuan, Baidu, iFlytek). — *preuve* : grep : api.deepseek.com, dashscope.aliyuncs.com, api.sensenova.cn, api.baichuan-ai.com, aip.baidubce.com, wss://vop.baidu.com, wss://spark-api.xf-yun.com, wss://iat-api.xfyun.cn, wss://rtasr.xfyun.cn, wss://tts-api.xfyun.cn, spark-api.cn-huabei-1.xf-yun.com ; archive:ukui-menu-extensions/ernie-bot/src/chat.cpp
- **[CONSTATÉ]** 4 paquets installés par l'ISO n'ont aucun code source (squelettes dh_make, debian/rules réduit à 'dh $@') : llm-backend, onnxruntime-backend, kylin-freetrial-ai-engine-plugin, kylin-freetrial-nlp-engine ; llm-backend et onnxruntime-backend sont tirés par les Depends de kytensor-client. — *preuve* : archive:llm-backend/debian/control (Depends kytensor-llm, Breaks/Replaces kytensor-llama) ; archive:onnxruntime-backend/debian/control (Depends libonnxruntime >= 1.20.1) ; archive:kytensor/debian/control (kytensor-client Depends: onnxruntime-backend, kytensor-python, llm-backend)
- **[CONSTATÉ]** 5 bibliothèques SDK ouvrent des connexions D-Bus pair-à-pair vers des sockets UNIX à chemin prévisible sous /tmp ; le serveur (runtime IA Kylin) n'appartient pas au lot. — *preuve* : archive:libkysdk-genai-nlp/src/genainlpserver.cpp:28 "unix:path=/tmp/.kylin-ai-runtime-unix/" + getuid() + "/genai-nlp.sock" ; idem genai-vision.sock, core-vision.sock, core-speech.sock (libkysdk-genai-vision, libkysdk-coreai-vision, libkysdk-coreai-speech) et kylin-coreai-embedding/src/server/*.cpp
- **[CONSTATÉ]** Les moteurs on-device sont des clients en loopback du service root kytensor (gRPC localhost:8001, HTTP localhost:8000) et dépendent de kytensor-client et kylin-ai-abstract-models. — *preuve* : grep "localhost:8001" dans kylin-ondevice-{nlp,speech,embedding}-engine, "localhost:8000" dans kylin-ondevice-vision-engine ; debian/control de ces paquets (Depends kytensor-client, kylin-ai-abstract-models)
- **[CONSTATÉ]** 9 plugins de moteur sont construits par la macro kylin_ai_add_engine_plugin(… AI_ENGINE …) du paquet CMake KylinAiEngine (libkylin-ai-engine-dev, hors lot) ; l'hôte qui les charge n'est pas dans le lot. — *preuve* : archive:kylin-{deepseek,qwen,sensetime,baidu,xunfei,baichuan,custom,ondevice,ondevice-traditional}-ai-engine-plugin/CMakeLists.txt
- **[CONSTATÉ]** 10 paquets moteurs surchargent dh_shlibdeps pour exclure libcurl4 des dépendances calculées. — *preuve* : debian/rules : override_dh_shlibdeps: dh_shlibdeps --dpkg-shlibdeps-params=-xlibcurl4 (deepseek, sensetime, baichuan, custom, baidu-nlp, baidu-speech, baidu-vision, xunfei-nlp, xunfei-speech, xunfei-vision)
- **[CONSTATÉ]** 3 paquets installent un .desktop dans /etc/xdg/autostart ; seul ukui-voice-wing est installé par l'ISO. — *preuve* : archive:kylin-asrassistant/CMakeLists.txt (DESTINATION /etc/xdg/autostart/) ; archive:ukui-intelligent-data-manager/CMakeLists.txt:154 ; archive:ukui-voice-wing/data/CMakeLists.txt (${CMAKE_INSTALL_PREFIX}/../etc/xdg/autostart)
- **[CONSTATÉ]** 2 politiques polkit du lot déclarent allow_any=yes pour exécuter /usr/bin/download-in-venv via pkexec ; seule celle de kylin-ai-model-manager est installée par CMake ; aucun des deux paquets n'est dans l'ISO. — *preuve* : archive:kylin-ai-model-manager/data/com.kylin.ai.model.manager.policy (<allow_any>yes</allow_any>, exec.path /usr/bin/download-in-venv) ; archive:kylin-model-management-tools/CMakeLists.txt:44 (install commenté)
- **[CONSTATÉ]** Des valeurs de type identifiant sont présentes dans le code source du lot (masquées ici) : une URL Git avec utilisateur:mot de passe dans deux scripts de kylin-agent, et une clé au format « sk- » + 32 hexadécimaux dans un test de kylin-custom-nlp-engine. — *preuve* : gitee:kylin-agent/scripts/agent_runtime_install.sh:46 REPO_URL_HTTPS="https://<utilisateur>:<masqué>@gitee.com/openkylin/agent-runtime.git" ; gitee:kylin-agent/scripts/agent_runtime_install_bak.sh:47 ; archive:kylin-custom-nlp-engine/test_custom_nlp_engine.cpp:31 "apiKey" : "sk-<masqué>"
- **[CONSTATÉ]** 6 dépôts du lot existent seulement sur Gitee (ni dans l'archive huanghe ni dans l'ISO) : kylin-agent, kylin-agent-armor, agent-runtime, computer-use-agent, kylin-voice-input, model-management-tool ; kylin-voice-input est un doublon exact de gitee:ukui-voice-wing. — *preuve* : fiche JSON (chemin_archive null, installe_par_defaut_iso vide) ; git log : kylin-voice-input et ukui-voice-wing tous deux à a97ce4d, diff -r vide hors .git
- **[CONSTATÉ]** Plusieurs paquets installés par l'ISO 3.0 portent en tête de changelog une distribution autre que huanghe. — *preuve* : debian/changelog : kylin-custom-ai-engine-plugin (nile), libkysdk-coreai-vision (nile), llm-backend (nile, 2025), onnxruntime-backend (nile, 2025), kylin-freetrial-ai-engine-plugin (nile, 2025), kylin-freetrial-nlp-engine (nile.bedrock)
- **[CONSTATÉ]** Sur 50 entrées du lot, 36 ont au moins un binaire installé par l'ISO officielle 3.0. — *preuve* : champ installe_par_defaut_iso de la fiche B02-IA-moteurs-agents.json (vide pour sensetime×2, baichuan×2, kylin-asrassistant, ukui-intelligent-data-manager, kylin-ai-model-manager, kylin-model-management-tools et les 6 dépôts Gitee-seuls)

