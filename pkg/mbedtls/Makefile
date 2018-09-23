PKG_NAME=mbedtls
PKG_URL=https://github.com/ARMmbed/mbedtls
PKG_VERSION=mbedtls-2.12.0
PKG_LICENSE=apache-2.0

.PHONY: all

all: git-download
	"$(MAKE)" -C $(PKG_BUILDDIR)/library -f $(CURDIR)/Makefile.mbedtls

include $(RIOTBASE)/pkg/pkg.mk
