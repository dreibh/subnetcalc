# New ports collection makefile for: subnetcalc
# Date created:                      27 July 2010
# Whom:                              Thomas Dreibholz <dreibh@iem.uni-due.de>
#
# $FreeBSD: ports/net/subnetcalc/Makefile,v 1.1 2010/08/04 20:00:20 ohauer Exp $
#

PORTNAME=	subnetcalc
PORTVERSION=	2.2.0
CATEGORIES=	net
MASTER_SITES=	http://www.iem.uni-due.de/~dreibh/subnetcalc/download/ \
		http://www.exp-math.uni-essen.de/~dreibh/subnetcalc/download/

MAINTAINER=	dreibh@iem.uni-due.de
COMMENT=	IPv4/IPv6 Subnet Calculator

LIB_DEPENDS=	GeoIP.5:${PORTSDIR}/net/GeoIP

GNU_CONFIGURE=	yes
CONFIGURE_ARGS=	"--with-geoip=${LOCALBASE}"
USE_GMAKE=	yes

LICENSE=	GPLv3
LICENSE_FILE=	${WRKSRC}/COPYING

PLIST_FILES=	bin/subnetcalc
MAN1=		subnetcalc.1

.include <bsd.port.mk>
