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

// TCP/IP connection classes

#include "TCPConnection.h"
#include "SDL_net.h"

//---------------------------------------------------------------------------------------------------//
// TCP/IP packet class

TCPPacket::TCPPacket()
{
	data = NULL;
	size = 0;
}

TCPPacket::TCPPacket( const TCPPacket &p )
{
	data = (unsigned char*)malloc(p.size);
	memcpy(data,p.data,p.size);
	size = p.size;
}

TCPPacket::~TCPPacket()
{
	if (data)
		free(data);
	data = NULL;
	size = 0;
}

TCPPacket& TCPPacket::operator = ( const TCPPacket &p )
{
	if (data)
		free (data);

	data = (unsigned char*)malloc(p.size);
	memcpy(data,p.data,p.size);
	size = p.size;
	return *this;
}

void TCPPacket::set ( unsigned char* buf, unsigned int len )
{
	if (data)
		free (data);

	data = (unsigned char*)malloc(len);
	memcpy(data,buf,len);
	size = len;
}

unsigned char* TCPPacket::get ( unsigned int &len )
{
	len = size;
	return data;
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP connection class for clients

// the info data for the class
struct TCPClientConnection::TCPClientConnectionInfo
{	
	//SOCKET	socket;
};

TCPClientConnection::TCPClientConnection()
{
	info = new TCPClientConnectionInfo;

	// do some TCP/IPish things here
}

TCPClientConnection::TCPClientConnection( std::string server, unsigned short port )
{
	// do some TCP/IPish things here
}

TCPClientConnection::~TCPClientConnection()
{
	// do some TCP/IPish things here
	if (info)
		delete(info);
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP listener class for servers

// the info data for the class
struct TCPServerConnection::TCPServerConnectionInfo
{	
	//SOCKET	socket;
};


TCPServerConnection::TCPServerConnection()
{
	info = new TCPServerConnectionInfo;
	// do some TCP/IPish things here
}

TCPServerConnection::TCPServerConnection( unsigned short port, unsigned int connections )
{
	// do some TCP/IPish things here
}

TCPServerConnection::~TCPServerConnection()
{
	// do some TCP/IPish things here
	if (info)
		delete(info);
}


// master connections class, Make this a singleton? there should only ever be one

struct TCPConnection::TCPConnectionInfo
{	
#ifdef _WIN32
	//WSADATA m_WSAData;
#endif
	bool initedSocketInterface;
};

TCPConnection::TCPConnection()
{
	info = new TCPConnectionInfo;
	info->initedSocketInterface = false;

	init();
}

TCPConnection::~TCPConnection()
{
	kill();
	if (info)
		delete(info);
}

teTCPError TCPConnection::init ( void )
{
	kill();
#ifdef _WIN32
	//if(WSAStartup( MAKEWORD( 2, 2 ), &info->m_WSAData ) ==0)
//		info->initedSocketInterface = true;
	return info->initedSocketInterface ? eTCPNoError : eTCPInitFailed;
#endif
	return eTCPNoError;
}

void TCPConnection::kill ( void )
{
	if (info->initedSocketInterface)
	{
#ifdef _WIN32
//		WSACleanup();
#endif//_WIN32
		info->initedSocketInterface = false;
	}
}

teTCPError TCPConnection::update ( void )
{
	return eTCPNoError;
}

TCPClientConnection* TCPConnection::newClientConnection ( std::string server, unsigned short port )
{
	TCPClientConnection*	connection = new  TCPClientConnection(server,port);
	
	clientConnections.push_back(connection);
	return connection;
}

TCPServerConnection* TCPConnection::newServerConnection ( unsigned short port, int connections )
{
	TCPServerConnection*	connection = new  TCPServerConnection(port,connections);

	serverConnections.push_back(connection);
	return connection;
}

void TCPConnection::deleteClientConnection ( TCPClientConnection* connection )
{
	tvClientConnectionList::iterator itr = clientConnections.begin();
	while( itr!=clientConnections.end() )
	{
		if (*itr == connection)
			itr = clientConnections.erase(itr);
		else
			itr++;
	}
	delete(connection);
}

void TCPConnection::deleteServerConnection ( TCPServerConnection* connection )
{
	tvServerConnectionList::iterator itr = serverConnections.begin();
	while( itr!=serverConnections.end() )
	{
		if (*itr == connection)
			itr = serverConnections.erase(itr);
		else
			itr++;
	}
	delete(connection);
}

std::string TCPConnection::getLocalHost ( void )
{
	return "127.0.0.1";
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
