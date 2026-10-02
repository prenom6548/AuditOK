-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: cryptsetup
Binary: cryptsetup, cryptsetup-bin, cryptsetup-ssh, cryptsetup-initramfs, cryptsetup-suspend, libcryptsetup12, libcryptsetup-dev, cryptsetup-udeb, libcryptsetup12-udeb
Architecture: linux-any all
Version: 2:2.8.4-1ok10
Maintainer: Debian Cryptsetup Team <pkg-cryptsetup-devel@alioth-lists.debian.net>
Uploaders: Jonas Meurer <jonas@freesources.org>, Guilhem Moulin <guilhem@debian.org>
Homepage: https://gitlab.com/cryptsetup/cryptsetup
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/cryptsetup-team/cryptsetup
Vcs-Git: https://salsa.debian.org/cryptsetup-team/cryptsetup.git -b debian/latest
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, bsdextrautils, dmsetup, dosfstools, e2fsprogs, expect, fdisk, genext2fs, initramfs-tools-core, jq, keyutils, kmod, libjson-perl, linux-image-686-pae, linux-image-amd64, linux-image-arm64, linux-image-armmp-lpae, linux-image-generic, lvm2, mdadm, netcat-openbsd, openssh-client, openssh-server, openssl, openssl-provider-legacy, procps, qemu-efi-aarch64, qemu-efi-arm, qemu-system, qemu-system-arm, qemu-system-x86, sharutils, sshpass, swtpm, swtpm-tools, systemd-cryptsetup, tpm2-tools, udev, xxd, zstd
Build-Depends: asciidoctor <!nodoc>, autoconf, automake (>= 1:1.12), autopoint, debhelper-compat (= 13), dh-strip-nondeterminism, docbook-xml <!nodoc>, docbook-xsl <!nodoc>, gettext, jq <!nocheck>, libblkid-dev, libdevmapper-dev, libjson-c-dev, libpopt-dev, libselinux1-dev, libsepol-dev, libssh-dev, libssl-dev (>> 3.2~), libtool, openssl-provider-legacy <!nocheck>, pkgconf, po-debconf, procps <!nocheck>, uuid-dev, xsltproc <!nodoc>, xxd <!nocheck>
Package-List:
 cryptsetup deb admin unknown arch=linux-any
 cryptsetup-bin deb admin unknown arch=linux-any
 cryptsetup-initramfs deb admin unknown arch=all
 cryptsetup-ssh deb admin unknown arch=linux-any
 cryptsetup-suspend deb admin unknown arch=linux-any
 cryptsetup-udeb udeb debian-installer unknown arch=linux-any profile=!noudeb
 libcryptsetup-dev deb libdevel unknown arch=linux-any
 libcryptsetup12 deb libs unknown arch=linux-any
 libcryptsetup12-udeb udeb debian-installer unknown arch=linux-any profile=!noudeb
Checksums-Sha1:
 489b7955ed6aca3849d322d8bda15f382396cf37 11583080 cryptsetup_2.8.4-1ok10.tar.xz
Checksums-Sha256:
 1ba3fa4a67c6320f0f37d8076887f397ccfbaac3d6c4279553d2d059722767d7 11583080 cryptsetup_2.8.4-1ok10.tar.xz
Files:
 0669c1647b3c7ab9ac0e2ac0d27fda2a 11583080 cryptsetup_2.8.4-1ok10.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqOyjEACgkQBxbwD4Qs
0nR9Dgv/XOkXtcmJ71nQxHR7c05ropg8AHRjYAP+YX6meyn1A9nYwRYhd4RdXwH3
EeLJY2k2wH0iq55ZYB7GP2n9Q3kZds877B6szvOJ5OSCEC7md+oYu8uFaUTOfri5
Oy5IXROrAupNPLW4bxPObuZda2CAuqfkhWLSPT9iwEBvj5lkJvRevYS/CdjY/9pE
Bz8JomAknJ0xKWsYxdWm7+JkJ6LPsT53ciEBZdCoMZJM8JIfvg17ZDJ4EOrLwy8h
5SYSvYX4F/p/s4cvXg8NepKp7ZJ/j33VwiT9hmabx+9EVVSMetA9KbQj+WaBWgbI
gb9wmJzJvuJLdrvK0BxSqfwH/X6AqCFdKJsM1t0Q8wrZC9LjOzhL07SBuTqAz4bl
88nSsvAf9sqE5yLBJO+z5lSND4ocODeZZFzDM0VXoaJ3p6MSYp2J807T84Pyej9J
GvFK+155RQswxyvMgaVAIBZHvs2c9kRlRwng0InGTaSdCOgG4nWkTC3H7NEnKm5t
KYNIIiNQ
=pJII
-----END PGP SIGNATURE-----
