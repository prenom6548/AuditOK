-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: sudo
Binary: sudo, libnss-sudo
Architecture: any all
Version: 1.9.17p2-ok2
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Marc Haber <mh+debian-packages@zugschlus.de>, Hanno Wagner <wagner@debian.org>, Hilko Bengen <bengen@debian.org>, Bastian Blank <waldi@debian.org>
Homepage: https://www.sudo.ws/
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/sudo-team/sudo
Vcs-Git: https://salsa.debian.org/sudo-team/sudo.git
Testsuite: autopkgtest
Testsuite-Triggers: adduser, cron, ldap-utils, slapd, sssd-common, sssd-ldap
Build-Depends: debhelper-compat (= 13), dh-sequence-installnss
Build-Depends-Arch: autoconf, bison, flex, libapparmor-dev [linux-any], libaudit-dev [linux-any], libldap2-dev, libpam0g-dev, libsasl2-dev, libselinux1-dev [linux-any], pkgconf, po-debconf, systemd-dev, tzdata, zlib1g-dev
Package-List:
 libnss-sudo deb admin optional arch=all
 sudo deb admin optional arch=any
Checksums-Sha1:
 c3ce3ba633ceb44fdd304337d3720138e3656c5b 5451682 sudo_1.9.17p2.orig.tar.gz
 f3b5739880c98c60ba3bd23bcf236f3a9f69737f 566 sudo_1.9.17p2.orig.tar.gz.asc
 cb6be01e5a4612852f104d17e893f8ee882df810 36164 sudo_1.9.17p2-ok2.debian.tar.xz
Checksums-Sha256:
 4a38a1ab3adb1199257edc2a7c4a2bd714665eb605b04368843b06dada2cfcfb 5451682 sudo_1.9.17p2.orig.tar.gz
 4605f51763a083ab1732da1ae8b28a28ddf8007f4d79d0de51b691a9fe4195ec 566 sudo_1.9.17p2.orig.tar.gz.asc
 521a723655d2f030d6ae23c31fa0c6d43a8fe79f0450f23ad080548c70868c44 36164 sudo_1.9.17p2-ok2.debian.tar.xz
Files:
 dcbf46f739ae06b076e1a11cbb271a10 5451682 sudo_1.9.17p2.orig.tar.gz
 c3f05694e2a90a54c0a7000988df26ee 566 sudo_1.9.17p2.orig.tar.gz.asc
 595c3085a7b3d21a97adc44e84ea5abc 36164 sudo_1.9.17p2-ok2.debian.tar.xz
Original-Maintainer: Debian Sudo Maintainers <sudo@packages.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnUvGQACgkQBxbwD4Qs
0nS3hQwApMvGKKOSQNNUbEw4OOsXwx+EzYfeHvKr8HzAWfl8tuM77LvwR7LVwOrX
ZQWAkFzTpEGaiPfV+sbmGKY9XiltlDkAXBq2u3HA6famcBy5xjzBXeC3MQ0SHfgB
3fTvJ8yTE2UtVaB4eRdtqKLJgpTUfBjhNXb5frw6swhPRXpar2fB5E9rYJ7GqzKX
m3S4k1626+i7x5Vg+QSCR/eK3myIEch3q2vBYIe8jY6LqhWGp6YFpzvfSOqWRPoz
/oC56Hb/j8WuL9px/Rs19TxaKOLDMQSxgFGQFJGGU9s/oZrqpqWZa6AGor1on945
HQk+wNLot3noNZEQCpniaPeY8ViCFbc5TWgHiON+JiLNoPDaYf+xOR5/p5eM8iHj
KHBxRD6zeBJTUnbbeYGylHa2YKt5iOGUawZsJfS3/GOupqG+ZlffRDYv11nqYKWI
13jpgV3sNEOP74ZHahSfD1zPTKVTEh+qshypELhlTqhPC19S4kWeq6pDtSUPabKt
7wESMpsL
=fXpk
-----END PGP SIGNATURE-----
