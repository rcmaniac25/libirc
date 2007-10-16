/* 
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

// defines the main implemenation of the engine application

#include <stdio.h>
#include "common.h"
#include "commandArgs.h"
#include "OSFile.h"
#include "agents.h"
#include "TextUtils.h"

AgentMap	      agents;
CCommandLineArgs      args;

COSDir		      configDir;

bool		      done = false;
float		      sleepTime = 0.5f;

void log ( const std::string & text )
{
  printf("%s\n",text.c_str());
}

void setConfigDir ( COSDir &dir )
{
  dir.SetStdDir("./users");
  if ( args.Exists("userDir"))
    dir.SetOSDir(args.GetDataS("userDir"));
}

void processArgs ( void )
{
  setConfigDir(configDir);
  if ( args.Exists("sleepTime"))
    sleepTime = args.GetDataF("sleepTime");
}

void buildAgentsFromDir ( void )
{
  COSDir	agentDir;

  while (configDir.GetNextDir(agentDir,false))
  {
    Agent	*agent = new Agent;
    if (agent->loadFromDir(agentDir.GetOSName()))
    {
      if ( agents.find(agent->getName()) != agents.end())
      {
	log (TextUtils::format("Duplicate agent %s found in %s: ignoring",agent->getName().c_str(),agentDir.GetOSName()));
	delete (agent);
      }
      else
      {
	agents[agent->getName()] = agent;
	agent->init();
      }
    }
    else
      delete (agent);
  }
}

void listenForClients ( void )
{
}

int main(int argc, char* argv[])
{
  args.Set(argc,argv);
  processArgs();

  // start the agents loading
  buildAgentsFromDir();

  // start listenign for clients
  listenForClients();

  // run our happy little loop	
  while (!done)
  {
    AgentMap::iterator itr = agents.begin();
    while ( itr != agents.end() )
    {
      // have every agent update itself, if it's dead for some reason
      // and never wants to come back, delete it from the list
      // if it was to be destroied, it would have killed itself from the
      // master reload list
      // if not, it'll be readded when the client conencts
      // and starts it up again.
      if (!itr->second->update())
      {
	delete(itr->second);
	agents.erase(itr++);
      }
      else
	itr++;
    }
    OSSleep(sleepTime);
  }
  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
