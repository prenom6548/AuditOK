-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: network-manager
Binary: network-manager, network-manager-dev, network-manager-l10n, libnm0, libnm-dev, gir1.2-nm-1.0, network-manager-config-connectivity-debian, network-manager-config-connectivity-ubuntu
Architecture: linux-any all
Version: 1.54.3-ok5
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Michael Biebl <biebl@debian.org>
Homepage: https://networkmanager.dev/
Standards-Version: 4.7.3
Vcs-Browser: https://git.launchpad.net/network-manager
Vcs-Git: https://git.launchpad.net/network-manager
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, dhcpcd-base, dnsmasq-base, easy-rsa, hostapd, iw, linux-headers-generic, netplan.io, network-manager-openvpn, openvpn, polkitd, python3, python3-dbusmock, python3-gi, python3-netaddr, rfkill, ubuntu-settings, udev, urfkill, wpasupplicant
Build-Depends: debhelper-compat (= 13), debhelper (>= 13.11.6), dh-sequence-gir, meson (>= 0.51.0), pkgconf, gettext (>= 0.19.8), libglib2.0-dev (>= 2.42), ppp-dev, libselinux1-dev, libaudit-dev, libgnutls28-dev (>= 2.12), uuid-dev, systemd-dev, libsystemd-dev (>= 209), libudev-dev (>= 175), gir1.2-gio-2.0-dev, gir1.2-girepository-2.0, gir1.2-girepository-2.0-dev, gobject-introspection, python3-gi, libpsl-dev (>= 0.1), libcurl4-gnutls-dev (>= 7.24.0), gtk-doc-tools, libglib2.0-doc, libmm-glib-dev (>= 0.7.991), libndp-dev, libreadline-dev, libnewt-dev (>= 0.52.15), libteam-dev (>= 1.9), libjansson-dev (>= 2.7), libbluetooth-dev (>= 5), libdbus-1-dev, libpolkit-gobject-1-dev, mobile-broadband-provider-info, valac, dbus-daemon <!nocheck>, python3-dbus <!nocheck>, python3-pexpect <!nocheck>, iproute2 <!nocheck>, libyaml-dev [!i386], libnetplan-dev (>= 1.2~) [!i386], netplan.io (>= 1.2~) [!i386] <!nocheck>
Package-List:
 gir1.2-nm-1.0 deb introspection unknown arch=linux-any
 libnm-dev deb libdevel unknown arch=linux-any
 libnm0 deb libs unknown arch=linux-any
 network-manager deb net unknown arch=linux-any
 network-manager-config-connectivity-debian deb net unknown arch=all
 network-manager-config-connectivity-ubuntu deb net unknown arch=all
 network-manager-dev deb devel unknown arch=all
 network-manager-l10n deb localization unknown arch=all
Checksums-Sha1:
 48d5e77733158accbd7b3e8b94b4b5d1fff0653c 17183884 network-manager_1.54.3.orig.tar.gz
 5945d002fa427890f4efaf8f303e09993d98a755 231692 network-manager_1.54.3-ok5.debian.tar.xz
Checksums-Sha256:
 4a4939326a3300008eb4dd6816805ac9d041969e4372cc4e986b7cdf14530b55 17183884 network-manager_1.54.3.orig.tar.gz
 5663a920ae79c95e891dceb19723cf711f5808e4510467e2a29eec8b55d3c5e6 231692 network-manager_1.54.3-ok5.debian.tar.xz
Files:
 dcb0475644a49f2c0e8e63df01ea4aa7 17183884 network-manager_1.54.3.orig.tar.gz
 39a6ee4949c74fa51c61c6b305767d78 231692 network-manager_1.54.3-ok5.debian.tar.xz
Debian-Vcs-Browser: https://salsa.debian.org/utopia-team/network-manager
Debian-Vcs-Git: https://salsa.debian.org/utopia-team/network-manager.git
Original-Maintainer: Utopia Maintenance Team <pkg-utopia-maintainers@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqqPeoACgkQBxbwD4Qs
0nTQ1Av/QsDPVvRtFVOGj03ADjMFDISBcHP9EI1WOlg/ruxrPl5WXBtEe7FyTF+T
RsslqVMINC4WwCkqJCxYfVtu1tf11RuPPzUHr27U6HYEo+fiGF+FnK2EbeBifNhi
/eBMKt8BApmTr9/oz/4GxJLXKTpvrRTIBMeeCgoqm+tR761UtJik1UaGtJ/Oe6s8
Ri/Uh1WcZyXKPtQa09MIWHUvjjLyVEOeJgBjb+FkaHX/x1fiG6EfQATbzKejnXlP
hwstpkIEr1WMrl8nUCTdsslZCHnc/90ruw5rFTEoVSf0+joCRZoiOpI4k4uuSp8p
yQZt9aejnYAkPlJAfqZCzIlS9J+ZYgF/XASmu15yVpTuBDGeCfRTU/Y+h2CepQnX
/0HKP0NRV7GwyOqaegNWnbmGEdPvRiC5V2GblREebBxK+gJ+d6ddDoMKmpPmFo3T
a5nym5Br4rgFRZGY9/73tqGGM8w6pdfJKR3BJM6CI+8GQN5jgUZHUJ99ENkzNAvH
rAdy7xIr
=LsN5
-----END PGP SIGNATURE-----
