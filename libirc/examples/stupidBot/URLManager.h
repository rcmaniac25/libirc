// GPx01
// Copyright (c) 2005 - 2006 Jeff Myers
//
// This package is free software;  you can redistribute it and/or
// modify it under the terms of the license found in the file
// named COPYING that should have accompanied this file.
//
// THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// URLManager.h, job based URL retreval manager

#ifndef URL_MANAGER_H
#define URL_MANAGER_H

// system headers
#ifdef _WIN32
#include <windows.h>
#define _WINSOCK2API_
#endif

#include <curl/curl.h>
#include <string>
#include <map>
#include <vector>

class URLJobHandler
{
public:
	virtual ~URLJobHandler();
	virtual bool write ( int jobID, unsigned char* data, unsigned int size );
	virtual bool progress ( int jobID, unsigned int total, unsigned int current );
	virtual bool done ( int jobID, unsigned char* data, unsigned int size );	// return true to have the URLManager delete the handler
	virtual bool error ( int jobID, unsigned int error );						// return true to have the URLManager delete the handler
};

typedef struct URLJobDescriptor
{
	URLJobDescriptor();

	std::string		URL;
	URLJobHandler	*handler;
	int				timeout;
	bool			nobody;

	std::string		userAgent;
	std::string		interfaceIP;

	bool			post;
	std::string		postData;
}URLJobDescriptor;

class URLManager
{
public:

	static URLManager& Instance()
	{
		static URLManager um;
		return um;
	} 

	~URLManager();

	int addJob ( std::string &URL, URLJobHandler *handler );
	int addJob ( URLJobDescriptor &descriptor );

	bool removeJob ( int job );

	void setMaxJobs ( int jobs );
	void setTimeOut ( int time );

	void updateJobs ( void );

	typedef struct 
	{
		std::string		url;
		int				jobID;
		
		int				timeout;
		bool			nobody;

		std::string		userAgent;
		std::string		interfaceIP;

		bool			post;
		std::string		postData;

		URLJobHandler	*handler;
		CURL			*easyHandle;

		unsigned char	*data;
		unsigned int	size;
	}URLJob;

protected:
	URLManager();

	std::map<int, URLJob > jobList;

	int	lastJob;
	int maxJobs;
	std::vector<int>	currentJobs;

	int timeout;

	void doJob ( void );

	int findJobFromHandle ( CURL* handle );
	// curl stuff
	CURLM				*multiHandle;
};

// some default job handlers

class URLFileDownloadHandler :URLJobHandler
{
public:
	URLFileDownloadHandler ( const char* filename, bool binary = true );
	virtual bool done ( int jobID, unsigned char* data, unsigned int size );	// return true to have the URLManager delete the handler
	virtual bool error ( int jobID, unsigned int error );						// return true to have the URLManager delete the handler

protected:
	std::string file;
	bool		binFile;
};

#endif // URL_MANAGER_H

