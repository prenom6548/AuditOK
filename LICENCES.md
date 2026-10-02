# Périmètre des licences

🇫🇷 Français (cette page) · 🇬🇧 [English version](LICENCES.en.md)

Ce dépôt réunit **deux natures de contenu très différentes**, qui ne relèvent
pas de la même licence. Cette distinction est essentielle, juridiquement comme
éthiquement : **nous ne relicencions pas le code d'autrui.**

Au sens du droit d'auteur, ce dépôt est une **simple agrégation** (au sens de
l'article 5 de l'AGPLv3, « mere aggregation ») : notre travail original, placé
sous AGPLv3+, est distribué **à côté** d'œuvres tierces qui conservent chacune
leur propre licence. L'AGPLv3 s'applique à notre travail ; elle **ne s'étend
pas** aux œuvres tierces agrégées.

---

## 1. Notre travail original — AGPL-3.0-or-later

**SPDX-License-Identifier : `AGPL-3.0-or-later`**

Tout ce qui a été **produit par cet audit** est placé sous licence publique
générale GNU Affero, version 3 ou ultérieure. Le texte complet figure dans le
fichier [`LICENSE`](LICENSE).

Sont concernés :

| Chemin | Nature |
|---|---|
| `rapport-audit/*.md` | Rapports, synthèse, inventaire, angles morts, protocole |
| `rapport-audit/*.csv` | Tableaux de constats produits par l'audit |
| `rapport-audit/outils/**` | Scripts et outils de reproduction écrits pour l'audit |
| `rapport-audit/donnees/**` (*voir réserve ci-dessous*) | Données d'analyse générées : empreintes, recensements, index unifiés, relevés |
| `README.md`, `LICENCES.md` | Documentation du dépôt |

> **Réserve sur `rapport-audit/donnees/`.** Ce dossier contient majoritairement
> nos sorties d'analyse (AGPLv3+), **mais aussi** quelques **artefacts publiés par
> openKylin** et redistribués tels quels à fin de vérification : index APT signés
> (`InRelease`, `Sources.gz`, `Packages.gz`, `Contents-*`), manifestes et métadonnées
> d'ISO. Ces artefacts appartiennent à leur éditeur (openKylin / KylinSoft) ; ils
> sont inclus pour permettre à quiconque de **re-vérifier** les signatures et les
> empreintes, non comme production originale de l'audit. Ils relèvent de la
> catégorie 2 ci-dessous.

Copyright © 2026 les contributeur·rice·s de cet audit.

Ce programme est un logiciel libre : vous pouvez le redistribuer et/ou le
modifier selon les termes de la licence publique générale GNU Affero telle que
publiée par la Free Software Foundation, soit la version 3 de la licence, soit
(à votre choix) toute version ultérieure. Il est distribué dans l'espoir qu'il
sera utile, mais **SANS AUCUNE GARANTIE** ; voir la licence pour les détails.

---

## 2. Code et artefacts tiers — licences d'origine, **non modifiées**

**`SPDX-License-Identifier` : ne s'applique pas uniformément — voir chaque composant.**

Le dossier [`sources-openkylin/`](sources-openkylin/) contient du **code source
tiers** (openKylin, et en amont Debian / Ubuntu / projets libres divers) ainsi que
des **paquets binaires** et des **artefacts de dépôt**. Ce contenu :

- **n'a pas été écrit par l'audit** ;
- est versé **à l'octet près**, sans aucune modification (copie des arbres analysés,
  cf. [`sources-openkylin/LISEZMOI.md`](sources-openkylin/LISEZMOI.md) et
  [`sources-openkylin/EXCLUSIONS.tsv`](sources-openkylin/EXCLUSIONS.tsv)) ;
- **conserve intégralement sa ou ses licences d'origine** : notre choix de l'AGPLv3
  pour notre travail **ne relicencie rien** et **ne peut rien relicencier** ici.

Chaque paquet source porte sa propre licence, généralement dans son fichier
`debian/copyright` et/ou ses en-têtes de fichiers. On y trouve notamment, selon
les composants : **GPL-2.0**, **GPL-3.0**, **LGPL-2.1**, **LGPL-3.0**,
**Apache-2.0**, **MIT**, **BSD** (2 et 3 clauses), **MPL**, **domaine public**,
et, pour certains composants, des **licences propriétaires**.

> **Pourquoi inclure ce code ?** Par exigence de **reproductibilité** : un audit
> neutre doit permettre à un tiers de vérifier chaque constat sur **exactement**
> les mêmes octets que ceux analysés. Les empreintes de tous les arbres figurent
> dans `rapport-audit/donnees/empreintes-arbres-sources.tsv`.

### 2 bis. Binaires propriétaires / sans source

Le sous-dossier `sources-openkylin/binaires-sans-source/` contient des paquets
`.deb` **dépourvus de source exploitable** (squelettes `dh_make`, ou composants
tiers propriétaires : par exemple WPS Office, Sogou, iFlytek, CrossOver, pilotes,
microcodes, modèles d'IA). Ils sont redistribués **tels que publiés** par les
dépôts officiels openKylin, avec leur provenance et leur SHA-256
(`rapport-audit/donnees/binaires-sans-source/provenance.tsv`).

**Ces composants restent soumis au contrat de licence de leur éditeur respectif,
qui peut être restrictif.** Leur présence ici ne vaut **ni** approbation, **ni**
octroi d'un quelconque droit par l'audit : vérifiez la licence applicable avant
tout usage ou redistribution.

---

## 3. En cas de doute

- Un fichier est sous `sources-openkylin/` → **licence d'origine du composant**
  (catégorie 2). Cherchez son `debian/copyright`.
- Un fichier est sous `rapport-audit/` → **AGPL-3.0-or-later** (catégorie 1),
  sauf les artefacts openKylin redistribués signalés plus haut.

Si un fichier ne portait, par erreur, aucune indication claire, la **catégorie 2
prévaut par prudence** pour tout contenu dérivé d'un tiers : dans le doute, on ne
s'attribue jamais le travail d'autrui.
