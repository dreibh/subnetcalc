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

#ifndef TOOLS_H
#define TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ext_socket.h>

#include <iostream>



unsigned long long getMicroTime();


int safestrcpy(char* dest, const char* src, const size_t size);
int safestrcat(char* dest, const char* src, const size_t size);
char* strindex(char* string, const char character);
char* strrindex(char* string, const char character);


union sockaddr_union {
   struct sockaddr          sa;
   struct sockaddr_in       in;
   struct sockaddr_in6      in6;
};

bool checkIPv6();
size_t getSocklen(const struct sockaddr* address);
int addresscmp(const struct sockaddr* a1, const struct sockaddr* a2, const bool port);
bool address2string(const struct sockaddr* address,
                    char*                  buffer,
                    const size_t           length,
                    const bool             port,
                    const bool             hideScope = false);
bool string2address(const char*           string,
                    union sockaddr_union* address,
                    const bool            readPort = true);

void printAddress(std::ostream&          os,
                  const struct sockaddr* address,
                  const bool             port      = true,
                  const bool             hideScope = false);

const char* getProtocolName(const uint8_t protocol);
uint16_t getPort(const struct sockaddr* address);
bool setPort(struct sockaddr* address, uint16_t port);
bool setNonBlocking(int fd);
int createAndBindSocket(const int      type,
                        const int      protocol,
                        const uint16_t localPort,
                        const bool     blocking = false);
uint64_t hton64(const uint64_t value);
uint64_t ntoh64(const uint64_t value);


typedef unsigned long long network_double_t;

network_double_t doubleToNetwork(double value);
double networkToDouble(const network_double_t value);


void installBreakDetector();
void uninstallBreakDetector();
bool breakDetected();
void sendBreak(const bool quiet);


#define RANDOM_CONSTANT    0
#define RANDOM_UNIFORM     1
#define RANDOM_EXPONENTIAL 2

double getRandomValue(const double x, const uint8_t rng);
uint8_t random8();
uint16_t random16();
uint64_t random64();
uint32_t random32();
double randomDouble();
double randomExpDouble(const double p);


#endif
