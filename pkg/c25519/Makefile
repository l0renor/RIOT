PKG_NAME    = c25519
PKG_URL     = https://www.dlbeer.co.nz/downloads
PKG_VERSION = 2017-10-05
PKG_EXT     = zip

PKG_BUILDDIR?=$(PKGDIRBASE)/$(PKG_NAME)

ifneq ($(RIOTBASE),)
include $(RIOTBASE)/Makefile.base
endif

.PHONY: all clean distclean

prepare: $(PKG_BUILDDIR)/

all: $(PKG_BUILDDIR)/
	"$(MAKE)" -C $(PKG_BUILDDIR)/src -f $(CURDIR)/Makefile.c25519

$(PKG_BUILDDIR)/: $(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)
	$(Q)$(UNZIP_HERE) -D -d $(PKGDIRBASE) $<

$(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT):
	mkdir -p $(PKGDIRBASE)
	$(Q)$(DOWNLOAD_TO_FILE) $@ $(PKG_URL)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)

clean::
	# Reset package to checkout state.
	rm -rf $(PKG_BUILDDIR)

distclean::
	rm -rf $(PKG_BUILDDIR) $(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)
