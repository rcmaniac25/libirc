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

// implementation of main libIRC Client classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "IRCTextUtils.h"

#ifndef _WIN32
  #include <unistd.h>
#else
  #include <windows.h>
  #include <time.h>
  #include <stdio.h>
#endif

class DefaultIRCLogHandler : public IRCClientLogHandler
{
public:
  virtual ~DefaultIRCLogHandler(){return;}
  virtual void log ( IRCClient &client, int level, std::string line )
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

static DefaultIRCLogHandler  defaultLogger;

// IRC class stuff

IRCClient::IRCClient(CIRCClientNetworkHandler *h)
:tcpConnection(TCPConnection::instance())
{
  netHandler = h;
  tcpClient = NULL;
  registerDefaultCommandHandlers();
  init();

  ircMessageTerminator = "\r\n";
  ircCommandDelimator   = " ";
  debugLogLevel = 0;
  ircServerPort = _DEFAULT_IRC_PORT;
  ircConnectionState = eNotConnected;
  logHandler = &defaultLogger;
  connected = false;
}

// irc client
IRCClient::~IRCClient()
{
  disconnect("shutting down");

  if (tcpClient)
    tcpConnection.deleteClientConnection(tcpClient);
  else if ( netHandler )
    netHandler->disconnect(true);

//  tcpConnection.kill();
}

// general connection methods
bool IRCClient::init ( void )
{
  minCycleTime = 0.1f;
  registered = false;
  connected = false;
  nickname = "";
  // if any old conenctions are around, kill em
  if (netHandler)
    netHandler->disconnect(false);
  else  if (tcpClient)
    tcpConnection.deleteClientConnection(tcpClient);

  tcpClient = NULL;

  // make sure the system we have is inited
  //tcpConnection.init();

  // just get us a new empty connection
  if (!netHandler)
    tcpClient = tcpConnection.newClientConnection("",0);

  if (tcpClient)
    tcpClient->addListener(this);

  return netHandler || (tcpClient != NULL);
 }

bool IRCClient::connect ( std::string server, int port )
{
  if (!netHandler && (!tcpClient || !server.size()))
    return false;

  connected = false;
  reportedServerHost = ircServerName = server;
  ircServerPort = _DEFAULT_IRC_PORT;
  if ( port > 0 )
    ircServerPort = (unsigned short)port;

  teTCPError err;
  if (netHandler)
    err = netHandler->connect(server.c_str(),ircServerPort) ? eTCPNoError : eTCPConnectionFailed;
  else
    err = tcpClient->connect(server,ircServerPort);

  ircConnectionState = err == eTCPNoError ? eTCPConenct : eNotConnected;

  return err == eTCPNoError;
}

bool IRCClient::disconnect ( std::string reason )
{
  connected = false;
  if (ircConnectionState >= eLoggedIn)
  {
    if (!reason.size())
      reason = "shuting down";

    IRCCommandInfo  info;
    info.params.push_back(reason);

    if (netHandler && !sendIRCCommand(eCMD_QUIT,info))
    {
      log("Disconnect Failed: QUIT command not sent",0);
      return false;
    }

    ircConnectionState = eNotConnected;
    if (netHandler)
    {
      netHandler->disconnect(false);
      return false;
    }
    else
      return tcpClient->disconnect() == eTCPNoError;
  }
  return false;
}

void IRCClient::addDataToLine ( std::string &theLine, unsigned int len, const char *data )
{
  unsigned int count = 0;

  while (count < len)
  {
    if (data[count] == 13)
    {
      if (theLine.size())
      {
	processIRCLine(theLine);
	theLine = "";
      }
      if (count != len-1)
	count++;
    }
    else
    {
      if (data[count] != 10)
	theLine += data[count];
    }
    count++;
  }
}

// update loop methods
bool IRCClient::process ( void )
{
  if (netHandler)
  {
    netHandler->update();

    std::string  theLine = lastRecevedData;

    int pendingCount = netHandler->pendingPackets();
    for ( int i = 0; i < pendingCount; i++ )
    {
      addDataToLine(theLine,netHandler->getPacketSize(i),netHandler->getPacketData(i));
      // save off anything left
      lastRecevedData = theLine;
    }
    netHandler->flushInboundPackets();
  }
  else
    return tcpConnection.update()==eTCPNoError;

  return true;
}


void IRCClient::pending ( TCPClientConnection *connection, int count )
{
  // we got some data, do something with it
  log("Data Pending notification",5);
  // ok we have to parse this tuff into "lines"
  std::string  theLine = lastRecevedData;
  tvPacketList  &packets = connection->getPackets();

  if (!connected)
  {
    // we know we are connected here cus we have data pending
    if (getConnectionState() < eTCPConenct)
      setConnectionState(eTCPConenct);

    trBaseEventInfo  info;  // no info
    callEventHandler(eIRCConnectedEvent,info);

    connected = true;
  }

  while(packets.size())
  {
    TCPPacket  &packet = *(packets.begin());

    unsigned int  len;
    char* data  = (char*)packet.get(len);
    addDataToLine(theLine,len,data);

    // save off anything left
    lastRecevedData = theLine;

    packets.erase(packets.begin());
  }
}

void IRCClient::processIRCLine ( std::string line )
{
  // we have a single line of text, do something with it.
  // see if it's a command, and or call any handlers that we have
  // also check for error returns

  // right now we don't know if it's an IRC or CTCP command so just go with the generic one
  // let the command parse it out into paramaters and find the command
  BaseIRCCommandInfo  commandInfo;
  commandInfo.parse(line);
  std::string handler;

  if (!commandInfo.prefixed)
    commandInfo.source = getServerHost();

  // call the "ALL" handler special if there is one
  receiveCommand(std::string("ALL"),commandInfo);

  if (atoi(commandInfo.command.c_str()) != 0) 
    receiveCommand(std::string("NUMERIC"),commandInfo);

  // notify any handlers for this specific command
  receiveCommand(commandInfo.command,commandInfo);
}

bool IRCClient::sendIRCCommandToServer ( teIRCCommands  command, std::string &data)
{
  std::string text = ircCommandParser.getCommandName(command) + ircCommandDelimator + data;
  return sendTextToServer(text);
}

// utility methods
bool IRCClient::sendTextToServer ( std::string &text )
{
  if (!text.size())
    return false;

  std::string message = text + ircMessageTerminator;

  if (netHandler)
  {
    if ( !netHandler->connected() )
      return false;

    return netHandler->send(message.c_str(),(int)message.size());
  }

  if (!tcpClient || !tcpClient->connected())
    return false;

  teTCPError  error = tcpClient->sendData(message);
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

  // prevent that thar flooding
  IRCOSSleep(minCycleTime);
  return true;
}

void  IRCClient::setLogHandler ( IRCClientLogHandler * logger )
{
  if (!logger)
    logHandler = &defaultLogger;
  else
    logHandler = logger;
}

void IRCClient::log ( const char *text, int level )
{
  log(std::string(text),level);
}

void IRCClient::log ( std::string text, int level )
{
  if (level <= debugLogLevel && logHandler)
    logHandler->log(*this,level,text);
}

void IRCClient::setLogfile ( std::string file )
{
  logfile = file;
}

std::string  IRCClient::getLogfile ( void )
{
  return logfile;
}

void IRCClient::setDebugLevel ( int level )
{
  debugLogLevel = level;
}

int IRCClient::getDebugLevel ( void )
{
  return debugLogLevel;
}

bool IRCClient::sendCommand ( std::string &commandName, BaseIRCCommandInfo &info )
{
  tmUserCommandHandlersMap::iterator    commandListItr = userCommandHandlers.find(commandName);

  bool callDefault = true;

  if (commandListItr != userCommandHandlers.end() && commandListItr->second.size())  // do we have a custom command handler
  {
    // someone has to want us to call the defalt now
    callDefault = false;
    // is this right?
    // should we do them all? or just the first one that "HANDLES" it?
    std::vector<IRCClientCommandHandler*>::iterator  itr = commandListItr->second.begin();
    while (itr != commandListItr->second.end())
    {
      if ( (*itr)->send(*this,commandName,info))
        callDefault = true;
      itr++;
    }
    return true;
  }

  if (callDefault)  // check for the default
  {
    tmCommandHandlerMap::iterator itr = defaultCommandHandlers.find(commandName);
    if (itr != defaultCommandHandlers.end())
    {
      itr->second->send(*this,commandName,info);
      return true;
    }
  }
  return false;
}

bool IRCClient::sendIRCCommand ( teIRCCommands  command, IRCCommandInfo &info )
{
  info.type = eIRCCommand;
  info.ircCommand = command;
  info.command = ircCommandParser.getCommandName(command);
  return sendCommand(info.command,info);
}

bool IRCClient::sendCTMPCommand ( teCTCPCommands  command, CTCPCommandINfo &info )
{
  info.type = eCTCPCommand;
  info.ctcpCommand = command;
  info.command = ctcpCommandParser.getCommandName(command);
  return sendCommand(info.command,info);
}

bool IRCClient::receiveCommand ( const std::string &commandName, BaseIRCCommandInfo &info )
{
  tmUserCommandHandlersMap::iterator    commandListItr = userCommandHandlers.find(commandName);

  bool callDefault = true;

  if (commandListItr != userCommandHandlers.end() && commandListItr->second.size())  // do we have a custom command handler
  {
    // someone has to want us to call the defalt now
    callDefault = false;
    // is this right?
    // should we do them all? or just the first one that "HANDLES" it?
    std::vector<IRCClientCommandHandler*>::iterator  itr = commandListItr->second.begin();
    while (itr != commandListItr->second.end())
    {
      if ( (*itr)->receive(*this,commandName,info))
        callDefault = true;
      itr++;
    }
    if (!callDefault)
      return true;
  }

  if (callDefault)  // check for the default
  {
    tmCommandHandlerMap::iterator itr = defaultCommandHandlers.find(commandName);
    if (itr != defaultCommandHandlers.end())
    {
      itr->second->receive(*this,commandName,info);
      return true;
    }
  }
  return false;
}

bool IRCClient::receiveIRCCommand ( teIRCCommands  command, IRCCommandInfo &info )
{
  info.type = eIRCCommand;
  info.ircCommand = command;
  return receiveCommand(info.command,info);
}

bool IRCClient::receiveCTMPCommand ( teCTCPCommands  command, CTCPCommandINfo &info )
{
  info.type = eCTCPCommand;
  info.ctcpCommand = command;
  return receiveCommand(info.command,info);
}

bool IRCClient::registerCommandHandler ( IRCClientCommandHandler *handler )
{
  if (!handler)
    return false;

  std::string command = handler->getCommandName();

  tmUserCommandHandlersMap::iterator  commandListItr = userCommandHandlers.find(command);
  if (commandListItr == userCommandHandlers.end())
  {
    std::vector<IRCClientCommandHandler*> handlerList;
    handlerList.push_back(handler);
    userCommandHandlers[command] = handlerList;
  }
  else
    commandListItr->second.push_back(handler);

  return true;
}

bool IRCClient::removeCommandHandler ( IRCClientCommandHandler *handler )
{
  if (!handler)
    return false;

  std::string command = handler->getCommandName();

  tmUserCommandHandlersMap::iterator    commandListItr = userCommandHandlers.find(command);
  if (commandListItr == userCommandHandlers.end())
    return false;
  else
  {
    // don't delete anything here, as we didn't allocate the handlers
    std::vector<IRCClientCommandHandler*>::iterator itr = std::find(commandListItr->second.begin(),commandListItr->second.end(),handler);
    if (itr != commandListItr->second.end())
      commandListItr->second.erase(itr);
  }
  return true;
}

int IRCClient::listUserHandledCommands ( std::vector<std::string> &commandList )
{
  commandList.clear();

  tmUserCommandHandlersMap::iterator  itr = userCommandHandlers.begin();

  while (itr != userCommandHandlers.end())
  {
    commandList.push_back(itr->first);
    itr++;
  }
  return (int)commandList.size();
}

int IRCClient::listDefaultHandledCommands ( std::vector<std::string> &commandList )
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

void IRCClient::addDefaultCommandHandlers ( IRCClientCommandHandler* handler )
{
  defaultCommandHandlers[handler->getCommandName()] = handler;
}

void IRCClient::clearDefaultCommandHandlers ( void )
{
  tmCommandHandlerMap::iterator  itr = defaultCommandHandlers.begin();

  while (itr != defaultCommandHandlers.end())
  {
    delete(itr->second);
    itr++;
  }
  defaultCommandHandlers.clear();
}

void IRCClient::registerDefaultCommandHandlers ( void )
{
  registerDefaultEventHandlers();

  userCommandHandlers.clear();
  clearDefaultCommandHandlers();

  // the "special" handlers
  addDefaultCommandHandlers(new IRCClientALLCommand );
  addDefaultCommandHandlers(new IRCClientNumericCommand );

  // basic IRC commands
  addDefaultCommandHandlers(new IRCClientNickCommand );
  addDefaultCommandHandlers(new IRCClientUserCommand );
  addDefaultCommandHandlers(new IRCClientPingCommand );
  addDefaultCommandHandlers(new IRCClientPongCommand );
  addDefaultCommandHandlers(new IRCClientNoticeCommand );
  addDefaultCommandHandlers(new IRCClientJoinCommand );
  addDefaultCommandHandlers(new IRCClientPartCommand );
  addDefaultCommandHandlers(new IRCClientQuitCommand );
  addDefaultCommandHandlers(new IRCClientModeCommand );
  addDefaultCommandHandlers(new IRCClientPrivMsgCommand );
  addDefaultCommandHandlers(new IRCClientKickCommand );
}

// logical event handlers

//tmIRCEventMap              defaultEventHandlers;
//tmIRCEventListMap          userEventHandlers;

void IRCClient::addDefaultEventHandlers ( teIRCEventType eventType, IRCClientEventCallback* handler )
{
  if (handler)
    defaultEventHandlers[eventType] = handler;
}

void IRCClient::clearDefaultEventHandlers ( void )
{
  tmIRCClientEventMap::iterator itr = defaultEventHandlers.begin();

  while ( itr != defaultEventHandlers.end())
  {
    if (itr->second && (itr->second != this) )
      delete(itr->second);
    itr++;
  }
  defaultEventHandlers.clear();
}

void IRCClient::registerDefaultEventHandlers ( void )
{
  userEventHandlers.clear();
  clearDefaultEventHandlers();

  addDefaultEventHandlers(eIRCNickNameError,this);
}

bool IRCClient::registerEventHandler ( teIRCEventType eventType, IRCClientEventCallback *handler )
{
  if (!handler)
    return false;

  tmIRCClientEventListMap::iterator    eventListItr = userEventHandlers.find(eventType);
  if (eventListItr == userEventHandlers.end())
  {
    tvIRCClientEventList handlerList;
    handlerList.push_back(handler);
    userEventHandlers[eventType] = handlerList;
  }
  else
    eventListItr->second.push_back(handler);

  return true;
}

bool IRCClient::removeEventHandler ( teIRCEventType eventType, IRCClientEventCallback *handler )
{
  if (!handler)
    return false;

  tmIRCClientEventListMap::iterator    eventListItr = userEventHandlers.find(eventType);
  if (eventListItr == userEventHandlers.end())
    return false;
  else
  {
    tvIRCClientEventList::iterator  itr = eventListItr->second.begin();
    while ( itr != eventListItr->second.end())
    {
      if ((*itr)== handler)
        eventListItr->second.erase(itr++);
      else
        itr++;
    }
  }
  return true;
}

void IRCClient::callEventHandler ( teIRCEventType eventType, trBaseEventInfo &info )
{
  bool callDefault = true;

  tmIRCClientEventListMap::iterator    eventListItr = userEventHandlers.find(eventType);

  // make sure the event type is cool
  info.eventType = eventType;

  if (eventListItr != userEventHandlers.end() && eventListItr->second.size())  // do we have a custom command handler
  {
    // someone has to want us to call the defalt now
    callDefault = false;
    // is this right?
    // should we do them all? or just the first one that "HANDLES" it?
    tvIRCClientEventList::iterator  itr = eventListItr->second.begin();
    while (itr != eventListItr->second.end())
    {
      if ( (*itr)->process(*this,eventType,info))
        callDefault = true;
      itr++;
    }
    if (!callDefault)
      return;
  }

  if (callDefault)  // check for the default
  {
    tmIRCClientEventMap::iterator itr = defaultEventHandlers.find(eventType);
    if (itr != defaultEventHandlers.end())
    {
      itr->second->process(*this,eventType,info);
      return;
    }
  }
  return;
}

// info methods

string_list IRCClient::listUsers ( std::string channel )
{
  if (channel.size())
    return userManager.listChannelUserNames(channel);
  
  return userManager.listUserNames();
}

string_list IRCClient::listChanOps ( std::string channel )
{
  string_list userNames;

  int channelID = userManager.getChannelID(channel);
  std::vector<int> userList = userManager.listChannelUsers(channelID);
  std::vector<int>::iterator itr = userList.begin();

  while ( itr != userList.end() )
  {
    if (userManager.userIsOp(*itr,channelID))
      userNames.push_back(userManager.getUserNick(*itr));

    itr++;
  }
  return userNames;
}

string_list IRCClient::listChannels ( void )
{
  return userManager.listChannelNames();
}

trIRCChannelPermissions IRCClient::getChanPerms ( std::string channel )
{
  return userManager.getChannelPerms(channel);
}

// default event handling

bool IRCClient::process ( IRCClient &ircClient, teIRCEventType  eventType, trBaseEventInfo &info )
{
  switch (eventType)
  {
    case eIRCNickNameError:
    {
      // atempt to keep adding crap to the nick till it goes
      requestedNick += '_';

      IRCCommandInfo  info;
      info.params.push_back(requestedNick);

      if (!sendIRCCommand(eCMD_NICK,info))
      {
        log("Nick Error Resned Failed: NICK command not sent",0);
        return false;
      }
      if (getConnectionState() < eSentNickAndUSer)
        setConnectionState(eSentNickAndUSer);
    }
    break;
  }
  return true;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
