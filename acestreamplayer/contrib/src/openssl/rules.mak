# openssl

OSSL_VERSION := 1.0.1e
OSSL_URL := http://www.openssl.org/source/openssl-$(OSSL_VERSION).tar.gz

$(TARBALLS)/openssl-$(OSSL_VERSION).tar.gz:
	$(call download,$(OSSL_URL))

.sum-openssl: openssl-$(OSSL_VERSION).tar.gz

openssl: openssl-$(OSSL_VERSION).tar.gz .sum-openssl
	$(UNPACK)
	$(APPLY) $(SRC)/openssl/openssl.patch
	$(MOVE)

OSSL_PLAFFORM := ''

ifdef HAVE_WIN32
OSSL_PLAFFORM := --cross-compile-prefix=$(HOST)- mingw
endif

.openssl: openssl
	cd $< && ./Configure $(OSSL_PLAFFORM) --openssldir=$(PREFIX)
	cd $< && $(MAKE) && $(MAKE) install
	touch $@
