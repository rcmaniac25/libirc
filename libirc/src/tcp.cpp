/*  Programmable IRC function library 
    Copyright (C) 1999-2002 Jorgen Sigvardsson <jorgen@wermland.se>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif /* defined(_WIN32) */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tcp.h"

SOCKET
tcp_connect(const char* server, unsigned short port)
{  
    struct hostent* hp;
    struct sockaddr_in sin;
    SOCKET s;

    if(!(hp = gethostbyname(server))) {
	return INVALID_SOCKET;
    }
    
    if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return INVALID_SOCKET;
    }
    
	memset(&sin, 0, sizeof(sin));
	memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    
    if(connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		return INVALID_SOCKET;
    }
    
    return s;
}

SOCKET
tcp_server(const char* addr, unsigned short port)
{
/* XXX: Implement me! */
    return INVALID_SOCKET;
}

void
tcp_close(SOCKET s) 
{
#ifdef _WIN32
     closesocket(s);
#else
     close(s);
#endif
}

int tcp_recv(SOCKET s, void* buf, int len)
{
	return recv(s, buf, len, 0);
}

int tcp_send(SOCKET s, const void* buf, int len)
{
	return send(s, buf, len, 0);
}
