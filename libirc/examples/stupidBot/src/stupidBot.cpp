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

#include <string>
#include <map>

typedef struct 
{
	std::string name;
	std::string message;
	bool				action;
}trFactoid;
typedef std::map<std::string,trFactoid>	factoidMap;
typedef std::map<std::string,std::string> string_map;
typedef struct 
{
	std::string server;
	int					port;
	string_list	nicks;
	int					nick;
	std::string host;
	std::string realName;
	string_list channels;
	string_list	masters;
	string_list unknownResponces;
	string_list	partMessages;
	string_list	quitMessages;
	factoidMap	factoids;
	string_map	joinMessages;
	std::string config;

}trStupidBotInfo;

trStupidBotInfo	theBotInfo;

IRCClient	client;
bool part = false;

void readConfig ( std::string file )
{
	theBotInfo.config = file;
	std::string config;

	// read the file data
	FILE *fp = fopen(file.c_str(),"rt");
	if (!fp)
		return;

	fseek(fp,SEEK_END,0);
	int len = ftell(fp);
	fseek(fp,SEEK_SET,0);

	char *data = (char*)malloc(len+1);
	fread(data,len,1,fp);
	fclose(fp);
	data[len] = 0;
	config = data;
	free(data);

	std::string lineEnd;
#ifdef _WIN32
	lineEnd = "/r/n";
#else
	lineEnd = "/n";
#endif

	string_list lines = string_util::tokenize(config,lineEnd);

	string_list::iterator itr = lines.begin();

	while( itr!= lines.end() )
	{

		string_list lineSections = string_util::tokenize(config,std::string(":"),1);

		if (lineSections.size()>1)
		{
			std::string command = string_util::tolower(lineSections[0]);
			std::string dataStr = lineSections[1];

			string_list params = string_util::tokenize(dataStr,std::string(" "),0,true);

			if (command == "nick")
			{
				theBotInfo.nicks.push_back(dataStr);
			}
			else if (command == "server")
			{
				theBotInfo.server = dataStr;
			}
			else if (command == "port")
			{
				theBotInfo.port = atoi(dataStr.c_str());
			}
			else if (command == "channel")
			{
				theBotInfo.channels.push_back(dataStr);
			}
			else if (command == "master")
			{
				theBotInfo.masters.push_back(dataStr);
			}
			else if (command == "joinmessage")
			{
				theBotInfo.joinMessages[params[0]] = dataStr;
			}
			else if (command == "dontknow")
			{
				if (dataStr[0] == "\"")
					dataStr.erase(0,1);

				if (dataStr[dataStr.size()-1] == "\"")
					dataStr.erase(dataStr.end()-1);

				theBotInfo.masters.push_back(dataStr);
			}
		}
		itr++;
	}
}


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
	std::map<std::string,std::string>::iterator itr = theBotInfo.joinMessages.find(info->channel);

	if (itr == theBotInfo.joinMessages.end())
		return;

	IRCCommandINfo	commandInfo;

	commandInfo.target = info->channel;
	commandInfo.params.push_back (itr->second);
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
			if (info->from == theBotInfo.master)
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
		else if ( info->params[1] == "raw")
		{
			client.sendTextToServer(info->getAsString(2));
		}
		else if ( info->params[1] == "part")
		{
			if (info->from == theBotInfo.master)
				client.part(info->target, "I must go");
		}
		else if ( info->params[1] == "join")
		{
			if (info->from == theBotInfo.master)
				client.join(info->getAsString(2));
		}
		else if ( info->params[1] == "users")
		{
			string_list userList = client.listUsers(info->target);
			string_list::iterator itr = userList.begin();

			std::string theLine = info->from + std::string(" this channel contains ");
			while( itr != userList.end())
			{
				theLine += *itr;
				itr++;
				if ( itr != userList.end() )
					theLine += ", ";
			}
			commandInfo.params.push_back(theLine);
			client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		}
		else if ( info->params[1] == "allusers")
		{
			string_list userList = client.listUsers(std::string(""));
			string_list::iterator itr = userList.begin();

			std::string theLine = info->from + std::string(" I know of ");
				while( itr != userList.end())
				{
					theLine += *itr;
					itr++;
					if ( itr != userList.end() )
						theLine += ", ";
				}
				commandInfo.params.push_back(theLine);
				client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
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


myEventCaller	eventHandler;


void registerEventHandlers ( void )
{
	client.registerEventHandler(eIRCNoticeEvent,&eventHandler);
	client.registerEventHandler(eIRCEndMOTDEvent,&eventHandler);
	client.registerEventHandler(eIRCChannelJoinEvent,&eventHandler);
	client.registerEventHandler(eIRCChannelMessageEvent,&eventHandler);
	client.registerEventHandler(eIRCNickNameError,&eventHandler);
}

void initInfo ( void )
{
	theBotInfo.nick = 0;

	theBotInfo.master = "Patlabor221";

	theBotInfo.server = "irc.freenode.net";
	theBotInfo.port = 6667;

	theBotInfo.nicks.push_back(std::string("stupid_Bot"));
	theBotInfo.nicks.push_back(std::string("stupiderBot"));
	theBotInfo.nicks.push_back(std::string("stupidestBot"));

	theBotInfo.host = "stupidBot";
	theBotInfo.realName = "a libIRC bot";

	theBotInfo.channels.push_back(std::string("#opencombat"));
	theBotInfo.channels.push_back(std::string("#brlcad"));

	theBotInfo.joinMessages["#opencombat"] = "May the Force Be With You";
	theBotInfo.joinMessages["#brlcad"] = "Whoa!";
	theBotInfo.joinMessages["#bzflag"] = "I'm not even suposed to be here today";
	theBotInfo.joinMessages["#libirc"] = "Greetings Program!";
}

void main ( void )
{
	initInfo();

	client.setDebugLevel(5);

	// clear the log
	fclose(fopen("irc.log","wt"));

	// set the log
	client.setLogfile("irc.log");
	registerEventHandlers();

	client.connect(theBotInfo.server,theBotInfo.port);

	while (client.process() && !part)
	{
		IRCOSSleep(1);
	}

	if (part)
	{
		client.disconnect("Quiting");
	}
};
