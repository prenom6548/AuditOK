-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: android-platform-system-core
Binary: android-sdk-libsparse-utils, android-liblog, android-liblog-dev, android-libcutils, android-libcutils-dev, adb, android-libsparse, android-libsparse-dev, android-libutils, android-libutils-dev, android-libziparchive, android-libziparchive-dev, android-platform-system-core-headers, android-libbacktrace, android-libbacktrace-dev, android-libadb, android-libadb-dev, android-libbase, android-libbase-dev, android-libcrypto-utils, android-libcrypto-utils-dev, simg2img, img2simg, append2simg, android-tools-adb, android-tools-fastboot, android-tools-mkbootimg, mkbootimg, fastboot, android-libnativebridge, android-libnativebridge-dev, android-libnativeloader, android-libnativeloader-dev
Architecture: any all
Version: 1:8.1.0+r23-ok2
Maintainer: Openkylin Developers <packaging@lists.openkylin.top>
Uploaders: Luo Yaoming <luoyaoming@kylinos.cn>
Homepage: https://android.googlesource.com/platform/system/core
Standards-Version: 4.2.1
Vcs-Browser: https://gitee.com/openkylin/android-platform-system-core
Vcs-Git: https://gitee.com/openkylin/android-platform-system-core.git
Testsuite: autopkgtest
Build-Depends: bash-completion, debhelper (>= 11~), dh-exec, dpkg-dev (>= 1.17.14)
Build-Depends-Arch: android-libboringssl-dev [amd64 i386 armel armhf arm64], android-libf2fs-utils-dev (>= 8.1.0+r23~) [amd64 i386 armel armhf arm64] <!stage1>, android-libnativehelper-dev (>= 8.1.0+r23) [amd64 i386 armel armhf arm64 mips mipsel mips64el] <!stage1>, android-libunwind-dev (>= 8.1.0+r23~) [amd64 i386 armel armhf arm64 mips mipsel mips64el], libgtest-dev [amd64 i386 armel armhf arm64] <!stage1>, libsafe-iop-dev [amd64 i386 armel armhf arm64 mips mipsel mips64el], libusb-1.0-0-dev [amd64 i386 armel armhf arm64], pandoc [amd64 i386 armel armhf arm64], zlib1g-dev [any]
Package-List:
 adb deb devel optional arch=amd64,i386,armel,armhf,arm64
 android-libadb deb libs optional arch=amd64,i386,armel,armhf,arm64
 android-libadb-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64
 android-libbacktrace deb libs optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-libbacktrace-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-libbase deb libs optional arch=any
 android-libbase-dev deb libdevel optional arch=any
 android-libcrypto-utils deb libs optional arch=amd64,i386,armel,armhf,arm64
 android-libcrypto-utils-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64
 android-libcutils deb libs optional arch=any
 android-libcutils-dev deb libdevel optional arch=any
 android-liblog deb libs optional arch=any
 android-liblog-dev deb libdevel optional arch=any
 android-libnativebridge deb libs optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el profile=!stage1
 android-libnativebridge-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el profile=!stage1
 android-libnativeloader deb libs optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el profile=!stage1
 android-libnativeloader-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el profile=!stage1
 android-libsparse deb libs optional arch=any
 android-libsparse-dev deb libdevel optional arch=any
 android-libutils deb libs optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-libutils-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-libziparchive deb libs optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-libziparchive-dev deb libdevel optional arch=amd64,i386,armel,armhf,arm64,mips,mipsel,mips64el
 android-platform-system-core-headers deb libdevel optional arch=all
 android-sdk-libsparse-utils deb utils optional arch=any
 android-tools-adb deb oldlibs optional arch=all
 android-tools-fastboot deb oldlibs optional arch=all
 android-tools-mkbootimg deb oldlibs optional arch=all
 append2simg deb oldlibs optional arch=all
 fastboot deb devel optional arch=amd64,i386,armel,armhf,arm64 profile=!stage1
 img2simg deb oldlibs optional arch=all
 mkbootimg deb utils optional arch=all
 simg2img deb oldlibs optional arch=all
Checksums-Sha1:
 4ba3a65094356e205335481c8acebe2c9fc34f0e 5229971 android-platform-system-core_8.1.0+r23.orig.tar.gz
 feac63cc7b30b094f3e97a2b6062fb40ced49df5 21600 android-platform-system-core_8.1.0+r23-ok2.debian.tar.xz
Checksums-Sha256:
 6e1902a4bda48c43c239ac38f854eb78de85a9c818784b60d93e069e6213f681 5229971 android-platform-system-core_8.1.0+r23.orig.tar.gz
 48b338d71fc3b4bcd81d9dc5e633098ad5c0d818d0ad4afe7d60f9777d4d2c72 21600 android-platform-system-core_8.1.0+r23-ok2.debian.tar.xz
Files:
 857436508c8084139d8115a8650ed116 5229971 android-platform-system-core_8.1.0+r23.orig.tar.gz
 4f1af318a55f92d8efa8491bfcb7e5ad 21600 android-platform-system-core_8.1.0+r23-ok2.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmMZj6oACgkQBxbwD4Qs
0nQ0Mgv/b6H7T8f0MJpOePFa0r7pypQfeHpbSHFFx0J676FcDhUMhiW5YZY8cSMJ
xZY7IwYVm+HfBiZLL8aKnIiWn8Cr8HSv2gWnFB1uKLniVaYtuRapFIBdrBg/QqGg
+g15R0mpIZyNRvgf1tQL3u9EBkBiEjk2L29VNWct9Rf0CwYhyM7F1euYZVVK+bl/
87TQUjfgSAymctb2o60QXjkzO71iRXjfkcMqYP6vc1QRzrd6l6LWt0QsxhLQFccR
UF84LbdBygvOJ1WFKRWzL1YNe1+u7SsECWs6SFTsFA7xJOHAFZzWD9Rnx5lazUPP
E9H+Cvcq2k5q9vU4n8RDbtjCZEbsiPigp4NhHISOaZ1rETz4EwS+OlAC0YHTQ+Yv
Ck2SvkqvKahH2pDVKVfahawdXJm2Xr6yJMxYlbsl6nkp/q10VmnO2eG7dgg8T8dI
Z9BtN9b3AX3ltPUGfpzzyymKciaLfsIL4RF6AJIDQ1QO8E+4kdX4gVq9aMCcpXkx
KE7vaLNO
=S7zR
-----END PGP SIGNATURE-----
