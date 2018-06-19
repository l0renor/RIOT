PKG_NAME=tinycbor
PKG_URL=https://github.com/intel/tinycbor
PKG_VERSION=9e226103da7977047633de532dff7ec0d6e7b1fa
PKG_LICENSE=MIT

.PHONY: all

all: git-download
	"$(MAKE)" -C $(PKG_BUILDDIR)/src -f $(CURDIR)/Makefile.tinycbor

include $(RIOTBASE)/pkg/pkg.mk
