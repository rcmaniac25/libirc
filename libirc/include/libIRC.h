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

// main libIRC header

#ifndef _LIBIRC_H_
#define _LIBIRC_H_

// IRC includes
#include "ircCommands.h"
#include "IRCEvents.h"
#include "TCPConnection.h"
#include "IRCUserManager.h"

#include "IRCClient.h"
#include "IRCServer.h"

// global includes
#include <string>

// simple OS indpendent sleep function
// used by so many things, so we have one here
void IRCOSSleep ( float fTime );
std::string getTimeStamp ( void );

std::string getLibVersion ( void );
void getLibVersion ( int &major, int &minor, int &rev );

#endif //_LIBIRC_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
