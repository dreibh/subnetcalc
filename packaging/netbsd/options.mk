# $NetBSD$

PKG_OPTIONS_VAR=		PKG_OPTIONS.subnetcalc
PKG_SUPPORTED_OPTIONS=		i18n geoip
PKG_SUGGESTED_OPTIONS=		i18n geoip

.include "../../mk/bsd.options.mk"

PLIST_VARS+=			i18n

###
### Internationalization (i18n) support
###
.if !empty(PKG_OPTIONS:Mi18n)
CMAKE_CONFIGURE_ARGS+=		-DWITH_I18N=ON
.include "../../devel/gettext-lib/buildlink3.mk"
PLIST.i18n=			yes
.else
CMAKE_CONFIGURE_ARGS+=		-DWITH_I18N=OFF
.endif

###
### GeoIP support
###
.if !empty(PKG_OPTIONS:Mgeoip)
CMAKE_CONFIGURE_ARGS+=		-DWITH_GEOIP=ON
.include "../../geography/libmaxminddb/buildlink3.mk"
.else
CMAKE_CONFIGURE_ARGS+=		-DWITH_GEOIP=OFF
.endif
