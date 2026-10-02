-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: libsoup3
Binary: libsoup-3.0-dev, libsoup-3.0-0, libsoup-3.0-common, libsoup-3.0-doc, gir1.2-soup-3.0, libsoup-3.0-tests
Architecture: any all
Version: 3.6.6-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: Jeremy Bícha <jbicha@ubuntu.com>
Homepage: https://libsoup.gnome.org/libsoup-3.0/index.html
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/gnome-team/libsoup3
Vcs-Git: https://salsa.debian.org/gnome-team/libsoup3.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, dbus, gnome-desktop-testing, winbind, xauth, xvfb
Build-Depends: debhelper-compat (= 13), dh-sequence-gir, dh-sequence-gnome, apache2 <!nocheck> <!noinsttest>, ca-certificates <!nocheck> <!noinsttest>, curl <!nocheck> <!noinsttest>, gi-docgen, gir1.2-gio-2.0-dev, glib-networking, gobject-introspection (>= 1.80), libapache2-mod-php (<< 2:9) [!i386] <!nocheck> <!noinsttest>, libbrotli-dev, libglib2.0-dev, libgnutls28-dev, libkrb5-dev, libnghttp2-dev, libnss-myhostname [linux-any] <!nocheck>, libpsl-dev, libsqlite3-dev, libsysprof-capture-4-dev [linux-any], meson, php (<< 2:9) [!i386] <!nocheck> <!noinsttest>, valac, winbind
Package-List:
 gir1.2-soup-3.0 deb introspection unknown arch=any
 libsoup-3.0-0 deb libs unknown arch=any
 libsoup-3.0-common deb devel unknown arch=all
 libsoup-3.0-dev deb libdevel unknown arch=any
 libsoup-3.0-doc deb doc unknown arch=all
 libsoup-3.0-tests deb misc unknown arch=any profile=!noinsttest
Checksums-Sha1:
 bf60e40726e94f5253a752071c9a80abb86ddc21 1572004 libsoup3_3.6.6.orig.tar.xz
 c821e0c882f07bdace66b38dc5d26f185caf6410 12112 libsoup3_3.6.6-ok1.debian.tar.xz
Checksums-Sha256:
 51ed0ae06f9d5a40f401ff459e2e5f652f9a510b7730e1359ee66d14d4872740 1572004 libsoup3_3.6.6.orig.tar.xz
 85b6eaeec8d21b0bf8757fa129966156cb72e56c524102ef506c8aa3d11fc5ec 12112 libsoup3_3.6.6-ok1.debian.tar.xz
Files:
 9e07742595800ecf301eafb52300dd89 1572004 libsoup3_3.6.6.orig.tar.xz
 20584acb4337933a5cae7123c71632f5 12112 libsoup3_3.6.6-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmocNbkACgkQBxbwD4Qs
0nR2KAv+KR6bY287e+XErlSylqaemXtdP54LUg3tCCbl7e+Cqn4YqoYaujT6jxPl
nI/Wc20enbh8cMCG+o9XX5b4qY6+DPv942TbgeD+8GhI7dfpubqkQB0uL8rnHM5X
rBbTpMCy9AAvqaLhvP3v24QePBWk63ovHOSpxgszoamzb0gz/wCG4NXxa3tikgfE
Vig2cwplqQf5FcpNPMYXOe3tjItXTREPEMGoB7V0nEwJFHWm18MfrGOEi/QX8KQG
Lb2JYcG74IprLKzZToDUxY6xDOIATqGel7XPhwXogcJf42+c4FeNjyg8sN+8tnm4
uW1LyQiIW78aHClrPChy3PJed0b6Paa7RxWPPjxT8Rp6G/gEgoaRlmQDI1YzrmYC
xcEU77y5/g8jSXd4QgbtHv9sw64EPVOpRRnfh5WrLjhVgKXH3YRJWYMQs4Qsdtmq
cNl9QC34Kw+f1ddY1wNOSdoj6eaVY4yxFgXzyuEJOWCHzIorgarXQeg4Pi7+F8Dr
zjd6gg38
=SFmg
-----END PGP SIGNATURE-----
