# main library
libc_rtlddir = /lib64

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (n32) alternative library
GLIBC_PASSES += mipsn32r6
DEB_ARCH_MULTILIB_PACKAGES += libc6-mipsn32 libc6-dev-mipsn32
libc6-mipsn32r6_shlib_dep = libc6-mipsn32 (>= $(shlib_dep_ver))
mipsn32r6_configure_target = mipsisa64r6-linux-gnuabin32
mipsn32r6_CC = $(CC) -mabi=n32
mipsn32r6_CXX = $(CXX) -mabi=n32
mipsn32r6_rtlddir = /lib32
mipsn32r6_slibdir = /lib32
mipsn32r6_libdir = /usr/lib32

# build 32-bit (o32) alternative library
GLIBC_PASSES += mipsr6
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips32 libc6-dev-mips32
libc6-mipsr6_shlib_dep = libc6-mips32 (>= $(shlib_dep_ver))
mipsr6_configure_target = mipsisa32r6-linux-gnu
mipsr6_CC = $(CC) -mabi=32
mipsr6_CXX = $(CXX) -mabi=32
mipsr6_rtlddir = /lib
mipsr6_slibdir = /libo32
mipsr6_libdir = /usr/libo32

define libc6-dev-mipsn32r6_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mipsn32)

mkdir -p debian/libc6-dev-mipsn32/usr/include/mipsisa64r6-linux-gnuabi64/gnu
cp -a debian/tmp-mipsn32/usr/include/gnu/lib-names-n32_hard_2008.h \
	debian/tmp-mipsn32/usr/include/gnu/stubs-n32_hard_2008.h \
	debian/libc6-dev-mipsn32/usr/include/mipsisa64r6-linux-gnuabi64/gnu

endef

define libc6-dev-mipsr6_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mipsisa64r6-linux-gnuabi64/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard_2008.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard_2008.h \
	debian/libc6-dev-mips32/usr/include/mipsisa64r6-linux-gnuabi64/gnu

endef

endif # multilib
