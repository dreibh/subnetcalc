/* $Id$
 *
 * IPv4/IPv6 Subnet Calculator
 * Copyright (C) 2002-2009 by Thomas Dreibholz
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
 * Contact: dreibh@iem.uni-due.de
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include "tools.h"


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


// ###### Print IPv4 address in binary digits ###############################
void printAddressBinary(std::ostream&         os,
                        const sockaddr_union& address,
                        const unsigned int    prefix,
                        const char*           indent = "")
{
   if(address.sa.sa_family == AF_INET) {
      os << indent;
      uint32_t a = ntohl(getIPv4Address(address));
      for(int i = 31;i >= 0;i--) {
         const uint32_t v = (uint32_t)1 << i;
         if(32 - i > (int)prefix) {   // Colourize output
            os << "\x1b[33m";
         }
         else {
            os << "\x1b[34m";
         }
         if(a >= v) {
            os << "1";
            a -= v;
         }
         else {
            os << "0";
         }
         os << "\x1b[0m";   // Turn off colour printing
         if( ((i % 8) == 0) && (i > 0) ) {
            os << " . ";
         }
      }
      os << std::endl;
   }
   else {
      int p = 128;
      for(int j = 0;j < 8;j++) {
         uint16_t a = ntohs(getIPv6Address(address).s6_addr16[j]);
         char str[16];
         snprintf((char*)&str, sizeof(str), "%04x", a);
         os << indent << str << " = ";
         for(int i = 15;i >= 0;i--) {
            const uint32_t v = (uint32_t)1 << i;
            if(p > (int)prefix) {   // Colourize output
               os << "\x1b[33m";
            }
            else {
               os << "\x1b[34m";
            }
            if(a >= v) {
               os << "1";
               a -= v;
            }
            else {
               os << "0";
            }
            os << "\x1b[0m";   // Turn off colour printing
            if( ((i % 8) == 0) && (i > 0) ) {
               os << " ";
            }
            p--;
         }
         os << std::endl;
      }
   }
}


// ###### Is given netmask valid? ###########################################
unsigned int getPrefixLength(const sockaddr_union& netmask)
{
   int  prefixLength;
   bool belongsToNetwork = true;

   if(netmask.sa.sa_family == AF_INET) {   puts("INETv4");
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
   printf("PL=%d\n",prefixLength);
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
   printAddress(&a.sa, false, os);
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


// ###### Print address properties ##########################################
void printAddressProperties(std::ostream&         os,
                            const sockaddr_union& address,
                            const sockaddr_union& netmask,
                            const unsigned int    prefix,
                            const sockaddr_union& network,
                            const sockaddr_union& broadcast)
{
   in_addr_t ipv4address;
   in6_addr  ipv6address;
   if(isIPv4(address)) {
      ipv4address = ntohl(getIPv4Address(address));
   }
   else {
      ipv6address = getIPv6Address(address);
   }

   // ====== Common properties ==============================================
   os << "Properties    =" << std::endl;
   if(address == network) {
      os << "   - " << address << " is a NETWORK address" << std::endl;
   }
   else if((isIPv4(address)) && (address == broadcast)) {
      os << "   - " << address << " is the BROADCAST address of "
         << network << "/" << prefix << std::endl;
   }
   else if(isMulticast(address)) {
      os << "   - " << address << " is a MULTICAST address" << std::endl;
   }
   else {
      os << "   - " << address << " is a HOST address in "
         << network << "/" << prefix << std::endl;
   }


   // ====== IPv4 properties ================================================
   if(isIPv4(address)) {
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
      }
      else if(IN_CLASSC(ipv4address)) {
         os << "   - Class C" << std::endl;
         if((a == 192) && (b == 168)) {
            os << "   - Private" << std::endl;
         }
      }
      else if(IN_CLASSD(ipv4address)) {
         os << "   - Class D (Multicast)" << std::endl;
      }
      else {
         os << "   - Invalid (not in class A, B, C or D)" << std::endl;
      }
   }


   // ====== IPv6 properties ================================================
   else {
      // ------ Special addresses -------------------------------------------
      if(IN6_IS_ADDR_LOOPBACK(&ipv6address)) {
         os << "   - Loopback address" << std::endl;
      }
      else if(IN6_IS_ADDR_UNSPECIFIED(&ipv6address)) {
         os << "   - Unspecified address" << std::endl;
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
         os << std::endl;

         // ------ Multicast flags ------------------------------------------
         const uint8_t flags = (((uint8_t*)&ipv6address)[1] & 0xf0) >> 4;
         if(flags == 0x1) {
            os << "      + Temporary" << std::endl;
         }
      }
   }
}



// ###### Main program ######################################################
int main(int argc, char** argv)
{
   int            prefix;
   unsigned int   hostBits;
   unsigned int   reservedHosts;
   double         maxHosts;
   sockaddr_union network;
   sockaddr_union address;
   sockaddr_union netmask;
   sockaddr_union broadcast;
   sockaddr_union wildcard;
   sockaddr_union host1;
   sockaddr_union host2;

   if(argc != 3) {
      printf("Usage: %s [Address] [Netmask]\n", argv[0]);
      exit(1);
   }

   if(string2address(argv[1], &address) == false) {
      printf("ERROR: Bad address %s!\n", argv[1]);
      exit(1);
   }
   if(string2address(argv[2], &netmask) == false) {
      printf("ERROR: Bad netmask %s!\n", argv[2]);
      exit(1);
   }
   prefix = getPrefixLength(netmask);
   if(prefix < 0) {
      std::cerr << "ERROR: Invalid netmask ";
      printAddress(&netmask.sa, false, std::cerr);
      std::cerr << "!" << std::endl;
      exit(1);
   }
   if(netmask.sa.sa_family != address.sa.sa_family) {
      std::cerr << "ERROR: Incompatible netmask ";
      printAddress(&netmask.sa, false, std::cerr);
      std::cerr << "!" << std::endl;
      exit(1);
   }


   network   = address & netmask;
   broadcast = network | (~netmask);
   wildcard  = ~netmask;
   if(isIPv4(address)) {
      reservedHosts = 2;
      hostBits      = 32 - prefix;
      host1         = network + 1;
      host2         = broadcast - 1;
   }
   else {
      reservedHosts = 1;
      hostBits      = 128 - prefix;
      host1         = network + 1;
      host2         = broadcast;   // There is no broadcast address for IPv6!
   }
   maxHosts = pow(2.0, (double)hostBits) - reservedHosts;

/*
   if(prefix < 31) {
      hosts     = (1 << (32 - prefix)) - 2;
      host1     = network + 1;
      host2     = broadcast - 1;
   }
   else if(prefix == 31) {
      hosts = 2;
      host1 = netmask;
      host2 = broadcast;
   }
   else {
      hosts = 1;
      host1 = broadcast;
      host2 = broadcast;
   }*/


   std::cout << "Address       = " << address        << std::endl;
   printAddressBinary(std::cout, address, prefix, "                ");
   std::cout << "Network       = " << network        << " / " << prefix << std::endl;
   std::cout << "Netmask       = " << netmask        << std::endl;
   if(isIPv4(address)) {
      std::cout << "Broadcast     = " << broadcast      << std::endl;
   }
   std::cout << "Wildcard Mask = " << wildcard       << std::endl;
   std::cout << "Hosts Bits    = " << hostBits       << std::endl;
   if(!isMulticast(address)) {
      char maxHostsString[128];
      snprintf((char*)&maxHostsString, sizeof(maxHostsString),
               "%1.0f  =  2^%u - %u", maxHosts, hostBits, reservedHosts);
      std::cout << "Max. Hosts    = " << maxHostsString << std::endl;
      std::cout << "Host Range    = { " << host1 << " - " << host2 << " }" << std::endl;
   }
   printAddressProperties(std::cout, address, netmask, prefix, network, broadcast);
}
