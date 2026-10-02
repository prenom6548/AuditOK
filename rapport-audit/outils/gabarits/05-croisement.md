## 3. Croisement code ↔ politique de confidentialité

La politique publiée (`https://www.openkylin.top/support/privacy_policy-cn.html`, version chinoise
archivée dans `donnees/politique-confidentialite/`) décrit une collecte au niveau du système. Extraits
datés, puis faits relevés dans le code (Phase 1 vagues 1-3), puis écarts.

### 3.1 Ce que la politique déclare (version CN)
- **l.176** : mise à jour → « informations système **non identifiantes** » (« 非个人身份识别的系统信息 »).
- **l.195-196** : collecte DAQ des « **informations matérielles et logicielles importantes** ».
- **l.203** : heures d'allumage/extinction et plages horaires.
- **l.192 / 206-210** : interactions panneau de configuration, menu démarrer, barre latérale, applications.
- **l.177/189/190/226-227** : mise à jour/installation/boutique (codes d'erreur, noms de paquets, versions…).
- **l.154** : transmission « **sûre et contrôlée** » ; **l.454** : stockage en Chine continentale.
- **l.296-297 / 381-382** : forum/site (IP, matériel, opérateur) ; analyse tierce **Baidu统计**.

### 3.2 Canaux de collecte relevés dans le code [CONSTATÉ]

| Canal | Données | Transport | Déclenchement | Lot |
|---|---|---|---|---|
| `kylin-daq` (root) → `datareporting.openkylin.top:30005` | `machine-id`, n° de série DMI, clé d'activation, versions, événements d'usage relayés | HTTPS **sans vérification de certificat** (`VerifyNone`) ; reconfiguration persistante sur réponse serveur | démon, minuteries | V1-L1 |
| Points d'usage (« 埋点 ») : ukui-control-center (189 points), kabase, ukui-search, peony, assistant IA | événements d'interface, fuseau horaire, nom du compte d'auto-connexion, chemins, modèle IA choisi ; identifiant persistant `~/.config/buried-point` | relayés vers `com.kylin.daq` (canal ci-dessus) | usage | V2-L2, V2-L3, V2-L4 |
| `kylin-system-updater` `DataAcquisition` → `archive1.kylinos.cn:32294` | MAC, n° de série, IP, paquets, codes d'erreur, journaux `.tar.gz` | **HTTP en clair** | fin d'installation / mise à jour | V1-L5 |
| `kylin-os-manager` `service-support` → `kom.kylinos.cn` | archive : n° de série (`dmidecode`), MAC, `/etc/.kyinfo`, journaux de politique, coordonnées ; vidages mémoire d'applications (core-dump-monitor) | HTTPS **sans vérification de certificat** (`VerifyNone`) ; `EnableUpload=true` par défaut | rétroaction / signal système non authentifié | V2-L3 |
| `driver-acquirer` (root) → `api.kylinos.cn` | VID/PID, fabricant, modèle des périphériques | HTTPS | recherche de pilotes | V2-L3 |
| `libkysdk-location` (root) → `api.map.baidu.com` | IP publique → localisation | **HTTP en clair**, clé Baidu en dur | appel D-Bus (API dite « non utilisée depuis 3.0.1.0 ») | V1-L3 |
| Moteurs IA cloud (Baidu, iFlytek, Qwen/DashScope, DeepSeek, personnalisé) | invites, audio du micro, textes lus, documents soumis | HTTPS ; **parole Baidu/iFlytek sans vérification de certificat** (libhv) | requête de l'utilisateur, clés de l'utilisateur | V2-L4 |
| Base de connaissances (`knowledge-base-service`) | extraits de documents + question | vers le modèle « Default » (cloud d'essai par défaut, [PROBABLE]) | question de l'utilisateur | V2-L4, V1-L8 |

### 3.3 Écarts constatés
1. **« Non identifiant » vs identifiants matériels.** Trois canaux distincts (DAQ, updater, service-support)
   transmettent des identifiants matériels stables et ré-identifiants (MAC, numéro de série, `machine-id`,
   clé d'activation). Écart de *qualification* ([CONSTATÉ] vs l.176).
2. **« Transmission sûre et contrôlée » vs transport non authentifié.** Trois canaux en HTTPS **sans
   vérification de certificat** (DAQ, service-support, moteurs de parole) et deux en **HTTP clair**
   (updater, géolocalisation). Écart de *moyen* ([CONSTATÉ] vs l.154) — le plus net de l'audit.
3. **Collectes non énumérées.** La politique ne mentionne ni l'envoi de **vidages mémoire**
   d'applications, ni les **journaux de politique de sécurité**, ni les **coordonnées** de l'archive de
   rétroaction, ni la remontée des identifiants de **périphériques** pour la recherche de pilotes.
4. **Consentement.**
   - Système (DAQ, updater, service-support) : chemins actifs par défaut (`upload_upgrade_log`,
     `EnableUpload=true`) ; aucune barrière de consentement lue dans ces composants [PROBABLE], à
     confirmer au premier démarrage (Phase 5, D6-D8).
   - Assistant IA : un accord bloquant s'affiche au premier lancement (refus = fermeture), mais son texte
     **ne nomme aucun destinataire** et ne mentionne ni le modèle d'essai cloud ni la télémétrie
     ([CONSTATÉ], V2-L4). Consentement présent mais **non éclairé**.

### 3.4 Ce qui est cohérent avec la politique
- La **nature** de la collecte système (matériel, horaires, interactions, erreurs de mise à jour) correspond
  aux catégories déclarées (l.176-227).
- Les **moteurs IA** ne partent que sur requête de l'utilisateur et avec ses propres clés ; aucun identifiant
  matériel n'y est joint ; l'assistant n'expédie que le contenu soumis (sélection après clic, capture
  d'écran seulement si le partage vocal est activé — désactivé par défaut).
- `ukui-clipboard` : historique en mémoire de session uniquement, non persisté, non transmis (mais sans
  exclusion des mots de passe).
- `kylin-bot` : aucun flux par défaut tant qu'aucun fournisseur n'est configuré.

### 3.5 Non couvert
- Destination réelle et TLS du moteur **freetrial** et de la clé par défaut (`libkylin-ai-default-key`) :
  binaires sans source.
- Contenu effectivement émis par un système réel (la VM de la Phase 5 n'aura pas de DMI réel).
