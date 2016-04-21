/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafStringSet.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafStringSet.h"

#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
mafStringSet::mafStringSet(wxInt32 nStringNumber, const std::vector<mafString> *pData)
//----------------------------------------------------------------------------
{
  wxInt32 nI;
  m_Data = NULL;
  m_StringNumber = nStringNumber;

  if(m_StringNumber > 0)
  {
    m_Data = (wxChar **)malloc(sizeof(wxChar *) * m_StringNumber);
    if(pData == NULL)
    {
      for(nI = 0; nI < m_StringNumber; nI++)
      {
        m_Data[nI] = NULL;
      }
    }
    else
    {
      for(nI = 0; nI < m_StringNumber; nI++)
      {
        m_Data[nI] = strdup(pData->at(nI).GetCStr());
      }
    }
  }
}

//----------------------------------------------------------------------------
mafStringSet::~mafStringSet()
//----------------------------------------------------------------------------
{
  wxInt32 nI;

  if(m_Data != NULL)
  {
    for(nI = 0; nI < m_StringNumber; nI++)
    {
      if(m_Data[nI] != NULL)
        free(m_Data[nI]);
    }
    free(m_Data);
  }
}
