/* libIRC
 * Copyright (c) 2007 Jeff Myers
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

// global includes
#include <string>
#include <vector>

#ifndef string_list
typedef std::vector<std::string> string_list;
#endif

// IRC includes
#include "ircCommands.h"
#include "IRCEvents.h"
#include "IRCUserManager.h"
#include "TCPConnection.h"

#define _DEFAULT_IRC_PORT  6667

/** An OS independent sleep function
 *
 * IRCOSSleep is used by a number of functions to 1) prevent 100% CPU
 * usage, and 2) to prevent from flooding the server
 */
void IRCOSSleep ( float fTime );

/** Returns the local time in stamp format */
std::string getTimeStamp ( void );

/** Returns the current library Version */

std::string getLibVersion ( void );
void getLibVersion ( int &major, int &minor, int &rev );

// info that is passed to a command handler
// handles standard commands and CTCP

/** the types of command info structures */
typedef enum
{
  eUnknownCommand = 0,
  eIRCCommand,
  eCTCPCommand,
  eDDECommand
}commndInfoTypes;

/**
 * base struct in witch all info structures are derived
 */
class BaseIRCCommandInfo
{
public:
  BaseIRCCommandInfo();
  virtual ~BaseIRCCommandInfo();

  void parse ( std::string line );
  std::string getAsString ( int start = 0, int end = -1 );

  commndInfoTypes  type;
  std::string command;

  std::string raw;
  std::vector<std::string> params;
  bool prefixed;
  std::string source;
  std::string target;
};

/**
 * a normal Internet Relay Chat command
 */
class IRCCommandInfo : public BaseIRCCommandInfo
{
public:
  teIRCCommands             ircCommand;
};

/**
 * a Client To Client Protocol command
 */
class CTCPCommandInfo : public BaseIRCCommandInfo
{
public:
  teCTCPCommands           ctcpCommand;
  std::string from;
  std::string to;
  bool        request;
};

/**
 * a Direct Client Connect command
 */
class DCCCommandInfo : public BaseIRCCommandInfo
{
  std::string from;
  std::string to;
  bool        request;
  std::string data;
};

#include "IRCClient.h"
#include "IRCServer.h"

#endif //_LIBIRC_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
