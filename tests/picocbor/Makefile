include ../Makefile.tests_common

TEST_ON_CI_WHITELIST += all

CFLAGS += -mfp16-format=ieee

USEMODULE += picocbor
USEMODULE += picocbor_float
USEMODULE += printf_float
# Used for verification
USEPKG += tinycbor
USEMODULE += tinycbor_float
USEMODULE += embunit

include $(RIOTBASE)/Makefile.include
