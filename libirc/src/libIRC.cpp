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

// IRC class stuff

IRCClient::IRCClient()
:tcpConnection(TCPConnection::instance())
{
	registerDefaultCommandhandalers();
	init();
	tcpClient = NULL;

	ircMessageTerminator = " \r\n";
	ircCommandDelimator	 = " ";
	debugLogLevel = 0;
	ircServerPort = 6667;
	ircConenctonState = eNotConnected;
	logHandaler = &defaultLoger;
}

IRCClient::~IRCClient()
{
	tcpConnection.kill();
}

// general connection methods
bool IRCClient::init ( void )
{
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

	ircServerName = server;
	ircServerPort = 6667;
	if ( port > 0 )
		ircServerPort = (unsigned short)port;

	teTCPError err = tcpClient->connect(server,ircServerPort);

	ircConenctonState = err == eTCPNoError ? eTCPConenct : eNotConnected;

	return err == eTCPNoError;
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
	ircConenctonState = eSentNickAndUSer;
	return  true;
}

bool IRCClient::disconnect ( void )
{
	return false;
}

// update loop methods
bool IRCClient::process ( void )
{
	return false;
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
		char	*temp;

		while ( count < len )
		{
			temp = strchr(data+count,0);
			if (temp)	// there was a NULL, lets read up to it
			{
				theLine += &(data[count]);

				// send the line off
				// do some processing on the line and send it
				processIRCLine(theLine);

				theLine = "";
				lastRecevedData = theLine;
				count += (int)strlen(&data[count])+1;
			}
			else
			{
				// the line goes over a packet, we will get more data soon, so just hang onto the last set of data
				char	*tempData = (char*)malloc(len-count+1);
				memcpy(tempData,&data[count],len-count);
				tempData[len-count] = 0;

				lastRecevedData = tempData;
				free(tempData);
				count = len;
			}
		}

		packets.erase(packets.begin());
	}
}

void IRCClient::processIRCLine ( std::string line )
{
	// we have a single line of text, do something with it.
	// see if it's a command, and or call any handalers that we have
	// also check for error returns
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

	if (commandListItr != userCommandHandalers.end() || !commandListItr->second.size())	// do we have a custom command handaler
	{
		// is this right?
		// should we do them all? or just the first one that "HANDLES" it?
		std::vector<IRCClientCommandHandaler*>::iterator	itr = commandListItr->second.begin();
		while (itr != commandListItr->second.end())
		{
			(*itr)->send(*this,commandName,info);
			itr++;
		}
		return true;
	}
	else	// check for the default
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
	userCommandHandalers.clear();
	clearDefaultCommandhandalers();

	addDefaultCommandhandalers(new IRCNickCommand );
	addDefaultCommandhandalers(new IRCUserCommand );
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
