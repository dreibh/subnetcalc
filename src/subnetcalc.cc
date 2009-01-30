/* $Id: subnetcalc.cc 2076 2008-12-28 15:52:34Z dreibh $
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// ###### Convert string to IPv4 address ####################################
bool getAddress(const char* str, uint32_t& address)
{
   in_addr iaddr;

   if(inet_aton(str, &iaddr) == 0) {
      return(false);
   }
   address = ntohl(iaddr.s_addr);
   return(true);
}


// ###### Get IPv4 address class ############################################
const char* getAddressClass(uint32_t address) {
   if((address & 0x80000000) == 0) {
      return("A");
   }
   else if((address & 0xc0000000) == 0x80000000) {
      return("B");
   }
   else if((address & 0xe0000000) == 0xc0000000) {
      return("C");
   }
   return("Invalid!");
}


// ###### Print IPv4 address in binary digits ###############################
void putAddressBinary(uint32_t address, const size_t prefix)
{
   for(int i = 31;i >= 0;i--) {
      const uint32_t v = (uint32_t)1 << i;
      if(32 - i > (int)prefix) {   // Colourize output
         printf("\x1b[33m");
      }
      else {
         printf("\x1b[34m");
      }
      if(address >= v) {
         printf("1");
         address -= v;
      }
      else {
         printf("0");
      }
      printf("\x1b[0m");   // Turn off colour printing
      if( ((i % 8) == 0) && (i > 0) ) printf(" . ");
   }
}


// ###### Print IPv4 address ################################################
void putAddress(const uint32_t address)
{
   char    str[256];
   in_addr iaddr;

   iaddr.s_addr = htonl(address);
   inet_ntop(AF_INET, (char*)&iaddr, (char*)&str, sizeof(str));
   fputs(str, stdout);
}


// ###### Calculate prefix length ###########################################
size_t prefixLength(const uint32_t netmask)
{
   for(int i = 31;i >= 0;i--) {
      if(!(netmask & (1 << (uint32_t)i))) {
         return(31 - i);
      }
   }
   return(32);
}


// ###### Is given netmask valid? ###########################################
bool verifyNetmask(const uint32_t netmask)
{
   bool host = true;

   for(int i = 0;i <= 31;i++) {
      if(netmask & (1 << (uint32_t)i)) {
         host = false;
      }
      else {
         if(host == false) {
            return(false);
         }
      }
   }
   return(true);
}



// ###### Main program ######################################################
int main(int argc, char** argv)
{
   size_t  prefix;
   size_t  hosts;
   uint32_t network;
   uint32_t address;
   uint32_t netmask;
   uint32_t broadcast;
   uint32_t wildcard;
   uint32_t host1;
   uint32_t host2;

   if(argc != 3) {
      printf("Usage: %s [Address] [Netmask]\n", argv[0]);
      exit(1);
   }

   if(!getAddress(argv[1], address)) {
      printf("ERROR: Bad address %s!\n", argv[1]);
      exit(1);
   }
   if(!getAddress(argv[2], netmask)) {
      printf("ERROR: Bad netmask %s!\n", argv[2]);
      exit(1);
   }
   if(!verifyNetmask(netmask)) {
      printf("ERROR: Invalid netmask "); putAddress(netmask); puts("!");
      exit(1);
   }

   prefix    = prefixLength(netmask);
   network   = address & netmask;
   broadcast = network | (~netmask);
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
   }
   wildcard  = ~netmask;

   printf("Address       = "); putAddress(address); puts("");
   printf("                "); putAddressBinary(address, prefix); puts("");
   printf("Address Class = %s\n", getAddressClass(network));
   printf("Network       = "); putAddress(network); printf(" / %u\n", (unsigned int)prefix);
   printf("                "); putAddressBinary(network, prefix); puts("");
   printf("Netmask       = "); putAddress(netmask); puts("");
   printf("                "); putAddressBinary(netmask, prefix); puts("");
   printf("Broadcast     = "); putAddress(broadcast); puts("");
   printf("                "); putAddressBinary(broadcast, prefix); puts("");
   printf("Wildcard Mask = "); putAddress(wildcard); puts("");
   printf("Host Bits     = %u\n", (unsigned int)(32 - prefix));
   printf("Max. Hosts    = %u\n", (unsigned int)hosts);
   printf("Host Range    = { ");
   putAddress(host1); printf(" - "); putAddress(host2);
   puts(" }");
}
