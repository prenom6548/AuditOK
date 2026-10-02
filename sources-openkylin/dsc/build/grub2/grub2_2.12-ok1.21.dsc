-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: grub2
Binary: grub2, grub-linuxbios, grub-efi, grub-common, grub2-common, grub-emu, grub-emu-dbg, grub-pc-bin, grub-pc-dbg, grub-pc, grub-rescue-pc, grub-coreboot-bin, grub-coreboot-dbg, grub-coreboot, grub-efi-ia32-bin, grub-efi-ia32-dbg, grub-efi-ia32, grub-efi-ia32-signed-template, grub-efi-amd64-bin, grub-efi-amd64-dbg, grub-efi-amd64, grub-efi-amd64-signed-template, grub-efi-ia64-bin, grub-efi-ia64-dbg, grub-efi-ia64, grub-efi-arm-bin, grub-efi-arm-dbg, grub-efi-arm, grub-efi-arm64-bin, grub-efi-arm64-dbg, grub-efi-arm64, grub-efi-arm64-signed-template, grub-efi-riscv64-bin, grub-efi-riscv64-dbg, grub-efi-riscv64, grub-efi-loong64-bin, grub-efi-loong64-dbg, grub-efi-loong64, grub-ieee1275-bin, grub-ieee1275-dbg, grub-ieee1275, grub-firmware-qemu, grub-uboot-bin, grub-uboot-dbg, grub-uboot, grub-xen-bin, grub-xen-dbg, grub-xen, grub-xen-host, grub-yeeloong-bin, grub-yeeloong-dbg, grub-yeeloong, grub-theme-starfield, grub-mount-udeb
Architecture: any
Version: 2.12-ok1.21
Maintainer: OpenKylin Developers <packaging@lists.openkylin.top>
Homepage: https://www.gnu.org/software/grub/
Standards-Version: 3.9.6
Vcs-Browser: https://gitee.com/openkylin/grub2
Vcs-Git: https://gitee.com/openkylin/grub2.git
Build-Depends: debhelper-compat (= 13), patchutils, python3, flex, bison, gawk, po-debconf, help2man, texinfo, gcc-12, gcc-12-multilib [i386 kopensolaris-i386 any-amd64 any-ppc64 any-sparc], xfonts-unifont, libfreetype6-dev, gettext, libdevmapper-dev [linux-any], libgeom-dev (>= 8.2+ds1-1~) [kfreebsd-any] | libgeom-dev (<< 8.2) [kfreebsd-any], libsdl2-dev [!hurd-any], xorriso, qemu-system [i386 kfreebsd-i386 kopensolaris-i386 any-amd64], cpio [i386 kopensolaris-i386 amd64 x32], parted [!hurd-any], libfuse3-dev [linux-any kfreebsd-any], fonts-dejavu-core, liblzma-dev, liblzo2-dev, lzop, dosfstools [any-i386 any-amd64 any-arm64], squashfs-tools [any-i386 any-amd64 any-arm64 any-loong64], wamerican, libparted-dev [any-powerpc any-ppc64 any-ppc64el], pkg-config, bash-completion, libefiboot-dev [i386 amd64 ia64 x32 armel armhf arm64 riscv64 loong64], libefivar-dev [i386 amd64 ia64 x32 armel armhf arm64 riscv64 loong64]
Build-Conflicts: autoconf2.13, libnvpair-dev, libzfs-dev
Package-List:
 grub-common deb admin optional arch=any
 grub-coreboot deb admin optional arch=any-i386,any-amd64
 grub-coreboot-bin deb admin optional arch=any-i386,any-amd64
 grub-coreboot-dbg deb debug optional arch=any-i386,any-amd64
 grub-efi deb admin optional arch=any-i386,any-amd64,any-arm64,any-ia64,any-arm,any-riscv64,any-loong64
 grub-efi-amd64 deb admin optional arch=kopensolaris-i386,any-amd64
 grub-efi-amd64-bin deb admin optional arch=kopensolaris-i386,any-amd64
 grub-efi-amd64-dbg deb debug optional arch=kopensolaris-i386,any-amd64
 grub-efi-amd64-signed-template deb admin optional arch=amd64
 grub-efi-arm deb admin optional arch=any-arm
 grub-efi-arm-bin deb admin optional arch=any-arm
 grub-efi-arm-dbg deb debug optional arch=any-arm
 grub-efi-arm64 deb admin optional arch=any-arm64
 grub-efi-arm64-bin deb admin optional arch=any-arm64
 grub-efi-arm64-dbg deb debug optional arch=any-arm64
 grub-efi-arm64-signed-template deb admin optional arch=arm64
 grub-efi-ia32 deb admin optional arch=any-i386,any-amd64
 grub-efi-ia32-bin deb admin optional arch=any-i386,any-amd64
 grub-efi-ia32-dbg deb debug optional arch=any-i386,any-amd64
 grub-efi-ia32-signed-template deb admin optional arch=i386
 grub-efi-ia64 deb admin optional arch=any-ia64
 grub-efi-ia64-bin deb admin optional arch=any-ia64
 grub-efi-ia64-dbg deb debug optional arch=any-ia64
 grub-efi-loong64 deb admin optional arch=any-loong64
 grub-efi-loong64-bin deb admin optional arch=any-loong64
 grub-efi-loong64-dbg deb debug optional arch=any-loong64
 grub-efi-riscv64 deb admin optional arch=any-riscv64
 grub-efi-riscv64-bin deb admin optional arch=any-riscv64
 grub-efi-riscv64-dbg deb debug optional arch=any-riscv64
 grub-emu deb admin optional arch=any-i386,any-amd64,any-powerpc
 grub-emu-dbg deb debug optional arch=any-i386,any-amd64,any-powerpc
 grub-firmware-qemu deb admin optional arch=any-i386,any-amd64
 grub-ieee1275 deb admin optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64
 grub-ieee1275-bin deb admin optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64
 grub-ieee1275-dbg deb debug optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64
 grub-linuxbios deb oldlibs optional arch=any-i386,any-amd64
 grub-mount-udeb udeb debian-installer optional arch=linux-any,kfreebsd-any
 grub-pc deb admin optional arch=any-i386,any-amd64
 grub-pc-bin deb admin optional arch=any-i386,any-amd64
 grub-pc-dbg deb debug optional arch=any-i386,any-amd64
 grub-rescue-pc deb admin optional arch=any-i386,any-amd64
 grub-theme-starfield deb admin optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64,any-mipsel,any-ia64,any-arm,any-arm64,any-riscv64,any-loong64
 grub-uboot deb admin optional arch=any-arm
 grub-uboot-bin deb admin optional arch=any-arm
 grub-uboot-dbg deb debug optional arch=any-arm
 grub-xen deb admin optional arch=i386,amd64
 grub-xen-bin deb admin optional arch=i386,amd64
 grub-xen-dbg deb debug optional arch=i386,amd64
 grub-xen-host deb admin optional arch=i386,amd64
 grub-yeeloong deb admin optional arch=any-mipsel
 grub-yeeloong-bin deb admin optional arch=any-mipsel
 grub-yeeloong-dbg deb debug optional arch=any-mipsel
 grub2 deb oldlibs optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64
 grub2-common deb admin optional arch=any-i386,any-amd64,any-powerpc,any-ppc64,any-ppc64el,any-sparc,any-sparc64,any-mipsel,any-ia64,any-arm,any-arm64,any-riscv64,any-loong64
Checksums-Sha1:
 4dae8f40e6405a56f8ed8f5a82250c7b7b97ce70 6589460 grub2_2.12.orig.tar.xz
 0bebca7f653985b906850694248eddefb8ed44de 1132508 grub2_2.12-ok1.21.debian.tar.xz
Checksums-Sha256:
 7a60c08b0ff1bac630cae6293b73871a541610a7fb1a7337aeb5e96f359cd650 6589460 grub2_2.12.orig.tar.xz
 54d4b9ea0dc8823b83703792b98b82ac588ddc694da639ae748e8fa9941750a1 1132508 grub2_2.12-ok1.21.debian.tar.xz
Files:
 b02415b4dcfd3d31482b33d3f6b79114 6589460 grub2_2.12.orig.tar.xz
 e68bf5f3dbbdcd99e3a66ee7aa32e528 1132508 grub2_2.12-ok1.21.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmncpRoACgkQBxbwD4Qs
0nQMQgv7BprmIrY1C4We5YBcBXeI3/ti9w1BxRbOQ3LP+FChNyugR/I7iic2LBXy
tUQmo+yU2RYurwWCS3Zs8v2Ocl7XrOiKzavOJ6PsQhoH9fedlSUCzSTNG/MzNDNr
vb3qOEnNATRone9o4+1an64rqdSbukwlTQz/9U6XZa04Cd7kqJE8E7kbFQaUCv7K
vByDTHQhMRMVyazrbQ1oqEv2AiFKq+FvoYIbknZMqMHqqLpRNhA9xlM8fKKAWtc4
gRBvQupa0JEORjvmg3vDfL2BIsfaNgMobkUIPXUjcx1Bto4Y76AwbcgfmtDgRpTD
eVOPCYMImfoDvnDDJd1d+CKDh5p6imW2O/k63kxrS2x1Zv0kcmhaYnd65OPrZRbX
fRlTi0G4Ttgarw25XIXEttt+TzzKDh7QWtSZFfLLwjaJ4CPj/QmW7HKXcr54qeiO
tdkjX66wbZd0GjYc0MGWTiEmg1Kr/Mn99Jd1a+sAI61SpSrkfNPbEE5sp55ygKrZ
QijcQYMU
=I1Aq
-----END PGP SIGNATURE-----
