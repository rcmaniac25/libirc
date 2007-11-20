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
  LibIRCBotConfigItem ( const char *_key = NULL, const char* _data = NULL );
  LibIRCBotConfigItem ( const std::string &_key, const std::string &_data );
  LibIRCBotConfigItem ( const std::string &_key );

  void set ( const char *data );
  void set ( const std::string &data );

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
  bool read ( const std::string &file );
  bool write ( const char * file );
  bool write ( const std::string &file );

  const LibIRCBotConfigDataValueList& getKeyItems( const char* key );
  const LibIRCBotConfigDataValueList& getKeyItems( const std::string &key );

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
  std::string realName;

  std::vector<std::string> nicks;
  std::vector<std::string> channels;

  std::string serverPassword;
  std::string nickservPassword;
}trLibIRCConnectionRecord;

class LibIRCBotMessage
{
public:
  std::string message;

  std::string from;
  std::string respond;
  std::string taragetUser;

  std::vector<std::string> params ( void );
  std::string param ( unsigned int index );
};

class LibIRCBot : public IRCClientEventCallback
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
  // will be blocking until quit is called.
  // will sleep for the sleepTime each loop
  int run ( void );

  // called once for each update loop
  // called internally buy run, but can be called
  // manually if not using run and you want to
  // mix the bot into your own code
  bool runOneLoop ( void );

  // IRC event callback
  // if you override this, you must call the default
  // ether before or after you do your own stuff
  // if you simply wish to register other events
  // then override the unhandledEvent method instead
  bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );

  // IRC Event callback for events that the base lib
  // does not handle by default
  // if you register any event handlers with the client
  // they will come through here.
  bool unhandledEvent ( trBaseEventInfo &eventInfo ){return true;}

  // common events
  // these are called on the major events.
  virtual void onLogn ( void ){};
  virtual void onChannelJoin ( const std::string &channel ){};
  virtual void onUserJoin ( const std::string &channel, const std::string &user ){};
  virtual void onUserPart ( const std::string &channel, const std::string &user, const std::string &reason ){};
  virtual void onPrivateMessage ( const LibIRCBotMessage &message ){};
  virtual void onChannelMessage ( const LibIRCBotMessage &message, bool isForMe ){};
  virtual void onNickNameError ( std::string &newNick ){};

protected:
  float		sleepTime;
  IRCClient	client;
  std::vector<std::string> nickShortcuts;

  void disconectFromServer ( const char* reason );
  void disconectFromServer ( const std::string &reason );

  void respond ( const LibIRCBotMessage &to, const char* text, bool action = false, bool privately = false );
  void respond ( const LibIRCBotMessage &to, const std::string &text, bool action = false,bool privately = false );
 
  void send ( const char* to, const char* text, bool action = false );
  void send ( const std::string to, const char* text, bool action = false );
  void send ( const char* to, const std::string &text, bool action = false );
  void send ( const std::string &to, const std::string &text, bool action = false );

  virtual bool isForMe ( const std::string &message );

private:
  trLibIRCConnectionRecord connectionRecord;
  bool			   commandsRegistered;
  bool			   disconnect;
  int			   pendingJoins;
  int			   currentNicIndex;

  bool verifyConRec ( void );
  void registerStandardEventHandlers ( void );
  bool connect ( void );
  void processPendingActions ( void );

  // event actions
  void serverLogin ( void );
  void welcomeMessage ( void );
  void channelJoin ( trClientJoinEventInfo* info );
  void userJoin ( trClientJoinEventInfo* info );
  void userPart ( trClientPartEventInfo* info );
  void chatMessage ( trClientMessageEventInfo* info, bool inChannel );
  bool nickError ( void );

  // utilities
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
