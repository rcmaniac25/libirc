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

// special case commands
// handles ALL posible messages, dosn't actualy DO anythign with them tho
class IRCALLCommand : public IRCClientCommandHandaler
{
public:
	IRCALLCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// numerics, handles any IRC numeric return code
class IRCNumericCommand : public IRCClientCommandHandaler
{
public:
	IRCNumericCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// Text based IRC commands

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

// IRC "PING" command
// paramaters {}
class IRCPingCommand : public IRCClientCommandHandaler
{
public:
	IRCPingCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// IRC "PONG" command
// paramaters {}
class IRCPongCommand : public IRCClientCommandHandaler
{
public:
	IRCPongCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// IRC "NOTICE" command
// paramaters {}
class IRCNoticeCommand : public IRCClientCommandHandaler
{
public:
	IRCNoticeCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// IRC "JOIN" command
// paramaters {chanel1,chanel2......}
class IRCJoinCommand : public IRCClientCommandHandaler
{
public:
	IRCJoinCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// IRC "MODE" command
// paramaters {target,modes}
class IRCModeCommand : public IRCClientCommandHandaler
{
public:
	IRCModeCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};

// IRC "PRIVMSG" command
// paramaters {target,modes}
class IRCPrivMsgCommand : public IRCClientCommandHandaler
{
public:
	IRCPrivMsgCommand();
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info );
};



#endif //_IRC_BASIC_COMMANDS_
