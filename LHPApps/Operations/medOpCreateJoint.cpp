/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateJoint.cpp,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:24 $
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


#include "medOpCreateJoint.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "medVMEJoint.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(medOpCreateJoint);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
medOpCreateJoint::medOpCreateJoint(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;  
}

//----------------------------------------------------------------------------
medOpCreateJoint::~medOpCreateJoint( ) 
//----------------------------------------------------------------------------
{
  mafDEL(m_Output);
}

//----------------------------------------------------------------------------
mafOp* medOpCreateJoint::Copy()   
//----------------------------------------------------------------------------
{
	return new medOpCreateJoint(m_Label);
}

//----------------------------------------------------------------------------
bool medOpCreateJoint::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return node != NULL;
}

//----------------------------------------------------------------------------
void medOpCreateJoint::OpRun()   
//----------------------------------------------------------------------------
{
	medVMEJoint* joint;

  mafNEW(joint);
  joint->SetName("Unknown Joint");

  m_Output = joint;
  mafEventMacro(mafEvent(this,OP_RUN_OK));
}

//----------------------------------------------------------------------------
void medOpCreateJoint::OpDo()
//----------------------------------------------------------------------------
{	
	if (m_Output != NULL)
		m_Output->ReparentTo(m_Input);
}

//----------------------------------------------------------------------------
void medOpCreateJoint::OpUndo()
//----------------------------------------------------------------------------
{
	if (m_Output != NULL)
		m_Output->ReparentTo(NULL);	
}
