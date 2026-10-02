#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 10 : permissions de fichiers et de répertoires (LECTURE SEULE)
# =============================================================================
# Relève les permissions (propriétaire, mode) de chemins cités par l'audit, pour
# trancher des hypothèses laissées « à confirmer » : un répertoire est-il
# réellement accessible en écriture à l'utilisateur ? un socket est-il ouvert ?
# Ne lit QUE des métadonnées (stat, ls -l, getfacl). Aucun contenu sensible n'est
# recopié, rien n'est modifié. Lancer en utilisateur STANDARD.
#
# Usage :   bash 10-permissions-fichiers.sh
# Résultat : resultats/10-permissions.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/10-permissions.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — permissions de fichiers (lecture seule)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "utilisateur: $(id -un) (uid $(id -u))"
echo

note () { printf '\n## %s\n' "$1"; }
voir () {  # voir <chemin> : mode, proprietaire, type — jamais le contenu
  for p in "$@"; do
    if [ -e "$p" ] || [ -L "$p" ]; then
      stat -c '%A  %U:%G  %n  (%F)' "$p" 2>/dev/null
    else
      echo "absent: $p"
    fi
  done
}
inscriptible_par_moi () {  # test NON destructif : droit d'écriture théorique
  for p in "$@"; do
    if [ -d "$p" ]; then
      if [ -w "$p" ]; then echo "INSCRIPTIBLE par $(id -un): $p"; else echo "non inscriptible: $p"; fi
    else
      echo "absent ou non-répertoire: $p"
    fi
  done
}

note "Répertoires de règles/actions polkit (hypothèses D1/D2)"
voir /usr/share/polkit-1/actions /etc/polkit-1/rules.d /usr/share/polkit-1/rules.d
voir /opt/kaiming/share/polkit-1/rules.d /var/opt/kaiming/share /opt/system/resource/polkit-1/actions
inscriptible_par_moi /opt/kaiming/share/polkit-1/rules.d /var/opt/kaiming/share /var/log/kaiming

note "Actions polkit de kaiming (présence et emplacement)"
voir /usr/share/polkit-1/actions/org.kylin.kaiming.policy \
     /opt/system/resource/polkit-1/actions/org.kylin.kaiming.policy

note "Sockets des services IA (hypothèse D13 : mode effectif)"
for d in /tmp/.kylin-ai-runtime-unix /tmp/.kylin-ai-business-unix; do
  echo "--- $d ---"; voir "$d"
  find "$d" -maxdepth 2 \( -type s -o -type d \) -printf '%M  %u:%g  %p  (%y)\n' 2>/dev/null
done

note "Magasin de CA additionnel (hypothèse chaîne de confiance)"
voir /opt/system/resource/ca-certificates
find /opt/system/resource/ca-certificates -maxdepth 1 -type f -printf '%M %u:%g %p\n' 2>/dev/null

note "Règle udev du framebuffer et périphériques (hypothèse D12)"
voir /dev/fb0 /dev/fb1
find /lib/udev/rules.d /etc/udev/rules.d -name '*fb*' -o -name '*framebuffer*' 2>/dev/null | while read -r f; do echo "règle: $f"; done

note "Compte test / sudoers (hypothèse D11)"
voir /etc/sudoers.d/test
getent passwd test 2>/dev/null || echo "pas de compte 'test'"

note "Racine de confiance /opt/system (hypothèse D16 : ld.so, Python, PATH, systemd)"
voir /opt /opt/system /opt/system/bin /opt/system/lib /opt/system/lib/shared-libs \
     /opt/system/lib/python3/dist-packages /opt/system/lib/systemd/system /opt/system/lib/systemd/user \
     /opt/system/resource /opt/system/conf /var/opt /var/opt/system
inscriptible_par_moi /var/opt/system /opt/system /opt/system/bin /opt/system/lib/shared-libs \
     /opt/system/lib/python3/dist-packages /opt/system/lib/systemd/system /opt/system/lib/systemd/user
echo "--- fichiers déclarant ces chemins (présence) ---"
voir /etc/ld.so.conf.d/zzz-ostree.conf /usr/lib/python3/dist-packages/opt_path.pth \
     /etc/ostree/ostree.conf.d/99-ostree.conf /etc/systemd/system-environment-generators/99-ostree
echo "--- éléments inscriptibles par d'autres que root sous /opt/system (2 niveaux) ---"
find /opt/system -maxdepth 2 \( -perm -0002 -o -perm -0020 \) -printf '%M %u:%g %p\n' 2>/dev/null | head -50
find /opt/system -maxdepth 2 ! -user root -printf 'non-root: %M %u:%g %p\n' 2>/dev/null | head -50
echo "--- journal du crochet dpkg (contient les lignes de commande dpkg) ---"
voir /var/log/ostree.log

note "aptdaemon : méthode SetEnviron encore présente ? (hypothèse D19, lecture du fichier installé)"
f=/usr/lib/python3/dist-packages/aptdaemon/core.py
if [ -r "$f" ]; then
  n="$(grep -c 'def SetEnviron' "$f")"
  echo "$f : occurrences de 'def SetEnviron' = $n (attendu : 0 sur openKylin 3.0)"
else
  echo "aptdaemon non installé ($f absent)"
fi

echo
echo "FIN"
