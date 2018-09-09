SRCS := plumbum.c
SRCS += plumbum_hdrs.c

ifneq (,$(filter plumbum_audio,$(USEMODULE)))
  DIRS += audio 
endif

include $(RIOTBASE)/Makefile.base
