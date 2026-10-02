-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: util-linux
Binary: util-linux, util-linux-locales, mount, bsdutils, bsdextrautils, fdisk, fdisk-udeb, libblkid1, libblkid1-udeb, libblkid-dev, libfdisk1, libfdisk1-udeb, libfdisk-dev, libmount1, libmount1-udeb, libmount-dev, libsmartcols1, libsmartcols1-udeb, libsmartcols-dev, libuuid1, uuid-runtime, libuuid1-udeb, uuid-dev, util-linux-udeb, rfkill, eject, eject-udeb, util-linux-extra, liblastlog2-2, liblastlog2-dev, libpam-lastlog2, lastlog2, login
Architecture: any all
Version: 2.41.3-ok9.1
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Homepage: https://github.com/util-linux/util-linux
Standards-Version: 4.7.0
Vcs-Browser: https://salsa.debian.org/debian/util-linux
Vcs-Git: https://salsa.debian.org/debian/util-linux.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, expect, passwd, pkg-config
Build-Depends: debhelper-compat (= 13), dh-exec, dh-package-notes, dh-sequence-installsysusers, dh-sequence-zz-debputy-rrr (>= 0.1.23~), asciidoctor <!stage1 !nodoc>, bc <!stage1 !nocheck>, bison, flex, gettext, libaudit-dev [linux-any] <!stage1>, libcap-ng-dev [linux-any] <!stage1>, libcrypt-dev <!stage1>, libcryptsetup-dev [linux-any] <!pkg.util-linux.noverity>, libncurses-dev, libpam0g-dev <!stage1>, libreadline-dev, libselinux1-dev [linux-any], libsqlite3-dev, libsystemd-dev [linux-any] <!stage1>, libtool, libudev-dev [linux-any] <!stage1>, netbase <!stage1 !nocheck>, pkgconf, po-debconf, po4a, socat <!stage1 !nocheck>, systemd [linux-any] <!stage1>, systemd-dev [linux-any] <!stage1>, zlib1g-dev
Build-Conflicts: libedit-dev
Package-List:
 bsdextrautils deb utils optional arch=any profile=!stage1
 bsdutils deb utils standard arch=any profile=!stage1
 eject deb utils optional arch=linux-any profile=!stage1
 eject-udeb udeb debian-installer optional arch=linux-any profile=!stage1,!noudeb
 fdisk deb utils important arch=any
 fdisk-udeb udeb debian-installer optional arch=hurd-any,linux-any profile=!stage1,!noudeb
 lastlog2 deb utils optional arch=any profile=!stage1
 libblkid-dev deb libdevel optional arch=any
 libblkid1 deb libs optional arch=any
 libblkid1-udeb udeb debian-installer optional arch=any profile=!noudeb
 libfdisk-dev deb libdevel optional arch=any
 libfdisk1 deb libs optional arch=any
 libfdisk1-udeb udeb debian-installer optional arch=any profile=!noudeb
 liblastlog2-2 deb libs optional arch=any
 liblastlog2-dev deb libdevel optional arch=any
 libmount-dev deb libdevel optional arch=linux-any
 libmount1 deb libs optional arch=any
 libmount1-udeb udeb debian-installer optional arch=linux-any profile=!noudeb
 libpam-lastlog2 deb admin optional arch=any profile=!stage1
 libsmartcols-dev deb libdevel optional arch=any
 libsmartcols1 deb libs optional arch=any
 libsmartcols1-udeb udeb debian-installer optional arch=any profile=!noudeb
 libuuid1 deb libs optional arch=any
 libuuid1-udeb udeb debian-installer optional arch=any profile=!noudeb
 login deb admin required arch=any profile=!stage1
 mount deb admin required arch=linux-any profile=!stage1
 rfkill deb utils optional arch=linux-any profile=!stage1
 util-linux deb utils required arch=any profile=!stage1 essential=yes
 util-linux-extra deb utils standard arch=any profile=!stage1
 util-linux-locales deb localization optional arch=all profile=!stage1
 util-linux-udeb udeb debian-installer optional arch=any profile=!stage1,!noudeb
 uuid-dev deb libdevel optional arch=any
 uuid-runtime deb utils optional arch=any profile=!stage1
Checksums-Sha1:
 03f949972209f0adb0b5b91e9b8fd1bb3b2f5c35 20862884 util-linux_2.41.3.orig.tar.gz
 74d3ebea5fda58a93337e4ba556628e098c7ca00 2183348 util-linux_2.41.3-ok9.1.debian.tar.xz
Checksums-Sha256:
 11bbf4ffc53caf350a0e468e993c93e6f7ace90c73812f85c29724eed9ce36b4 20862884 util-linux_2.41.3.orig.tar.gz
 5bffab82fabee3ef3fa10d99829ad10c3addde05e6e248bb08b203e04320e975 2183348 util-linux_2.41.3-ok9.1.debian.tar.xz
Files:
 4b0fc53a848c4aed45dc5b8beb1ebee7 20862884 util-linux_2.41.3.orig.tar.gz
 0687121326e946fc991a58935b22a6f6 2183348 util-linux_2.41.3-ok9.1.debian.tar.xz
Original-Maintainer: Chris Hofstaedtler <zeha@debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqB5JsACgkQBxbwD4Qs
0nTaFwv+KxhVYLwtoz3dMs5IJri4AxG4xPDu0dqrHcXakeCS2C9OtQL7nESGqGp2
M/ZDLvvgDsCv0DsTADfWI+gdi+5sQQ+4NTwbYZFLyHjtVvkN/CLfhR6QRJhpDmQQ
+SGI9R5aVNSLdonB71GNVIPiy2HejpxbtAPrWMMXJGFQxMylCXBXxcu8ucYBPUb1
QG9HFk6FswRuqitLB9VD55VIr1eyvWqSG2qk1Y6YZEXI1iWDnwIVTtpbGqnKQgZq
7HlrvvtCY+y5B/KhAs3BZpY/iNPsewuZQn9oEPjXNrWVt7rer7kTJDy7CTX5uLPU
rMlkdi50x/wISm/WgSA8peW+o1eOojvRXZ1Rz74KdcBWEvitqsIuQOq0QIFg4HiC
3Yo8ch9LfSTYTPDDtqGP1xs7hdoFTu9ckyuuh6kyL3jTeEtb0Uc/NfZtuAIFQKba
xv7l1IqBYJHYQvIenTz48P5xdZDG0QZZanKrELGY/vzlonnweTBrkdQtOLvSczKI
crb5j2kD
=0MEk
-----END PGP SIGNATURE-----
