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

// simple TCP/IP connection sample

#include <stdio.h>
#include <stdlib.h>

#include "libIRC.h"

TCPConnection	tcpConnection;

class myClientDataPendingListener : public TCPClientDataPendingListener
{
public:
	virtual ~myClientDataPendingListener(){return;};
	virtual void pending ( TCPClientConnection *connection, int count );

};

int state = -1;

void sendToServer ( TCPClientConnection *connection, std::string text )
{
	std::string messageToSend = text;

	if (messageToSend.size())
	{
		messageToSend += " \r\n";

		connection->sendData((void*)messageToSend.c_str(),(int)messageToSend.size());
		printf("Sending Message: '%s'\n",text.c_str());
	}
}

void myClientDataPendingListener::pending ( TCPClientConnection *connection, int count )
{
	printf("Receved %d messages from connection\n",count);

	tvPacketList	&packetList = connection->getPackets();

	tvPacketList::iterator itr = packetList.begin();

	int message = 0;
	while (itr != packetList.end())
	{
		unsigned int len;

		TCPPacket	&packet = *itr;
		packet.get(len);

		if (len >0)
		{
			unsigned char* data = (unsigned char*)malloc(len+1);
			memset(data,0,len+1);
			memcpy(data,packet.get(len),len);
			std::string line = (char*)data;
			free(data);
			printf("Message %d: '%s'\n",message,line.c_str());
		}
		message++;
		itr++;
	}

	std::string messateToSend;

	switch (state)
	{
		case -1:
			sendToServer(connection,"NICK libIRCTestApp");
			sendToServer(connection,"USER libirctest foo bar :libirctest");
			state = 0;
		break;

		case 0:
			sendToServer(connection,"JOIN #BZFlag");
			sendToServer(connection,"MODE #BZFlag");
			state =1;
		break;
	}
	packetList.clear();
}

void main ( void )
{
	std::string		server = "irc.freenode.net";
	unsigned short	port = 6667;

	myClientDataPendingListener	listener;

	// just shove in some crap because we just want the connection
	TCPClientConnection*	client =  tcpConnection.newClientConnection ( server, 0 );

	// the client failed
	if (!client)
	{
		printf("Client init failed\n");
		return;
	}

	client->addListener(&listener);

	// ok lets just try to open a connection
	printf("Atempting connection to %s:%d\n",server.c_str(),port);
	client->connect(server,port);

	if (!client->connected())
	{
		printf("Connection to %s:%d failed\n",server.c_str(),port);

		// it failed so clean it up
		tcpConnection.deleteClientConnection(client);
		return;
	}

	// it worked
	printf("Connection to %s:%d completed\n",server.c_str(),port);

	// now run till we die
	//printf("sending some newlines\n");
	//char	somestuff[]="\n\n\n";
	//client->sendData(somestuff,(int)strlen(somestuff));

	while (!tcpConnection.update())
	{

		IRCOSSleep(1);
	}

};