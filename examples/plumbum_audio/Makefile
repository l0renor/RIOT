# name of your application
APPLICATION := plumbum_audio

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

BOARD_INSUFFICIENT_MEMORY := calliope-mini chronos hifive1 microbit msb-430 msb-430h \
                             nucleo-f031k6 nucleo-f042k6 nucleo-f303k8 nucleo-l031k6 \
                             nucleo-f030r8 nucleo-f070rb nucleo-f072rb nucleo-f103rb nucleo-f302r8 \
                             nucleo-f334r8 nucleo-l053r8 spark-core stm32f0discovery telosb \
                             wsn430-v1_3b wsn430-v1_4 z1

# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

USEMODULE += plumbum
USEMODULE += plumbum_audio
USEMODULE += usb_hid_keyboard
USEMODULE += sam_usb

USEMODULE += xtimer
FEATURES_REQUIRED += periph_pwm

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
