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

// basic IRC commands

#include "ircBasicCommands.h"

std::string delim = " ";

// IRC "NICK" command

IRCNickCommand::IRCNickCommand()
{
	name = "NICK";
}

bool IRCNickCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	return false;
}

bool IRCNickCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;

	// NICK
	commandLine = ircInfo.params[0];
	client.sendIRCCommandToServer(eCMD_NICK,commandLine);

	return true;
}

// IRC "USER" command

IRCUserCommand::IRCUserCommand()
{
	name = "USER";
}

bool IRCUserCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	return false;
}

bool IRCUserCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{	
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;

	//username host server fullname
	commandLine = ircInfo.params[0] + delim + ircInfo.params[1] + delim + ircInfo.params[2] + delim + ircInfo.params[3];
	client.sendIRCCommandToServer(eCMD_USER,commandLine);

	return true;
}



