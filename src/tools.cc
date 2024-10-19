/*
 * IPv4/IPv6 Subnet Calculator
 * Copyright (C) 2009-2024 by Thomas Dreibholz
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

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <math.h>

#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>

#include <iostream>


// ###### Get current time ##################################################
unsigned long long getMicroTime()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (((unsigned long long)tv.tv_sec * (unsigned long long)1000000) +
          (unsigned long long)tv.tv_usec);
}


// ###### Length-checking strcpy() ##########################################
int safestrcpy(char* dest, const char* src, const size_t size)
{
   assert(size > 0);
   strncpy(dest, src, size);
   dest[size - 1] = 0x00;
   return strlen(dest) < size;
}


// ###### Length-checking strcat() ##########################################
int safestrcat(char* dest, const char* src, const size_t size)
{
   const size_t l1 = strlen(dest);
   const size_t l2 = strlen(src);

   assert(size > 0);
   strncat(dest, src, size - l1 - 1);
   dest[size - 1] = 0x00;
   return (l1 + l2 < size);
}


// ###### Find first occurrence of character in string #######################
char* strindex(char* string, const char character)
{
   if(string != nullptr) {
      while(*string != character) {
         if(*string == 0x00) {
            return nullptr;
         }
         string++;
      }
      return string;
   }
   return nullptr;
}



// ###### Find last occurrence of character in string #######################
char* strrindex(char* string, const char character)
{
   const char* original = string;

   if(original != nullptr) {
      string = (char*)&string[strlen(string)];
      while(*string != character) {
         if(string == original) {
            return nullptr;
         }
         string--;
      }
      return string;
   }
   return nullptr;
}


// ###### Check for support of IPv6 #########################################
bool checkIPv6()
{
   int sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
   if(sd >= 0) {
      close(sd);
      return true;
   }
   return false;
}


// ###### Does the given address have a translation prefix? #################
bool hasTranslationPrefix(const sockaddr_in6* address)
{
   return (ntohs(address->sin6_addr.s6_addr16[0]) == 0x64   &&
           ntohs(address->sin6_addr.s6_addr16[1]) == 0xff9b &&
           ntohs(address->sin6_addr.s6_addr16[2]) <= 1);
}


// ###### Get socklen for given address #####################################
size_t getSocklen(const struct sockaddr* address)
{
   switch(address->sa_family) {
      case AF_INET:
         return sizeof(struct sockaddr_in);
       break;
      case AF_INET6:
         return sizeof(struct sockaddr_in6);
       break;
      default:
         return sizeof(struct sockaddr);
       break;
   }
}


// ###### Format an IPv6-embedded IPv6 address ##############################
inline bool formatEmbeddedAddress(const struct sockaddr_in6* ipv6address,
                                  char*                      str,
                                  const size_t               maxlen)
{
   struct in6_addr prefix = ipv6address->sin6_addr;

   // Set the suffix to a predictable value.
   prefix.s6_addr32[3] = 0xffffffff;
   if(inet_ntop(AF_INET6, &prefix, str, maxlen) == nullptr) {
      return false;
   }

   // Overwrite the predictable suffix with the IPv4 address.
   const size_t   pl    = strnlen(str, maxlen) - 9;
   const uint32_t upper = ntohs(ipv6address->sin6_addr.s6_addr16[6]);
   const uint32_t lower = ntohs(ipv6address->sin6_addr.s6_addr16[7]);

   const struct in_addr suffix = { .s_addr = htonl((upper << 16) | lower) };
   return inet_ntop(AF_INET, &suffix, str + pl, maxlen - pl) != nullptr;
}


// ###### Format an IPv6 address ############################################
static inline bool formatIPv6Address(const struct sockaddr_in6* ipv6address,
                                     char*                      str,
                                     const size_t               maxlen)
{
   if(hasTranslationPrefix(ipv6address)) {
      return formatEmbeddedAddress(ipv6address, str, maxlen);
   }

   return (inet_ntop(AF_INET6, &ipv6address->sin6_addr, str, maxlen) != nullptr);
}


// ###### Convert address to string #########################################
bool address2string(const struct sockaddr* address,
                    char*                  buffer,
                    const size_t           length,
                    const bool             port,
                    const bool             hideScope)
{
   const struct sockaddr_in*  ipv4address;
   const struct sockaddr_in6* ipv6address;
   char                       str[128];
   char                       scope[IFNAMSIZ + 16];
   char                       ifnamebuffer[IFNAMSIZ];
   const char*                ifname;

   switch(address->sa_family) {
      case AF_INET:
         ipv4address = (const struct sockaddr_in*)address;
         if(port) {
            snprintf(buffer, length,
                     "%s:%d", inet_ntoa(ipv4address->sin_addr), ntohs(ipv4address->sin_port));
         }
         else {
            snprintf(buffer, length, "%s", inet_ntoa(ipv4address->sin_addr));
         }
         return true;
       break;
      case AF_INET6:
         ipv6address = (const struct sockaddr_in6*)address;
         if( (!hideScope) &&
             (IN6_IS_ADDR_LINKLOCAL(&ipv6address->sin6_addr) ||
              IN6_IS_ADDR_MC_LINKLOCAL(&ipv6address->sin6_addr)) ) {
            ifname = if_indextoname(ipv6address->sin6_scope_id, (char*)&ifnamebuffer);
            if(ifname == nullptr) {
               safestrcpy((char*)&ifnamebuffer, "(BAD!)", sizeof(ifnamebuffer));
               return false;
            }
            snprintf((char*)&scope, sizeof(scope), "%%%s", ifname);
         }
         else {
            scope[0] = 0x00;
         }
         if(formatIPv6Address(ipv6address, str, sizeof(str))) {
            if(port) {
               snprintf(buffer, length,
                        "[%s%s]:%d", str, scope, ntohs(ipv6address->sin6_port));
            }
            else {
               snprintf(buffer, length, "%s%s", str, scope);
            }
            return true;
         }
       break;
      case AF_UNSPEC:
         safestrcpy(buffer, "(unspecified)", length);
         return true;
       break;
   }
   return false;
}


// ###### Convert string to address #########################################
bool string2address(const char*           string,
                    union sockaddr_union* address,
                    const bool            readPort)
{
   char                 host[128];
   char                 port[128];
   struct sockaddr_in*  ipv4address = (struct sockaddr_in*)address;
   struct sockaddr_in6* ipv6address = (struct sockaddr_in6*)address;
   int                  portNumber  = 0;
   char*                p1;

   struct addrinfo  hints;
   struct addrinfo* res;
   bool isNumeric;
   bool isIPv6;
   size_t hostLength;
   size_t i;

   if(strlen(string) > sizeof(host)) {
      return false;
   }
   strcpy((char*)&host,string);
   strcpy((char*)&port, "0");

   // ====== Handle RFC2732-compliant addresses =============================
   if(string[0] == '[') {
      p1 = strindex(host,']');
      if(p1 != nullptr) {
         if((p1[1] == ':') || (p1[1] == '!')) {
            strcpy((char*)&port, &p1[2]);
         }
         memmove((char*)&host, (char*)&host[1], (long)p1 - (long)host - 1);
         host[(long)p1 - (long)host - 1] = 0x00;
      }
   }

   // ====== Handle standard address:port ===================================
   else {
      if(readPort) {
         unsigned int colons = 0;
         for(size_t i = 0;i < strlen(host);i++) {
            if(host[i] == ':') {
               colons++;
            }
         }
         if(colons == 1) {
            p1 = strrindex(host,':');
            if(p1 == nullptr) {
               p1 = strrindex(host,'!');
            }
            if(p1 != nullptr) {
               p1[0] = 0x00;
               strcpy((char*)&port, &p1[1]);
            }
         }
      }
   }

   // ====== Check port number ==============================================
   portNumber = ~0;
   if((sscanf(port, "%d", &portNumber) != 1) ||
      (portNumber < 0) ||
      (portNumber > 65535)) {
      return false;
   }


   // ====== Create address structure =======================================

   // ====== Get information for host =======================================
   res        = nullptr;
   isNumeric  = true;
   isIPv6     = false;
   hostLength = strlen(host);

   memset((char*)&hints, 0, sizeof(hints));
   hints.ai_socktype = SOCK_DGRAM;
#ifdef AI_IDN
   hints.ai_flags    = AI_IDN;
#else
#warning No IDN support for getaddrinfo()!
   hints.ai_flags    = 0;
#endif

   for(i = 0;i < hostLength;i++) {
      if(host[i] == ':') {
         isIPv6 = true;
         break;
      }
   }
   if(!isIPv6) {
      for(i = 0;i < hostLength;i++) {
         if(!(isdigit(host[i]) || (host[i] == '.'))) {
            isNumeric = false;
            break;
         }
       }
   }
   if(isNumeric) {
      hints.ai_flags |= AI_NUMERICHOST;
   }

   // First try IPv6 ...
   hints.ai_family = AF_INET6;
   if(getaddrinfo(host, nullptr, &hints, &res) != 0) {
      // ... then (if there is no AAAA record), try also IPv4
      hints.ai_family = AF_UNSPEC;
      if(getaddrinfo(host, nullptr, &hints, &res) != 0) {
         return false;
      }
   }

   memset((char*)address,0,sizeof(union sockaddr_union));
   memcpy((char*)address,res->ai_addr,res->ai_addrlen);

   switch(ipv4address->sin_family) {
      case AF_INET:
         ipv4address->sin_port = htons(portNumber);
#ifdef HAVE_SIN_LEN
         ipv4address->sin_len  = sizeof(struct sockaddr_in);
#endif
       break;
      case AF_INET6:
         ipv6address->sin6_port = htons(portNumber);
#ifdef HAVE_SIN6_LEN
         ipv6address->sin6_len  = sizeof(struct sockaddr_in6);
#endif
       break;
      default:
         return false;
       break;
   }

   freeaddrinfo(res);
   return true;
}


// ###### Print address #####################################################
void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  const bool             port,
                  const bool             hideScope)
{
   static char str[128];

   if(address2string(address, (char*)&str, sizeof(str), port, hideScope)) {
      os << str;
   }
   else {
      os << "(invalid!)";
   }
}


// ###### Create formatted string (printf-like) #############################
std::string format(const char* fmt, ...)
{
   char buffer[16384];
   va_list va;
   va_start(va, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, va);
   va_end(va);
   return(std::string(buffer));
}
