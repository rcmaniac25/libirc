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

/* Test checkin! */

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#endif /* !defined(_WIN32) */


#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "conf.h"

#include "map.h"
#include "list.h"
#include "irc_core.h"
#include "tcp.h"
#include "cmd.h"
#include "ctcp.h"
#include "trace.h"
#include "defs.h"
#include "util.h"

#include "errcodes.h"
#include "macros_priv.h"

/* Local functions */
static int handle_connection(srv_h srv);
static void execute_errhandler(srv_h srv, int errcode);
static void parse_message(srv_h srv, char* msg);

/* Global variables */
errhandler_t	errorHandler		= NULL;
slist_t			servers				= EMPTY_LIST;
map_t			callbacks			= NULL;
callback_t		fallback_callback	= NULL;

/* Local variables */

/* Exported functions */
int
irc_login( srv_h	srv,
		  const char*	nick,
		  const char*	user,
		  const char*	fullname )
{
	char buf[129];
	if(irc_send_cmd(srv, CMD_NICK, nick) < 0)
		return -1;
	snprintf(buf, 128, "%s %s %s :%s", 
		user, "XXX", srv->host, fullname);
	if(irc_send_cmd(srv, CMD_USER, buf) < 0)
		return -1;
	return 0;
}


void
irc_set_data( srv_h server,
			 void* data )
{
	server->data = data;
}

void*
irc_get_data( srv_h server )
{
	return server->data;
}

void 
irc_set_callback( callback_t	cb,
				 const char*	cmd )
{
	if(callbacks == NULL) {
		map_type_t	type;
#ifdef CALLBACK_COLLECTION_IS_AVLTREE
		map_prop_value_t props[] = { 
			{ MP_KEY_CMP_FUNC, VALUE(strcmp) } 
		};
		type = MT_TREE;
#elif defined(CALLBACK_COLLECTION_IS_HASHTABLE)
		map_prop_value_t props[] = {
			{ MP_TABLE_SIZE,		VALUE(337) },
			{ MP_KEY_CMP_FUNC,	VALUE(strcmp) },
			{ MP_KEY_HASH_FUNC,	VALUE(hashpjw) }
		};
		type = MT_HASHTABLE;
#else
#error Define either CALLBACK_COLLECTION_IS_HASHTABLE or CALLBACK_COLLECTION_IS_AVLTREE
#endif
		callbacks = map_create(type, props, sizeof(props) / sizeof(props[0]));
	}

	if(!map_put(callbacks, (void*)cmd, cb))
		assert(0);
}

void
irc_set_fallback_callback( callback_t cb )
{
	fallback_callback = cb;
}

void
irc_set_errhandler( errhandler_t	errh ) 

{
	errorHandler = errh;
}

void
irc_add_server_listener( srv_h server )
{
	servers = slist_append(servers, server);
}

void
irc_remove_server_listener( srv_h server )
{
	servers = slist_remove(servers, server);
}


int
irc_handle_server( srv_h server )
{
	return handle_connection(server);
}

/* Hidden but exported functions */
int handle_connection(srv_h srv);

srv_h
irc_connect(	const char*	server,
		unsigned short		port)
{
     srv_h		srv;
     buf_sock_t	sock;
     
     if(!(sock = buf_sock_connect(server, port)))
	  return NULL;
     
     srv = (srv_h)malloc(sizeof(struct _srv));
     srv->sock = sock;
     srv->data = NULL;    
     srv->port = port;
     srv->host = strdup(server);
     
     return srv;
}

void
irc_disconnect( srv_h server )
{
     buf_sock_close(server->sock);
     free(server);
}

int
irc_send_cmd( srv_h server,
	      const char* cmd,
	      const char* params )
{
     if(buf_sock_write(server->sock, cmd) != BS_OK)
	  return -1;
     if(buf_sock_write(server->sock, " ") != BS_OK)
	  return -1;
     if(buf_sock_write_line(server->sock, params) != BS_OK)
	  return -1;
     return 0;
}

int
irc_send_raw( srv_h server, 
	      const char* rawtext )
{
     if(buf_sock_write_line(server->sock, rawtext) != BS_OK)
	  return -1;
     return 0;
}

SOCKET
irc_getfdset( fd_set* set )
{
     SOCKET	maxfd;
     slist_t	i;
     srv_h	srv;
     
     FD_ZERO(set);
#ifdef _WIN32
	 maxfd = 0;
#else
     maxfd = -1;
#endif

     for(i = servers; i != EMPTY_LIST; i = slist_next(i)) {
	  SOCKET srv_fd;
	  
	  srv = (srv_h)slist_value(i);
	  srv_fd = buf_sock_get_raw_socket(srv->sock);
	  FD_SET(srv_fd, set);
	  if(srv_fd > maxfd)
	       maxfd = srv_fd;
     }
     
     return maxfd;
}

srv_h
irc_nextsrv( fd_set* set )
{
     slist_t i;
     srv_h srv;
     
     for(i = servers; i != EMPTY_LIST; i = slist_next(i)) {
	  SOCKET srv_fd;

	  srv = (srv_h)slist_value(i);
	  srv_fd = buf_sock_get_raw_socket(srv->sock);
	  if(FD_ISSET(srv_fd, set)) {
	       FD_CLR(srv_fd, set);
	       return srv;
	  }
     }
     return NULL;
}

/* Local functions */
int handle_connection(srv_h srv)
{
	buf_lines_t		lines;
	buf_sock_status_t	status;

	status = buf_sock_read_lines(srv->sock, &lines);
	if(BS_OK == status) {
	     int i;
	     for(i = 0; i < lines.num; ++i)
		  parse_message(srv, lines.lines[i]);
	     buf_sock_free_lines(&lines);
	     return 0;
	} else if(BS_INCOMPLETE_READ == status) {
	     return 0;
	} else if(BS_EOF == status) {
	     execute_errhandler(srv, ERR_EOF);
	     return -1;
	} else if(BS_ERR == status) {
	     execute_errhandler(srv, ERR_CONN_BROKEN);
	     return -1;
	} else {
	     assert(0);
	     return -1;
	}
}

/* XXX: Hack.. trims the white spaces at the end of a string */
static void util_trim_end(char* str)
{
	size_t len;

	assert(str != NULL);
	len = strlen(str);
	if(len < 1 || str[len - 1] != ' ')
		return;
	while(len > 0 && str[len - 1] == ' ') {
		str[len - 1] = '\0';
		len--;
	}
}

static void execute_errhandler(srv_h srv, int errcode)
{
     DO_ERROR(srv, errcode);
}

static unsigned int calc_params(char* msg)
{
     unsigned int num;

     num = 1; /* There is always one parameter */
     while(*msg && *msg != ':')
     {
	  if(*msg == ' ') { num++; while(*msg == ' ') msg++; }
	  else msg++;
     }
     
     return num;
}

static void parse_params(char* msg, char** arr)
{
     while(*msg && *msg != ':') {
	  if(*msg == ' ') {
	       while(*msg == ' ') {
		    *msg = '\0';
		    msg++;
	       }
	  } else {
	       *arr = msg;
	       arr++;
	       while(*msg && *msg != ' ') msg++;
	  }
     }
     if(*msg == ':')
	  *arr = msg + 1;
}

static void parse_message(srv_h srv, char* msg)
{
     char* arr_params[MAX_PARAMS];
     unsigned int num_params;
     const char* prefix;
     const char* cmd;
     char* i;
     callback_t cb;
     
#ifndef NDEBUG
     TRACE("%s", msg);
#endif  /* !NDEBUG */
     
	/* If there are no callbacks, then processing the
	   message is unnecessary */
     if(callbacks == NULL)
	  return;
     
     /* Strip all newline crap at the end of the message (if any present) */
     for(i = msg + strlen(msg) - 1; *i == '\r' || *i == '\n'; i--)
	  *i = '\0';
     
     /* If message starts with ':' -> it has a prefix */
     if(*msg == ':') {
	  prefix = msg + 1;
	  /* Wipe out all spaces trailing the prefix */
	  for(i = msg + 1; *i != ' '; i++);
	  for(; *i == ' '; i++)
	       *i = '\0';
	  msg = i;
     } else {
	  prefix = NULL;
     }
     cmd = msg;
     for(i = msg; *i != ' ' && *i; i++);
     for(; *i == ' '; i++)
	  *i = '\0';
     
     msg = i;
     util_trim_end(msg);
     num_params = calc_params(msg);
     parse_params(msg, arr_params);
     
#if !defined(DONT_PROCESS_CTCP)
     /* CTCP Notice.. reroute to CTCP layer */
     if(!strcasecmp(cmd, "NOTICE") && *arr_params[1] == CTCP_MARKER) {
	  ctcp_dispatch_msg(srv, arr_params[0], prefix, FALSE, arr_params[1]);
	  
	  /* CTCP Request */
     } else if(!strcasecmp(cmd, "PRIVMSG") && *arr_params[1] == CTCP_MARKER) {
	  ctcp_dispatch_msg(srv, arr_params[0], prefix, TRUE, arr_params[1]);
	  
     } else { /* Not a CTCP message! */
#endif
	  /* Now we now what command it is.. lets see if
	     there is a callback for it. If not, use the fallback callback */
	  if(!map_get(callbacks, (void*)cmd, (void**)&cb) || cb == NULL)
	       cb = fallback_callback;
	  
	  /* If the fallback_callback is NULL, drop message */
	  if(cb == NULL)
	       return;
	  
	  cb(srv, prefix, cmd, arr_params, num_params);    
#if !defined(DONT_PROCESS_CTCP)
     }
#endif
}
