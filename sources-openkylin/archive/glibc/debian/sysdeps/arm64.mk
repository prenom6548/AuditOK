# configuration options for all flavours
extra_config_options = --enable-multi-arch --enable-memory-tagging
extra_cflags = -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer

# main library
libc_mvec = yes

CC = $(DEB_HOST_GNU_TYPE)-$(BASE_CC)$(DEB_GCC_VERSION) -mbranch-protection=standard
CXX = $(DEB_HOST_GNU_TYPE)-$(BASE_CXX)$(DEB_GCC_VERSION) -mbranch-protection=standard
