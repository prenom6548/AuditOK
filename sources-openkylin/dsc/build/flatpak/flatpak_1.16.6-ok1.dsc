-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: flatpak
Binary: flatpak, flatpak-tests, gir1.2-flatpak-1.0, libflatpak-dev, libflatpak-doc, libflatpak0
Architecture: linux-any all
Version: 1.16.6-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders:  Matthias Klumpp <mak@debian.org>, Simon McVittie <smcv@debian.org>,
Homepage: https://flatpak.org/
Standards-Version: 4.7.3
Vcs-Browser: https://salsa.debian.org/debian/flatpak
Vcs-Git: https://salsa.debian.org/debian/flatpak.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, gettext, gnome-desktop-testing, python3
Build-Depends: debhelper (>= 13.11.6~), debhelper-compat (= 13), dh-exec (>= 0.23~), dh-sequence-gir, attr <!nocheck>, bison, bubblewrap (>= 0.10.0~), ca-certificates <!nocheck>, curl <!nocheck>, dbus-daemon, desktop-file-utils <!nocheck>, docbook-xml <!nodoc>, docbook-xsl <!nodoc>, fuse3, gir1.2-gio-2.0-dev, gir1.2-gobject-2.0-dev, gnupg <!nocheck>, gobject-introspection (>= 1.80), gtk-update-icon-cache <!nocheck>, hicolor-icon-theme <!nocheck>, libappstream-dev (>= 0.12.0), libarchive-dev (>= 2.8.0), libattr1-dev, libcap-dev, libcurl4-openssl-dev | libcurl-dev, libdconf-dev, libdw-dev, libelf-dev, libfuse3-dev (>= 3.1.1), libgdk-pixbuf-2.0-dev, libglib2.0-dev (>= 2.44), libgpgme-dev (>= 1.1.8), libjson-glib-dev, libmalcontent-0-dev (>= 0.4.0), libostree-dev (>= 2020.8), libpolkit-agent-1-dev, libpolkit-gobject-1-dev, libseccomp-dev (>= 2.5.2), libsystemd-dev, libwayland-dev, libxau-dev, libxml2-dev (>= 2.4), libxml2-utils, libzstd-dev, meson (>= 0.53.0), ostree (>= 2020.8) <!nocheck>, pkgconf, polkitd <!nocheck> <!noinsttest>, procps, python3-pyparsing, python3:any, shared-mime-info <!nocheck>, socat <!nocheck> <!noinsttest>, wayland-protocols, xdg-dbus-proxy (>= 0.1.0), xmlto <!nodoc>, xsltproc <!nodoc>
Build-Depends-Indep: gtk-doc-tools <!nodoc>, libglib2.0-doc <!nodoc>, libostree-doc <!nodoc>
Package-List:
 flatpak deb admin unknown arch=linux-any
 flatpak-tests deb misc unknown arch=linux-any profile=!noinsttest
 gir1.2-flatpak-1.0 deb introspection unknown arch=linux-any
 libflatpak-dev deb libdevel unknown arch=linux-any
 libflatpak-doc deb doc unknown arch=all profile=!nodoc
 libflatpak0 deb libs unknown arch=linux-any
Checksums-Sha1:
 735ac6e954b284d9eeaadcd260b4a20483534323 1242088 flatpak_1.16.6.orig.tar.xz
 ea4f197fe73b7a7427c11f5d38fa89ede3df44fe 14524 flatpak_1.16.6-ok1.debian.tar.xz
Checksums-Sha256:
 1e63e7f3fe44b602f34d92a6fe46fd8a3bc6be9460c03c2681e57976c658eec3 1242088 flatpak_1.16.6.orig.tar.xz
 a68bd09231ddb6872ad307a94c213a4126b8c6752042772021f5414477728547 14524 flatpak_1.16.6-ok1.debian.tar.xz
Files:
 4c18bbd3a7eb15232030605165b263e3 1242088 flatpak_1.16.6.orig.tar.xz
 9f3988f8e352e88683d2a7d954e5aee7 14524 flatpak_1.16.6-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEIY28jELT8B2BBl7CzkkjbC042SIFAmn7vJIACgkQzkkjbC04
2SKcQQwAsUl0rh8ly0IHwUpUECCl8Owk2zFwHQ2vIVQzwLcBYTybKAOBJz6I7pqr
LMjA6f8OJs7fCvh57CiQUjo1aaLiU4ZpIVOi/NB/mONqXXVhKPvHnR2f5Q2n7PMq
HdTvnvun87Hc/wTBW4ybniGP8rX2sLKEO3CofRU7yJuKlIaOet/w8OzLRooLVtjA
xoDQPUsOSjd+aVFGDy6ZMNVbxdPtfgaZGE7nTgBQFH9vU+AHrpfNnRYuSQ3v7NQP
5cLrg65UUt9BGl1X1FMje0SluIeFH878xlPP2VvHHQm1733gMzS8WfReNCD4C31I
k88f/FGQU25Y3Rq9JNVwg2spONy0/Zu1HUf2umXIfd0K5m56A/jkMDhAZ7tpUG9s
LDOpJoDSswt7Te5Sif9Yb1SojwDv/Zb3eg3Ieyhar8oOtFYQUAbj9HzqRzcbeBiI
iJBDFWaWoM5/H8XMoTY9Szn64LiLmXnG2+LbE7Z+zYgnfx/zobaRQb3O9d+Ut9p7
ktjs6FAe
=NRld
-----END PGP SIGNATURE-----
