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

IRCServer::IRCServer()
:tcpConnection(TCPConnection::instance())
{
}

IRCServer::~IRCServer()
{
}

void	IRCServer::setLogHandler ( IRCServerLogHandler * loger )
{

}

void IRCServer::setLogfile ( std::string file )
{

}

std::string  IRCServer::getLogfile ( void )
{
	return std::string("file");
}

void IRCServer::setDebugLevel ( int level )
{

}

int IRCServer::getDebugLevel ( void )
{
	return 0;
}

bool IRCServer::init ( void )
{
	return false;
}

bool IRCServer::listen ( int maxConnections, int port )
{
	return false;
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
	return;
}

void IRCServer::log ( const char *text, int level )
{

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
