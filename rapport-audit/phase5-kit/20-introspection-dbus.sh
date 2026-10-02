#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 20 : introspection des services D-Bus système (LECTURE SEULE)
# =============================================================================
# Objectif : vérifier, sur un système réel, quelles méthodes les services root
# exposent et sous quelle politique d'accès — sans APPELER aucune méthode.
# L'introspection liste les interfaces ; elle ne déclenche aucune action.
# Le script lit aussi les fichiers de politique .conf (publics) pour constater
# la valeur `context` effective. Rien n'est modifié. Lancer en utilisateur STANDARD.
#
# Usage :   bash 20-introspection-dbus.sh
# Résultat : resultats/20-dbus.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/20-dbus.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — introspection D-Bus système (lecture seule, aucun appel de méthode)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "utilisateur: $(id -un) (uid $(id -u))"
echo

# Services root cités par l'audit (noms de bus).
SERVICES="
com.kylin.ProcessManagerDaemon
org.ukui.Service
org.ukui.ServiceManager
com.kylin.systemupgrade
com.kylin.UpgradeStrategies
org.ukui.Biometric
com.KylinOsManager
com.kylin.kysdk.conf2
com.kysdk.location
com.kylin.ukui.SettingsDaemon
org.kaiming.proxy
"

echo "## 1. Noms de bus présents sur le bus SYSTÈME"
if command -v busctl >/dev/null 2>&1; then
  have="$(busctl --system list --no-pager --no-legend 2>/dev/null | awk '{print $1}')"
  for s in $SERVICES; do
    if printf '%s\n' "$have" | grep -qx "$s"; then echo "présent : $s"; else echo "absent  : $s"; fi
  done
else
  echo "busctl indisponible"
fi

echo
echo "## 2. Politiques D-Bus installées (valeur 'context', lecture des .conf)"
# Les fichiers de politique sont publics ; on en extrait seulement le contexte d'accès.
for d in /usr/share/dbus-1/system.d /etc/dbus-1/system.d; do
  [ -d "$d" ] || continue
  grep -lEi 'kylin|ukui|kaiming|kysdk|biometric' "$d"/*.conf 2>/dev/null | while read -r f; do
    ctx="$(grep -oE 'context="[a-z]+"' "$f" 2>/dev/null | sort -u | tr '\n' ' ')"
    echo "$(basename "$f") : context= ${ctx:-aucun}"
  done
done

echo
echo "## 3. Introspection des interfaces exposées (liste des méthodes, SANS appel)"
# 'busctl introspect' ne fait que lire la description d'interface ; il n'invoque rien.
if command -v busctl >/dev/null 2>&1; then
  for s in $SERVICES; do
    if printf '%s\n' "$have" | grep -qx "$s" 2>/dev/null; then
      echo "----- $s -----"
      # On tente d'introspecter la racine ; en cas d'échec, on ne force rien.
      busctl --system introspect "$s" / --no-pager 2>/dev/null \
        | awk '$2=="method"{print "  méthode: "$1" "$3}' | head -200
      echo "  (si vide : objet racine non introspectable sans chemin précis — voir la fiche du lot)"
    fi
  done
else
  echo "busctl indisponible : installez systemd/dbus ou relevez les .conf manuellement."
fi

echo
echo "RAPPEL : ce script n'appelle AUCUNE méthode. Pour trancher l'exploitabilité réelle,"
echo "l'étape suivante (hors de ce kit) consisterait à tester un appel INOFFENSIF en lecture"
echo "depuis un compte non privilégié — à ne faire que dans la VM jetable et à documenter."
echo "FIN"
