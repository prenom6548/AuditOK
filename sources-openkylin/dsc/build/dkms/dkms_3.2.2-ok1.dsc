-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: dkms
Binary: dkms, dh-dkms, dkms-test-dkms, dkms-noautoinstall-test-dkms, dkms-replace-test-dkms
Architecture: all
Version: 3.2.2-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: David Paleino <dapal@debian.org>, Petter Reinholdtsen <pere@debian.org>, Aron Xu <aron@debian.org>, Andreas Beckmann <anbe@debian.org>,
Homepage: https://github.com/dell/dkms
Standards-Version: 4.7.2
Vcs-Browser: https://salsa.debian.org/debian/dkms
Vcs-Git: https://salsa.debian.org/debian/dkms.git
Testsuite: autopkgtest, autopkgtest-pkg-dkms
Testsuite-Triggers: linux-headers-generic, linux-headers-rpi, linux-image-generic, openssl
Build-Depends: debhelper-compat (= 13)
Package-List:
 dh-dkms deb kernel optional arch=all
 dkms deb kernel optional arch=all
 dkms-noautoinstall-test-dkms deb kernel optional arch=all
 dkms-replace-test-dkms deb kernel optional arch=all
 dkms-test-dkms deb kernel optional arch=all
Checksums-Sha1:
 72f49d5ddcc970507871c62a74027aaeccea1a71 110268 dkms_3.2.2.orig.tar.gz
 468c7cc8b326b7ee11994c49449a880fe035e39a 14968 dkms_3.2.2-ok1.debian.tar.xz
Checksums-Sha256:
 23e40a2b2e60047002727425e2307617d9899e51bfe22306a2c874259ed9605a 110268 dkms_3.2.2.orig.tar.gz
 1a9a9bbfc5bfc9a27e4988c83aa6b599c5848c6a6168c85929e18e87cf83e35b 14968 dkms_3.2.2-ok1.debian.tar.xz
Files:
 41ce8cf99e97cfd84842ff833e7c68d8 110268 dkms_3.2.2.orig.tar.gz
 9dd646e87180c12a771375c33ae76a74 14968 dkms_3.2.2-ok1.debian.tar.xz
Original-Maintainer: Dynamic Kernel Module System Team <dkms@packages.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmodRKUACgkQBxbwD4Qs
0nQwbAv+P1A+4KWVdbghF7NcPnNpkT2zJRknLqrRo5rAllWZ+6g2WennEWPdBBko
uxn+5Ti9ZCfBD+ITZwkawSwIIdydrZusor+gw1YWunnd3nMcabRATD/DX1KQlfDd
o/AF9YqWMm4+ga17kmn20YaNniGv0ccUdpupfbDP9eS9gRbehCc/mbmoJkEQZCIz
D5eoUJ4QNIpJ/e2KTtGmASMjVhqH9i/xbRNze2YYZcNcuU+I5nl+jHbsiAocbDRB
YZWgNbvvuVc7FweTdHP06y2R2DSwfm35JRHY2+tgsZzdRAskjbPwGpIe4QayiTGQ
++3gtboggVVfL4Ubzo+SVEluZXmbbJT/9xo5VlPBqLxbvCkUw6ixsrdJwob18/zR
DYniDAyWFR4g1+ddisyXLMHexcldWKJRnrEOQMbcOFLcOHviyZ3grAifp/TAW/TI
QmWJ4U7KjzAfb/nThRSle0dbbbISfdNmynsXf2nRVzKkysjmmN+/M+VmyFXUuFbS
K/+Emc9q
=IHMp
-----END PGP SIGNATURE-----
