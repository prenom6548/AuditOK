-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: dbus
Binary: dbus, dbus-session-bus-common, dbus-system-bus-common, dbus-bin, dbus-daemon, dbus-1-doc, dbus-tests, dbus-udeb, dbus-user-session, dbus-x11, libdbus-1-3, libdbus-1-3-udeb, libdbus-1-dev
Architecture: any all
Version: 1.16.2-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://dbus.freedesktop.org/
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/utopia-team/dbus
Vcs-Git: https://salsa.debian.org/utopia-team/dbus.git
Testsuite: autopkgtest
Testsuite-Triggers: apparmor, build-essential, gnome-desktop-testing, init, systemd, xauth, xvfb
Build-Depends: debhelper (>= 13.11.6~), debhelper-compat (= 13), dh-exec, libapparmor-dev [linux-any], libaudit-dev [linux-any], libcap-ng-dev [linux-any], libexpat-dev, libglib2.0-dev <!nocheck> <!noinsttest>, libnss-wrapper <!nocheck>, libselinux1-dev [linux-any], libsystemd-dev [linux-any], libx11-dev, meson, pkgconf, valgrind-if-available, xml-core, xmlto <!nodoc>
Build-Depends-Indep: doxygen <!nodoc>, ducktype <!nodoc>, xsltproc <!nodoc>, yelp-tools <!nodoc>
Package-List:
 dbus deb admin standard arch=any
 dbus-1-doc deb doc optional arch=all profile=!nodoc
 dbus-bin deb admin optional arch=any
 dbus-daemon deb admin optional arch=any
 dbus-session-bus-common deb admin optional arch=all
 dbus-system-bus-common deb admin optional arch=all
 dbus-tests deb misc optional arch=any profile=!noinsttest
 dbus-udeb udeb debian-installer optional arch=any profile=!noudeb
 dbus-user-session deb admin optional arch=linux-any
 dbus-x11 deb x11 optional arch=any
 libdbus-1-3 deb libs optional arch=any
 libdbus-1-3-udeb udeb debian-installer optional arch=any profile=!noudeb
 libdbus-1-dev deb libdevel optional arch=any
Checksums-Sha1:
 7cbee4a1101c1d37652705f80cec2eabc5333065 1110804 dbus_1.16.2.orig.tar.xz
 a28447b5e3aaf4725b44f06cb28eea1f9588beb5 32600 dbus_1.16.2-ok1.debian.tar.xz
Checksums-Sha256:
 f75290ef2261876be81e67adb73cf1af5aa4f7f581676e793010e2fad378e8e2 1110804 dbus_1.16.2.orig.tar.xz
 470ea49b937a4f278eb292c353100abcd189fdd77f7dc2480e5d5ede1b1617d6 32600 dbus_1.16.2-ok1.debian.tar.xz
Files:
 521bb03b64626780bef101374846eec9 1110804 dbus_1.16.2.orig.tar.xz
 e5ed8e0e6dcfad765b0a30ef243e547e 32600 dbus_1.16.2-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnzHdoACgkQBxbwD4Qs
0nQslQwAl1G0So9vogH60rLJuEFzuUA8O8wkndvEU2f/n3373kLsvIjinCSoTgDp
B20hEm3lPfqd7dAXsr9P59g6bn9kFQaSjEH3bhGQwwcDA5lgWv0FLJh4wo04L7xW
WrgscK4Pj5OgqKbXrTlOD6g6aZgmm1LMVdxxjz3l5P1F/hm64gkx7Aks6QZd0Ba3
zg5Sx6v/wD31wCSpkwWNz26TfH1XRXXRxoG9euNNjzvZgxmlg0Igwt8un40Zbc5E
2U7smOe73a614y5ffdI4fZfZHtMC4annDWWdSGwls9pvYYc7jGcW9Ye0ptg6X+K1
kaI0/rwL+/THFaWzdcNrha2jXC9KA90CfYmX/hTJKSjVoA8TdysNdhKk85STi9Ym
8IeOwX+jKEOqx/M2ryyP24XVv+rClYThTYIEaGafQDlXqqMAZj2ptkfQsoajGkUE
QVsvB1Ciq9CUc2XYhXDPam/W+CyRV0NO7xRyRQyFEihllIBXzbXwZrmL1VMdkr0W
kktRTrZ7
=oa9j
-----END PGP SIGNATURE-----
