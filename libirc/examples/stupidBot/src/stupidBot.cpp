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
bool part = false;


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

void joinedChannel ( trJoinEventInfo *info )
{
	IRCCommandINfo	commandInfo;

	commandInfo.target = info->channel;
	commandInfo.params.push_back (std::string("Go Go SupidBot"));
	client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
}

void channelMessage ( trMessageEventInfo *info )
{
	std::string myNick = client.getNick();

	IRCCommandINfo	commandInfo;
	commandInfo.target = info->target;

	std::string firstWord = info->params[0];
	if ( (*(firstWord.end()-1) == ':') || (*(firstWord.end()-1) == ',') )
		firstWord.erase(firstWord.end()-1);

	if ( firstWord == myNick )
	{
		// its for me
		if ( info->params[1] == "quit")
		{
			if (info->from == "Patlabor221")
				part = true;
		}
		else if ( info->params[1] == "Hello")
		{
			std::string message = std::string("Hey ") + info->from + std::string(" how are you?");
			client.sendMessage(info->target,message);
		}
		else if ( info->params[1] == "status")
		{
			commandInfo.params.push_back( std::string("online"));
			client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		}
		else if ( info->params[1] == "channel")
		{
			commandInfo.params.push_back(info->target);
			client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		}
		else if ( info->params[1] == "channels")
		{
			string_list	chans = client.listChanels();
			string_list::iterator itr = chans.begin();
			std::string plural = chans.size()>1 ? "s" : "";

			std::string theLine = info->from + std::string(" I am presently in") + string_util::format(" %d channel%s, including; ",chans.size(),plural.c_str());
			while ( itr != chans.end() )
			{
				theLine += *itr;
				itr++;
				if ( itr != chans.end() )
					theLine += ", ";
			}
			commandInfo.params.push_back(theLine);
			client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		}
		else if ( info->params[1] == "dance")
		{
				client.sendMessage(info->target,"waves it like it just don't care",true);
		}
		else
		{
			commandInfo.params.push_back( info->from + std::string(" WTF are you on about?"));
			client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);

		}
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

		case eIRCChannelJoinEvent:
			joinedChannel((trJoinEventInfo*)&info);
			break;

		case eIRCChannelMessageEvent:
			channelMessage ((trMessageEventInfo*)&info);
			break;

		case eIRCNickNameError:
			theBotInfo.nick++;
			if (theBotInfo.nick < (int)theBotInfo.nicks.size())
			{
				// try our next name
				client.login(theBotInfo.nicks[theBotInfo.nick],theBotInfo.host,theBotInfo.realName);
				return false;
			}
			else	// we are out of names, let the default try it
				return true;

			break;
	}
	return true;
}


myEventCaller	eventHandaler;


void registerEventHandalers ( void )
{
	client.registerEventHandaler(eIRCNoticeEvent,&eventHandaler);
	client.registerEventHandaler(eIRCEndMOTDEvent,&eventHandaler);
	client.registerEventHandaler(eIRCChannelJoinEvent,&eventHandaler);
	client.registerEventHandaler(eIRCChannelMessageEvent,&eventHandaler);
	client.registerEventHandaler(eIRCNickNameError,&eventHandaler);
}

void initInfo ( void )
{
	theBotInfo.nick = 0;

	theBotInfo.server = "irc.freenode.net";
	theBotInfo.port = 6667;

	theBotInfo.nicks.push_back(std::string("stupid_Bot"));
	theBotInfo.nicks.push_back(std::string("stupiderBot"));
	theBotInfo.nicks.push_back(std::string("stupidestBot"));

	theBotInfo.host = "stupidBot";
	theBotInfo.realName = "a libIRC bot";

	theBotInfo.channels.push_back(std::string("#bzflag"));
	theBotInfo.channels.push_back(std::string("#opencombat"));
	theBotInfo.channels.push_back(std::string("#brlcad"));
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

	while (client.process() && !part)
	{
		IRCOSSleep(1);
	}

	if (part)
	{
		client.disconnect();
	}
};