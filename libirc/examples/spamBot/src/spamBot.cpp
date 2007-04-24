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

unsigned int				chatToKeep = 40;

string_list					spamMatches;
string_list					spamHosts;

typedef struct 
{
	std::vector<std::string> history;
}trUserInfo;

typedef enum
{
	eUnverified,
	eVerifcationPending,
	eVerified
}teVerificationState;

typedef struct 
{
	std::string				master;
	teVerificationState		verification;
	std::string				hostmask;
}trMaster;

typedef struct 
{
	std::string to;
	std::string message;
}trPrivateMessageRecord;

typedef struct trChannelInfo
{
	bool								checkRepeats;
	bool								fiterMessages;
	bool								filterHosts;
	unsigned int						banTolerance;
	bool								kickBeforeBan;

	unsigned int						repeatLimits;
	
	std::map<std::string,trUserInfo>	userInfo;
	std::string							lastMessage;
	std::string							lastHostMask;

	std::vector<trMaster>				masters;
	std::vector<std::string>			whiteHosts;
	std::vector<std::string>			whiteNicks;
	string_list							joinMessags;
	std::vector<trPrivateMessageRecord> connectMessages;

	std::map<std::string, unsigned int>			offsenseList;

	bool								hibernate;

	trChannelInfo()
	{
		checkRepeats = true;
		fiterMessages = true;
		filterHosts = true;
		banTolerance = 3;
		kickBeforeBan = true;
		repeatLimits = 4;
		hibernate = false;
	}
}trChannelInfo;

std::map<std::string, trChannelInfo> channelInfo;

class BotNumericsHandler : public IRCClientCommandHandler
{
public:
	BotNumericsHandler();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

typedef struct 
{
	string_list commandStrings;
	std::string server;
	int			port;

	string_list	nicks;
	int			nick;
	bool		ghost;
	std::string password;

	std::string username;
	std::string host;
	std::string realName;
	
	string_list startupChannels;

	trChannelInfo masterInfo;

	string_list unknownResponces;
	string_list	partMessages;
	string_list	quitMessages;
	
	std::string config;
	std::string databaseFile;
	
}trStupidBotInfo;

trStupidBotInfo	theBotInfo;

IRCClient	client;
bool part = false;

class botCommandHandler
{
public:
	botCommandHandler(){return;}
	virtual ~botCommandHandler(){return;}
	virtual bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false ) = 0;
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("No help available"));
		return true;	
	};
	std::string name;
};

typedef std::map<std::string, botCommandHandler*> tmBotCommandHandlerMap;

tmBotCommandHandlerMap		botCommands;

void registerBotCommands ( void );

void installBotCommand ( botCommandHandler* Handler )
{
	if (!Handler)
		return;

	botCommands[string_util::tolower(Handler->name)] = Handler;
}

bool callBotCommand ( std::string command, std::string source, std::string from, trMessageEventInfo *info, bool privMsg = false )
{
	tmBotCommandHandlerMap::iterator itr = botCommands.find(command);
	if (itr == botCommands.end())
		return false;

	std::string respondTo = privMsg ? from : info->target;
	return itr->second->command(command,source,from,info,respondTo,privMsg);
}

std::string getRandomString ( string_list &source )
{
	return source[(int)(rand()%(source.size()))];
}

trChannelInfo& getChannelInfo ( std::string &channel )
{
	if (channelInfo.find(channel) == channelInfo.end())
	{
		trChannelInfo info;
		info.checkRepeats = theBotInfo.masterInfo.checkRepeats;
		info.fiterMessages = theBotInfo.masterInfo.fiterMessages;
		info.filterHosts = theBotInfo.masterInfo.filterHosts;
	}

	return channelInfo[channel];
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
			std::string key = string_util::tolower(nubs[0]);
			if (key=="filter")
				spamMatches.push_back(nubs[1]);
			else if (key=="host")
				spamHosts.push_back(nubs[1]);
			else if (key == "whitenick")
				theBotInfo.masterInfo.whiteNicks.push_back(nubs[1]);
			else if (key == "whitehost")
				theBotInfo.masterInfo.whiteHosts.push_back(nubs[1]);
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
	
	for ( unsigned int i = 0; i < theBotInfo.masterInfo.whiteNicks.size(); i++)
		fprintf(fp,"whitenick^%s%s",theBotInfo.masterInfo.whiteNicks[i].c_str(),lineEnd.c_str());
	
	for ( unsigned int i = 0; i < theBotInfo.masterInfo.whiteHosts.size(); i++)
		fprintf(fp,"whitehost^%s%s",theBotInfo.masterInfo.whiteHosts[i].c_str(),lineEnd.c_str());

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

	// defautl data
	theBotInfo.ghost = false;

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

			if (command == "identifier")
			{
				theBotInfo.commandStrings.push_back(string_util::tolower(dataStr));
			}
			else if (command == "nick")
			{
				theBotInfo.nicks.push_back(dataStr);
			}
			else if (command == "ghost")
			{
				theBotInfo.ghost = dataStr != "0";
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
				theBotInfo.startupChannels.push_back(dataStr);
			}
			else if (command == "master")
			{
				trMaster	master;
				master.verification = eUnverified;
				if (params.size() > 1)
				{
					master.master = params[0];
					master.hostmask = params[1];
				}
				else
					master.master = string_util::tolower(dataStr);
				theBotInfo.masterInfo.masters.push_back(master);
			}
			else if (command == "chanmaster")
			{
				if (params.size() > 1)
				{
					trMaster	master;
					master.verification = eUnverified;
					if (params.size() > 2)
					{
						master.master = params[1];
						master.hostmask = params[2];
					}
					else
						master.master = string_util::tolower(dataStr);

					getChannelInfo(params[0]).masters.push_back(master);
				}
			}
			else if (command == "chanopt")
			{
				if (params.size() > 2)
				{
					if ( params[1] == "filterchat")
						getChannelInfo(params[0]).fiterMessages = params[2] != "0";
					else if ( params[1] == "filterhosts")
						getChannelInfo(params[0]).filterHosts = params[2] != "0";
					else if ( params[1] == "checkRepeats")
					{
						getChannelInfo(params[0]).checkRepeats = params[2] != "0";
						getChannelInfo(params[0]).repeatLimits = atoi(params[0].c_str());
						if ( getChannelInfo(params[0]).repeatLimits < 2 )
							getChannelInfo(params[0]).repeatLimits = 2;
					}
					else if (params[1] == "bantol" )
						getChannelInfo(params[0]).banTolerance = atoi(params[0].c_str()); 
					else if ( params[1] == "benice")
						getChannelInfo(params[0]).kickBeforeBan = params[2] != "0"; 
					else if ( params[1] == "whitehost")
						getChannelInfo(params[0]).whiteHosts.push_back(params[2]); 
					else if ( params[1] == "whitenick")
						getChannelInfo(params[0]).whiteNicks.push_back(params[2]); 
				}
			}
			else if (command == "chanconnectmessage")
			{
				if (params.size() > 2)
				{
					trPrivateMessageRecord record;
					record.to = params[1];
					record.message = params[2];
					getChannelInfo(params[0]).connectMessages.push_back(record);
				}
			}
			else if (command == "joinmessage")
			{
				getChannelInfo(params[0]).joinMessags.push_back(params[1]);
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
			else if (command == "database")
				theBotInfo.databaseFile = params[0];
			else if (command == "password")
				theBotInfo.password = params[0];
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

	fprintf(fp,"ghost:%s%s",theBotInfo.ghost ? "1" : "0",lineEnd.c_str());

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
	stringItr = theBotInfo.startupChannels.begin();

	for ( unsigned int i = 0; i <  theBotInfo.startupChannels.size(); i++ )
	{
		fprintf(fp,"channel:%s%s",theBotInfo.startupChannels[i].c_str(),lineEnd.c_str());
		
		trChannelInfo	&info = getChannelInfo(theBotInfo.startupChannels[i]);

		if(info.kickBeforeBan)
			fprintf(fp,"chanopt:benice 1 %s",lineEnd.c_str());
		else
			fprintf(fp,"chanopt:benice 0 %s",lineEnd.c_str());

		if(info.fiterMessages)
			fprintf(fp,"chanopt:filterchat 1 %s",lineEnd.c_str());
		else
			fprintf(fp,"chanopt:filterchat 0 %s",lineEnd.c_str());

		if(info.filterHosts)
			fprintf(fp,"chanopt:filterhosts 1 %s",lineEnd.c_str());
		else
			fprintf(fp,"chanopt:filterhosts 0 %s",lineEnd.c_str());

		fprintf(fp,"chanopt:bantol %d %s",info.banTolerance,lineEnd.c_str());
		fprintf(fp,"chanopt:checkRepeats %d %s",info.repeatLimits,lineEnd.c_str());
		for ( unsigned int t = 0; t < info.whiteHosts.size(); t++ )
			fprintf(fp,"chanmaster:whitehost %s%s",info.whiteHosts[t].c_str(),lineEnd.c_str());
		for ( unsigned int t = 0; t < info.whiteNicks.size(); t++ )
			fprintf(fp,"chanmaster:whitenick %s%s",info.whiteNicks[t].c_str(),lineEnd.c_str());

		for ( unsigned int t = 0; t < info.masters.size(); t++ )
		{
			if (info.masters[t].hostmask.size())
				fprintf(fp,"chanmaster:%s %s%s",info.masters[t].master.c_str(),info.masters[t].hostmask.c_str(),lineEnd.c_str());
			else
				fprintf(fp,"chanmaster:%s%s",info.masters[t].master.c_str(),lineEnd.c_str());
		}

		for ( unsigned int t = 0; t < info.connectMessages.size(); t++ )
			fprintf(fp,"chanconnectmessage:%s \"%s\"%s",info.connectMessages[t].to.c_str(),info.connectMessages[t].message.c_str(),lineEnd.c_str());	
	
		for ( unsigned int t = 0; t < info.joinMessags.size(); t++ )
			fprintf(fp,"joinmessage:\"%s\"%s",info.joinMessags[t].c_str(),lineEnd.c_str());
	}

	fprintf(fp,"%s",lineEnd.c_str());

	// masters

	for ( unsigned int i = 0; i <  theBotInfo.masterInfo.masters.size(); i++ )
	{
		if (theBotInfo.masterInfo.masters[i].hostmask.size())
			fprintf(fp,"master:%s %s%s",theBotInfo.masterInfo.masters[i].master.c_str(),theBotInfo.masterInfo.masters[i].hostmask.c_str(),lineEnd.c_str());
		else
			fprintf(fp,"master:%s%s",theBotInfo.masterInfo.masters[i].master.c_str(),lineEnd.c_str());
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
	fclose(fp);
}

void login ( void )
{
	client.login(theBotInfo.nicks[theBotInfo.nick],theBotInfo.username,theBotInfo.realName,theBotInfo.host);
}

void handleNickServ ( void )
{
	if (!theBotInfo.password.size())
		return;

	std::string message = "register " + theBotInfo.password;
	client.sendMessage(std::string("nickserv"),message);
	message = "identify " + theBotInfo.password;
	client.sendMessage(std::string("nickserv"),message);
}

void joinChannels ( void )
{
	handleNickServ();
	for (unsigned int i = 0; i < theBotInfo.startupChannels.size(); i++)
		client.join(theBotInfo.startupChannels[i]);
}

void joinedChannel ( trJoinEventInfo *info )
{
	trChannelInfo	&chanInfo = getChannelInfo(info->channel);

	// blow out the collection stuff
	chanInfo.lastMessage = "";
	chanInfo.lastHostMask = "";
	chanInfo.userInfo.clear();
	chanInfo.offsenseList.clear();
	chanInfo.hibernate = false;

	// do any on join channel messages
	for ( unsigned int i = 0; i < chanInfo.connectMessages.size(); i++ )
		client.sendMessage(chanInfo.connectMessages[i].to,chanInfo.connectMessages[i].message);

	for ( unsigned int i = 0; i < chanInfo.joinMessags.size(); i++ )
		client.sendMessage(info->channel,chanInfo.joinMessags[i]);
}

bool checkForRepeatedMessages ( std::string &channel, std::string &hostmask, std::string &message )
{
	trChannelInfo	&chanInfo = getChannelInfo(channel);

	if (!theBotInfo.masterInfo.checkRepeats || !chanInfo.checkRepeats )
		return false;

	unsigned int autoSpamSizeLimit = 10;

	// TODO, check the time on this
	if (chanInfo.lastHostMask.size() || chanInfo.lastMessage.size())
	{
		if ( chanInfo.lastHostMask == hostmask && chanInfo.lastMessage == message && message.size() > autoSpamSizeLimit )
			return true; // it's the same thing, fromt he same person and its long it's spam
	}

	// it was difrent, save it
	chanInfo.lastHostMask = hostmask;
	chanInfo.lastMessage = message;

	if (chanInfo.userInfo.find(hostmask) == chanInfo.userInfo.end())
	{
		trUserInfo temp;
		chanInfo.userInfo[hostmask] = temp;
	}

	trUserInfo &userInfo = chanInfo.userInfo[hostmask];

	userInfo.history.push_back(message);
	if ( userInfo.history.size() > chatToKeep )
		userInfo.history.erase(userInfo.history.begin());

	// find the smallest number to check;
	unsigned int itemsToCheck = chatToKeep;
	if ( itemsToCheck > theBotInfo.masterInfo.repeatLimits )
		itemsToCheck = theBotInfo.masterInfo.repeatLimits;
	if ( itemsToCheck > chanInfo.repeatLimits )
		itemsToCheck = chanInfo.repeatLimits;

	if(userInfo.history.size() < itemsToCheck )
		return false;

	bool same = true;
	for ( unsigned int i = 1; i < itemsToCheck && same; i++ )
	{
		// if one is dif, then we are done
		std::string lineToCheck = userInfo.history[userInfo.history.size()-i-1];
		if ( message != lineToCheck ) 
			same = false;
	}

	return same;
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

bool banOnOffense ( std::string &channel, std::string &user, unsigned int shitFactor )
{
	if (!theBotInfo.masterInfo.kickBeforeBan)
		return true;
	
	if ( shitFactor >= theBotInfo.masterInfo.banTolerance )
		return true;

	trChannelInfo &chanInfo = getChannelInfo(channel);

	if (!chanInfo.kickBeforeBan)
		return true;

	if ( shitFactor >= chanInfo.banTolerance )
		return true;

	return false;
}

void checkForSpam ( std::string &channel, std::string &from, std::string &message )
{
	bool spam = false;
	std::string hostmask = client.getUserManager().getUserHost(from);
	trChannelInfo &chanInfo = getChannelInfo(channel);

	if (chanInfo.hibernate)
		return;

	if (checkForRepeatedMessages(channel,hostmask,message))
		spam = true;
	else if (checkForSpamFilter(channel,from,message))
		spam = true;
	else if (checkForSpamHost(channel,hostmask))
		spam = true;
	
	if (spam)
	{
		if(chanInfo.offsenseList.find(hostmask) == chanInfo.offsenseList.end())
			chanInfo.offsenseList[hostmask] = 0;

		chanInfo.offsenseList[hostmask]++;

		if (banOnOffense(channel,from,chanInfo.offsenseList[hostmask]))
			client.ban(hostmask,channel);
		client.kick(from,channel,std::string("spaming"));
	}
}

void userJoin( trJoinEventInfo *info )
{
	if (!info->channel.size() && !info->user.size())
		return;
	
	std::string hostmask = client.getUserManager().getUserHost(info->user);
	trChannelInfo	&chanInfo = getChannelInfo(info->channel);

	if(chanInfo.offsenseList.find(hostmask) == chanInfo.offsenseList.end())
		chanInfo.offsenseList[hostmask] = 0;

	if (checkForSpamHost(info->channel,hostmask ))
	{
		chanInfo.offsenseList[hostmask]++;
		if (banOnOffense(info->channel,info->user,chanInfo.offsenseList[hostmask]))
			client.ban(hostmask,info->channel);

		client.kick(info->user,info->channel,std::string("spamHost"));
	}
}

bool isWhiteList ( std::string nick, std::string hostmask )
{
	nick = string_util::tolower(nick);
	hostmask = string_util::tolower(hostmask);

	for ( unsigned int i = 0; i < theBotInfo.masterInfo.whiteNicks.size(); i++ )
	{
		if ( nick == string_util::tolower(theBotInfo.masterInfo.whiteNicks[i]) )
			return true;
	}

	for ( unsigned int i = 0; i < theBotInfo.masterInfo.whiteHosts.size(); i++ )
	{
		if ( hostmask == string_util::tolower(theBotInfo.masterInfo.whiteHosts[i]) )
			return true;
	}

	return false;
}

bool isWhiteList ( std::string channel, std::string nick, std::string hostmask )
{
	trChannelInfo	&chanInfo = getChannelInfo(channel);

	nick = string_util::tolower(nick);
	hostmask = string_util::tolower(hostmask);

	for ( unsigned int i = 0; i < chanInfo.whiteNicks.size(); i++ )
	{
		if ( nick == string_util::tolower(chanInfo.whiteNicks[i]) )
			return true;
	}

	for ( unsigned int i = 0; i < chanInfo.whiteHosts.size(); i++ )
	{
		if ( hostmask == string_util::tolower(chanInfo.whiteHosts[i]) )
			return true;
	}

	return false;
}

bool isMaster ( std::string name )
{
	name = string_util::tolower(name);
	for (unsigned int i = 0; i < theBotInfo.masterInfo.masters.size(); i++)
	{
		if ( name == theBotInfo.masterInfo.masters[i].master)
			return true;
	}
	
	return false;
}

bool isChannelMaster ( std::string &channel, std::string name, std::string hostmask )
{
	name = string_util::tolower(name);
	trChannelInfo	&chanInfo = getChannelInfo(channel);

	for (unsigned int i = 0; i < chanInfo.masters.size(); i++)
	{
		if ( name == chanInfo.masters[i].master && hostmask == chanInfo.masters[i].hostmask )
			return true;
	}

	return false;
}

bool isMasterVerified ( std::string name, std::string hostmask )
{
	name = string_util::tolower(name);

	for (unsigned int i = 0; i < theBotInfo.masterInfo.masters.size(); i++)
	{
		if (name == theBotInfo.masterInfo.masters[i].master)
		{
			if(theBotInfo.masterInfo.masters[i].verification != eVerified)
				return false;

			if(theBotInfo.masterInfo.masters[i].hostmask != hostmask)
				return false;

			return true;
		}
	}
	return false;
}

void verifyMaster ( std::string name, std::string hostmask )
{
	name = string_util::tolower(name);

	for (unsigned int i = 0; i < theBotInfo.masterInfo.masters.size(); i++)
	{
		if (name == theBotInfo.masterInfo.masters[i].master)
		{
			theBotInfo.masterInfo.masters[i].hostmask = hostmask;
			theBotInfo.masterInfo.masters[i].verification = eVerified;
		}
	}
}

void unverifyMaster ( std::string name )
{
	name = string_util::tolower(name);

	for (unsigned int i = 0; i < theBotInfo.masterInfo.masters.size(); i++)
	{
		if (name == theBotInfo.masterInfo.masters[i].master)
		{
			theBotInfo.masterInfo.masters[i].hostmask = "";
			theBotInfo.masterInfo.masters[i].verification = eUnverified;
		}
	}
}

bool checkMaster ( std::string name, std::string hostmask, std::string reply )
{
	name = string_util::tolower(name);

	for (unsigned int i = 0; i < theBotInfo.masterInfo.masters.size(); i++)
	{
		if (name == theBotInfo.masterInfo.masters[i].master)
		{
			if (!theBotInfo.masterInfo.masters[i].hostmask.size())
			{
				if(theBotInfo.masterInfo.masters[i].verification != eVerified)
				{
					client.sendMessage(reply,"unverified");
					return false;
				}

				if(theBotInfo.masterInfo.masters[i].hostmask != hostmask)
				{
					client.sendMessage(reply,"hostmask does not match verification");
					return false;
				}
			}
			else
			{
				if(theBotInfo.masterInfo.masters[i].hostmask != hostmask)
				{
					client.sendMessage(reply,"hostmask does not match verification");
					return false;
				}
			}

			return true;
		}
	}
	client.sendMessage(reply,"You're not the boss of me");
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

bool isExempt ( std::string channel, std::string name, std::string hostmask )
{
	if (isWhiteList(name,hostmask) || isWhiteList(channel,name,hostmask))
		return true; // in a whitelist

	if ( isMasterVerified(name,hostmask) || isChannelMaster(channel,name,hostmask))
		return true;

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
		// see if there is a command Handler for it
		if (!callBotCommand(command,info->source,info->from,info))
		{
			// it's not, we dono what it is, but it was addressed to us
			std::string	dono = getRandomString(theBotInfo.unknownResponces);

			dono = string_util::replace_all(dono,"%u",info->from);
			dono = string_util::replace_all(dono,"%c",info->target);
			dono = string_util::replace_all(dono,"%b",client.getNick());

			client.sendMessage(info->target,dono);
		}
	}
	else
	{
		std::string hostmask = client.getUserManager().getUserHost(info->from);

		if (!isExempt(info->target,info->from,hostmask))
			checkForSpam(info->target,info->from,info->message);
	}
}

void privateMessage ( trMessageEventInfo *info )
{
	IRCCommandINfo	commandInfo;
	commandInfo.target = info->target;

	std::string command = string_util::tolower(info->params[0]);

	// its for me, it's allways for me here
	// see if there is a command Handler for it

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

			client.sendMessage(info->from, dono);
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
		client.disconnect("Quitting");
	}

	return 0;
}

class quitCommand : public botCommandHandler
{
public:
	quitCommand() {name = "quit";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg = false );
};

bool quitCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo, bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	part = true;
	return true;
}

class flushCommand : public botCommandHandler
{
public:
	flushCommand() {name = "flush";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool flushCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	saveConfig();

	std::string message = "Ok, " + from;
	client.sendMessage(respondTo,message);

	return true;
}

class rawCommand : public botCommandHandler
{
public:
	rawCommand() {name = "raw";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool rawCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	int	paramOffset = privMsg ? 0 : 1;

	std::string text = info->getAsString(1+paramOffset);
	client.sendTextToServer(text);
	return true;
}

class channelsCommand : public botCommandHandler
{
public:
	channelsCommand() {name = "channels";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool channelsCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

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

class partCommand : public botCommandHandler
{
public:
	partCommand() {name = "part";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool partCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

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

class joinCommand : public botCommandHandler
{
public:
	joinCommand() {name = "join";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool joinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

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

class permaJoinCommand : public botCommandHandler
{
public:
	permaJoinCommand() {name = "permajoin";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool permaJoinCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string joinTarget;

	if (privMsg)
	{
		if ( info->params.size()<2)
			client.sendMessage(respondTo,"Usage: permajoin SOME_CHANNEL");
		else
			joinTarget = info->params[1];
	}
	else
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: permajoin SOME_CHANNEL");
		else
			joinTarget = info->params[2];
	}

	if (joinTarget.size())
	{
		std::string message = "Ok, " + from;
		client.sendMessage(respondTo,message);
		if (client.join(joinTarget))
			theBotInfo.startupChannels.push_back(joinTarget);
	}
	return true;
}

class addmasterCommand : public botCommandHandler
{
public:
	addmasterCommand() {name = "addmaster";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addmasterCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

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

		trMaster master;
		master.hostmask = client.getUserManager().getUserHost(newMaster);
		master.verification = eUnverified;
		master.master = string_util::tolower(newMaster);
		theBotInfo.masterInfo.masters.push_back(master);
	}
	return true;
}

class helpCommand : public botCommandHandler
{
public:
	helpCommand() {name = "help";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool helpCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	std::string helpTopic;

	if (privMsg)
	{
		if ( info->params.size()>1)
			helpTopic = info->params[1];
	}
	else
	{
		if ( info->params.size()>2)
			helpTopic = info->params[2];
	}

	if (!helpTopic.size())
	{
		tmBotCommandHandlerMap::iterator itr = botCommands.begin();
		std::string message = "Current commands;";

		while (itr != botCommands.end())
		{
			message += std::string(" ") + itr->first;
			itr++;
		}
		
		client.sendMessage(respondTo,message);

		return true;
	}

	helpTopic = string_util::tolower(helpTopic);

	if (botCommands.find(helpTopic)== botCommands.end())
		client.sendMessage(respondTo,std::string("Command Not Found"));
	else
		botCommands[helpTopic]->help(respondTo,privMsg);

	return true;
}

class kickCommand : public botCommandHandler
{
public:
	kickCommand() {name = "kick";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool kickCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{	
	if (privMsg)
		return true;

	std::string hostmask = client.getUserManager().getUserHost(from);
	if (!isChannelMaster(respondTo,from,hostmask))
	{
		if(!checkMaster(from,hostmask,respondTo))
			return true;
	}

	std::string bastard;
	std::string channel;
	std::string reason;


	if ( info->params.size()<3)
		client.sendMessage(respondTo,"Usage: kick SOME_POOR_BASTARD SOME_LAME_REASON");
	else
	{
		bastard = info->params[2];
		reason = info->getAsString(3);
	}

	if (bastard.size())
	{
		client.sendMessage(respondTo,std::string("OK ") + from);

		client.kick(bastard,respondTo,reason);
	}
	return true;
}

class addSpamCommand : public botCommandHandler
{
public:
	addSpamCommand() {name = "addspam";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: addspam SOME TEXT"));
		client.sendMessage(respondTo,std::string("Adds chat text to watch for action"));
		return true;	
	};
};

bool addSpamCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string text;

	if (privMsg)
	{
		if ( info->params.size()<1)
			client.sendMessage(respondTo,"Usage: addspam SOME TEXT");
		else
		{
			text = info->getAsString(1);
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

class addSpamHostCommand : public botCommandHandler
{
public:
	addSpamHostCommand() {name = "addspamhost";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: addspamhost SOME TEXT"));
		client.sendMessage(respondTo,std::string("Adds a hostmask to watch for action"));
		return true;	
	};
};

bool addSpamHostCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string text;

	if (privMsg)
	{
		if ( info->params.size()<1)
			client.sendMessage(respondTo,"Usage: addspamhost SOME TEXT");
		else
		{
			text = info->getAsString(1);
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

class masterVerifyCommand : public botCommandHandler
{
public:
	masterVerifyCommand() {name = "mverify";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool masterVerifyCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
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
			client.sendMessage(respondTo,"Usage: mverify the_password");
		else
		{
			if (info->getAsString(1) == theBotInfo.password)
			{
				verifyMaster(from,client.getUserManager().getUserHost(from));
				client.sendMessage(respondTo,"verified");
			}
		}
	}
	return true;
}

bool setOpt ( trChannelInfo &chanInfo, std::string &option, std::string value )
{
	value = string_util::tolower(value);
	bool valueB = false;

	if ( value == "0" || value == "off" || value == "disable" || value == "false" || value == "f")
		valueB = false;
	if ( value == "1" || value == "on" || value == "enable" || value == "true" || value == "t")
		valueB = true;

	option = string_util::tolower(option);

	if ( option == "repeatcheck" )
	{
		chanInfo.checkRepeats = valueB;
		chanInfo.repeatLimits = 3;
	}
	else if ( option == "filtercheck" )
		chanInfo.fiterMessages = valueB;
	else if ( option == "hostcheck" )
		chanInfo.filterHosts = valueB;
	else if ( option == "bantol" )
		chanInfo.banTolerance = atoi(value.c_str());
	else if ( option == "kickb4ban" )
		chanInfo.kickBeforeBan = valueB;
	else if ( option == "hibernate" )
		chanInfo.hibernate = valueB;
	else
		return false;

	if ( chanInfo.banTolerance  < 1 )
		chanInfo.banTolerance = 1;

	return true;
}

class setOptCommand : public botCommandHandler
{
public:
	setOptCommand() {name = "setopt";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: setopt SOME_OPTION SOME_VALUE"));
		client.sendMessage(respondTo,std::string("Valid Options: kickb4ban, repeatcheck, filtercheck, hostcheck, bantol"));
		client.sendMessage(respondTo,std::string("Valid Values: 0, Off, False, 1, On, True"));
		return true;	
	};
};

bool setOptCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string option;
	std::string value;
	bool		valueB = false;

	if (privMsg)
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: setopt SOME_OPTION SOME_VALUE");
		else
		{
			option = info->params[1];
			value = info->params[2];
		}
	}
	else
	{
		if ( info->params.size()<4)
			client.sendMessage(respondTo,"Usage: setopt SOME_OPTION SOME_VALUE");
		else{
			option = info->params[2];
			value = info->params[3];
		}
	}

	if(!setOpt(theBotInfo.masterInfo,option,value))
	{
		client.sendMessage(respondTo,std::string("Unknown option ") + option);
		return true;
	}

	client.sendMessage(respondTo,std::string("OK ") + from);

	return true;
}

bool setWhiteList ( trChannelInfo &chanInfo, std::string &command, std::string &value )
{
	if ( command == "nick)" )
		chanInfo.whiteNicks.push_back(value);
	else if ( command == "host" )
		chanInfo.whiteHosts.push_back(value);
	else if ( command == "nick-" )
	{
		string_list::iterator itr = chanInfo.whiteNicks.begin();
		while (itr != chanInfo.whiteNicks.end())
		{
			if (*itr == value)
				itr == chanInfo.whiteNicks.erase(itr);
			else
				itr++;
		}
	}
	else if ( command == "host-" )
	{
		string_list::iterator itr = chanInfo.whiteHosts.begin();
		while (itr != chanInfo.whiteHosts.end())
		{
			if (*itr == value)
				itr == chanInfo.whiteHosts.erase(itr);
			else
				itr++;
		}
	}
	else
		return false;

	return true;
}

class whiteListCommand : public botCommandHandler
{
public:
	whiteListCommand() {name = "whitelist";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: whitelist SOME_OPTION SOME_VALUE"));
		client.sendMessage(respondTo,std::string("Valid Options: nick, host, nick-, host-"));
		client.sendMessage(respondTo,std::string("Valid Values: a nick or a host"));
		return true;	
	};
};

bool whiteListCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string option;
	std::string value;

	if (privMsg)
	{
		if ( info->params.size()<3)
			client.sendMessage(respondTo,"Usage: whitelist SOME_OPTION SOME_VALUE");
		else
		{
			option = info->params[1];
			value = info->params[2];
		}
	}
	else
	{
		if ( info->params.size()<4)
			client.sendMessage(respondTo,"Usage: whitelist SOME_OPTION SOME_VALUE");
		else{
			option = info->params[2];
			value = info->params[3];
		}
	}

	value = string_util::tolower(value);
	option = string_util::tolower(option);

	if (!setWhiteList ( theBotInfo.masterInfo, option, value ))
	{
		client.sendMessage(respondTo,std::string("Unknown option ") + option);
		return true;
	}

	writeDatabase();

	client.sendMessage(respondTo,std::string("OK ") + from);

	return true;
}

class setChanOptCommand : public botCommandHandler
{
public:
	setChanOptCommand() {name = "setchanopt";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: setChanOpt SOME_OPTION SOME_VALUE"));
		client.sendMessage(respondTo,std::string("Valid Options: hibernate, kickb4ban, repeatcheck, filtercheck, hostcheck, bantol"));
		client.sendMessage(respondTo,std::string("Valid Values: 0, Off, False, 1, On, True"));
		client.sendMessage(respondTo,std::string("Only Sets the optiosn for this channel"));
		return true;	
	};
};

bool setChanOptCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (privMsg)
		return false;

	// has to be a channel master or a master

	std::string hostmask = client.getUserManager().getUserHost(from);
	if (!isChannelMaster(respondTo,from,hostmask))
	{
		if(!checkMaster(from,hostmask,respondTo))
			return true;
	}

	std::string option;
	std::string value;
	bool		valueB = false;

	trChannelInfo &chanInfo = getChannelInfo(respondTo);

	if ( info->params.size()<4)
		client.sendMessage(respondTo,"Usage: setopt SOME_OPTION SOME_VALUE");
	else
	{
		option = info->params[2];
		value = info->params[3];
	}

	if(!setOpt(chanInfo,option,value))
	{
		client.sendMessage(respondTo,std::string("Unknown option ") + option);
		return true;
	}

	client.sendMessage(respondTo,std::string("OK ") + from);
	saveConfig();

	return true;
}

class addChanMasterCommand : public botCommandHandler
{
public:
	addChanMasterCommand() {name = "addChanMaster";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
};

bool addChanMasterCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (!checkMaster(from,client.getUserManager().getUserHost(from),respondTo))
		return true;

	std::string newMaster;

	if (privMsg)
		return true;

	if ( info->params.size()<3)
		client.sendMessage(respondTo,"Usage: addChanMaster SOME_DUDE");
	else
		newMaster = info->params[2];

	trChannelInfo &chanInfo = getChannelInfo(respondTo);

	if (newMaster.size())
	{
		std::string message = "Ok, " + from + ", user, " + newMaster + " added to channel masters list";
		client.sendMessage(respondTo,message);

		trMaster master;
		master.hostmask = client.getUserManager().getUserHost(newMaster);
		master.verification = eUnverified;
		master.master = string_util::tolower(newMaster);
		chanInfo.masters.push_back(master);

		saveConfig();
	}
	return true;
}

class chanWhiteListCommand : public botCommandHandler
{
public:
	chanWhiteListCommand() {name = "chanwhitelist";}
	bool command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg = false );
	virtual bool help ( std::string respondTo, bool privMsg = false )
	{
		client.sendMessage(respondTo,std::string("Usage: chanWhiteListCommand SOME_OPTION SOME_VALUE"));
		client.sendMessage(respondTo,std::string("Valid Options: nick, host, nick-, host-"));
		client.sendMessage(respondTo,std::string("Valid Values: a nick or a host"));
		return true;	
	};
};

bool chanWhiteListCommand::command ( std::string command, std::string source, std::string from, trMessageEventInfo *info, std::string respondTo , bool privMsg )
{
	if (privMsg)
		return true;
		

	std::string hostmask = client.getUserManager().getUserHost(from);
	if (!isChannelMaster(respondTo,from,hostmask))
	{
		if(!checkMaster(from,hostmask,respondTo))
			return true;
	}

	std::string option;
	std::string value;
	trChannelInfo &chanInfo = getChannelInfo(respondTo);

	if ( info->params.size()<4)
		client.sendMessage(respondTo,"Usage: whitelist SOME_OPTION SOME_VALUE");
	else
	{
		option = info->params[2];
		value = info->params[3];
	}

	value = string_util::tolower(value);
	option = string_util::tolower(option);

	if (!setWhiteList ( chanInfo, option, value ))
	{
		client.sendMessage(respondTo,std::string("Unknown option ") + option);
		return true;
	}

	saveConfig();

	client.sendMessage(respondTo,std::string("OK ") + from);

	return true;
}

void registerBotCommands ( void )
{
	installBotCommand(new quitCommand);
	installBotCommand(new flushCommand);
	installBotCommand(new rawCommand);
	installBotCommand(new channelsCommand);
	installBotCommand(new partCommand);
	installBotCommand(new joinCommand);
	installBotCommand(new permaJoinCommand);
	installBotCommand(new addmasterCommand);
	installBotCommand(new helpCommand);
	installBotCommand(new kickCommand);
	installBotCommand(new addSpamCommand);
	installBotCommand(new addSpamHostCommand);
	installBotCommand(new masterVerifyCommand);
	installBotCommand(new setOptCommand);
	installBotCommand(new whiteListCommand);

	// channel commands
	installBotCommand(new setChanOptCommand);
	installBotCommand(new addChanMasterCommand);
	installBotCommand(new chanWhiteListCommand);

}
