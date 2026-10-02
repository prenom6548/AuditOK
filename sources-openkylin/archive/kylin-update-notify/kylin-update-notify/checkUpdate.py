#!/usr/bin/python3
import apt
import subprocess

def calculate_upgradable_pkgs():
    cache = apt.Cache()
    pkgs_to_upgrade = []

    for pkg in cache:
        if pkg.is_installed and pkg.is_upgradable :
            pkgs_to_upgrade.append(pkg.name)

    if cache.get_changes():
        cache.clear()

    if len(pkgs_to_upgrade) != 0:
        subprocess.Popen('dbus-send --system --type=signal / com.kylin.update.notification.DownloadFinish', shell=True)

if __name__ == "__main__":
    calculate_upgradable_pkgs()
