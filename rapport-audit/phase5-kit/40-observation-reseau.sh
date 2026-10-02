#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 40 : flux réseau sortants (OBSERVATION, pas d'attaque)
# =============================================================================
# Objectif : voir ce que le système tente d'envoyer (télémétrie), vers où, et si
# le certificat TLS est vérifié — en OBSERVANT le trafic, sans l'envoyer vers les
# vrais serveurs. On capture le trafic et on résout tous les noms vers une adresse
# locale inerte (pot de miel), de sorte qu'aucune donnée réelle ne quitte la VM.
#
# PRÉREQUIS dans la VM (à installer AVANT) : tcpdump.  (Facultatif : un petit
# serveur local pour voir le contenu — non inclus ici pour rester simple/neutre.)
#
# CE SCRIPT A BESOIN DES DROITS root DANS LA VM pour capturer (tcpdump).
# C'est de l'OBSERVATION du trafic de la VM, pas une attaque d'un tiers.
#
# Usage :   sudo bash 40-observation-reseau.sh [duree_secondes]
# Résultat : resultats/40-reseau.txt  +  resultats/40-capture.pcap
# =============================================================================
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
OUT="$DIR/resultats/40-reseau.txt"
PCAP="$DIR/resultats/40-capture.pcap"
DUREE="${1:-120}"
mkdir -p "$DIR/resultats"
exec > "$OUT" 2>&1

echo "### Phase 5 — observation des flux réseau (la VM doit être ISOLÉE du vrai réseau)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "duree_capture_s: $DUREE"
echo

echo "## AVERTISSEMENT"
echo "Ce script suppose que la VM est en réseau isolé (hôte-seul / NAT sans route utile)."
echo "Ne le lancez PAS sur une VM connectée à un réseau de production."
echo

echo "## 1. Destinations réseau codées en dur citées par l'audit (pour mémoire)"
cat <<'EOF'
  datareporting.openkylin.top    (kylin-daq, télémétrie — TLS non vérifié constaté dans le code)
  archive1.kylinos.cn            (kylin-system-updater DataAcquisition — HTTP clair constaté)
  kom.kylinos.cn                 (kylin-os-manager service-support — TLS non vérifié constaté)
  api.kylinos.cn                 (driver-acquirer)
  api.map.baidu.com              (libkysdk-location — HTTP clair constaté)
EOF
echo

echo "## 2. Connexions réseau établies au moment du relevé (ss, lecture seule)"
ss -tunp 2>/dev/null | head -100 || echo "ss indisponible"
echo

echo "## 3. Résolution DNS de ces noms (pour savoir vers quelle IP la VM pointerait)"
for h in datareporting.openkylin.top archive1.kylinos.cn kom.kylinos.cn api.kylinos.cn api.map.baidu.com; do
  ip="$(getent hosts "$h" 2>/dev/null | awk '{print $1}' | tr '\n' ' ')"
  echo "  $h -> ${ip:-non résolu}"
done
echo

echo "## 4. Capture du trafic pendant ${DUREE}s (métadonnées ; contenu TLS non déchiffré ici)"
if command -v tcpdump >/dev/null 2>&1; then
  echo "capture en cours vers $PCAP ..."
  timeout "$DUREE" tcpdump -n -w "$PCAP" 'tcp and (port 80 or port 443 or port 30005 or port 32294)' 2>/dev/null
  echo "capture terminée."
  echo "--- résumé des IP/ports contactés ---"
  tcpdump -n -r "$PCAP" 2>/dev/null | awk '{print $3" -> "$5}' | sed 's/:[0-9]*$//' | sort | uniq -c | sort -rn | head -40
else
  echo "tcpdump absent : installez-le dans la VM (sudo apt install tcpdump) puis relancez."
fi
echo
echo "NOTE : pour VOIR le contenu envoyé malgré TLS et confirmer l'absence de vérification du"
echo "certificat, il faut un mandataire d'inspection (ex. mitmproxy) configuré en pot de miel."
echo "Ce kit se limite à la capture de métadonnées pour rester simple et non intrusif."
echo "FIN"
