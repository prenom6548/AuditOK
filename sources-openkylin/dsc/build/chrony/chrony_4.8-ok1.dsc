-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: chrony
Binary: chrony
Architecture: linux-any
Version: 4.8-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://chrony-project.org
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/debian/chrony
Vcs-Git: https://salsa.debian.org/debian/chrony.git -b debian/latest
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, build-essential, ca-certificates, dpkg-dev, ethtool, gnutls-bin, iproute2, isc-dhcp-client, isc-dhcp-server, kmod, wget
Build-Depends: asciidoctor, bison, debhelper-compat (= 13), dh-apparmor, dh-sequence-installsysusers, gnutls-bin <!nocheck>, iproute2 [linux-any] <!nocheck>, libcap-dev [linux-any], libedit-dev, libgnutls28-dev, libseccomp-dev (>= 2.5.5-2~) [amd64 arm64 armel armhf hppa i386 loong64 mips mipsel mips64el powerpc powerpcspe ppc64 ppc64el riscv64 s390x x32], nettle-dev, pkgconf, pps-tools [linux-any], procps <!nocheck>
Package-List:
 chrony deb net unknown arch=linux-any
Checksums-Sha1:
 3dd11b96147dbea4f46209b608edac4cc8373f42 649577 chrony_4.8.orig.tar.gz
 50c5c51aaab96bbce75634a175e0cca78e0097bd 26292 chrony_4.8-ok1.debian.tar.xz
Checksums-Sha256:
 739a930d9370bc0ad7722446e34f374c0cb2838e4a890152799e79c66652ed18 649577 chrony_4.8.orig.tar.gz
 d977ae79905cc5f19ceaf61bc5cf990f15b4d0cc6522f7fda6b5de84636aa586 26292 chrony_4.8-ok1.debian.tar.xz
Files:
 9e2cd75c904737ca913782c9173c3b3d 649577 chrony_4.8.orig.tar.gz
 b95a43789f0f44e664317210e32fa337 26292 chrony_4.8-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnOIGYACgkQBxbwD4Qs
0nTfbAwApgiNfy4j6WgOIhMpRbC5+7yKzKBI7kAU9KfS+355WACpYa9ZvaIxu9cc
bG95IgWFrYFaPe5B5puWw0uviCeeW0gP73LwAcELAZL5qxw7VkFD3rhZ06Jkc/io
u/f9KznzRpfLDQJyrAGu9OHeea07PM2muICFkGbsBAfogzrZlngKJhCg1SDG/Sc3
UaGSF+Qbw0FyCwWElofYlxfEUbxVc1jkO3QBCDOjjRcDskVCPLa3SyzH22jlGtLW
+33/tTyBxNqugse+fN47f5f2mGj07RfgZnpkLLaIjOI/PSM03B0Pw/aIevFs9FGm
QBc03BsTbhOgiTLZAZYcUdG+uy6b5W2jiEnXKhN7hoqKHsEDq9nfswztkgWH8HbO
0gqxciJOrVXJJTLgHaYnih49W10dYth9Q7X6PPfFRm6rrSG5+V/obnAA+VnuOJbt
Rkf0hmejiuVcRRwOx3AvIgSFt/0228V/bSsqWT+kgnVYojxy0TUGX/EvLbio7OZP
U3/umhEq
=9wpw
-----END PGP SIGNATURE-----
