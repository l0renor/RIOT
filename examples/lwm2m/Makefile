# name of your application
APPLICATION = lwm2m_updater

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

BOARD_INSUFFICIENT_MEMORY := calliope-mini chronos microbit msb-430 msb-430h \
                             nucleo32-f031 nucleo32-f042 nucleo32-f303 nucleo32-l031 \
                             nucleo-f030 nucleo-f070 nucleo-f072 nucleo-f103 nucleo-f302 \
                             nucleo-f334 nucleo-l053 spark-core stm32f0discovery telosb \
                             wsn430-v1_3b wsn430-v1_4 z1

# include ota modules
#USEMODULE += ota_suit
#USEMODULE += suit
#USEMODULE += firmware
USEMODULE += lwm2m
USEMODULE += lwm2m_device
USEMODULE += lwm2m_firmware
USEMODULE += rdcli_simple_standalone
USEMODULE += rdcli_lwm2m

# include CoAP server
USEMODULE += nanocoap_sock

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_udp
USEMODULE += gnrc_sock_udp
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
USEMODULE += posix

USEMODULE += libcose_crypt_tweetnacl

DEVELHELP = 0

# For debugging and demonstration purposes, we limit the lifetime to the minimal
# allowed value of 60s (see draft-ietf-core-resource-directory-11, Table 2)
RD_LT ?= 300
# Override this variable to set the RD server address (default is the all nodes
# multicast address)
RD_ADDR ?= \"2001:db8:aaab:0:12e2:d5ff:ff00:1d9\"
RD_PORT ?= 5685
CFLAGS += -DRDCLI_LT=$(RD_LT)
CFLAGS += -DRDCLI_SERVER_ADDR=$(RD_ADDR)
CFLAGS += -DRDCLI_SERVER_PORT=$(RD_PORT)

FEATURES_REQUIRED += riotboot
CFLAGS += -I$(CURDIR)
include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
ifneq (,$(filter cc110x,$(USEMODULE)))          # radio is cc110x sub-GHz
  DEFAULT_CHANNEL ?= 0
  CFLAGS += -DCC110X_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
else
  ifneq (,$(filter at86rf212b,$(USEMODULE)))    # radio is IEEE 802.15.4 sub-GHz
    DEFAULT_CHANNEL ?= 5
    CFLAGS += -DIEEE802154_DEFAULT_SUBGHZ_CHANNEL=$(DEFAULT_CHANNEL)
  else                                          # radio is IEEE 802.15.4 2.4 GHz
    DEFAULT_CHANNEL ?= 26
    CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
  endif
endif
