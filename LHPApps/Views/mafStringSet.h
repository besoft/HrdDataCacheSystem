/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafStringSet.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

/**
* Structure to save number of properties in string set
*
*/
#ifndef __mafStringSet_H__
#define __mafStringSet_H__

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------
#include <vector>
#include "wx/wxprec.h" 
#include "wx/string.h"
#include "wx/window.h"

#include "mafString.h"


//----------------------------------------------------------------------------
// class mafStringSet
//----------------------------------------------------------------------------
class mafStringSet
{
public:
  mafStringSet(wxInt32 nStringNumber, const std::vector<mafString> *pData = NULL);
  virtual ~mafStringSet();
  wxInt32 GetStringNumber() const {return m_StringNumber;}
  wxInt32 SetStringNumber(wxInt32 nSN) {wxInt32 nOld = m_StringNumber; m_StringNumber = nSN; return nOld;}
  wxChar  **GetData() {return m_Data;}
  wxChar  ** const GetData() const {return m_Data;}

protected:
  //number of strings
  wxInt32 m_StringNumber;
  wxChar  **m_Data;
};
#endif
