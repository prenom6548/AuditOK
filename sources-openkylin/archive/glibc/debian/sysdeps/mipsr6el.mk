# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (n32) alternative library
GLIBC_PASSES += mipsn32r6el
DEB_ARCH_MULTILIB_PACKAGES += libc6-mipsn32 libc6-dev-mipsn32
libc6-mipsn32r6el_shlib_dep = libc6-mipsn32 (>= $(shlib_dep_ver))
mipsn32r6el_configure_target = mipsisa64r6el-linux-gnuabin32
mipsn32r6el_CC = $(CC) -mabi=n32
mipsn32r6el_CXX = $(CXX) -mabi=n32
mipsn32r6el_rtlddir = /lib32
mipsn32r6el_slibdir = /lib32
mipsn32r6el_libdir = /usr/lib32

# build 64-bit alternative library
GLIBC_PASSES += mips64r6el
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips64 libc6-dev-mips64
libc6-mips64r6el_shlib_dep = libc6-mips64 (>= $(shlib_dep_ver))
mips64r6el_configure_target = mipsisa64r6el-linux-gnuabi64
mips64r6el_CC = $(CC) -mabi=64
mips64r6el_CXX = $(CXX) -mabi=64
mips64r6el_rtlddir = /lib64
mips64r6el_slibdir = /lib64
mips64r6el_libdir = /usr/lib64

define libc6-dev-mips64r6el_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mips64)

mkdir -p debian/libc6-dev-mips64/usr/include/mipsisa32r6el-linux-gnu/gnu
cp -a debian/tmp-mips64/usr/include/gnu/lib-names-n64_hard_2008.h \
	debian/tmp-mips64/usr/include/gnu/stubs-n64_hard_2008.h \
	debian/libc6-dev-mips64/usr/include/mipsisa32r6el-linux-gnu/gnu

endef

define libc6-dev-mipsn32r6el_extra_pkg_install

mkdir -p debian/libc6-dev-mipsn32/usr/include/mipsisa32r6el-linux-gnu/gnu
cp -a debian/tmp-mipsn32/usr/include/gnu/lib-names-n32_hard_2008.h \
	debian/tmp-mipsn32/usr/include/gnu/stubs-n32_hard_2008.h \
	debian/libc6-dev-mipsn32/usr/include/mipsisa32r6el-linux-gnu/gnu

endef

# Need to put a tri-arch aware version of ldd in the base package
define mipsn32r6el_extra_install
cp debian/tmp-mipsn32/usr/bin/ldd debian/tmp/usr/bin
endef

endif # multilib
