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

#ifndef _LIBIRC_BOTLIB_H_
#define _LIBIRC_BOTLIB_H_

#include "libIRC.h"
#include "IRCTextUtils.h"

class LibIRCBotConfigDataValue
{
public:
  LibIRCBotConfigDataValue(const char* data = NULL );
  LibIRCBotConfigDataValue ( const std::string &data );

  std::vector<std::string> params ( void );
  std::string param ( unsigned int index );

  std::string value;
};

typedef std::vector<LibIRCBotConfigDataValue> LibIRCBotConfigDataValueList;

class LibIRCBotConfigItem
{
public:
  void set ( const char *data );

  bool write ( std::string &config );

  std::string key;
  LibIRCBotConfigDataValueList values;
};

typedef std::map<std::string,LibIRCBotConfigItem>  LibIRCBotConfigItemMap;

class LibIRCBotConfig
{
public:
  LibIRCBotConfig( const char* file = NULL );
  ~LibIRCBotConfig();

  bool read ( const char * file );
  bool write ( const char * file );

  LibIRCBotConfigDataValueList& getKeyItems( const char* key );
  LibIRCBotConfigDataValueList& getKeyItems( const std::string &key );

  void addItem ( const char* key, const char* value );
  void addItem ( const std::string &key, const std::string &value );

protected:
  LibIRCBotConfigItemMap  items;
};

typedef struct  
{
  std::string server;
  int port;

  std::string hostmask;
  std::string username;

  std::vector<std::string> nicks;
  std::vector<std::string> channels;

  std::string serverPassword;
  std::string nickservPassword;
}trLibIRCConnectionRecord;

class LibIRCBot
{
public:
  LibIRCBot();
  virtual ~LibIRCBot();

  // called before connection to set the connection info for the bot
  virtual void init ( trLibIRCConnectionRecord &conRec ) = 0;

  // called before run and the main update loop
  // when using the built in main
  // return true if you want to quit out after
  // parsing the command line options
  virtual bool prerun ( int argc, char *argv[] ) { return false; }

  // called to start the server run the vents
  // will be blocking untill quit is called.
  // will sleep for the sleepTime each loop
  int run ( void );

  // called once for each update loop
  // called internaly buy run, but can be called
  // manualy if not using run and you want to
  // mix the bot into your own code
  bool runOneLoop ( void );

protected:
  float	    sleepTime;

public:
  trLibIRCConnectionRecord connectionRecord;

  bool connect ( void );
};

#ifndef _LIBIRC_NO_BOTMAIN
extern LibIRCBot *theBot;
#endif

#ifdef _WIN32
#ifndef _NO_WINMAIN
#define _USE_WINMAIN
#endif
#endif

#ifdef _USE_WINMAIN
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  std::vector<std::string> cliParams = string_util::tokenize(std::string(lpCmdLine),std::string(" "));
  int argc = (int)cliParams.size();
  char** argv = (char**)malloc(sizeof(char*) * argc);
  for ( int i = 0; i < argc; i++ )
    argv[i] = (char*)cliParams[i].c_str();
#else
int main ( int argc, char *argv[] )
{
#endif
  LibIRCBot *bot = theBot;
  if (!bot || bot->prerun(argc,argv))
    return -1;
  bot->run();

#ifdef _USE_WINMAIN
  free(argv);
#endif
  return 0;
}

#endif //_LIBIRC_BOTLIB_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
