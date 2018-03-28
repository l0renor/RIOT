PKG_NAME=libsodium
PKG_URL=https://github.com/jedisct1/libsodium.git
PKG_VERSION=1.0.16
PKG_LICENSE=ISC


# Enable on native to build for x86
#LIBSODIUM_CFLAGS = -m32

# enable for non-native
LIBSODIUM_LDFLAGS += -specs=nano.specs -specs=nosys.specs
all: git-download
	mkdir -p $(PKG_BUILDDIR)/output
	cd $(PKG_BUILDDIR) && ./autogen.sh
	cd $(PKG_BUILDDIR) && CPP="$(CPP)" CC="$(CC)" CXX="$(CXX)"\
		AR="$(AR)" RANLIB="$(RANLIB)" NM="$(NM)" \
		STRIP="$(STRIP)" \
		CPPFLAGS="$(CFLAGS_CPU) " \
		CFLAGS="$(CFLAGS_CPU) $(LIBSODIUM_CFLAGS)" \
		CXXFLAGS="$(CFLAGS_CPU) -fno-exceptions -fno-rtti " \
		LDFLAGS="$(CFLAGS_CPU) $(LIBSODIUM_LDFLAGS) \
		-Wl,--gc-sections -Wl,-Map=map.map " \
		./configure --host=$(TARGET_ARCH) --target=$(TARGET_ARCH) \
		--prefix=$(PKG_BUILDDIR)/output --without-pthreads --enable-minimal --disable-asm --disable-pie --disable-ssp --enable-fast-install=no --enable-shared=no
	make -C $(PKG_BUILDDIR) install
	cp $(PKG_BUILDDIR)/output/lib/libsodium.a $(BINDIR)/libsodium.a


include $(RIOTBASE)/pkg/pkg.mk
