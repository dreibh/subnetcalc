/* $Id$
 *
 * Network Performance Meter
 * Copyright (C) 2009 by Thomas Dreibholz
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
#include <ext_socket.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>

#include <iostream>


/* ###### Get current timer ############################################## */
unsigned long long getMicroTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return(((unsigned long long)tv.tv_sec * (unsigned long long)1000000) +
         (unsigned long long)tv.tv_usec);
}


/* ###### Length-checking strcpy() ###################################### */
int safestrcpy(char* dest, const char* src, const size_t size)
{
   assert(size > 0);
   strncpy(dest, src, size);
   dest[size - 1] = 0x00;
   return(strlen(dest) < size);
}


/* ###### Length-checking strcat() ###################################### */
int safestrcat(char* dest, const char* src, const size_t size)
{
   const size_t l1 = strlen(dest);
   const size_t l2 = strlen(src);

   assert(size > 0);
   strncat(dest, src, size - l1 - 1);
   dest[size - 1] = 0x00;
   return(l1 + l2 < size);
}


/* ###### Find first occurrence of character in string ################### */
char* strindex(char* string, const char character)
{
   if(string != NULL) {
      while(*string != character) {
         if(*string == 0x00) {
            return(NULL);
         }
         string++;
      }
      return(string);
   }
   return(NULL);
}



/* ###### Find last occurrence of character in string #################### */
char* strrindex(char* string, const char character)
{
   const char* original = string;

   if(original != NULL) {
      string = (char*)&string[strlen(string)];
      while(*string != character) {
         if(string == original) {
            return(NULL);
         }
         string--;
      }
      return(string);
   }
   return(NULL);
}


/* ###### Check for support of IPv6 ###################################### */
bool checkIPv6()
{
   int sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
   if(sd >= 0) {
      close(sd);
      return(true);
   }
   return(false);
}


/* ###### Get socklen for given address ################################## */
size_t getSocklen(const struct sockaddr* address)
{
   switch(address->sa_family) {
      case AF_INET:
         return(sizeof(struct sockaddr_in));
       break;
      case AF_INET6:
         return(sizeof(struct sockaddr_in6));
       break;
      default:
         return(sizeof(struct sockaddr));
       break;
   }
}


/* ###### Compare addresses ############################################## */
int addresscmp(const struct sockaddr* a1, const struct sockaddr* a2, const bool port)
{
   uint16_t     p1, p2;
   uint32_t     x1[4];
   uint32_t     x2[4];
   int          result;

   if( ((a1->sa_family == AF_INET) || (a1->sa_family == AF_INET6)) &&
       ((a2->sa_family == AF_INET) || (a2->sa_family == AF_INET6)) ) {
      if(a1->sa_family == AF_INET6) {
         memcpy((void*)&x1, (void*)&((struct sockaddr_in6*)a1)->sin6_addr, 16);
      }
      else {
         x1[0] = 0;
         x1[1] = 0;
         x1[2] = htonl(0xffff);
         x1[3] = *((uint32_t*)&((struct sockaddr_in*)a1)->sin_addr);
      }

      if(a2->sa_family == AF_INET6) {
         memcpy((void*)&x2, (void*)&((struct sockaddr_in6*)a2)->sin6_addr, 16);
      }
      else {
         x2[0] = 0;
         x2[1] = 0;
         x2[2] = htonl(0xffff);
         x2[3] = *((uint32_t*)&((struct sockaddr_in*)a2)->sin_addr);
      }

      result = memcmp((void*)&x1,(void*)&x2,16);
      if(result != 0) {
         return(result);
      }

      if(port) {
         p1 = getPort((struct sockaddr*)a1);
         p2 = getPort((struct sockaddr*)a2);
         if(p1 < p2) {
            return(-1);
         }
         else if(p1 > p2) {
            return(1);
         }
      }
      return(0);
   }
   return(0);
}


/* ###### Convert address to string ###################################### */
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
         return(true);
       break;
      case AF_INET6:
         ipv6address = (const struct sockaddr_in6*)address;
         if( (!hideScope) &&
             (IN6_IS_ADDR_LINKLOCAL(&ipv6address->sin6_addr) ||
              IN6_IS_ADDR_MC_LINKLOCAL(&ipv6address->sin6_addr)) ) {
            ifname = if_indextoname(ipv6address->sin6_scope_id, (char*)&ifnamebuffer);
            if(ifname == NULL) {
               safestrcpy((char*)&ifnamebuffer, "(BAD!)", sizeof(ifnamebuffer));
               ifname = (const char*)&ifnamebuffer;
               return(false);
            }
            snprintf((char*)&scope, sizeof(scope), "%%%s", ifname);
         }
         else {
            scope[0] = 0x00;
         }
         if(inet_ntop(AF_INET6, &ipv6address->sin6_addr, str, sizeof(str)) != NULL) {
            if(port) {
               snprintf(buffer, length,
                        "[%s%s]:%d", str, scope, ntohs(ipv6address->sin6_port));
            }
            else {
               snprintf(buffer, length, "%s%s", str, scope);
            }
            return(true);
         }
       break;
      case AF_UNSPEC:
         safestrcpy(buffer, "(unspecified)", length);
         return(true);
       break;
   }
   return(false);
}


/* ###### Convert string to address ###################################### */
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
      return(false);
   }
   strcpy((char*)&host,string);
   strcpy((char*)&port, "0");

   /* ====== Handle RFC2732-compliant addresses ========================== */
   if(string[0] == '[') {
      p1 = strindex(host,']');
      if(p1 != NULL) {
         if((p1[1] == ':') || (p1[1] == '!')) {
            strcpy((char*)&port, &p1[2]);
         }
         memmove((char*)&host, (char*)&host[1], (long)p1 - (long)host - 1);
         host[(long)p1 - (long)host - 1] = 0x00;
      }
   }

   /* ====== Handle standard address:port ================================ */
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
            if(p1 == NULL) {
               p1 = strrindex(host,'!');
            }
            if(p1 != NULL) {
               p1[0] = 0x00;
               strcpy((char*)&port, &p1[1]);
            }
         }
      }
   }

   /* ====== Check port number =========================================== */
   portNumber = ~0;
   if((sscanf(port, "%d", &portNumber) != 1) ||
      (portNumber < 0) ||
      (portNumber > 65535)) {
      puts("q2");
      return(false);
   }


   /* ====== Create address structure ==================================== */

   /* ====== Get information for host ==================================== */
   res        = NULL;
   isNumeric  = true;
   isIPv6     = false;
   hostLength = strlen(host);

   memset((char*)&hints,0,sizeof(hints));
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_family   = AF_UNSPEC;

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
      hints.ai_flags = AI_NUMERICHOST;
   }

   if(getaddrinfo(host, NULL, &hints, &res) != 0) {
      return(false);
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
         return(false);
       break;
   }

   freeaddrinfo(res);
   return(true);
}


/* ###### Print address ################################################## */
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


/* ###### Get protocol name ############################################## */
const char* getProtocolName(const uint8_t protocol)
{
   const char* protocolName = "unknown?!";
   switch(protocol) {
      case IPPROTO_SCTP:
         protocolName = "SCTP";
         break;
      case IPPROTO_TCP:
         protocolName = "TCP";
         break;
      case IPPROTO_UDP:
         protocolName = "UDP";
         break;
#ifdef IPPROTO_DCCP
      case IPPROTO_DCCP:
         protocolName = "DCCP";
         break;
#endif
   }
   return(protocolName);
}


/* ###### Get port ####################################################### */
uint16_t getPort(const struct sockaddr* address)
{
   if(address != NULL) {
      switch(address->sa_family) {
         case AF_INET:
            return(ntohs(((struct sockaddr_in*)address)->sin_port));
          break;
         case AF_INET6:
            return(ntohs(((struct sockaddr_in6*)address)->sin6_port));
          break;
         default:
            return(0);
          break;
      }
   }
   return(0);
}


/* ###### Set port ####################################################### */
bool setPort(struct sockaddr* address, uint16_t port)
{
   if(address != NULL) {
      switch(address->sa_family) {
         case AF_INET:
            ((struct sockaddr_in*)address)->sin_port = htons(port);
            return(true);
          break;
         case AF_INET6:
            ((struct sockaddr_in6*)address)->sin6_port = htons(port);
            return(true);
          break;
      }
   }
   return(false);
}


/* ###### Create server socket of appropriate family and bind it ######### */
int createAndBindSocket(const int      type,
                        const int      protocol,
                        const uint16_t localPort,
                        const bool     blocking)
{
   char           localAddressString[32];
   sockaddr_union localAddress;

   if(checkIPv6()) {
      snprintf((char*)&localAddressString, sizeof(localAddressString), "[::]:%u", localPort);
   }
   else {
      snprintf((char*)&localAddressString, sizeof(localAddressString), "0.0.0.0:%u", localPort);
   }
   assert(string2address(localAddressString, &localAddress) == true);

   int sd = ext_socket(localAddress.sa.sa_family, type, protocol);
   if(sd < 0) {
      return(-2);
   }
   if(ext_bind(sd, &localAddress.sa, getSocklen(&localAddress.sa)) != 0) {
      ext_close(sd);
      return(-3);
   }
   ext_listen(sd, 10);
   if(!blocking) {
      setNonBlocking(sd);
   }
   return(sd);
}


/* ###### Set blocking mode ############################################## */
bool setNonBlocking(int fd)
{
   int flags = ext_fcntl(fd,F_GETFL,0);
   if(flags != -1) {
      flags |= O_NONBLOCK;
      if(ext_fcntl(fd,F_SETFL, flags) == 0) {
         return(true);
      }
   }
   return(false);
}



/* ###### Convert byte order of 64 bit value ############################# */
static uint64_t byteswap64(const uint64_t value)
{
#if BYTE_ORDER == LITTLE_ENDIAN
   const uint32_t a = (uint32_t)(value >> 32);
   const uint32_t b = (uint32_t)(value & 0xffffffff);
   return( (int64_t)((a << 24) | ((a & 0x0000ff00) << 8) |
           ((a & 0x00ff0000) >> 8) | (a >> 24)) |
           ((int64_t)((b << 24) | ((b & 0x0000ff00) << 8) |
           ((b & 0x00ff0000) >> 8) | (b >> 24)) << 32) );
#elif BYTE_ORDER == BIG_ENDIAN
   return(value);
#else
#error Byte order undefined!
#endif
}


/* ###### Convert byte order of 64 bit value ############################# */
uint64_t hton64(const uint64_t value)
{
   return(byteswap64(value));
}


/* ###### Convert byte order of 64 bit value ############################# */
uint64_t ntoh64(const uint64_t value)
{
   return(byteswap64(value));
}



#ifndef HAVE_IEEE_FP
#warning Is this code really working correctly?

#define DBL_EXP_BITS  11
#define DBL_EXP_BIAS  1023
#define DBL_EXP_MAX   ((1L << DBL_EXP_BITS) - 1 - DBL_EXP_BIAS)
#define DBL_EXP_MIN   (1 - DBL_EXP_BIAS)
#define DBL_FRC1_BITS 20
#define DBL_FRC2_BITS 32
#define DBL_FRC_BITS  (DBL_FRC1_BITS + DBL_FRC2_BITS)


struct IeeeDouble {
#if __BYTE_ORDER == __BIG_ENDIAN
   unsigned int s : 1;
   unsigned int e : 11;
   unsigned int f1 : 20;
   unsigned int f2 : 32;
#elif  __BYTE_ORDER == __LITTLE_ENDIAN
   unsigned int f2 : 32;
   unsigned int f1 : 20;
   unsigned int e : 11;
   unsigned int s : 1;
#else
#error Unknown byteorder settings!
#endif
};


/* ###### Convert double to machine-independent form ##################### */
network_double_t doubleToNetwork(const double d)
{
   struct IeeeDouble ieee;

   if(isnan(d)) {
      // NaN
      ieee.s = 0;
      ieee.e = DBL_EXP_MAX + DBL_EXP_BIAS;
      ieee.f1 = 1;
      ieee.f2 = 1;
   } else if(isinf(d)) {
      // +/- infinity
      ieee.s = (d < 0);
      ieee.e = DBL_EXP_MAX + DBL_EXP_BIAS;
      ieee.f1 = 0;
      ieee.f2 = 0;
   } else if(d == 0.0) {
      // zero
      ieee.s = 0;
      ieee.e = 0;
      ieee.f1 = 0;
      ieee.f2 = 0;
   } else {
      // finite number
      int exp;
      double frac = frexp (fabs (d), &exp);

      while (frac < 1.0 && exp >= DBL_EXP_MIN) {
         frac = ldexp (frac, 1);
         --exp;
      }
      if (exp < DBL_EXP_MIN) {
          // denormalized number (or zero)
          frac = ldexp (frac, exp - DBL_EXP_MIN);
          exp = 0;
      } else {
         // normalized number
         assert((1.0 <= frac) && (frac < 2.0));
         assert((DBL_EXP_MIN <= exp) && (exp <= DBL_EXP_MAX));

         exp += DBL_EXP_BIAS;
         frac -= 1.0;
      }
      ieee.s = (d < 0);
      ieee.e = exp;
      ieee.f1 = (unsigned long)ldexp (frac, DBL_FRC1_BITS);
      ieee.f2 = (unsigned long)ldexp (frac, DBL_FRC_BITS);
   }
   return(hton64(*((network_double_t*)&ieee)));
}


/* ###### Convert machine-independent form to double ##################### */
double networkToDouble(network_double_t value)
{
   network_double_t   hValue;
   struct IeeeDouble* ieee;
   double             d;

   hValue = ntoh64(value);
   ieee = (struct IeeeDouble*)&hValue;
   if(ieee->e == 0) {
      if((ieee->f1 == 0) && (ieee->f2 == 0)) {
         // zero
         d = 0.0;
      } else {
         // denormalized number
         d  = ldexp((double)ieee->f1, -DBL_FRC1_BITS + DBL_EXP_MIN);
         d += ldexp((double)ieee->f2, -DBL_FRC_BITS  + DBL_EXP_MIN);
         if (ieee->s) {
            d = -d;
         }
      }
   } else if(ieee->e == DBL_EXP_MAX + DBL_EXP_BIAS) {
      if((ieee->f1 == 0) && (ieee->f2 == 0)) {
         // +/- infinity
         d = (ieee->s) ? -INFINITY : INFINITY;
      } else {
         // not a number
         d = NAN;
      }
   } else {
      // normalized number
      d = ldexp(ldexp((double)ieee->f1, -DBL_FRC1_BITS) +
                ldexp((double)ieee->f2, -DBL_FRC_BITS) + 1.0,
                      ieee->e - DBL_EXP_BIAS);
      if(ieee->s) {
         d = -d;
      }
   }
   return(d);
}

#else

union DoubleIntUnion
{
   double             Double;
   unsigned long long Integer;
};

/* ###### Convert double to machine-independent form ##################### */
network_double_t doubleToNetwork(const double d)
{
   union DoubleIntUnion valueUnion;
   valueUnion.Double = d;
   return(hton64(valueUnion.Integer));
}

/* ###### Convert machine-independent form to double ##################### */
double networkToDouble(network_double_t value)
{
   union DoubleIntUnion valueUnion;
   valueUnion.Integer = ntoh64(value);
   return(valueUnion.Double);
}

#endif



/* Kill after timeout: Send kill signal, if Ctrl-C is pressed again
   after more than KILL_TIMEOUT microseconds */
#define KILL_AFTER_TIMEOUT
#define KILL_TIMEOUT 2000000


/* ###### Global variables ############################################### */
static bool   DetectedBreak = false;
static bool   PrintedBreak  = false;
static bool   Quiet         = false;
static pid_t  MainThreadPID = 0;
#ifdef KILL_AFTER_TIMEOUT
static bool               PrintedKill   = false;
static unsigned long long LastDetection = (unsigned long long)-1;
#endif


/* ###### Handler for SIGINT ############################################# */
void breakDetector(int signum)
{
   DetectedBreak = true;

#ifdef KILL_AFTER_TIMEOUT
   if(!PrintedKill) {
      const unsigned long long now = getMicroTime();
      if(LastDetection == (unsigned long long)-1) {
         LastDetection = now;
      }
      else if(now - LastDetection >= 2000000) {
         PrintedKill = true;
         fprintf(stderr,"\x1b[0m\n*** Kill ***\n\n");
         kill(MainThreadPID,SIGKILL);
      }
   }
#endif
}


/* ###### Install break detector ######################################### */
void installBreakDetector()
{
   DetectedBreak = false;
   PrintedBreak  = false;
   Quiet         = false;
   MainThreadPID = getpid();
#ifdef KILL_AFTER_TIMEOUT
   PrintedKill   = false;
   LastDetection = (unsigned long long)-1;
#endif
   signal(SIGINT,&breakDetector);
}


/* ###### Unnstall break detector ######################################## */
void uninstallBreakDetector()
{
   signal(SIGINT,SIG_DFL);
#ifdef KILL_AFTER_TIMEOUT
   PrintedKill   = false;
   LastDetection = (unsigned long long)-1;
#endif
   /* No reset here!
      DetectedBreak = false;
      PrintedBreak  = false; */
   Quiet         = false;
}


/* ###### Check, if break has been detected ############################## */
bool breakDetected()
{
   if((DetectedBreak) && (!PrintedBreak)) {
      if(!Quiet) {
         fprintf(stderr,"\x1b[0m\n*** Break ***    Signal #%d\n\n",SIGINT);
      }
      PrintedBreak = getMicroTime();
   }
   return(DetectedBreak);
}


/* ###### Send break to main thread ###################################### */
void sendBreak(const bool quiet)
{
   Quiet = quiet;
   kill(MainThreadPID,SIGINT);
}



/* ###### Get random value using specified random number generator #### */
double getRandomValue(const double x, const uint8_t rng)
{
   double value;
   switch(rng) {
      case RANDOM_CONSTANT:
         value = x;
       break;
      case RANDOM_EXPONENTIAL:
         value = randomExpDouble(x);
       break;
      case RANDOM_UNIFORM:
         value = x * randomDouble();
       break;
      default:
         assert(false);
       break;
   }
   return(value);
}



/*
   It is tried to use /dev/urandom as random source first, since
   it provides high-quality random numbers. If /dev/urandom is not
   available, use the clib's random() function with a seed given
   by the current microseconds time. However, the random number
   quality is much lower since the seed may be easily predictable.
*/

#define RS_TRY_DEVICE 0
#define RS_DEVICE     1
#define RS_CLIB       2

static int   RandomSource = RS_TRY_DEVICE;
static FILE* RandomDevice = NULL;


/* ###### Get 8-bit random value ######################################### */
uint8_t random8()
{
   return((uint8_t)random32());
}


/* ###### Get 16-bit random value ######################################## */
uint16_t random16()
{
   return((uint16_t)random32());
}


/* ###### Get 64-bit random value ######################################## */
uint64_t random64()
{
   return( (((uint64_t)random32()) << 32) | (uint64_t)random32() );
}


/* ###### Get 32-bit random value ######################################## */
uint32_t random32()
{
#ifdef NDEBUG
#warning Using OMNeT++ random generator instead of time-seeded one!
   const double value = uniform(0.0, (double)0xffffffff);
   return((uint32_t)rint(value));
#else
   uint32_t number;

   switch(RandomSource) {
      case RS_DEVICE:
         if(fread(&number, sizeof(number), 1, RandomDevice) == 1) {
            return(number);
         }
         RandomSource = RS_CLIB;
      case RS_CLIB:
         return(random());
       break;
      case RS_TRY_DEVICE:
         RandomDevice = fopen("/dev/urandom", "r");
         if(RandomDevice != NULL) {
            if(fread(&number, sizeof(number), 1, RandomDevice) == 1) {
               srandom(number);
               RandomSource = RS_DEVICE;
               return(number);
            }
            fclose(RandomDevice);
         }
         RandomSource = RS_CLIB;
         srandom((unsigned int)(getMicroTime() & (uint64_t)0xffffffff));
      break;
   }
   return(random());
#endif
}


/* ###### Get double random value ######################################## */
double randomDouble()
{
   return( (double)random32() / (double)4294967296.0 );
}


/* ###### Get exponential-distributed double random value ################ */
double randomExpDouble(const double p)
{
   return( -p * log(randomDouble()) );
}
