NAME = simg2simg
SOURCES = simg2simg.c sparse_crc32.c
SOURCES := $(foreach source, $(SOURCES), libsparse/$(source))
CFLAGS += -Ilibsparse/include -fpermissive
LDFLAGS += -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -Wl,-rpath-link=. \
           -L. -lsparse

build: $(SOURCES)
	$(CC) $^ -o libsparse/$(NAME) $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) libsparse/$(NAME)
