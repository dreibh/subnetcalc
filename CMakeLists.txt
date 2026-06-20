# ==========================================================================
#             ____        _     _   _      _    ____      _
#            / ___| _   _| |__ | \ | | ___| |_ / ___|__ _| | ___
#            \___ \| | | | '_ \|  \| |/ _ \ __| |   / _` | |/ __|
#             ___) | |_| | |_) | |\  |  __/ |_| |__| (_| | | (__
#            |____/ \__,_|_.__/|_| \_|\___|\__|\____\__,_|_|\___|
#
#                    ---  IPv4/IPv6 Subnet Calculator  ---
#                   https://www.nntb.no/~dreibh/subnetcalc/
# ==========================================================================
#
# SubNetCalc - IPv4/IPv6 Subnet Calculator
# Copyright (C) 2024-2026 by Thomas Dreibholz
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: thomas.dreibholz@gmail.com

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(subnetcalc LANGUAGES C CXX)

SET(BUILD_MAJOR "2")
SET(BUILD_MINOR "7")
SET(BUILD_PATCH "4")
SET(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH})

SET(CMAKE_CXX_STANDARD          17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS        OFF)


#############################################################################
#### CMAKE INCLUDES                                                      ####
#############################################################################

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckCSourceRuns)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckStructHasMember)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(GNUInstallDirs)


#############################################################################
#### PACKAGING                                                           ####
#############################################################################

SET(CPACK_SOURCE_GENERATOR "TXZ")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH}")
SET(CPACK_SOURCE_IGNORE_FILES "\\\\.git;\\\\.swp$;~$;\\\\.\\\\#;/\\\\#")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "^${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}[_-]")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.make$|\\\\.log$")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/CMakeCache\\\\.txt$")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/(CMakeFiles|CMakeScripts|_CPack_Packages)/")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/package-version\\\\.h$")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/packaging\\\\.conf$")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "^${PROJECT_SOURCE_DIR}/(po.*/|src.*/|)Makefile$")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "^${PROJECT_SOURCE_DIR}/(po.*|src.*|[^/]*)\\\\.cmake$")
INCLUDE(CPack)

ADD_CUSTOM_TARGET(dist COMMAND ${CMAKE_MAKE_PROGRAM} clean package_source)


#############################################################################
#### UNINSTALL                                                           ####
#############################################################################

IF(NOT TARGET uninstall)
   CONFIGURE_FILE(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)

   ADD_CUSTOM_TARGET(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
ENDIF()


#############################################################################
#### OS DEPENDENT                                                        ####
#############################################################################

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
   MESSAGE(STATUS "${CMAKE_SYSTEM_NAME} supported")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
   MESSAGE(STATUS "${CMAKE_SYSTEM_NAME} supported")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "NetBSD")
   MESSAGE(STATUS "${CMAKE_SYSTEM_NAME} supported")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
   MESSAGE(STATUS "${CMAKE_SYSTEM_NAME} supported")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   MESSAGE(STATUS "${CMAKE_SYSTEM_NAME} supported")
ELSE()
   MESSAGE(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not supported (yet?)")
ENDIF()


#############################################################################
#### CHECK STRUCT MEMBERS                                                ####
#############################################################################

CHECK_STRUCT_HAS_MEMBER("struct sockaddr" "sa_len" "sys/types.h;sys/socket.h" HAVE_SA_LEN)
IF (HAVE_SA_LEN)
   MESSAGE(STATUS "HAVE_SA_LEN")
   ADD_DEFINITIONS(-DHAVE_SA_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in" "sin_len" "sys/types.h;netinet/in.h" HAVE_SIN_LEN)
IF (HAVE_SIN_LEN)
   MESSAGE(STATUS "HAVE_SIN_LEN")
   ADD_DEFINITIONS(-DHAVE_SIN_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6" "sin6_len" "sys/types.h;netinet/in.h" HAVE_SIN6_LEN)
IF (HAVE_SIN6_LEN)
   MESSAGE(STATUS "HAVE_SIN6_LEN")
   ADD_DEFINITIONS(-DHAVE_SIN6_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" "ss_len" "sys/types.h;sys/socket.h" HAVE_SS_LEN)
IF (HAVE_SS_LEN)
   MESSAGE(STATUS "HAVE_SS_LEN")
   ADD_DEFINITIONS(-DHAVE_SS_LEN)
ENDIF()


#############################################################################
#### OPTIONS                                                             ####
#############################################################################

OPTION(WITH_I18N  "Build with Internationalization (i18n) support" ON)
OPTION(WITH_GEOIP "Build with GeoIP support"                       ON)


#############################################################################
#### REQUIREMENTS                                                        ####
#############################################################################

CHECK_CXX_SOURCE_COMPILES("
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
int main(int argc, char** argv)
{
    int x = NI_IDN;
    return 0;
}
" HAVE_NI_IDN)

# ====== IDN2 necessary to support IDN if not provided by libc ==============
IF (NOT HAVE_NI_IDN)
   # There is no IDN support directly in getaddrinfo() and getnameinfo()
   # => libidn2 is required for IDN support
   FIND_LIBRARY(LIBIDN2_LIBRARY NAMES idn2 PATHS /usr/local/lib)
   FIND_PATH(LIBIDN2_INCLUDE_DIR NAMES idn2.h PATHS /usr/local/include)
   IF ((LIBIDN2_LIBRARY) AND (LIBIDN2_INCLUDE_DIR))
      MESSAGE(STATUS "IDN2 library found:")
      MESSAGE(STATUS " LIBIDN2_LIBRARY:     ${LIBIDN2_LIBRARY}")
      MESSAGE(STATUS " LIBIDN2_INCLUDE_DIR: ${LIBIDN2_INCLUDE_DIR}")
   ELSE()
      MESSAGE(FATAL_ERROR
              "Cannot find IDN2 library! Try:\n"
              " * FreeBSD: sudo pkg install -y libidn2\n"
              " * NetBSD:  sudo pkgin -y install libidn2\n"
              " * OpenBSD: sudo pkg_add libidn2\n"
              " * Apple:   brew install libidn2\n")
   ENDIF()
ENDIF()

# ====== Liberty ============================================================
IF (CMAKE_SYSTEM_NAME MATCHES "NetBSD")
   FIND_LIBRARY(LIBIBERTY_LIBRARY NAMES iberty PATHS /usr/pkg/lib)
   FIND_PATH(LIBIBERTY_INCLUDE_DIR NAMES libiberty.h PATHS /usr/pkg/include)
   IF ((LIBIBERTY_LIBRARY) AND (LIBIBERTY_INCLUDE_DIR))
      ADD_DEFINITIONS(-DHAVE_LIBIBERTY)
      MESSAGE(STATUS "Libiberty library found:")
      MESSAGE(STATUS " LIBIBERTY_LIBRARY:     ${LIBIBERTY_LIBRARY}")
      MESSAGE(STATUS " LIBIBERTY_INCLUDE_DIR: ${LIBIBERTY_INCLUDE_DIR}")
   ELSE()
      MESSAGE(FATAL_ERROR
              "Cannot find Libiberty library! Try:\n"
              " * NetBSD: sudo pkgin -y install liberty")
   ENDIF()
ENDIF()


# ====== GeoIP ==============================================================
IF (WITH_GEOIP)
   FIND_LIBRARY(MAXMINDDB_LIBRARY NAMES maxminddb)
   FIND_PATH(MAXMINDDB_INCLUDE_DIR NAMES maxminddb.h)
   IF ((MAXMINDDB_LIBRARY) AND (MAXMINDDB_INCLUDE_DIR))
      FIND_PACKAGE_HANDLE_STANDARD_ARGS(GeoIP DEFAULT_MSG MAXMINDDB_LIBRARY MAXMINDDB_INCLUDE_DIR)
      ADD_DEFINITIONS(-DHAVE_MAXMINDDB)
      MESSAGE(STATUS "MaxMindDB library found:")
      MESSAGE(STATUS " MAXMINDDB_LIBRARY:     ${MAXMINDDB_LIBRARY}")
      MESSAGE(STATUS " MAXMINDDB_INCLUDE_DIR: ${MAXMINDDB_INCLUDE_DIR}")
   ELSE()
      MESSAGE(FATAL_ERROR
              "Cannot find MaxMindDB library! Try:\n"
              " * Ubuntu/Debian: sudo apt install -y libmaxminddb-dev\n"
              " * Fedora:        sudo dnf install -y libmaxminddb-devel\n"
              " * OpenSUSE:      sudo zypper install -y libmaxminddb-devel\n"
              " * Alpine:        sudo apk add libmaxminddb-dev\n"
              " * FreeBSD:       sudo pkg install -y libmaxminddb\n"
              " * NetBSD:        sudo pkgin -y install libmaxminddb\n"
              " * OpenBSD:       sudo pkg_add libmaxminddb\n"
              " * Apple:         brew install libmaxminddb")
   ENDIF()
ENDIF()


#############################################################################
#### INTERNATIONALISATION (I18N)                                         ####
#############################################################################

IF (WITH_I18N)
   FIND_PACKAGE(Intl)
   IF (Intl_FOUND)
      MESSAGE(STATUS "Internationalization (i18n) support found:")
      MESSAGE(STATUS " Intl_INCLUDE_DIRS: ${Intl_INCLUDE_DIRS}")
      MESSAGE(STATUS " Intl_LIBRARIES:    ${Intl_LIBRARIES}")
      MESSAGE(STATUS " Version:           ${Intl_VERSION}")
   ELSE()
      MESSAGE(FATAL_ERROR
            "Cannot find Internationalization (i18n) support! Try:\n"
            " * Ubuntu/Debian: sudo apt install -y libc6-dev\n"
            " * Fedora:        sudo dnf install -y gettext-devel\n"
            " * OpenSUSE:      sudo zypper install -y gettext-devel\n"
            " * Alpine:        sudo apk add gettext-dev\n"
            " * FreeBSD:       sudo pkg install -y gettext\n"
            " * NetBSD:        sudo pkgin -y install gettext\n"
            " * OpenBSD:       sudo pkg_add gettext-runtime gettext-tools\n"
            " * Apple:         brew install gettext")
   ENDIF()

   FIND_PROGRAM(XGETTEXT xgettext HINTS /usr/pkg/bin /opt/homebrew/opt/gettext/bin /usr/local/opt/gettext/bin)
   FIND_PROGRAM(MSGMERGE msgmerge HINTS /usr/pkg/bin /opt/homebrew/opt/gettext/bin /usr/local/opt/gettext/bin)
   FIND_PROGRAM(MSGFMT   msgfmt   HINTS /usr/pkg/bin /opt/homebrew/opt/gettext/bin /usr/local/opt/gettext/bin)

   IF (XGETTEXT AND MSGMERGE AND MSGFMT)
      MESSAGE(STATUS "GNU gettext support found:")

      EXECUTE_PROCESS(COMMAND ${XGETTEXT} --version
                      OUTPUT_VARIABLE XGETTEXT_OUTPUT
                      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      STRING(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+|[0-9]+\\.[0-9]+)" XGETTEXT_VERSION "${XGETTEXT_OUTPUT}")

      EXECUTE_PROCESS(COMMAND ${MSGMERGE} --version
                      OUTPUT_VARIABLE MSGMERGE_OUTPUT
                      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      STRING(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+|[0-9]+\\.[0-9]+)" MSGMERGE_VERSION "${MSGMERGE_OUTPUT}")

      EXECUTE_PROCESS(COMMAND ${MSGFMT} --version
                      OUTPUT_VARIABLE MSGFMT_OUTPUT
                      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      STRING(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+|[0-9]+\\.[0-9]+)" MSGFMT_VERSION "${MSGFMT_OUTPUT}")

      MESSAGE(STATUS " xgettext: ${XGETTEXT} (version ${XGETTEXT_VERSION})")
      MESSAGE(STATUS " msgmerge: ${MSGMERGE} (version ${MSGMERGE_VERSION})")
      MESSAGE(STATUS " msgfmt:   ${MSGFMT} (version ${MSGFMT_VERSION})")
   ENDIF()

   SET(GETTEXT_MIN_VERSION 0.19)
   IF ((NOT XGETTEXT) OR (NOT MSGMERGE) OR (NOT MSGFMT)       OR
       (XGETTEXT_VERSION VERSION_LESS ${GETTEXT_MIN_VERSION}) OR
       (MSGMERGE_VERSION VERSION_LESS ${GETTEXT_MIN_VERSION}) OR
       (MSGFMT_VERSION   VERSION_LESS ${GETTEXT_MIN_VERSION}))
      MESSAGE(FATAL_ERROR
              "Cannot find gettext tools >= ${GETTEXT_MIN_VERSION}! Try:\n"
              " * Ubuntu/Debian: sudo apt install -y gettext\n"
              " * Fedora:        sudo dnf install -y gettext\n"
              " * OpenSUSE:      sudo zypper install -y gettext\n"
              " * Alpine:        sudo apk add gettext-dev\n"
              " * FreeBSD:       sudo pkg install -y gettext-tools\n"
              " * NetBSD:        sudo pkgin -y install gettext-tools\n"
              " * OpenBSD:       sudo pkg_add gettext-tools\n"
              " * Apple:         brew install gettext")
   ENDIF()

   ADD_DEFINITIONS(-DENABLE_NLS)
ENDIF()


#############################################################################
#### SUBDIRECTORIES                                                      ####
#############################################################################

ADD_SUBDIRECTORY(src)

IF (WITH_I18N)
   ADD_SUBDIRECTORY(po)
ENDIF()
