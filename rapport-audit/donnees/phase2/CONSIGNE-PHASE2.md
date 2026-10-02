# Consigne commune — Phase 2 (différentiel des forks amont P2)

Audit de sécurité indépendant, neutre et reproductible d'openKylin 3.0 « huanghe ». Tu es auditeur senior.
Posture : ni présomption de culpabilité, ni naïveté. Tu cherches des FAITS vérifiables. Tu ne conclus JAMAIS
« sûr » par absence de preuve : tu chiffres ta couverture réelle.

## Objet
Composants P2 = **forks de logiciels critiques en amont** (noyau, systemd, openssl, sudo, openssh…). Pour
ces paquets **non natifs**, l'amont `.orig` est identique à Debian/Ubuntu ; **le delta openKylin est dans
`debian/`**, principalement dans **`debian/patches/`** (série quilt), plus `debian/rules`, les scripts de
mainteneur (`*.preinst/postinst/prerm/postrm`), `debian/*.install`, `debian/control` et la configuration
(`debian/config/` pour le noyau). **C'est ce delta que tu audites.**

## Règles strictes
1. LECTURE SEULE. Ne modifie/crée/supprime AUCUN fichier sous `/home/user/openkylin-src/` ni dans
   `/home/user/AuditOK/`. Tu écris UNIQUEMENT ton JSON de résultats dans
   `/tmp/claude-0/-home-user-AuditOK/b9eeaf80-1104-5cd1-a604-a12e4e20f033/scratchpad/phase2/`.
2. Pas de compilation, pas d'exécution de binaires/scripts/code du corpus, pas d'accès réseau.
   `file`, `strings`, `readelf`, `grep`, `sed -n`, `cat` sont autorisés (lecture).
3. Vérifie d'abord l'empreinte de ton arbre et compare à
   `/home/user/AuditOK/rapport-audit/donnees/empreintes-arbres-sources.tsv` (colonne 3). Signale tout écart.
   `cd /home/user/openkylin-src && find archive/<nom> -path '*/.git' -prune -o -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum`
4. Secrets : ne recopie jamais la valeur ; écris `<masqué>` + fichier:ligne.
5. Chaque constat = fichier relatif + numéro de ligne + extrait court. Aucune exception.

## Méthode différentielle
- **Lis la série de patches** (`debian/patches/series`) puis **chaque patch**. Pour chacun :
  - Résume ce qu'il change réellement (fichiers amont touchés, nature).
  - **Vérifie que le TITRE/nom du patch correspond à son CONTENU** (risque connu chez openKylin : des
    correctifs au titre trompeur — p. ex. « update changelog » contenant du code). Signale tout écart.
  - Classe-le : **[LÉGITIME]** (rétroportage amont connu, i18n, correctif bénin), **[À EXAMINER]**
    (compris, sans risque établi), **[SUSPECT]** (écarte la posture de sécurité de l'amont, modifie un
    mécanisme sensible, ou titre trompeur).
- Audite aussi `debian/rules` (durcissement retiré ? `hardening`, `DEB_BUILD_MAINT_OPTIONS`, tests
  désactivés, options de configuration), les **scripts de mainteneur** (exécutés en root : commandes
  dangereuses, écriture hors dpkg, `kysec_set`, liens, chemins monde-inscriptibles), `debian/*.install`
  (fichiers déposés), `debian/control` (dépendances, Provides/Replaces/Conflicts inhabituels), et pour le
  noyau `debian/config/` (drapeaux de sécurité désactivés : LEGACY_TIOCSTI, LOCKDOWN, MODULE_SIG, BPF,
  STACKPROTECTOR, FORTIFY, RANDSTRUCT, etc. — signale les divergences de durcissement).
- Cherche spécifiquement : désactivation de vérifications TLS/crypto, bypass d'authentification, portes
  dérobées (comptes/clés en dur), changements de valeurs par défaut sensibles, nouveaux points réseau,
  SUID/capabilities, désactivation de SELinux/AppArmor/seccomp, affaiblissement de PAM.

## Échelle de sévérité
- **critique** : exploitable à distance, ou par tout utilisateur local → root sans condition ; bypass
  d'authentification ; exécution de code sur un canal non vérifié.
- **elevee** : élévation locale sous conditions réalistes ; affaiblissement crypto/TLS/auth sur flux
  sensible ; durcissement noyau/compilation retiré créant un risque réel.
- **moyenne** : faiblesse sous conditions particulières ; défaut de durcissement notable.
- **faible** : durcissement mineur manquant. **informationnelle** : fait d'inventaire utile.

## Registres
- **CONSTATÉ** (lu, fichier:ligne) / **PROBABLE** (indice fort, dis ce qui manque). Pour tout constat
  ≥ moyenne : **scénario d'exploitation réaliste** ET **contre-lecture bénigne plausible**.

## Couverture (obligatoire)
Compte les patches et les fichiers `debian/` pertinents ; indique combien lus intégralement/partiellement,
le pourcentage, et la liste des patches lus intégralement.

## Format de sortie (OBLIGATOIRE) — JSON `phase2/<lot>.json`
```json
{"lot":"P2-<nom>","composants":["<nom>"],"empreintes":{"archive/<nom>":"<sha256>","conforme":true},
 "findings":[{"depot":"<nom>","fichier":"<chemin relatif>","ligne":"<n ou n-m>","type":"reseau|telemetrie|crypto|persistance|obfuscation|confiance|privileges|durcissement",
   "severite":"critique|elevee|moyenne|faible|informationnelle","extrait":"<=300 car, secrets masqués>",
   "explication":"<fait + portée>","scenario":"<si >= moyenne>","contre_lecture":"<si >= moyenne>","registre":"CONSTATÉ|PROBABLE"}],
 "patches":[{"nom":"<fichier patch>","titre_declare":"<entête>","contenu_reel":"<résumé>","titre_fidele":true,"classement":"[LÉGITIME]|[À EXAMINER]|[SUSPECT]","fichiers_amont":["..."],"resume":"<court>"}],
 "flux_reseau":[],"collectes":[],
 "couverture":[{"arbre":"archive/<nom>","patches_total":0,"patches_lus":0,"fichiers_debian":0,"pct_patches":0,"patches_lus_liste":["..."]}],
 "non_couvert":["..."],
 "synthese":"<10-20 lignes, factuelles, avec registres>"}
```
Ta réponse finale (texte) = résumé de 30 lignes max (constats principaux avec sévérité et registre,
nombre de patches par classement, couverture). PAS le JSON entier. N'affiche PAS de code d'exploitation
dans ta réponse texte ; décris les risques, cite fichier:ligne.
