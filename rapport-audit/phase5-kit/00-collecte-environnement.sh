#!/usr/bin/env bash
# =============================================================================
# Phase 5 — Observation 00 : collecte de l'environnement (LECTURE SEULE)
# =============================================================================
# Ce script ne fait que LIRE l'état du système et écrire un fichier de résultats.
# Il ne modifie rien, n'installe rien, n'exécute aucun composant audité.
# À lancer DANS une machine virtuelle openKylin 3.0 jetable, en tant
# qu'utilisateur STANDARD (non root). Voir LISEZMOI.md.
#
# Usage :   bash 00-collecte-environnement.sh
# Résultat : resultats/00-environnement.txt
# =============================================================================
set -u
OUT="$(dirname "$0")/resultats/00-environnement.txt"
mkdir -p "$(dirname "$OUT")"
exec > "$OUT" 2>&1

echo "### Phase 5 — environnement (lecture seule)"
echo "date_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null)"
echo "utilisateur: $(id 2>/dev/null)"
echo

echo "## Version du système"
cat /etc/os-release 2>/dev/null
echo "--- /etc/.kyinfo (présence seulement, contenu non recopié) ---"
ls -l /etc/.kyinfo 2>/dev/null || echo "absent"
echo

echo "## Noyau"
uname -a 2>/dev/null
echo "--- options de durcissement du noyau installé (/boot/config-*) ---"
cfg="/boot/config-$(uname -r)"
if [ -r "$cfg" ]; then
  grep -E '^(CONFIG_SECURITY_LOCKDOWN_LSM|CONFIG_LOCK_DOWN_KERNEL_FORCE|CONFIG_MODULE_SIG|CONFIG_MODULE_SIG_FORCE|CONFIG_IMA|CONFIG_EVM|CONFIG_BPF_UNPRIV_DEFAULT_OFF|CONFIG_STACKPROTECTOR_STRONG|CONFIG_FORTIFY_SOURCE|CONFIG_RANDOMIZE_BASE|CONFIG_STRICT_DEVMEM|CONFIG_DEVMEM|CONFIG_LEGACY_TIOCSTI|CONFIG_HARDENED_USERCOPY|CONFIG_INIT_ON_ALLOC_DEFAULT_ON|CONFIG_LSM)=' "$cfg" 2>/dev/null | sort
else
  echo "config noyau illisible : $cfg"
fi
echo "--- LSM actifs ---"
cat /sys/kernel/security/lsm 2>/dev/null || echo "n.d."
echo "--- verrou (lockdown) ---"
cat /sys/kernel/security/lockdown 2>/dev/null || echo "n.d."
echo

echo "## Module de sécurité kysec (présence seulement)"
command -v kysec-status kysec_set setstatus getstatus 2>/dev/null || echo "aucun outil kysec dans le PATH"
echo

echo "## Services système en cours (racine root)"
systemctl list-units --type=service --state=running --no-pager --no-legend 2>/dev/null \
  | awk '{print $1}' | sort || echo "n.d."
echo

echo "## Résumé"
echo "Fichier produit par 00-collecte-environnement.sh — à joindre aux autres résultats."
echo "FIN"
