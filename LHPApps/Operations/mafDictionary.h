/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafDictionary.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mafDictionary_H__
#define __mafDictionary_H__


#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>
#include "wx/string.h"
#include "mafString.h"

void ParseString(wxString &pFirstLine, wxString &sOne, wxString &sTwo);
bool ReadDictionary(mafString *fileName, std::vector<std::pair<wxString, wxString> >&  dictionary);
wxString const *LookupUserName(wxString const *name, std::vector<std::pair<wxString, wxString> >&  dictionary);
wxString const *LookupStdName(wxString const *name, std::vector<std::pair<wxString, wxString> >&  dictionary);


#endif