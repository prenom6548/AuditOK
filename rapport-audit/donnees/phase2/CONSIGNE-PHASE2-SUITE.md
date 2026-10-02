# Consigne complémentaire — Phase 2, suite (forks avec sources de référence)

S'applique **en plus** de [`CONSIGNE-PHASE2.md`](CONSIGNE-PHASE2.md), dont toutes les règles restent valables
(lecture seule stricte, aucune exécution, aucun accès réseau, fichier : ligne + extrait pour chaque constat,
secrets masqués, registres, couverture chiffrée, format JSON).

## Ce qui change : une référence vérifiée est disponible

Pour ces forks, l'outil [`../../outils/references_p2.py`](../../outils/references_p2.py) a récupéré, par une
chaîne authentifiée (InRelease vérifié par `gpgv` → index → `.dsc` → fichiers, SHA-256 à chaque étape), les
sources homologues d'Ubuntu, de Debian et, pour la glibc et grub2, l'archive GNU signée. Il a comparé fichier
par fichier l'archive amont (`orig`) et le `debian/` d'openKylin à chacune de ces références.

| Élément | Emplacement (lecture seule) |
|---|---|
| Comparaisons par paquet | `/home/user/AuditOK/rapport-audit/donnees/phase2/references/<paquet>.json` |
| Provenance de chaque fichier de référence | `/home/user/AuditOK/rapport-audit/donnees/phase2/references/provenance.tsv` |
| `debian/` openKylin audité | `/home/user/AuditOK/sources-openkylin/archive/<paquet>/debian/` |
| `orig` openKylin décompressé | `/home/user/openkylin-src/ref-p2/openkylin/<paquet>_<version>/orig/` |
| `debian/` d'une référence | `/home/user/openkylin-src/ref-p2/<ubuntu\|debian>/<paquet>_<version>/deb/` |
| `orig` d'une référence | `/home/user/openkylin-src/ref-p2/<ubuntu\|debian>/<paquet>_<version>/orig/` |
| Archive GNU décompressée (glibc, grub2) | `/home/user/openkylin-src/ref-p2/gnu/<paquet>-src/` |

Ne modifie rien dans ces dossiers. `diff -u`, `cmp`, `sha256sum` sont autorisés (lecture).

## Méthode

1. **Lis `references/<paquet>.json`** : référence la plus proche, série de correctifs comparée, état de l'`orig`.
2. **Correctifs identiques octet pour octet à la référence** (`serie.identiques_octet`) : hérités. Liste-les dans
   `patches` avec `classement` `[LÉGITIME]`, `titre_fidele` `true`, `contenu_reel`
   « hérité : identique octet pour octet à <référence> ». Lis au moins leur en-tête. Compte-les dans la
   couverture comme **vérifiés par empreinte** (pas « lus »).
3. **Correctifs propres à openKylin** (`serie.propres_openkylin`) et **de même nom mais de contenu différent**
   (`serie.meme_nom_contenu_different`) : **lecture intégrale**, avec `diff -u` contre la version de référence
   quand elle existe. Applique toute la méthode de `CONSIGNE-PHASE2.md` (fidélité du titre, classement).
4. **Correctifs retirés par openKylin** (`serie.retires_par_openkylin`) : examine chacun dans la référence. Le
   retrait d'un correctif de sécurité (CVE, durcissement) est un constat (`type` `durcissement` ou `confiance`).
   Un retrait justifié (correctif intégré en amont, fonction Ubuntu sans objet) est à noter comme tel.
5. **Autres fichiers de `debian/`** ajoutés ou modifiés par rapport à la référence (`debian.liste_modifies`,
   `debian.liste_ajoutes_openkylin`) : lis le `diff -u` de chacun (règles de construction, scripts de mainteneur,
   `control`, fichiers `.install`, configuration).
6. **Archive amont (`orig`)** : si elle n'est identique octet pour octet à aucune référence, analyse les fichiers
   listés dans `orig.contenu` (modifiés, ajoutés, absents) par `diff -u` entre l'`orig` openKylin, l'`orig` de
   référence et, s'il existe, l'arbre GNU signé. Qualifie la nature de chaque écart (fichier généré,
   empaquetage, horodatage, **ou code**). C'est le **delta hors `debian/`**, que les lots précédents ne
   pouvaient pas vérifier : un écart de code non expliqué dans l'amont est un constat.
7. Pour un paquet **natif** openKylin (sans référence), lis tous ses fichiers intégralement.

## Ajout au format de sortie

Ajoute à ton JSON une clé `reference` :

```json
"reference": {"<paquet>": {"plus_proche": "<archive suite version>", "orig": "<identique à … | écarts : n fichiers, nature>",
  "patches_herites_identiques": 0, "patches_propres_openkylin": 0, "patches_modifies": 0, "patches_retires": 0,
  "fichiers_debian_hors_patches_modifies_ou_ajoutes": 0}}
```

Écris uniquement `/tmp/claude-0/-home-user-AuditOK/b9eeaf80-1104-5cd1-a604-a12e4e20f033/scratchpad/phase2/P2-<lot>.json`.
