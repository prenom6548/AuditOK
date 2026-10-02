-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: avahi
Binary: avahi-daemon, avahi-dnsconfd, avahi-autoipd, python3-avahi, avahi-utils, avahi-discover, libavahi-common3, libavahi-common-data, libavahi-common-dev, libavahi-core7, libavahi-core-dev, libavahi-client3, libavahi-client-dev, libavahi-glib1, libavahi-glib-dev, libavahi-gobject0, libavahi-gobject-dev, libavahi-compat-libdnssd1, libavahi-compat-libdnssd-dev, libavahi-ui-gtk3-0, libavahi-ui-gtk3-dev, avahi-ui-utils, gir1.2-avahi-0.6
Architecture: any all
Version: 0.8-ok2
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: Sjoerd Simons <sjoerd@debian.org>, Sebastian Dröge <slomo@debian.org>, Loic Minier <lool@dooz.org>, Michael Biebl <biebl@debian.org>
Homepage: https://avahi.org/
Standards-Version: 4.6.2
Vcs-Browser: https://salsa.debian.org/utopia-team/avahi
Vcs-Git: https://salsa.debian.org/utopia-team/avahi.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, pkgconf
Build-Depends: debhelper (>= 13.11.6~), debhelper-compat (= 13), dh-sequence-gir <!nogir>, dh-sequence-python3, intltool, libcap-dev [linux-any], libdaemon-dev, libdbus-1-dev, libexpat-dev, libgdbm-dev, libgirepository1.0-dev <!nogir>, libglib2.0-dev, libgtk-3-dev <!pkg.avahi.nogui>, pkgconf, python-gi-dev <!nopython>, python3, python3-dbus <!nopython>, python3-gdbm, python3-gi <!nopython>, xmltoman
Package-List:
 avahi-autoipd deb net optional arch=linux-any
 avahi-daemon deb net optional arch=any
 avahi-discover deb net optional arch=all profile=!nopython,!pkg.avahi.nogui
 avahi-dnsconfd deb net optional arch=any
 avahi-ui-utils deb utils optional arch=any profile=!pkg.avahi.nogui
 avahi-utils deb net optional arch=any
 gir1.2-avahi-0.6 deb introspection optional arch=any profile=!nogir
 libavahi-client-dev deb libdevel optional arch=any
 libavahi-client3 deb libs optional arch=any
 libavahi-common-data deb libs optional arch=any
 libavahi-common-dev deb libdevel optional arch=any
 libavahi-common3 deb libs optional arch=any
 libavahi-compat-libdnssd-dev deb libdevel optional arch=any
 libavahi-compat-libdnssd1 deb libs optional arch=any
 libavahi-core-dev deb libdevel optional arch=any
 libavahi-core7 deb libs optional arch=any
 libavahi-glib-dev deb libdevel optional arch=any
 libavahi-glib1 deb libs optional arch=any
 libavahi-gobject-dev deb libdevel optional arch=any profile=!nogir
 libavahi-gobject0 deb libs optional arch=any
 libavahi-ui-gtk3-0 deb libs optional arch=any profile=!pkg.avahi.nogui
 libavahi-ui-gtk3-dev deb libdevel optional arch=any profile=!pkg.avahi.nogui
 python3-avahi deb python optional arch=any profile=!nopython
Checksums-Sha1:
 969a50ae18c8d8e2288435a75666dd076e69852a 1591458 avahi_0.8.orig.tar.gz
 cbd74e7d3c74865b37632424b4c5240718871b32 26008 avahi_0.8-ok2.debian.tar.xz
Checksums-Sha256:
 060309d7a333d38d951bc27598c677af1796934dbd98e1024e7ad8de798fedda 1591458 avahi_0.8.orig.tar.gz
 f58d0a738660dde21e8a6a9f6c30465f92e86d1e54e9d08413ec402d1150f931 26008 avahi_0.8-ok2.debian.tar.xz
Files:
 229c6aa30674fc43c202b22c5f8c2be7 1591458 avahi_0.8.orig.tar.gz
 9d04a4d0f761b632d6de5c14abd36852 26008 avahi_0.8-ok2.debian.tar.xz
Original-Maintainer: Utopia Maintenance Team <pkg-utopia-maintainers@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmiV174ACgkQBxbwD4Qs
0nSc7wv/U+atPiegsx71vkboSqpg5r/+M7d3/mmpMQ7Q7cSSBQn6k8gqCg92DrOA
9pdxHxJFKMbcQL4oM/MXZSWdU5crktGJ7x4IkoLQlF9dLbxShF1jD37/8snCgGLQ
5wMqWdO1lYBxq2DlnmCTQ+jHMYjG+BG2fCbeibnWzBfEY+PzmSb0TsAhrOEaOZB7
NpJTc8zaSWyitjXM6jK3UATAmOjD410SJhsEc/ClOr9VHLVECi33pcOTY28W8SX0
tdMkS9MI1IS1aglT6VVwYGMe3cJHrjuUREDIGiDYnPj/LksYps2W4w/ujtPycfrq
F9ysngmpILML+z6TWeU5Icr3mzlOceep/WjrTeku3W3zlI7/LdSTjEOojc4T6jaK
e4NG50ZI1zc3sncU8sOarEoMpPfrR1hUaEqu04QTmE4XbSHwBCBgOHGgxoQJGeBd
pmtpPQofP9KmaAv2k2NfyRAME7GkXau/0AWvpsPN95Ay8IZIMMDdmuxQtk6eryiR
pv8dKnhP
=or4G
-----END PGP SIGNATURE-----
