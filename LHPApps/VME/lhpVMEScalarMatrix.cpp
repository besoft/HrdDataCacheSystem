/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVMEScalarMatrix.cpp,v $
  Language:  C++
  Date:      $Date: 2009-11-03 12:58:03 $
  Version:   $Revision: 1.1.2.1 $
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


#include "lhpVMEScalarMatrix.h"
#include "mafGUI.h"

//-------------------------------------------------------------------------
mafCxxTypeMacro(lhpVMEScalarMatrix)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
lhpVMEScalarMatrix::lhpVMEScalarMatrix()
:mafVMEScalarMatrix()
//-------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
lhpVMEScalarMatrix::~lhpVMEScalarMatrix()
//-------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
mafGUI* lhpVMEScalarMatrix::CreateGui()
//-------------------------------------------------------------------------
{
  m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
  return m_Gui;
}
//-------------------------------------------------------------------------
char** lhpVMEScalarMatrix::GetIcon() 
//-------------------------------------------------------------------------
{
#include "mafVMEScalar.xpm"
  return mafVMEScalar_xpm;
}