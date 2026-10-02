# 01 — Inventaire et matrice de couverture (Phase 0)

> **Statut** : livrable de Phase 0, **validé** et conservé comme état initial de l'audit. L'état actuel (vagues 1 à 3,
> Phase 2, vérification contradictoire) figure dans [`00-synthese.md`](00-synthese.md) (mise à jour du 2026-10-02).
> Relevés effectués le 2026-10-01 (UTC). Registres : **[CONSTATÉ]**, **[PROBABLE]**, **[NON COUVERT]**.
> Données brutes : [`donnees/`](donnees/) — Outils de reproduction : [`outils/`](outils/).

---

## 0. Résumé de la Phase 0

| Indicateur | Valeur | Registre |
|---|---|---|
| Dépôts openKylin présents localement au démarrage | **0** (dépôt de travail vide) | CONSTATÉ |
| Version auditée | openKylin **3.0 « huanghe »** (ISO Desktop x86_64 du 18/09/2026 ; archive du 30/09/2026) | CONSTATÉ |
| Sources APT configurées par défaut sur un système installé | 5 entrées sur 3 hôtes HTTP (`archive.build` : huanghe, -security, -updates ; `software.openkylin.top` ; PPA `anything3.0`) | CONSTATÉ |
| Paquets sources de l'archive par défaut (`archive.build`, huanghe) | **3 238** (main 3 190, pty 45, cross 2, security 1) | CONSTATÉ |
| … dont d'origine Kylin (heuristique documentée) | 306 | PROBABLE (heuristique) |
| … dont `orig.tar` identique octet pour octet à Debian/Ubuntu | 2 112 | CONSTATÉ |
| Paquets installés par l'ISO | 1 920 binaires (1 035 sources) | CONSTATÉ |
| Candidats retenus pour l'audit | **368** (P1 : 278 ; P2 : 75 ; P3 : 15) | — |
| Sources acquises et authentifiées (SHA-256 contre index signés) | 311 arbres d'archive (4,9 Go) + 40 dépôts Gitee (2,0 Go) ; 0 échec | CONSTATÉ |
| Paquets sources « coquilles vides » (aucun code, binaire ≥ 100 Ko) | 49 (dont 17 installés par défaut), + 22 à faible charge binaire | CONSTATÉ |
| Points d'entrée recensés (Phase 0.2) | 329 composants, 14 lots + contrôle critique (aucun omis ; 34 écarts traités ; 21 points d'entrée ajoutés) | CONSTATÉ |
| Composants classés X (non auditables au niveau source) | 76, dont 36 installés par défaut | CONSTATÉ |
| Binaires sans aucune source (`software.openkylin.top`) | 3 132 (dont 5 installés par défaut) | CONSTATÉ |

Les angles morts sont détaillés dans [`06-angles-morts.md`](06-angles-morts.md).

---

## 1. Méthode et sources de vérité

### 1.1 Constat de départ
**[CONSTATÉ]** Le dépôt de travail `prenom6548/AuditOK` était vide (aucun commit) : aucun dépôt openKylin n'était présent
localement. L'inventaire a donc été construit à partir des sources publiques, en privilégiant **ce qui est réellement
distribué** plutôt que ce qui est hébergé.

### 1.2 Sources consultées (lecture seule)

| Source | Usage | Authenticité |
|---|---|---|
| `http://archive.build.openkylin.top/openkylin/` (suites `huanghe`, `-security`, `-updates`, `-proposed`) | Index `Sources`/`Packages` de référence (dépôt configuré par défaut) | `InRelease` signé — « openKylin Archive Signing Key », `09FF C10E A273 DD29 A986 B110 8B31 3CEA FF59 2D96` — signature vérifiée par `gpgv` |
| `http://software.openkylin.top/openkylin/` (`huanghe`, `main all`) | Dépôt applicatif (boutique) | `InRelease` signé, même clé — vérifié |
| `http://ppa.build.openkylin.top/kylinsoft/anything3.0/openkylin/` | PPA activé par défaut | `InRelease` signé, clé « *openKylin Build Service PPA for kylinsoft developers* » `3EA5 5C18 3BD8 90B2 D91C 3A20 2636 AEDC 49E9 E2C8` — vérifié |
| `http://archive.openkylin.top/openkylin/` | Archive concurrente (comparaison) | `InRelease` signé, clé d'archive — vérifié |
| ISO `openKylin-Desktop-V3.0-20260918-x86_64.iso` (miroir `mirrors.nju.edu.cn`) | Manifeste des paquets installés, scripts post-installation | Lecture ISO 9660 par requêtes HTTP partielles ; empreinte de l'ISO **non vérifiable** (seules des MD5 non nominatives sont publiées) |
| `gitee.com/openkylin` (4 770 dépôts annoncés) | Forge canonique : présence, branches, étiquettes `build/<version>`, clones superficiels | `git ls-remote` / `git clone --depth 1` en HTTPS |
| `github.com/openkylin` | Miroir : comparaison des têtes de branches | `git ls-remote` en HTTPS |
| `gitcode.com/openkylin` | Forge du noyau et d'`openhitls` (découverte en cours d'inventaire) | `git ls-remote` |
| Debian (sid, trixie, bookworm — `deb.debian.org`) et Ubuntu (jammy, noble, plucky, questing, resolute — `archive.ubuntu.com`) | Identification de l'amont et comparaison des empreintes `orig.tar` | Index téléchargés en HTTPS/HTTP |
| `www.openkylin.top/support/privacy_policy-cn.html` | Référence pour la Phase 3 | Page archivée par empreinte SHA-256 (`donnees/politique-confidentialite/`) |

### 1.3 Chaîne d'acquisition reproductible
1. Téléchargement des `InRelease`, vérification de signature (`gpgv`) avec les clés extraites des paquets keyring de l'archive.
2. Téléchargement des index `Sources.gz`/`Packages.gz`, vérification de leur SHA-256 contre l'`InRelease` signé.
3. Téléchargement des fichiers sources (`.dsc`, `.orig.tar.*`, `.debian.tar.*`), vérification SHA-256 contre l'index, puis
   extraction par `dpkg-source -x` (corrections quilt appliquées : c'est l'arbre qui est compilé).
4. Clones Gitee superficiels (`--depth 1`) sur la branche `openkylin/huanghe` si elle existe, sinon la branche par défaut ;
   commit relevé dans `outils`/journal.
5. Empreinte de chaque arbre analysé (SHA-256 de la liste triée des SHA-256 de fichiers) :
   [`donnees/empreintes-arbres-sources.tsv`](donnees/empreintes-arbres-sources.tsv). Toute analyse ultérieure devra
   rejouer cette empreinte pour garantir qu'elle porte sur le même corpus.

Les scripts sont dans [`outils/`](outils/) (variables `AUDIT_WORK` et `AUDIT_SRC`). Le corpus lui-même (≈ 8,4 Go) n'est pas
versionné ; il se reconstruit à l'identique tant que l'archive conserve les mêmes versions.

---

## 2. Paysage de distribution réellement configuré

**[CONSTATÉ]** Le script `/.kylin-post-actions` de l'ISO (copie : [`donnees/iso-3.0/dot-kylin-post-actions`](donnees/iso-3.0/dot-kylin-post-actions))
supprime la configuration APT de construction (dont `/etc/apt/apt.conf.d/00secure`) et écrit les sources suivantes :

| Dépôt (protocole) | Suites / composants | Clé `Signed-By` | Contenu au 30/09/2026 |
|---|---|---|---|
| `archive.build.openkylin.top/openkylin` (**HTTP**) | `huanghe` main cross pty | clé d'archive | 3 237 sources ; 13 341 binaires amd64 |
| idem | `huanghe-security` | clé d'archive | **1 seule source** (`python-librt`) |
| idem | `huanghe-updates` | clé d'archive | vide (dernière date : 15/10/2024) |
| `ppa.build.openkylin.top/kylinsoft/anything3.0` (**HTTP**) | `huanghe` main | clé PPA distincte | 37 252 sources ; 61 744 binaires amd64 |
| `software.openkylin.top/openkylin` (**HTTP**) | `huanghe` main all | clé d'archive | **0 source** ; 3 566 binaires amd64 (3 132 noms uniques) |

Observations factuelles :
- **[CONSTATÉ]** Le transport est HTTP pour les trois hôtes ; l'intégrité repose sur les signatures OpenPGP des `InRelease`
  (vérifiées ici) et les empreintes SHA-256 qu'ils contiennent. Aucun champ `Valid-Until` n'est présent (protection contre le
  rejeu d'index anciens à examiner en Phase 4).
- **[CONSTATÉ]** Il n'existe pas de canal de sécurité effectif : les correctifs arrivent directement dans `huanghe`.
- **[CONSTATÉ]** Le PPA `anything3.0` porte `Origin: LP-PPA-kylinsoft-anything3.0` (nommage de type Launchpad) ; 143 paquets
  de l'installation par défaut n'existent que dans ce PPA.
- **[CONSTATÉ]** Le paquet `openkylin-archive-anything` installe aussi la clé PPA dans `/etc/apt/trusted.gpg.d/`
  (confiance globale, non restreinte par `Signed-By`) et une source `nile` (2.0) ; le paquet `ubuntukylin-keyring` installe
  par lien symbolique une clé Ubuntu Kylin de 2014 sans expiration dans `trusted.gpg.d`.
- **[CONSTATÉ]** `openkylin-keyring` contient, outre la clé d'archive, une clé « *Kylin Deb Signing Key `<release@kylinos.cn>`* »
  (`ED6B 59E1 119F E97E C326 743E 5CFF 40DB 5578 1542`, créée le 09/01/2026, sans expiration) sous
  `/usr/share/keyrings/deb-sign/` — usage à établir (Phase 4).
- **[CONSTATÉ]** L'archive `archive.openkylin.top` publie une suite `huanghe` **différente** (28/08/2026, 3 042 sources,
  versions plus anciennes : p. ex. `systemd 255.2` contre `259.5`) : un tiers qui se fierait à cet hôte n'auditerait pas
  ce qui est installé.

Preuves : [`donnees/apt/`](donnees/apt/) (InRelease signés, sorties `gpgv`, empreintes des index, empreintes des clés).

---

## 3. Inventaire global de l'archive par défaut (3 238 paquets sources)

Table complète, une ligne par paquet source : [`donnees/inventaire-paquets-sources.csv`](donnees/inventaire-paquets-sources.csv)
(version, format, présence Debian/Ubuntu, origine Kylin, comparaison `orig.tar`, taille, coquille vide, binaires installés
par l'ISO, présence Gitee/GitHub, relation des têtes de branches, étiquettes `build/`, Vcs-Git, mainteneur, priorité).

| Classement | Nombre | Commentaire |
|---|---|---|
| Présents dans Debian sid/trixie/bookworm ou Ubuntu jammy→resolute | 2 960 | forks ou recompilations d'amont |
| Absents de Debian et d'Ubuntu | 277 | composants propres, anciennes versions, ou tiers |
| Format `3.0 (quilt)` / `3.0 (native)` / `1.0` | 2 900 / 334 / 3 | les natifs effacent la frontière amont/delta |
| `orig.tar` **identique** (SHA-256) à Debian/Ubuntu | 2 112 | code amont prouvé inchangé ; seul `debian/` diffère |
| `orig.tar` de même nom, **empreinte différente** | 322 | à départager en Phase 2 (ré-empaquetage ou modification) |
| `orig.tar` sans homologue de même nom | 469 | versions absentes des distributions de référence |
| Origine Kylin (heuristique) | 306 | dont 196 installés par défaut |
| Sans dépôt Gitee homonyme | 190 | dont 74 d'origine Kylin sans **aucun** dépôt public |

Heuristique « origine Kylin » : nom (`kylin`, `ukui`, `peony`, `kysdk`, `kmre`, `kare`, `kaiming`, `openkylin`, `biometric`…),
mainteneur `@kylinos.cn`, ou paquet natif absent de Debian/Ubuntu. Elle a été contrôlée par croisement avec le manifeste de
l'ISO (40 paquets ajoutés en v3).

---

## 4. Forges

| Constat | Valeur | Registre |
|---|---|---|
| Dépôts annoncés par l'organisation Gitee `openkylin` (compte entreprise `openkylin-enterprise`) | 4 770 | CONSTATÉ |
| Énumération anonyme possible | 400 (au-delà : redirection vers la connexion ; API d'organisation : HTTP 500) | CONSTATÉ |
| Noms vérifiés par `git ls-remote` (Gitee + GitHub) | 3 404 | CONSTATÉ |
| Dépôts Gitee identifiés | 3 139 (≈ 66 % des 4 770) | CONSTATÉ |
| Présents sur Gitee et GitHub, têtes identiques / divergentes | 1 085 / **1 693** | CONSTATÉ |
| Présents sur Gitee, vides ou absents sur GitHub | 156 / 190 | CONSTATÉ |
| Absents des deux forges | 263 | CONSTATÉ |
| Forge supplémentaire | `gitcode.com/openkylin` (noyau `linux`, `openhitls`) | CONSTATÉ |
| Forges internes référencées (`Vcs-Git`, `Homepage`) | `gitlab2.kylin.com` (6 Vcs-Git, 193 Homepage), `gogs.kylin.com` (20 Homepage) | CONSTATÉ |
| Étiquettes `build/<version>` sur Gitee | présentes pour la plupart des composants UKUI/IA (permettent d'aligner le code Gitee sur le binaire) | CONSTATÉ |

Détail par dépôt : [`donnees/forges-gitee-github.csv`](donnees/forges-gitee-github.csv).
**[NON COUVERT]** L'antériorité des branches divergentes (GitHub en retard vs contenu différent) n'est pas établie en Phase 0 ;
elle le sera pour chaque dépôt audité en Phase 1/2 (`git merge-base`).

---

## 5. Composants P1 (propres à Kylin) — inventaire et points d'entrée

### 5.1 Synthèse par groupe

| Groupe | Composants | Installés par défaut | Code : complet / partiel-Gitee / aucun | Lignes de code (corpus) | Surface installée par défaut : unités système / unités utilisateur / D-Bus système / polkit / PAM / autostart / timers+cron / initramfs+ld.so | Budget L3 / L2 / L1 / X |
|---|---|---|---|---|---|---|
| IA (kylin-ai-*, moteurs, agents) | 78 | 55 | 64 / 6 / 8 | 2 567 801 | 1 / 5 / 0 / 1 / 0 / 2 / 0 / 0 | 37 / 18 / 15 / 8 |
| kylin-code (IDE) | 2 | 0 | 0 / 2 / 0 | 1 855 433 | 0 / 0 / 0 / 0 / 0 / 0 / 0 / 0 | 0 / 1 / 1 / 0 |
| Boutique, installation d'applications | 11 | 4 | 5 / 1 / 5 | 187 988 | 3 / 3 / 2 / 2 / 0 / 1 / 3 / 0 | 3 / 1 / 1 / 6 |
| Updateur maison | 6 | 2 | 5 / 0 / 1 | 65 849 | 2 / 0 / 1 / 2 / 0 / 2 / 0 / 0 | 2 / 2 / 1 / 1 |
| Installateur, outils ISO/sauvegarde | 7 | 5 | 6 / 1 / 0 | 161 640 | 2 / 0 / 3 / 3 / 0 / 0 / 0 / 4 | 4 / 3 / 0 / 0 |
| Android (KMRE) et compatibilité (KARE) | 15 | 1 | 1 / 8 / 6 | 1 196 127 | 1 / 0 / 3 / 0 / 0 / 2 / 0 / 0 | 3 / 4 / 2 / 6 |
| Gestion des processus (« Graded Freeze ») | 5 | 4 | 4 / 0 / 1 | 43 394 | 4 / 2 / 3 / 0 / 0 / 0 / 0 / 0 | 3 / 1 / 0 / 1 |
| Télémétrie, diagnostic, identité, sécurité | 30 | 23 | 22 / 2 / 6 | 1 080 844 | 13 / 0 / 8 / 8 / 1 / 17 / 0 / 3 | 10 / 10 / 4 / 6 |
| UKUI cœur et services de session | 43 | 36 | 43 / 0 / 0 | 1 892 320 | 6 / 3 / 9 / 4 / 2 / 22 / 0 / 0 | 11 / 25 / 7 / 0 |
| Système immuable OSTree | 4 | 3 | 1 / 0 / 3 | 136 | 6 / 0 / 0 / 1 / 0 / 1 / 0 / 5 | 1 / 0 / 0 / 3 |
| Applications Kylin, thèmes, polices | 38 | 33 | 33 / 1 / 4 | 1 126 070 | 1 / 1 / 1 / 0 / 0 / 4 / 0 / 1 | 6 / 14 / 14 / 4 |
| Binaires tiers / propriétaires, modèles | 39 | 17 | 0 / 0 / 39 | 0 | 0 / 0 / 0 / 0 / 0 / 0 / 0 / 0 | 0 / 0 / 0 / 39 |

Lecture de la ligne « surface installée » : nombre de fichiers de points d'entrée effectivement posés sur un système
installé depuis l'ISO 3.0, mesuré dans l'index `Contents-amd64` signé (composant `main`) et, pour les composants `pty` et
`software`, dans les listes de fichiers des paquets binaires officiels (empreintes vérifiées). Le groupe « binaires tiers »
n'a pas été mesuré. « Lignes de code (corpus) » inclut le code amont embarqué dans certains forks propres
(p. ex. `kytensor` ← Triton, `kylin-kmre-emugl` ← AOSP emugl) : c'est un volume à lire, pas un volume « écrit par Kylin ».

Détail par composant (points d'entrée, privilèges, déclencheurs, indices réseau, blobs, budget) :
[`01a-points-entree-detail.md`](01a-points-entree-detail.md).

### 5.2 Points d'entrée saillants (faits de configuration à instruire en Phase 1 — aucune conclusion à ce stade)

**Télémétrie / collecte** (installés par défaut)
- **[CONSTATÉ]** `kylin-daq` : service système **root** (`com.kylin.daq`, `/usr/lib/systemd/system/kylin-daq.service`) ;
  sa configuration installée définit l'URL de remontée `https://datareporting.openkylin.top:30005/`
  (`archive:kylin-daq/conf/kylinDaqConf.json` l. 39). Il est la cible D-Bus de `libkysdk-datacollect` et
  `libkysdk-diagnostics`.
- **[CONSTATÉ]** Des appels `kdk_dia_upload_*` / `kdk_dia_*` (SDK de collecte) existent dans au moins 5 composants UKUI,
  `kylin-usb-creator`, l'outil de sauvegarde, et `kylin-process-manager` est lié à `kysdk-datacollect`.
- **[CONSTATÉ]** Le service root `com.kysdk.location` (`location.py`) contient des URL de géolocalisation par IP
  (`api.ip.sb`, `api.ipify.org`, `api.map.baidu.com`, `ifconfig.me`, `ipinfo.io`).
- **[CONSTATÉ]** `kydoctor-server` (sans source) : unité système, service D-Bus système `com.kylin.kydoctor`,
  `ld.so.conf.d`, autostart de session `user_cmd.desktop` ; dépend de `libcurl4` et `sqlcipher`.

**Identité, authentification, privilèges**
- **[CONSTATÉ — signalement précoce]** `kaiming` (installé) lie une politique polkit autorisant tout utilisateur, sans
  authentification et même hors session active (`allow_any=yes`), à exécuter via `pkexec` `/usr/bin/mount`,
  `/usr/bin/umount`, `/usr/bin/setfattr` et `/opt/kaiming-tools/bin/kaiming`. Chaîne prouvée sur le binaire distribué :
  [`donnees/signalements-precoces/kaiming-polkit/`](donnees/signalements-precoces/kaiming-polkit/). Exploitabilité
  (élévation locale de privilèges) : **[PROBABLE]**, à confirmer en Phase 1 puis en dynamique.
- **[CONSTATÉ]** `kylin-mfa` (sans source, non installé par défaut) installe un module PAM `pam_pwdless.so` et des
  profils `pam-configs` ; `kylin-id` (sans source, installé) : unité système `kylinID.service`, D-Bus système, action
  polkit `org.ukui.cloudAccount.userBound`.
- **[CONSTATÉ]** Le fichier PAM de `ukui-screensaver` accepte sans mot de passe les membres du groupe `nopasswdlogin` ;
  `pkexec.bioctl` (biométrie) est `allow_active=yes` ; `ukui-system-monitor` pose `cap_net_raw,cap_dac_read_search`.
- **[CONSTATÉ]** Un mécanisme Kylin de listes blanches d'appelants D-Bus (`*.limit` + `*.limit.verify`) est déployé par
  plusieurs paquets ; deux fichiers autorisent `LD_LIBRARY_PATH` dans leur liste d'environnement.

**Installation, comptes, session**
- **[CONSTATÉ]** `kylin-os-installer` : si `/proc/cmdline` contient `auditmode` ou `ple-mode`, le script
  `scripts/after/21setup-auditmode` crée un compte `test` à mot de passe fixe (valeur non reproduite ici), active
  l'autologin LightDM et écrit `test ALL=(ALL) NOPASSWD: ALL` ; le mode OEM écrit aussi une règle `NOPASSWD`.
- **[CONSTATÉ]** `kylin-wayland-compositor` (installé) pose la règle udev `SUBSYSTEM=="graphics", GROUP="video", MODE="0666"`
  (`data/udev-rules/90-framebuffer.rules`, installée par `data/meson.build`).
- **[CONSTATÉ]** La politique D-Bus système de `ukui-service-manager` (installé) autorise **tout utilisateur** à posséder le
  nom `org.ukui.serviceManager` (`data/org.ukui.serviceManager.conf` l. 12-13) ; le service root associé charge des greffons
  `.so` (chaîne d'activation `register-ukui-system-service` → `registerServiceByPlugin`).

**Mise à jour, installation, confiance**
- **[CONSTATÉ]** `kylin-system-updater` (installé) : `kylin-unattended-upgrades.service`, démons D-Bus système root, liste
  blanche d'exécutables appelants ; le fichier `/etc/apt/apt.conf.d/kylin-unattended-upgrades` impose **globalement**
  `DPkg::Options { "--force-confnew"; }` (les fichiers de configuration modifiés localement sont remplacés lors des
  mises à jour) et `Allowed-Origins { "Kylin:10.1"; ":default"; }` ; `kylin-software-properties` (installé, sans source) :
  tâche cron `timermanager` et timer système de mise à jour des sources APT.
- **[CONSTATÉ]** Remotes OSTree `http://ostree.openkylin.top/…` configurés avec `gpg-verify=false` ; ajout de clé dans
  `trusted.gpg.d` par `aptss` ; `revisions-manager` embarque 24 `.deb` précompilés (docker, containerd, runc, iptables).
- **[CONSTATÉ]** **`kylin-wget`** (installé, sans dépôt public) **remplace GNU wget** (`Provides/Conflicts/Replaces: wget`,
  installe `/usr/bin/wget`, réécriture Rust, version affichée `1.21.4-ok4`) ; `kylin-time` remplace de même GNU `time`.

**IA**
- **[CONSTATÉ]** Seul service système de la pile : `kytensor.service` (root, sans `User=`), qui charge des backends natifs et
  des modèles Python depuis `/usr/share/kylin-ai/model-repository` et `/opt/appdata/kylin-ai/model-repository`.
- **Hypothèse de Phase 1** (indice **[CONSTATÉ]** dans `archive:kylin-ai-runtime/src/server/abstractserver.cpp` l. 44-50) :
  l'IPC du sous-système IA passe par des serveurs D-Bus pair-à-pair sur sockets Unix sous `/tmp` sans observateur
  d'authentification, et gRPC « insecure » en local ; 10 moteurs « cloud » embarquent les points de
  terminaison DeepSeek, Alibaba DashScope, SenseNova, Baichuan, Baidu, iFlytek.

**kylin-code** (non installé par défaut)
- **[CONSTATÉ]** Fork de code-oss **1.101.2** ; `updateCheckUrl = https://kylinhorn.cn/updateCheck/kylinide` (URL
  construite `/${id}?data=…`, `window.ts` l. 696-701) ; `gIndexUrl` → manifestes Gitee désignant des scripts JS/shell à
  télécharger, dont un hôte `http://toolhub.kylinos.cn:8000` en clair ; extension intégrée `config-gitee-sync` activée à
  chaque démarrage ; script `close_telemetry.sh` (non lu en Phase 0).

**Android / KMRE** (non installé par défaut)
- **[CONSTATÉ]** 3 modules noyau hors arbre via DKMS (`kmre-binder`, `kmre-ashmem`, `kmre-virtwifi`) ; 2 services D-Bus
  root ouverts à tout utilisateur ; sockets UNIX en `0777` ; dépendance à docker ; code Gitee **non aligné** sur les
  versions distribuées.

**Réseau**
- **[CONSTATÉ]** `kylin-connectivity` (non installé) écoute en TCP sur toutes les interfaces (27188, 27181, 27192),
  embarque `anti-control-server/-client` précompilés, `dsoftbus` et un serveur FTP.
- **[CONSTATÉ]** `kylin-ipmsg` (messagerie LAN, **installé par défaut**, sans autostart) ouvre, lorsqu'il est lancé, un
  serveur TCP 39901 et une socket UDP 39900 sur toutes les interfaces IPv4, avec un protocole applicatif maison.
- **[CONSTATÉ]** Démons réseau amont installés par défaut : `samba` (serveur complet : unités `smbd`, `nmbd`,
  `samba-ad-dc`), `avahi-daemon` (mDNS), `cups-daemon`, `strongswan-charon` ; leur activation effective au démarrage
  et leurs interfaces d'écoute sont **[NON COUVERT]** en Phase 0 (préréglages systemd et configuration à vérifier en
  Phase 2/5).
- **[CONSTATÉ]** Clients HTTP en clair : `weather.ubuntukylin.com:8001`, `ip-api.com`, `myip.ipip.net`,
  `whois.pconline.com.cn` (géolocalisation IP de `kylin-weather`) ; adresse privée codée en dur `172.20.184.183:8888`
  (`kylin-printer`) ; 12 applications Kylin sont liées à `kysdk-datacollect`.

---

## 6. Forks d'amont P2 — identification de l'amont et volume du delta

Table : [`donnees/forks-p2-delta-preliminaire.csv`](donnees/forks-p2-delta-preliminaire.csv). Extrait des P2-A :

| Paquet | Version openKylin | Format | Homologue(s) même version amont | `orig.tar` vs Debian/Ubuntu | Correctifs `debian/patches` (dont nommés Kylin) | Entrées changelog openKylin | Installé par défaut |
|---|---|---|---|---|---|---|---|
| `linux` | 7.0.0-3.0ok1 | (quilt) | ubuntu_resolute_main:7.0.0-14.14 | EMPREINTE-DIFFERENTE | 8 (0) | 9 | oui |
| `systemd` | 259.5-ok1.5 | (quilt) | ubuntu_resolute_main:259.5-0ubuntu3 | IDENTIQUE | 39 (1) | 6 | oui |
| `openssl` | 3.5.5-ok8 | (quilt) | ubuntu_resolute_main:3.5.5-1ubuntu3 | IDENTIQUE | 20 (0) | 8 | oui |
| `openssl1` | 1.1.1f-ok10 | (quilt) | — (aucun) | amont-inconnu | 39 (0) | 10 | oui |
| `gnutls28` | 3.8.12-ok3 | (quilt) | ubuntu_resolute_main:3.8.12-2ubuntu1 | EMPREINTE-DIFFERENTE | 2 (0) | 19 | oui |
| `nss` | 2:3.120-ok1 | (quilt) | ubuntu_resolute_main:2:3.120-1ubuntu2 | IDENTIQUE | 4 (0) | 1 | oui |
| `libgcrypt20` | 1.12.1-ok1 | (quilt) | — (aucun) | amont-inconnu | 0 (0) | 3 | oui |
| `ca-certificates` | 20240203-ok3 | (native) | ubuntu_noble_main:20240203 | aucun-orig(natif) | 0 (0) | 3 | oui |
| `gnupg2` | 2.4.9-ok1.2 | (quilt) | debian_sid_main:2.4.9-4, debian_sid_main:2.4.9-7 | amont-inconnu | 1 (0) | 3 | oui |
| `glib-networking` | 2.80.1-ok1 | (quilt) | debian_trixie_main:2.80.1-1, ubuntu_plucky_main:2.80.1-1 | IDENTIQUE | 0 (0) | 1 | oui |
| `curl` | 8.18.0-ok2k0.1 | (quilt) | ubuntu_resolute_main:8.18.0-1ubuntu2 | EMPREINTE-DIFFERENTE | 0 (0) | 2 | oui |
| `wget` | 1.25.0-ok1 | (quilt) | debian_sid_main:1.25.0-2, debian_sid_main:1.25.0-3 | IDENTIQUE | 4 (0) | 1 | non |
| `network-manager` | 1.54.3-ok5 | (quilt) | ubuntu_resolute_main:1.54.3-2ubuntu3 | amont-inconnu | 4 (0) | 8 | oui |
| `openssh` | 1:10.2p1-ok3 | (native) | ubuntu_resolute_main:1:10.2p1-2ubuntu3 | aucun-orig(natif) | 35 (0) | 3 | oui |
| `wpa` | 2:2.11-ok1 | (quilt) | ubuntu_questing_main:2:2.11-0ubuntu4, ubuntu_resolute_main:2:2.11-0ubuntu5 | IDENTIQUE | 16 (0) | 1 | oui |
| `firefox` | 125.0.3+build1-ok7.1 | (quilt) | — (aucun) | amont-inconnu | 3 (0) | 15 | non |
| `apt` | 3.1.16-ok4 | (native) | — (aucun) | aucun-orig(natif) | 0 (0) | 20 | oui |
| `dpkg` | 1.23.7-ok3 | (quilt) | debian_sid_main:1.23.7 | amont-inconnu | 1 (1) | 5 | oui |
| `python-apt` | 3.0.0-ok4 | (quilt) | debian_trixie_main:3.0.0, ubuntu_plucky_main:3.0.0 | amont-inconnu | 2 (1) | 16 | oui |
| `aptdaemon` | 1.1.1+bzr982-ok7 | (quilt) | ubuntu_jammy_main:1.1.1+bzr982-0ubuntu39, ubuntu_noble_main:1.1.1+bzr982-0ubuntu44 | amont-inconnu | 1 (0) | 7 | oui |
| `libqapt` | 3.0.5-ok5 | (native) | debian_bookworm_main:3.0.5-2, debian_sid_main:3.0.5-5 | aucun-orig(natif) | 0 (0) | 5 | oui |
| `update-manager` | 1:20.04.10-ok9 | (quilt) | — (aucun) | amont-inconnu | 8 (0) | 8 | non |
| `python-pip` | 25.1.1+dfsg-1ok2 | (quilt) | debian_trixie_main:25.1.1+dfsg-1, ubuntu_questing_universe:25.1.1+dfsg-1ubuntu2 | IDENTIQUE | 10 (0) | 2 | oui |
| `ostree` | 2024.5-1ok22update5 | (native) | ubuntu_noble_universe:2024.5-1build2 | aucun-orig(natif) | 3 (0) | 51 | oui |
| `sudo` | 1.9.17p2-ok2 | (quilt) | debian_sid_main:1.9.17p2-9, ubuntu_questing_main:1.9.17p2-1ubuntu1 | IDENTIQUE | 8 (0) | 2 | oui |
| `pam` | 1.7.0-ok3 | (quilt) | debian_sid_main:1.7.0-8, debian_trixie_main:1.7.0-5 | IDENTIQUE | 30 (1) | 3 | oui |
| `policykit-1` | 127-ok3 | (quilt) | debian_sid_main:127-3, ubuntu_resolute_main:127-2ubuntu1 | EMPREINTE-DIFFERENTE | 2 (0) | 4 | oui |
| `shadow` | 1:4.17.4-ok1 | (quilt) | debian_trixie_main:1:4.17.4-2, ubuntu_questing_main:1:4.17.4-2ubuntu2 | IDENTIQUE | 23 (0) | 1 | oui |
| `lightdm` | 1.32.0-ok11 | (quilt) | debian_trixie_main:1.32.0-6, ubuntu_plucky_universe:1.32.0-6ubuntu3 | amont-inconnu | 14 (1) | 11 | oui |
| `glibc` | 2.43-ok2 | (quilt) | debian_sid_main:2.43-3, debian_sid_main:2.43-4 | EMPREINTE-DIFFERENTE | 86 (0) | 2 | oui |
| `grub2` | 2.12-ok1.21 | (quilt) | debian_sid_main:2.12-9+deb13u2, debian_trixie_main:2.12-9+deb13u2 | EMPREINTE-DIFFERENTE | 133 (0) | 22 | oui |
| `grub2-signed` | 1.202-ok1 | (quilt) | ubuntu_noble_main:1.202 | amont-inconnu | 0 (0) | 1 | non |
| `whoopsie` | 0.2.77-ok1 | (quilt) | ubuntu_jammy_main:0.2.77 | amont-inconnu | 0 (0) | 1 | non |
| `whoopsie-preferences` | 23-ok1 | (quilt) | ubuntu_jammy_main:23 | amont-inconnu | 0 (0) | 1 | non |
| `apport` | 2.28.1-ok3 | (quilt) | ubuntu_noble-security_main:2.28.1-0ubuntu3.8, ubuntu_noble_main:2.28.1-0ubuntu2 | IDENTIQUE | 6 (1) | 3 | oui |

Observations P2 issues de l'inventaire (Phase 0, sans lecture des hunks) :
- **[CONSTATÉ]** La plupart des forks critiques ont un homologue Ubuntu 26.04 « resolute » ou Debian sid **de même version
  amont** : l'audit différentiel pourra comparer à un paquet de référence publié, et non à une reconstruction.
- **[CONSTATÉ]** `openssh 1:10.2p1-ok3` et `apt 3.1.16-ok4` sont en format **natif** (arbre amont + delta fusionnés) ;
  `ca-certificates`, `libqapt`, `ostree`, `cryptsetup`, `kwin`, `samba`, `libarchive`, `procps` aussi. Le delta devra être
  reconstitué par diff complet contre l'amont.
- **[CONSTATÉ]** `orig.tar` de même nom mais d'empreinte différente : `linux`, `glibc`, `grub2`, `gnutls28`, `curl`,
  `policykit-1`, `dbus`, `krb5`, `chrony`. Vérification contre les publications amont signées = préalable à la Phase 2.
- **[CONSTATÉ]** Plusieurs correctifs Kylin portent un **titre sans rapport avec leur contenu** (« update changelog »,
  « changelog », « sync upstream ») alors qu'ils modifient du code — p. ex. dans `openssl` (`crypto/x509/x509_vfy.c`,
  `ssl/quic/*`), `openssl1`, `firefox` (`nsHttpHandler.cpp`), `util-linux`, `qtwebengine` (bac à sable Chromium, v8),
  `qtbase`, `coreutils`. **Conséquence méthodologique : en Phase 2, chaque hunk est lu ; aucun tri par titre.**
- **[CONSTATÉ]** Les 8 correctifs noyau Kylin ont pour auteur `root <root@localhost>` ; des composants de confiance et de
  cryptographie spécifiques Hygon sont ajoutés au noyau et à GRUB.
- **[CONSTATÉ]** `libssl1.1 1.1.1f-ok10` (branche OpenSSL 1.1.1, fin de vie en septembre 2023) est installée par défaut à
  côté de `libssl3t64 3.5.5` ; ses consommateurs seront établis en Phase 2.
- **[CONSTATÉ]** `ca-certificates` (natif) porte une modification Kylin relative aux certificats « 国密 » (cryptographie
  nationale chinoise) : l'ajout éventuel d'autorités racines au magasin système est un point prioritaire (axe 1-E).
- **[CONSTATÉ]** `grub2-signed` republie un GRUB EFI signé avec la clé UEFI de Canonical ; `shim`/`shim-signed` sont
  absents de l'archive — la chaîne Secure Boot effective est à établir (Phase 4).
- **[CONSTATÉ]** `policykit-1 124-ok6.1` installé par l'ISO n'est produit par aucune source publiée (la source est en 127).
- **[CONSTATÉ]** Les forks P2 dont le binaire installé ne correspond à aucune source publiée (p. ex. `coreutils 9.4-ok4`,
  `libsqlcipher1 4.5.6-ok1` dans l'ISO) sont signalés dans les fiches du lot P2-B.

---

## 7. Infrastructure de construction et de distribution (P3)

| Élément | Constat | Registre |
|---|---|---|
| Construction de l'ISO | `genisoimage … -publisher "http://www.kylinos.cn/" -p "kylin livebuildd"` ; `/.disk/kylin-build` : buildid 3171 ; `/.disk/base.commit` : commit OSTree `7abfcb4c…` de la référence `openkylin:openkylin/desktop/v3.0/community/general/all/release/amd64` | CONSTATÉ |
| Outillage de construction | Interne Kylin (« kylin livebuildd », base OSTree) ; non publié sous cette forme. Dépôts Gitee apparentés : `openkylin-docker-images`, `release-management`, `server-package-list`, `ukui-iso-customer` (outil PyInstaller précompilé) | CONSTATÉ / NON COUVERT |
| Post-installation | `/.kylin-post-actions` : suppression de `00secure`/`00recommends`, réécriture des sources APT (3 hôtes HTTP), installation NVIDIA par `dpkg -i /opt/nvidia/*.deb` | CONSTATÉ |
| Paquets retirés après installation | `filesystem.manifest-remove` liste `sudo`, `sudo-common`, `casper`, `user-setup`… ; l'effet réel dépend de l'installateur `kylin-os-installer` | CONSTATÉ (liste) / PROBABLE (effet) |
| Comptes à privilèges créés par l'installateur | `sudoers NOPASSWD` en mode OEM ; **mode « audit »** déclenché par la ligne de commande du noyau (`auditmode`/`ple-mode`) créant un compte `test` à mot de passe fixe, autologin et `NOPASSWD` (`archive:kylin-os-installer/scripts/after/21setup-auditmode`) ; `casper` (session live) | CONSTATÉ |
| Clés de confiance installées par défaut | clé d'archive (`trusted.gpg.d` + `Signed-By`), clé PPA (`trusted.gpg.d`, confiance globale), clé Ubuntu Kylin 2014 (lien en `postinst`), clé « Kylin Deb Signing Key » (`/usr/share/keyrings/deb-sign/`) | CONSTATÉ |
| Reproductibilité | 49 + 22 paquets sources sans code ; 1 016 couples (source, version) de binaires indexés dont la source exacte n'est plus publiée (760 sources) ; 76 paquets de l'ISO sans source exacte ; artefacts de poste de développeur dans des sources (liens absolus `/home/like/…` dans `kylin-theme`, chemins `/home/user/桌面/…` dans `yhkylin-backup-tools`) | CONSTATÉ |
| Gouvernance des dépôts | `community/permissions/gitee.json` : pour les 21 dépôts du lot P3 présents, le nombre de comptes ayant le droit `push` est égal au nombre d'administrateurs | CONSTATÉ |
| Authentification de l'ISO | MD5 non nominatives uniquement ; pas de signature ; `SHA256SUMS` interne à l'ISO (non signé) | CONSTATÉ |

---

## 8. Matrice de couverture prévisionnelle (Phase 0)

> **Mise à jour du 2026-10-02.** Cette matrice est la prévision initiale. La couverture réellement atteinte figure
> dans [`00-synthese.md`](00-synthese.md) § 5 et, par lot, dans la clé `couverture` des fiches de
> [`donnees/phase1/`](donnees/phase1/) et [`donnees/phase2/`](donnees/phase2/). Les vagues 1 à 3 ont été menées ;
> KMRE et kylin-code n'ont pas été audités (voir [`06-angles-morts.md`](06-angles-morts.md)).

Lecture : « Lignes de code propres » = lignes des fichiers de code des arbres acquis, hors répertoires de code tiers
(`third-party/`, `3rdparty/`, `vendor/`, `node_modules/`…), hors `debian/` et hors traductions Qt ; pour les forks P2, la
cible est le **delta Kylin** (voir §6) et non l'arbre. « Effort indicatif » convertit l'effort relatif 1–5 proposé par
composant en heures-auditeur (2 / 6 / 16 / 40 / 80 h) : c'est un ordre de grandeur pour arbitrer, pas un devis.
Pourcentages normalisés après critique : L3 90–100 %, L2 30–60 %, L1 5–15 %, X 0 %.

| Priorité / groupe | Composants | Lignes de code propres | L3 | L2 | L1 | X | Effort indicatif (h-auditeur) | Couverture prévue (% des lignes propres lues) |
|---|---|---|---|---|---|---|---|---|
| P1 — IA (kylin-ai-*, moteurs, agents) | 78 | 2 567 801 | 37 | 18 | 15 | 8 | 976 | 35 % |
| P1 — kylin-code (IDE) | 2 | 1 855 433 | 0 | 1 | 1 | 0 | 46 | 30 % |
| P1 — Boutique, installation d'applications | 11 | 187 988 | 3 | 1 | 1 | 6 | 194 | 44 % |
| P1 — Updateur maison | 6 | 65 849 | 2 | 2 | 1 | 1 | 146 | 67 % |
| P1 — Installateur, outils ISO/sauvegarde | 7 | 161 640 | 4 | 3 | 0 | 0 | 190 | 63 % |
| P1 — Android (KMRE) et compatibilité (KARE) | 15 | 1 196 127 | 3 | 4 | 2 | 6 | 248 | 28 % |
| P1 — Gestion des processus (« Graded Freeze ») | 5 | 43 394 | 3 | 1 | 0 | 1 | 56 | 88 % |
| P1 — Télémétrie, diagnostic, identité, sécurité | 30 | 1 080 844 | 10 | 10 | 4 | 6 | 614 | 42 % |
| P1 — UKUI cœur et services de session | 43 | 1 892 320 | 11 | 25 | 7 | 0 | 786 | 39 % |
| P1 — Système immuable OSTree | 4 | 136 | 1 | 0 | 0 | 3 | 8 | 100 % |
| P1 — Applications Kylin, thèmes, polices | 38 | 1 126 070 | 6 | 14 | 14 | 4 | 348 | 30 % |
| P1 — Binaires tiers / propriétaires, modèles | 39 | 0 | 0 | 0 | 0 | 39 | 0 | 0 % |
| P2 — P2-A crypto/TLS | 8 | delta (voir §6) | 6 | 0 | 2 | 0 | 66 | — |
| P2 — P2-A identité / privilèges | 5 | delta (voir §6) | 5 | 0 | 0 | 0 | 60 | — |
| P2 — P2-A systemd | 1 | delta (voir §6) | 1 | 0 | 0 | 0 | 6 | — |
| P2 — P2-A libc / amorçage | 3 | delta (voir §6) | 1 | 1 | 0 | 1 | 34 | — |
| P2 — P2-A noyau | 1 | delta (voir §6) | 0 | 1 | 0 | 0 | 80 | — |
| P2 — P2-A paquets / mises à jour | 8 | delta (voir §6) | 4 | 2 | 2 | 0 | 118 | — |
| P2 — P2-A rapports d'erreurs | 3 | delta (voir §6) | 3 | 0 | 0 | 0 | 10 | — |
| P2 — P2-A clients réseau | 6 | delta (voir §6) | 1 | 4 | 1 | 0 | 48 | — |
| P2 — P2-B autres forks | 40 | delta (voir §6) | 6 | 5 | 28 | 1 | 260 | — |
| P3 — P3 construction / ISO | 8 | — | 0 | 2 | 6 | 0 | 28 | — |
| P3 — P3 confiance / dépôts | 7 | — | 6 | 0 | 1 | 0 | 14 | — |
| **Total** | **368** | **10 177 602** (P1) | 113 | 94 | 85 | 76 | **4336** | **35 %** (P1) |

**Proposition d'allocation par vagues (validée en Phase 0)** — fondée sur trois critères cumulés : installé par défaut,
privilège root / exposition réseau / collecte, et absence de contrôle par l'écosystème amont.

| Vague | Périmètre | Profondeur | Justification |
|---|---|---|---|
| **1** | `kaiming` (polkit), `kylin-daq`, `libkysdk-diagnosetest` (`datacollect`), `libkysdk-base`, `libkysdk-system` (dont `location.py`), `kylin-process-manager`, `ukui-input-gather`, `ukui-service-manager`, `kylin-system-updater`, `kylin-update-frontend`, `kylin-wget`, `kylin-bot`, `kytensor`, `kylin-ai-runtime`, `kylin-os-installer` (modes OEM/audit), `kylin-wayland-compositor` (udev 0666), `ostree` (remotes `gpg-verify=false`), `ca-certificates`, `openkylin-keyring`/`-archive-anything` | L3 | Installés par défaut, root ou collecte, chemins de mise à jour et de confiance |
| **2** | Reste de G01 (IA) et G08 (télémétrie/identité) installés ; UKUI à services système (`ukui-control-center`, `-settings-daemon`, `-screensaver`, `-greeter`, `kylin-nm`, `biometric-*`, `peony`) ; P2-A (noyau, systemd, glibc, sudo, PAM, polkit, OpenSSH, OpenSSL ×2, GnuTLS, NSS, curl, APT, dpkg, aptdaemon, NetworkManager, wpa) en différentiel | L3/L2 | Surface root de la session et de l'amorçage ; forks critiques |
| **3** | Composants non installés par défaut mais distribués (KMRE, `kylin-connectivity`, `kylin-code`, `kaiming-builder`, `revisions-manager`), applications Kylin liées à `kysdk-datacollect`, P2-B, P3 | L2/L1 | Activables par l'utilisateur ou la boutique |
| **X** | 76 composants sans code auditable (dont **36 installés par défaut** : boutique, `kylin-sso-client`/`kylin-id`, `kydoctor`, `openkylin-token-center`, démons OSTree, moteurs et modèles IA, microcodes, `linux-firmware`, WPS…) et 3 132 binaires de `software.openkylin.top` | X → Phase 5 | Analyse binaire et dynamique uniquement |

---

## 9. Décisions demandées à l'auditeur principal (tranchées)

> **Suite donnée.** Ces décisions ont été tranchées au fil de l'audit ; leur mise en œuvre est décrite dans
> [`00-synthese.md`](00-synthese.md) (périmètre, vagues, vérification contradictoire) et
> [`06-angles-morts.md`](06-angles-morts.md) (composants sans source audités sur binaire, éléments non audités).

1. **Allocation** : validez-vous les vagues 1 → 3 ci-dessus et l'ordre P1 → P2 → Phases 3-4 → 5 ? Souhaitez-vous
   déplacer des composants (p. ex. remonter KMRE, non installé par défaut, en vague 1) ?
2. **Version de référence** : l'audit porte sur `archive.build.openkylin.top` (30/09/2026, celle qu'utilise un système
   installé) et sur l'ISO Desktop 3.0 du 18/09/2026. Confirmez-vous ce choix plutôt qu'`archive.openkylin.top` (plus
   ancienne) ?
3. **Composants sans source** : acceptez-vous de les traiter en X (documentation des points d'entrée installés, `file`,
   `strings`, `readelf`, sans désassemblage approfondi) et de reporter leur comportement à la Phase 5 ? Ou souhaitez-vous
   une analyse binaire statique plus poussée (désassemblage) pour une liste restreinte (p. ex. `kydoctor-server`,
   `kylin-id`, `openkylin-token-center`, `ostree-maintain-daemon`) ?
4. **Audit « par procuration »** : pour les composants livrés sans source mais dont un code Gitee d'une **autre**
   version existe (boutique, KMRE, `kylin-code`), autorisez-vous cet audit, explicitement étiqueté comme non probant pour
   le binaire livré ?
5. **Signalement précoce** : le cas `kaiming`/polkit (élévation locale probable) doit-il être instruit immédiatement en
   tête de Phase 1 ?
6. **PPA `anything3.0`** (37 252 sources, 143 paquets installés par défaut) : se limite-t-on aux 143 paquets installés
   et aux paquets d'origine Kylin, ou faut-il un échantillonnage plus large ?
7. **Architectures** : l'audit reste-t-il limité à amd64 ?
