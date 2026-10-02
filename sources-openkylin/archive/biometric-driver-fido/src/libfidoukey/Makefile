NULL =
ARCH = $(shell dpkg-architecture -qDEB_HOST_ARCH)
EXTRA_LIBS_DIR ?= /usr/lib/biometric-authentication/drivers/extra/fidoukey
LIBFIDOUKEY_DIR ?= /usr/lib/biometric-authentication/drivers/extra/fidoukey
LOCALEDIR ?= /usr/share/locale

PKG_CONFIG_LIBS = libbiometric glib-2.0 libusb-1.0 sqlite3 openssl libfido2 openssl
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_CONFIG_LIBS))
PKG_LDFLAGS = $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

COMM_CFLAGS = -Wall -g -Wl,-rpath=$(EXTRA_LIBS_DIR),--disable-new-dtags
COMM_LDFLAGS = $(PKG_LDFLAGS) -I/usr/include/libbiometric -I../extra/ -L../extra/

DRV_CFLAGS = $(COMM_CFLAGS) -shared -fpic \
	$(PKG_CFLAGS) \
	-DLOCALEDIR=\"$(LOCALEDIR)\" \
	$(NULL)

TOOL_CFLAGS = $(COMM_CFLAGS) $(PKG_CFLAGS)
TOOL_LDFLAGS = $(COMM_LDFLAGS) -lstdc++ 

LIBFIDOUKEY = libfidoukey.so
#DISCOVER_TOOL = usb-seetaface

LIBFIDOUKEY_SOURCE_FILE = fidoukey_realization.c fidoukey_internal.c
LIBFIDOUKEY_HEADER_FILE = fidoukey_realization.h fidoukey_internal.h

#DISCOVER_TOOL_SOURCE_FILE = usb_seetaface.c

COMMON_HEADER_FILE = 

#all: $(LIBARATEK) $(DISCOVER_TOOL)
all: $(LIBFIDOUKEY)

$(LIBFIDOUKEY): $(LIBFIDOUKEY_SOURCE_FILE) $(LIBFIDOUKEY_HEADER_FILE) $(COMMON_HEADER_FILE)
	gcc $(DRV_CFLAGS) $(LIBFIDOUKEY_SOURCE_FILE) -o $(LIBFIDOUKEY) $(COMM_LDFLAGS) 

#$(DISCOVER_TOOL): $(DISCOVER_TOOL_SOURCE_FILE) $(COMMON_HEADER_FILE)
#	gcc $(TOOL_CFLAGS) $(DISCOVER_TOOL_SOURCE_FILE) -o $(DISCOVER_TOOL) $(TOOL_LDFLAGS)


.PHONY: clean
clean:
	rm -rf $(LIBFIDOUKEY)

install: all
	install -p -D -m 0644 $(LIBFIDOUKEY) $(DESTDIR)/$(LIBFIDOUKEY_DIR)/$(LIBFIDOUKEY)


uninstall:
	rm -rf $(DESTDIR)/$(LIBFIDOUKEY_DIR)/$(LIBFIDOUKEY)