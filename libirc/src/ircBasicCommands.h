/* libIRC
* Copyright (c) 2007 Christopher Sean Morrison
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
class IRCClientALLCommand : public IRCClientCommandHandler
{
public:
  IRCClientALLCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// numerics, handles any IRC numeric return code
class IRCClientNumericCommand : public IRCClientCommandHandler
{
public:
  IRCClientNumericCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// Text based IRC commands

// IRC "NICK" command
// paramaters {NICKNAME}
class IRCClientNickCommand : public IRCClientCommandHandler
{
public:
  IRCClientNickCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "USER" command
// paramaters {USERNAME, HOST, SERVER, REAL_NAME}
class IRCClientUserCommand : public IRCClientCommandHandler
{
public:
  IRCClientUserCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "PING" command
// paramaters {}
class IRCClientPingCommand : public IRCClientCommandHandler
{
public:
  IRCClientPingCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "PONG" command
// paramaters {}
class IRCClientPongCommand : public IRCClientCommandHandler
{
public:
  IRCClientPongCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "NOTICE" command
// paramaters {}
class IRCClientNoticeCommand : public IRCClientCommandHandler
{
public:
  IRCClientNoticeCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "JOIN" command
// paramaters {chanel1,chanel2......}
class IRCClientJoinCommand : public IRCClientCommandHandler
{
public:
  IRCClientJoinCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "PART" command
// paramaters {channel reason}
class IRCClientPartCommand : public IRCClientCommandHandler
{
public:
  IRCClientPartCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "QUIT" command
// paramaters {channel reason}
class IRCClientQuitCommand : public IRCClientCommandHandler
{
public:
  IRCClientQuitCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "MODE" command
// paramaters {target,modes}
class IRCClientModeCommand : public IRCClientCommandHandler
{
public:
  IRCClientModeCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "PRIVMSG" command
// paramaters 
class IRCClientPrivMsgCommand : public IRCClientCommandHandler
{
public:
  IRCClientPrivMsgCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

// IRC "KICK" command
// paramaters {user,reason}
class IRCClientKickCommand : public IRCClientCommandHandler
{
public:
  IRCClientKickCommand();
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo  &info );
};

#endif //_IRC_BASIC_COMMANDS_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
