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
#ifndef __IRC_CORE_H__
#define __IRC_CORE_H__

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#endif /* !defined(_WIN32) */

#include "defs.h"
#include "bufsock.h"

/* Macros and constants */
#define IRC_CALLBACK(x) void x( srv_h srv,		\
                            const char* prefix,	\
                            const char* cmd,	\
                            char** params,	\
			    unsigned int numparams)
#define ERRORHANDLER(x) void x( srv_h srv,		\
                                int errcode,   )

typedef struct _srv* srv_h;
/* Types */
typedef void (*callback_t)(srv_h, const char*, const char*, char**, unsigned int);
typedef void (*errhandler_t)(srv_h, int);

#ifdef __cplusplus
extern "C" {
#endif 

/* Structures */
struct _srv
{
    buf_sock_t	sock;
    char*		host;
    unsigned short		port;
    void* 	data;
};

/* Function prototypes - low level functions */
srv_h irc_connect(	const char* server, 
			unsigned short port		);
int  irc_login(		srv_h	 srv,	
			const char* nick, 
			const char* user, 
			const char* fullname	);
void irc_disconnect(	srv_h server		);
void irc_set_data(	srv_h server,
			void* data 		);
void* irc_get_data(	srv_h server		);
void irc_set_callback(	callback_t cb, 
			const char* cmd		);
void irc_set_fallback_callback(callback_t cb	);
void irc_set_errhandler(errhandler_t errh	);
void irc_main(		void			);
SOCKET irc_getfdset(	fd_set* set		);
int irc_handle_server(	srv_h server		);
srv_h irc_nextsrv(	fd_set* set		);
int  irc_send_raw(	srv_h server, 
			const char* rawtext	);
int  irc_send_cmd(	srv_h server, 
			const char* cmd, 
			const char* params	);
void irc_add_server_listener(srv_h server	);
void irc_remove_server_listener(srv_h server	);

/* Function prototypes - high level IRC functions */
/* Before you start to use these functions, look them up
   in the RFC 1459 for more information about their parameters
   and such. 
   
   irc_do_XXXX corresponds to message XXXX.
   
*/
/* Messaging operations */
void irc_do_privmsg(	srv_h srv, 
			const char* to, 
			const char* msg		);
void irc_do_notice(	srv_h srv, 
			const char* to, 
			const char* msg		);
/* Channel operations */
void irc_do_join(	srv_h srv, 
			const char* channel,
			const char* key		);
void irc_do_part(	srv_h srv, 
			const char* channel	);
void irc_do_chan_mode(	srv_h srv,
			const char* channel,
			const char* mode,
			const char* limit,
			const char* user,
			const char* banmask	);
void irc_do_user_mode(	srv_h srv,
			const char* user,
			const char* mode		);
void irc_do_topic(	srv_h srv,
			const char* channel,
			const char* topic		);
void irc_do_names(	srv_h srv,
			const char* channellist	);

void irc_do_list(	srv_h srv,
			const char* channellist,
			const char* server		);
void irc_do_invite(	srv_h srv,
			const char* nick,
			const char* channel	);
void irc_do_kick(	srv_h srv,
			const char* channel,
			const char* nick,
			const char* reason		);
/* Server operations */
void irc_do_version(	srv_h srv,
			const char* server		);
void irc_do_stats(	srv_h srv,
			const char* query,
			const char* server		);
void irc_do_links(	srv_h srv,
			const char* remote_server,
			const char* server_mask	);
void irc_do_time(	srv_h srv,
			const char* server		);
void irc_do_connect(	srv_h srv,
			const char* targ_server,
			const char* port,
			const char* remote_server	);
void irc_do_trace(	srv_h srv,
			const char* server		);
void irc_do_admin(	srv_h srv,
			const char* server		);
void irc_do_info(	srv_h srv,
			const char* server		);

/* User based queries */
void irc_do_who(	srv_h srv,
			const char* name,
			int opersOnly	);
void irc_do_whois(	srv_h srv,
			const char* server,
			const char* nickmasklist	);
void irc_do_whowas(	srv_h srv,
			const char* nick,
			const char* count,
			const char* server		);
/* Misc operations */
void irc_do_ping(	srv_h srv,
			const char* arg		);
void irc_do_pong(	srv_h srv,
			const char* arg		);
void irc_do_quit(	srv_h srv,
			const char* reason		);
void irc_do_oper(	srv_h srv,
			const char* user,
			const char* password	);
void irc_do_kill(	srv_h srv,
			const char* nick,
			const char* reason		);

/* Optional operations */
void irc_do_away(	srv_h srv,
			const char* message	);
void irc_do_rehash(	srv_h srv		);
void irc_do_restart(	srv_h srv		);
void irc_do_summon(	srv_h srv, 
			const char* user,
			const char* server		);
void irc_do_users(	srv_h srv,
			const char* server		);
void irc_do_wallops(	srv_h srv,
			const char* message	);
void irc_do_userhost(	srv_h srv,
			const char* nicklist	);
void irc_do_ison(	srv_h srv,
			const char* nicklist	);

#ifdef __cplusplus
}
#endif __cplusplus

#endif /* __IRC_CORE_H__ */
