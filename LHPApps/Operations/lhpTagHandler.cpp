/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandler.cpp,v $
  Language:  C++
  Date:      $Date: 2009-09-04 13:09:37 $
  Version:   $Revision: 1.1.1.1.2.1 $
  Authors:   Stefano Perticoni - Daniele Giunchi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h"
//----------------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------------

#include "lhpUtils.h"
#include "lhpTagHandler.h"
#include "mafDecl.h"
#include "lhpBuilderDecl.h"

mafCxxTypeMacro(lhpTagHandlerInputOutputParametersCargo);
//------------------------------------------------------------------------
lhpTagHandlerInputOutputParametersCargo::lhpTagHandlerInputOutputParametersCargo()
//------------------------------------------------------------------------------
{
  m_InputVme = NULL;
	m_InputUser = NULL;
	m_InputMSF = "";

  m_TagHandlerGeneratedString = "NOT YET HANDLED!";
}


mafCxxTypeMacro(lhpTagHandler);
//------------------------------------------------------------------------
lhpTagHandler::lhpTagHandler()
//------------------------------------------------------------------------------
{
	m_PythonExe ="python.exe ";
	m_PythonwExe ="pythonw.exe ";
	m_VMEUploaderDownloaderDir  = (lhpUtils::lhpGetApplicationDirectory() + "\\..\\VMEUploaderDownloaderRefactor\\").c_str();
}

//------------------------------------------------------------------------
void lhpTagHandler::ExtractTagName()
//------------------------------------------------------------------------
{
  // tag name from type
  m_TagName = this->GetTypeName();
  int endPos = m_TagName.FindFirst("_");
  m_TagName.Erase(0, endPos);
}