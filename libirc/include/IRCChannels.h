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

// channel structs and APIs

#ifndef __IRC_CHANNELS_H__
#define __IRC_CHANNELS_H__

#include <string>
#include <vector>
#include <map>

#include "TextUtils.h"

typedef struct 
{
	std::string mode;
	bool				ircOp;
	bool				identified;
	bool				invisible;
	bool				wallops;
	bool				inviteable;
	bool				messageable;
	bool				ctcpReceipt;
	bool				away;
	bool				idle;
}trIRCUserPermisions;

typedef struct 
{
	std::string mode;
	bool				chanOp;
	bool				voice;
	bool				quieted;
}trIRCChannelUserPermisions;

typedef std::map<std::string,trIRCChannelUserPermisions>	tmIRCChannelUserPermisionsMap;

typedef struct 
{
	std::string nick;
	std::string host;
	std::string lastMessage;
	trIRCUserPermisions	perms;
	tmIRCChannelUserPermisionsMap	channels;
}trIRCUser;

typedef std::map<std::string, trIRCUser> tvIRCUserMap;
typedef std::vector<trIRCUser> tvIRCUserList;
typedef std::vector<trIRCUser*> tvIRCUserRefList;

typedef struct 
{
	std::string mode;
	bool				allowColors;
	bool				forwarded;
	bool				inviteOnly;
	bool				anyInvite;
	bool				juped;
	int					userLimit;
	bool				moderated;
	bool				externalMessages;
	bool				permanent;
	bool				regForVoice;
	bool				secret;
	bool				reducedModeraton;
}trIRCChannelPermisions;

class IRCChannel
{
public:
	IRCChannel();
	~IRCChannel();

	std::string getName ( void );
	std::string getTopic ( void );

	std::vector<std::string>	listUsers ( void );
	trIRCUser&	getUserInfo ( std::string nick );
	trIRCChannelUserPermisions& getUserChanPerms ( std::string nick );

// state maintence
	void setName ( std::string text );
	void setTopic ( std::string text );

	void join ( trIRCUser *user );
	void part ( trIRCUser *user );
	void kick ( trIRCUser *user );

protected:
	trIRCChannelPermisions	perms;

	tvIRCUserRefList				users;
	std::string							name;
	std::string							topic;
};

typedef std::vector<IRCChannel>	tvChannelList; 
typedef std::map<std::string,IRCChannel>	tmChannelMap; 

#endif // __IRC_CHANNELS_H__ 
