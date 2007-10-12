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

// libIRC Server header

#ifndef _IRC_SERVER_H_
#define _IRC_SERVER_H_

// IRC includes
#include "libIRC.h"
#include "ircCommands.h"
#include "IRCEvents.h"
#include "TCPConnection.h"
#include "IRCUserManager.h"

// global includes
#include <string>
#include <vector>
#include <map>

// need this later
class IRCServer;

/**
 * info that is passed to a command handler handles standard commands
 * and CTCP
 */
class IRCServerLogHandler
{
public:
  virtual ~IRCServerLogHandler(){return;}
  virtual void log ( IRCServer &server, int level, std::string line ) = 0;
};

class IRCServerConnectedClient
{
public:
  IRCServerConnectedClient ( IRCServer *_server, TCPServerConnectedPeer* _peer );
  virtual ~IRCServerConnectedClient();

  unsigned int getClientID ( void ) { return clientID;}
  virtual bool sendText ( const std::string &text );
  virtual bool sendText ( const  char*text );
  std::string lastData;

  std::string getHostMask ( void );
  bool getIP ( unsigned char ip[4] );

  virtual bool joinChannel ( const char* channel );
  virtual bool partChannel ( const char* channel );

protected:
  unsigned int clientID;
  bool localUser;
  
  // local user connection
  TCPServerConnectedPeer  *peer;
  IRCServer *server;

  // remote user connection
  std::string remoteServer;

  std::vector<std::string> channels;
};

class IRCServerChannel
{
public:
  IRCServerChannel( const char * _name );
  virtual ~IRCServerChannel();

  virtual void addMember ( IRCServerConnectedClient *member );
  virtual void removeMember ( IRCServerConnectedClient *member );

  virtual bool sendText ( const std::string &text );
  virtual bool sendText ( const char*text );

protected:
  std::string name;
  class ChannelUserData
  {
  public:
    virtual ~ChannelUserData(){};
    bool voice;
    bool op;
  };

  virtual ChannelUserData* newUserData ( void ){ return new ChannelUserData;}
  virtual void deleteUserData ( ChannelUserData *data ){ delete(data);}

  typedef std::map <IRCServerConnectedClient*, ChannelUserData*> MemberDataMap;
  MemberDataMap members;
};

/**
 * base command handler for any command
 */
class IRCServerCommandHandler
{
public:
  IRCServerCommandHandler(){return;}
  virtual ~IRCServerCommandHandler(){return;}

  // called when the system wishes to know the name of this command
  virtual std::string getCommandName ( void ){return name;}

  // the send and receve methods return true if the default handler is to be called
  // it is recomended that the default ALWAYS be called, as it often sets internal data for other mesages

  // called when the client receves a command of this type
  virtual bool receve ( IRCServer *server, IRCServerConnectedClient *client, const std::string &command, const BaseIRCCommandInfo  &info ){return true;}

  // called when the user wishes to send a command of this type
  virtual bool send ( IRCServer *server, IRCServerConnectedClient *client, const std::string &command, const BaseIRCCommandInfo  &info ){return true;}

protected:
  std::string name;
};

class IRCServer : public TCPServerDataPendingListener, IRCServerEventCallback
{
public:
  IRCServer();
  virtual ~IRCServer();

  // loging
  void  setLogHandler ( IRCServerLogHandler * logger );

  virtual void setLogfile ( std::string file );
  virtual std::string  getLogfile ( void );

  virtual void setDebugLevel ( int level );
  virtual int getDebugLevel ( void );

  // general connection methods
  virtual bool setHostName ( const char* host ){if(host)hostname = host; return hostname.size()!=0;}

  virtual bool listen ( int maxConnections = 32, int port = -1 );
  virtual bool disconnect ( std::string reason );

  void setFloodProtectTime ( float time ){minCycleTime = time;}
  float getFloodProtectTime ( void ){return minCycleTime;}

  // update methods
  virtual bool process ( void );

  // low level log calls
  virtual void log ( std::string text, int level = 0 );
  virtual void log ( const char *text, int level = 0 );

  virtual bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer );
  virtual void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count );
  virtual void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false );

  // virtual methods for basic IRC functions
  virtual bool allowConnection ( const char* hostmask, unsigned char ip[4] );
  virtual void clientConnect ( IRCServerConnectedClient *client );
  virtual void clientDisconnect ( IRCServerConnectedClient *client );

  virtual void clientIRCCommand ( const BaseIRCCommandInfo &command, IRCServerConnectedClient *client );

  IRCServerChannel *getChannel ( const char *name );
  IRCServerChannel *getChannel ( const std::string& name );

  // commands
  virtual bool receveCommand ( const std::string &commandName, IRCServerConnectedClient *client, const BaseIRCCommandInfo &info );

  //command handler methods... for lower level API
  virtual bool registerCommandHandler ( IRCServerCommandHandler *handler );
  virtual bool removeCommandHandler ( IRCServerCommandHandler *handler );

  virtual int listUserHandledCommands ( std::vector<std::string> &commandList );
  virtual int listDefaultHandledCommands ( std::vector<std::string> &commandList );

  //event handler methods.... for higher level API
  virtual bool registerEventHandler ( teIRCEventType eventType, IRCServerEventCallback *handler );
  virtual bool removeEventHandler ( teIRCEventType eventType, IRCServerEventCallback *handler );
  virtual void callEventHandler ( teIRCEventType eventType, trBaseServerEventInfo &info );

  // internal event callbacks
  virtual bool process ( IRCServer *ircServer, teIRCEventType  eventType, trBaseServerEventInfo &info );

  // super high level common overides
  typedef struct 
  {
    std::string welome;
    std::string host;
    std::string created;
    std::string info;
    std::string bounce;

    std::string motd;
  }ConnectionText;

  virtual void getConnectionText ( IRCServerConnectedClient *client, ConnectionText &text ){return;}

protected:
  friend class IRCServerConnectedClient;

  // overide thsese if you want to derive your own classes
  virtual IRCServerChannel* newChannel ( const char * name ){ return new IRCServerChannel(name); }
  virtual void deleteChannel ( IRCServerChannel *channel ){ delete(channel); }

  virtual IRCServerConnectedClient* newClient ( IRCServer *_server, TCPServerConnectedPeer* _peer  ){ return new IRCServerConnectedClient(_server,_peer); }
  virtual void deleteClient ( IRCServerConnectedClient *client ){ delete(client); }

  // common functions
  bool sendTextToPeer ( const std::string &text, TCPServerConnectedPeer *peer );

  virtual void processIRCLine ( std::string line, IRCServerConnectedClient *client );

  // networking
  TCPServerConnection	  *tcpServer;  
  TCPConnection		  &tcpConnection;
  int			  ircServerPort;

  std::string		  hostname;

  // loging
  IRCServerLogHandler        *logHandler;
  std::string            logfile;
  int                debugLogLevel;

  // helpers
  std::string ircMessageTerminator;
  std::string ircCommandDelimator;

  // flood protection
  float              minCycleTime;

  // users
  typedef std::vector<IRCServerConnectedClient*> ClientList;
  ClientList  clients;

  ClientList::iterator getClientItr ( IRCServerConnectedClient *client );

  // channels
  typedef std::map<std::string, IRCServerChannel*> ChannelMap;
  ChannelMap channels;

  // command handlers

  typedef std::map<std::string, IRCServerCommandHandler*>  tmCommandHandlerMap;
  typedef std::map<std::string, std::vector<IRCServerCommandHandler*> >  tmCommandHandlerListMap;

  tmCommandHandlerMap      defaultCommandHandlers;
  tmCommandHandlerListMap  userCommandHandlers;

  void addDefaultCommandHandlers ( IRCServerCommandHandler* handler );
  void clearDefaultCommandHandlers ( void );
  void registerDefaultCommandHandlers ( void );

  // event handlers
  tmIRCServerEventMap        defaultEventHandlers;
  tmIRCServerEventListMap      userEventHandlers;

  void addDefaultEventHandlers ( teIRCEventType eventType, IRCServerEventCallback* handler );
  void clearDefaultEventHandlers ( void );
  void registerDefaultEventHandlers ( void );

  // utilities

  void sendLinesOutToClient ( int numeric, IRCServerConnectedClient *client, const std::string &text );

};

#endif //_IRC_SERVER_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
