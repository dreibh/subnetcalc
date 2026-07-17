# $NetBSD$

DISTNAME=	subnetcalc-2.7.4
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/subnetcalc/download/
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	thomas.dreibholz@gmail.com
HOMEPAGE=	https://www.nntb.no/~dreibh/subnetcalc/
COMMENT=	IPv4/IPv6 Subnet Calculator
LICENSE=	gnu-gpl-v3

USE_CMAKE=	yes
USE_LANGUAGES=	c c++

.include "options.mk"

.include "../../devel/libidn2/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
