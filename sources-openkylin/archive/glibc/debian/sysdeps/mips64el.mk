# main library
libc_rtlddir = /lib64

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (n32) alternative library
GLIBC_PASSES += mipsn32el
DEB_ARCH_MULTILIB_PACKAGES += libc6-mipsn32 libc6-dev-mipsn32
libc6-mipsn32el_shlib_dep = libc6-mipsn32 (>= $(shlib_dep_ver))
mipsn32el_configure_target = mips64el-linux-gnuabin32
mipsn32el_CC = $(CC) -mabi=n32
mipsn32el_CXX = $(CXX) -mabi=n32
mipsn32el_rtlddir = /lib32
mipsn32el_slibdir = /lib32
mipsn32el_libdir = /usr/lib32

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

define libc6-dev-mipsn32el_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mipsn32)

mkdir -p debian/libc6-dev-mipsn32/usr/include/mips64el-linux-gnuabi64/gnu
cp -a debian/tmp-mipsn32/usr/include/gnu/lib-names-n32_hard.h \
	debian/tmp-mipsn32/usr/include/gnu/stubs-n32_hard.h \
	debian/libc6-dev-mipsn32/usr/include/mips64el-linux-gnuabi64/gnu

endef

define libc6-dev-mipsel_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mips64el-linux-gnuabi64/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard.h \
	debian/libc6-dev-mips32/usr/include/mips64el-linux-gnuabi64/gnu

endef

endif # multilib
