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

// implementation of main libIRC Server classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "TextUtils.h"

#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
	#include <time.h>
	#include <stdio.h>
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


IRCServer::IRCServer()
:tcpConnection(TCPConnection::instance())
{
	tcpServer = NULL;
	init();

	debugLogLevel = 0;
	logHandler = &defaultLoger;
}

IRCServer::~IRCServer()
{
}

void IRCServer::setLogHandler ( IRCServerLogHandler * loger )
{
	if (!loger)
		logHandler = &defaultLoger;
	else
		logHandler = loger;
}

void IRCServer::setLogfile ( std::string file )
{
	logfile = file;
}

std::IRCServer  IRCClient::getLogfile ( void )
{
	return logfile;
}

void IRCServer::setDebugLevel ( int level )
{
	debugLogLevel = level;
}

int IRCServer::getDebugLevel ( void )
{
	return debugLogLevel;
}

bool IRCServer::init ( void )
{
	minCycleTime = 0.1f;
	
	// if any old conenctions are around, kill em
	if (tcpServer)
		tcpConnection.deleteServerConnection(tcpServer);

	tcpServer = NULL;
	ircServerPort = _DEFAULT_IRC_PORT;

	// just get us a new empty connection
	tcpServer = tcpConnection.newServerConnection(0,1);

	if (tcpServer)
		tcpServer->addListener(this);

	return tcpServer != NULL;
}

bool IRCServer::listen ( int maxConnections, int port )
{
	if (!tcpServer || !maxConnections)
		return false;

	ircServerPort = _DEFAULT_IRC_PORT;
	if ( port > 0 )
		ircServerPort = (unsigned short)port;

	tcpServer->
	teTCPError err = tcpClient->connect(server,ircServerPort);

	ircConenctonState = err == eTCPNoError ? eTCPConenct : eNotConnected;

	return err == eTCPNoError;
}

bool IRCServer::disconnect ( std::string reason )
{
	return false;
}

bool IRCServer::process ( void )
{
	return false;
}

void IRCServer::log ( std::string text, int level )
{
	if (level <= debugLogLevel && logHandler)
		logHandler->log(*this,level,text);
}

void IRCServer::log ( const char *text, int level )
{
	log(std::string(text),level);
}

bool IRCServer::process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info )
{
	return false;
}

bool IRCServer::accept ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
{
	return false;
}

void IRCServer::pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, int count )
{

}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
