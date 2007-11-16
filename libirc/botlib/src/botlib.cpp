/* libIRC
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

#include "botlib.h"

#ifndef _LIBIRC_NO_BOTMAIN
LibIRCBot *theBot = NULL;
#endif

LibIRCBot::LibIRCBot()
{
#ifndef _LIBIRC_NO_BOTMAIN
  theBot = this;
#endif

  sleepTime = 1;
}

LibIRCBot::~LibIRCBot()
{
#ifndef _LIBIRC_NO_BOTMAIN
  theBot = NULL;
#endif
}


int LibIRCBot::run ( void )
{
  init(connectionRecord);
  if (!connect())
    return -1;

  while (!runOneLoop())
    IRCOSSleep(sleepTime)
}

bool LibIRCBot::runOneLoop ( void )
{
  return true;
}

bool LibIRCBot::connect ( void )
{
  return true;
}


//----------------LibIRCBotConfigItem
void LibIRCBotConfigItem::set( const char *data )
{
}

bool LibIRCBotConfigItem::write( std::string &config )
{
}

//----------------LibIRCBotConfig
LibIRCBotConfig::LibIRCBotConfig( const char* file )
{
  if (file)
    read(file);
}

LibIRCBotConfig::~LibIRCBotConfig()
{
}

bool LibIRCBotConfig::read ( const char * file )
{
  return false;
}

bool LibIRCBotConfig::write ( const char * file )
{
  return false;
}

LibIRCBotConfigDataValueList& LibIRCBotConfig::getKeyItems( const char* key )
{
  static LibIRCBotConfigDataValueList list;
  return list;
}

LibIRCBotConfigDataValueList& LibIRCBotConfig::getKeyItems( const std::string &key )
{
  static LibIRCBotConfigDataValueList list;
  return list;
}

void LibIRCBotConfig::addItem ( const char* key, const char* value )
{
}

void LibIRCBotConfig::addItem ( const std::string &key, const std::string &value )
{
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
