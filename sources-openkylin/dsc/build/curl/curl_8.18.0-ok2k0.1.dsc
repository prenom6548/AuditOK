-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: curl
Binary: curl, libcurl4t64, libcurl3t64-gnutls, libcurl4-openssl-dev, libcurl4-gnutls-dev, libcurl4-doc
Architecture: any all
Version: 8.18.0-ok2k0.1
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Carlos Henrique Lima Melara <charlesmelara@riseup.net>, Samuel Henrique <samueloph@debian.org>, Sergio Durigan Junior <sergiodj@debian.org>,
Homepage: https://curl.se/
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/debian/curl
Vcs-Git: https://salsa.debian.org/debian/curl.git
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, build-essential, gcc, libc-dev, libcurl-dev, libldap-dev, pkgconf, slapd
Build-Depends: debhelper-compat (= 13), dpkg-dev (>= 1.22.5), autoconf, automake, ca-certificates, gnutls-bin [!ppc64el] <!nocheck>, groff-base, libbrotli-dev, libgnutls28-dev, libidn2-dev, libkrb5-dev, libldap2-dev <!pkg.curl.noldap>, libnghttp2-dev, libpsl-dev, librtmp-dev, libssh2-1-dev, libssl-dev, libtool, libzstd-dev, locales-all <!nocheck>, openssh-server <!nocheck>, python3-impacket [!i386 !alpha !hppa !m68k !sh4] <!nocheck>, python3:native <!nocheck>, quilt, stunnel4 [!i386] <!nocheck>, zlib1g-dev
Build-Conflicts: autoconf2.13, automake1.4
Package-List:
 curl deb web unknown arch=any
 libcurl3t64-gnutls deb libs unknown arch=any
 libcurl4-doc deb doc unknown arch=all
 libcurl4-gnutls-dev deb libdevel unknown arch=any
 libcurl4-openssl-dev deb libdevel unknown arch=any
 libcurl4t64 deb libs unknown arch=any
Checksums-Sha1:
 601f5eb9ec0e1b5ea08fa22361ef914cee10335a 4235746 curl_8.18.0.orig.tar.gz
 1e53d5fa1d456e5c0367d09941a494e1abfc8f74 21628 curl_8.18.0-ok2k0.1.debian.tar.xz
Checksums-Sha256:
 bfaeb060ba7b6240b04e2ffb2360fcb3b74688493dede0b2ad2a2fe5deab926b 4235746 curl_8.18.0.orig.tar.gz
 b9bf5ffed609cb090bda48b089a55f6d90bb010e5deb5d61110758dcc79f5958 21628 curl_8.18.0-ok2k0.1.debian.tar.xz
Files:
 d160def88a8555690ff5250817226c3b 4235746 curl_8.18.0.orig.tar.gz
 2461c08975370ea419a2b92ebdaba581 21628 curl_8.18.0-ok2k0.1.debian.tar.xz
Original-Maintainer: Debian Curl Maintainers <team+curl@tracker.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGyBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmnvJ0AACgkQBxbwD4Qs
0nS+6Qv4wP6gbCUrNjRxWQ6NY1ijxa2wGsF6DpjbGBYKYKdGvDYN1rmrdcxc0m2x
uWzZW2MAx/LLEYFHqqU9oOM08nYfkw2YXpPUz7+yZLC7IB0XckYkfUzsuo314/ZE
Swj2roPdUjUzXUdkbf5fVu6Y4URx64THM7W1HWQ7L9uYY1htKpwap1/c5WGCFJg6
bxTvFYq+aPF+TDtrYVQzJCKhlCFdqRpBZB/ulVVz9bLLdHlRPN1NRJvRxyCm0oFV
U6ak1uCyfd8tWiRrfB1w+mTpfl0KmyMn+XTPDf7ZzTX9zxfOWm3KZ2fp3TYjWBg/
WHSUYF4i3/yKLBN6b5ub09tytJWpRJC3O9fpruUW1WAvUYdXFPKb/CKQMJ3+0HOe
blkUCDImXpEknxY4kEKw4spCSiVC8EaaR/WU5hY0+KSnVp5iocTDsdC4+VPsJu7W
8T1ic1bAB49gOOS1EGin1HwcCFblXZ3WaSu7Nd1OiwKLLv9pDQYtLfPB86A32gaa
A+t7A7E=
=jOKu
-----END PGP SIGNATURE-----
