-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: policykit-1
Binary: polkitd, pkexec, policykit-1-doc, libpolkit-gobject-1-0, libpolkit-gobject-1-dev, libpolkit-agent-1-0, libpolkit-agent-1-dev, gir1.2-polkit-1.0
Architecture: any all
Version: 127-ok3
Maintainer: Utopia Maintenance Team <pkg-utopia-maintainers@lists.alioth.debian.org>
Uploaders:  Michael Biebl <biebl@debian.org>, Martin Pitt <mpitt@debian.org>, Simon McVittie <smcv@debian.org>, Luca Boccassi <bluca@debian.org>,
Homepage: https://github.com/polkit-org/polkit/
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/utopia-team/polkit
Vcs-Git: https://salsa.debian.org/utopia-team/polkit.git
Testsuite: autopkgtest
Build-Depends: dbus-daemon <!nocheck>, debhelper (>= 13.11.6~), debhelper-compat (= 13), dh-package-notes, dh-sequence-gir, docbook-xsl, dpkg-build-api (= 1), duktape-dev, gir1.2-gio-2.0-dev, gobject-introspection (>= 1.78.1-9~), intltool, libexpat1-dev, libglib2.0-dev, libpam0g-dev, libselinux1-dev [linux-any], libsystemd-dev [linux-any], systemd-dev [linux-any], meson (>= 0.50.0), mount <!nocheck>, pkgconf, xml-core, xsltproc, python3-dbusmock <!nocheck>
Build-Depends-Indep: gtk-doc-tools <!nodoc>, libglib2.0-doc <!nodoc>, libgtk-3-doc <!nodoc>
Package-List:
 gir1.2-polkit-1.0 deb introspection unknown arch=any
 libpolkit-agent-1-0 deb libs unknown arch=any
 libpolkit-agent-1-dev deb libdevel unknown arch=any
 libpolkit-gobject-1-0 deb libs unknown arch=any
 libpolkit-gobject-1-dev deb libdevel unknown arch=any
 pkexec deb admin unknown arch=linux-any
 policykit-1-doc deb doc unknown arch=all profile=!nodoc
 polkitd deb admin unknown arch=linux-any
Checksums-Sha1:
 7821638babf551ce397e5825f5d2fbd0a3db4a01 472908 policykit-1_127.orig.tar.gz
 bc2708ff9f3bd23e2058e71f32dfbcf1b8cae269 32748 policykit-1_127-ok3.debian.tar.xz
Checksums-Sha256:
 e86df964f6db49d2914c6f96b795dab35ba94332bd4bdcc5b8a07a6758d20b31 472908 policykit-1_127.orig.tar.gz
 2258d9ff409165a56eb929b8d875a65b15f5c96e76ffd6e68b13deca350c0eab 32748 policykit-1_127-ok3.debian.tar.xz
Files:
 ee2853ec8f1e3bceaa1190890312d07b 472908 policykit-1_127.orig.tar.gz
 eb57441ddaf64b13c2b75d039a076cc5 32748 policykit-1_127-ok3.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmoBnAwACgkQBxbwD4Qs
0nT1lgv/QrnTeunFlVcCHJJuYUk80Qkp1lhZUW4/y/tQisVX+l/6g+UVdGrGHoP/
MKI0bBpL0jkz/nas33ZUIiZaYO154j0gQi+0vJtHU1zGieDXJqrbFPuUBOxNRCvn
S6jW6j+GSyh05DPmp9NJOX1TVG7TsfLlGbWN5sMKujZfB7H6Q/sTWpdMrI5wjeIT
3YabSBeHJkzFINvuoKhd9hEM3pE0Z5uLgb2w66vetsLyQZrOXrxDcg+UaiF/73Nt
uFaDn78jdHPRP4SIX3yT+daj5O9To75NUEHDN9NmYmciIzk5iMaB9H8qIXxZVGoE
z1Ka6C74Xr/y5iUVDB6DyeJHcvMh2nOnnOdpccSEsb2VI5YPH8cddrAFEouh+Xat
8cUEZC9kH/5aGZqcwM/jBP8S9ozHMWPHw4V4kHYomAHSNWiQ+ca+P4Vu5MY4Xt+g
LjsjnebIv/33a5JVGU++v2fe60IVOOVuqULaeNQdq9ITZGl5reRoazW1u0ZJP1tt
wCMVsQbS
=cShU
-----END PGP SIGNATURE-----
