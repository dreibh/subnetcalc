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
// Copyright (C) 2024-2026 by Thomas Dreibholz
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

#include <chrono>
#include <cstdint>
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

inline unsigned long long getMicroTime()
{
   return static_cast<unsigned long long>(
      std::chrono::duration_cast<std::chrono::microseconds>(
         std::chrono::system_clock::now().time_since_epoch()).count());
}

union sockaddr_union {
   struct sockaddr     sa;
   struct sockaddr_in  in;
   struct sockaddr_in6 in6;
};

bool checkIPv6();
bool hasTranslationPrefix(const sockaddr_in6* address);
size_t getSocklen(const struct sockaddr* address);

std::string address2string(const struct sockaddr* address,
                           bool                   port      = false,
                           bool                   hideScope = false);

bool string2address(std::string_view      string,
                    union sockaddr_union* address,
                    bool                  readPort = true);

void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  bool                   port      = true,
                  bool                   hideScope = false);

// Standard C++20 Formatter for sockaddr_union
template<>
struct std::formatter<sockaddr_union> : std::formatter<std::string_view> {
   auto format(const sockaddr_union& addr, std::format_context& ctx) const {
      return std::formatter<std::string_view>::format(address2string(&addr.sa, false, true), ctx);
   }
};

#endif
