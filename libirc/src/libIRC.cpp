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

// implementation of main libIRC classes

//********************************************************************************//
//															version info																			//
//********************************************************************************//

#define _MAJOR_VERS	0
#define _MINOR_VERS	0
#define _REVISION		2

//********************************************************************************//

#include "libIRC.h"

#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
	#include <time.h>
	#include <stdio.h>
#endif

// sleep util
void IRCOSSleep ( float fTime )
{
#ifdef _WIN32
	Sleep((DWORD)(1000.0f * fTime));
#else
	usleep((unsigned int )(100000 * fTime));
#endif
}


std::string getTimeStamp ( void )
{
	std::string timeString;

#ifdef _WIN32
	struct tm *newtime;
	time_t aclock;

	time( &aclock );   // Get time in seconds
	newtime = localtime( &aclock );   // Convert time to struct tm form 

	/* Print local time as a string */
	timeString = asctime( newtime );
#endif//_WIN32

	return timeString;
}


std::string getLibVersion ( void )
{
	return string_util::format("libIRC %d.%d.%d",_MAJOR_VERS, _MINOR_VERS, _REVISION);
}

void getLibVersion ( int &major, int &minor, int &rev )
{
	major = _MAJOR_VERS;
	minor = _MINOR_VERS;
	rev = _REVISION;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
