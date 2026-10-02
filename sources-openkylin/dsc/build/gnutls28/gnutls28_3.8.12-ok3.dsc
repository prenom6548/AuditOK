-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: gnutls28
Binary: libgnutls28-dev, libgnutls30t64, gnutls-bin, gnutls-doc, libgnutls-openssl27t64, libgnutls-dane0t64
Architecture: any all
Version: 3.8.12-ok3
Maintainer: Openkylin Developers <packaging@lists.openkylin.top>
Homepage: https://www.gnutls.org/
Standards-Version: 4.6.2
Vcs-Browser: https://gitee.com/openkylin/gnutls28
Vcs-Git: https://gitee.com/openkylin/gnutls28.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, ca-certificates, datefudge, freebsd-net-tools, net-tools, openssl, softhsm2
Build-Depends: dpkg-dev (>= 1.22.5), bison, ca-certificates <!nocheck>, chrpath, datefudge <!nocheck>, debhelper-compat (= 13), freebsd-net-tools [kfreebsd-i386 kfreebsd-amd64] <!nocheck>, gperf, libcmocka-dev <!nocheck>, libidn2-dev, libp11-kit-dev, libssl-dev <!nocheck>, libtasn1-6-dev, libunbound-dev, libunistring-dev, net-tools [!kfreebsd-i386 !kfreebsd-amd64] <!nocheck>, nettle-dev (>= 3.6), openssl <!nocheck>, pkgconf, python3:any, softhsm2 <!nocheck>
Build-Depends-Indep: gtk-doc-tools, texinfo, texlive-latex-base, texlive-plain-generic
Build-Conflicts: libgnutls-dev
Package-List:
 gnutls-bin deb net optional arch=any
 gnutls-doc deb doc optional arch=all
 libgnutls-dane0t64 deb libs optional arch=any
 libgnutls-openssl27t64 deb libs optional arch=any
 libgnutls28-dev deb libdevel optional arch=any
 libgnutls30t64 deb libs optional arch=any
Checksums-Sha1:
 939b5d6fcfff2e2894df67aef9b36a4c02844f3b 6936480 gnutls28_3.8.12.orig.tar.xz
 e5abb0927852829360585724c8a00161b9370e60 44272 gnutls28_3.8.12-ok3.debian.tar.xz
Checksums-Sha256:
 601b126abf05de9b0985ee285b619705fb2655e32575291e31ac93e8dc39167e 6936480 gnutls28_3.8.12.orig.tar.xz
 382cb97e97ae733f0f4963a0ada1ae52f5eda7c6c3a9df5f40f54e74e905a4fc 44272 gnutls28_3.8.12-ok3.debian.tar.xz
Files:
 e7f188ec5dfee84fd3703a22da948640 6936480 gnutls28_3.8.12.orig.tar.xz
 d75aa679a1eb7fe765ffff5b74b731d0 44272 gnutls28_3.8.12-ok3.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnuyk8ACgkQBxbwD4Qs
0nSOGgv/X2KAtXDI658KOof6F0ctU8vZfAJDwsUKx/12kAwq5V8/JZYIOQ98UMaH
al5ceMsSXF4LpGf9letyPmw1qJQRqMWfobuZNZGJvozW6JkVfLiFU3pf1qntMG+c
BhVH2DvaAxj5w7+X7hqjqI1GkKAV6OwNm9mMtTlxbgIKM9aAymjXmVYYrfKHJr8k
qKCoMm6ChIht8CPr0jC8MxnNof4EfiZoPpSdjA4n+bEp5qVOlrUOHszJHnsMJKc1
auO6fhruBsRION2lvie8U2+jQzBr+pifwJt9RG7O4EUDyqjsYDiMGu5Ot2JziP38
zdXt/1kS7bDGtrnZpwLA7aZbV9+ML8afOeHBH96r0LT/O5+obOK4/oXJH+XbjyrI
H8qeBK/F6sXbEEzYYBscfr/JDR6lHnbXTmNOfzCv/zeVjokspYdfEB2S7p1o64Yq
m8dWPYiUCT0gPlnZOomBEjr8oSsxg+aJ81DLuVVs/qRKv7HqwJG5x1drKxyPMWh2
rS3T1PLE
=9QhL
-----END PGP SIGNATURE-----
