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

#include "URLManager.h"
#include "OSFile.h"

#include "log.h"

URLJobHandler::~URLJobHandler()
{
}

bool URLJobHandler::write ( int jobID, unsigned char* data, unsigned int size )
{
	return false;
}

bool URLJobHandler::progress ( int jobID, unsigned int total, unsigned int current )
{
	return false;
}

bool URLJobHandler::done ( int jobID, unsigned char* data, unsigned int size )
{
	return false;
}

bool URLJobHandler::error ( int jobID, unsigned int error )
{
	return false;
}

URLJobHandler	defaultHandler;

// the handler thingy

size_t writeFunction(void *ptr, size_t size, size_t nmemb, void *userData )
{
	size_t realSize = size* nmemb;
	unsigned char * data = (unsigned char*)ptr;

	URLManager::URLJob *jobPointer = (URLManager::URLJob*)userData;
	if (!jobPointer || ! jobPointer->handler)
		return realSize;

	if (!jobPointer->data)	// save off our copy of the data
	{
		jobPointer->data = (unsigned char*)malloc(realSize);
		memcpy(jobPointer->data,data,realSize);
		jobPointer->size = (unsigned int )realSize;
	}
	else
	{
		if(!realloc(jobPointer->data,jobPointer->size+realSize))
			return 0;
		memcpy(jobPointer->data+jobPointer->size,data,realSize);
		jobPointer->size += (unsigned int)realSize;
	}

	if (jobPointer->handler->write(jobPointer->jobID,(unsigned char*)data,(unsigned int)realSize))
		return 0;
	return realSize;
}

int progressFunction (void *userData, double dltotal, double dlnow, double ultotal, double ulnow)
{
	URLManager::URLJob *jobPointer = (URLManager::URLJob*)userData;
	if (!jobPointer || ! jobPointer->handler)
		return 1;

	if (jobPointer->handler->progress(jobPointer->jobID,(unsigned int)dltotal,(unsigned int)dlnow))
		return 1;

	return 0;
}

//---------------------URLManager-------------------------


URLJobDescriptor::URLJobDescriptor()
{
	handler = NULL;
	timeout = -1;
	nobody = false;
	post = false;

}

URLManager::URLManager()
{
	lastJob = 0;
	maxJobs = 2;
	multiHandle = NULL;
	timeout = 120;

	if (curl_global_init(CURL_GLOBAL_WIN32) == CURLE_OK)
		multiHandle = curl_multi_init();
	else
		log("URLManager init failed");
}

URLManager::~URLManager()
{
}

int URLManager::addJob ( std::string &URL, URLJobHandler *handler )
{
	URLJobDescriptor descriptor;

	descriptor.URL = URL;
	descriptor.handler = handler;

	return addJob(descriptor);
}

int URLManager::addJob (  URLJobDescriptor &descriptor )
{
	if (!multiHandle)
		return -1;

	URLJob job;
	job.url = descriptor.URL;
	job.jobID = ++lastJob;
	job.data = NULL;
	job.size = 0;
	job.nobody = descriptor.nobody;
	job.interfaceIP = descriptor.interfaceIP;
	job.userAgent = descriptor.userAgent;
	job.post = descriptor.post;
	job.postData = descriptor.postData;

	if ( descriptor.handler )
		job.handler =descriptor.handler;
	else
		job.handler = &defaultHandler;
	job.easyHandle = NULL;

	job.timeout = descriptor.timeout;
	if ( job.timeout < 1)
		job.timeout = timeout;

	jobList[job.jobID] = job;
	doJob();

	return lastJob;
}

bool URLManager::removeJob ( int job )
{
	if (!multiHandle)
		return false;

	return false;
}

void URLManager::setMaxJobs ( int jobs )
{
	if (jobs > 1 )
		maxJobs = jobs;
}

void URLManager::setTimeOut ( int time )
{
	if ( time > 0 )
		timeout = time;
}

void URLManager::updateJobs ( void )
{
	if (!multiHandle || !currentJobs.size())
		return;

	int activeTransfers = 0;

	CURLMcode result = curl_multi_perform(multiHandle, &activeTransfers);
	if ( result > CURLM_OK)
		return;

	// see who is done if anyone
	CURLMsg *pendingMsg;
	int      msgs_in_queue;

	bool done = false;
	while (!done)
	{
		pendingMsg = curl_multi_info_read(multiHandle, &msgs_in_queue);
		if (!pendingMsg)
			done = true;
		else if (pendingMsg->msg ==CURLMSG_DONE)
		{
			// one of our boys is done;
			unsigned int jobID = findJobFromHandle(pendingMsg->easy_handle);
			if ( jobID > 0 )
			{
				URLJob &job = jobList[jobID];

				bool killHandler = false;

				if ((CURLcode)pendingMsg->data.result == CURLE_OK)
					killHandler = job.handler->done(jobID,job.data,job.size);
				else
					killHandler = job.handler->error(jobID,(unsigned int)pendingMsg->data.result);

				if (killHandler)
					delete(job.handler);

				if ( job.data )
					free(job.data);
				if (job.easyHandle)
				{
					curl_multi_remove_handle(multiHandle,job.easyHandle);
					curl_easy_cleanup(job.easyHandle);
				}
				jobList.erase(jobList.find(jobID));
			}
			done = msgs_in_queue > 0;
		}
	}

	doJob();
}

void URLManager::doJob ( void )
{
	if (!multiHandle || !jobList.size())
		return;

	if ( (int)currentJobs.size() >= maxJobs )	
		return;	// we are doing too many now

	// while we have jobs to do.
	bool done = false;
	while ( !done )
	{
		int jobToDo = 0;
		// find a job to do;
		std::map<int, URLJob >::iterator jobItr = jobList.begin();
		while ( jobItr != jobList.end() )
		{
			if ( !jobItr->second.easyHandle )
			{
				jobToDo = jobItr->second.jobID;
				jobItr = jobList.end();
			}
			else
				jobItr++;
		}
		if (jobToDo < 1)	// there were no jobs that are not in process
			done = true;
		else
		{
			URLJob &job = jobList[jobToDo];

			job.easyHandle = curl_easy_init();	
			if (!job.easyHandle) // this would be bad
				return;

			curl_easy_setopt(job.easyHandle, CURLOPT_WRITEFUNCTION,writeFunction);
			curl_easy_setopt(job.easyHandle, CURLOPT_WRITEDATA, &job);
			curl_easy_setopt(job.easyHandle, CURLOPT_PROGRESSFUNCTION,progressFunction);
			curl_easy_setopt(job.easyHandle, CURLOPT_PROGRESSDATA, &job);
			curl_easy_setopt(job.easyHandle, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(job.easyHandle, CURLOPT_TIMEOUT, job.timeout);
			curl_easy_setopt(job.easyHandle, CURLOPT_URL,job.url.c_str());

			if ( job.interfaceIP.size() )
				curl_easy_setopt(job.easyHandle, CURLOPT_INTERFACE,job.interfaceIP.c_str());

			if ( job.userAgent.size() )
				curl_easy_setopt(job.easyHandle, CURLOPT_USERAGENT,job.userAgent.c_str());

			if ( job.nobody )
				curl_easy_setopt(job.easyHandle, CURLOPT_NOBODY,1);

			if ( job.post )
			{
				curl_easy_setopt(job.easyHandle, CURLOPT_POSTFIELDS, job.postData.c_str());
				curl_easy_setopt(job.easyHandle, CURLOPT_POSTFIELDSIZE, job.postData.size());
				curl_easy_setopt(job.easyHandle, CURLOPT_POST,1);
			}
		//	else
		//		curl_easy_setopt(job.easyHandle, CURLOPT_HTTPGET,1);

			// add the job to our multi handle
			curl_multi_add_handle(multiHandle, job.easyHandle);
			
			currentJobs.push_back(job.jobID);

			if ((int)currentJobs.size() >= maxJobs)	// don't add any more jobs, since we are full
				done = true;
		}
	}
}

int URLManager::findJobFromHandle ( CURL* handle )
{
	if (!handle)
		return -1;

	for ( unsigned int i = 0; i < currentJobs.size(); i++ )
	{
		if ( jobList[currentJobs[i]].easyHandle == handle)
			return currentJobs[i];
	}
	return -1;
}

//-----------------------------URLFileDownloadHandler--------------------------

URLFileDownloadHandler::URLFileDownloadHandler ( const char* filename, bool binary )
{
	if (filename)
		file = filename;
	binFile = binary;
}

bool URLFileDownloadHandler::done ( int jobID, unsigned char* data, unsigned int size )
{
	if (!file.size())
		return true;

	FILE	*fp = fopen(file.c_str(),binFile ? "wb" : "w");
	if (fp)
	{
		fwrite(data,size,1,fp);
		fclose(fp);
	}
	return true;
}

bool URLFileDownloadHandler::error ( int jobID, unsigned int error )
{
	// maybe so some sorta assert?
	return true;
}



