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

// implementation of main libIRC classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "TextUtils.h"

// event trigers from low level messages
// this way the low level events don't need the logic for the high level events.

void IRCClient::noticeMessage ( trMessageEventInfo	&info )
{
	if (info.params[1] == "Looking")
	{
		if (getConnectionState() < eTCPConenct)
			setConnectionState(eTCPConenct);

		callEventHandler(eIRCNoticeEvent,info);
	}
}

void IRCClient::welcomeMessage ( trMessageEventInfo	&info )
{
	setNick(info.target);
	requestedNick = info.target;

	// we know we are conencted here
	if (getConnectionState() < eLogedIn)
		setConnectionState(eLogedIn);

	callEventHandler(eIRCWelcomeEvent,info);
}

void IRCClient::endMOTD ( void )
{
	// we know we are conencted here
	if (getConnectionState() < eTCPConenct)
		setConnectionState(eTCPConenct);

	trBaseEventInfo	info;	// no info
	callEventHandler(eIRCEndMOTDEvent,info);
}	

void IRCClient::joinMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	trJoinEventInfo	joinInfo;
	if (who == getNick())	// we joined a channel
	{	
		IRCChannel	channel;
		channel.setName(info.target);
		channels[channel.getName()] = channel;

		joinInfo.eventType = eIRCChannelJoinEvent;
	}
	else	// someone else joined a channel we are in
	{
		trIRCUser	&user = getUserRecord(getCleanNick(who));

		channels[info.target].join(&user,parseNickMode(who));
		joinInfo.eventType = eIRCUserJoinEvent;
	}	

	joinInfo.channel = info.target;
	joinInfo.user = who;
	callEventHandler(joinInfo.eventType,joinInfo);
}

void IRCClient::partMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	trPartEventInfo	partInfo;
	if (who == getNick())	// we parted a channel
	{	
		tmChannelMap::iterator itr = channels.find(info.target);
		if ( itr == channels.end())
		{
			// we should not hit here
			// this is when we get a part for a channel we have not goten a join for
			return;
		}

		partInfo.eventType = eIRCChannelPartEvent;
		partInfo.channel = info.target;
		partInfo.user = who;
		callEventHandler(partInfo.eventType,partInfo);

		channels.erase(itr);
	}
	else	// someone else joined a channel we are in
	{
		trIRCUser	&user = getUserRecord(getCleanNick(who));

		channels[info.target].part(&user);
		partInfo.eventType = eIRCUserPartEvent;
		partInfo.channel = info.target;
		partInfo.user = who;
		callEventHandler(partInfo.eventType,partInfo);
	}	
}

void IRCClient::setChannelMode ( std::string channel, std::string mode )
{
	channels[channel].setMode(mode);

	trModeEventInfo	info;
	info.eventType = eIRCChannelModeSet;
	info.target = channel;
	info.from = reportedServerHost;
	info.mode = mode;
	callEventHandler(info.eventType,info);
}

void IRCClient::setChannelTopicMessage ( std::string channel, std::string topic, std::string source )
{
	channels[channel].setTopic(topic);

	trMessageEventInfo	info;
	info.eventType = eIRCTopicChangeEvent;
	info.target = channel;
	info.source = source;
	info.message = topic;
	callEventHandler(info.eventType,info);
}

void IRCClient::modeCommand ( BaseIRCCommandInfo	&info )
{
	std::string who = info.target;
	trModeEventInfo	modeInfo;
	modeInfo.target = who;

	// figure out who the message is from, is it form a channel or from a dude
	if (who[0] == '#' )
	{

		tmChannelMap::iterator itr = channels.find(who);

		modeInfo.from = info.source;
		modeInfo.mode = info.params[0];

		if ( itr == channels.end())
		{
			// we should not hit here
			// this is when we get a mode for a channel we have not goten a join for
			return;
		}

		if (info.params.size() > 1)	// if there is mor then one param then it's a mode change for a user
		{
			modeInfo.eventType = eIRCChannelUserModeSet;
			std::string user = getCleanNick(info.params[1]);
			modeInfo.message = info.getAsString(2);

			trIRCUser	&ircUser = getUserRecord(user);
			itr->second.setUserMode(&ircUser,modeInfo.mode,modeInfo.from);
		}
		else
		{	
			modeInfo.eventType = eIRCChannelModeSet;
			itr->second.setMode(modeInfo.mode);
		}		
	}
	else	// it's amode for a user ( like US )
	{
		modeInfo.eventType = eIRCUserModeSet;
	}

	callEventHandler(modeInfo.eventType,modeInfo);
}

void IRCClient::addChannelUsers ( std::string channel, string_list newUsers )
{
	string_list::iterator	itr = newUsers.begin();
	while ( itr != newUsers.end() )
	{
		trIRCUser	&user = getUserRecord(getCleanNick(*itr));

		channels[channel].join(&user,parseNickMode(*itr));
		itr++;
	}
}

bool IRCClient::removeChannelUser ( std::string channel, std::string name )
{
	if (name == getNick())
		return false;

	trIRCUser	&user = getUserRecord(getCleanNick(name));

	channels[channel].part(&user);
	return true;
}

void IRCClient::endChannelUsersList ( std::string channel )
{
	trBaseEventInfo	info;
	info.eventType = eIRCChanInfoCompleteEvent;
	callEventHandler(info.eventType,info);
}

void IRCClient::privMessage ( BaseIRCCommandInfo	&info )
{
	trMessageEventInfo	msgInfo;
	msgInfo.source = info.source;
	msgInfo.target = info.target;
	msgInfo.message = info.getAsString();
	msgInfo.params = info.params;
	msgInfo.from = string_util::tokenize(msgInfo.source,std::string("!"))[0];

	// lop off the ':'
	msgInfo.message.erase(msgInfo.message.begin());
	msgInfo.params[0].erase(msgInfo.params[0].begin());

	// figure out who the message is from, is it form a channel or from a dude
	if (info.target.c_str()[0] == '#' )
		msgInfo.eventType = eIRCChannelMessageEvent;
	else
		msgInfo.eventType = eIRCPrivateMessageEvent;

	callEventHandler(msgInfo.eventType,msgInfo);
}

void IRCClient::nickNameError ( int error, std::string message )
{
	trNickErrorEventInfo	info;
	info.error = error;

	if (getConnectionState() < eLogedIn)
		setConnectionState(eTCPConenct);

	info.error = error;
	info.message = message;
	info.eventType = eIRCNickNameError;
	callEventHandler(info.eventType,info);
}
