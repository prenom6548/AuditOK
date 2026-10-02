if NEED_UD650

EXTRA_DIST = ud650-extra.so.x86_64 ud650-extra.so.aarch64

if HAS_BIO_DRIVER_DIR
ed_dir = $(BIO_DRIVER_DIR)/extra/
else
ed_dir = $(datadir)/biometric-authentication/drivers/extra/
endif

extradrvdir = $(ed_dir)
extradrv_DATA = ud650-extra.so

ARCH = `uname -m`

ud650-extra.so:
	cp ud650-extra.so.${ARCH} ud650-extra.so

clean-local:
	rm -rf ud650-extra.so

endif
