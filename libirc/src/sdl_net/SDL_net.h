/*
    SDL_net:  An example cross-platform network library for use with SDL
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* $Id$ */

/* 2004-09-22 modified to be stand alone code, independent of SDL for use in libIRC -  Jeff Myers */

#ifndef _SDLNET_STANDALONE_H
#define _SDLNET_STANDALONE_H

// SDL error stuff form SDL_error.h
void SDLNet_SetError(const char *error);
const char* SDLNet_GetError(void);
void SDLNet_ClearError(void);

// endian action

// byte order setuff from SDLNET_BYTEORDER.h
/* Macros for determining the byte-order of this platform */

/* The two types of endianness */
#define SDLNET_LIL_ENDIAN	1234
#define SDLNET_BIG_ENDIAN	4321

/* Pardon the mess, I'm trying to determine the endianness of this host.
I'm doing it by preprocessor defines rather than some sort of configure
script so that application code can use this too.  The "right" way would
be to dynamically generate this file on install, but that's a lot of work.
*/
#if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
	(defined(__alpha__) || defined(__alpha)) || \
	defined(__arm__) || \
	(defined(__mips__) && defined(__MIPSEL__)) || \
	defined(__SYMBIAN32__) || \
	defined(__x86_64__) || \
	defined(__LITTLE_ENDIAN__)
#define SDLNET_BYTEORDER	SDLNET_LIL_ENDIAN
#else
#define SDLNET_BYTEORDER	SDLNET_BIG_ENDIAN
#endif

#define Uint32	unsigned int
#define Uint16	unsigned short
#define Uint8	unsigned char

/* The macros used to swap values */
/* Try to use superfast macros on systems that support them */
#ifdef linux
#include <asm/byteorder.h>
#ifdef __arch__swab16
#define SDLNET_Swap16  __arch__swab16
#endif
#ifdef __arch__swab32
#define SDLNET_Swap32  __arch__swab32
#endif
#endif /* linux */
/* Use inline functions for compilers that support them, and static
functions for those that do not.  Because these functions become
static for compilers that do not support inline functions, this
header should only be included in files that actually use them.
*/
#ifndef SDLNET_Swap16
static inline Uint16 SDLNET_Swap16(Uint16 D) {
	return((D<<8)|(D>>8));
}
#endif
#ifndef SDLNET_Swap32
static inline Uint32 SDLNET_Swap32(Uint32 D) {
	return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}
#endif
#ifdef SDL_HAS_64BIT_TYPE
#ifndef SDLNET_Swap64
static inline Uint64 SDLNET_Swap64(Uint64 val) {
	Uint32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = (Uint32)(val&0xFFFFFFFF);
	val >>= 32;
	hi = (Uint32)(val&0xFFFFFFFF);
	val = SDLNET_Swap32(lo);
	val <<= 32;
	val |= SDLNET_Swap32(hi);
	return(val);
}
#endif
#else
#ifndef SDLNET_Swap64
/* This is mainly to keep compilers from complaining in SDL code.
If there is no real 64-bit datatype, then compilers will complain about
the fake 64-bit datatype that SDL provides when it compiles user code.
*/
#define SDLNET_Swap64(X)	(X)
#endif
#endif /* SDL_HAS_64BIT_TYPE */


/* Byteswap item from the specified endianness to the native endianness */
#if SDLNET_BYTEORDER == SDLNET_LIL_ENDIAN
#define SDLNET_SwapLE16(X)	(X)
#define SDLNET_SwapLE32(X)	(X)
#define SDLNET_SwapLE64(X)	(X)
#define SDLNET_SwapBE16(X)	SDLNET_Swap16(X)
#define SDLNET_SwapBE32(X)	SDLNET_Swap32(X)
#define SDLNET_SwapBE64(X)	SDLNET_Swap64(X)
#else
#define SDLNET_SwapLE16(X)	SDLNET_Swap16(X)
#define SDLNET_SwapLE32(X)	SDLNET_Swap32(X)
#define SDLNET_SwapLE64(X)	SDLNET_Swap64(X)
#define SDLNET_SwapBE16(X)	(X)
#define SDLNET_SwapBE32(X)	(X)
#define SDLNET_SwapBE64(X)	(X)
#endif

/* Initialize/Cleanup the network API
   SDL must be initialized before calls to functions in this library,
   because this library uses utility functions from the SDL library.
*/
int SDLNet_Init(void);
void SDLNet_Quit(void);

/***********************************************************************/
/* IPv4 hostname resolution API                                        */
/***********************************************************************/

typedef struct {
	Uint32 host;			/* 32-bit IPv4 host address */
	Uint16 port;			/* 16-bit protocol port */
} IPaddress;

/* Resolve a host name and port to an IP address in network form.
   If the function succeeds, it will return 0.
   If the host couldn't be resolved, the host portion of the returned
   address will be INADDR_NONE, and the function will return -1.
   If 'host' is NULL, the resolved host will be set to INADDR_ANY.
 */
#ifndef INADDR_ANY
#define INADDR_ANY		0x00000000
#endif
#ifndef INADDR_NONE
#define INADDR_NONE		0xFFFFFFFF
#endif
#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST	0xFFFFFFFF
#endif

int SDLNet_ResolveHost(IPaddress *address, const char *host, Uint16 port);

/* Resolve an ip address to a host name in canonical form.
   If the ip couldn't be resolved, this function returns NULL,
   otherwise a pointer to a static buffer containing the hostname
   is returned.  Note that this function is not thread-safe.
*/
const char * SDLNet_ResolveIP(IPaddress *ip);


/***********************************************************************/
/* TCP network API                                                     */
/***********************************************************************/

typedef struct _TCPsocket *TCPsocket;

/* Open a TCP network socket
   If ip.host is INADDR_NONE or INADDR_ANY, this creates a local server
   socket on the given port, otherwise a TCP connection to the remote
   host and port is attempted. The address passed in should already be
   swapped to network byte order (addresses returned from 
   SDLNet_ResolveHost() are already in the correct form).
   The newly created socket is returned, or NULL if there was an error.
*/
TCPsocket SDLNet_TCP_Open(IPaddress *ip);

/* Accept an incoming connection on the given server socket.
   The newly created socket is returned, or NULL if there was an error.
*/
TCPsocket SDLNet_TCP_Accept(TCPsocket server);

/* Get the IP address of the remote system associated with the socket.
   If the socket is a server socket, this function returns NULL.
*/
IPaddress * SDLNet_TCP_GetPeerAddress(TCPsocket sock);

/* Send 'len' bytes of 'data' over the non-server socket 'sock'
   This function returns the actual amount of data sent.  If the return value
   is less than the amount of data sent, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int SDLNet_TCP_Send(TCPsocket sock, void *data, int len);

/* Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
   and store them in the buffer pointed to by 'data'.
   This function returns the actual amount of data received.  If the return
   value is less than or equal to zero, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int SDLNet_TCP_Recv(TCPsocket sock, void *data, int maxlen);

/* Close a TCP network socket */
void SDLNet_TCP_Close(TCPsocket sock);


/***********************************************************************/
/* UDP network API                                                     */
/***********************************************************************/

/* The maximum channels on a a UDP socket */
#define SDLNET_MAX_UDPCHANNELS	32
/* The maximum addresses bound to a single UDP socket channel */
#define SDLNET_MAX_UDPADDRESSES	4

typedef struct _UDPsocket *UDPsocket;
typedef struct {
	int channel;		/* The src/dst channel of the packet */
	Uint8 *data;		/* The packet data */
	int len;		/* The length of the packet data */
	int maxlen;		/* The size of the data buffer */
	int status;		/* packet status after sending */
	IPaddress address;		/* The source/dest address of an incoming/outgoing packet */
} UDPpacket;

/* Allocate/resize/free a single UDP packet 'size' bytes long.
   The new packet is returned, or NULL if the function ran out of memory.
 */
UDPpacket * SDLNet_AllocPacket(int size);
int SDLNet_ResizePacket(UDPpacket *packet, int newsize);
void SDLNet_FreePacket(UDPpacket *packet);

/* Allocate/Free a UDP packet vector (array of packets) of 'howmany' packets,
   each 'size' bytes long.
   A pointer to the first packet in the array is returned, or NULL if the
   function ran out of memory.
 */
UDPpacket ** SDLNet_AllocPacketV(int howmany, int size);
void SDLNet_FreePacketV(UDPpacket **packetV);


/* Open a UDP network socket
   If 'port' is non-zero, the UDP socket is bound to a local port.
   The 'port' should be given in native byte order, but is used
   internally in network (big endian) byte order, in addresses, etc.
   This allows other systems to send to this socket via a known port.
*/
UDPsocket SDLNet_UDP_Open(Uint16 port);

/* Bind the address 'address' to the requested channel on the UDP socket.
   If the channel is -1, then the first unbound channel will be bound with
   the given address as it's primary address.
   If the channel is already bound, this new address will be added to the
   list of valid source addresses for packets arriving on the channel.
   If the channel is not already bound, then the address becomes the primary
   address, to which all outbound packets on the channel are sent.
   This function returns the channel which was bound, or -1 on error.
*/
int SDLNet_UDP_Bind(UDPsocket sock, int channel, IPaddress *address);

/* Unbind all addresses from the given channel */
void SDLNet_UDP_Unbind(UDPsocket sock, int channel);

/* Get the primary IP address of the remote system associated with the 
   socket and channel.  If the channel is -1, then the primary IP port
   of the UDP socket is returned -- this is only meaningful for sockets
   opened with a specific port.
   If the channel is not bound and not -1, this function returns NULL.
 */
IPaddress * SDLNet_UDP_GetPeerAddress(UDPsocket sock, int channel);

/* Send a vector of packets to the the channels specified within the packet.
   If the channel specified in the packet is -1, the packet will be sent to
   the address in the 'src' member of the packet.
   Each packet will be updated with the status of the packet after it has 
   been sent, -1 if the packet send failed.
   This function returns the number of packets sent.
*/
int SDLNet_UDP_SendV(UDPsocket sock, UDPpacket **packets, int npackets);

/* Send a single packet to the specified channel.
   If the channel specified in the packet is -1, the packet will be sent to
   the address in the 'src' member of the packet.
   The packet will be updated with the status of the packet after it has
   been sent.
   This function returns 1 if the packet was sent, or 0 on error.

   NOTE:
   The maximum size of the packet is limited by the MTU (Maximum Transfer Unit)
   of the transport medium.  It can be as low as 250 bytes for some PPP links,
   and as high as 1500 bytes for ethernet.
*/
int SDLNet_UDP_Send(UDPsocket sock, int channel, UDPpacket *packet);

/* Receive a vector of pending packets from the UDP socket.
   The returned packets contain the source address and the channel they arrived
   on.  If they did not arrive on a bound channel, the the channel will be set
   to -1.
   The channels are checked in highest to lowest order, so if an address is
   bound to multiple channels, the highest channel with the source address
   bound will be returned.
   This function returns the number of packets read from the network, or -1
   on error.  This function does not block, so can return 0 packets pending.
*/
int SDLNet_UDP_RecvV(UDPsocket sock, UDPpacket **packets);

/* Receive a single packet from the UDP socket.
   The returned packet contains the source address and the channel it arrived
   on.  If it did not arrive on a bound channel, the the channel will be set
   to -1.
   The channels are checked in highest to lowest order, so if an address is
   bound to multiple channels, the highest channel with the source address
   bound will be returned.
   This function returns the number of packets read from the network, or -1
   on error.  This function does not block, so can return 0 packets pending.
*/
int SDLNet_UDP_Recv(UDPsocket sock, UDPpacket *packet);

/* Close a UDP network socket */
void SDLNet_UDP_Close(UDPsocket sock);


/***********************************************************************/
/* Hooks for checking sockets for available data                       */
/***********************************************************************/

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;

/* Any network socket can be safely cast to this socket type */
typedef struct {
	int ready;
} *SDLNet_GenericSocket;

/* Allocate a socket set for use with SDLNet_CheckSockets()
   This returns a socket set for up to 'maxsockets' sockets, or NULL if
   the function ran out of memory.
 */
SDLNet_SocketSet SDLNet_AllocSocketSet(int maxsockets);

/* Add a socket to a set of sockets to be checked for available data */
#define SDLNet_TCP_AddSocket(set, sock) \
			SDLNet_AddSocket(set, (SDLNet_GenericSocket)sock)
#define SDLNet_UDP_AddSocket(set, sock) \
			SDLNet_AddSocket(set, (SDLNet_GenericSocket)sock)
int SDLNet_AddSocket(SDLNet_SocketSet set, SDLNet_GenericSocket sock);

/* Remove a socket from a set of sockets to be checked for available data */
#define SDLNet_TCP_DelSocket(set, sock) \
			SDLNet_DelSocket(set, (SDLNet_GenericSocket)sock)
#define SDLNet_UDP_DelSocket(set, sock) \
			SDLNet_DelSocket(set, (SDLNet_GenericSocket)sock)
int SDLNet_DelSocket(SDLNet_SocketSet set, SDLNet_GenericSocket sock);

/* This function checks to see if data is available for reading on the
   given set of sockets.  If 'timeout' is 0, it performs a quick poll,
   otherwise the function returns when either data is available for
   reading, or the timeout in milliseconds has elapsed, which ever occurs
   first.  This function returns the number of sockets ready for reading, 
   or -1 if there was an error with the select() system call.
*/
int SDLNet_CheckSockets(SDLNet_SocketSet set, Uint32 timeout);

/* After calling SDLNet_CheckSockets(), you can use this function on a
   socket that was in the socket set, to find out if data is available
   for reading.
*/
#define SDLNet_SocketReady(sock) \
		((sock != NULL) && ((SDLNet_GenericSocket)sock)->ready)

/* Free a set of sockets allocated by SDL_NetAllocSocketSet() */
void SDLNet_FreeSocketSet(SDLNet_SocketSet set);


/***********************************************************************/
/* Platform-independent data conversion functions                      */
/***********************************************************************/

/* Write a 16/32 bit value to network packet buffer */
void SDLNet_Write16(Uint16 value, void *area);
void SDLNet_Write32(Uint32 value, void *area);

/* Read a 16/32 bit value from network packet buffer */
Uint16 SDLNet_Read16(void *area);
Uint32 SDLNet_Read32(void *area);

/* Inline macro functions to read/write network data */

/* Warning, some systems have data access alignment restrictions */
#if defined(sparc) || defined(mips)
#define SDL_DATA_ALIGNED	1
#endif
#ifndef SDL_DATA_ALIGNED
#define SDL_DATA_ALIGNED	0
#endif

/* Write a 16 bit value to network packet buffer */
#if !SDL_DATA_ALIGNED
#define SDLNet_Write16(value, areap)	\
	(*(Uint16 *)(areap) = SDLNET_SwapBE16(value))
#else
#if SDLNET_BYTEORDER == SDLNET_BIG_ENDIAN
#define SDLNet_Write16(value, areap)	\
do 					\
{					\
	Uint8 *area = (Uint8 *)(areap);	\
	area[0] = (value >>  8) & 0xFF;	\
	area[1] =  value        & 0xFF;	\
} while ( 0 )
#else
#define SDLNet_Write16(value, areap)	\
do 					\
{					\
	Uint8 *area = (Uint8 *)(areap);	\
	area[1] = (value >>  8) & 0xFF;	\
	area[0] =  value        & 0xFF;	\
} while ( 0 )
#endif
#endif /* !SDL_DATA_ALIGNED */

/* Write a 32 bit value to network packet buffer */
#if !SDL_DATA_ALIGNED
#define SDLNet_Write32(value, areap) 	\
	*(Uint32 *)(areap) = SDLNET_SwapBE32(value);
#else
#if SDLNET_BYTEORDER == SDLNET_BIG_ENDIAN
#define SDLNet_Write32(value, areap) 	\
do					\
{					\
	Uint8 *area = (Uint8 *)(areap);	\
	area[0] = (value >> 24) & 0xFF;	\
	area[1] = (value >> 16) & 0xFF;	\
	area[2] = (value >>  8) & 0xFF;	\
	area[3] =  value       & 0xFF;	\
} while ( 0 )
#else
#define SDLNet_Write32(value, areap) 	\
do					\
{					\
	Uint8 *area = (Uint8 *)(areap);	\
	area[3] = (value >> 24) & 0xFF;	\
	area[2] = (value >> 16) & 0xFF;	\
	area[1] = (value >>  8) & 0xFF;	\
	area[0] =  value       & 0xFF;	\
} while ( 0 )
#endif
#endif /* !SDL_DATA_ALIGNED */

/* Read a 16 bit value from network packet buffer */
#if !SDL_DATA_ALIGNED
#define SDLNet_Read16(areap) 		\
	(SDLNET_SwapBE16(*(Uint16 *)(areap)))
#else
#if SDLNET_BYTEORDER == SDLNET_BIG_ENDIAN
#define SDLNet_Read16(areap) 		\
	((((Uint8 *)areap)[0] <<  8) | ((Uint8 *)areap)[1] <<  0)
#else
#define SDLNet_Read16(areap) 		\
	((((Uint8 *)areap)[1] <<  8) | ((Uint8 *)areap)[0] <<  0)
#endif
#endif /* !SDL_DATA_ALIGNED */

/* Read a 32 bit value from network packet buffer */
#if !SDL_DATA_ALIGNED
#define SDLNet_Read32(areap) 		\
	(SDLNET_SwapBE32(*(Uint32 *)(areap)))
#else
#if SDLNET_BYTEORDER == SDLNET_BIG_ENDIAN
#define SDLNet_Read32(areap) 		\
	((((Uint8 *)areap)[0] << 24) | (((Uint8 *)areap)[1] << 16) | \
	 (((Uint8 *)areap)[2] <<  8) |  ((Uint8 *)areap)[3] <<  0)
#else
#define SDLNet_Read32(areap) 		\
	((((Uint8 *)areap)[3] << 24) | (((Uint8 *)areap)[2] << 16) | \
	 (((Uint8 *)areap)[1] <<  8) |  ((Uint8 *)areap)[0] <<  0)
#endif
#endif /* !SDL_DATA_ALIGNED */

#ifdef MACOS_OPENTRANSPORT
#endif

//#include "close_code.h"

#endif /* _SDLnet_h */
