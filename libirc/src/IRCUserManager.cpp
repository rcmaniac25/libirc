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
#include "IRCUserManager.h"

class IRCChannelManager;



string_list trIRCUserRecord::getChannels ( void )
{
	string_list	names;
	tvIRCUserChannelList::iterator itr = channels.begin();
	while ( itr != channels.end())
	{
		names.push_back(itr->name);
		itr++;
	}
	return names;
}

IRCUserManager::IRCUserManager()
{
	channelManager = NULL;
	//tmIRCUserIDMap			users;
	//tmIRCUserIDNameMap	usernames;
}

IRCUserManager::~IRCUserManager()
{

}

int IRCUserManager::getUserID ( std::string &name )
{
	tmIRCUserIDNameMap::iterator itr = usernames.find(name);
	if ( itr == usernames.end())
	{
		trIRCUserRecord	user
		return -1;
	}
}

trIRCUserRecord& IRCUserManager::getUserInfo ( int id )
{

}

trIRCUserRecord& IRCUserManager::getUserInfo ( std::string &name )
{

}

void IRCUserManager::userJoinChannel ( std::string &user, std::string &channel );
void IRCUserManager::userPartChannel ( std::string &user, std::string &channel );

