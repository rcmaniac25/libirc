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

// hihger level IRC events

#ifndef __IRC_EVENTS_H__
#define __IRC_EVENTS_H__

#include <vector>
#include <map>
#include "IRCTextUtils.h"

class IRCClient;
class IRCServer;
class IRCServerConnectedClient;

typedef enum
{
  eIRCNULLEvent = 0,
  eIRCNoticeEvent,
  eIRCNickNameError,
  eIRCNickNameChange,
  eIRCWelcomeEvent,
  eIRCEndMOTDEvent,
  eIRCConnectedEvent,
  eIRCChannelJoinEvent,
  eIRCChannelPartEvent,
  eIRCChannelBanEvent,
  eIRCChannelMessageEvent,
  eIRCPrivateMessageEvent,
  eIRCTopicEvent,
  eIRCUserJoinEvent,
  eIRCUserPartEvent,
  eIRCUserKickedEvent,
  eIRCTopicChangeEvent,
  eIRCChanInfoCompleteEvent,
  eIRCChannelModeSet,
  eIRCChannelUserModeSet,
  eIRCUserModeSet,
  eIRCQuitEvent,
  eIRCInviteEvent,
  eIRCLastEvent
}teIRCEventType;

/**
 * basic structure that all events are based on events with no data
 * use this.
 */
typedef struct trBaseEventInfo
{
  teIRCEventType  eventType;
}trBaseEventInfo;

/**
 * basic structure that all server events are based on events with no
 * data use this.
 */
typedef struct trBaseServerEventInfo : public trBaseEventInfo
{
  trBaseServerEventInfo(IRCServerConnectedClient*c)
  {
    client = c;
  }
  IRCServerConnectedClient	*client;
}trBaseServerEventInfo;

/**
 * nickname error type events, used for eIRCNickNameError
 */
typedef struct trClientNickErrorEventInfo : public trBaseEventInfo
{
  int  error;
  std::string message;
}trClientNickErrorEventInfo;

/**
 * join type events, used for eIRCChannelJoinEvent, eIRCUserJoinEvent, eIRCInviteEvent
 */
typedef struct trClientJoinEventInfo : public trBaseEventInfo
{
  std::string channel;
  std::string user;
}trClientJoinEventInfo;

/**
 * mode type evetns, used for
 * eIRCUserModeSet,eIRCChannelModeSet,eIRCChannelUserModeSet
 */
typedef struct trClientModeEventInfo : public trBaseEventInfo
{
  std::string target;
  std::string from;
  std::string mode;
  std::string message;
}trClientModeEventInfo;

/**
 * nick change type events, used for  eIRCNickNameChange
 */
typedef struct trClientNickChangeEventInfo : public trBaseEventInfo
{
  std::string oldname;
  std::string newName;
}trClientNickChangeEventInfo;

/**
 * part type events, used for eIRCChannelPartEvent, eIRCUserPartEvent,
 * eIRCQuitEvetnt
 */
typedef struct trClientPartEventInfo : public trBaseEventInfo
{
  std::string channel;
  std::string user;
  std::string reason;
}trClientPartEventInfo;

/**
 * kick and ban type events, used for
 * eIRCChannelKickEvent,eIRCChannelBanEvent, eIRCUserPartEvent
 */
typedef struct trClientKickBanEventInfo : public trBaseEventInfo
{
  std::string channel;
  std::string user;
  std::string reason;
  std::string kicker;
}trClientKickBanEventInfo;

/**
 * message events, used for eIRCChannelMessageEvent,
 * eIRCPrivateMessageEvent, eIRCNoticeEvent, eIRCWelcomeEvent,
 * eIRCTopicChangeEvent
 */
typedef struct trClientMessageEventInfo : public trBaseEventInfo
{
  std::string  target;
  std::string source;
  std::string from;
  std::string message;
  std::vector<std::string> params;

  std::string getAsString ( int start = 0, int end = -1 ) {return string_util::getStringFromList(params," ",start,end);}
}trClientMessageEventInfo;

class IRCClientEventCallback
{
public:
  virtual ~IRCClientEventCallback(){return;}
  virtual bool process ( IRCClient &ircClient, teIRCEventType  eventType, trBaseEventInfo &info ) = 0;
};

class IRCServerEventCallback
{
public:
  virtual ~IRCServerEventCallback(){return;}
  virtual bool process ( IRCServer *ircServer, teIRCEventType  eventType, trBaseServerEventInfo &info ) = 0;
};

typedef std::vector<IRCClientEventCallback*>  tvIRCClientEventList;
typedef std::map<teIRCEventType,tvIRCClientEventList> tmIRCClientEventListMap;
typedef std::map<teIRCEventType,IRCClientEventCallback*> tmIRCClientEventMap;

typedef std::vector<IRCServerEventCallback*>  tvIRCServerEventList;
typedef std::map<teIRCEventType,tvIRCServerEventList> tmIRCServerEventListMap;
typedef std::map<teIRCEventType,IRCServerEventCallback*> tmIRCServerEventMap;

#endif // __IRC_EVENTS_H__ 

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
