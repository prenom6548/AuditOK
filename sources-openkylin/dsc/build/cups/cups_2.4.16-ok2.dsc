-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: cups
Binary: libcups2t64, cups, cups-bsd, cups-client, cups-common, cups-core-drivers, cups-daemon, cups-ipp-utils, cups-ppdc, cups-server-common, libcups2-dev, libcupsimage2t64, libcupsimage2-dev
Architecture: any all
Version: 2.4.16-ok2
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders:  Till Kamppeter <till.kamppeter@gmail.com>, Thorsten Alteholz <debian@alteholz.de>,
Homepage: https://github.com/OpenPrinting/cups/
Standards-Version: 4.6.2
Vcs-Browser: https://salsa.debian.org/printing-team/cups
Vcs-Git: https://salsa.debian.org/printing-team/cups.git
Testsuite: autopkgtest
Build-Depends: dpkg-dev (>= 1.22.5), autoconf, automake, debhelper-compat (= 13), debhelper (>= 13.11.6), dh-strip-nondeterminism, libavahi-client-dev, libavahi-common-dev, libdbus-1-dev, libgnutls28-dev, libkrb5-dev, libpam0g-dev, libpaper-dev, libsystemd-dev [linux-any], libtool, libusb-1.0-0-dev [!hurd-any], patch, pkg-config, po-debconf, po4a, zlib1g-dev, libapparmor-dev, libsnapd-glib-dev (>= 1.61)
Build-Depends-Arch: dpkg-dev (>= 1.22.5), dh-apparmor
Build-Conflicts: libgmp-dev (<< 2:6)
Package-List:
 cups deb net optional arch=any
 cups-bsd deb net optional arch=any
 cups-client deb net optional arch=any
 cups-common deb net optional arch=all
 cups-core-drivers deb net optional arch=any
 cups-daemon deb net optional arch=any
 cups-ipp-utils deb net optional arch=any
 cups-ppdc deb utils optional arch=any
 cups-server-common deb net optional arch=all
 libcups2-dev deb libdevel optional arch=any
 libcups2t64 deb libs optional arch=any
 libcupsimage2-dev deb libdevel optional arch=any
 libcupsimage2t64 deb libs optional arch=any
Checksums-Sha1:
 d31642b9d38985f65010587b8fba8e02f1515316 8159166 cups_2.4.16.orig.tar.gz
 1b2397d3cc208a2b8315234f501b6bf76b859b7a 228 cups_2.4.16.orig.tar.gz.asc
 ed190ecb9ff79d7b0965df4b856785dd3a887524 305880 cups_2.4.16-ok2.debian.tar.xz
Checksums-Sha256:
 0339587204b4f9428dd0592eb301dec0bf9ea6ea8dce5d9690d56be585aba92d 8159166 cups_2.4.16.orig.tar.gz
 94714372b58a5680d661613ad984444e555201b27bd562490c5a077d810b712d 228 cups_2.4.16.orig.tar.gz.asc
 7eb24c7c81e512f632e347891139b89d190007a18a670e6268f6b85607487036 305880 cups_2.4.16-ok2.debian.tar.xz
Files:
 53cccb5d69eaed7e4c6fe66fa12b11c4 8159166 cups_2.4.16.orig.tar.gz
 f6e930af04f29c1132749be302b2d87d 228 cups_2.4.16.orig.tar.gz.asc
 45df95933d7907a168229b12fd25faab 305880 cups_2.4.16-ok2.debian.tar.xz
Original-Maintainer: Debian Printing Team <debian-printing@lists.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmp+tCYACgkQBxbwD4Qs
0nSy2gwAl+kI2QFv8m4lVp00tgnALeK9x6xM0ph80zjJfPUp2O1PopDHjuX/CF8d
YbKg1OqswaGTWH9ZkZjW1w7TN8NCKGswkbTYouF7QUOmkpf8xi89Ak4zJSdt3/dY
F99YwSoiu+v3h8aZEqFoF54jL1kzzqsJ4XJ5o5I3fgKp2CU1Q8UYXoInezHLkZmD
tZkFRfFH27EJot+plSH0qxoJ1jMenBdVuknzmHz6rP4+sLPsIStcKtAcjQr2w1bW
VpxR4WYT7kurEY+Chs1IiySJfm9Gk5B21rUs7ILAwRE1NelMux3PuhWAaqFIqOfR
cCBW67s3u5xn1wzSNrWVuK/1+kEcGkoofFm//wsjG52zyKTIvv5iabYFWdhJ7hkE
auiDTJoi+NXqiz7aSUzAcpfPPZ7TFA5gm1LTfyNwdM/pFQpHlCtoNsv3WlhZB4v2
C1bQrB0mL3H4nLW4fw3NfW2p8w1FwV/dfLaGK1Xs9Q8I4QSKfW0Vx6NX6lWd5bf/
CKxOIfu7
=14Jo
-----END PGP SIGNATURE-----
