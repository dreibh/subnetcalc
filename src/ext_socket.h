/*
 *  $Id$
 *
 * SocketAPI implementation for the sctplib.
 * Copyright (C) 1999-2009 by Thomas Dreibholz
 *
 * Realized in co-operation between
 * - Siemens AG
 * - University of Essen, Institute of Computer Networking Technology
 * - University of Applied Sciences, Muenster
 *
 * Acknowledgement
 * This work was partially funded by the Bundesministerium fuer Bildung und
 * Forschung (BMBF) of the Federal Republic of Germany (Foerderkennzeichen 01AK045).
 * The authors alone are responsible for the contents.
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
 * Contact: discussion@sctp.de
 *          dreibh@iem.uni-due.de
 *          tuexen@fh-muenster.de
 *
 * Purpose: Extended Socket API
 *
 */


#ifndef EXTSOCKET_H
#define EXTSOCKET_H


#ifndef HAVE_KERNEL_SCTP


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <poll.h>


#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif


#define SOCKETAPI_MAJOR_VERSION  0x2
#define SOCKETAPI_MINOR_VERSION  0x2200


/*
   The socketapi library internally combines the
   flags and sinfo_flags fields! Here, it is
   ensured to define unique value.
*/
#define MSG_UNORDERED     MSG_DONTROUTE
#define MSG_UNBUNDLED     MSG_CTRUNC
#ifndef MSG_NOTIFICATION
#define MSG_NOTIFICATION  MSG_OOB
#endif
#ifndef MSG_EOF
#define MSG_EOF           MSG_FIN
#endif
#define MSG_SHUTDOWN      MSG_EOF
#define MSG_MULTIADDRS    MSG_TRUNC

#ifdef MSG_RST
#define MSG_ABORT         MSG_RST
#define MSG_PR_SCTP_TTL   MSG_ERRQUEUE
#define MSG_ADDR_OVER     MSG_MORE
#define MSG_SEND_TO_ALL   MSG_PROXY
#else
#define MSG_ABORT         0x200
#define MSG_PR_SCTP_TTL   0x400
#define MSG_ADDR_OVER     0x800
#define MSG_SEND_TO_ALL   0xc00
#endif

#define SCTP_UNORDERED    MSG_UNORDERED
#define SCTP_UNBUNDLED    MSG_UNBUNDLED
#define SCTP_NOTIFICATION MSG_NOTIFICATION
#define SCTP_ABORT        MSG_ABORT
#define SCTP_EOF          MSG_EOF
#define SCTP_ADDR_OVER    MSG_ADDR_OVER
#define SCTP_SEND_TO_ALL  MSG_SEND_TO_ALL
#define SCTP_MULTIADDRS   MSG_MULTIADDRS


typedef unsigned int   sctp_assoc_t;
typedef unsigned short sctp_stream_t;


#define SCTP_UNDEFINED 0

#define SCTP_INIT 1
struct sctp_initmsg {
   uint16_t sinit_num_ostreams;
   uint16_t sinit_max_instreams;
   uint16_t sinit_max_attempts;
   uint16_t sinit_max_init_timeo;
};

#define SCTP_SNDRCV 2
struct sctp_sndrcvinfo
{
   uint16_t     sinfo_stream;
   uint16_t     sinfo_ssn;
   /* !!! 32 bits instead of 16 bits !!! */
   uint32_t     sinfo_flags;
   uint32_t     sinfo_ppid;
   uint32_t     sinfo_context;
   uint32_t     sinfo_timetolive;
   uint32_t     sinfo_tsn;
   uint32_t     sinfo_cumtsn;
   sctp_assoc_t sinfo_assoc_id;
};

#define SCTP_ASSOC_CHANGE 1
struct sctp_assoc_change
{
   uint16_t     sac_type;
   uint16_t     sac_flags;
   uint32_t     sac_length;
   uint16_t     sac_state;
   uint16_t     sac_error;
   uint16_t     sac_outbound_streams;
   uint16_t     sac_inbound_streams;
   sctp_assoc_t sac_assoc_id;
};
#define SCTP_COMM_UP        11
#define SCTP_COMM_LOST      12
#define SCTP_RESTART        13
#define SCTP_SHUTDOWN_COMP  14
#define SCTP_CANT_STR_ASSOC 15

#define SCTP_PEER_ADDR_CHANGE 2
struct sctp_paddr_change
{
    uint16_t                spc_type;
    uint16_t                spc_flags;
    uint32_t                spc_length;
    struct sockaddr_storage spc_aaddr;
    int                     spc_state;
    int                     spc_error;
    sctp_assoc_t            spc_assoc_id;
};
#define SCTP_ADDR_REACHABLE   21
#define SCTP_ADDR_UNREACHABLE 22
#define SCTP_ADDR_REMOVED     23
#define SCTP_ADDR_ADDED       24
#define SCTP_ADDR_MADE_PRIM   25
#define SCTP_ADDR_CONFIRMED   26

#define SCTP_REMOTE_ERROR 3
struct sctp_remote_error
{
   uint16_t     sre_type;
   uint16_t     sre_flags;
   uint32_t     sre_length;
   uint16_t     sre_error;
   sctp_assoc_t sre_assoc_id;
   uint8_t      sre_data[0];
};

#define SCTP_SEND_FAILED 4
struct sctp_send_failed
{
   uint16_t               ssf_type;
   uint16_t               ssf_flags;
   uint32_t               ssf_length;
   uint32_t               ssf_error;
   struct sctp_sndrcvinfo ssf_info;
   sctp_assoc_t           ssf_assoc_id;
   uint8_t                ssf_data[0];
};
#define SCTP_DATA_UNSENT 41
#define SCTP_DATA_SENT   42


#define SCTP_SHUTDOWN_EVENT 5
struct sctp_shutdown_event
{
   uint16_t     sse_type;
   uint16_t     sse_flags;
   uint32_t     sse_length;
   sctp_assoc_t sse_assoc_id;
};


#define SCTP_ADAPTATION_INDICATION 6
struct sctp_adaptation_event
{
   uint16_t     sai_type;
   uint16_t     sai_flags;
   uint32_t     sai_length;
   uint32_t     sai_adaptation_ind;
   sctp_assoc_t sai_assoc_id;
};


#define SCTP_PARTIAL_DELIVERY_EVENT 7
#define SCTP_PARTIAL_DELIVERY_ABORTED 1
struct sctp_pdapi_event
{
   uint16_t     pdapi_type;
   uint16_t     pdapi_flags;
   uint32_t     pdapi_length;
   uint32_t     pdapi_indication;
   sctp_assoc_t pdapi_assoc_id;
};


/*
   For interal implementation usage only!
 */
#define SCTP_DATA_ARRIVE 8
#define SCTP_ARRIVE_UNORDERED (1 << 0)
struct sctp_data_arrive
{
   uint16_t      sda_type;
   uint16_t      sda_flags;
   uint32_t      sda_length;
   sctp_assoc_t  sda_assoc_id;
   sctp_stream_t sda_stream;
   uint32_t      sda_ppid;
   uint32_t      sda_bytes_arrived;
};


union sctp_notification {
   struct {
      uint16_t sn_type;
      uint16_t sn_flags;
      uint32_t sn_length;
   }                           sn_header;
   struct sctp_assoc_change    sn_assoc_change;
   struct sctp_paddr_change    sn_paddr_change;
   struct sctp_remote_error    sn_remote_error;
   struct sctp_send_failed     sn_send_failed;
   struct sctp_shutdown_event  sn_shutdown_event;
   struct sctp_adaptation_event  sn_adaptation_event;
   struct sctp_pdapi_event     sn_pdapi_event;

   struct sctp_data_arrive     sn_data_arrive;
};





struct sctp_rtoinfo
{
   sctp_assoc_t srto_assoc_id;
   uint32_t     srto_initial;
   uint32_t     srto_max;
   uint32_t     srto_min;
};


struct sctp_assocparams
{
   sctp_assoc_t sasoc_assoc_id;
   uint16_t     sasoc_asocmaxrxt;
   uint16_t     sasoc_number_peer_destinations;
   uint32_t     sasoc_peer_rwnd;
   int32_t      sasoc_local_rwnd;
   uint32_t     sasoc_cookie_life;
};


struct sctp_setprim
{
   sctp_assoc_t            ssp_assoc_id;
   struct sockaddr_storage ssp_addr;
};


struct sctp_setpeerprim
{
   sctp_assoc_t            sspp_assoc_id;
   struct sockaddr_storage sspp_addr;
};


struct sctp_setstrm_timeout
{
   sctp_assoc_t ssto_assoc_id;
   uint32_t     ssto_timeout;
   uint16_t     ssto_streamid_start;
   uint16_t     ssto_streamid_end;
};


#define SPP_HB_ENABLE         (1 << 0)
#define SPP_HB_DISABLE        (1 << 1)
#define SPP_PMTUD_ENABLE      (1 << 2)
#define SPP_PMTUD_DISABLE     (1 << 3)
#define SPP_SACKDELAY_ENABLE  (1 << 4)
#define SPP_SACKDELAY_DISABLE (1 << 5)

struct sctp_paddrparams {
   sctp_assoc_t            spp_assoc_id;
   struct sockaddr_storage spp_address;
   uint32_t                spp_hbinterval;
   uint16_t                spp_pathmaxrxt;
   uint32_t                spp_pathmtu;
   uint32_t                spp_sackdelay;
   uint32_t                spp_flags;
};


struct sctp_paddrinfo {
   sctp_assoc_t            spinfo_assoc_id;
   struct sockaddr_storage spinfo_address;
   int32_t                 spinfo_state;
   uint32_t                spinfo_cwnd;
   uint32_t                spinfo_srtt;
   uint32_t                spinfo_rto;
   uint32_t                spinfo_mtu;
};


#define SCTP_INACTIVE 0
#define SCTP_ACTIVE   1

struct sctp_status
{
   sctp_assoc_t          sstat_assoc_id;
   int32_t               sstat_state;
   uint32_t              sstat_rwnd;
   uint16_t              sstat_unackdata;
   uint16_t              sstat_penddata;
   uint16_t              sstat_instrms;
   uint16_t              sstat_outstrms;
   uint32_t              sstat_fragmentation_point;
   struct sctp_paddrinfo sstat_primary;
};


struct sctp_event_subscribe
{
   uint8_t sctp_data_io_event;
   uint8_t sctp_association_event;
   uint8_t sctp_address_event;
   uint8_t sctp_send_failure_event;
   uint8_t sctp_peer_error_event;
   uint8_t sctp_shutdown_event;
   uint8_t sctp_partial_delivery_event;
   uint8_t sctp_adaptation_layer_event;
};


struct sctp_assoc_value {
   sctp_assoc_t assoc_id;
   uint32_t     assoc_value;
};


struct sctp_sack_info {
   sctp_assoc_t sack_assoc_id;
   uint32_t     sack_delay;
   uint32_t     sack_freq;
};


#define SCTP_INITMSG                1000
#define SCTP_AUTOCLOSE              1001

#define SCTP_RTOINFO                1010
#define SCTP_ASSOCINFO              1011
#define SCTP_PRIMARY_ADDR           1012
#define SCTP_SET_PEER_PRIMARY_ADDR  1013
#define SCTP_SET_STREAM_TIMEOUTS    1014
#define SCTP_PEER_ADDR_PARAMS       1015
#define SCTP_STATUS                 1016
#define SCTP_GET_PEER_ADDR_INFO     1017

#define SCTP_NODELAY                1018
#define SCTP_SET_DEFAULT_SEND_PARAM 1019
#define SCTP_EVENTS                 1020
#define SCTP_DELAYED_SACK           1021
#define SCTP_FRAGMENT_INTERLEAVE    1022
#define SCTP_PARTIAL_DELIVERY_POINT 1023
#define SCTP_MAXSEG                 1024
#define SCTP_I_WANT_MAPPED_V4_ADDR  1025



#ifdef __cplusplus
extern "C" {
#endif

unsigned int socketAPIGetVersion();
int ext_socket(int domain, int type, int protocol);
int ext_open(const char* pathname, int flags, mode_t mode);
int ext_creat(const char* pathname, mode_t mode);
int ext_bind(int sockfd, struct sockaddr* my_addr, socklen_t addrlen);
int ext_connect(int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen);
int ext_listen(int s, int backlog);
int ext_accept(int s,  struct  sockaddr * addr,  socklen_t* addrlen);
int ext_shutdown(int s, int how);
int ext_close(int fd);
int ext_getsockname(int sockfd, struct sockaddr* name, socklen_t* namelen);
int ext_getpeername(int sockfd, struct sockaddr* name, socklen_t* namelen);
int ext_fcntl(int fd, int cmd, ...);
int ext_ioctl(int d, int request, const void* argp);
int ext_getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen);
int ext_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
ssize_t ext_recv(int s, void* buf, size_t len, int flags);
ssize_t ext_recvfrom(int  s,  void * buf,  size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);
ssize_t ext_recvmsg(int s, struct msghdr* msg, int flags);
ssize_t ext_send(int s, const void* msg, size_t len, int flags);
ssize_t ext_sendto(int s, const void* msg, size_t len, int flags, const struct sockaddr* to, socklen_t tolen);
ssize_t ext_sendmsg(int s, const struct msghdr* msg, int flags);
ssize_t ext_read(int fd, void* buf, size_t count);
ssize_t ext_write(int fd, const void* buf, size_t count);
int ext_select(int n, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
int ext_poll(struct pollfd* fdlist, long unsigned int count, int time);


/* For internal usage only! */
int ext_recvmsg2(int sockfd, struct msghdr* msg, int flags,
                 const int receiveNotifications);


#define SCTP_BINDX_ADD_ADDR 1
#define SCTP_BINDX_REM_ADDR 2

int sctp_bindx(int              sockfd,
               struct sockaddr* addrs,
               int              addrcnt,
               int              flags);

int ext_connectx(int                    sockfd,
                 const struct sockaddr* addrs,
                 int                    addrcnt,
                 sctp_assoc_t*          id);
#define sctp_connectx ext_connectx

int sctp_peeloff(int sockfd, sctp_assoc_t id);

int sctp_getpaddrs(int sockfd, sctp_assoc_t id, struct sockaddr** addrs);
void sctp_freepaddrs(struct sockaddr* addrs);

int sctp_getladdrs(int sockfd, sctp_assoc_t id, struct sockaddr** addrs);
void sctp_freeladdrs(struct sockaddr* addrs);

int sctp_opt_info(int sd, sctp_assoc_t assocID, int opt, void* arg, socklen_t* size);

ssize_t sctp_send(int                           s,
                  const void*                   data,
                  size_t                        len,
                  const struct sctp_sndrcvinfo* sinfo,
                  int                           flags);
ssize_t sctp_sendx(int                           sd,
                   const void*                   data,
                   size_t                        len,
                   const struct sockaddr*        addrs,
                   int                           addrcnt,
                   const struct sctp_sndrcvinfo* sinfo,
                   int                           flags);
ssize_t sctp_sendmsg(int                    s,
                     const void*            data,
                     size_t                 len,
                     const struct sockaddr* to,
                     socklen_t              tolen,
                     uint32_t               ppid,
                     uint32_t               flags,
                     uint16_t               stream_no,
                     uint32_t               timetolive,
                     uint32_t               context);
ssize_t sctp_recvmsg(int                     s,
                     void*                   msg,
                     size_t                  len,
                     struct sockaddr*        from,
                     socklen_t*              fromlen,
                     struct sctp_sndrcvinfo* sinfo,
                     int*                    msg_flags);
int ext_pipe(int fds[2]);


/**
  * Check, if SCTP support is available.
  *
  * @return true, if SCTP is available; false otherwise.
  */
int sctp_isavailable();

/**
  * Enable or disable OOTB handling.
  *
  * @param enable 0 to disable, <>0 to enable OOTB handling.
  * @return 0 for success, error code in case of error.
  */
int sctp_enableOOTBHandling(const unsigned int enable);

/**
  * Enable or disable CRC32 checksum.
  *
  * @param enable 0 to disable (use Adler32), <>0 to enable CRC32.
  * @return 0 for success, error code in case of error.
  */
int sctp_enableCRC32(const unsigned int enable);


#ifdef __cplusplus
}
#endif


#else


#ifdef __cplusplus
#define ext_socket(a,b,c) ::socket(a,b,c)
#define ext_bind(a,b,c) ::bind(a,b,c)
#define ext_connect(a,b,c) ::connect(a,b,c)
#define ext_listen(a,b) ::listen(a,b)
#define ext_accept(a,b,c) ::accept(a,b,c)
#define ext_shutdown(a,b) ::shutdown(a,b)
#define ext_close(a) ::close(a)
#define ext_getsockname(a,b,c) ::getsockname(a,b,c)
#define ext_getpeername(a,b,c) ::getpeername(a,b,c)
#define ext_fcntl(a,b,c) ::fcntl(a,b,c)
#define ext_ioctl(a,b,c) ::ioctl(a,b,c)
#define ext_getsockopt(a,b,c,d,e) ::getsockopt(a,b,c,d,e)
#define ext_setsockopt(a,b,c,d,e) ::setsockopt(a,b,c,d,e)
#define ext_recv(a,b,c,d) ::recv(a,b,c,d)
#define ext_recvfrom(a,b,c,d,e,f) ::recvfrom(a,b,c,d,e,f)
#define ext_recvmsg(a,b,c) ::recvmsg(a,b,c)
#define ext_send(a,b,c,d) ::send(a,b,c,d)
#define ext_sendto(a,b,c,d,e,f) ::sendto(a,b,c,d,e,f)
#define ext_sendmsg(a,b,c) ::sendmsg(a,b,c)
#define ext_read(a,b,c) ::read(a,b,c)
#define ext_write(a,b,c) ::write(a,b,c)
#define ext_select(a,b,c,d,e) ::select(a,b,c,d,e)
#ifndef USE_SELECT
#define ext_poll(a,b,c) ::poll(a,b,c)
#endif
#define ext_pipe(a) ::pipe(a)
#else
#define ext_socket(a,b,c) socket(a,b,c)
#define ext_bind(a,b,c) bind(a,b,c)
#define ext_connect(a,b,c) connect(a,b,c)
#define ext_listen(a,b) listen(a,b)
#define ext_accept(a,b,c) accept(a,b,c)
#define ext_shutdown(a,b) shutdown(a,b)
#define ext_close(a) close(a)
#define ext_getsockname(a,b,c) getsockname(a,b,c)
#define ext_getpeername(a,b,c) getpeername(a,b,c)
#define ext_fcntl(a,b,c) fcntl(a,b,c)
#define ext_ioctl(a,b,c) ioctl(a,b,c)
#define ext_getsockopt(a,b,c,d,e) getsockopt(a,b,c,d,e)
#define ext_setsockopt(a,b,c,d,e) setsockopt(a,b,c,d,e)
#define ext_recv(a,b,c,d) recv(a,b,c,d)
#define ext_recvfrom(a,b,c,d,e,f) recvfrom(a,b,c,d,e,f)
#define ext_recvmsg(a,b,c) recvmsg(a,b,c)
#define ext_send(a,b,c,d) send(a,b,c,d)
#define ext_sendto(a,b,c,d,e,f) sendto(a,b,c,d,e,f)
#define ext_sendmsg(a,b,c) sendmsg(a,b,c)
#define ext_read(a,b,c) read(a,b,c)
#define ext_write(a,b,c) write(a,b,c)
#define ext_select(a,b,c,d,e) select(a,b,c,d,e)
#ifndef USE_SELECT
#define ext_poll(a,b,c) poll(a,b,c)
#endif
#define ext_pipe(a) pipe(a)
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <netinet/sctp.h>

#ifndef SCTP_DELAYED_SACK
#define SCTP_DELAYED_SACK SCTP_DELAYED_ACK_TIME
#endif

#endif

#endif
