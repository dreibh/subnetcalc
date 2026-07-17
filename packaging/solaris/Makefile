#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

BUILD_BITS=		64
BUILD_STYLE=		cmake
include ../../../make-rules/shared-macros.mk

COMPONENT_NAME=		subnetcalc
COMPONENT_VERSION=	2.7.4
COMPONENT_SUMMARY=	IPv4/IPv6 Subnet Calculator
COMPONENT_PROJECT_URL=	https://www.nntb.no/~dreibh/subnetcalc/
COMPONENT_SRC=		$(COMPONENT_NAME)-$(COMPONENT_VERSION)
COMPONENT_ARCHIVE=	$(COMPONENT_SRC).tar.xz
COMPONENT_ARCHIVE_URL=	https://www.nntb.no/~dreibh/subnetcalc/download/$(COMPONENT_ARCHIVE)
COMPONENT_ARCHIVE_HASH=	sha256:2ef4e8bf77bbc9f866cd433ee5e8dcc3924a7e858128371863d6720584fa0033
COMPONENT_FMRI=		net/subnetcalc
COMPONENT_CLASSIFICATION=System/Administration and Configuration
COMPONENT_LICENSE=	GPLv3+
COMPONENT_LICENSE_FILE=	$(COMPONENT_SRC)/COPYING

TEST_TARGET=		$(NO_TESTS)

# Feature flags passed directly to CMake
CMAKE_OPTIONS += -DWITH_I18N=ON
CMAKE_OPTIONS += -DWITH_GEOIP=ON

# System runtime/build dependencies on OpenIndiana
REQUIRED_PACKAGES += library/libidn2
REQUIRED_PACKAGES += library/libmaxminddb

include $(WS_MAKE_RULES)/common.mk
