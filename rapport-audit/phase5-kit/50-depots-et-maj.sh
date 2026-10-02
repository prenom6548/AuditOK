#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 50 : dépôts APT et remotes OSTree (LECTURE SEULE)
# =============================================================================
# Vérifie la configuration réelle de la chaîne de mise à jour : expiration des
# index APT (Valid-Until), restriction de clé par dépôt (Signed-By), transport
# (HTTP/HTTPS), et vérification de signature des remotes OSTree (gpg-verify).
# Ne lit que des fichiers de configuration publics. Rien n'est modifié, aucune
# mise à jour n'est lancée. Lancer en utilisateur STANDARD (sudo seulement si
# certains fichiers ne sont pas lisibles autrement).
#
# Usage :   bash 50-depots-et-maj.sh
# Résultat : resultats/50-depots.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/50-depots.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — dépôts APT et remotes OSTree (lecture seule)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo

echo "## 1. Sources APT configurées (transport http/https)"
for f in /etc/apt/sources.list /etc/apt/sources.list.d/*.list /etc/apt/sources.list.d/*.sources; do
  [ -r "$f" ] || continue
  echo "----- $f -----"
  grep -vE '^\s*#' "$f" 2>/dev/null | grep -vE '^\s*$'
done
echo

echo "## 2. Restriction de clé par dépôt (Signed-By) présente ?"
grep -rniE 'signed-by' /etc/apt/sources.list /etc/apt/sources.list.d/ 2>/dev/null \
  || echo "aucune directive Signed-By trouvée (toutes les clés de trusted.gpg.d font foi pour tous les dépôts)"
echo

echo "## 3. Expiration des index (Valid-Until) dans les InRelease en cache"
found=0
for f in /var/lib/apt/lists/*InRelease; do
  [ -r "$f" ] || continue
  found=1
  vu="$(grep -m1 '^Valid-Until:' "$f" 2>/dev/null)"
  echo "$(basename "$f") : ${vu:-PAS de Valid-Until}"
done
[ "$found" = 0 ] && echo "aucun InRelease en cache (lancez d'abord 'sudo apt update' une fois dans la VM, puis relancez)"
echo

echo "## 4. Options APT livrées (force-confnew, AllowInsecure, hooks Pre/Post-Invoke)"
grep -rniE 'force-confnew|AllowInsecureRepositories|AllowDowngradeToInsecureRepositories|Pre-Invoke|Post-Invoke|Dl-Limit' \
  /etc/apt/apt.conf /etc/apt/apt.conf.d/ 2>/dev/null | sed 's/^/  /' || echo "  aucune de ces options"
echo

echo "## 5. Remotes OSTree (transport et gpg-verify) — système immuable"
if [ -d /etc/ostree/remotes.d ]; then
  for f in /etc/ostree/remotes.d/*.conf; do
    [ -r "$f" ] || continue
    echo "----- $f -----"
    grep -iE 'url=|gpg-verify|contenturl=' "$f" 2>/dev/null | sed 's/^/  /'
  done
else
  echo "pas de /etc/ostree/remotes.d (système non-OSTree, ou variante classique)"
fi
command -v ostree >/dev/null 2>&1 && { echo "--- ostree remote list ---"; ostree remote list -v 2>/dev/null | head -40; }
echo
echo "## 6. Crochets et détournements de dpkg/apt (hypothèse D21)"
for f in /etc/dpkg/dpkg.cfg.d/*; do
  [ -r "$f" ] || continue
  grep -HnE '^\s*(pre-invoke|post-invoke|no-debsig|force-)' "$f" 2>/dev/null | sed 's/^/  /'
done
echo "--- détournements (dpkg-divert --list, lecture seule) ---"
command -v dpkg-divert >/dev/null 2>&1 && dpkg-divert --list 2>/dev/null | grep -E '/usr/bin/(dpkg|apt|apt-get)( |$)' | sed 's/^/  /' \
  || echo "  aucun détournement de dpkg/apt/apt-get"
ls -l /usr/bin/dpkg /usr/bin/apt /usr/bin/apt-get 2>/dev/null | sed 's/^/  /'
echo

echo "## 7. Versions installées des paquets en retard de correctifs selon l'audit (D21)"
dpkg-query -W -f='  ${Package} ${Version}\n' libc6 network-manager apport python3-apport aptdaemon \
  grub-common grub-efi-amd64-bin dpkg passwd 2>/dev/null
echo "  (à comparer aux versions citées dans ../02-findings.csv : glibc 2.43-ok2, network-manager 1.54.3-ok5, grub2 2.12-ok1.21…)"
echo
echo "FIN"
