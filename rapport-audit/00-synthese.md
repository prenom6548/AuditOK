# 00 — Synthèse de l'audit openKylin 3.0 « huanghe »

🇫🇷 Français (cette page) · 🇬🇧 [English version](en/00-synthesis.md)

> **État : Phases 0, 1 (vagues 1-3 et approfondissements) et 2 (18 lots de forks et 2 lots transverses) terminées ; tous les
> constats critiques et élevés ont fait l'objet d'une vérification contradictoire ; Phase 5 livrée sous forme
> d'un kit d'observation, tenu à jour au fil de l'audit.** Relevés du 2026-10-01 (UTC) ; vague 3 du 2026-10-02.
> Registres : **[CONSTATÉ]** (preuve fichier : ligne), **[PROBABLE]** (indice fort non confirmé),
> **[NON COUVERT]** (hors de portée). Principe : **on ne conclut jamais « sûr » par absence de preuve.**

## 1. Verdict global

| Question | Réponse à l'état actuel |
|---|---|
| Porte dérobée / code distant non sollicité ? | **Aucune porte dérobée [CONSTATÉ]** dans le code et les binaires lus. En revanche, la boutique d'applications installée par défaut ouvre un **canal de gestion à distance contrôlé par l'éditeur**, actif sans inscription du poste (installations bornées aux paquets signés, désinstallations non bornées) (§ 3.3). Non exclue sur le volume non lu et les composants opaques (§ 6). |
| Faiblesses de sécurité réelles ? | **Oui [CONSTATÉ]** : 704 constats ; après vérification, **9 critiques** et **56 élevés** (§ 3). |
| Collecte au-delà du déclaré ? | **Écarts [CONSTATÉ]** de qualification, de transport et de consentement (§ 4, `05-donnees.md`). |
| Forks critiques amont (OpenSSH, systemd, OpenSSL 3, sudo, APT, crypto, glibc, GRUB, dpkg…) ? | **Sans porte dérobée ni code amont falsifié [CONSTATÉ]** : le code amont vérifié est authentique (glibc identique à l'archive GNU signée, GRUB identique à une pré-version GNU signée). Mais **retards de correctifs de sécurité** (glibc, GRUB, NetworkManager, apport) et **binaires livrés sans les protections standard de compilation** (§ 3.3, § 3.4). |

**Lecture d'ensemble.** Le motif dominant, constaté et confirmé par la vérification contradictoire, n'est pas
une porte dérobée distante mais une **ingénierie des privilèges défaillante dans les composants propres à
Kylin** : de nombreux services système exposés à tout utilisateur local sans contrôle effectif de l'appelant,
des politiques d'autorisation trop larges, et des entrées non validées dans du code privilégié. Le mécanisme
de liste blanche prévu pour restreindre ces services (`.limit`) **n'est pas appliqué par défaut**. S'y ajoutent
une **télémétrie à identifiants matériels** transmise par des canaux non authentifiés, une **chaîne de mise à
jour OSTree sans vérification d'authenticité**, et une **traçabilité médiocre** de l'empaquetage (42 correctifs
au titre sans rapport avec leur contenu).

La fin de la Phase 2 ajoute deux faiblesses **transverses**, sans indice d'intention malveillante : des
**binaires distribués massivement privés des protections standard de compilation** (PIE, FORTIFY, `BIND_NOW`),
y compris des binaires setuid et des services root, et un **retard de correctifs de sécurité** sur des composants
fondamentaux (glibc, chargeur GRUB resté en pré-version, NetworkManager).

La vague 3 (boutique d'applications, identité Kylin ID, réinitialisation des mots de passe locaux) confirme
le motif dominant : deux nouvelles **élévations locales de privilèges critiques** dans le service de
réinitialisation des mots de passe (`kpr-backend`, source publiée et conforme au binaire livré). Dans deux
composants **livrés sans source** (boutique, Kylin ID), elle établit des connexions d'identité sans vérification
de certificat et un **canal de gestion à distance** de la boutique, ciblé par identifiants matériels et actif
par défaut. Les lots UKUI complémentaires (24 composants installés par défaut) confirment le motif : des services
système root du réseau, du Bluetooth et du partage de fichiers sans contrôle effectif de l'appelant. À l'inverse,
le module PAM biométrique ne laisse pas passer en cas d'échec, et un service d'authentification contrôle
réellement ses appelants.

## 2. Avancement

| Phase | Statut | Livrables |
|---|---|---|
| 0 — Inventaire, points d'entrée, matrice | Terminée | [`01-inventaire.md`](01-inventaire.md), [`01a-points-entree-detail.md`](01a-points-entree-detail.md), [`06-angles-morts.md`](06-angles-morts.md) |
| 1 — Statique P1 : vague 1 (11 lots) + 4 approfondissements | Terminée | [`02-findings.csv`](02-findings.csv), [`donnees/phase1/`](donnees/phase1/) |
| 1 — Statique P1 : vague 2 (4 lots : session et shell UKUI, identité/télémétrie, IA cloud) | Terminée | idem |
| 1 — Statique P1 : vague 3 (7 lots : boutique, Kylin ID, réinitialisation des mots de passe — contenu binaire pour les deux premiers, sans source — puis 24 composants UKUI en 4 lots) | Terminée | idem, [`donnees/phase1/vague3/`](donnees/phase1/vague3/) |
| 2 — Différentiel des forks P2 (18 lots de forks + 2 lots transverses : binaires sans source, durcissement des binaires) | Terminée ; sources de référence Ubuntu/Debian/GNU vérifiées | [`03-diffs-suspects.md`](03-diffs-suspects.md), [`donnees/phase2/`](donnees/phase2/) |
| 3 — Flux de données | Vagues 1-2 consolidées | [`05-donnees.md`](05-donnees.md) |
| Vérification contradictoire (83 constats ≥ élevé) | Terminée | [`07-verification.md`](07-verification.md), [`donnees/verification/`](donnees/verification/) |
| 4 — Chaîne de construction | Éléments relevés | — |
| 5 — Kit d'observation (VM) | Livrée sous forme de kit, mis à jour à chaque vague ; exécution laissée à la communauté | [`phase5-kit/`](phase5-kit/), méthode : [`04-protocole-dynamique.md`](04-protocole-dynamique.md) |

## 3. Résultats

**704 constats** : 226 vague 1 (+ approfondissements), 106 vague 2, 118 vague 3, 254 Phase 2. Sévérités **après
vérification** : **9 critiques · 56 élevés · 160 moyens · 189 faibles · 290 informationnels**.

### 3.1 Vérification contradictoire (83 constats critiques/élevés)

Chaque constat a été relu par un auditeur sceptique chargé de le réfuter : **57 CONFIRMÉS, 23 NUANCÉS,
3 INDÉTERMINÉS, 0 RÉFUTÉ**. Les nuances ont surtout **abaissé** des sévérités (conditions plus étroites :
authentification encore exigée, architecture non installée, chemin par défaut différent) ; dans deux cas, la
relecture a au contraire **réfuté des contre-lectures bénignes** (service bien activé, code bien atteignable).
La seconde vague de vérification (groupes G8 et G9, 8 constats de fin de Phase 2) a **confirmé** la régression
du durcissement des binaires et ramené à « moyenne » les constats liés à GRUB : sur la 3.0 amd64, GRUB démarre
sans shim et rien n'est signé, il n'y a donc pas de Secure Boot à contourner (les failles restent présentes).
La troisième (groupes G10 à G15, 15 constats de la vague 3) en a confirmé 12, nuancé 2 (dont le service
d'identité ouvert à tous, ramené à « moyenne » faute de prise de contrôle root ou de vol de jeton) et laissé 1
indéterminé (capacités du moniteur système, à observer sur un système réel).
Détail : [`07-verification.md`](07-verification.md).

### 3.2 Constats critiques (9, tous CONFIRMÉS) — élévation locale de privilèges

Décrits ici par catégorie, à des fins défensives ; références exactes dans `02-findings.csv`.

| Composant (installé par défaut) | Catégorie | Lot |
|---|---|---|
| `kaiming` — politique polkit (2 actions) | autorisation accordée sans authentification à tout utilisateur | V1-L0 |
| `kaiming` — déploiement d'applications | contenu non authentifié traité avec privilèges | V1-L14 |
| `kylin-process-manager` | service privilégié, entrée non validée, sans contrôle d'appelant | V1-L4 |
| `ukui-service-manager` | service privilégié sans contrôle d'appelant | V1-L4 |
| `kytensor` (serveur d'inférence IA) | service privilégié à API locale non authentifiée | V1-L8 |
| `biometric-authentication` | défaut de gestion mémoire dans un service privilégié ouvert à tous | V2-L3 |
| `libkba` (`kpr-backend`) — réinitialisation des mots de passe locaux | service privilégié sans contrôle d'appelant : modification du mot de passe d'un autre compte, root compris | V3-L3 |
| `libkba` (`kpr-backend`) | entrée non validée transmise à une commande exécutée avec privilèges | V3-L3 |

### 3.3 Thèmes élevés (56)

- **Services système ouverts à tout utilisateur** (`context="default"`, sans polkit ni contrôle d'UID effectif) :
  kylin-process-manager, kysdk (conf2, location), kylin-system-updater, ukui-settings-daemon,
  ukui-control-center, kylin-os-manager, proxy kaiming. Liste blanche `.limit` non appliquée.
- **Télémétrie et transport** : trois canaux transmettent des identifiants matériels (série, MAC, `machine-id`,
  clé d'activation) — `kylin-daq` et `service-support` en HTTPS **sans vérification de certificat**, l'updater
  en **HTTP clair** ; moteurs de parole IA (Baidu, iFlytek) sans vérification de certificat.
- **Chaîne de confiance** : remotes OSTree en HTTP avec `gpg-verify=false` (intégrité sans authenticité) ;
  dépôts APT sans `Valid-Until` ni `Signed-By` (moyens).
- **Interface graphique** : compositeur Wayland sans contrôle d'accès aux protocoles privilégiés (capture,
  entrées, saisie clavier exclusive).
- **Agents IA** : configuration de `kylin-bot` livrée sans garde-fous ; services IA à sockets peu protégées.
- **polkit (fork)** : correctif intitulé « update changelog » modifiant le moteur d'autorisation [CONSTATÉ].
- **Durcissement des binaires distribués** [CONFIRMÉ] : sur 18 paquets fondamentaux comparés aux mêmes paquets
  Ubuntu, **16 exécutables PIE sur 234** contre 234/234, FORTIFY 14/262 contre 226/262, `BIND_NOW` 127/262
  contre 251/262 ; binaires setuid (`passwd`, `chsh`, `su`, `mount`…) et services root (`dbus-daemon`, `polkitd`,
  `pkexec`) concernés ; aucun des 52 exécutables propres à openKylin relevés n'est PIE ; signature de compilation
  non optimisée dans 20 paquets sur 32. Relevé par `readelf` sur les paquets officiels, sans exécution.
- **Boutique d'applications** (installée par défaut, sans source publiée) : canal de gestion à distance **actif
  par défaut**, sans inscription du poste. Des « stratégies » téléchargées depuis le serveur de l'éditeur, ciblées
  par identifiants matériels, peuvent déclencher des installations silencieuses (bornées aux paquets signés des
  dépôts) et des désinstallations silencieuses non bornées. Identifiants matériels (MAC, numéros de série, code
  d'activation) joints à chaque requête, absents de la rubrique « boutique » de la politique de confidentialité.
- **Identité Kylin ID** (installée par défaut, sans source) : connexions au service d'identité **sans vérification
  de certificat** (identifiants, jetons, codes SMS, série matérielle).
- **Réinitialisation des mots de passe** (`kpr-backend`) : politique D-Bus ouverte, liste blanche `.limit`
  inopérante avec le `dbus` livré, clé de chiffrement fixe rendant le jeton forgeable.
- **Services système UKUI** (installés par défaut) : gestion du réseau (`kylin-nm`), Bluetooth et partage de
  fichiers Samba (`peony-share`) exposés à tout utilisateur local sans contrôle effectif de l'appelant ; pour le
  Bluetooth, commande privilégiée construite à partir du nom d'un fichier reçu (conditionnée à l'activation du
  Bluetooth et à l'acceptation du transfert) ; moniteur système doté de capacités privilégiées à l'installation
  (indéterminé).

### 3.4 Forks amont (Phase 2)

| Résultat | Forks |
|---|---|
| **Sans affaiblissement constaté** du delta openKylin | OpenSSH (base Ubuntu, aucun correctif Kylin), systemd, OpenSSL 3.5, sudo, sudo-common, APT, gnutls/nss/libgcrypt/gnupg2/glib-networking/curl, pam/shadow/lightdm (constats faibles/moyens seulement) ; glibc (code amont identique à l'archive GNU signée, 86 correctifs identiques au fond à Ubuntu), dpkg (source Ubuntu reprise telle quelle), wpa, network-manager (aucun code propre) |
| **Écarts notables** | polkit (moteur modifié sous un titre trompeur) ; noyau : amd64 aligné sur Ubuntu sauf mitigation Downfall désactivée, **loong64** à durcissement largement retiré (hors architecture installée) ; correctifs matériels Hygon ; **GRUB** : l'archive « 2.12 » est la pré-version **2.12~rc1** (signée GNU) renommée, privée de 108 commits amont dont les correctifs NTFS CVE-2023-4692/4693, vérificateur Hygon TPCM ajouté sous un titre « changelog », démarrage **sans shim** sur l'ISO ; **dpkg** : la politique de durcissement de `dpkg-buildflags` n'est jamais chargée (module mal nommé) |
| **Retard de correctifs de sécurité** | glibc (au moins 11 CVE corrigées depuis par Ubuntu ou Debian), GRUB (22 CVE de 2025), NetworkManager (CVE-2026-19685, corrigée par Ubuntu 10 jours avant les relevés), apport (CVE-2025-5054 et -5467 ; paquet non installé par défaut) |
| **Non auditable ici** | `coreutils 9.4-ok4` (commandes de base installées) **sans source dans aucun index signé** ; `libssl1.1` (OpenSSL 1.1.1 en fin de vie) installée ; archives amont de curl et gnutls différentes d'Ubuntu ; `ostree-extra-utils` (**installé par défaut** : crochet dpkg root, `/opt/system` ajouté aux chemins de bibliothèques, de Python et de systemd) et `dpkg-wrapper` (détourne `dpkg` et `apt`) : binaires signés dont la source publiée est une **coquille vide** |

**Traçabilité** : **42 correctifs** sur 11 lots portent un titre sans rapport avec leur contenu ; leur contenu,
lu intégralement, est le plus souvent bénin (désactivation de tests, correctifs CVE annoncés), à l'exception
notable de polkit. S'y ajoutent des correctifs Ubuntu **intégrés sans déclaration** à l'archive amont
(NetworkManager, aptdaemon) et un **vidage mémoire de 84 Mo** d'un poste de développeur laissé dans l'archive
source d'aptdaemon (variables d'environnement ; aucun secret détecté).

## 4. Croisement avec la politique de confidentialité

La **nature** de la collecte système correspond aux catégories déclarées. Écarts [CONSTATÉ] :
(1) collecte dite « non identifiante » (l.176) alors que des identifiants matériels stables sont transmis ;
(2) transmission promise « sûre et contrôlée » (l.154) alors que plusieurs canaux sont non authentifiés ou en
clair ; (3) collectes non énumérées (vidages mémoire, journaux de politique de sécurité, coordonnées,
identifiants de périphériques, identifiants matériels joints par la boutique à chaque requête) ; (4) consentement non établi pour les canaux système, **présent mais non
éclairé** pour l'assistant IA. Détail : [`05-donnees.md`](05-donnees.md) § 3.

## 5. Couverture réelle

Très inégale et assumée : arbres petits et sensibles lus **intégralement** ; gros arbres lus **sur leur
surface privilégiée** (services root, IPC, réseau, télémétrie) avec un faible pourcentage global.

| Couverture | Arbres |
|---|---|
| **100 %** | kylin-daq, libkysdk-diagnosetest, kylin-wget, ukui-input-gather, ukui-service-manager (98 %), ca-certificates, trousseaux ; correctifs P2 : OpenSSH, systemd, OpenSSL 3, sudo, coreutils, polkit, pam/shadow/lightdm, crypto, glibc, grub2 (133/133 statués, écarts de l'archive amont 191/191), dpkg, network-manager, wpa, aptdaemon, apport ; contenu des binaires `ostree-extra-utils` et `dpkg-wrapper` ; fichiers texte et scripts de la boutique et de kylin-id (exécutables caractérisés) |
| **25-90 %** (lignes) | kba (`kpr-backend`) 83 %, kylin-system-updater 88 %, ukui-clipboard 44 %, ostree 41 % (delta amont 100 %), biometric-authentication 38 %, kylin-ai-runtime 31 %, libkysdk-base 27 %, kaiming 24 % |
| **< 15 %** (surface privilégiée ciblée) | composants UKUI de la vague 3 (24 arbres : services système lus intégralement, interfaces de session par recherche ciblée, 2-8 % des lignes), compositeur Wayland 11 % (tous les protocoles exposés lus), kylin-bot 7 %, kytensor 7 %, libkysdk-system 7 %, kylin-os-manager 5 %, UKUI session/shell 0-6 % |

Un défaut situé hors des zones lues ne serait pas détecté. Détail par lot : fiches `donnees/phase1/`,
`donnees/phase2/` (clé `couverture`).

## 6. Limites de l'audit (liste explicite)

- **Volume** : ~90 composants analysés sur 368 retenus (3 238 sources dans l'archive par défaut) ; PPA
  `anything3.0` (37 252 sources) et `software.openkylin.top` (3 132 binaires) couverts par inventaire seul.
- **Composants sans source audités sur binaire** (vague 3 : boutique, kylin-id ; Phase 2 : ostree-extra-utils,
  dpkg-wrapper) : fichiers texte et scripts lus intégralement, exécutables caractérisés (`readelf`, chaînes,
  désassemblage ciblé) mais **non décompilés intégralement**.
- **Binaires sans source** : 76 composants classés X (36 installés par défaut) ; s'y ajoutent `coreutils
  9.4-ok4` et les greffons IA (moteur « freetrial », clé par défaut) dont la destination réelle est inconnue.
- **Archives amont** non présentes pour une partie des forks P2 : delta hors `debian/` non vérifiable pour curl,
  gnutls, noyau 7.0, Triton, llama.cpp ; **vérifié** pour glibc, grub2, dpkg, network-manager, wpa, aptdaemon et
  apport grâce à des références Ubuntu, Debian et GNU signées (`outils/references_p2.py`).
- **Durcissement des binaires** : mesuré sur un échantillon de 32 paquets ; la configuration du compilateur
  gcc-15 d'openKylin (source hors corpus) n'est pas établie.
- **Statique uniquement** : aucune compilation, exécution ni accès réseau depuis l'analyse. Les constats sont
  confirmés **dans le code**, non **sur un système en fonctionnement** ; 2 verdicts INDÉTERMINÉS et plusieurs
  conditions d'atteignabilité sont renvoyés à la Phase 5 (permissions réelles, umask, navigateur).
- **Liste blanche `.limit`** : le composant qui l'appliquerait (dbus/kysdk patché) est absent du corpus ;
  l'audit le considère comme non appliqué par défaut, ce qui est vérifié sur la configuration livrée.
- **Égalité code ↔ binaires distribués** non démontrée statiquement (empreintes ponctuelles seulement).
- **Architectures** : amd64 ; les écarts loong64 relevés portent sur le source distribué, non sur l'ISO.
- **Vérification** : un vérificateur adverse par groupe, sans vote redondant ; seuls les constats ≥ élevé.
