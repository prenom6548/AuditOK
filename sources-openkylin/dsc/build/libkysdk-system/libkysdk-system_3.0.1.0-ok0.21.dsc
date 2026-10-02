-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: libkysdk-system
Binary: libkysdk-system, libkysdk-system-dev, libkysdk-disk, libkysdk-disk-dev, libkysdk-systime, libkysdk-systime-dev, libkysdk-sysinfo, libkysdk-sysinfo-dev, libkysdk-filesystem, libkysdk-filesystem-dev, libkysdk-hardware, libkysdk-hardware-dev, libkysdk-package, libkysdk-package-dev, libkysdk-proc, libkysdk-proc-dev, libkysdk-powermanagement, libkysdk-powermanagement-dev, libkysdk-ocr, libkysdk-ocr-dev, libkysdk-systemcommon, libkysdk-location, libkysdk-location-dev, libkysdk-net, libkysdk-net-dev, libkysdk-realtime, libkysdk-realtime-dev, libkysdk-system-java, libkysdk-system-python, libkysdk-system-dbus, libkysdk-system-javascript-http, libkysdk-system-javascript-websocket, libkysdk-logrotate, libkysdk-imageproc, libkysdk-imageproc-dev, libkysdk-battery, libkysdk-battery-dev, libkysdk-accounts, libkysdk-accounts-dev, libkysdk-storage, libkysdk-storage-dev, libkysdk-search, libkysdk-search-dev, libkysdk-global, libkysdk-global-dev, libkysdk-sysinfo-data,
 libkysdk-sysinfo-bin, libkysdk-pnp, libkysdk-pnp-dev, libkysdk-uri, libkysdk-uri-dev, libkysdk-devicemanager, libkysdk-devicemanager-dev, libkysdk-shutdown, libkysdk-shutdown-dev, libkysdk-appability, libkysdk-appability-dev, libkysdk-sensors, libkysdk-sensors-dev, libkysdk-restart, libkysdk-restart-dev, libkysdk-exchange, libkysdk-exchange-dev, libkysdk-media,
 libkysdk-media-dev
Architecture: any all
Version: 3.0.1.0-ok0.21
Maintainer: kylin <chenzhikai@kylinos.cn>
Homepage: http://gitlab2.kylin.com/kysdk/kysdk-system
Standards-Version: 4.4.1
Build-Depends: debhelper-compat (= 12), cmake, libc6-dev, libsystemd-dev, libcjson-dev, libdbus-1-dev, libdbus-glib-1-dev, libglib2.0-dev, libkysdk-log-dev (>= 1.2.0), libkysdk-config-dev (>= 1.2.0), libkysdk-utils-dev (>= 1.2.0), libkysdk-timer-dev (>= 1.2.0), libqt5core5a, qtbase5-dev, libudev-dev, libopencv-dev, libarchive-dev, libtesseract-dev, libcups2-dev, libcurl4-openssl-dev, libxrandr-dev, libqt5dbus5, libqt5webchannel5-dev, libqt5websockets5-dev, libkmod-dev, libhd-dev, libpci-dev, libbluetooth-dev, libsensors-dev, libleptonica-dev, libusb-1.0-0-dev, libnl-3-dev, libnl-genl-3-dev, libnl-route-3-dev, libpolkit-gobject-1-dev, libkysdk-gsetting-dev, libgtk-3-dev, libmagic-dev, libsqlite3-dev, libmagickwand-dev, libnm-dev, libkysdk-conf2-dev, libsoup2.4-dev, libzip-dev, zlib1g-dev, liblzma-dev, liblz4-dev, libjson-c-dev, libfdisk-dev, libparted-dev, libmount-dev, libzstd-dev, libx11-dev, libasound2-dev, libssl-dev, libelf-dev
Package-List:
 libkysdk-accounts deb utils optional arch=any
 libkysdk-accounts-dev deb utils optional arch=all
 libkysdk-appability deb utils optional arch=any
 libkysdk-appability-dev deb utils optional arch=all
 libkysdk-battery deb utils optional arch=any
 libkysdk-battery-dev deb utils optional arch=all
 libkysdk-devicemanager deb utils optional arch=any
 libkysdk-devicemanager-dev deb utils optional arch=all
 libkysdk-disk deb utils optional arch=any
 libkysdk-disk-dev deb utils optional arch=all
 libkysdk-exchange deb utils optional arch=any
 libkysdk-exchange-dev deb utils optional arch=all
 libkysdk-filesystem deb utils optional arch=any
 libkysdk-filesystem-dev deb utils optional arch=all
 libkysdk-global deb utils optional arch=any
 libkysdk-global-dev deb utils optional arch=all
 libkysdk-hardware deb utils optional arch=any
 libkysdk-hardware-dev deb utils optional arch=all
 libkysdk-imageproc deb utils optional arch=any
 libkysdk-imageproc-dev deb utils optional arch=all
 libkysdk-location deb utils optional arch=any
 libkysdk-location-dev deb utils optional arch=all
 libkysdk-logrotate deb utils optional arch=any
 libkysdk-media deb utils optional arch=any
 libkysdk-media-dev deb utils optional arch=all
 libkysdk-net deb utils optional arch=any
 libkysdk-net-dev deb utils optional arch=all
 libkysdk-ocr deb utils optional arch=any
 libkysdk-ocr-dev deb utils optional arch=all
 libkysdk-package deb utils optional arch=any
 libkysdk-package-dev deb utils optional arch=all
 libkysdk-pnp deb utils optional arch=any
 libkysdk-pnp-dev deb utils optional arch=all
 libkysdk-powermanagement deb utils optional arch=any
 libkysdk-powermanagement-dev deb utils optional arch=all
 libkysdk-proc deb utils optional arch=any
 libkysdk-proc-dev deb utils optional arch=all
 libkysdk-realtime deb utils optional arch=any
 libkysdk-realtime-dev deb utils optional arch=all
 libkysdk-restart deb utils optional arch=any
 libkysdk-restart-dev deb utils optional arch=all
 libkysdk-search deb utils optional arch=any
 libkysdk-search-dev deb utils optional arch=all
 libkysdk-sensors deb utils optional arch=any
 libkysdk-sensors-dev deb utils optional arch=all
 libkysdk-shutdown deb utils optional arch=any
 libkysdk-shutdown-dev deb utils optional arch=all
 libkysdk-storage deb utils optional arch=any
 libkysdk-storage-dev deb utils optional arch=all
 libkysdk-sysinfo deb utils optional arch=any
 libkysdk-sysinfo-bin deb utils optional arch=any
 libkysdk-sysinfo-data deb utils optional arch=any
 libkysdk-sysinfo-dev deb utils optional arch=all
 libkysdk-system deb utils optional arch=any
 libkysdk-system-dbus deb utils optional arch=any
 libkysdk-system-dev deb utils optional arch=all
 libkysdk-system-java deb utils optional arch=any
 libkysdk-system-javascript-http deb utils optional arch=any
 libkysdk-system-javascript-websocket deb utils optional arch=any
 libkysdk-system-python deb utils optional arch=any
 libkysdk-systemcommon deb utils optional arch=any
 libkysdk-systime deb utils optional arch=any
 libkysdk-systime-dev deb utils optional arch=all
 libkysdk-uri deb utils optional arch=any
 libkysdk-uri-dev deb utils optional arch=all
Checksums-Sha1:
 cad1961f847e34b0d08ea9c96366cb99c6415fc6 66585512 libkysdk-system_3.0.1.0.orig.tar.xz
 26d5fcd4b2d57875fb26458fa96e5d414f47b869 68448148 libkysdk-system_3.0.1.0-ok0.21.debian.tar.xz
Checksums-Sha256:
 a17e0bb5eb52a0cc6da13c17d04d1cc2b057fac9a93a059b6e2920c313dc5ac3 66585512 libkysdk-system_3.0.1.0.orig.tar.xz
 2875b1bbbe0f4694f54f42cff858721ac45995ea68b87dc1f5b36ca2b421fae8 68448148 libkysdk-system_3.0.1.0-ok0.21.debian.tar.xz
Files:
 52f8dce5fda598fbdeaddf605f84858e 66585512 libkysdk-system_3.0.1.0.orig.tar.xz
 875e14338973e78787841a0105631596 68448148 libkysdk-system_3.0.1.0-ok0.21.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqNZ30ACgkQBxbwD4Qs
0nSSlwwAkR30lzSXAj3MjBIvpl5bHvjI45wCP1zaKFgZfx6eGY+yf0o7gDI0U59N
V47o+eIDxdlecxvM0kVjrw6/WH/47gRwyH1ZNwHP6ySrpEiSQSoRT0LmcyKrYSrF
spLqdYq3EF4LKoPfSdIJbb86ZnIACYE41iy98Bl52mdp+qFiCBRXStRzs2OIpHGW
CWyNPTdA46XZ5t14oyOnzipUkxX0NxGMLbxO4T4qNxIe7uawWKEr+wQSEyJyUH3n
n+zegFhBZg202MoDnfcRAcw9vtPCGv+uwgGU3qcgxZdgT81gOSPW3YD1W3PuMAiE
6JLguBcPm5VyhtdQ20fm7nH9KzX2MaQ/0IuIIUfO6oNSJZ+7ByDuiFw4KFRjHtoS
C5lPYeMV8ZMtx2Fm+4CCH6AsrvQWeTdgmHB/wdkvC9onqMgl251WPnUnivFdUXw3
NUCTRPhMWB7P5qUnmiYS1ZejFFUvKDIi7/6D4PUiUNd7pbhkzxgyqG5BcexwnxpM
x0D9knRU
=+7oe
-----END PGP SIGNATURE-----
