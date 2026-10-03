# Audit de sécurité indépendant d'openKylin 3.0 « huanghe »

🇫🇷 Français (cette page) · 🇬🇧 [English version](README.en.md)

> **Audit indépendant, neutre et reproductible.** Ce dépôt n'est **pas** affilié à
> openKylin, à KylinSoft, ni à aucun de leurs partenaires. Il ne reflète aucune
> position institutionnelle : il consigne des **faits vérifiables** et leurs limites.

**État : analyse statique terminée.** Phases 0, 1 (vagues 1-3) et 2 (18 forks critiques, binaires sans
source, durcissement des binaires) menées à leur terme ; les 83 constats critiques et élevés ont été soumis à
une vérification contradictoire (menée par l'IA — voir « Transparence ») ; la phase dynamique (5) prend la
forme d'un [**kit d'observation**](rapport-audit/phase5-kit/LISEZMOI.md) en lecture seule, à exécuter sur une
machine virtuelle ; les **angles morts connus** sont documentés. Voir
[`rapport-audit/00-synthese.md`](rapport-audit/00-synthese.md).

---

## Transparence : comment cet audit a été produit

Cet audit a été **produit avec Claude Code**, un assistant d'intelligence artificielle d'Anthropic, **sous la
direction d'une personne non spécialiste en sécurité** qui l'a orchestré et a décidé de le publier. Nous le
disons d'emblée — l'historique Git le montre de toute façon.

Deux précisions d'honnêteté, parce qu'elles comptent :

- **Aucun expert humain indépendant n'a validé ce travail.** La vérification contradictoire citée plus bas a,
  elle aussi, été menée par l'IA (un second passage sceptique chargé de réfuter chaque constat) : c'est mieux
  qu'un seul regard, mais ce n'est pas une relecture humaine qualifiée.
- **Une IA peut se tromper avec aplomb.** Un constat peut être plausible et pourtant faux — une ligne mal
  interprétée, un garde-fou présent ailleurs. Lors de la vérification, une vingtaine de constats ont d'ailleurs
  été nuancés ou restreints.

C'est pourquoi ce dépôt est conçu pour **ne rien exiger qu'on le croie sur parole** — à l'opposé des rapports
produits en masse par IA, vagues et invérifiables, qui épuisent les mainteneurs :

- **Chaque constat est vérifiable** : dépôt + fichier + numéro de ligne + extrait.
- **Tout est reproductible** : acquisition par chaîne signée, empreintes fournies, outils en lecture seule
  (voir « Reproductibilité et vérification »).
- **Aucun code d'exploitation**, et l'éditeur a été **prévenu avant publication**.
- Les **limites et la couverture réelle** sont chiffrées et assumées (voir « Limites de l'audit »).

**Ne nous croyez pas : vérifiez.** Prenez n'importe quel constat, ouvrez le fichier à la ligne indiquée, jugez
par vous-même. La validation doit venir de personnes compétentes dans le code concerné — openKylin au premier
chef. Toute erreur signalée et étayée est la bienvenue : une infirmation sur pièces vaut autant qu'un constat.

## Pourquoi ce dépôt

openKylin est une distribution GNU/Linux dérivée de Debian/Ubuntu, avec un bureau
UKUI, une option de système immuable (OSTree), une couche de compatibilité Android
(KMRE), un SDK d'intelligence artificielle maison et de nombreux composants
développés par l'écosystème Kylin. Sa surface est vaste, en partie spécifique, et
une part du code n'a pas d'équivalent public amont.

Cet audit poursuit un objectif simple : **permettre à quiconque de se forger une
opinion étayée**, sur pièces, plutôt que sur une réputation ou une rumeur. Nous
publions nos constats, **mais aussi** nos outils, nos empreintes et le périmètre
exact de ce que nous avons — et surtout de ce que nous n'avons **pas** — examiné.

## Posture d'analyse

Trois principes, appliqués sans exception :

1. **Ni présomption de culpabilité, ni naïveté.** Nous cherchons des faits, pas une
   conclusion décidée d'avance.
2. **On ne conclut jamais « sûr » par absence de preuve.** L'absence d'un constat
   n'est pas une preuve d'innocuité : elle reflète d'abord notre couverture, que
   nous chiffrons (quels fichiers, quel pourcentage du volume, quels chemins non
   explorés).
3. **Tout constat est tracé** : dépôt + fichier + numéro de ligne + extrait. Sans
   exception.

Chaque affirmation porte l'un de trois **registres** :

| Registre | Sens |
|---|---|
| **[CONSTATÉ]** | Lu dans le code ; preuve citée (fichier : ligne). |
| **[PROBABLE]** | Indice fort, non confirmé ; ce qui manque est précisé. |
| **[NON COUVERT]** | Hors de portée de l'analyse menée ; ne permet aucune conclusion. |

Pour tout constat de sévérité moyenne ou plus, nous donnons **à la fois** un
scénario d'exploitation réaliste **et** une contre-lecture bénigne plausible — un
test d'honnêteté qui protège le lecteur comme l'édité.

## Comment lire cet audit

Dans l'ordre recommandé :

1. [`rapport-audit/00-synthese.md`](rapport-audit/00-synthese.md) — verdict à trois
   niveaux, constats structurants, et la **liste explicite des limites**.
2. [`rapport-audit/01-inventaire.md`](rapport-audit/01-inventaire.md) — méthode,
   paysage APT réel, inventaire des paquets sources, forges, et **matrice de
   couverture**.
3. [`rapport-audit/06-angles-morts.md`](rapport-audit/06-angles-morts.md) — ce que
   l'audit **ne peut pas** trancher (binaires sans source, paquets à source vide,
   composants sans dépôt public, etc.).
4. [`rapport-audit/02-findings.csv`](rapport-audit/02-findings.csv) — 704 constats traçables (fichier : ligne,
   scénario, contre-lecture, verdict de vérification). Pour le consulter directement sur GitHub sous forme de
   tableau filtrable : [`02-findings-apercu.csv`](rapport-audit/02-findings-apercu.csv) (mêmes constats, sans les colonnes longues, explication parfois abrégée — GitHub n'affiche en
   tableau que les CSV de 512 Ko au plus).
5. [`rapport-audit/07-verification.md`](rapport-audit/07-verification.md) — vérification contradictoire des
   constats critiques et élevés.

## Organisation du dépôt

```
.
├── LICENSE              # AGPL-3.0 (texte intégral) — s'applique à NOTRE travail
├── LICENCES.md          # Périmètre des licences — à lire avant toute réutilisation
├── README.md            # Ce fichier
│
├── rapport-audit/       # Notre travail original (AGPL-3.0-or-later)
│   ├── 00-synthese.md           # Synthèse et verdict
│   ├── 01-inventaire.md         # Inventaire + matrice de couverture
│   ├── 01a-points-entree*       # Recensement des points d'entrée (par lots)
│   ├── 02-findings.csv          # 704 constats (fichier : ligne, extrait, vérification)
│   ├── 02-findings-apercu.csv   # Idem, allégé : consultable en tableau sur GitHub
│   ├── 03-diffs-suspects.md     # Différentiel des forks amont (Phase 2)
│   ├── 04-protocole-dynamique.md# Méthode de la Phase 5 (référence du kit)
│   ├── 05-donnees.md            # Flux réseau et collectes de données
│   ├── 06-angles-morts.md       # Angles morts et limites
│   ├── 07-verification.md       # Vérification contradictoire (critiques et élevés)
│   ├── phase5-kit/              # Phase 5 : kit d'observation en lecture seule, pour une VM
│   ├── en/                      # Traductions anglaises des rapports principaux
│   ├── donnees/                 # Données d'analyse générées + artefacts openKylin
│   │                            #   redistribués (index APT signés, manifestes ISO)
│   └── outils/                  # Scripts de reproduction (lecture seule stricte)
│
└── sources-openkylin/   # Code et artefacts TIERS — licences d'origine (voir LICENCES.md)
    ├── archive/                 # Paquets sources extraits, vérifiés par SHA-256
    ├── gitee/                   # Clones superficiels de dépôts openKylin
    ├── dsc/                     # Fichiers .dsc des paquets sources
    ├── index-apt/               # Index de dépôts signés (vérifiables)
    ├── binaires-sans-source/    # Paquets .deb sans source exploitable
    ├── LISEZMOI.md              # Provenance et authenticité du corpus
    └── EXCLUSIONS.tsv           # Fichiers exclus (limites de taille) + leur SHA-256
```

## Reproductibilité et vérification

La reproductibilité est au cœur de la démarche. Deux garanties concrètes :

- **Chaque arbre analysé a une empreinte.** Elles figurent dans
  `rapport-audit/donnees/empreintes-arbres-sources.tsv`. Toute ré-analyse doit
  d'abord vérifier qu'elle porte sur des octets identiques :

  ```sh
  cd sources-openkylin
  find archive/<paquet> -path '*/.git' -prune -o -type f -print0 \
    | LC_ALL=C sort -z | xargs -0 sha256sum | sha256sum
  ```

- **La chaîne de confiance de distribution est vérifiable.** Les index APT inclus
  (`InRelease`, `Sources.gz`…) sont signés ; leurs signatures se contrôlent avec
  `gpgv` et les clés documentées dans `rapport-audit/donnees/apt/`. Les archives
  sources ont été authentifiées par SHA-256 contre ces index signés.

Les outils de reproduction (lecture seule, paramétrés par les variables
`AUDIT_WORK` et `AUDIT_SRC`) sont décrits dans
[`rapport-audit/outils/LISEZMOI.md`](rapport-audit/outils/LISEZMOI.md).

> **Note technique.** Certains arbres embarquent des règles `filter=lfs`. Les
> fichiers sont versionnés en octets bruts. À l'extraction, neutralisez les filtres
> (`GIT_LFS_SKIP_SMUDGE=1`, puis `printf '* -text -eol -filter\n' > .git/info/attributes
> && git checkout -f`) pour retrouver les mêmes octets.

## Limites de l'audit (à lire absolument)

Un audit honnête énonce ses limites aussi clairement que ses constats. Les
principales, à l'état actuel :

- **Volume partiel.** Seule une fraction des milliers de paquets sources est
  examinée en profondeur ; le reste n'est couvert que par inventaire. La couverture
  réelle est chiffrée, composant par composant, dans l'inventaire.
- **Binaires non auditables.** De nombreux composants sont distribués sans source
  exploitable (squelettes de paquets, binaires propriétaires, modèles d'IA, image
  Android) : ils ne peuvent être examinés que par inventaire et métadonnées.
- **Égalité code ↔ binaires distribués non démontrée statiquement.** Nous ne
  compilons ni n'exécutons le corpus. La correspondance exacte entre un binaire
  installé et sa source relève de la phase dynamique : le kit de la Phase 5, à exécuter en VM.
- **Analyse statique, en lecture seule stricte.** Aucun fichier du corpus n'a été
  modifié, compilé ou exécuté ; aucun accès réseau n'a été fait depuis l'analyse.
  La confirmation sur un système réel passe par le kit de la Phase 5.

La liste exhaustive et à jour est maintenue dans
[`rapport-audit/00-synthese.md`](rapport-audit/00-synthese.md) (§ Limites) et
[`rapport-audit/06-angles-morts.md`](rapport-audit/06-angles-morts.md).

## Usage responsable

Ce dépôt a vocation à **aider** : utilisateurs soucieux de leur sécurité,
administrateurs, chercheur·se·s, et l'éditeur lui-même. Dans cet esprit :

- Les constats sont le résultat d'une **analyse statique indépendante** ; sauf
  mention explicite [CONSTATÉ] assortie d'une preuve, plusieurs restent à confirmer
  dynamiquement.
- Ce travail ne fournit **aucun outil d'exploitation** clé en main : il décrit des
  chaînes de risque à des fins défensives et de vérification.
- **L'éditeur a été prévenu avant la publication de ce dépôt.** La coordination suivie
  (relances, délai, suivi des correctifs) reste **ouverte à qui, dans la communauté, souhaite
  s'en saisir** ; toute personne qui confirme une faiblesse peut aussi en informer openKylin
  par ses canaux de signalement.

## Licence

- **Notre travail** (rapports, outils, données d'analyse générées) :
  **AGPL-3.0-or-later** — texte intégral dans [`LICENSE`](LICENSE).
- **Le code et les artefacts tiers** sous `sources-openkylin/` conservent **leurs
  licences d'origine** (GPL, LGPL, Apache, MIT, BSD… et parfois propriétaires).
  **Nous ne relicencions rien.**

Le périmètre précis — et pourquoi ces deux natures coexistent dans un même dépôt
(simple agrégation au sens de l'AGPLv3) — est détaillé dans
[`LICENCES.md`](LICENCES.md). **Lisez-le avant toute réutilisation**, en particulier
avant de redistribuer un composant tiers.

## Contribuer

Les contributions qui **renforcent la rigueur** sont bienvenues : correction d'une
erreur de lecture, confirmation ou infirmation d'un constat sur pièces, extension
de la couverture, reproduction indépendante. Toute contribution doit respecter la
posture ci-dessus : faits tracés (fichier : ligne), registres explicites, et aucune
conclusion « sûr » tirée d'une absence de preuve.
