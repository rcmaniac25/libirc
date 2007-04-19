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
#include "IRCTextUtils.h"

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

std::map<std::string,std::map<std::string,std::string> > lastChatMessages;

string_list								spamMatches;
string_list								spamHosts;

typedef struct 
{
	string_list commandStrings;
	std::string server;
	int			port;
	string_list	nicks;
	int			nick;
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
	std::string databaseFile;
	bool		beNice;
}trStupidBotInfo;

trStupidBotInfo	theBotInfo;

IRCClient	client;
bool part = false;

class botCommandHandaler
{
public:
	botCommandHandaler(){return;}
	virtual ~botCommandHandaler(){return;}
	virtual bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false ) = 0;
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

bool callBotCommand ( std::string command, std::string source, std::string from, trMessageEventInfo *info, bool privMsg = false )
{
	tmBotCommandHandalerMap::iterator itr = botCommands.find(command);
	if (itr == botCommands.end())
		return false;

	std::string respondTo = privMsg ? from : info->target;
	return itr->second->command(command,source,from,info,respondTo,privMsg);
}

std::string getRandomString ( string_list &source )
{
	return source[(int)(rand()%(source.size()))];
}

void readDatabase ( void )
{
	spamMatches.clear();
	spamHosts.clear();

	if ( !theBotInfo.databaseFile.size() )
		return;

	FILE *fp = fopen(theBotInfo.databaseFile.c_str(),"rt");
	if (!fp)
		return;

	std::string dataFile;

	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *data = (char*)malloc(len+1);
	fread(data,len,1,fp);
	fclose(fp);
	data[len] = 0;
	dataFile = data;
	free(data);

	std::string lineEnd;
#ifdef _WIN32
	lineEnd = "\r\n";
#else
	lineEnd = "\n";
#endif

	string_list	lines = string_util::tokenize(dataFile,lineEnd);
	for ( unsigned int i = 0; i < lines.size(); i++ )
	{	
		string_list nubs = string_util::tokenize(lines[i],std::string("^"));
		if ( nubs.size() == 2 )
		{
			if (string_util::tolower(nubs[0])=="filter")
				spamMatches.push_back(nubs[1]);
			else if (string_util::tolower(nubs[0])=="host")
				spamHosts.push_back(nubs[1]);
		}
	}
}

void writeDatabase ( void )
{
	if (!theBotInfo.databaseFile.size())
		return;

	FILE *fp = fopen(theBotInfo.databaseFile.c_str(),"wt");
	if (!fp)
		return;

	std::string lineEnd;
#ifdef _WIN32
	lineEnd = "\n";
#else
	lineEnd = "\n";
#endif

	for ( unsigned int i = 0; i < spamMatches.size(); i++)
		fprintf(fp,"filter^%s%s",spamMatches[i].c_str(),lineEnd.c_str());

	for ( unsigned int i = 0; i < spamHosts.size(); i++)
		fprintf(fp,"host^%s%s",spamHosts[i].c_str(),lineEnd.c_str());

	fclose(fp);
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

	theBotInfo.beNice = false;

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

			if (command == "identifyer")
			{
				theBotInfo.commandStrings.push_back(string_util::tolower(dataStr));
			}
			else if (command == "nick")
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
			else if (command == "database")
				theBotInfo.databaseFile = params[0];
		}
		itr++;
	}
	fclose(fp);

	readDatabase();
}

void saveConfig ( void )
{
	writeDatabase();

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

	stringItr = theBotInfo.commandStrings.begin();
	while ( stringItr != theBotInfo.commandStrings.end() )
	{
		fprintf(fp,"idenitfyer:%s%s",stringItr->c_str(),lineEnd.c_str());
		stringItr++;
	}
	fprintf(fp,"%s",lineEnd.c_str());

	// server info
	fprintf(fp,"server:%s%s",theBotInfo.server.c_str(),lineEnd.c_str());
	fprintf(fp,"port:%d%s%s",theBotInfo.port,lineEnd.c_str(),lineEnd.c_str());

	fprintf(fp,"username:%s%s",theBotInfo.username.c_str(),lineEnd.c_str());
	fprintf(fp,"host:%s%s",theBotInfo.host.c_str(),lineEnd.c_str());
	fprintf(fp,"realname:%s%s%s",theBotInfo.realName.c_str(),lineEnd.c_str(),lineEnd.c_str());
	if (theBotInfo.databaseFile.size())
		fprintf(fp,"database:%s%s",theBotInfo.databaseFile.c_str(),lineEnd.c_str());

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
	if (lastChatMessages.find(info->channel) != lastChatMessages.end())
		lastChatMessages[info->channel].clear();

	std::map<std::string,std::string>::iterator itr = theBotInfo.joinMessages.find(info->channel);

	if (itr == theBotInfo.joinMessages.end())
		return;

	IRCCommandINfo	commandInfo;

	commandInfo.target = info->channel;
	commandInfo.params.push_back (itr->second);
	client.sendIRCCommand(eCMD_PRIVMSG,commandInfo);
}

bool checkForRepeatedMessages ( std::string &target, std::string &from, std::string &message )
{
	bool spam = false;
	if (lastChatMessages.find(target) == lastChatMessages.end())
	{
		std::map<std::string,std::string>	temp;
		lastChatMessages[target] = temp;
	}
	std::map<std::string,std::string> &chanList = lastChatMessages[target];

	if (chanList.find(from) == chanList.end())
		spam = false;
	else
		spam = message == chanList[from];

	chanList[from] = message;
	return spam;
}

bool checkForSpamFilter ( std::string &target, std::string &from, std::string &message )
{
	if (!message.size())
		return false;

	std::string testMessage = string_util::tolower(message);

	for (unsigned int i = 0; i < spamMatches.size(); i++)
	{
		std::string text = string_util::tolower(spamMatches[i]);
		
		if (strstr(testMessage.c_str(),text.c_str()))
			return true;
	}

	return false;
}

bool checkForSpamHost ( std::string &target, std::string &host )
{
	if (!host.size())
		return false;

	std::string testHost = string_util::tolower(host);

	for (unsigned int i = 0; i < spamHosts.size(); i++)
	{
		std::string text = string_util::tolower(spamHosts[i]);

		if (strstr(testHost.c_str(),text.c_str()))
			return true;
	}

	return false;
}

void checkForSpam ( std::string &target, std::string &from, std::string &message )
{
	bool spam = false;

	if (checkForRepeatedMessages(target,from,message))
		spam = true;
	else if (checkForSpamFilter(target,from,message))
		spam = true;
	else if (checkForSpamHost(target,client.getUserManager().getUserHost(from)))
		spam = true;
	
	if (spam)
	{
		if (!theBotInfo.beNice)
			client.ban(client.getUserManager().getUserHost(from),target);
		client.kick(from,target,std::string("spam"));
	}
}


void userJoin( trJoinEventInfo *info )
{
	if (!info->channel.size() && !info->user.size())
		return;

	if (checkForSpamHost(info->channel, client.getUserManager().getUserHost(info->user)))
	{
		//client.ban(client.getUserManager().getUserHost(info->user),info->channel);
		client.kick(info->user,info->channel,std::string("spamHost"));
	}
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

bool isForMe ( std::string word )
{
	std::string myNick = string_util::tolower(client.getNick());

	if (word == myNick)
		return true;

	string_list::iterator	itr = theBotInfo.commandStrings.begin();
	while ( itr != theBotInfo.commandStrings.end() )
	{
		if (*itr == word)
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

	if ( isForMe(firstWord) )
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
	else
	{
		checkForSpam(info->target,info->from,info->message);
	}
}

void privateMessage ( trMessageEventInfo *info )
{
	IRCCommandINfo	commandInfo;
	commandInfo.target = info->target;

	std::string command = string_util::tolower(info->params[0]);

	// its for me, it's allways for me here
	// see if there is a command handaler for it

	if (command == "factoid" || !callBotCommand(command,info->source,info->from,info,true))
	{
		// ignore CTCP messages other than version
		char ctcpmarker = 0x01;
		if (command[0] == ctcpmarker)
		{
			if (string_util::compare_nocase(command.substr(1, 7), "VERSION") == 0)
			{
				std::string reply = "StupidBot: " + getLibVersion();
				client.sendCTCPReply(info->from, eCMD_CTCP_VERSION, reply);
			}
			return;
		}

		// see if it's a factoid
		if (!callBotCommand("factoid",info->source,info->from,info,true))
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

void userKicked (trKickBanEventInfo *info)
{
	std::string	message = "whoa, ";
	message += info->user + " got kicked by " + info->kicker + " for " + info->reason + " that has to suck!";
//	client.sendMessage(info->channel,message);
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

		case eIRCUserJoinEvent:
			userJoin((trJoinEventInfo*)&info);

		case eIRCChannelJoinEvent:
			joinedChannel((trJoinEventInfo*)&info);
			break;

		case eIRCPrivateMessageEvent:
			privateMessage ((trMessageEventInfo*)&info);
			break;

		case eIRCChannelMessageEvent:
			channelMessage ((trMessageEventInfo*)&info);
			break;

		case eIRCUserKickedEvent:
			userKicked ((trKickBanEventInfo*)&info);
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
	client.registerEventHandler(eIRCPrivateMessageEvent,&eventHandler);
	client.registerEventHandler(eIRCUserKickedEvent,&eventHandler);
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
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false );
};

bool quitCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	part = true;
	return true;
}

class helloCommand : public botCommandHandaler
{
public:
	helloCommand() {name = "hello";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false );
};

bool helloCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg )
{
	std::string message = std::string("Hey ") + from + std::string(" how are you?");
	client.sendMessage(respondTo,message);
	return true;
}

class factoidCommand : public botCommandHandaler
{
public:
	factoidCommand() {name = "factoid";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false );
};

bool factoidCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg )
{
	std::string factoid = string_util::tolower(info->params[1]);
	
	// it may have a ?, lop it off
	if (*(factoid.end()-1) == '?')
		factoid.erase(factoid.end()-1);

	int	paramOffset = privMsg ? 0 : 1;

	factoidMap::iterator itr = theBotInfo.factoids.find(factoid);
	if (itr == theBotInfo.factoids.end())
	{

		if ( info->params[1+paramOffset] == "is" )
		{
			trFactoid	newFactoid;
			
			newFactoid.name = factoid;
			newFactoid.action = (string_util::tolower(info->params[2+paramOffset]) == "<reply>");

			if (!newFactoid.action)
				newFactoid.response = (string_util::tolower(info->params[2+paramOffset]) == "<respond>");
			else
				newFactoid.response = false;

			newFactoid.message = info->getAsString(newFactoid.action || newFactoid.response ? 3+paramOffset : 2+paramOffset);

			theBotInfo.factoids[factoid] = newFactoid;

			std::string message = "Ok, " + from;
			client.sendMessage(respondTo,message,itr->second.action);
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

	client.sendMessage(respondTo,message,itr->second.action);

	return true;
}

class channelCommand : public botCommandHandaler
{
public:
	channelCommand() {name = "channel";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool channelCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (privMsg)
		client.sendMessage(respondTo,"Invalid command, no channel");
	else
		client.sendMessage(respondTo,info->target);
	return true;
}

class flushCommand : public botCommandHandaler
{
public:
	flushCommand() {name = "flush";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool flushCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	saveConfig();

	std::string message = "Ok, " + from;
	client.sendMessage(respondTo,message);

	return true;
}

class rawCommand : public botCommandHandaler
{
public:
	rawCommand() {name = "raw";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool rawCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	int	paramOffset = privMsg ? 0 : 1;

	std::string text = info->getAsString(1+paramOffset);
	client.sendTextToServer(text);
	return true;
}

class channelsCommand : public botCommandHandaler
{
public:
	channelsCommand() {name = "channels";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool channelsCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
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
	client.sendMessage(respondTo,theLine);
	return true;
}

class partCommand : public botCommandHandaler
{
public:
	partCommand() {name = "part";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool partCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}


	std::string partTarget;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: part SOME_CHANNEL");
		else
			partTarget = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			partTarget = info->target;
		else
			partTarget = info->params[2];
	}

	if (partTarget.size())
	{
		std::string message = "Ok, " + from;
		client.sendMessage(respondTo,message);

		client.part(partTarget, getRandomString(theBotInfo.partMessages));

		if (!client.listChanels().size())
			part = true;
	}
	return true;
}

class joinCommand : public botCommandHandaler
{
public:
	joinCommand() {name = "join";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool joinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	std::string joinTarget;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: join SOME_CHANNEL");
		else
			joinTarget = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: join SOME_CHANNEL");
		else
			joinTarget = info->params[2];
	}
	
	if (joinTarget.size())
	{
		std::string message = "Ok, " + from;
		client.sendMessage(respondTo,message);
		client.join(joinTarget);
	}
	return true;
}

class usersCommand : public botCommandHandaler
{
public:
	usersCommand() {name = "users";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool usersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	std::string channel;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: users SOME_CHANNEL");
		else
			channel = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			channel = info->target;
		else
			channel = info->params[2];
	}

	if (channel.size())
	{
		string_list userList = client.listUsers(channel);
		string_list::iterator itr = userList.begin();

		std::string theLine = info->from + std::string(" this channel contains ");
		while( itr != userList.end())
		{
			theLine += *itr;
			itr++;
			if ( itr != userList.end() )
				theLine += ", ";
		}
		client.sendMessage(respondTo,theLine);
	}

	return true;
}

class allUsersCommand : public botCommandHandaler
{
public:
	allUsersCommand() {name = "allusers";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool allUsersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
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
	client.sendMessage(respondTo,theLine);
	return true;
}

class addjoinCommand : public botCommandHandaler
{
public:
	addjoinCommand() {name = "addjoin";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addjoinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	std::string channel;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: users SOME_CHANNEL");
		else
			channel = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			channel = info->target;
		else
			channel = info->params[2];
	}
	if (channel.size())
	{
		std::string message = "Ok, " + from + ", channel, " + channel + " added to startup list";
		client.sendMessage(respondTo,message);
		client.join(channel);
		theBotInfo.channels.push_back(channel);
	}
	return true;
}

class addmasterCommand : public botCommandHandaler
{
public:
	addmasterCommand() {name = "addmaster";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addmasterCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	std::string newMaster;
	
	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: addMaster SOME_DUDE");
		else
			newMaster = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: addMaster SOME_DUDE");
		else
			newMaster = info->params[2];
	}
	
	if (newMaster.size())
	{
		std::string message = "Ok, " + from + ", user, " + newMaster + " added to masters list";
		client.sendMessage(respondTo,message);

		theBotInfo.masters.push_back(string_util::tolower(newMaster));
	}
	return true;
}

class libVersCommand : public botCommandHandaler
{
public:
	libVersCommand() {name = "libversion";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool libVersCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	client.sendMessage(respondTo, getLibVersion());
	return true;
}

class longTestCommand : public botCommandHandaler
{
public:
	longTestCommand() {name = "longtest";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool longTestCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	std::string longString ;

	int paramOffset = privMsg ? 0 : 1;

	if ((int)info->params.size() <= 1+paramOffset)
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

	client.sendMessage(respondTo,longString);
	return true;
}

class chanPermsCommand : public botCommandHandaler
{
public:
	chanPermsCommand() {name = "chanperms";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool chanPermsCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	std::string channel;
	
	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: chanperms SOME_CHANNEL");
		else
			channel = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: chanperms SOME_CHANNEL");
		else
			channel = info->params[2];
	}

	if (channel.size())
	{
		trIRCChannelPermisions perms = client.getChanPerms(channel);

		std::string message = from + ", " +channel + " mode is " + perms.mode;
		client.sendMessage(respondTo,message);
	}
	return true;
}

class userInfoCommand : public botCommandHandaler
{
public:
	userInfoCommand() {name = "userinfo";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool userInfoCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	std::string userName;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: userinfo SOME_USER");
		else
			userName = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: userinfo SOME_USER");
		else
			userName = info->params[2];
	}

	if (userName.size())
	{
		IRCUserManager	&userManager = client.getUserManager();

		if (!userManager.userExists(userName))
		{
			client.sendMessage(respondTo,std::string("Sorry ") + source + " I don't know about " + userName);
			return true;
		}

		int userID = userManager.getUserID(userName);

		client.sendMessage(respondTo,std::string("OK ") + from);
		client.sendMessage(respondTo,userName + "'s host is " + userManager.getUserHost(userID));
		if ( !userManager.userHasChannels(userID) )
			client.sendMessage(respondTo,"and isn't in any channels");
		else
		{
			string_list chans = userManager.listUserChannelNames(userID);
			std::string message = "is in the folowing channel";
			if ( chans.size() > 1)
				message += "s";

			string_list::iterator itr = chans.begin();
			while ( itr != chans.end() )
				message += std::string(" ") + *(itr++);
			client.sendMessage(respondTo,message);
		}

		if ( userManager.userIsIdentified(userID) )
			client.sendMessage(respondTo,"and is identified");

		trIRCUserPermisions	perms = userManager.getUserPerms(userID);

		if (perms.mode.size())
			client.sendMessage(respondTo,std::string("MODE is set to \"") + perms.mode + "\"");

		std::string lastSaid = userManager.getUserLastMessage(userID);
		if ( lastSaid.size() )
		{
			client.sendMessage(respondTo,std::string("last said \"") + lastSaid + "\"");
			if (userManager.getUserLastMessageChannel(userID) != -1)
				client.sendMessage(respondTo,std::string("in ") + userManager.getUserLastMessageChannelName(userID) );
		}
	}
	return true;
}

class chanInfoCommand : public botCommandHandaler
{
public:
	chanInfoCommand() {name = "chaninfo";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool chanInfoCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	std::string channelName;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: chaninfo SOME_CHANNEL");
		else
			channelName = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: chaninfo SOME_CHANNEL");
		else
			channelName = info->params[2];
	}

	if (channelName.size())
	{
		IRCUserManager	&userManager = client.getUserManager();

		if (!userManager.channelExists(channelName))
		{
			client.sendMessage(respondTo,std::string("Sorry ") + from + " I don't know about " + channelName);
			return true;
		}

		int chanID = userManager.getChannelID(channelName);

		client.sendMessage(respondTo,std::string("OK ") + from);
		client.sendMessage(respondTo,channelName + "'s topic is " + userManager.getChannelTopic(chanID));

		string_list ops = client.listChanOps(channelName);

		std::string message = "it's operators are, ";
		string_list::iterator itr = ops.begin();
		while ( itr != ops.end() )
			message += std::string(" ") + *(itr++);
		client.sendMessage(respondTo,message);

		trIRCChannelPermisions	perms = userManager.getChannelPerms(chanID);

		if (perms.mode.size())
			client.sendMessage(respondTo,std::string("MODE is set to \"") + perms.mode + "\"");

		std::string lastSaid = userManager.getChannelLastMessage(chanID);
		if ( lastSaid.size() )
		{
			client.sendMessage(respondTo,std::string("last message was \"") + lastSaid + "\"");
			client.sendMessage(respondTo,std::string("by ") + userManager.getChannelLastMessageUserName(chanID));
		}
	}
	return true;
}

class helpCommand : public botCommandHandaler
{
public:
	helpCommand() {name = "help";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool helpCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	tmBotCommandHandalerMap::iterator itr = botCommands.begin();
	std::string message = "Current commands;";

	while (itr != botCommands.end())
	{
		message += std::string(" ") + itr->first;
		itr++;
	}
	
	client.sendMessage(respondTo,message);

	return true;
}

class factoidListCommand : public botCommandHandaler
{
public:
	factoidListCommand() {name = "factoidlist";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool factoidListCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	factoidMap::iterator itr = theBotInfo.factoids.begin();
	std::string message = "Current factoids;";

	while (itr != theBotInfo.factoids.end())
	{
		message += std::string(" ") + itr->first;
		itr++;
	}

	client.sendMessage(respondTo,message);

	return true;
}

class kickCommand : public botCommandHandaler
{
public:
	kickCommand() {name = "kick";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool kickCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	std::string bastard;
	std::string channel;
	std::string reason;

	if (privMsg)
	{
		if ( info->params.size()<4)
			client.sendMessage(respondTo,"Usage: kick SOME_CHANNEL SOME_POOR_BASTARD SOME_LAME_REASON");
		else
		{
			bastard = info->params[1];
			channel = info->params[2];
			reason = info->getAsString(3);
		}
	}
	else
	{
		if ( info->params.size()<5)
			client.sendMessage(respondTo,"Usage: kick SOME_CHANNEL SOME_POOR_BASTARD SOME_LAME_REASON");
		else{
			bastard = info->params[2];
			channel = info->params[3];
			reason = info->getAsString(4);
		}
	}

	if (bastard.size())
	{
		client.sendMessage(respondTo,std::string("OK ") + from);

		client.kick(bastard,channel,reason);
	}
	return true;
}

class addSpamCommand : public botCommandHandaler
{
public:
	addSpamCommand() {name = "addspam";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addSpamCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}
	std::string text;

	if (privMsg)
	{
		if ( info->params.size()<1)
			client.sendMessage(respondTo,"Usage: addspam SOME TEXT");
		else
		{
			text = info->getAsString(2);
		}
	}
	else
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: addspam SOME TEXT");
		else{
			text = info->getAsString(2);
		}
	}

	if (text.size())
	{
		spamMatches.push_back(text);
		client.sendMessage(respondTo,std::string("OK ") + from);

		writeDatabase();
	}
	return true;
}

class addSpamHostCommand : public botCommandHandaler
{
public:
	addSpamHostCommand() {name = "addspamhost";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addSpamHostCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}
	std::string text;

	if (privMsg)
	{
		if ( info->params.size()<1)
			client.sendMessage(respondTo,"Usage: addspamhost SOME TEXT");
		else
		{
			text = info->getAsString(2);
		}
	}
	else
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: addspamhost SOME TEXT");
		else{
			text = info->getAsString(2);
		}
	}

	if (text.size())
	{
		spamHosts.push_back(text);
		client.sendMessage(respondTo,std::string("OK ") + from);

		writeDatabase();
	}
	return true;
}

class beNiceCommand : public botCommandHandaler
{
public:
	beNiceCommand() {name = "benice";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool beNiceCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	theBotInfo.beNice = true;
	client.sendMessage(respondTo,std::string("Playing it safe ") + from);
	return true;
}

class hardballCommand : public botCommandHandaler
{
public:
	hardballCommand() {name = "hardball";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool hardballCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!isMaster(from))
	{
		client.sendMessage(respondTo,"You're not the boss of me");
		return true;
	}

	theBotInfo.beNice = false;
	client.sendMessage(respondTo,std::string("Playing for keeps ") + from);
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
	installBotCommand(new userInfoCommand);
	installBotCommand(new chanInfoCommand);
	installBotCommand(new helpCommand);
	installBotCommand(new factoidListCommand);
	installBotCommand(new kickCommand);
	installBotCommand(new addSpamCommand);
	installBotCommand(new addSpamHostCommand);
	installBotCommand(new beNiceCommand);
	installBotCommand(new hardballCommand);

}

