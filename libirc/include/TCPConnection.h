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

class TCPConnection;

typedef enum
{
	eTCPNoError = 0,
	eTCPNotInit,
	eTCPTimeout,
	eTCPBadAddress,
	eTCPBadPort,
	eTCBSocketNFG,
	eTCPInitFailed,
	eTCPUnknownError
}teTCPError;

// Packet data class
// this class is realy just to hold data, and make sure it get deallocated.
class TCPPacket
{
public:
	TCPPacket();
	TCPPacket( const TCPPacket &p );
	~TCPPacket();

	TCPPacket& operator = ( const TCPPacket &p );

	void set ( unsigned char* buf, unsigned int len );
	unsigned char* get ( unsigned int &len );

protected:
	unsigned char	*data;
	unsigned int	size;
};

// TCP/IP client connection to some host on some port
class TCPClientConnection
{
public:
	TCPClientConnection();
	TCPClientConnection( std::string server, unsigned short port );
	~TCPClientConnection();


protected:
	// who's your daddy
	friend	class TCPConnection;
	TCPConnection			*parent;

	struct TCPClientConnectionInfo;
	TCPClientConnectionInfo	*info;
};

// TCP/IP server connection listening for some connections on some port
class TCPServerConnection
{
public:
	TCPServerConnection();
	TCPServerConnection( unsigned short port, unsigned int connections );
	~TCPServerConnection();

protected:
	// who's your daddy
	friend	class TCPConnection;
	TCPConnection			*parent;

	struct TCPServerConnectionInfo;
	TCPServerConnectionInfo	*info;
};

// master TCP/IP connection manager
// takes care of loading and clearing out the TCP stack if it needs it
// gives out client and server connections
// handles the non blocking updates of connections.
class TCPConnection
{
public:
	TCPConnection();
	~TCPConnection();

	// initalises the socket system, on windows inits WSA
	// called automaticly by the constructor, but exposted
	// in case the client needs to reinit the socket system
	teTCPError init ( void );

	// kills the socket system, closes all connections,
	// and on windows cleans up the WSA system
	void kill ( void );

	// has all connections check for new data
	teTCPError update ( void );

	// returns a new client conenction to the specified host and port
	TCPClientConnection* newClientConnection ( std::string server, unsigned short port );

	// returns a new server connection that will listen for the 
	// specified number of connections on the specfied port
	TCPServerConnection* newServerConnection ( unsigned short port, int connections );

	// disconnects and removes connections
	void deleteClientConnection ( TCPClientConnection* connection );
	void deleteServerConnection ( TCPServerConnection* connection );

	// returns the lost host
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
