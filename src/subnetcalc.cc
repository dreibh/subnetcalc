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

#if defined(__sun)
#define __EXTENSIONS__ 1
#endif

#include <bit>
#include <cassert>
#include <cctype>
#include <charconv>
#include <clocale>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#include <getopt.h>

#if !defined(_WIN32)
#include <netdb.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <process.h>
#include <shellapi.h>
#define isatty _isatty
#define fileno _fileno
#endif

#if defined(HAVE_LIBIBERTY)
#include <libiberty.h>
extern "C" {
int getopt_long_only(int argc, char* const* argv, const char* optstring,
                     const struct option* longopts, int* longindex);
}
#endif

#ifdef HAVE_MAXMINDDB
#include <maxminddb.h>
#endif

#if !defined(NI_IDN)
#include <idn2.h>
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#else
#define bindtextdomain(domain, dirname) { }
#define textdomain(domain)              { }
#define gettext(string)                 (string)
#define ngettext(singular, plural, n)   (((n) == 1) ? (singular) : (plural))
#endif

#include "tools.h"
#include "package-version.h"


// ###### C++20 gettext runtime format helper ################################
template<typename... Args>
static std::string formatText(const char* messageIdentifier,
                              const Args&... arguments)
{
   try {
      return std::vformat(gettext(messageIdentifier),
                          std::make_format_args(arguments...));
   }
   catch(const std::format_error&) {
      return gettext(messageIdentifier);
   }
}


// ###### Is given address an IPv4 address? #################################
inline bool isIPv4(const sockaddr_union& address)
{
   return (address.sa.sa_family == AF_INET);
}


// ###### Extract IPv4 address from address #################################
inline in_addr_t getIPv4Address(const sockaddr_union& address)
{
   assert(address.sa.sa_family == AF_INET);
   return address.in.sin_addr.s_addr;
}


// ###### Extract IPv6 address from address #################################
inline in6_addr getIPv6Address(const sockaddr_union& address)
{
   assert(address.sa.sa_family == AF_INET6);
   return address.in6.sin6_addr;
}


// ###### Is given address a multicast address? #############################
inline bool isMulticast(const sockaddr_union& address)
{
   if(isIPv4(address)) {
      return IN_CLASSD(ntohl(getIPv4Address(address)));
   }
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      return IN6_IS_ADDR_MULTICAST(&ipv6address);
   }
}


// ###### Read prefix from parameter ########################################
int readPrefix(std::string_view      parameter,
               const sockaddr_union& forAddress,
               sockaddr_union&       netmask)
{
   if(parameter.empty()) {
      return -1;
   }
   for(char c : parameter) {
      if(!std::isdigit(static_cast<unsigned char>(c))) {
         return -1;
      }
   }

   int prefix = 0;
   auto [ptr, ec] = std::from_chars(parameter.data(), parameter.data() + parameter.size(), prefix);
   if((ec != std::errc{}) || (prefix < 0)) {
      return -1;
   }

   netmask = forAddress;
   if(netmask.sa.sa_family == AF_INET) {
      if(prefix > 32) {
         return -1;
      }
      netmask.in.sin_addr.s_addr = htonl(prefix == 0 ? 0U : (~uint32_t{0} << (32 - prefix)));
   }
   else {
      if(prefix > 128) {
         return -1;
      }
      int remainingPrefix = prefix;
      for(int j = 0; j < 16; j++) {
         if(remainingPrefix >= 8) {
            netmask.in6.sin6_addr.s6_addr[j] = 0xff;
            remainingPrefix -= 8;
         }
         else if(remainingPrefix > 0) {
            netmask.in6.sin6_addr.s6_addr[j] = static_cast<uint8_t>(0xffu << (8 - remainingPrefix));
            remainingPrefix = 0;
         }
         else {
            netmask.in6.sin6_addr.s6_addr[j] = 0x00;
         }
      }
   }
   return prefix;
}


// ###### Generate a unique local IPv6 address ##############################
void generateUniqueLocal(sockaddr_union& address,
                         const bool      highQualityRng = false)
{
   uint8_t buffer[5];
   if(address.sa.sa_family != AF_INET6) {
      // TRANSLATORS: Error message when attempting to generate a unique local
      // address without specifying an IPv6 target address.
      std::cerr << gettext("ERROR: An IPv6 address must be given to generate a unique local address!") << '\n';
      std::exit(1);
   }

#if !defined(_WIN32)
   const char*   randomFile = highQualityRng ? "/dev/random" : "/dev/urandom";
   std::ifstream randomStream(randomFile, std::ios::binary);
   if(randomStream) {
      // TRANSLATORS: Status message when generating unique local address:
      // {0} = random number generator device path (e.g. /dev/urandom)
      std::cout << formatText("Generating Unique Local IPv6 address (using {0}) ...", randomFile) << '\n';
      if(!randomStream.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
         // TRANSLATORS: Error message when reading from random device fails:
         // {0} = random device file path
         std::cerr << formatText("ERROR: Unable to read from {0}!", randomFile) << '\n';
         std::exit(1);
      }
   }
   else {
      // TRANSLATORS: Error message when opening random device fails:
      // {0} = random device file path
      std::cerr << formatText("ERROR: Unable to open {0}!", randomFile) << '\n';
      std::exit(1);
   }
#else
   (void)highQualityRng;
   std::random_device rd;
   for(auto& byte : buffer) {
      byte = static_cast<uint8_t>(rd());
   }
#endif

   address.in6.sin6_addr.s6_addr[0] = 0xfd;
   std::memcpy(&address.in6.sin6_addr.s6_addr[1], buffer, sizeof(buffer));
}


// ###### Print address in binary digits ####################################
void printAddressBinary(std::ostream&         os,
                        const sockaddr_union& address,
                        const unsigned int    prefix,
                        const bool            colourMode = true,
                        std::string_view      indent     = "")
{
   if(address.sa.sa_family == AF_INET) {
      os << indent;
      uint32_t a = ntohl(getIPv4Address(address));
      for(int i = 31; i >= 0; i--) {
         const uint32_t v = (1U << static_cast<uint32_t>(i));
         if(colourMode) {
            os << ((32 - i > static_cast<int>(prefix)) ? "\x1b[33m" : "\x1b[34m");
         }
         if(a >= v) {
            os << '1';
            a -= v;
         }
         else {
            os << '0';
         }
         if(colourMode) {
            os << "\x1b[0m";
         }
         if(((i % 8) == 0) && (i > 0)) {
            os << " . ";
         }
      }
      os << '\n';
   }
   else {
      int            p           = 0;
      const in6_addr ipv6Address = getIPv6Address(address);
      for(int j = 0; j < 8; j++) {
         const uint16_t a = static_cast<uint16_t>((ipv6Address.s6_addr[j * 2] << 8) | ipv6Address.s6_addr[j * 2 + 1]);
         os << indent << std::format("{:04x} = ", a);
         for(int i = 15; i >= 0; i--) {
            const uint32_t v = (1U << static_cast<uint32_t>(i));
            if(colourMode) {
               os << ((p < static_cast<int>(prefix)) ? "\x1b[33m" : "\x1b[34m");
            }
            if(a >= v) {
               os << '1';
            }
            else {
               os << '0';
            }
            if(colourMode) {
               os << "\x1b[0m";
            }
            if(((i % 8) == 0) && (i > 0)) {
               os << ' ';
            }
            p++;
         }
         os << '\n';
      }
   }
}


// ###### Is given netmask valid? ###########################################
int getPrefixLength(const sockaddr_union& netmask)
{
   if(netmask.sa.sa_family == AF_INET) {
      const uint32_t a = ntohl(getIPv4Address(netmask));
      const int ones = std::countl_one(a);
      if(std::popcount(a) != ones) {
         return -1;
      }
      return ones;
   }
   else {
      const in6_addr ip6 = getIPv6Address(netmask);
      int totalPrefix = 0;
      bool seenZero   = false;

      for(int j = 0; j < 4; j++) {
         const uint32_t a = (static_cast<uint32_t>(ip6.s6_addr[j * 4])     << 24) |
                            (static_cast<uint32_t>(ip6.s6_addr[j * 4 + 1]) << 16) |
                            (static_cast<uint32_t>(ip6.s6_addr[j * 4 + 2]) << 8)  |
                            (static_cast<uint32_t>(ip6.s6_addr[j * 4 + 3]));
         const int ones = std::countl_one(a);
         if(seenZero) {
            if(a != 0) {
               return -1;
            }
         }
         else {
            if(std::popcount(a) != ones) {
               return -1;
            }
            totalPrefix += ones;
            if(ones < 32) {
               seenZero = true;
            }
         }
      }
      return totalPrefix;
   }
}


// ###### "&" operator for addresses ########################################
sockaddr_union operator&(const sockaddr_union& a1, const sockaddr_union& a2)
{
   assert(a1.sa.sa_family == a2.sa.sa_family);

   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr &= a2.in.sin_addr.s_addr;
   }
   else {
      for(int j = 0; j < 16; j++) {
         a.in6.sin6_addr.s6_addr[j] &= a2.in6.sin6_addr.s6_addr[j];
      }
   }
   return a;
}


// ###### "|" operator for addresses ########################################
sockaddr_union operator|(const sockaddr_union& a1, const sockaddr_union& a2)
{
   assert(a1.sa.sa_family == a2.sa.sa_family);

   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr |= a2.in.sin_addr.s_addr;
   }
   else {
      for(int j = 0; j < 16; j++) {
         a.in6.sin6_addr.s6_addr[j] |= a2.in6.sin6_addr.s6_addr[j];
      }
   }
   return a;
}


// ###### "~" operator for addresses ########################################
sockaddr_union operator~(const sockaddr_union& a1)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = ~a1.in.sin_addr.s_addr;
   }
   else {
      for(int j = 0; j < 16; j++) {
         a.in6.sin6_addr.s6_addr[j] = static_cast<uint8_t>(~a1.in6.sin6_addr.s6_addr[j]);
      }
   }
   return a;
}


// ###### Output operator for addresses #####################################
std::ostream& operator<<(std::ostream& os, const sockaddr_union& a)
{
   printAddress(os, &a.sa, false, true);
   return os;
}


// ###### "+" operator for addresses ########################################
sockaddr_union operator+(const sockaddr_union& a1, uint32_t n)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = htonl(ntohl(a.in.sin_addr.s_addr) + n);
   }
   else {
      union {
         uint8_t  b[16];
         uint32_t w[4];
      } local;
      std::memcpy(local.b, a.in6.sin6_addr.s6_addr, 16);
      for(int j = 3; j >= 0; j--) {
         const uint64_t sum = static_cast<uint64_t>(ntohl(local.w[j])) + static_cast<uint64_t>(n);
         local.w[j] = htonl(static_cast<uint32_t>(sum & 0xffffffffULL));
         n = static_cast<uint32_t>(sum >> 32);
      }
      std::memcpy(a.in6.sin6_addr.s6_addr, local.b, 16);
   }
   return a;
}


// ###### "-" operator for addresses ########################################
sockaddr_union operator-(const sockaddr_union& a1, uint32_t n)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = htonl(ntohl(a.in.sin_addr.s_addr) - n);
   }
   else {
      union {
         uint8_t  b[16];
         uint32_t w[4];
      } local;
      std::memcpy(local.b, a.in6.sin6_addr.s6_addr, 16);
      for(int j = 3; j >= 0; j--) {
         const uint64_t sum = static_cast<uint64_t>(ntohl(local.w[j])) - static_cast<uint64_t>(n);
         local.w[j] = htonl(static_cast<uint32_t>(sum & 0xffffffffULL));
         n = static_cast<uint32_t>(sum >> 32);
      }
      std::memcpy(a.in6.sin6_addr.s6_addr, local.b, 16);
   }
   return a;
}


// ###### "==" operator for addresses #######################################
bool operator==(const sockaddr_union& a1, const sockaddr_union& a2)
{
   assert(a1.sa.sa_family == a2.sa.sa_family);

   if(a1.sa.sa_family == AF_INET) {
      return (a1.in.sin_addr.s_addr == a2.in.sin_addr.s_addr);
   }
   else {
      return (std::memcmp(a1.in6.sin6_addr.s6_addr, a2.in6.sin6_addr.s6_addr, 16) == 0);
   }
}


// ###### Print IPv6 unicast properties of given address ####################
void printUnicastProperties(std::ostream&   os,
                            const in6_addr& ipv6address,
                            const bool      colourMode  = true,
                            const bool      hasSubnetID = true,
                            const bool      hasGlobalID = false)
{
   uint16_t word[8];
   for(int i = 0; i < 8; i++) {
      word[i] = static_cast<uint16_t>((ipv6address.s6_addr[i * 2] << 8) | ipv6address.s6_addr[i * 2 + 1]);
   }

   // ====== Global ID ======================================================
   if(hasGlobalID) {
      os << "      + " << std::format("{:<32}", gettext("Global ID"))
         << std::format(" = {:02x}{:04x}{:04x}\n", word[0] & 0xff, word[1], word[2]);
   }

   // ====== Subnet ID ======================================================
   if(hasSubnetID) {
      os << "      + " << std::format("{:<32}", gettext("Subnet ID"))
         << std::format(" = {:04x}\n", word[3]);
   }

   // ====== Interface ID ===================================================
   if(colourMode) {
      os << "      + " << std::format("{:<32}", gettext("Interface ID"))
         << std::format(" = \x1b[36m{:04x}:{:02x}\x1b[37m{:02x}:{:02x}\x1b[38m{:02x}:{:04x}\x1b[0m\n",
                        word[4], (word[5] & 0xff00) >> 8, (word[5] & 0x00ff),
                        (word[6] & 0xff00) >> 8, (word[6] & 0x00ff), word[7]);
   }
   else {
      os << "      + " << std::format("{:<32}", gettext("Interface ID"))
         << std::format(" = {:04x}:{:02x}{:02x}:{:02x}{:02x}:{:04x}\n",
                        word[4], (word[5] & 0xff00) >> 8, (word[5] & 0x00ff),
                        (word[6] & 0xff00) >> 8, (word[6] & 0x00ff), word[7]);
   }

   if(((word[5] & 0x00ff) == 0x00ff) && ((word[6] & 0xff00) == 0xfe00)) {
      if(colourMode) {
         os << "      + " << std::format("{:<32}", gettext("MAC Address"))
            << std::format(" = \x1b[36m{:02x}:{:02x}:{:02x}\x1b[0m:\x1b[38m{:02x}:{:02x}:{:02x}\x1b[0m\n",
                           ipv6address.s6_addr[8] ^ 0x02, ipv6address.s6_addr[9], ipv6address.s6_addr[10],
                           ipv6address.s6_addr[13], ipv6address.s6_addr[14], ipv6address.s6_addr[15]);
      }
      else {
         os << "      + " << std::format("{:<32}", gettext("MAC Address"))
            << std::format(" = {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\n",
                           ipv6address.s6_addr[8] ^ 0x02, ipv6address.s6_addr[9], ipv6address.s6_addr[10],
                           ipv6address.s6_addr[13], ipv6address.s6_addr[14], ipv6address.s6_addr[15]);
      }
   }

   // ====== Solicited Node Multicast Address ===============================
   if(colourMode) {
      os << "      + " << std::format("{:<32}", gettext("Solicited Node Multicast Address"))
         << std::format(" = \x1b[32mff02::1:ff\x1b[38m{:02x}:{:04x}\x1b[0m\n", word[6] & 0xff, word[7]);
   }
   else {
      os << "      + " << std::format("{:<32}", gettext("Solicited Node Multicast Address"))
         << std::format(" = ff02::1:ff{:02x}:{:04x}\n", word[6] & 0xff, word[7]);
   }
}


// ###### Print address properties ##########################################
void printAddressProperties(std::ostream&         os,
                            const sockaddr_union& address,
                            const sockaddr_union& netmask,
                            const unsigned int    prefix,
                            const sockaddr_union& network,
                            const sockaddr_union& broadcast,
                            const bool            colourMode)
{
   const std::string addressString = address2string(&address.sa, false, false);

   // ====== Common properties ==============================================
   os << std::format("{:<14} = \n", gettext("Properties"));
   os << "   - ";
   if(isMulticast(address)) {
      // TRANSLATORS: Address property: {0} = address string
      os << formatText("{0} is a MULTICAST address", addressString);
   }
   else if(isIPv4(address) && (prefix < 32) && (address == broadcast)) {
      const std::string networkString = address2string(&network.sa, false, false);
      // TRANSLATORS: Address property: {0} = address, {1} = network, {2} = prefix
      os << formatText("{0} is the BROADCAST address of {1}/{2}", addressString, networkString, prefix);
   }
   else if((address == network) &&
           ((isIPv4(address) && (prefix < 32)) || (!isIPv4(address) && (prefix < 128)))) {
      // TRANSLATORS: Address property: {0} = address string
      os << formatText("{0} is a NETWORK address", addressString);
   }
   else {
      const std::string networkString = address2string(&network.sa, false, false);
      // TRANSLATORS: Address property: {0} = address, {1} = network, {2} = prefix
      os << formatText("{0} is a HOST address in {1}/{2}", addressString, networkString, prefix);
   }
   os << '\n';

   // ====== IPv4 properties ================================================
   if(isIPv4(address)) {
      const in_addr_t    ipv4address = ntohl(getIPv4Address(address));
      const unsigned int a           = ipv4address >> 24;
      const unsigned int b           = (ipv4address & 0x00ff0000) >> 16;

      if(IN_CLASSA(ipv4address)) {
         os << "   - " << gettext("Class A") << '\n';
         if(ipv4address == INADDR_LOOPBACK) {
            os << "   - " << gettext("Loopback address") << '\n';
         }
         else if(a == IN_LOOPBACKNET) {
            os << "   - " << gettext("In loopback network") << '\n';
         }
         else if(a == 10) {
            os << "   - " << gettext("Private") << '\n';
         }
      }
      else if(IN_CLASSB(ipv4address)) {
         os << "   - " << gettext("Class B") << '\n';
         if((a == 172) && ((b >= 16) && (b <= 31))) {
            os << "   - " << gettext("Private") << '\n';
         }
         else if((a == 169) && (b == 254)) {
            os << "   - " << gettext("Link-local address") << '\n';
         }
      }
      else if(IN_CLASSC(ipv4address)) {
         os << "   - " << gettext("Class C") << '\n';
         if((a == 192) && (b == 168)) {
            os << "   - " << gettext("Private") << '\n';
         }
      }
      else if(IN_CLASSD(ipv4address)) {
         os << "   - " << gettext("Class D (Multicast)") << '\n';
         // ------ Multicast scope ------------------------------------------
         os << "      + " << gettext("Scope: ");
         if(a == 224) {
            os << gettext("link-local");
         }
         else if((a == 239) && (b >= 192) && (b <= 251)) {
            os << gettext("organization-local");
         }
         else if((a == 239) && (b >= 252) && (b <= 255)) {
            os << gettext("site-local");
         }
         else {
            os << gettext("global");
         }
         os << '\n';

         // ------ Corresponding MAC address --------------------------------
         const std::string macAddress = std::format("01:00:5e:{:02x}:{:02x}:{:02x}",
                                                    (ipv4address & 0x007f0000) >> 16,
                                                    (ipv4address & 0x0000ff00) >> 8,
                                                    (ipv4address & 0x000000ff));
         // TRANSLATORS: Multicast MAC address: {0} = MAC address string
         os << "      + " << formatText("Corresponding multicast MAC address: {0}", macAddress) << '\n';

         // ------ Source-specific multicast --------------------------------
         if(a == 232) {
            os << "      + " << gettext("Source-specific multicast") << '\n';
         }
      }
      else {
         os << "   - " << gettext("Invalid (not in class A, B, C or D)") << '\n';
      }
   }

   // ====== IPv6 properties ================================================
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      const uint16_t word0       = static_cast<uint16_t>((ipv6address.s6_addr[0] << 8) | ipv6address.s6_addr[1]);
      const uint16_t word1       = static_cast<uint16_t>((ipv6address.s6_addr[2] << 8) | ipv6address.s6_addr[3]);
      const uint16_t word5       = static_cast<uint16_t>((ipv6address.s6_addr[10] << 8) | ipv6address.s6_addr[11]);
      const uint16_t word6       = static_cast<uint16_t>((ipv6address.s6_addr[12] << 8) | ipv6address.s6_addr[13]);
      const uint16_t word7       = static_cast<uint16_t>((ipv6address.s6_addr[14] << 8) | ipv6address.s6_addr[15]);

      // ------ Special addresses -------------------------------------------
      if(IN6_IS_ADDR_LOOPBACK(&ipv6address)) {
         os << "   - " << gettext("Loopback address") << '\n';
      }
      else if(IN6_IS_ADDR_UNSPECIFIED(&ipv6address)) {
         os << "   - " << gettext("Unspecified address") << '\n';
      }
      else if(IN6_IS_ADDR_V4COMPAT(&ipv6address)) {
         os << "   - " << gettext("IPv4-compatible IPv6 address") << '\n';
      }
      else if(IN6_IS_ADDR_V4MAPPED(&ipv6address)) {
         os << "   - " << gettext("IPv4-mapped IPv6 address") << '\n';
      }
      else if(hasTranslationPrefix(&address.in6)) {
         os << "   - " << gettext("IPv4-embedded IPv6 address") << '\n';
      }

      // ------ Multicast addresses -----------------------------------------
      else if(IN6_IS_ADDR_MULTICAST(&ipv6address)) {
         os << "   - " << gettext("Multicast Properties") << '\n';
         os << "      + " << gettext("Scope: ");
         if(IN6_IS_ADDR_MC_NODELOCAL(&ipv6address)) {
            os << gettext("node-local");
         }
         else if(IN6_IS_ADDR_MC_LINKLOCAL(&ipv6address)) {
            os << gettext("link-local");
         }
         else if(IN6_IS_ADDR_MC_SITELOCAL(&ipv6address)) {
            os << gettext("site-local");
         }
         else if(IN6_IS_ADDR_MC_ORGLOCAL(&ipv6address)) {
            os << gettext("organization-local");
         }
         else if(IN6_IS_ADDR_MC_GLOBAL(&ipv6address)) {
            os << gettext("global");
         }
         else {
            os << gettext("unknown");
         }
         os << '\n';

         const uint8_t flags = (ipv6address.s6_addr[1] & 0xf0) >> 4;
         if(flags == 0x1) {
            os << "      + " << gettext("Temporarily-allocated address") << '\n';
         }

         const std::string macAddress = std::format("33:33:{:02x}:{:02x}:{:02x}:{:02x}",
                                                    ipv6address.s6_addr[12], ipv6address.s6_addr[13],
                                                    ipv6address.s6_addr[14], ipv6address.s6_addr[15]);
         // TRANSLATORS: Multicast MAC address: {0} = MAC address string
         os << "      + " << formatText("Corresponding multicast MAC address: {0}", macAddress) << '\n';

         if(((word0 & 0xfff0) == 0xff30) && (word1 == 0x0000)) {
            os << "      + " << gettext("Source-specific multicast") << '\n';
         }

         if((word0 == 0xff02) && (word5 == 0x0001) && ((word6 & 0xff00) == 0xff00)) {
            const std::string nodeAddress = std::format("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xx{:02x}:{:04x}",
                                                        word6 & 0xff, word7);
            // TRANSLATORS: Solicited node multicast target: {0} = target node mask
            os << "      + " << formatText("Address is solicited node multicast address for {0}", nodeAddress) << '\n';
         }
      }

      // ------ Link-local Unicast ------------------------------------------
      else if(IN6_IS_ADDR_LINKLOCAL(&ipv6address)) {
         os << "   - " << gettext("Link-Local Unicast Properties:") << '\n';
         printUnicastProperties(std::cout, ipv6address, colourMode, false, false);
      }

      // ------ Site-Local Unicast ------------------------------------------
      else if(IN6_IS_ADDR_SITELOCAL(&ipv6address)) {
         os << "   - " << gettext("Site-Local Unicast Properties:") << '\n';
         printUnicastProperties(std::cout, ipv6address, colourMode, true, false);
      }

      // ------ Unique Local Unicast ----------------------------------------
      else if((word0 & 0xfc00) == 0xfc00) {
         os << "   - " << gettext("Unique Local Unicast Properties:") << '\n';
         if(word0 & 0x0100) {
            os << "      + " << gettext("Locally chosen") << '\n';
         }
         else {
            os << "      + " << gettext("Assigned by global instance") << '\n';
         }
         printUnicastProperties(std::cout, ipv6address, colourMode, true, true);
      }

      // ------ Global Unicast ----------------------------------------------
      else if((word0 & 0xe000) == 0x2000) {
         os << "   - " << gettext("Global Unicast Properties:") << '\n';
         printUnicastProperties(std::cout, ipv6address, colourMode, false, false);

         if((word0 & 0x2002) == 0x2002) {
            sockaddr_union sixToFour { };
            sixToFour.sa.sa_family       = AF_INET;
            const uint32_t u = static_cast<uint32_t>((ipv6address.s6_addr[2] << 8) | ipv6address.s6_addr[3]);
            const uint32_t l = static_cast<uint32_t>((ipv6address.s6_addr[4] << 8) | ipv6address.s6_addr[5]);
            sixToFour.in.sin_addr.s_addr = htonl((u << 16) | l);
            os << "      + " << std::format("{:<32} = {}\n", gettext("6-to-4 Address"), sixToFour);
         }
      }
   }
}


#if defined(__SIZEOF_INT128__)
// ###### Convert unsigned 128 bit integer to string ########################
std::string toString(unsigned __int128 num)
{
   if(num == 0) {
      return "0";
   }
   std::string str;
   while(num != 0) {
      const int digit = static_cast<int>(num % 10);
      str.push_back(static_cast<char>('0' + digit));
      num /= 10;
   }
   std::reverse(str.begin(), str.end());
   return str;
}
#endif


#ifdef HAVE_MAXMINDDB
// ###### Find location of MaxMindDB .mmdb file #############################
std::string find_mmdb_path(const std::string& mmdbFileName)
{
   static const std::vector<std::string> mmdbDirectories = {
      "/usr/share/GeoIP",
      "/var/lib/GeoIP",
      "/usr/local/share/GeoIP",
      "/opt/homebrew/var/GeoIP",
      "/usr/local/var/GeoIP",
      "/etc/GeoIP"
#if defined(_WIN32)
      , "C:/ProgramData/GeoIP"
#endif
   };

   for(const auto& directory : mmdbDirectories) {
      const std::filesystem::path absolutePath = std::filesystem::path(directory) / mmdbFileName;
      if(std::filesystem::exists(absolutePath)) {
         return absolutePath.string();
      }
   }
   return {};
}
#endif


// ###### Version ###########################################################
[[noreturn]] static void version()
{
   std::cout << "SubNetCalc " << SUBNETCALC_VERSION << '\n';
   std::exit(0);
}


// ###### Usage #############################################################
[[noreturn]] static void usage(const char* program, const int exitCode)
{
   // TRANSLATORS: Command-line syntax message prefix:
   // "Usage: <program> [options]".
   std::cerr << gettext("Usage:") << " " << program
             << " address/prefix | address/netmask | address [prefix] | address [netmask]\n"
                " [-4|--ipv4] [-6|--ipv6]\n"
                " [-u|--uniquelocal] [-U|--uniquelocalhq]\n"
                " [-n|--noreverselookup]\n"
                " [-g|--nogeoiplookup]\n"
                " [-c|--nocolour|--nocolor]\n"
                " [-h|--help] [-v|--version]\n";
   std::exit(exitCode);
}


// ###### Main program ######################################################
int main(int argc, char** argv)
{
#if defined(_WIN32)
   SetConsoleCP(CP_UTF8);
   SetConsoleOutputCP(CP_UTF8);

   std::vector<std::string> argumentStrings;
   std::vector<char*>       argPointers;

   int wideArgc = 0;
   wchar_t** wideArgv = CommandLineToArgvW(GetCommandLineW(), &wideArgc);
   if(wideArgv != nullptr) {
      for(int i = 0; i < wideArgc; ++i) {
         const int length = WideCharToMultiByte(CP_UTF8, 0, wideArgv[i], -1,
                                                nullptr, 0, nullptr, nullptr);
         if(length > 0) {
            std::string buffer(static_cast<size_t>(length), '\0');
            WideCharToMultiByte(CP_UTF8, 0, wideArgv[i], -1,
                                buffer.data(), length, nullptr, nullptr);
            buffer.pop_back();
            argumentStrings.push_back(std::move(buffer));
         }
         else {
            argumentStrings.emplace_back();
         }
      }
      LocalFree(wideArgv);
   }

   argPointers.reserve(argumentStrings.size() + 1);
   for(auto& stringArgument : argumentStrings) {
      argPointers.push_back(stringArgument.data());
   }
   argPointers.push_back(nullptr);

   argc = static_cast<int>(argumentStrings.size());
   argv = argPointers.data();
#endif

   // ====== Initialise i18n and locale support =============================
   if(std::setlocale(LC_ALL, "") == nullptr) {
      std::setlocale(LC_ALL, "C.UTF-8");
   }

   // ====== Handle arguments ===============================================
   static const struct option long_options[] = {
      { "ipv4",            no_argument, nullptr, '4' },
      { "ipv6",            no_argument, nullptr, '6' },
      { "uniquelocal",     no_argument, nullptr, 'u' },
      { "uniquelocalhq",   no_argument, nullptr, 'U' },
      { "nocolour",        no_argument, nullptr, 'c' },
      { "nocolor",         no_argument, nullptr, 'c' },
      { "noreverselookup", no_argument, nullptr, 'n' },
      { "nogeoiplookup",   no_argument, nullptr, 'g' },
      { "help",            no_argument, nullptr, 'h' },
      { "version",         no_argument, nullptr, 'v' },
      { nullptr,           0,           nullptr, 0   }
   };

   int          addressFamily   = AF_UNSPEC;
   bool         colourMode      = true;
   bool         noReverseLookup = false;
   bool         noGeoIPLookup   = false;
   unsigned int uniqueLocal     = 0;
   int option    { 0 };
   int longIndex { 0 };

   while((option = getopt_long_only(argc, argv, "46uUcnghv", long_options, &longIndex)) != -1) {
      switch(option) {
         case '4':
            addressFamily = AF_INET;
            break;
         case '6':
            addressFamily = AF_INET6;
            break;
         case 'n':
            noReverseLookup = true;
            break;
         case 'g':
            noGeoIPLookup = true;
            break;
         case 'c':
            colourMode = false;
            break;
         case 'u':
            uniqueLocal = 1;
            break;
         case 'U':
            uniqueLocal = 2;
            break;
         case 'v':
            version();
            break;
         case 'h':
         case '?':
            usage(argv[0], (option == 'h') ? 0 : 1);
            break;
         default:
            std::cerr << "INTERNAL ERROR: Unhandled option!\n";
            return 1;
      }
   }

   if((optind + 1 != argc) && (optind + 2 != argc)) {
      usage(argv[0], 1);
   }

   // [WinSockScope...]

   int                prefix        = 0;
   unsigned int       hostBits      = 0;
   unsigned int       reservedHosts = 0;
#if defined(__SIZEOF_INT128__)
   unsigned __int128  maxHosts      = 0;
#else
   unsigned long long maxHosts      = 0;
#endif
   sockaddr_union     network   { };
   sockaddr_union     address   { };
   sockaddr_union     netmask   { };
   sockaddr_union     broadcast { };
   sockaddr_union     wildcard  { };
   sockaddr_union     host1     { };
   sockaddr_union     host2     { };

   // ====== Get address and netmask from arguments =========================
   const std::string_view firstArg = argv[optind];
   const auto slashPos = firstArg.find('/');

   if(slashPos != std::string_view::npos) {
      const std::string_view addressPart = firstArg.substr(0, slashPos);
      const std::string_view netmaskPart = firstArg.substr(slashPos + 1);

      if(!string2address(addressPart, &address, true, addressFamily)) {
         // TRANSLATORS: Error message when address string is invalid:
         // {0} = input address
         std::cerr << formatText("ERROR: Invalid address {0}!", addressPart) << '\n';
         return 1;
      }
      if(readPrefix(netmaskPart, address, netmask) < 0) {
         if(!string2address(netmaskPart, &netmask, true, addressFamily)) {
            // TRANSLATORS: Error message when netmask string is invalid:
            // {0} = input netmask
            std::cerr << formatText("ERROR: Invalid netmask {0}!", netmaskPart) << '\n';
            return 1;
         }
      }
   }
   else {
      if(!string2address(firstArg, &address, true, addressFamily)) {
         std::cerr << formatText("ERROR: Invalid address {0}!", firstArg) << '\n';
         return 1;
      }
      if(optind + 1 < argc) {
         const std::string_view secondArg = argv[optind + 1];
         if(!secondArg.empty() && (secondArg.front() != '-')) {
            if(readPrefix(secondArg, address, netmask) < 0) {
               if(!string2address(secondArg, &netmask, true, addressFamily)) {
                  std::cerr << formatText("ERROR: Invalid netmask {0}!", secondArg) << '\n';
                  return 1;
               }
            }
         }
         else {
            prefix = readPrefix((address.sa.sa_family == AF_INET) ? "32" : "128", address, netmask);
            assert(prefix >= 0);
         }
      }
      else {
         prefix = readPrefix((address.sa.sa_family == AF_INET) ? "32" : "128", address, netmask);
         assert(prefix >= 0);
      }
   }

   // ====== Get prefix length ==============================================
   prefix = getPrefixLength(netmask);
   if(prefix < 0) {
      const std::string addressString = address2string(&netmask.sa, false, false);
      std::cerr << formatText("ERROR: Invalid netmask {0}!", addressString) << '\n';
      return 1;
   }
   if(netmask.sa.sa_family != address.sa.sa_family) {
      const std::string addressString = address2string(&netmask.sa, false, false);
      // TRANSLATORS: Error message when address and netmask families mismatch:
      // {0} = netmask string
      std::cerr << formatText("ERROR: Incompatible netmask {0}!", addressString) << '\n';
      return 1;
   }

   // ====== Unique Local IPv6 address generation ===========================
   if(uniqueLocal > 0) {
      generateUniqueLocal(address, (uniqueLocal > 1));
   }

   // ====== Calculate network address, hosts, etc. =========================
   network   = address & netmask;
   broadcast = network | (~netmask);
   wildcard  = ~netmask;

   if(isIPv4(address)) {
      reservedHosts = 2;
      hostBits      = 32 - static_cast<unsigned int>(prefix);
      host1         = network + 1;
      host2         = broadcast - 1;
      if(prefix >= 31) {
         reservedHosts = 0;
         host1 = network;
         host2 = broadcast;
      }
   }
   else {
      reservedHosts = 1;
      hostBits      = 128 - static_cast<unsigned int>(prefix);
      if(prefix < 128) {
         host1 = network + 1;
         host2 = broadcast;
      }
      else {
         reservedHosts = 0;
         host1         = network;
         host2         = broadcast;
      }
   }

#if defined(__SIZEOF_INT128__)
   if(hostBits >= 128) {
      maxHosts = ~static_cast<unsigned __int128>(0) - reservedHosts;
   }
   else if(hostBits > 0) {
      maxHosts = (static_cast<unsigned __int128>(1) << hostBits) - reservedHosts;
   }
   else {
      maxHosts = 0;
   }
#else
   if(hostBits == 64) {
      maxHosts = ~0ULL - reservedHosts;
   }
   else if((hostBits < 64) && (hostBits > 0)) {
      maxHosts = (1ULL << hostBits) - reservedHosts;
   }
   else {
      maxHosts = 0;
   }
#endif

   // ====== Print results ==================================================
   std::cout << std::format("{:<14} = {}\n", gettext("Address"), address);
   printAddressBinary(std::cout, address, static_cast<unsigned int>(prefix), colourMode,
                      std::format("{:<14}      ", " "));
   std::cout << std::format("{:<14} = {} / {}\n", gettext("Network"), network, prefix)
             << std::format("{:<14} = {}\n", gettext("Netmask"), netmask);

   if(isIPv4(address)) {
      std::cout << std::format("{:<14} = ", gettext("Broadcast"));
      if(reservedHosts == 2) {
         std::cout << broadcast;
      }
      else {
         std::cout << gettext("not needed on Point-to-Point links");
      }
      std::cout << '\n';
   }

   std::cout << std::format("{:<14} = {}\n", gettext("Wildcard Mask"), wildcard);
   if(isIPv4(address)) {
      std::cout << std::format("{:<14} = {:08X}\n", gettext("Hex. Address"), ntohl(address.in.sin_addr.s_addr));
   }
   std::cout << std::format("{:<14} = {}\n", gettext("Host Bits"), hostBits);

   if(!isMulticast(address)) {
      std::string maxHostsString;
      if(maxHosts > 0) {
#if defined(__SIZEOF_INT128__)
         maxHostsString = std::format("{}   (2^{} - {})", toString(maxHosts), hostBits, reservedHosts);
#else
         maxHostsString = std::format("{}   (2^{} - {})", maxHosts, hostBits, reservedHosts);
#endif
      }
      else {
         maxHostsString = std::format("2^{} - {}", hostBits, reservedHosts);
      }

      std::cout << std::format("{:<14} = {}\n", gettext("Max. Hosts"), maxHostsString)
                << std::format("{:<14} = {{ {} - {} }}\n", gettext("Host Range"), host1, host2);
   }

   // ====== Properties =====================================================
   printAddressProperties(std::cout, address, netmask, static_cast<unsigned int>(prefix),
                          network, broadcast, colourMode);

   // ====== GeoIP ==========================================================
// ====== GeoIP ==========================================================
#ifdef HAVE_MAXMINDDB
   if(!noGeoIPLookup) {
      const std::string mmdbASNDatabasePath = find_mmdb_path("GeoLite2-ASN.mmdb");
      if(!mmdbASNDatabasePath.empty()) {
         MMDB_s mmdb;
         if(MMDB_open(mmdbASNDatabasePath.c_str(), MMDB_MODE_MMAP, &mmdb) == MMDB_SUCCESS) {
            int mmdbErrorCode = 0;
            // FIX: Remove 'const' here so &mmdbLookupResult.entry is mutable MMDB_entry_s*
            auto mmdbLookupResult = MMDB_lookup_sockaddr(&mmdb, &address.sa, &mmdbErrorCode);
            if(mmdbLookupResult.found_entry) {
               std::string organisation = gettext("Unknown");
               MMDB_entry_data_s mmdbEntryData;
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "autonomous_system_organization", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  organisation = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }
               std::cout << std::format("{:<14} = {}\n", gettext("GeoIP AS Info"), organisation);
            }
            MMDB_close(&mmdb);
         }
      }
   }

   if(!noGeoIPLookup) {
      const std::string mmdbCityDatabasePath = find_mmdb_path("GeoLite2-City.mmdb");
      if(!mmdbCityDatabasePath.empty()) {
         MMDB_s mmdb;
         if(MMDB_open(mmdbCityDatabasePath.c_str(), MMDB_MODE_MMAP, &mmdb) == MMDB_SUCCESS) {
            int mmdbErrorCode = 0;
            // FIX: Remove 'const' here as well
            auto mmdbLookupResult = MMDB_lookup_sockaddr(&mmdb, &address.sa, &mmdbErrorCode);
            if(mmdbLookupResult.found_entry) {
               MMDB_entry_data_s mmdbEntryData;

               std::string country = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "country", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  country = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               std::string code = "??";
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "country", "iso_code", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  code = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               std::cout << std::format("{:<14} = {} ({})\n", gettext("GeoIP Country"), country, code);

               std::string postalCode;
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "postal", "code", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  postalCode = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               std::string city = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "city", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  city = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               std::string region = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "subdivisions", "0", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  region = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               std::string timeZone;
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "time_zone", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  timeZone = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               double latitude = 0.0;
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "latitude", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  latitude = mmdbEntryData.double_value;
               }

               double longitude = 0.0;
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "longitude", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  longitude = mmdbEntryData.double_value;
               }

               // TRANSLATORS: Cardinal direction abbreviation: North
               // TRANSLATORS: Cardinal direction abbreviation: South
               const char* latDirection = (latitude >= 0.0) ? gettext("N") : gettext("S");

               // TRANSLATORS: Cardinal direction abbreviation: East
               // TRANSLATORS: Cardinal direction abbreviation: West
               const char* lonDirection = (longitude >= 0.0) ? gettext("E") : gettext("W");

               std::cout << std::format("{:<14} = {}{}{}, {} ({:.4f}°{}, {:.4f}°{}{}{})\n",
                                        gettext("GeoIP Region"),
                                        postalCode,
                                        (!postalCode.empty() ? " " : ""),
                                        city,
                                        region,
                                        std::fabs(latitude),  latDirection,
                                        std::fabs(longitude), lonDirection,
                                        (!timeZone.empty() ? ", " : ""),
                                        timeZone);
            }
            MMDB_close(&mmdb);
         }
      }
   }
#endif

   // ====== Reverse lookup =================================================
   if(!noReverseLookup) {
      if(isatty(fileno(stdout))) {
         std::cout << gettext("Performing reverse DNS lookup ...");
         std::cout.flush();
      }

      char hostname[NI_MAXHOST];
      const int error = getnameinfo(&address.sa,
                                    static_cast<socklen_t>(getSocklen(&address.sa)),
                                    hostname, sizeof(hostname),
                                    nullptr, 0,
#ifdef NI_IDN
                                    NI_NAMEREQD | NI_IDN
#else
                                    NI_NAMEREQD
#endif
                                   );

      if(isatty(fileno(stdout))) {
         std::cout << "\r\x1b[K";
      }
      std::cout << std::format("{:<14} = ", gettext("DNS Hostname"));
      std::cout.flush();

      if(error == 0) {
#ifndef NI_IDN
         char* utf8hostname = nullptr;
         if(idn2_to_unicode_8z8z(hostname, &utf8hostname, 0) == IDN2_OK) {
            std::cout << utf8hostname << '\n';
            idn2_free(utf8hostname);
         }
         else {
            std::cout << hostname << '\n';
         }
#else
         std::cout << hostname << '\n';
#endif
      }
      else {
         std::cout << '(' << gai_strerror(error) << ")\n";
      }
   }

   return 0;
}
