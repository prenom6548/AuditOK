-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: ubiquity
Binary: ubiquity, ubiquity-ubuntu-artwork, ubiquity-frontend-gtk, ubiquity-frontend-gtk-panel, ubiquity-frontend-kde, ubiquity-frontend-debconf, oem-config, oem-config-gtk, oem-config-kde, oem-config-debconf, oem-config-remaster
Architecture: any all
Version: 24.04.5-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: Colin Watson <cjwatson@ubuntu.com>, Evan Dandrea <ev@ubuntu.com>
Standards-Version: 3.9.4
Vcs-Browser: https://code.launchpad.net/~ubuntu-installer/ubiquity/+git/ubiquity/+ref/master
Vcs-Git: https://git.launchpad.net/ubiquity
Testsuite: autopkgtest
Testsuite-Triggers: humanity-icon-theme, python3-mock, xvfb
Build-Depends: adwaita-icon-theme, apt, autopoint, bf-utf-source, check, dctrl-tools, debconf-utils, debhelper (>= 11), dh-autoreconf, dh-di (>= 3), dh-python, dpkg-dev (>= 1.16.1~), gir1.2-nma-1.0, gir1.2-soup-3.0, gir1.2-timezonemap-1.0, gir1.2-webkit2-4.1, gir1.2-xkl-1.0, gobject-introspection, imagemagick, intltool (>= 0.40.0), intltool-debian (>= 0.30+20040212), iso-codes, isoquery, keymapper (>= 0.5.3-7), libblkid-dev, libbogl-dev, libcairo2-dev, libdebconfclient0-dev (>= 0.68), libdebian-installer4-dev (>= 0.76), libgirepository1.0-dev, libglib2.0-dev, libgtk-3-dev (>= 3.20), libindicator3-dev, libiw-dev (>= 27+28pre9), liblocale-gettext-perl, libparted-dev (>= 2.2), librsvg2-bin, libsubunit-dev, locales, pkg-config, po-debconf (>= 1.0), pycodestyle, pyflakes3 (>= 0.7.2), python-gi-dev, python3-all (>= 3.1), python3-apt (>= 0.7.100.3~), python3-cairo, python3-dbus, python3-debconf, python3-gi, python3-gi-cairo, python3-icu (>= 1.0), python3-mock (>= 0.7.0), python3-pam, qrencode, rename, scour, tzdata, ubuntu-artwork, udev, wget, xkb-data (>= 0.9), xvfb
Package-List:
 oem-config deb admin optional arch=all
 oem-config-debconf deb admin optional arch=all
 oem-config-gtk deb admin optional arch=all
 oem-config-kde deb admin optional arch=all
 oem-config-remaster deb admin optional arch=all
 ubiquity deb admin optional arch=any
 ubiquity-frontend-debconf deb admin optional arch=any
 ubiquity-frontend-gtk deb admin optional arch=amd64,arm64,armhf,i386,ppc64el
 ubiquity-frontend-gtk-panel deb admin optional arch=amd64,arm64,armhf,i386,ppc64el
 ubiquity-frontend-kde deb admin optional arch=all
 ubiquity-ubuntu-artwork deb admin optional arch=all
Checksums-Sha1:
 3dee9ef6b9c717c738edd639420fb5dbf7fe278e 9834056 ubiquity_24.04.5.orig.tar.gz
 32ceab218187119d744ca1037b668035d7c48c72 652008 ubiquity_24.04.5-ok1.debian.tar.xz
Checksums-Sha256:
 76caa7d6a1ebdd02b21f841a74c2903b8e7a877edf2b8bdcc57d15e90b5f9ede 9834056 ubiquity_24.04.5.orig.tar.gz
 e6ff2400559e9da8a58f7070853df308fe4f8591ac3371b46a703ca6ddea7148 652008 ubiquity_24.04.5-ok1.debian.tar.xz
Files:
 9fda61a06315d6a9cafc045cc65d220c 9834056 ubiquity_24.04.5.orig.tar.gz
 2bcc2153fcd54a159a89da3c4baebb4d 652008 ubiquity_24.04.5-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmY4fmwACgkQBxbwD4Qs
0nTruwv/cFrKZEOMCa0FeErkCqBgoKRIlByhZwFkoDeWCy37bpZkS0GuzzU7YPId
7fyULjcOi4tGci4TFepJuI3z1WXEb+C4bK9KzHc6c6wzg+T4C0FUSkaKmmaOIq0o
VIGvAnw58ajku1chh85nC0It6H9JMVVc0Mb044Zp4biEnRwLK0MR/xxbEG5X6u9Q
YGMPVvufEhz8SBXBC55BMzxPsleqKoed+XasT3ni5gkvAWDRaaf7XiN/kBshwI6g
sf/OLGTpiBWj0TDax5ef+Ov0pzr2R/0aIBU2LYtTZkZUmHGrOCz9oRFLfurKyRD/
77J59ZjSFWt2Ldfle34rsZbLeuSi/X221Fc94aovo/oNtmbR2hjmnJK1k0a820Qb
5TslISJPPAfEuBfJxGIYjEUTUveV4/cUdJun7mikcc1BswWXFzeRaGtVTAT8GqTR
q6BUBD7LPJCdWw7jlizZIp3ZNGr1KIIwrFoHpxawOVDDOd4thgc0nMnwMGel/huw
wQEKUOx7
=S54Z
-----END PGP SIGNATURE-----
