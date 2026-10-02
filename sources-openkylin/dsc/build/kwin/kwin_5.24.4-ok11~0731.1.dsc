-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (native)
Source: kwin
Binary: kwin-common, kwin-data, kwin-dev, kwin-wayland, kwin-wayland-backend-drm, kwin-wayland-backend-fbdev, kwin-wayland-backend-virtual, kwin-wayland-backend-wayland, kwin-wayland-backend-x11, kwin-x11, libkwineffects13, libkwinglutils13, libkwinxrenderutils13
Architecture: any all
Version: 4:5.24.4-ok11~0731.1
Maintainer: Openkylin Developers <packaging@lists.openkylin.top>
Uploaders: Scarlett Moore <sgmoore@kde.org>, Patrick Franz <patfra71@gmail.com>, Norbert Preining <norbert@preining.info>,
Homepage: https://projects.kde.org/projects/kde/workspace/kwin
Standards-Version: 4.5.1
Vcs-Browser: https://code.launchpad.net/~kubuntu-packagers/kubuntu-packaging/+git/kwin
Vcs-Git: https://git.launchpad.net/~kubuntu-packagers/kubuntu-packaging/+git/kwin
Build-Depends: breeze-dev (>= 4:5.24.4~), cmake (>= 3.16~), debhelper-compat (= 13), dh-python, extra-cmake-modules (>= 5.90.0~), gettext, hwdata, kinit-dev (>= 5.90.0~), kscreenlocker-dev (>= 5.24.4~), libcap-dev, libdrm-dev (>= 2.4.62~), libegl1-mesa-dev (>= 1.2~), libepoxy-dev (>= 1.2), libfontconfig-dev, libfontconfig1-dev, libfreetype-dev, libfreetype6-dev, libgbm-dev, libice-dev, libinput-dev (>= 1.20.0-ok5), libkdecorations2-dev (>= 4:5.24.4-ok4), libkf5completion-dev (>= 5.90.0~), libkf5config-dev (>= 5.90.0~), libkf5configwidgets-dev (>= 5.90.0~), libkf5coreaddons-dev (>= 5.90.0~), libkf5crash-dev (>= 5.90.0~), libkf5declarative-dev (>= 5.90.0~), libkf5doctools-dev (>= 5.90.0~), libkf5globalaccel-dev (>= 5.90.0~), libkf5i18n-dev (>= 5.90.0~), libkf5iconthemes-dev (>= 5.90.0~), libkf5idletime-dev (>= 5.90.0~), libkf5kio-dev (>= 5.90.0~), libkf5newstuff-dev (>= 5.90.0~), libkf5notifications-dev (>= 5.90.0~), libkf5package-dev (>= 5.90.0~), libkf5plasma-dev (>= 5.90.0~), libkf5service-dev (>= 5.90.0~), libkf5textwidgets-dev (>= 5.90.0~), libkf5wayland-dev (>= 4:5.92.0-ok4.1), libkf5widgetsaddons-dev (>= 5.90.0~), libkf5windowsystem-dev (>= 5.92.0-ok5.2), libkf5xmlgui-dev (>= 5.90.0~), liblcms2-dev, libpipewire-0.3-dev, libqaccessibilityclient-qt5-dev, libqt5sensors5-dev (>= 5.15.0~), libqt5x11extras5-dev (>= 5.15.0~), libsm-dev, libudev-dev, libukuiinputgatherclient-dev, libwayland-dev (>= 1.2), libx11-xcb-dev, libxcb-composite0-dev (>= 1.10~), libxcb-cursor-dev, libxcb-damage0-dev (>= 1.10~), libxcb-glx0-dev (>= 1.10~), libxcb-icccm4-dev, libxcb-image0-dev, libxcb-keysyms1-dev, libxcb-randr0-dev (>= 1.10~), libxcb-render0-dev (>= 1.10~), libxcb-shape0-dev (>= 1.10~), libxcb-shm0-dev (>= 1.10~), libxcb-sync-dev (>= 1.10~), libxcb-util-dev, libxcb-xfixes0-dev (>= 1.10~), libxcb-xtest0-dev, libxcb1-dev (>= 1.10~), libxcursor-dev, libxi-dev, libxkbcommon-dev (>= 0.7.0~), pkg-config, pkg-kde-tools (>= 0.15.18~), plasma-wayland-protocols (>= 1.6.0-ok3.1), python3, qtbase5-dev (>= 5.15.0~), qtbase5-private-dev (>= 5.15.0~), qtdeclarative5-dev (>= 5.15.0~), qtscript5-dev (>= 5.15.0~), qttools5-dev (>= 5.15.0~), qtwayland5-dev-tools (>= 5.15.0~), wayland-protocols (>= 1.19~), libgsettings-qt-dev, libxtst-dev, libkysdk-diagnosetest-dev (>= 2.3.1.0), libdisplay-info-dev (>= 0.1.1~), libqt5waylandclient5-dev (>= 5.15.0~), kylin-wayland-protocols (>= 1.0.0-ok3.3)
Package-List:
 kwin-common deb kde optional arch=any
 kwin-data deb kde optional arch=all
 kwin-dev deb libdevel optional arch=any
 kwin-wayland deb kde optional arch=any
 kwin-wayland-backend-drm deb libs optional arch=any
 kwin-wayland-backend-fbdev deb libs optional arch=any
 kwin-wayland-backend-virtual deb libs optional arch=any
 kwin-wayland-backend-wayland deb libs optional arch=any
 kwin-wayland-backend-x11 deb libs optional arch=any
 kwin-x11 deb kde optional arch=any
 libkwineffects13 deb libs optional arch=any
 libkwinglutils13 deb libs optional arch=any
 libkwinxrenderutils13 deb libs optional arch=any
Checksums-Sha1:
 203a83313b996da94dcbde9186ac00fcc0540f4b 6893732 kwin_5.24.4-ok11~0731.1.tar.xz
Checksums-Sha256:
 c0357d981a50037f40bac5e35790f4ae30276238b5c12817e349c9d8b64f4833 6893732 kwin_5.24.4-ok11~0731.1.tar.xz
Files:
 f9ee42edb965eef7863b4b270607fcc7 6893732 kwin_5.24.4-ok11~0731.1.tar.xz
Original-Maintainer: Debian Qt/KDE Maintainers <debian-qt-kde@lists.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmpsY4UACgkQBxbwD4Qs
0nQ46gv/SqAWNUq8T6OKrmB90gNp4SRmH0ICbDz97KE/CKA+UnViZgEQOIIWJGFH
z6Sr5MTqQ1z5y1lWB1bgwRXWHk6HQpqx55B/BEx8DyrTey2kmQChJlfrv6TVnYjG
FeCKm2LdJYaajRhpdrrZB1hv6hBZeluHHfvmnsH/nMi8EtN1R3iD3UnTfE4EgiVb
EtjaJTAhkC6dqCAmPeZLCyEmMcm7jPjkvhSJe9SLa5fpRo0oBh+hf7jXoIqvMjVp
me/hSmoj889p43Wn1dcwvzApnTeM7ccVDEb+jwPUQYDM1G5z2s4g2OgwrMXsRsrw
NFOVnG2uGd4JAvHgStECFRw1PnFZnExly/c7xG5YnfqdpB8SDqXWVoiiOmllLnnK
rvs44jD5D/Eo8yNe/XVRhQdOPVS2L93w4F8aeJJ0n0AGmg+uY8gBQBSAPCzhsZWM
rmdxxzUGOafnGXN8O6wCgsqFX9UaqT8YqFk7aWBosSE3zkTdF5TsWxITge1dUEvP
90hn1rwg
=DWBx
-----END PGP SIGNATURE-----
