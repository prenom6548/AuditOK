CUR_DIR = $(shell pwd)
ROOT_DIR ?= $(shell readlink -m ../../)
LOCALEDIR ?= /usr/share/locale

GMSGFMT = /usr/bin/msgfmt
MSGFMT = /usr/bin/msgfmt
XGETTEXT = /usr/bin/xgettext
INTLTOOL_UPDATE = /usr/bin/intltool-update
INTLTOOL_EXTRACT = /usr/bin/intltool-extract
MSGMERGE = INTLTOOL_EXTRACT="$(INTLTOOL_EXTRACT)" XGETTEXT="$(XGETTEXT)" srcdir=$(CUR_DIR) $(INTLTOOL_UPDATE) --gettext-package $(GETTEXT_PACKAGE) --dist
GENPOT   = INTLTOOL_EXTRACT="$(INTLTOOL_EXTRACT)" XGETTEXT="$(XGETTEXT)" srcdir=$(CUR_DIR) $(INTLTOOL_UPDATE) --gettext-package $(GETTEXT_PACKAGE) --pot

GETTEXT_PACKAGE = biometric-driver-fido
USE_LINGUAS = zh_CN

#POTFILES = \
#	$(ROOT_DIR)/src/em1600_internal.c \
#	$(ROOT_DIR)/src/em1600.c


CATALOGS=$(shell LINGUAS="$(USE_LINGUAS)"; for lang in $$LINGUAS; do printf "$$lang.mo "; done)

all: $(CATALOGS)

%.mo: %.po
	msgfmt $*.po -o $*.mo

install: all
	linguas="$(USE_LINGUAS)"; \
	for lang in $$linguas; do \
		dir=$(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES; \
		install -d $$dir; \
		if test -r $$lang.mo; then \
			install -m 644 $$lang.mo $$dir/$(GETTEXT_PACKAGE).mo; \
		fi; \
	done;

uninstall:
	for lang in $$linguas; do \
		dir=$(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES; \
		rm -rf  $$dir/$(GETTEXT_PACKAGE).mo; \
	done;


$(GETTEXT_PACKAGE).pot: $(POTFILES)
	$(GENPOT)

update-po:
	$(MAKE) clean
	$(MAKE) $(GETTEXT_PACKAGE).pot
	tmpdir=`pwd`; \
	linguas="$(USE_LINGUAS)"; \
	for lang in $$linguas; do \
		echo "$$lang:"; \
		result="`$(MSGMERGE) -o $$tmpdir/$$lang.new.po $$lang`"; \
		if $$result; then \
			if cmp $(CUR_DIR)/$$lang.po $$tmpdir/$$lang.new.po >/dev/null 2>&1; then \
				rm -f $$tmpdir/$$lang.new.po; \
			else \
				if mv -f $$tmpdir/$$lang.new.po $$lang.po; then \
					:; \
				else \
					echo "msgmerge for $$lang.po failed: cannot move $$tmpdir/$$lang.new.po to $$lang.po" 1>&2; \
					rm -f $$tmpdir/$$lang.new.po; \
					exit 1; \
				fi; \
			fi; \
		else \
			echo "msgmerge for $$lang.gmo failed!"; \
			rm -f $$tmpdir/$$lang.new.po; \
		fi; \
	done

clean:
	rm -rf *.pot *.mo

