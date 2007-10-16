/* ICPA
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

#include "common.h"

#ifndef _WIN32
  #include <unistd.h>
#else
  #include <windows.h>
  #include <time.h>
  #include <stdio.h>
#endif

void OSSleep ( float fTime )
{
#ifdef _WIN32
  Sleep((DWORD)(1000.0f * fTime));
#else
  usleep((unsigned int )(100000 * fTime));
#endif
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
