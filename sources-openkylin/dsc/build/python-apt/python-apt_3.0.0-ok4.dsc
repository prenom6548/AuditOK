-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: python-apt
Binary: python-apt-doc, python-apt-dev, python-apt-common, python3-apt
Architecture: any all
Version: 3.0.0-ok4
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Standards-Version: 4.5.0
Vcs-Browser: https://gitee.com/openkylin/python-apt
Vcs-Git: https://gitee.com/openkylin/python-apt.git
Testsuite: autopkgtest
Testsuite-Triggers: apt-utils, binutils, dirmngr, distro-info-data, fakeroot, gnupg, intltool, pycodestyle, pyflakes3, python3-all
Build-Depends: apt (>= 1.0.9.4), apt-utils <!nocheck>, debhelper-compat (= 12), dh-python, distro-info-data <!nocheck>, fakeroot <!nocheck>, libapt-pkg-dev (>= 2.7.11~), python3-all (>= 3.11), python3-all-dev (>= 3.3), python3-distutils-extra (>= 2.0), python3-setuptools, python3-sphinx (>= 0.5), gnupg <!nocheck>, dirmngr <!nocheck> | gnupg (<< 2) <!nocheck>, pycodestyle <!nocheck>, pyflakes3 <!nocheck>
Package-List:
 python-apt-common deb python optional arch=all
 python-apt-dev deb python optional arch=all
 python-apt-doc deb doc optional arch=all
 python3-apt deb python optional arch=any
Checksums-Sha1:
 c91033ee594c2c79ea05e238bf674021ce9b5804 415300 python-apt_3.0.0.orig.tar.gz
 f7287820457692ef778f54a91851f8a22fafd885 28732 python-apt_3.0.0-ok4.debian.tar.xz
Checksums-Sha256:
 a55b19fb3fa190feabaa579e2417eebd1323bcddb4050178370e36eb324efadc 415300 python-apt_3.0.0.orig.tar.gz
 fecba41a64c438ac3832444fd8e9830ceeca2c67565069e87b616b1d7ab40567 28732 python-apt_3.0.0-ok4.debian.tar.xz
Files:
 e7505db1e6761725929817f32a0d89c6 415300 python-apt_3.0.0.orig.tar.gz
 a57097257de1563736c291a1a7af54d6 28732 python-apt_3.0.0-ok4.debian.tar.xz
Original-Maintainer: APT Development Team <deity@lists.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmp5fz4ACgkQBxbwD4Qs
0nSrAwv+LKQG9h5Fjm6Ia+pNxhWeIIouSLM5psrX5hiyEnxXHJSOcicqfLPZhKq/
oud++jmEUbARUsNHrz3zbCvwh5KYiDvcIv+MAkDcw3b6D8AM+apRfuKbBtekDqDg
nJWuGi8x2eENzXHRiS/3rZH1VouNRwNJxVpSsAsV0l1iQLJlPIxIHpIfg8jcQ5F1
xUQDGLD4bim3/IvEyl4lN1GMbDK6Pj0ii2uN6yfzQ1z9w9bldwf8O3l3oiGJC+Fu
VpaD3NGVsPSCFm1jUupujiCYxdnu72sic1z4kiUT6gHP9OQhjWlZOjMdwqWEkXzf
z+QhA6fFxbbnMdtRUsM8clGm0KXj392KneINoqt1u8c7eFwNdn/ivwB5vOoZiuUX
xyxWcvcWBOcp/SZcHmLnaDeqDQ+wLGbfIT2pr5Zjf7nexrg7gjM1V3tjeV39J2Q2
IYTnz0eWluHrXjejZga73nx1YZqLqvgErFX1fynkjMCQMTJFBii3AxWkTEKDnr9+
n3l/yjhG
=LwMW
-----END PGP SIGNATURE-----
