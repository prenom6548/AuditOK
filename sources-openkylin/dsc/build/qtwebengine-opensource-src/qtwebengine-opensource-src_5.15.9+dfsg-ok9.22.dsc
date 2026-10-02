-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: qtwebengine-opensource-src
Binary: qtwebengine5-dev, qtwebengine5-private-dev, qtpdf5-dev, libqt5webengine5, libqt5webenginecore5, libqt5webenginewidgets5, libqt5pdf5, libqt5pdfwidgets5, libqt5webengine-data, qt5-image-formats-plugin-pdf, qml-module-qtwebengine, qml-module-qtquick-pdf, qtwebengine5-dev-tools, qtwebengine5-examples, qtpdf5-examples, qtwebengine5-doc, qtwebengine5-doc-html, qtpdf5-doc, qtpdf5-doc-html
Architecture: amd64 arm64 armhf i386 mips64el mipsel riscv64 loong64 all
Version: 5.15.9+dfsg-ok9.22
Maintainer: Debian Qt/KDE Maintainers <debian-qt-kde@lists.debian.org>
Uploaders: Scarlett Moore <sgmoore@kde.org>, Sandro Knauß <hefee@debian.org>, Dmitry Shachnev <mitya57@debian.org>, Simon Quigley <tsimonq2@debian.org>
Homepage: https://doc.qt.io/qt-5/qtwebengine-index.html
Standards-Version: 4.6.0
Vcs-Browser: https://salsa.debian.org/qt-kde-team/qt/qtwebengine
Vcs-Git: https://salsa.debian.org/qt-kde-team/qt/qtwebengine.git
Build-Depends: binutils (>= 2.32-0), bison, chrpath, debhelper-compat (= 13), flex, gperf, khronos-api, libasound2-dev [linux-any], libavcodec-dev (>= 7:4.0~), libavformat-dev (>= 7:4.0~), libavutil-dev (>= 7:4.0~), libcap-dev [linux-any], libdbus-1-dev, libegl1-mesa-dev, libevent-dev, libflac-dev, libfontconfig1-dev, libgl-dev (>= 1.3) [!armel !armhf], libgl1-mesa-dri, libgles-dev [armel armhf], libglib2.0-dev, libglu1-mesa-dev [!armel !armhf] | libglu-dev [!armel !armhf], libgstreamer-plugins-base1.0-dev, libgstreamer1.0-dev, libharfbuzz-dev, libicu-dev (>= 64~), libjpeg-dev, libjs-d3, libjsoncpp-dev, libkrb5-dev, liblcms2-dev, libminizip-dev, libnss3-dev, libopus-dev (>= 1.3.1), libpci-dev, libpng-dev, libprotobuf-dev, libpulse-dev, libqt5opengl5-dev (>= 5.15.3+dfsg~), libqt5svg5-dev (>= 5.15.3~), libqt5webchannel5-dev (>= 5.15.3~), libsnappy-dev, libsqlite3-dev, libusb-1.0-0-dev, libvpx-dev (>= 1.8), libwebp-dev, libx11-xcb-dev, libxcb-dri3-dev, libxcomposite-dev, libxcursor-dev, libxdamage-dev, libxkbfile-dev, libxml2-dev, libxnvctrl-dev, libxrandr-dev, libxrender-dev, libxslt1-dev, libxss-dev, libxtst-dev, mesa-common-dev, ninja-build, node-pako, node-rollup-plugin-terser, node-yargs, nodejs, pkg-config, pkg-kde-tools, protobuf-compiler, python2, qtbase5-dev (>= 5.15.3+dfsg~), qtbase5-private-dev (>= 5.15.3+dfsg~), qtdeclarative5-private-dev (>= 5.15.3+dfsg~), qtpositioning5-dev (>= 5.15.3+dfsg~), qtquickcontrols2-5-dev (>= 5.15.3+dfsg~), qttools5-dev (>= 5.15.3~), re2c, rollup, ruby, xauth, xvfb, yui-compressor
Build-Depends-Indep: qdoc-qt5 (>= 5.15.3~) <!nodoc>, qhelpgenerator-qt5 (>= 5.15.3~) <!nodoc>, qtattributionsscanner-qt5 (>= 5.15.3~) <!nodoc>, qtbase5-doc-html (>= 5.15.3+dfsg~) <!nodoc>
Package-List:
 libqt5pdf5 deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 libqt5pdfwidgets5 deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 libqt5webengine-data deb libs optional arch=all
 libqt5webengine5 deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 libqt5webenginecore5 deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 libqt5webenginewidgets5 deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qml-module-qtquick-pdf deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qml-module-qtwebengine deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qt5-image-formats-plugin-pdf deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtpdf5-dev deb libdevel optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtpdf5-doc deb doc optional arch=all profile=!nodoc
 qtpdf5-doc-html deb doc optional arch=all profile=!nodoc
 qtpdf5-examples deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtwebengine5-dev deb libdevel optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtwebengine5-dev-tools deb devel optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtwebengine5-doc deb doc optional arch=all profile=!nodoc
 qtwebengine5-doc-html deb doc optional arch=all profile=!nodoc
 qtwebengine5-examples deb libs optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
 qtwebengine5-private-dev deb libdevel optional arch=amd64,arm64,armhf,i386,mips64el,mipsel,riscv64,loong64
Checksums-Sha1:
 1d38320372c39706aff704e90f10eabc918f7ddc 285389632 qtwebengine-opensource-src_5.15.9+dfsg.orig.tar.xz
 02e699d075b754e8ef40e865eda8013536bb7059 507224 qtwebengine-opensource-src_5.15.9+dfsg-ok9.22.debian.tar.xz
Checksums-Sha256:
 c9bda55109375f2ef8e2c5d4073fe2606f1e3b78d26d332fc17d2dc71a6405f8 285389632 qtwebengine-opensource-src_5.15.9+dfsg.orig.tar.xz
 dfce728e9181d0bf075b1438b254a7823ba22a1642c0f78ca9eedd60d48b075c 507224 qtwebengine-opensource-src_5.15.9+dfsg-ok9.22.debian.tar.xz
Files:
 b7d38ee9ff61147572bee588717ab5a6 285389632 qtwebengine-opensource-src_5.15.9+dfsg.orig.tar.xz
 a744b68d527723eb9090813bab2a6fe8 507224 qtwebengine-opensource-src_5.15.9+dfsg-ok9.22.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqEC0QACgkQBxbwD4Qs
0nSaugv/YHkRMljdoGqJ+VRLNOoxCQ2voAPLJhYaFMfT8OuKlrIUi3XdFzrRwWBW
ssbtJg2Rar7MWoB+G9tjNIZSt9yyaTyab8as/CnwZCslzUopyaECMFQpI4Y5kofP
Yg6M/d2tMh2vR6fW5CgEbKhGrYxIwYVkypFmYXcDzIDz0b9CLSksNQ8eLI53akbQ
t6UAS5TUVFsVHI7E7BV7TTzB5seM44Ut2G8UndgrR4JBCBbAThzrHfWr1M3qWLEy
RBwCxxBbKqEOTxy8njR5CCKzJJxb0Ki4YriDsArNQVPNR3XRrUOdMQOAQ1qnivCl
XmlkaZ3sHEzVQPK7mICA0YJeAsND4ICq+c1RUB69dKiyK/qM75fB0mTfmlcIHZsy
AgXaCcw8hKczevWcLZWxA1MHasrGgSdIvwGtNTeOYzZa1omx4ECe/2iYRsxEdgDH
eIOTAgoqi1+1KKfpgUoZe5k4s9HykEsAeIA4rK0XE+3SP1GwjsTVNjDcN5OmQtyH
buE3JRLK
=ZAkB
-----END PGP SIGNATURE-----
