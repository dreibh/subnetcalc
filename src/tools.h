#ifndef TOOLS_H
#define TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <chrono>
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

inline unsigned long long getMicroTime()
{
   return static_cast<unsigned long long>(
      std::chrono::duration_cast<std::chrono::microseconds>(
         std::chrono::system_clock::now().time_since_epoch()).count());
}

union sockaddr_union {
   struct sockaddr     sa;
   struct sockaddr_in  in;
   struct sockaddr_in6 in6;
};

bool checkIPv6();
bool hasTranslationPrefix(const sockaddr_in6* address);
size_t getSocklen(const struct sockaddr* address);

std::string address2string(const struct sockaddr* address,
                           bool                   port      = false,
                           bool                   hideScope = false);

bool string2address(std::string_view      string,
                    union sockaddr_union* address,
                    bool                  readPort      = true,
                    int                   addressFamily = AF_UNSPEC);

void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  bool                   port      = true,
                  bool                   hideScope = false);

// Standard C++20 Formatter for sockaddr_union
template<>
struct std::formatter<sockaddr_union> : std::formatter<std::string_view> {
   auto format(const sockaddr_union& addr, std::format_context& ctx) const {
      return std::formatter<std::string_view>::format(address2string(&addr.sa, false, true), ctx);
   }
};

#endif
