# $OpenBSD$

COMMENT =	IPv4/IPv6 subnet address calculator

DISTNAME =	subnetcalc-2.7.4
CATEGORIES =	net

HOMEPAGE =	https://www.nntb.no/~dreibh/subnetcalc/
MAINTAINER =	thomas.dreibholz@gmail.com

# GPLv3+
PERMIT_PACKAGE =	Yes

MASTER_SITES =	https://www.nntb.no/~dreibh/subnetcalc/download/
EXTRACT_SUFX =	.tar.xz

# OpenBSD uses MODULES to pull in build systems and common environments
MODULES =	devel/cmake \
		devel/gettext

# WANTLIB ensures the package manager tracks exact shared library dependencies
WANTLIB =	c m idn2 intl

LIB_DEPENDS =	devel/libidn2

# Handle build-time options via FLAVORS instead of FreeBSD OPTIONS
FLAVORS =	geoip
FLAVOR ?=

.if ${FLAVOR}:Mgeoip
LIB_DEPENDS +=	net/libmaxminddb
WANTLIB +=	maxminddb
CONFIGURE_ARGS += -DWITH_GEOIP=ON
.else
CONFIGURE_ARGS += -DWITH_GEOIP=OFF
.endif

CONFIGURE_ARGS += -DWITH_I18N=ON

.include <bsd.port.mk>
