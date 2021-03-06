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

// basic IRC commands

#include "ircBasicCommands.h"
#include "IRCTextUtils.h"
#include "IRCEvents.h"

std::string delim = " ";

// IRC "NICK" command

IRCClientNickCommand::IRCClientNickCommand()
{
  name = "NICK";
}

bool IRCClientNickCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.nickCommand(info);
  return true;
}

bool IRCClientNickCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine;

  // NICK
  commandLine = ircInfo.params[0];
  return client.sendIRCCommandToServer(eCMD_NICK,commandLine);
}

// IRC "USER" command

IRCClientUserCommand::IRCClientUserCommand()
{
  name = "USER";
}

bool IRCClientUserCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  return true;
}

bool IRCClientUserCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{  
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine;

  //username host server fullname
  commandLine = ircInfo.params[0] + delim + ircInfo.params[1] + delim + ircInfo.params[2] + delim + std::string(":") + ircInfo.params[3];
  return client.sendIRCCommandToServer(eCMD_USER,commandLine);
}

// IRC "PING" command
IRCClientPingCommand::IRCClientPingCommand()
{
  name = "PING";
}

bool IRCClientPingCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  ircInfo;
  ircInfo.command = eCMD_PONG;
  ircInfo.raw = info.raw;
  return client.sendIRCCommand(eCMD_PONG,ircInfo);
}

bool IRCClientPingCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine = info.target;
  if(info.params.size())
  {
	  commandLine += " " + info.params[0];
  }
  // PING
  return client.sendIRCCommandToServer(eCMD_PING, commandLine);
}

// IRC "PONG" command
IRCClientPongCommand::IRCClientPongCommand()
{
  name = "PONG";
}

bool IRCClientPongCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  // we do nothing on a pong
  return true;
}

bool IRCClientPongCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine = info.raw.substr(5, info.raw.length()-5);

  // PING
  return client.sendIRCCommandToServer(eCMD_PONG,commandLine);
}

// IRC "NOTICE" command
IRCClientNoticeCommand::IRCClientNoticeCommand()
{
  name = "NOTICE";
}

bool IRCClientNoticeCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  trClientMessageEventInfo  messageInfo;

  messageInfo.eventType = eIRCNoticeEvent;
  messageInfo.source = info.source;
  messageInfo.params = info.params;
  messageInfo.message = info.getAsString();
  client.noticeMessage(messageInfo);
  return true;
}

bool IRCClientNoticeCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
	IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

	std::string commandLine = ircInfo.target + delim + std::string(":") + info.getAsString();
	return client.sendIRCCommandToServer(eCMD_NOTICE,commandLine);
}

// IRC "JOIN" command
IRCClientJoinCommand::IRCClientJoinCommand()
{
  name = "JOIN";
}

bool IRCClientJoinCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.joinMessage(info);
  return true;  
}

bool IRCClientJoinCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  std::string commandLine;

  if (info.params.size())
  {
  string_list::iterator itr = info.params.begin();

    while ( itr != info.params.end() )
    {
      commandLine += *itr;
      itr++;

      if (itr != info.params.end())
        commandLine += ",";
    }
  }
  else
    commandLine = info.target;

  // JOIN CHANNEL1,CHANNEL2,....,CHANNELN
  return client.sendIRCCommandToServer(eCMD_JOIN,commandLine);
}

// IRC "PART" command
IRCClientPartCommand::IRCClientPartCommand()
{
  name = "PART";
}

bool IRCClientPartCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.partMessage(info);
  return true;  
}

bool IRCClientPartCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  std::string commandLine = info.target;

  if (info.params.size())
  {
	  commandLine += delim + std::string(":") + info.params[0];
  }

  // PART CHANNEL REASON
  return client.sendIRCCommandToServer(eCMD_PART,commandLine);
}

// IRC "QUIT" command

IRCClientQuitCommand::IRCClientQuitCommand()
{
  name = "QUIT";
}

bool IRCClientQuitCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.quitMessage(info);
  return true;
}

bool IRCClientQuitCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  std::string commandLine;

  if(info.params.size())
  {
	commandLine = std::string(":") + info.params[0];
  }

  // QUIT CHANNEL REASON
  return client.sendIRCCommandToServer(eCMD_QUIT,commandLine);

}

// IRC "MODE" command
IRCClientModeCommand::IRCClientModeCommand()
{
  name = "MODE";
}

bool IRCClientModeCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  // we got a mode message, see what the deal is
  client.modeCommand(info);
  return true;  
}

bool IRCClientModeCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  // MODE TARGET modes
  std::string modeline = info.target;

  if ( info.params.size())
  {
    modeline += delim + info.params[0];

    if ( info.params.size() > 1)
      modeline += delim + info.params[1];
  }
  return client.sendIRCCommandToServer(eCMD_MODE, modeline);
}

// IRC "PRIVMSG" command
IRCClientPrivMsgCommand::IRCClientPrivMsgCommand()
{
  name = "PRIVMSG";
}

bool IRCClientPrivMsgCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.privMessage(info);
  return true;
}

bool IRCClientPrivMsgCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine;
  ircInfo.target = 
  //username host server fullname
  commandLine = ircInfo.target + delim + std::string(":") + info.getAsString();
  return client.sendIRCCommandToServer(eCMD_PRIVMSG,commandLine);
}

// IRC "KICK" command

IRCClientKickCommand::IRCClientKickCommand()
{
  name = "KICK";
}

bool IRCClientKickCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  client.kickCommand(info);
  return true;
}

bool IRCClientKickCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

  std::string commandLine;

  // KICK target user :reason
  commandLine = ircInfo.target + delim + ircInfo.params[0]+ delim + std::string(":") + info.getAsString(1);
  return client.sendIRCCommandToServer(eCMD_KICK,commandLine);
}


IRCClientInviteCommand::IRCClientInviteCommand()
{
	name = "INVITE";
}

bool IRCClientInviteCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
	client.inviteCommand(info);
	return true;
}

bool IRCClientInviteCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
	IRCCommandInfo  &ircInfo = (IRCCommandInfo&)info;

	std::string commandLine;

	// KICK target user :reason
	commandLine = ircInfo.target + delim + ircInfo.params[0]+ delim + std::string(":") + info.getAsString(1);
	return client.sendIRCCommandToServer(eCMD_INVITE,commandLine);
}

  // special case commands

// Generic handler for ALL
IRCClientALLCommand::IRCClientALLCommand()
{
  name = "ALL";
}

bool IRCClientALLCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  // just log it out
  client.log(string_util::format("ALL::command %s from %s for %s containing %s",info.command.c_str(),info.source.c_str(),info.target.c_str(),info.getAsString().c_str()),4);
  client.log(string_util::format("ALL::raw %s",info.raw.c_str()),6);
  client.log(std::string(" "),6);
  return true;
}

bool IRCClientALLCommand::send ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  // just log it out
  client.log(string_util::format("ALL::command %s: to server containing %s",command.c_str(),info.getAsString().c_str()),4);
  return true;
}

// numerics

IRCClientNumericCommand::IRCClientNumericCommand()
{
  name = "NUMERIC";
}

bool IRCClientNumericCommand::receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
{
  int numeric = atoi(info.command.c_str());

  switch(numeric)
  {
    case 0:
      client.log(string_util::format("NUMERIC::Unknown code: %s",info.command.c_str()),2);
    break;

    case RPL_WELCOME:// "Welcome to the Internet Relay Network <nick>!<user>@<host>"
      {
        trClientMessageEventInfo  messageInfo;

        messageInfo.eventType = eIRCNoticeEvent;
        messageInfo.target = info.target;
        messageInfo.source = info.source;
        messageInfo.params = info.params;
        messageInfo.message = info.getAsString();

        client.welcomeMessage(messageInfo);
      }
      break;

    case RPL_YOURHOST: //"Your host is <servername>, running version <ver>"
      client.setServerHost(info.params[3]);
      break;

    case RPL_CREATED: //"This server was created <date>"
    case RPL_MYINFO: //"<servername> <version> <available user modes> <available channel modes>"
    case RPL_ISUPPORT: //"<feature>[,<feature] :are supported by this server"
    case RPL_TRACELINK: //"Link <version & debug level> <destination> <next server> V<protocol version> <link uptime in seconds> <backstream sendq> <upstream sendq>"
    case RPL_TRACECONNECTING: //"Try. <class> <server>"
    case RPL_TRACEHANDSHAKE: //"H.S. <class> <server>"
    case RPL_TRACEUNKNOWN:
    case RPL_TRACEOPERATOR:
    case RPL_TRACEUSER:
    case RPL_TRACESERVER:
    case RPL_TRACESERVICE:
    case RPL_TRACENEWTYPE:
    case RPL_TRACECLASS:
    case RPL_TRACERECONNECT:
    case RPL_STATSLINKINFO:
    case RPL_STATSCOMMANDS:
    case RPL_ENDOFSTATS:
    case RPL_UMODEIS:
    case RPL_SERVLIST:
    case RPL_SERVLISTEND:
    case RPL_STATSUPTIME:
    case RPL_STATSOLINE:
    case RPL_LUSERCLIENT:
    case RPL_LUSEROP:
    case RPL_LUSERUNKNOWN:
    case RPL_LUSERCHANNELS:
    case RPL_LUSERME:
    case RPL_ADMINME:
    case RPL_ADMINLOC1:
    case RPL_ADMINLOC2:
    case RPL_ADMINEMAIL:
    case RPL_TRACELOG:
    case RPL_TRACEEND:
    case RPL_TRYAGAIN:
    case RPL_AWAY:
    case RPL_USERHOST:
    case RPL_ISON:
    case RPL_UNAWAY:
    case RPL_NOWAWAY:
    case RPL_WHOISUSER:
    case RPL_WHOISSERVER:
    case RPL_WHOISOPERATOR:
    case RPL_WHOWASUSER:
    case RPL_ENDOFWHO:
    case RPL_WHOISIDLE:
    case RPL_ENDOFWHOIS:
    case RPL_WHOISCHANNELS:
    case RPL_LISTSTART:
    case RPL_LIST:
    case RPL_LISTEND:
      break;

    case RPL_CHANNELMODEIS:
      {  
        // first string is the channel this is for
        std::string channel = info.params[0];

        // let the big guy know we got some
        client.setChannelMode(channel,info.params[1]);
      }
      break;
    case RPL_UNIQOPIS:
    case RPL_NOTOPIC:
      break;

    case RPL_TOPIC:
      {  
        // first string is the channel this is for
        std::string channel = info.params[0];

        // get that topic
        std::string topic = info.getAsString(1);
        // the first thing is a : we don't want that ether
        topic.erase(topic.begin());

        // let the big guy know we got some
        client.setChannelTopicMessage(channel,topic,info.source);
      }
      break;

    case RPL_INVITING:
    case RPL_SUMMONING:
    case RPL_INVITELIST:
    case RPL_ENDOFINVITELIST:
    case RPL_EXCEPTLIST:
    case RPL_ENDOFEXCEPTLIST:
    case RPL_VERSION:
    case RPL_WHOREPLY:
      break;

    case RPL_NAMREPLY:
      {  
        // first string is the channel this is for
        std::string channel = info.params[1];

        string_list userList = info.params;
        // we don't need the = channel, we saved it
        userList.erase(userList.begin(),userList.begin()+2);

        // the first thing is a : we don't want that ether
        userList[0].erase(userList[0].begin());

        // let the big guy know we got some
        client.addChannelUsers(channel,userList);
      }
      break;
    case RPL_LINKS:
    case RPL_ENDOFLINKS:
      break;

    case RPL_ENDOFNAMES:
      client.endChannelUsersList(info.params[0]);
      break;

    case RPL_BANLIST:
    case RPL_ENDOFBANLIST:
    case RPL_ENDOFWHOWAS:
    case RPL_INFO:
      break;

    case RPL_MOTD:
        client.addMOTD(info.getAsString());
      break;

    case RPL_ENDOFINFO:
      //client.endMOTD();
      break;

    case RPL_MOTDSTART:
      client.beginMOTD();
      break;
    
    case RPL_ENDOFMOTD:
      client.endMOTD();
      break;

    case RPL_YOUREOPER:
    case RPL_REHASHING:
    case RPL_YOURESERVICE:
    case RPL_TIME:
    case RPL_USERSSTART:
    case RPL_USERS:
    case RPL_ENDOFUSERS:
    case RPL_NOUSERS:
    case ERR_NOSUCHNICK:
    case ERR_NOSUCHSERVER:
    case ERR_NOSUCHCHANNEL:
    case ERR_CANNOTSENDTOCHAN:
    case ERR_TOOMANYCHANNELS:
    case ERR_WASNOSUCHNICK:
    case ERR_TOOMANYTARGETS:
    case ERR_NOSUCHSERVICE:
    case ERR_NOORIGIN:
    case ERR_NORECIPIENT:
    case ERR_NOTEXTTOSEND:
    case ERR_NOTOPLEVEL:
    case ERR_WILDTOPLEVEL:
    case ERR_BADMASK:
    case ERR_UNKNOWNCOMMAND:
    case ERR_NOMOTD:
    case ERR_NOADMININFO:
    case ERR_FILEERROR:
      break;

    case ERR_NONICKNAMEGIVEN:
    case ERR_ERRONEUSNICKNAME:
    case ERR_NICKNAMEINUSE:
    case ERR_NICKCOLLISION:
      client.nickNameError(numeric,info.getAsString());
      break;

    case ERR_UNAVAILRESOURCE:
    case ERR_USERNOTINCHANNEL:
    case ERR_NOTONCHANNEL:
    case ERR_USERONCHANNEL:
    case ERR_NOLOGIN:
    case ERR_SUMMONDISABLED:
    case ERR_USERSDISABLED:
    case ERR_NOTREGISTERED:
    case ERR_NEEDMOREPARAMS:
    case ERR_ALREADYREGISTRED:
    case ERR_NOPERMFORHOST:
    case ERR_PASSWDMISMATCH:
    case ERR_YOUREBANNEDCREEP:
    case ERR_YOUWILLBEBANNED:
    case ERR_KEYSET:
    case ERR_CHANNELISFULL:
    case ERR_UNKNOWNMODE:
    case ERR_INVITEONLYCHAN:
    case ERR_BANNEDFROMCHAN:
    case ERR_BADCHANNELKEY:
    case ERR_BADCHANMASK:
    case ERR_NOCHANMODES:
    case ERR_BANLISTFULL:
    case ERR_NOPRIVILEGES:
    case ERR_CHANOPRIVSNEEDED:
    case ERR_CANTKILLSERVER:
    case ERR_RESTRICTED:
    case ERR_UNIQOPPRIVSNEEDED:
    case ERR_NOOPERHOST:
    case ERR_UMODEUNKNOWNFLAG:
    case ERR_USERSDONTMATCH:
    default:
      client.log(string_util::format("NUMERIC::code: %d",numeric),4);
      break;

  }
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8


