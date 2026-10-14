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

SET(TEST_LOCALE_DIR "${CMAKE_BINARY_DIR}/locale")


# ###### Helper for deterministic commands (expected return code: 0) #######
FUNCTION(ADD_SUBNETCALC_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      ENVIRONMENT "LC_ALL=C;SUBNETCALC_LOCALEDIR=${TEST_LOCALE_DIR}"
      FAIL_REGULAR_EXPRESSION "INTERNAL ERROR;ERROR:"
   )
ENDFUNCTION()


# ###### Helper for network/DNS lookups (Skips test if offline/unresolved) #
FUNCTION(ADD_SUBNETCALC_DNS_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      ENVIRONMENT "LC_ALL=C;SUBNETCALC_LOCALEDIR=${TEST_LOCALE_DIR}"
      PASS_REGULAR_EXPRESSION "Address[ ]+= "
      FAIL_REGULAR_EXPRESSION "INTERNAL ERROR"
      SKIP_REGULAR_EXPRESSION "ERROR: Invalid address"
      LABELS "network;dns"
   )
ENDFUNCTION()


# ###### Helper for expected failures (expected return code: non-zero) ######
FUNCTION(ADD_SUBNETCALC_FAIL_TEST TEST_NAME)
   ADD_TEST(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:subnetcalc> ${ARGN}
   )
   SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES
      WILL_FAIL TRUE
      ENVIRONMENT "LC_ALL=C;SUBNETCALC_LOCALEDIR=${TEST_LOCALE_DIR}"
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
      IF(ARG_MAX_HOSTS EQUAL 0)
         STRING(APPEND T_PARTS ".*Max\\. Hosts[ ]+= 2\\^0 - 0")
      ELSE()
         STRING(APPEND T_PARTS ".*Max\\. Hosts[ ]+= ${ARG_MAX_HOSTS}")
      ENDIF()
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND T_PARTS ".*Host Range[ ]+= \\{[ ]*${ARG_HOST_MIN} - ${ARG_HOST_MAX}[ ]*\\}")
   ENDIF()
   IF(ARG_CLASS)
      STRING(APPEND T_PARTS ".*${ESC_CLASS}")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND T_PARTS ".*${ARG_FLAG}")
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
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND T_PARTS ".*${ARG_UNICAST_TYPE}")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND T_PARTS ".*${ARG_ULA_TYPE}")
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
      STRING(APPEND J_PARTS ".*\"max_hosts\":[ ]*${ARG_MAX_HOSTS}")
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND J_PARTS ".*\"min\":[ ]*\"${ARG_HOST_MIN}\".*\"max\":[ ]*\"${ARG_HOST_MAX}\"")
   ENDIF()
   STRING(APPEND J_PARTS ".*\"type\":[ ]*\"${ARG_TYPE}\"")
   IF(ARG_CLASS)
      STRING(APPEND J_PARTS ".*\"class\":[ ]*\"${ESC_CLASS}\"")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND J_PARTS ".*\"${ARG_FLAG}\"")
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
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND J_PARTS ".*\"unicast_type\":[ ]*\"${ARG_UNICAST_TYPE}\"")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND J_PARTS ".*\"ula_type\":[ ]*\"${ARG_ULA_TYPE}\"")
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
      STRING(APPEND Y_PARTS ".*max_hosts:[ ]*${ARG_MAX_HOSTS}")
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND Y_PARTS ".*min:[ ]*\"${ARG_HOST_MIN}\".*max:[ ]*\"${ARG_HOST_MAX}\"")
   ENDIF()
   STRING(APPEND Y_PARTS ".*type:[ ]*\"${ARG_TYPE}\"")
   IF(ARG_CLASS)
      STRING(APPEND Y_PARTS ".*class:[ ]*\"${ESC_CLASS}\"")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND Y_PARTS ".*-[ ]*\"${ARG_FLAG}\"")
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
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND Y_PARTS ".*unicast_type:[ ]*\"${ARG_UNICAST_TYPE}\"")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND Y_PARTS ".*ula_type:[ ]*\"${ARG_ULA_TYPE}\"")
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
      STRING(APPEND X_PARTS ".*<max_hosts>${ARG_MAX_HOSTS}</max_hosts>")
   ENDIF()
   IF(ARG_HOST_MIN AND ARG_HOST_MAX)
      STRING(APPEND X_PARTS ".*<min>${ARG_HOST_MIN}</min>.*<max>${ARG_HOST_MAX}</max>")
   ENDIF()
   STRING(APPEND X_PARTS ".*<type>${ARG_TYPE}</type>")
   IF(ARG_CLASS)
      STRING(APPEND X_PARTS ".*<class>${ESC_CLASS}</class>")
   ENDIF()
   IF(ARG_FLAG)
      STRING(APPEND X_PARTS ".*<flag>${ARG_FLAG}</flag>")
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
   IF(ARG_UNICAST_TYPE)
      STRING(APPEND X_PARTS ".*<unicast_type>${ARG_UNICAST_TYPE}</unicast_type>")
   ENDIF()
   IF(ARG_ULA_TYPE)
      STRING(APPEND X_PARTS ".*<ula_type>${ARG_ULA_TYPE}</ula_type>")
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
         ENVIRONMENT "LC_ALL=C;SUBNETCALC_LOCALEDIR=${TEST_LOCALE_DIR}"
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

SET_TESTS_PROPERTIES(cli_version_short cli_version_long PROPERTIES
   PASS_REGULAR_EXPRESSION "SubNetCalc [0-9]+\\.[0-9]+"
)
SET_TESTS_PROPERTIES(cli_help_short cli_help_long PROPERTIES
   PASS_REGULAR_EXPRESSION "Usage:.*subnetcalc.*address/prefix"
)
SET_TESTS_PROPERTIES(
   cli_version_short cli_version_long cli_help_short cli_help_long
   PROPERTIES LABELS "cli"
)


#############################################################################
#### IPV4 MATRIX TESTS                                                   ####
#############################################################################

# --- Private Subnets -------------------------------------------------------
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
   CLASS                "Invalid (not in class A, B, C or D)"
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
   MAX_HOSTS            0
   HOST_MIN             255.255.255.255
   HOST_MAX             255.255.255.255
   TYPE                 host
   CLASS                "Invalid (not in class A, B, C or D)"
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
   HOST_MAX             127.0.0.254
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
   MAX_HOSTS            0
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
   MAX_HOSTS            0
   HOST_MIN             ::1
   HOST_MAX             ::1
   TYPE                 host
   SPECIAL              "Loopback address"
)

ADD_SUBNETCALC_MATRIX_TEST(
   NAME                 ipv6_v4compat
   ARGS                 ::192.0.2.1 128 -c -n -g
   ADDRESS              ::192.0.2.1
   EXPANDED_ADDRESS     0000:0000:0000:0000:0000:0000:c000:0201
   FAMILY               IPv6
   NETWORK              ::192.0.2.1
   PREFIX               128
   NETMASK              ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
   WILDCARD             ::
   HOST_BITS            0
   MAX_HOSTS            0
   HOST_MIN             ::192.0.2.1
   HOST_MAX             ::192.0.2.1
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
   MAX_HOSTS            0
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
   WILDCARD             ::255.255.255.255
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
   WILDCARD             ::255.255.255.255
   HOST_BITS            32
   MAX_HOSTS            4294967295
   HOST_MIN             64:ff9b:1:2:3:4:0.0.0.1
   HOST_MAX             64:ff9b:1:2:3:4:255.255.255.255
   TYPE                 host
   SPECIAL              "IPv4-embedded IPv6 address"
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
   MAX_HOSTS            1
   HOST_MIN             2001:db8:abcd:1234:5678:9abc:def0:1
   HOST_MAX             2001:db8:abcd:1234:5678:9abc:def0:1
   TYPE                 host
   UNICAST_TYPE         "Global Unicast"
)


#############################################################################
#### CLI LOOKUP OPTIONS                                                  ####
#############################################################################

ADD_SUBNETCALC_TEST(cli_nocolour_short       10.1.1.1 24 -c)
ADD_SUBNETCALC_TEST(cli_nocolour_long_uk     10.1.1.1 24 --nocolour)
ADD_SUBNETCALC_TEST(cli_nocolor_long_us      10.1.1.1 24 --nocolor)
ADD_SUBNETCALC_TEST(cli_noreverse_short      8.8.8.8 32 -n)
ADD_SUBNETCALC_TEST(cli_noreverse_long       8.8.8.8 32 --noreverselookup)
ADD_SUBNETCALC_TEST(cli_nogeoip_short        8.8.8.8 32 -g)
ADD_SUBNETCALC_TEST(cli_nogeoip_long         8.8.8.8 32 --nogeoiplookup)
ADD_SUBNETCALC_TEST(cli_combined_flags       2001:4860:4860::8888 128 -c -n -g)
ADD_SUBNETCALC_TEST(cli_combined_short       2001:4860:4860::8888 128 -cng)

SET_TESTS_PROPERTIES(
   cli_nocolour_short cli_nocolour_long_uk cli_nocolor_long_us
   cli_noreverse_short cli_noreverse_long cli_nogeoip_short
   cli_nogeoip_long cli_combined_flags cli_combined_short
   PROPERTIES LABELS "cli;options"
)


#############################################################################
#### ULA GENERATION                                                      ####
#############################################################################

ADD_SUBNETCALC_TEST(ula_gen_urandom_short    fd01:1122:3344:affe:0102:03ff:fe04:0506 64 -u)
ADD_SUBNETCALC_TEST(ula_gen_urandom_long     fd01:1122:3344:affe:0102:03ff:fe04:0506 64 --uniquelocal)
ADD_SUBNETCALC_TEST(ula_gen_random_short     fd01:1122:3344:affe:0102:03ff:fe04:0506 64 -U)
ADD_SUBNETCALC_TEST(ula_gen_random_long      fd01:1122:3344:affe:0102:03ff:fe04:0506 64 --uniquelocalhq)
ADD_SUBNETCALC_TEST(ula_gen_slash_input      fd01:1122:3344:affe::1/64 -u)

SET_TESTS_PROPERTIES(
   ula_gen_urandom_short ula_gen_urandom_long ula_gen_random_short
   ula_gen_random_long ula_gen_slash_input
   PROPERTIES
      PASS_REGULAR_EXPRESSION "Generating Unique Local IPv6 address.*Unique Local Unicast Properties"
      LABELS "ipv6;ula"
)


#############################################################################
#### DNS AND IDN RESOLUTION                                              ####
#############################################################################

# Standard ASCII Domain Names
ADD_SUBNETCALC_DNS_TEST(dns_ascii_heise             www.heise.de 24)
ADD_SUBNETCALC_DNS_TEST(dns_ascii_heise_slash       www.heise.de/24)
ADD_SUBNETCALC_DNS_TEST(dns_ascii_google            google.com 32)

# German IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_buecher        www.xn--bcher-kva.de 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_buecher             www.bücher.de 24)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_muenchen       www.xn--mnchen-3ya.de/24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_muenchen            www.münchen.de/24)

# Norwegian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_snoe           www.xn--sn-mka.no 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_snoe                www.snø.no 24)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_blaa           www.xn--bl-zia.no/24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_blaa                www.blå.no/24)

# Swedish IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_raek           xn--rksmrgs-5wao1o.se 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_raek                räksmörgås.se 24)
ADD_SUBNETCALC_DNS_TEST(dns_punycode_goeteborg      www.xn--gteborg-90a.se/24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_goeteborg           www.göteborg.se/24)

# Russian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_president      xn--d1abbgf6aiiy.xn--p1ai 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_president           президент.рф 24)

# Chinese IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_renmin         xn--gmq282eogn.cn/24 -n)
ADD_SUBNETCALC_DNS_TEST(dns_idn_renmin              人民网.cn/24 -n)

# Japanese IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_nihongo        xn--wgv71a119e.jp 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_nihongo             日本語.jp 24)

# Georgian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_horoscope      xn--todblabgkj5k.xn--node 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_horoscope           ჰოროსკოპი.გე 24)

# Arabic IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_dubai          xn--ngbn6f.xn--mgbaam7a8h 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_dubai               دبي.امارات 24)

# Hebrew IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_isoc           xn----zhcbgfhe2aacg8fb5i.xn--4dbrk0ce 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_isoc                איגוד-האינטרנט.ישראל 24)

# Indian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_india          xn--i1bj3fqcyde.xn--11b7cb3a6a.xn--h2brj9c 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_india               इंडिया.सरकार.भारत 24)

# Greek IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_greece         www.xn--qxaa9ba.gr 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_greece              www.εεττ.gr 24)

# Farsi IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_farsi          xn--mgbu7dsvrfc.xn--mgba3a4f16a 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_farsi               ایرنیک.ایران 24)

# Korean IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_korea          xn--3e0bx5e6xzftae3gxzpskhile.xn--3e0b707e 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_korea               한국인터넷진흥원.한국 24)

# Slovakian IDN: Punycode then IDN
ADD_SUBNETCALC_DNS_TEST(dns_punycode_slovakia       xn--preov-xdb.eu 24)
ADD_SUBNETCALC_DNS_TEST(dns_idn_slovakia            prešov.eu 24)


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
ADD_SUBNETCALC_FAIL_TEST(fail_too_many_args         10.1.1.1 24 255.255.255.0)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_octet_256        256.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_extra_octet      10.1.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_double_dot       10.1..1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_trailing_dot     10.1.1.1. 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_leading_dot      .10.1.1.1 24)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_prefix_33        10.1.1.1 33)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_alpha_prefix     10.1.1.1 24a)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_negative_prefix  10.1.1.1 -5)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_mask_gaps        10.1.1.1 255.0.255.0)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv4_mask_octet_256   10.1.1.1 255.255.255.256)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_bad_hex          fd00::ghjk 64)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_triple_colon     fd00:::1 64)
ADD_SUBNETCALC_FAIL_TEST(fail_ipv6_prefix_129       fd00::1 129)
ADD_SUBNETCALC_FAIL_TEST(fail_mismatch_v4_v6        10.1.1.1 ffff:ffff:ffff::)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v4_v6_pref_1    www.heise.de 64 -4)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v4_v6_pref_2    www.nntb.no 64 -4)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v6_v4_mask_1    www.heise.de 255.255.255.0 -6)
ADD_SUBNETCALC_FAIL_TEST(fail_force_v6_v4_mask_2    www.nntb.no 255.255.255.0 -6)
ADD_SUBNETCALC_FAIL_TEST(fail_v4_with_ula           10.1.1.1 24 -u)
ADD_SUBNETCALC_FAIL_TEST(fail_double_slash          10.1.1.1/24/32)
ADD_SUBNETCALC_FAIL_TEST(fail_nonexistent_domain    invalid.domain.test 24)
ADD_SUBNETCALC_FAIL_TEST(fail_unknown_flag          10.1.1.1 24 --invalid-flag)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_csv    10.1.1.1 24 -F csv)
ADD_SUBNETCALC_FAIL_TEST(fail_invalid_format_html   10.1.1.1 24 --format html)

SET_TESTS_PROPERTIES(
   fail_no_args fail_too_many_args fail_ipv4_octet_256 fail_ipv4_extra_octet
   fail_ipv4_double_dot fail_ipv4_trailing_dot fail_ipv4_leading_dot
   fail_ipv4_prefix_33 fail_ipv4_alpha_prefix fail_ipv4_negative_prefix
   fail_ipv4_mask_gaps fail_ipv4_mask_octet_256 fail_ipv6_bad_hex
   fail_ipv6_triple_colon fail_ipv6_prefix_129 fail_mismatch_v4_v6
   fail_force_v4_v6_pref_1 fail_force_v4_v6_pref_2 fail_force_v6_v4_mask_1
   fail_force_v6_v4_mask_2 fail_v4_with_ula fail_double_slash
   fail_nonexistent_domain fail_unknown_flag fail_invalid_format_csv
   fail_invalid_format_html
   PROPERTIES LABELS "negative;error"
)
