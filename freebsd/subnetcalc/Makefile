# Created by: Thomas Dreibholz <dreibh@iem.uni-due.de>
# $FreeBSD: head/net/subnetcalc/Makefile 427596 2016-12-03 08:11:52Z jhale $

PORTNAME=	subnetcalc
PORTVERSION=	2.4.13
CATEGORIES=	net
MASTER_SITES=	https://www.uni-due.de/~be0001/subnetcalc/download/

MAINTAINER=	dreibh@iem.uni-due.de
COMMENT=	IPv4/IPv6 Subnet Calculator

LICENSE=	GPLv3+
LICENSE_FILE=   ${WRKSRC}/COPYING

LIB_DEPENDS=	libGeoIP.so:net/GeoIP

USES=           tar:xz cmake

CMAKE_ARGS+=    -DCMAKE_INSTALL_MANDIR=${PREFIX}/man

.include <bsd.port.mk>
