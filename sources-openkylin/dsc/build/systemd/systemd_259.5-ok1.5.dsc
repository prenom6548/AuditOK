-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: systemd
Binary: systemd, systemd-sysv, systemd-container, systemd-journal-remote, systemd-coredump, systemd-timesyncd, systemd-tests, libpam-systemd, libnss-myhostname, libnss-mymachines, libnss-resolve, libnss-systemd, libsystemd0, libsystemd-dev, libsystemd-shared, udev, libudev1, libudev-dev, systemd-standalone-sysusers, systemd-standalone-tmpfiles, systemd-standalone-shutdown, systemd-oomd, systemd-userdbd, systemd-homed, systemd-boot, systemd-boot-tools, systemd-boot-efi, systemd-ukify, systemd-resolved, systemd-dev, systemd-repart, systemd-cryptsetup
Architecture: linux-any loong64 all
Version: 259.5-ok1.5
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Marco d'Itri <md@linux.it>, Sjoerd Simons <sjoerd@debian.org>, Martin Pitt <mpitt@debian.org>, Felipe Sateler <fsateler@debian.org>, Luca Boccassi <bluca@debian.org>, Nick Rosbrook <enr0n@ubuntu.com>,
Homepage: https://systemd.io
Standards-Version: 4.7.2
Vcs-Browser: https://git.launchpad.net/~ubuntu-core-dev/ubuntu/+source/systemd
Vcs-Git: https://git.launchpad.net/~ubuntu-core-dev/ubuntu/+source/systemd
Testsuite: autopkgtest
Testsuite-Triggers: acl, apparmor, attr, bash, bind9-dnsutils, bind9-host, bsdextrautils, bsdutils, btrfs-progs, bubblewrap, build-essential, busybox-static, ca-certificates, cron, cryptsetup-bin, curl, dbus-user-session, debian-archive-keyring, dhcpcd-base, dhcpcd5, dmeventd, dnsmasq-base, dosfstools, e2fsprogs, erofs-utils, evemu-tools, fdisk, gdm3, git, gnupg, gnutls-bin, gperf, iproute2, iputils-ping, jq, kbd, knot, knot-dnssecutils, knot-dnsutils, kpartx, less, libacl1-dev, libapparmor-dev, libarchive-dev, libblkid-dev, libcap2-bin, libcrypt-dev, libcryptsetup-dev, libdw-dev, libelf-dev, libfido2-dev, libgcrypt20-dev, libidn2-dev, libkmod-dev, libmount-dev, libp11-kit-dev, libpam0g-dev, libpcre2-dev, libpwquality-dev, libqrencode-dev, libseccomp-dev, libselinux1-dev, libssl-dev, libtss2-dev, linux-image-amd64, linux-image-arm64, linux-image-generic, locales, locales-all, lvm2, lz4, mdadm, meson, mkosi, mount, mtools, multipath-tools, net-tools, netcat-openbsd, netlabel-tools, network-manager, nftables, nvme-cli, open-iscsi, opensc, openssh-client, openssh-server, openssl, ovmf, ovmf-ia32, perl, pkgconf, plymouth, polkitd, psmisc, python3, python3-colorama, python3-cryptography, python3-jinja2, python3-pefile, python3-pexpect, qemu-efi-aarch64, qemu-efi-arm, qemu-efi-loongarch64, qemu-efi-riscv64, qemu-system-arm, qemu-system-misc, qemu-system-ppc, qemu-system-s390x, qemu-system-x86, quota, reprepro, rsyslog, sbsigntool, screen, seabios, socat, softhsm2, squashfs-tools, strace, stress, swtpm, swtpm-tools, sysvinit-utils, tgt, tpm2-tools, tree, tzdata, tzdata-legacy, ubuntu-keyring, uidmap, xkb-data, xserver-xorg, xserver-xorg-video-dummy, xz-utils, zstd
Build-Depends: debhelper-compat (= 13), debhelper (>= 13.11.6), dh-exec, dh-package-notes, dh-sequence-dlopenlibdeps <!stage1>, dh-sequence-installnss, dh-sequence-installsysusers, dh-dlopenlibdeps, pkgconf, xsltproc <!nodoc>, docbook-xsl <!nodoc>, docbook-xml <!nodoc>, meson (>= 0.60.0), rsync | meson (>= 1.3.0~), clang:native [amd64 arm64 ppc64el s390x] <!pkg.systemd.nobpf !stage1>, linux-bpf-dev [amd64 arm64 ppc64el s390x] <!pkg.systemd.nobpf !stage1>, libbpf-dev [amd64 arm64 ppc64el s390x] <!pkg.systemd.nobpf !stage1>, bpftool:native [amd64 arm64 ppc64el s390x] <!pkg.systemd.nobpf !stage1 !pkg.systemd.upstream>, linux-tools-generic <pkg.systemd.upstream> | bpftool <pkg.systemd.upstream>, gettext, gperf, libcrypt-dev, libpam0g-dev, libapparmor-dev (>= 2.13) <!stage1>, libidn2-dev <!stage1>, libiptc-dev <!stage1>, libaudit-dev <!stage1>, libdbus-1-dev (>= 1.3.2) <!nocheck> <!noinsttest>, libcryptsetup-dev (>= 2:2.4) <!stage1>, libselinux1-dev (>= 2.1.9), libacl1-dev, liblzma-dev, liblz4-dev (>= 1.3.0), lz4 <!nocheck>, libbz2-dev <!stage1>, zlib1g-dev <!stage1> | libz-dev <!stage1>, libcurl4-openssl-dev <!stage1>, libmicrohttpd-dev <!stage1>, libpcre2-dev <!stage1>, libgcrypt20-dev, libkmod-dev (>= 15), libblkid-dev (>= 2.24), libmount-dev (>= 2.30), libfdisk-dev (>= 2.33), libseccomp-dev (>= 2.3.1) [amd64 arm64 armel armhf i386 hppa loong64 mips mipsel mips64 mips64el x32 powerpc ppc64 ppc64el riscv64 s390x], libdw-dev (>= 0.158) <!stage1>, libpolkit-gobject-1-dev <!stage1>, libzstd-dev (>= 1.4.0), libtss2-dev [!i386] <!stage1>, libfido2-dev <!stage1>, libp11-kit-dev <!stage1>, libssl-dev <!stage1>, libqrencode-dev [!i386] <!stage1>, libxen-dev [amd64 arm64] <!stage1>, libpwquality-dev <!stage1>, libarchive-dev <!stage1>, linux-base <!nocheck>, acl <!nocheck>, python3:native, python3-jinja2:native, python3-lxml:native, python3-pefile:native <!stage1 !nocheck> <!stage1 !noinsttest>, python3-pyelftools [amd64 i386 arm64 armhf loong64 riscv64] <!stage1>, jq [amd64 arm64] <!stage1>, python3-pyparsing <!nocheck>, python3-evdev:native <!nocheck>, tzdata-legacy <!nocheck> | tzdata (<< 2025a-2) <!nocheck>, libcap2-bin <!nocheck>, iproute2 <!nocheck>, zstd <!nocheck>, gawk <!nocheck>, fdisk <!nocheck>
Package-List:
 libnss-myhostname deb admin optional arch=linux-any
 libnss-mymachines deb admin optional arch=linux-any
 libnss-resolve deb admin optional arch=linux-any
 libnss-systemd deb admin standard arch=linux-any
 libpam-systemd deb admin standard arch=linux-any
 libsystemd-dev deb libdevel optional arch=linux-any
 libsystemd-shared deb libs optional arch=linux-any
 libsystemd0 deb libs optional arch=linux-any
 libudev-dev deb libdevel optional arch=linux-any
 libudev1 deb libs optional arch=linux-any
 systemd deb admin important arch=linux-any
 systemd-boot deb admin optional arch=amd64,i386,arm64,armhf,loong64,riscv64 profile=!stage1
 systemd-boot-efi deb admin optional arch=amd64,i386,arm64,armhf,loong64,riscv64 profile=!stage1
 systemd-boot-tools deb admin optional arch=linux-any profile=!stage1
 systemd-container deb admin optional arch=linux-any profile=!stage1
 systemd-coredump deb admin optional arch=linux-any profile=!stage1
 systemd-cryptsetup deb admin important arch=linux-any profile=!stage1
 systemd-dev deb devel optional arch=all
 systemd-homed deb admin optional arch=linux-any profile=!stage1
 systemd-journal-remote deb admin optional arch=linux-any profile=!stage1
 systemd-oomd deb admin optional arch=linux-any
 systemd-repart deb admin optional arch=linux-any
 systemd-resolved deb admin important arch=linux-any
 systemd-standalone-shutdown deb admin optional arch=linux-any
 systemd-standalone-sysusers deb admin optional arch=linux-any
 systemd-standalone-tmpfiles deb admin optional arch=linux-any
 systemd-sysv deb admin important arch=linux-any
 systemd-tests deb admin optional arch=linux-any profile=!noinsttest
 systemd-timesyncd deb admin standard arch=linux-any
 systemd-ukify deb admin optional arch=all profile=!stage1,!pkg.systemd.noukify
 systemd-userdbd deb admin optional arch=linux-any profile=!stage1
 udev deb admin important arch=linux-any
Checksums-Sha1:
 1a2a1445a8b646416bedbe52cb7a16571329da88 17336729 systemd_259.5.orig.tar.gz
 e9d57f187531f8f1b0e6599a986e9b51ed366868 157820 systemd_259.5-ok1.5.debian.tar.xz
Checksums-Sha256:
 80ed55a8a69c4bd1fb12a36659303372b37baf9ee224ef4f032db4b748be0f76 17336729 systemd_259.5.orig.tar.gz
 90a6a16355b1d4c509474ed075bbc8e413b917b12f849ef0db3f3dae6282f0d7 157820 systemd_259.5-ok1.5.debian.tar.xz
Files:
 6ac65dfbd3c9dc14f3bf4086f83b8aae 17336729 systemd_259.5.orig.tar.gz
 5689cccd9fe927c53eb35f09e881a857 157820 systemd_259.5-ok1.5.debian.tar.xz
Original-Maintainer: Debian systemd Maintainers <pkg-systemd-maintainers@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqOPHkACgkQBxbwD4Qs
0nSO1AwAqaFDXtBT7ghMxP1jgTolJx9del/DwGsqGqz4PQ+FjDu5PqORhnUPaGiS
QG7Uktziqj+x1CCLgGDLRi7+F/HI4MHpht5dnSFgIeuyY1NJgLZZB8zNEupHaHwb
f4iA4ki8lMB5iJJpw1FD4/edIumbQdbnMniMzBxM5c+mTLBcAgl1aONT3JSVko7F
8aepBHwLg/eDDtVs53BPSXvZppS1jyOcNWU0AJGWvLW7P+FDpYVv9kSjJAYIMc03
7WoNDWhYDde5Gtxx1fM76grwm5xgvLVX59T81MDDH8h030lM5HM/z3282MzocYol
jQOI2ecdyjWadvLdLQu7Rq/3iBYUE0cvXdnsVLWk32jTq/2pUs/NdIdPsRu6djyv
dKTIk/WOUQmCVjU94IrGgT4VMdKJKHHVKzNRxGKYx9On3czgKVvYAL+JAQf2EdPw
0/0G5wh3romg8Rs6FD65ir4QLkVRrHsv4+Hh0YXA4PEeBfG3M+cZYrdnMtA5gZku
/5eTKxU9
=qeKx
-----END PGP SIGNATURE-----
