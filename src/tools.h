#ifndef TOOLS_H
#define TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdint>
#include <cstring>
#include <format>
#include <iosfwd>
#include <string>
#include <string_view>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using in_addr_t = uint32_t;
#ifndef IN_LOOPBACKNET
#define IN_LOOPBACKNET 127
#endif
#endif


union sockaddr_union {
   struct sockaddr     sa;
   struct sockaddr_in  in;
   struct sockaddr_in6 in6;
};

sockaddr_union operator&(const sockaddr_union& address1,
                         const sockaddr_union& address2);
sockaddr_union operator|(const sockaddr_union& address1,
                         const sockaddr_union& address2);
sockaddr_union operator~(const sockaddr_union& address1);
sockaddr_union operator+(const sockaddr_union& address1, uint32_t n);
sockaddr_union operator-(const sockaddr_union& address1, uint32_t n);
bool operator==(const sockaddr_union& address1,
                const sockaddr_union& address2);


bool checkIPv6();
bool hasTranslationPrefix(const sockaddr_in6* address);
size_t getSocklen(const struct sockaddr* address);

std::string address2string(const sockaddr_union& address,
                           bool                  port      = false,
                           bool                  hideScope = true);

bool string2address(std::string_view      string,
                    union sockaddr_union* address,
                    bool                  readPort      = true,
                    int                   addressFamily = AF_UNSPEC);


template<>
struct std::formatter<sockaddr_union> : std::formatter<std::string_view> {
   auto format(const sockaddr_union& address,
               std::format_context&  context) const {
      return std::formatter<std::string_view>::format(
                address2string(address, false, true), context);
   }
};

#endif
