# name of your application
APPLICATION = cryptotest

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..
CFLAGS += "-DTHREAD_STACKSIZE_MAIN=((8*THREAD_STACKSIZE_DEFAULT)+THREAD_EXTRA_STACKSIZE_PRINTF)"


# Uncomment this to enable scheduler statistics for ps:
USEMODULE += schedstatistics

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1


# Modules to include:
USEMODULE += ps
USEPKG += mbedtls
USEMODULE += random
# include and auto-initialize all available sensors

ifneq (,$(filter wolfssl,$(USEPKG)))
  # WolfSSL flash optimization
  CFLAGS += -DUSE_SLOW_SHA512
  USEMODULE += wolfssl
  USEMODULE += wolfcrypt
  USEMODULE += wolfcrypt_ed25519
  USEMODULE += wolfcrypt_sha512
#  USEMODULE += wolfcrypt_misc
  USEMODULE += wolfcrypt_random
endif

#CFLAGS += "-DDO_SIGN"
include $(RIOTBASE)/Makefile.include
