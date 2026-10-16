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
USE_TOOLS+=		pkg-config msgfmt

# Configure CMake
CMAKE_CONFIGURE_ARGS+=	-DCMAKE_BUILD_TYPE=Release
CMAKE_CONFIGURE_ARGS+=	-DWITH_GEOIP=ON

# Run CTest with network-dependent tests excluded
do-test:
	cd ${WRKSRC} && ${SETENV} ${TEST_ENV} ctest --output-on-failure -LE network

.include "../../devel/cmake/build.mk"
.include "../../devel/libidn2/buildlink3.mk"
.include "../../geography/libmaxminddb/buildlink3.mk"
.include "../../devel/gettext-lib/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
