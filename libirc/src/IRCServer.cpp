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

// implementation of main libIRC Server classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "IRCTextUtils.h"
#include <algorithm>

#ifndef _WIN32
  #include <unistd.h>
#else
  #include <windows.h>
  #include <time.h>
  #include <stdio.h>
#endif

class DefaultServerIRCLogHandler : public IRCServerLogHandler
{
public:
  virtual ~DefaultServerIRCLogHandler(){return;}
  virtual void log ( IRCServer &client, int level, std::string line )
  {
    printf("log# %d:%s\n",level,line.c_str());

    if (client.getLogfile().size())
    {
      FILE *fp = fopen(client.getLogfile().c_str(),"at");

      if (fp)
      {
        fprintf(fp,"log# %d:%s\n",level,line.c_str());
        fclose(fp);
      }
    }
  }
};

static DefaultServerIRCLogHandler  defaultLogger;

//------------------------------------IRCServerConnectedClient-----------------------------------------
IRCServerConnectedClient::IRCServerConnectedClient ( IRCServer *_server, TCPServerConnectedPeer* _peer )
{
  peer = _peer;
  clientID = peer->getUID();
  server = _server;
}

IRCServerConnectedClient::~IRCServerConnectedClient()
{
}

bool IRCServerConnectedClient::sendText ( const std::string &text )
{ 
  if (!server)
    return false;

  return server->sendTextToPeer(text,peer);
}

bool IRCServerConnectedClient::sendText ( const char *text )
{ 
  if (!server || !text)
    return false;

  return server->sendTextToPeer(std::string(text),peer);
}

std::string IRCServerConnectedClient::getHostMask ( void )
{
  if (!peer)
    return std::string();

  return peer->getHostMask();
}

bool IRCServerConnectedClient::getIP ( unsigned char ip[4] )
{
  if (!peer)
    return false;

  return peer->getIP(ip);
}

bool IRCServerConnectedClient::joinChannel ( const char* channel )
{
  if (!channel)
    return false;

  std::string chanName = string_util::toupper(std::string(channel));

  if (std::find(channels.begin(),channels.end(),chanName) != channels.end())
    return false;

  channels.push_back(chanName);
  return true;
}

bool IRCServerConnectedClient::partChannel ( const char* channel )
{
  if (!channel)
    return false;

  std::string chanName = string_util::toupper(std::string(channel));

  std::vector<std::string>::iterator itr = std::find(channels.begin(),channels.end(),chanName);
  if ( itr == channels.end())
    return false;

  channels.erase(itr);
  return true;
}

//---------------------------IRCServerChannel--------------------------------------
IRCServerChannel::IRCServerChannel( const char * _name )
{
  if (_name)
    name = _name;

  name = string_util::toupper(name);
}

IRCServerChannel::~IRCServerChannel()
{
  MemberDataMap::iterator itr = members.begin();
  while (itr != members.end())
    deleteUserData((itr++)->second);
}

void IRCServerChannel::addMember ( IRCServerConnectedClient *member )
{
  MemberDataMap::iterator itr = members.find(member);
  if (itr != members.end())
    return;

  members[member] = newUserData();
}

void IRCServerChannel::removeMember ( IRCServerConnectedClient *member )
{
  MemberDataMap::iterator itr = members.find(member);
  if (itr == members.end())
    return;

  deleteUserData(itr->second);
  members.erase(itr);
}

bool IRCServerChannel::sendText ( const std::string &text )
{
  if (!text.size())
    return false;

  // format the command here so it's FROM the channel


  MemberDataMap::iterator itr = members.begin();
  while (itr != members.end())
  {
    itr->first->sendText(text);
    itr++;
  }
  return true;
}

bool IRCServerChannel::sendText ( const char*text )
{
  if (!text)
    return false;

  return sendText(std::string(text));
}


//---------------------------IRCServer--------------------------------------
IRCServer::IRCServer()
:tcpConnection(TCPConnection::instance())
{
  tcpServer = NULL;

  debugLogLevel = 0;
  logHandler = &defaultLogger;

  ircMessageTerminator = "\r\n";
  ircCommandDelimator   = " ";

  minCycleTime = 0.1f;
}

IRCServer::~IRCServer()
{
  disconnect(std::string("deconstruction"));
}

IRCServer::ClientList::iterator IRCServer::getClientItr ( IRCServerConnectedClient *client )
{
  return std::find(clients.begin(), clients.end(),client);
}

void IRCServer::setLogHandler ( IRCServerLogHandler * logger )
{
  if (!logger)
    logHandler = &defaultLogger;
  else
    logHandler = logger;
}

void IRCServer::setLogfile ( std::string file )
{
  logfile = file;
}

std::string  IRCServer::getLogfile ( void )
{
  return logfile;
}

void IRCServer::setDebugLevel ( int level )
{
  debugLogLevel = level;
}

int IRCServer::getDebugLevel ( void )
{
  return debugLogLevel;
}

bool IRCServer::listen ( int maxConnections, int port )
{
  if (tcpServer)
  {
    tcpServer->disconnect();
    tcpConnection.deleteServerConnection(tcpServer);
  }

  if (maxConnections < 0)
    maxConnections = 128;

  ircServerPort = _DEFAULT_IRC_PORT;
  if ( port > 0 )
    ircServerPort = (unsigned short)port;

  tcpServer = tcpConnection.newServerConnection(ircServerPort,maxConnections);
  tcpServer->addListener(this);

  return tcpServer->getLastError() == eTCPNoError;
}

bool IRCServer::disconnect ( std::string reason )
{
  if (tcpServer)
  {
    tcpServer->disconnect();
    tcpConnection.deleteServerConnection(tcpServer);
    tcpServer = NULL;

    for (unsigned int i = 0; i < (unsigned int)clients.size(); i++ )
      deleteClient(clients[i]);

    for (unsigned int i = 0; i < (unsigned int)channels.size(); i++ )
      deleteClient(clients[i]);

    clients.clear();
    return true;
  }

  return false;
}

bool IRCServer::process ( void )
{
  return tcpConnection.update() == eTCPNoError;
}

void IRCServer::log ( std::string text, int level )
{
  if (level <= debugLogLevel && logHandler)
    logHandler->log(*this,level,text);
}

void IRCServer::log ( const char *text, int level )
{
  log(std::string(text),level);
}

void IRCServer::processIRCLine ( std::string line, IRCServerConnectedClient *client )
{
  // we have a single line of text, do something with it.
  // see if it's a command, and or call any handlers that we have
  // also check for error returns

  // right now we don't know if it's an IRC or CTCP command so just go with the generic one
  // let the command parse it out into paramaters and find the command
  BaseIRCCommandInfo  commandInfo;
  commandInfo.parse(line);
  std::string handler;

  //if (!commandInfo.prefixed)
 //   commandInfo.source = getServerHost();

  clientIRCCommand(commandInfo,client);
}

bool IRCServer::sendTextToPeer ( const std::string &text, TCPServerConnectedPeer *peer )
{
  if (!peer || !tcpServer->listening())
    return false;

  std::string message = text;
  if (text.size())
  {
    message += ircMessageTerminator;
    teTCPError  error = peer->sendData(message);
    if (error == eTCPNoError)
      log("Send Data:" + text,2);
    else
    {
      switch (error)
      {
      case eTCPNotInit:
        log("Send Data Error: TCP Not Initalised: data=" + text,0);
        break;

      case eTCPSocketNFG:
        log("Send Data Error: Bad Socket: data=" + text,0);
        break;

      case eTCPDataNFG:
        log("Send Data Error: Bad Data",0);
        break;

      case eTCPConnectionFailed:
        log("Send Data Error: TCP Connection failed",0);
        break;

      default:
        log("Send Data Error:Unknown Error",0);
      }
      return false;
    }
  }
  else
    return false;

  // prevent that thar flooding
  IRCOSSleep(minCycleTime);
  return true;
}

bool IRCServer::connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
{
  if (!connection || !peer)
    return false;

  unsigned char ip[4];
  peer->getIP(ip);

  if (!allowConnection(peer->getHostMask().c_str(),ip))
    return false;

  unsigned int index = (unsigned int )clients.size();
  IRCServerConnectedClient*  client = newClient(this,peer);

  clients.push_back(client);
  peer->setParam(clients[index]);

  clientConnect(clients[index]);

  return true;
}

void IRCServer::pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count )
{
  IRCServerConnectedClient* client = (IRCServerConnectedClient*)peer->getParam();
  if (!client)  // somehow out of band connection, screw it
    return;

  tvPacketList &packets = peer->getPackets();
  std::string theLine = client->lastData;

  for ( unsigned int p = 0; p < packets.size(); p++ )
  {
    TCPPacket  &packet = packets[p];

    unsigned int size;
    unsigned char*  data = packet.get(size);

    for ( unsigned int i = 0; i < size; i++ )
    {
      if ( data[i] != 13 )
  theLine += data[i];
      else
      {
  processIRCLine(theLine,client);
  theLine = "";
      }
    }
  }

  client->lastData = theLine;
  peer->flushPackets();
}

void IRCServer::disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced )
{
  IRCServerConnectedClient* client = (IRCServerConnectedClient*)peer->getParam();
  if (!client)  // somehow out of band connection, screw it
    return;

  clientDisconnect(client);

  ClientList::iterator clientItr = getClientItr(client);
  if (clientItr != clients.end())
    clients.erase(clientItr);

  deleteClient(client);
}

// base IRC event handlers

void IRCServer::clientConnect ( IRCServerConnectedClient *client )
{
}

void IRCServer::clientDisconnect ( IRCServerConnectedClient *client )
{
}

bool IRCServer::allowConnection ( const char* hostmask, unsigned char ip[4] )
{
  return true;
}

void IRCServer::clientIRCCommand ( const BaseIRCCommandInfo &command, IRCServerConnectedClient *client )
{
  receveCommand(std::string("ALL"),client,command);

  if (atoi(command.command.c_str()) != 0) 
    receveCommand( std::string("NUMERIC"),client,command);

  // notify any handlers for this specific command
  receveCommand(command.command,client,command);
}

bool IRCServer::receveCommand ( const std::string &commandName, IRCServerConnectedClient *client, const BaseIRCCommandInfo &info )
{
  tmCommandHandlerListMap::iterator itr = userCommandHandlers.find(commandName);
  if ( itr != userCommandHandlers.end() )
  {
    bool callDefault = false;
    for ( unsigned int i = 0; i < (unsigned int) itr->second.size(); i++ )
    {
      if ( itr->second[i]->receve(this,client,commandName,info))
	callDefault = true;
    }
    if (!callDefault)
      return true;
  }

  tmCommandHandlerMap::iterator defaultIter = defaultCommandHandlers.find(commandName);
  if ( defaultIter != defaultCommandHandlers.end() && defaultIter->second )
    return defaultIter->second->receve(this,client,commandName,info);

  return false;
}

void IRCServer::addDefaultCommandHandlers ( IRCServerCommandHandler* handler )
{
  defaultCommandHandlers[handler->getCommandName()] = handler;
}

void IRCServer::clearDefaultCommandHandlers ( void )
{
  tmCommandHandlerMap::iterator  itr = defaultCommandHandlers.begin();

  while (itr != defaultCommandHandlers.end())
  {
    delete(itr->second);
    itr++;
  }
  defaultCommandHandlers.clear();
}

void IRCServer::registerDefaultCommandHandlers ( void )
{

}

bool IRCServer::registerCommandHandler ( IRCServerCommandHandler *handler )
{
  if (!handler)
    return false;

  std::string command = handler->getCommandName();

  tmCommandHandlerListMap::iterator  commandListItr = userCommandHandlers.find(command);
  if (commandListItr == userCommandHandlers.end())
  {
    std::vector<IRCServerCommandHandler*> handlerList;
    handlerList.push_back(handler);
    userCommandHandlers[command] = handlerList;
  }
  else
    commandListItr->second.push_back(handler);

  return true;
}

bool IRCServer::removeCommandHandler ( IRCServerCommandHandler *handler )
{
  if (!handler)
    return false;

  std::string command = handler->getCommandName();

  tmCommandHandlerListMap::iterator    commandListItr = userCommandHandlers.find(command);
  if (commandListItr == userCommandHandlers.end())
    return false;
  else
  {
    std::vector<IRCServerCommandHandler*>::iterator  itr = commandListItr->second.begin();
    while ( itr != commandListItr->second.end())
    {
      if (*itr == handler)
	itr = commandListItr->second.erase(itr);
      else
	itr++;
    }
  }
  return true;
}

int IRCServer::listUserHandledCommands ( std::vector<std::string> &commandList )
{
  commandList.clear();

  tmCommandHandlerListMap::iterator  itr = userCommandHandlers.begin();

  while (itr != userCommandHandlers.end())
  {
    commandList.push_back(itr->first);
    itr++;
  }
  return (int)commandList.size();
}

int IRCServer::listDefaultHandledCommands ( std::vector<std::string> &commandList )
{
  commandList.clear();

  tmCommandHandlerMap::iterator  itr = defaultCommandHandlers.begin();

  while (itr != defaultCommandHandlers.end())
  {
    commandList.push_back(itr->first);
    itr++;
  }
  return (int)commandList.size();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
