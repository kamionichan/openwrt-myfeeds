include $(TOPDIR)/rules.mk

PKG_NAME:=qnap-301w-aqr-fw
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/qnap-301w-aqr-fw
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_qualcommax_ipq807x +aq-fw-download
  TITLE:=QNAP 301W Aquantia PHY firmware loader
endef

define Package/qnap-301w-aqr-fw/description
  Load QNAP-provisioned AQR113C 5.4.C firmware into both PHYs at boot.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS) \
		-o $(PKG_BUILD_DIR)/qnap-aqr-mdio ./src/aqr-mdio.c
endef

define Package/qnap-301w-aqr-fw/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/sbin $(1)/lib/firmware/marvell
	$(INSTALL_BIN) ./files/qnap-aqr-fw.init $(1)/etc/init.d/qnap-aqr-fw
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qnap-aqr-mdio $(1)/usr/sbin/qnap-aqr-mdio
	$(INSTALL_DATA) ./files/AQR-G4_v5.4.C-AQR_CIG_WF-1945_0x0_ID44778_VER1630.cld $(1)/lib/firmware/marvell/
	$(INSTALL_DATA) ./files/AQR-G4_v5.4.C-AQR_CIG_WF-1945_0x8_ID44776_VER1630.cld $(1)/lib/firmware/marvell/
endef

define Package/qnap-301w-aqr-fw/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || /etc/init.d/qnap-aqr-fw enable
exit 0
endef

$(eval $(call BuildPackage,qnap-301w-aqr-fw))
