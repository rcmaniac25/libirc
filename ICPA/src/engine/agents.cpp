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

#include "agents.h"
#include "OSFile.h"
#include "TextUtils.h"

//--------------------IRCServerChannel------------------------------
ServerChannel::~ServerChannel()
{
}

//--------------------IRCServerPrivateMessage------------------------------
ServerPrivateMessage::~ServerPrivateMessage()
{
}

//--------------------AgentConnectedServer------------------------------
AgentConnectedServer::AgentConnectedServer()
{

}

AgentConnectedServer::~AgentConnectedServer()
{

}

bool AgentConnectedServer::connected ( void )
{
	return false;
}


//--------------------Agent------------------------------

Agent::Agent()
{
}

Agent::Agent(const char* workingDir)
{
}

Agent::~Agent()
{
}

bool Agent::valid ( void )
{
	return false;
}

bool Agent::init ( void )
{
	return valid();
}

bool Agent::connected ( void )
{
	return valid();
}

bool Agent::update ( void )
{
	return valid();
}

bool Agent::loadFromDir ( const char* dir )
{
	if (dirName.size() || !dir)
		return false;

	COSFile	file;
	file.OSName(dir);
	std::string fileName = file.GetStdName();
	fileName += "/config.cfg";
	file.StdName(fileName.c_str());

	if (!file.Open("rt"))
		return false;

	agentName = "";

	std::string configText;
	if(!file.GetFileText(configText))
		return false;

	file.Close();

	configText = TextUtils::replace_all(configText,std::string("\r"),std::string(""));
	std::vector<std::string> lines = TextUtils::tokenize(configText,std::string("\n"));

	for ( int i = 0; i < (int)lines.size(); i++ )
	{
		const std::string &line = lines[i];
		std::vector<std::string> chunks = TextUtils::tokenize(line,std::string(" "),0,true);
		if ( chunks.size() > 1 )
		{
			std::string command = TextUtils::tolower(chunks[0]);
			if ( command == "name" )
				agentName = chunks[1];
		}
	}

	return agentName.size() > 0;
}

bool Agent::saveToDir ( void )
{
	if (!dirName.size())
		return false;

	return false;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

