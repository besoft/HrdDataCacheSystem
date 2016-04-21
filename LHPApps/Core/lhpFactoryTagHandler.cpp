/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpFactoryTagHandler.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Marco Petrone
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpFactoryTagHandler.h"
#include "mafVersion.h"
#include "mafIndent.h"

#include <string>
#include <ostream>

// #include "lhpTagHandler.h"  to be defined in a separate file...
// #include "lhpDefaultTagHandler.h..." to be defined in a separate file...

lhpFactoryTagHandler *lhpFactoryTagHandler::m_Instance=NULL;

mafCxxTypeMacro(lhpFactoryTagHandler);

std::vector<std::string> lhpFactoryTagHandler::m_TagHandlerNames;

//----------------------------------------------------------------------------
// This is used to register the factory when linking statically
int lhpFactoryTagHandler::Initialize()
//----------------------------------------------------------------------------
{
  if (m_Instance==NULL)
  {
    m_Instance=lhpFactoryTagHandler::New();

    if (m_Instance)
    {
      m_Instance->RegisterFactory(m_Instance);
      return MAF_OK;  
    }
    else
    {
      return MAF_ERROR;
    }
  }
  
  return MAF_OK;
}

//------------------------------------------------------------------------
lhpFactoryTagHandler::lhpFactoryTagHandler()
//------------------------------------------------------------------------------
{
  m_Instance = NULL;
  
  // lhpPlugTagHandlerMacro(lhpTagHandler,"General tag handler");

}

//------------------------------------------------------------------------------
const char* lhpFactoryTagHandler::GetMAFSourceVersion() const
//------------------------------------------------------------------------------
{
  // to be defined LHP_SOURCE_VERSION...
  // return LHP_SOURCE_VERSION;
  // return "LHP_SOURCE_VERSION_UNDEFINED!!! Fix LHP_SOURCE_VERSION";
  return MAF_SOURCE_VERSION;
}

//------------------------------------------------------------------------------
const char* lhpFactoryTagHandler::GetDescription() const
//------------------------------------------------------------------------------
{
  return "Factory for LHDL tags handlers";
}

//------------------------------------------------------------------------------
lhpTagHandler *lhpFactoryTagHandler::CreateTagHandlerInstance(const char *type_name)
//------------------------------------------------------------------------------
{
  return lhpTagHandler::SafeDownCast(Superclass::CreateInstance(type_name));
}

//------------------------------------------------------------------------------
void lhpFactoryTagHandler::RegisterNewTagHandler(const char* tagHandlerName, const char* description, mafCreateObjectFunction createFunction)
//------------------------------------------------------------------------------
{
  m_TagHandlerNames.push_back(tagHandlerName);
  RegisterNewObject(tagHandlerName,description,createFunction);
}
