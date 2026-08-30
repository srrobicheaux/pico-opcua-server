#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#define NO_SYS                  0
#define SYS_LIGHTWEIGHT_PROT    1

/* Enable NetBIOS Name Service for Windows / Matrikon resolution */
#define LWIP_NETBIOSNS          1

/* Ensure mDNS has enough service slots allocated */
#define MDNS_MAX_SERVICES       4

#define LWIP_IGMP               1
#define LWIP_IPV4               1
#define LWIP_UDP                1
#define LWIP_DHCP               1
#define LWIP_SOCKET             1
#define LWIP_COMPAT_SOCKETS     1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1

#define LWIP_NETCONN            1
#define LWIP_HAVE_LOOPIF        1
#define LWIP_LOOPBACK_MAX_PBUFS 8

#define MEM_ALIGNMENT           4
/* Expanded lwIP heap to support up to 10-12 concurrent active TCP buffers */
#define MEM_SIZE                (48 * 1024)
#define PBUF_POOL_SIZE          32

#define TCPIP_THREAD_STACKSIZE  4096
#define TCPIP_THREAD_PRIO       3
#define TCPIP_MBOX_SIZE         16
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_TCP_RECVMBOX_SIZE 8
#define DEFAULT_ACCEPTMBOX_SIZE  8
#define LWIP_TIMEVAL_PRIVATE    0
#define LWIP_PROVIDE_ERRNO      1
#define LWIP_ERRNO_STDINCLUDE   1

#define LWIP_ENABLE_NETIF_HOSTNAME 1
#define LWIP_NETIF_HOSTNAME     1

/* Match netconns and PCBs for multi-client commercial tools */
#define MEMP_NUM_NETCONN        16
#define MEMP_NUM_TCP_PCB        16
#define MEMP_NUM_TCP_PCB_LISTEN 8

/* Moderate TCP Window sizes (5.8 KB per socket instead of 11.6 KB) */
#define TCP_MSS                 1460
#define TCP_WND                 (4 * TCP_MSS)
#define TCP_SND_BUF             (4 * TCP_MSS)
#define MEMP_NUM_TCP_SEG        (4 * TCP_SND_QUEUELEN)

/* mDNS, SNTP, and System Timeouts */
#define LWIP_MDNS_RESPONDER     1
#define LWIP_NUM_NETIF_CLIENT_DATA 1
#define MDNS_RESP_USENETIF_EXTCALLBACK 1
/* Increased to prevent sys_timeout allocations from failing during reconnects */
#define MEMP_NUM_SYS_TIMEOUT    (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8)

#define LWIP_SNTP               1
#define LWIP_DNS                1
#define SNTP_SERVER_DNS         1

/* Shorten TIME_WAIT state to 1 second for instant socket reuse */
#define TCP_MSL                 1000 

#define SNTP_SET_SYSTEM_TIME_US(sec, us) \
    do { \
        struct timeval tv = { .tv_sec = sec, .tv_usec = us }; \
        settimeofday(&tv, NULL); \
    } while(0)

#endif /* _LWIPOPTS_H */