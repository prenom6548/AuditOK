# Sources openKylin acquises pour l'audit

🇫🇷 Français (cette page) · 🇬🇧 [English version](LISEZMOI.en.md)

> **Licence : code et artefacts TIERS.** Tout le contenu de ce dossier conserve
> **sa licence d'origine** (openKylin, et en amont Debian/Ubuntu/projets libres ;
> parfois propriétaire). Il n'est **pas** couvert par l'AGPLv3 du reste du dépôt :
> l'audit ne relicencie rien. Voir [`../LICENCES.md`](../LICENCES.md) (§ 2) et, pour
> chaque paquet, son `debian/copyright`.

Corpus utilisé par l'audit (`../rapport-audit/`), versé dans ce dépôt à la demande de l'auditeur principal.
Relevés du 2026-10-01 (UTC). Aucun fichier n'a été modifié : il s'agit d'une copie à l'octet près des arbres analysés,
moins les exclusions listées dans [`EXCLUSIONS.tsv`](EXCLUSIONS.tsv).

| Dossier | Contenu | Provenance et authenticité |
|---|---|---|
| `archive/<paquet>/` | 311 paquets sources extraits par `dpkg-source -x` (correctifs quilt appliqués) ; pour les forks P2 non natifs, seulement `debian/` | `http://archive.build.openkylin.top/openkylin/` (suite `huanghe`) et PPA `anything3.0` ; fichiers vérifiés par SHA-256 contre l'index `Sources` dont l'`InRelease` signé a été vérifié |
| `gitee/<dépôt>/` | 40 clones superficiels (1 commit) | `https://gitee.com/openkylin/<dépôt>` ; branche et commit dans [`gitee/COMMITS-GITEE.tsv`](gitee/COMMITS-GITEE.tsv) ; dossiers `.git` retirés |
| `dsc/` | 312 fichiers `.dsc` des paquets sources acquis | idem `archive/` |
| `index-apt/` | `InRelease`, `Sources.gz`, `Packages.gz`, `Contents-amd64.gz` des dépôts par défaut et de l'archive `archive.openkylin.top` | Signatures vérifiables avec les clés de `../rapport-audit/donnees/apt/` |
| `binaires-sans-source/` | 42 paquets `.deb` de composants sans source exploitable (≤ 100 Mo) | URL et SHA-256 dans `../rapport-audit/donnees/binaires-sans-source/provenance.tsv` |

## Vérifier qu'un arbre est celui qui a été audité

L'empreinte de chaque arbre figure dans `../rapport-audit/donnees/empreintes-arbres-sources.tsv` (colonne 3).
Depuis ce dossier :

```sh
find archive/<paquet> -path '*/.git' -prune -o -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum
```

Exceptions attendues : les deux arbres d'archive amputés d'un fichier de plus de 100 Mo
(`kylin-ai-business-framework-service`, `libkysdk-system`) ont une empreinte différente ici ; les fichiers manquants se
récupèrent dans le `debian.tar` du paquet (URL et SHA-256 dans l'index `Sources`) et leur SHA-256 figure dans
`EXCLUSIONS.tsv`.

## Exclusions

- Dossiers `.git` (Git ne peut pas versionner un chemin `.git`) — dont un dossier `.git` présent **dans le paquet source**
  `ukui-voice-wing` lui-même.
- Fichiers de plus de 100 Mo (limite GitHub) : deux correctifs `debian/patches` intitulés « update changelog »
  (234 Mo et 441 Mo) et deux paquets binaires (`kylin-software-center`, `com.iflytek.iflyime`).
- Archives `orig.tar.*` / `debian.tar.*` téléchargées : non dupliquées (contenu extrait dans `archive/`, re-téléchargeables
  à l'identique via les index signés).

## Note technique

Plusieurs arbres contiennent leurs propres `.gitattributes` (dont des règles `filter=lfs`). Les fichiers ont été
versionnés en octets bruts, sans conversion ni LFS. À l'extraction, désactivez les filtres pour obtenir les mêmes octets,
par exemple : `GIT_LFS_SKIP_SMUDGE=1 git clone …`, puis
`printf '* -text -eol -filter\n' > .git/info/attributes && git checkout -f`.
