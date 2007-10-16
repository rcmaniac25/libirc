// prefs.cpp
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

#include "prefs.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include<stdlib.h>
#endif //WIN32

#include <stdio.h>
#include <stdlib.h>

#include "TextUtils.h"

// prefs class
CPrefsManager::CPrefsManager()
{
	items.clear();
}

CPrefsManager::~CPrefsManager()
{
	Update();
}

const char* CPrefsManager::GetPrefsPath ( void )
{
	std::string	filePathName;
#ifdef _WIN32

	char	szSysDir[MAX_PATH] = {0};
	char	buffer[MAX_PATH] = {0};

	// do the old way
	GetWindowsDirectory(szSysDir,MAX_PATH);
	filePathName = szSysDir;
	filePathName += "\\";

	// see if we can get it the new way
	HRESULT hr;
	if (SUCCEEDED (hr = SHGetSpecialFolderPath(NULL, buffer,  CSIDL_APPDATA, FALSE)))
	{
		strcpy(szSysDir, buffer);
		strcat(szSysDir, "\\");
		filePathName = szSysDir;
	}

#else
	// do some sort of home dir thing here
	filePathName =  getenv("HOME");
#endif

	static COSFile	temp;
	temp.OSName(filePathName.c_str());

	return temp.GetStdName();
}

const char* CPrefsManager::GetPrefsFileName ( const char *pathName )
{
	std::string	filePathName;
	static COSFile temp(pathName);
#ifdef _WIN32
	filePathName += temp.GetOSName();	
	filePathName += ".cfg";
#else
	// do some sort of home dir thing here
	filePathName += "/.";
	filePathName += temp.GetOSName();
#endif
	temp.OSName(filePathName.c_str());
	return temp.GetStdName();
}

void CPrefsManager::Init ( std::string fileName )
{
	Init(fileName.c_str());
}

void CPrefsManager::Init ( const char *pathName )
{
	if (!pathName)
		return;

	std::string	filePathName = GetPrefsPath();
	filePathName += GetPrefsFileName(pathName);	
	file.SetUseGlobalPath(false);
	file.StdName(filePathName.c_str());

	readFile(false);
}

void CPrefsManager::Update ( void )
{
	writeFile();
}

bool CPrefsManager::ItemExists ( const char* group, const char* szName )
{
	if (!szName)
		return false;

	std::string name = szName;
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));
	
	if (groupItr == items.end())
		return false;

	PrefsItemList::iterator itemItr = groupItr->second.find(name);

	if ( itemItr == groupItr->second.end() )
		return false;

	return itemItr->second.size() > 0;
}

void CPrefsManager::SetItem ( const char* group, const char* szName, int iData, bool push )
{
	if (!szName)
		return;

	char	szData[256];
	sprintf(szData,"%d",iData);

	SetItem(group,szName,szData,push);
}

void CPrefsManager::SetItem ( const char* group, const char* szName, float fData, bool push )
{
	if (!szName)
		return;

	char	szData[256];
	sprintf(szData,"%f",fData);

	SetItem(group,szName,szData,push);
}

void CPrefsManager::SetItem ( const char* group, const char* szName, const char* pData, bool push )
{
	if (!szName || !pData)
		return;

	std::string	name = szName;
	std::string data = pData;
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	// find the group
	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));
	if ( groupItr == items.end() )
	{
		// she got no group so add one
		std::vector<std::string> itemList;
		itemList.push_back(data);
		PrefsItemList itemGroup;
		itemGroup[name] = itemList;
		items[TextUtils::toupper(groupToUse)] = itemGroup;
		return;
	}

	// we have the group, see if we have the item
	PrefsItemList::iterator itemItr = groupItr->second.find(name);
	if ( itemItr == groupItr->second.end() )
	{
		std::vector<std::string> itemList;
		itemList.push_back(data);
		groupItr->second[name] = itemList;
		return;
	}
	// just another entry for the existing items
	if (!push)
		itemItr->second.clear();
	itemItr->second.push_back(data);
}

int CPrefsManager::GetItemI ( const char* group, const char* szName )
{
	const char *item = GetItemS(group,szName);
	if (!item)
		return 0;

	return atoi(item);
}

float CPrefsManager::GetItemF ( const char* group, const char* szName )
{
	const char *item = GetItemS(group,szName);
	if (!item)
		return 0;

	return (float)atof(item);
}

const char* CPrefsManager::GetItemS ( const char* group, const char* szName )
{
	if (!szName)
		return NULL;

	std::string name = szName;
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));

	if (groupItr == items.end())
		return NULL;

	PrefsItemList::iterator itemItr = groupItr->second.find(name);

	if ( itemItr == groupItr->second.end() )
		return NULL;

	if (itemItr->second.size() < 1 )
		return NULL;

	return itemItr->second[0].c_str();
}

std::vector<std::string> CPrefsManager::GetItemV ( const char* group, const char* szName )
{
	std::vector<std::string> temp;

	if (!szName)
		return temp;

	std::string name = szName;
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));

	if (groupItr == items.end())
		return temp;

	PrefsItemList::iterator itemItr = groupItr->second.find(name);

	if ( itemItr == groupItr->second.end() )
		return temp;

	return itemItr->second;
}

void CPrefsManager::clearItem ( const char* group, const char* szName )
{
	if (!szName)
		return;

	std::string name = szName;
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));

	if (groupItr == items.end())
		return;

	PrefsItemList::iterator itemItr = groupItr->second.find(name);

	if ( itemItr == groupItr->second.end() )
		return;

	groupItr->second.erase(itemItr);
}

void CPrefsManager::readFile ( bool merge )
{
	if (!merge)
		items.clear();

	file.Open("rt");
	if (!file.IsOpen())
		return;

	std::string group;
	std::string item;
	std::string value;

	char szName[256] = {0};
	char szData[256];

	std::vector<std::string> fileLines = file.ParseFile(std::string("\n"));

	for ( unsigned int i = 0; i < fileLines.size(); i++)
	{
		szName[0] = 0;
		szData[0] = 0;

		std::string line = TextUtils::trim_whitespace(fileLines[i]);
		if (line.size())
		{
			if (line[0] == '[')	// it's a group tag
			{
				line = TextUtils::toupper(TextUtils::replace_all(std::string(line.c_str()+1),std::string("]"),std::string("")));
				if ( line == "END" )
					group = "";
				else
					group = line;
			}
			else	// it's a line item
			{
				szName[0] = 0;
				sscanf(line.c_str(),"%s",szName);
				item = szName;
				if ( item.size() && (line.size() > item.size()+1) )
				{
					value = line.c_str()+item.size()+1;

					std::string groupToUse = "ROOT";
					if ( group.size() )
						groupToUse = group;	
					
					SetItem(groupToUse.c_str(),item.c_str(),value.c_str(),true);
				}
			}
		}
	}
	file.Close();
}

void CPrefsManager::writeFile ( void )
{
	file.Open("wt");
	if (!file.IsOpen())
		return;
	
	std::string line;

	PrefsGroups::iterator groupItr = items.begin();

	while ( groupItr != items.end() )
	{
		line = "[" + groupItr->first +  "]\n";
		file.Write(line);
		
		PrefsItemList::iterator itemItr = groupItr->second.begin();
		while ( itemItr != groupItr->second.end() )
		{
			if ( itemItr->second.size() )
			{
				for ( unsigned int i = 0; i < itemItr->second.size(); i++ )
				{
					line = itemItr->first + "\t" + itemItr->second[i] + "\n";
					file.Write(line);
				}
			}
			itemItr++;
		}
		file.Write(std::string("[end]\n\n"));
		groupItr++;
	}
	file.Close();
}

int CPrefsManager::CountGroups ( void )
{
	return (int)items.size();
}
std::vector<std::string> CPrefsManager::GetGroups ( void )
{
	std::vector<std::string>	list;

	PrefsGroups::iterator itr = items.begin();

	while ( itr != items.end() )
	{
		list.push_back(itr->first);
		itr++;
	}
	return list;
}

std::vector<std::string> CPrefsManager::ListGroupItems ( const char* group )
{
	std::vector<std::string>	list;

	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));

	if (groupItr == items.end())
		return list;

	PrefsItemList &groupList = groupItr->second;

	PrefsItemList::iterator itr = groupList.begin();
	while ( itr != groupList.end() )
	{
		list.push_back(itr->first);
		itr++;
	}
	return list;
}

int CPrefsManager::CountGroupItems ( const char* group )
{
	std::string groupToUse = "ROOT";
	if (group)
		groupToUse = group;

	PrefsGroups::iterator groupItr = items.find(TextUtils::toupper(groupToUse));

	if (groupItr == items.end())
		return 0;

	return (int)groupItr->second.size();
}




