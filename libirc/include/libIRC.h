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
#include "TCPConnection.h"

// global includes
#include <string>
#include <vector>
#include <map>

// simple OS indpendent sleep function
// used by so many things, so we have one here
void IRCOSSleep ( float fTime );

// need this later
class IRCClient;

// info that is passed to a command handaler
// handles standard commands and CTCP

// the types of command info structures
typedef enum
{
	eUnknown = 0,
	eIRCCommand,
	eCTCPCommand,
	eDDECommand
}commndInfoTypes;

// base struct in witch all info structures are derived
struct BaseIRCCommandInfo
{
  BaseIRCCommandInfo(){type = eUnknown;command = "NULL";};
  ~BaseIRCCommandInfo(){return;};

  commndInfoTypes	type;
  std::string command;
  std::string raw;
};

// a normal Internet Relay Chat command
struct IRCCommandINfo : public BaseIRCCommandInfo
{
	teIRCCommands						 ircCommand;
	std::vector<std::string> params;
};

// a Client To Client Protocol command
struct CTCPCommandINfo : public BaseIRCCommandInfo
{
	teCTCPCommands					 ctcpCommand;
	std::vector<std::string> params;
	std::string from;
	std::string to;
	bool				request;
};

// a Direct Client Connect command
struct DCCCommandINfo : public BaseIRCCommandInfo
{
	std::string from;
	std::string to;
	bool				request;
	std::string data;
};

// base command handaler for any command
class IRCClientCommandHandaler
{
public:
  IRCClientCommandHandaler(){return;}
  virtual ~IRCClientCommandHandaler(){return;}

  // called when the system wishes to know the name of this command
  virtual std::string getCommandName ( void ){return name;}

  // called when the client receves a command of this type
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return false;}

  // called when the user wishes to send a command of this type
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return false;}
protected:
	std::string name;
};

class IRCClientLogHandaler
{
public:
	virtual ~IRCClientLogHandaler(){return;}
	virtual void log ( IRCClient &client, int level, std::string line ) = 0;
};

class IRCClient : public TCPClientDataPendingListener
{
public:
  IRCClient();
	virtual ~IRCClient();

  // general connection methods
  virtual bool init ( void );
  virtual bool connect ( std::string server, int port );
  virtual bool login ( std::string &nick, std::string &username, std::string &fullname);
  virtual bool disconnect ( void );

  // update loop methods
  virtual bool process ( void );

  // sending commands
  virtual bool send ( std::string command, std::string target, std::string data );
  virtual bool send ( std::string &command, BaseIRCCommandInfo &info );
  virtual bool sendRaw ( std::string data );

  //command handaler methods
	virtual bool registerCommandHandaler ( IRCClientCommandHandaler *handaler );
	virtual bool removeCommandHandaler ( IRCClientCommandHandaler *handaler );
	virtual int listUserHandledCommands ( std::vector<std::string> &commandList );
	virtual int listDefaultHandledCommands ( std::vector<std::string> &commandList );

	virtual bool sendCommand ( std::string &commandName, BaseIRCCommandInfo &info );
	virtual bool sendIRCCommand ( teIRCCommands	command, IRCCommandINfo &info );
	virtual bool sendCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info );

	void	setLogHandaler ( IRCClientLogHandaler * loger );

	// --------------------------------------------------------------------------------------
	// generaly not called by the client app

	// called by the TCP/IP connection when we get data
	virtual void pending ( TCPClientConnection *connection, int count );

	// debug API
	virtual void setLogfile ( std::string file );
	virtual std::string  getLogfile ( void );
	virtual void setDebugLevel ( int level );
	virtual int getDebugLevel ( void );

	// tutilitys generaly used only by command handalers
	// data sending stuff
	bool sendIRCCommandToServer ( teIRCCommands	command, std::string &data);
	bool sendCTCPCommandToServer ( teCTCPCommands	command, std::string &data);

	// the most RAWEST data transfer
	bool sendTextToServer ( std::string &text );

	// utilitys
	void log ( std::string &text, int level = 0 );
	void log ( const char *text, int level = 0 );

protected:
	friend class IRCClientCommandHandaler;

	TCPClientConnection		*tcpClient;	
	TCPConnection					&tcpConnection;
	int										debugLogLevel;
	std::string						logfile;

	// irc data
	std::string						ircServerName;
	unsigned short				ircServerPort;

	// IRC "constants"
	std::string		ircMessageTerminator;
	std::string		ircCommandDelimator;

	// the wonderfull connection state
	typedef enum
	{
		eNotConnected = 0,
		eTCPConenct,
		eSentNickAndUSer,
		eLogedIn,
		eRegistered,
		eInChannel,
		eLastState
	}teIRCConnectionState;

	virtual teIRCConnectionState getConnectionState ( void ){return ircConenctonState;}
	virtual void setConnectionState ( teIRCConnectionState state ){ircConenctonState = state;}

	teIRCConnectionState	ircConenctonState;

	// the command handalers
	typedef std::map<std::string, IRCClientCommandHandaler*>	tmCommandHandalerMap;
	typedef std::map<std::string, std::vector<IRCClientCommandHandaler*> >	tmUserCommandHandalersMap;

	tmCommandHandalerMap			defaultCommandHandalers;
	tmUserCommandHandalersMap	userCommandHandalers;

	void addDefaultCommandhandalers ( IRCClientCommandHandaler* handaler );
	void clearDefaultCommandhandalers ( void );
	void registerDefaultCommandhandalers ( void );

	IRCClientLogHandaler			*logHandaler;
};

#endif //_LIBIRC_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
