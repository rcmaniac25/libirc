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
#include "TextUtils.h"

std::string delim = " ";

// Generic handaler for ALL
IRCALLCommand::IRCALLCommand()
{
	name = "ALL";
}

bool IRCALLCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// just log it out
	client.log(string_util::format("ALL::command %s from %s for %s containing %s",info.command.c_str(),info.source.c_str(),info.target.c_str(),info.getAsString().c_str()),4);
	client.log(string_util::format("ALL::raw %s",info.raw.c_str()),6);
	client.log(std::string(" "),6);
	return true;
}

bool IRCALLCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// just log it out
	client.log(string_util::format("ALL::command %s: to server containing %s",command.c_str(),info.getAsString().c_str()),4);
	return true;
}

// IRC "NICK" command

IRCNickCommand::IRCNickCommand()
{
	name = "NICK";
}

bool IRCNickCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	return true;
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
	return true;
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


// IRC "PING" command
IRCPingCommand::IRCPingCommand()
{
	name = "PING";
}

bool IRCPingCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	ircInfo;
	ircInfo.command = eCMD_PONG;
	client.sendIRCCommand(eCMD_PONG,ircInfo);
	return true;
}

bool IRCPingCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;
	// PING
	client.sendIRCCommandToServer(eCMD_PING,commandLine);
	return true;
}

// IRC "PONG" command

IRCPongCommand::IRCPongCommand()
{
	name = "PONG";
}

bool IRCPongCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// we do nothing on a pong
	return true;
}

bool IRCPongCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;
	// PING
	client.sendIRCCommandToServer(eCMD_PONG,commandLine);
	return true;
}

