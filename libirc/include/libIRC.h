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

// main libIRC header

#ifndef _LIBIRC_H_
#define _LIBIRC_H_

// IRC includes
#include "ircCommands.h"

// global includes
#include <string>
#include <vector>

// need this later
class IRCClient;

// info that is passed to a command handaler
// handles standard commands and CTCP
class IRCCommandInfo
{
public:
  IRCCommandInfo(){ctcp = false;request = false;};
  ~IRCCommandInfo(){return;};

  std::vector<std::string> params;
  std::string command;
  std::string prefix;
  bool ctcp;
  std::string from;
  std::string to;
  bool request;
};

class IRCClientCommandHandaler
{
public:
  IRCClientCommandHandaler(){return;}
  virtual ~IRCClientCommandHandaler(){return;}

  // called when the system wishes to know the name of this command
  virtual std::string getCommandName ( void ){return "NULL";}

  // called when the client receves a command of this type
  virtual bool receve ( IRCClient &client, std::string &command, IRCCommandInfo	&info ){return false;}

  // called when the user wishes to send a command of this type
  virtual bool send ( IRCClient &client, std::string &command, IRCCommandInfo	&info ){return false;}
};

class IRCClient
{
public:
  IRCClient();
  ~IRCClient();

  // general connection methods
  bool init ( void );
  bool connect ( std::string server, int port );
  bool login ( std::string nick,  std::string username, std::string fullname);
  bool disconnect ( void );

  // update loop methods
  bool process ( void );

  // sending commands
  bool send ( std::string command, std::string target, std::string data );
  bool send ( std::string &command, IRCCommandInfo &info );
  bool sendRaw ( std::string data );

  //command handaler methods
  bool registerCommandHandaler ( std::string command, IRCClientCommandHandaler &handaler );
  int listCommandHandalers ( std::vector<std::string> &commandList );
};

#endif //_LIBIRC_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
