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

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "TextUtils.h"

#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
#endif

class DefaultIRCLogHandler : public IRCClientLogHandler
{
public:
	virtual ~DefaultIRCLogHandler(){return;}
	virtual void log ( IRCClient &client, int level, std::string line )
	{
		printf("log# %d:%s\n",level,line.c_str());

		if (client.getLogfile().size())
		{
			FILE *fp = fopen(client.getLogfile().c_str(),"at");

			if (fp)
			{
				fprintf(fp,"log# %d:%s\n",level,line.c_str());
				fclose(fp);
			}
		}
	}
};

DefaultIRCLogHandler	defaultLoger;

// sleep util
void IRCOSSleep ( float fTime )
{
#ifdef _WIN32
	Sleep((DWORD)(1000.0f * fTime));
#else
	usleep((unsigned int )(100000 * fTime));
#endif
}


// base message class

BaseIRCCommandInfo::BaseIRCCommandInfo()
{
	type = eUnknown;command = "NULL";
}

BaseIRCCommandInfo::~BaseIRCCommandInfo()
{
	return;
}

void BaseIRCCommandInfo::parse ( std::string line )
{
	params = string_util::tokenize(line,std::string(" "));
	raw = line;
	prefixed = line.c_str()[0] ==':';
	if (prefixed)
	{
		params[0].erase(params[0].begin());
		source = params[0];
		// pull off the source
		params.erase(params.begin());
	}
	else
		source = "HOST";

	// make sure we have a command
	if (params.size() > 0)
	{
		command = params[0];
		// pull off the command
		params.erase(params.begin());
	}
	else
		command = "NULL";

	// make sure we have a target
	if (params.size() > 0)
	{
		target = params[0];
		// pull off the command
		params.erase(params.begin());

		// pull off the :
		if (target.c_str()[0] == ':')
			target.erase(target.begin());
	}
	else
		target = "NULL";
}

std::string BaseIRCCommandInfo::getAsString ( int start, int end )
{
	return getStringFromList(params," ",start,end);
}

// IRC class stuff

IRCClient::IRCClient()
:tcpConnection(TCPConnection::instance())
{
	tcpClient = NULL;
	registerDefaultCommandhandlers();
	init();

	ircMessageTerminator = "\r\n";
	ircCommandDelimator	 = " ";
	debugLogLevel = 0;
	ircServerPort = 6667;
	ircConenctonState = eNotConnected;
	logHandler = &defaultLoger;
}

// irc client
IRCClient::~IRCClient()
{
	disconnect("shuting down");

	if (tcpClient)
		tcpConnection.deleteClientConnection(tcpClient);

//	tcpConnection.kill();
}

// general connection methods
bool IRCClient::init ( void )
{
	registered = false;
	nickname = "";
	// if any old conenctions are around, kill em
	if (tcpClient)
		tcpConnection.deleteClientConnection(tcpClient);

	tcpClient = NULL;

	// make sure the system we have is inited
	//tcpConnection.init();

	// just get us a new empty connection
	tcpClient = tcpConnection.newClientConnection("",0);
	
	if (tcpClient)
		tcpClient->addListener(this);

	return tcpClient != NULL;
}

bool IRCClient::connect ( std::string server, int port )
{
	if (!tcpClient || !server.size())
		return false;

	reportedServerHost = ircServerName = server;
	ircServerPort = 6667;
	if ( port > 0 )
		ircServerPort = (unsigned short)port;

	teTCPError err = tcpClient->connect(server,ircServerPort);

	ircConenctonState = err == eTCPNoError ? eTCPConenct : eNotConnected;

	return err == eTCPNoError;
}

bool IRCClient::disconnect ( std::string reason )
{
	if (ircConenctonState >= eLogedIn)
	{
		if (!reason.size())
			reason = "shuting down";

		IRCCommandINfo	info;
		info.params.push_back(reason);

		if (!sendIRCCommand(eCMD_QUIT,info))
		{
			log("Discoonect Failed: QUIT command not sent",0);
			return false;
		}

		channels.clear();
		userList.clear();

		teTCPError err = tcpClient->disconnect();

		ircConenctonState = eNotConnected;

		return err == eTCPNoError;
	}
	return false;
}

// update loop methods
bool IRCClient::process ( void )
{
	return tcpConnection.update()==eTCPNoError;
}

bool IRCClient::login ( std::string &nick, std::string &username, std::string &fullname)
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
	info.params.push_back(std::string("localhost"));
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
	// make sure we have a record of it
	if (channels.find(channel) == channels.end())
		return false;

// notify that we parted the channel
	removeChannelUser(channel,getNick());

	trPartEventInfo	eventInfo;

	eventInfo.eventType = eIRCChannelPartEvent;
	eventInfo.reason = reason;
	eventInfo.user = getNick();

	callEventHandler(eventInfo.eventType,eventInfo);

	channels.erase(channels.find(channel));
	return true;
}

bool IRCClient::sendMessage ( std::string target, std::string message, bool isAction )
{
	IRCCommandINfo	commandInfo;
	commandInfo.target = target;

	std::string messageToSend;

	if(isAction)
		messageToSend += (char)0x01 + std::string("ACTION ");

	messageToSend += message;

	if(isAction)
		messageToSend +=(char)0x01;

	commandInfo.params.push_back(messageToSend);
	sendIRCCommand(eCMD_PRIVMSG,commandInfo);
	return true;
}

void IRCClient::pending ( TCPClientConnection *connection, int count )
{
	// we got some data, do something with it
	log("Data Pending notification",5);
	// ok we have to parse this tuff into "lines"
	std::string	theLine = lastRecevedData;
	tvPacketList	&packets = connection->getPackets();

	while(packets.size())
	{
		TCPPacket	&packet = *(packets.begin());

		unsigned int	len;
		char* data  = (char*)packet.get(len);
		unsigned int count = 0;

		while (count < len)
		{
			if (data[count] == 13)
			{
				if (theLine.size())
				{
					processIRCLine(theLine);
					theLine = "";
				}
				if (count != len-1)
					count++;
			}
			else
			{
				if (data[count] != 10)
				{
					theLine += data[count];
				}
			}
			count++;
		}
		// save off anything left
		lastRecevedData = theLine;

		packets.erase(packets.begin());
	}
}

void IRCClient::processIRCLine ( std::string line )
{
	// we have a single line of text, do something with it.
	// see if it's a command, and or call any handlers that we have
	// also check for error returns

	// right now we don't know if it's an IRC or CTCP command so just go with the generic one
	// let the command parse it out into paramaters and find the command
	BaseIRCCommandInfo	commandInfo;
	commandInfo.parse(line);
	std::string handler;

	if (!commandInfo.prefixed)
		commandInfo.source = getServerHost();

	// call the "ALL" handler special if there is one
	handler = std::string("ALL");
	receveCommand(handler,commandInfo);

	if (atoi(commandInfo.command.c_str()) != 0) {
	  handler = std::string("NUMERIC");
	  receveCommand(handler,commandInfo);
	}

	// notify any handlers for this specific command
	receveCommand(commandInfo.command,commandInfo);
}

bool IRCClient::sendIRCCommandToServer ( teIRCCommands	command, std::string &data)
{
  std::string text = ircCommandParser.getCommandName(command) + ircCommandDelimator + data;
  return sendTextToServer(text);
}

bool IRCClient::sendCTCPCommandToServer ( teCTCPCommands	command, std::string &data)
{
  std::string text = ctcpCommandParser.getCommandName(command) + ircCommandDelimator + data;
  return sendTextToServer(text);
}

// utility methods
bool IRCClient::sendTextToServer ( std::string &text )
{
	if (!tcpClient || !tcpClient->connected())
		return false;

	std::string message = text;
	if (text.size())
	{
		message += ircMessageTerminator;
		teTCPError	error = tcpClient->sendData(message);
		if (error == eTCPNoError)
			log("Send Data:" + text,2);
		else
		{
			switch (error)
			{
				case eTCPNotInit:
					log("Send Data Error: TCP Not Initalised: data=" + text,0);
				break;

				case eTCPSocketNFG:
					log("Send Data Error: Bad Socket: data=" + text,0);
				break;

				case eTCPDataNFG:
					log("Send Data Error: Bad Data",0);
				break;

				case eTCPConnectionFailed:
					log("Send Data Error: TCP Connection failed",0);
				break;

				default:
					log("Send Data Error:Unknown Error",0);
			}
			return false;
		}
	}
	else
		return false;

	return true;
}

void	IRCClient::setLogHandler ( IRCClientLogHandler * loger )
{
	if (!loger)
		logHandler = &defaultLoger;
	else
		logHandler = loger;
}

void IRCClient::log ( const char *text, int level )
{
	log(std::string(text),level);
}

void IRCClient::log ( std::string text, int level )
{
	if (level <= debugLogLevel && logHandler)
		logHandler->log(*this,level,text);
}

void IRCClient::setLogfile ( std::string file )
{
	logfile = file;
}

std::string  IRCClient::getLogfile ( void )
{
	return logfile;
}

void IRCClient::setDebugLevel ( int level )
{
	debugLogLevel = level;
}

int IRCClient::getDebugLevel ( void )
{
	return debugLogLevel;
}

bool IRCClient::sendCommand ( std::string &commandName, BaseIRCCommandInfo &info )
{
	tmUserCommandHandlersMap::iterator		commandListItr = userCommandHandlers.find(commandName);

	bool callDefault = true;

	if (commandListItr != userCommandHandlers.end() && commandListItr->second.size())	// do we have a custom command handler
	{
		// someone has to want us to call the defalt now
		callDefault = false;
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		std::vector<IRCClientCommandHandler*>::iterator	itr = commandListItr->second.begin();
		while (itr != commandListItr->second.end())
		{
			if ( (*itr)->send(*this,commandName,info))
				callDefault = true;
			itr++;
		}
		return true;
	}

	if (callDefault)	// check for the default
	{
		tmCommandHandlerMap::iterator itr = defaultCommandHandlers.find(commandName);
		if (itr != defaultCommandHandlers.end())
		{
			itr->second->send(*this,commandName,info);
			return true;
		}
	}
	return false;
}

bool IRCClient::sendIRCCommand ( teIRCCommands	command, IRCCommandINfo &info )
{
	info.type = eIRCCommand;
	info.ircCommand = command;
	info.command = ircCommandParser.getCommandName(command);
	return sendCommand(info.command,info);
}

bool IRCClient::sendCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info )
{
	info.type = eCTCPCommand;
	info.ctcpCommand = command;
	info.command = ctcpCommandParser.getCommandName(command);
	return sendCommand(info.command,info);
}

bool IRCClient::receveCommand ( std::string &commandName, BaseIRCCommandInfo &info )
{
	tmUserCommandHandlersMap::iterator		commandListItr = userCommandHandlers.find(commandName);

	bool callDefault = true;

	if (commandListItr != userCommandHandlers.end() && commandListItr->second.size())	// do we have a custom command handler
	{
		// someone has to want us to call the defalt now
		callDefault = false;
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		std::vector<IRCClientCommandHandler*>::iterator	itr = commandListItr->second.begin();
		while (itr != commandListItr->second.end())
		{
			if ( (*itr)->receve(*this,commandName,info))
				callDefault = true;
			itr++;
		}
		if (!callDefault)
			return true;
	}

	if (callDefault)	// check for the default
	{
		tmCommandHandlerMap::iterator itr = defaultCommandHandlers.find(commandName);
		if (itr != defaultCommandHandlers.end())
		{
			itr->second->receve(*this,commandName,info);
			return true;
		}
	}
	return false;
}

bool IRCClient::receveIRCCommand ( teIRCCommands	command, IRCCommandINfo &info )
{
	info.type = eIRCCommand;
	info.ircCommand = command;
	return receveCommand(info.command,info);
}

bool IRCClient::receveCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info )
{
	info.type = eCTCPCommand;
	info.ctcpCommand = command;
	return receveCommand(info.command,info);
}

bool IRCClient::registerCommandHandler ( IRCClientCommandHandler *handler )
{
	if (!handler)
		return false;

	std::string command = handler->getCommandName();

	tmUserCommandHandlersMap::iterator		commandListItr = userCommandHandlers.find(command);
	if (commandListItr == userCommandHandlers.end())
	{
		std::vector<IRCClientCommandHandler*> handlerList;
		handlerList.push_back(handler);
		userCommandHandlers[command] = handlerList;
	}
	else
		commandListItr->second.push_back(handler);

	return true;
}

bool IRCClient::removeCommandHandler ( IRCClientCommandHandler *handler )
{
	if (!handler)
		return false;

	std::string command = handler->getCommandName();

	tmUserCommandHandlersMap::iterator		commandListItr = userCommandHandlers.find(command);
	if (commandListItr == userCommandHandlers.end())
		return false;
	else
	{
		std::vector<IRCClientCommandHandler*>::iterator	itr = commandListItr->second.begin();
		while ( itr != commandListItr->second.end())
		{
			if (*itr == handler)
				itr = commandListItr->second.erase(itr);
			else
				itr++;
		}
	}
	return true;
}

int IRCClient::listUserHandledCommands ( std::vector<std::string> &commandList )
{
	commandList.clear();

	tmUserCommandHandlersMap::iterator	itr = userCommandHandlers.begin();

	while (itr != userCommandHandlers.end())
	{
		commandList.push_back(itr->first);
		itr++;
	}
	return (int)commandList.size();
}

int IRCClient::listDefaultHandledCommands ( std::vector<std::string> &commandList )
{
	commandList.clear();

	tmCommandHandlerMap::iterator	itr = defaultCommandHandlers.begin();

	while (itr != defaultCommandHandlers.end())
	{
		commandList.push_back(itr->first);
		itr++;
	}
	return (int)commandList.size();
}

void IRCClient::addDefaultCommandhandlers ( IRCClientCommandHandler* handler )
{
	defaultCommandHandlers[handler->getCommandName()] = handler;
}

void IRCClient::clearDefaultCommandhandlers ( void )
{
	tmCommandHandlerMap::iterator	itr = defaultCommandHandlers.begin();

	while (itr != defaultCommandHandlers.end())
	{
		delete(itr->second);
		itr++;
	}
	defaultCommandHandlers.clear();
}

void IRCClient::registerDefaultCommandhandlers ( void )
{
	registerDefaultEventHandlers();

	userCommandHandlers.clear();
	clearDefaultCommandhandlers();

	// the "special" handlers
	addDefaultCommandhandlers(new IRCALLCommand );
	addDefaultCommandhandlers(new IRCNumericCommand );

	// basic IRC commands
	addDefaultCommandhandlers(new IRCNickCommand );
	addDefaultCommandhandlers(new IRCUserCommand );
	addDefaultCommandhandlers(new IRCPingCommand );
	addDefaultCommandhandlers(new IRCPongCommand );
	addDefaultCommandhandlers(new IRCNoticeCommand );
	addDefaultCommandhandlers(new IRCJoinCommand );
	addDefaultCommandhandlers(new IRCPartCommand );
	addDefaultCommandhandlers(new IRCQuitCommand );
	addDefaultCommandhandlers(new IRCModeCommand );
	addDefaultCommandhandlers(new IRCPrivMsgCommand );
}

// logical event handlers

//tmIRCEventMap							defaultEventHandlers;
//tmIRCEventListMap					userEventHandlers;

void IRCClient::addDefaultEventHandlers ( teIRCEventType eventType, IRCBasicEventCallback* handler )
{
	if (handler)
		defaultEventHandlers[eventType] = handler;
}

void IRCClient::clearDefaultEventHandlers ( void )
{
	tmIRCEventMap::iterator itr = defaultEventHandlers.begin();

	while ( itr != defaultEventHandlers.end())
	{
		if (itr->second && (itr->second != this) )
			delete(itr->second);
		itr++;
	}
	defaultEventHandlers.clear();
}

void IRCClient::registerDefaultEventHandlers ( void )
{
	userEventHandlers.clear();
	clearDefaultEventHandlers();

	addDefaultEventHandlers(eIRCNickNameError,this);
}

bool IRCClient::registerEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler )
{
	if (!handler)
		return false;

	tmIRCEventListMap::iterator		eventListItr = userEventHandlers.find(eventType);
	if (eventListItr == userEventHandlers.end())
	{
		tvIRCEventList handlerList;
		handlerList.push_back(handler);
		userEventHandlers[eventType] = handlerList;
	}
	else
		eventListItr->second.push_back(handler);

	return true;
}

bool IRCClient::removeEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler )
{
	if (!handler)
		return false;

	tmIRCEventListMap::iterator		eventListItr = userEventHandlers.find(eventType);
	if (eventListItr == userEventHandlers.end())
		return false;
	else
	{
		tvIRCEventList::iterator	itr = eventListItr->second.begin();
		while ( itr != eventListItr->second.end())
		{
			if ((*itr)== handler)
				itr = eventListItr->second.erase(itr);
			else
				itr++;
		}
	}
	return true;
}

void IRCClient::callEventHandler ( teIRCEventType eventType, trBaseEventInfo &info )
{
	bool callDefault = true;

	tmIRCEventListMap::iterator		eventListItr = userEventHandlers.find(eventType);

	// make sure the event type is cool
	info.eventType = eventType;

	if (eventListItr != userEventHandlers.end() && eventListItr->second.size())	// do we have a custom command handler
	{
		// someone has to want us to call the defalt now
		callDefault = false;
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		tvIRCEventList::iterator	itr = eventListItr->second.begin();
		while (itr != eventListItr->second.end())
		{
			if ( (*itr)->process(*this,eventType,info))
				callDefault = true;
			itr++;
		}
		if (!callDefault)
			return;
	}

	if (callDefault)	// check for the default
	{
		tmIRCEventMap::iterator itr = defaultEventHandlers.find(eventType);
		if (itr != defaultEventHandlers.end())
		{
			itr->second->process(*this,eventType,info);
			return;
		}
	}
	return;
}

// user management
trIRCUser& IRCClient::getUserRecord ( std::string name )
{
	if (name.c_str()[0] == '@')
		name.erase(0,1);

	tvIRCUserMap::iterator	itr = userList.find(name);
	if (itr == userList.end())
	{
		trIRCUser	user;
		user.nick = name;
		return userList[name] = user;
	}
	return userList[name];
}

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
		channels[info.target].join(&(getUserRecord(who)));
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
	if (who == getNick())	// we joined a channel
	{	
		IRCChannel	channel;
		channel.setName(info.target);
		channels[channel.getName()] = channel;

		partInfo.eventType = eIRCChannelPartEvent;
	}
	else	// someone else joined a channel we are in
	{
		channels[info.target].part(&(getUserRecord(who)));
		partInfo.eventType = eIRCUserPartEvent;
	}	

	partInfo.channel = info.target;
	partInfo.user = who;
	callEventHandler(partInfo.eventType,partInfo);
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

void IRCClient::addChannelUsers ( std::string channel, string_list newUsers )
{
	string_list::iterator	itr = newUsers.begin();
	while ( itr != newUsers.end() )
	{
		channels[channel].join(&(getUserRecord(*itr)));
		itr++;
	}
}

bool IRCClient::removeChannelUser ( std::string channel, std::string name )
{
	if (name == getNick())
		return false;

	channels[channel].part(&(getUserRecord(name)));
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

// info methods

string_list IRCClient::listUsers ( std::string channel )
{
	string_list userNames;

	if (channels.find(channel) != channels.end())
	{
		return channels.find(channel)->second.listUsers();
	}
	else
	{
		tvIRCUserMap::iterator	itr = userList.begin();
		while (itr != userList.end())
		{
			userNames.push_back(itr->second.nick);
			itr++;
		}
	}
	return userNames;
}

string_list IRCClient::listChanels ( void )
{
	string_list	chanList;

	tmChannelMap::iterator itr = channels.begin();

	while ( itr != channels.end() )
	{
		chanList.push_back(itr->first);
		itr++;
	}
	return chanList;
}

// default event handling

bool IRCClient::process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info )
{
	switch (eventType)
	{
		case eIRCNickNameError:
		{
			// atempt to keep adding crap to the nick till it goes
			requestedNick += '_';

			IRCCommandINfo	info;
			info.params.push_back(requestedNick);

			if (!sendIRCCommand(eCMD_NICK,info))
			{
				log("Nick Error Resned Failed: NICK command not sent",0);
				return false;
			}
			if (getConnectionState() < eSentNickAndUSer)
				setConnectionState(eSentNickAndUSer);
		}
		break;
	}
	return true;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
