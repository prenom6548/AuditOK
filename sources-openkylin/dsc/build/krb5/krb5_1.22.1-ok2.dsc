-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: krb5
Binary: krb5-user, krb5-kdc, krb5-kdc-ldap, krb5-admin-server, krb5-kpropd, krb5-multidev, libkrb5-dev, libkrb5-dbg, krb5-pkinit, krb5-otp, krb5-k5tls, krb5-doc, libkrb5-3, libgssapi-krb5-2, libgssrpc4t64, libkadm5srv-mit12, libkadm5clnt-mit12, libk5crypto3, libkdb5-10t64, libkrb5support0, libkrad0, krb5-gss-samples, krb5-locales, libkrad-dev
Architecture: any all
Version: 1.22.1-ok2
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Russ Allbery <rra@debian.org>, Benjamin Kaduk <kaduk@mit.edu>
Homepage: https://web.mit.edu/kerberos/
Standards-Version: 4.7.0
Vcs-Browser: https://salsa.debian.org/debian/krb5
Vcs-Git: https://salsa.debian.org/debian/krb5
Testsuite: autopkgtest
Testsuite-Triggers: ldap-utils, libsasl2-modules-gssapi-mit, slapd
Build-Depends: byacc | bison, comerr-dev, debhelper-compat (= 13), docbook-to-man, dpkg-dev (>= 1.22.5), libkeyutils-dev [linux-any], libldap2-dev <!stage1>, liblmdb-dev, libnss-myhostname, libsasl2-dev <!stage1>, libssl-dev, libverto-dev, pkgconf, ss-dev
Build-Depends-Arch: dpkg-dev (>= 1.22.5), keyutils [linux-any] <!nocheck>, libcmocka-dev <!nocheck>, python3-kdcproxy <!nocheck>, python3-pyrad <!nocheck>
Build-Depends-Indep: doxygen, doxygen-latex, python3, python3-cheetah, python3-lxml, python3-sphinx, tex-gyre
Package-List:
 krb5-admin-server deb net optional arch=any
 krb5-doc deb doc optional arch=all
 krb5-gss-samples deb net optional arch=any
 krb5-k5tls deb net optional arch=any
 krb5-kdc deb net optional arch=any
 krb5-kdc-ldap deb net optional arch=any profile=!stage1
 krb5-kpropd deb net optional arch=any
 krb5-locales deb localization optional arch=all
 krb5-multidev deb libdevel optional arch=any
 krb5-otp deb net optional arch=any
 krb5-pkinit deb net optional arch=any
 krb5-user deb net optional arch=any
 libgssapi-krb5-2 deb libs optional arch=any
 libgssrpc4t64 deb libs optional arch=any
 libk5crypto3 deb libs optional arch=any
 libkadm5clnt-mit12 deb libs optional arch=any
 libkadm5srv-mit12 deb libs optional arch=any
 libkdb5-10t64 deb libs optional arch=any
 libkrad-dev deb libdevel optional arch=any
 libkrad0 deb libs optional arch=any
 libkrb5-3 deb libs optional arch=any
 libkrb5-dbg deb debug optional arch=any
 libkrb5-dev deb libdevel optional arch=any
 libkrb5support0 deb libs optional arch=any
Checksums-Sha1:
 8978d3cc0f4c2b664adf1e51ed18c821321175b1 8643123 krb5_1.22.1.orig.tar.gz
 2bac488ad3af0b5ab9311b99c4e7b507edb055c1 108116 krb5_1.22.1-ok2.debian.tar.xz
Checksums-Sha256:
 11b4d3b957a4f3d282bc4c82ffa094179731a654c33176d707aab4705a07a102 8643123 krb5_1.22.1.orig.tar.gz
 946ac5edce96ee48b3c967d8c55aef85bae9e9cae1aa0a12afb584c6dc5d5d64 108116 krb5_1.22.1-ok2.debian.tar.xz
Files:
 4a80c9c3d8a77148a0be08610021990a 8643123 krb5_1.22.1.orig.tar.gz
 e1352b5e3757c5299a7b4b5846d81086 108116 krb5_1.22.1-ok2.debian.tar.xz
Original-Maintainer: Sam Hartman <hartmans@debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqCbhwACgkQBxbwD4Qs
0nSrUAwAhjMAM811o7Sr438SxDc3rddGd7fkjh0C5g50eXk38+rQbKrgacCTpvlo
lZdFpKFnWyyEc536GmXG/fpruuQ4Rin7HSxgq+wIkmeHnA+sDImmY3C1kbU1oCEx
yNjS0irpdTLMj0RpaXIgNJocsPoYd9krgzrIPUeRtNwDleYHKbQBmtKGZ1HVxZHc
o3YNa/PkkxSR5ZhlpZHRCqMcnTxm9IrMs/+Gu7z+RQpxlrOo2EeKXUCK8JmjfUPI
tCtEJr6RPU/LO1AwHi1HmoNMjykS9PrBBmPW8YYcqmy2FiIln6zYpGvg6n+QF0Wj
mH6PUFG2IpNU1WcguW5kf/FU0gM56vuV3F3uJfhFWaN7ytmKz7DEQb1A1e6Yf8+x
5laoQD7v39cn9PVG8jV3yhAWMpCFxWdZ4ks5b5j6QYtWyG3jHGYNXVYz8liL0K7d
QLKskyjEKlCXDrEJpw9fwlVBsVCkAJVcb91WcsO3FueuUFJYCliAWVU23XArz9Iu
BpZLowGb
=nNKC
-----END PGP SIGNATURE-----
