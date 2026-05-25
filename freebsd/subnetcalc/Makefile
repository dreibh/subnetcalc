PORTNAME=	subnetcalc
DISTVERSION=	2.6.6
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/subnetcalc/download/

MAINTAINER=	thomas.dreibholz@gmail.com
COMMENT=	IPv4/IPv6 Subnet Calculator
WWW=		https://www.nntb.no/~dreibh/subnetcalc/

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=	libidn2.so:dns/libidn2

USES=		cmake gettext tar:xz

OPTIONS_DEFINE= 	I18N \
			GEOIP

OPTIONS_DEFAULT=	I18N \
			GEOIP

I18N_DESC=		Build with Internationalization (i18n) support
GEOIP_DESC=		Build with GeoIP support

I18N_CMAKE_BOOL=	WITH_I18N
GEOIP_CMAKE_BOOL=	WITH_GEOIP

PLIST_FILES=	bin/subnetcalc \
		share/bash-completion/completions/subnetcalc \
		share/locale/de/LC_MESSAGES/subnetcalc.mo \
		share/locale/ka/LC_MESSAGES/subnetcalc.mo \
		share/locale/nb/LC_MESSAGES/subnetcalc.mo \
		share/man/man1/subnetcalc.1.gz

.include <bsd.port.mk>
