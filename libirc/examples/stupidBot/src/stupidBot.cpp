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
	bool				response;
}trFactoid;

typedef std::map<std::string,trFactoid>	factoidMap;
typedef std::map<std::string,std::string> string_map;

typedef struct 
{
	std::string server;
	int					port;
	string_list	nicks;
	int					nick;
	std::string username;
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

class botCommandHandaler
{
public:
	botCommandHandaler(){return;}
	virtual ~botCommandHandaler(){return;}
	virtual bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info ) = 0;
	std::string name;
};

typedef std::map<std::string, botCommandHandaler*> tmBotCommandHandalerMap;

tmBotCommandHandalerMap		botCommands;

void registerBotCommands ( void );

void installBotCommand ( botCommandHandaler* handaler )
{
	if (!handaler)
		return;

	botCommands[string_util::tolower(handaler->name)] = handaler;
}

bool callBotCommand ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	tmBotCommandHandalerMap::iterator itr = botCommands.find(command);
	if (itr == botCommands.end())
		return false;

	return itr->second->command(command,source,from,info);
}

std::string getRandomString ( string_list &source )
{
	return source[(int)(rand()%(source.size()))];
}

void readConfig ( std::string file )
{
	theBotInfo.config = file;
	std::string config;

	// read the file data
	FILE *fp = fopen(file.c_str(),"rt");
	if (!fp)
		return;

	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *data = (char*)malloc(len+1);
	fread(data,len,1,fp);
	fclose(fp);
	data[len] = 0;
	config = data;
	free(data);

	std::string lineEnd;
#ifdef _WIN32
	lineEnd = "\r\n";
#else
	lineEnd = "\n";
#endif

	string_list lines = string_util::tokenize(config,lineEnd);

	string_list::iterator itr = lines.begin();

	while( itr!= lines.end() )
	{
		std::string line = *itr;

		string_list lineSections = string_util::tokenize(line,std::string(":"),0);

		std::string test = lineSections[0];
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
			else if (command == "username")
			{
				theBotInfo.username = dataStr;
			}
			else if (command == "host")
			{
				theBotInfo.host = dataStr;
			}
			else if (command == "realname")
			{
				theBotInfo.realName = dataStr;
			}
			else if (command == "channel")
			{
				theBotInfo.channels.push_back(dataStr);
			}
			else if (command == "master")
			{
				theBotInfo.masters.push_back(string_util::tolower(dataStr));
			}
			else if (command == "joinmessage")
			{
				theBotInfo.joinMessages[params[0]] = params[1];
			}
			else if (command == "dontknow")
			{
				if (dataStr[0] == '\"')
					dataStr.erase(0,1);

				if (dataStr[dataStr.size()-1] == '\"')
					dataStr.erase(dataStr.end()-1);

				theBotInfo.unknownResponces.push_back(dataStr);
			}
			else if (command == "part")
			{
				if (dataStr[0] == '\"')
					dataStr.erase(0,1);

				if (dataStr[dataStr.size()-1] == '\"')
					dataStr.erase(dataStr.end()-1);

				theBotInfo.partMessages.push_back(dataStr);
			}
			else if (command == "quit")
			{
				if (dataStr[0] == '\"')
					dataStr.erase(0,1);

				if (dataStr[dataStr.size()-1] == '\"')
					dataStr.erase(dataStr.end()-1);

				theBotInfo.quitMessages.push_back(dataStr);
			}	
			else if (command == "action")
			{
				trFactoid	factoid;
				factoid.action = true;
				factoid.response = false;
				factoid.name = params[0];
				factoid.message = params[1];
				theBotInfo.factoids[string_util::tolower(factoid.name)] = factoid;
			}
			else if (command == "factoid")
			{
				trFactoid	factoid;
				factoid.action = false;
				factoid.response = false;
				factoid.name = params[0];
				factoid.message = params[1];
				theBotInfo.factoids[string_util::tolower(factoid.name)] = factoid;
			}
			else if (command == "response")
			{
				trFactoid	factoid;
				factoid.action = false;
				factoid.response = true;
				factoid.name = params[0];
				factoid.message = params[1];
				theBotInfo.factoids[string_util::tolower(factoid.name)] = factoid;
			}
		}
		itr++;
	}
	fclose(fp);
}

void saveConfig ( void )
{
	std::string file = theBotInfo.config;
	// read the file data
	FILE *fp = fopen(file.c_str(),"wt");
	if (!fp)
		return;

	std::string lineEnd;
#ifdef _WIN32
	lineEnd = "\n";
#else
	lineEnd = "\n";
#endif
	
	// nicknames
	string_list::iterator	stringItr = theBotInfo.nicks.begin();

	while ( stringItr != theBotInfo.nicks.end() )
	{
		fprintf(fp,"nick:%s%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// server info
	fprintf(fp,"server:%s%s",theBotInfo.server.c_str(),lineEnd.c_str());
	fprintf(fp,"port:%d%s%s",theBotInfo.port,lineEnd.c_str(),lineEnd.c_str());

	fprintf(fp,"username:%s%s",theBotInfo.username.c_str(),lineEnd.c_str());
	fprintf(fp,"host:%s%s",theBotInfo.host.c_str(),lineEnd.c_str());
	fprintf(fp,"realname:%s%s%s",theBotInfo.realName.c_str(),lineEnd.c_str(),lineEnd.c_str());

	// channels
	stringItr = theBotInfo.channels.begin();

	while ( stringItr != theBotInfo.channels.end() )
	{
		fprintf(fp,"channel:%s%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// masters
	stringItr = theBotInfo.masters.begin();

	while ( stringItr != theBotInfo.masters.end() )
	{
		fprintf(fp,"master:%s%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());
	
	// join messages
	string_map::iterator stringMapItr = theBotInfo.joinMessages.begin();

	while ( stringMapItr != theBotInfo.joinMessages.end() )
	{
		fprintf(fp,"joinmessage:%s \"%s\"%s",stringMapItr->first.c_str(),stringMapItr->second.c_str(),lineEnd.c_str());
		stringMapItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// don't knows
	stringItr = theBotInfo.unknownResponces.begin();

	while ( stringItr != theBotInfo.unknownResponces.end() )
	{
		fprintf(fp,"dontknow:\"%s\"%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// part
	stringItr = theBotInfo.partMessages.begin();

	while ( stringItr != theBotInfo.partMessages.end() )
	{
		fprintf(fp,"part:\"%s\"%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// quit
	stringItr = theBotInfo.quitMessages.begin();

	while ( stringItr != theBotInfo.quitMessages.end() )
	{
		fprintf(fp,"quit:\"%s\"%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());
		
	// factoids
	factoidMap::iterator	factoidItr = theBotInfo.factoids.begin();

	while ( factoidItr != theBotInfo.factoids.end() )
	{
		if ( factoidItr->second.action )
		{
			fprintf(fp,"action:%s \"%s\"%s",factoidItr->first.c_str(),factoidItr->second.message.c_str(),lineEnd.c_str());
		}
		else
		{
			if ( factoidItr->second.response )
				fprintf(fp,"response:%s \"%s\"%s",factoidItr->first.c_str(),factoidItr->second.message.c_str(),lineEnd.c_str());
			else
				fprintf(fp,"factoid:%s \"%s\"%s",factoidItr->first.c_str(),factoidItr->second.message.c_str(),lineEnd.c_str());
		}
		factoidItr++;
	}
	fclose(fp);
}

void login ( void )
{
	client.login(theBotInfo.nicks[theBotInfo.nick],theBotInfo.username,theBotInfo.realName,theBotInfo.host);
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

bool isMaster ( std::string name )
{
	string_list::iterator itr = theBotInfo.masters.begin();

	name = string_util::tolower(name);

	while ( itr != theBotInfo.masters.end() )
	{	
		if ( name == *itr)
			return true;

		itr++;
	}
	return false;
}

void channelMessage ( trMessageEventInfo *info )
{
	std::string myNick = string_util::tolower(client.getNick());

	IRCCommandINfo	commandInfo;
	commandInfo.target = info->target;

	std::string firstWord = info->params[0];
	if ( (*(firstWord.end()-1) == ':') || (*(firstWord.end()-1) == ',') )
		firstWord.erase(firstWord.end()-1);

	firstWord = string_util::tolower(firstWord);

	bool master = isMaster(info->from);

	if ( firstWord == myNick )
	{
		std::string command = string_util::tolower(info->params[1]);
		// its for me
		// see if there is a command handaler for it
		if (command == "factoid" || !callBotCommand(command,info->source,info->from,info))
		{
			// see if it's a factoid
			if (!callBotCommand("factoid",info->source,info->from,info))
			{
				// it's not, we dono what it is, but it was addressed to us
				std::string	dono = getRandomString(theBotInfo.unknownResponces);

				dono = string_util::replace_all(dono,"%u",info->from);
				dono = string_util::replace_all(dono,"%c",info->target);
				dono = string_util::replace_all(dono,"%b",client.getNick());

				client.sendMessage(info->target,dono);
			}
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
				client.login(theBotInfo.nicks[theBotInfo.nick],theBotInfo.username,theBotInfo.realName,theBotInfo.host);
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

void initInfo ( std::string config )
{
	theBotInfo.nick = 0;
	
	readConfig(config);

	registerBotCommands();
}

int main ( int argc, char *argv[] )
{
	std::string Config = "sample.cfg";

	if (argc>1)
		Config = argv[1];

	initInfo(Config);

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

	return 0;
}

class quitCommand : public botCommandHandaler
{
public:
	quitCommand() {name = "quit";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool quitCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	part = true;
	return true;
}

class helloCommand : public botCommandHandaler
{
public:
	helloCommand() {name = "hello";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool helloCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	std::string message = std::string("Hey ") + from + std::string(" how are you?");
	client.sendMessage(info->target,message);
	return true;
}

class factoidCommand : public botCommandHandaler
{
public:
	factoidCommand() {name = "factoid";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool factoidCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	std::string factoid = string_util::tolower(info->params[1]);
	
	// it may have a ?, lop it off
	if (*(factoid.end()-1) == '?')
		factoid.erase(factoid.end()-1);

	factoidMap::iterator itr = theBotInfo.factoids.find(factoid);
	if (itr == theBotInfo.factoids.end())
	{

		if ( info->params[2] == "is" )
		{
			trFactoid	newFactoid;
			
			newFactoid.name = factoid;
			newFactoid.action = (string_util::tolower(info->params[3]) == "<reply>");

			if (!newFactoid.action)
				newFactoid.response = (string_util::tolower(info->params[3]) == "<respond>");
			else
				newFactoid.response = false;

			newFactoid.message = info->getAsString(newFactoid.action || newFactoid.response ? 4 : 3);

			theBotInfo.factoids[factoid] = newFactoid;

			std::string message = "Ok, " + from;
			client.sendMessage(info->target,message,itr->second.action);
			return true;
		}
		return false;
	}

	std::string message;
	
	if (!itr->second.response && !itr->second.action)
		message = itr->second.name + " is ";

	message += itr->second.message;

	message = string_util::replace_all(message,"%u",from);
	message = string_util::replace_all(message,"%c",info->target);
	message = string_util::replace_all(message,"%b",client.getNick());

	client.sendMessage(info->target,message,itr->second.action);

	return true;
}

class channelCommand : public botCommandHandaler
{
public:
	channelCommand() {name = "channel";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool channelCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	client.sendMessage(info->target,info->target);
	return true;
}

class flushCommand : public botCommandHandaler
{
public:
	flushCommand() {name = "flush";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool flushCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	saveConfig();

	std::string message = "Ok, " + from;
	client.sendMessage(info->target,message);

	return true;
}

class rawCommand : public botCommandHandaler
{
public:
	rawCommand() {name = "raw";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool rawCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string text = info->getAsString(2);
	client.sendTextToServer(text);
	return true;
}

class channelsCommand : public botCommandHandaler
{
public:
	channelsCommand() {name = "channels";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool channelsCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
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
	client.sendMessage(info->target,theLine);
	return true;
}

class partCommand : public botCommandHandaler
{
public:
	partCommand() {name = "part";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool partCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string message = "Ok, " + from;
	client.sendMessage(info->target,message);

	client.part(info->target, getRandomString(theBotInfo.partMessages));

	if (!client.listChanels().size())
		part = true;
	return true;
}

class joinCommand : public botCommandHandaler
{
public:
	joinCommand() {name = "join";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool joinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string message = "Ok, " + from;
	client.sendMessage(info->target,message);
	client.join(info->getAsString(2));
	return true;
}

class usersCommand : public botCommandHandaler
{
public:
	usersCommand() {name = "users";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool usersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
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
	client.sendMessage(info->target,theLine);

	return true;
}

class allUsersCommand : public botCommandHandaler
{
public:
	allUsersCommand() {name = "allusers";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool allUsersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
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
	client.sendMessage(info->target,theLine);
	return true;
}

class addjoinCommand : public botCommandHandaler
{
public:
	addjoinCommand() {name = "addjoin";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool addjoinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string channel = info->getAsString(2);

	std::string message = "Ok, " + from + ", channel, " + channel + " added to startup list";
	client.sendMessage(info->target,message);
	client.join(channel);
	theBotInfo.channels.push_back(channel);
	return true;
}

class addmasterCommand : public botCommandHandaler
{
public:
	addmasterCommand() {name = "addmaster";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool addmasterCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string newMaster = info->getAsString(2);

	std::string message = "Ok, " + from + ", user, " + newMaster + " added to masters list";
	client.sendMessage(info->target,message);

	theBotInfo.masters.push_back(string_util::tolower(newMaster));
	return true;
}

class libVersCommand : public botCommandHandaler
{
public:
	libVersCommand() {name = "libversion";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool libVersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	client.sendMessage(info->target,client.getLibVersion());
	return true;
}

class longTestCommand : public botCommandHandaler
{
public:
	longTestCommand() {name = "longtest";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool longTestCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	std::string longString ;

	if (info->params.size() <= 2)
	{
		longString = "this is a realy,";

		for ( int i =0; i < 100; i++)
			longString += " realy,";

		longString += " long string";

	}
	else
	{
		longString = "this is a long string\nwith\nsome\nnewlines in it\n\nIsn't it cool?";
	}

	client.sendMessage(info->target,longString);
	return true;
}

class chanPermsCommand : public botCommandHandaler
{
public:
	chanPermsCommand() {name = "chanperms";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info );
};

bool chanPermsCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info )
{
	if (!isMaster(from))
	{
		client.sendMessage(info->target,"You're not the boss of me");
		return true;
	}

	std::string channel = info->getAsString(2);

	trIRCChannelPermisions perms = client.getChanPerms(channel);

	std::string message = from + ", " +channel + " mode is " + perms.mode;
	client.sendMessage(info->target,message);
	return true;
}

void registerBotCommands ( void )
{
	installBotCommand(new quitCommand);
	installBotCommand(new helloCommand);
	installBotCommand(new factoidCommand);
	installBotCommand(new channelCommand);
	installBotCommand(new flushCommand);
	installBotCommand(new rawCommand);
	installBotCommand(new channelsCommand);
	installBotCommand(new partCommand);
	installBotCommand(new joinCommand);
	installBotCommand(new usersCommand);
	installBotCommand(new allUsersCommand);
	installBotCommand(new addjoinCommand);
	installBotCommand(new addmasterCommand);
	installBotCommand(new libVersCommand);
	installBotCommand(new longTestCommand);
	installBotCommand(new chanPermsCommand);

}


