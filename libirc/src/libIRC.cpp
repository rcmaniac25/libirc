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

IRCClient::IRCClient()
{
}

IRCClient::~IRCClient()
{
}

// general connection methods
bool IRCClient::init ( void )
{
	return false;
}

bool IRCClient::connect ( std::string server, int port )
{
	return false;
}

bool IRCClient::login ( std::string nick,  std::string username, std::string fullname)
{
	return false;
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

//command handaler methods
bool IRCClient::registerCommandHandaler ( std::string command, IRCClientCommandHandaler &handaler )
{
	return false;
}

int IRCClient::listCommandHandalers ( std::vector<std::string> &commandList )
{
	commandList.clear();
	return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
