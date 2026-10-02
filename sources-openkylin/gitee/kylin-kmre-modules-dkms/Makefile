#/usr/bin/make
SRC = $(DESTDIR)/usr/src
SHARE = $(DESTDIR)/usr/share/$(NAME)-dkms

all: clean modules install

modules:
	$(MAKE) -C binder/
	$(MAKE) -C ashmem/
	$(MAKE) -C virtwifi/

clean:
	$(MAKE) -C binder/ clean
	$(MAKE) -C ashmem/ clean
	$(MAKE) -C virtwifi/ clean
