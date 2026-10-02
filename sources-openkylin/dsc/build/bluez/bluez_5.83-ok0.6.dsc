-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: bluez
Binary: libbluetooth3, libbluetooth-dev, bluetooth, bluez, bluez-cups, bluez-obexd, bluez-meshd, bluez-hcidump, bluez-test-tools, bluez-test-scripts, bluez-source
Architecture: linux-any all
Version: 5.83-ok0.6
Maintainer: Debian Bluetooth Maintainers <team+pkg-bluetooth@tracker.debian.org>
Uploaders: Nobuhiro Iwamatsu <iwamatsu@debian.org>
Homepage: http://www.bluez.org
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/bluetooth-team/bluez
Vcs-Git: https://salsa.debian.org/bluetooth-team/bluez.git
Testsuite: autopkgtest
Build-Depends: debhelper-compat (= 13), flex, bison, libdbus-1-dev (>= 1.6), libebook1.2-dev [!i386 !alpha !m68k !ppc64 !sh4 !x32], libglib2.0-dev, libdw-dev, libudev-dev, libreadline-dev, libical-dev, libasound2-dev, libell-dev (>= 0.39), libjson-c-dev (>= 0.13), python3-docutils, python3-pygments, udev, check <!nocheck>, systemd-dev
Package-List:
 bluetooth deb admin optional arch=all
 bluez deb admin optional arch=linux-any
 bluez-cups deb admin optional arch=linux-any
 bluez-hcidump deb admin optional arch=linux-any
 bluez-meshd deb admin optional arch=linux-any
 bluez-obexd deb admin optional arch=linux-any
 bluez-source deb admin optional arch=all
 bluez-test-scripts deb admin optional arch=all
 bluez-test-tools deb admin optional arch=linux-any
 libbluetooth-dev deb libdevel optional arch=linux-any
 libbluetooth3 deb libs optional arch=linux-any
Checksums-Sha1:
 ad4823acf2906e38b77305a329654e1f0bce0263 2492776 bluez_5.83.orig.tar.xz
 5bae00966a1d708d13ad14828c0afbe3e2ccedd3 23604 bluez_5.83-ok0.6.debian.tar.xz
Checksums-Sha256:
 108522d909d220581399bfec93daab62035539ceef3dda3e79970785c63bd24c 2492776 bluez_5.83.orig.tar.xz
 8a6c9b79f257f8ce75808db22dc8b3b68e8ecbea068eb8719047ef76022670e0 23604 bluez_5.83-ok0.6.debian.tar.xz
Files:
 59cf5b86dd008180503a159109bbfb9a 2492776 bluez_5.83.orig.tar.xz
 071891001c3a5af6e6f827363231acb6 23604 bluez_5.83-ok0.6.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqigV4ACgkQBxbwD4Qs
0nTUFgv/aLptC8yCilPxL3SyGvh+lgoY7wVZrXSEM5cWUXe/Z08iZSzsH7KmYcKF
+pAFBToZtRCX3KbWzS9FC0+W9tubqCVIyOCeVTjHoWhwrPNRzzS+4c+eFxZYXXOK
73Kw91JXzt6sZU5cx/pf9GNqZydWI3kjQcptkwbXAVrtMY1/DKWz8XDbHYaPb9wJ
1o5WaLXgyCZhrlqIOOoVVuCx/WCpwUwFRDDOvoV18vH5UnGWOYAB4J5FkIqWvAYU
xkSSwsO5pTnWMMamqNxcpzepCpf28z5bP0bM2gNcatqo7N734fqD7YWh5F+/MQDc
/kgcjE6frBaag7GzSzRuBm+fTMkk8178nMebVYeiCQL6Qb+ZYeZDUkwE/H2LDvs5
L6GT+djKILAuK60MibvdLDBex6qTgfBxTDHTJGXLDgaJEtG9BBFqTabK/3PVvlDh
5O4akBIRDWyLxUyXnNFIu+m2PNxGj6eRYfvzMPXgbo2flQTEpG5vmI3DGLT1R4Xh
JTnHaT9n
=Y7/B
-----END PGP SIGNATURE-----
