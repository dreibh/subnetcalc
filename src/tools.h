// ==========================================================================
//             ____        _     _   _      _    ____      _
//            / ___| _   _| |__ | \ | | ___| |_ / ___|__ _| | ___
//            \___ \| | | | '_ \|  \| |/ _ \ __| |   / _` | |/ __|
//             ___) | |_| | |_) | |\  |  __/ |_| |__| (_| | | (__
//            |____/ \__,_|_.__/|_| \_|\___|\__|\____\__,_|_|\___|
//
//                    ---  IPv4/IPv6 Subnet Calculator  ---
//                   https://www.nntb.no/~dreibh/subnetcalc/
// ==========================================================================
//
// SubNetCalc - IPv4/IPv6 Subnet Calculator
// Copyright (C) 2009-2026 by Thomas Dreibholz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Contact: thomas.dreibholz@gmail.com

#ifndef TOOLS_H
#define TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <iosfwd>
#include <string>
#include <string_view>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using in_addr_t = uint32_t;
#ifndef IN_LOOPBACKNET
#define IN_LOOPBACKNET 127
#endif
#endif


union sockaddr_union {
   struct sockaddr     sa;
   struct sockaddr_in  in;
   struct sockaddr_in6 in6;
};

sockaddr_union operator&(const sockaddr_union& address1,
                         const sockaddr_union& address2);
sockaddr_union operator|(const sockaddr_union& address1,
                         const sockaddr_union& address2);
sockaddr_union operator~(const sockaddr_union& address1);
sockaddr_union operator+(const sockaddr_union& address1, uint32_t n);
sockaddr_union operator-(const sockaddr_union& address1, uint32_t n);
bool operator==(const sockaddr_union& address1,
                const sockaddr_union& address2);

inline bool isIPv4(const sockaddr_union& address) {
   return (address.sa.sa_family == AF_INET);
}

inline in_addr_t getIPv4Address(const sockaddr_union& address) {
   assert(address.sa.sa_family == AF_INET);
   return address.in.sin_addr.s_addr;
}

inline in6_addr getIPv6Address(const sockaddr_union& address) {
   assert(address.sa.sa_family == AF_INET6);
   return address.in6.sin6_addr;
}

inline bool isMulticast(const sockaddr_union& address) {
   if(isIPv4(address)) {
      return IN_CLASSD(ntohl(getIPv4Address(address)));
   }
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      return IN6_IS_ADDR_MULTICAST(&ipv6address);
   }
}


bool checkIPv6();
bool hasTranslationPrefix(const sockaddr_in6* address);
size_t getSocklen(const struct sockaddr* address);

std::string address2string(const sockaddr_union& address,
                           bool                  port      = false,
                           bool                  hideScope = true);
std::string mask2string(const sockaddr_union& mask);

bool string2address(std::string_view      string,
                    union sockaddr_union* address,
                    bool                  readPort      = true,
                    int                   addressFamily = AF_UNSPEC);


template<>
struct std::formatter<sockaddr_union> : std::formatter<std::string_view> {
   auto format(const sockaddr_union& address,
               std::format_context&  context) const {
      return std::formatter<std::string_view>::format(
                address2string(address, false, true), context);
   }
};

#endif
