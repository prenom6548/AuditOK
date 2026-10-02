-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: samba
Binary: samba, samba-libs, samba-common, samba-common-bin, samba-ad-dc, samba-ad-provision, smbclient, samba-testsuite, registry-tools, samba-dev, python3-samba, samba-dsdb-modules, samba-vfs-modules, libsmbclient, libsmbclient-dev, winbind, libpam-winbind, libnss-winbind, libwbclient0, libwbclient-dev, ctdb, libldb2, ldb-tools, libldb-dev, python3-ldb, python3-ldb-dev
Architecture: any all
Version: 2:4.19.2+dfsg-ok2~1112
Maintainer: Debian Samba Maintainers <pkg-samba-maint@lists.alioth.debian.org>
Uploaders: Steve Langasek <vorlon@debian.org>, Jelmer Vernooĳ <jelmer@debian.org>, Mathieu Parent <sathieu@debian.org>, Andrew Bartlett <abartlet+debian@catalyst.net.nz>, Michael Tokarev <mjt@tls.msk.ru>
Homepage: https://www.samba.org
Standards-Version: 4.6.2
Vcs-Browser: https://salsa.debian.org/samba-team/samba
Vcs-Git: https://salsa.debian.org/samba-team/samba.git
Testsuite: autopkgtest
Testsuite-Triggers: cifs-utils, coreutils, passwd, systemd
Build-Depends: debhelper-compat (= 13), dh-exec
Build-Depends-Arch: dh-sequence-python3, bison, docbook-xml, docbook-xsl, xsltproc, flex, perl:any, po-debconf, libtalloc-dev (>= 2.4.1~), python3-talloc-dev (>= 2.4.1~), libtevent-dev (>= 0.15.0~), libtdb-dev (>= 1.4.9~), python3-tdb (>= 1.4.9~), pkg-config, libacl1-dev, libarchive-dev, libavahi-client-dev, libavahi-common-dev, libblkid-dev, libbsd-dev, libcap-dev [linux-any], libcephfs-dev [amd64 arm64 mips64el ppc64el riscv64 s390x], librados-dev [amd64 arm64 mips64el ppc64el riscv64 s390x], libcmocka-dev (>= 1.1.3), libcups2-dev, libdbus-1-dev, libglusterfs-dev [amd64 arm64 ppc64el ppc64 riscv64 mips64el s390x ia64 sparc64], libgnutls28-dev, libgpgme11-dev, libicu-dev, libjansson-dev, libjson-perl, libkeyutils-dev, libkrb5-dev (>= 1.20.0~) <pkg.samba.mitkrb5>, libldap2-dev, liblmdb-dev, libpam0g-dev, libparse-yapp-perl, libpcap-dev [hurd-i386 kfreebsd-any], libpopt-dev, libreadline-dev, libsystemd-dev [linux-any], libtasn1-6-dev (>= 3.8), libtasn1-bin, liburing-dev [linux-any] <!pkg.samba.nouring>, xfslibs-dev [linux-any], zlib1g-dev (>= 1:1.2.3), python3-dev:any | python3-dev, libpython3-dev, python3-dnspython, python3-etcd, python3-markdown
Package-List:
 ctdb deb net optional arch=any
 ldb-tools deb utils optional arch=any
 libldb-dev deb libdevel optional arch=any
 libldb2 deb libs optional arch=any
 libnss-winbind deb admin optional arch=any
 libpam-winbind deb admin optional arch=any
 libsmbclient deb libs optional arch=any
 libsmbclient-dev deb libdevel optional arch=any
 libwbclient-dev deb libdevel optional arch=any
 libwbclient0 deb libs optional arch=any
 python3-ldb deb python optional arch=any
 python3-ldb-dev deb libdevel optional arch=any
 python3-samba deb python optional arch=any
 registry-tools deb net optional arch=any
 samba deb net optional arch=any
 samba-ad-dc deb net optional arch=all
 samba-ad-provision deb net optional arch=all
 samba-common deb net optional arch=all
 samba-common-bin deb net optional arch=any
 samba-dev deb devel optional arch=any
 samba-dsdb-modules deb libs optional arch=any
 samba-libs deb libs optional arch=any
 samba-testsuite deb net optional arch=any
 samba-vfs-modules deb net optional arch=any
 smbclient deb net optional arch=any
 winbind deb net optional arch=any
Checksums-Sha1:
 5b88c3ade9085970a83d9fb5d72bc3fbfc4c7564 24549620 samba_4.19.2+dfsg-ok2~1112.tar.xz
Checksums-Sha256:
 ac08c3685dee96d94e931f15df2b9116a6185e1252e52b8d9d39355ac915f067 24549620 samba_4.19.2+dfsg-ok2~1112.tar.xz
Files:
 bf5d853ef8ed6de559d8c6e0861b51ae 24549620 samba_4.19.2+dfsg-ok2~1112.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmkT6tYACgkQBxbwD4Qs
0nSPrQv/cwoVu6Jx4pl5Bz3HT3H9QEqvkTGRr+SrtcfZQAfPPpY3eqPAbd5m4EjU
09cs86CSKauzAeA1+d/dCxy/LcwJBMg8Hs4z/Zzk6IT819WatCR26F4v1Qus3rBO
LIluDnZoWxCQIlpds25QwKnhSM4Y/kD7Rr3yRGZIAgYQJ0MomyNS/seM0eYd7E3N
A6k6CoM0UJGEBnZJE4cTMrGpJRY8vaqI6/WbqwPN4BUqWhqj0czN5WU7TdI3sKxK
6ZAVNo+m9y/pH1sLAZIZeytDjDYc/ZR9bhzHGnJkW3bXx/5LIGRZcTLZPE/ecyN+
iHvuGeHPTLXAj1IrsDDMocTNXuVHobIo0TlmuuhXW5CLypMLqZtaU5Lr+fi5JEh3
FM62DSgsHyk9/xA4n8yIaEZg0jkUHF3JwEw7AgY47DN1L2oKqe9FaOT04/CkF7ja
8gXDqalmeTTb5JC5bhB1ZObiX6zIaBp/QUJSngQrMINM+g7UMYYEX4P7ZD3GFBmb
B/m+AqMq
=7175
-----END PGP SIGNATURE-----
