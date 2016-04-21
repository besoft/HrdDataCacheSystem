/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMoveVMEUp.cpp,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:23 $
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

#include "lhpOpMoveVMEUp.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMoveVMEUp);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMoveVMEUp::lhpOpMoveVMEUp(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = false;  	
}
//----------------------------------------------------------------------------
lhpOpMoveVMEUp::~lhpOpMoveVMEUp( ) 
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpMoveVMEUp::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpMoveVMEUp(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpMoveVMEUp::Accept(mafNode *node)
//----------------------------------------------------------------------------
{	
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpMoveVMEUp::OpRun()   
//----------------------------------------------------------------------------
{ 	
	if (wxMessageBox(_("This operation changes the position of selected VME in the tree. The change WILL NOT BE apparent in VME Tree until you SAVE and RELOAD the MSF! Do you want to continue?"),
		wxMessageBoxCaptionStr, wxICON_QUESTION | wxYES_NO) == wxYES)
	{
		mafEventMacro(mafEvent(this, OP_RUN_OK));		
	}
	else
	{
		mafEventMacro(mafEvent(this, OP_RUN_CANCEL));		
	}
}

//----------------------------------------------------------------------------
void lhpOpMoveVMEUp::OpDo()
//----------------------------------------------------------------------------
{	
	mafNode* parent = m_Input->GetParent();
	const mafNode::mafChildrenVector* children_const = parent->GetChildren();
	mafNode::mafChildrenVector* children = const_cast< mafNode::mafChildrenVector* > (children_const);

	int count = (int)children->size();
	for (int i = 1; i < count; i++)
	{
		if ((mafNode*)children->at(i) == m_Input)
		{
			mafNode* tmp = children->at(i);
			children->at(i) = children->at(i - 1);
			children->at(i - 1) = tmp;
		}
	}
}


#endif