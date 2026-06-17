include $(TOPDIR)/rules.mk

PKG_NAME:=custom-ca
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/custom-ca
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Custom Root CA Certificate
  DEPENDS:=+ca-bundle +ca-certificates
endef

define Package/custom-ca/description
  Install custom root CA certificate into OpenWrt trust store.
endef

define Build/Compile
endef

define Package/custom-ca/install
	$(INSTALL_DIR) $(1)/usr/share/ca-certificates/custom
	$(INSTALL_DATA) ./files/my-root-ca.crt $(1)/usr/share/ca-certificates/custom/my-root-ca.crt

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/99-custom-ca $(1)/etc/uci-defaults/99-custom-ca
endef

$(eval $(call BuildPackage,custom-ca))
