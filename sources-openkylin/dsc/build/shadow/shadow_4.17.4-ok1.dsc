-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: shadow
Binary: passwd, login.defs, uidmap, libsubid5, libsubid-dev
Architecture: any all
Version: 1:4.17.4-ok1
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders:  Serge Hallyn <serge@hallyn.com>, Chris Hofstaedtler <zeha@debian.org>
Homepage: https://github.com/shadow-maint/shadow
Standards-Version: 4.7.0
Vcs-Browser: https://salsa.debian.org/debian/shadow
Vcs-Git: https://salsa.debian.org/debian/shadow.git -b master
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, augeas-tools, expect, libnss-extrausers, procps
Build-Depends: bison, debhelper-compat (= 13), dh-package-notes, dh-sequence-zz-debputy-rrr (>= 0.1.23~), docbook-xml <!nodoc>, docbook-xsl <!nodoc>, gettext, itstool <!nodoc>, libacl1-dev, libattr1-dev, libaudit-dev [linux-any], libbsd-dev, libcrypt-dev, libltdl-dev, libpam0g-dev, libselinux1-dev [linux-any], libsemanage-dev [linux-any], libxml2-utils <!nodoc>, pkgconf, systemd-dev [linux-any], xsltproc <!nodoc>
Package-List:
 libsubid-dev deb libdevel optional arch=any
 libsubid5 deb libs optional arch=any
 login.defs deb admin required arch=all
 passwd deb admin required arch=any
 uidmap deb admin optional arch=any
Checksums-Sha1:
 6472ed66405924de1f564d0a18f01f12e5ce44d0 2326584 shadow_4.17.4.orig.tar.xz
 7bd20c7d3f803fa040e0778ff8fb01423b61fbdb 488 shadow_4.17.4.orig.tar.xz.asc
 de4c2310a0277624b40de97c8f86d4bd9f0343ee 127092 shadow_4.17.4-ok1.debian.tar.xz
Checksums-Sha256:
 554801054694ff7d8a7abdf0d6ece34e2f16e111673cc01b8c9ee1278451181e 2326584 shadow_4.17.4.orig.tar.xz
 82437d2b54790b3a6734c526b757e693cd0c734ec6afd69f9a9cf0eb1dad8dc4 488 shadow_4.17.4.orig.tar.xz.asc
 819a9c576772bd0431c167707248416ce49c33c58f6150c110a80e42766866dc 127092 shadow_4.17.4-ok1.debian.tar.xz
Files:
 469666ea82c203ce5b0116d26b3793a9 2326584 shadow_4.17.4.orig.tar.xz
 ac356bfd174072b7bee29779ec6357e9 488 shadow_4.17.4.orig.tar.xz.asc
 00cdd0b2b105903d60aedaf30f504bd2 127092 shadow_4.17.4-ok1.debian.tar.xz
Original-Maintainer: Shadow package maintainers <pkg-shadow-devel@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnPbe4ACgkQBxbwD4Qs
0nT3YgwAjWkndc8rU+cyKpjUVU5575rXSPcKoUg97TL5FFA5r8ReT2MTYjAsuI38
NOlyTuCW9sF6iYdTukdkiGZdnf2/N3U7c5dD1OAOlunZDScGTPmo8+UgIbAyGXzf
kPK9i7exwflhG2EuvEDHjxJDY4ga6sH3J9SV/yfi6fSmdwwy6MO/4E3t3+eK6Le0
I9X1OF3qhM9kZp4znrGWkb4sxlxccftiRz66lrgU+IQXi33yYCru9j8D0e0HZ5hJ
jUogAiENh2hLhmgBDViT6G9r1I5DxW7ZMv7cKqAru5chMFN0gak9QFX2T8Oh2hbq
Tyv/tuEqn8gbM5yFF0eX+g8G2Ps9ss6La9rqSEJhM5S2lCCxWp4+TYjLh8Lk7YuF
6xHDrnMwvscWGZPgiV10A66NS1n0/Rx2BpUcqVzx1lsexpX9Z+d5I6Q6PY8Z5s2V
ygCF5P3RSKyIVM/4O7/21bWNapIXzdii60Neg21+Qh9K4re7wq2b+m4Y+EMxR+v0
Ic2rNfKm
=3mpy
-----END PGP SIGNATURE-----
