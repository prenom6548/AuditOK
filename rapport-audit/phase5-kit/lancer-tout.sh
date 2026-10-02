#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Lanceur : exécute les observations qui NE demandent PAS root.
# =============================================================================
# Enchaîne 00, 10, 20, 30, 50, 60 en utilisateur standard. Toutes sont en LECTURE SEULE.
# L'observation réseau (40) demande root et une VM isolée : elle est lancée à part,
# volontairement, pour que vous gardiez la main (voir LISEZMOI.md, étape 5).
#
# Usage :   bash lancer-tout.sh
# Résultats : dossier resultats/  +  une archive resultats-phase5.tar.gz à renvoyer.
# =============================================================================
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR" || exit 1

if [ "$(id -u)" = "0" ]; then
  echo "Veuillez lancer ce script en UTILISATEUR STANDARD (pas en root, pas avec sudo)."
  echo "C'est voulu : on observe ce qu'un utilisateur ordinaire peut voir et faire."
  exit 1
fi

echo "Phase 5 — observations en lecture seule (aucune modification du système)."
for s in 00-collecte-environnement.sh 10-permissions-fichiers.sh 20-introspection-dbus.sh 30-polkit-actions.sh 50-depots-et-maj.sh 60-binaires-et-amorcage.sh; do
  echo "  -> $s"
  bash "$s"
done

tar -czf "$DIR/resultats-phase5.tar.gz" -C "$DIR" resultats 2>/dev/null
echo
echo "Terminé. Résultats dans : $DIR/resultats/"
echo "Archive à renvoyer       : $DIR/resultats-phase5.tar.gz"
echo "(Étape facultative, root + VM isolée : sudo bash 40-observation-reseau.sh)"
