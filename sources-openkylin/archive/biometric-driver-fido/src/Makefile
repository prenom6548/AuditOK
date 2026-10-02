NULL =

EXTRA_LIBS_DIR ?= /usr/lib/biometric-authentication/drivers/extra/fidoukey
FIDOUKEY_MODULE_DIR ?= /usr/lib/biometric-authentication/drivers
DISCOVER_TOOL_DIR ?= /usr/lib/biometric-authentication/discover-tools
LOCALEDIR ?= /usr/share/locale

PKG_CONFIG_LIBS = libbiometric glib-2.0 libusb-1.0 sqlite3
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_CONFIG_LIBS))
PKG_LDFLAGS = $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

COMM_CFLAGS = -Wall -g -Wl,-rpath=$(EXTRA_LIBS_DIR),--disable-new-dtags
COMM_LDFLAGS = $(PKG_LDFLAGS) -L./libfidoukey -lfidoukey

DRV_CFLAGS = $(COMM_CFLAGS) -shared -fpic \
	$(PKG_CFLAGS) \
	-DLOCALEDIR=\"$(LOCALEDIR)\" \
	$(NULL)

TOOL_CFLAGS = $(COMM_CFLAGS) $(PKG_CFLAGS)
TOOL_LDFLAGS = $(COMM_LDFLAGS)  -lstdc++

FIDOUKEY_MODULE = bio_drv_fidoukey.so
DISCOVER_TOOL = usb-bio_fidoukey

FIDOUKEY_MODULE_SOURCE_FILE = fidoukey_detect.c

DISCOVER_TOOL_SOURCE_FILE = usb_fidoukey_detect.c

COMMON_HEADER_FILE = 

#all: extra libFIDOUKEY $(FIDOUKEY_MODULE) $(DISCOVER_TOOL) translation
all: libfidoukey $(FIDOUKEY_MODULE) translation

$(FIDOUKEY_MODULE): $(FIDOUKEY_MODULE_SOURCE_FILE) $(FIDOUKEY_MODULE_HEADER_FILE) $(COMMON_HEADER_FILE)
	gcc $(DRV_CFLAGS) $(FIDOUKEY_MODULE_SOURCE_FILE) -o $(FIDOUKEY_MODULE) $(COMM_LDFLAGS)

#$(DISCOVER_TOOL): $(DISCOVER_TOOL_SOURCE_FILE) $(COMMON_HEADER_FILE)
#	gcc $(TOOL_CFLAGS) $(DISCOVER_TOOL_SOURCE_FILE) -o $(DISCOVER_TOOL) $(TOOL_LDFLAGS)

#.PHONY: extra
#extra:
#	make -C extra

.PHONY: libfidoukey
libfidoukey:
	make -C libfidoukey

translation:
	make -C po/ all

.PHONY: clean
clean: libfidoukey-clean
	rm -rf $(FIDOUKEY_MODULE) $(DISCOVER_TOOL)
	make clean -C po/

#.PHONY: extra-clean
#extra-clean:
#	make clean -C extra

.PHONY: libfidoukey-clean
libfidoukey-clean:
	make clean -C libfidoukey

install: all
	install -p -D -m 0644 $(FIDOUKEY_MODULE) $(DESTDIR)/$(FIDOUKEY_MODULE_DIR)/$(FIDOUKEY_MODULE)
	#install -p -D -m 0755 $(DISCOVER_TOOL) $(DESTDIR)/$(DISCOVER_TOOL_DIR)/$(DISCOVER_TOOL)
#	make install -C extra/
	make install -C libfidoukey/
	make install -C po/

uninstall:
	rm -rf $(DESTDIR)/$(FIDOUKEY_MODULE_DIR)/$(FIDOUKEY_MODULE)
	#rm -rf $(DESTDIR)/$(FIDOUKEY_MODULE_DIR)/$(DISCOVER_TOOL)
#	make uninstall -C extra/
	make uninstall -C libfidoukey/
	make uninstall -C po/
