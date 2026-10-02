NULL =

EXTRA_LIBS_DIR ?= /usr/lib/biometric-authentication/drivers/extra/wechat
LIBWECHAT_DIR ?= /usr/lib/biometric-authentication/drivers/extra/wechat
LOCALEDIR ?= /usr/share/locale

PKG_CONFIG_LIBS = libbiometric glib-2.0 libusb-1.0 sqlite3 opencv4 libcurl
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_CONFIG_LIBS))
PKG_LDFLAGS = $(shell pkg-config --libs $(PKG_CONFIG_LIBS))

COMM_CFLAGS = -Wall -g -Wl,--as-needed -Wl,-rpath=$(EXTRA_LIBS_DIR),--disable-new-dtags -fstack-protector-all -D_FORTIFY_SOURCE=2
COMM_LDFLAGS = $(PKG_LDFLAGS) -lpthread -z noexecstack -z relro -z now


DRV_CFLAGS = $(COMM_CFLAGS) -shared -fpic \
	$(PKG_CFLAGS) \
	-DLOCALEDIR=\"$(LOCALEDIR)\" \
	$(NULL)

TOOL_CFLAGS = $(COMM_CFLAGS) $(PKG_CFLAGS)
TOOL_LDFLAGS = $(COMM_LDFLAGS) -lstdc++  -lpthread

LIBWECHAT = libwechat.so

LIBWECHAT_SOURCE_FILE = wechat_realization.cpp wechat_internal.cpp cJSON.c ksb_http.c kywechat.c
LIBWECHAT_HEADER_FILE = wechat_realization.h wechat_internal.h cJSON.h ksb_http.h kywechat.h


COMMON_HEADER_FILE =

all: $(LIBWECHAT)
$(LIBWECHAT): $(LIBWECHAT_SOURCE_FILE) $(LIBWECHAT_HEADER_FILE) $(COMMON_HEADER_FILE)
	g++ $(DRV_CFLAGS) $(LIBWECHAT_SOURCE_FILE)  -o $(LIBWECHAT) $(COMM_LDFLAGS)

#$(DISCOVER_TOOL): $(DISCOVER_TOOL_SOURCE_FILE) $(COMMON_HEADER_FILE)
#	gcc $(TOOL_CFLAGS) $(DISCOVER_TOOL_SOURCE_FILE) -o $(DISCOVER_TOOL) $(TOOL_LDFLAGS)


.PHONY: clean
clean:
	rm -rf $(LIBWECHAT)

install: all
	install -p -D -m 0644 $(LIBWECHAT) $(DESTDIR)/$(LIBWECHAT_DIR)/$(LIBWECHAT)
#	install -p -D -m 0755 $(DISCOVER_TOOL) $(DESTDIR)/$(DISCOVER_TOOL_DIR)/$(DISCOVER_TOOL)


uninstall:
	rm -rf $(DESTDIR)/$(LIBWECHAT_DIR)/$(LIBWECHAT)
#	rm -rf $(DESTDIR)/$(LIBARATEK_DIR)/$(DISCOVER_TOOL)
