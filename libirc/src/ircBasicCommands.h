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

#ifndef _IRC_BASIC_COMMANDS_
#define _IRC_BASIC_COMMANDS_

#include "libIRC.h"

// IRC "NICK" command
// paramaters {NICKNAME}
class IRCNickCommand : public IRCClientCommandHandaler
{
public:
	IRCNickCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};


// IRC "USER" command
// paramaters {USERNAME, HOST, SERVER, REAL_NAME}
class IRCUserCommand : public IRCClientCommandHandaler
{
public:
	IRCUserCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};


#endif //_IRC_BASIC_COMMANDS_
