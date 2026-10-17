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

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#if defined(__sun)
#define __EXTENSIONS__ 1
#endif

#include <algorithm>
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
#include <langinfo.h>
#include <netdb.h>
#include <unistd.h>
#include <wchar.h>
#else
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
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


// ###### Output Format #####################################################
enum class OutputFormat {
   Text,
   JSON,
   YAML,
   XML
};


// ###### Collected Subnet and Address Details ##############################
struct SubnetDetails {
   // Core Address & Network Information
   std::string  address;
   std::string  expandedAddress; // 8-word zero-padded IPv6 representation
   std::string  family;          // "IPv4" or "IPv6"
   std::string  network;
   int          prefix { 0 };
   std::string  netmask;
   bool         hasBroadcast { false };
   std::string  broadcast;
   std::string  wildcard;
   bool         hasHexAddress { false };
   std::string  hexAddress;
   unsigned int hostBits { 0 };
   bool         isMulticast { false };
   bool         hasMaxHosts { false }; // true when decimal host count is available
   std::string  maxHostsCount;         // Pure decimal string ("0", "254") for machine formats
   std::string  maxHostsText;          // Formula string ("254   (2^8 - 2)") for Text
   std::string  hostMin;
   std::string  hostMax;

   // Common Classification
   std::string  addressType;     // "host", "network", "broadcast", "multicast"
   std::string  addressTypeDescription;

   // IPv4 Specific
   std::string              ipv4Class;
   std::vector<std::string> ipv4Flags;
   std::string              multicastScope;
   std::string              multicastMac;
   bool                     isSourceSpecificMulticast { false };

   // IPv6 Specific
   std::string  ipv6Special;
   bool         isTransientMulticast { false };
   std::string  solicitedNodeFor;
   std::string  unicastType;
   std::string  ulaType;
   bool         hasGlobalID { false };
   std::string  globalID;
   bool         hasSubnetID { false };
   std::string  subnetID;
   std::string  interfaceID;
   bool         hasMACAddress { false };
   std::string  macAddress;
   std::string  solicitedNodeMulticast;
   bool         hasSixToFour { false };
   std::string  sixToFourAddress;

   // GeoIP Information
   bool         hasGeoIPASN { false };
   uint32_t     geoIPASNumber { 0 };
   std::string  geoIPASOrganization;
   std::string  geoIPASInfo;
   bool         hasGeoIPCity { false };
   std::string  geoIPCountry;
   std::string  geoIPCountryCode;
   std::string  geoIPPostalCode;
   std::string  geoIPCity;
   std::string  geoIPRegion;
   std::string  geoIPTimeZone;
   double       geoIPLatitude { 0.0 };
   double       geoIPLongitude { 0.0 };

   // Reverse DNS Information
   bool         hasDNSHostname { false };
   std::string  dnsHostname;
   std::string  dnsError;
};


// ###### C++20 gettext runtime format helper ###############################
template<typename... Args>
static std::string formatText(std::string_view formatString,
                              const Args&...   arguments)
{
   try {
      return std::vformat(formatString, std::make_format_args(arguments...));
   }
   catch(const std::format_error&) {
      return std::string(formatString);
   }
}


// ###### Determine Visual Terminal Column Width for UTF-8 Strings ##########
static size_t displayWidth(std::string_view text)
{
   if(text.empty()) {
      return 0;
   }
#if !defined(_WIN32)
   const std::string textStr(text);
   const size_t len = textStr.size();
   std::vector<wchar_t> wbuf(len + 1, 0);
   const size_t numChars = std::mbstowcs(wbuf.data(), textStr.c_str(), len);
   if(numChars != static_cast<size_t>(-1)) {
      const int width = wcswidth(wbuf.data(), numChars);
      if(width >= 0) {
         return static_cast<size_t>(width);
      }
   }
#endif
   size_t count = 0;
   for(const unsigned char c : text) {
      if((c & 0xc0) != 0x80) {
         count++;
      }
   }
   return count;
}


// ###### Detect Right-to-Left (RTL) text direction #########################
static bool isRTL()
{
   // TRANSLATORS: Text direction: LTR (Left-to-Right) or RTL (Right-to-Left)
   return std::strcmp(gettext("LTR"), "RTL") == 0;
}


// ###### Escape JSON/YAML strings safely ###################################
static std::string escapeJSON(std::string_view str)
{
   std::string result;
   result.reserve(str.size());
   for(char c : str) {
      switch(c) {
         case '"':  result += "\\\""; break;
         case '\\': result += "\\\\"; break;
         case '\b': result += "\\b";  break;
         case '\f': result += "\\f";  break;
         case '\n': result += "\\n";  break;
         case '\r': result += "\\r";  break;
         case '\t': result += "\\t";  break;
         default:
            if(static_cast<unsigned char>(c) < 0x20) {
               result += std::format("\\u{:04x}", static_cast<unsigned char>(c));
            }
            else {
               result += c;
            }
            break;
      }
   }
   return result;
}


// ###### Escape XML strings safely #########################################
static std::string escapeXML(std::string_view str)
{
   std::string result;
   result.reserve(str.size());
   for(char c : str) {
      switch(c) {
         case '&':  result += "&amp;";  break;
         case '<':  result += "&lt;";   break;
         case '>':  result += "&gt;";   break;
         case '"':  result += "&quot;"; break;
         case '\'': result += "&apos;"; break;
         default:   result += c;        break;
      }
   }
   return result;
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


// ###### Format mask to string without IPv4-embedded notation ##############
static std::string mask2string(const sockaddr_union& mask)
{
   if(isIPv4(mask)) {
      return address2string(mask, false);
   }

   const in6_addr ip6 = getIPv6Address(mask);
   uint16_t words[8];
   for(int i = 0; i < 8; i++) {
      words[i] = static_cast<uint16_t>((ip6.s6_addr[i * 2] << 8) | ip6.s6_addr[i * 2 + 1]);
   }

   // Find longest run of zeros (RFC 5952 section 4.2):
   int best_base = -1;
   int best_len  = 0;
   int cur_base  = -1;
   int cur_len   = 0;

   for(int i = 0; i < 8; i++) {
      if(words[i] == 0) {
         if(cur_base == -1) {
            cur_base = i;
            cur_len  = 1;
         }
         else {
            cur_len++;
         }
         if(cur_len > best_len) {
            best_base = cur_base;
            best_len  = cur_len;
         }
      }
      else {
         cur_base = -1;
         cur_len  = 0;
      }
   }

   // RFC 5952: "The symbol '::' MUST NOT be used to shorten just
   // one 16-bit 0 field.":
   if(best_len <= 1) {
      best_base = -1;
      best_len  = 0;
   }

   std::string out;
   int i = 0;
   while(i < 8) {
      if(i == best_base) {
         out += "::";
         i += best_len;
      }
      else {
         if(!out.empty() && !out.ends_with("::")) {
            out += ':';
         }
         out += std::format("{:x}", words[i]);
         i++;
      }
   }
   if(out.empty()) {
      out = "::";
   }
   return out;
}


// ###### Read prefix from parameter ########################################
int readPrefix(std::string_view      parameter,
               const sockaddr_union& address,
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

   netmask = address;
   if(netmask.sa.sa_family == AF_INET) {
      if(prefix > 32) {
         return -1;
      }
      netmask.in.sin_addr.s_addr =
         htonl(prefix == 0 ? 0U : (~uint32_t{0} << (32 - prefix)));
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
            netmask.in6.sin6_addr.s6_addr[j] =
               static_cast<uint8_t>(0xffu << (8 - remainingPrefix));
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
void generateUniqueLocal(sockaddr_union&    address,
                         const bool         highQualityRNG = false,
                         const OutputFormat outputFormat   = OutputFormat::Text)
{
   uint8_t buffer[5];
   if(address.sa.sa_family != AF_INET6) {
      // TRANSLATORS: Error message when attempting to generate a unique local
      // address without specifying an IPv6 target address.
      std::cerr << gettext("ERROR: An IPv6 address must be given to generate a unique local address!") << '\n';
      std::exit(1);
   }

#if !defined(_WIN32)
   const char*   randomFile = highQualityRNG ? "/dev/random" : "/dev/urandom";
   std::ifstream randomStream(randomFile, std::ios::binary);
   if(randomStream) {
      if(outputFormat == OutputFormat::Text) {
         // TRANSLATORS: Status message when generating unique local address:
         // {0} = random number generator device path (e.g. /dev/urandom)
         std::cout << formatText(gettext("Generating Unique Local IPv6 address (using {0}) ..."), randomFile) << '\n';
      }
      if(!randomStream.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
         // TRANSLATORS: Error message when reading from random device fails:
         // {0} = random device file path
         std::cerr << formatText(gettext("ERROR: Unable to read from {0}!"), randomFile) << '\n';
         std::exit(1);
      }
   }
   else {
      // TRANSLATORS: Error message when opening random device fails:
      // {0} = random device file path
      std::cerr << formatText(gettext("ERROR: Unable to open {0}!"), randomFile) << '\n';
      std::exit(1);
   }
#else
   (void)highQualityRNG;
   if(outputFormat == OutputFormat::Text) {
      // TRANSLATORS: Status message when generating unique local address:
      // {0} = random number generator device path (e.g. /dev/urandom)
      std::cout << formatText(gettext("Generating Unique Local IPv6 address (using {0}) ..."), "std::random_device") << '\n';
   }
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
      const uint32_t a = ntohl(getIPv4Address(address));
      for(int i = 31; i >= 0; i--) {
         if(colourMode) {
            os << ((32 - i > static_cast<int>(prefix)) ? "\x1b[33m" : "\x1b[34m");
         }
         os << (((a & (1U << static_cast<uint32_t>(i))) != 0) ? '1' : '0');
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
            if(colourMode) {
               os << ((p < static_cast<int>(prefix)) ? "\x1b[34m" : "\x1b[33m");
            }
            os << (((a & (1U << static_cast<uint32_t>(i))) != 0) ? '1' : '0');
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
      const uint32_t a    = ntohl(getIPv4Address(netmask));
      const int      ones = std::countl_one(a);
      if(std::popcount(a) != ones) {
         return -1;
      }
      return ones;
   }
   else {
      const in6_addr ip6         = getIPv6Address(netmask);
      int            totalPrefix = 0;
      bool           seenZero    = false;

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


// ###### Output Text Format ################################################
void outputPlainText(std::ostream&         os,
                     const SubnetDetails&  details,
                     const sockaddr_union& address,
                     const sockaddr_union& netmask,
                     const unsigned int    prefix,
                     const bool            colourMode)
{
   const bool rtl = isRTL();

   struct FieldEntry {
      std::string label;
      std::string value;
      bool        isHeaderOnly { false };
   };

   std::vector<FieldEntry> fields;

   // TRANSLATORS: Output field label for the input IP address
   const char* addressLabel = gettext("Address");
   fields.push_back({ addressLabel, details.address, false });

   const char* expAddressLabel = nullptr;
   if(!details.expandedAddress.empty()) {
      // TRANSLATORS: Output field label for the expanded IPv6 address
      expAddressLabel = gettext("Exp. Address");
      fields.push_back({ expAddressLabel, details.expandedAddress, false });
   }

   // TRANSLATORS: Output field label for the network IP address
   const char* networkLabel = gettext("Network");
   fields.push_back({ networkLabel, std::format("{} / {}", details.network, details.prefix), false });

   // TRANSLATORS: Output field label for the subnet mask
   const char* netmaskLabel = gettext("Netmask");
   fields.push_back({ netmaskLabel, details.netmask, false });

   if(details.family == "IPv4") {
      // TRANSLATORS: Output field label for the broadcast address
      const char* broadcastLabel = gettext("Broadcast");
      std::string broadcastVal = details.hasBroadcast ? details.broadcast :
                                 // TRANSLATORS: Explanation when broadcast address is not applicable
                                 gettext("not needed on Point-to-Point links");
      fields.push_back({ broadcastLabel, broadcastVal, false });
   }

   // TRANSLATORS: Output field label for the wildcard mask (inverted netmask)
   const char* wildcardLabel = gettext("Wildcard Mask");
   fields.push_back({ wildcardLabel, details.wildcard, false });

   if(details.hasHexAddress) {
      // TRANSLATORS: Output field label for the hexadecimal representation of the IPv4 address
      const char* hexAddressLabel = gettext("Hex. Address");
      fields.push_back({ hexAddressLabel, details.hexAddress, false });
   }

   // TRANSLATORS: Output field label for the number of host bits in the subnet
   const char* hostBitsLabel = gettext("Host Bits");
   fields.push_back({ hostBitsLabel, std::to_string(details.hostBits), false });

   if(!details.isMulticast) {
      // TRANSLATORS: Output field label for the maximum number of usable hosts
      const char* maxHostsLabel = gettext("Max. Hosts");
      fields.push_back({ maxHostsLabel, details.maxHostsText, false });

      // TRANSLATORS: Output field label for the usable host address range
      const char* hostRangeLabel = gettext("Host Range");
      fields.push_back({ hostRangeLabel, std::format("{{ {} - {} }}", details.hostMin, details.hostMax), false });
   }

   // TRANSLATORS: Output field label for IP address properties
   const char* propertiesLabel = gettext("Properties");
   fields.push_back({ propertiesLabel, "", true });

   if(details.hasGeoIPASN) {
      // TRANSLATORS: Output field label for GeoIP Autonomous System info
      const char* asInfoLabel = gettext("GeoIP AS Info");
      fields.push_back({ asInfoLabel, details.geoIPASInfo, false });
   }

   if(details.hasGeoIPCity) {
      // TRANSLATORS: Output field label for GeoIP country info
      const char* countryLabel = gettext("GeoIP Country");
      fields.push_back({ countryLabel, std::format("{} ({})", details.geoIPCountry, details.geoIPCountryCode), false });

      // TRANSLATORS: Cardinal direction abbreviation: North
      const char* latDir = (details.geoIPLatitude >= 0.0) ? gettext("N") : gettext("S");
      // TRANSLATORS: Cardinal direction abbreviation: East
      const char* lonDir = (details.geoIPLongitude >= 0.0) ? gettext("E") : gettext("W");

      // TRANSLATORS: Output field label for GeoIP region / location info
      const char* regionLabel = gettext("GeoIP Region");
      std::string regionVal = std::format("{}{}{}, {} ({:.4f}°{}, {:.4f}°{}{}{})",
                                          details.geoIPPostalCode,
                                          (!details.geoIPPostalCode.empty() ? " " : ""),
                                          details.geoIPCity,
                                          details.geoIPRegion,
                                          std::fabs(details.geoIPLatitude),  latDir,
                                          std::fabs(details.geoIPLongitude), lonDir,
                                          (!details.geoIPTimeZone.empty() ? ", " : ""),
                                          details.geoIPTimeZone);
      fields.push_back({ regionLabel, regionVal, false });
   }

   if(details.hasDNSHostname || !details.dnsError.empty()) {
      // TRANSLATORS: Output field label for DNS hostname
      const char* dnsHostnameLabel = gettext("DNS Hostname");
      std::string dnsVal = details.hasDNSHostname ? details.dnsHostname : details.dnsError;
      fields.push_back({ dnsHostnameLabel, dnsVal, false });
   }

   size_t maxLabelWidth = 0;
   for(const auto& f : fields) {
      maxLabelWidth = std::max(maxLabelWidth, displayWidth(f.label));
   }

   for(const auto& f : fields) {
      const size_t labelW = displayWidth(f.label);
      const size_t pad    = (maxLabelWidth >= labelW) ? (maxLabelWidth - labelW) : 0;

      if(!rtl) {
         if(f.isHeaderOnly) {
            os << f.label << std::string(pad, ' ') << " = \n";
         }
         else {
            os << f.label << std::string(pad, ' ') << " = " << f.value << '\n';
         }
      }
      else {
         if(f.isHeaderOnly) {
            os << "= " << std::string(pad, ' ') << f.label << '\n';
         }
         else {
            os << f.value << " = " << std::string(pad, ' ') << f.label << '\n';
         }
      }

      // Print binary representation immediately under the address block
      const bool hasExp = !details.expandedAddress.empty();
      if((hasExp && expAddressLabel && (f.label == expAddressLabel)) ||
         (!hasExp && (f.label == addressLabel))) {
         printAddressBinary(os, address, prefix, colourMode,
                            std::string(maxLabelWidth + 6, ' '));
      }
      else if(f.isHeaderOnly && (f.label == propertiesLabel)) {
         os << "   - " << details.addressTypeDescription << '\n';

         if(details.family == "IPv4") {
            if(!details.ipv4Class.empty()) {
               if(details.ipv4Class == "Class A") {
                  // TRANSLATORS: IPv4 network class designation
                  os << "   - " << gettext("Class A") << '\n';
               }
               else if(details.ipv4Class == "Class B") {
                  // TRANSLATORS: IPv4 network class designation
                  os << "   - " << gettext("Class B") << '\n';
               }
               else if(details.ipv4Class == "Class C") {
                  // TRANSLATORS: IPv4 network class designation
                  os << "   - " << gettext("Class C") << '\n';
               }
               else if(details.ipv4Class == "Class D (Multicast)") {
                  // TRANSLATORS: IPv4 network class designation
                  os << "   - " << gettext("Class D (Multicast)") << '\n';
               }
               else {
                  // TRANSLATORS: Error message when IPv4 address is outside classes A through D
                  os << "   - " << gettext("Invalid (not in class A, B, C or D)") << '\n';
               }
            }

            for(const auto& flag : details.ipv4Flags) {
               if(flag == "Loopback address") {
                  // TRANSLATORS: IPv4 address property
                  os << "   - " << gettext("Loopback address") << '\n';
               }
               else if(flag == "In loopback network") {
                  // TRANSLATORS: IPv4 address property
                  os << "   - " << gettext("In loopback network") << '\n';
               }
               else if(flag == "Private") {
                  // TRANSLATORS: IPv4 address property
                  os << "   - " << gettext("Private") << '\n';
               }
               else if(flag == "Link-local address") {
                  // TRANSLATORS: IPv4 address property
                  os << "   - " << gettext("Link-local address") << '\n';
               }
            }

            if(!details.multicastScope.empty()) {
               // TRANSLATORS: Multicast scope label prefix
               os << "      + " << gettext("Scope: ");
               if(details.multicastScope == "link-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("link-local");
               }
               else if(details.multicastScope == "organization-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("organization-local");
               }
               else if(details.multicastScope == "site-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("site-local");
               }
               else {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("global");
               }
               os << '\n';

               // TRANSLATORS: Multicast MAC address: {0} = MAC address string
               os << "      + " << formatText(gettext("Corresponding multicast MAC address: {0}"), details.multicastMac) << '\n';

               if(details.isSourceSpecificMulticast) {
                  // TRANSLATORS: Multicast property
                  os << "      + " << gettext("Source-specific multicast") << '\n';
               }
            }
         }
         else {
            if(!details.ipv6Special.empty()) {
               if(details.ipv6Special == "Loopback address") {
                  // TRANSLATORS: IPv6 address property
                  os << "   - " << gettext("Loopback address") << '\n';
               }
               else if(details.ipv6Special == "Unspecified address") {
                  // TRANSLATORS: IPv6 address property
                  os << "   - " << gettext("Unspecified address") << '\n';
               }
               else if(details.ipv6Special == "IPv4-compatible IPv6 address") {
                  // TRANSLATORS: IPv6 address property
                  os << "   - " << gettext("IPv4-compatible IPv6 address") << '\n';
               }
               else if(details.ipv6Special == "IPv4-mapped IPv6 address") {
                  // TRANSLATORS: IPv6 address property
                  os << "   - " << gettext("IPv4-mapped IPv6 address") << '\n';
               }
               else if(details.ipv6Special == "IPv4-embedded IPv6 address") {
                  // TRANSLATORS: IPv6 address property
                  os << "   - " << gettext("IPv4-embedded IPv6 address") << '\n';
               }
            }
            else if(details.isMulticast) {
               // TRANSLATORS: Section header for IPv6 multicast properties
               os << "   - " << gettext("Multicast Properties") << '\n';
               // TRANSLATORS: Multicast scope label prefix
               os << "      + " << gettext("Scope: ");
               if(details.multicastScope == "node-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("node-local");
               }
               else if(details.multicastScope == "link-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("link-local");
               }
               else if(details.multicastScope == "site-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("site-local");
               }
               else if(details.multicastScope == "organization-local") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("organization-local");
               }
               else if(details.multicastScope == "global") {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("global");
               }
               else {
                  // TRANSLATORS: Multicast scope level
                  os << gettext("unknown");
               }
               os << '\n';

               if(details.isTransientMulticast) {
                  // TRANSLATORS: IPv6 multicast flag property
                  os << "      + " << gettext("Temporarily-allocated address") << '\n';
               }

               // TRANSLATORS: Multicast MAC address: {0} = MAC address string
               os << "      + " << formatText(gettext("Corresponding multicast MAC address: {0}"), details.multicastMac) << '\n';

               if(details.isSourceSpecificMulticast) {
                  // TRANSLATORS: Multicast property
                  os << "      + " << gettext("Source-specific multicast") << '\n';
               }

               if(!details.solicitedNodeFor.empty()) {
                  // TRANSLATORS: Solicited node multicast target: {0} = target node mask
                  os << "      + " << formatText(gettext("Address is solicited node multicast address for {0}"), details.solicitedNodeFor) << '\n';
               }
            }
            else {
               if(details.unicastType == "Link-Local Unicast") {
                  // TRANSLATORS: Section header for link-local unicast properties
                  os << "   - " << gettext("Link-Local Unicast Properties:") << '\n';
               }
               else if(details.unicastType == "Site-Local Unicast") {
                  // TRANSLATORS: Section header for site-local unicast properties
                  os << "   - " << gettext("Site-Local Unicast Properties:") << '\n';
               }
               else if(details.unicastType == "Unique Local Unicast") {
                  // TRANSLATORS: Section header for unique local unicast properties
                  os << "   - " << gettext("Unique Local Unicast Properties:") << '\n';
                  if(details.ulaType == "Locally chosen") {
                     // TRANSLATORS: Unique local address allocation type
                     os << "      + " << gettext("Locally chosen") << '\n';
                  }
                  else if(details.ulaType == "Assigned by global instance") {
                     // TRANSLATORS: Unique local address allocation type
                     os << "      + " << gettext("Assigned by global instance") << '\n';
                  }
               }
               else if(details.unicastType == "Global Unicast") {
                  // TRANSLATORS: Section header for global unicast properties
                  os << "   - " << gettext("Global Unicast Properties:") << '\n';
               }

               struct PropEntry {
                  std::string label;
                  std::string value;
               };
               std::vector<PropEntry> propEntries;

               if(details.hasGlobalID) {
                  // TRANSLATORS: IPv6 unique local unicast property label
                  propEntries.push_back({ gettext("Global ID"), details.globalID });
               }
               if(details.hasSubnetID) {
                  // TRANSLATORS: IPv6 unique local unicast property label
                  propEntries.push_back({ gettext("Subnet ID"), details.subnetID });
               }

               // TRANSLATORS: IPv6 unicast property label
               const char* interfaceIdLabel = gettext("Interface ID");
               if(colourMode) {
                  const in6_addr ipv6address = getIPv6Address(address);
                  uint16_t word[8];
                  for(int i = 0; i < 8; i++) {
                     word[i] = static_cast<uint16_t>((ipv6address.s6_addr[i * 2] << 8) | ipv6address.s6_addr[i * 2 + 1]);
                  }
                  propEntries.push_back({ interfaceIdLabel,
                     std::format("\x1b[36m{:04x}:{:02x}\x1b[37m{:02x}:{:02x}\x1b[38m{:02x}:{:04x}\x1b[0m",
                                 word[4], (word[5] & 0xff00) >> 8, (word[5] & 0x00ff),
                                 (word[6] & 0xff00) >> 8, (word[6] & 0x00ff), word[7]) });
               }
               else {
                  propEntries.push_back({ interfaceIdLabel, details.interfaceID });
               }

               if(details.hasMACAddress) {
                  // TRANSLATORS: IPv6 unicast MAC address label derived from EUI-64
                  const char* macAddressLabel = gettext("MAC Address");
                  if(colourMode) {
                     const in6_addr ipv6address = getIPv6Address(address);
                     propEntries.push_back({ macAddressLabel,
                        std::format("\x1b[36m{:02x}:{:02x}:{:02x}\x1b[0m:\x1b[38m{:02x}:{:02x}:{:02x}\x1b[0m",
                                    ipv6address.s6_addr[8] ^ 0x02, ipv6address.s6_addr[9], ipv6address.s6_addr[10],
                                    ipv6address.s6_addr[13], ipv6address.s6_addr[14], ipv6address.s6_addr[15]) });
                  }
                  else {
                     propEntries.push_back({ macAddressLabel, details.macAddress });
                  }
               }

               // TRANSLATORS: IPv6 unicast property label
               const char* snmcLabel = gettext("Solicited Node Multicast Address");
               if(colourMode) {
                  const in6_addr ipv6address = getIPv6Address(address);
                  uint16_t word[8];
                  for(int i = 0; i < 8; i++) {
                     word[i] = static_cast<uint16_t>((ipv6address.s6_addr[i * 2] << 8) | ipv6address.s6_addr[i * 2 + 1]);
                  }
                  propEntries.push_back({ snmcLabel,
                     std::format("\x1b[32mff02::1:ff\x1b[38m{:02x}:{:04x}\x1b[0m", word[6] & 0xff, word[7]) });
               }
               else {
                  propEntries.push_back({ snmcLabel, details.solicitedNodeMulticast });
               }

               if(details.hasSixToFour) {
                  // TRANSLATORS: 6-to-4 encapsulation address label
                  propEntries.push_back({ gettext("6-to-4 Address"), details.sixToFourAddress });
               }

               size_t maxPropLabelWidth = 0;
               for(const auto& prop : propEntries) {
                  maxPropLabelWidth = std::max(maxPropLabelWidth, displayWidth(prop.label));
               }

               for(const auto& prop : propEntries) {
                  const size_t propW   = displayWidth(prop.label);
                  const size_t propPad = (maxPropLabelWidth >= propW) ? (maxPropLabelWidth - propW) : 0;
                  if(!rtl) {
                     os << "      + " << prop.label << std::string(propPad, ' ') << " = " << prop.value << '\n';
                  }
                  else {
                     os << "      + " << prop.value << " = " << std::string(propPad, ' ') << prop.label << '\n';
                  }
               }
            }
         }
      }
   }
}


// ###### Output JSON Format ################################################
void outputJSON(std::ostream& os, const SubnetDetails& details)
{
   std::vector<std::string> entries;

   entries.push_back(std::format("  \"address\": \"{}\"", escapeJSON(details.address)));

   if(!details.expandedAddress.empty()) {
      entries.push_back(std::format("  \"expanded_address\": \"{}\"", escapeJSON(details.expandedAddress)));
   }

   entries.push_back(std::format("  \"family\": \"{}\"", escapeJSON(details.family)));
   entries.push_back(std::format("  \"network\": \"{}\"", escapeJSON(details.network)));
   entries.push_back(std::format("  \"prefix\": {}", details.prefix));
   entries.push_back(std::format("  \"netmask\": \"{}\"", escapeJSON(details.netmask)));

   if(details.hasBroadcast) {
      entries.push_back(std::format("  \"broadcast\": \"{}\"", escapeJSON(details.broadcast)));
   }

   entries.push_back(std::format("  \"wildcard\": \"{}\"", escapeJSON(details.wildcard)));

   if(details.hasHexAddress) {
      entries.push_back(std::format("  \"hex\": \"{}\"", escapeJSON(details.hexAddress)));
   }

   entries.push_back(std::format("  \"host_bits\": {}", details.hostBits));

   if(!details.isMulticast) {
      if(details.hasMaxHosts) {
         entries.push_back(std::format("  \"max_hosts\": {}", details.maxHostsCount));
      }

      std::string hostRange = "  \"host_range\": {\n";
      hostRange += std::format("    \"min\": \"{}\",\n", escapeJSON(details.hostMin));
      hostRange += std::format("    \"max\": \"{}\"\n", escapeJSON(details.hostMax));
      hostRange += "  }";
      entries.push_back(hostRange);
   }

   // Properties object
   {
      std::string prop = "  \"properties\": {\n";
      std::vector<std::string> propEntries;
      propEntries.push_back(std::format("    \"type\": \"{}\"", escapeJSON(details.addressType)));
      propEntries.push_back(std::format("    \"description\": \"{}\"", escapeJSON(details.addressTypeDescription)));

      if(!details.ipv4Class.empty()) {
         propEntries.push_back(std::format("    \"class\": \"{}\"", escapeJSON(details.ipv4Class)));
      }
      if(!details.ipv4Flags.empty()) {
         std::string flagsStr = "    \"flags\": [";
         for(size_t i = 0; i < details.ipv4Flags.size(); ++i) {
            flagsStr += (i > 0 ? ", " : "") + std::format("\"{}\"", escapeJSON(details.ipv4Flags[i]));
         }
         flagsStr += "]";
         propEntries.push_back(flagsStr);
      }
      if(!details.multicastScope.empty()) {
         propEntries.push_back(std::format("    \"scope\": \"{}\"", escapeJSON(details.multicastScope)));
      }
      if(!details.multicastMac.empty()) {
         propEntries.push_back(std::format("    \"multicast_mac\": \"{}\"", escapeJSON(details.multicastMac)));
      }
      if(details.isSourceSpecificMulticast) {
         propEntries.push_back("    \"source_specific_multicast\": true");
      }
      if(!details.ipv6Special.empty()) {
         propEntries.push_back(std::format("    \"special\": \"{}\"", escapeJSON(details.ipv6Special)));
      }
      if(details.isTransientMulticast) {
         propEntries.push_back("    \"transient\": true");
      }
      if(!details.solicitedNodeFor.empty()) {
         propEntries.push_back(std::format("    \"solicited_node_for\": \"{}\"", escapeJSON(details.solicitedNodeFor)));
      }
      if(!details.unicastType.empty()) {
         propEntries.push_back(std::format("    \"unicast_type\": \"{}\"", escapeJSON(details.unicastType)));
      }
      if(!details.ulaType.empty()) {
         propEntries.push_back(std::format("    \"ula_type\": \"{}\"", escapeJSON(details.ulaType)));
      }
      if(details.hasGlobalID) {
         propEntries.push_back(std::format("    \"global_id\": \"{}\"", escapeJSON(details.globalID)));
      }
      if(details.hasSubnetID) {
         propEntries.push_back(std::format("    \"subnet_id\": \"{}\"", escapeJSON(details.subnetID)));
      }
      if(!details.interfaceID.empty()) {
         propEntries.push_back(std::format("    \"interface_id\": \"{}\"", escapeJSON(details.interfaceID)));
      }
      if(details.hasMACAddress) {
         propEntries.push_back(std::format("    \"mac_address\": \"{}\"", escapeJSON(details.macAddress)));
      }
      if(!details.solicitedNodeMulticast.empty()) {
         propEntries.push_back(std::format("    \"solicited_node_multicast\": \"{}\"", escapeJSON(details.solicitedNodeMulticast)));
      }
      if(details.hasSixToFour) {
         propEntries.push_back(std::format("    \"six_to_four\": \"{}\"", escapeJSON(details.sixToFourAddress)));
      }

      for(size_t i = 0; i < propEntries.size(); ++i) {
         prop += propEntries[i];
         if(i + 1 < propEntries.size()) {
            prop += ",\n";
         }
         else {
            prop += "\n";
         }
      }
      prop += "  }";
      entries.push_back(prop);
   }

   // GeoIP object
   if(details.hasGeoIPASN || details.hasGeoIPCity) {
      std::string geo = "  \"geoip\": {\n";
      std::vector<std::string> geoEntries;

      if(details.hasGeoIPASN) {
         if(details.geoIPASNumber > 0) {
            geoEntries.push_back(std::format("    \"asn\": {}", details.geoIPASNumber));
         }
         if(!details.geoIPASOrganization.empty()) {
            geoEntries.push_back(std::format("    \"as_organization\": \"{}\"", escapeJSON(details.geoIPASOrganization)));
         }
      }
      if(details.hasGeoIPCity) {
         geoEntries.push_back(std::format("    \"country\": \"{}\"", escapeJSON(details.geoIPCountry)));
         geoEntries.push_back(std::format("    \"country_code\": \"{}\"", escapeJSON(details.geoIPCountryCode)));
         if(!details.geoIPPostalCode.empty()) {
            geoEntries.push_back(std::format("    \"postal_code\": \"{}\"", escapeJSON(details.geoIPPostalCode)));
         }
         geoEntries.push_back(std::format("    \"city\": \"{}\"", escapeJSON(details.geoIPCity)));
         geoEntries.push_back(std::format("    \"region\": \"{}\"", escapeJSON(details.geoIPRegion)));
         geoEntries.push_back(std::format("    \"timezone\": \"{}\"", escapeJSON(details.geoIPTimeZone)));
         geoEntries.push_back(std::format("    \"latitude\": {:.4f}", details.geoIPLatitude));
         geoEntries.push_back(std::format("    \"longitude\": {:.4f}", details.geoIPLongitude));
      }

      for(size_t i = 0; i < geoEntries.size(); ++i) {
         geo += geoEntries[i];
         if(i + 1 < geoEntries.size()) {
            geo += ",\n";
         }
         else {
            geo += "\n";
         }
      }
      geo += "  }";
      entries.push_back(geo);
   }

   // DNS object
   if(details.hasDNSHostname) {
      std::string dns = "  \"dns\": {\n";
      dns += std::format("    \"hostname\": \"{}\"\n", escapeJSON(details.dnsHostname));
      dns += "  }";
      entries.push_back(dns);
   }
   else if(!details.dnsError.empty()) {
      std::string dns = "  \"dns\": {\n";
      dns += std::format("    \"error\": \"{}\"\n", escapeJSON(details.dnsError));
      dns += "  }";
      entries.push_back(dns);
   }

   os << "{\n";
   for(size_t i = 0; i < entries.size(); ++i) {
      os << entries[i];
      if(i + 1 < entries.size()) {
         os << ",\n";
      }
      else {
         os << "\n";
      }
   }
   os << "}\n";
}


// ###### Output YAML Format ################################################
void outputYAML(std::ostream& os, const SubnetDetails& details)
{
   os << std::format("address: \"{}\"\n", escapeJSON(details.address));
   if(!details.expandedAddress.empty()) {
      os << std::format("expanded_address: \"{}\"\n", escapeJSON(details.expandedAddress));
   }
   os << std::format("family: \"{}\"\n", escapeJSON(details.family));
   os << std::format("network: \"{}\"\n", escapeJSON(details.network));
   os << std::format("prefix: {}\n", details.prefix);
   os << std::format("netmask: \"{}\"\n", escapeJSON(details.netmask));

   if(details.hasBroadcast) {
      os << std::format("broadcast: \"{}\"\n", escapeJSON(details.broadcast));
   }

   os << std::format("wildcard: \"{}\"\n", escapeJSON(details.wildcard));

   if(details.hasHexAddress) {
      os << std::format("hex: \"{}\"\n", escapeJSON(details.hexAddress));
   }

   os << std::format("host_bits: {}\n", details.hostBits);

   if(!details.isMulticast) {
      if(details.hasMaxHosts) {
         os << std::format("max_hosts: {}\n", details.maxHostsCount);
      }
      os << "host_range:\n";
      os << std::format("  min: \"{}\"\n", escapeJSON(details.hostMin));
      os << std::format("  max: \"{}\"\n", escapeJSON(details.hostMax));
   }

   // Properties object
   os << "properties:\n";
   os << std::format("  type: \"{}\"\n", escapeJSON(details.addressType));
   os << std::format("  description: \"{}\"\n", escapeJSON(details.addressTypeDescription));

   if(!details.ipv4Class.empty()) {
      os << std::format("  class: \"{}\"\n", escapeJSON(details.ipv4Class));
   }
   if(!details.ipv4Flags.empty()) {
      os << "  flags:\n";
      for(const auto& flag : details.ipv4Flags) {
         os << std::format("    - \"{}\"\n", escapeJSON(flag));
      }
   }
   if(!details.multicastScope.empty()) {
      os << std::format("  scope: \"{}\"\n", escapeJSON(details.multicastScope));
   }
   if(!details.multicastMac.empty()) {
      os << std::format("  multicast_mac: \"{}\"\n", escapeJSON(details.multicastMac));
   }
   if(details.isSourceSpecificMulticast) {
      os << "  source_specific_multicast: true\n";
   }
   if(!details.ipv6Special.empty()) {
      os << std::format("  special: \"{}\"\n", escapeJSON(details.ipv6Special));
   }
   if(details.isTransientMulticast) {
      os << "  transient: true\n";
   }
   if(!details.solicitedNodeFor.empty()) {
      os << std::format("  solicited_node_for: \"{}\"\n", escapeJSON(details.solicitedNodeFor));
   }
   if(!details.unicastType.empty()) {
      os << std::format("  unicast_type: \"{}\"\n", escapeJSON(details.unicastType));
   }
   if(!details.ulaType.empty()) {
      os << std::format("  ula_type: \"{}\"\n", escapeJSON(details.ulaType));
   }
   if(details.hasGlobalID) {
      os << std::format("  global_id: \"{}\"\n", escapeJSON(details.globalID));
   }
   if(details.hasSubnetID) {
      os << std::format("  subnet_id: \"{}\"\n", escapeJSON(details.subnetID));
   }
   if(!details.interfaceID.empty()) {
      os << std::format("  interface_id: \"{}\"\n", escapeJSON(details.interfaceID));
   }
   if(details.hasMACAddress) {
      os << std::format("  mac_address: \"{}\"\n", escapeJSON(details.macAddress));
   }
   if(!details.solicitedNodeMulticast.empty()) {
      os << std::format("  solicited_node_multicast: \"{}\"\n", escapeJSON(details.solicitedNodeMulticast));
   }
   if(details.hasSixToFour) {
      os << std::format("  six_to_four: \"{}\"\n", escapeJSON(details.sixToFourAddress));
   }

   // GeoIP object
   if(details.hasGeoIPASN || details.hasGeoIPCity) {
      os << "geoip:\n";
      if(details.hasGeoIPASN) {
         if(details.geoIPASNumber > 0) {
            os << std::format("  asn: {}\n", details.geoIPASNumber);
         }
         if(!details.geoIPASOrganization.empty()) {
            os << std::format("  as_organization: \"{}\"", escapeJSON(details.geoIPASOrganization));
         }
      }
      if(details.hasGeoIPCity) {
         os << std::format("  country: \"{}\"\n", escapeJSON(details.geoIPCountry));
         os << std::format("  country_code: \"{}\"\n", escapeJSON(details.geoIPCountryCode));
         if(!details.geoIPPostalCode.empty()) {
            os << std::format("  postal_code: \"{}\"\n", escapeJSON(details.geoIPPostalCode));
         }
         os << std::format("  city: \"{}\"\n", escapeJSON(details.geoIPCity));
         os << std::format("  region: \"{}\"\n", escapeJSON(details.geoIPRegion));
         os << std::format("  timezone: \"{}\"\n", escapeJSON(details.geoIPTimeZone));
         os << std::format("  latitude: {:.4f}\n", details.geoIPLatitude);
         os << std::format("  longitude: {:.4f}\n", details.geoIPLongitude);
      }
   }

   // DNS object
   if(details.hasDNSHostname) {
      os << "dns:\n";
      os << std::format("  hostname: \"{}\"\n", escapeJSON(details.dnsHostname));
   }
   else if(!details.dnsError.empty()) {
      os << "dns:\n";
      os << std::format("  error: \"{}\"\n", escapeJSON(details.dnsError));
   }
}


// ###### Output XML Format #################################################
void outputXML(std::ostream& os, const SubnetDetails& details)
{
   os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
   os << "<subnetcalc>\n";
   os << std::format("  <address>{}</address>\n", escapeXML(details.address));
   if(!details.expandedAddress.empty()) {
      os << std::format("  <expanded_address>{}</expanded_address>\n", escapeXML(details.expandedAddress));
   }
   os << std::format("  <family>{}</family>\n", escapeXML(details.family));
   os << std::format("  <network>{}</network>\n", escapeXML(details.network));
   os << std::format("  <prefix>{}</prefix>\n", details.prefix);
   os << std::format("  <netmask>{}</netmask>\n", escapeXML(details.netmask));

   if(details.hasBroadcast) {
      os << std::format("  <broadcast>{}</broadcast>\n", escapeXML(details.broadcast));
   }

   os << std::format("  <wildcard>{}</wildcard>\n", escapeXML(details.wildcard));

   if(details.hasHexAddress) {
      os << std::format("  <hex>{}</hex>\n", escapeXML(details.hexAddress));
   }

   os << std::format("  <host_bits>{}</host_bits>\n", details.hostBits);

   if(!details.isMulticast) {
      if(details.hasMaxHosts) {
         os << std::format("  <max_hosts>{}</max_hosts>\n", details.maxHostsCount);
      }
      os << "  <host_range>\n";
      os << std::format("    <min>{}</min>\n", escapeXML(details.hostMin));
      os << std::format("    <max>{}</max>\n", escapeXML(details.hostMax));
      os << "  </host_range>\n";
   }

   // Properties object
   os << "  <properties>\n";
   os << std::format("    <type>{}</type>\n", escapeXML(details.addressType));
   os << std::format("    <description>{}</description>\n", escapeXML(details.addressTypeDescription));

   if(!details.ipv4Class.empty()) {
      os << std::format("    <class>{}</class>\n", escapeXML(details.ipv4Class));
   }
   if(!details.ipv4Flags.empty()) {
      os << "    <flags>\n";
      for(const auto& flag : details.ipv4Flags) {
         os << std::format("      <flag>{}</flag>\n", escapeXML(flag));
      }
      os << "    </flags>\n";
   }
   if(!details.multicastScope.empty()) {
      os << std::format("    <scope>{}</scope>\n", escapeXML(details.multicastScope));
   }
   if(!details.multicastMac.empty()) {
      os << std::format("    <multicast_mac>{}</multicast_mac>\n", escapeXML(details.multicastMac));
   }
   if(details.isSourceSpecificMulticast) {
      os << "    <source_specific_multicast>true</source_specific_multicast>\n";
   }
   if(!details.ipv6Special.empty()) {
      os << std::format("    <special>{}</special>\n", escapeXML(details.ipv6Special));
   }
   if(details.isTransientMulticast) {
      os << "    <transient>true</transient>\n";
   }
   if(!details.solicitedNodeFor.empty()) {
      os << std::format("    <solicited_node_for>{}</solicited_node_for>\n", escapeXML(details.solicitedNodeFor));
   }
   if(!details.unicastType.empty()) {
      os << std::format("    <unicast_type>{}</unicast_type>\n", escapeXML(details.unicastType));
   }
   if(!details.ulaType.empty()) {
      os << std::format("    <ula_type>{}</ula_type>\n", escapeXML(details.ulaType));
   }
   if(details.hasGlobalID) {
      os << std::format("    <global_id>{}</global_id>\n", escapeXML(details.globalID));
   }
   if(details.hasSubnetID) {
      os << std::format("    <subnet_id>{}</subnet_id>\n", escapeXML(details.subnetID));
   }
   if(!details.interfaceID.empty()) {
      os << std::format("    <interface_id>{}</interface_id>\n", escapeXML(details.interfaceID));
   }
   if(details.hasMACAddress) {
      os << std::format("    <mac_address>{}</mac_address>\n", escapeXML(details.macAddress));
   }
   if(!details.solicitedNodeMulticast.empty()) {
      os << std::format("    <solicited_node_multicast>{}</solicited_node_multicast>\n", escapeXML(details.solicitedNodeMulticast));
   }
   if(details.hasSixToFour) {
      os << std::format("    <six_to_four>{}</six_to_four>\n", escapeXML(details.sixToFourAddress));
   }
   os << "  </properties>\n";

   // GeoIP object
   if(details.hasGeoIPASN || details.hasGeoIPCity) {
      os << "  <geoip>\n";
      if(details.hasGeoIPASN) {
         if(details.geoIPASNumber > 0) {
            os << std::format("    <asn>{}</asn>\n", details.geoIPASNumber);
         }
         if(!details.geoIPASOrganization.empty()) {
            os << std::format("    <as_organization>{}</as_organization>\n", escapeXML(details.geoIPASOrganization));
         }
      }
      if(details.hasGeoIPCity) {
         os << std::format("    <country>{}</country>\n", escapeXML(details.geoIPCountry));
         os << std::format("    <country_code>{}</country_code>\n", escapeXML(details.geoIPCountryCode));
         if(!details.geoIPPostalCode.empty()) {
            os << std::format("    <postal_code>{}</postal_code>\n", escapeXML(details.geoIPPostalCode));
         }
         os << std::format("    <city>{}</city>\n", escapeXML(details.geoIPCity));
         os << std::format("    <region>{}</region>\n", escapeXML(details.geoIPRegion));
         os << std::format("    <timezone>{}</timezone>\n", escapeXML(details.geoIPTimeZone));
         os << std::format("    <latitude>{:.4f}</latitude>\n", details.geoIPLatitude);
         os << std::format("    <longitude>{:.4f}</longitude>\n", details.geoIPLongitude);
      }
      os << "  </geoip>\n";
   }

   // DNS object
   if(details.hasDNSHostname) {
      os << "  <dns>\n";
      os << std::format("    <hostname>{}</hostname>\n", escapeXML(details.dnsHostname));
      os << "  </dns>\n";
   }
   else if(!details.dnsError.empty()) {
      os << "  <dns>\n";
      os << std::format("    <error>{}</error>\n", escapeXML(details.dnsError));
      os << "  </dns>\n";
   }

   os << "</subnetcalc>\n";
}


// ###### Version ###########################################################
[[noreturn]] static void version()
{
   std::cout << "SubNetCalc " << SUBNETCALC_VERSION << '\n';
   std::exit(0);
}


// ------ BEGIN-OF-USAGE-BLOCK ------
// ###### Usage #############################################################
#define HEADER(s)    "\x1b[4m" << (s) << "\x1b[0m"
#define CMD(s)       "\x1b[1m" << (s) << "\x1b[0m"
#define OPT(s)       "\x1b[1m" s "\x1b[0m"
#define ARG(s)       "\x1b[4m" s "\x1b[0m"
#define SYN(s)       "\x1b[1m" s "\x1b[0m"
#define PIPE         SYN(" | ")
#define ENTRY(s)     "\t" SYN("[") s SYN("]\n")

[[ noreturn ]]
static void usage(const char* program, const int exitCode)
{
   // TRANSLATORS: Command-line syntax message prefix:
   // "Usage: <program> [options]".
   std::cerr << HEADER(gettext("Usage:")) << "\n"
             << CMD(program) << " "
                SYN("{") " " ARG("address/prefix") PIPE ARG("address/netmask") PIPE ARG("address") " " ARG("prefix") PIPE ARG("address") " " ARG("netmask") " " SYN("}") "\n"
                   ENTRY(OPT("-F") " " ARG("text|json|yaml|xml") PIPE OPT("--format") " " ARG("text|json|yaml|xml"))
                   ENTRY(OPT("-u") PIPE OPT("--uniquelocal"))
                   ENTRY(OPT("-U") PIPE OPT("--uniquelocalhq"))
                   ENTRY(OPT("-n") PIPE OPT("--noreverselookup"))
                   ENTRY(OPT("-g") PIPE OPT("--nogeoiplookup"))
                   ENTRY(OPT("-c") PIPE OPT("--nocolour") PIPE OPT("--nocolor"))
                   ENTRY(OPT("-4") PIPE OPT("--ipv4"))
                   ENTRY(OPT("-6") PIPE OPT("--ipv6"))
             << CMD(program) << "\n"
                   ENTRY(OPT("-h") PIPE OPT("--help"))
             << CMD(program) << "\n"
                   ENTRY(OPT("-v") PIPE OPT("--version"));
   exit(exitCode);
}
// ------ END-OF-USAGE-BLOCK ------



// ====== Main program ######################################################
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
            std::string buffer(static_cast<size_t>(length), 0x00);
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

   // ====== Initialise locale support ======================================
   if(std::setlocale(LC_ALL, "") == nullptr) {
      std::setlocale(LC_ALL, "C.UTF-8");
   }
   try {
      std::locale userLocale("");
      std::cout.imbue(userLocale);
      std::cerr.imbue(userLocale);
   } catch (const std::runtime_error&) {
      try {
         std::locale fallbackLocale("C.UTF-8");
         std::cout.imbue(fallbackLocale);
         std::cerr.imbue(fallbackLocale);
      } catch (const std::runtime_error&) {
         // Defaults to std::locale::classic() if C.UTF-8 is unavailable
      }
   }

   // ====== Initialise i18n support ========================================
#if defined(ENABLE_NLS)
#if !defined(_WIN32)
   const char* textdomaindir     = std::getenv("TEXTDOMAINDIR");
#else
   char*       textdomaindir     = nullptr;
   size_t      textdomaindirsize = 0;
   _dupenv_s(&textdomaindir, &textdomaindirsize, "TEXTDOMAINDIR");
#endif
   bindtextdomain("subnetcalc",
                  ( ((textdomaindir) && (textdomaindir[0] != 0x00)) ?
                       textdomaindir : SUBNETCALC_LOCALEDIR ));
#if defined(_WIN32)
   if(textdomaindir) {
       free(textdomaindir);
   }
#endif
   bind_textdomain_codeset("subnetcalc", "UTF-8");
   textdomain("subnetcalc");
#endif

   // ====== Handle arguments ===============================================
   static const struct option long_options[] = {
      { "ipv4",            no_argument,       nullptr, '4' },
      { "ipv6",            no_argument,       nullptr, '6' },
      { "uniquelocal",     no_argument,       nullptr, 'u' },
      { "uniquelocalhq",   no_argument,       nullptr, 'U' },
      { "nocolour",        no_argument,       nullptr, 'c' },
      { "nocolor",         no_argument,       nullptr, 'c' },
      { "noreverselookup", no_argument,       nullptr, 'n' },
      { "nogeoiplookup",   no_argument,       nullptr, 'g' },
      { "format",          required_argument, nullptr, 'F' },
      { "help",            no_argument,       nullptr, 'h' },
      { "version",         no_argument,       nullptr, 'v' },
      { nullptr,           0,                 nullptr, 0   }
   };

   OutputFormat outputFormat    = OutputFormat::Text;
   int          addressFamily   = AF_UNSPEC;
   bool         colourMode      = true;
   bool         noReverseLookup = false;
   bool         noGeoIPLookup   = false;
   unsigned int uniqueLocal     = 0;
   int          option          { 0 };
   int          longIndex       { 0 };

   while((option = getopt_long_only(argc, argv, "46uUcnghvF:", long_options, &longIndex)) != -1) {
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
         case 'F': {
            const std::string_view fmt = optarg;
            if(fmt == "text") {
               outputFormat = OutputFormat::Text;
            }
            else if(fmt == "json") {
               outputFormat = OutputFormat::JSON;
            }
            else if(fmt == "yaml") {
               outputFormat = OutputFormat::YAML;
            }
            else if(fmt == "xml") {
               outputFormat = OutputFormat::XML;
            }
            else {
               // TRANSLATORS: Error message when output format is invalid:
               // {0} = input format
               std::cerr << formatText(gettext("ERROR: Invalid format {0}!"), fmt) << '\n';
               return 1;
            }
            break;
         }
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

#if defined(_WIN32)
   struct WinSockScope {
      WSADATA Data { };
      WinSockScope() { WSAStartup(MAKEWORD(2, 2), &Data); }
      ~WinSockScope() { WSACleanup(); }
   } winSockScope;
#endif

   if(outputFormat != OutputFormat::Text) {
      std::cout.imbue( std::locale::classic());
   }

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
   const auto             slashPos = firstArg.find('/');

   if(slashPos != std::string_view::npos) {
      const std::string_view addressPart = firstArg.substr(0, slashPos);
      const std::string_view netmaskPart = firstArg.substr(slashPos + 1);

      if(!string2address(addressPart, &address, true, addressFamily)) {
         // TRANSLATORS: Error message when address string is invalid:
         // {0} = input address
         std::cerr << formatText(gettext("ERROR: Invalid address {0}!"), addressPart) << '\n';
         return 1;
      }
      if(readPrefix(netmaskPart, address, netmask) < 0) {
         if(!string2address(netmaskPart, &netmask, true, addressFamily)) {
            // TRANSLATORS: Error message when netmask string is invalid:
            // {0} = input netmask
            std::cerr << formatText(gettext("ERROR: Invalid netmask {0}!"), netmaskPart) << '\n';
            return 1;
         }
      }
   }
   else {
      if(!string2address(firstArg, &address, true, addressFamily)) {
         // TRANSLATORS: Error message when address string is invalid:
         // {0} = input address
         std::cerr << formatText(gettext("ERROR: Invalid address {0}!"), firstArg) << '\n';
         return 1;
      }
      if(optind + 1 < argc) {
         const std::string_view secondArg = argv[optind + 1];
         if(!secondArg.empty() && (secondArg.front() != '-')) {
            if(readPrefix(secondArg, address, netmask) < 0) {
               if(!string2address(secondArg, &netmask, true, addressFamily)) {
                  // TRANSLATORS: Error message when netmask string is invalid:
                  // {0} = input netmask
                  std::cerr << formatText(gettext("ERROR: Invalid netmask {0}!"), secondArg) << '\n';
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
      const std::string addressString = address2string(netmask, false);
      // TRANSLATORS: Error message when netmask string is invalid:
      // {0} = input netmask
      std::cerr << formatText(gettext("ERROR: Invalid netmask {0}!"), addressString) << '\n';
      return 1;
   }
   if(netmask.sa.sa_family != address.sa.sa_family) {
      const std::string addressString = address2string(netmask, false);
      // TRANSLATORS: Error message when address and netmask families mismatch:
      // {0} = netmask string
      std::cerr << formatText(gettext("ERROR: Incompatible netmask {0}!"), addressString) << '\n';
      return 1;
   }

   // ====== Unique Local IPv6 address generation ===========================
   if(uniqueLocal > 0) {
      generateUniqueLocal(address, (uniqueLocal > 1), outputFormat);
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
   if((hostBits < 64) && (hostBits > 0)) {
      maxHosts = (1ULL << hostBits) - reservedHosts;
   }
   else {
      maxHosts = 0;
   }
#endif

   // =======================================================================
   // STEP 1: Collect all details of the address into variables
   // =======================================================================
   SubnetDetails details;
   details.address     = address2string(address, false);
   details.family      = isIPv4(address) ? "IPv4" : "IPv6";
   details.network     = address2string(network, false);
   details.prefix      = prefix;
   details.netmask     = address2string(netmask, false);
   details.wildcard    = mask2string(wildcard);
   details.hostBits    = hostBits;
   details.isMulticast = isMulticast(address);

   if(isIPv4(address)) {
      details.hasHexAddress = true;
      details.hexAddress    = std::format("{:08X}", ntohl(address.in.sin_addr.s_addr));
      if(reservedHosts == 2) {
         details.hasBroadcast = true;
         details.broadcast    = address2string(broadcast, false);
      }
   }
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      uint16_t word[8];
      for(int i = 0; i < 8; i++) {
         word[i] = static_cast<uint16_t>((ipv6address.s6_addr[i * 2] << 8) | ipv6address.s6_addr[i * 2 + 1]);
      }
      details.expandedAddress = std::format("{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}",
                                            word[0], word[1], word[2], word[3],
                                            word[4], word[5], word[6], word[7]);
   }

   if(!details.isMulticast) {
      details.hostMin = address2string(host1, false);
      details.hostMax = address2string(host2, false);

#if defined(__SIZEOF_INT128__)
      details.hasMaxHosts   = true;
      details.maxHostsCount = (maxHosts > 0) ? toString(maxHosts) : "0";
      if(maxHosts > 0) {
         details.maxHostsText = std::format("{}   (2^{} - {})", details.maxHostsCount, hostBits, reservedHosts);
      }
      else {
         details.maxHostsText = std::format("2^{} - {}", hostBits, reservedHosts);
      }
#else
      if(hostBits < 64) {
         details.hasMaxHosts   = true;
         details.maxHostsCount = (maxHosts > 0) ? std::to_string(maxHosts) : "0";
         if(maxHosts > 0) {
            details.maxHostsText = std::format("{}   (2^{} - {})", details.maxHostsCount, hostBits, reservedHosts);
         }
         else {
            details.maxHostsText = std::format("2^{} - {}", hostBits, reservedHosts);
         }
      }
      else {
         details.hasMaxHosts  = false;
         details.maxHostsText = std::format("2^{} - {}", hostBits, reservedHosts);
      }
#endif
   }

   // ====== Collect Classification & Properties ============================
   if(isMulticast(address)) {
      details.addressType = "multicast";
      // TRANSLATORS: Address property: {0} = address string
      details.addressTypeDescription =
         formatText(gettext("{0} is a MULTICAST address"), details.address);
   }
   else if(isIPv4(address) && (prefix < 32) && (address == broadcast)) {
      details.addressType = "broadcast";
      // TRANSLATORS: Address property: {0} = address, {1} = network, {2} = prefix
      details.addressTypeDescription =
         formatText(gettext("{0} is the BROADCAST address of {1}/{2}"),
                    details.address, details.network, prefix);
   }
   else if((address == network) &&
           ((isIPv4(address) && (prefix < 32)) || (!isIPv4(address) && (prefix < 128)))) {
      details.addressType = "network";
      // TRANSLATORS: Address property: {0} = address string
      details.addressTypeDescription =
         formatText(gettext("{0} is a NETWORK address"), details.address);
   }
   else {
      details.addressType = "host";
      // TRANSLATORS: Address property: {0} = address, {1} = network, {2} = prefix
      details.addressTypeDescription =
         formatText(gettext("{0} is a HOST address in {1}/{2}"),
                    details.address, details.network, prefix);
   }

   if(isIPv4(address)) {
      const in_addr_t    ipv4address = ntohl(getIPv4Address(address));
      const unsigned int a           = ipv4address >> 24;
      const unsigned int b           = (ipv4address & 0x00ff0000) >> 16;

      if(IN_CLASSA(ipv4address)) {
         details.ipv4Class = "Class A";
         if(ipv4address == INADDR_LOOPBACK) {
            details.ipv4Flags.push_back("Loopback address");
         }
         else if(a == IN_LOOPBACKNET) {
            details.ipv4Flags.push_back("In loopback network");
         }
         else if(a == 10) {
            details.ipv4Flags.push_back("Private");
         }
      }
      else if(IN_CLASSB(ipv4address)) {
         details.ipv4Class = "Class B";
         if((a == 172) && ((b >= 16) && (b <= 31))) {
            details.ipv4Flags.push_back("Private");
         }
         else if((a == 169) && (b == 254)) {
            details.ipv4Flags.push_back("Link-local address");
         }
      }
      else if(IN_CLASSC(ipv4address)) {
         details.ipv4Class = "Class C";
         if((a == 192) && (b == 168)) {
            details.ipv4Flags.push_back("Private");
         }
      }
      else if(IN_CLASSD(ipv4address)) {
         details.ipv4Class = "Class D (Multicast)";
         if(a == 224) {
            details.multicastScope = "link-local";
         }
         else if((a == 239) && (b >= 192) && (b <= 251)) {
            details.multicastScope = "organization-local";
         }
         else if((a == 239) && (b >= 252) && (b <= 255)) {
            details.multicastScope = "site-local";
         }
         else {
            details.multicastScope = "global";
         }

         details.multicastMac = std::format("01:00:5e:{:02x}:{:02x}:{:02x}",
                                            (ipv4address & 0x007f0000) >> 16,
                                            (ipv4address & 0x0000ff00) >> 8,
                                            (ipv4address & 0x000000ff));

         if(a == 232) {
            details.isSourceSpecificMulticast = true;
         }
      }
      else {
         details.ipv4Class = "Invalid (not in class A, B, C or D)";
      }
   }
   else {
      // IPv6 Properties
      const in6_addr ipv6address = getIPv6Address(address);
      const uint16_t word0       = static_cast<uint16_t>((ipv6address.s6_addr[0] << 8) | ipv6address.s6_addr[1]);
      const uint16_t word1       = static_cast<uint16_t>((ipv6address.s6_addr[2] << 8) | ipv6address.s6_addr[3]);
      const uint16_t word5       = static_cast<uint16_t>((ipv6address.s6_addr[10] << 8) | ipv6address.s6_addr[11]);
      const uint16_t word6       = static_cast<uint16_t>((ipv6address.s6_addr[12] << 8) | ipv6address.s6_addr[13]);
      const uint16_t word7       = static_cast<uint16_t>((ipv6address.s6_addr[14] << 8) | ipv6address.s6_addr[15]);

      if(IN6_IS_ADDR_LOOPBACK(&ipv6address)) {
         details.ipv6Special = "Loopback address";
      }
      else if(IN6_IS_ADDR_UNSPECIFIED(&ipv6address)) {
         details.ipv6Special = "Unspecified address";
      }
      else if(IN6_IS_ADDR_V4COMPAT(&ipv6address)) {
         details.ipv6Special = "IPv4-compatible IPv6 address";
      }
      else if(IN6_IS_ADDR_V4MAPPED(&ipv6address)) {
         details.ipv6Special = "IPv4-mapped IPv6 address";
      }
      else if(hasTranslationPrefix(&address.in6)) {
         details.ipv6Special = "IPv4-embedded IPv6 address";
      }
      else if(IN6_IS_ADDR_MULTICAST(&ipv6address)) {
         if(IN6_IS_ADDR_MC_NODELOCAL(&ipv6address)) {
            details.multicastScope = "node-local";
         }
         else if(IN6_IS_ADDR_MC_LINKLOCAL(&ipv6address)) {
            details.multicastScope = "link-local";
         }
         else if(IN6_IS_ADDR_MC_SITELOCAL(&ipv6address)) {
            details.multicastScope = "site-local";
         }
         else if(IN6_IS_ADDR_MC_ORGLOCAL(&ipv6address)) {
            details.multicastScope = "organization-local";
         }
         else if(IN6_IS_ADDR_MC_GLOBAL(&ipv6address)) {
            details.multicastScope = "global";
         }
         else {
            details.multicastScope = "unknown";
         }

         const uint8_t flags = (ipv6address.s6_addr[1] & 0xf0) >> 4;
         if(flags == 0x1) {
            details.isTransientMulticast = true;
         }

         details.multicastMac = std::format("33:33:{:02x}:{:02x}:{:02x}:{:02x}",
                                            ipv6address.s6_addr[12], ipv6address.s6_addr[13],
                                            ipv6address.s6_addr[14], ipv6address.s6_addr[15]);

         if(((word0 & 0xfff0) == 0xff30) && (word1 == 0x0000)) {
            details.isSourceSpecificMulticast = true;
         }

         if((word0 == 0xff02) && (word5 == 0x0001) && ((word6 & 0xff00) == 0xff00)) {
            details.solicitedNodeFor = std::format("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xx{:02x}:{:04x}",
                                                   word6 & 0xff, word7);
         }
      }
      else {
         // Unicast properties
         bool hasSubnetID = true;
         bool hasGlobalID = false;

         if(IN6_IS_ADDR_LINKLOCAL(&ipv6address)) {
            details.unicastType = "Link-Local Unicast";
            hasSubnetID         = false;
         }
         else if(IN6_IS_ADDR_SITELOCAL(&ipv6address)) {
            details.unicastType = "Site-Local Unicast";
         }
         else if((word0 & 0xfc00) == 0xfc00) {
            details.unicastType = "Unique Local Unicast";
            hasGlobalID         = true;
            if(word0 & 0x0100) {
               details.ulaType = "Locally chosen";
            }
            else {
               details.ulaType = "Assigned by global instance";
            }
         }
         else if((word0 & 0xe000) == 0x2000) {
            details.unicastType = "Global Unicast";
            hasSubnetID         = false;
            if((word0 & 0x2002) == 0x2002) {
               details.hasSixToFour = true;
               sockaddr_union sixToFour { };
               sixToFour.sa.sa_family       = AF_INET;
               const uint32_t u = static_cast<uint32_t>((ipv6address.s6_addr[2] << 8) | ipv6address.s6_addr[3]);
               const uint32_t l = static_cast<uint32_t>((ipv6address.s6_addr[4] << 8) | ipv6address.s6_addr[5]);
               sixToFour.in.sin_addr.s_addr = htonl((u << 16) | l);
               details.sixToFourAddress     = address2string(sixToFour, false);
            }
         }

         uint16_t word[8];
         for(int i = 0; i < 8; i++) {
            word[i] = static_cast<uint16_t>((ipv6address.s6_addr[i * 2] << 8) | ipv6address.s6_addr[i * 2 + 1]);
         }

         if(hasGlobalID) {
            details.hasGlobalID = true;
            details.globalID    = std::format("{:02x}{:04x}{:04x}", word[0] & 0xff, word[1], word[2]);
         }
         if(hasSubnetID) {
            details.hasSubnetID = true;
            details.subnetID    = std::format("{:04x}", word[3]);
         }

         details.interfaceID = std::format("{:04x}:{:02x}{:02x}:{:02x}{:02x}:{:04x}",
                                           word[4], (word[5] & 0xff00) >> 8, (word[5] & 0x00ff),
                                           (word[6] & 0xff00) >> 8, (word[6] & 0x00ff), word[7]);

         if(((word[5] & 0x00ff) == 0x00ff) && ((word[6] & 0xff00) == 0xfe00)) {
            details.hasMACAddress = true;
            details.macAddress    = std::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                                                ipv6address.s6_addr[8] ^ 0x02, ipv6address.s6_addr[9], ipv6address.s6_addr[10],
                                                ipv6address.s6_addr[13], ipv6address.s6_addr[14], ipv6address.s6_addr[15]);
         }

         details.solicitedNodeMulticast = std::format("ff02::1:ff{:02x}:{:04x}", word[6] & 0xff, word[7]);
      }
   }

   // ====== Collect GeoIP Information ======================================
#ifdef HAVE_MAXMINDDB
   if(!noGeoIPLookup) {
      const std::string mmdbASNDatabasePath = find_mmdb_path("GeoLite2-ASN.mmdb");
      if(!mmdbASNDatabasePath.empty()) {
         MMDB_s mmdb;
         if(MMDB_open(mmdbASNDatabasePath.c_str(), MMDB_MODE_MMAP, &mmdb) == MMDB_SUCCESS) {
            int mmdbErrorCode = 0;
            auto mmdbLookupResult = MMDB_lookup_sockaddr(&mmdb, &address.sa, &mmdbErrorCode);
            if(mmdbLookupResult.found_entry) {
               MMDB_entry_data_s mmdbEntryData;

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "autonomous_system_number", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data &&
                  (mmdbEntryData.type == MMDB_DATA_TYPE_UINT32)) {
                  details.geoIPASNumber = mmdbEntryData.uint32;
               }

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "autonomous_system_organization", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPASOrganization = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               if(!details.geoIPASOrganization.empty() && (details.geoIPASNumber > 0)) {
                  details.geoIPASInfo = std::format("{} (AS{})", details.geoIPASOrganization, details.geoIPASNumber);
                  details.hasGeoIPASN = true;
               }
               else if(details.geoIPASNumber > 0) {
                  details.geoIPASInfo = std::format("AS{}", details.geoIPASNumber);
                  details.hasGeoIPASN = true;
               }
               else if(!details.geoIPASOrganization.empty()) {
                  details.geoIPASInfo = details.geoIPASOrganization;
                  details.hasGeoIPASN = true;
               }
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
            auto mmdbLookupResult = MMDB_lookup_sockaddr(&mmdb, &address.sa, &mmdbErrorCode);
            if(mmdbLookupResult.found_entry) {
               MMDB_entry_data_s mmdbEntryData;

               // TRANSLATORS: Default fallback text when GeoIP data is unavailable
               details.geoIPCountry = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "country", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPCountry = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               details.geoIPCountryCode = "??";
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "country", "iso_code", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPCountryCode = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "postal", "code", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPPostalCode = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               // TRANSLATORS: Default fallback text when GeoIP data is unavailable
               details.geoIPCity = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "city", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPCity = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               // TRANSLATORS: Default fallback text when GeoIP data is unavailable
               details.geoIPRegion = gettext("Unknown");
               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "subdivisions", "0", "names", "en", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPRegion = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "time_zone", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPTimeZone = std::string(mmdbEntryData.utf8_string, mmdbEntryData.data_size);
               }

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "latitude", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPLatitude = mmdbEntryData.double_value;
               }

               if((MMDB_get_value(&mmdbLookupResult.entry, &mmdbEntryData,
                                  "location", "longitude", nullptr) == MMDB_SUCCESS) &&
                  mmdbEntryData.has_data) {
                  details.geoIPLongitude = mmdbEntryData.double_value;
               }

               details.hasGeoIPCity = true;
            }
            MMDB_close(&mmdb);
         }
      }
   }
#endif

   // ====== Collect Reverse DNS Information ================================
   if(!noReverseLookup) {
      if((outputFormat == OutputFormat::Text) && isatty(fileno(stdout))) {
         // TRANSLATORS: Progress message while performing reverse DNS lookup
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

      if((outputFormat == OutputFormat::Text) && isatty(fileno(stdout))) {
         std::cout << "\r\x1b[K";
      }

      if(error == 0) {
#ifndef NI_IDN
         char* utf8hostname = nullptr;
         if(idn2_to_unicode_8z8z(hostname, &utf8hostname, 0) == IDN2_OK) {
            details.dnsHostname = utf8hostname;
            idn2_free(utf8hostname);
         }
         else {
            details.dnsHostname = hostname;
         }
#else
         details.dnsHostname = hostname;
#endif
         details.hasDNSHostname = true;
      }
      else {
         details.dnsError = std::format("({})", gai_strerror(error));
      }
   }

   // =======================================================================
   // STEP 2: Output
   // =======================================================================
   switch(outputFormat) {
      case OutputFormat::JSON:
         outputJSON(std::cout, details);
         break;
      case OutputFormat::YAML:
         outputYAML(std::cout, details);
         break;
      case OutputFormat::XML:
         outputXML(std::cout, details);
         break;
      case OutputFormat::Text:
      default:
         outputPlainText(std::cout, details, address, netmask,
                         static_cast<unsigned int>(prefix), colourMode);
         break;
   }

   return 0;
}
