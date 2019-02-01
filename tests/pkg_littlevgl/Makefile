include ../Makefile.tests_common

BOARD_INSUFFICIENT_MEMORY := chronos msb-430 msb-430h nucleo32-f031 nucleo32-f042 \
                             nucleo32-l031 nucleo-f030 nucleo-l053 \
                             stm32f0discovery telosb z1

INCLUDES += -I$(CURDIR)
USEPKG += lvgl
LINKFLAGS += `sdl2-config --libs`


# include this for printing IP addresses
USEMODULE += shell_commands
USEMODULE += xtimer
USEMODULE += lv_core
USEMODULE += lv_objx
USEMODULE += lv_misc
USEMODULE += lv_themes
USEMODULE += lv_draw
USEMODULE += lv_fonts
USEMODULE += lv_hal

# Use different settings when compiling for one of the following (low-memory)
# boards
LOW_MEMORY_BOARDS := nucleo-f334


include $(RIOTBASE)/Makefile.include
