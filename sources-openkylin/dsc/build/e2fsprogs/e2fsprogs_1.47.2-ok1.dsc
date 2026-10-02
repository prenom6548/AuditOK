-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: e2fsprogs
Binary: fuse2fs, fuseext2, logsave, e2fsck-static, e2fsprogs-l10n, libcom-err2, comerr-dev, libss2, ss-dev, e2fsprogs-udeb, libext2fs2t64, libext2fs-dev, e2fsprogs
Architecture: any all
Version: 1.47.2-ok1
Maintainer: Theodore Y. Ts'o <tytso@mit.edu>
Homepage: http://e2fsprogs.sourceforge.net
Standards-Version: 4.7.0
Vcs-Browser: https://git.kernel.org/pub/scm/fs/ext2/e2fsprogs.git
Vcs-Git: https://git.kernel.org/pub/scm/fs/ext2/e2fsprogs.git -b debian/master
Testsuite: autopkgtest
Testsuite-Triggers: fuse3
Build-Depends: dpkg-dev (>= 1.22.5), gettext, texinfo, pkgconf, libarchive-dev <!nocheck>, libfuse3-dev [linux-any kfreebsd-any] <!pkg.e2fsprogs.no-fuse2fs>, debhelper-compat (= 12), dh-exec, libblkid-dev, uuid-dev, m4, udev [linux-any], systemd [linux-any], systemd-dev [linux-any], cron [linux-any], dh-sequence-movetousr
Package-List:
 comerr-dev deb libdevel optional arch=any
 e2fsck-static deb admin optional arch=any profile=!pkg.e2fsprogs.no-static
 e2fsprogs deb admin important arch=any
 e2fsprogs-l10n deb localization optional arch=all
 e2fsprogs-udeb udeb debian-installer optional arch=any profile=!noudeb
 fuse2fs deb admin optional arch=linux-any,kfreebsd-any profile=!pkg.e2fsprogs.no-fuse2fs
 fuseext2 deb oldlibs optional arch=all profile=!pkg.e2fsprogs.no-fuse2fs
 libcom-err2 deb libs optional arch=any
 libext2fs-dev deb libdevel optional arch=any
 libext2fs2t64 deb libs optional arch=any
 libss2 deb libs optional arch=any
 logsave deb admin optional arch=any
 ss-dev deb libdevel optional arch=any
Checksums-Sha1:
 2197a81512f3d463f450a57832798006ca920b8e 9996725 e2fsprogs_1.47.2.orig.tar.gz
 735a4878a0e81c275808fe373a7ec67c8b7b5d99 488 e2fsprogs_1.47.2.orig.tar.gz.asc
 a82d5adbd023adfaf03bea2fe62ad1c55b4d6ca2 105432 e2fsprogs_1.47.2-ok1.debian.tar.xz
Checksums-Sha256:
 6dcd67ff9d8b13274ba3f088e4318be4f5b71412cd863524423fc49d39a6371f 9996725 e2fsprogs_1.47.2.orig.tar.gz
 2063f62a198dd3df21f789c990c2cf9b4a5de24ed55f0b78d86e97e98daffc9d 488 e2fsprogs_1.47.2.orig.tar.gz.asc
 9e7ab9efa3178ffc76629ef8d1ac822ac87c3238dda9abab9a6cadf0fe8a7b10 105432 e2fsprogs_1.47.2-ok1.debian.tar.xz
Files:
 752e5a3ce19aea060d8a203f2fae9baa 9996725 e2fsprogs_1.47.2.orig.tar.gz
 29fd2ebec43fedc5c4f3729af5788c79 488 e2fsprogs_1.47.2.orig.tar.gz.asc
 9a2e52e2bd7808c4220ffefe61697fa3 105432 e2fsprogs_1.47.2-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmniBj8ACgkQBxbwD4Qs
0nRjsQwAl4mi197GgMvZHCdsox99ekZ/gg5xvu+VS2jGIGHHzJ41ejSA5guYqR2g
tPwrKxKG/unYhnp5DiFe/CO4NzJAAHwaY9n6wWz9nxfZ6NlLomOusALNvL+pndB1
rpbjjCXLu7A9WLnsXz8wFRZXsuca8MdUdPUthbAhTX9qm27CJqIvGsV0fLDEW0RH
sgbrnrKheX5A6ZU9Dujxcq4uHswAx9IzfeARsNpSznEkOHYkklyagD9fFVE5xs6H
5ZCBG59vW9wyYhNfWCEOT8rHqqbe9Q0gNUp8iaF3CY/mqj1YnOpCcNvGFqUDNH5W
RGBaDJs8eM+lkfOfEETn4MVsJxIefr+q9U4gMJqJLEkG7TBOaIPPOSVyvEWRnX2h
1mi6+iyAXoJUNmrRpBlyNYVPmPxIs6MpA6RAErx0L+Zb3n2D3jiDIHFfNrXiKT/z
eCEeo0ZImT30KV/s6UgKvk7Ly7ndB0wEpkWgCt+di+HGWk6AWLfsSR1vOIr/Xsa/
kHNONqL4
=zVuU
-----END PGP SIGNATURE-----
