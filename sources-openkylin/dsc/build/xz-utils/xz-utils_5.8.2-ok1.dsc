-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: xz-utils
Binary: liblzma5, liblzma5-udeb, xz-utils, xzdec, liblzma-dev, liblzma-doc
Architecture: any all
Version: 5.8.2-ok1
Maintainer: Sebastian Andrzej Siewior <sebastian@breakpoint.cc>
Uploaders:  Jonathan Nieder <jrnieder@gmail.com>, Otto Kekäläinen <otto@debian.org>,
Homepage: https://tukaani.org/xz/
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/debian/xz-utils
Vcs-Git: https://salsa.debian.org/debian/xz-utils.git
Testsuite: autopkgtest
Testsuite-Triggers: autoconf, automake, debhelper-compat, gettext, libtool
Build-Depends: debhelper-compat (= 13), dpkg-dev (>= 1.16.2), autoconf (>= 2.64~), automake, autopoint | cvs, autopoint | gettext (<< 0.18-1), gettext, libtool (>= 2.2), po4a
Build-Depends-Indep: doxygen, graphviz
Build-Conflicts: automake1.4
Package-List:
 liblzma-dev deb libdevel unknown arch=any
 liblzma-doc deb doc unknown arch=all
 liblzma5 deb libs unknown arch=any
 liblzma5-udeb udeb debian-installer unknown arch=any
 xz-utils deb utils standard arch=any
 xzdec deb utils unknown arch=any
Checksums-Sha1:
 f856840adb9f313b6ce9189a40a5c9d89b177cfe 2850734 xz-utils_5.8.2.orig.tar.gz
 2e60771ac710fe68c770a4dc7037d6c2de3eb82e 27204 xz-utils_5.8.2-ok1.debian.tar.xz
Checksums-Sha256:
 dcbfe29e3210025fd12bbf8d365ffb64345238f4fe3e7eadf2ed52ba80738802 2850734 xz-utils_5.8.2.orig.tar.gz
 35a4342d192f342dbcb4cbd2a9c7d01816ad80e4dc86219790d786433033acdc 27204 xz-utils_5.8.2-ok1.debian.tar.xz
Files:
 7f49f2709cee0928a035324840ddd321 2850734 xz-utils_5.8.2.orig.tar.gz
 39b176af0ec05678322b5dfb96ff6fb4 27204 xz-utils_5.8.2-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnlhKQACgkQBxbwD4Qs
0nR38AwAsIoNRmLnFJBqPRGIMdJNvPFMZ9o5nx4h9lpgpGv7c9j3kp5Up+qsnu5G
tAP6P6HdbDrDc/dnnZuV7TiE5t7ZDGL+YylDBxISGzAQyJoWesFnnk9s26w4X3QO
/V0b4lA9BDiG5VDdXiKuC9FKNeKgASknyFTAtIX2JcuxXmg7wENDWmi6O19LXVbc
KkjEHKm+eGeDNUVwW0goqr/CbGYD40eGJ3dn+2dp/UNSg9YXtlprKYqNlTdCANEQ
MfA7qvMjYXLXAVEfPulfM5400IohoR75lw94WRYpYuqId/TZoI+w3+qmvbEWwWHj
/OoR37EQvmXcU+kcbiGqfpL0DNb7rYRaV6qK7cKWScIES7rpcgNCY8pwq1CoOX4P
dOSCx37J8ioABHMo5c0BeMdhga1qSKRFHvVOQ0mm/YWFUUPmNWCjjd2eOFIbRMO2
1OGRXgnkJxLtBmQscVthSxPJN5lkX0EU6QlcVILpguylHwSvi+s+ZSHjM/FbIbOw
uuDhcfMq
=vF69
-----END PGP SIGNATURE-----
