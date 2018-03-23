PKG_NAME=libcose
PKG_URL=/home/hydrazine/dev/libcose
PKG_VERSION=ba87e34858ed5f63668130020efe271b58ead6e6
PKG_LICENSE=LGPL

.PHONY: all

all: git-download
	"$(MAKE)" -C $(PKG_BUILDDIR)/src -f $(CURDIR)/Makefile.libcose
	"$(MAKE)" -C $(PKG_BUILDDIR)/src/crypt -f $(CURDIR)/Makefile.libcose_crypt


include $(RIOTBASE)/pkg/pkg.mk
