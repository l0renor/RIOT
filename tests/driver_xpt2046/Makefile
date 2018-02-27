include ../Makefile.tests_common

BOARD := nucleo-f446

USEMODULE += xpt2046
USEMODULE += xtimer
USEMODULE += printf_float

# set default device parameters in case they are undefined
TEST_SPI ?= SPI_DEV\(1\)
TEST_SPI_CS ?= GPIO_PIN\(PORT_B,1\)
TEST_SPI_CLK ?= SPI_CLK_100KHZ

# export parameters
CFLAGS += -DTEST_SPI=$(TEST_SPI)
CFLAGS += -DTEST_SPI_CS=$(TEST_SPI_CS)
CFLAGS += -DTEST_SPI_CLK=$(TEST_SPI_CLK)

DEVELHELP := 1

include $(RIOTBASE)/Makefile.include
