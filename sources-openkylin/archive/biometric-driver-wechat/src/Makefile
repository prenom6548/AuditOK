NULL =

EXTRA_LIBS_DIR ?= /usr/lib/biometric-authentication/drivers/extra/wechat
WECHAT_MODULE_DIR ?= /usr/lib/biometric-authentication/drivers
DISCOVER_TOOL_DIR ?= /usr/lib/biometric-authentication/discover-tools
LOCALEDIR ?= /usr/share/locale

PKG_CONFIG_LIBS = libbiometric glib-2.0 libusb-1.0 sqlite3
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_CONFIG_LIBS))
PKG_LDFLAGS = $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

COMM_CFLAGS = -Wall -g -Wl,--as-needed -Wl,-rpath=$(EXTRA_LIBS_DIR),--disable-new-dtags -fstack-protector-all -D_FORTIFY_SOURCE=2
COMM_LDFLAGS = $(PKG_LDFLAGS) -L./libwechat -lwechat -z noexecstack -z relro -z now

DRV_CFLAGS = $(COMM_CFLAGS) -shared -fpic \
	$(PKG_CFLAGS) \
	-DLOCALEDIR=\"$(LOCALEDIR)\" \
	$(NULL)

TOOL_CFLAGS = $(COMM_CFLAGS) $(PKG_CFLAGS)
TOOL_LDFLAGS = $(COMM_LDFLAGS) -lstdc++

WECHAT_MODULE = wechat_driver.so
#DISCOVER_TOOL = usb-eyecool_facedetect

WECHAT_MODULE_SOURCE_FILE = wechat_driver.c
#DISCOVER_TOOL_SOURCE_FILE = usb_eyecool_facedetect.c

COMMON_HEADER_FILE =

all: libwechat $(WECHAT_MODULE) translation

$(WECHAT_MODULE): $(WECHAT_MODULE_SOURCE_FILE) $(WECHAT_MODULE_HEADER_FILE) $(COMMON_HEADER_FILE)
	gcc $(DRV_CFLAGS) $(WECHAT_MODULE_SOURCE_FILE) -o $(WECHAT_MODULE) $(COMM_LDFLAGS)

#$(DISCOVER_TOOL): $(DISCOVER_TOOL_SOURCE_FILE) $(COMMON_HEADER_FILE)
#	gcc $(TOOL_CFLAGS) $(DISCOVER_TOOL_SOURCE_FILE) -o $(DISCOVER_TOOL) $(TOOL_LDFLAGS)

.PHONY: extra
extra:
	make -C extra/out

.PHONY: libwechat
libwechat:
	make -C libwechat

translation:
	make -C po/ all

.PHONY: clean
clean: libwechat-clean
	rm -rf $(WECHAT_MODULE) $(DISCOVER_TOOL)
#	make clean -C po/

.PHONY: libwechat-clean
libwechat-clean:
	make clean -C libwechat

install: all
	install -p -D -m 0644 $(WECHAT_MODULE) $(DESTDIR)/$(WECHAT_MODULE_DIR)/$(WECHAT_MODULE)
#	install -p -D -m 0755 $(DISCOVER_TOOL) $(DESTDIR)/$(DISCOVER_TOOL_DIR)/$(DISCOVER_TOOL)
#	make install -C extra/out/
	make install -C libwechat/
	make install -C po/

uninstall:
	rm -rf $(DESTDIR)/$(WECHAT_MODULE_DIR)/$(WECHAT_MODULE)
	#rm -rf $(DESTDIR)/$(EYECOOL_FACE_MODULE_DIR)/$(DISCOVER_TOOL)
	make uninstall -C libwechat/