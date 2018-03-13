PKG_NAME=tweetnacl
PKG_URL=https://github.com/kaspar030/tweetnacl
PKG_VERSION=416bec5f3abd8cdda94ddbc1c093e5243703aee4
PKG_LICENSE=PD

.PHONY: all

all: git-download
	$(Q)"$(MAKE)" -C $(PKG_BUILDDIR) -f $(CURDIR)/Makefile.riot

include $(RIOTBASE)/pkg/pkg.mk
