-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: dpkg
Binary: dpkg, libdpkg-dev, dpkg-dev, libdpkg-perl, dselect
Architecture: any all
Version: 1.23.7-ok3
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Homepage: https://wiki.debian.org/Teams/Dpkg
Standards-Version: 4.7.3
Vcs-Browser: https://gitee.com/openkylin/dpkg
Vcs-Git: https://gitee.com/openkylin/dpkg.git
Testsuite: autopkgtest
Testsuite-Triggers: autoconf, build-essential, eatmydata, file, libmd-dev, pkgconf
Build-Depends: debhelper-compat (= 13), debhelper (>= 13.10~), pkgconf, gettext (>= 0.19.7), po4a (>= 0.59), libmd-dev, zlib1g-dev, libbz2-dev, liblzma-dev (>= 5.4.0), libzstd-dev (>= 1.4.0), libselinux-dev [linux-any] | libselinux1-dev [linux-any], libncurses-dev (>= 6.1+20180210), bzip2 <!nocheck>, xz-utils (>= 5.4.0) <!nocheck>, zstd <!nocheck>, git <pkg.dpkg.author-release>, ca-certificates <pkg.dpkg.author-release>, libmodule-build-perl <pkg.dpkg.author-release>, fakeroot <pkg.dpkg.author-testing>, sq <pkg.dpkg.author-testing>, sqv (>= 1.3.0~) <pkg.dpkg.author-testing>, sqop <pkg.dpkg.author-testing>, sqopv <pkg.dpkg.author-testing>, rsop <pkg.dpkg.author-testing>, rsopv <pkg.dpkg.author-testing>, gosop <pkg.dpkg.author-testing>, gpg-sq <pkg.dpkg.author-testing>, gpgv-sq <pkg.dpkg.author-testing>, gnupg <pkg.dpkg.author-testing>, cppcheck <pkg.dpkg.author-testing>, shellcheck <pkg.dpkg.author-testing>, aspell <pkg.dpkg.author-testing>, aspell-en <pkg.dpkg.author-testing>, codespell <pkg.dpkg.author-testing>, i18nspector <pkg.dpkg.author-testing>, libtest-minimumversion-perl <pkg.dpkg.author-testing>, libtest-perl-critic-perl <pkg.dpkg.author-testing>, libtest-pod-coverage-perl <pkg.dpkg.author-testing>, libtest-pod-perl <pkg.dpkg.author-testing>, libtest-spelling-perl <pkg.dpkg.author-testing>, libtest-strict-perl <pkg.dpkg.author-testing>, libtest-synopsis-perl <pkg.dpkg.author-testing>, lcov <pkg.dpkg.code-coverage>, libdevel-cover-perl <pkg.dpkg.code-coverage>
Package-List:
 dpkg deb admin required arch=any essential=yes
 dpkg-dev deb utils optional arch=all
 dselect deb admin optional arch=any
 libdpkg-dev deb libdevel optional arch=any
 libdpkg-perl deb perl optional arch=all
Checksums-Sha1:
 57cc6e79780fee0af69740d16cc6cbdbf95a0b18 12485193 dpkg_1.23.7.orig.tar.gz
 85263cd6a10e4d4a22e302f9810cebc7a0fe83ac 16900 dpkg_1.23.7-ok3.debian.tar.xz
Checksums-Sha256:
 a86894a012f604092b2db684a2e84182ad74ea8942944f0089170b82d1d97cf1 12485193 dpkg_1.23.7.orig.tar.gz
 eacbe398eeedc45d3472bd4bff398aba0b308054015d8c39bbf741bfbdc11afb 16900 dpkg_1.23.7-ok3.debian.tar.xz
Files:
 1ccf40c4ab3e75fdbe05a36dd2d0a5c7 12485193 dpkg_1.23.7.orig.tar.gz
 fe4f8598a994b1d8b955a0d1b61bc15d 16900 dpkg_1.23.7-ok3.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmo7nxMACgkQBxbwD4Qs
0nQqZwv5AWQNwmj/DPfP6RyQbM8Z8mGvz3Lj8WiD2GedlF5PhzEZdNlwgj8nQqoW
sOUAuCWdJoXYUqueJWo4m3Am+1f2FACz4kTqJYpahF0e9//rWC6Q0Pxtu7Re7BHY
VylivQA8Oa0QB4RnyevcJ8v+ZPCPLvqRxGISVwz9RxZVOK8sYsyMA7gMWTMXZ6BR
CunI6s/yaitBnFhXaw/okBza9Xb/Xl6JvW8UcGMy+5iTWquzOpTryQeZtkePKK1W
V7OdoU04NfS34nWn3lgxD5XLmGn+L02FUrcPHmxMexu3aj/QYWNZw4ifpi+eX8Fw
9m1P8gGZeTzVj8h7HyC9n0cqHe4isUcKp+ch2UJIDyGgD2R4fs/ZjdqheGA0kIRa
rQY3blQltq/+ZvphmJSPNmkZXiSWhohkJONmk/zee1NiCj6GtJK46FfFOUKHcqdB
MKaufp85DvzH2lohkXHIdU1It67qgLadFIxuvq4ucWSaUprHMGvR08XJ9phkWFqL
lhueJxWc
=fs/w
-----END PGP SIGNATURE-----
