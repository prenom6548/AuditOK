NAME = fastboot
SOURCES = bootimg_utils.cpp \
          engine.cpp \
          fastboot.cpp \
          fs.cpp\
          protocol.cpp \
          socket.cpp \
          tcp.cpp \
          udp.cpp \
          util.cpp \
          usb_linux.cpp \

SOURCES := $(foreach source, $(SOURCES), fastboot/$(source))
CXXFLAGS += -fpermissive
CPPFLAGS += -DUSE_F2FS -DFASTBOOT_VERSION='"$(DEB_VERSION)"' \
            -Iinclude \
            -Imkbootimg \
            -Iadb \
            -Idemangle/include \
            -Ibase/include \
            -I/usr/include/android \
            -I/usr/include/android/f2fs_utils \
            -I/usr/include/android/openssl \
            -Ilibsparse/include \
            -Ilibziparchive/include
LDFLAGS += -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -Wl,-rpath-link=. \
           -L. -lziparchive -lsparse -lbase -lcutils -ladb -lutils \
           -L/usr/lib/$(DEB_HOST_MULTIARCH)/android -lf2fs_utils

build: $(SOURCES)
	$(CXX) $^ -o fastboot/$(NAME) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

clean:
	$(RM) fastboot/$(NAME)