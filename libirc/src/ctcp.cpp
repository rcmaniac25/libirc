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

#include "conf.h"

#if !defined(DONT_PROCESS_CTCP)
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

#include "irc_core.h"
#include "ctcp.h"
#include "macros_priv.h"
#include "cmd.h"
#include "map.h"
#include "util.h"
#include "defs.h"

/* Local variables */
static map_t callbacks = NULL;
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

static void
ctcp_send_raw( srv_h srv,
	       const char* to, 
	       int isRequest,
	       const char* cmd,
	       const char* args )
{
     char* ctcp_cmd = compose("%c%s %s%c", '\001', cmd, args, '\001');
     if(isRequest)
	  irc_do_privmsg(srv, to, ctcp_cmd);
     else
	  irc_do_notice(srv, to, ctcp_cmd);
}

/* Exported functions */
void
ctcp_send_error( srv_h srv,
		 const char* to, 
		 const char* cmd,
		 const char* arg )
{
     char* err_cmd = compose("%s %s", cmd, arg);
     ctcp_send_raw(srv, to, FALSE, CMD_CTCP_ERRMSG, err_cmd);
}

void
ctcp_set_callback( ctcp_callback_t	cb,
		   const char*		cmd )
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

void ctcp_dispatch_msg( srv_h srv,
			const char* to, 
			const char* from,
			int isRequest,
			char* line )
{
     char* i;
     char* cmd;
     char* param;
     ctcp_callback_t cb;

     /* If there are no callbacks, then processing the
	message is unnecessary */
     if(callbacks == NULL)
	  return;

     cmd = line + 1;
     for(i = cmd; *i != ' ' && *i != '\001'; i++);
     *i = '\0';
     i++;

     if(map_get(callbacks, cmd, (void**)&cb) == FALSE || cb == NULL)
	  return;

     param = i;
     /* Remove the last CTCP marker */
     while(*i != '\001' && *i != '\0') i++;
     *i = '\0';

     cb(srv, to, from, isRequest, cmd, param);
}

void ctcp_send_req( srv_h srv, 
		    const char* to,
		    const char* cmd,
		    const char* args )
{
     ctcp_send_raw(srv, to, TRUE, cmd, args);
}

void ctcp_send_resp( srv_h srv,
		     const char* to,
		     const char* cmd,
		     const char* args )
{
     ctcp_send_raw(srv, to, FALSE, cmd, args);
}

void ctcp_do_action( srv_h srv,
		     const char* to,
		     const char* action )
{
     ctcp_send_req(srv, to, CMD_CTCP_ACTION, action);
}

void ctcp_req_version( srv_h srv, 
		       const char* to )
{
     ctcp_send_req(srv, to, CMD_CTCP_VERSION, "");
}

void ctcp_req_ping( srv_h srv, 
		    const char* to,
		    const char* arg )
{
     ctcp_send_req(srv, to, CMD_CTCP_PING, arg);
}

void ctcp_req_cliinfo( srv_h srv, 
		       const char* to )
{
     ctcp_send_req(srv, to, CMD_CTCP_CLIENTINFO, "");
}

void ctcp_req_userinfo( srv_h srv, 
			const char* to )
{
     ctcp_send_req(srv, to, CMD_CTCP_USERINFO, "");
}

void ctcp_req_time( srv_h srv, 
		    const char* to )
{
     ctcp_send_req(srv, to, CMD_CTCP_TIME, "");
}

void ctcp_resp_version( srv_h srv, 
			const char* to,
			const char* version )
{
     ctcp_send_resp(srv, to, CMD_CTCP_VERSION, version);
}

void ctcp_resp_ping( srv_h srv, 
		     const char* to,
		     const char* arg )
{
     ctcp_send_resp(srv, to, CMD_CTCP_PONG, arg);
}

void ctcp_resp_cliinfo( srv_h srv, 
			const char* to,
			const char* info )
{
     ctcp_send_resp(srv, to, CMD_CTCP_CLIENTINFO, info);
}

void ctcp_resp_userinfo( srv_h srv, 
			 const char* to,
			 const char* info )
{
     ctcp_send_resp(srv, to, CMD_CTCP_USERINFO, info);
}

void ctcp_resp_time( srv_h srv, 
		     const char* to,
		     const char* time )
{
     ctcp_send_resp(srv, to, CMD_CTCP_TIME, time);
}

#endif /* !defined(DONT_PROCESS_CTCP) */
