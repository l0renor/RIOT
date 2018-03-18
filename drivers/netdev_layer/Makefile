ifneq (,$(filter netdev_layer_null,$(USEMODULE)))
  DIRS += null
endif

ifneq (,$(filter netdev_layer_eth,$(USEMODULE)))
  DIRS += eth
endif

ifneq (,$(filter netdev_layer_ieee802154,$(USEMODULE)))
  DIRS += ieee802154
endif

include $(RIOTBASE)/Makefile.base
