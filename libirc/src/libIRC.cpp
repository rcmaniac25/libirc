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

#include "libIRC.h"
#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
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

// IRC class stuff

IRCClient::IRCClient()
tcpConnection(TCPConnection::instance())
{
	init();
	tcpClient = NULL;

	ircMessageTerminator = " \r\n";
	debugLogLevel = 0;
}

IRCClient::~IRCClient()
{
	tcpConnection.kill();
}

// general connection methods
bool IRCClient::init ( void )
{
	// if any old conenctions are around, kill em
	if (tcpClient)
		tcpConnection.deleteClientConnection(tcpClient);

	tcpClient = NULL;

	// make sure the system we have is inited
	tcpConnection.init();

	// just get us a new empty connection
	tcpClient = tcpConnection.newClientConnection("",0);
	
	if (tcpClient)
		tcpClient->addListener(this);

	return tcpClient != NULL;
}

bool IRCClient::connect ( std::string server, int port )
{
	if (!tcpClient)
		return false;

	unsigned short port2Use = 6667;
	if ( port > 0 )
		port2Use = (unsigned short)port;

	return tcpClient->connect(server,port2Use) == eTCPNoError;
}

bool IRCClient::login ( std::string &nick, std::string &username, std::string &fullname)
{
	if (!tcpClient || !tcpClient->connected())
		return false;

	if (!nick.size())
		nick << "SomeLazyUser" << ((int)this);

	if (!username.size())
		username = "libIRCUser";

	if (!fullname.size())
		fullname = "Lazy libIRC programer";

	return false;
}

bool IRCClient::disconnect ( void )
{
	return false;
}

// update loop methods
bool IRCClient::process ( void )
{
	return false;
}

// sending commands
bool IRCClient::send ( std::string command, std::string target, std::string data )
{
	return false;
}

bool IRCClient::send ( std::string &command, BaseIRCCommandInfo &info )
{
	return false;
}

bool IRCClient::sendRaw ( std::string data )
{
	return false;
}

//command handaler methods
bool IRCClient::registerCommandHandaler ( std::string command, IRCClientCommandHandaler &handaler )
{
	return false;
}

int IRCClient::listCommandHandalers ( std::vector<std::string> &commandList )
{
	commandList.clear();
	return 0;
}

void IRCClient::pending ( TCPClientConnection *connection, int count )
{
	// we got some data, do something with it
	log("Data Pending notification",5);
}


// utility methods
bool IRCClient::sendTextToServer ( std::string text )
{
	if (!tcpClient || !tcpClient->connected())
		return false;

	std::string message = text;
	if (text.size())
	{
		message += ircMessageTerminator;
		teTCPError	error = tcpClient->sendData(message);
		if (error == eTCPNoError)
			log("Send Data:" + text,2);
		else
		{
			switch (error)
			{
				case eTCPNotInit:
					log("Send Data Error: TCP Not Initalised: data=" + text,0);
				break;

				case eTCPSocketNFG:
					log("Send Data Error: Bad Socket: data=" + text,0);
				break;

				case eTCPDataNFG:
					log("Send Data Error: Bad Data",0);
				break;

				case eTCPConnectionFailed:
					log("Send Data Error: TCP Connection failed",0);
				break;

				default:
					log("Send Data Error:Unknown Error",0);
			}
			return false;
		}
	}
	else
		return false;

	return true;
}

void IRCClient::log ( std::string &text, int level = 0 )
{
	if (level <= debugLogLevel)
	{
		printf("log# %d:%s\n",level,text.c_str());

		if (logfile.size())
		{
			FILE *fp = fopen(logfile.c_str(),"at");

			if (fp)
			{
				fprintf("log# %d:%s\n",level,text.c_str());
				fclose(fp);
			}
		}
	}
}

void IRCClient::setLogfile ( std::string file )
{
	logfile = file;
}

void IRCClient::setDebugLevel ( int level )
{
	debugLogLevel = level;
}

int IRCClient::getDebugLevel ( void )
{
	return debugLogLevel;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
