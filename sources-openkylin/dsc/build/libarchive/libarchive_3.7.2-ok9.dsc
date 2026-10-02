-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: libarchive
Binary: libarchive-dev, libarchive13t64, libarchive-tools
Architecture: any
Version: 3.7.2-ok9
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://www.libarchive.org/
Standards-Version: 4.7.3
Vcs-Browser: https://gitee.com/openkylin/libarchive
Vcs-Git: https://gitee.com/openkylin/libarchive.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, file, pkg-config
Build-Depends: debhelper-compat (= 13), dpkg-build-api (= 1), dpkg-dev (>= 1.22.5), dh-package-notes, pkgconf, libbz2-dev, liblz4-dev, liblzma-dev, libxml2-dev, libzstd-dev, zlib1g-dev, libacl1-dev [!hurd-any], libext2fs-dev, sharutils <!nocheck>, nettle-dev, locales <!nocheck> | locales-all <!nocheck>
Package-List:
 libarchive-dev deb libdevel optional arch=any
 libarchive-tools deb utils optional arch=any
 libarchive13t64 deb libs optional arch=any
Checksums-Sha1:
 886ccb6828427fc5c38f8cbde3e966ea72a4b84f 4391656 libarchive_3.7.2-ok9.tar.xz
Checksums-Sha256:
 cd3a85eea5480a0e9b273a7613ba6c2d3aff8fd9956c1702f32a7f8bb51e1b22 4391656 libarchive_3.7.2-ok9.tar.xz
Files:
 017438deb264bfe357d70bb11331d80d 4391656 libarchive_3.7.2-ok9.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnoI88ACgkQBxbwD4Qs
0nRfLAwArxuR9Oqv2fG7WhjaFJnn803+4vx3ttyfVZNxquhoLtcjWJi62uVngcLO
5JIQsQsZfuMdHKWMGsHXAlbXqkMrvY/W7PvwvQLBbunIjH7tMuJPWwcr8NPRoH/m
vHh3wy+a4p1I9udusqLSkt4JxE/CHwXPTN1raUvsfYk4b/+IfU/SJLI8k3zdmFxA
4LIUQWXV4ieQxl8k77N1uM7pvIBq6g5sYUo5f9rl+dpVJoPP8xUoSBN0Z2DtB795
fkfe5oarzBT9DpTZxhZtGy4/Y86NIUw/r71mmHxt8TSCRYgeuJRuVnS45zdCTRCI
G+o+91SLupkhhblTBTxR73+vV2oMjellphMa6TTKha6vZAhYSMjyynMChkGyOX07
4E24gk4FstZ2nrFhWFYLy7lfEDCxx0Fy6KigUMTYehma9UDNYxhIH6K0Arucequy
vATqiatTKzHGVtQD9IxXq1leH3nELET/bmwUzayB81R5426YrQNNNQAu9egasFSm
cbMyxF4Q
=UBCO
-----END PGP SIGNATURE-----
