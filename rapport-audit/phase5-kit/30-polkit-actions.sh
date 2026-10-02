#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 30 : actions polkit et invite d'authentification (LECTURE SEULE)
# =============================================================================
# Objectif : constater la configuration d'autorisation déclarée par polkit pour
# les actions citées — sans exécuter l'action associée.
#   - `pkaction --verbose` affiche la politique (allow_any/allow_active...) : c'est
#     de la lecture de configuration, pas une exécution.
#   - `pkcheck ... --process $$` interroge polkit pour savoir si l'appelant SERAIT
#     autorisé, et demande l'authentification seulement si la politique l'exige.
#     Il NE lance PAS le programme privilégié. Si une invite de mot de passe
#     apparaît, vous pouvez l'ANNULER : l'observation (invite oui/non) est déjà faite.
# Rien n'est modifié. Lancer en utilisateur STANDARD.
#
# Usage :   bash 30-polkit-actions.sh
# Résultat : resultats/30-polkit.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/30-polkit.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — actions polkit (lecture de configuration ; aucune action privilégiée lancée)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "utilisateur: $(id -un) (uid $(id -u))"
echo

ACTIONS="
org.kylin.kaiming.mount
org.kylin.kaiming.umount
org.kylin.kaiming.unsetkysec
org.kylin.kaiming.install
"

echo "## 1. Configuration déclarée de chaque action (pkaction --verbose)"
if command -v pkaction >/dev/null 2>&1; then
  for a in $ACTIONS; do
    echo "----- $a -----"
    pkaction --action-id "$a" --verbose 2>&1 | sed -n '1,40p' || echo "  action absente"
    echo
  done
else
  echo "pkaction indisponible — relevez manuellement /usr/share/polkit-1/actions/org.kylin.kaiming.policy"
fi

echo "## 2. Toutes les actions dont la politique implicite vaut 'yes' (sans authentification)"
# Lecture seule : on liste les actions potentiellement ouvertes sans mot de passe.
if command -v pkaction >/dev/null 2>&1; then
  for a in $(pkaction 2>/dev/null); do
    v="$(pkaction --action-id "$a" --verbose 2>/dev/null | grep -iE 'implicit (any|inactive|active)')"
    if printf '%s' "$v" | grep -qiE ':\s*yes'; then
      echo "ACTION OUVERTE (implicit yes) : $a"
      printf '%s\n' "$v" | sed 's/^/    /'
    fi
  done
else
  echo "n.d."
fi

echo
echo "## 3. L'action demande-t-elle une authentification ? (pkcheck — n'exécute rien)"
# pkcheck interroge polkit pour l'appelant courant. Il NE lance PAS l'action.
# Observable : 'autorisé' sans invite => action ouverte ; invite => authentification requise (annulable).
if command -v pkcheck >/dev/null 2>&1; then
  for a in $ACTIONS; do
    echo "----- $a -----"
    echo "  (si une fenêtre de mot de passe s'ouvre, ANNULEZ-la : la réponse est déjà 'authentification requise')"
    pkcheck --action-id "$a" --process "$$" 2>&1; rc=$?
    echo "  code_retour=$rc  (0 = autorisé sans condition ; non-zéro = refus ou authentification requise)"
    echo
  done
else
  echo "pkcheck indisponible."
fi

echo "RAPPEL : aucune de ces commandes n'exécute mount/umount/setfattr/kaiming."
echo "FIN"
