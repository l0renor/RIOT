SRC := nanocoap.c
ifneq (,$(filter nanocoap_sock,$(USEMODULE)))
  SRC += sock.c
endif
include $(RIOTBASE)/Makefile.base
