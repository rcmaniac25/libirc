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

// prefs.h

#ifndef _PREFS_H_
#define _PREFS_H_

#ifdef _WIN32
#pragma warning( disable : 4786 )  // Disable warning message
#endif

#include <string>
#include <map>
#include "OSFile.h"

typedef std::map<std::string,std::vector<std::string> >	PrefsItemList;
typedef std::map<std::string,PrefsItemList> PrefsGroups;

class CPrefsManager
{
public:
  CPrefsManager();
  ~CPrefsManager();

  void Init ( const char *szFileName );
  void Init ( std::string fileName );
  void Update ( void );

  bool ItemExists ( const char* group, const char* szName );
  void SetItem ( const char* group, const char* szName, int iData, bool push = false );
  void SetItem ( const char* group, const char* szName, float fData, bool push = false );
  void SetItem ( const char* group, const char* szName, const char* pData, bool push = false );

  void clearItem ( const char* group, const char* szName );

  int GetItemI ( const char* group, const char* szName );
  float GetItemF ( const char* group, const char* szName );
  const char* GetItemS ( const char* group, const char* szName );
  std::vector<std::string> GetItemV ( const char* group, const char* szName );

  int CountGroups ( void );
  std::vector<std::string> GetGroups ( void );
  std::vector<std::string> ListGroupItems ( const char* group );
  int CountGroupItems ( const char* group );

  const char* GetPrefsPath ( void );
  const char* GetPrefsFileName ( const char *pathName  );

protected:
  PrefsGroups		items;
  COSFile			file;

  void readFile ( bool merge );
  void writeFile ( void );
};

#endif//_PREFS_H_
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
