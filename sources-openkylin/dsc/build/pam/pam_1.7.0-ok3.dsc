-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: pam
Binary: libpam0g, libpam-modules, libpam-modules-bin, libpam-runtime, libpam0g-dev, libpam-doc
Architecture: any all
Version: 1.7.0-ok3
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Homepage: http://www.linux-pam.org/
Standards-Version: 4.7.0
Vcs-Git: https://code.launchpad.net/~ubuntu-core-dev/ubuntu/+source/pam/+git/pam
Testsuite: autopkgtest
Testsuite-Triggers: python3-pam
Build-Depends: debhelper-compat (= 13), dh-exec, flex, libcrypt-dev, libdb-dev, libselinux1-dev [linux-any], libsystemd-dev [linux-any] <!stage1>, po-debconf, meson, libaudit-dev [linux-any] <!stage1>, pkgconf, libfl-dev, libfl-dev:native
Build-Depends-Indep: docbook-xsl-ns, docbook5-xml, xsltproc, libxml2-utils, w3m, fop
Build-Conflicts: libdb4.2-dev, libxcrypt-dev
Package-List:
 libpam-doc deb doc optional arch=all profile=!nodoc
 libpam-modules deb admin required arch=any
 libpam-modules-bin deb admin required arch=any
 libpam-runtime deb admin required arch=all
 libpam0g deb libs optional arch=any
 libpam0g-dev deb libdevel optional arch=any
Checksums-Sha1:
 935f3a737f834ac94a6600e4e3619de47e0cfa6a 507824 pam_1.7.0.orig.tar.xz
 0cc8ae9ba7a17610041e702b70eaba8765f93ced 801 pam_1.7.0.orig.tar.xz.asc
 d2b00d6a7d1ebe69f1116b6623d6dffea3a4a154 144656 pam_1.7.0-ok3.debian.tar.xz
Checksums-Sha256:
 57dcd7a6b966ecd5bbd95e1d11173734691e16b68692fa59661cdae9b13b1697 507824 pam_1.7.0.orig.tar.xz
 7a8ea18ec7d9dd1f8cbf9055c32128cbca8241aa63e9fea44d56ce6f0e15e441 801 pam_1.7.0.orig.tar.xz.asc
 a87ee5342118b4af1de06677cd5ca12d6109db23c379e6d03beaeb2a00b40489 144656 pam_1.7.0-ok3.debian.tar.xz
Files:
 c1e41d59d6852e45d0f953c8c8f869d6 507824 pam_1.7.0.orig.tar.xz
 9a57369709c01169ecc6b2ff59a43db6 801 pam_1.7.0.orig.tar.xz.asc
 58b19537109f151a07e1ab4d05fcef49 144656 pam_1.7.0-ok3.debian.tar.xz
Debian-Vcs-Browser: https://salsa.debian.org/vorlon/pam
Debian-Vcs-Git: https://salsa.debian.org/vorlon/pam.git
Original-Maintainer: Sam Hartman <hartmans@debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqNVgMACgkQBxbwD4Qs
0nROtQwAuQyFBDWHZXS2884NaAX2XcDNwItceu7GuoF9KzIfNHq7xkaQBIuPqKAo
y5Vn2HeNZxZ3j8hYxyHOvIfNkBUqNGYJrN85o2DhafBuUdNwRfMWT/gi20YaEhQF
TOWeZiNRVdM8ddelyWO4pFt8mSLn6MpLThCIitL1tDpVr7GBhAeLVD9SYpKQ80CF
ZDuEImlW0eg428g5hv6HhKwquqYYbV+tX2hrjF0Hki6GWbYTJZRgbvJGL/3zSYcg
5YS/i6BQrAmzEX0MpguY5g084vl4HsPUP0rJgq0ghuKGP0L41Ub3j6AxVHT2rFcc
FZ9S1f0fIOBf9XDhzGSVgoApRpe+DhQ2+1y/ejY/fNmQ/cgFLaEqs0XBzIPY8Z6E
hXdmZDdDSKJoh0mnKOc/A3IS84vtzz1Jx6M+bF/1fih4Zbw/ibY8l1FNqsaEz/6K
H00u9UQaG6Zwz+kguZpkPGujQZViFSHh0t484xsUElMVBvwujhiSA59OLyX5CpuW
FjL++zcq
=ZHi5
-----END PGP SIGNATURE-----
