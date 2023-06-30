/*
 * IPv4/IPv6 Subnet Calculator
 * Copyright (C) 2002-2021 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: thomas.dreibholz@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <locale.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#ifdef HAVE_GEOIP
#include <GeoIP.h>
#include <GeoIPCity.h>
#endif

#include "tools.h"



#ifdef __FreeBSD__   // FreeBSD
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#ifdef __APPLE__   // MacOS X
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#ifdef __sun   // SunOS and Solaris
#define s6_addr16 _S6_un._S6_u16
#define s6_addr32 _S6_un._S6_u32
#endif


// ###### Is given address an IPv4 address? #################################
inline bool isIPv4(const sockaddr_union& address)
{
   return(address.sa.sa_family == AF_INET);
}


// ###### Extract IPv4 address from address #################################
inline in_addr_t getIPv4Address(const sockaddr_union& address)
{
   assert(address.sa.sa_family == AF_INET);
   return(address.in.sin_addr.s_addr);
}


// ###### Extract IPv4 address from address #################################
inline in6_addr getIPv6Address(const sockaddr_union& address)
{
   assert(address.sa.sa_family == AF_INET6);
   return(address.in6.sin6_addr);
}


// ###### Is given address a multicast address? #############################
inline bool isMulticast(const sockaddr_union& address)
{
   if(isIPv4(address)) {
      return(IN_CLASSD(ntohl(getIPv4Address(address))));
   }
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      return(IN6_IS_ADDR_MULTICAST(&ipv6address));
   }
}


// ###### Read prefix from parameter ########################################
int readPrefix(const char*           parameter,
               const sockaddr_union& forAddress,
               sockaddr_union&       netmask)
{
   for(size_t i = 0;i < strlen(parameter);i++) {
      if(!isdigit(parameter[i])) {
         return(-1);
      }
   }
   int prefix = atol(parameter);
   if(prefix < 0) {
      return(-1);
   }
   netmask = forAddress;
   if(netmask.sa.sa_family == AF_INET) {
      if(prefix > 32) {
         return(-1);
      }
      int p = prefix;
      netmask.in.sin_addr.s_addr = 0;
      for(int i = 31;i >= 0;i--) {
         if(p > 0) {
            netmask.in.sin_addr.s_addr |= (1 << i);
         }
         p--;
      }
      netmask.in.sin_addr.s_addr = ntohl(netmask.in.sin_addr.s_addr);
   }
   else {
      if(prefix > 128) {
         return(-1);
      }
      int p = prefix;
      for(int j = 0;j < 16;j++) {
         netmask.in6.sin6_addr.s6_addr[j] = 0;
         for(int i = 7;i >= 0;i--) {
            if(p > 0) {
               netmask.in6.sin6_addr.s6_addr[j] |= (1 << i);
            }
            p--;
         }
      }
   }
   return(prefix);
}


// ###### Generate a unique local IPv6 address ##############################
void generateUniqueLocal(sockaddr_union& address,
                         const bool      highQualityRng = false)
{
   uint8_t buffer[5];
   if(address.sa.sa_family != AF_INET6) {
      std::cerr << "ERROR: An IPv6 address must be given to generate a unique local address!" << std::endl;
      exit(1);
   }

#if defined(__LINUX__) || defined(__linux__) || defined(__linux)
   // ====== Read random number from random device ==========================
   const char* randomFile = (highQualityRng == true) ? "/dev/random" : "/dev/urandom";
   FILE* fh = fopen(randomFile, "r");
   if(fh != NULL) {
      std::cout << "Generating Unique Local IPv6 address (using " << randomFile << ") ..." << std::endl;

      if(fread((char*)&buffer, 5, 1, fh) != 1) {
         std::cerr << "ERROR: Unable to read from " << randomFile << "!" << std::endl;
         exit(1);
      }
      fclose(fh);
   }
   else {
      std::cerr << "ERROR: Unable to open " << randomFile << "!" << std::endl;
      exit(1);
   }
#else
   // ====== Get random number using random() function ======================
#warning Using default random number generator on non-Linux system!
   srandom((unsigned int)getMicroTime());
   for(size_t i = 0;i < sizeof(buffer);i++) {
      buffer[i] = (uint8_t)(random() % 0xff);
   }
#endif

   // ====== Create IPv6 Unique Local address ===============================
   address.in6.sin6_addr.s6_addr[0] = 0xfd;
   memcpy((char*)&address.in6.sin6_addr.s6_addr[1], (const char*)&buffer, sizeof(buffer));
}


// ###### Print IPv4 address in binary digits ###############################
void printAddressBinary(std::ostream&         os,
                        const sockaddr_union& address,
                        const unsigned int    prefix,
                        const bool            colourMode = true,
                        const char*           indent     = "")
{
   if(address.sa.sa_family == AF_INET) {
      os << indent;
      uint32_t a = ntohl(getIPv4Address(address));
      for(int i = 31;i >= 0;i--) {
         const uint32_t v = (uint32_t)1 << i;
         if(colourMode) {
            if(32 - i > (int)prefix) {   // Colourize output
               os << "\x1b[33m";
            }
            else {
               os << "\x1b[34m";
            }
         }
         if(a >= v) {
            os << "1";
            a -= v;
         }
         else {
            os << "0";
         }
         if(colourMode) {
            os << "\x1b[0m";   // Turn off colour printing
         }
         if( ((i % 8) == 0) && (i > 0) ) {
            os << " . ";
         }
      }
      os << std::endl;
   }
   else {
      int      p           = 0;
      in6_addr ipv6Address = getIPv6Address(address);
      for(int j = 0;j < 8;j++) {
         uint16_t a = ntohs(ipv6Address.s6_addr16[j]);
         char str[16];
         snprintf((char*)&str, sizeof(str), "%04x", a);
         os << indent << str << " = ";
         for(int i = 15;i >= 0;i--) {
            const uint32_t v = (uint32_t)1 << i;
            if(colourMode) {
               if(p < (int)prefix) {   // Colourize output
                  os << "\x1b[33m";
               }
               else {
                  os << "\x1b[34m";
               }
            }
            if(a >= v) {
               os << "1";
               a -= v;
            }
            else {
               os << "0";
            }
            if(colourMode) {
               os << "\x1b[0m";   // Turn off colour printing
            }
            if( ((i % 8) == 0) && (i > 0) ) {
               os << " ";
            }
            p++;
         }
         os << std::endl;
      }
   }
}


// ###### Is given netmask valid? ###########################################
int getPrefixLength(const sockaddr_union& netmask)
{
   int  prefixLength;
   bool belongsToNetwork = true;

   if(netmask.sa.sa_family == AF_INET) {
      prefixLength     = 32;
      const uint32_t a = ntohl(getIPv4Address(netmask));
      for(int i = 31;i >= 0;i--) {
         if(!(a & (1 << (uint32_t)i))) {
            belongsToNetwork = false;
            prefixLength--;
         }
         else {
            if(belongsToNetwork == false) {
               return(-1);
            }
         }
      }
   }
   else {
      prefixLength = 128;
      for(int j = 0;j < 4;j++) {
         const uint32_t a = ntohl(getIPv6Address(netmask).s6_addr32[j]);
         for(int i = 31;i >= 0;i--) {
            if(!(a & (1 << (uint32_t)i))) {
               belongsToNetwork = false;
               prefixLength--;
            }
            else {
               if(belongsToNetwork == false) {
                  return(-1);
               }
            }
         }
      }
   }
   return(prefixLength);
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
      for(int j = 0;j < 4;j++) {
         a.in6.sin6_addr.s6_addr32[j] &= a2.in6.sin6_addr.s6_addr32[j];
      }
   }
   return(a);
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
      for(int j = 0;j < 4;j++) {
         a.in6.sin6_addr.s6_addr32[j] |= a2.in6.sin6_addr.s6_addr32[j];
      }
   }
   return(a);
}


// ###### "~" operator for addresses ########################################
sockaddr_union operator~(const sockaddr_union& a1)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = ~a1.in.sin_addr.s_addr;
   }
   else {
      for(int j = 0;j < 4;j++) {
         a.in6.sin6_addr.s6_addr32[j] = ~a1.in6.sin6_addr.s6_addr32[j];
      }
   }
   return(a);
}


// ###### Output operator for addresses #####################################
std::ostream& operator<<(std::ostream& os, const sockaddr_union& a)
{
   printAddress(os, &a.sa, false, true);
   return(os);
}


// ###### "+" operator for addresses ########################################
sockaddr_union operator+(const sockaddr_union& a1, uint32_t n)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = htonl(ntohl(a.in.sin_addr.s_addr) + n);
   }
   else {
      for(int j = 3;j >= 0;j--) {
         const uint64_t sum = (uint64_t)ntohl(a.in6.sin6_addr.s6_addr32[j]) + (uint64_t)n;
         a.in6.sin6_addr.s6_addr32[j] = htonl((uint32_t)(sum & 0xffffffffULL));
         n = (uint32_t)(sum >> 32);
      }
   }
   return(a);
}


// ###### "-" operator for addresses ########################################
sockaddr_union operator-(const sockaddr_union& a1, uint32_t n)
{
   sockaddr_union a = a1;
   if(a.sa.sa_family == AF_INET) {
      a.in.sin_addr.s_addr = htonl(ntohl(a.in.sin_addr.s_addr) - n);
   }
   else {
      for(int j = 3;j >= 0;j--) {
         const uint64_t sum = (uint64_t)ntohl(a.in6.sin6_addr.s6_addr32[j]) - (uint64_t)n;
         a.in6.sin6_addr.s6_addr32[j] = htonl((uint32_t)(sum & 0xffffffffULL));
         n = (uint32_t)(sum >> 32);
      }
   }
   return(a);
}


// ###### "==" operator for addresses #######################################
int operator==(const sockaddr_union& a1, const sockaddr_union& a2)
{
   assert(a1.sa.sa_family == a2.sa.sa_family);

   if(a1.sa.sa_family == AF_INET) {
      return(a1.in.sin_addr.s_addr == a2.in.sin_addr.s_addr);
   }
   else {
      for(int j = 3;j >= 0;j--) {
         if(a1.in6.sin6_addr.s6_addr32[j] != a2.in6.sin6_addr.s6_addr32[j]) {
            return(false);
         }
      }
      return(true);
   }
}


// ###### Print IPv6 unicast properties of given address ####################
void printUnicastProperties(std::ostream&   os,
                            const in6_addr& ipv6address,
                            const bool      colourMode  = true,
                            const bool      hasSubnetID = true,
                            const bool      hasGlobalID = false)
{
   // ====== Global ID ======================================================
   if(hasGlobalID) {
      char globalIDString[16];
      snprintf((char*)&globalIDString, sizeof(globalIDString), "%02x%04x%04x",
               ntohs(ipv6address.s6_addr16[0]) & 0xff,
               ntohs(ipv6address.s6_addr16[1]),
               ntohs(ipv6address.s6_addr16[2]));
      os << "      + Global ID    = " << globalIDString << std::endl;
   }

   // ====== Subnet ID ======================================================
   if(hasSubnetID) {
      char           subnetIDString[16];
      const uint16_t subnetID = ntohs(ipv6address.s6_addr16[3]);
      snprintf((char*)&subnetIDString, sizeof(subnetIDString), "%04x", subnetID);
      os << "      + Subnet ID    = " << subnetIDString << std::endl;
   }

   // ====== Interface ID ===================================================
   char           interfaceIDString[128];
   const uint16_t interfaceID[4] = { ntohs(ipv6address.s6_addr16[4]),
                                     ntohs(ipv6address.s6_addr16[5]),
                                     ntohs(ipv6address.s6_addr16[6]),
                                     ntohs(ipv6address.s6_addr16[7]) };
   snprintf((char*)&interfaceIDString, sizeof(interfaceIDString),
            ((colourMode == true) ? "\x1b[36m%04x:%02x\x1b[37m%02x:%02x\x1b[38m%02x:%04x\x1b[0m" :
                                    "%04x:%02x%02x:%02x%02x:%04x"),
            interfaceID[0],
            (interfaceID[1] & 0xff00) >> 8, (interfaceID[1] & 0x00ff),
            (interfaceID[2] & 0xff00) >> 8, (interfaceID[2] & 0x00ff),
            interfaceID[3]);
   os << "      + Interface ID = " << interfaceIDString << std::endl;

   if( ((interfaceID[1] & 0x00ff) == 0x00ff) &&
       ((interfaceID[2] & 0xff00) == 0xfe00) ) {
      snprintf((char*)&interfaceIDString, sizeof(interfaceIDString),
               ((colourMode == true) ? "\x1b[36m%02x:%02x:%02x\x1b[0m:\x1b[38m%02x:%02x:%02x\x1b[0m" :
                                       "%02x:%02x:%02x:%02x:%02x:%02x"),
               ipv6address.s6_addr[8] ^ 0x02,
               ipv6address.s6_addr[9],
               ipv6address.s6_addr[10],
               ipv6address.s6_addr[13],
               ipv6address.s6_addr[14],
               ipv6address.s6_addr[15]);
      os << "      + MAC address  = " << interfaceIDString << std::endl;
   }

   // ====== Solicited Node Multicast Address ===============================
   char snmcAddressString[32];
   snprintf((char*)&snmcAddressString, sizeof(snmcAddressString),
            ((colourMode == true) ? "\x1b[32mff02::1:ff\x1b[38m%02x:%04x\x1b[0m" :
                                    "ff02::1:ff%02x:%04x"),
            ntohs(ipv6address.s6_addr16[6]) & 0xff,
            ntohs(ipv6address.s6_addr16[7]));
   os << "      + Sol. Node MC = " << snmcAddressString << std::endl;
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
   // ====== Common properties ==============================================
   os << "Properties    =" << std::endl;
   if(isMulticast(address)) {
      os << "   - " << address << " is a MULTICAST address" << std::endl;
   }
   else if(address == network) {
      os << "   - " << address << " is a NETWORK address" << std::endl;
   }
   else if((isIPv4(address)) && (address == broadcast)) {
      os << "   - " << address << " is the BROADCAST address of "
         << network << "/" << prefix << std::endl;
   }
   else {
      os << "   - " << address << " is a HOST address in "
         << network << "/" << prefix << std::endl;
   }


   // ====== IPv4 properties ================================================
   if(isIPv4(address)) {
      const in_addr_t    ipv4address = ntohl(getIPv4Address(address));
      const unsigned int a           = ipv4address >> 24;
      const unsigned int b           = (ipv4address & 0x00ff0000) >> 16;

      if(IN_CLASSA(ipv4address)) {
         os << "   - Class A" << std::endl;
         if(ipv4address == INADDR_LOOPBACK) {
            os << "   - Loopback address" << std::endl;
         }
         else if(a == IN_LOOPBACKNET) {
            os << "   - In loopback network" << std::endl;
         }
         else if(a == 10) {
            os << "   - Private" << std::endl;
         }
      }
      else if(IN_CLASSB(ipv4address)) {
         os << "   - Class B" << std::endl;
         if((a == 172) && ((b >= 16) && (b <= 31))) {
            os << "   - Private" << std::endl;
         }
         else if((a == 169) && (b == 254)) {
            os << "   - Link-local address" << std::endl;
         }
      }
      else if(IN_CLASSC(ipv4address)) {
         os << "   - Class C" << std::endl;
         if((a == 192) && (b == 168)) {
            os << "   - Private" << std::endl;
         }
      }
      else if(IN_CLASSD(ipv4address)) {
         os << "   - Class D (Multicast)" << std::endl;
         // ------ Multicast scope ------------------------------------------
         os << "      + Scope: ";
         if(a == 224) {
            os << "link-local";
         }
         else if((a == 239) && (b >= 192) && (b <= 251)) {
            os << "organization-local";
         }
         else if((a == 239) && (b >= 252) && (b <= 255)) {
            os << "site-local";
         }
         else {
            os << "global";
         }
         os << std::endl;

         // ------ Corresponding MAC address --------------------------------
         char macAddressString[32];
         snprintf((char*)&macAddressString, sizeof(macAddressString),
                  "01:00:5e:%02x:%02x:%02x",
                  (ipv4address & 0x007f0000) >> 16,
                  (ipv4address & 0x0000ff00) >> 8,
                  (ipv4address & 0x000000ff));
         os << "      + Corresponding multicast MAC address: " << macAddressString << std::endl;

         // ------ Source-specific multicast --------------------------------
         if(a == 232) {
            os << "      + Source-specific multicast" << std::endl;
         }
      }
      else {
         os << "   - Invalid (not in class A, B, C or D)" << std::endl;
      }
   }


   // ====== IPv6 properties ================================================
   else {
      const in6_addr ipv6address = getIPv6Address(address);
      const uint16_t a           = ntohs(ipv6address.s6_addr16[0]);
      const uint16_t b           = ntohs(ipv6address.s6_addr16[1]);

      // ------ Special addresses -------------------------------------------
      if(IN6_IS_ADDR_LOOPBACK(&ipv6address)) {
         os << "   - Loopback address" << std::endl;
      }
      else if(IN6_IS_ADDR_UNSPECIFIED(&ipv6address)) {
         os << "   - Unspecified address" << std::endl;
      }
      else if(IN6_IS_ADDR_V4COMPAT(&ipv6address)) {
         os << "   - IPv4-compatible IPv6 address" << std::endl;
      }
      else if(IN6_IS_ADDR_V4MAPPED(&ipv6address)) {
         os << "   - IPv4-mapped IPv6 address" << std::endl;
      }

      // ------ Multicast addresses -----------------------------------------
      else if(IN6_IS_ADDR_MULTICAST(&ipv6address)) {
         // ------ Multicast scope ------------------------------------------
         os << "   - Multicast Properties" << std::endl;
         os << "      + Scope: ";
         if(IN6_IS_ADDR_MC_NODELOCAL(&ipv6address)) {
            os << "node-local";
         }
         else if(IN6_IS_ADDR_MC_LINKLOCAL(&ipv6address)) {
            os << "link-local";
         }
         else if(IN6_IS_ADDR_MC_SITELOCAL(&ipv6address)) {
            os << "site-local";
         }
         else if(IN6_IS_ADDR_MC_ORGLOCAL(&ipv6address)) {
            os << "organization-local";
         }
         else if(IN6_IS_ADDR_MC_GLOBAL(&ipv6address)) {
            os << "global";
         }
         else {
            os << "unknown";
         }
         os << std::endl;

         // ------ Multicast flags ------------------------------------------
         const uint8_t flags = (((uint8_t*)&ipv6address)[1] & 0xf0) >> 4;
         if(flags == 0x1) {
            os << "      + Temporary-allocated address" << std::endl;
         }

         // ------ Corresponding MAC address --------------------------------
         char macAddressString[32];
         snprintf((char*)&macAddressString, sizeof(macAddressString),
                  "33:33:%02x:%02x:%02x:%02x",
                  ipv6address.s6_addr[12],
                  ipv6address.s6_addr[13],
                  ipv6address.s6_addr[14],
                  ipv6address.s6_addr[15]);
         os << "      + Corresponding multicast MAC address: " << macAddressString << std::endl;

         // ------ Source-specific multicast --------------------------------
         if( ((a & 0xfff0) == 0xff30) && (b == 0x0000) ) {
            // FF0x:0::/32
            os << "      + Source-specific multicast" << std::endl;
         }

         // ------ Solicited node multicast address -------------------------
         // FF02::1:FF00:0/104
         if((a == 0xff02) &&
            (ntohs(ipv6address.s6_addr16[5]) == 0x0001) &&
            (ntohs(ipv6address.s6_addr16[6]) & 0xff00) == 0xff00) {
            char nodeAddressString[64];
            snprintf((char*)&nodeAddressString, sizeof(nodeAddressString),
                     "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xx%02x:%04x",
                     ntohs(ipv6address.s6_addr16[6]) & 0xff,
                     ntohs(ipv6address.s6_addr16[7]));
            os << "      + Address is solicited node multicast address for "
               << nodeAddressString << std::endl;
         }
      }

      // ------ Link-local Unicast ------------------------------------------
      else if(IN6_IS_ADDR_LINKLOCAL(&ipv6address)) {
         os << "   - Link-Local Unicast Properties:" << std::endl;
         printUnicastProperties(std::cout, ipv6address, colourMode, false, false);
      }

      // ------ Site-Local Unicast ------------------------------------------
      else if(IN6_IS_ADDR_SITELOCAL(&ipv6address)) {
         os << "   - Site-Local Unicast Properties:" << std::endl;
         printUnicastProperties(std::cout, ipv6address, colourMode, true, false);
      }

      // ------ Unique Local Unicast ----------------------------------------
      else if((a & 0xfc00) == 0xfc00) {
         os << "   - Unique Local Unicast Properties:" << std::endl;
         if(a & 0x0100) {
            os << "      + Locally chosen" << std::endl;
         }
         else {
            os << "      + Assigned by global instance" << std::endl;
         }
         printUnicastProperties(std::cout, ipv6address, colourMode, true, true);
      }

      // ------ Global Unicast ----------------------------------------------
      else if((a & 0xe000) == 0x2000) {
         os << "   - Global Unicast Properties:" << std::endl;
         printUnicastProperties(std::cout, ipv6address, colourMode, false, false);

         // ------ 6to4 Address ---------------------------------------------
         if((a & 0x2002) == 0x2002) {
            os << "      + 6to4 address = ";
            sockaddr_union sixToFour;
            sixToFour.sa.sa_family       = AF_INET;
            const uint32_t u = ntohs(((const uint16_t*)&ipv6address.s6_addr)[1]);
            const uint32_t l = ntohs(((const uint16_t*)&ipv6address.s6_addr)[2]);
            sixToFour.in.sin_addr.s_addr = htonl((u << 16) | l);
            printAddress(std::cerr, &sixToFour.sa, false);
            os << std::endl;
         }

      }
   }
}



// ###### Main program ######################################################
int main(int argc, char** argv)
{
   bool               colourMode      = true;
   bool               noReverseLookup = false;
   int                options;
   int                prefix;
   unsigned int       hostBits;
   unsigned int       reservedHosts;
   unsigned long long maxHosts;
   sockaddr_union     network;
   sockaddr_union     address;
   sockaddr_union     netmask;
   sockaddr_union     broadcast;
   sockaddr_union     wildcard;
   sockaddr_union     host1;
   sockaddr_union     host2;


   // ====== Initialize locale ==============================================
   setlocale(LC_ALL, "");   // Necessary to work with IDN names!

   // ====== Print usage ====================================================
   if(argc < 2) {
      printf("Usage: %s [Address{/{Netmask|Prefix}}] "
             "{Netmask|Prefix} {-n} {-uniquelocal|-uniquelocalhq} {-nocolour|-nocolor}\n", argv[0]);
      exit(1);
   }

   // ====== Get address and netmask from one parameter =====================
   char* slash = index(argv[1], '/');
   if(slash) {
      slash[0] = 0x00;
      if(string2address(argv[1], &address) == false) {
         printf("ERROR: Bad address %s!\n", argv[1]);
         exit(1);
      }
      if( readPrefix((const char*)&slash[1], address, netmask) < 0 ) {
         if(string2address((const char*)&slash[1], &netmask) == false) {
            printf("ERROR: Bad netmask %s!\n", (const char*)&slash[1]);
            exit(1);
         }
      }
      options = 2;
   }

   // ====== Get address and netmask from separate parameters ===============
   else if(slash == NULL) {
      if(string2address(argv[1], &address) == false) {
         printf("ERROR: Bad address %s!\n", argv[1]);
         exit(1);
      }
      if(argc > 2) {
         options = 3;
         // ------ Get netmask or prefix ------------------------------------
         if(argv[2][0] != '-') {
            if( readPrefix(argv[2], address, netmask) < 0 ) {
               if(string2address(argv[2], &netmask) == false) {
                  printf("ERROR: Bad netmask %s!\n", argv[2]);
                  exit(1);
               }
            }
         }
         else {
            // ------ No netmask or prefix => use default for convenience ---
            options = 2;
            prefix = readPrefix((address.sa.sa_family == AF_INET) ? "32" : "128", address, netmask);
            assert(prefix >= 0);
         }
      }
      else {
         // ------ No netmask or prefix => use default for convenience ------
         options = 2;
         prefix = readPrefix((address.sa.sa_family == AF_INET) ? "32" : "128", address, netmask);
         assert(prefix >= 0);
      }
   }


   // ====== Get prefix length ==============================================
   prefix = getPrefixLength(netmask);
   if(prefix < 0) {
      std::cerr << "ERROR: Invalid netmask ";
      printAddress(std::cerr, &netmask.sa, false);
      std::cerr << "!" << std::endl;
      exit(1);
   }
   if(netmask.sa.sa_family != address.sa.sa_family) {
      std::cerr << "ERROR: Incompatible netmask ";
      printAddress(std::cerr, &netmask.sa, false);
      std::cerr << "!" << std::endl;
      exit(1);
   }


   // ====== Handle optional parameters =====================================
   for(int i = options;i < argc;i++) {
      if(strcmp(argv[i], "-uniquelocal") == 0) {
         generateUniqueLocal(address);
      }
      else if(strcmp(argv[i], "-uniquelocalhq") == 0) {
         generateUniqueLocal(address, true);
      }
      else if( (strcmp(argv[i], "-nocolour") == 0) || (strcmp(argv[i], "-nocolor") == 0) ) {
         colourMode = false;
      }
      else if(strcmp(argv[i], "-n") == 0) {
         noReverseLookup = true;
      }
      else {
         std::cerr << "ERROR: Bad argument " << argv[i] << "!" << std::endl;
         exit(1);
      }
   }


   // ====== Calculate network address, hosts, etc. =========================
   network   = address & netmask;
   broadcast = network | (~netmask);
   wildcard  = ~netmask;
   if(isIPv4(address)) {
      reservedHosts = 2;
      hostBits      = 32 - prefix;
      host1         = network + 1;
      host2         = broadcast - 1;
      if(prefix >= 31) {   // Special case for Point-to-Point links
         reservedHosts = 0;
         host1 = network;
         host2 = broadcast;
      }
   }
   else {
      reservedHosts = 1;
      if(prefix < 128) {
         hostBits = 128 - prefix;
         host1    = network + 1;
         host2    = broadcast;   // There is no broadcast address for IPv6!
      }
      else {
         hostBits = 1;
         host1    = network;
         host2    = broadcast;   // There is no broadcast address for IPv6!
      }
   }
   if(hostBits <= 64) {
      maxHosts = (unsigned long long)pow(2.0, (double)hostBits) - reservedHosts;
   }
   else {
      maxHosts = 0;   // Not enough accuracy for such a large number!
   }


   // ====== Print results ==================================================
   std::cout << "Address       = " << address << std::endl;
   printAddressBinary(std::cout, address, prefix, colourMode, "                   ");
   std::cout << "Network       = " << network << " / " << prefix << std::endl;
   std::cout << "Netmask       = " << netmask << std::endl;
   if(isIPv4(address)) {
      if(reservedHosts == 2) {
         std::cout << "Broadcast     = " << broadcast << std::endl;
      }
      else {
         std::cout << "Broadcast     = not needed on Point-to-Point links" << std::endl;
      }
   }
   std::cout << "Wildcard Mask = " << wildcard << std::endl;
   std::cout << "Hosts Bits    = " << hostBits << std::endl;
   if(!isMulticast(address)) {
      char maxHostsString[128];
      if(maxHosts > 0) {
         snprintf((char*)&maxHostsString, sizeof(maxHostsString),
                  "%llu   (2^%u - %u)", maxHosts, hostBits, reservedHosts);
      }
      else {
         snprintf((char*)&maxHostsString, sizeof(maxHostsString),
                  "2^%u - %u", hostBits, reservedHosts);
      }
      std::cout << "Max. Hosts    = " << maxHostsString << std::endl;
      std::cout << "Host Range    = { " << host1 << " - " << host2 << " }" << std::endl;
   }


   // ====== Properties =====================================================
   printAddressProperties(std::cout, address, netmask, prefix, network, broadcast, colourMode);


   // ====== GeoIP ==========================================================
#ifdef HAVE_GEOIP
   // libGeoIP prints an error message each time it cannot open a database.
   // Unfortunately, packaging of these databases is very confusing: some
   // distributions include some of them, etc.. Just avoid annoying the user
   // by printing these errors to /dev/null. The error condition of a
   // non-existing database is handled by subnetcalc anyway.
   if(freopen("/dev/null", "w", stderr) == NULL) { }

   const char* country = NULL;
   const char* code    = NULL;
   if(address.sa.sa_family == AF_INET) {
      GeoIP* geoIP = GeoIP_open_type(GEOIP_ASNUM_EDITION, GEOIP_STANDARD);
      if(geoIP) {
         const char* org = GeoIP_name_by_ipnum(geoIP, ntohl(address.in.sin_addr.s_addr));
         std::cout << "GeoIP AS Info = "
                   << ((org != NULL) ? org : "Unknown") << std::endl;
         GeoIP_delete(geoIP);
      }
      geoIP = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
      if(geoIP) {
         country = GeoIP_country_name_by_ipnum(geoIP, ntohl(address.in.sin_addr.s_addr));
         code    = GeoIP_country_code_by_ipnum(geoIP, ntohl(address.in.sin_addr.s_addr));
         std::cout << "GeoIP Country = "
                   << ((country != NULL) ? country: "Unknown")
                   << " (" << ((code != NULL) ? code : "??") << ")" << std::endl;
         GeoIP_delete(geoIP);
      }
      geoIP = GeoIP_open_type(GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD);
      if(geoIP) {
         GeoIPRecord* gir = GeoIP_record_by_ipnum(geoIP, ntohl(address.in.sin_addr.s_addr));
         if(gir != NULL) {
            const char* timeZone = GeoIP_time_zone_by_country_and_region(
                                      gir->country_code, gir->region);
            std::cout << "GeoIP Region  = "
                      << ((gir->postal_code != NULL) ? gir->postal_code : "")
                      << ((gir->postal_code != NULL) ? " " : "")
                      << ((gir->city != NULL) ? gir->city : "Unknown")
                      << ", " << ((gir->region != NULL) ? gir->region : "Unknown")
                      << " ("
                      << fabs(gir->latitude)  << "°" << ((gir->latitude > 0)  ? "N" : "S") << ", "
                      << fabs(gir->longitude) << "°" << ((gir->longitude > 0) ? "E" : "W")
                      << ((timeZone != NULL) ? ", " : "")
                      << ((timeZone != NULL) ? timeZone : "")
                      << ")"
                      << std::endl;
            GeoIPRecord_delete(gir);
         }
         GeoIP_delete(geoIP);
      }
   }
#ifdef HAVE_GEOIP_IPV6
   else if(address.sa.sa_family == AF_INET6) {
      GeoIP* geoIP = GeoIP_open_type(GEOIP_ASNUM_EDITION_V6, GEOIP_STANDARD);
      if(geoIP) {
         const char* org = GeoIP_name_by_ipnum_v6(geoIP, address.in6.sin6_addr);
         std::cout << "GeoIP AS Info = "
                   << ((org != NULL) ? org : "Unknown") << std::endl;
         GeoIP_delete(geoIP);
      }
      geoIP = GeoIP_open_type(GEOIP_COUNTRY_EDITION_V6, GEOIP_STANDARD);
      if(geoIP) {
         country = GeoIP_country_name_by_ipnum_v6(geoIP, address.in6.sin6_addr);
         code    = GeoIP_country_code_by_ipnum_v6(geoIP, address.in6.sin6_addr);
         std::cout << "GeoIP Country = "
                   << ((country != NULL) ? country: "Unknown")
                   << " (" << ((code != NULL) ? code : "??") << ")" << std::endl;
         GeoIP_delete(geoIP);
      }
      geoIP = GeoIP_open_type(GEOIP_CITY_EDITION_REV1_V6, GEOIP_STANDARD);
      if(geoIP) {
         GeoIPRecord* gir = GeoIP_record_by_ipnum_v6(geoIP, address.in6.sin6_addr);
         if(gir != NULL) {
            const char* timeZone = GeoIP_time_zone_by_country_and_region(
                                      gir->country_code, gir->region);
            std::cout << "GeoIP Region  = "
                      << ((gir->postal_code != NULL) ? gir->postal_code : "")
                      << ((gir->postal_code != NULL) ? " " : "")
                      << ((gir->city != NULL) ? gir->city : "Unknown")
                      << ", " << ((gir->region != NULL) ? gir->region : "Unknown")
                      << " ("
                      << fabs(gir->latitude)  << "°" << ((gir->latitude > 0)  ? "N" : "S") << ", "
                      << fabs(gir->longitude) << "°" << ((gir->longitude > 0) ? "E" : "W")
                      << ((timeZone != NULL) ? ", " : "")
                      << ((timeZone != NULL) ? timeZone : "")
                      << ")"
                      << std::endl;
            GeoIPRecord_delete(gir);
         }
         GeoIP_delete(geoIP);
      }
   }
#else
#warning This used version of GeoIP does not yet support IPv6!
#endif
#endif


   // ====== Reverse lookup =================================================
   if(noReverseLookup == false) {
      std::cout << "Performing reverse DNS lookup ..."; std::cout.flush();
      char hostname[NI_MAXHOST];
      int error = getnameinfo(&address.sa,
                              (address.sa.sa_family == AF_INET6) ?
                                 sizeof(sockaddr_in6) : sizeof(sockaddr_in),
                              (char*)&hostname, sizeof(hostname),
                              NULL, 0,
#ifdef NI_IDN
                              NI_NAMEREQD|NI_IDN
#else
#warning No IDN support for getnameinfo()!
                              NI_NAMEREQD
#endif
                              );
      std::cout << ((colourMode == true) ? "\r\x1b[K" : "\r");
      std::cout << "DNS Hostname  = "; std::cout.flush();
      if(error == 0) {
         std::cout << hostname << std::endl;
      }
      else {
         std::cout << "(" << gai_strerror(error) << ")" << std::endl;
      }
   }
}
