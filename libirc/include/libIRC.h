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
#include "IRCEvents.h"
#include "TCPConnection.h"
#include "IRCChannels.h"

// global includes
#include <string>
#include <vector>
#include <map>

// simple OS indpendent sleep function
// used by so many things, so we have one here
void IRCOSSleep ( float fTime );

// need this later
class IRCClient;

// info that is passed to a command handler
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
class BaseIRCCommandInfo
{
public:
  BaseIRCCommandInfo();
  virtual ~BaseIRCCommandInfo();

	void parse ( std::string line );
	std::string getAsString ( int start = 0, int end = -1 );

  commndInfoTypes	type;
  std::string command;

  std::string raw;
	std::vector<std::string> params;
	bool prefixed;
	std::string source;
	std::string target;
};

// a normal Internet Relay Chat command
class IRCCommandINfo : public BaseIRCCommandInfo
{
public:
	teIRCCommands						 ircCommand;
};

// a Client To Client Protocol command
class CTCPCommandINfo : public BaseIRCCommandInfo
{
public:
	teCTCPCommands					 ctcpCommand;
	std::string from;
	std::string to;
	bool				request;
};

// a Direct Client Connect command
class DCCCommandINfo : public BaseIRCCommandInfo
{
	std::string from;
	std::string to;
	bool				request;
	std::string data;
};

// base command handler for any command
class IRCClientCommandHandler
{
public:
  IRCClientCommandHandler(){return;}
  virtual ~IRCClientCommandHandler(){return;}

  // called when the system wishes to know the name of this command
  virtual std::string getCommandName ( void ){return name;}

	// the send and receve methods return true if the default handler is to be called
	// it is recomended that the default ALWAYS be called, as it often sets internal data for other mesages

  // called when the client receves a command of this type
  virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return true;}

  // called when the user wishes to send a command of this type
  virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return true;}
protected:
	std::string name;
};

class IRCClientLogHandler
{
public:
	virtual ~IRCClientLogHandler(){return;}
	virtual void log ( IRCClient &client, int level, std::string line ) = 0;
};

class IRCClient : public TCPClientDataPendingListener, IRCBasicEventCallback
{
public:
  IRCClient();
	virtual ~IRCClient();

	std::string getLibVersion ( void );
	void getLibVersion ( int &major, int &minor, int &rev );

	// loging
	void	setLogHandler ( IRCClientLogHandler * loger );

	virtual void setLogfile ( std::string file );
	virtual std::string  getLogfile ( void );

	virtual void setDebugLevel ( int level );
	virtual int getDebugLevel ( void );

  // general connection methods
  virtual bool init ( void );
  virtual bool connect ( std::string server, int port );
	virtual bool disconnect ( std::string reason );

	void setFloodProtectTime ( float time ){minCycleTime = time;}
	float getFloodProtectTime ( void ){return minCycleTime;}

  // update methods
  virtual bool process ( void );

	// basic IRC operations
	virtual bool login ( std::string &nick, std::string &username, std::string &fullname, std::string &host);
	virtual bool join ( std::string channel );
	virtual bool part ( std::string channel, std::string reason );
	virtual bool sendMessage ( std::string target, std::string message, bool isAction = false );

	// IRC info operations
	virtual string_list listUsers ( std::string channel );
	virtual string_list listChanels ( void );
	virtual string_list listChanOps ( std::string channel );

	virtual trIRCChannelPermisions getChanPerms ( std::string channel );

	//event handler methods.... for higher level API
	virtual bool registerEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler );
	virtual bool removeEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler );
	virtual void callEventHandler ( teIRCEventType eventType, trBaseEventInfo &info );

  //command handler methods... for lower level API
	virtual bool registerCommandHandler ( IRCClientCommandHandler *handler );
	virtual bool removeCommandHandler ( IRCClientCommandHandler *handler );
	virtual int listUserHandledCommands ( std::vector<std::string> &commandList );
	virtual int listDefaultHandledCommands ( std::vector<std::string> &commandList );

	// command sending and receving methods called by handlers
	virtual bool sendCommand ( std::string &commandName, BaseIRCCommandInfo &info );
	virtual bool sendIRCCommand ( teIRCCommands	command, IRCCommandINfo &info );
	virtual bool sendCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info );

	virtual bool receveCommand ( std::string &commandName, BaseIRCCommandInfo &info );
	virtual bool receveIRCCommand ( teIRCCommands	command, IRCCommandINfo &info );
	virtual bool receveCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info );

	// --------------------------------------------------------------------------------------
	// generaly not called by the client app

	// called by the TCP/IP connection when we get data
	virtual void pending ( TCPClientConnection *connection, int count );

	// tutilitys generaly used only by command handlers
	// data sending stuff
	virtual bool sendIRCCommandToServer ( teIRCCommands	command, std::string &data);
	virtual bool sendCTCPCommandToServer ( teCTCPCommands	command, std::string &data);

	// the most RAWEST data transfer
	virtual bool sendTextToServer ( std::string &text );

	// low level log calls
	virtual void log ( std::string text, int level = 0 );
	virtual void log ( const char *text, int level = 0 );

	// info returned from IRC sessions, used to maintain the internal state, and dispatch high level events from low level messages
	void setServerHost ( std::string host ) {host = reportedServerHost;}
	std::string getServerHost ( void ){return reportedServerHost;}

	void noticeMessage ( trMessageEventInfo	&info );
	void welcomeMessage ( trMessageEventInfo	&info );

	void beginMOTD ( void ){MOTD = "";}
	void addMOTD ( std::string line ) {MOTD += line + std::string("\n");}
	void endMOTD ( void );
	std::string getMOTD ( void ){return MOTD;}

	void joinMessage ( BaseIRCCommandInfo	&info );
	void partMessage ( BaseIRCCommandInfo	&info );

	void setNick ( std::string text ) {nickname=text;}
	std::string getNick ( void ) {return nickname;}

	void setChannelMode ( std::string channel, std::string mode );
	void setChannelTopicMessage ( std::string channel, std::string topic, std::string source );
	void addChannelUsers ( std::string channel, string_list newUsers );
	void endChannelUsersList ( std::string channel );

	void privMessage ( BaseIRCCommandInfo	&info );

	void nickNameError ( int error, std::string message );

	void modeCommand ( BaseIRCCommandInfo	&info );

	// used by the defalt event handlers
	bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );

protected:
	friend class IRCClientCommandHandler;

	// networking
	TCPClientConnection		*tcpClient;	
	TCPConnection					&tcpConnection;

	// irc data
	std::string						ircServerName;
	std::string						reportedServerHost;
	unsigned short				ircServerPort;
	std::string						lastRecevedData;

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
		eLastState
	}teIRCConnectionState;

	teIRCConnectionState	ircConenctonState;

	bool									registered;

	virtual teIRCConnectionState getConnectionState ( void ){return ircConenctonState;}
	virtual void setConnectionState ( teIRCConnectionState state ){ircConenctonState = state;}

	// receved data processing
	void processIRCLine ( std::string line );

	// the command handlers
	typedef std::map<std::string, IRCClientCommandHandler*>	tmCommandHandlerMap;
	typedef std::map<std::string, std::vector<IRCClientCommandHandler*> >	tmUserCommandHandlersMap;

	tmCommandHandlerMap			defaultCommandHandlers;
	tmUserCommandHandlersMap	userCommandHandlers;

	void addDefaultCommandhandlers ( IRCClientCommandHandler* handler );
	void clearDefaultCommandhandlers ( void );
	void registerDefaultCommandhandlers ( void );

	// event handlers
	tmIRCEventMap							defaultEventHandlers;
	tmIRCEventListMap					userEventHandlers;

	void addDefaultEventHandlers ( teIRCEventType eventType, IRCBasicEventCallback* handler );
	void clearDefaultEventHandlers ( void );
	void registerDefaultEventHandlers ( void );

	// user management
	trIRCUser& getUserRecord ( std::string name );
	bool removeChannelUser (std::string channel, std::string name );

	// loging
	IRCClientLogHandler			*logHandler;
	std::string								logfile;
	int												debugLogLevel;

	// info from the connection
	std::string								MOTD;
	std::string								requestedNick;
	std::string								nickname;
	tmChannelMap							channels;

	tvIRCUserMap							userList;

	// flood protection
	float											minCycleTime;

	teNickModes parseNickMode ( std::string &nick );
	std::string getCleanNick ( std::string &nick );
};

#endif //_LIBIRC_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
