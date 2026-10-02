# main library
libc_rtlddir = /lib32

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit alternative library
GLIBC_PASSES += mips64el
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips64 libc6-dev-mips64
libc6-mips64el_shlib_dep = libc6-mips64 (>= $(shlib_dep_ver))
mips64el_configure_target = mips64el-linux-gnuabi64
mips64el_CC = $(CC) -mabi=64
mips64el_CXX = $(CXX) -mabi=64
mips64el_rtlddir = /lib64
mips64el_slibdir = /lib64
mips64el_libdir = /usr/lib64

# build 32-bit (o32) alternative library
GLIBC_PASSES += mipsel
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips32 libc6-dev-mips32
libc6-mipsel_shlib_dep = libc6-mips32 (>= $(shlib_dep_ver))
mipsel_configure_target = mipsel-linux-gnu
mipsel_CC = $(CC) -mabi=32
mipsel_CXX = $(CXX) -mabi=32
mipsel_rtlddir = /lib
mipsel_slibdir = /libo32
mipsel_libdir = /usr/libo32

define libc6-dev-mips64el_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mips64)

mkdir -p debian/libc6-dev-mips64/usr/include/mips64el-linux-gnuabin32/gnu
cp -a debian/tmp-mips64/usr/include/gnu/lib-names-n64_hard.h \
	debian/tmp-mips64/usr/include/gnu/stubs-n64_hard.h \
	debian/libc6-dev-mips64/usr/include/mips64el-linux-gnuabin32/gnu

endef

define libc6-dev-mipsel_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mips64el-linux-gnuabin32/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard.h \
	debian/libc6-dev-mips32/usr/include/mips64el-linux-gnuabin32/gnu

endef

endif # multilib
