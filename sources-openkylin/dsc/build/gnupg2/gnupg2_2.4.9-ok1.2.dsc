-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: gnupg2
Binary: gpgconf, gnupg-agent, gpg-agent, gpg-wks-server, gpg-wks-client, scdaemon, gpgsm, gpg, gnupg, gnupg2, gpgv, dirmngr, tpm2daemon, gpgv-udeb, gpgv-static, gpgv-win32, gnupg-l10n, gnupg-utils
Architecture: any all
Version: 2.4.9-ok1.2
Maintainer: Debian GnuPG Maintainers <pkg-gnupg-maint@lists.alioth.debian.org>
Uploaders:  Eric Dorland <eric@debian.org>, Daniel Kahn Gillmor <dkg@fifthhorseman.net>, Andreas Metzler <ametzler@debian.org>,
Homepage: https://www.gnupg.org/
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/debian/gnupg2
Vcs-Git: https://salsa.debian.org/debian/gnupg2.git
Testsuite: autopkgtest
Testsuite-Triggers: coreutils, debian-archive-keyring, emacs-nox, findutils, gnupg1, wine32
Build-Depends: debhelper-compat (= 13), dh-sequence-builtusing, automake, autopoint, fig2dev <!nodoc>, file, gettext, ghostscript <!nodoc>, gpgrt-tools, imagemagick (>= 0.1) <!nodoc>, libassuan-dev (>= 2.5.0), libbz2-dev, libc-dev-bin, libgcrypt20-dev (>= 1.9.1), libgnutls28-dev (>= 3.2), libgpg-error-dev (>= 1.46), libksba-dev (>= 1.6.3), libldap2-dev <!pkg.gnupg2.gpgvonly>, libnpth0-dev (>= 1.2), libreadline-dev <!pkg.gnupg2.gpgvonly>, librsvg2-bin <!nodoc>, libsqlite3-dev <!pkg.gnupg2.gpgvonly>, libtss2-dev [linux-any] <!pkg.gnupg2.gpgvonly>, libusb-1.0-0-dev <!pkg.gnupg2.gpgvonly>, openssh-client <!nocheck !pkg.gnupg2.gpgvonly>, pkgconf, swtpm [amd64 arm64 armel armhf i386 mips64el ppc64el riscv64 s390x] <!nocheck !pkg.gnupg2.gpgvonly>, texinfo <!nodoc>, unicode-data, zlib1g-dev | libz-dev
Build-Depends-Indep: binutils-multiarch [!amd64 !i386] <!pkg.gnupg2.gpgvonly>, libassuan-mingw-w64-dev (>= 2.5.0) <!pkg.gnupg2.gpgvonly>, libgcrypt-mingw-w64-dev (>= 1.9.1) <!pkg.gnupg2.gpgvonly>, libgpg-error-mingw-w64-dev (>= 1.50-4~) <!pkg.gnupg2.gpgvonly>, libksba-mingw-w64-dev (>= 1.6.3) <!pkg.gnupg2.gpgvonly>, libnpth-mingw-w64-dev (>= 1.2) <!pkg.gnupg2.gpgvonly>, libz-mingw-w64-dev <!pkg.gnupg2.gpgvonly>, mingw-w64 <!pkg.gnupg2.gpgvonly>
Package-List:
 dirmngr deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gnupg deb metapackages optional arch=all profile=!pkg.gnupg2.gpgvonly
 gnupg-agent deb oldlibs optional arch=all profile=!pkg.gnupg2.gpgvonly
 gnupg-l10n deb localization optional arch=all profile=!pkg.gnupg2.gpgvonly
 gnupg-utils deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gnupg2 deb oldlibs optional arch=all profile=!pkg.gnupg2.gpgvonly
 gpg deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpg-agent deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpg-wks-client deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpg-wks-server deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpgconf deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpgsm deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpgv deb utils important arch=any
 gpgv-static deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 gpgv-udeb udeb debian-installer optional arch=any profile=!noudeb,!pkg.gnupg2.gpgvonly
 gpgv-win32 deb utils optional arch=all profile=!pkg.gnupg2.gpgvonly
 scdaemon deb utils optional arch=any profile=!pkg.gnupg2.gpgvonly
 tpm2daemon deb utils optional arch=linux-any profile=!pkg.gnupg2.gpgvonly
Checksums-Sha1:
 54b823635893c7991d12ee086be0c767d5f9bd4a 5888416 gnupg2_2.4.9.orig.tar.xz
 5c1ee5ea1597ad372e7b89de67b1617033eb5d59 58792 gnupg2_2.4.9-ok1.2.debian.tar.xz
Checksums-Sha256:
 acc99b99323dad48f60c81ae03910cdcff77ea4384fc7eddc784a6099a149a30 5888416 gnupg2_2.4.9.orig.tar.xz
 eb86554c5309e6d8b740a3d132432cd5caada91263e1caf67d0aee42215e3757 58792 gnupg2_2.4.9-ok1.2.debian.tar.xz
Files:
 c5b7ec8968810b65ed2c46ef507f40d1 5888416 gnupg2_2.4.9.orig.tar.xz
 c401170514e6611b2336871ae86f250b 58792 gnupg2_2.4.9-ok1.2.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnfPqEACgkQBxbwD4Qs
0nRp3Qv/bnv7sfGP67bjtIk/D60F3mV4FGsySWlqH2jfj53EUPc6BwgO3KGqcRwT
cmDPEQzgEb99W33go3fZHFz428Ee/Z5vk861IYlxSgGwUvLf4wDbS1cuUJZW/qRz
o0B9q5ZHtoKOD6dhuYouF21VNMtGAkUBtnlgq3iwLtEyUeUahlCy0qKk8yVgu3qu
2TEpJ23ECyTntz7cDMDSeyAn1LaJ8zwIsVPqgnV6n8597mEq5tI3UfoVUnILZnVS
3PemCa0ARg+QXT+MqPQ7m3ThB8d/xzSFxpuEOuWj5D1+8VjvlNSwMfckqDWEWmjY
TeDT8LPuKTdiD63ySdDwpaSRa+AC/OPuTmCEaZO7iIwlZvqfUo5q41q3zxNhwzsO
NMpRjvkLjncEiy1sykk/1HLgyLqa1lUKDk5PKEEgLWosgjPs1C1noWptmR3BgVHR
wcrYx4pCaqttSgF/ZtwQSRv5FXZiNwq+xUh8MLe4mzPrOH+qt+Nr0Cc9AKFQO2Pg
om9smkOb
=Xlna
-----END PGP SIGNATURE-----
