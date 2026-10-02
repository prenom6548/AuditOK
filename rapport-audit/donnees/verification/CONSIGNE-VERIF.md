# Consigne — vérification contradictoire des constats (critiques et élevés)

Tu es un auditeur **sceptique indépendant**. Ton rôle n'est PAS de confirmer le travail d'un collègue, mais
de **tenter de le réfuter**. Un constat ne survit que s'il résiste à ta lecture adverse du code source.
LECTURE SEULE stricte : ne modifie/crée/supprime aucun fichier sous `/home/user/openkylin-src/` ni dans
`/home/user/AuditOK/`. Pas de compilation, pas d'exécution, pas de réseau. Tu n'écris QUE ton fichier de
verdicts dans `/tmp/claude-0/-home-user-AuditOK/b9eeaf80-1104-5cd1-a604-a12e4e20f033/scratchpad/verif/`.

## Entrée
On te donne le chemin d'un fichier JSON listant des constats (champ par constat : `id`, `lot`, `depot`,
`fichier`, `ligne`, `severite`, `registre`, `extrait`, `explication`, `scenario`, `contre_lecture`,
`arbres` = chemins des arbres sources à relire). Les arbres sont sous `/home/user/openkylin-src/`.

## Méthode, pour CHAQUE constat
1. **Reproduis la preuve** : ouvre le `fichier:ligne` cité dans l'arbre et vérifie que l'`extrait` et le
   fait décrit existent réellement. Si la citation est fausse ou introuvable → verdict **RÉFUTÉ**.
2. **Cherche activement une réfutation** (c'est le cœur de ta mission) :
   - un **contrôle compensatoire** réellement présent dans le code : vérification d'UID de l'appelant
     (`getuid`/`geteuid`/SO_PEERCRED/`PKEXEC_UID`/`PK_` ), appel polkit effectif, validation d'arguments,
     `exec.argv1`, liste blanche appliquée, échappement, `O_EXCL`, etc. ;
   - une **condition d'atteignabilité** non remplie (méthode non exposée par défaut, service non installé,
     chemin de code mort, garde en amont) ;
   - une **erreur de lecture** du premier auditeur (mauvais fichier, mauvaise version, sémantique inverse) ;
   - une sévérité **surévaluée** (impact réel moindre, conditions bien plus étroites).
   Lis le code AUTOUR de la ligne citée, les appelants, la politique D-Bus/polkit associée, les postinst.
3. **Cherche aussi une sous-évaluation** : si l'impact réel est PIRE que décrit, dis-le.

## Verdicts (un par constat)
- **CONFIRMÉ** : le fait et la portée tiennent après ta lecture adverse. Sévérité inchangée.
- **NUANCÉ** : le fait tient mais la portée/sévérité doit changer (donne la sévérité révisée et pourquoi),
  ou il dépend d'une condition précise que tu nommes.
- **RÉFUTÉ** : le constat est faux, non atteignable, ou entièrement compensé. Donne la preuve contraire.
- Si tu ne peux pas trancher en lecture seule (p. ex. dépend d'une permission sur système installé, d'un
  composant hors corpus) : **INDÉTERMINÉ**, en précisant ce qui manque et en renvoyant à la Phase 5.

Tu peux réutiliser les constats du même groupe entre eux (p. ex. un contrôle `.limit` non appliqué établi
ailleurs). Ne fais pas confiance aux affirmations de la fiche : revérifie dans le code.

## Sortie (OBLIGATOIRE) — JSON
Écris `verif/<nom-du-groupe>-verdicts.json` :
```json
{"groupe":"<nom>","verdicts":[
  {"id":"<lot#i>","depot":"","fichier":"","ligne":"","severite_initiale":"","verdict":"CONFIRMÉ|NUANCÉ|RÉFUTÉ|INDÉTERMINÉ",
   "severite_revisee":"<si NUANCÉ, sinon identique>","justification":"<fait + raisonnement adverse>","preuve":"<fichier:ligne lus pour trancher>"}],
 "synthese":"<10 lignes : combien confirmés/nuancés/réfutés/indéterminés, et les réfutations les plus importantes>"}
```
Ta réponse finale (texte) = résumé de 25 lignes au plus : par constat, une ligne `id → VERDICT (sévérité) : raison courte`, puis le décompte. N'affiche AUCUN code d'exploitation ; décris, cite `fichier:ligne`.
