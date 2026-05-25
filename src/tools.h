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

#include <cstdarg>
#include <iosfwd>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


unsigned long long getMicroTime();


bool safestrcpy(char* dest, const char* src, const size_t size);
bool safestrcat(char* dest, const char* src, const size_t size);


union sockaddr_union {
   struct sockaddr          sa;
   struct sockaddr_in       in;
   struct sockaddr_in6      in6;
};

bool checkIPv6();
bool hasTranslationPrefix(const sockaddr_in6* address);
size_t getSocklen(const struct sockaddr* address);
bool address2string(const struct sockaddr* address,
                    char*                  buffer,
                    const size_t           length,
                    const bool             port,
                    const bool             hideScope = false);
bool string2address(const char*           string,
                    union sockaddr_union* address,
                    const bool            readPort = true);

void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  const bool             port      = true,
                  const bool             hideScope = false);
std::string format(const char* fmt, ...);

#endif
