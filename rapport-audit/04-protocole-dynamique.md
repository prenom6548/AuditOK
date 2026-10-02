# 04 — Protocole de validation dynamique (Phase 5)

🇫🇷 Français (cette page) · 🇬🇧 [English version](en/04-dynamic-protocol.md)

> **Nature de ce document.** Il décrit des **procédures d'observation et de vérification** destinées à
> **confirmer ou infirmer**, en environnement isolé, les hypothèses issues de l'analyse statique
> (Phases 1-2). Il **ne fournit aucune recette d'exploitation** : chaque test mesure un état (une invite
> d'authentification apparaît-elle ? un flux part-il en clair ? un fichier est-il inscriptible ?), il ne
> « arme » rien. Objectif : faire passer les constats du registre **[PROBABLE]** à **[CONFIRMÉ]** ou
> **[INFIRMÉ]**, avec preuve horodatée.
>
> **Statut : la Phase 5 est livrée sous forme d'un kit d'observation**, [`phase5-kit/`](phase5-kit/), mis à
> jour à chaque vague de l'audit. Ce protocole en est la référence méthodologique. Son exécution est laissée
> à la communauté, toujours dans une machine virtuelle dédiée.

## 0. Principes éthiques et de sûreté

1. **Confirmer, pas exploiter.** On observe le comportement du système ; on ne développe pas d'outil
   offensif réutilisable. Les tests d'élévation s'arrêtent à la **preuve de l'absence de contrôle**
   (p. ex. « la méthode root répond à un appelant non privilégié »), pas à la prise de contrôle effective.
2. **Isolation stricte.** Tout s'exécute **dans une VM jetable**, sur un **réseau sans accès Internet
   réel**. Aucun binaire du corpus n'est exécuté sur l'hôte. Aucune donnée réelle (identifiants matériels,
   télémétrie) ne quitte le banc de test.
3. **Usage responsable.** Ce protocole observe et vérifie ; il ne fournit aucun mode opératoire
   d'exploitation. Toute confirmation est à manier à des fins défensives (cf. `README.md` § Usage responsable).
4. **Reproductibilité.** Image de base figée (empreinte de l'ISO notée), snapshots, journalisation
   horodatée, un test = un snapshot propre.
5. **Outils libres de préférence** (cohérent avec la ligne de l'audit) : virtualisation **QEMU/KVM +
   libvirt** ; capture **tcpdump/Wireshark** ; mandataire d'inspection TLS **mitmproxy** ; résolution
   menteuse **dnsmasq/unbound** ; audit **auditd**, `busctl`/`dbus-monitor`, `strace`. Éviter les
   hyperviseurs propriétaires.

## 1. Banc de test

### 1.1 Topologie
```
[ Hôte d'analyse ]                         (ne reçoit que journaux + captures)
   └─ QEMU/KVM
       ├─ VM « cible » : openKylin 3.0 huanghe (amd64), installée depuis l'ISO auditée
       └─ VM « bac réseau » (ou namespace) : passerelle du seul réseau de la cible
            ├─ dnsmasq : résout TOUS les domaines vers le bac (sinkhole)
            ├─ mitmproxy : termine le TLS et journalise le contenu (teste VerifyNone)
            ├─ serveur HTTP/HTTPS menteur : répond aux API de télémétrie / OSTree / dépôts
            └─ tcpdump : capture pcap de tout le segment
```
La cible n'a **aucune** route vers Internet : son unique voisin est le bac réseau, qui **capture et
simule**. On observe ce que la cible *tente* d'envoyer et *accepte* de recevoir, sans rien exfiltrer.

### 1.2 Images et comptes
- **Image de base** : installation standard depuis l'ISO (empreinte ISO notée) ; snapshot `base`.
- **Variante immuable** : installation OSTree, si disponible ; snapshot `base-ostree`.
- **Comptes** : `audituser` (utilisateur standard, non sudoer) pour tous les tests d'élévation locale ;
  l'accès root n'est utilisé que pour *relever* l'état (permissions, services), jamais comme point de
  départ d'un test d'élévation.

### 1.3 Instrumentation activée avant chaque test
- `auditd` avec règles sur `execve`, `ptrace`, montages, écritures dans `/usr/share/polkit-1`,
  `/etc/polkit-1`, `/opt`, `/var/lib/dpkg/plugins`.
- `dbus-monitor --system` et `busctl` pour l'introspection et la trace IPC.
- `tcpdump` côté bac réseau ; `mitmproxy` en mandataire transparent.
- `journalctl -f` et horodatage NTP figé (pas de synchro réseau).

## 2. Matrice des hypothèses à trancher

Chaque ligne renvoie à `02-findings.csv` (colonnes `lot`, `fichier`). Résultat attendu formulé en
**observable**, pas en gain d'accès.

| # | Constat (lot) | Sévérité statique | Hypothèse à trancher | Observable décisif |
|---|---|---|---|---|
| D1 | kaiming polkit (L0) | critique | `pkexec` des actions `org.kylin.kaiming.*` s'exécute **sans invite** pour un utilisateur standard | `pkcheck`/`pkaction` + absence d'invite d'authentification |
| D2 | rules.d kaiming (L0) | élevée | `/opt/kaiming/share/polkit-1/rules.d` et `/var/opt/kaiming/share/*` **inscriptibles** par un utilisateur | `stat`/`getfacl` sur système installé |
| D3 | kylin-process-manager `LockGSettingsKey` (L4) | critique | la méthode D-Bus root est **appelable sans contrôle** par `audituser` | introspection `busctl` + trace `dbus-monitor` d'un appel **inoffensif** |
| D4 | ukui-service-manager `launchServices` (L4) | critique | le slot root accepte un **chemin de greffon arbitraire** sans vérif d'UID | introspection ; test avec un `.so` **inerte** de diagnostic (journalise son chargement, ne fait rien d'autre) |
| D5 | kytensor service root (L8) | critique | l'API locale (127.0.0.1:8000/8001) répond **sans authentification** | requête **inoffensive** `/v2/health` + vérif `User=` du service |
| D6 | kylin-daq TLS (L1) | élevée | téléversement HTTPS avec **certificat non vérifié** (`VerifyNone`) | `mitmproxy` intercepte le flux **sans** certificat de confiance installé |
| D7 | kylin-daq reconfig (L1) | élevée | la **réponse serveur** reconfigure le démon (persistée) | réponse menteuse du bac → diff de `/var/cache/kylin-daq/kylinDaqConf.json` |
| D8 | updater DataAcquisition (L5) | élevée | canal **HTTP en clair** vers `archive1.kylinos.cn:32294` | pcap en clair après une mise à jour simulée |
| D9 | libkysdk-location (L3) | élevée | requête `api.map.baidu.com` **en HTTP clair** avec clé en dur | pcap + vérif que le service est encore sollicité en 3.0 |
| D10 | OSTree gpg-verify=false (L10) | élevée | un commit **non signé** servi par le bac est **accepté** | trace du `pull` sur remote menteur (sans déployer de contenu malveillant) |
| D11 | installateur auditmode (L9) | élevée | `auditmode`/`ple-mode` (et `noauditmode`, `auditmode=0`) créent le compte `test` NOPASSWD **persistant** | présence de `/etc/sudoers.d/test`, autologin ; recherche de `kylin-audit-mode` |
| D12 | compositeur `/dev/fb*` 0666 + protocoles privilégiés (L9) | élevée | capture d'écran / entrées virtuelles **sans contrôle d'accès** | règle udev effective + test d'un client Wayland non autorisé (capture d'un **écran de test**) |
| D13 | kylin-ai-runtime sockets (L8) | élevée/moyenne | mode **effectif** des sockets dans un répertoire `0777` ; envoi **cloud par défaut** | `stat` des sockets + capture du flux cloud freetrial au premier usage |
| D14 | IPC kysdk `context=default` + listes blanches (L2/L3/L4) | moyenne/élevée | les **listes blanches `.limit`** sont-elles réellement appliquées ? | appel d'une méthode d'info depuis `audituser` ; présence/effet du dbus patché |
| D15 | durcissement noyau (Phase 2) | selon diff | options de durcissement **réellement** absentes sur le noyau installé | `/boot/config-*`, `/sys/kernel/security`, `sysctl` |
| D16 | ostree-extra-utils, `/opt/system` (P2) | élevée | `/opt/system` et ses sous-dossiers (bibliothèques, Python, `PATH`, unités systemd) **inscriptibles** par un non-root, directement ou via kaiming | `stat` des dossiers ; recherche d'éléments non-root ou inscriptibles (script 10) |
| D17 | durcissement des binaires (P2) | élevée | binaires setuid et services root **installés** sans PIE, `BIND_NOW`, canari, FORTIFY ni CET | `file` et `readelf` sur les binaires installés (script 60) |
| D18 | grub2 2.12~rc1 (P2) | élevée | GRUB installé = 2.12~rc1, **sans shim** ; Secure Boot ; entrée de restauration d'usine sans mot de passe GRUB | `dpkg-query`, `modinfo.sh`, variables EFI, lecture de `grub.cfg` (script 60) |
| D19 | aptdaemon `SetEnviron` (P2) | à confirmer | la méthode D-Bus root sans contrôle est **absente** de la version installée | lecture de `aptdaemon/core.py` installé, sans réveiller le service (script 10) |
| D20 | network-manager → dnsmasq (P2) | moyenne | le **démon** `dnsmasq` est actif et écoute au démarrage | `systemctl is-enabled/is-active`, `ss -lntu` (script 60) |
| D21 | dpkg et correctifs (P2) | moyenne | crochet `pre-invoke` root actif ; `dpkg`/`apt` détournés ; versions installées en retard de correctifs de sécurité | `/etc/dpkg/dpkg.cfg.d`, `dpkg-divert --list`, `dpkg-query` (script 50) |

## 3. Procédures détaillées par grappe

> Convention de résultat pour chaque test : **[CONFIRMÉ]** (observable présent), **[INFIRMÉ]** (contrôle
> compensatoire constaté — p. ex. `kysec` bloque effectivement), **[INDÉTERMINÉ]** (non reproductible sur
> le banc). On note toujours la **preuve** (capture, log, horodatage).

### 3.1 Élévation locale par polkit — `kaiming` (D1, D2)
1. Depuis `audituser` : `pkaction --action-id org.kylin.kaiming.mount --verbose` → relever
   `implicit any/inactive/active`.
2. Observer la présence/absence d'**invite** d'authentification via `pkcheck --action-id … --process $$`
   (sans lancer l'action privilégiée réelle).
3. Relever les permissions de `/opt/kaiming/share/polkit-1/rules.d`, `/var/opt/kaiming/share/*`,
   `/usr/share/polkit-1/actions/org.kylin.kaiming.policy` (`stat`, `getfacl`).
4. Vérifier si `kysec` impose une restriction compensatoire (statut `kysec-status`, attributs sur
   `/usr/bin/mount`).
5. **Observable décisif** : implicit `yes` + aucune invite = D1 **[CONFIRMÉ]** ; répertoire `rules.d`
   inscriptible par l'utilisateur = D2 **[CONFIRMÉ]**.

### 3.2 IPC D-Bus root sans contrôle (D3, D4, D14)
1. `busctl --system list` puis `busctl introspect` des services root concernés (ProcessManagerDaemon,
   `org.ukui.Service*`, `com.kylin.systemupgrade`, dispatchers kysdk).
2. Relever la politique installée (`/usr/share/dbus-1/system.d/*.conf`) : `context` effectif.
3. Depuis `audituser`, appeler une méthode **en lecture seule / inoffensive** (p. ex. une méthode d'info)
   et constater si l'appel **aboutit** sans contrôle. Ne **pas** appeler les primitives dangereuses
   (injection, chargement `.so`, montage) ; pour D4, se limiter à charger un **greffon de diagnostic
   inerte** qui journalise son propre chargement.
4. Pour D14 : vérifier si un `dbus-daemon`/module kysec patché applique les `.limit` (sinon la politique
   `context=default` gouverne réellement). Relever `/etc/dbus-1/conf/*.limit` et leur prise en compte.

### 3.3 Service IA root `kytensor` (D5)
1. `systemctl show kytensor.service -p User` → confirmer l'exécution en root.
2. `ss -ltnp` → confirmer l'écoute locale 8000/8001.
3. Requête **inoffensive** `GET /v2/health` depuis `audituser` → l'absence d'authentification est
   l'observable. **Ne pas** soumettre de modèle à backend exécutable.

### 3.4 Télémétrie et TLS (D6, D7, D8, D9)
1. Au **premier démarrage** : vérifier si une étape de **consentement** (OOBE) est présentée.
2. Capturer les flux `kylin-daq` : confirmer la destination (`datareporting.openkylin.top:30005`), et —
   via `mitmproxy` sans certificat de confiance installé dans la cible — confirmer que le flux est
   **intercepté quand même** (⇒ `VerifyNone`, D6 **[CONFIRMÉ]**).
3. Relever les en-têtes (`X-Uid`, `X-Service-Serial-No`, `X-Os-Serial-Number`…) et le **contenu** réel
   des messages ; comparer à la politique de confidentialité (`05-donnees.md`).
4. D7 : faire répondre le bac avec un objet `bo` modifié → vérifier la **fusion persistante** dans
   `kylinDaqConf.json`.
5. D8 : déclencher une mise à jour simulée → capturer le **HTTP en clair** vers `archive1.kylinos.cn`.
6. D9 : observer si `libkysdk-location` émet encore vers `api.map.baidu.com` (en-tête du code : « non
   utilisée depuis 3.0.1.0 » — à trancher).
7. Trancher l'effet de `QSsl::TlsV1SslV3` (négociation réellement bornée à TLS 1.0 ?) via la poignée TLS
   observée.

### 3.5 Chaîne de confiance OSTree / CA (D10)
1. Relever `/etc/ostree/remotes.d/*.conf` sur la cible immuable : `gpg-verify`, URL.
2. Pointer le remote vers le bac et servir un **dépôt OSTree de test non signé** (contenu bénin) :
   observer si `ostree pull` / l'updater **accepte** sans erreur de signature (D10 **[CONFIRMÉ]** si
   accepté). **Ne pas** déployer de charge malveillante ; l'observable est l'acceptation du commit non
   authentifié.
3. Relever le contenu et l'effet de `/opt/system/resource/ca-certificates` (ajout de CA hors debconf).

### 3.6 Installateur / compositeur (D11, D12)
1. D11 : installer en passant successivement `auditmode`, `ple-mode`, `noauditmode`, `auditmode=0` sur la
   ligne de commande noyau ; après install, vérifier `/etc/sudoers.d/test`, l'autologin, et la **présence**
   du paquet `kylin-audit-mode` (absent du corpus) ; constater si le compte persiste.
2. Lire les entrées **GRUB/isolinux** de l'ISO (montée en lecture seule) pour voir si un menu active ces
   modes (angle mort L9).
3. D12 : relever la règle udev effective sur `/dev/fb*` (`stat`) ; avec un **client Wayland de test**,
   tenter une capture d'un **écran de test** depuis une autre session/utilisateur → l'observable est
   l'absence de refus. Relever aussi l'état `security-context` et les protocoles `screencopy`,
   `virtual-keyboard`, `data-control`.

### 3.7 Agents IA (D13)
1. `stat` des sockets sous `/tmp/.kylin-ai-runtime-unix/<uid>/` → mode **effectif** (le répertoire est
   `0777` ; le socket l'est-il ?).
2. Au premier usage de l'assistant : capturer le flux → confirmer l'envoi **par défaut** au service cloud
   *freetrial*, le point de terminaison, le TLS, et la présence/absence de **consentement**.
3. Confirmer que les clés RSA / mots de passe « codés en dur » déchiffrent bien les secrets stockés
   (observation, valeurs **masquées** dans les livrables).

### 3.8 Durcissement noyau (D15, selon Phase 2)
1. Relever `/boot/config-$(uname -r)` : état des options signalées par le lot `P2-linux`
   (LOCKDOWN, MODULE_SIG, BPF_UNPRIV, STACKPROTECTOR, LEGACY_TIOCSTI, STRICT_DEVMEM…).
2. `sysctl -a` et `/sys/kernel/security/` : LSM actifs, `lockdown`.
3. Comparer à un noyau Debian/Ubuntu de référence (valeurs attendues).

## 4. Vérification « code ↔ binaire distribué »

La compilation est **interdite** ; on procède donc par **concordance**, non par reconstruction :
- empreintes des binaires installés vs paquets `.deb` vs arbres sources (déjà partiellement fait en
  Phase 0) ;
- `strings`/`readelf`/symboles pour retrouver les chemins de debug, versions, et chaînes attendues ;
- pour les composants **sans source** (greffons IA, etc.), l'opacité est **assumée** et documentée en
  angle mort — la Phase 5 ne lève pas cette limite.

## 5. Livrables de la Phase 5

- `donnees/phase5/` : captures `pcap`, exports `mitmproxy`, relevés `stat`/`busctl`/`journalctl`,
  horodatés, une fiche résultat JSON par test (`{id, hypothese, resultat, preuve, horodatage}`).
- Mise à jour de `02-findings.csv` : chaque constat testé reçoit un **registre dynamique**
  (CONFIRMÉ/INFIRMÉ/INDÉTERMINÉ) ; les `[PROBABLE]` tranchés basculent en conséquence.
- Mise à jour de `00-synthese.md` : verdict consolidé après confrontation statique ↔ dynamique.

## 6. Limites de la Phase 5 (explicites)

- **Matériel virtuel** : DMI/numéros de série/MAC de la VM diffèrent d'une machine réelle ; la télémétrie
  observée peut être **partielle** ou dégradée (certains relevés matériels échouent en VM).
- **Serveurs simulés** : le bac réseau imite les API ; il ne reproduit pas le comportement **réel** des
  serveurs openKylin (réponses, conditions d'activation côté serveur).
- **Portée** : amd64 et la configuration par défaut ; les variantes (arm64/loong64, Server, Embedded,
  AgentOS) et les parcours d'installation exotiques ne sont pas couverts.
- **Composants opaques** : les greffons/binaires sans source restent non vérifiables au-delà de leur
  comportement réseau observable.
- **Conditions d'activation** : un comportement conditionné (date, géographie, serveur distant, matériel
  spécifique) peut ne pas se déclencher sur le banc — l'absence d'observation n'est pas une preuve
  d'absence (principe directeur de l'audit).
