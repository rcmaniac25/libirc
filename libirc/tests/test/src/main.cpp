/*  Programmable IRC function library 
    Copyright (C) 1999-2000 Jorgen Sigvardsson <jorgen@cs.kau.se>
    $Id$
    
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
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#endif /* _WIN32 */

#include "irc_core.h"
#include "cmd.h"
#include "ctcp.h"
#include "trace.h"
#include "irc_rpl.h"
#include "irc_version.h"

IRC_CALLBACK(ping) 
{
    printf("I got a ping...\n");
    irc_do_pong(srv, params[0]);
}

CTCP_CALLBACK(action)
{
    fprintf(stderr, "* %s %s\n", from, params);
}

CTCP_CALLBACK(version) 
{
     char* i;
     if(isRequest) {
	  char buf[100];
	  fprintf(stderr, "[CTCP VERSION REQ] from %s\n", from);
	  /* find the '!' in the 'from' field and strip it out.
	     from is on the form nick!user@host while IRC uses 
	     nick as recipients */
	  i = (char*)from; while(*i != '!' && *i != '\0') i++; *i = '\0';

	  sprintf(buf, "testIRC 0.1 based on %s %d.%d - by jorgen@wermland.se",
		  LIBIRC_NAME, LIBIRC_MAJOR, LIBIRC_MINOR);
	  ctcp_resp_version(srv, from, buf);
     } else {
	  fprintf(stderr, "[CTCP VERSION RESP] from %s - %s\n",
		  from, params);
     }
}

IRC_CALLBACK(privmsg) 
{
     fprintf(stderr, "<(%s)>/%s %s\n", prefix, params[0], params[1]);
}

IRC_CALLBACK(notice) 
{
     fprintf(stderr, "[NOTICE] <(%s)>/%s %s\n", prefix, params[0], params[1]);
}

IRC_CALLBACK(act_now) 
{
     irc_do_join(srv, "#karlstad", "");
     irc_do_privmsg(srv, "#karlstad", "Hello");
     printf("Joined channel #karlstad\n");
}

void poll_loop(srv_h srv)
{
     fd_set	set;
     SOCKET		maxfd;
     char	buf[100];
     srv_h	i;
     
     printf("Prompt> ");
     while(1) {
	  
	  maxfd = irc_getfdset(&set);
	  if(maxfd < 0) /* No more servers to keep track of... */
	       break;  
	  
	  FD_SET(0, &set);
	  while(1) {
	       if(select((int)maxfd + 1, &set, 0, 0, 0) < 0) {
		    if(errno != EINTR) {
			 perror("select");
			 return;
		    }
	       }
	       else 
		    break;
	  }
	  
	  if(FD_ISSET(0, &set)) {
	       irc_send_raw(srv, fgets(buf, sizeof(buf) / sizeof(buf[0]) - 1, stdin));
	       printf("Prompt> ");
	  }
	  
	  while((i = irc_nextsrv(&set))) {
	       if(irc_handle_server(i) < 0) {
		    irc_remove_server_listener(i);
		    irc_disconnect(i);
	       }
	  }
     }	
}

int main(void) 
{
    srv_h srv;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 0), &wsa);
#endif /* _WIN32 */

    trace_start(2); /* Trace to stderr */
    
    srv = irc_connect("irc.freenode.net", 6667);
		if (!srv)
		{
			printf("server object NILL");
			return 0;
		}
    irc_login(srv, "libIRCTEst", "libIRCTEst", "libIRCTEst");
    irc_add_server_listener(srv);
    irc_set_callback(ping, CMD_PING);
    irc_set_callback(privmsg, CMD_PRIVMSG);
    irc_set_callback(notice, CMD_NOTICE);
    irc_set_callback(act_now, RPL_ENDOFMOTD);
    ctcp_set_callback(action, CMD_CTCP_ACTION);
    ctcp_set_callback(version, CMD_CTCP_VERSION);
    poll_loop(srv);

    return 0;
}
