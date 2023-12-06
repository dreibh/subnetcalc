# Created by: Thomas Dreibholz <thomas.dreibholz@gmail.com>
# $FreeBSD: head/net/subnetcalc/Makefile 509302 2019-08-19 19:22:25Z jgh $

PORTNAME=	subnetcalc
PORTVERSION=	2.4.23
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/subnetcalc/download/

MAINTAINER=	thomas.dreibholz@gmail.com
COMMENT=	IPv4/IPv6 Subnet Calculator

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

USES=		tar:xz cmake
CMAKE_ARGS+=	-DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man

PLIST_FILES=	bin/subnetcalc \
		man/man1/subnetcalc.1.gz

.include <bsd.port.mk>
