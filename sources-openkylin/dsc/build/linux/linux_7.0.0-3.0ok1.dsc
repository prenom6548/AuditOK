-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: linux
Binary: linux-headers-7.0.0-3, linux-tools-7.0.0-3, linux-cloud-tools-7.0.0-3, linux-libc-dev, linux-tools-common, linux-cloud-tools-common, linux-tools-host, linux-source-7.0.0, linux-doc, linux-bpf-dev, bpftool, linux-perf, linux-image-unsigned-7.0.0-3-generic, linux-image-unsigned-7.0.0-3-generic-dbgsym, linux-image-7.0.0-3-generic, linux-image-7.0.0-3-generic-dbgsym, linux-modules-7.0.0-3-generic, linux-headers-7.0.0-3-generic, linux-lib-rust-7.0.0-3-generic, linux-tools-7.0.0-3-generic, linux-cloud-tools-7.0.0-3-generic, linux-buildinfo-7.0.0-3-generic, linux-image-unsigned-7.0.0-3-generic-64k, linux-image-unsigned-7.0.0-3-generic-64k-dbgsym, linux-modules-7.0.0-3-generic-64k, linux-headers-7.0.0-3-generic-64k, linux-lib-rust-7.0.0-3-generic-64k, linux-tools-7.0.0-3-generic-64k, linux-cloud-tools-7.0.0-3-generic-64k, linux-buildinfo-7.0.0-3-generic-64k
Architecture: all amd64 armhf arm64 loong64 i386 ppc64el riscv64 s390x
Version: 7.0.0-3.0ok1
Maintainer: Kylin Software Kernel Team <kernel-bot@kylinos.cn>
Standards-Version: 3.9.4.0
Vcs-Git: https://gitcode.com/openkylin/linux
Testsuite: autopkgtest
Testsuite-Triggers: @builddeps@, build-essential, fakeroot, fuse-overlayfs, gcc-multilib, gdb, git, python3, snapd
Build-Depends: gcc:native, gcc-aarch64-linux-gnu [arm64] <cross>, gcc-arm-linux-gnueabihf [armhf] <cross>, gcc-powerpc64le-linux-gnu [ppc64el] <cross>, gcc-riscv64-linux-gnu [riscv64] <cross>, gcc-s390x-linux-gnu [s390x] <cross>, gcc-x86-64-linux-gnu [amd64] <cross>, gcc-loongarch64-linux-gnu [loong64] <cross>, autoconf <!stage1>, automake <!stage1>, bc <!stage1>, bindgen:native [amd64 arm64 armhf riscv64 loong64], bison <!stage1>, clang-21:native [amd64 arm64 armhf riscv64 loong64] | clang-22:native [amd64 arm64 armhf riscv64 loong64], cpio, curl <!stage1>, debhelper-compat (= 10), default-jdk-headless:native <!stage1>, dkms <!stage1>, flex <!stage1>, gawk <!stage1>, java-common <!stage1>, kmod <!stage1>, libaudit-dev <!stage1>, libbpf-dev <!stage1>, libcap-dev <!stage1>, libdebuginfod-dev [amd64 arm64 armhf riscv64 loong64] <!stage1>, libdw-dev <!stage1>, libelf-dev <!stage1>, libiberty-dev <!stage1>, liblzma-dev <!stage1>, libnewt-dev <!stage1>, libnl-3-dev, libnl-genl-3-dev, libnuma-dev [amd64 arm64 loong64] <!stage1>, libpci-dev <!stage1>, libssl-dev <!stage1>, libstdc++-dev, libtool <!stage1>, libtraceevent-dev [amd64 arm64 armhf riscv64 loong64] <!stage1>, libtracefs-dev [amd64 arm64 armhf riscv64 loong64] <!stage1>, libudev-dev <!stage1>, libunwind8-dev [amd64 arm64 armhf ppc64el] <!stage1>, llvm-21-dev | llvm-22-dev, makedumpfile:native [amd64] <!stage1>, openssl <!stage1>, pahole (>= 1.29) [amd64 arm64 armhf riscv64 loong64] | dwarves (>= 1.21) [amd64 arm64 armhf riscv64 loong64] <!stage1>, pkg-config <!stage1>, python3:native <!stage1>, python3-dev:native <!stage1>, libpython3-dev <!stage1> <cross>, python3-setuptools, rsync [!i386] <!stage1>, rust-src:native [amd64 arm64 armhf ppc64el riscv64 s390x], rustc:native (>= 1.82) [amd64 arm64 armhf riscv64 loong64], rustfmt:native [amd64 arm64 armhf ppc64el riscv64 s390x], uuid-dev <!stage1>, zstd <!stage1>, bpftool:native [amd64 arm64 armhf riscv64 loong64] <!stage1 cross>
Build-Depends-Indep: asciidoc <!stage1>, bzip2 <!stage1>, python3-docutils <!stage1>, sharutils <!stage1>, xmlto <!stage1>
Package-List:
 bpftool deb devel optional arch=amd64,armhf,arm64,riscv64,loong64
 linux-bpf-dev deb devel optional arch=amd64,armhf,arm64,i386,riscv64,loong64
 linux-buildinfo-7.0.0-3-generic deb kernel optional arch=amd64,armhf,arm64,ppc64el,riscv64,s390x,loong64 profile=!stage1
 linux-buildinfo-7.0.0-3-generic-64k deb kernel optional arch=arm64 profile=!stage1
 linux-cloud-tools-7.0.0-3 deb devel optional arch=amd64,armhf profile=!stage1
 linux-cloud-tools-7.0.0-3-generic deb devel optional arch=amd64,armhf,arm64,ppc64el,riscv64,s390x,loong64 profile=!stage1
 linux-cloud-tools-7.0.0-3-generic-64k deb devel optional arch=arm64 profile=!stage1
 linux-cloud-tools-common deb kernel optional arch=all profile=!stage1
 linux-doc deb doc optional arch=all profile=!stage1
 linux-headers-7.0.0-3 deb devel optional arch=all profile=!stage1
 linux-headers-7.0.0-3-generic deb devel optional arch=amd64,armhf,arm64,ppc64el,riscv64,s390x,loong64 profile=!stage1
 linux-headers-7.0.0-3-generic-64k deb devel optional arch=arm64 profile=!stage1
 linux-image-7.0.0-3-generic deb kernel optional arch=armhf,ppc64el,riscv64,s390x profile=!stage1
 linux-image-7.0.0-3-generic-dbgsym deb devel optional arch=armhf,ppc64el,riscv64,s390x profile=!stage1
 linux-image-unsigned-7.0.0-3-generic deb kernel optional arch=amd64,arm64,loong64 profile=!stage1
 linux-image-unsigned-7.0.0-3-generic-64k deb kernel optional arch=arm64 profile=!stage1
 linux-image-unsigned-7.0.0-3-generic-64k-dbgsym deb devel optional arch=arm64 profile=!stage1
 linux-image-unsigned-7.0.0-3-generic-dbgsym deb devel optional arch=amd64,arm64,loong64 profile=!stage1
 linux-lib-rust-7.0.0-3-generic deb devel optional arch=amd64 profile=!stage1
 linux-lib-rust-7.0.0-3-generic-64k deb devel optional arch=amd64 profile=!stage1
 linux-libc-dev deb devel optional arch=amd64,armhf,arm64,i386,loong64,ppc64el,riscv64,s390x
 linux-modules-7.0.0-3-generic deb kernel optional arch=amd64,armhf,arm64,ppc64el,riscv64,s390x,loong64 profile=!stage1
 linux-modules-7.0.0-3-generic-64k deb kernel optional arch=arm64 profile=!stage1
 linux-perf deb devel optional arch=amd64,armhf,arm64,riscv64,loong64
 linux-source-7.0.0 deb devel optional arch=all profile=!stage1
 linux-tools-7.0.0-3 deb devel optional arch=amd64,armhf,arm64,loong64 profile=!stage1
 linux-tools-7.0.0-3-generic deb devel optional arch=amd64,armhf,arm64,ppc64el,riscv64,s390x,loong64 profile=!stage1
 linux-tools-7.0.0-3-generic-64k deb devel optional arch=arm64 profile=!stage1
 linux-tools-common deb kernel optional arch=all profile=!stage1
 linux-tools-host deb kernel optional arch=all profile=!stage1
Checksums-Sha1:
 dd791a0238605d9b2aa8be71e2ae1f9cbbf53add 260153025 linux_7.0.0.orig.tar.gz
 54519141b8df9ef819a01804893ff06ecbc22a66 1268944 linux_7.0.0-3.0ok1.debian.tar.xz
Checksums-Sha256:
 d937cff0b080a3502d7143b6d27b941512350fc1c3257c6d0ce484ed23962f37 260153025 linux_7.0.0.orig.tar.gz
 cc83cf6f7b49a438ebd20de344e0c5d6d7e8b8143c9f37fd9a213758dc6db3a7 1268944 linux_7.0.0-3.0ok1.debian.tar.xz
Files:
 f8e6e722f4fbc88a53a12a9b9af08c20 260153025 linux_7.0.0.orig.tar.gz
 2af1ca6f58fd772b0b39905519ab80ea 1268944 linux_7.0.0-3.0ok1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGzBAEBCgAdFiEEZPu3n16uZNeS6Uz+BxbwD4Qs0nQFAmqqaSkACgkQBxbwD4Qs
0nTWCgv+LMsqGLf6bAor+B3tLs7GY2scEo/HOFnE36E7oJFZAWl1VXbJd1NJkdau
8ww1f0FAhiGhsfrLLEFUUK7Agbhlz6DRlclCx1rdbCfl24BvyC6H05Oz/D1uMdwl
ijuNZkpNoai/BUeLVYQURKvhW6g6Sw6k+WG4AE8hMhJjsWAzIL/++jJe8rbgGdyU
UqrqJjY9Ixeyf3YrcegrcVeTffg5AiVjGvPLhugm5gw+JatA8s9OSqbk/879HafK
ymM2G2/AkQSWJ8ATMnwinr7jARhsGorp+zfcHmiiwiWHYq0y5CMOTZYCqiKo2ask
cXsOng3MR4WAkPEuAlcukVcMtjniaD0k9JUtF12rVnbCOq01lc58qQvGS67l6WhV
F+pGwydLN55sXWO5NgpF8g020GIGvwWOx+LpgC8ADdGaRYV4wrkj4E+bvF7bISFZ
Hsycody04NeBUPrEBnZHbGxVxi7+VevpU49KZG09QCAkkiLMWRT/ACsslXeuolLx
5c4vq9Ga
=KAlv
-----END PGP SIGNATURE-----
