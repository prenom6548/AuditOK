# 06 — Angles morts de l'audit

🇫🇷 Français (cette page) · 🇬🇧 [English version](en/06-blind-spots.md)

> **Statut** : document produit en fin de Phase 0 (inventaire), **avant** toute lecture sémantique du code.
> Il sera complété à chaque phase. Date des relevés : 2026-10-01 (UTC).
> Registres : **[CONSTATÉ]** = preuve (fichier, index signé, réponse serveur) ; **[PROBABLE]** = indice fort non confirmé ; **[NON COUVERT]** = hors de portée de l'audit à ce stade.

Un angle mort n'est pas une accusation : c'est une zone où l'audit **ne peut pas** établir ce que fait le logiciel
distribué. L'absence de constat dans ces zones ne vaut jamais « sûr ».

---

## 1. Composants distribués dont le paquet source ne contient aucun code

**[CONSTATÉ]** 49 paquets sources de `archive.build.openkylin.top` (suite `huanghe`) ne contiennent **aucun fichier hors
`debian/`** ; 48 d'entre eux ne contiennent que le squelette `dh_make` non rempli (10 fichiers `*.ex`), alors que leurs
binaires pèsent de 100 Ko à 4,7 Go. Le binaire distribué ne peut donc pas avoir été construit à partir de la source publiée.
Liste exhaustive : [`donnees/sources-coquilles-vides.csv`](donnees/sources-coquilles-vides.csv).

**Test d'honnêteté** : `binutils-mingw-w64` apparaît dans cette liste mais relève d'un schéma Debian légitime
(construction à partir du paquet `binutils-source`) ; il est retenu comme **faux positif** de la détection automatique.

**[CONSTATÉ]** 22 autres paquets sources sans aucun code ont des binaires de moins de 100 Ko ; certains sont des
métapaquets légitimes (`meta-kde`, `xfce4`, `rust-defaults`…), mais d'autres livrent bien des fichiers exécutables ou des
bibliothèques sans source : `libkylin-ai-default-key`, `dpkg-wrapper`, `kaiming-box`, `kmre`, `liblol`/`liblol-dkms`,
`app-compat-dkms`, `kylin-kmre-image-update-x64`, `kylin-ai-cloud-model` (qui produit les trois paquets
`ai-kylin-*-cloud-model` installés par défaut), `kylin-freetrial-ai-engine-plugin`, `libime-cpis`, `wemeet`. Le seuil de
100 Ko utilisé ci-dessus sous-estime donc le phénomène ; la liste à jour est maintenue par les agents de recensement
(voir [`01a-points-entree-detail.md`](01a-points-entree-detail.md), nature « absent-ou-vide »).

### 1.1 Installés par défaut par l'ISO officielle Desktop 3.0 (build 20260918, x86_64) — 17 paquets

| Paquet source | Rôle déclaré (métadonnées) | Code ailleurs ? |
|---|---|---|
| `kylin-software-center` | Boutique d'applications (« *Software store access the server to get data* ») | Gitee `master` (5 111 fichiers) porte **un autre produit** (`ubuntu-kylin-software-center 4.5.77.1`) que la version livrée (5.0.9.1) : le code du binaire distribué n'est pas publié ; **contenu binaire audité en vague 3** (`donnees/phase1/V3-L1-boutique.json`) |
| `kylin-sso-client` | Fournit `kylin-id`, `kylin-cloud-sync` (« *UI of cloud-sync* ») | **Aucun dépôt public** ; **contenu binaire audité en vague 3** (`donnees/phase1/V3-L2-kylin-id.json`) |
| `kydoctor` | `kydoctor-server` (dépend de `libcurl4`, `sqlcipher`, `libsystemd0`) — description non renseignée | **Aucun dépôt public** |
| `openkylin-token-center` | Non décrit | **Aucun dépôt public** |
| `kylin-software-properties` | Gestionnaire de sources APT | Gitee : `debian/` seulement |
| `ostree-maintain-daemon`, `ostree-coredata-protect`, `ostree-extra-utils` | Démons/outils du système immuable OSTree | **Aucun dépôt public** |
| `llm-backend`, `onnxruntime-backend`, `onnxruntime` | Moteurs d'inférence IA | Aucun dépôt public (amont `onnxruntime` public, mais binaire non reconstruit ici) |
| `kylin-freetrial-nlp-engine` | Moteur NLP « essai gratuit » | Gitee : branche `upstream` |
| `kylin-screencap`, `kylincameralibs`, `kylin-sane-virtual-backend` | Capture d'écran, caméra, scanner virtuel | **Aucun dépôt public** |
| `kare` | Environnement de compatibilité applicative | Gitee : branche `openkylin/nile` (1 Mo) |
| `wps-office`, `kylin-fonts`, `openkylin-fonts` | Bureautique propriétaire, polices | Sans objet / aucun |

### 1.2 Non installés par défaut mais distribués — exemples à fort enjeu
`kylin-code` (IDE, 356 Mo de binaires ; code présent sur Gitee, voir §7), `kylin-mfa` (authentification multifacteur, aucun dépôt),
`kylin-driver-manager-service`, `kylin-background-upgrade`, `kylin-web-app-engine`, `kmre-apk-installer` et toute la pile
Android `kylin-kmre-*` (daemon, manager, window, emugl, display-control, modules DKMS, image système Android de 1,5 Go).

---

## 2. Dépôt `software.openkylin.top` : binaires uniquement

- **[CONSTATÉ]** Le dépôt est activé par défaut (`/.kylin-post-actions` de l'ISO, signé par la clé d'archive) mais ne publie
  **aucun index `Sources`** : 3 132 paquets binaires (composants `main` et `all`), 0 source.
- **[CONSTATÉ]** 5 paquets en proviennent dans l'installation par défaut : `net.mozilla.firefox 146.0.1` (le navigateur
  réellement installé), `kylin-cua 1.0.0` (agent « computer use »), `com.iflytek.iflyime` (méthode de saisie iFlytek),
  `app.web.openkylincommunity`, `fonts-urw-base35`.
- **[CONSTATÉ]** Contenus notables (non installés par défaut, livrables via la boutique) : `audit-kylin` (description :
  outil de « vérification d'informations classifiées », analyse hors ligne et en ligne), `kguard` (« *Kylin lightweight HIDS* »),
  `kguard-net-ai` (« *AI-powered network traffic detection engine* »), agents de sécurité tiers (`cn.com.topsec.edragent`,
  `cn.com.topsec.topnac`, `com.360.epp`, `cn.huorong.esm`, `cn.com.sangfor.atrust`), modèles IA « cloud »
  (`ai-kylin-deepseek-*-cloud-model`, `ai-kylin-qwen-plus-cloud-model`), « compétences » IA (`ai-kylin-skill-*`).
- **[CONSTATÉ]** L'index `Packages` comporte des champs non conformes (`Installed-Size: 1.9 MB`), signe d'un outillage de
  publication hors chaîne Debian standard.
- **[NON COUVERT]** Le comportement de ces 3 132 binaires est hors de portée de l'audit statique.

## 3. Composants d'origine Kylin sans aucun dépôt public

**[CONSTATÉ]** 74 paquets sources d'origine Kylin publiés dans `archive.build` n'ont de dépôt ni sur Gitee ni sur GitHub
(absent ou vide), dont **35 installés par défaut** — notamment `kydoctor`, `kylin-sso-client`, `kylin-bot`, `kylin-wget`,
`openkylin-token-center`, `libkysdk-mcp`, `libkyai-business-framework`, `kylin-ai-cloud-model`, `ostree-*`,
`ukui-service-manager`, `kytensor`, `llm-backend`. Pour ceux dont le paquet source contient du code — p. ex. `kylin-wget`
(85 fichiers, Rust/Python), `libkysdk-mcp` (120 fichiers, C++), `kylin-bot` (1 629 fichiers, Rust/TypeScript/JavaScript) —
l'archive reste la seule référence ; l'historique de développement est inaccessible.
Liste : colonne `gitee`/`github` de [`donnees/inventaire-paquets-sources.csv`](donnees/inventaire-paquets-sources.csv).

## 4. Forges non publiques référencées par les métadonnées

- **[CONSTATÉ]** 6 paquets sources déclarent un `Vcs-Git` sur `gitlab2.kylin.com` (forge interne, non joignable
  publiquement) : `kylin-ai-abstract-models`, `kylin-ondevice-vision-engine`, `kylin-fix-boot-grub`, `libkysdk-desktop`,
  `libkysdk-diagnosetest` (qui produit `libkysdk-datacollect`, installé par défaut), `ukui-kwin-effects`.
- **[CONSTATÉ]** 193 entrées binaires déclarent un `Homepage` sur `gitlab2.kylin.com` (25 sources, dont
  `kylin-process-manager`, `kylin-software-center`, `kylin-connectivity`, `libkysdk-base`, `libkysdk-system`) et 20 sur
  `gogs.kylin.com` (biométrie).
- **[PROBABLE]** Le développement de référence de ces composants se fait sur une forge interne ; Gitee en serait un
  reflet partiel. L'égalité « Gitee = forge interne = binaire » n'est pas démontrable de l'extérieur.

## 5. Binaires précompilés présents dans les arbres sources (« blobs »)

**[CONSTATÉ]** L'extraction structurelle des composants P1 recense 604 fichiers binaires (ELF, archives, paquets `.deb`,
`.vsix`, `.whl`, modèles `.onnx`/`.gguf`, bases SQLite), soit 1,40 Go. Exemples non reconstruits depuis les sources présentes :

| Arbre | Fichier | Nature |
|---|---|---|
| `kylin-connectivity` (archive) | `3rdparty/tools/amd/anti-control-server`, `anti-control-client` | ELF x86-64 (12 Mo / 10 Mo) |
| `kylin-connectivity` (archive) | `3rdparty/tools/{amd,arm}/kylin-screencast.AppImage` | ELF/AppImage (81–86 Mo) |
| `yhkylin-systembackup-tools` (archive, **installé par défaut**) | `systembackup-daemon/systembackup-daemon` | ELF (26 Mo) |
| `yhkylin-backup-tools`, `yhkylin-rockbackup-tools` | `kybackup/kybackup`, `backup-daemon/backup-daemon` | ELF (15–29 Mo) |
| `revisions-manager` (archive) | `daemon/data/depends/*/docker.io_*.deb`, `containerd_*.deb` | paquets `.deb` embarqués |
| `kylin-ai-business-framework-service` | `python-depends/*/numpy-*.whl`, `transformers-*.whl` | wheels Python précompilées |
| `kylin-camera` | `camera` | ELF (12,9 Mo) |
| `extensions-repo` (Gitee, kylin-code) | 66 fichiers `.vsix` (Java, Gradle, Maven, Python, remote-dev…) | extensions empaquetées |
| `kylin-software-center` (Gitee) | `data/uksc.db` | base SQLite (14,7 Mo) |

Détail complet : champ `blobs` de [`donnees/points-entree-extraction-structurelle.json`](donnees/points-entree-extraction-structurelle.json).
**[NON COUVERT]** Le contenu de ces fichiers ne peut pas être audité au niveau source ; seuls `file`/`strings`/`readelf`
seront possibles (Phase 1-D).

## 6. Écart entre ISO officielle et dépôts publiés

- **[CONSTATÉ]** ISO Desktop 3.0 x86_64 du 18/09/2026 : 1 920 paquets installés ; 1 651 proviennent de `archive.build/main`,
  **143 uniquement du PPA `anything3.0`** (clé distincte « *openKylin Build Service PPA for kylinsoft developers* »,
  `Origin: LP-PPA-kylinsoft-anything3.0`), 13 de `pty`, 5 de `software.openkylin.top`.
- **[CONSTATÉ]** 18 binaires de l'ISO ne sont plus indexés à leur version exacte ; une douzaine restent dans le `pool`,
  **5 en ont été retirés (HTTP 404)** dont `openkylin-token-center 1.0.0-ok1.7` et `ostree-coredata-protect 2.0.0.0-0k2.23.6.0`,
  et `kylin-update-notify` n'apparaît dans **aucun** index.
- **[CONSTATÉ]** Pour 76 paquets installés, aucune source à la version exacte n'est publiée ; exemples notables :
  `policykit-1 124-ok6.1` (la source publiée est en 127 et ne produit plus ce binaire), `coreutils 9.4-ok4`,
  `libsqlcipher1 4.5.6-ok1`.
- **[CONSTATÉ]** Au total, **76 composants candidats sont classés X** (non auditables au niveau source), dont **36
  installés par défaut**.
- **[CONSTATÉ]** `/.kylin-post-actions` installe des pilotes NVIDIA par `dpkg -i /opt/nvidia/nvidia-*.deb` (hors
  vérification de signature APT) ; ces `.deb` résident dans le système de fichiers de l'ISO.
- **[NON COUVERT]** Le `filesystem.squashfs` (3,2 Go) n'a pas été extrait : l'audit statique ne vérifie pas que les fichiers
  installés correspondent octet pour octet aux paquets publiés (à traiter en Phase 4/5).
- **[NON COUVERT]** Variantes non examinées : ISO arm64, loong64, « AgentOS », Server, Embedded, WSL.
- **[CONSTATÉ]** Seules des empreintes **MD5** sont publiées (`www.openkylin.top/md5.html`), sans nom de fichier ni date de
  build alors que trois ISO Desktop x86 3.0 coexistent ; aucune signature détachée sur deux miroirs.

## 7. Forges : énumération incomplète et divergences

- **[CONSTATÉ]** L'organisation Gitee `openkylin` annonce **4 770 dépôts** ; l'énumération anonyme est bloquée au-delà de
  400 (redirection vers la connexion) et l'API d'organisation renvoie HTTP 500. Les dépôts ont donc été vérifiés **par nom**
  (3 404 noms testés) : 3 139 dépôts Gitee identifiés sur 4 770 annoncés (≈ 66 %) ; environ 1 630 dépôts, non liés
  à un paquet distribué, n'ont pas été identifiés.
- **[CONSTATÉ]** Sur 2 778 dépôts présents sur Gitee et GitHub, **1 693 ont des têtes de branches divergentes** ; 156 sont
  vides sur GitHub, 190 absents. L'antériorité (miroir en retard vs contenu différent) n'est pas encore établie.
- **[CONSTATÉ]** Une troisième forge est utilisée : `gitcode.com/openkylin` (Vcs-Git du noyau `linux 7.0.0-3.0ok1` et
  d'`openhitls`).
- **[CONSTATÉ]** Les bibliothèques `libkylin-ai-system-private` et `libkysdk-ai-private` ont un dépôt Gitee réduit à un
  README modèle ; **leur code est en revanche publié dans le PPA** (paquet source avec C++ et protocoles gRPC/D-Bus).
- **[CONSTATÉ]** Une archive concurrente `archive.openkylin.top` publie une suite `huanghe` différente (daté du 28/08/2026,
  3 042 sources) de celle effectivement configurée (`archive.build.openkylin.top`, 30/09/2026, 3 238 sources).

## 8. Forks d'amont (P2) : limites à ce stade

- **[CONSTATÉ]** 334 paquets sont en format **natif** (`3.0 (native)`) : la frontière code amont / modifications
  openKylin est effacée ; l'audit différentiel exigera de reconstruire l'arbre amont.
- **[CONSTATÉ]** 322 paquets ont un `orig.tar` de **même nom mais d'empreinte différente** de Debian/Ubuntu (dont, parmi les
  installés : `glibc`, `curl`, `gnutls28`, `grub2`, `dbus`, `linux`, `nettle`, `libseccomp`, `krb5`, `policykit-1`, `sqlite3`,
  `python-cryptography`, `tzdata`). Cela peut relever d'un simple ré-empaquetage ; à départager en Phase 2.
- **[NON COUVERT]** En Phase 0, seules les parties `debian/` des forks P2 ont été acquises (les arbres amont complets le
  seront en Phase 2).

## 9. Composants tiers propriétaires (P1-X)

**[NON COUVERT]** `wps-office`, `sogoupinyin`, `crossover`, `cpis`/`libime-cpis`, pilotes `arise`/NVIDIA, cuDNN/CUDA,
microcodes Intel/AMD, `linux-firmware`, modèles IA (`kylin-qwen2.5-3b-*`, `kylin-*-model-onnx`, `kylin-datamanagement-models`,
3,3 Go), image Android `kylin-kmre-image-data-x64` (1,5 Go), bases KARE (1,7 Go). Leur comportement ne peut être établi
que par analyse binaire et dynamique (Phase 5).

## 10. Limites méthodologiques de la Phase 0

- Classement « origine Kylin » par heuristique (noms, mainteneurs `kylinos.cn`, format natif absent de Debian/Ubuntu) :
  des faux positifs/négatifs sont possibles ; la liste a été complétée par contrôle croisé avec l'ISO (v3, 368 candidats).
- Détection des points d'entrée par motifs de chemins et signatures de contenu, sans exécution ; les indices d'écoute réseau
  sont des correspondances lexicales à confirmer.
- Seule l'architecture **amd64** a été indexée ; arm64, loong64, riscv64 et i386 ne sont pas couverts.
- Sources de vérité utilisées : index APT signés (clé `09FF C10E A273 DD29 A986 B110 8B31 3CEA FF59 2D96`), ISO lue par
  requêtes partielles, forges consultées en lecture. Aucune de ces sources ne prouve, à elle seule, l'égalité
  « code publié = binaire exécuté ».

## Angles morts révélés par les Phases 1-2 et la vague 3 (relevés des 2026-10-01 et 2026-10-02)

| Angle mort | Portée | Source / preuve |
|---|---|---|
| **`coreutils 9.4-ok4`** : binaire installé par défaut (commandes de base `ls`, `cp`, `rm`, `chmod`…) **sans source dans aucun des 10 index `Sources` signés** ; seule la 9.5 (`gnu-coreutils`, préfixée) est publiée et auditée | binaires les plus exécutés du système, inauditables statiquement | `donnees/phase2/P2-index-sources.json` ; manifeste ISO l.56 |
| **`libssl1.1`** (OpenSSL 1.1.1f, fin de vie) installée ; archive amont repaquetée absente du corpus | correctifs CVE postérieurs à 2020 non vérifiables nominativement | `donnees/phase2/P2-openssl.json` |
| **Archives amont** absentes ou différentes d'Ubuntu : curl, gnutls28 (`.orig` ≠ Ubuntu), noyau 7.0, Triton (kytensor), llama.cpp (kytensor-llm) | delta openKylin hors `debian/` non vérifiable | `donnees/phase2/`, `donnees/phase1/V1-L13-*` |
| **Greffons IA sans source** : `libkylin-ai-default-key`, `kylin-freetrial-nlp-engine`, `llm-backend`, `onnxruntime-backend` | destination réelle, TLS et consentement du modèle cloud d'essai par défaut | `donnees/phase1/V1-L8-*`, `V2-L4-*` |
| **Liste blanche `.limit`** : le composant qui l'appliquerait (dbus/kysdk patché) est absent du corpus ; la configuration livrée la désactive (`audit: false`) | contrôle d'accès réel des services système | `donnees/verification/G2-*`, `G5-*` |
| **Services D-Bus tiers invoqués** non audités : `org.ukui.UniauthBackend`, `com.kylin.kim.backend`, `com.kylin.RemoteDesktop` | authentification à l'écran de verrouillage, accès distant | `donnees/phase1/V2-L1-*` |
| **Binaires externes** : `kylinsigntool` (vérification de signature des `.deb`), `kylin-os-manager-remote-user` (assistance à distance), blob `ud650-extra.so` | chaînes de confiance et d'accès distant | `donnees/phase1/V1-L5-*`, `V2-L3-*` |
| **Conditions d'exécution** non tranchables statiquement : permissions réelles de `rules.d` kaiming, umask des sockets IA, protections « Local Network Access » de Firefox 146, postes d'usine en `ple-mode` | atteignabilité de plusieurs constats | `07-verification.md` (INDÉTERMINÉ), `04-protocole-dynamique.md` |
| **`ostree-extra-utils`** (installé par défaut) et **`dpkg-wrapper`** : source publiée réduite à une coquille `dh_make`, générée après coup (10 versions côté binaire, une seule côté source) ; ELF `unified-conf` caractérisé sans source | contenu réel et historique non traçables au source | `donnees/phase2/P2-binaires-sans-source.json` |
| **Droits réels de `/opt/system`** et `/var/opt/system`, désormais ajoutés aux chemins de bibliothèques, de Python, de `PATH` et de systemd ; exposition de `kare` (source publiée = `debian/` seul) | inscriptibilité par un non-root = exécution de code root | kit Phase 5 (D16), `donnees/verification/G9-*` |
| **Compilateur `gcc-15` d'openKylin** et journaux de construction (hors corpus) | cause de l'absence de PIE et de la compilation non optimisée | `donnees/phase2/P2-durcissement-binaires.json`, `donnees/verification/mesures-G9/` |
| **Images EFI de GRUB livrées** et infrastructure de signature (`grub2-signed` télécharge des images non publiées) ; chemin `os-prober` → `grub-mount` (même pilote NTFS, en root) | exposition réelle des CVE GRUB | `donnees/phase2/P2-grub2*.json`, `donnees/verification/G8-*`, kit (D18) |
| **aptdaemon ok1 à ok5** (openKylin 1.x) : `debian/` et binaires indisponibles | exposition passée à la méthode `SetEnviron` | `donnees/phase2/P2-outils-ubuntu.json` |
| **`kylin-system-updater` `GetBackendStatus`** (ouvert à tous) réécrit `LANG`/`LANGUAGE` du démon root : piste relevée pendant la vérification G8, non instruite | effet réel sur le démon root | `donnees/verification/G8-*`, kit (script 20, introspection) |
| **Exécutables compilés de la boutique et de kylin-id** : caractérisés (`readelf`, chaînes, désassemblage ciblé), non décompilés intégralement | comportements hors des chemins examinés | `donnees/phase1/V3-L1-*`, `V3-L2-*` |
| **Serveur de « stratégies » de la boutique** (`api.kylinos.cn`) : contenu réellement poussé, critères de ciblage, fréquence | usage effectif du canal de gestion à distance | `donnees/verification/G10-*` |
| **Service d'identité côté serveur** (`id.kylinos.cn`) : limitation des envois de SMS, émission des jetons | exposition effective de kylin-id | `donnees/verification/G11-*` |
| **Bus système installé** : configuration effective de `dbus` et absence de tout composant appliquant `.limit` à l'exécution | atteignabilité réelle de `kpr-backend` et des autres services ouverts | `donnees/verification/G12-*` |
| **Interfaces de session UKUI** (lots V3-L4 à V3-L7) : services système lus intégralement, interfaces de session examinées par recherche ciblée (2 à 8 % des lignes lues) | l'absence de flux réseau repose sur ce tri, non sur une lecture exhaustive | `donnees/phase1/V3-L4-*` à `V3-L7-*` |
| **Pile Bluetooth amont** (`bluez`, `obexd` : seul `debian/` dans le corpus) : filtrage éventuel des noms de fichiers reçus, exigence d'appairage | atteignabilité réelle du constat Bluetooth | `donnees/verification/G14-*` |
| **État installé du moniteur système** (capacités du binaire, options de montage de `/usr`) | sévérité réelle du constat resté indéterminé | `donnees/verification/G13-*` |
| **KMRE et kylin-code** : sources d'archive vides ; les versions publiées sur Gitee diffèrent toutes des versions distribuées (généralement plus anciennes). Seul `kare` (environnement d'exécution d'applications) est installé par défaut ; les autres paquets KMRE, dont le module noyau, et kylin-code ne le sont pas. Paquets binaires KMRE inventoriés (chaîne signée, sans exécution), non analysés | non audités | § 1, `donnees/phase1/vague3/V3-L8-kmre-binaires.json` |

**Résolu en vague 3** : `org.kylinID.service` (kylin-id), cité jusqu'ici comme non audité, l'a été, de même que
`org.kylin.KprBackend` (kba) (`donnees/phase1/V3-L2-*`, `V3-L3-*`) — la boutique et kylin-id, faute de source, sur
leur contenu binaire. 24 composants UKUI installés par défaut ont en outre été audités (lots V3-L4 à V3-L7).

**Résolu en Phase 2** : `sudo-common 1.2ok1` (porte `/etc/sudoers` et la PAM de sudo), absent du corpus initial,
a été acquis par la chaîne signée et audité intégralement (`donnees/phase2/P2-sudo-common.json`). Les archives
amont de glibc, grub2, dpkg, network-manager, wpa, aptdaemon et apport ont été comparées à des références Ubuntu,
Debian et GNU signées (`outils/references_p2.py`, `donnees/phase2/references/`).
