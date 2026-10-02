NAME := libadb

LIBADB_SRC_FILES := \
    adb.cpp \
    adb_io.cpp \
    adb_listeners.cpp \
    adb_trace.cpp \
    adb_utils.cpp \
    fdevent.cpp \
    sockets.cpp \
    socket_spec.cpp \
    sysdeps/errno.cpp \
    transport.cpp \
    transport_local.cpp \
    transport_usb.cpp \

LIBADB_linux_SRC_FILES := \
    sysdeps_unix.cpp \
    sysdeps/posix/network.cpp \
    client/usb_dispatch.cpp \
    client/usb_libusb.cpp \
    client/usb_linux.cpp \

LIBDIAGNOSE_USB_SRC_FILES = diagnose_usb.cpp

LOCAL_SRC_FILES := \
    $(LIBADB_SRC_FILES) \
    $(LIBADB_linux_SRC_FILES) \
    $(LIBDIAGNOSE_USB_SRC_FILES) \
    adb_auth_host.cpp \
    services.cpp \
    transport_mdns_unsupported.cpp

SOURCES := $(foreach source, $(LOCAL_SRC_FILES), adb/$(source))
CXXFLAGS += -fpermissive -std=c++14
CPPFLAGS += -I/usr/include/android -Iadb -Iinclude -Ibase/include \
	    -Ilibcrypto_utils/include -DADB_HOST=1 -DADB_VERSION='"$(DEB_VERSION)"'
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
           -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -L/usr/lib/$(DEB_HOST_MULTIARCH)/android -lcrypto \
           -lpthread -latomic -L. -lbase -lcutils -lcrypto_utils -lusb-1.0

build: $(SOURCES)
	$(CXX) $^ -o $(NAME).so.0 $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*
