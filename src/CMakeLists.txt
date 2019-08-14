#############################################################################
# PROGRAMS
#############################################################################

ADD_EXECUTABLE(subnetcalc subnetcalc.cc tools.cc)
IF (GEOIP_FOUND)
   TARGET_LINK_LIBRARIES(subnetcalc ${GEOIP_LIBRARY})
ENDIF()
INSTALL(TARGETS subnetcalc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES subnetcalc.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
