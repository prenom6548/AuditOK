NAME:= libcrypto_utils
SOURCES := android_pubkey.c
SOURCES := $(foreach source, $(SOURCES), libcrypto_utils/$(source))
CPPFLAGS += -Ilibcrypto_utils/include -Iinclude -I/usr/include/android
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
	-Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
	-L/usr/lib/$(DEB_HOST_MULTIARCH)/android \
	-lcrypto

build: $(SOURCES)
	$(CC) $^ -o $(NAME).so.0  $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*

