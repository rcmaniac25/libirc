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

class DefaultIRCLogHandaler : public IRCClientLogHandaler
{
public:
	virtual ~DefaultIRCLogHandaler(){return;}
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

DefaultIRCLogHandaler	defaultLoger;

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

std::string BaseIRCCommandInfo::getAsString ( int pos )
{
	std::vector<std::string>::iterator itr = params.begin() + pos;

	std::string  temp;

	while ( itr != params.end())
	{
		temp += *itr;
		itr++;
		if (itr != params.end())
			temp += " ";
	}
	return temp;
}

// IRC class stuff

IRCClient::IRCClient()
:tcpConnection(TCPConnection::instance())
{
	tcpClient = NULL;
	registerDefaultCommandhandalers();
	init();

	ircMessageTerminator = " \r\n";
	ircCommandDelimator	 = " ";
	debugLogLevel = 0;
	ircServerPort = 6667;
	ircConenctonState = eNotConnected;
	logHandaler = &defaultLoger;
}

// irc client
IRCClient::~IRCClient()
{
	tcpConnection.kill();
}

// general connection methods
bool IRCClient::init ( void )
{
	nickname = "";
	// if any old conenctions are around, kill em
	if (tcpClient)
		tcpConnection.deleteClientConnection(tcpClient);

	tcpClient = NULL;

	// make sure the system we have is inited
	tcpConnection.init();

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

bool IRCClient::disconnect ( void )
{
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
	info.params.push_back(channel);

	if (!sendIRCCommand(eCMD_JOIN,info))
	{
		log("Goin Failed: JOIN command not sent",0);
		return false;
	}

	if (!sendIRCCommand(eCMD_MODE,info))
	{
		log("Goin Failed: MODE command not sent",0);
		return false;
	}

	return true;
}

// sending commands
bool IRCClient::send ( std::string command, std::string target, std::string data )
{
	return false;
}

bool IRCClient::send ( std::string &command, BaseIRCCommandInfo &info )
{
	return false;
}

bool IRCClient::sendRaw ( std::string data )
{
	return false;
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
	// see if it's a command, and or call any handalers that we have
	// also check for error returns

	// right now we don't know if it's an IRC or CTCP command so just go with the generic one
	// let the command parse it out into paramaters and find the command
	BaseIRCCommandInfo	commandInfo;
	commandInfo.parse(line);

	if (!commandInfo.prefixed)
		commandInfo.source = getServerHost();

	// call the "ALL" handaler special if there is one
	receveCommand(std::string("ALL"),commandInfo);

	if (atoi(commandInfo.command.c_str()) != 0)
		receveCommand(std::string("NUMERIC"),commandInfo);

	// notify any handalers for this specific command
	receveCommand(commandInfo.command,commandInfo);
}


bool IRCClient::sendIRCCommandToServer ( teIRCCommands	command, std::string &data)
{
	return sendTextToServer(ircCommandParser.getCommandName(command) + ircCommandDelimator + data);
}

bool IRCClient::sendCTCPCommandToServer ( teCTCPCommands	command, std::string &data)
{
	return sendTextToServer(ctcpCommandParser.getCommandName(command) + ircCommandDelimator + data);
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

void	IRCClient::setLogHandaler ( IRCClientLogHandaler * loger )
{
	if (!loger)
		logHandaler = &defaultLoger;
	else
		logHandaler = loger;
}

void IRCClient::log ( const char *text, int level )
{
	log(std::string(text),level);
}

void IRCClient::log ( std::string &text, int level )
{
	if (level <= debugLogLevel && logHandaler)
		logHandaler->log(*this,level,text);
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
	tmUserCommandHandalersMap::iterator		commandListItr = userCommandHandalers.find(commandName);

	bool callDefault = true;

	if (commandListItr != userCommandHandalers.end() && commandListItr->second.size())	// do we have a custom command handaler
	{
		// someone has to want us to call the defalt now
		callDefault = false;
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		std::vector<IRCClientCommandHandaler*>::iterator	itr = commandListItr->second.begin();
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
		tmCommandHandalerMap::iterator itr = defaultCommandHandalers.find(commandName);
		if (itr != defaultCommandHandalers.end())
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
	tmUserCommandHandalersMap::iterator		commandListItr = userCommandHandalers.find(commandName);

	bool callDefault = true;

	if (commandListItr != userCommandHandalers.end() && commandListItr->second.size())	// do we have a custom command handaler
	{
		// someone has to want us to call the defalt now
		callDefault = false;
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		std::vector<IRCClientCommandHandaler*>::iterator	itr = commandListItr->second.begin();
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
		tmCommandHandalerMap::iterator itr = defaultCommandHandalers.find(commandName);
		if (itr != defaultCommandHandalers.end())
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

bool IRCClient::registerCommandHandaler ( IRCClientCommandHandaler *handaler )
{
	if (!handaler)
		return false;

	std::string command = handaler->getCommandName();

	tmUserCommandHandalersMap::iterator		commandListItr = userCommandHandalers.find(command);
	if (commandListItr == userCommandHandalers.end())
	{
		std::vector<IRCClientCommandHandaler*> handalerList;
		handalerList.push_back(handaler);
		userCommandHandalers[command] = handalerList;
	}
	else
		commandListItr->second.push_back(handaler);

	return true;
}

bool IRCClient::removeCommandHandaler ( IRCClientCommandHandaler *handaler )
{
	if (!handaler)
		return false;

	std::string command = handaler->getCommandName();

	tmUserCommandHandalersMap::iterator		commandListItr = userCommandHandalers.find(command);
	if (commandListItr == userCommandHandalers.end())
		return false;
	else
	{
		std::vector<IRCClientCommandHandaler*>::iterator	itr = commandListItr->second.begin();
		while ( itr != commandListItr->second.end())
		{
			if (*itr == handaler)
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

	tmUserCommandHandalersMap::iterator	itr = userCommandHandalers.begin();

	while (itr != userCommandHandalers.end())
	{
		commandList.push_back(itr->first);
		itr++;
	}
	return (int)commandList.size();
}

int IRCClient::listDefaultHandledCommands ( std::vector<std::string> &commandList )
{
	commandList.clear();

	tmCommandHandalerMap::iterator	itr = defaultCommandHandalers.begin();

	while (itr != defaultCommandHandalers.end())
	{
		commandList.push_back(itr->first);
		itr++;
	}
	return (int)commandList.size();
}

void IRCClient::addDefaultCommandhandalers ( IRCClientCommandHandaler* handaler )
{
	defaultCommandHandalers[handaler->getCommandName()] = handaler;
}

void IRCClient::clearDefaultCommandhandalers ( void )
{
	tmCommandHandalerMap::iterator	itr = defaultCommandHandalers.begin();

	while (itr != defaultCommandHandalers.end())
	{
		delete(itr->second);
		itr++;
	}
	defaultCommandHandalers.clear();
}

void IRCClient::registerDefaultCommandhandalers ( void )
{
	registerDefaultEventHandalers();

	userCommandHandalers.clear();
	clearDefaultCommandhandalers();

	// the "special" handalers
	addDefaultCommandhandalers(new IRCALLCommand );
	addDefaultCommandhandalers(new IRCNumericCommand );

	// basic IRC commands
	addDefaultCommandhandalers(new IRCNickCommand );
	addDefaultCommandhandalers(new IRCUserCommand );
	addDefaultCommandhandalers(new IRCPingCommand );
	addDefaultCommandhandalers(new IRCPongCommand );
	addDefaultCommandhandalers(new IRCNoticeCommand );
	addDefaultCommandhandalers(new IRCJoinCommand );
	addDefaultCommandhandalers(new IRCModeCommand );
}

// logical event handalers

//tmIRCEventMap							defaultEventHandalers;
//tmIRCEventListMap					userEventHandalers;

void IRCClient::addDefaultEventHandalers ( teIRCEventType eventType, IRCBasicEventCallback* handaler )
{
	if (handaler)
		defaultEventHandalers[eventType] = handaler;
}

void IRCClient::clearDefaultEventHandalers ( void )
{
	tmIRCEventMap::iterator itr = defaultEventHandalers.begin();

	while ( itr != defaultEventHandalers.end())
	{
		if (itr->second)
			delete(itr->second);
		itr++;
	}
	defaultEventHandalers.clear();
}

void IRCClient::registerDefaultEventHandalers ( void )
{
	userEventHandalers.clear();
	clearDefaultEventHandalers();

}

bool IRCClient::registerEventHandaler ( teIRCEventType eventType, IRCBasicEventCallback *handaler )
{
	if (!handaler)
		return false;

	tmIRCEventListMap::iterator		eventListItr = userEventHandalers.find(eventType);
	if (eventListItr == userEventHandalers.end())
	{
		tvIRCEventList handalerList;
		handalerList.push_back(handaler);
		userEventHandalers[eventType] = handalerList;
	}
	else
		eventListItr->second.push_back(handaler);

	return true;
}

bool IRCClient::removeEventHandaler ( teIRCEventType eventType, IRCBasicEventCallback *handaler )
{
	if (!handaler)
		return false;

	tmIRCEventListMap::iterator		eventListItr = userEventHandalers.find(eventType);
	if (eventListItr == userEventHandalers.end())
		return false;
	else
	{
		tvIRCEventList::iterator	itr = eventListItr->second.begin();
		while ( itr != eventListItr->second.end())
		{
			if ((*itr)== handaler)
				itr = eventListItr->second.erase(itr);
			else
				itr++;
		}
	}
	return true;
}

void IRCClient::callEventHandaler ( teIRCEventType eventType, trBaseEventInfo &info )
{
	bool callDefault = true;

	tmIRCEventListMap::iterator		eventListItr = userEventHandalers.find(eventType);

	// make sure the event type is cool
	info.eventType = eventType;

	if (eventListItr != userEventHandalers.end() && eventListItr->second.size())	// do we have a custom command handaler
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
		tmIRCEventMap::iterator itr = defaultEventHandalers.find(eventType);
		if (itr != defaultEventHandalers.end())
		{
			itr->second->process(*this,eventType,info);
			return;
		}
	}
	return;
}

// event trigers from low level messages
// this way the low level events don't need the logic for the high level events.

void IRCClient::noticeMessage ( trMessageEventInfo	&info )
{
	if (info.params[1] == "Looking")
	{
		if (getConnectionState() < eTCPConenct)
			setConnectionState(eTCPConenct);

		callEventHandaler(eIRCNoticeEvent,info);
	}
}

void IRCClient::welcomeMessage ( trMessageEventInfo	&info )
{
	setNick(info.target);

	// we know we are conencted here
	if (getConnectionState() < eLogedIn)
		setConnectionState(eLogedIn);

	callEventHandaler(eIRCWelcomeEvent,info);
}

void IRCClient::endMOTD ( void )
{
	// we know we are conencted here
	if (getConnectionState() < eTCPConenct)
		setConnectionState(eTCPConenct);

	trBaseEventInfo	info;	// no info
	callEventHandaler(eIRCEndMOTDEvent,info);
}	

void IRCClient::joinMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	if (who == getNick())	// we joined a channel
	{	
		IRCChannel	channel;
		channel.setName(info.target);
		channels[channel.getName()] = channel;
	}
	else	// someone else joined a channel we are in
	{
		//channels[info.target].join()
	}
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
