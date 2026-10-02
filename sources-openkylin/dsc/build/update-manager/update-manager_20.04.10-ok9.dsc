-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: update-manager
Binary: update-manager-core, python3-update-manager, update-manager
Architecture: all
Version: 1:20.04.10-ok9
Maintainer: Packaging SIG <packaging@lists.openkylin.top>
Standards-Version: 3.8.0
Vcs-Git: http://gitee.com/openkylin/update-manager.git
Testsuite: autopkgtest
Testsuite-Triggers: aptdaemon, pycodestyle, pyflakes3, python3-mock, python3-nose, xvfb
Build-Depends: debhelper (>= 9), dh-python, python3-all (>= 3.3.0-2), python3-distutils-extra (>= 2.38), python3-dbus, python3-distupgrade, python3-distro-info, python3-gi (>= 3.8), python3-yaml, gir1.2-gtk-3.0, gir1.2-snapd-1, lsb-release, apt-clone (>= 0.2.3~ubuntu1)
Build-Depends-Indep: libxml-parser-perl, intltool
Package-List:
 python3-update-manager deb python optional arch=all
 update-manager deb gnome optional arch=all
 update-manager-core deb admin optional arch=all
Checksums-Sha1:
 5da949f54050a14be0d3faf76a22f326bf6774a0 2994973 update-manager_20.04.10.orig.tar.gz
 ee6a5fe794e2aab27dd88c944ae7e95d7b5e0f6e 87420 update-manager_20.04.10-ok9.debian.tar.xz
Checksums-Sha256:
 b4b9cb609dd4ea3c55c71f9860d28b389af7ab57396d56a0f38fef6770e9032b 2994973 update-manager_20.04.10.orig.tar.gz
 cc3eddf805740c16c3c031da50aaa220830423e47d89a03583fb69e0552b3959 87420 update-manager_20.04.10-ok9.debian.tar.xz
Files:
 a5d5093efa4f56475051c59beeceea32 2994973 update-manager_20.04.10.orig.tar.gz
 49f6dd1983fc75973e6eea8d1feb515a 87420 update-manager_20.04.10-ok9.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmNzBsUACgkQBxbwD4Qs
0nT54AwAk2kmZ+IQdQ/xujrosBZb3MYYbhsSFrFbvrqa9Bf0A69K3uF+jsEb+n5Y
qsXy8n2Dvw48cW96iuazMOQUV2nn/Q4RLsMuREw+SS5nxIWOxIMiiRmNJVWXhi/Z
Cq3lFpCSjiHeOKDgJBjxIjrBQ5dQY1U7KflUTu72eTBnPKWhZUgev/nn7/6a5rYp
cs6mLpFze0jDf00XZzcAtZAoV5zb3v+4oR2S9+ZB0DaFvayCBPhH0kZv3/0Y9cbn
qepok2KOMfdj6pU3ABC7/owGBygQjYHEr1anfM7R79pBQV+jfUCDA+IL+3PO75Be
TemoA4cALhPfv5sosg7GPZvb5/VGMv95xQbwERi6r9DlKFqrKYMOFGYNfEa8O3yg
ZYsP1PWRVxHNbYJdtb/Z54yXXGbOsy9L6tBRBU4weEOlBMuEVA9JthPQgAV+mvgn
xvKRB222Tk/DhCW2JrDioPK06xUtcOBnDoVQ7JQ8dtOYj345kTVNNcIXeC+fjwH/
9do74Bnq
=Ve4u
-----END PGP SIGNATURE-----
