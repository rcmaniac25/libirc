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

// libIRC Server header

#ifndef _IRC_SERVER_H_
#define _IRC_SERVER_H_

// IRC includes
#include "ircCommands.h"
#include "IRCEvents.h"
#include "TCPConnection.h"
#include "IRCUserManager.h"

// global includes
#include <string>
#include <vector>
#include <map>

// need this later
class IRCServer;

// info that is passed to a command handler
// handles standard commands and CTCP

class IRCServerLogHandler
{
public:
	virtual ~IRCServerLogHandler(){return;}
	virtual void log ( IRCServer &server, int level, std::string line ) = 0;
};

class IRCServer : public TCPServerDataPendingListener, IRCBasicEventCallback
{
public:
	IRCServer();
	virtual ~IRCServer();

	// loging
	void	setLogHandler ( IRCServerLogHandler * loger );

	virtual void setLogfile ( std::string file );
	virtual std::string  getLogfile ( void );

	virtual void setDebugLevel ( int level );
	virtual int getDebugLevel ( void );

	// general connection methods
	virtual bool init ( void );
	virtual bool listen ( int maxConnections, int port );
	virtual bool disconnect ( std::string reason );

	void setFloodProtectTime ( float time ){minCycleTime = time;}
	float getFloodProtectTime ( void ){return minCycleTime;}

	// update methods
	virtual bool process ( void );

	// low level log calls
	virtual void log ( std::string text, int level = 0 );
	virtual void log ( const char *text, int level = 0 );

	bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );

	virtual bool accept ( TCPServerConnection *connection, TCPServerConnectedPeer *peer );
	virtual void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, int count );

protected:
	friend class IRCClientCommandHandler;

	// networking
	TCPServerConnection		*tcpServer;	
	TCPConnection			&tcpConnection;


	// loging
	IRCServerLogHandler				*logHandler;
	std::string								logfile;
	int												debugLogLevel;

	// info from the connection
	std::string								MOTD;
	std::string								requestedNick;
	std::string								nickname;

	IRCUserManager						userManager;
	// flood protection
	float											minCycleTime;
};

#endif //_IRC_SERVER_H_