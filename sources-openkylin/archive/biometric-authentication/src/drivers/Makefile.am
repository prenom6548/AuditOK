NULL =

SUBDIRS = community-multidevice

if NEED_UD650
SUBDIRS += extra
endif

if HAS_BIO_DRIVER_DIR
drv_dir = $(BIO_DRIVER_DIR)
else
drv_dir = $(datadir)/biometric-authentication/driver
endif

libdrvdir = $(drv_dir)

if NEED_UD650
libdrv_LTLIBRARIES = ud650.la
endif

DOMAIN_NAME = -DLIBBIOMETRIC_DOMAIN_NAME=\""$(PACKAGE)"\"
LOCALEDIR_CFS = -DLOCALEDIR=\""$(localedir)"\"

COMM_CFS = -Wall -g -shared -fpic -fstack-protector-all \
	-I$(top_srcdir)/src/lib $(LOCALEDIR_CFS) $(DOMAIN_NAME)
	$(NULL)

COMM_LDFS = ../lib/libbiometric.la -module -avoid-version -z noexecstack -z relro -z now

if HAS_BIO_DRIVER_DIR
COMM_CFS += -DBIO_DRIVER_DIR=$(drv_dir)
endif

if NEED_UD650
ud650_la_SOURCES = ud650.h ud650.c driver_ids.h

ud650_la_INCLUDE = biometric_common.h biometric_stroge.h

ud650_la_CFLAGS = $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(LIBUSB_1_CFLAGS) $(SQLITE3_CFLAGS) \
	-fPIC \
	$(NULL)

ud650_la_LDFLAGS = $(COMM_LDFS) \
	$(GLIB_2_LIBS) $(LIBUSB_1_LIBS) $(SQLITE3_LIBS) \
	$(NULL)
endif

