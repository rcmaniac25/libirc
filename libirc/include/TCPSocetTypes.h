/* libIRC
* Copyright (c) 2004 Christopher Sean Morrison
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// TCP/IP socket types and includes

#ifndef _TCP_SOCKETTYPES_H_
#define _TCP_SOCKETTYPES_H_

// includes
#ifdef WIN32	// the winderz'
	// winsock is not quite like berkley sockets, so we have to make some defines
	#include <winsock2.h>      
	#include <Ws2tcpip.h>
#else 
	#ifdef __APPLE__	// the 'tosh
		#include <sys/types.h> 
		#include <sys/socket.h>
	#else // the 'nixes
		#include <sys/types.h>      // header containing all basic data types and
		#include <sys/socket.h>     // header containing socket data types and
		#include <netinet/in.h>     // IPv4 and IPv6 stuff
		#include <unistd.h>         // for gethostname()
		#include <netdb.h>          // for DNS - gethostbyname()
		#include <arpa/inet.h>      // contains all inet_* functions
		#include <errno.h>          // contains the error functions
		#include <fcntl.h>          // file control
	#endif // the 'nixes
#endif

// types and defines
#ifdef WIN32 	// the winderz'
	#ifndef socklen_t		// only some versions of winsock don't define this
		typedef unsigned int socklen_t;
	#endif
	typedef SOCKET sock; 

#else       
	#ifdef __APPLE__	// the 'tosh
		typedef int sock;
	#else				// UNIX/Linux
		typedef int sock; 
	#endif

#endif

typedef unsigned short		port;
typedef unsigned long		ipaddress;


#endif//_TCP_SOCKETTYPES_H_