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

// stupidBot

#include <stdio.h>
#include <stdlib.h>

#include "libIRC.h"
#include "TextUtils.h"

typedef struct 
{
	std::string server;
	int					port;
	string_list	nicks;
	int					nick;
	std::string host;
	std::string realName;
	string_list channels;
}trStupidBotInfo;

trStupidBotInfo	theBotInfo;

IRCClient	client;

void login ( void )
{
	client.login(theBotInfo.nicks[theBotInfo.nick],theBotInfo.host,theBotInfo.realName);
}

void joinChannels ( void )
{
	string_list::iterator itr = theBotInfo.channels.begin();

	while ( itr != theBotInfo.channels.end() )
	{
		client.join(*itr);
		itr++;
	}
}

class myEventCaller : public IRCBasicEventCallback
{
public:
	bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );
};

bool myEventCaller::process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info )
{
	switch (eventType)
	{
		case eIRCNoticeEvent:
			login();
			break;

		case eIRCEndMOTDEvent:
			joinChannels();
			break;
	}
	return true;
}


myEventCaller	eventHandaler;


void registerEventHandalers ( void )
{
	client.registerEventHandaler(eIRCNoticeEvent,&eventHandaler);
	client.registerEventHandaler(eIRCEndMOTDEvent,&eventHandaler);
}

void initInfo ( void )
{
	theBotInfo.nick = 0;

	theBotInfo.server = "irc.freenode.net";
	theBotInfo.port = 6667;

	theBotInfo.nicks.push_back(std::string("boboBot"));
	theBotInfo.nicks.push_back(std::string("boboBot1"));
	theBotInfo.nicks.push_back(std::string("boboBot2"));

	theBotInfo.host = "stupidBot";
	theBotInfo.realName = "a libIRC bot";

	theBotInfo.channels.push_back(std::string("#opencombat"));
	theBotInfo.channels.push_back(std::string("#brlcad"));
	theBotInfo.channels.push_back(std::string("#bzflag"));
}

void main ( void )
{
	initInfo();

	client.setDebugLevel(5);

	// clear the log
	fclose(fopen("irc.log","wt"));

	// set the log
	client.setLogfile("irc.log");
	registerEventHandalers();

	client.connect(theBotInfo.server,theBotInfo.port);

	while (client.process())
	{
		IRCOSSleep(1);
	}
};