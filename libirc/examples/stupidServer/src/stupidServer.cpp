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

bool quit = false;

class MyIRCServer : public IRCServer
{
public:
	virtual void clientConnect ( IRCServerConnectedClient *client );
	virtual void clientDisconnect ( IRCServerConnectedClient *client );
	virtual void clientIRCCommand ( const std::string &command, IRCServerConnectedClient *client );

	virtual bool process ( void );
};

int main ( int argc, char *argv[] )
{
	std::string Config = "sample.cfg";

	if (argc>1)
		Config = argv[1];

	MyIRCServer	server;

	server.listen();
	while (!server.process() &&!quit)
		IRCOSSleep(0.01f);

	if (quit)
		server.disconnect(std::string("Shutdown"));

	return 0;
}

void MyIRCServer::clientConnect ( IRCServerConnectedClient *client )
{

}

void MyIRCServer::clientDisconnect ( IRCServerConnectedClient *client )
{

}

void MyIRCServer::clientIRCCommand ( const std::string &command, IRCServerConnectedClient *client )
{

}

bool MyIRCServer::process ( void )
{
	return IRCServer::process();
}


