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

// hihger level IRC events

#ifndef __IRC_EVENTS_H__
#define __IRC_EVENTS_H__

#include <vector>
#include <map>

class IRCClient;

typedef enum
{
	eIRCNULLEvent = 0,
	eIRCNoticeEvent,
	eIRCNickNameError,
	eIRCWelcomeEvent,
	eIRCEndMOTDEvent,
	eIRCChannelJoinEvent,
	eIRCChannelPartEvent,
	eIRCChannelKickEvent,
	eIRCChannelBanEvent,
	eIRCChannelMessageEvent,
	eIRCPrivateMessageEvent,
	eIRCTopicEvent,
	eIRCUserJoinEvent,
	eIRCUserPartEvent,
	eIRCUserKickedEvent,
	eIRCTopicChangeEvent,
	eIRCChanInfoCompleteEvent,
	eIRCLastEvent
}teIRCEventType;

// basic structiure that all events are based on
// events with no data use this
typedef struct trBaseEventInfo
{
	teIRCEventType	eventType;
}trBaseEventInfo;

// nickname error type events, used for eIRCNickNameError
typedef struct trNickErrorEventInfo : public trBaseEventInfo
{
	int	error;
	std::string message;
}trNickErrorEventInfo;

// join type evetns, used for  eIRCChannelJoinEvent, eIRCUserJoinEvent
typedef struct trJoinEventInfo : public trBaseEventInfo
{
	std::string channel;
	std::string user;
}trJoinEventInfo;

// part type evetns, used for eIRCChannelPartEvent, eIRCUserPartEvent
typedef struct trPartEventInfo : public trBaseEventInfo
{
	std::string channel;
	std::string user;
	std::string reason;
}trPartEventInfo;

// kick and ban type events , used for eIRCChannelKickEvent,eIRCChannelBanEvent, eIRCUserPartEvent
typedef struct trKickBanEventInfo : public trBaseEventInfo
{
	std::string channel;
	std::string user;
	std::string reason;
	std::string kicker;
}trKickBanEventInfo;

// message events, used for eIRCChannelMessageEvent, eIRCPrivateMessageEvent, eIRCNoticeEvent, eIRCWelcomeEvent, eIRCTopicChangeEvent
typedef struct trMessageEventInfo : public trBaseEventInfo
{
	std::string	target;
	std::string source;
	std::string from;
	std::string message;
	std::vector<std::string> params;
}trMessageEventInfo;

class IRCBasicEventCallback
{
public:
	virtual ~IRCBasicEventCallback(){return;}
	virtual bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info ) = 0;
};

typedef std::vector<IRCBasicEventCallback*>	tvIRCEventList;
typedef std::map<teIRCEventType,tvIRCEventList> tmIRCEventListMap;
typedef std::map<teIRCEventType,IRCBasicEventCallback*> tmIRCEventMap;

#endif // __IRC_EVENTS_H__ 
