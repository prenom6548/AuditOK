# Kit d'observation — Phase 5

🇫🇷 Français (cette page) · 🇬🇧 [English version](LISEZMOI.en.md)

Ce kit permet à **quiconque dispose d'une machine virtuelle openKylin 3.0** de vérifier sur un système
réel une partie des hypothèses que l'audit n'a pu établir que **dans le code**. Il complète
[`../04-protocole-dynamique.md`](../04-protocole-dynamique.md), qui en décrit la méthode générale.

> **Ce que fait ce kit : il observe et il lit.** Il relève des permissions de fichiers, la configuration
> d'autorisation déclarée par polkit et par D-Bus, la configuration des dépôts de mise à jour, et les flux
> réseau sortants. **Il n'exécute aucun composant audité, n'appelle aucune méthode privilégiée, et ne
> contient aucun code d'exploitation.** Il ne modifie pas le système.

## Ce kit est la Phase 5 de l'audit

L'audit ne dispose pas de machine virtuelle : sa Phase 5 (validation sur un système réel) est donc
**livrée sous la forme de ce kit**, que la communauté exécute. Il est **mis à jour à chaque vague** de
l'analyse statique : toute nouvelle hypothèse observable sans risque y reçoit un contrôle, et l'historique
ci-dessous indique ce qui a changé.

| Version | Date | Contenu |
|---|---|---|
| 1 | 2026-10-01 | Hypothèses D1 à D15 (Phases 1 et 2, vagues 1-2) |
| 2 | 2026-10-01 | Hypothèses D16 à D21 (Phase 2, fin) : racine de confiance `/opt/system`, durcissement des binaires installés, GRUB et Secure Boot, `SetEnviron` d'aptdaemon, `dnsmasq`, crochets et détournements de dpkg ; nouveau script 60 |

## Pour qui ?

Pour un membre de la communauté qui utilise déjà, ou sait installer, openKylin dans une machine virtuelle.
Aucune expertise en sécurité n'est requise pour **lancer** le kit ; en revanche, **interpréter** les
résultats demande un peu d'habitude — d'où la possibilité de les renvoyer (§ *Et ensuite*).

## Règle d'or : toujours dans une VM jetable

- Lancez ce kit **uniquement dans une machine virtuelle** dédiée, jamais sur votre ordinateur principal.
- Prenez un **instantané (snapshot)** de la VM avant de commencer : vous pourrez revenir en arrière.
- Pour l'observation réseau (script 40), mettez la VM en **réseau isolé** (« hôte seul » ou NAT sans
  accès utile), afin qu'aucune donnée ne parte vers de vrais serveurs.

Sur Linux, **GNOME Boxes** ou **Virtual Machine Manager** (QEMU/KVM, logiciels libres) suffisent. Le
disque d'installation officiel d'openKylin 3.0 se télécharge depuis les miroirs du projet ; vérifiez son
empreinte avant installation.

## Les scripts

| Script | Observe | Droits |
|---|---|---|
| `00-collecte-environnement.sh` | Version du système, options de durcissement du noyau, LSM actifs, services en cours | utilisateur |
| `10-permissions-fichiers.sh` | Permissions de répertoires et de sockets cités par l'audit (inscriptibles ? ouverts ?), dont `/opt/system` ; présence de `SetEnviron` dans aptdaemon | utilisateur |
| `20-introspection-dbus.sh` | Services D-Bus présents, valeur `context` de leurs politiques, méthodes exposées (sans les appeler) | utilisateur |
| `30-polkit-actions.sh` | Politique d'autorisation déclarée des actions kaiming ; une authentification est-elle demandée ? | utilisateur |
| `50-depots-et-maj.sh` | Transport des dépôts, `Signed-By`, `Valid-Until`, options APT, `gpg-verify` des remotes OSTree, crochets et détournements de dpkg/apt, versions installées | utilisateur |
| `60-binaires-et-amorcage.sh` | Protections des binaires setuid et des services root (PIE, RELRO, canari, FORTIFY, CET), version de GRUB, shim, Secure Boot, mot de passe GRUB, `dnsmasq` | utilisateur |
| `40-observation-reseau.sh` | Flux sortants (télémétrie) : destinations, ports, volume | **root**, VM isolée |
| `lancer-tout.sh` | Lance d'un coup tous les scripts « utilisateur » et prépare une archive | utilisateur |

## Mode d'emploi (5 étapes)

1. **Installez openKylin 3.0** dans une VM, configuration par défaut. Créez un **compte utilisateur
   ordinaire** (sans droits d'administration) pour les observations.
2. **Prenez un instantané** de la VM.
3. **Copiez ce dossier** `phase5-kit/` dans la VM.
4. Ouvrez un terminal **en utilisateur ordinaire** dans ce dossier et lancez `bash lancer-tout.sh`.
   Si une fenêtre de mot de passe s'ouvre pendant le script 30, **annulez-la** : le simple fait qu'elle
   s'ouvre est déjà le résultat recherché (« authentification requise »).
5. *(Facultatif)* Pour la télémétrie, mettez la VM en **réseau isolé**, installez `tcpdump`, puis lancez
   `sudo bash 40-observation-reseau.sh 120`. Pendant la capture, utilisez normalement le système (panneau
   de configuration, mise à jour) pour déclencher d'éventuels envois.

Les résultats sont écrits dans `resultats/`, et `lancer-tout.sh` en prépare une archive
`resultats-phase5.tar.gz`.

## Ce que chaque résultat permet de trancher

Les hypothèses renvoient à la matrice de [`../04-protocole-dynamique.md`](../04-protocole-dynamique.md) § 2.

| Résultat | Hypothèses | Ce qui est tranché |
|---|---|---|
| `00-environnement.txt` | D15 | Durcissement réel du noyau installé ; présence de `kysec` |
| `10-permissions.txt` | D2, D12, D13, D16, D19 | Répertoires `rules.d` de kaiming réellement inscriptibles ? mode des sockets IA ? `/dev/fb*` en 0666 ? compte `test` ? |
| `20-dbus.txt` | D3, D14 | Les services root sont-ils réellement en `context=default` ? quelles méthodes sont exposées ? |
| `30-polkit.txt` | D1 | Les actions kaiming s'exécutent-elles sans invite d'authentification ? |
| `50-depots.txt` | D10, D21 | Dépôts APT sans `Valid-Until`/`Signed-By` ? remotes OSTree en `gpg-verify=false` ? dpkg/apt détournés ? versions en retard de correctifs ? |
| `60-binaires-amorcage.txt` | D17, D18, D20 | Binaires setuid et services root sans protections ? GRUB 2.12~rc1 sans shim ? Secure Boot actif ? `dnsmasq` en écoute ? |
| `40-reseau.txt` | D6, D8, D9 | Quelles destinations la télémétrie contacte-t-elle, et par quel transport ? |

Chaque hypothèse devient alors **CONFIRMÉE**, **INFIRMÉE** ou **INDÉTERMINÉE** sur un système réel, à
reporter dans `../02-findings.csv` (colonne de vérification) et `../07-verification.md`.

## Ce que ce kit ne fait pas

- Il ne confirme pas l'**exploitation** de bout en bout : il établit les **préconditions** observables
  (absence de contrôle, permissions ouvertes, transport non vérifié). C'est volontaire et suffisant pour
  qualifier le risque, sans produire d'outil offensif.
- Il ne couvre pas tout : l'installateur et le compositeur (hypothèses D11, D12) demandent des essais
  manuels décrits dans le protocole, non scriptés ici.

## Et ensuite ?

Déposez l'archive `resultats-phase5.tar.gz` là où la communauté pourra la reprendre (pièce jointe d'un
ticket, ou dossier `resultats/` du dépôt). Les sorties ne contiennent que des métadonnées de
configuration ; **relisez-les** avant publication pour vous assurer qu'aucune information personnelle de
votre VM ne s'y trouve. Toute personne à l'aise avec ces sujets pourra alors convertir ces observations en
verdicts dans le rapport.
