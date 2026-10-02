-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: apt
Binary: apt, libapt-pkg6.0t64, apt-doc, libapt-pkg-dev, libapt-pkg-doc, apt-utils, apt-transport-https
Architecture: any all
Version: 3.1.16-ok4
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Standards-Version: 4.1.1
Vcs-Browser: https://gitee.com/openkylin/apt
Vcs-Git: https://gitee.com/openkylin/apt.git
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, aptitude, db-util, dpkg, fakeroot, g++, gdb, gdb-minimal, gnupg, gnupg1, gnupg2, gpgv, gpgv1, gpgv2, libfile-fcntllock-perl, lsof, pkg-config, python3-apt, stunnel4, valgrind, wget
Build-Depends: cmake (>= 3.4), debhelper-compat (= 12), docbook-xml <!nodoc>, docbook-xsl <!nodoc>, dpkg-dev (>= 1.20.8), dpkg-dev (>= 1.22.5) <!pkg.apt.ci>, gettext (>= 0.12), googletest <!nocheck> | libgtest-dev <!nocheck>, libbz2-dev, libdb-dev, libgcrypt20-dev, libgnutls28-dev (>= 3.4.6), liblz4-dev (>= 0.0~r126), liblzma-dev, libseccomp-dev (>= 2.4.2) [amd64 arm64 armel armhf i386 mips mips64el mipsel ppc64el s390x hppa powerpc powerpcspe ppc64 x32], libssl-dev, libsystemd-dev [linux-any], libudev-dev [linux-any], libxxhash-dev (>= 0.8), libzstd-dev (>= 1.0), ninja-build, pkg-config, po4a (>= 0.34-2) <!nodoc>, sqv (>= 1.3.0) [amd64 arm64 armel armhf i386 mips64el ppc64el riscv64 s390x hurd-amd64 hurd-i386 loong64 powerpc ppc64 sparc64] <!pkg.apt.nosqv> | gpgv, triehash, xsltproc <!nodoc>, zlib1g-dev
Build-Depends-Indep: doxygen <!nodoc !pkg.apt.nodoxygen>, graphviz <!nodoc !pkg.apt.nodoxygen>, w3m <!nodoc>
Package-List:
 apt deb admin required arch=any
 apt-doc deb doc optional arch=all profile=!nodoc
 apt-transport-https deb oldlibs optional arch=all
 apt-utils deb admin required arch=any
 libapt-pkg-dev deb libdevel optional arch=any
 libapt-pkg-doc deb doc optional arch=all profile=!nodoc
 libapt-pkg6.0t64 deb libs optional arch=any
Checksums-Sha1:
 f45a71631f524d0afab25056d9535cf742fee2cc 2342304 apt_3.1.16-ok4.tar.xz
Checksums-Sha256:
 0ecd39d70ae1461b1ed49f8977814d2b885b4eb95fe5a3408dc33e94ce49b635 2342304 apt_3.1.16-ok4.tar.xz
Files:
 d6d9acf0580d9525962fb848bda3c9b5 2342304 apt_3.1.16-ok4.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnx1FEACgkQBxbwD4Qs
0nTc1Qv/cnGOYLemstFdKqpoDqMLFubt7MhlivN0fwddZPiSIt0I1Kj4tMPHUUXA
lH9RdBNPqbfSFS2OgJVdG5RbWGygaON15OsioiwuEoZjPPx/lPYkdC5dRbmOI57q
Iu/b/dIjn//6TcjoqiLYOujSY6cCxuIsPajCADjX2NXczYSpPQ5h0lPQf0ZE+bDk
I0Pm3yuM8dxKbgrn8pWNMiEDupzotw85OJzL2IkgvxwzdiEaUPtn461T/K00Vt/7
h47RVmIPJGMC4VusCCOaulID/nCgl0OzqVzkECMHASk7FKFlkpmNl3YdBtdZqOtY
/8qyBFtaRR/fXUeA0piJWaCwCWMPxazclqjDneIECgWIekhgY3Hs15FdNG0OjdUH
S/+jXDG4SN6XU8d2wR2hA9TdXilrZqInZJTPjbgHWxyyjDi0w7VFY6KItuGMgcaX
eHKiuz/jZHlq4bDQ7Nd+3ozqCK147gDZkANqbcAPFK7oRtOk+xQ4Y7OxaGTY+tSG
4jzz4jTT
=iL9q
-----END PGP SIGNATURE-----
