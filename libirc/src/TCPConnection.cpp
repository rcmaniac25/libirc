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

#define DEFAULT_READ_CHUNK 512

#include <map>
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

TCPPacket::TCPPacket( unsigned char* buf, unsigned int len )
{
	data = NULL;
	size = 0;
	set(buf,len);
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
class TCPClientConnection::TCPClientConnectionInfo
{	
public:
	TCPClientConnectionInfo();
	teTCPError	lastError;
	IPaddress	serverIP;
	TCPsocket	socket;
	int readChunkSize;
};

TCPClientConnection::TCPClientConnectionInfo::TCPClientConnectionInfo()
{
	teTCPError	lastError = eTCPNotInit;;
	serverIP.host = 0;
	serverIP.port = 0;
	socket = NULL;
	readChunkSize = DEFAULT_READ_CHUNK;;
}

TCPClientConnection::TCPClientConnection()
{
	info = new TCPClientConnectionInfo;
	parent = NULL;
	info->lastError = eTCPNotInit;
	info->serverIP.host = 0;
	info->serverIP.port = 0;
	info->socket = NULL;
	info->readChunkSize = 512;
}

TCPClientConnection::TCPClientConnection( std::string server, unsigned short port, TCPConnection *parentConnection )
{
	info = new TCPClientConnectionInfo;
	info->lastError = eTCPNotInit;
	parent = parentConnection;

	if (server.size() && port != 0)
		connect(server,port);
}

TCPClientConnection::~TCPClientConnection()
{
	// do some TCP/IPish things here
	if (info)
		delete(info);
}

teTCPError TCPClientConnection::connect ( std::string server, unsigned short port )
{
	if (!info)
		return setError(eTCPNotInit);

	if ( SDLNet_ResolveHost(&info->serverIP, server.c_str(), port))
		return setError(eTCPUnknownError);

	if ( info->serverIP.host == INADDR_NONE )
		return setError(eTCPBadAddress);

	return connect();	
}

teTCPError TCPClientConnection::connect ( void )
{
	disconnect();

	if ( info->serverIP.host == 0 || info->serverIP.port == 0 || info->serverIP.host == INADDR_NONE )
		return setError(eTCPBadAddress);

	info->socket = SDLNet_TCP_Open(&info->serverIP);
	if ( info->socket == NULL )
		return setError(eTCPConnectionFailed);

	if (parent)
		parent->addClientSocket(this);

	return setError(eTCPNoError);
}

teTCPError TCPClientConnection::disconnect( void )
{
	if (!connected())
		return setError(eTCPSocketNFG);

	SDLNet_TCP_Close(info->socket);
	info->socket = NULL;

	if (parent)
		parent->removeClientSocket(this);

	return setError(eTCPNoError);
}

bool TCPClientConnection::connected ( void )
{
	if (!info)
		return false;

	return info->socket != NULL;
}

bool TCPClientConnection::packets ( void )
{
	return packetList.size() > 0;
}

tvPacketList& TCPClientConnection::getPackets ( void )
{
	return packetList;
}

void TCPClientConnection::readData ( void )
{
	bool done = false;
	int dataRead = 0;

	unsigned char	*chunk = (unsigned char*)malloc(info->readChunkSize);
	unsigned char	*data = NULL;
	int		totalSize = 0;

	while (!done)
	{
		dataRead =  SDLNet_TCP_Recv(info->socket, chunk, (int)info->readChunkSize);
		if (dataRead > 0)
		{
			if (data)
				data = (unsigned char*)realloc(data,totalSize+dataRead);
			else
				data = (unsigned char*)malloc(dataRead);

			if (data)
			{
				memcpy(&data[totalSize],chunk,dataRead);
				totalSize += dataRead;

				if (dataRead < info->readChunkSize)
					done = true;
			}
			else	// there was an error
				done = true;

		}
		else // there was an error
			done = true;
	}

	if (chunk)
		free(chunk);

	if (data)
	{
		packetList.push_back(TCPPacket(data,totalSize));
		free(data);
	}

	// notify any listeners
	callDataPendingListeners((int)packetList.size());
}

teTCPError TCPClientConnection::sendData ( void *data, int len )
{
	if (!info)
		return setError(eTCPNotInit);

	if (!info->socket)
		return setError(eTCPSocketNFG);

	if (!data || len < 1)
		return setError(eTCPDataNFG);

	int lenSent = SDLNet_TCP_Send(info->socket,data,len);

	if (lenSent < len)
		return setError(eTCPConnectionFailed);

	return setError(eTCPNoError);
}

teTCPError TCPClientConnection::sendData ( const char *data, int len )
{
	return sendData((void*)data,len);
}

teTCPError TCPClientConnection::sendData ( std::string data )
{
	return sendData(data.c_str(),(int)data.size());
}

// data pending listeners
void TCPClientConnection::addListener ( TCPClientDataPendingListener* listener )
{
	if (!listener)
		return;

	dataPendingList.push_back(listener);
}

void TCPClientConnection::removeListener ( TCPClientDataPendingListener* listener )
{
	if (!listener)
		return;

	tvClientDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		if (*itr == listener)
			itr = dataPendingList.erase(itr);
		else
			itr++;
	}
}

void TCPClientConnection::callDataPendingListeners ( int count )
{
	if ( count >1 )
		return;

	tvClientDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		(*itr)->pending(this,count);
		itr++;
	}
}

teTCPError TCPClientConnection::getLastError ( void )
{
	return info->lastError;
}

teTCPError TCPClientConnection::setError ( teTCPError error )
{
	if (info)
		info->lastError = error;
	return error;
}

void TCPClientConnection::setReadChunkSize ( unsigned int size )
{
	if (info && size > 0)
		info->readChunkSize = size;
}

unsigned int TCPClientConnection::getReadChunkSize ( void )
{
	return info ? info->readChunkSize : 0;
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP listener class for server connected peers

class TCPServerConnectedPeer::TCPServerConnectedPeerInfo
{	
public:
	TCPServerConnectedPeerInfo();
	TCPsocket	socket;
};

TCPServerConnectedPeer::TCPServerConnectedPeerInfo::TCPServerConnectedPeerInfo()
{
	socket = NULL;
}

TCPServerConnectedPeer::TCPServerConnectedPeer()
{
	info = new TCPServerConnectedPeerInfo;
}

TCPServerConnectedPeer::~TCPServerConnectedPeer()
{
	if (info)
		delete(info);
}

// data pending
bool TCPServerConnectedPeer::packets ( void )
{
	return packetList.size() > 0;
}

tvPacketList& TCPServerConnectedPeer::getPackets ( void )
{
	return packetList;
}

const std::string TCPServerConnectedPeer::getAddress ( void )
{
	return "something";
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP listener class for servers

// the info data for the class
struct TCPServerConnection::TCPServerConnectionInfo
{	
	teTCPError	lastError;
	int			maxUsers;
	IPaddress	serverIP;
	TCPsocket	socket;
	int readChunkSize;
};

TCPServerConnection::TCPServerConnection()
{
	info = new TCPServerConnectionInfo;
	info->lastError = eTCPNotInit;
	info->serverIP.port = info->maxUsers = 0;
	info->socket = NULL;
	parent = NULL;
	readChunkSize = 512;
}

TCPServerConnection::TCPServerConnection( unsigned short port, unsigned int connections, TCPConnection *parentConnection )
{
	info = new TCPServerConnectionInfo;
	info->lastError = eTCPNotInit;
	parent = parentConnection;
	info->socket = NULL;
	readChunkSize = 512;
	listen(port,connections);
}

TCPServerConnection::~TCPServerConnection()
{
	// do some TCP/IPish things here
	if (info)
		delete(info);
}

teTCPError TCPServerConnection::listen ( unsigned short port, unsigned int connections )
{
	disconnect();

	if (!info)
		return setError(eTCPNotInit);

	if ( port == 0)
		return setError(eTCPBadPort);

	info->maxUsers = connections;
	info->serverIP.host = INADDR_ANY;
	info->serverIP.port = port;

	info->socket = SDLNet_TCP_Open(&info->serverIP);
	if ( info->socket == NULL )
		return setError(eTCPConnectionFailed);

	if (parent)
		parent->addClientSocket(this);

	return setError(eTCPNoError);
}

teTCPError TCPServerConnection::disconnect( void )
{
	if (!info)
		return setError(eTCPNotInit);

	if (!listening())
		return setError(eTCPSocketNFG);

	SDLNet_TCP_Close(info->socket);
	info->socket = NULL;

	if (parent)
		parent->removeServerSocket(this);

	return setError(eTCPNoError);
}

bool TCPServerConnection::listening ( void )
{
	if (!info)
		return false;

	return info->socket != NULL;
}

teTCPError TCPServerConnection::getLastError ( void )
{
	return info->lastError;
}

teTCPError TCPServerConnection::setError ( teTCPError error )
{
	if (info)
		info->lastError = error;
	return error;
}

// data pending listeners
void TCPServerConnection::addListener ( TCPServerDataPendingListener* listener )
{
	if (!listener)
		return;

	dataPendingList.push_back(listener);
}

void TCPServerConnection::removeListener ( TCPServerDataPendingListener* listener )
{
	if (!listener)
		return;

	tvServerDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		if (*itr == listener)
			itr = dataPendingList.erase(itr);
		else
			itr++;
	}
}

void TCPServerConnection::callDataPendingListeners ( int count )
{
	if ( count >1 )
		return;

		// TODO this is wrong, each client has a pending list loop thru them
}


// master connections class

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
TCPConnection* Singleton<TCPConnection>::_instance = (TCPConnection*)0;


typedef std::map<TCPsocket, TCPClientConnection* > tmClientSocketMap;
typedef std::map<TCPsocket, TCPServerConnection* > tmServerSocketMap;

class TCPConnection::TCPConnectionInfo
{	
public:
	TCPConnectionInfo();
	tmClientSocketMap	clientSockets;
	SDLNet_SocketSet	clientSocketSet;
	tmServerSocketMap	serverSockets;
	SDLNet_SocketSet	serverSocketSet;
	bool initedSocketInterface;
	int	 timeout;
};

TCPConnection::TCPConnectionInfo::TCPConnectionInfo()
{
	clientSocketSet = NULL;
	serverSocketSet = NULL;
	initedSocketInterface = false;
	timeout = 0;
}

TCPConnection::TCPConnection()
{
	info = new TCPConnectionInfo;
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
	if(SDLNet_Init() ==0)
		info->initedSocketInterface = true;
	return info->initedSocketInterface ? eTCPNoError : eTCPInitFailed;
}

void TCPConnection::kill ( void )
{
	if (info->initedSocketInterface)
	{
		SDLNet_Quit();
		info->initedSocketInterface = false;
	}
}

teTCPError TCPConnection::update ( void )
{
	if (!info)
		return eTCPNotInit;

	bool	selectError = false;
	if (info->clientSocketSet)
	{
		int items = SDLNet_CheckSockets(info->clientSocketSet,info->timeout);
		if (items == -1)
			selectError = true;
		if (items > 0)
		{
			tmClientSocketMap::iterator itr = info->clientSockets.begin();
			while (itr != info->clientSockets.end())
			{
				if (SDLNet_SocketReady(itr->first))
					itr->second->readData();
				itr++;
			}
		}
	}

	if (info->serverSocketSet)
	{
		int items = SDLNet_CheckSockets(info->clientSocketSet,info->timeout);
		if (items == -1)
			return eTCPSelectFailed;
		if (items > 0)
		{
			tmClientSocketMap::iterator itr = info->clientSockets.begin();
			while (itr != info->clientSockets.end())
			{
				if (SDLNet_SocketReady(itr->first))
					itr->second->readData();
				itr++;
			}
		}
	}

	return selectError ? eTCPSelectFailed : eTCPNoError;
}

void TCPConnection::setUpdateTimeout ( int timeout )
{
	if (info && timeout > 0)
		info->timeout = timeout;
}


TCPClientConnection* TCPConnection::newClientConnection ( std::string server, unsigned short port )
{
	TCPClientConnection*	connection = new  TCPClientConnection(server,port,this);
	
	clientConnections.push_back(connection);
	return connection;
}

TCPServerConnection* TCPConnection::newServerConnection ( unsigned short port, int connections )
{
	TCPServerConnection*	connection = new  TCPServerConnection(port,connections,this);

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

bool TCPConnection::addClientSocket ( TCPClientConnection* client )
{
	if (info->clientSocketSet)
		SDLNet_FreeSocketSet(info->clientSocketSet);

	info->clientSockets[client->info->socket] = client;

	info->clientSocketSet = SDLNet_AllocSocketSet((int)info->clientSockets.size());

	if (!info->clientSocketSet)
		return false;

	tmClientSocketMap::iterator itr = info->clientSockets.begin();
	while (itr != info->clientSockets.end())
	{
		SDLNet_TCP_AddSocket(info->clientSocketSet,itr->first);
		itr++;
	}

	return true;
}

bool TCPConnection::addServerSocket ( TCPServerConnection* server )
{
	if (info->serverSocketSet)
		SDLNet_FreeSocketSet(info->serverSocketSet);

	info->serverSockets[server->info->socket] = server;

	info->clientSocketSet = SDLNet_AllocSocketSet((int)info->serverSockets.size());

	if (!info->clientSocketSet)
		return false;

	tmClientSocketMap::iterator itr = info->clientSockets.begin();
	while (itr != info->clientSockets.end())
	{
		SDLNet_TCP_AddSocket(info->clientSocketSet,itr->first);
		itr++;
	}

	return true;
}

bool TCPConnection::removeClientSocket ( TCPClientConnection* client )
{
	tmClientSocketMap::iterator itr = info->clientSockets.find(client->info->socket);
	if (itr == info->clientSockets.end())
		return false;

	SDLNet_TCP_DelSocket(info->clientSocketSet,client->info->socket);
	info->clientSockets.erase(itr);
	return true;
}

bool TCPConnection::removeServerSocket ( TCPServerConnection* server )
{
	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
