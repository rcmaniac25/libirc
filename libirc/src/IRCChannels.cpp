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

#include "IRCChannels.h"


IRCChannel::IRCChannel()
{
	//trIRCChannelPermisions	perms;

	//tvIRCUserRefList				users;
	//std::string							name;
//	std::string							topic;
	name = "UNKNOWN";
}

IRCChannel::~IRCChannel()
{

}

std::string IRCChannel::getName ( void )
{
	return name;
}

std::string IRCChannel::getTopic ( void )
{
	return topic;
}

string_list	IRCChannel::listUsers ( bool justOps )
{
	tvIRCUserRefList::iterator itr = users.begin();
	string_list usersList;

	while ( itr != users.end() )
	{
		if (!justOps || (justOps && (*itr)->channels[name].chanOp))
			usersList.push_back((*itr)->nick);
		itr++;
	}
	return usersList;
}

trIRCUser&	IRCChannel::getUserInfo ( std::string nick )
{
	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( (*itr)->nick == nick )
			return *(*itr);
		else
			itr++;
	}

	static trIRCUser	thing;
	return thing;
}

trIRCChannelUserPermisions& IRCChannel::getUserChanPerms ( std::string nick )
{
	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( (*itr)->nick == nick )
			return (*itr)->channels[name];
		else
			itr++;
	}
	static trIRCChannelUserPermisions	thing;
	return thing;
}

// state maintence
void IRCChannel::setName ( std::string text )
{
	name = text;
}

void IRCChannel::setTopic ( std::string text )
{
	topic = text;
}

void IRCChannel::join ( trIRCUser *user, bool op )
{
	if (!user)
		return;

	users.push_back(user);

	trIRCChannelUserPermisions	chanPerms;

	chanPerms.chanOp = op;
	chanPerms.quieted = false;
	chanPerms.voice = false;

	user->channels[name] = chanPerms;
}

void IRCChannel::part ( trIRCUser *user )
{
	if (!user)
		return;

	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( (*itr)->nick == user->nick )
			itr = users.erase(itr);
		else
			itr++;
	}

	if (user->channels.find(name) != user->channels.end())
		user->channels.erase(user->channels.find(name));
}

void IRCChannel::kick ( trIRCUser *user )
{
	if (!user)
		return;

	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( (*itr)->nick == user->nick )
			itr = users.erase(itr);
		else
			itr++;
	}

	if (user->channels.find(name) != user->channels.end())
		user->channels.erase(user->channels.find(name));
}


