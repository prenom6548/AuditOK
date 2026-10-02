-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: iptables
Binary: iptables, libxtables12, libxtables-dev, libiptc-dev, libip4tc2, libip4tc-dev, libip6tc2, libip6tc-dev
Architecture: linux-any
Version: 1.8.11-ok3
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Arturo Borrero Gonzalez <arturo@debian.org>, Alberto Molina Coballes <alb.molina@gmail.com>, Jeremy Sowden <azazel@debian.org>
Homepage: https://www.netfilter.org/
Standards-Version: 4.7.0
Vcs-Browser: https://salsa.debian.org/pkg-netfilter-team/pkg-iptables
Vcs-Git: https://salsa.debian.org/pkg-netfilter-team/pkg-iptables.git
Testsuite: autopkgtest
Testsuite-Triggers: python3
Build-Depends: debhelper-compat (= 13), libmnl-dev, libnetfilter-conntrack-dev, libnfnetlink-dev, libnftnl-dev (>= 1.1.6), netbase (>= 6.0), python3:native <!nocheck>
Package-List:
 iptables deb net optional arch=linux-any
 libip4tc-dev deb libdevel optional arch=linux-any
 libip4tc2 deb libs optional arch=linux-any
 libip6tc-dev deb libdevel optional arch=linux-any
 libip6tc2 deb libs optional arch=linux-any
 libiptc-dev deb libdevel optional arch=linux-any
 libxtables-dev deb libdevel optional arch=linux-any
 libxtables12 deb libs optional arch=linux-any
Checksums-Sha1:
 e5b10a1a4ebc850631d93236ae88cac207e1d7f8 649284 iptables_1.8.11.orig.tar.xz
 b629ad93f19427d6db41508eb4cec9cd4189796c 833 iptables_1.8.11.orig.tar.xz.asc
 5cda8c924fa00178ed1e714c8b3ace14a760d81a 15104 iptables_1.8.11-ok3.debian.tar.xz
Checksums-Sha256:
 d87303d55ef8c92bcad4dd3f978b26d272013642b029425775f5bad1009fe7b2 649284 iptables_1.8.11.orig.tar.xz
 bb1775b8a1b978ef96c0bde2e597d9f375848b38bc25143a91357e61e5d19144 833 iptables_1.8.11.orig.tar.xz.asc
 9f1e7e64eee6009672013d802f467d8f3e15361d73361a4efae59e498513a73e 15104 iptables_1.8.11-ok3.debian.tar.xz
Files:
 44191ce12055da9790a92429c771ba50 649284 iptables_1.8.11.orig.tar.xz
 b31298ccaff7c04b1d39881221c5d0ea 833 iptables_1.8.11.orig.tar.xz.asc
 1923dc805355ccdc5aa7170633f528a0 15104 iptables_1.8.11-ok3.debian.tar.xz
Original-Maintainer: Debian Netfilter Packaging Team <pkg-netfilter-team@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmngSecACgkQBxbwD4Qs
0nTEjwwAgACqkWN6k1K1C6Sjxf0wgPuQ5fHmx0zFBfAH5uvdOJy8FDC90dLOA5Zn
9eGRWUAGy8TVXEw1dvPYWDslvhOW8ZURD02BXTUepzLQeETl2MVtfIqf/MKJsEcN
IbYCHFWnRkkE4cNZBqQQMWk2AptzBG4nQXAFL+uN8cX+EfgZwNnS+xC784Dj0ZHy
JqTpjs2EhjDcaYTPEOCYSxI6swSSXvejGefkoWQ3Q+5FA1zIpjZcGO3m5yLr2ke5
DqaXUUgprzzrDiAz8+FtZwM9skUWTvej+Z7ZtR20deQdC/dqsE6VCZGGal51NW8H
zMliYVm6xqyqczKM5mqjA+ZFpdougSrKuuKFzYjATVjE+fxehPT1KTzDbuDPqA/I
pW8uJuzsu4Qhu0hR6/XjlkrVjQH2gWiV3uxvANSiNenlLw7rUPCbja4JRgyMrQym
FsjqhiBzScc9QMIBRWRICpgBicRnOe9qCnAl1Hzz22/l2AB21+274P4adYAGJ/uA
BIjYPPns
=I/Aw
-----END PGP SIGNATURE-----
