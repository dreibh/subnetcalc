/* $Id$
 *
 * IPv4 Subnet Calculator
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

// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>


bool isIPv4(const sockaddr_union& address)
{
printf("%08x %08x %08x %08x\n",
        address.in6.sin6_addr.s6_addr32[0],address.in6.sin6_addr.s6_addr32[1],address.in6.sin6_addr.s6_addr32[2],address.in6.sin6_addr.s6_addr32[3]);

   return( IN6_IS_ADDR_V4MAPPED(&address.in6.sin6_addr) ||
           IN6_IS_ADDR_V4COMPAT(&address.in6.sin6_addr) );
}

// // ###### Convert string to IPv4 address ####################################
// bool getAddress(const char* str, uint32_t& address)
// {
//    in_addr iaddr;
//
//    if(inet_aton(str, &iaddr) == 0) {
//       return(false);
//    }
//    address = ntohl(iaddr.s_addr);
//    return(true);
// }
//
//
// // ###### Get IPv4 address class ############################################
// const char* getAddressClass(uint32_t address) {
//    if((address & 0x80000000) == 0) {
//       return("A");
//    }
//    else if((address & 0xc0000000) == 0x80000000) {
//       return("B");
//    }
//    else if((address & 0xe0000000) == 0xc0000000) {
//       return("C");
//    }
//    return("Invalid!");
// }
//
//
// // ###### Print IPv4 address in binary digits ###############################
// void putAddressBinary(uint32_t address, const size_t prefix)
// {
//    for(int i = 31;i >= 0;i--) {
//       const uint32_t v = (uint32_t)1 << i;
//       if(32 - i > (int)prefix) {   // Colourize output
//          printf("\x1b[33m");
//       }
//       else {
//          printf("\x1b[34m");
//       }
//       if(address >= v) {
//          printf("1");
//          address -= v;
//       }
//       else {
//          printf("0");
//       }
//       printf("\x1b[0m");   // Turn off colour printing
//       if( ((i % 8) == 0) && (i > 0) ) printf(" . ");
//    }
// }
//
//
// // ###### Print IPv4 address ################################################
// void putAddress(const uint32_t address)
// {
//    char    str[256];
//    in_addr iaddr;
//
//    iaddr.s_addr = htonl(address);
//    inet_ntop(AF_INET, (char*)&iaddr, (char*)&str, sizeof(str));
//    fputs(str, stdout);
// }
//


// ###### Is given netmask valid? ###########################################
unsigned int getPrefixLength(const sockaddr_union& netmask)
{
   int  prefixLength;
   bool host = true;

   if(netmask.sa.sa_family == AF_INET) {
      prefixLength     = 32;
      const uint32_t a = ntohl(netmask.in.sin_addr.s_addr);
      for(int i = 0;i <= 31;i++) {
         if(a & (1 << (uint32_t)i)) {
            host = false;
         }
         else {
            if(host == false) {
               return(-1);
            }
         }
         prefixLength--;
      }
   }
   else {
      prefixLength = 128;
      for(int j = 3;j >= 0;j--) {
         const uint32_t a = ntohl(netmask.in6.sin6_addr.s6_addr32[j]);
         for(int i = 0;i <= 31;i++) {
            if(a & (1 << (uint32_t)i)) {
               host = false;
            }
            else {
               if(host == false) {
                  return(-1);
               }
            }
            prefixLength--;
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
   printAddress(&a.sa, false, os);
   return(os);
}


// ###### "+" operator for addresses ########################################
sockaddr_union operator+(const sockaddr_union& a1, uint32_t n)
{
   sockaddr_union a = a1;
   for(int j = 3;j >= 0;j--) {
      const uint64_t sum = (uint64_t)a.in6.sin6_addr.s6_addr32[j] + (uint64_t)n;
      a.in6.sin6_addr.s6_addr32[j] = (uint32_t)(sum & 0xffffffffULL);
      n = (uint32_t)(sum >> 32);
   }
   return(a);
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

   network   = address & netmask;
   broadcast = network | (~netmask);
   wildcard  = ~netmask;

   if(isIPv4(address)) {
      puts("IS-IPv4");
      hostBits      = 128 - prefix;
      reservedHosts = 2;
      maxHosts      = pow(2.0, (double)hostBits);
      host1         = network + 1;
   }
   else {
      puts("IS-IPv6");
      hostBits      = 128 - prefix;
      reservedHosts = 1;
      maxHosts      = pow(2.0, (double)hostBits);
   }

/*   if(isIPv4(address)) {

      hosts -= 2;
      host1  = network + 1;
      host2  = broadcast - 1;
   }
   else {
      hosts -= 1;
      host1  = network + 1;
      host2  = broadcast;
   }*/
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


   char maxHostsString[128];
   snprintf((char*)&maxHostsString, sizeof(maxHostsString),
            "%1.0f  =  2^%u - %u", maxHosts, hostBits, reservedHosts);

   std::cout << "Address       = " << address        << std::endl;
   std::cout << "Network       = " << network        << " / " << prefix << std::endl;
   std::cout << "Netmask       = " << netmask        << std::endl;
   std::cout << "Broadcast     = " << broadcast      << std::endl;
   std::cout << "Wildcard Mask = " << wildcard       << std::endl;
   std::cout << "Hosts Bits    = " << hostBits       << std::endl;
   std::cout << "Max. Hosts    = " << maxHostsString << std::endl;
   std::cout << "HostRange     = { " << host1 << " - " << host2 << " }" << std::endl;
}
