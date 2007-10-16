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

#include "agents.h"
#include "OSFile.h"
#include "TextUtils.h"

//--------------------IRCServerChannel------------------------------
ServerChannel::~ServerChannel()
{
}

//--------------------IRCServerPrivateMessage------------------------------
ServerPrivateMessage::~ServerPrivateMessage()
{
}

//--------------------AgentConnectedServer------------------------------
AgentConnectedServer::AgentConnectedServer()
{
  currentHost = -1;
  ircConenction.registerEventHandler(eIRCConnectedEvent,this);
  ircConenction.registerEventHandler(eIRCNickNameError,this);
  ircConenction.registerEventHandler(eIRCChannelMessageEvent,this);
  ircConenction.registerEventHandler(eIRCPrivateMessageEvent,this);
  ircConenction.registerEventHandler(eIRCNickNameChange,this);
}

AgentConnectedServer::~AgentConnectedServer()
{

}

bool AgentConnectedServer::init ( const std::string &name, const std::string &rootDir )
{
  if (!hostnames.size() || !name.size() || !rootDir.size() )
    return false;

  networkName = name;
  dataDir = rootDir + name;

  if ( connectionStatus == eConnecting || connectionStatus == eConnected )
    return true;

  // try the first host, prefered hosts will be at the front
  currentHost = 0;
  ircConenction.connect(hostnames[currentHost].host,hostnames[currentHost].port);
  
  connectionStatus = eConnecting;
  return false;
}

bool AgentConnectedServer::update ( void )
{
  return true;
}

bool AgentConnectedServer::connected ( void )
{
  return false;
}

void AgentConnectedServer::addHost ( ServerHost &host, bool prefered )
{
  if (prefered)
    hostnames.insert(hostnames.begin(),host);
  else
    hostnames.push_back(host);
}

void AgentConnectedServer::addNick ( const std::string & nick )
{
  if (nick.size())
    ircNicks.push_back(nick);
}

void AgentConnectedServer::setUserInfo ( const std::string &r, const std::string &u )
{
  username = u;
  fullname = r;
}


bool AgentConnectedServer::process ( IRCClient &ircClient, teIRCEventType  eventType, trBaseEventInfo &info )
{
  switch (eventType)
  {
  case eIRCConnectedEvent:
    connectionStatus = eConnected;
    connectToServer();
    break;

  case eIRCNickNameChange:
    currentNick = ((trClientNickChangeEventInfo&)info).newName;
    joinChannels();
    break;

  case eIRCNickNameError:
    tryNextNick();
    break;

  default:
    return false;
  }

  return true;
}

void AgentConnectedServer::connectToServer ( void )
{
  std::string nick;
  if (!ircNicks.size())
    nick = TextUtils::format("%d",(int)rand());
  else
    nick = ircNicks[0];

  if (!username.size())
    username = nick;
  if (!fullname.size())
    fullname = nick;

  lastAtemptedNick = 0;
  if (!ircConenction.login(nick,username,fullname,std::string("localhost")))
    connectionStatus = eErrored;
}

void AgentConnectedServer::joinChannels ( void )
{
}

void AgentConnectedServer::tryNextNick ( void )
{
  if ( lastAtemptedNick >= (int)ircNicks.size() )
  {
    ircConenction.disconnect(std::string("no available nicks"));
    connectionStatus = eErrored;
  }
  else
  {
    if (!ircConenction.login(ircNicks[++lastAtemptedNick],username,fullname,std::string("localhost")))
      connectionStatus = eErrored;
  }
}

//--------------------Agent------------------------------

Agent::Agent()
{
}

Agent::Agent(const char* workingDir)
{
}

Agent::~Agent()
{
}

bool Agent::valid ( void )
{
  AgentConnectedServersMap::iterator itr = servers.begin();
  while ( itr != servers.end() )
    if ((itr++)->second->valid())
      return true;
 
  return false;
}

bool Agent::connected ( void )
{
  return valid();
}

bool Agent::update ( void )
{
  AgentConnectedServersMap::iterator itr = servers.begin();
  while ( itr != servers.end() )
    (itr++)->second->update();

  return valid();
}

bool Agent::loadFromDir ( const char* dir )
{
  if (dirName.size() || !dir)
    return false;

  COSFile	file;
  file.OSName(dir);
  std::string fileName = file.GetStdName();
  fileName += "/config.cfg";
  file.StdName(fileName.c_str());

  if (!file.Open("rt"))
    return false;

  agentName = "";

  std::string configText;
  if(!file.GetFileText(configText))
    return false;

  file.Close();

  configText = TextUtils::replace_all(configText,std::string("\r"),std::string(""));
  std::vector<std::string> lines = TextUtils::tokenize(configText,std::string("\n"));
	
  for ( int i = 0; i < (int)lines.size(); i++ )
  {
    const std::string &line = lines[i];
    std::vector<std::string> chunks = TextUtils::tokenize(line,std::string(" "),0,true);
    if ( chunks.size() > 1 )
    {
      std::string command = TextUtils::tolower(chunks[0]);
      if ( command == "name" )
	agentName = chunks[1];
      else if ( command == "server" )
      {
	// server name host port prefered
	if ( chunks.size() >= 4 )
	{
	  ServerHost host;
	  host.host = chunks[2];
	  host.port = atoi(chunks[3].c_str());

	  bool prefered = false;
	  if ( chunks.size() > 4 )
	    prefered = TextUtils::tolower(chunks[4]) == "prefered";

	  getServer(TextUtils::tolower(chunks[1]))->addHost(host,prefered);
	}
      }
      else if ( command == "nick" && chunks.size() > 2)
	getServer(TextUtils::tolower(chunks[1]))->addNick(chunks[2]);
      else if ( command == "user" && chunks.size() > 3)
	getServer(TextUtils::tolower(chunks[1]))->setUserInfo(chunks[2],chunks[3]);
    }
  }

  if (!agentName.size())
    return false;

  return agentName.size() > 0;
}

bool Agent::init ( void )
{
  // after we have the server list, we check them all to make
  // sure they have a name. if they don't
  // then we use the agent name.
  // we also tell them all to each scan thier own dirs

  AgentConnectedServersMap::iterator itr = servers.begin();
  while ( itr != servers.end() )
  {
    if (!itr->second->validNickList())
      itr->second->addNick(agentName);

    if (!itr->second->valid())
      servers.erase(itr++);
    else
    {
      // if it's valid, then tell it to load up and connect
      // all agents in the config file are consisered to be auto load.
      // there may be more agent dirs, but ones not
      // in the config will have to be manualy started by the clients
      // and possibly added to the config after that.
      if (!itr->second->init(itr->first,dirName))
	servers.erase(itr++);
      else
	itr++;
    }
  }

  return valid();
}

bool Agent::saveToDir ( void )
{
  if (!dirName.size())
    return false;

  return false;
}

AgentConnectedServer* Agent::getServer ( const std::string &name )
{
  AgentConnectedServersMap::iterator itr = servers.find(name);

  if (itr == servers.end())
  {
    AgentConnectedServer *s = new AgentConnectedServer;
    servers[name] = s;
    return s;
  }
   
  return itr->second;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
