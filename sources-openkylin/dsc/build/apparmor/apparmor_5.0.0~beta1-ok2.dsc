-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: apparmor
Binary: apparmor, apparmor-utils, apparmor-profiles, libapparmor-dev, libapparmor1, libapache2-mod-apparmor, libpam-apparmor, apparmor-notify, python3-libapparmor, python3-apparmor, dh-apparmor
Architecture: linux-any all
Version: 5.0.0~beta1-ok2
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: intrigeri <intrigeri@debian.org>
Homepage: https://apparmor.net/
Standards-Version: 4.6.1
Vcs-Browser: https://salsa.debian.org/apparmor-team/apparmor/tree/ubuntu/master
Vcs-Git: https://salsa.debian.org/apparmor-team/apparmor.git -b ubuntu/master
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, apparmor-profiles-extra, bind9, chrony, cups-browsed, cups-daemon, evince, haveged, libreoffice-common, libvirt-daemon-system, linux-image-amd64, linux-image-generic, man-db, onioncircuits, tcpdump, tor
Build-Depends: apache2-dev, autoconf, autoconf-archive, automake, bison, bzip2, chrpath, debhelper-compat (= 13), dejagnu <!nocheck>, dh-apache2, dh-python, dh-sequence-python3, flake8 <!nocheck>, flex, liblocale-gettext-perl <!nocheck>, libpython3-all-dev, libpam-dev, libtool, libzstd-dev, net-tools, perl <!nocheck>, pkgconf, po-debconf, python3:any, python3-all:any, python3-all-dev:any, python3-gi <!nocheck>, python3-notify2 <!nocheck>, python3-psutil <!nocheck>, python3-setuptools, python3-tk <!nocheck>, socat <!nocheck>, swig
Package-List:
 apparmor deb admin optional arch=linux-any
 apparmor-notify deb admin optional arch=all
 apparmor-profiles deb admin optional arch=all
 apparmor-utils deb admin optional arch=all
 dh-apparmor deb devel optional arch=all
 libapache2-mod-apparmor deb httpd optional arch=linux-any
 libapparmor-dev deb libdevel optional arch=linux-any
 libapparmor1 deb libs optional arch=linux-any
 libpam-apparmor deb admin optional arch=linux-any
 python3-apparmor deb python optional arch=all
 python3-libapparmor deb python optional arch=linux-any
Checksums-Sha1:
 ffe3bd322763dafeda28c7d0ea67f3871afeb443 6273799 apparmor_5.0.0~beta1.orig.tar.bz2
 9c14c3648e20997af6c478fe2850ca6166b964ff 79816 apparmor_5.0.0~beta1-ok2.debian.tar.xz
Checksums-Sha256:
 cc998a4c2ab7b85d6b0b8d7db8766abc232a8812fdb2af63a31b2dbda0d66e77 6273799 apparmor_5.0.0~beta1.orig.tar.bz2
 b18863029967b83f1aa3747cda763256a1a54fe822a834ec39d87215be317ad5 79816 apparmor_5.0.0~beta1-ok2.debian.tar.xz
Files:
 89f073017691be118d1725c704a7841d 6273799 apparmor_5.0.0~beta1.orig.tar.bz2
 4d1341a873c7008017c57302c02fbea0 79816 apparmor_5.0.0~beta1-ok2.debian.tar.xz
Original-Maintainer: Debian AppArmor Team <pkg-apparmor-team@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmp+ikIACgkQBxbwD4Qs
0nQTswv/YqShAnFPz3BLcXCK+YHoLejvOMg0i0XnX9E+H9Dk3hI4cpTxmjBl0LHx
WT015VjqkkubPmAHnDuQfynJoPxj6ErhXOcWewijWE7/Pe25TnbjHvRrV7TwZWh8
q7mWIldYNmLA+wqH2BgsKkuB6I8izwICsuBu3Ic1ZyW77Jp7NJ6bM/Ke4AOFZuti
lXFp8zu2K7UY9pPc+zxfJYMzf7dMs5hvO4FAERkvRMkrOIblpvhRtzEhsDlJFmzp
b4/beiPZYmcbuT6fBG3AlVl2O7CBaGXN5XFqKy7bJUOgeFZVQbkyDyZsowDPMa8v
J4PX6NmRCSzQONUcDjTYu+Jbo/iU7nAU6a2iFA9xBhJp5uctqaMPNzw5JZ+DS6vg
vkvn4ht0IUXLk32NRA0uDco7BLE5UJ7eE/avG6pvtvGNGRIdT+CUaQxxpUPttJvF
5HF+iZh3coAFTZKRkODU/839PsFPr/8Dtl56kkgxUXIfiKjihhXzuH4XK9xiT57N
7k481MfV
=w6tk
-----END PGP SIGNATURE-----
