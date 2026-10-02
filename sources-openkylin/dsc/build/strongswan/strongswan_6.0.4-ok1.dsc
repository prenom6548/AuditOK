-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: strongswan
Binary: strongswan, libstrongswan, libstrongswan-standard-plugins, libstrongswan-extra-plugins, libcharon-extauth-plugins, libcharon-extra-plugins, strongswan-starter, strongswan-libcharon, strongswan-charon, strongswan-nm, charon-cmd, strongswan-pki, strongswan-swanctl, charon-systemd
Architecture: any all
Version: 6.0.4-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: http://www.strongswan.org
Standards-Version: 4.7.1
Vcs-Browser: https://gitee.com/openkylin/strongswan
Vcs-Git: https://gitee.com/openkylin/strongswan.git
Testsuite: autopkgtest
Testsuite-Triggers: bind9-dnsutils, dctrl-tools, libtss2-tcti-tabrmd0, lsb-release, snapd
Build-Depends: bison, bzip2, debhelper-compat (= 12), dh-apparmor, dpkg-dev (>= 1.16.2), flex, gperf, libiptc-dev [linux-any], libcap-dev [linux-any], libcurl4-openssl-dev | libcurl3-dev | libcurl2-dev, libgcrypt20-dev | libgcrypt11-dev, libgmp3-dev, libkrb5-dev, libldap2-dev, libnm-dev [linux-any], libpam0g-dev, libsqlite3-dev, libssl-dev (>= 0.9.8), libsystemd-dev [linux-any], libtool, libtss2-dev, libxml2-dev, pkgconf, po-debconf, systemd-dev [linux-any], tzdata
Package-List:
 charon-cmd deb net optional arch=any
 charon-systemd deb net optional arch=linux-any
 libcharon-extauth-plugins deb net optional arch=any
 libcharon-extra-plugins deb net optional arch=any
 libstrongswan deb net optional arch=any
 libstrongswan-extra-plugins deb net optional arch=any
 libstrongswan-standard-plugins deb net optional arch=any
 strongswan deb net optional arch=all
 strongswan-charon deb net optional arch=any
 strongswan-libcharon deb net optional arch=any
 strongswan-nm deb net optional arch=linux-any
 strongswan-pki deb net optional arch=any
 strongswan-starter deb net optional arch=any
 strongswan-swanctl deb net optional arch=any
Checksums-Sha1:
 3eb4872f5b03ea99ba2d1ef276c8b0b54b1da675 7991797 strongswan_6.0.4.orig.tar.gz
 4d01eef26c3b9cf0d17b0995e911af0dc1d00d88 122588 strongswan_6.0.4-ok1.debian.tar.xz
Checksums-Sha256:
 3655293871f622aaf79863a720b1e9f452b0624eb50d0d497c31b37db64be578 7991797 strongswan_6.0.4.orig.tar.gz
 fa386163242a9d8e43c4ab652439c9a41714d578c6ede1d9e9aed2c96f20d52b 122588 strongswan_6.0.4-ok1.debian.tar.xz
Files:
 789c4c18901cfb7e5d11c9e4403e62e5 7991797 strongswan_6.0.4.orig.tar.gz
 1f7b36d0601670bbb48969437f9e4b24 122588 strongswan_6.0.4-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmpHjtEACgkQBxbwD4Qs
0nRMcAv/SAJtV+27Sn26vBXQY1V1Jg+IbpCRtTsednD+nbV5lnHrsDmgNKAT2Irq
1MhuVLpvIM8n5WlsWJt68rY493RfyIqiYDrUHxFfn5UBB4o9hK7bsYAoUrexGAUK
wAZBXHFLI4FoatuDu6TzU3TCBuI9W0bwTEMW6pfD+xQNJMqHtj0PCd+YX0rFOrlV
LEX7wwCmO3R7jMepmmKiok414M72q/NiuWwBzkMTQilEq/dYDyZnLTmjvSgKaV+H
CanIMwm5vRJlX1EPNK/xPRCeN3phwujc95Mm6VM98PZbZ5NduFkeS/BP7glfBxPD
Yovw7xDL5FZeudvOoIUfGdlA631iL738Qi4Gt53J99aCayPLIhBOAZUEz7XdJO2L
6oADlD8HTnnwwG2Q6cNfZna33D8jqMDb67Q2zFixnx2y/aK/pZZ8frCg/7fTomRu
ypeuNiRyhGvDQTcIhZl4JJVOQmoVHlUNHWreLAypAPKBBlGnOw+in1ucPfQKOF3/
WthbYcB8
=24Ti
-----END PGP SIGNATURE-----
