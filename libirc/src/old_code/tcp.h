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
#ifndef __TCP_H__
#define __TCP_H__

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* Function prototypes */
SOCKET tcp_connect(const char* server, unsigned short port);
SOCKET tcp_server(const char* addr, unsigned short port);
void tcp_close(SOCKET s);
int tcp_recv(SOCKET s, void* buf, int len);
int tcp_send(SOCKET s, const void* buf, int len);

#ifdef __cplusplus
}
#endif 

#endif /* __TCP_H__ */
