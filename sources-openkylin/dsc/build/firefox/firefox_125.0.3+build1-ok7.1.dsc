-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: firefox
Binary: firefox, firefox-dbg, firefox-dev, firefox-mozsymbols, firefox-geckodriver, firefox-locale-af, firefox-locale-an, firefox-locale-ar, firefox-locale-as, firefox-locale-ast, firefox-locale-az, firefox-locale-be, firefox-locale-bg, firefox-locale-bn, firefox-locale-br, firefox-locale-bs, firefox-locale-ca, firefox-locale-cak, firefox-locale-cs, firefox-locale-csb, firefox-locale-cy, firefox-locale-da, firefox-locale-de, firefox-locale-el, firefox-locale-en, firefox-locale-eo, firefox-locale-es, firefox-locale-et, firefox-locale-eu, firefox-locale-fa, firefox-locale-fi, firefox-locale-fr, firefox-locale-fy, firefox-locale-ga, firefox-locale-gd, firefox-locale-gl, firefox-locale-gn, firefox-locale-gu, firefox-locale-he, firefox-locale-hi, firefox-locale-hr, firefox-locale-hsb, firefox-locale-hu, firefox-locale-hy, firefox-locale-ia, firefox-locale-id, firefox-locale-is, firefox-locale-it, firefox-locale-ja, firefox-locale-ka, firefox-locale-kab, firefox-locale-kk,
 firefox-locale-km, firefox-locale-kn, firefox-locale-ko, firefox-locale-ku, firefox-locale-lg, firefox-locale-lt, firefox-locale-lv, firefox-locale-mai, firefox-locale-mk, firefox-locale-ml, firefox-locale-mn, firefox-locale-mr, firefox-locale-ms, firefox-locale-my, firefox-locale-nb, firefox-locale-ne, firefox-locale-nl, firefox-locale-nn, firefox-locale-nso, firefox-locale-oc, firefox-locale-or, firefox-locale-pa, firefox-locale-pl, firefox-locale-pt, firefox-locale-ro, firefox-locale-ru, firefox-locale-si, firefox-locale-sk, firefox-locale-sl, firefox-locale-sq, firefox-locale-sr, firefox-locale-sv, firefox-locale-sw, firefox-locale-szl, firefox-locale-ta, firefox-locale-te, firefox-locale-tg, firefox-locale-th, firefox-locale-tr, firefox-locale-uk, firefox-locale-ur, firefox-locale-uz, firefox-locale-vi, firefox-locale-xh, firefox-locale-zh-hans, firefox-locale-zh-hant,
 firefox-locale-zu
Architecture: any
Version: 125.0.3+build1-ok7.1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Standards-Version: 3.9.1
Testsuite: autopkgtest
Testsuite-Triggers: coreutils, python3-apt, python3-selenium
Build-Depends: cdbs, locales, debhelper (>= 9), autotools-dev, autoconf2.13, zip, libx11-dev, libx11-xcb-dev, libxt-dev, libxext-dev, libgtk-3-dev (>= 3.14), libglib2.0-dev (>= 2.42), libpango1.0-dev (>= 1.14.0), libfontconfig1-dev, libfreetype6-dev (>= 2.0.1), icu-devtools [s390x], libstartup-notification0-dev, libasound2-dev, libcurl4-openssl-dev, libssl-dev, libdbus-glib-1-dev (>= 0.60), lsb-release, libiw-dev, mesa-common-dev, libnotify-dev (>= 0.4), libxrender-dev, libpulse-dev, nasm (>= 2.14.02), yasm (>= 1.1), unzip, dbus-x11, xvfb, python3 (>= 3.8), libffi-dev (>= 3.0.9) [powerpc ppc64el s390x], clang-17 | clang-13 | clang-12 | clang-10, libclang-17-dev | libclang-13-dev | libclang-12-dev | libclang-10-dev, llvm-17-dev | llvm-13-dev | llvm-12-dev | llvm-10-dev, cargo (>= 1.74.0), rustc (>= 1.74.0), nodejs (>= 12.22.12)
Package-List:
 firefox deb web optional arch=any
 firefox-dbg deb debug extra arch=any
 firefox-dev deb oldlibs extra arch=any
 firefox-geckodriver deb web optional arch=any
 firefox-locale-af deb web optional arch=any
 firefox-locale-an deb web optional arch=any
 firefox-locale-ar deb web optional arch=any
 firefox-locale-as deb web optional arch=any
 firefox-locale-ast deb web optional arch=any
 firefox-locale-az deb web optional arch=any
 firefox-locale-be deb web optional arch=any
 firefox-locale-bg deb web optional arch=any
 firefox-locale-bn deb web optional arch=any
 firefox-locale-br deb web optional arch=any
 firefox-locale-bs deb web optional arch=any
 firefox-locale-ca deb web optional arch=any
 firefox-locale-cak deb web optional arch=any
 firefox-locale-cs deb web optional arch=any
 firefox-locale-csb deb web optional arch=any
 firefox-locale-cy deb web optional arch=any
 firefox-locale-da deb web optional arch=any
 firefox-locale-de deb web optional arch=any
 firefox-locale-el deb web optional arch=any
 firefox-locale-en deb web optional arch=any
 firefox-locale-eo deb web optional arch=any
 firefox-locale-es deb web optional arch=any
 firefox-locale-et deb web optional arch=any
 firefox-locale-eu deb web optional arch=any
 firefox-locale-fa deb web optional arch=any
 firefox-locale-fi deb web optional arch=any
 firefox-locale-fr deb web optional arch=any
 firefox-locale-fy deb web optional arch=any
 firefox-locale-ga deb web optional arch=any
 firefox-locale-gd deb web optional arch=any
 firefox-locale-gl deb web optional arch=any
 firefox-locale-gn deb web optional arch=any
 firefox-locale-gu deb web optional arch=any
 firefox-locale-he deb web optional arch=any
 firefox-locale-hi deb web optional arch=any
 firefox-locale-hr deb web optional arch=any
 firefox-locale-hsb deb web optional arch=any
 firefox-locale-hu deb web optional arch=any
 firefox-locale-hy deb web optional arch=any
 firefox-locale-ia deb web optional arch=any
 firefox-locale-id deb web optional arch=any
 firefox-locale-is deb web optional arch=any
 firefox-locale-it deb web optional arch=any
 firefox-locale-ja deb web optional arch=any
 firefox-locale-ka deb web optional arch=any
 firefox-locale-kab deb web optional arch=any
 firefox-locale-kk deb web optional arch=any
 firefox-locale-km deb web optional arch=any
 firefox-locale-kn deb web optional arch=any
 firefox-locale-ko deb web optional arch=any
 firefox-locale-ku deb web optional arch=any
 firefox-locale-lg deb web optional arch=any
 firefox-locale-lt deb web optional arch=any
 firefox-locale-lv deb web optional arch=any
 firefox-locale-mai deb web optional arch=any
 firefox-locale-mk deb web optional arch=any
 firefox-locale-ml deb web optional arch=any
 firefox-locale-mn deb web optional arch=any
 firefox-locale-mr deb web optional arch=any
 firefox-locale-ms deb web optional arch=any
 firefox-locale-my deb web optional arch=any
 firefox-locale-nb deb web optional arch=any
 firefox-locale-ne deb web optional arch=any
 firefox-locale-nl deb web optional arch=any
 firefox-locale-nn deb web optional arch=any
 firefox-locale-nso deb web optional arch=any
 firefox-locale-oc deb web optional arch=any
 firefox-locale-or deb web optional arch=any
 firefox-locale-pa deb web optional arch=any
 firefox-locale-pl deb web optional arch=any
 firefox-locale-pt deb web optional arch=any
 firefox-locale-ro deb web optional arch=any
 firefox-locale-ru deb web optional arch=any
 firefox-locale-si deb web optional arch=any
 firefox-locale-sk deb web optional arch=any
 firefox-locale-sl deb web optional arch=any
 firefox-locale-sq deb web optional arch=any
 firefox-locale-sr deb web optional arch=any
 firefox-locale-sv deb web optional arch=any
 firefox-locale-sw deb web optional arch=any
 firefox-locale-szl deb web optional arch=any
 firefox-locale-ta deb web optional arch=any
 firefox-locale-te deb web optional arch=any
 firefox-locale-tg deb web optional arch=any
 firefox-locale-th deb web optional arch=any
 firefox-locale-tr deb web optional arch=any
 firefox-locale-uk deb web optional arch=any
 firefox-locale-ur deb web optional arch=any
 firefox-locale-uz deb web optional arch=any
 firefox-locale-vi deb web optional arch=any
 firefox-locale-xh deb web optional arch=any
 firefox-locale-zh-hans deb web optional arch=any
 firefox-locale-zh-hant deb web optional arch=any
 firefox-locale-zu deb web optional arch=any
 firefox-mozsymbols deb debug extra arch=amd64,i386
Checksums-Sha1:
 5c8cbb92a26c78bbbb9fe154095e2e45a9721512 813417506 firefox_125.0.3+build1.orig.tar.gz
 3b7b21cc4015b05a2e4522d402b7c7f3583e2712 2474512 firefox_125.0.3+build1-ok7.1.debian.tar.xz
Checksums-Sha256:
 078ba2055fe95bf2326bb4cf5902f97983e8c53092b1ed45cc359177b7469b49 813417506 firefox_125.0.3+build1.orig.tar.gz
 2022e8664a5b47bc42b3a567d28fded2d9ba224820f53c6fa43816f7f52bb97e 2474512 firefox_125.0.3+build1-ok7.1.debian.tar.xz
Files:
 a0e2e23f728f3c0635846b8738dd9cee 813417506 firefox_125.0.3+build1.orig.tar.gz
 8b3f2f8df1fcdaa655887d97ab89799d 2474512 firefox_125.0.3+build1-ok7.1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmc7Km8ACgkQBxbwD4Qs
0nS8Kwv9ExeY3BO/Hm2fGxkKXDg7dt+akRtm70eu8pWUo/1lGcJiBRYyHprjW3B4
KBr0Tfg/IlIGm3PMqEvlOBw5r6a5P1dJd4PsiMeW4q9YuLFR5bIiHjECQnDdJXcS
GEFlmCSXCThxfVGCixm7CN95bfN9pgG0Rp+iROTiZwEq3YJg8t6+0zuhesR+fzbO
OjA/Qu+VtnCisGlqOHNHjE5pgOvc9hr78O8lQODuq8FWEvd9Gw5AABz4npvPnJvK
bDSm5lnp0TXnZDADMm2MNyrSgm59kNwS7MdQ91ttD1HUVFtoq2fbo+KYnH4DN9Qy
DTgtQ21h+qESyR7isOpDyNwrwrNS0Ladg03C8D7KNkbHug+/fgYeP6ZL36McxEQw
SQJt0nmi/LIkr02AiFomryUwLmZ6szcjSSLtmXfhPQTepuCfmFe1M8ex+YTO/Iwd
0YabGysbjnkz57a3wh5uBq+oZzlJtFs0Wd0aiNx03bUkpCkbeTJesxXB83ZmiCM/
m/cpLHD/
=aDhD
-----END PGP SIGNATURE-----
