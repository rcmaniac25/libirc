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

// simple IRC connection sample

#include <stdio.h>
#include <stdlib.h>

#include "libIRC.h"

IRCClient	client;

class myEndMOTDCallback : public IRCClientEventCallback
{
public:
	bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );
};

class myAllCallback : public IRCClientCommandHandler
{
public:
	myAllCallback()
	{
		name = "ALL";
	}
	virtual bool receive ( IRCClient &client, const std::string &command, BaseIRCCommandInfo  &info )
	{
		std::string line = info.raw;
		FILE *fp = fopen("botlog.log","at");
		if (fp)
		{
			fprintf(fp,"%s\n",info.raw.c_str());
			fclose(fp);
		}
		return false;
	}
};


bool myEndMOTDCallback::process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info )
{
	printf("********************** starting up ************************\n");
	ircClient.join("#libirc");
	return true;
}

myEndMOTDCallback	startupCallback;
myAllCallback		allCallback;

int main ( int argc, char *argv[] )
{
	client.setDebugLevel(5);

	// clear the log
	fclose(fopen("irc.log","wt"));
	fclose(fopen("botlog.log","wt"));

	// set the log
	client.setLogfile("irc.log");
	client.registerEventHandler(eIRCNoticeEvent,&startupCallback);
	client.registerCommandHandler(&allCallback);

	client.connect("irc.efnet.net",6667);
	std::string name = std::string("billybot");
	std::string username = std::string("billy");
	std::string fullname = std::string("William Shatner");
	std::string meh = std::string("");
	client.login(name, username, fullname, meh);

	while (client.process())
	{
		IRCOSSleep(1);
	}

	return 0;
}
