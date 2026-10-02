-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: nftables
Binary: nftables, libnftables-dev, libnftables1, python3-nftables
Architecture: linux-any
Version: 1.1.6-ok1
Maintainer: Debian Netfilter Packaging Team <team+pkg-netfilter-team@tracker.debian.org>
Uploaders: Arturo Borrero Gonzalez <arturo@debian.org>, Jeremy Sowden <azazel@debian.org>
Homepage: https://www.netfilter.org/
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/pkg-netfilter-team/pkg-nftables
Vcs-Git: https://salsa.debian.org/pkg-netfilter-team/pkg-nftables.git
Testsuite: autopkgtest
Testsuite-Triggers: gawk, kmod, python3, systemd
Build-Depends: automake, debhelper-compat (= 13), dh-python, libeditreadline-dev, libgmp-dev, libjansson-dev, libmnl-dev, libnftnl-dev (>= 1.3.1), libpython3-all-dev, libtool, libxtables-dev, pybuild-plugin-pyproject, python3-all:any, python3-setuptools
Package-List:
 libnftables-dev deb libdevel optional arch=linux-any
 libnftables1 deb libs optional arch=linux-any
 nftables deb net important arch=linux-any
 python3-nftables deb python optional arch=linux-any
Checksums-Sha1:
 8446d54ccd914ad3abd99aff5a09a238fd0fe001 1025092 nftables_1.1.6.orig.tar.xz
 9c0c799b2776bb59bdb10b9c2bad91a2526e50c0 833 nftables_1.1.6.orig.tar.xz.asc
 cc88c85b9846911eba3b9a000e7f4404956b48d2 16544 nftables_1.1.6-ok1.debian.tar.xz
Checksums-Sha256:
 372931bda8556b310636a2f9020adc710f9bab66f47efe0ce90bff800ac2530c 1025092 nftables_1.1.6.orig.tar.xz
 2ee74c450487ea3ff8e80ff8605c38eb19057801190e794c5be29e75a457ed8d 833 nftables_1.1.6.orig.tar.xz.asc
 5239d43da2834009a9b9bb4203eb6ee85fc3a0daa79511f4dedd95faa5b897d0 16544 nftables_1.1.6-ok1.debian.tar.xz
Files:
 638ddff35ca429f68860a437d53237b4 1025092 nftables_1.1.6.orig.tar.xz
 bcb5d4a8e921931e2103490612839fa9 833 nftables_1.1.6.orig.tar.xz.asc
 7ceccf5f38b6863add44d25ff28cc924 16544 nftables_1.1.6-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnPcjAACgkQBxbwD4Qs
0nQaswwAkOIIgQEPyLG9jBpxMxqvHjYCR9HInY899R9jB7Ov1B/qG2wl033asV+c
r5BJVxv7nL62mXVInLucs7DEDzAbbnvaTYnGluWoKfrClH3/eT2IBZow0ImBr5I/
m3yhMlDeMcoQcCjtQ5bVQ+8jAXn7obp6B7t70nrhbkKjF/1KGjxbqRq5kHnADOPu
IcyuF+c3CdMQq5VdSlQGDxt/1Y32QZbtstjPvBntMO7yujadPBS7s6hr9osHzqMY
qS709+eZ9/QrRDssPEB581YOL4UYPlLkEMA3w3OwdnjX6gR4q28Xi2FeE1lw4aQm
VWf/sFuzT5ndUXmj7FoF87inWKCaNiWRbnfs8MQNM5KMaRDwhd/sYlDBLfV5hoyE
CS8pVpGsukpH8Q+UeQfT4p/Op/5slsMUaoDr38EEhgXhu+QAuE75Ags3L92vmg7/
0muyHtA8qFFf9MK32CHIQ3yPrJU8wLu3VjRs//JqSl5kHaMnjzhIM44iXtH7Y5LU
mXgXDHlF
=5/a6
-----END PGP SIGNATURE-----
