# System-Tools
# Copyright (C) 2013-2024 by Thomas Dreibholz
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
# Contact: dreibh@simula.no


#############################################################################
#### TRANSLATIONS                                                        ####
#############################################################################

SET(copyrightHolder   "Copyright (C) 2013-2024 by Thomas Dreibholz")
SET(bugTrackerAddress "https://github.com/dreibh/system-tools/issues")

FILE(GLOB sources "*.sources")

# ====== Find all text domains ==============================================
FOREACH(source IN LISTS sources)
   # The .sources file lists the source files for each text domain:
   GET_FILENAME_COMPONENT(potFile "${source}" NAME_WE)
   SET(potFile "${CMAKE_CURRENT_BINARY_DIR}/${potFile}.pot")
   GET_FILENAME_COMPONENT(textdomain "${source}" NAME_WE)


   # ====== Create POT file (translation template) ==========================
   FILE(READ "${source}" potInputFiles)
   STRING(REGEX REPLACE "[ \n]" ";" potInputFiles "${potInputFiles}")
   SET(LIST potInputFilesForDependency "")
   SET(potInputFilesForGetText "")
   FOREACH(file ${potInputFiles})
      GET_FILENAME_COMPONENT(fileName ${file} NAME)
      IF (${fileName} MATCHES "^[-][-]")
         # Option -> Just add it for gettext
         SET(potInputFilesForGetText "${potInputFilesForGetText} F=${fileName}")
      ELSE()
         # File -> Add absolute file name
         SET(potInputFilesForGetText "${potInputFilesForGetText} ${PROJECT_SOURCE_DIR}/${file}")
         LIST(APPEND potInputFilesForDependency ${potInputFilesAbsolute} "${PROJECT_SOURCE_DIR}/${file}")
      ENDIF()
   ENDFOREACH()

   MESSAGE("Translation template: ${textdomain} (${potInputFiles} -> ${potFile})")
   ADD_CUSTOM_COMMAND(OUTPUT "${potFile}"
                      COMMAND "${XGETTEXT}"
                         --from-code=utf-8
                         --default-domain="${textdomain}"
                         --package-name="${PROJECT_NAME}"
                         --package-version="${BUILD_VERSION}"
                         --copyright-holder="${copyrightHolder}"
                         --msgid-bugs-address="${bugTrackerAddress}"
                         -p "${CMAKE_CURRENT_SOURCE_DIR}"
                         -o "${potFile}"
                         ${potInputFiles}
                       COMMAND sed -e "s/charset=CHARSET/charset=UTF-8/g" -i~ "${potFile}"
                       WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                       DEPENDS ${potInputFilesForDependency} "${source}"
                       VERBATIM)
   SET(generate_PotFile "generate_pot_${textdomain}")
   ADD_CUSTOM_TARGET(${generate_PotFile} ALL DEPENDS "${potFile}")


   # ====== Process PO files (translations) =================================
   FILE(GLOB_RECURSE poFiles "*/${textdomain}.po")
   FOREACH(poFile IN LISTS poFiles)

      # ====== Update PO file (translation) =================================
      GET_FILENAME_COMPONENT(languageDirectory "${poFile}" DIRECTORY)
      GET_FILENAME_COMPONENT(language "${languageDirectory}" NAME "${CMAKE_CURRENT_SOURCE_DIR}")
      SET(poStampFile "${CMAKE_CURRENT_BINARY_DIR}/${language}/.${textdomain}.po.stamp")

      MESSAGE("Translation update: ${textdomain}, ${language} (${potFile} -> ${poFile})")
      # The .po file cannot be OUTPUT, since "make clean" would then delete it
      # => Using a stamp file as placeholder to ensure the dependency.
      ADD_CUSTOM_COMMAND(OUTPUT "${poStampFile}"
                         COMMAND "${MSGMERGE}"
                            --update "${poFile}" "${potFile}"
                         COMMAND touch "${poStampFile}"
                         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                         DEPENDS ${generate_PotFile} "${potFile}")
      SET(generate_PoFile "generate_po_${textdomain}_${language}")
      ADD_CUSTOM_TARGET(${generate_PoFile} DEPENDS "${poStampFile}")


      # ====== Compile PO file to MO file ===================================
      FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${language}/LC_MESSAGES")
      SET(moFile "${CMAKE_CURRENT_BINARY_DIR}/${language}/LC_MESSAGES/${textdomain}.mo")

      MESSAGE("Compiling translation: ${textdomain}, ${language} (${poFile} -> ${moFile})")
      ADD_CUSTOM_COMMAND(OUTPUT "${moFile}"
                         COMMAND "${MSGFMT}"
                            --statistics
                            --check
                            --output-file="${moFile}"
                            ${poFile}
                         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                         DEPENDS ${generate_PoFile} "${poFile}")
      SET(generate_MoFile "generate_mo_${textdomain}_${language}")
      ADD_CUSTOM_TARGET(${generate_MoFile} ALL DEPENDS "${moFile}")


      # ====== Install MO file ==============================================
      INSTALL(FILES "${moFile}"
              DESTINATION "${CMAKE_INSTALL_DATADIR}/locale/${language}/LC_MESSAGES")

   ENDFOREACH()
ENDFOREACH()
