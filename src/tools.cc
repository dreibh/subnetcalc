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

#include "tools.h"

#include <charconv>
#include <cstring>
#include <format>
#include <iostream>

#if !defined(_WIN32)
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#else
#include <windows.h>
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE
#endif
#endif

#ifndef NI_IDN
#include <idn2.h>
#endif


// ###### Check for support of IPv6 #########################################
bool checkIPv6()
{
#if !defined(_WIN32)
   int sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
   if(sd >= 0) {
      close(sd);
      return true;
   }
#else
   SOCKET sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
   if(sd != INVALID_SOCKET) {
      closesocket(sd);
      return true;
   }
#endif
   return false;
}


// ###### Does the given address have a translation prefix? #################
bool hasTranslationPrefix(const sockaddr_in6* address)
{
   const uint16_t word0 = (address->sin6_addr.s6_addr[0] << 8) | address->sin6_addr.s6_addr[1];
   const uint16_t word1 = (address->sin6_addr.s6_addr[2] << 8) | address->sin6_addr.s6_addr[3];
   const uint16_t word2 = (address->sin6_addr.s6_addr[4] << 8) | address->sin6_addr.s6_addr[5];

   return (word0 == 0x64) && (word1 == 0xff9b) && (word2 <= 1);
}


// ###### Get socklen for given address #####################################
size_t getSocklen(const struct sockaddr* address)
{
   switch(address->sa_family) {
      case AF_INET:  return sizeof(struct sockaddr_in);
      case AF_INET6: return sizeof(struct sockaddr_in6);
      default:       return sizeof(struct sockaddr);
   }
}


// ###### Format an IPv6-embedded IPv6 address ##############################
static bool formatEmbeddedAddress(const struct sockaddr_in6* ipv6address,
                                  std::string&               out)
{
   char str[INET6_ADDRSTRLEN];
   struct in6_addr prefix = ipv6address->sin6_addr;

   prefix.s6_addr[12] = 0xff;
   prefix.s6_addr[13] = 0xff;
   prefix.s6_addr[14] = 0xff;
   prefix.s6_addr[15] = 0xff;
   if(inet_ntop(AF_INET6, &prefix, str, sizeof(str)) == nullptr) {
      return false;
   }

   const size_t pl = std::strlen(str) - 9;
   const uint32_t upper = (ipv6address->sin6_addr.s6_addr[12] << 8) | ipv6address->sin6_addr.s6_addr[13];
   const uint32_t lower = (ipv6address->sin6_addr.s6_addr[14] << 8) | ipv6address->sin6_addr.s6_addr[15];
   const uint32_t suffix = htonl((upper << 16) | lower);

   char v4str[INET_ADDRSTRLEN];
   if(inet_ntop(AF_INET, &suffix, v4str, sizeof(v4str)) == nullptr) {
      return false;
   }

   out.assign(str, pl);
   out.append(v4str);
   return true;
}


// ###### Format an IPv6 address ############################################
static bool formatIPv6Address(const struct sockaddr_in6* ipv6address,
                              std::string&               out)
{
   if(hasTranslationPrefix(ipv6address)) {
      return formatEmbeddedAddress(ipv6address, out);
   }
   char str[INET6_ADDRSTRLEN];
   if(inet_ntop(AF_INET6, &ipv6address->sin6_addr, str, sizeof(str)) != nullptr) {
      out = str;
      return true;
   }
   return false;
}


// ###### Convert address to string #########################################
std::string address2string(const struct sockaddr* address,
                           const bool             port,
                           const bool             hideScope)
{
   switch(address->sa_family) {
      case AF_INET: {
         const auto* ipv4 = reinterpret_cast<const struct sockaddr_in*>(address);
         char str[INET_ADDRSTRLEN];
         if(inet_ntop(AF_INET, &ipv4->sin_addr, str, sizeof(str)) != nullptr) {
            return port ? std::format("{}:{}", str, ntohs(ipv4->sin_port)) : std::string(str);
         }
         break;
      }
      case AF_INET6: {
         const auto* ipv6 = reinterpret_cast<const struct sockaddr_in6*>(address);
         std::string scope { };
         if(!hideScope && (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&ipv6->sin6_addr))) {
            char ifname[IFNAMSIZ];
            if(if_indextoname(ipv6->sin6_scope_id, ifname) != nullptr) {
               scope = std::format("%{}", ifname);
            }
            else {
               scope = std::format("%{}", ipv6->sin6_scope_id);
            }
         }
         std::string addrStr;
         if(formatIPv6Address(ipv6, addrStr)) {
            return port ? std::format("[{}{}]:{}", addrStr, scope, ntohs(ipv6->sin6_port))
                        : std::format("{}{}", addrStr, scope);
         }
         break;
      }
      case AF_UNSPEC:
         return "(unspecified)";
   }
   return "(invalid!)";
}


// ###### Convert string to address #########################################
bool string2address(std::string_view      string,
                    union sockaddr_union* address,
                    const bool            readPort,
                    const int             addressFamily)
{
   std::string_view host = string;
   uint16_t         portNumber = 0;

   // ====== Handle RFC 2732 [addr]:port format ==============================
   if(!host.empty() && host.front() == '[') {
      const auto closeBracket = host.find(']');
      if(closeBracket != std::string_view::npos) {
         if(readPort && closeBracket + 1 < host.size() && host[closeBracket + 1] == ':') {
            const auto portStr = host.substr(closeBracket + 2);
            if(std::from_chars(portStr.data(), portStr.data() + portStr.size(), portNumber).ec != std::errc{}) {
               return false;
            }
         }
         host = host.substr(1, closeBracket - 1);
      }
   }
   // ====== Handle standard host:port format ================================
   else if(readPort) {
      const auto firstColon = host.find(':');
      const auto lastColon  = host.rfind(':');
      if(firstColon != std::string_view::npos && firstColon == lastColon) {
         const auto portStr = host.substr(lastColon + 1);
         if(std::from_chars(portStr.data(), portStr.data() + portStr.size(), portNumber).ec != std::errc{}) {
            return false;
         }
         host = host.substr(0, lastColon);
      }
   }

   const std::string hostStr(host);

   // ====== Fast-path: try inet_pton for literal IP addresses ===============
   if(addressFamily == AF_INET6 || addressFamily == AF_UNSPEC) {
      struct in6_addr in6;
      if(inet_pton(AF_INET6, hostStr.c_str(), &in6) == 1) {
         std::memset(address, 0, sizeof(union sockaddr_union));
         address->in6.sin6_family = AF_INET6;
         address->in6.sin6_addr   = in6;
         address->in6.sin6_port   = htons(portNumber);
#ifdef HAVE_SIN6_LEN
         address->in6.sin6_len    = sizeof(struct sockaddr_in6);
#endif
         return true;
      }
   }
   if(addressFamily == AF_INET || addressFamily == AF_UNSPEC) {
      struct in_addr in4;
      if(inet_pton(AF_INET, hostStr.c_str(), &in4) == 1) {
         std::memset(address, 0, sizeof(union sockaddr_union));
         address->in.sin_family = AF_INET;
         address->in.sin_addr   = in4;
         address->in.sin_port   = htons(portNumber);
#ifdef HAVE_SIN_LEN
         address->in.sin_len    = sizeof(struct sockaddr_in);
#endif
         return true;
      }
   }

   // ====== Resolve domain name =============================================
   struct addrinfo hints { };
   hints.ai_socktype = SOCK_DGRAM;
#ifdef AI_IDN
   hints.ai_flags = AI_IDN;
#endif

   char* punycode = nullptr;
#ifndef AI_IDN
   if(idn2_to_ascii_8z(hostStr.c_str(), &punycode, IDN2_NONTRANSITIONAL) != IDN2_OK) {
      if(idn2_to_ascii_8z(hostStr.c_str(), &punycode, 0) != IDN2_OK) {
         punycode = nullptr;
      }
   }
#endif

   const char* targetHost = (punycode != nullptr) ? punycode : hostStr.c_str();
   struct addrinfo* result = nullptr;

   if(addressFamily == AF_INET) {
      hints.ai_family = AF_INET;
      if(getaddrinfo(targetHost, nullptr, &hints, &result) != 0) {
#ifndef AI_IDN
         if(punycode != nullptr) {
            idn2_free(punycode);
         }
#endif
         return false;
      }
   }
   else if(addressFamily == AF_INET6) {
      hints.ai_family = AF_INET6;
      if(getaddrinfo(targetHost, nullptr, &hints, &result) != 0) {
#ifndef AI_IDN
         if(punycode != nullptr) {
            idn2_free(punycode);
         }
#endif
         return false;
      }
   }
   else {
      // Default: try IPv6 first, then fall back to IPv4
      hints.ai_family = AF_INET6;
      if(getaddrinfo(targetHost, nullptr, &hints, &result) != 0) {
         hints.ai_family = AF_UNSPEC;
         if(getaddrinfo(targetHost, nullptr, &hints, &result) != 0) {
#ifndef AI_IDN
            if(punycode != nullptr) {
               idn2_free(punycode);
            }
#endif
            return false;
         }
      }
   }

#ifndef AI_IDN
   if(punycode != nullptr) {
      idn2_free(punycode);
   }
#endif

   std::memset(address, 0, sizeof(union sockaddr_union));
   std::memcpy(address, result->ai_addr, result->ai_addrlen);

   if(address->sa.sa_family == AF_INET) {
      address->in.sin_port = htons(portNumber);
   }
   else if(address->sa.sa_family == AF_INET6) {
      address->in6.sin6_port = htons(portNumber);
   }

   freeaddrinfo(result);
   return true;
}


// ###### Print address =====================================================
void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  const bool             port,
                  const bool             hideScope)
{
   os << address2string(address, port, hideScope);
}
