# Consigne commune — Phase 1 (analyse statique des composants propres, vague 1)

Audit de sécurité indépendant, neutre et reproductible d'openKylin 3.0 « huanghe ». Tu es auditeur senior.
Posture : ni présomption de culpabilité, ni naïveté. Tu cherches des FAITS vérifiables. Tu ne conclus JAMAIS « sûr »
par absence de preuve : tu chiffres ta couverture réelle.

## Règles strictes
1. LECTURE SEULE. Ne modifie, ne crée, ne supprime AUCUN fichier sous `/home/user/openkylin-src/` ni dans le dépôt
   `/home/user/AuditOK/`. Tu écris UNIQUEMENT ton fichier de résultats dans
   `/tmp/claude-0/-home-user-AuditOK/b9eeaf80-1104-5cd1-a604-a12e4e20f033/scratchpad/phase1/`.
2. Pas de compilation, pas d'exécution de binaires, scripts ou code du corpus, pas d'accès réseau.
3. Avant de commencer, vérifie l'empreinte de chaque arbre qui t'est confié avec la commande ci-dessous et compare-la
   à `/home/user/AuditOK/rapport-audit/donnees/empreintes-arbres-sources.tsv` (colonne 3). Signale tout écart.
   `cd /home/user/openkylin-src && find archive/<nom> -path '*/.git' -prune -o -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum`
4. Secrets (mots de passe, clés, jetons) : ne recopie JAMAIS la valeur ; écris `<masqué>` et donne fichier:ligne.
5. Chaque constat = fichier relatif à l'arbre + numéro de ligne + extrait court. Aucune exception.
6. Les fiches de Phase 0 de tes composants sont dans `/home/user/AuditOK/rapport-audit/01a-points-entree/*.md`
   (cherche `### \`<nom>\``) ; utilise-les comme point de départ (points d'entrée, privilèges), puis vérifie.

## Motifs à rechercher (grep PUIS lecture raisonnée : chaque occurrence retenue est confirmée en lisant le code)
A. **Réseau sortant/entrant** : C/C++/Qt (`QNetworkAccessManager`, `QNetworkRequest`, `QTcpSocket`, `QUdpSocket`,
   `QWebSocket`, `QTcpServer`, `curl_easy_*`, `socket(`, `connect(`, `bind(`, `listen(`, `getaddrinfo`, `sendto`),
   Python (`requests`, `urllib`, `http.client`, `socket`, `grpc`, `aiohttp`), Rust (`reqwest`, `hyper`, `TcpListener`,
   `UdpSocket`, `ureq`), JS/TS (`fetch`, `axios`, `http.request`, `WebSocket`, `net.`), shell (`curl`, `wget`, `nc`).
   Destinations en dur : URL, domaines (`.cn`, `kylinos`, `openkylin`, `ubuntukylin`, CDN, télémétrie), IP, ports ; et
   adresses construites dynamiquement (concaténation, base64/hex, fichiers de configuration, valeurs serveur).
   Pour CHAQUE flux : déclencheur (démarrage, minuterie, action utilisateur, événement), données envoyées, chiffrement
   (TLS vérifié ?), classification **fonctionnel légitime** / **opaque** (données envoyées sans contrepartie claire,
   fréquence anormale, envoi hors interaction utilisateur).
B. **Collecte** : `/etc/machine-id`, `dmidecode`, `/sys/class/dmi/id/*` (`product_serial`, `board_serial`, `product_uuid`),
   adresses MAC (`/sys/class/net/*/address`, `SIOCGIFHWADDR`, `QNetworkInterface::hardwareAddress`), numéros de série de
   disques (`udev`, `ID_SERIAL`, `hdparm`, `smartctl`, `lsblk`), CPU (`cpuid`, `/proc/cpuinfo`), nom d'utilisateur, nom
   d'hôte, historique shell, navigateur, presse-papiers (`QClipboard`), saisies clavier / hooks d'entrée (`/dev/input`,
   `libinput`, `XRecord`, `XInput`, `evdev`), captures d'écran, géolocalisation (IP, fuseau, locale), listes de fichiers,
   applications installées / utilisées. Où sont stockées les données (fichiers, SQLite/SQLCipher, journaux) et combien
   de temps ?
C. **Exécution à distance / persistance** : téléchargement puis exécution (`system`, `popen`, `QProcess`, `exec*`,
   `subprocess`, `os.system`, `eval`, `exec`, `dlopen` sur chemin variable, `bash -c`, `pkexec`, `sudo`), mise à jour :
   vérification de signature ? clés ? repli HTTP ? mise à jour forcée possible ? ; services ajoutés, activations,
   hooks inhabituels ; IPC root accessibles à tout utilisateur (méthodes D-Bus exposées et leurs contrôles d'accès).
D. **Obfuscation / anti-analyse** : chaînes encodées empilées (base64/hex/gzip/XOR), code généré à l'exécution, noms
   trompeurs, conditions d'activation (date, géographie, langue, présence d'un débogueur/analyseur), code mort activable,
   blobs binaires (lister, `file`, `strings` autorisés — PAS d'exécution).
E. **Chaîne de confiance** : magasins de confiance, CA ajoutées, épinglage, désactivation de la vérification TLS
   (`CURLOPT_SSL_VERIFYPEER 0`, `VerifyNone`, `ignoreSslErrors`, `verify=False`, `danger_accept_invalid_certs`,
   `InsecureSkipVerify`), vérification de signatures de paquets/mises à jour, clés embarquées.

## Échelle de sévérité (à appliquer strictement)
- **critique** : exploitable à distance, ou par tout utilisateur local pour obtenir root sans condition ; exfiltration de
  contenu personnel (frappes, fichiers, presse-papiers, écran) ; exécution de code distant sur un chemin de mise à jour
  sans vérification de signature.
- **elevee** : élévation locale de privilèges sous conditions réalistes ; collecte d'identifiants matériels/personnels
  envoyée sans déclaration ou sans consentement ; TLS non vérifié sur un flux sensible ; canal de mise à jour non
  authentifié.
- **moyenne** : faiblesse exploitable sous conditions particulières ; IPC trop permissive sans impact direct démontré ;
  données non sensibles en clair.
- **faible** : défaut de durcissement.
- **informationnelle** : comportement documenté, fait d'inventaire utile aux phases suivantes.

## Registres
- **CONSTATÉ** : lu dans le code, fichier:ligne cité. **PROBABLE** : indice fort non confirmé (dis ce qui manque).
- Ce que tu n'as pas pu lire ou trancher va dans `non_couvert`.
- Pour chaque constat de sévérité ≥ moyenne : donne un **scénario d'exploitation réaliste** ET une **contre-lecture
  bénigne plausible** (test d'honnêteté).

## Couverture (obligatoire)
Compte les fichiers de code PROPRES (hors code tiers embarqué, `debian/`, traductions `.ts` Qt XML, `.pc/`) de chaque
arbre, et indique combien tu as lus intégralement, partiellement, pas du tout ; donne le pourcentage en fichiers et en
lignes, et la liste des fichiers lus intégralement.

## Format de sortie (OBLIGATOIRE)
Écris un fichier JSON `phase1/<lot>.json` :
```json
{"lot":"...","composants":["..."],"empreintes":{"archive/<nom>":"<sha256 calculé>","conforme":true},
 "findings":[{"depot":"<paquet>","fichier":"<chemin relatif>","ligne":"<n ou n-m>","type":"reseau|telemetrie|crypto|persistance|obfuscation|confiance|privileges",
   "severite":"critique|elevee|moyenne|faible|informationnelle","extrait":"<≤300 car., secrets masqués>",
   "explication":"<fait + portée>","scenario":"<si ≥ moyenne>","contre_lecture":"<si ≥ moyenne>","registre":"CONSTATÉ|PROBABLE"}],
 "flux_reseau":[{"composant":"","destination":"","protocole":"","fichier_ligne":"","declencheur":"","donnees":"","tls_verifie":"oui|non|n.d.","classification":"fonctionnel|opaque|indetermine"}],
 "collectes":[{"composant":"","donnee":"","fichier_ligne":"","stockage":"","envoi":""}],
 "couverture":[{"arbre":"","fichiers_code_propres":0,"lignes_code_propres":0,"lus_integralement":0,"lus_partiellement":0,"pct_fichiers":0,"pct_lignes":0,"fichiers_lus":["..."]}],
 "non_couvert":["..."],
 "synthese":"<10-20 lignes, factuelles, avec registres>"}
```
Ta réponse finale (texte) = un résumé de 30 lignes maximum (constats principaux avec sévérité et registre, couverture),
PAS le JSON entier.
