DEVELHELP ?= 0
include ../Makefile.tests_common

USEMODULE += suit_v4_cbor
USEPKG += tinycbor

manifest.cbor: manifest.yaml
	python yaml2cbor.py $< $@

manifest.cbor.h: manifest.cbor
	xxd -i $< $@

all: manifest.cbor.h

include $(RIOTBASE)/Makefile.include
