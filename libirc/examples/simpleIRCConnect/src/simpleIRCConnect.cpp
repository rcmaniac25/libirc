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


void main ( void )
{
	IRCClient	client;

	client.setDebugLevel(5);
	client.setLogfile("irc.log");

	client.connect("irc.freenode.net",6667);
	client.login(std::string("libIRCTest2"),std::string("libIRCTest2"),std::string("libIRCTest2"));

	client.sendRaw(std::string("JOIN #bzflag,#brlcad"));
	while (client.process())
	{
		IRCOSSleep(1);
	}
};