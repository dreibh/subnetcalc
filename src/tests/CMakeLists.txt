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

SET(TEST_LOCALE_DIR "${CMAKE_BINARY_DIR}/po")


# ###### Helper for deterministic commands (expected return code: 0) #######
FUNCTION(ADD_SUBNETCALC_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET(TEST_LABELS "cli")
   FOREACH(ARG ${ARGN})
      IF(ARG MATCHES "^-[0-9a-zA-Z]*6" OR ARG STREQUAL "--ipv6")
         LIST(APPEND TEST_LABELS "ipv6")
      ELSEIF(ARG MATCHES "^-[0-9a-zA-Z]*4" OR ARG STREQUAL "--ipv4")
         LIST(APPEND TEST_LABELS "ipv4")
      ENDIF()
   ENDFOREACH()
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      ENVIRONMENT "LC_ALL=C.UTF-8;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
      FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
      LABELS "${TEST_LABELS}"
   )
ENDFUNCTION()


# ###### Helper for network/DNS lookups (Skips test if offline/unresolved) #
FUNCTION(ADD_SUBNETCALC_DNS_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET(TEST_LABELS "network;dns")
   FOREACH(ARG ${ARGN})
      IF(ARG MATCHES "^-[0-9a-zA-Z]*6" OR ARG STREQUAL "--ipv6")
         LIST(APPEND TEST_LABELS "ipv6")
      ELSEIF(ARG MATCHES "^-[0-9a-zA-Z]*4" OR ARG STREQUAL "--ipv4")
         LIST(APPEND TEST_LABELS "ipv4")
      ENDIF()
   ENDFOREACH()
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      ENVIRONMENT "LC_ALL=C.UTF-8;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
      PASS_REGULAR_EXPRESSION "Address[ ]+= "
      FAIL_REGULAR_EXPRESSION "INTERNAL ERROR"
      SKIP_REGULAR_EXPRESSION "ERROR: Invalid address"
      LABELS "${TEST_LABELS}"
   )
ENDFUNCTION()


# ###### Helper for expected failures (expected return code: non-zero) ######
FUNCTION(ADD_SUBNETCALC_FAIL_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET(TEST_LABELS "negative;error")
   FOREACH(ARG ${ARGN})
      IF(ARG MATCHES "^-[0-9a-zA-Z]*6" OR ARG STREQUAL "--ipv6")
         LIST(APPEND TEST_LABELS "ipv6")
      ELSEIF(ARG MATCHES "^-[0-9a-zA-Z]*4" OR ARG STREQUAL "--ipv4")
         LIST(APPEND TEST_LABELS "ipv4")
      ENDIF()
   ENDFOREACH()
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      WILL_FAIL TRUE
      ENVIRONMENT "LC_ALL=C.UTF-8;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
      LABELS "${TEST_LABELS}"
   )
ENDFUNCTION()


# ###### Matrix Test Helper (Runs each test in text, json, yaml, xml) #######
FUNCTION(ADD_SUBNETCALC_MATRIX_TEST)
   SET(oneValueArgs
      NAME
      ADDRESS
      EXPANDED_ADDRESS
      FAMILY
      NETWORK
      PREFIX
      NETMASK
      BROADCAST
      WILDCARD
      HEX
      HOST_BITS
      MAX_HOSTS
      HOST_MIN
      HOST_MAX
      TYPE
      CLASS
      FLAG
      FLAG2
      SPECIAL
      SCOPE
      MULTICAST_MAC
      UNICAST_TYPE
      ULA_TYPE
      GLOBAL_ID
      SUBNET_ID
      INTERFACE_ID
      MAC_ADDRESS
      SNMC
      SIX_TO_FOUR
      TRANSIENT
      SSM
      SOLICITED_NODE_FOR
   )
   SET(multiValueArgs ARGS)
   CMAKE_PARSE_ARGUMENTS(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   # Escape parentheses in class names (e.g. "Class D (Multicast)")
   IF(ARG_CLASS)
      STRING(REPLACE "(" "\\(" ESC_CLASS "${ARG_CLASS}")
      STRING(REPLACE ")" "\\)" ESC_CLASS "${ESC_CLASS}")
   ENDIF()

   # ====== 1. Build Text Format Regex Pattern ==============================
   SET(T_PARTS "Address[ ]+= ${ARG_ADDRESS}")
   IF(ARG_EXPANDED_ADDRESS)
      STRING(APPEND T_PARTS ".*Exp\\. Address[ ]+= ${ARG_EXPANDED_ADDRESS}")
   ENDIF()
   STRING(APPEND T_PARTS ".*Network[ ]+= ${ARG_NETWORK} / ${ARG_PREFIX}")
   STRING(APPEND T_PARTS ".*Netmask[ ]+= ${ARG_NETMASK}")
   IF(ARG_BROADCAST)
      STRING(APPEND T_PARTS ".*Broadcast[ ]+= ${ARG_BROADCAST}")
   ENDIF()
   STRING(APPEND T_PARTS ".*Wildcard Mask[ ]+= ${ARG_WILDCARD}")
   IF(ARG_HEX)
      STRING(APPEND T_PARTS ".*Hex\\. Address[ ]+= ${ARG_HEX}")
   ENDIF()
   STRING(APPEND T_PARTS ".*Host Bits[ ]+= ${ARG_HOST_BITS}")
   IF(DEFINED ARG_MAX_HOSTS)
      IF(HAVE_INT128 OR ARG_HOST_BITS LESS 64)
         STRING(APPEND T_PARTS ".*Max\\. Hosts[ ]+= ${ARG_MAX_HOSTS}[ ]+\\(2\\^${ARG_HOST_BITS} - ")
      ELSE()
         STRING(APPEND T_PARTS ".*Max\\. Hosts[ ]+= 2\\^${ARG_HOST_BITS}")
      ENDIF()
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND T_PARTS ".*Host Range[ ]+= \\{[ ]*${ARG_HOST_MIN} - ${ARG_HOST_MAX}[ ]*\\}")
   ENDIF()
   IF(ARG_CLASS)
      STRING(APPEND T_PARTS ".*${ESC_CLASS}")
   ENDIF()
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND T_PARTS ".*${ARG_UNICAST_TYPE}")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND T_PARTS ".*${ARG_ULA_TYPE}")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND T_PARTS ".*${ARG_FLAG}")
   ENDIF()
   IF(ARG_FLAG2)
      STRING(APPEND T_PARTS ".*${ARG_FLAG2}")
   ENDIF()
   IF(ARG_SPECIAL)
      STRING(APPEND T_PARTS ".*${ARG_SPECIAL}")
   ENDIF()
   IF(ARG_SCOPE)
      STRING(APPEND T_PARTS ".*Scope:[ ]*${ARG_SCOPE}")
   ENDIF()
   IF(ARG_TRANSIENT)
      STRING(APPEND T_PARTS ".*Temporarily-allocated address")
   ENDIF()
   IF(ARG_MULTICAST_MAC)
      STRING(APPEND T_PARTS ".*Corresponding multicast MAC address:[ ]*${ARG_MULTICAST_MAC}")
   ENDIF()
   IF(ARG_SSM)
      STRING(APPEND T_PARTS ".*Source-specific multicast")
   ENDIF()
   IF(ARG_SOLICITED_NODE_FOR)
      STRING(APPEND T_PARTS ".*Address is solicited node multicast address for ${ARG_SOLICITED_NODE_FOR}")
   ENDIF()
   IF(ARG_GLOBAL_ID)
      STRING(APPEND T_PARTS ".*Global ID[ ]+= ${ARG_GLOBAL_ID}")
   ENDIF()
   IF(ARG_SUBNET_ID)
      STRING(APPEND T_PARTS ".*Subnet ID[ ]+= ${ARG_SUBNET_ID}")
   ENDIF()
   IF(ARG_INTERFACE_ID)
      STRING(APPEND T_PARTS ".*Interface ID[ ]+= .*${ARG_INTERFACE_ID}")
   ENDIF()
   IF(ARG_MAC_ADDRESS)
      STRING(APPEND T_PARTS ".*MAC Address[ ]+= .*${ARG_MAC_ADDRESS}")
   ENDIF()
   IF(ARG_SNMC)
      STRING(APPEND T_PARTS ".*Solicited Node Multicast Address[ ]+= .*${ARG_SNMC}")
   ENDIF()
   IF(ARG_SIX_TO_FOUR)
      STRING(APPEND T_PARTS ".*6-to-4 Address[ ]+= ${ARG_SIX_TO_FOUR}")
   ENDIF()

   # ====== 2. Build JSON Format Regex Pattern ==============================
   SET(J_PARTS "\"address\":[ ]*\"${ARG_ADDRESS}\"")
   IF(ARG_EXPANDED_ADDRESS)
      STRING(APPEND J_PARTS ".*\"expanded_address\":[ ]*\"${ARG_EXPANDED_ADDRESS}\"")
   ENDIF()
   STRING(APPEND J_PARTS ".*\"family\":[ ]*\"${ARG_FAMILY}\"")
   STRING(APPEND J_PARTS ".*\"network\":[ ]*\"${ARG_NETWORK}\"")
   STRING(APPEND J_PARTS ".*\"prefix\":[ ]*${ARG_PREFIX}")
   STRING(APPEND J_PARTS ".*\"netmask\":[ ]*\"${ARG_NETMASK}\"")
   IF(ARG_BROADCAST)
      STRING(APPEND J_PARTS ".*\"broadcast\":[ ]*\"${ARG_BROADCAST}\"")
   ENDIF()
   STRING(APPEND J_PARTS ".*\"wildcard\":[ ]*\"${ARG_WILDCARD}\"")
   IF(ARG_HEX)
      STRING(APPEND J_PARTS ".*\"hex\":[ ]*\"${ARG_HEX}\"")
   ENDIF()
   STRING(APPEND J_PARTS ".*\"host_bits\":[ ]*${ARG_HOST_BITS}")
   IF(DEFINED ARG_MAX_HOSTS)
      IF(HAVE_INT128 OR ARG_HOST_BITS LESS 64)
         STRING(APPEND J_PARTS ".*\"max_hosts\":[ ]*${ARG_MAX_HOSTS}")
      ENDIF()
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND J_PARTS ".*\"min\":[ ]*\"${ARG_HOST_MIN}\".*\"max\":[ ]*\"${ARG_HOST_MAX}\"")
   ENDIF()
   STRING(APPEND J_PARTS ".*\"type\":[ ]*\"${ARG_TYPE}\"")
   IF(ARG_CLASS)
      STRING(APPEND J_PARTS ".*\"class\":[ ]*\"${ESC_CLASS}\"")
   ENDIF()
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND J_PARTS ".*\"unicast_type\":[ ]*\"${ARG_UNICAST_TYPE}\"")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND J_PARTS ".*\"ula_type\":[ ]*\"${ARG_ULA_TYPE}\"")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND J_PARTS ".*\"${ARG_FLAG}\"")
   ENDIF()
   IF(ARG_FLAG2)
      STRING(APPEND J_PARTS ".*\"${ARG_FLAG2}\"")
   ENDIF()
   IF(ARG_SPECIAL)
      STRING(APPEND J_PARTS ".*\"special\":[ ]*\"${ARG_SPECIAL}\"")
   ENDIF()
   IF(ARG_SCOPE)
      STRING(APPEND J_PARTS ".*\"scope\":[ ]*\"${ARG_SCOPE}\"")
   ENDIF()
   IF(ARG_MULTICAST_MAC)
      STRING(APPEND J_PARTS ".*\"multicast_mac\":[ ]*\"${ARG_MULTICAST_MAC}\"")
   ENDIF()
   IF(ARG_SSM)
      STRING(APPEND J_PARTS ".*\"source_specific_multicast\":[ ]*true")
   ENDIF()
   IF(ARG_TRANSIENT)
      STRING(APPEND J_PARTS ".*\"transient\":[ ]*true")
   ENDIF()
   IF(ARG_SOLICITED_NODE_FOR)
      STRING(APPEND J_PARTS ".*\"solicited_node_for\":[ ]*\"${ARG_SOLICITED_NODE_FOR}\"")
   ENDIF()
   IF(ARG_GLOBAL_ID)
      STRING(APPEND J_PARTS ".*\"global_id\":[ ]*\"${ARG_GLOBAL_ID}\"")
   ENDIF()
   IF(ARG_SUBNET_ID)
      STRING(APPEND J_PARTS ".*\"subnet_id\":[ ]*\"${ARG_SUBNET_ID}\"")
   ENDIF()
   IF(ARG_INTERFACE_ID)
      STRING(APPEND J_PARTS ".*\"interface_id\":[ ]*\"${ARG_INTERFACE_ID}\"")
   ENDIF()
   IF(ARG_MAC_ADDRESS)
      STRING(APPEND J_PARTS ".*\"mac_address\":[ ]*\"${ARG_MAC_ADDRESS}\"")
   ENDIF()
   IF(ARG_SNMC)
      STRING(APPEND J_PARTS ".*\"solicited_node_multicast\":[ ]*\"${ARG_SNMC}\"")
   ENDIF()
   IF(ARG_SIX_TO_FOUR)
      STRING(APPEND J_PARTS ".*\"six_to_four\":[ ]*\"${ARG_SIX_TO_FOUR}\"")
   ENDIF()

   # ====== 3. Build YAML Format Regex Pattern ==============================
   SET(Y_PARTS "address:[ ]*\"${ARG_ADDRESS}\"")
   IF(ARG_EXPANDED_ADDRESS)
      STRING(APPEND Y_PARTS ".*expanded_address:[ ]*\"${ARG_EXPANDED_ADDRESS}\"")
   ENDIF()
   STRING(APPEND Y_PARTS ".*family:[ ]*\"${ARG_FAMILY}\"")
   STRING(APPEND Y_PARTS ".*network:[ ]*\"${ARG_NETWORK}\"")
   STRING(APPEND Y_PARTS ".*prefix:[ ]*${ARG_PREFIX}")
   STRING(APPEND Y_PARTS ".*netmask:[ ]*\"${ARG_NETMASK}\"")
   IF(ARG_BROADCAST)
      STRING(APPEND Y_PARTS ".*broadcast:[ ]*\"${ARG_BROADCAST}\"")
   ENDIF()
   STRING(APPEND Y_PARTS ".*wildcard:[ ]*\"${ARG_WILDCARD}\"")
   IF(ARG_HEX)
      STRING(APPEND Y_PARTS ".*hex:[ ]*\"${ARG_HEX}\"")
   ENDIF()
   STRING(APPEND Y_PARTS ".*host_bits:[ ]*${ARG_HOST_BITS}")
   IF(DEFINED ARG_MAX_HOSTS)
      IF(HAVE_INT128 OR ARG_HOST_BITS LESS 64)
         STRING(APPEND Y_PARTS ".*max_hosts:[ ]*${ARG_MAX_HOSTS}")
      ENDIF()
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND Y_PARTS ".*min:[ ]*\"${ARG_HOST_MIN}\".*max:[ ]*\"${ARG_HOST_MAX}\"")
   ENDIF()
   STRING(APPEND Y_PARTS ".*type:[ ]*\"${ARG_TYPE}\"")
   IF(ARG_CLASS)
      STRING(APPEND Y_PARTS ".*class:[ ]*\"${ESC_CLASS}\"")
   ENDIF()
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND Y_PARTS ".*unicast_type:[ ]*\"${ARG_UNICAST_TYPE}\"")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND Y_PARTS ".*ula_type:[ ]*\"${ARG_ULA_TYPE}\"")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND Y_PARTS ".*-[ ]*\"${ARG_FLAG}\"")
   ENDIF()
   IF(ARG_FLAG2)
      STRING(APPEND Y_PARTS ".*-[ ]*\"${ARG_FLAG2}\"")
   ENDIF()
   IF(ARG_SPECIAL)
      STRING(APPEND Y_PARTS ".*special:[ ]*\"${ARG_SPECIAL}\"")
   ENDIF()
   IF(ARG_SCOPE)
      STRING(APPEND Y_PARTS ".*scope:[ ]*\"${ARG_SCOPE}\"")
   ENDIF()
   IF(ARG_MULTICAST_MAC)
      STRING(APPEND Y_PARTS ".*multicast_mac:[ ]*\"${ARG_MULTICAST_MAC}\"")
   ENDIF()
   IF(ARG_SSM)
      STRING(APPEND Y_PARTS ".*source_specific_multicast:[ ]*true")
   ENDIF()
   IF(ARG_TRANSIENT)
      STRING(APPEND Y_PARTS ".*transient:[ ]*true")
   ENDIF()
   IF(ARG_SOLICITED_NODE_FOR)
      STRING(APPEND Y_PARTS ".*solicited_node_for:[ ]*\"${ARG_SOLICITED_NODE_FOR}\"")
   ENDIF()
   IF(ARG_GLOBAL_ID)
      STRING(APPEND Y_PARTS ".*global_id:[ ]*\"${ARG_GLOBAL_ID}\"")
   ENDIF()
   IF(ARG_SUBNET_ID)
      STRING(APPEND Y_PARTS ".*subnet_id:[ ]*\"${ARG_SUBNET_ID}\"")
   ENDIF()
   IF(ARG_INTERFACE_ID)
      STRING(APPEND Y_PARTS ".*interface_id:[ ]*\"${ARG_INTERFACE_ID}\"")
   ENDIF()
   IF(ARG_MAC_ADDRESS)
      STRING(APPEND Y_PARTS ".*mac_address:[ ]*\"${ARG_MAC_ADDRESS}\"")
   ENDIF()
   IF(ARG_SNMC)
      STRING(APPEND Y_PARTS ".*solicited_node_multicast:[ ]*\"${ARG_SNMC}\"")
   ENDIF()
   IF(ARG_SIX_TO_FOUR)
      STRING(APPEND Y_PARTS ".*six_to_four:[ ]*\"${ARG_SIX_TO_FOUR}\"")
   ENDIF()

   # ====== 4. Build XML Format Regex Pattern ===============================
   SET(X_PARTS "<address>${ARG_ADDRESS}</address>")
   IF(ARG_EXPANDED_ADDRESS)
      STRING(APPEND X_PARTS ".*<expanded_address>${ARG_EXPANDED_ADDRESS}</expanded_address>")
   ENDIF()
   STRING(APPEND X_PARTS ".*<family>${ARG_FAMILY}</family>")
   STRING(APPEND X_PARTS ".*<network>${ARG_NETWORK}</network>")
   STRING(APPEND X_PARTS ".*<prefix>${ARG_PREFIX}</prefix>")
   STRING(APPEND X_PARTS ".*<netmask>${ARG_NETMASK}</netmask>")
   IF(ARG_BROADCAST)
      STRING(APPEND X_PARTS ".*<broadcast>${ARG_BROADCAST}</broadcast>")
   ENDIF()
   STRING(APPEND X_PARTS ".*<wildcard>${ARG_WILDCARD}</wildcard>")
   IF(ARG_HEX)
      STRING(APPEND X_PARTS ".*<hex>${ARG_HEX}</hex>")
   ENDIF()
   STRING(APPEND X_PARTS ".*<host_bits>${ARG_HOST_BITS}</host_bits>")
   IF(DEFINED ARG_MAX_HOSTS)
      IF(HAVE_INT128 OR ARG_HOST_BITS LESS 64)
         STRING(APPEND X_PARTS ".*<max_hosts>${ARG_MAX_HOSTS}</max_hosts>")
      ENDIF()
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND X_PARTS ".*<min>${ARG_HOST_MIN}</min>.*<max>${ARG_HOST_MAX}</max>")
   ENDIF()
   STRING(APPEND X_PARTS ".*<type>${ARG_TYPE}</type>")
   IF(ARG_CLASS)
      STRING(APPEND X_PARTS ".*<class>${ESC_CLASS}</class>")
   ENDIF()
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND X_PARTS ".*<unicast_type>${ARG_UNICAST_TYPE}</unicast_type>")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND X_PARTS ".*<ula_type>${ARG_ULA_TYPE}</ula_type>")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND X_PARTS ".*<flag>${ARG_FLAG}</flag>")
   ENDIF()
   IF(ARG_FLAG2)
      STRING(APPEND X_PARTS ".*<flag>${ARG_FLAG2}</flag>")
   ENDIF()
   IF(ARG_SPECIAL)
      STRING(APPEND X_PARTS ".*<special>${ARG_SPECIAL}</special>")
   ENDIF()
   IF(ARG_SCOPE)
      STRING(APPEND X_PARTS ".*<scope>${ARG_SCOPE}</scope>")
   ENDIF()
   IF(ARG_MULTICAST_MAC)
      STRING(APPEND X_PARTS ".*<multicast_mac>${ARG_MULTICAST_MAC}</multicast_mac>")
   ENDIF()
   IF(ARG_SSM)
      STRING(APPEND X_PARTS ".*<source_specific_multicast>true</source_specific_multicast>")
   ENDIF()
   IF(ARG_TRANSIENT)
      STRING(APPEND X_PARTS ".*<transient>true</transient>")
   ENDIF()
   IF(ARG_SOLICITED_NODE_FOR)
      STRING(APPEND X_PARTS ".*<solicited_node_for>${ARG_SOLICITED_NODE_FOR}</solicited_node_for>")
   ENDIF()
   IF(ARG_GLOBAL_ID)
      STRING(APPEND X_PARTS ".*<global_id>${ARG_GLOBAL_ID}</global_id>")
   ENDIF()
   IF(ARG_SUBNET_ID)
      STRING(APPEND X_PARTS ".*<subnet_id>${ARG_SUBNET_ID}</subnet_id>")
   ENDIF()
   IF(ARG_INTERFACE_ID)
      STRING(APPEND X_PARTS ".*<interface_id>${ARG_INTERFACE_ID}</interface_id>")
   ENDIF()
   IF(ARG_MAC_ADDRESS)
      STRING(APPEND X_PARTS ".*<mac_address>${ARG_MAC_ADDRESS}</mac_address>")
   ENDIF()
   IF(ARG_SNMC)
      STRING(APPEND X_PARTS ".*<solicited_node_multicast>${ARG_SNMC}</solicited_node_multicast>")
   ENDIF()
   IF(ARG_SIX_TO_FOUR)
      STRING(APPEND X_PARTS ".*<six_to_four>${ARG_SIX_TO_FOUR}</six_to_four>")
   ENDIF()

   # ====== Register 4 Tests per Matrix Item ================================
   FOREACH(FMT text json yaml xml)
      SET(TEST_NAME "${ARG_NAME}_${FMT}")
      ADD_TEST(
         NAME ${TEST_NAME}
         COMMAND $<TARGET_FILE:subnetcalc> ${ARG_ARGS} -F ${FMT}
      )

      IF(FMT STREQUAL "text")
         SET(MATCH_EXPR "${T_PARTS}")
      ELSEIF(FMT STREQUAL "json")
         SET(MATCH_EXPR "${J_PARTS}")
      ELSEIF(FMT STREQUAL "yaml")
         SET(MATCH_EXPR "${Y_PARTS}")
      ELSEIF(FMT STREQUAL "xml")
         SET(MATCH_EXPR "${X_PARTS}")
      ENDIF()

      SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
         ENVIRONMENT "LC_ALL=C.UTF-8;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
         PASS_REGULAR_EXPRESSION "${MATCH_EXPR}"
         FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>"
         LABELS "matrix;format_${FMT}"
      )
   ENDFOREACH()
ENDFUNCTION()


#############################################################################
#### BASIC CLI OPTIONS                                                   ####
#############################################################################

ADD_SUBNETCALC_TEST(cli_version_short -v)
ADD_SUBNETCALC_TEST(cli_version_long  --version)
ADD_SUBNETCALC_TEST(cli_help_short    -h)
ADD_SUBNETCALC_TEST(cli_help_long     --help)

# Localization catalog verification tests
ADD_SUBNETCALC_TEST(cli_help_i18n_de  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_nb  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_ar  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_he  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_mn  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_lt  -h)

SET_TESTS_PROPERTIES(cli_version_short cli_version_long PROPERTIES
   PASS_REGULAR_EXPRESSION "SubNetCalc [0-9]+\\.[0-9]+\\.[0-9]+"
)

# Flexible check for default Usage output (exit code 0 validated by ADD_SUBNETCALC_TEST)
SET_TESTS_PROPERTIES(cli_help_short cli_help_long PROPERTIES
   PASS_REGULAR_EXPRESSION "Usage:"
)

# German catalog translation ("Aufruf:")
SET_TESTS_PROPERTIES(cli_help_i18n_de PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=de;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Aufruf:"
   LABELS "cli;i18n;locale_de"
)

# Norwegian Bokmål catalog translation ("Bruk:")
SET_TESTS_PROPERTIES(cli_help_i18n_nb PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=nb;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Bruk:"
   LABELS "cli;i18n;locale_nb"
)

# Arabic catalog translation ("الاستخدام:")
SET_TESTS_PROPERTIES(cli_help_i18n_ar PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=ar;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "الاستخدام:"
   LABELS "cli;i18n;locale_ar;rtl"
)

# Hebrew catalog translation ("שימוש:")
SET_TESTS_PROPERTIES(cli_help_i18n_he PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=he;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "שימוש:"
   LABELS "cli;i18n;locale_he;rtl"
)

# Mongolian catalog translation ("Хэрэглээ:")
SET_TESTS_PROPERTIES(cli_help_i18n_mn PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=mn;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Хэрэглээ:"
   LABELS "cli;i18n;locale_mn"
)

# Lithuanian catalog translation ("Naudojimas:")
SET_TESTS_PROPERTIES(cli_help_i18n_lt PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=lt;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Naudojimas:"
   LABELS "cli;i18n;locale_lt"
)


#############################################################################
#### LOCALIZATION & BIDIRECTIONAL (RTL) TESTS                            ####
#############################################################################

# --- Arabic IPv4 Functional Calculation ------------------------------------
ADD_SUBNETCALC_TEST(i18n_ar_ipv4_calc 192.168.0.255/24 -c -n -g)
SET_TESTS_PROPERTIES(i18n_ar_ipv4_calc PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=ar;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "العنوان.*=.*192\\.168\\.0\\.255.*الشبكة.*=.*192\\.168\\.0\\.0 / 24.*قناع الشبكة.*=.*255\\.255\\.255\\.0.*البث العام.*=.*192\\.168\\.0\\.255.*قناع البدل.*=.*0\\.0\\.0\\.255.*العنوان بالست عشري.*=.*C0A800FF.*بتات المضيف.*=.*8.*أقصى عدد للمضيفين.*=.*254.*نطاق المضيفين.*=.*\\{ 192\\.168\\.0\\.1 - 192\\.168\\.0\\.254 \\}.*الخصائص.*=.*192\\.168\\.0\\.255.*هو عنوان البث العام.*192\\.168\\.0\\.0/24.*الفئة C.*خاص"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
   LABELS "i18n;locale_ar;rtl;ipv4"
)

# --- Arabic IPv6 ULA Sub-properties Functional Test ------------------------
ADD_SUBNETCALC_TEST(i18n_ar_ipv6_ula fc00:1:3:4::/64 -c -n -g)
SET_TESTS_PROPERTIES(i18n_ar_ipv6_ula PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=ar;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "العنوان.*=.*fc00:1:3:4::.*العنوان الموسّع.*=.*fc00:0001:0003:0004:0000:0000:0000:0000.*الشبكة.*=.*fc00:1:3:4:: / 64.*قناع الشبكة.*=.*ffff:ffff:ffff:ffff::.*خصائص البث الفردي المحلي الفريد.*مُعيَّن من قِبل هيئة عالمية.*المُعرّف العالمي.*=.*0000010003.*مُعرّف الشبكة الفرعية.*=.*0004.*مُعرّف الواجهة.*=.*0000:0000:0000:0000"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
   LABELS "i18n;locale_ar;rtl;ipv6;ula"
)

# --- Hebrew IPv4 Functional Calculation ------------------------------------
ADD_SUBNETCALC_TEST(i18n_he_ipv4_calc 192.168.0.255/24 -c -n -g)
SET_TESTS_PROPERTIES(i18n_he_ipv4_calc PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=he;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "כתובת.*=.*192\\.168\\.0\\.255.*רשת.*=.*192\\.168\\.0\\.0 / 24.*מסכת רשת.*=.*255\\.255\\.255\\.0.*שידור.*=.*192\\.168\\.0\\.255.*מסכת Wildcard.*=.*0\\.0\\.0\\.255.*כתובת הקס.*=.*C0A800FF.*סיביות מארח.*=.*8.*מקס.*מארחים.*=.*254.*טווח מארחים.*=.*\\{ 192\\.168\\.0\\.1 - 192\\.168\\.0\\.254 \\}.*מאפיינים.*=.*192\\.168\\.0\\.255 היא כתובת BROADCAST של 192\\.168\\.0\\.0/24.*מחלקה C.*פרטית"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
   LABELS "i18n;locale_he;rtl;ipv4"
)

# --- Hebrew IPv6 ULA Sub-properties Functional Test ------------------------
ADD_SUBNETCALC_TEST(i18n_he_ipv6_ula fc00:1:3:4::/64 -c -n -g)
SET_TESTS_PROPERTIES(i18n_he_ipv6_ula PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=he;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "כתובת.*=.*fc00:1:3:4::.*כתובת מורחבת.*=.*fc00:0001:0003:0004:0000:0000:0000:0000.*רשת.*=.*fc00:1:3:4:: / 64.*מסכת רשת.*=.*ffff:ffff:ffff:ffff::.*מאפייני Unique Local Unicast:.*הוקצה על ידי מופע גלובלי.*מזהה גלובלי.*=.*0000010003.*מזהה תת.*רשת.*=.*0004.*מזהה ממשק.*=.*0000:0000:0000:0000"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
   LABELS "i18n;locale_he;rtl;ipv6;ula"
)


ADD_SUBNETCALC_TEST(cli_help_i18n_da  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_sv  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_nn  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_ru  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_ja  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_zh  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_sk  -h)
ADD_SUBNETCALC_TEST(cli_help_i18n_bs  -h)

SET_TESTS_PROPERTIES(cli_help_i18n_da PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=da;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Brug:"
   LABELS "cli;i18n;locale_da"
)

SET_TESTS_PROPERTIES(cli_help_i18n_sv PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=sv;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Användning:"
   LABELS "cli;i18n;locale_sv"
)

SET_TESTS_PROPERTIES(cli_help_i18n_nn PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=nn;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Bruk:"
   LABELS "cli;i18n;locale_nn"
)

SET_TESTS_PROPERTIES(cli_help_i18n_ru PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=ru;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Использование:"
   LABELS "cli;i18n;locale_ru"
)

SET_TESTS_PROPERTIES(cli_help_i18n_ja PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=ja;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "使用法:"
   LABELS "cli;i18n;locale_ja"
)

SET_TESTS_PROPERTIES(cli_help_i18n_zh PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=zh_CN;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "用法："
   LABELS "cli;i18n;locale_zh"
)

SET_TESTS_PROPERTIES(cli_help_i18n_sk PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=sk;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Použitie:"
   LABELS "cli;i18n;locale_sk"
)

SET_TESTS_PROPERTIES(cli_help_i18n_bs PROPERTIES
   ENVIRONMENT "LC_ALL=en_US.UTF-8;LANGUAGE=bs;TEXTDOMAINDIR=${TEST_LOCALE_DIR}"
   PASS_REGULAR_EXPRESSION "Upotreba:"
   LABELS "cli;i18n;locale_bs"
)


#############################################################################
#### IPV4 MATRIX TESTS                                                   ####
#############################################################################

# --- Private & Public Boundary Subnets -------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_c_private
   ARGS                 192.168.1.1 24 -c -n -g
   ADDRESS              192.168.1.1
   FAMILY               IPv4
   NETWORK              192.168.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.168.1.255
   WILDCARD             0.0.0.255
   HEX                  C0A80101
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.168.1.1
   HOST_MAX             192.168.1.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_c_private_boundary_low
   ARGS                 192.167.255.255 24 -c -n -g
   ADDRESS              192.167.255.255
   FAMILY               IPv4
   NETWORK              192.167.255.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.167.255.255
   WILDCARD             0.0.0.255
   HEX                  C0A7FFFF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.167.255.1
   HOST_MAX             192.167.255.254
   TYPE                 broadcast
   CLASS                "Class C"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_c_private_boundary_high
   ARGS                 192.169.0.1 24 -c -n -g
   ADDRESS              192.169.0.1
   FAMILY               IPv4
   NETWORK              192.169.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.169.0.255
   WILDCARD             0.0.0.255
   HEX                  C0A90001
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.169.0.1
   HOST_MAX             192.169.0.254
   TYPE                 host
   CLASS                "Class C"
)

SET_TESTS_PROPERTIES(
   ipv4_c_private_boundary_low_text ipv4_c_private_boundary_low_json
   ipv4_c_private_boundary_low_yaml ipv4_c_private_boundary_low_xml
   ipv4_c_private_boundary_high_text ipv4_c_private_boundary_high_json
   ipv4_c_private_boundary_high_yaml ipv4_c_private_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_b_private
   ARGS                 172.16.0.1 12 -c -n -g
   ADDRESS              172.16.0.1
   FAMILY               IPv4
   NETWORK              172.16.0.0
   PREFIX               12
   NETMASK              255.240.0.0
   BROADCAST            172.31.255.255
   WILDCARD             0.15.255.255
   HEX                  AC100001
   HOST_BITS            20
   MAX_HOSTS            1048574
   HOST_MIN             172.16.0.1
   HOST_MAX             172.31.255.254
   TYPE                 host
   CLASS                "Class B"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_b_private_boundary_low
   ARGS                 172.15.255.255 16 -c -n -g
   ADDRESS              172.15.255.255
   FAMILY               IPv4
   NETWORK              172.15.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            172.15.255.255
   WILDCARD             0.0.255.255
   HEX                  AC0FFFFF
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             172.15.0.1
   HOST_MAX             172.15.255.254
   TYPE                 broadcast
   CLASS                "Class B"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_b_private_boundary_high_12
   ARGS                 172.32.0.0 12 -c -n -g
   ADDRESS              172.32.0.0
   FAMILY               IPv4
   NETWORK              172.32.0.0
   PREFIX               12
   NETMASK              255.240.0.0
   BROADCAST            172.47.255.255
   WILDCARD             0.15.255.255
   HEX                  AC200000
   HOST_BITS            20
   MAX_HOSTS            1048574
   HOST_MIN             172.32.0.1
   HOST_MAX             172.47.255.254
   TYPE                 network
   CLASS                "Class B"
)

SET_TESTS_PROPERTIES(
   ipv4_b_private_boundary_low_text ipv4_b_private_boundary_low_json
   ipv4_b_private_boundary_low_yaml ipv4_b_private_boundary_low_xml
   ipv4_b_private_boundary_high_12_text ipv4_b_private_boundary_high_12_json
   ipv4_b_private_boundary_high_12_yaml ipv4_b_private_boundary_high_12_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_b_public_boundary
   ARGS                 172.32.0.1 16 -c -n -g
   ADDRESS              172.32.0.1
   FAMILY               IPv4
   NETWORK              172.32.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            172.32.255.255
   WILDCARD             0.0.255.255
   HEX                  AC200001
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             172.32.0.1
   HOST_MAX             172.32.255.254
   TYPE                 host
   CLASS                "Class B"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_a_private
   ARGS                 10.0.0.1 8 -c -n -g
   ADDRESS              10.0.0.1
   FAMILY               IPv4
   NETWORK              10.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            10.255.255.255
   WILDCARD             0.255.255.255
   HEX                  0A000001
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             10.0.0.1
   HOST_MAX             10.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_a_prefix_0
   ARGS                 10.1.1.1/0 -c -n -g
   ADDRESS              10.1.1.1
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               0
   NETMASK              0.0.0.0
   BROADCAST            255.255.255.255
   WILDCARD             255.255.255.255
   HEX                  0A010101
   HOST_BITS            32
   MAX_HOSTS            4294967294
   HOST_MIN             0.0.0.1
   HOST_MAX             255.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

# --- Formatting Syntaxes ---------------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_slash_notation_24
   ARGS                 10.1.1.1/24 -c -n -g
   ADDRESS              10.1.1.1
   FAMILY               IPv4
   NETWORK              10.1.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            10.1.1.255
   WILDCARD             0.0.0.255
   HEX                  0A010101
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             10.1.1.1
   HOST_MAX             10.1.1.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_slash_netmask_16
   ARGS                 10.1.1.1/255.255.0.0 -c -n -g
   ADDRESS              10.1.1.1
   FAMILY               IPv4
   NETWORK              10.1.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            10.1.255.255
   WILDCARD             0.0.255.255
   HEX                  0A010101
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             10.1.0.1
   HOST_MAX             10.1.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_space_netmask_24
   ARGS                 192.168.1.1 255.255.255.0 -c -n -g
   ADDRESS              192.168.1.1
   FAMILY               IPv4
   NETWORK              192.168.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.168.1.255
   WILDCARD             0.0.0.255
   HEX                  C0A80101
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.168.1.1
   HOST_MAX             192.168.1.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_shorthand_3part
   ARGS                 10.1.1 24 -c -n -g
   ADDRESS              10.1.0.1
   FAMILY               IPv4
   NETWORK              10.1.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            10.1.0.255
   WILDCARD             0.0.0.255
   HEX                  0A010001
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             10.1.0.1
   HOST_MAX             10.1.0.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_shorthand_2part
   ARGS                 10.1 16 -c -n -g
   ADDRESS              10.0.0.1
   FAMILY               IPv4
   NETWORK              10.0.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            10.0.255.255
   WILDCARD             0.0.255.255
   HEX                  0A000001
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             10.0.0.1
   HOST_MAX             10.0.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_shorthand_1part
   ARGS                 167772161 24 -c -n -g
   ADDRESS              10.0.0.1
   FAMILY               IPv4
   NETWORK              10.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            10.0.0.255
   WILDCARD             0.0.0.255
   HEX                  0A000001
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             10.0.0.1
   HOST_MAX             10.0.0.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

# --- Special Ranges & Edge Cases -------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_link_local
   ARGS                 169.254.10.20 16 -c -n -g
   ADDRESS              169.254.10.20
   FAMILY               IPv4
   NETWORK              169.254.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            169.254.255.255
   WILDCARD             0.0.255.255
   HEX                  A9FE0A14
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             169.254.0.1
   HOST_MAX             169.254.255.254
   TYPE                 host
   CLASS                "Class B"
   FLAG                 "Link-local address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_link_local_boundary_low
   ARGS                 169.253.255.255 16 -c -n -g
   ADDRESS              169.253.255.255
   FAMILY               IPv4
   NETWORK              169.253.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            169.253.255.255
   WILDCARD             0.0.255.255
   HEX                  A9FDFFFF
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             169.253.0.1
   HOST_MAX             169.253.255.254
   TYPE                 broadcast
   CLASS                "Class B"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_link_local_boundary_high
   ARGS                 169.255.0.1 16 -c -n -g
   ADDRESS              169.255.0.1
   FAMILY               IPv4
   NETWORK              169.255.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            169.255.255.255
   WILDCARD             0.0.255.255
   HEX                  A9FF0001
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             169.255.0.1
   HOST_MAX             169.255.255.254
   TYPE                 host
   CLASS                "Class B"
)

SET_TESTS_PROPERTIES(
   ipv4_link_local_boundary_low_text ipv4_link_local_boundary_low_json
   ipv4_link_local_boundary_low_yaml ipv4_link_local_boundary_low_xml
   ipv4_link_local_boundary_high_text ipv4_link_local_boundary_high_json
   ipv4_link_local_boundary_high_yaml ipv4_link_local_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Link-local address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_cgnat_rfc6598
   ARGS                 100.64.1.1 10 -c -n -g
   ADDRESS              100.64.1.1
   FAMILY               IPv4
   NETWORK              100.64.0.0
   PREFIX               10
   NETMASK              255.192.0.0
   BROADCAST            100.127.255.255
   WILDCARD             0.63.255.255
   HEX                  64400101
   HOST_BITS            22
   MAX_HOSTS            4194302
   HOST_MIN             100.64.0.1
   HOST_MAX             100.127.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Shared address space for CGN"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_cgnat_boundary_low
   ARGS                 100.63.255.255 16 -c -n -g
   ADDRESS              100.63.255.255
   FAMILY               IPv4
   NETWORK              100.63.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            100.63.255.255
   WILDCARD             0.0.255.255
   HEX                  643FFFFF
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             100.63.0.1
   HOST_MAX             100.63.255.254
   TYPE                 broadcast
   CLASS                "Class A"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_cgnat_boundary_high
   ARGS                 100.128.0.0 16 -c -n -g
   ADDRESS              100.128.0.0
   FAMILY               IPv4
   NETWORK              100.128.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            100.128.255.255
   WILDCARD             0.0.255.255
   HEX                  64800000
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             100.128.0.1
   HOST_MAX             100.128.255.254
   TYPE                 network
   CLASS                "Class A"
)

SET_TESTS_PROPERTIES(
   ipv4_cgnat_boundary_low_text ipv4_cgnat_boundary_low_json
   ipv4_cgnat_boundary_low_yaml ipv4_cgnat_boundary_low_xml
   ipv4_cgnat_boundary_high_text ipv4_cgnat_boundary_high_json
   ipv4_cgnat_boundary_high_yaml ipv4_cgnat_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Shared address space for CGN"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol
   ARGS                 192.0.0.1 24 -c -n -g
   ADDRESS              192.0.0.1
   FAMILY               IPv4
   NETWORK              192.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.0.0.255
   WILDCARD             0.0.0.255
   HEX                  C0000001
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.0.0.1
   HOST_MAX             192.0.0.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "IPv4 Service Continuity Prefix"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_service_cont_high
   ARGS                 192.0.0.7 32 -c -n -g
   ADDRESS              192.0.0.7
   FAMILY               IPv4
   NETWORK              192.0.0.7
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C0000007
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.7
   HOST_MAX             192.0.0.7
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "IPv4 Service Continuity Prefix"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_dummy
   ARGS                 192.0.0.8 32 -c -n -g
   ADDRESS              192.0.0.8
   FAMILY               IPv4
   NETWORK              192.0.0.8
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C0000008
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.8
   HOST_MAX             192.0.0.8
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "IPv4 Dummy Address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_pcp
   ARGS                 192.0.0.9 32 -c -n -g
   ADDRESS              192.0.0.9
   FAMILY               IPv4
   NETWORK              192.0.0.9
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C0000009
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.9
   HOST_MAX             192.0.0.9
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "Port Control Protocol Anycast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_turn
   ARGS                 192.0.0.10 32 -c -n -g
   ADDRESS              192.0.0.10
   FAMILY               IPv4
   NETWORK              192.0.0.10
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C000000A
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.10
   HOST_MAX             192.0.0.10
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "TURN Anycast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_subassign_gap
   ARGS                 192.0.0.11 32 -c -n -g
   ADDRESS              192.0.0.11
   FAMILY               IPv4
   NETWORK              192.0.0.11
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C000000B
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.11
   HOST_MAX             192.0.0.11
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
)

SET_TESTS_PROPERTIES(
   ipv4_ietf_protocol_subassign_gap_text ipv4_ietf_protocol_subassign_gap_json
   ipv4_ietf_protocol_subassign_gap_yaml ipv4_ietf_protocol_subassign_gap_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;IPv4 Service Continuity Prefix;IPv4 Dummy Address;Port Control Protocol Anycast;TURN Anycast;NAT64/DNS64 Discovery"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_nat64
   ARGS                 192.0.0.170 32 -c -n -g
   ADDRESS              192.0.0.170
   FAMILY               IPv4
   NETWORK              192.0.0.170
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C00000AA
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.170
   HOST_MAX             192.0.0.170
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "NAT64/DNS64 Discovery"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_nat64_2
   ARGS                 192.0.0.171 32 -c -n -g
   ADDRESS              192.0.0.171
   FAMILY               IPv4
   NETWORK              192.0.0.171
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  C00000AB
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             192.0.0.171
   HOST_MAX             192.0.0.171
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "NAT64/DNS64 Discovery"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_unassigned
   ARGS                 192.0.0.100 24 -c -n -g
   ADDRESS              192.0.0.100
   FAMILY               IPv4
   NETWORK              192.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.0.0.255
   WILDCARD             0.0.0.255
   HEX                  C0000064
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.0.0.1
   HOST_MAX             192.0.0.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "IETF Protocol Assignments"
)

SET_TESTS_PROPERTIES(
   ipv4_ietf_protocol_unassigned_text ipv4_ietf_protocol_unassigned_json
   ipv4_ietf_protocol_unassigned_yaml ipv4_ietf_protocol_unassigned_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;IPv4 Service Continuity Prefix;IPv4 Dummy Address;Port Control Protocol Anycast;TURN Anycast;NAT64/DNS64 Discovery"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_ietf_protocol_boundary
   ARGS                 192.0.1.1 24 -c -n -g
   ADDRESS              192.0.1.1
   FAMILY               IPv4
   NETWORK              192.0.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.0.1.255
   WILDCARD             0.0.0.255
   HEX                  C0000101
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.0.1.1
   HOST_MAX             192.0.1.254
   TYPE                 host
   CLASS                "Class C"
)

SET_TESTS_PROPERTIES(
   ipv4_ietf_protocol_boundary_text ipv4_ietf_protocol_boundary_json
   ipv4_ietf_protocol_boundary_yaml ipv4_ietf_protocol_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;IETF Protocol Assignments;Documentation"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet1_boundary_low
   ARGS                 192.0.1.255 24 -c -n -g
   ADDRESS              192.0.1.255
   FAMILY               IPv4
   NETWORK              192.0.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.0.1.255
   WILDCARD             0.0.0.255
   HEX                  C00001FF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.0.1.1
   HOST_MAX             192.0.1.254
   TYPE                 broadcast
   CLASS                "Class C"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet2_boundary_low
   ARGS                 198.51.99.255 24 -c -n -g
   ADDRESS              198.51.99.255
   FAMILY               IPv4
   NETWORK              198.51.99.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            198.51.99.255
   WILDCARD             0.0.0.255
   HEX                  C63363FF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             198.51.99.1
   HOST_MAX             198.51.99.254
   TYPE                 broadcast
   CLASS                "Class C"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet3_boundary_low
   ARGS                 203.0.112.255 24 -c -n -g
   ADDRESS              203.0.112.255
   FAMILY               IPv4
   NETWORK              203.0.112.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            203.0.112.255
   WILDCARD             0.0.0.255
   HEX                  CB0070FF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             203.0.112.1
   HOST_MAX             203.0.112.254
   TYPE                 broadcast
   CLASS                "Class C"
)

SET_TESTS_PROPERTIES(
   ipv4_doc_testnet1_boundary_low_text ipv4_doc_testnet1_boundary_low_json
   ipv4_doc_testnet1_boundary_low_yaml ipv4_doc_testnet1_boundary_low_xml
   ipv4_doc_testnet2_boundary_low_text ipv4_doc_testnet2_boundary_low_json
   ipv4_doc_testnet2_boundary_low_yaml ipv4_doc_testnet2_boundary_low_xml
   ipv4_doc_testnet3_boundary_low_text ipv4_doc_testnet3_boundary_low_json
   ipv4_doc_testnet3_boundary_low_yaml ipv4_doc_testnet3_boundary_low_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Documentation"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet1
   ARGS                 192.0.2.1 24 -c -n -g
   ADDRESS              192.0.2.1
   FAMILY               IPv4
   NETWORK              192.0.2.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            192.0.2.255
   WILDCARD             0.0.0.255
   HEX                  C0000201
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             192.0.2.1
   HOST_MAX             192.0.2.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Documentation"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet2
   ARGS                 198.51.100.1 24 -c -n -g
   ADDRESS              198.51.100.1
   FAMILY               IPv4
   NETWORK              198.51.100.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            198.51.100.255
   WILDCARD             0.0.0.255
   HEX                  C6336401
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             198.51.100.1
   HOST_MAX             198.51.100.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Documentation"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_doc_testnet3
   ARGS                 203.0.113.1 24 -c -n -g
   ADDRESS              203.0.113.1
   FAMILY               IPv4
   NETWORK              203.0.113.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            203.0.113.255
   WILDCARD             0.0.0.255
   HEX                  CB007101
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             203.0.113.1
   HOST_MAX             203.0.113.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Documentation"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_benchmarking_low
   ARGS                 198.18.0.1 15 -c -n -g
   ADDRESS              198.18.0.1
   FAMILY               IPv4
   NETWORK              198.18.0.0
   PREFIX               15
   NETMASK              255.254.0.0
   BROADCAST            198.19.255.255
   WILDCARD             0.1.255.255
   HEX                  C6120001
   HOST_BITS            17
   MAX_HOSTS            131070
   HOST_MIN             198.18.0.1
   HOST_MAX             198.19.255.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Benchmarking"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_benchmarking_high
   ARGS                 198.19.255.254 15 -c -n -g
   ADDRESS              198.19.255.254
   FAMILY               IPv4
   NETWORK              198.18.0.0
   PREFIX               15
   NETMASK              255.254.0.0
   BROADCAST            198.19.255.255
   WILDCARD             0.1.255.255
   HEX                  C613FFFE
   HOST_BITS            17
   MAX_HOSTS            131070
   HOST_MIN             198.18.0.1
   HOST_MAX             198.19.255.254
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Benchmarking"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_benchmarking_boundary_low
   ARGS                 198.17.255.255 16 -c -n -g
   ADDRESS              198.17.255.255
   FAMILY               IPv4
   NETWORK              198.17.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            198.17.255.255
   WILDCARD             0.0.255.255
   HEX                  C611FFFF
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             198.17.0.1
   HOST_MAX             198.17.255.254
   TYPE                 broadcast
   CLASS                "Class C"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_benchmarking_boundary_high
   ARGS                 198.20.0.0 16 -c -n -g
   ADDRESS              198.20.0.0
   FAMILY               IPv4
   NETWORK              198.20.0.0
   PREFIX               16
   NETMASK              255.255.0.0
   BROADCAST            198.20.255.255
   WILDCARD             0.0.255.255
   HEX                  C6140000
   HOST_BITS            16
   MAX_HOSTS            65534
   HOST_MIN             198.20.0.1
   HOST_MAX             198.20.255.254
   TYPE                 network
   CLASS                "Class C"
)

SET_TESTS_PROPERTIES(
   ipv4_benchmarking_boundary_low_text ipv4_benchmarking_boundary_low_json
   ipv4_benchmarking_boundary_low_yaml ipv4_benchmarking_boundary_low_xml
   ipv4_benchmarking_boundary_high_text ipv4_benchmarking_boundary_high_json
   ipv4_benchmarking_boundary_high_yaml ipv4_benchmarking_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Benchmarking"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_pre_multicast_boundary
   ARGS                 223.255.255.255 24 -c -n -g
   ADDRESS              223.255.255.255
   FAMILY               IPv4
   NETWORK              223.255.255.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            223.255.255.255
   WILDCARD             0.0.0.255
   HEX                  DFFFFFFF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             223.255.255.1
   HOST_MAX             223.255.255.254
   TYPE                 broadcast
   CLASS                "Class C"
)

SET_TESTS_PROPERTIES(
   ipv4_pre_multicast_boundary_text ipv4_pre_multicast_boundary_json
   ipv4_pre_multicast_boundary_yaml ipv4_pre_multicast_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;multicast;multicast_mac;source_specific_multicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_class_e_reserved
   ARGS                 240.0.0.1 4 -c -n -g
   ADDRESS              240.0.0.1
   FAMILY               IPv4
   NETWORK              240.0.0.0
   PREFIX               4
   NETMASK              240.0.0.0
   BROADCAST            255.255.255.255
   WILDCARD             15.255.255.255
   HEX                  F0000001
   HOST_BITS            28
   MAX_HOSTS            268435454
   HOST_MIN             240.0.0.1
   HOST_MAX             255.255.255.254
   TYPE                 host
   CLASS                "Class E (Reserved)"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_class_e_high
   ARGS                 255.255.255.254 32 -c -n -g
   ADDRESS              255.255.255.254
   FAMILY               IPv4
   NETWORK              255.255.255.254
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  FFFFFFFE
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             255.255.255.254
   HOST_MAX             255.255.255.254
   TYPE                 host
   CLASS                "Class E (Reserved)"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_class_e_14_boundary
   ARGS                 240.0.0.0 14 -c -n -g
   ADDRESS              240.0.0.0
   FAMILY               IPv4
   NETWORK              240.0.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            240.3.255.255
   WILDCARD             0.3.255.255
   HEX                  F0000000
   HOST_BITS            18
   MAX_HOSTS            262142
   HOST_MIN             240.0.0.1
   HOST_MAX             240.3.255.254
   TYPE                 network
   CLASS                "Class E (Reserved)"
)

SET_TESTS_PROPERTIES(
   ipv4_class_e_reserved_text ipv4_class_e_reserved_json
   ipv4_class_e_reserved_yaml ipv4_class_e_reserved_xml
   ipv4_class_e_high_text ipv4_class_e_high_json
   ipv4_class_e_high_yaml ipv4_class_e_high_xml
   ipv4_class_e_14_boundary_text ipv4_class_e_14_boundary_json
   ipv4_class_e_14_boundary_yaml ipv4_class_e_14_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Limited broadcast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_network_address
   ARGS                 10.1.1.0 24 -c -n -g
   ADDRESS              10.1.1.0
   FAMILY               IPv4
   NETWORK              10.1.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            10.1.1.255
   WILDCARD             0.0.0.255
   HEX                  0A010100
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             10.1.1.1
   HOST_MAX             10.1.1.254
   TYPE                 network
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_broadcast_address
   ARGS                 10.1.1.255 24 -c -n -g
   ADDRESS              10.1.1.255
   FAMILY               IPv4
   NETWORK              10.1.1.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            10.1.1.255
   WILDCARD             0.0.0.255
   HEX                  0A0101FF
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             10.1.1.1
   HOST_MAX             10.1.1.254
   TYPE                 broadcast
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_default_route
   ARGS                 0.0.0.0 0 -c -n -g
   ADDRESS              0.0.0.0
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               0
   NETMASK              0.0.0.0
   BROADCAST            255.255.255.255
   WILDCARD             255.255.255.255
   HEX                  00000000
   HOST_BITS            32
   MAX_HOSTS            4294967294
   HOST_MIN             0.0.0.1
   HOST_MAX             255.255.255.254
   TYPE                 network
   CLASS                "Class A"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_unspecified_host_32
   ARGS                 0.0.0.0 32 -c -n -g
   ADDRESS              0.0.0.0
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  00000000
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             0.0.0.0
   HOST_MAX             0.0.0.0
   TYPE                 host
   CLASS                "Class A"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_limited_broadcast
   ARGS                 255.255.255.255 32 -c -n -g
   ADDRESS              255.255.255.255
   FAMILY               IPv4
   NETWORK              255.255.255.255
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  FFFFFFFF
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             255.255.255.255
   HOST_MAX             255.255.255.255
   TYPE                 host
   FLAG                 "Limited broadcast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_broadcast_in_default_0
   ARGS                 255.255.255.255 0 -c -n -g
   ADDRESS              255.255.255.255
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               0
   NETMASK              0.0.0.0
   BROADCAST            255.255.255.255
   WILDCARD             255.255.255.255
   HEX                  FFFFFFFF
   HOST_BITS            32
   MAX_HOSTS            4294967294
   HOST_MIN             0.0.0.1
   HOST_MAX             255.255.255.254
   TYPE                 broadcast
   FLAG                 "Limited broadcast"
)

SET_TESTS_PROPERTIES(
   ipv4_limited_broadcast_text ipv4_limited_broadcast_json
   ipv4_limited_broadcast_yaml ipv4_limited_broadcast_xml
   ipv4_broadcast_in_default_0_text ipv4_broadcast_in_default_0_json
   ipv4_broadcast_in_default_0_yaml ipv4_broadcast_in_default_0_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Class E"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_host
   ARGS                 127.0.0.1 8 -c -n -g
   ADDRESS              127.0.0.1
   FAMILY               IPv4
   NETWORK              127.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            127.255.255.255
   WILDCARD             0.255.255.255
   HEX                  7F000001
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             127.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Loopback address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net_host_8
   ARGS                 127.0.0.2 8 -c -n -g
   ADDRESS              127.0.0.2
   FAMILY               IPv4
   NETWORK              127.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            127.255.255.255
   WILDCARD             0.255.255.255
   HEX                  7F000002
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             127.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "In loopback network"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net_127_1
   ARGS                 127.1.2.3 8 -c -n -g
   ADDRESS              127.1.2.3
   FAMILY               IPv4
   NETWORK              127.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            127.255.255.255
   WILDCARD             0.255.255.255
   HEX                  7F010203
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             127.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "In loopback network"
)

SET_TESTS_PROPERTIES(
   ipv4_loopback_net_host_8_text ipv4_loopback_net_host_8_json
   ipv4_loopback_net_host_8_yaml ipv4_loopback_net_host_8_xml
   ipv4_loopback_net_127_1_text ipv4_loopback_net_127_1_json
   ipv4_loopback_net_127_1_yaml ipv4_loopback_net_127_1_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Loopback address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net_boundary_low
   ARGS                 126.255.255.255 8 -c -n -g
   ADDRESS              126.255.255.255
   FAMILY               IPv4
   NETWORK              126.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            126.255.255.255
   WILDCARD             0.255.255.255
   HEX                  7EFFFFFF
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             126.0.0.1
   HOST_MAX             126.255.255.254
   TYPE                 broadcast
   CLASS                "Class A"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net_boundary_high
   ARGS                 128.0.0.1 8 -c -n -g
   ADDRESS              128.0.0.1
   FAMILY               IPv4
   NETWORK              128.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            128.255.255.255
   WILDCARD             0.255.255.255
   HEX                  80000001
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             128.0.0.1
   HOST_MAX             128.255.255.254
   TYPE                 host
   CLASS                "Class B"
)

SET_TESTS_PROPERTIES(
   ipv4_loopback_net_boundary_low_text ipv4_loopback_net_boundary_low_json
   ipv4_loopback_net_boundary_low_yaml ipv4_loopback_net_boundary_low_xml
   ipv4_loopback_net_boundary_high_text ipv4_loopback_net_boundary_high_json
   ipv4_loopback_net_boundary_high_yaml ipv4_loopback_net_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Loopback address;In loopback network"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net
   ARGS                 127.0.0.2 24 -c -n -g
   ADDRESS              127.0.0.2
   FAMILY               IPv4
   NETWORK              127.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            127.0.0.255
   WILDCARD             0.0.0.255
   HEX                  7F000002
   HOST_BITS            8
   MAX_HOSTS            254
   HOST_MIN             127.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "In loopback network"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_p2p_31
   ARGS                 10.1.1.1 31 -c -n -g
   ADDRESS              10.1.1.1
   FAMILY               IPv4
   NETWORK              10.1.1.0
   PREFIX               31
   NETMASK              255.255.255.254
   WILDCARD             0.0.0.1
   HEX                  0A010101
   HOST_BITS            1
   MAX_HOSTS            2
   HOST_MIN             10.1.1.0
   HOST_MAX             10.1.1.1
   TYPE                 broadcast
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_p2p_31_network
   ARGS                 10.1.1.0 31 -c -n -g
   ADDRESS              10.1.1.0
   FAMILY               IPv4
   NETWORK              10.1.1.0
   PREFIX               31
   NETMASK              255.255.255.254
   WILDCARD             0.0.0.1
   HEX                  0A010100
   HOST_BITS            1
   MAX_HOSTS            2
   HOST_MIN             10.1.1.0
   HOST_MAX             10.1.1.1
   TYPE                 network
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_host_32
   ARGS                 10.1.1.1 32 -c -n -g
   ADDRESS              10.1.1.1
   FAMILY               IPv4
   NETWORK              10.1.1.1
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  0A010101
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             10.1.1.1
   HOST_MAX             10.1.1.1
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

# --- Multicast -------------------------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_linklocal
   ARGS                 224.0.0.1 24 -c -n -g
   ADDRESS              224.0.0.1
   FAMILY               IPv4
   NETWORK              224.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            224.0.0.255
   WILDCARD             0.0.0.255
   HEX                  E0000001
   HOST_BITS            8
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "link-local"
   MULTICAST_MAC        "01:00:5e:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_zero_host_byte
   ARGS                 224.0.0.0 24 -c -n -g
   ADDRESS              224.0.0.0
   FAMILY               IPv4
   NETWORK              224.0.0.0
   PREFIX               24
   NETMASK              255.255.255.0
   BROADCAST            224.0.0.255
   WILDCARD             0.0.0.255
   HEX                  E0000000
   HOST_BITS            8
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "link-local"
   MULTICAST_MAC        "01:00:5e:00:00:00"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_ssm_boundary_low
   ARGS                 231.255.255.255 8 -c -n -g
   ADDRESS              231.255.255.255
   FAMILY               IPv4
   NETWORK              231.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            231.255.255.255
   WILDCARD             0.255.255.255
   HEX                  E7FFFFFF
   HOST_BITS            24
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "global"
   MULTICAST_MAC        "01:00:5e:7f:ff:ff"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_ssm_boundary_high
   ARGS                 233.0.0.1 8 -c -n -g
   ADDRESS              233.0.0.1
   FAMILY               IPv4
   NETWORK              233.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            233.255.255.255
   WILDCARD             0.255.255.255
   HEX                  E9000001
   HOST_BITS            24
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "global"
   MULTICAST_MAC        "01:00:5e:00:00:01"
)

SET_TESTS_PROPERTIES(
   ipv4_multicast_ssm_boundary_low_text ipv4_multicast_ssm_boundary_low_json
   ipv4_multicast_ssm_boundary_low_yaml ipv4_multicast_ssm_boundary_low_xml
   ipv4_multicast_ssm_boundary_high_text ipv4_multicast_ssm_boundary_high_json
   ipv4_multicast_ssm_boundary_high_yaml ipv4_multicast_ssm_boundary_high_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Source-specific multicast;source_specific_multicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_ssm
   ARGS                 232.1.2.3 8 -c -n -g
   ADDRESS              232.1.2.3
   FAMILY               IPv4
   NETWORK              232.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            232.255.255.255
   WILDCARD             0.255.255.255
   HEX                  E8010203
   HOST_BITS            24
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "global"
   MULTICAST_MAC        "01:00:5e:01:02:03"
   SSM                  TRUE
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_org_local_boundary_low
   ARGS                 239.191.255.255 14 -c -n -g
   ADDRESS              239.191.255.255
   FAMILY               IPv4
   NETWORK              239.188.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            239.191.255.255
   WILDCARD             0.3.255.255
   HEX                  EFBFFFFF
   HOST_BITS            18
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "global"
   MULTICAST_MAC        "01:00:5e:3f:ff:ff"
)

SET_TESTS_PROPERTIES(
   ipv4_multicast_org_local_boundary_low_text ipv4_multicast_org_local_boundary_low_json
   ipv4_multicast_org_local_boundary_low_yaml ipv4_multicast_org_local_boundary_low_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;organization-local"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_org_local_boundary_high
   ARGS                 239.196.0.0 14 -c -n -g
   ADDRESS              239.196.0.0
   FAMILY               IPv4
   NETWORK              239.196.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            239.199.255.255
   WILDCARD             0.3.255.255
   HEX                  EFC40000
   HOST_BITS            18
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "organization-local"
   MULTICAST_MAC        "01:00:5e:44:00:00"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_org_local
   ARGS                 239.192.1.1 14 -c -n -g
   ADDRESS              239.192.1.1
   FAMILY               IPv4
   NETWORK              239.192.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            239.195.255.255
   WILDCARD             0.3.255.255
   HEX                  EFC00101
   HOST_BITS            18
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "organization-local"
   MULTICAST_MAC        "01:00:5e:40:01:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_site_local_boundary_low
   ARGS                 239.251.255.255 14 -c -n -g
   ADDRESS              239.251.255.255
   FAMILY               IPv4
   NETWORK              239.248.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            239.251.255.255
   WILDCARD             0.3.255.255
   HEX                  EFFBFFFF
   HOST_BITS            18
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "organization-local"
   MULTICAST_MAC        "01:00:5e:7b:ff:ff"
)

SET_TESTS_PROPERTIES(
   ipv4_multicast_site_local_boundary_low_text ipv4_multicast_site_local_boundary_low_json
   ipv4_multicast_site_local_boundary_low_yaml ipv4_multicast_site_local_boundary_low_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;site-local"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_site_local
   ARGS                 239.252.1.1 14 -c -n -g
   ADDRESS              239.252.1.1
   FAMILY               IPv4
   NETWORK              239.252.0.0
   PREFIX               14
   NETMASK              255.252.0.0
   BROADCAST            239.255.255.255
   WILDCARD             0.3.255.255
   HEX                  EFFC0101
   HOST_BITS            18
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "site-local"
   MULTICAST_MAC        "01:00:5e:7c:01:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_global
   ARGS                 225.1.1.1 8 -c -n -g
   ADDRESS              225.1.1.1
   FAMILY               IPv4
   NETWORK              225.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            225.255.255.255
   WILDCARD             0.255.255.255
   HEX                  E1010101
   HOST_BITS            24
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "global"
   MULTICAST_MAC        "01:00:5e:01:01:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_class_a_zero_octet_host
   ARGS                 0.1.2.3 8 -c -n -g
   ADDRESS              0.1.2.3
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            0.255.255.255
   WILDCARD             0.255.255.255
   HEX                  00010203
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             0.0.0.1
   HOST_MAX             0.255.255.254
   TYPE                 host
   CLASS                "Class A"
)

SET_TESTS_PROPERTIES(
   ipv4_class_a_zero_octet_host_text ipv4_class_a_zero_octet_host_json
   ipv4_class_a_zero_octet_host_yaml ipv4_class_a_zero_octet_host_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Loopback address;In loopback network;Private;Shared address space for CGN"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_loopback_net_broadcast_8
   ARGS                 127.255.255.255 8 -c -n -g
   ADDRESS              127.255.255.255
   FAMILY               IPv4
   NETWORK              127.0.0.0
   PREFIX               8
   NETMASK              255.0.0.0
   BROADCAST            127.255.255.255
   WILDCARD             0.255.255.255
   HEX                  7FFFFFFF
   HOST_BITS            24
   MAX_HOSTS            16777214
   HOST_MIN             127.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 broadcast
   CLASS                "Class A"
   FLAG                 "In loopback network"
)

SET_TESTS_PROPERTIES(
   ipv4_loopback_net_broadcast_8_text ipv4_loopback_net_broadcast_8_json
   ipv4_loopback_net_broadcast_8_yaml ipv4_loopback_net_broadcast_8_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Loopback address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv4_multicast_top_32
   ARGS                 239.255.255.255 32 -c -n -g
   ADDRESS              239.255.255.255
   FAMILY               IPv4
   NETWORK              239.255.255.255
   PREFIX               32
   NETMASK              255.255.255.255
   WILDCARD             0.0.0.0
   HEX                  EFFFFFFF
   HOST_BITS            0
   TYPE                 multicast
   CLASS                "Class D (Multicast)"
   SCOPE                "site-local"
   MULTICAST_MAC        "01:00:5e:7f:ff:ff"
)

# --- Arbitrary Bit Boundaries ----------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv4_p1
   ARGS                 10.1.2.3 1 -c -n -g
   ADDRESS              10.1.2.3
   FAMILY               IPv4
   NETWORK              0.0.0.0
   PREFIX               1
   NETMASK              128.0.0.0
   BROADCAST            127.255.255.255
   WILDCARD             127.255.255.255
   HEX                  0A010203
   HOST_BITS            31
   MAX_HOSTS            2147483646
   HOST_MIN             0.0.0.1
   HOST_MAX             127.255.255.254
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv4_p27
   ARGS                 192.168.1.200 27 -c -n -g
   ADDRESS              192.168.1.200
   FAMILY               IPv4
   NETWORK              192.168.1.192
   PREFIX               27
   NETMASK              255.255.255.224
   BROADCAST            192.168.1.223
   WILDCARD             0.0.0.31
   HEX                  C0A801C8
   HOST_BITS            5
   MAX_HOSTS            30
   HOST_MIN             192.168.1.193
   HOST_MAX             192.168.1.222
   TYPE                 host
   CLASS                "Class C"
   FLAG                 "Private"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv4_p30
   ARGS                 10.0.0.1 30 -c -n -g
   ADDRESS              10.0.0.1
   FAMILY               IPv4
   NETWORK              10.0.0.0
   PREFIX               30
   NETMASK              255.255.255.252
   BROADCAST            10.0.0.3
   WILDCARD             0.0.0.3
   HEX                  0A000001
   HOST_BITS            2
   MAX_HOSTS            2
   HOST_MIN             10.0.0.1
   HOST_MAX             10.0.0.2
   TYPE                 host
   CLASS                "Class A"
   FLAG                 "Private"
)


#############################################################################
#### IPV6 MATRIX TESTS                                                   ####
#############################################################################

# --- Formatting Syntaxes ---------------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_slash_netmask_64
   ARGS                 2001:db8::1/ffff:ffff:ffff:ffff:: -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_space_netmask_64
   ARGS                 2001:db8::1 ffff:ffff:ffff:ffff:: -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_full_mask_128
   ARGS                 2001:db8::1/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:db8::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::1
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_zero_mask_0
   ARGS                 2001:db8::1/:: -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ::
   PREFIX               0
   NETMASK              ::
   WILDCARD             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            128
   MAX_HOSTS            340282366920938463463374607431768211454
   HOST_MIN             ::1
   HOST_MAX             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

# --- Special & Embedded Types ----------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_unspecified
   ARGS                 :: 0 -c -n -g
   ADDRESS              ::
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:0000:0000:0000
   FAMILY               IPv6
   NETWORK              ::
   PREFIX               0
   NETMASK              ::
   WILDCARD             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            128
   MAX_HOSTS            340282366920938463463374607431768211454
   HOST_MIN             ::1
   HOST_MAX             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 network
   SPECIAL              "Unspecified address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_unspecified_128
   ARGS                 :: 128 -c -n -g
   ADDRESS              ::
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:0000:0000:0000
   FAMILY               IPv6
   NETWORK              ::
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             ::
   HOST_MAX             ::
   TYPE                 host
   SPECIAL              "Unspecified address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_loopback
   ARGS                 ::1 128 -c -n -g
   ADDRESS              ::1
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             ::1
   HOST_MAX             ::1
   TYPE                 host
   SPECIAL              "Loopback address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_v4compat
   ARGS                 ::192.0.2.1 128 -c -n -g
   ADDRESS              "(::192\\.0\\.2\\.1|::c000:201)"
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:0000:c000:0201
   FAMILY               IPv6
   NETWORK              "(::192\\.0\\.2\\.1|::c000:201)"
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             "(::192\\.0\\.2\\.1|::c000:201)"
   HOST_MAX             "(::192\\.0\\.2\\.1|::c000:201)"
   TYPE                 host
   SPECIAL              "IPv4-compatible IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_v4mapped
   ARGS                 ::ffff:192.0.2.1 128 -c -n -g
   ADDRESS              ::ffff:192.0.2.1
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:ffff:c000:0201
   FAMILY               IPv6
   NETWORK              ::ffff:192.0.2.1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             ::ffff:192.0.2.1
   HOST_MAX             ::ffff:192.0.2.1
   TYPE                 host
   SPECIAL              "IPv4-mapped IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_nat64_wkp
   ARGS                 64:ff9b::1.2.3.4 96 -c -n -g
   ADDRESS              64:ff9b::1.2.3.4
   EXPANDED_ADDRESS     0064:ff9b:0000:0000:0000:0000:0102:0304
   FAMILY               IPv6
   NETWORK              64:ff9b::0.0.0.0
   PREFIX               96
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff
   HOST_BITS            32
   MAX_HOSTS            4294967295
   HOST_MIN             64:ff9b::0.0.0.1
   HOST_MAX             64:ff9b::255.255.255.255
   TYPE                 host
   SPECIAL              "IPv4-embedded IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_nat64_custom
   ARGS                 64:ff9b:1:2:3:4:5.6.7.8 96 -c -n -g
   ADDRESS              64:ff9b:1:2:3:4:5.6.7.8
   EXPANDED_ADDRESS     0064:ff9b:0001:0002:0003:0004:0506:0708
   FAMILY               IPv6
   NETWORK              64:ff9b:1:2:3:4:0.0.0.0
   PREFIX               96
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff
   HOST_BITS            32
   MAX_HOSTS            4294967295
   HOST_MIN             64:ff9b:1:2:3:4:0.0.0.1
   HOST_MAX             64:ff9b:1:2:3:4:255.255.255.255
   TYPE                 host
   SPECIAL              "IPv4-embedded IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_nat64_local_use_48
   ARGS                 64:ff9b:1::1 48 -c -n -g
   ADDRESS              64:ff9b:1::0\\.0\\.0\\.1
   EXPANDED_ADDRESS     0064:ff9b:0001:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              64:ff9b:1::0\\.0\\.0\\.0
   PREFIX               48
   NETMASK              ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff:ffff
   HOST_BITS            80
   MAX_HOSTS            1208925819614629174706175
   HOST_MIN             64:ff9b:1::0\\.0\\.0\\.1
   HOST_MAX             64:ff9b:1:ffff:ffff:ffff:255\\.255\\.255\\.255
   TYPE                 host
   SPECIAL              "IPv4-embedded IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_nat64_non_translation
   ARGS                 64:ff9b:2::1 96 -c -n -g
   ADDRESS              64:ff9b:2::1
   EXPANDED_ADDRESS     0064:ff9b:0002:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              64:ff9b:2::
   PREFIX               96
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff
   HOST_BITS            32
   MAX_HOSTS            4294967295
   HOST_MIN             64:ff9b:2::1
   HOST_MAX             64:ff9b:2::ffff:ffff
   TYPE                 host
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_nat64_word2_boundary
   ARGS                 64:ff9b:2::1.2.3.4 96 -c -n -g
   ADDRESS              64:ff9b:2::102:304
   EXPANDED_ADDRESS     0064:ff9b:0002:0000:0000:0000:0102:0304
   FAMILY               IPv6
   NETWORK              64:ff9b:2::
   PREFIX               96
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff
   HOST_BITS            32
   MAX_HOSTS            4294967295
   HOST_MIN             64:ff9b:2::1
   HOST_MAX             64:ff9b:2::ffff:ffff
   TYPE                 host
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0102:0304
   SNMC                 ff02::1:ff02:0304
)

SET_TESTS_PROPERTIES(
   ipv6_nat64_non_translation_text ipv6_nat64_non_translation_json
   ipv6_nat64_non_translation_yaml ipv6_nat64_non_translation_xml
   ipv6_nat64_word2_boundary_text ipv6_nat64_word2_boundary_json
   ipv6_nat64_word2_boundary_yaml ipv6_nat64_word2_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;IPv4-embedded IPv6 address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_discard_rfc6666
   ARGS                 100::1 64 -c -n -g
   ADDRESS              100::1
   EXPANDED_ADDRESS     0100:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              100::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             100::1
   HOST_MAX             100::ffff:ffff:ffff:ffff
   TYPE                 host
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

# --- Unicast: Link-Local, Site-Local, ULA, Global --------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_link_local_eui64
   ARGS                 fe80::0211:22ff:fe33:4455 64 -c -n -g
   ADDRESS              fe80::211:22ff:fe33:4455
   EXPANDED_ADDRESS     fe80:0000:0000:0000:0211:22ff:fe33:4455
   FAMILY               IPv6
   NETWORK              fe80::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fe80::1
   HOST_MAX             fe80::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Link-Local Unicast"
   INTERFACE_ID         0211:22ff:fe33:4455
   MAC_ADDRESS          00:11:22:33:44:55
   SNMC                 ff02::1:ff33:4455
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_link_local_eui64_u_bit_set
   ARGS                 fe80::0011:22ff:fe33:4455 64 -c -n -g
   ADDRESS              fe80::11:22ff:fe33:4455
   EXPANDED_ADDRESS     fe80:0000:0000:0000:0011:22ff:fe33:4455
   FAMILY               IPv6
   NETWORK              fe80::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fe80::1
   HOST_MAX             fe80::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Link-Local Unicast"
   INTERFACE_ID         0011:22ff:fe33:4455
   MAC_ADDRESS          02:11:22:33:44:55
   SNMC                 ff02::1:ff33:4455
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_link_local
   ARGS                 fe80::1 64 -c -n -g
   ADDRESS              fe80::1
   EXPANDED_ADDRESS     fe80:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fe80::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fe80::1
   HOST_MAX             fe80::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Link-Local Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_site_local
   ARGS                 fec0::1 64 -c -n -g
   ADDRESS              fec0::1
   EXPANDED_ADDRESS     fec0:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fec0::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fec0::1
   HOST_MAX             fec0::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Site-Local Unicast"
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_boundary_low
   ARGS                 fbff:ffff::1 64 -c -n -g
   ADDRESS              fbff:ffff::1
   EXPANDED_ADDRESS     fbff:ffff:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fbff:ffff::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fbff:ffff::1
   HOST_MAX             fbff:ffff::ffff:ffff:ffff:ffff
   TYPE                 host
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_ula_boundary_low_text ipv6_ula_boundary_low_json
   ipv6_ula_boundary_low_yaml ipv6_ula_boundary_low_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Unique Local Unicast;ula_type;global_id"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_boundary_high
   ARGS                 fe00::1 64 -c -n -g
   ADDRESS              fe00::1
   EXPANDED_ADDRESS     fe00:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fe00::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fe00::1
   HOST_MAX             fe00::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Unique Local Unicast"
   ULA_TYPE             "Assigned by global instance"
   GLOBAL_ID            0000000000
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_pre_gua_boundary
   ARGS                 1fff:ffff::1 64 -c -n -g
   ADDRESS              1fff:ffff::1
   EXPANDED_ADDRESS     1fff:ffff:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              1fff:ffff::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             1fff:ffff::1
   HOST_MAX             1fff:ffff::ffff:ffff:ffff:ffff
   TYPE                 host
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_pre_gua_boundary_text ipv6_pre_gua_boundary_json
   ipv6_pre_gua_boundary_yaml ipv6_pre_gua_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Global Unicast;unicast_type"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_local
   ARGS                 fd01:1122:3344:affe:0102:03ff:fe04:0506 64 -c -n -g
   ADDRESS              fd01:1122:3344:affe:102:3ff:fe04:506
   EXPANDED_ADDRESS     fd01:1122:3344:affe:0102:03ff:fe04:0506
   FAMILY               IPv6
   NETWORK              fd01:1122:3344:affe::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fd01:1122:3344:affe::1
   HOST_MAX             fd01:1122:3344:affe:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Unique Local Unicast"
   ULA_TYPE             "Locally chosen"
   GLOBAL_ID            0111223344
   SUBNET_ID            affe
   INTERFACE_ID         0102:03ff:fe04:0506
   MAC_ADDRESS          03:02:03:04:05:06
   SNMC                 ff02::1:ff04:0506
)

SET_TESTS_PROPERTIES(
   ipv6_ula_local_text ipv6_ula_local_json
   ipv6_ula_local_yaml ipv6_ula_local_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Assigned by global instance"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_local_8
   ARGS                 fd00::1 8 -c -n -g
   ADDRESS              fd00::1
   EXPANDED_ADDRESS     fd00:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fd00::
   PREFIX               8
   NETMASK              ff00::
   WILDCARD             ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            120
   MAX_HOSTS            1329227995784915872903807060280344575
   HOST_MIN             fd00::1
   HOST_MAX             fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Unique Local Unicast"
   ULA_TYPE             "Locally chosen"
   GLOBAL_ID            0000000000
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_global
   ARGS                 fc00:1122:3344:affe::1 64 -c -n -g
   ADDRESS              fc00:1122:3344:affe::1
   EXPANDED_ADDRESS     fc00:1122:3344:affe:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fc00:1122:3344:affe::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             fc00:1122:3344:affe::1
   HOST_MAX             fc00:1122:3344:affe:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Unique Local Unicast"
   ULA_TYPE             "Assigned by global instance"
   GLOBAL_ID            0011223344
   SUBNET_ID            affe
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_ula_global_text ipv6_ula_global_json
   ipv6_ula_global_yaml ipv6_ula_global_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Locally chosen"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ula_global_8
   ARGS                 fc00::1 8 -c -n -g
   ADDRESS              fc00::1
   EXPANDED_ADDRESS     fc00:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              fc00::
   PREFIX               8
   NETMASK              ff00::
   WILDCARD             ff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            120
   MAX_HOSTS            1329227995784915872903807060280344575
   HOST_MIN             fc00::1
   HOST_MAX             fcff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Unique Local Unicast"
   ULA_TYPE             "Assigned by global instance"
   GLOBAL_ID            0000000000
   SUBNET_ID            0000
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ietf_protocol_low
   ARGS                 2001::1 64 -c -n -g
   ADDRESS              2001::1
   EXPANDED_ADDRESS     2001:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001::1
   HOST_MAX             2001::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   FLAG                 "IETF Protocol Assignments"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ietf_protocol
   ARGS                 2001:1::1 64 -c -n -g
   ADDRESS              2001:1::1
   EXPANDED_ADDRESS     2001:0001:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:1::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:1::1
   HOST_MAX             2001:1::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   FLAG                 "IETF Protocol Assignments"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ietf_protocol_high
   ARGS                 2001:1ff::1 64 -c -n -g
   ADDRESS              2001:1ff::1
   EXPANDED_ADDRESS     2001:01ff:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:1ff::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:1ff::1
   HOST_MAX             2001:1ff::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   FLAG                 "IETF Protocol Assignments"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_benchmarking_boundary
   ARGS                 2001:2:1::1 48 -c -n -g
   ADDRESS              2001:2:1::1
   EXPANDED_ADDRESS     2001:0002:0001:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:2:1::
   PREFIX               48
   NETMASK              ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff:ffff
   HOST_BITS            80
   MAX_HOSTS            1208925819614629174706175
   HOST_MIN             2001:2:1::1
   HOST_MAX             2001:2:1:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   FLAG                 "IETF Protocol Assignments"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_ietf_protocol_low_text ipv6_ietf_protocol_low_json
   ipv6_ietf_protocol_low_yaml ipv6_ietf_protocol_low_xml
   ipv6_ietf_protocol_text ipv6_ietf_protocol_json
   ipv6_ietf_protocol_yaml ipv6_ietf_protocol_xml
   ipv6_ietf_protocol_high_text ipv6_ietf_protocol_high_json
   ipv6_ietf_protocol_high_yaml ipv6_ietf_protocol_high_xml
   ipv6_benchmarking_boundary_text ipv6_benchmarking_boundary_json
   ipv6_benchmarking_boundary_yaml ipv6_benchmarking_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Benchmarking"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_benchmarking
   ARGS                 2001:2::1 48 -c -n -g
   ADDRESS              2001:2::1
   EXPANDED_ADDRESS     2001:0002:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:2::
   PREFIX               48
   NETMASK              ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff:ffff
   HOST_BITS            80
   MAX_HOSTS            1208925819614629174706175
   HOST_MIN             2001:2::1
   HOST_MAX             2001:2:0:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   FLAG                 "IETF Protocol Assignments"
   FLAG2                "Benchmarking"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_ietf_protocol_boundary
   ARGS                 2001:200::1 64 -c -n -g
   ADDRESS              2001:200::1
   EXPANDED_ADDRESS     2001:0200:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:200::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:200::1
   HOST_MAX             2001:200::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_ietf_protocol_boundary_text ipv6_ietf_protocol_boundary_json
   ipv6_ietf_protocol_boundary_yaml ipv6_ietf_protocol_boundary_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;IETF Protocol Assignments;Benchmarking"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_6to4_global
   ARGS                 2002:0a01:0101::1 48 -c -n -g
   ADDRESS              2002:a01:101::1
   EXPANDED_ADDRESS     2002:0a01:0101:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2002:a01:101::
   PREFIX               48
   NETMASK              ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff:ffff
   HOST_BITS            80
   MAX_HOSTS            1208925819614629174706175
   HOST_MIN             2002:a01:101::1
   HOST_MAX             2002:a01:101:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
   SIX_TO_FOUR          10.1.1.1
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_6to4_network
   ARGS                 2002:0a01:0101:: 48 -c -n -g
   ADDRESS              2002:a01:101::
   EXPANDED_ADDRESS     2002:0a01:0101:0000:0000:0000:0000:0000
   FAMILY               IPv6
   NETWORK              2002:a01:101::
   PREFIX               48
   NETMASK              ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff:ffff
   HOST_BITS            80
   MAX_HOSTS            1208925819614629174706175
   HOST_MIN             2002:a01:101::1
   HOST_MAX             2002:a01:101:ffff:ffff:ffff:ffff:ffff
   TYPE                 network
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0000
   SNMC                 ff02::1:ff00:0000
   SIX_TO_FOUR          10.1.1.1
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_non_6to4_global
   ARGS                 2003:db8::1 64 -c -n -g
   ADDRESS              2003:db8::1
   EXPANDED_ADDRESS     2003:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2003:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2003:db8::1
   HOST_MAX             2003:db8::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

SET_TESTS_PROPERTIES(
   ipv6_non_6to4_global_text ipv6_non_6to4_global_json
   ipv6_non_6to4_global_yaml ipv6_non_6to4_global_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;6-to-4;six_to_four"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_doc_rfc3849
   ARGS                 2001:db8::1 64 -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_non_eui64_unicast
   ARGS                 2001:db8::1234:5678:9abc:def0 64 -c -n -g
   ADDRESS              2001:db8::1234:5678:9abc:def0
   EXPANDED_ADDRESS     2001:0db8:0000:0000:1234:5678:9abc:def0
   FAMILY               IPv6
   NETWORK              2001:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         1234:5678:9abc:def0
   SNMC                 ff02::1:ffbc:def0
)

SET_TESTS_PROPERTIES(
   ipv6_non_eui64_unicast_text ipv6_non_eui64_unicast_json
   ipv6_non_eui64_unicast_yaml ipv6_non_eui64_unicast_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;MAC Address;mac_address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_global_host_128
   ARGS                 2001:db8::1/128 -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001:db8::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            1
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::1
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0001
   SNMC                 ff02::1:ff00:0001
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_global_network_address
   ARGS                 2001:db8:: 64 -c -n -g
   ADDRESS              2001:db8::
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0000
   FAMILY               IPv6
   NETWORK              2001:db8::
   PREFIX               64
   NETMASK              ffff:ffff:ffff:ffff::
   WILDCARD             ::ffff:ffff:ffff:ffff
   HOST_BITS            64
   MAX_HOSTS            18446744073709551615
   HOST_MIN             2001:db8::1
   HOST_MAX             2001:db8::ffff:ffff:ffff:ffff
   TYPE                 network
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0000
   SNMC                 ff02::1:ff00:0000
)

# Ensure IPv6 output never emits Broadcast / broadcast
SET_TESTS_PROPERTIES(
   ipv6_slash_netmask_64_text ipv6_slash_netmask_64_json
   ipv6_slash_netmask_64_yaml ipv6_slash_netmask_64_xml
   ipv6_global_network_address_text ipv6_global_network_address_json
   ipv6_global_network_address_yaml ipv6_global_network_address_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Broadcast;broadcast"
)

# --- Multicast Scopes & Flags ----------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_node_local
   ARGS                 ff01::1 128 -c -n -g
   ADDRESS              ff01::1
   EXPANDED_ADDRESS     ff01:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff01::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "node-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_link_local
   ARGS                 ff02::2 128 -c -n -g
   ADDRESS              ff02::2
   EXPANDED_ADDRESS     ff02:0000:0000:0000:0000:0000:0000:0002
   FAMILY               IPv6
   NETWORK              ff02::2
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:00:00:00:02"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_site_local
   ARGS                 ff05::1 128 -c -n -g
   ADDRESS              ff05::1
   EXPANDED_ADDRESS     ff05:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff05::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "site-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_org_local
   ARGS                 ff08::1 128 -c -n -g
   ADDRESS              ff08::1
   EXPANDED_ADDRESS     ff08:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff08::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "organization-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_global
   ARGS                 ff0e::1 128 -c -n -g
   ADDRESS              ff0e::1
   EXPANDED_ADDRESS     ff0e:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff0e::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "global"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_unknown_scope
   ARGS                 ff03::1 128 -c -n -g
   ADDRESS              ff03::1
   EXPANDED_ADDRESS     ff03:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff03::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "unknown"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_non_transient
   ARGS                 ff02::1 128 -c -n -g
   ADDRESS              ff02::1
   EXPANDED_ADDRESS     ff02:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff02::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
)

SET_TESTS_PROPERTIES(
   ipv6_multicast_non_transient_text ipv6_multicast_non_transient_json
   ipv6_multicast_non_transient_yaml ipv6_multicast_non_transient_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Temporarily-allocated address;transient"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_transient
   ARGS                 ff12::1 128 -c -n -g
   ADDRESS              ff12::1
   EXPANDED_ADDRESS     ff12:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff12::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
   TRANSIENT            TRUE
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_transient_global
   ARGS                 ff1e::1 128 -c -n -g
   ADDRESS              ff1e::1
   EXPANDED_ADDRESS     ff1e:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff1e::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "global"
   MULTICAST_MAC        "33:33:00:00:00:01"
   TRANSIENT            TRUE
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_solicited_node
   ARGS                 ff02::1:ff11:2233 104 -c -n -g
   ADDRESS              ff02::1:ff11:2233
   EXPANDED_ADDRESS     ff02:0000:0000:0000:0000:0001:ff11:2233
   FAMILY               IPv6
   NETWORK              ff02::1:ff00:0
   PREFIX               104
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ff00:0
   WILDCARD             ::ff:ffff
   HOST_BITS            24
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:ff:11:22:33"
   SOLICITED_NODE_FOR   "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xx11:2233"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_solicited_node_128
   ARGS                 ff02::1:ff11:2233 128 -c -n -g
   ADDRESS              ff02::1:ff11:2233
   EXPANDED_ADDRESS     ff02:0000:0000:0000:0000:0001:ff11:2233
   FAMILY               IPv6
   NETWORK              ff02::1:ff11:2233
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:ff:11:22:33"
   SOLICITED_NODE_FOR   "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xx11:2233"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_non_solicited_node
   ARGS                 ff02:1::1:ff11:2233 128 -c -n -g
   ADDRESS              ff02:1::1:ff11:2233
   EXPANDED_ADDRESS     ff02:0001:0000:0000:0000:0001:ff11:2233
   FAMILY               IPv6
   NETWORK              ff02:1::1:ff11:2233
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:ff:11:22:33"
)

SET_TESTS_PROPERTIES(
   ipv6_multicast_non_solicited_node_text ipv6_multicast_non_solicited_node_json
   ipv6_multicast_non_solicited_node_yaml ipv6_multicast_non_solicited_node_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;solicited node multicast address for;solicited_node_for"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_ssm
   ARGS                 ff3e:0::123:4 128 -c -n -g
   ADDRESS              ff3e::123:4
   EXPANDED_ADDRESS     ff3e:0000:0000:0000:0000:0000:0123:0004
   FAMILY               IPv6
   NETWORK              ff3e::123:4
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "global"
   MULTICAST_MAC        "33:33:01:23:00:04"
   SSM                  TRUE
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_ssm_linklocal
   ARGS                 ff32::1 128 -c -n -g
   ADDRESS              ff32::1
   EXPANDED_ADDRESS     ff32:0000:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ff32::1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:00:00:00:01"
   SSM                  TRUE
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_non_ssm
   ARGS                 ff3e:1::123:4 128 -c -n -g
   ADDRESS              ff3e:1::123:4
   EXPANDED_ADDRESS     ff3e:0001:0000:0000:0000:0000:0123:0004
   FAMILY               IPv6
   NETWORK              ff3e:1::123:4
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "global"
   MULTICAST_MAC        "33:33:01:23:00:04"
)

SET_TESTS_PROPERTIES(
   ipv6_multicast_non_ssm_text ipv6_multicast_non_ssm_json
   ipv6_multicast_non_ssm_yaml ipv6_multicast_non_ssm_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;Source-specific multicast;source_specific_multicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_global_prefix_0
   ARGS                 2001:db8::/0 -c -n -g
   ADDRESS              2001:db8::
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0000
   FAMILY               IPv6
   NETWORK              ::
   PREFIX               0
   NETMASK              ::
   WILDCARD             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            128
   MAX_HOSTS            340282366920938463463374607431768211454
   HOST_MIN             ::1
   HOST_MAX             ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
   INTERFACE_ID         0000:0000:0000:0000
   SNMC                 ff02::1:ff00:0000
)


ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_multicast_low_snmc_word6
   ARGS                 ff02::1:0011:2233 128 -c -n -g
   ADDRESS              ff02::1:11:2233
   EXPANDED_ADDRESS     ff02:0000:0000:0000:0000:0001:0011:2233
   FAMILY               IPv6
   NETWORK              ff02::1:11:2233
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   TYPE                 multicast
   SCOPE                "link-local"
   MULTICAST_MAC        "33:33:00:11:22:33"
)

SET_TESTS_PROPERTIES(
   ipv6_multicast_low_snmc_word6_text ipv6_multicast_low_snmc_word6_json
   ipv6_multicast_low_snmc_word6_yaml ipv6_multicast_low_snmc_word6_xml
   PROPERTIES FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;: null;<null>;solicited node multicast address for;solicited_node_for"
)

# --- Arbitrary Bit Boundaries ----------------------------------------------
ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv6_p1
   ARGS                 2001:db8:abcd:1234::1 1 -c -n -g
   ADDRESS              2001:db8:abcd:1234::1
   EXPANDED_ADDRESS     2001:0db8:abcd:1234:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              ::
   PREFIX               1
   NETMASK              8000::
   WILDCARD             7fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            127
   MAX_HOSTS            170141183460469231731687303715884105727
   HOST_MIN             ::1
   HOST_MAX             7fff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv6_p16_rfc5952
   ARGS                 2001:db8::1 16 -c -n -g
   ADDRESS              2001:db8::1
   EXPANDED_ADDRESS     2001:0db8:0000:0000:0000:0000:0000:0001
   FAMILY               IPv6
   NETWORK              2001::
   PREFIX               16
   NETMASK              ffff::
   WILDCARD             0:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   HOST_BITS            112
   MAX_HOSTS            5192296858534827628530496329220095
   HOST_MIN             2001::1
   HOST_MAX             2001:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv6_p127
   ARGS                 2001:db8:abcd:1234:5678:9abc:def0:1 127 -c -n -g
   ADDRESS              2001:db8:abcd:1234:5678:9abc:def0:1
   EXPANDED_ADDRESS     2001:0db8:abcd:1234:5678:9abc:def0:0001
   FAMILY               IPv6
   NETWORK              2001:db8:abcd:1234:5678:9abc:def0:0
   PREFIX               127
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe
   WILDCARD             ::1
   HOST_BITS            1
   MAX_HOSTS            2
   HOST_MIN             2001:db8:abcd:1234:5678:9abc:def0:0
   HOST_MAX             2001:db8:abcd:1234:5678:9abc:def0:1
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 boundary_ipv6_p127_network
   ARGS                 2001:db8:abcd:1234:5678:9abc:def0:0 127 -c -n -g
   ADDRESS              2001:db8:abcd:1234:5678:9abc:def0:0
   EXPANDED_ADDRESS     2001:0db8:abcd:1234:5678:9abc:def0:0000
   FAMILY               IPv6
   NETWORK              2001:db8:abcd:1234:5678:9abc:def0:0
   PREFIX               127
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe
   WILDCARD             ::1
   HOST_BITS            1
   MAX_HOSTS            2
   HOST_MIN             2001:db8:abcd:1234:5678:9abc:def0:0
   HOST_MAX             2001:db8:abcd:1234:5678:9abc:def0:1
   TYPE                 network
   UNICAST_TYPE         "Global Unicast"
)


#############################################################################
#### CLI LOOKUP OPTIONS & EDGE CASES                                     ####
#############################################################################

ADD_SUBNETCALC_TEST(cli_nocolour_short            10.1.1.1 24 -c)
ADD_SUBNETCALC_TEST(cli_nocolour_long_uk          10.1.1.1 24 --nocolour)
ADD_SUBNETCALC_TEST(cli_nocolor_long_us           10.1.1.1 24 --nocolor)
ADD_SUBNETCALC_TEST(cli_noreverse_short           8.8.8.8 32 -n)
ADD_SUBNETCALC_TEST(cli_noreverse_long            8.8.8.8 32 --noreverselookup)
ADD_SUBNETCALC_TEST(cli_nogeoip_short             8.8.8.8 32 -g)
ADD_SUBNETCALC_TEST(cli_nogeoip_long              8.8.8.8 32 --nogeoiplookup)
ADD_SUBNETCALC_TEST(cli_combined_flags            2001:4860:4860::8888 128 -c -n -g)
ADD_SUBNETCALC_TEST(cli_combined_short            2001:4860:4860::8888 128 -cng)
ADD_SUBNETCALC_TEST(cli_double_dash_options       -c -n -g -- 192.168.1.1 24)
ADD_SUBNETCALC_TEST(cli_single_dash_long_opts     10.1.1.1/24 -nocolor -noreverselookup -nogeoiplookup)
ADD_SUBNETCALC_TEST(cli_family_override_v4        10.1.1.1/24 -6 -4 -c -n -g)
ADD_SUBNETCALC_TEST(cli_family_match_v4           10.1.1.1 24 -4 -c -n -g)
ADD_SUBNETCALC_TEST(cli_family_match_v6           2001:db8::1 64 -6 -c -n -g)
ADD_SUBNETCALC_TEST(cli_format_equals_json        10.1.1.1/24 --format=json -n -g)
ADD_SUBNETCALC_TEST(cli_format_equals_yaml        10.1.1.1/24 --format=yaml -n -g)
ADD_SUBNETCALC_TEST(cli_format_equals_xml         10.1.1.1/24 --format=xml -n -g)
ADD_SUBNETCALC_TEST(cli_format_equals_text        10.1.1.1/24 --format=text -c -n -g)
ADD_SUBNETCALC_TEST(cli_json_with_dns_block       127.0.0.1/8 -g -F json)
ADD_SUBNETCALC_TEST(cli_default_prefix_ipv4       192.168.1.1 -c -n -g)
ADD_SUBNETCALC_TEST(cli_default_prefix_ipv6       2001:db8::1 -c -n -g)
ADD_SUBNETCALC_TEST(cli_localhost_with_port       localhost:8080 -4 -c -n -g)
ADD_SUBNETCALC_TEST(cli_ipv4_with_port            192.168.1.1:8080/24 -c -n -g)
ADD_SUBNETCALC_TEST(cli_ipv4_port_zero            192.168.1.1:0/24 -c -n -g)
ADD_SUBNETCALC_TEST(cli_ipv6_bracket_no_port      "[2001:db8::1]/64" -c -n -g)
ADD_SUBNETCALC_TEST(cli_ipv6_bracket_with_port    "[2001:db8::1]:443/64" -c -n -g)
ADD_SUBNETCALC_TEST(cli_p2p_31_broadcast_text     10.1.1.0 31 -c -n -g)
ADD_SUBNETCALC_TEST(cli_colour_ipv4               192.168.1.1/24 -n -g)
ADD_SUBNETCALC_TEST(cli_colour_ipv6_eui64         fe80::0211:22ff:fe33:4455/64 -n -g)
ADD_SUBNETCALC_TEST(cli_nocolour_ipv6_eui64       fe80::0211:22ff:fe33:4455/64 -c -n -g)

SET_TESTS_PROPERTIES(
   cli_nocolour_short cli_nocolour_long_uk cli_nocolor_long_us
   cli_noreverse_short cli_noreverse_long cli_nogeoip_short
   cli_nogeoip_long cli_combined_flags cli_combined_short
   PROPERTIES LABELS "cli;options"
)

SET_TESTS_PROPERTIES(cli_double_dash_options PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 192\\.168\\.1\\.0 / 24"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_single_dash_long_opts PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 10\\.1\\.1\\.0 / 24"
   LABELS "cli;options;ipv4"
)

SET_TESTS_PROPERTIES(cli_family_override_v4 cli_family_match_v4 PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 10\\.1\\.1\\.0 / 24"
   LABELS "cli;options;ipv4"
)

SET_TESTS_PROPERTIES(cli_family_match_v6 PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 2001:db8:: / 64"
   LABELS "cli;options;ipv6"
)

SET_TESTS_PROPERTIES(cli_format_equals_json PROPERTIES
   PASS_REGULAR_EXPRESSION "\"address\":[ ]*\"10\\.1\\.1\\.1\""
   LABELS "cli;format_json"
)

SET_TESTS_PROPERTIES(cli_format_equals_yaml PROPERTIES
   PASS_REGULAR_EXPRESSION "address:[ ]*\"10\\.1\\.1\\.1\""
   LABELS "cli;format_yaml"
)

SET_TESTS_PROPERTIES(cli_format_equals_xml PROPERTIES
   PASS_REGULAR_EXPRESSION "<address>10\\.1\\.1\\.1</address>"
   LABELS "cli;format_xml"
)

SET_TESTS_PROPERTIES(cli_format_equals_text PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 10\\.1\\.1\\.1"
   LABELS "cli;format_text"
)

SET_TESTS_PROPERTIES(cli_json_with_dns_block PROPERTIES
   PASS_REGULAR_EXPRESSION "\"dns\":[ ]*\\{"
   LABELS "cli;ipv4;format_json"
)

SET_TESTS_PROPERTIES(cli_default_prefix_ipv4 PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 192\\.168\\.1\\.1 / 32"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_default_prefix_ipv6 PROPERTIES
   PASS_REGULAR_EXPRESSION "Network[ ]+= 2001:db8::1 / 128"
   LABELS "cli;ipv6"
)

SET_TESTS_PROPERTIES(cli_localhost_with_port PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 127\\.0\\.0\\.1.*Network[ ]+= 127\\.0\\.0\\.1 / 32"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_ipv4_with_port PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 192\\.168\\.1\\.1.*Network[ ]+= 192\\.168\\.1\\.0 / 24"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_ipv4_port_zero PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 192\\.168\\.1\\.1.*Network[ ]+= 192\\.168\\.1\\.0 / 24"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_ipv6_bracket_no_port PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 2001:db8::1.*Network[ ]+= 2001:db8:: / 64"
   LABELS "cli;ipv6"
)

SET_TESTS_PROPERTIES(cli_ipv6_bracket_with_port PROPERTIES
   PASS_REGULAR_EXPRESSION "Address[ ]+= 2001:db8::1.*Network[ ]+= 2001:db8:: / 64"
   LABELS "cli;ipv6"
)

SET_TESTS_PROPERTIES(cli_p2p_31_broadcast_text PROPERTIES
   PASS_REGULAR_EXPRESSION "Broadcast[ ]+= not needed on Point-to-Point links"
   LABELS "cli;ipv4"
)

STRING(ASCII 27 ESC)

ADD_SUBNETCALC_TEST(cli_nocolour_ipv4_binary       10.1.1.1/24 -c -n -g)
SET_TESTS_PROPERTIES(cli_nocolour_ipv4_binary PROPERTIES
   PASS_REGULAR_EXPRESSION "00001010 \. 00000001 \. 00000001 \. 00000001"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;${ESC}"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_colour_ipv4 PROPERTIES
   PASS_REGULAR_EXPRESSION "${ESC}\\[34m1${ESC}\\[0m.*${ESC}\\[33m1${ESC}\\[0m"
   LABELS "cli;ipv4"
)

SET_TESTS_PROPERTIES(cli_colour_ipv6_eui64 PROPERTIES
   PASS_REGULAR_EXPRESSION "Interface ID[ ]+= ${ESC}\\[36m0211:22${ESC}\\[37mff:fe${ESC}\\[38m33:4455${ESC}\\[0m.*MAC Address[ ]+= ${ESC}\\[36m00:11:22${ESC}\\[0m:${ESC}\\[38m33:44:55${ESC}\\[0m.*Solicited Node Multicast Address[ ]+= ${ESC}\\[32mff02::1:ff${ESC}\\[38m33:4455${ESC}\\[0m"
   LABELS "cli;ipv6"
)

SET_TESTS_PROPERTIES(cli_nocolour_ipv6_eui64 PROPERTIES
   PASS_REGULAR_EXPRESSION "Interface ID[ ]+= 0211:22ff:fe33:4455.*MAC Address[ ]+= 00:11:22:33:44:55"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;${ESC}"
   LABELS "cli;ipv6"
)


#############################################################################
#### ULA GENERATION                                                      ####
#############################################################################

ADD_SUBNETCALC_TEST(ula_gen_urandom_short    fd01:1122:3344:affe:0102:03ff:fe04:0506 64 -u)
ADD_SUBNETCALC_TEST(ula_gen_urandom_long     fd01:1122:3344:affe:0102:03ff:fe04:0506 64 --uniquelocal)
ADD_SUBNETCALC_TEST(ula_gen_random_short     fd01:1122:3344:affe:0102:03ff:fe04:0506 64 -U)
ADD_SUBNETCALC_TEST(ula_gen_random_long      fd01:1122:3344:affe:0102:03ff:fe04:0506 64 --uniquelocalhq)
ADD_SUBNETCALC_TEST(ula_gen_slash_input      fd01:1122:3344:affe::1/64 -u)
ADD_SUBNETCALC_TEST(ula_gen_from_unspecified :: 64 -u -c -n -g)
ADD_SUBNETCALC_TEST(ula_gen_json             fd01:1122:3344:affe::1/64 -u -F json)
ADD_SUBNETCALC_TEST(ula_gen_yaml             fd01:1122:3344:affe::1/64 -u -F yaml)
ADD_SUBNETCALC_TEST(ula_gen_xml              fd01:1122:3344:affe::1/64 -u -F xml)

SET_TESTS_PROPERTIES(
   ula_gen_urandom_short ula_gen_urandom_long ula_gen_random_short
   ula_gen_random_long ula_gen_slash_input ula_gen_from_unspecified
   PROPERTIES
      PASS_REGULAR_EXPRESSION "Generating Unique Local IPv6 address.*Unique Local Unicast Properties"
      LABELS "ipv6;ula"
)

SET_TESTS_PROPERTIES(ula_gen_json PROPERTIES
   PASS_REGULAR_EXPRESSION "^\\{.*\"address\":[ ]*\"fd[0-9a-f][0-9a-f]:"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;Generating Unique Local"
   LABELS "ipv6;ula;format_json"
)

SET_TESTS_PROPERTIES(ula_gen_yaml PROPERTIES
   PASS_REGULAR_EXPRESSION "^address:[ ]*\"fd[0-9a-f][0-9a-f]:"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;Generating Unique Local"
   LABELS "ipv6;ula;format_yaml"
)

SET_TESTS_PROPERTIES(ula_gen_xml PROPERTIES
   PASS_REGULAR_EXPRESSION "^<\\?xml.*<address>fd[0-9a-f][0-9a-f]:"
   FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:;Generating Unique Local"
   LABELS "ipv6;ula;format_xml"
)


#############################################################################
#### DNS AND IDN RESOLUTION                                              ####
#############################################################################

# Standard ASCII Domain Names
ADD_SUBNETCALC_DNS_TEST(dns_ascii_heise             www.heise.de 24 -4)
ADD_SUBNETCALC_DNS_TEST(dns_ascii_heise_slash       www.heise.de/24 -4)
ADD_SUBNETCALC_DNS_TEST(dns_ascii_google            google.com)

# German IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_buecher        www.xn--bcher-kva.de)
ADD_SUBNETCALC_DNS_TEST(dns_idn_buecher             www.bücher.de)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_muenchen       www.xn--mnchen-3ya.de/24 -4)
ADD_SUBNETCALC_DNS_TEST(dns_idn_muenchen            www.münchen.de/24 -4)

# Norwegian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_snoe           www.xn--sn-mka.no)
ADD_SUBNETCALC_DNS_TEST(dns_idn_snoe                www.snø.no)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_blaa           www.xn--bl-zia.no/24 -4)
ADD_SUBNETCALC_DNS_TEST(dns_idn_blaa                www.blå.no/24 -4)

# Swedish IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_raek           xn--rksmrgs-5wao1o.se)
ADD_SUBNETCALC_DNS_TEST(dns_idn_raek                räksmörgås.se)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_goeteborg      www.xn--gteborg-90a.se/24 -4)
ADD_SUBNETCALC_DNS_TEST(dns_idn_goeteborg           www.göteborg.se/24 -4)

# Russian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_president      xn--d1abbgf6aiiy.xn--p1ai)
ADD_SUBNETCALC_DNS_TEST(dns_idn_president           президент.рф)

# Chinese IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_renmin         xn--gmq282eogn.cn/24 -4 -n)
ADD_SUBNETCALC_DNS_TEST(dns_idn_renmin              人民网.cn/24 -4 -n)

# Japanese IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_nihongo        xn--wgv71a119e.jp)
ADD_SUBNETCALC_DNS_TEST(dns_idn_nihongo             日本語.jp)

# Georgian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_horoscope      xn--todblabgkj5k.xn--node)
ADD_SUBNETCALC_DNS_TEST(dns_idn_horoscope           ჰოროსკოპი.გე)

# Arabic IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_dubai          xn--ngbn6f.xn--mgbaam7a8h)
ADD_SUBNETCALC_DNS_TEST(dns_idn_dubai               دبي.امارات)

# Hebrew IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_isoc           xn----zhcbgfhe2aacg8fb5i.xn--4dbrk0ce)
ADD_SUBNETCALC_DNS_TEST(dns_idn_isoc                איגוד-האינטרנט.ישראל)

# Indian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_india          xn--11b3cgab9b4bm5d.xn--h2brj9c)
ADD_SUBNETCALC_DNS_TEST(dns_idn_india               मेरीसरकार.भारत)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_india_mha      xn--i1b5bzbybhfo5c8b4bxh.xn--11b7cb3a6a.xn--h2brj9c -n)
ADD_SUBNETCALC_DNS_TEST(dns_idn_india_mha           गृहमंत्रालय.सरकार.भारत -n)

# Greek IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_greece         www.xn--qxaa9ba.gr -n)
ADD_SUBNETCALC_DNS_TEST(dns_idn_greece              www.εεττ.gr -n)

# Farsi IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_farsi          xn--mgbu7dsvrfc.xn--mgba3a4f16a)
ADD_SUBNETCALC_DNS_TEST(dns_idn_farsi               ایرنیک.ایران)

# Korean IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_korea          xn--3e0bx5e6xzftae3gxzpskhile.xn--3e0b707e)
ADD_SUBNETCALC_DNS_TEST(dns_idn_korea               한국인터넷진흥원.한국)

# Slovakian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_slovakia       xn--preov-xdb.eu)
ADD_SUBNETCALC_DNS_TEST(dns_idn_slovakia            prešov.eu)


#############################################################################
#### FORCED ADDRESS FAMILY                                               ####
#############################################################################

ADD_SUBNETCALC_DNS_TEST(force_ipv4_nntb             www.nntb.no 24 -4)
ADD_SUBNETCALC_DNS_TEST(force_ipv4_nntb_slash       www.nntb.no/24 --ipv4)
ADD_SUBNETCALC_DNS_TEST(force_ipv4_heise            www.heise.de 24 -4)
ADD_SUBNETCALC_DNS_TEST(force_ipv4_heise_slash      www.heise.de/24 --ipv4)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_nntb             www.nntb.no 64 -6)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_nntb_slash       www.nntb.no/64 --ipv6)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_heise            www.heise.de 64 -6)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_heise_slash      www.heise.de/64 --ipv6)
ADD_SUBNETCALC_DNS_TEST(force_ipv4_nntb_cng         www.nntb.no 24 -4 -n -g)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_nntb_cng         www.nntb.no 64 -6 -n -g)
ADD_SUBNETCALC_DNS_TEST(force_ipv4_heise_cn         www.heise.de 24 -4 -c -n)
ADD_SUBNETCALC_DNS_TEST(force_ipv6_heise_cn         www.heise.de 64 -6 -c -n)


#############################################################################
#### ERROR HANDLING                                                      ####
#############################################################################

ADD_SUBNETCALC_FAIL_TEST(fail_no_args)
ADD_SUBNETCALC_FAIL_TEST(fail_too_many_args               10.1.1.1 24 255.255.255.0)
ADD_SUBNETCALC_FAIL_TEST(fail_missing_format_arg_short    10.1.1.1 24 -F)
ADD_SUBNETCALC_FAIL_TEST(fail_missing_format_arg_long     10.1.1.1 24 --format)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_octet_256              256.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_extra_octet            10.1.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_double_dot             10.1..1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_trailing_dot           10.1.1.1. 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_leading_dot            .10.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_shorthand_trail_dot    10.1. 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_shorthand_1part_ovf    4294967296 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_shorthand_ovf_1        256.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_shorthand_ovf_2        10.16777216 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_shorthand_ovf_3        10.1.65536 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_prefix_33              10.1.1.1 33)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_plus_prefix            10.1.1.1/+24)
ADD_SUBNETCALC_FAIL_TEST(fail_prefix_int_overflow         10.1.1.1 999999999999999999999)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_alpha_prefix           10.1.1.1 24a)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_negative_prefix        10.1.1.1 -5)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_mask_gaps              10.1.1.1 255.0.255.0)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_mask_octet_256         10.1.1.1 255.255.255.256)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_bad_hex                fd00::ghjk 64)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_triple_colon           fd00:::1 64)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_prefix_129             fd00::1 129)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_mask_word_gaps         fd00::1 ffff:0:ffff::)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_mask_partial_word_gap  fd00::1 ffff:8000:1::)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_mask_bit_gaps          fd00::1 ff0f::)
ADD_SUBNETCALC_FAIL_TEST(fail_mismatch_v4_v6              10.1.1.1 ffff:ffff:ffff::)
ADD_SUBNETCALC_FAIL_TEST(fail_mismatch_v6_v4              fd00::1 255.255.255.0)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v4_on_ipv6_lit        2001:db8::1 64 -4)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v6_on_ipv4_lit        10.1.1.1 24 -6)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v4_v6_pref_1          www.heise.de 64 -4)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v4_v6_pref_2          www.nntb.no 64 -4)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v6_v4_mask_1          www.heise.de 255.255.255.0 -6)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v6_v4_mask_2          www.nntb.no 255.255.255.0 -6)
ADD_SUBNETCALC_FAIL_TEST(fail_v4_with_ula                 10.1.1.1 24 -u)
ADD_SUBNETCALC_FAIL_TEST(fail_v4_with_ula_hq              10.1.1.1 24 -U)
ADD_SUBNETCALC_FAIL_TEST(fail_double_slash                10.1.1.1/24/32)
ADD_SUBNETCALC_FAIL_TEST(fail_empty_prefix_slash          10.1.1.1/)
ADD_SUBNETCALC_FAIL_TEST(fail_empty_address_slash         /24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_empty_port             10.1.1.1:/24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_port_alpha             10.1.1.1:80abc/24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_port_overflow          10.1.1.1:99999/24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_unclosed_bracket       "[2001:db8::1/64")
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_trailing_bracket       "[2001:db8::1]foo/64")
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_empty_bracket_port     "[2001:db8::1]:/64")
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_bracket_port_overflow  "[::1]:65536/128")
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_netmask_bracket_port   "2001:db8::1/[ffff:ffff:ffff:ffff::]:80")
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_netmask_bracket_trail  "2001:db8::1/[ffff:ffff:ffff:ffff::]foo")
ADD_SUBNETCALC_FAIL_TEST(fail_netmask_with_port           10.1.1.1/255.255.255.0:80)
ADD_SUBNETCALC_FAIL_TEST(fail_nonexistent_domain          invalid.domain.test 24)
ADD_SUBNETCALC_FAIL_TEST(fail_unknown_flag                10.1.1.1 24 --invalid-flag)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_csv          10.1.1.1 24 -F csv)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_html         10.1.1.1 24 --format html)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_uppercase_json 10.1.1.1 24 -F JSON)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_uppercase_yaml 10.1.1.1 24 --format=YAML)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_uppercase_xml  10.1.1.1 24 --format=XML)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_mixed_text     10.1.1.1 24 --format=Text)
