-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: kmod
Binary: kmod, libkmod2, libkmod-dev, kmod-udeb, libkmod2-udeb
Architecture: linux-any
Version: 34.2-2-ok2
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Standards-Version: 4.7.2.0
Vcs-Browser: https://salsa.debian.org/md/kmod
Vcs-Git: https://salsa.debian.org/md/kmod.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, pkgconf
Build-Depends: debhelper-compat (= 13), meson, pahole <!nocheck>, pkgconf, liblzma-dev (>= 5.4.5-0.2), libssl-dev, libzstd-dev, zstd <!nocheck>, linux-headers-generic [!armel !i386 !mips64el !sh4 !x32] <!nocheck>, linux-headers-mips64r6el [mips64el] <!nocheck>, linux-headers-rpi [armel] <!nocheck>, linux-headers-sh7785lcr [sh4] <!nocheck>, gtk-doc-tools <!nodoc>, scdoc
Package-List:
 kmod deb admin important arch=linux-any
 kmod-udeb udeb debian-installer important arch=linux-any profile=!noudeb
 libkmod-dev deb libdevel optional arch=linux-any
 libkmod2 deb libs optional arch=linux-any
 libkmod2-udeb udeb debian-installer important arch=linux-any profile=!noudeb
Checksums-Sha1:
 52785be951db4aff9eccc3a7d791d62521c6a336 192484 kmod_34.2-2.orig.tar.xz
 a791f0c1c32afcc6c51ea8cc79940c884428a48e 18152 kmod_34.2-2-ok2.debian.tar.xz
Checksums-Sha256:
 d6c5524610d35c5bf7e5b66441149734a13a64f4d7dca6872d26ec116ed7261c 192484 kmod_34.2-2.orig.tar.xz
 e4c2a772eea3add1d86e2b37928c065da6b12313d6a3266f44029a9f79bbd97d 18152 kmod_34.2-2-ok2.debian.tar.xz
Files:
 20689297dbc92f73221d4e556e5c467f 192484 kmod_34.2-2.orig.tar.xz
 93f4eda354fb61b5ad3527c29d54c21f 18152 kmod_34.2-2-ok2.debian.tar.xz
Original-Maintainer: Marco d'Itri <md@linux.it>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnh5uYACgkQBxbwD4Qs
0nTxYAwAu7PXDnqWil3Cse72SaW9+m97Q+sESH/9WHtlOn2yrLiTe/lEjwjNwXKN
Dno7ckttlvfqP9TTbmwV7YC5j7JceOIUqP1dLxsJgZci/1hz+vjpguVqvownpPSz
foYuJI0Hi7KLhhFdpvrS8an/NwWs4aApC3bAExoRAYOU7XRPt+SPX/delzRljX0R
urOqbyQJVu3RejCgmsZx2KL8COEwrx1ihSljlh3BoJVr08imIv1Ui/zSOBSzxZLz
dVHGIR5gx3h0EMA2CI8m6f0K/vUyHTjofr/YlCPs6zmweys5jkmyCf4EXVaY0iZg
hWPo7agI3ofL8oAxuTZ61TylRI8xy2QO2uRHpsMC4Q9DJjC6IuT0lzW0Kd+o9Va8
JFINoXCdwELy22GiP97EaLVrscl5JA1/Ze/o9ctwEHLaIkcs2NkNromgxkyUUgvz
ars5OGaz9lDiVzb3noHNVABQwDeTefknQFQ1aL1QVkeiGV8LhAxtJZNtyAS/7zxr
dMfAkGF+
=oJKk
-----END PGP SIGNATURE-----
