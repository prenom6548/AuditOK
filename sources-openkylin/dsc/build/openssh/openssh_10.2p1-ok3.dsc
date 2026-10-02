-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: openssh
Binary: openssh-client, openssh-client-gssapi, openssh-server, openssh-server-gssapi, openssh-sftp-server, openssh-tests, ssh, ssh-askpass-gnome, openssh-client-udeb, openssh-server-udeb
Architecture: any all
Version: 1:10.2p1-ok3
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders:  Colin Watson <cjwatson@debian.org>, Matthew Vernon <matthew@debian.org>,
Homepage: https://www.openssh.com/
Standards-Version: 4.6.2
Vcs-Browser: https://salsa.debian.org/ssh-team/openssh
Vcs-Git: https://salsa.debian.org/ssh-team/openssh.git
Testsuite: autopkgtest
Testsuite-Triggers: adduser, devscripts, dropbear, haveged, krb5-admin-server, krb5-kdc, openssl, putty-tools, python3-twisted, sudo, systemd, sysvinit-utils, xinetd
Build-Depends: debhelper (>= 13.1~), debhelper-compat (= 13), dh-exec, dh-runit (>= 2.8.8), dh-sequence-movetousr, libaudit-dev [linux-any], libedit-dev, libfido2-dev (>= 1.5.0) [linux-any], libgtk-3-dev <!pkg.openssh.nognome>, libkrb5-dev | heimdal-dev, libpam0g-dev | libpam-dev, libselinux1-dev [linux-any], libssl-dev (>= 1.1.1), libwrap0-dev | libwrap-dev, pkgconf, zlib1g-dev, systemd-dev
Package-List:
 openssh-client deb net standard arch=any
 openssh-client-gssapi deb net optional arch=all
 openssh-client-udeb udeb debian-installer optional arch=any profile=!noudeb
 openssh-server deb net optional arch=any
 openssh-server-gssapi deb net optional arch=all
 openssh-server-udeb udeb debian-installer optional arch=any profile=!noudeb
 openssh-sftp-server deb net optional arch=any
 openssh-tests deb net optional arch=any
 ssh deb net optional arch=all
 ssh-askpass-gnome deb gnome optional arch=any profile=!pkg.openssh.nognome
Checksums-Sha1:
 ccb5f0f873aafe94b4466b14214dae4b5b9752cc 1530128 openssh_10.2p1-ok3.tar.xz
Checksums-Sha256:
 730cfe21858b183a72fc2d5ecd9d4a1ec98a4442be92a6b1a9a995b89b9a0576 1530128 openssh_10.2p1-ok3.tar.xz
Files:
 2d4be71a536aee26fe8233dcf1b7e581 1530128 openssh_10.2p1-ok3.tar.xz
Original-Maintainer: Debian OpenSSH Maintainers <debian-ssh@lists.debian.org>

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEEFM4holeXTubEy6bpzRDvoMIUgegFAmnoKDwACgkQzRDvoMIU
gehV5hAApD87Nx8LQTun6H4VGRf2WT9EQg1pFbNQjmX1n8VO9zMFxzImDgp+gSjM
znJvQUANnaBUNkrDPKGkbADK74UpKJtQomUYZYzH2L9q8drJcOPiKfF3JVwyrgUl
ZD6DRU8A6cBBpV+XtzBP1h3z3xaT+5sfh8+2VQ0d4p4pccM76mibgXzoiUeFjMaQ
mMQcDAaa5L9iHFoMF2gRPVQ/U+Txistkjtg3TG8Kv7HzdTW8LCbtjO7IE5bYCKYB
p+rJTSMELqk8wGCqOwDVf3Ss4GGmzNWunDAMjbv3EUQOQKXNeHdBzOafWEEpSPf6
K5jitKgX2fF/eFmX95zfynZg6I3v5fXkcVGLVPvBHUKeSIBS5CJwILwv4+foeP4H
waV6Dnv6+/DqnTx9MVHU1D/jNbFJskXF/glE8x5kvwESY9SawQOOOyYJc7Tmweb5
pusQkIMB0GRroez4YwDZthPDhgioP8O3Lo7lXZB78L1UukrlIVLoCe3SgvDTreL0
KbFPety7WPke345w5oQzSR8D3ajHjGgPtJd1k2obREWbshMYW6LVFTANPHO6Dzn0
vNYssSbqzG9gRavN0CTr6M+AwjwMFK9fSvw+g8qPAgO4HraiyO0pfr7Ls08nVjiW
YJR8D5Ut/7C8rRz+b58bqUSEcQlm/9Yj+RsEknS68EgsZWoSbD8=
=pqlZ
-----END PGP SIGNATURE-----
