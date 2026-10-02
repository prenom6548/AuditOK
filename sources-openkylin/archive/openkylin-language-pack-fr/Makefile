DESTDIR ?=
PREFIX ?=

PO_FILES = $(shell find target/po/fr -type f -name "*.po")
MO_FILES = $(PO_FILES:.po=.mo)
TS_FILES = $(shell find target/ts/fr -type f -name "*.ts")
QM_FILES = $(TS_FILES:.ts=.qm)

.PHONY: all clean install install-mo install-qm

$(MO_FILES): 
	python3 bin/fmt.py target/po/fr/ 

target/po/fr/%.qm: target/po/fr/%.ts
	lconvert -i $< -o $@

target/ts/fr/%.qm: target/ts/fr/%.ts
	lconvert -i $< -o $@

clean:
	rm -f $(MO_FILES) $(QM_FILES)

install-mo:
	install -d $(DESTDIR)$(PREFIX)/share/locale-langpack/fr/LC_MESSAGES/
	#install -m 644 $(MO_FILES) $(DESTDIR)$(PREFIX)/share/locale-langpack/fr/LC_MESSAGES/

install-qm: $(QM_FILES)

install: install-mo
