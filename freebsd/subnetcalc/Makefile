PORTNAME=	subnetcalc
DISTVERSION=	2.7.1
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/subnetcalc/download/

MAINTAINER=	thomas.dreibholz@gmail.com
COMMENT=	IPv4/IPv6 Subnet Calculator
WWW=		https://www.nntb.no/~dreibh/subnetcalc/

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=	libidn2.so:dns/libidn2

USES=		cmake tar:xz

OPTIONS_DEFINE= 	I18N \
			GEOIP
OPTIONS_DEFAULT=	${OPTIONS_DEFINE}
OPTIONS_SUB=		yes

I18N_DESC=		Build with Internationalization (i18n) support
GEOIP_DESC=		Build with GeoIP support

I18N_CMAKE_BOOL=	WITH_I18N
I18N_USES=		gettext

GEOIP_CMAKE_BOOL=	WITH_GEOIP
GEOIP_LIB_DEPENDS=	libmaxminddb.so:net/libmaxminddb

.include <bsd.port.mk>
