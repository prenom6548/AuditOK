# main library
libc_rtlddir = /lib32

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

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

# build 32-bit (o32) alternative library
GLIBC_PASSES += mipsr6el
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips32 libc6-dev-mips32
libc6-mipsr6el_shlib_dep = libc6-mips32 (>= $(shlib_dep_ver))
mipsr6el_configure_target = mipsisa32r6el-linux-gnu
mipsr6el_CC = $(CC) -mabi=32
mipsr6el_CXX = $(CXX) -mabi=32
mipsr6el_rtlddir = /lib
mipsr6el_slibdir = /libo32
mipsr6el_libdir = /usr/libo32

define libc6-dev-mips64r6el_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mips64)

mkdir -p debian/libc6-dev-mips64/usr/include/mipsisa64r6el-linux-gnuabin32/gnu
cp -a debian/tmp-mips64/usr/include/gnu/lib-names-n64_hard_2008.h \
	debian/tmp-mips64/usr/include/gnu/stubs-n64_hard_2008.h \
	debian/libc6-dev-mips64/usr/include/mipsisa64r6el-linux-gnuabin32/gnu

endef

define libc6-dev-mipsr6el_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mipsisa64r6el-linux-gnuabin32/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard_2008.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard_2008.h \
	debian/libc6-dev-mips32/usr/include/mipsisa64r6el-linux-gnuabin32/gnu

endef

endif # multilib
