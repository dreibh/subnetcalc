# $NetBSD$

DISTNAME=	subnetcalc-3.0.0
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/subnetcalc/download/
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	dreibh@simula.no
HOMEPAGE=	https://www.nntb.no/~dreibh/subnetcalc/
COMMENT=	IPv4/IPv6 Subnet Calculator
LICENSE=	gnu-gpl-v3

USE_LANGUAGES=		c c++
USE_CXX_FEATURES+=	c++20
USE_TOOLS+=		pkg-config

CMAKE_CONFIGURE_ARGS+=	-DCMAKE_BUILD_TYPE=Release

do-test:
	cd ${WRKSRC} && ${SETENV} ${TEST_ENV} ctest --output-on-failure -E network

.include "options.mk"

.include "../../devel/cmake/build.mk"
.include "../../mk/bsd.pkg.mk"
