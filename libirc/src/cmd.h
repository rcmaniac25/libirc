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
#ifndef __CMD_H__
#define __CMD_H__

/* Commands used in the IRC protocol (RFC 1459) */
#define CMD_NICK	"NICK"
#define CMD_USER	"USER"
#define CMD_PING	"PING"
#define CMD_PONG	"PONG"
#define CMD_PRIVMSG	"PRIVMSG"
#define CMD_JOIN	"JOIN"
#define CMD_PART	"PART"
#define CMD_NOTICE	"NOTICE"
#define CMD_MODE	"MODE"
#define CMD_TOPIC	"TOPIC"
#define CMD_NAMES	"NAMES"
#define CMD_LIST	"LIST"
#define CMD_INVITE	"INVITE"
#define CMD_KICK	"KICK"
#define CMD_VERSION	"VERSION"
#define CMD_STATS	"STATS"
#define CMD_LINKS	"LINKS"
#define CMD_TIME	"TIME"
#define CMD_CONNECT	"CONNECT"
#define CMD_TRACE	"TRACE"
#define CMD_ADMIN	"ADMIN"
#define CMD_INFO	"INFO"
#define CMD_WHO		"WHO"
#define CMD_WHOIS	"WHOIS"
#define CMD_WHOWAS	"WHOWAS"
#define CMD_QUIT	"QUIT"
#define CMD_OPER	"OPER"
#define CMD_KILL	"KILL"
#define CMD_AWAY	"AWAY"
#define CMD_REHASH	"REHASH"
#define CMD_RESTART	"RESTART"
#define CMD_SUMMON	"SUMMON"
#define CMD_USERS	"USERS"
#define CMD_WALLOPS	"WALLOPS"
#define CMD_USERHOST	"USERHOST"
#define CMD_ISON	"ISON"

/* Commands used in the CTCP protocol(http://www.invlogic.com/irc/ctcp.html)*/
#define CMD_CTCP_ACTION		"ACTION"
#define CMD_CTCP_VERSION	"VERSION"
#define CMD_CTCP_PING		"PING"
#define CMD_CTCP_PONG		"PONG"
#define CMD_CTCP_CLIENTINFO	"CLIENTINFO"
#define CMD_CTCP_USERINFO	"USERINFO"
#define CMD_CTCP_TIME		"TIME"
#define CMD_CTCP_ERRMSG		"ERRMSG"

#endif /* __CMD_H__ */
