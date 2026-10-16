# $NetBSD$

PKG_OPTIONS_VAR=		PKG_OPTIONS.subnetcalc
PKG_SUPPORTED_OPTIONS=		geoip idn nls
PKG_SUGGESTED_OPTIONS=		geoip idn nls

.include "../../mk/bsd.options.mk"

PLIST_VARS+=			nls

###
### MaxMind GeoIP database lookup support
###
.if !empty(PKG_OPTIONS:Mgeoip)
CMAKE_CONFIGURE_ARGS+=		-DWITH_GEOIP=ON
.  include "../../geography/libmaxminddb/buildlink3.mk"
.else
CMAKE_CONFIGURE_ARGS+=		-DWITH_GEOIP=OFF
.endif

###
### Internationalized Domain Names (IDN) via libidn2
###
.if !empty(PKG_OPTIONS:Midn)
CMAKE_CONFIGURE_ARGS+=		-DWITH_IDN=ON
.  include "../../devel/libidn2/buildlink3.mk"
.else
CMAKE_CONFIGURE_ARGS+=		-DWITH_IDN=OFF
.endif

###
### Native Language Support (Gettext translations)
###
.if !empty(PKG_OPTIONS:Mnls)
USE_TOOLS+=			msgfmt
CMAKE_CONFIGURE_ARGS+=		-DENABLE_NLS=ON
PLIST.nls=			yes
.  include "../../devel/gettext-lib/buildlink3.mk"
.else
CMAKE_CONFIGURE_ARGS+=		-DENABLE_NLS=OFF
.endif
