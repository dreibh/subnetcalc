# ===========================================================================
#                ____        _ _     _     _____           _
#               | __ ) _   _(_) | __| |   |_   _|__   ___ | |___
#               |  _ \| | | | | |/ _` |_____| |/ _ \ / _ \| / __|
#               | |_) | |_| | | | (_| |_____| | (_) | (_) | \__ \
#               |____/ \__,_|_|_|\__,_|     |_|\___/ \___/|_|___/
#
#                              --- Build-Tools ---
#                    https://www.nntb.no/~dreibh/build-tools/
# ===========================================================================
#
# Build-Tools
# Copyright (C) 2017-2026 by Thomas Dreibholz
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

FILE(STRINGS "${PO_FILE}" poLanguageLines REGEX "^\"Language:")
SET(poLanguageMatched FALSE)
FOREACH(poLangLine IN LISTS poLanguageLines)
   IF (poLangLine MATCHES "^\"Language:[ \t]*${EXPECTED_LANGUAGE}[ \t]*\\\\n\"$")
      set(poLanguageMatched TRUE)
   ENDIF()
ENDFOREACH()
if(NOT poLanguageMatched)
   MESSAGE(FATAL_ERROR
      "Language mismatch in ${PO_FILE}: directory is '${EXPECTED_LANGUAGE}', "
      "found header: ${poLanguageLines} (expected '\"Language: ${EXPECTED_LANGUAGE}\\n\"')!")
endif()
