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

bool IRCClient::login ( std::string &nick, std::string &username, std::string &fullname, std::string &host )
{
	if (!tcpClient || !tcpClient->connected())
		return false;

	char	someNumber[64];
	sprintf(someNumber,"%d",rand());

	if (!nick.size())
		nick = std::string("SomeLazyUser") + std::string(someNumber);

	if (!username.size())
		username = "libIRCUser";

	if (!fullname.size())
		fullname = "Lazy libIRC programer";

	if (!host.size())
		fullname = "localhost";

	requestedNick = nick;

	IRCCommandINfo	info;
	info.params.push_back(nick);

	if (!sendIRCCommand(eCMD_NICK,info))
	{
		log("Login Failed: NICK command not sent",0);
		return false;
	}

	info.params.clear();
	info.params.push_back(username);
	info.params.push_back(host);
	info.params.push_back(ircServerName);
	info.params.push_back(fullname);

	if (!sendIRCCommand(eCMD_USER,info))
	{
		log("Login Failed: USER command not sent",0);
		return false;
	}

	if (getConnectionState() < eSentNickAndUSer)
		setConnectionState(eSentNickAndUSer);

	return  true;
}

bool IRCClient::join ( std::string channel )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	if (!sendIRCCommand(eCMD_JOIN,info))
	{
		log("Join Failed: JOIN command not sent",0);
		return false;
	}

	if (!sendIRCCommand(eCMD_MODE,info))
	{
		log("Join Failed: MODE command not sent",0);
		return false;
	}

	return true;
}

bool IRCClient::part ( std::string channel, std::string reason )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	info.params.push_back(reason);

	if (!sendIRCCommand(eCMD_PART,info))
	{
		log("part Failed: PART command not sent",0);
		return false;
	}

	// notify that we parted the channel
	userManager.userPartChannel(getNick(),channel);

	trPartEventInfo	eventInfo;

	eventInfo.eventType = eIRCChannelPartEvent;
	eventInfo.reason = reason;
	eventInfo.user = getNick();

	callEventHandler(eventInfo.eventType,eventInfo);

	// todo, we realy should go and remove the channel from our listing and kill any dead users
	return true;
}

bool IRCClient::sendMessage ( std::string target, std::string message, bool isAction )
{
	std::string messageHeader;
	std::string messageFooter;

	int sliceBoundry = 400;

	int headerLen = (int)strlen("PRIVMSG  :") + (int)target.size();
	if(isAction)
		headerLen += (int)strlen("*ACTION **");

	if(isAction)
		messageHeader += (char)0x01 + std::string("ACTION ");

	if(isAction)
		messageFooter +=(char)0x01;

	string_list	messages = string_util::slice(message,sliceBoundry-headerLen,true);

	string_list::iterator	itr = messages.begin();
	while ( itr != messages.end() )
	{
		std::string message = messageHeader+*itr+messageFooter;
		int len = (int)message.size();

		IRCCommandINfo	commandInfo;
		commandInfo.target = target;
		commandInfo.params.clear();
		commandInfo.params.push_back(message);
		sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		itr++;
	}
	return true;
}

bool IRCClient::kick ( std::string user, std::string channel, std::string reason )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	if (!userManager.userInChannel(user,channel))
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	info.params.push_back(user);
	info.params.push_back(reason);

	if (!sendIRCCommand(eCMD_KICK,info))
	{
		log("Kick Failed: KICK command not sent",0);
		return false;
	}

	return true;
}
