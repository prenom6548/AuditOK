-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: openssl
Binary: openssl, openssl-provider-legacy, libssl3t64, libcrypto3-udeb, libssl3-udeb, libssl-dev, libssl-doc
Architecture: any all
Version: 3.5.5-ok8
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Christoph Martin <christoph.martin@uni-mainz.de>, Kurt Roeckx <kurt@roeckx.be>, Sebastian Andrzej Siewior <sebastian@breakpoint.cc>
Homepage: https://openssl-library.org
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/debian/openssl
Vcs-Git: https://salsa.debian.org/debian/openssl.git
Testsuite: autopkgtest
Build-Depends: debhelper-compat (= 13), dpkg-dev (>= 1.22.5), libzstd-dev, zlib1g-dev, libjitterentropy3-dev
Package-List:
 libcrypto3-udeb udeb debian-installer optional arch=any profile=!noudeb
 libssl-dev deb libdevel optional arch=any
 libssl-doc deb doc optional arch=all
 libssl3-udeb udeb debian-installer optional arch=any profile=!noudeb
 libssl3t64 deb libs optional arch=any
 openssl deb utils optional arch=any
 openssl-provider-legacy deb utils optional arch=any
Checksums-Sha1:
 72a5ebbdd30bc28a66f069e2d50c66a007c324d2 53104821 openssl_3.5.5.orig.tar.gz
 ff7a37d551ce7f25695266d29fb1439ba3f6b43f 833 openssl_3.5.5.orig.tar.gz.asc
 7bb1e06b39d51397178c1f3d4e47b85d5849161f 78008 openssl_3.5.5-ok8.debian.tar.xz
Checksums-Sha256:
 b28c91532a8b65a1f983b4c28b7488174e4a01008e29ce8e69bd789f28bc2a89 53104821 openssl_3.5.5.orig.tar.gz
 eaef5b1054b84b8d1e6c61c9fc8867828be5ce686d0221580faf8bdc16489da9 833 openssl_3.5.5.orig.tar.gz.asc
 fa10d16f848ee1c9a8b72c47dcd46920d613bfc1c93edd6e8d4efbd4189bcfec 78008 openssl_3.5.5-ok8.debian.tar.xz
Files:
 9c86d929c3d1067e2c88239d7d1ce81b 53104821 openssl_3.5.5.orig.tar.gz
 7e827079c420b263e8dadfe6fafcdf40 833 openssl_3.5.5.orig.tar.gz.asc
 70814562b4b568f80ecad0e510918122 78008 openssl_3.5.5-ok8.debian.tar.xz
Original-Maintainer: Debian OpenSSL Team <pkg-openssl-devel@alioth-lists.debian.net>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqVIKoACgkQBxbwD4Qs
0nTq0Av/fnnMXn2dPMzsyelOXT/owixdorKLNB1zFJR2zFWYsb2wV+gsBaBJfqDi
dWIs3WnRKuV1ivWpmR9McDcj0DWQ0BUDLSB9jmO/C1cEWP7ndGhDneqxinitR3Sc
dduayKfgczdqvnBLOhYphTleCi1M2IbN+gVQ+ARsf6xh7QMCPFQuWPdhZBY/C6IN
xtah/idh5qFvXLwr3ql/Vz7TH4fJ0lOX1UR49yBq2PGCKVYlu8RJSJflVwgoe3Jc
xlkgKpVlg64ZgoL5mUmlVRYPfVpSid50H51i2YbWEicjqauxf7VsWHMMfCAHlv7U
rHJ4fZlXKBiYd3haWDrzSH0vmvEyLlq260WrPIhgIDrXhfASe2QKJ6Z1rL3Q2TQp
eQOofF2Vvt3GgScPfW1Cq3YjVErpkzdHdE0As9ml4oV7Saxou2GKcJS4oEVI2qWG
r5A2yEqm3L3M1kQeEZMuyF95ayT94GyrtI9PHALn7vceY2UfSL/rS+1KbiyegHJQ
TNO536Un
=CXOp
-----END PGP SIGNATURE-----
