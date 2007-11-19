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

#include "botlib.h"

#ifndef _LIBIRC_NO_BOTMAIN
LibIRCBot *theBot = NULL;
#endif

LibIRCBot::LibIRCBot()
{
#ifndef _LIBIRC_NO_BOTMAIN
  theBot = this;
#endif

  disconnect = false;
  sleepTime = 1;
  commandsRegistered = false;
}

LibIRCBot::~LibIRCBot()
{
#ifndef _LIBIRC_NO_BOTMAIN
  theBot = NULL;
#endif
}

int LibIRCBot::run ( void )
{
  connectionRecord.port = 6667;

  init(connectionRecord);
  if (!verifyConRec() || !connect())
    return -1;

  while (!runOneLoop())
    IRCOSSleep(sleepTime);

  client.disconnect("quiting");
  return 0;
}

bool LibIRCBot::runOneLoop ( void )
{
  if (disconnect)
    return false;

  client.process();
  processPendingActions();
  return true;
}

bool LibIRCBot::connect ( void )
{
  currentNicIndex = 0;
  disconnect = false;
  pendingJoins = -1;
  registerStandardEventHandlers();
  bool connected = client.connect(connectionRecord.server,connectionRecord.port);

  return connected;
}

bool LibIRCBot::process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info )
{
  switch (eventType)
  {
    case eIRCConnectedEvent:
      serverLogin();
      return true;

    case eIRCWelcomeEvent:
      welcomeMessage();
      return true;

    case eIRCChannelJoinEvent:
      channelJoin((trClientJoinEventInfo*)&info);
      return true;

    case eIRCUserJoinEvent:
      userJoin((trClientJoinEventInfo*)&info);
      return true;

   case eIRCUserPartEvent:
      userPart((trClientPartEventInfo*)&info);
      return true;

    case eIRCPrivateMessageEvent:
      chatMessage ((trClientMessageEventInfo*)&info,false);
      return true;

    case eIRCChannelMessageEvent:
      chatMessage ((trClientMessageEventInfo*)&info,true);
      return true;

    case eIRCNickNameError:
      return nickError();
  }
  return unhandledEvent(info);
}

void LibIRCBot::processPendingActions ( void )
{
  if (client.getNick().size() && pendingJoins > 0)
  {
    client.join(connectionRecord.channels[pendingJoins-1]);
    pendingJoins--;
  }
}

bool LibIRCBot::verifyConRec ( void )
{
  if (!connectionRecord.server.size())
    return false;
  
  if (connectionRecord.port <  1)
    connectionRecord.port = 6667;

  if (!connectionRecord.nicks.size() || !connectionRecord.nicks[0].size())
    return false;

  return true;
}

void  LibIRCBot::registerStandardEventHandlers ( void )
{
  if (commandsRegistered)
    return;

  client.registerEventHandler(eIRCConnectedEvent,this);
  client.registerEventHandler(eIRCWelcomeEvent,this);

  client.registerEventHandler(eIRCChannelJoinEvent,this);
  client.registerEventHandler(eIRCUserJoinEvent,this);
  client.registerEventHandler(eIRCUserPartEvent,this);
 
  client.registerEventHandler(eIRCNickNameError,this);
  client.registerEventHandler(eIRCNickNameChange,this);

  client.registerEventHandler(eIRCChannelMessageEvent,this);
  client.registerEventHandler(eIRCPrivateMessageEvent,this);
}

void LibIRCBot::serverLogin ( void )
{
  client.login(connectionRecord.nicks[currentNicIndex],connectionRecord.username,connectionRecord.realName,connectionRecord.hostmask);
  onLogn();
}

void LibIRCBot::welcomeMessage ( void )
{
  pendingJoins = (int)connectionRecord.channels.size();
  processPendingActions();
}

void LibIRCBot::channelJoin ( trClientJoinEventInfo* info )
{
  onChannelJoin(info->channel);
}

void LibIRCBot::userJoin ( trClientJoinEventInfo* info )
{
  onUserJoin(info->channel,info->user);
}

void LibIRCBot::userPart ( trClientPartEventInfo* info )
{
  onUserPart(info->channel,info->user,info->reason);
}

void LibIRCBot::chatMessage ( trClientMessageEventInfo* info, bool inChannel )
{
  LibIRCBotMessage message;
  message.message = info->message;
  message.respond = info->target;
  message.from = info->from;

  if(inChannel)
    onChannelMessage(message,isForMe(info->message));
  else
    onPrivateMessage(message);
}

bool LibIRCBot::nickError ( void )
{
  currentNicIndex++;

  std::string nick;
  if (currentNicIndex < (int)connectionRecord.nicks.size())
    nick = connectionRecord.nicks[currentNicIndex];

  onNickNameError(nick);
  if (nick.size())
  {
    client.login(nick,connectionRecord.username,connectionRecord.realName,connectionRecord.hostmask);
    return false;
  }
   
  return true;
}

void LibIRCBot::disconectFromServer ( const char* r )
{
  std::string reason;
  if(r)
    reason = r;
  client.disconnect(reason);
  disconnect = true;
}

void LibIRCBot::disconectFromServer ( const std::string& reason )
{
  client.disconnect(reason);
  disconnect = true;
}

void LibIRCBot::respond ( const LibIRCBotMessage &to, const char* text, bool action, bool privately )
{
  if (!to.respond.size() || !text)
    return;

  bool isChannel = to.respond[0] == '#';
  std::string target = to.respond;
  if (isChannel && privately)
    target = to.from;
   client.sendMessage(target,std::string(text),action);
}

void LibIRCBot::respond ( const LibIRCBotMessage &to, const std::string &text, bool action, bool privately )
{ 
  if(!to.respond.size() || !text.size())
    return;

  respond(to,text.c_str(),action,privately);
}

void LibIRCBot::send ( const char* to, const char* text, bool action )
{ 
  if(to && text)
    client.sendMessage(std::string(to),std::string(text),action);
}

void LibIRCBot::send ( const std::string to, const char* text, bool action  )
{ 
  if(text && to.size())
    client.sendMessage(to,std::string(text),action);
}

void LibIRCBot::send ( const char* to, const std::string &text, bool action  )
{ 
  if(to && text.size())
    client.sendMessage(std::string(to),text,action);
}

void LibIRCBot::send ( const std::string &to, const std::string &text, bool action  )
{ 
  if (to.size() && text.size())
    client.sendMessage(to,text,action);
}

bool LibIRCBot::isForMe ( const std::string &message )
{
  if (!message.size())
    return false;

  // check for nickshortcuts
  // if the first caracter(s) are one of the shortcuts then go and let that fly
  if (nickShortcuts.size())
  {
    for ( int i = 0; i < (int)nickShortcuts.size(); i++ )
    {
      if (nickShortcuts[i].size() && message.size() > nickShortcuts[i].size())
      {
	if (strncasecmp(nickShortcuts[i].c_str(),message.c_str(),nickShortcuts[i].size()) == 0)
	  return true;
      }
    }
  }

  std::string nickName = client.getNick();
  if (message.size() > nickName.size() + 2)
  {
    if (strncasecmp(nickName.c_str(),message.c_str(),nickName.size()) == 0)
    {
      char charAfterNick = message.c_str()[nickName.size()+1];
      if ( !string_util::isAlphabetic(charAfterNick)  && !string_util::isNumeric(charAfterNick) )
	return true;
    }
  }
  return false;
}


//----------------LibIRCBotConfigDataValue
LibIRCBotConfigDataValue::LibIRCBotConfigDataValue(const char* data )
{
  if (data)
    value =data;
}

LibIRCBotConfigDataValue::LibIRCBotConfigDataValue ( const std::string &data )
{
  value = data;
}

std::vector<std::string> LibIRCBotConfigDataValue::params ( void )
{
  return string_util::tokenize(value,std::string(" "),0,true);
}

std::string LibIRCBotConfigDataValue::param ( unsigned int index )
{
  std::vector<std::string> params = string_util::tokenize(value,std::string(" "),0,true);
  if ( index >= params.size() )
    return std::string("");
  return params[index];
}

//----------------LibIRCBotConfigItem

LibIRCBotConfigItem::LibIRCBotConfigItem ( const char *_key, const char* _data )
{
  if (_key)
    key = _key;
  set(_data);
}

LibIRCBotConfigItem::LibIRCBotConfigItem ( const std::string &_key, const std::string &_data )
{
  key = _key;
  set(_data);
}

void LibIRCBotConfigItem::set( const char *data )
{
  if (data)
    values.push_back(LibIRCBotConfigDataValue(data));
}

void LibIRCBotConfigItem::set( const std::string &data )
{
  values.push_back(LibIRCBotConfigDataValue(data));
}

bool LibIRCBotConfigItem::write( std::string &config )
{
  if (!values.size())
    return false;

  for ( int i = 0; i < (int)values.size(); i++ )
    config += key + ":" + values[i].value + "\n";

  return true;
}

//----------------LibIRCBotConfig
LibIRCBotConfig::LibIRCBotConfig( const char* file )
{
  if (file)
    read(file);
}

LibIRCBotConfig::~LibIRCBotConfig()
{
}

bool LibIRCBotConfig::read ( const char * file )
{
  return false;
}

bool LibIRCBotConfig::write ( const char * file )
{
  return false;
}

LibIRCBotConfigDataValueList& LibIRCBotConfig::getKeyItems( const char* key )
{
  static LibIRCBotConfigDataValueList list;
  return list;
}

LibIRCBotConfigDataValueList& LibIRCBotConfig::getKeyItems( const std::string &key )
{
  static LibIRCBotConfigDataValueList list;
  return list;
}

void LibIRCBotConfig::addItem ( const char* key, const char* value )
{
}

void LibIRCBotConfig::addItem ( const std::string &key, const std::string &value )
{
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
