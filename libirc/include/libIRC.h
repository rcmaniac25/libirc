/* libIRC
* Copyright (c) 2004 Sean Morrison
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// main libIRC header

#ifndef _LIBIRC_H_
#define _LIBIRC_H_

class IRCClient
{
public:
	IRCClient();
	~IRCClient();

	// general connection stuff
	bool init ( void );
	bool connect ( const char* server, int port );
	bool login ( const char* nick,  const char* username = NULL, const char *fullname = NULL );
	bool disconnect ( void );
};

#endif //_LIBIRC_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8