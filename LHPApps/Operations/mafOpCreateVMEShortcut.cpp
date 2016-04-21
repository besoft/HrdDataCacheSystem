/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpCreateVMEShortcut.cpp,v $
  Language:  C++
  Date:      $Date: 2011-03-03 11:37:46 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011
  University of West Bohemia
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "mafOpCreateVMEShortcut.h"
#include "mafDecl.h"
#include "mafEvent.h"

#include "../VME/mafVMEShortcut.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafOpCreateVMEShortcut);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafOpCreateVMEShortcut::mafOpCreateVMEShortcut(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;
  m_Shortcut   = NULL;
}
//----------------------------------------------------------------------------
mafOpCreateVMEShortcut::~mafOpCreateVMEShortcut( ) 
//----------------------------------------------------------------------------
{
  mafDEL(m_Shortcut);
}
//----------------------------------------------------------------------------
mafOp* mafOpCreateVMEShortcut::Copy()   
//----------------------------------------------------------------------------
{
	return new mafOpCreateVMEShortcut(m_Label);
}
//----------------------------------------------------------------------------
bool mafOpCreateVMEShortcut::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void mafOpCreateVMEShortcut::OpRun()   
//----------------------------------------------------------------------------
{
  mafNEW(m_Shortcut);
  m_Shortcut->SetName("Unknown Shortcut");
  m_Output = m_Shortcut;
  mafEventMacro(mafEvent(this,OP_RUN_OK));
}
//----------------------------------------------------------------------------
void mafOpCreateVMEShortcut::OpDo()
//----------------------------------------------------------------------------
{
  m_Shortcut->ReparentTo(m_Input);
}
