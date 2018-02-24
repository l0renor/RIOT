PKG_NAME=lvgl
PKG_URL=/home/hydrazine/dev/lvgl
PKG_VERSION=e54541e71d2564a8209a553772f0124bedd02efa
PKG_LICENSE=MIT

CFLAGS += -std=c11
.PHONY: all

all: git-download
	"$(MAKE)" -C $(PKG_BUILDDIR)

include $(RIOTBASE)/pkg/pkg.mk
