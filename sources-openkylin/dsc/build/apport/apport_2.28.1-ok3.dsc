-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: apport
Binary: apport, python3-problem-report, python3-apport, apport-core-dump-handler, apport-retrace, apport-valgrind, apport-gtk, apport-kde, dh-apport, apport-noui
Architecture: all
Version: 2.28.1-ok3
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://wiki.ubuntu.com/Apport
Standards-Version: 4.6.2
Vcs-Browser: https://code.launchpad.net/~ubuntu-core-dev/ubuntu/+source/apport/+git/apport
Vcs-Git: https://git.launchpad.net/~ubuntu-core-dev/ubuntu/+source/apport
Testsuite: autopkgtest
Testsuite-Triggers: dbus, dbus-x11, default-jdk-headless, dictionaries-common, dirmngr, gcc, gnome-icon-theme, gpg, gvfs-daemons, iputils-ping, java-sdk-headless, kmod, libc6-dev, psmisc, python3-dbus, python3-psutil, python3-pytest, python3-systemd, python3-zstandard, ubuntu-dbgsym-keyring, ubuntu-keyring, xvfb
Build-Depends: debhelper-compat (= 13), dh-translations, gdb, gir1.2-glib-2.0 (>= 1.29.17), pkgconf, python3-all, python3-gi
Build-Depends-Indep: default-jdk-headless | java-sdk-headless, dh-python, gir1.2-gtk-3.0 (>= 3.1.90), gir1.2-wnck-3.0, intltool, iputils-ping, kmod, pylint, python3-apt (>= 0.7.9), python3-dbus, python3-distutils-extra (>= 2.24~), python3-launchpadlib, python3-psutil, python3-pytest, python3-requests, python3-rpm, python3-systemd, python3-yaml, python3-zstandard, valgrind
Package-List:
 apport deb utils optional arch=all
 apport-core-dump-handler deb utils optional arch=all
 apport-gtk deb gnome optional arch=all
 apport-kde deb kde optional arch=all
 apport-noui deb utils optional arch=all
 apport-retrace deb devel optional arch=all
 apport-valgrind deb devel optional arch=all
 dh-apport deb devel optional arch=all
 python3-apport deb python optional arch=all
 python3-problem-report deb python optional arch=all
Checksums-Sha1:
 463d838470b27fc561e1095a79c3ab97259aa5cc 640692 apport_2.28.1.orig.tar.xz
 5222f7b1ed976b69c6ae6c0138ecd44cbff48783 833 apport_2.28.1.orig.tar.xz.asc
 e3668908d54a4e4201ce1731767adadaefc87ace 30820 apport_2.28.1-ok3.debian.tar.xz
Checksums-Sha256:
 b9a834451c835e965660bc20bb4c70368ece37b945a35566fc4b03a5bae31287 640692 apport_2.28.1.orig.tar.xz
 68605f2b6fdbce3fcc3bb78de50591112e29cbaa1f69f175e66a1d0f8c6ff09e 833 apport_2.28.1.orig.tar.xz.asc
 f791e51a3cac37998dbc04fb6a434c732f806621ec016a39ad3c739deb2d4386 30820 apport_2.28.1-ok3.debian.tar.xz
Files:
 eeff37d2dd222f12bb1c6526aeafcafe 640692 apport_2.28.1.orig.tar.xz
 94e0c4f1c4086374a090c40802dbc776 833 apport_2.28.1.orig.tar.xz.asc
 20d8ed5b2aed8847d854d7a3f5407416 30820 apport_2.28.1-ok3.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmZn8KUACgkQBxbwD4Qs
0nSuZQv+NiUS6DlsDl6OT73KpFyoQ5FEoa0NLbn9iyIOcV/dIVG3Bt8qyAQHQRqY
QIE4tl/1wY/GfWNRi8O1hYZYPJr5y6Yqz9lj8Cp0y5Pe5LOFIakLL8VO36iTKzzT
gmvU6t59LT4bCM1skgZtyq32B7tFE/yd/sQHASZWVEeALIsZ1vTQSm3pd3wBzp6L
egtVW7GeGGADbdszc89s0Q/98B3X+OpwxAUOifzCIPQWZCiyQrwC0Kj+iSD6V7WS
HRBfSxpLsoYgw3pkNHQWVEt0kqkNx+I6viPG3JDeHXwpf15HxIb9a1tCgG8YXVRt
xZIqimfOdnK20ASdzfLr9Xp3MHVoKFkT+gT5VoWlUZAw1VdrnZNRmF5LScIRJ2EA
YFnFLzmk9LdxYYQ3ql8DBPfwgWJd0VK0/ONRXtuASbkuJkxwsFyUWSduPC2KECAt
JznipAB+4UU56LyeXc3jIyOdK5NcTh8+zvH5z75hIVUH7KxBvx8liT1rNkGcGI7k
EBadeX2t
=xmvK
-----END PGP SIGNATURE-----
