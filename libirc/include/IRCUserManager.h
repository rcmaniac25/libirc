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

// user manager

#ifndef _IRC_USER_MANAGER_
#define _IRC_USER_MANAGER_

#include "TextUtils.h"
#include <vector>
#include <string>
#include <map>

class IRCChannelManager;

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

typedef struct 
{
	int									id;
	std::string					name;
	trIRCUserPermisions	perms;
}trIRCUserChannelRecord;

typedef std::vector<trIRCUserChannelRecord> tvIRCUserChannelList;

typedef struct 
{
	int											id;
	std::string							nick;
	std::string							host;

	int											lastMessageChannel;
	std::string							lastMessage;

	trIRCUserPermisions			perms;
	tvIRCUserChannelList		channels;

	string_list getChannels ( void );
}trIRCUserRecord;

typedef std::map<std::string,int> tmIRCUserIDNameMap;
typedef std::map<int,trIRCUserRecord> tmIRCUserIDMap;

class IRCUserManager
{
public:
		IRCUserManager();
		~IRCUserManager();

		void setChannelManager ( IRCChannelManager *chanMan ){channelManager = chanMan;}

		int getUserID ( std::string &name );
		trIRCUserRecord& getUserInfo ( int id );
		trIRCUserRecord& getUserInfo ( std::string &name );

		void userJoinChannel ( std::string &user, std::string &channel );
		void userPartChannel ( std::string &user, std::string &channel );

protected:
	IRCChannelManager		*channelManager;
	tmIRCUserIDMap			users;
	tmIRCUserIDNameMap	usernames;
	unsigned int				lastID;
};

#endif//_IRC_USER_MANAGER_