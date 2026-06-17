include $(TOPDIR)/rules.mk

PKG_NAME:=custom-ca
PKG_VERSION:=1.0
PKG_RELEASE:=1

CA_CERT:=KamiStep_Root_CA.crt
CA_HASH:=$(shell openssl x509 -hash -noout -in $(CURDIR)/files/$(CA_CERT))

include $(INCLUDE_DIR)/package.mk

define Package/custom-ca
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Custom root CA
  DEPENDS:=+ca-bundle
endef

define Package/custom-ca/description
  Install custom root CA certificate, OpenSSL hash symlink, and append it to OpenWrt CA bundle.
endef

define Build/Compile
endef

define Package/custom-ca/install
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_DATA) ./files/$(CA_CERT) $(1)/etc/ssl/certs/$(CA_CERT)

	ln -sf $(CA_CERT) $(1)/etc/ssl/certs/$(CA_HASH).0
endef

define Package/custom-ca/postinst
#!/bin/sh

ROOT="$${IPKG_INSTROOT:-$${APK_ROOT:-}}"

CERT="$$ROOT/etc/ssl/certs/KamiStep_Root_CA.crt"
BUNDLE="$$ROOT/etc/ssl/certs/ca-certificates.crt"

MARK_BEGIN="# BEGIN CUSTOM CA"
MARK_END="# END CUSTOM CA"

mkdir -p "$$(dirname "$$BUNDLE")"

if [ -f "$$CERT" ]; then
	if [ ! -f "$$BUNDLE" ]; then
		touch "$$BUNDLE"
	fi

	if ! grep -q "$$MARK_BEGIN" "$$BUNDLE"; then
		{
			echo ""
			echo "$$MARK_BEGIN"
			cat "$$CERT"
			echo "$$MARK_END"
		} >> "$$BUNDLE"
	fi
fi

exit 0
endef

$(eval $(call BuildPackage,custom-ca))
