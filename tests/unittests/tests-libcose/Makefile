include $(RIOTBASE)/Makefile.base

USEPKG += cn-cbor
USEPKG += tweetnacl
USEPKG += libcose
USEMODULE += memarray
# Tests will fail on platforms <64 bit if not set.
# Workaround for missing overflow detection in cn-cbor.
CFLAGS += -DCBOR_NO_LL
CFLAGS += -g3
CFLAGS += -DUSE_CBOR_CONTEXT

