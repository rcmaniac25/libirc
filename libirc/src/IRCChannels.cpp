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
	perms.mode = "NONE";
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

trIRCChannelPermisions&	IRCChannel::getChanPerms ( void )
{
	return perms;
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

void IRCChannel::setMode ( std::string mode )
{
	perms.mode = mergeModes(perms.mode,mode);
	
	// parse the mode
	perms.externalMessages = string_util::charExists(mode,'n');
	perms.allowColors = string_util::charExists(mode,'c');
	perms.forwarded = string_util::charExists(mode,'f');
	perms.anyInvite = string_util::charExists(mode,'g');
	perms.inviteOnly = string_util::charExists(mode,'i');
	perms.juped = string_util::charExists(mode,'j');
	perms.moderated = string_util::charExists(mode,'m');
	perms.reducedModeraton = string_util::charExists(mode,'z');
	perms.regForVoice = string_util::charExists(mode,'R');
	perms.regOnly = string_util::charExists(mode,'r');
	perms.secret = string_util::charExists(mode,'s');
}

void IRCChannel::setUserMode ( trIRCUser &user, std::string mode, std::string from )

	user.channels[name].mode = mergeModes(user->channels[name].mode,mode);
	user.channels[name].chanOp = string_util::charExists(mode,'o');
	user.channels[name].voice = string_util::charExists(mode,'v');
	user.channels[name].quieted = string_util::charExists(mode,'q');
}

void IRCChannel::join ( trIRCUser &user, teNickModes mode )
{
	users.push_back(user);

	trIRCChannelUserPermisions	chanPerms;

	chanPerms.chanOp = mode == eOperator;
	chanPerms.quieted = false;
	chanPerms.voice = mode != eNoMode;

	user.channels[name] = chanPerms;
}

void IRCChannel::part ( trIRCUser &user )
{
	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( itr->nick == user.nick )
			itr = users.erase(itr);
		else
			itr++;
	}

	if (user.channels.find(name) != user.channels.end())
		user.channels.erase(user.channels.find(name));
}

void IRCChannel::kick ( trIRCUser &user )
{
	tvIRCUserRefList::iterator itr = users.begin();

	while ( itr != users.end() )
	{
		if ( itr->nick == user.nick )
			itr = users.erase(itr);
		else
			itr++;
	}

	if (user.channels.find(name) != user.channels.end())
		user.channels.erase(user.channels.find(name));
}

std::string IRCChannel::mergeModes ( std::string mode, std::string modMode )
{
	bool add = modMode[0] == '+';

	std::string newMode = mode;
	if (newMode == "NONE")
		newMode = "";

	std::string::iterator	modModeItr = modMode.begin();
	modModeItr++;

	while (modModeItr != modMode.end())
	{
		char item = *modModeItr;

		if (add)
		{
			if (!string_util::charExists(newMode,item))
				newMode += *modModeItr;
		}
		else
		{
			if (string_util::charExists(newMode,item))
			{
				string_util::eraseFirstOf(newMode,item);
			}
		}
		modModeItr++;
	}
	return newMode;
}

