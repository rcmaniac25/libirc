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
#include <stdarg.h>
#include <stdio.h>
#include "irc_core.h"
#include "macros_priv.h"
#include "cmd.h"
#include "defs.h"

static char buf[DEFAULT_BUF_SIZE]; /* Local 'utility buffer' */

/* Local functions */
static char* compose( char* fmt, ... )
{
    va_list ap;
    
    va_start(ap, fmt);
    vsnprintf(buf, DEFAULT_BUF_SIZE, fmt, ap);
    va_end(ap);
    return buf;
}

/* Exported functions */
void irc_do_privmsg( srv_h srv, 
		     const char* to,
		     const char* msg )
{
    irc_send_cmd(srv, CMD_PRIVMSG, compose("%s :%s", to, msg));
}

void irc_do_notice( srv_h srv, 
		     const char* to,
		     const char* msg )
{
    irc_send_cmd(srv, CMD_NOTICE, compose("%s :%s", to, msg));
}

void irc_do_join( srv_h srv,
		  const char* channel,
		  const char* key )
{
    irc_send_cmd(srv, CMD_JOIN, compose("%s %s", channel, key));
}

void irc_do_part( srv_h srv,
		  const char* channel )
{
    irc_send_cmd(srv, CMD_PART, channel);
}

void irc_do_pong( srv_h srv,
		  const char* arg )
{
    irc_send_cmd(srv, CMD_PONG, compose(":%s", arg));
}

void irc_do_ping( srv_h srv,
		  const char* arg )
{
    irc_send_cmd(srv, CMD_PING, compose(":%s", arg));
}

void irc_do_chan_mode(	srv_h srv,
			const char* channel,
			const char* mode,
			const char* limit,
			const char* user,
			const char* banmask	)
{
    irc_send_cmd(srv, CMD_MODE, compose("%s %s %s %s %s",
					 channel, mode,
					 limit, user, banmask));
}

void irc_do_user_mode(	srv_h srv,
			const char* user,
			const char* mode		)
{
    irc_send_cmd(srv, CMD_MODE, compose("%s %s", user, mode));
}

void irc_do_topic(	srv_h srv,
			const char* channel,
			const char* topic		)
{
    irc_send_cmd(srv, CMD_TOPIC, compose("%s :%s", channel, topic));
}

void irc_do_names(	srv_h srv,
			const char* channellist	)
{
    irc_send_cmd(srv, CMD_NAMES, compose("%s", channellist));
}

void irc_do_list(	srv_h srv,
			const char* channellist,
			const char* server		)
{
    irc_send_cmd(srv, CMD_LIST, compose("%s %s", channellist, server));
}

void irc_do_invite(	srv_h srv,
			const char* nick,
			const char* channel	)
{
    irc_send_cmd(srv, CMD_INVITE, compose("%s %s", nick, channel));
}

void irc_do_kick(	srv_h srv,
			const char* channel,
			const char* nick,
			const char* reason		)
{
    irc_send_cmd(srv, CMD_KICK, compose("%s %s :%s", 
					 channel, nick,
					 reason));
}


void irc_do_version(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_VERSION, server);
}

void irc_do_stats(	srv_h srv,
			const char* query,
			const char* server		)
{
    irc_send_cmd(srv, CMD_STATS, compose("%s %s", query, server));
}

void irc_do_links(	srv_h srv,
			const char* remote_server,
			const char* server_mask	)
{
    irc_send_cmd(srv, CMD_LINKS, compose("%s %s", remote_server,
					  server_mask));
}

void irc_do_time(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_TIME, server);
}

void irc_do_connect(	srv_h srv,
			const char* targ_server,
			const char* port,
			const char* remote_server	)
{
    irc_send_cmd(srv, CMD_CONNECT, compose("%s %s %s",
					    targ_server, port,
					    remote_server));
}

void irc_do_trace(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_TRACE, server);
}

void irc_do_admin(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_ADMIN, server);
}

void irc_do_info(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_INFO, server);
}

void irc_do_who(	srv_h srv,
			const char* name,
			int opersOnly	)
{
    irc_send_cmd(srv, CMD_WHO, compose("%s %s", name, opersOnly ? "o" : ""));
}

void irc_do_whois(	srv_h srv,
			const char* server,
			const char* nickmasklist	)
{
    irc_send_cmd(srv, CMD_WHOIS, compose("%s %s", server, nickmasklist));
}

void irc_do_whowas(	srv_h srv,
			const char* nick,
			const char* count,
			const char* server		)
{
    irc_send_cmd(srv, CMD_WHOWAS, compose("%s %s %s", 
					   nick, count, server));
}

void irc_do_quit(	srv_h srv,
			const char* reason		)

{
    irc_send_cmd(srv, CMD_QUIT, compose(":%s", reason));
}

void irc_do_oper(	srv_h srv,
			const char* user,
			const char* password	)
{
    irc_send_cmd(srv, CMD_OPER, compose("%s %s", user, password));
}

void irc_do_kill(	srv_h srv,
			const char* nick,
			const char* reason		)
{
    irc_send_cmd(srv, CMD_KILL, compose("%s :%s", nick, reason));
}

void irc_do_away(	srv_h srv,
			const char* message	)
{
    irc_send_cmd(srv, CMD_AWAY, compose(":%s", message));
}

void irc_do_rehash(	srv_h srv		)
{
    irc_send_cmd(srv, CMD_REHASH, "");
}

void irc_do_restart(	srv_h srv		)
{
    irc_send_cmd(srv, CMD_RESTART, "");
}

void irc_do_summon(	srv_h srv, 
			const char* user,
			const char* server		)
{
    irc_send_cmd(srv, CMD_SUMMON, compose("%s %s", user, server));
}

void irc_do_users(	srv_h srv,
			const char* server		)
{
    irc_send_cmd(srv, CMD_USERS, server);
}

void irc_do_wallops(	srv_h srv,
			const char* message	)
{
    irc_send_cmd(srv, CMD_WALLOPS, compose(":%s", message));
}

void irc_do_userhost(	srv_h srv,
			const char* nicklist	)
{
    irc_send_cmd(srv, CMD_USERHOST, nicklist);
}

void irc_do_ison(	srv_h srv,
			const char* nicklist	)
{
    irc_send_cmd(srv, CMD_ISON, nicklist);
}
