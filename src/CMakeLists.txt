#############################################################################
# PROGRAMS
#############################################################################

ADD_EXECUTABLE(subnetcalc subnetcalc.cc tools.cc)
TARGET_LINK_LIBRARIES(subnetcalc ${Intl_LIBRARIES})
IF (GEOIP_FOUND)
   TARGET_LINK_LIBRARIES(subnetcalc ${GeoIP_LIBRARY})
ENDIF()
INSTALL(TARGETS subnetcalc   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES   subnetcalc.1 DESTINATION         ${CMAKE_INSTALL_MANDIR}/man1)
INSTALL(FILES       subnetcalc.bash-completion
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
        RENAME      subnetcalc)
