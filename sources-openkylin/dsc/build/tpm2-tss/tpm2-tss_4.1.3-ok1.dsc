-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: tpm2-tss
Binary: libtss2-esys-3.0.2-0t64, libtss2-fapi1t64, libtss2-mu-4.0.1-0t64, libtss2-rc0t64, libtss2-sys1t64, libtss2-tcti-cmd0t64, libtss2-tcti-device0t64, libtss2-tcti-mssim0t64, libtss2-tcti-swtpm0t64, libtss2-tcti-pcap0t64, libtss2-tcti-spi-helper0t64, libtss2-tcti-spi-ftdi0, libtss2-tcti-spi-ltt2go0, libtss2-tcti-spidev0, libtss2-tcti-i2c-helper0, libtss2-tcti-i2c-ftdi0, libtss2-tcti-libtpms0t64, libtss2-tctildr0t64, libtss2-policy0t64, libtss2-dev, tpm2-tss, tpm-udev, libtss2-doc
Architecture: any all
Version: 4.1.3-ok1
Maintainer: Mathieu Trudel-Lapierre <cyphermox@ubuntu.com>
Uploaders: Ying-Chun Liu (PaulLiu) <paulliu@debian.org>, Ivan Hu <ivan.hu@ubuntu.com>, Mario Limonciello <superm1@gmail.com>
Homepage: https://github.com/tpm2-software/tpm2-tss
Standards-Version: 4.5.0
Vcs-Browser: https://salsa.debian.org/debian/tpm2-tss
Vcs-Git: https://salsa.debian.org/debian/tpm2-tss.git
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, iproute2, procps, swtpm, swtpm-tools, udev
Build-Depends: acl, autoconf, autoconf-archive, debhelper-compat (= 13), dh-sequence-installsysusers, docbook-xml, docbook-xsl, doxygen, dpkg-dev (>= 1.22.5), libcmocka-dev (>= 1.0), libcurl4-openssl-dev | libcurl-dev, libftdi1-dev, libini-config-dev, libjson-c-dev, libltdl-dev, libssl-dev, libtool, libtpms-dev, libusb-1.0-0-dev, pkgconf, systemd | systemd-standalone-tmpfiles | systemd-tmpfiles, systemd | systemd-standalone-sysusers | systemd-sysusers, uthash-dev, uuid-dev, xsltproc
Package-List:
 libtss2-dev deb libdevel optional arch=any
 libtss2-doc deb doc optional arch=all
 libtss2-esys-3.0.2-0t64 deb libs optional arch=any
 libtss2-fapi1t64 deb libs optional arch=any
 libtss2-mu-4.0.1-0t64 deb libs optional arch=any
 libtss2-policy0t64 deb libs optional arch=any
 libtss2-rc0t64 deb libs optional arch=any
 libtss2-sys1t64 deb libs optional arch=any
 libtss2-tcti-cmd0t64 deb libs optional arch=any
 libtss2-tcti-device0t64 deb libs optional arch=any
 libtss2-tcti-i2c-ftdi0 deb libs optional arch=any
 libtss2-tcti-i2c-helper0 deb libs optional arch=any
 libtss2-tcti-libtpms0t64 deb libs optional arch=any
 libtss2-tcti-mssim0t64 deb libs optional arch=any
 libtss2-tcti-pcap0t64 deb libs optional arch=any
 libtss2-tcti-spi-ftdi0 deb libs optional arch=any
 libtss2-tcti-spi-helper0t64 deb libs optional arch=any
 libtss2-tcti-spi-ltt2go0 deb libs optional arch=any
 libtss2-tcti-spidev0 deb libs optional arch=any
 libtss2-tcti-swtpm0t64 deb libs optional arch=any
 libtss2-tctildr0t64 deb libs optional arch=any
 tpm-udev deb libs optional arch=all
 tpm2-tss deb libs optional arch=any
Checksums-Sha1:
 1059c8551037a1d17a06b1a131b198e884024f16 1231980 tpm2-tss_4.1.3.orig.tar.gz
 57ce55995d66a4373597a76949e1fb44d76f6b21 25768 tpm2-tss_4.1.3-ok1.debian.tar.xz
Checksums-Sha256:
 8a389bda64690910e7af7deaf64703a3faed416dc538c9c3e4f893f24893524d 1231980 tpm2-tss_4.1.3.orig.tar.gz
 69fa53a8c334592ab0801790beebf770ec1553e4f808c1e070ff1a3059aab3fc 25768 tpm2-tss_4.1.3-ok1.debian.tar.xz
Files:
 54bef1f2646dbdf849043fbb55406448 1231980 tpm2-tss_4.1.3.orig.tar.gz
 8b891c225cd9975bda0f12a44ad999db 25768 tpm2-tss_4.1.3-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnYjPEACgkQBxbwD4Qs
0nTb7wv/Vtzl1yHiYi/hO8Y8WnvkkaWjJPvmzYNFdW0LrVv8izNLpaS0NAIkuKVw
X+G7SevG+OxPAMtpIJz7aCYebezjpSAbem5g6akZTyp+Yk9wy/31hLHRqABpj1zf
iUWQrzWweT3sHVZ+IY1pMKYywT+5xc8xemRIQwXvur6jRxF60mjmaj/FHjUvXtIb
gtBGSpwf9x0SnVByrdQb92NJsFDocBAGYXtL8un1XrOT2tSiIYZAYawokTmCxSHO
2g5X6/apqwGXXlqdAPKbss86B+ZXJurNWHw8rbK9PZE1CenuJcZIXvzvWJJ6c3Oy
WPPGFCn3mvLS8sG0APaH3hquKc/rQk0J2uIHcyHODmgqLknj5H1wdrX027O/bKAQ
969Yp96LpId1yo9jgcG3hsror9+tZVQ/ncgV8vzPW4jLtdsus27KcCaTDLDjIenr
vOl0oyaCYu0vmmFb0NcLl7rLxW7p3ufHT4lQnfpIi/ickAzU1MzPo01gcJb3XvxY
1qe95/SQ
=VBEO
-----END PGP SIGNATURE-----
