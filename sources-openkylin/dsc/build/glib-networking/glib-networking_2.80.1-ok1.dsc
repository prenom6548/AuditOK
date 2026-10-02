-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: glib-networking
Binary: glib-networking, glib-networking-services, glib-networking-common, glib-networking-tests
Architecture: any all
Version: 2.80.1-ok1
Maintainer: openKylin Developers <packaging@lists.openkylin.top>
Uploaders: Iain Lane <laney@debian.org>, Jeremy Bícha <jbicha@ubuntu.com>
Standards-Version: 4.7.0
Vcs-Browser: https://salsa.debian.org/gnome-team/glib-networking
Vcs-Git: https://salsa.debian.org/gnome-team/glib-networking.git
Testsuite: autopkgtest
Testsuite-Triggers: dbus, gnome-desktop-testing, xauth, xvfb
Build-Depends: debhelper-compat (= 13), ca-certificates <!nocheck>, dh-sequence-gnome, gsettings-desktop-schemas-dev, meson (>= 0.60.0), libglib2.0-dev (>= 2.73.3), libgnutls28-dev (>= 3.7.4), libproxy-dev (>= 0.5)
Package-List:
 glib-networking deb libs optional arch=any
 glib-networking-common deb libs optional arch=all
 glib-networking-services deb libs optional arch=any
 glib-networking-tests deb misc optional arch=any
Checksums-Sha1:
 7ca7f9eaaeaf8a7125c8baffc136bb1283c188a5 290828 glib-networking_2.80.1.orig.tar.xz
 b703093dc80ce7c5902e7d007e054444a06805a4 3252 glib-networking_2.80.1-ok1.debian.tar.xz
Checksums-Sha256:
 b80e2874157cd55071f1b6710fa0b911d5ac5de106a9ee2a4c9c7bee61782f8e 290828 glib-networking_2.80.1.orig.tar.xz
 788615c0f5c4f347aebfa0e2927f1c0a408a1f8e314bfb1187729ac92c9b184a 3252 glib-networking_2.80.1-ok1.debian.tar.xz
Files:
 405e6c058723217a1307ba8415615f9d 290828 glib-networking_2.80.1.orig.tar.xz
 7220296a6ff8840e877efa1389ff58e5 3252 glib-networking_2.80.1-ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmodVHoACgkQBxbwD4Qs
0nSEtwv+K5+x83vT93uAEfQFJtP8UhJehjpwMX7l4X8HWwz1jXsBb4gd7TU24g62
jx0iy6GV5w+8BI5ggqL0dJNbL9fLDpusY2nOuPqMTgN/k85sKzduoVQYhhNCcmiO
vInJP6jgazm9j1LHG9BbYmvHVrr8vX6YepKL1LsXlYcY8rhl1juZ4ncaF9ew3+cu
XHgXfzgkF4S4A3N80trjj547gV2c4GVrwsFYRfBmkjjY/ykL3kjAEBlAhNroytl5
CSLY5EEy1OJ1j1ts4VWMiqasEF868scShR8WjkLcuFlMpp8poWORopZ7CsnQQnJS
1NXhkw6cWWZn0SjOYUxU4wHStCk24uLPWuSnzWKs3D2Es1NgphUipu2y+hHM/wp2
0fWqo8dSMW+OG1vGvjdzEB8HxV9RgK/MQs0s61TfqdFqyk9JGIsgwtbhPVOzw4jV
tK6zkXISGwq0qIdjV81IO8XOpP04Qowzm9rrBtIoUWU0s7FYLP9gC+PPE0m96ynP
cpkZRyYW
=gkb1
-----END PGP SIGNATURE-----
