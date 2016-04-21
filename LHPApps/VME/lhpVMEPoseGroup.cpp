/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVMEPoseGroup.cpp,v $
  Language:  C++
  Date:      $Date: 2010-03-18 15:48:02 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2007
  SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpVMEPoseGroup.h"
#include "mafGUI.h"

//-------------------------------------------------------------------------
mafCxxTypeMacro(lhpVMEPoseGroup)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
lhpVMEPoseGroup::lhpVMEPoseGroup():
mafVMEGenericAbstract()
//-------------------------------------------------------------------------
{
  mafVMEOutputNULL *output=mafVMEOutputNULL::New(); // an output with no data
  SetOutput(output);
}
//-------------------------------------------------------------------------
lhpVMEPoseGroup::~lhpVMEPoseGroup()
//-------------------------------------------------------------------------
{
  SetOutput(NULL);

  // data pipe destroyed in mafVME
  // data vector destroyed in mafVMEGenericAbstract
}
//-------------------------------------------------------------------------
char** lhpVMEPoseGroup::GetIcon() 
//-------------------------------------------------------------------------
{
#include "mafVMEGroup.xpm"
  return mafVMEGroup_xpm;
}
//-------------------------------------------------------------------------
mafVMEOutput *lhpVMEPoseGroup::GetOutput()
//-------------------------------------------------------------------------
{
  // allocate the right type of output on demand
  if (m_Output==NULL)
  {
    SetOutput(mafVMEOutputNULL::New()); // create the output
  }
  return m_Output;
}

//-------------------------------------------------------------------------
mafGUI* lhpVMEPoseGroup::CreateGui()
//-------------------------------------------------------------------------
{
  m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
  m_Gui->Divider();
  return m_Gui;
}
//-----------------------------------------------------------------------
void lhpVMEPoseGroup::Print(std::ostream& os, const int tabs)
//-----------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
}