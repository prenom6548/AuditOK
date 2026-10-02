-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: xdg-desktop-portal
Binary: xdg-desktop-portal, xdg-desktop-portal-dev, xdg-desktop-portal-tests
Architecture: any all
Version: 1.21.1+ds-ok2
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders:  Simon McVittie <smcv@debian.org>,
Homepage: https://flatpak.github.io/xdg-desktop-portal/
Standards-Version: 4.7.4
Vcs-Browser: https://salsa.debian.org/debian/xdg-desktop-portaltree/ubuntu/latest
Vcs-Git: https://salsa.debian.org/debian/xdg-desktop-portal.git -b ubuntu/latest
Testsuite: autopkgtest
Testsuite-Triggers: gnome-desktop-testing
Build-Depends: debhelper-compat (= 13), bubblewrap, dbus-daemon, fuse3 <!nocheck>, geoclue-2.0 <!nocheck>, gstreamer1.0-plugins-good, gstreamer1.0-tools, libcap2-bin <!nocheck>, libflatpak-dev, libfuse3-dev, libgdk-pixbuf-2.0-dev, libgeoclue-2-dev, libglib2.0-dev, libgstreamer-plugins-base1.0-dev, libgudev-1.0-dev, libjson-glib-dev, libpipewire-0.3-dev, librsvg2-common <!nocheck>, libsystemd-dev, libumockdev-dev <!nocheck> <!noinsttest>, meson, pipewire <!nocheck>, procps <!nocheck>, python3 <!nocheck> <!noinsttest>, python3-dbus <!nocheck> <!noinsttest>, python3-dbusmock <!nocheck> <!noinsttest>, python3-docutils <!nodoc>, python3-gi <!nocheck> <!noinsttest>, python3-pytest <!nocheck> <!noinsttest>, umockdev <!nocheck>
Build-Depends-Indep: dh-sequence-sphinxdoc <!nodoc>, furo <!nodoc>, python3-sphinx <!nodoc>, python3-sphinx-copybutton <!nodoc>, python3-sphinxext-opengraph <!nodoc>
Package-List:
 xdg-desktop-portal deb admin unknown arch=linux-any
 xdg-desktop-portal-dev deb admin unknown arch=all
 xdg-desktop-portal-tests deb admin unknown arch=any profile=!noinsttest
Checksums-Sha1:
 72b7add6d0794ac9d6a566dab8eea98ff99ba300 906776 xdg-desktop-portal_1.21.1+ds.orig.tar.xz
 bf3b3dc82a3e6daf13d37da4faec1d9116c455fe 37596 xdg-desktop-portal_1.21.1+ds-ok2.debian.tar.xz
Checksums-Sha256:
 21586f1612520a707b3675c50a34350a44566c67b9faeddf7ac15743ff23a457 906776 xdg-desktop-portal_1.21.1+ds.orig.tar.xz
 44d7f88eca9201ae646d9e427b50cc00569695b2880a6f5389d2b0ac7b478502 37596 xdg-desktop-portal_1.21.1+ds-ok2.debian.tar.xz
Files:
 3fcdccf33d532d39bfa91ad0bd5dc3bc 906776 xdg-desktop-portal_1.21.1+ds.orig.tar.xz
 6b6caecc981efbfeebf51d6282ef8070 37596 xdg-desktop-portal_1.21.1+ds-ok2.debian.tar.xz
Debian-Vcs-Browser: https://salsa.debian.org/debian/xdg-desktop-portal
Debian-Vcs-Git: https://salsa.debian.org/debian/xdg-desktop-portal.git
Original-Maintainer: Utopia Maintenance Team <pkg-utopia-maintainers@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqg/r8ACgkQBxbwD4Qs
0nTObQv/dN1GugwIvrJ6MV1/C0hElLUBn0GEv5LD5QDyKaEvrZ4FOExmxNcYCptf
Y+NlKWg3bsPbI4q9hfm+MGmMZ5ntO3aBJwxqUzfnHfsbTgxgnnUgekyCQlKedMVG
dtLMRdEMatsUKlZfe6/E5Janlw2XAF1Ffm5XDNRFHBdJn5McY/UE4ycV9FYsoYKs
rNWvVu6jvOKufzvzNEUSHWRAFAbgz8j42f9yte9ipochPK4Y2RYJhdbbhuLECobb
Py8XZCwUk4ubTSy3pBYEI3Nuh4lIge7Yfs8PjfqsldC285CjiUyY+nU9GmG87afS
vEO9dUj/pX3nAXHUKCcz+T/Xas9ziyHOTWPKoQ4UQIoob5eGH7LB2BNJvrX5q1ss
LkBc1icrfemLa9a/SlQP5+gJNdliSsvfDZbn0Z0V3BIJxgncHPE/mdi7Hg4pEFTi
7yfWULFyg+Chnh8t47quv2Mb2PSM70nU45KBth8IyS03Y0jCS37AP8khY7dz5szA
c/tCo9O2
=nH3s
-----END PGP SIGNATURE-----
