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
#ifndef __CTCP_H__
#define __CTCP_H__

#define CTCP_CALLBACK(x) void x( srv_h srv, \
				 const char* to, \
				 const char* from, \
				 int isRequest, \
				 const char* cmd, \
				 const char* params )

#define CTCP_MARKER	'\001'

typedef void (*ctcp_callback_t)(srv_h, const char*, const char*, int, const char*, const char*);

#ifdef __cplusplus
extern "C" {
#endif 

void ctcp_set_callback(	ctcp_callback_t	cb,
			const char*	cmd	);
void ctcp_dispatch_msg(	srv_h		srv,
			const char*	to, 
			const char*	from,
			int	isRequest,
			char*		line	);
void ctcp_send_req(	srv_h		srv,
			const char*	to, 
			const char*	cmd,
			const char*	args	);
void ctcp_send_resp(	srv_h		srv,
			const char*	to, 
			const char*	cmd,
			const char*	args	);
void ctcp_send_error(	srv_h		srv,
			const char*	to, 
			const char*	cmd,
			const char*	arg	);
void ctcp_do_action(	srv_h		srv, 
			const char*	to,
			const char*	action	);
void ctcp_req_version(	srv_h		srv,
			const char*	to	);
void ctcp_req_ping(	srv_h		srv,
			const char*	to,
			const char*	arg	);
void ctcp_req_cliinfo(	srv_h		srv,
			const char*	to	);
void ctcp_req_userinfo(	srv_h		srv,
			const char*	to	);
void ctcp_req_time(	srv_h		srv,
			const char*	to	);
void ctcp_resp_version(	srv_h		srv,
			const char*	to,
			const char*	version );
void ctcp_resp_ping(	srv_h		srv,
			const char*	to,
			const char*	arg	);
void ctcp_resp_cliinfo(	srv_h		srv,
			const char*	to,
			const char*	info	);
void ctcp_resp_userinfo(	srv_h		srv,
				const char*	to,
				const char*	info );
void ctcp_resp_time(	srv_h		srv,
			const char*	to,
			const char*	time	);

#ifdef __cplusplus
}
#endif 

#endif /* __CTCP_H__ */
