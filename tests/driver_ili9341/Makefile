include ../Makefile.tests_common

BOARD := nucleo-f401re
BOARD := stm32f429i-disc1

DEVELHELP ?= 1

USEMODULE += ili9341
USEMODULE += xtimer

## set default device parameters in case they are undefined
#TEST_SPI ?= SPI_DEV\(0\)
#TEST_SPI_CS ?= GPIO_PIN\(PORT_B,6\)
#TEST_SPI_CLK ?= SPI_CLK_10MHZ
#TEST_PIN_DC ?= GPIO_PIN\(PORT_C,7\)
#TEST_PIN_RST ?= GPIO_UNDEF
#
## export parameters
#CFLAGS += -DTEST_SPI=$(TEST_SPI)
#CFLAGS += -DTEST_SPI_CS=$(TEST_SPI_CS)
#CFLAGS += -DTEST_SPI_CLK=$(TEST_SPI_CLK)
#CFLAGS += -DTEST_PIN_DC=$(TEST_PIN_DC)
#CFLAGS += -DTEST_PIN_RST=$(TEST_PIN_RST)

CFLAGS += -DILI9341_LE_MODE

include $(RIOTBASE)/Makefile.include
