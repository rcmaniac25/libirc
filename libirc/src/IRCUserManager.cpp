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

IRCUserManager::IRCUserManager()
{
	//std::map<int,trIRCUserRecord>	users;
	//std::map<int,trIRChannelRecord> channels;

	autoPurgeOnLastPart = true;
	lastUserID = 0;
	lastChannelID = 0;
}

IRCUserManager::~IRCUserManager()
{

}

// user info API
int IRCUserManager::getUserID ( std::string &name )
{

}

std::string IRCUserManager::getUserNick ( int id )
{

}

std::string IRCUserManager::getUserHost ( int id );
std::string IRCUserManager::getUserHost ( std::string &name );
void IRCUserManager::setUserHost ( int id, std::string &host );
void IRCUserManager::setUserHost ( std::string &name, std::string &host );

std::string IRCUserManager::getUserLastMessage ( int id );
std::string IRCUserManager::getUserLastMessage ( std::string &name );

int IRCUserManager::getUserLastMessageChannel ( int id );
int IRCUserManager::getUserLastMessageChannel ( std::string &name );
std::string IRCUserManager::getUserLastMessageChannelName ( int id );
std::string IRCUserManager::getUserLastMessageChannelName ( std::string &name );

trIRCUserPermisions IRCUserManager::getUserPerms ( int id );
trIRCUserPermisions IRCUserManager::getUserPerms ( std::string &name );

std::vector<int> IRCUserManager::listUsers ( void );
std::vector<std::string> IRCUserManager::listUserNames ( void );

// user in channel API
bool IRCUserManager::userHasChannels ( int id );
bool IRCUserManager::userHasChannels ( std::string &name );

bool IRCUserManager::userInChannel ( int id, int channel );
bool IRCUserManager::userInChannel ( int id, std::string& channel );
bool IRCUserManager::userInChannel ( std::string &name, int channel );
bool IRCUserManager::userInChannel ( std::string &name, std::string& channel );

trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( int id, int channel );
trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( int id, std::string& channel );
trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( std::string &name, int channel );
trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( std::string &name, std::string& channel );

bool IRCUserManager::userIsIdentified ( int id );
bool IRCUserManager::userIsIdentified ( std::string &name );

bool IRCUserManager::userIsOp ( int id, int channel );
bool IRCUserManager::userIsOp ( int id, std::string& channel );
bool IRCUserManager::userIsOp ( std::string &name, int channel );
bool IRCUserManager::userIsOp ( std::string &name, std::string& channel );

bool IRCUserManager::userHasVoice ( int id, int channel );
bool IRCUserManager::userHasVoice ( int id, std::string& channel );
bool IRCUserManager::userHasVoice ( std::string &name, int channel );
bool IRCUserManager::userHasVoice ( std::string &name, std::string& channel );

std::vector<int> IRCUserManager::listUserChannels ( int id );
std::vector<int> IRCUserManager::listUserChannels ( std::string &name );
std::vector<std::string> IRCUserManager::listUserChannelNames ( int id );
std::vector<std::string> IRCUserManager::listUserChannelNames ( std::string &name );

// channel API
int IRCUserManager::getChannelID ( std::string &channel );
std::string IRCUserManager::getChannelName ( int id );

trIRCChannelPermisions IRCUserManager::getChannelPerms ( int id );
trIRCChannelPermisions IRCUserManager::getChannelPerms ( std::string &channel );

std::string IRCUserManager::getChannelLastMessage ( int id );
std::string IRCUserManager::getChannelLastMessage ( std::string &channel );

int IRCUserManager::getChannelLastMessageUser ( int id );
int IRCUserManager::getChannelLastMessageUser ( std::string &channel );
std::string IRCUserManager::getChannelLastMessageUserName ( int id );
std::string IRCUserManager::getChannelLastMessageUserName ( std::string &channel );

std::vector<int> IRCUserManager::listChannelUsers ( int id );
std::vector<int> IRCUserManager::listChannelsUser ( std::string &name );
std::vector<std::string> IRCUserManager::listChanneUserlNames ( int id );
std::vector<std::string> IRCUserManager::listChannelUserNames ( std::string &name );


// state update from the IRC data stream
void IRCUserManager::userJoinChannel ( int user,  int channel );
void IRCUserManager::userJoinChannel ( int user, std::string &channel );
void IRCUserManager::userJoinChannel ( std::string &user, int channel );
void IRCUserManager::userJoinChannel ( std::string &user, std::string &channel );

void IRCUserManager::userJoinChannel ( int user,  int channel );
void IRCUserManager::userPartChannel ( int user, std::string &channel );
void IRCUserManager::userPartChannel ( std::string &user, int channel );
void IRCUserManager::userPartChannel ( std::string &user, std::string &channel );

void IRCUserManager::nickChange ( std::string &oldNick, std::string &newNick );

void IRCUserManager::messageReceved ( std::string &target, std::string &source, std::string &message );

void IRCUserManager::modeReceved ( std::string &target, std::string &source, std::string &mode );


// utilitys
void IRCUserManager::purgeNonChannelUsers ( void )
{
	std::map<int,trIRCUserRecord>::iterator userItr = users.begin();
	while (userItr != users.end())
	{
		(userItr++)->second.lastMessage = "";

}

void IRCUserManager::purgeLastMessages ( void )
{
	std::map<int,trIRCUserRecord>::iterator userItr = users.begin();
	while (userItr != users.end())
		(userItr++)->second.lastMessage = "";

	std::map<int,trIRCChannelRecord>::iterator chanItr = channels.begin();
	while (chanItr != channels.end())
		(chanItr++)->second.lastMessage = "";
}

trIRCUserRecord& IRCUserManager::getUserInfo ( int id )
{
	return users[id];
}

trIRCUserRecord& IRCUserManager::getUserInfo ( std::string &name )
{
	std::map<std::string,int>::iterator itr = userNameLookup.find(getCleanNick(name));

	if ( itr == users.end())
	{
		trIRCUserRecord	user;
		user.nick = getCleanNick(name);
		users[lastUserID] = user;
		userNameLookup[user.nick] = lastUserID;
		return getUserInfo(lastUserID++);
	}
	return getUserInfo(itr->second);
}

trIRCChannelRecord& IRCUserManager::getChannelInfo ( int id )
{
	return channels[id];
}

trIRCChannelRecord& IRCUserManager::getChannelInfo ( std::string &channel )
{
	std::map<std::string,int>::iterator itr = channelNameLookup.find(name);

	if ( itr == users.end())
	{
		trIRCChannelRecord	user;
		user.nick = name;
		users[lastUserID] = user;
		return getChannelInfo(lastUserID++);
	}
	return getChannelInfo(itr->second);
}

std::string IRCUserManager::getCleanNick ( std::string &nick )
{
	if (nick.size() < 2)
		return nick;

	if (nick[0] == '@' || nick[0] == '+')
	{
		std::string	temp = nick;
		temp.erase(temp.begin());
		return temp;
	}
	return nick;
}
