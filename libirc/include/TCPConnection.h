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

// TCP/IP connection class header

#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include <vector>

class TCPClientConnection
{
public:
	TCPClientConnection();
	TCPClientConnection( std::string server, unsigned short port );
	~TCPClientConnection();

protected:
	struct TCPClientConnectionInfo;
	TCPClientConnectionInfo	*info;
};

class TCPServerConnection
{
public:
	TCPServerConnection();
	TCPServerConnection( unsigned short port, unsigned int connections );
	~TCPServerConnection();

protected:
	struct TCPServerConnectionInfo;
	TCPServerConnectionInfo	*info;
};

class TCPConnection
{
public:
	TCPConnection();
	~TCPConnection();

	bool init ( void );
	void kill ( void );

	TCPClientConnection* newClientConnection ( std::string server, unsigned short port );
	TCPServerConnection* newServerConnection ( unsigned short port, int connections );

	void deleteClientConnection ( TCPClientConnection* connection );
	void deleteServerConnection ( TCPServerConnection* connection );

	std::string getLocalHost ( void );

protected:
	typedef std::vector<TCPClientConnection*>	tvClientConnectionList;
	tvClientConnectionList		clientConnections;

	typedef std::vector<TCPServerConnection*>	tvServerConnectionList;
	tvServerConnectionList		serverConnections;

	struct TCPConnectionInfo;
	TCPConnectionInfo	*info;
};

#endif //_TCP_CONNECTION_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
