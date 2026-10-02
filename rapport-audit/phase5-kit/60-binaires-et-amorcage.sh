#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 60 : durcissement des binaires, amorçage, services réseau (LECTURE SEULE)
# =============================================================================
# Hypothèses D17, D18, D20 (voir ../04-protocole-dynamique.md § 2).
# - D17 : les binaires setuid/setgid et les services root INSTALLÉS sont-ils compilés
#         avec les protections standard (PIE, RELRO complet, canari de pile, FORTIFY, CET) ?
# - D18 : version de GRUB installée, présence de shim, état de Secure Boot, entrée de
#         restauration d'usine protégée par mot de passe GRUB ?
# - D20 : le démon dnsmasq est-il actif et en écoute au démarrage ?
# Commandes utilisées : find, stat, file, readelf (si présent), dpkg-query, systemctl
# is-enabled/is-active, ss, lecture de grub.cfg et des variables EFI. Aucun binaire
# observé n'est exécuté ; rien n'est modifié. Lancer en utilisateur STANDARD.
#
# Usage :   bash 60-binaires-et-amorcage.sh
# Résultat : resultats/60-binaires-amorcage.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/60-binaires-amorcage.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — binaires, amorçage, services réseau (lecture seule)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "utilisateur: $(id -un) (uid $(id -u))"
echo

protections () {  # protections <fichier> : lecture seule des en-têtes ELF
  local f="$1" t
  t="$(file -b "$f" 2>/dev/null)"
  case "$t" in *pie*|*"shared object"*) pie=oui ;; *) pie=NON ;; esac
  if command -v readelf >/dev/null 2>&1; then
    now=NON; readelf -W -d "$f" 2>/dev/null | grep -qE 'BIND_NOW|FLAGS_1.*NOW' && now=oui
    ssp=NON; readelf -W --dyn-syms "$f" 2>/dev/null | grep -q '__stack_chk_fail' && ssp=oui
    fort=NON; readelf -W --dyn-syms "$f" 2>/dev/null | grep -qE 'UND +__[a-z0-9_]+_chk(@| |$)' && fort=oui
    cet=NON; readelf -W -n "$f" 2>/dev/null | grep -qE 'x86 feature:.*IBT.*SHSTK' && cet=oui
    printf '  %-40s PIE=%-3s BIND_NOW=%-3s canari=%-3s FORTIFY=%-3s CET=%s\n' "$f" "$pie" "$now" "$ssp" "$fort" "$cet"
  else
    printf '  %-40s PIE=%-3s (readelf absent : installez binutils pour le détail)\n' "$f" "$pie"
  fi
}

echo "## D17-a. Binaires setuid/setgid installés (système de fichiers racine)"
find / -xdev -type f -perm /6000 2>/dev/null | sort | while read -r f; do
  printf '%s  ' "$(stat -c '%A %U:%G' "$f" 2>/dev/null)"; protections "$f"
done
echo

echo "## D17-b. Services root et binaires exposés cités par l'audit"
for f in /usr/sbin/sshd /usr/bin/dbus-daemon /usr/lib/polkit-1/polkitd /usr/bin/pkexec \
         /usr/sbin/NetworkManager /usr/sbin/wpa_supplicant /usr/lib/systemd/systemd /usr/bin/curl \
         /usr/lib/xorg/Xorg /usr/bin/dpkg; do
  [ -e "$f" ] && protections "$f" || echo "  absent: $f"
done
echo

echo "## D18-a. Versions installées (base de données dpkg, lecture seule)"
dpkg-query -W -f='  ${Package} ${Version} ${db:Status-Abbrev}\n' \
  'grub*' shim-signed grub2-signed ostree-extra-utils dpkg-wrapper 2>/dev/null | grep -v '^  .* un *$'
for f in /usr/lib/grub/x86_64-efi/modinfo.sh /usr/lib/grub/i386-pc/modinfo.sh; do
  [ -r "$f" ] && grep -E '^grub_(version|package_version)=' "$f" | sed "s|^|  $f: |"
done
echo

echo "## D18-b. Chargeur EFI : shim présent ?"
if [ -r /boot/efi/EFI ]; then
  find /boot/efi/EFI -maxdepth 2 -type f -iname '*.efi' -printf '  %M %u:%g %p\n' 2>/dev/null
else
  echo "  /boot/efi non lisible par cet utilisateur (normal) : relevé à faire avec 'sudo ls -l /boot/efi/EFI/*/'"
fi
echo

echo "## D18-c. État de Secure Boot (variable EFI publique)"
sb="$(ls /sys/firmware/efi/efivars/SecureBoot-* 2>/dev/null | head -1)"
if [ -n "$sb" ] && [ -r "$sb" ]; then
  v="$(od -An -t u1 "$sb" 2>/dev/null | awk '{print $NF}')"
  echo "  SecureBoot = ${v:-?} (1 = actif, 0 = inactif)"
elif [ -d /sys/firmware/efi ]; then
  echo "  amorçage EFI, variable SecureBoot non lisible"
else
  echo "  amorçage BIOS (pas de /sys/firmware/efi)"
fi
command -v mokutil >/dev/null 2>&1 && mokutil --sb-state 2>/dev/null | sed 's/^/  mokutil: /'
echo

echo "## D18-d. Menu GRUB : entrée de restauration d'usine et mot de passe GRUB"
for f in /boot/grub/grub.cfg /boot/grub2/grub.cfg; do
  [ -r "$f" ] || continue
  echo "  fichier: $f ($(stat -c '%A %U:%G' "$f"))"
  echo "  entrées mentionnant ostree-factory-restore : $(grep -c 'ostree-factory-restore' "$f")"
  echo "  directives superusers/password (nombre, valeurs NON affichées) : $(grep -cE '^\s*(set superusers|password(_pbkdf2)?)' "$f")"
  echo "  entrées --unrestricted : $(grep -c -- '--unrestricted' "$f")"
done
echo

echo "## D18-e. os-prober (lance grub-mount en root sur les autres partitions, même pilote NTFS que GRUB)"
grep -HE '^\s*GRUB_DISABLE_OS_PROBER' /etc/default/grub /etc/default/grub.d/*.cfg 2>/dev/null | sed 's/^/  /' \
  || echo "  GRUB_DISABLE_OS_PROBER non défini (os-prober actif si installé)"
dpkg-query -W -f='  ${Package} ${Version}\n' os-prober 2>/dev/null || echo "  os-prober non installé"
echo

echo "## D20. dnsmasq : activé, actif, en écoute ?"
if command -v systemctl >/dev/null 2>&1; then
  echo "  dnsmasq.service : enabled=$(systemctl is-enabled dnsmasq 2>/dev/null) active=$(systemctl is-active dnsmasq 2>/dev/null)"
fi
if command -v ss >/dev/null 2>&1; then
  echo "  sockets en écoute sur le port 53 (sans nom de processus, non root) :"
  ss -H -lntu 2>/dev/null | awk '$5 ~ /:53$/ {print "    "$1" "$5}'
fi
echo
echo "FIN"
