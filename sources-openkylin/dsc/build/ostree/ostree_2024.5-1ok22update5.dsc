-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: ostree
Binary: gir1.2-ostree-1.0, libostree-1-1, libostree-dev, libostree-doc, ostree, ostree-boot, ostree-tests
Architecture: linux-any all
Version: 2024.5-1ok22update5
Maintainer: Utopia Maintenance Team <pkg-utopia-maintainers@lists.alioth.debian.org>
Uploaders:  Matthias Klumpp <mak@debian.org>, Simon McVittie <smcv@debian.org>, Sjoerd Simons <sjoerd@debian.org>
Homepage: https://github.com/ostreedev/ostree/
Standards-Version: 4.6.2
Vcs-Browser: https://salsa.debian.org/debian/ostree
Vcs-Git: https://salsa.debian.org/debian/ostree.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, gnome-desktop-testing, pkgconf
Build-Depends: attr, autoconf, autotools-dev, gettext, automake, bison, bsdextrautils <!nocheck>, ca-certificates, cpio, debhelper (>= 13.11.6~), debhelper-compat (= 13), dh-sequence-gir, docbook-xml <!nodoc>, docbook-xsl <!nodoc>, libext2fs-dev, elfutils, fuse3:native, gir1.2-gio-2.0-dev, gjs:native [!alpha !sw64 !hppa !ia64 !m68k !sh4 !sparc64 !x32], gnupg <!nocheck>, gobject-introspection, gobject-introspection (>= 1.78.1-7~) <cross>, gtk-doc-tools <!nodoc>, libarchive-dev, libjson-glib-dev, libattr1-dev, libavahi-client-dev, libavahi-glib-dev, libcap-dev, libcurl4-gnutls-dev | libcurl-dev, libfuse3-dev, libglib2.0-dev, libgpgme-dev, liblzma-dev, libmount-dev, libselinux1-dev, libsoup-3.0-dev (>= 3.0.0), libsystemd-dev, libtool, pkgconf, procps, python3 <!nocheck>, python3-yaml <!nocheck>, xsltproc <!nodoc>, zlib1g-dev
Build-Depends-Indep: libglib2.0-doc
Package-List:
 gir1.2-ostree-1.0 deb introspection optional arch=linux-any
 libostree-1-1 deb libs optional arch=linux-any
 libostree-dev deb libdevel optional arch=linux-any
 libostree-doc deb doc optional arch=all profile=!nodoc
 ostree deb admin optional arch=linux-any
 ostree-boot deb admin optional arch=linux-any
 ostree-tests deb admin optional arch=linux-any
Checksums-Sha1:
 c34f06034514f92a756310f08038dc8c3551ca70 2230572 ostree_2024.5-1ok22update5.tar.xz
Checksums-Sha256:
 f5680ea01ea771953f5d1400a98c39bfdbadac7d6b67704da7db316468fea859 2230572 ostree_2024.5-1ok22update5.tar.xz
Files:
 c8bb60798508eaf0ee716c7daa087729 2230572 ostree_2024.5-1ok22update5.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqfcOYACgkQBxbwD4Qs
0nTmCwv+NHKXSwG3VepaiQm6zueKwZqLpgolJAld14IdbrvDD8kvuMhovWzewG3u
6T+K3x0dGxgnSTHEAOAU4zsAmZ4xG9wvS53LIIwrLq5qR3DbsmPV+GB6ZMrJya2G
v4UBC4UVlWlsVPEs+heaaeYwIBzM0clV7XrDjQaacBexGejbHE44DO82f69hglLG
qFGJi5+Cn5engVxAZ0fYdp/7bf3+HmXH3HPNhf65/+dPCYai68GxxLC72o/IqI/Z
Q9Ky+81pUVi1tLeWSY0apigIggppM9On53JSQmHQZ+Zc00ci7bOtYETxYjt3bi5X
9NHHPgNCYX+GNG5SQDS7Ft4NqMrVEsPtCgn7ZsDWGK9FOU1IPheZt8qBOQARTTaG
48rZLs/dYMuSmLPyDuwHvIlPHIRY76tfSKPgVLbGtB5iFxiLunbuOh3jLUFZaO82
nBcWvlt+54MkfGUJD26pn/wNj8TN3U4jI9BvERUfO4AYkFRi9UDR9BtFMq0ta5YR
ErQRAbUa
=NsOk
-----END PGP SIGNATURE-----
