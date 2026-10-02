-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: xorg-server
Binary: xserver-xorg-core, xserver-xorg-core-udeb, xserver-xorg-dev, xnest, xvfb, xserver-xephyr, xserver-common, xorg-server-source, xserver-xorg-legacy
Architecture: any all
Version: 2:21.1.21-ok1
Maintainer: Ubuntu X-SWAT <ubuntu-x@lists.ubuntu.com>
Homepage: https://www.x.org/
Standards-Version: 3.9.8
Vcs-Browser: https://salsa.debian.org/xorg-team/xserver/xorg-server
Vcs-Git: https://salsa.debian.org/xorg-team/xserver/xorg-server.git
Testsuite: autopkgtest
Testsuite-Triggers: x11-utils, xauth
Build-Depends: debhelper-compat (= 12), po-debconf, quilt, pkgconf, bison, flex, xauth, xutils-dev (>= 1:7.6+4), xfonts-utils (>= 1:7.5+1), x11proto-dev (>= 2021.5), xtrans-dev (>= 1.3.5), libxau-dev (>= 1:1.0.5-2), libxcvt-dev, libxdmcp-dev (>= 1:0.99.1), libxfont-dev (>= 1:2.0.1), libxkbfile-dev (>= 1:0.99.1), libpixman-1-dev (>= 0.27.2), libpciaccess-dev (>= 0.12.901), nettle-dev, libudev-dev (>= 151-3) [linux-any], libselinux-dev (>= 2.0.80) [linux-any], libaudit-dev [linux-any], libdrm-dev (>= 2.4.107-5~) [!hurd-any], libglvnd-dev, mesa-common-dev, libunwind-dev [amd64 arm64 armel hppa i386 ia64 mips64 mips64el mipsel powerpc powerpcspe ppc64 ppc64el sh4], libxmuu-dev (>= 1:0.99.1), libxext-dev (>= 1:0.99.1), libx11-dev (>= 2:1.6), libxrender-dev (>= 1:0.9.0), libxi-dev (>= 2:1.8), libxpm-dev (>= 1:3.5.3), libxaw7-dev (>= 1:0.99.1), libxt-dev (>= 1:0.99.1), libxmu-dev (>= 1:0.99.1), libxtst-dev (>= 1:0.99.1), libxres-dev (>= 1:0.99.1), libxfixes-dev (>= 1:3.0.0), libxv-dev, libxinerama-dev, libxshmfence-dev (>= 1.1) [!hurd-any], libepoxy-dev [linux-any kfreebsd-any], libegl1-mesa-dev [linux-any kfreebsd-any], libgbm-dev (>= 10.2) [linux-any kfreebsd-any], libx11-xcb-dev, libxcb1-dev, libxcb-xkb-dev, libxcb-shape0-dev, libxcb-render0-dev, libxcb-render-util0-dev, libxcb-util-dev, libxcb-image0-dev, libxcb-icccm4-dev, libxcb-shm0-dev, libxcb-keysyms1-dev, libxcb-randr0-dev, libxcb-xv0-dev, libxcb-glx0-dev, libxcb-xf86dri0-dev (>= 1.6), xkb-data, x11-utils <!nocheck>, x11-xkb-utils, libbsd-dev, libdbus-1-dev (>= 1.0) [linux-any], libsystemd-dev [linux-any], systemd-dev [linux-any]
Build-Depends-Indep: xz-utils
Package-List:
 xnest deb x11 optional arch=any
 xorg-server-source deb x11 optional arch=all
 xserver-common deb x11 optional arch=all
 xserver-xephyr deb x11 optional arch=any
 xserver-xorg-core deb x11 optional arch=any
 xserver-xorg-core-udeb udeb debian-installer optional arch=any profile=!noudeb
 xserver-xorg-dev deb x11 optional arch=any
 xserver-xorg-legacy deb x11 optional arch=any
 xvfb deb x11 optional arch=any
Checksums-Sha1:
 b541a0ad9249820246c293604ccfddc39d8ddc97 9028119 xorg-server_21.1.21.orig.tar.gz
 95a7e28455e03622134254a12f8d63f65e05605d 104188 xorg-server_21.1.21-ok1.debian.tar.xz
Checksums-Sha256:
 c7183a9eb81c44c45d4d122941045bd1d54cd5c492f6f3facd1024f0c3c17e0f 9028119 xorg-server_21.1.21.orig.tar.gz
 06dd7baa2e025770b9f1e4f053b225f9d4c4f86c496fc7141d370a75c9ee52d3 104188 xorg-server_21.1.21-ok1.debian.tar.xz
Files:
 b6b1e59b6371538fa4ba8f4599a797f7 9028119 xorg-server_21.1.21.orig.tar.gz
 1c5abf8149db7535f025ae5f8026c333 104188 xorg-server_21.1.21-ok1.debian.tar.xz
Original-Maintainer: Debian X Strike Force <debian-x@lists.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnllNEACgkQBxbwD4Qs
0nQWTAwAk888B7I+n9fWRZA3/GEoHrlSJEeN4xminCVvIDkALWYD35S6HwtNgCIF
p+RAyf096jWdJKEO00GNm5Nvmxoachd6fAAz/8KI7McP1t1C5/svs4CXuYPFCIoV
iY5mjjcoo6dqv+X59bhrrw/7PvMC5AT2xcOdbv0C7ZBbU4RtIz1oEO/GqYdMKfkh
fTo9Eb1jpwJ2aLi4uZg15M5O97Ycddj/K/o/+6wZGE1dC6zjovjy4MhcbDq8lgyX
JxuoHNxwzETGerUuVcoOrqAs7yHDrJxRQReIduuntW1xWzKisbpCYj60MV13xKv4
+qnzEx1FbhrrMNAkWK3QJ3ebhBeNK/rPYFYQa4QdNb23kN9HhYOoSdVm4U9qFeQO
4AaGfhoH+j54zwaexPpE6d/fIJ0AoXDCtm+5LWnS9NmTp5PjXaWSNBQxGFFGBHar
GvQd5vb3hsDYU5ZNaFAL82BlMQB8VPEghd81HgnHwICRVYu0xsAAZ/E6Y1G6uNEY
qIWkUBF1
=FEQ+
-----END PGP SIGNATURE-----
