-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: sqlcipher
Binary: sqlcipher, libsqlcipher2, libsqlcipher-dev
Architecture: any
Version: 4.13.0-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://www.zetetic.net/sqlcipher/
Description: Encryption in the SQLite 3 database engine
 SQLCipher is a C library that implements an encryption in the SQLite 3
 database engine.  Programs that link with the SQLCipher library can have SQL
 database access without running a separate RDBMS process.  It allows one to
 have per-database or page-by-page encryption using AES-256 from OpenSSL.
 .
 SQLCipher has a small footprint and great performance so it’s ideal for
 protecting embedded application databases and is well suited for mobile
 development.
 .
  * as little as 5-15% overhead for encryption
  * 100% of data in the database file is encrypted
  * Uses good security practices (CBC mode, key derivation)
  * Zero-configuration and application level cryptography
  * Algorithms provided by the peer reviewed OpenSSL crypto library.
 .
 SQLCipher has broad platform support for with C/C++, Obj-C, QT,
 Win32/.NET/Mono, Java, Python, Ruby, Linux, Mac OS X, iPhone/iOS, Android,
 Xamarin.iOS, and Xamarin.Android.
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/debian/sqlcipher
Vcs-Git: https://salsa.debian.org/debian/sqlcipher.git
Testsuite: autopkgtest
Build-Depends: debhelper-compat (= 13), libreadline-dev | libeditreadline-dev, libssl-dev, chrpath, pkgconf, tcl8.6-dev
Package-List:
 libsqlcipher-dev deb libdevel optional arch=any
 libsqlcipher2 deb libs optional arch=any
 sqlcipher deb database optional arch=any
Checksums-Sha1:
 0c031a87cb5eb9793d461514cce77b578f5366f2 19019453 sqlcipher_4.13.0.orig.tar.gz
 609e5d9b031580859e453d1150a6b19e5b43bfd6 7936 sqlcipher_4.13.0-ok1.debian.tar.xz
Checksums-Sha256:
 7ca5c11f70e460d6537844185621d5b3d683a001e6bad223d15bdf8eff322efa 19019453 sqlcipher_4.13.0.orig.tar.gz
 4b1f15093515b1b7a05730507d9ca41551914fc71fb4339352d9d9b416b3814f 7936 sqlcipher_4.13.0-ok1.debian.tar.xz
Files:
 dda5d55f5cd3a7b4f42ff12108d86350 19019453 sqlcipher_4.13.0.orig.tar.gz
 42c39d84538b35c0f58d3c4f135d3bef 7936 sqlcipher_4.13.0-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmofwEEACgkQBxbwD4Qs
0nQAbgv+OrzaufUM7uUS7soq4WavTfL2GPvQlCSxVKHcqh/y8wwn/cpe1OvdEMpJ
2u1egCT2tEgxl8L01tqTxqW4rQYs/HUud/82Kq69yEQjOdek0ARk3CTZFzgmmT06
1+0HL5QbsMASNYzoJIKENS2ylO5cKyZb0utfYAVpPfZH+OAqWibSmXNabypMuPDz
ay/HDGOriJNtkKMb5J8jityHXaKrYdafmHnw6x9NJpzcdadU4/m18+Gfb5H0fFt3
yXeQGbx9gYGyMOzhFBjQiIW9MKyjiqCW4CTbCaeS0A2tefIJ2WWxzu/lm9lEZlI2
Z7oQpIaNuCfu2uUJyhLR1ab8aw13U7af17/vWEouwd/iSQFnu8vvhwp3ZLCE3Oa4
Bck+1qIJb8/EuHNbUKmD/K5CZ+gy0kh/+Zk7NbnFzLeKgnR17rwYtHZIjVbRLA6t
1OInCLhRelNAibPO12sGyqleMjPIXkgnlsQ4MzPoVOvxspEfPjU0T0pO802gep62
Wo4gZaUI
=21VA
-----END PGP SIGNATURE-----
