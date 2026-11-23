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
# Copyright (C) 2009-2027 by Thomas Dreibholz
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
#### TRANSLATIONS                                                        ####
#############################################################################

SET(FIX_POT_CHARSET_SCRIPT   "${PROJECT_SOURCE_DIR}/cmake/fix-pot-charset.cmake")
SET(CHECK_PO_LANGUAGE_SCRIPT "${PROJECT_SOURCE_DIR}/cmake/check-po-language.cmake")

FILE(GLOB sources CONFIGURE_DEPENDS "*.sources")

# ====== Find all text domains ==============================================
FOREACH(source IN LISTS sources)
   # The .sources file lists the source files for each text domain:
   GET_FILENAME_COMPONENT(textdomain "${source}" NAME_WE)
   SET(potFile "${CMAKE_CURRENT_BINARY_DIR}/${textdomain}.pot")

   # ====== Common xgettext extraction flags and keywords ===================
   SET(xgettextCommonOptions
      --from-code=utf-8
      --add-comments=TRANSLATORS:
      "--default-domain=${textdomain}"
      "--package-name=${PROJECT_NAME}"
      "--package-version=${PROJECT_VERSION}"
      "--copyright-holder=${PROJECT_COPYRIGHT}"
      "--msgid-bugs-address=${PROJECT_BUGTRACKER_URL}"
      --keyword=_
      --keyword=gettext
      --keyword=ngettext:1,2
      --keyword=dgettext:2
      --keyword=dcgettext:2
      --keyword=dngettext:2,3
      --keyword=dcngettext:2,3
      --keyword=eval_gettext
      --keyword=eval_ngettext:1,2
      --keyword=formatText:1
      --flag=formatText:1:no-c-format
      --no-wrap
      --force-po
   )

   # ====== Parse .sources line by line =====================================
   FILE(STRINGS "${source}" sourceLines)
   LIST(FILTER sourceLines EXCLUDE REGEX "^[ \t]*#")
   LIST(FILTER sourceLines EXCLUDE REGEX "^[ \t]*$")

   SET(potInputFilesForDependency "")
   SET(xgettextCommands "")
   SET(isFirstEntry TRUE)

   FOREACH(line IN LISTS sourceLines)
      SEPARATE_ARGUMENTS(lineArgs NATIVE_COMMAND "${line}")

      # Extract source files (excluding CLI options like --language=...) for CMake dependencies
      FOREACH(arg IN LISTS lineArgs)
         IF(NOT arg MATCHES "^[-][-]")
            LIST(APPEND potInputFilesForDependency "${PROJECT_SOURCE_DIR}/${arg}")
         ENDIF()
      ENDFOREACH()

      # First pass initializes the POT file; subsequent passes join with -j
      IF(isFirstEntry)
         SET(isFirstEntry FALSE)
         LIST(APPEND xgettextCommands
              COMMAND "${XGETTEXT}"
                 ${xgettextCommonOptions}
                 -o "${potFile}"
                 ${lineArgs}
         )
      ELSE()
         LIST(APPEND xgettextCommands
              COMMAND "${XGETTEXT}"
                 ${xgettextCommonOptions}
                 -j
                 -o "${potFile}"
                 ${lineArgs}
         )
      ENDIF()
   ENDFOREACH()

   # Ensure UTF-8 charset header in generated template:
   LIST(APPEND xgettextCommands
      COMMAND "${CMAKE_COMMAND}" -D "POT_FILE=${potFile}" -P "${FIX_POT_CHARSET_SCRIPT}"
   )

   MESSAGE(STATUS "Translation template: ${textdomain} (${source} -> ${potFile})")
   ADD_CUSTOM_COMMAND(OUTPUT "${potFile}"
                      ${xgettextCommands}
                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      DEPENDS ${potInputFilesForDependency} "${source}" "${FIX_POT_CHARSET_SCRIPT}"
                      VERBATIM)
   SET(generate_PotFile "generate_pot_${textdomain}")
   ADD_CUSTOM_TARGET(${generate_PotFile} ALL DEPENDS "${potFile}")


   # ====== Process PO files (translations) =================================
   FILE(GLOB_RECURSE poFiles CONFIGURE_DEPENDS "*/${textdomain}.po")
   FOREACH(poFile IN LISTS poFiles)

      # ====== Verify that PO file Language header matches directory ========
      GET_FILENAME_COMPONENT(languageDirectory "${poFile}" DIRECTORY)
      GET_FILENAME_COMPONENT(language "${languageDirectory}" NAME)

      FILE(STRINGS "${poFile}" poLanguageLines REGEX "^\"Language:")
      SET(poLanguageMatched FALSE)
      FOREACH(poLangLine IN LISTS poLanguageLines)
         IF(poLangLine MATCHES "^\"Language:[ \t]*${language}[ \t]*\\\\n\"$")
            SET(poLanguageMatched TRUE)
         ENDIF()
      ENDFOREACH()
      IF(NOT poLanguageMatched)
         MESSAGE(FATAL_ERROR
            "Language mismatch in ${poFile}: directory is '${language}', "
            "found header: ${poLanguageLines} (expected '\"Language: ${language}\\n\"')!")
      ENDIF()


      # ====== Copy and update PO file in build directory ===================
      SET(mergedPoDir "${CMAKE_CURRENT_BINARY_DIR}/${language}")
      SET(mergedPoFile "${mergedPoDir}/${textdomain}.merged.po")

      MESSAGE(STATUS "Translation update: ${textdomain}, ${language} (${poFile} + ${potFile} -> ${mergedPoFile})")
      ADD_CUSTOM_COMMAND(OUTPUT "${mergedPoFile}"
                         COMMAND "${CMAKE_COMMAND}"
                            -D "PO_FILE=${poFile}"
                            -D "EXPECTED_LANGUAGE=${language}"
                            -P "${CHECK_PO_LANGUAGE_SCRIPT}"
                         COMMAND "${CMAKE_COMMAND}" -E make_directory "${mergedPoDir}"
                         COMMAND "${CMAKE_COMMAND}" -E copy "${poFile}" "${mergedPoFile}"
                         COMMAND "${MSGMERGE}"
                            --update
                            --backup=none
                            --no-wrap
                            "${mergedPoFile}"
                            "${potFile}"
                         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                         DEPENDS ${generate_PotFile} "${potFile}" "${poFile}" "${CHECK_PO_LANGUAGE_SCRIPT}"
                         VERBATIM)
      SET(generate_PoFile "generate_po_${textdomain}_${language}")
      ADD_CUSTOM_TARGET(${generate_PoFile} DEPENDS "${mergedPoFile}")


      # ====== Compile merged PO file to MO file ============================
      SET(moDir "${CMAKE_CURRENT_BINARY_DIR}/${language}/LC_MESSAGES")
      SET(moFile "${moDir}/${textdomain}.mo")

      MESSAGE(STATUS "Compiling translation: ${textdomain}, ${language} (${mergedPoFile} -> ${moFile})")
      ADD_CUSTOM_COMMAND(OUTPUT "${moFile}"
                         COMMAND "${CMAKE_COMMAND}" -E make_directory "${moDir}"
                         COMMAND "${MSGFMT}"
                            --statistics
                            --check
                            "--output-file=${moFile}"
                            "${mergedPoFile}"
                         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                         DEPENDS ${generate_PoFile} "${mergedPoFile}"
                         VERBATIM)
      SET(generate_MoFile "generate_mo_${textdomain}_${language}")
      ADD_CUSTOM_TARGET(${generate_MoFile} ALL DEPENDS "${moFile}")


      # ====== Install MO file ==============================================
      INSTALL(FILES "${moFile}"
              DESTINATION "${CMAKE_INSTALL_DATADIR}/locale/${language}/LC_MESSAGES")

   ENDFOREACH()
ENDFOREACH()
