-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: bash
Binary: bash, bash-static, bash-builtins, bash-doc
Architecture: any all
Version: 5.3-ok1
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Homepage: http://tiswww.case.edu/php/chet/bash/bashtop.html
Standards-Version: 4.7.3
Testsuite: autopkgtest
Build-Depends: autoconf, autotools-dev, bison, libncurses5-dev, texinfo, texi2html, debhelper (>= 11), gettext, sharutils, locales <!nocheck>, time <!nocheck>, xz-utils
Build-Depends-Indep: texlive-latex-base, ghostscript, texlive-fonts-recommended, man2html-base
Build-Conflicts: r-base-core
Package-List:
 bash deb shells required arch=any essential=yes
 bash-builtins deb utils optional arch=any
 bash-doc deb doc optional arch=all
 bash-static deb shells optional arch=any
Checksums-Sha1:
 2e1b210a4d7e3969158fcaa03502fe9e4dff0bf5 5571836 bash_5.3.orig.tar.xz
 355b80a0f4487ba989c1ce832fb3873e204c1f56 62696 bash_5.3-ok1.debian.tar.xz
Checksums-Sha256:
 a70de6bb41f5e192534a5a1836b1d7fad9a8d4818a6e1506d70f38441552c17a 5571836 bash_5.3.orig.tar.xz
 cd0fb41b31e21428f32fc0518764ec08ac4f259ddf92a196319ce2b2f5f2ffe7 62696 bash_5.3-ok1.debian.tar.xz
Files:
 a05b24d17da376b93b0bef3f8664845d 5571836 bash_5.3.orig.tar.xz
 a4ae4a8003cdb2c92c67a0938d17f96a 62696 bash_5.3-ok1.debian.tar.xz
Original-Maintainer: Matthias Klose <doko@debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmm3cQYACgkQBxbwD4Qs
0nSyywv9GdvDD72DaP9E38PTmVASZZr6309f1S8IYgefSCszLoF0fJN0HsMZ8Jzd
4BPJepZWQve23Dz5j9WCg/SEY+meoyz/pbltuzgZOAxgQH+Ggq/AobrbsCuPRCGi
4A6BdcokQLfr1QuxYVv8GJ8951oOZKAlIkYTKfYDXqkftqszVtUYrO2Ab/29S9Jd
z6JHvtPpabqpS5X6xUzdevUKMET48GVHmlAYOVA1ivnhf2Wa6zFBEofwwNJ4pnPv
jJRiANPrGcePi+qrA6IT0AebFjQM9e7/ZcZPOVyqKS0cKRQt1Uadt7Fm6gwelz5O
vzxLkLhDY0UALDUpaxcoRM42+XcoYbmgGFPjg4n4TzlyirFk+dPXqYlVRl5gulwf
9qU6Gv4ib2QYx55bClBhjAgX8FGLYQafo5cVtERmplBH42WTFmUIsYdfuoULdJRI
b8CllBjBzV1R+yrT3O6UnjIoRufeHpT3iFlt2ihXNWythm4+UQegNIXGYsxNgsY6
0m3zBSJp
=NON5
-----END PGP SIGNATURE-----
