-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: wpa
Binary: hostapd, wpagui, wpasupplicant, wpasupplicant-udeb, eapoltest, libwpa-client-dev
Architecture: linux-any kfreebsd-any
Version: 2:2.11-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders:  Andrej Shadura <andrewsh@debian.org>
Homepage: https://w1.fi/wpa_supplicant/
Standards-Version: 4.4.1
Vcs-Browser: https://salsa.debian.org/debian/wpa
Vcs-Git: https://salsa.debian.org/debian/wpa.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential
Build-Depends: debhelper-compat (= 13), libdbus-1-dev, libssl-dev, qtbase5-dev [linux-any] <!pkg.wpa.nogui>, libncurses5-dev, libpcsclite-dev, libnl-3-dev [linux-any], libnl-genl-3-dev [linux-any], libnl-route-3-dev [linux-any], libpcap-dev [kfreebsd-any], libbsd-dev [kfreebsd-any], libreadline-dev, pkgconf | pkg-config, docbook-to-man <!nodoc>, docbook-utils <!nodoc>
Package-List:
 eapoltest deb net optional arch=linux-any,kfreebsd-any
 hostapd deb net optional arch=linux-any,kfreebsd-any
 libwpa-client-dev deb libdevel optional arch=linux-any,kfreebsd-any
 wpagui deb net optional arch=linux-any profile=!pkg.wpa.nogui
 wpasupplicant deb net optional arch=linux-any,kfreebsd-any
 wpasupplicant-udeb udeb debian-installer standard arch=linux-any profile=!noudeb
Checksums-Sha1:
 0da1d92bec75b661347613181b96adee623d5061 2780956 wpa_2.11.orig.tar.xz
 35fc0766ff4755e42ad8e6b7041e7d293f7c808b 55200 wpa_2.11-ok1.debian.tar.xz
Checksums-Sha256:
 9f1109e2aef8e591004bec8e9d8dd363be61c2edaf99f4cc14e35b74316d5a45 2780956 wpa_2.11.orig.tar.xz
 397a1547f52fdda3c95a673e7f07ae4db6019be8bb5b66da8cd8df0ac2ef6625 55200 wpa_2.11-ok1.debian.tar.xz
Files:
 b35896f8d15b14c9d26091f3addd7105 2780956 wpa_2.11.orig.tar.xz
 be85c7c71c34a878513654b3c822e96e 55200 wpa_2.11-ok1.debian.tar.xz
Original-Maintainer: Debian wpasupplicant Maintainers <wpa@packages.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmofvncACgkQBxbwD4Qs
0nTJnAv/bPIwvPphJvsRA+9/w05x/dUSMYyRWNACNuBmv7sL/L/qqrzs6YrKkBk5
FEx8E8J2o4Xa0yyqSw/yxts3XINqoR4GyX7V9FWL8P5hrz1Xl3gU7iCwHN3M0eKa
aNILwSurBKpVE0F96aZiqgBcPE7uSIthLBa1ztaYDq/Yi7cCST3X/afbymXg139t
OvcxSoaZfMu7NpdC5ou3m6E68JggeYn46EPxoEOyvPIJXJijEcpLO/vbqb3KZ/ZY
jYvFs32DHX6kvvxjS277w+tI+7fMlSpqOkvn95PdJfYdiQfF/RTgTmdrJx8/1dIY
qSbPRA0jrFrle0A2xsAzyazu09Xn0mtGg7IEdaccKYnAKR8Iav7fRFN6aWgkmbcF
+RsaYcnCU9+CBOv+dnapbTf5BtRWLbLRpMHr+3PKWLLvW1x7zY99+Wkgx34LkkFf
OB/c2mIE7wDnzieb3e5+JA8s4EkM5HkT148yvIgeyRd7/5h42HotnZst3GcCvzcN
iH/DBbfP
=zHJF
-----END PGP SIGNATURE-----
