/* 
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

#ifndef _AGENTS_H_
#define _AGENTS_H_

#include "libIRC.h"
#include <string>
#include <vector>

//The specific hostname/port of an IRC server
typedef struct
{
  std::string host;
  int	      port;
}ServerHost;
typedef std::vector<ServerHost>	ServerHostList;

// A channel on an IRC network
class ServerChannel
{
public:
  virtual ~ServerChannel();
protected:
  std::string	name;		      // channel name
  bool		autoConnect;	      // true if the channel is to be automaticly conencted on startup
};
typedef std::map<std::string,ServerChannel> ServerChannelMap;

// A private conversation with a user on an IRC network
class ServerPrivateMessage
{
public:
  virtual ~ServerPrivateMessage();

protected:
  std::string name;
};
typedef std::map<std::string,ServerPrivateMessage> ServerPrivateMessageMap;

// An IRC network that an Agent connects to.
class AgentConnectedServer : public IRCClientEventCallback
{
public:
  AgentConnectedServer();
  virtual ~AgentConnectedServer();

  bool connected ( void );

  bool init ( const std::string &name, const std::string &rootDir );
  bool update ( void );

  void addHost ( ServerHost &host, bool prefered );
  void addNick ( const std::string &nick );

  void setUserInfo ( const std::string &r, const std::string &u );

  bool valid ( void ) { return validNickList()&& hostnames.size();}
  bool validNickList ( void ){return ircNicks.size()>0;}

  virtual bool process ( IRCClient &ircClient, teIRCEventType  eventType, trBaseEventInfo &info );

protected:
  ServerHostList		  hostnames;		// the list of entry point addresses to connect to the network
  int				  currentHost;		// index of the current host we are trying to use
  
  ServerChannelMap		  channels;		// the list of active channels that we are listening to.
  ServerPrivateMessageMap	  privateMessages;	// the list of active private message converations that we are listenting too.

  std::vector<std::string>	  ircNicks;		// the list of nicknames to use on this network
  int				  lastAtemptedNick;	// the last name in the list we tried to use
  std::string			  currentNick;		// the current nick of the agent on this network.
  std::string			  username;
  std::string			  fullname;

private:
  IRCClient			  ircConenction;
  std::string			  networkName;
  std::string			  dataDir;

  typedef enum
  {
    eIdle,
    eConnecting,
    eConnected,
    eDisconected,
    eErrored
  }Status;

  Status			   connectionStatus;

  void connectToServer ( void );
  void joinChannels ( void );
  void tryNextNick ( void );

};
typedef std::map<std::string, AgentConnectedServer*> AgentConnectedServersMap;

// An IRC listen agent.
// Agents connect to a number of IRC networks, and listen for data on a number of channels.
// Clients connect to the engine and are attached to an agent to handle all IRC intefaces.
// Each Agent connection has it's own directory to store it's associted data ( maybe a database later ).
class Agent
{
public:
  Agent();
  Agent( const char* dir );			  // this implies a new agent with out a dir that needs to be saved when save is called
						  // we'll auto save after first connect
  virtual ~Agent();

  virtual bool loadFromDir ( const char* dir );	  // this is called for existing agents that are restarting, they will auto connect
  virtual bool saveToDir ( void );		  // called by all agents to save data out to some form of perma storage

  virtual bool valid ( void );
 
  virtual bool init ( void );

  virtual bool connected ( void );
  virtual bool update ( void );

  const std::string &getName ( void ) { return agentName; }
protected:
  std::string			dirName;
  std::string			agentName;

  AgentConnectedServersMap	servers;	  // the list of connected IRC networks

private:
    AgentConnectedServer* getServer ( const std::string &name );
};
typedef std::map<std::string,Agent*>	AgentMap;

#endif // _AGENTS_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
