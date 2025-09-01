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
# Copyright (C) 2024-2025 by Thomas Dreibholz
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


#############################################################################
#### VERSION FILE                                                        ####
#############################################################################

CONFIGURE_FILE (
   "${CMAKE_CURRENT_SOURCE_DIR}/package-version.h.in"
   "${CMAKE_CURRENT_BINARY_DIR}/package-version.h"
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


#############################################################################
#### PROGRAMS                                                            ####
#############################################################################

ADD_EXECUTABLE(subnetcalc subnetcalc.cc tools.cc)
TARGET_LINK_LIBRARIES(subnetcalc ${Intl_LIBRARIES})
IF (GEOIP_FOUND)
   TARGET_LINK_LIBRARIES(subnetcalc ${GEOIP_LIBRARY})
ENDIF()
INSTALL(TARGETS     subnetcalc   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES       subnetcalc.1 DESTINATION         ${CMAKE_INSTALL_MANDIR}/man1)
INSTALL(FILES       subnetcalc.bash-completion
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
        RENAME      subnetcalc)
