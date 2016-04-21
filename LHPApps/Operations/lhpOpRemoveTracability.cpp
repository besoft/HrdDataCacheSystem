/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpRemoveTracability.cpp,v $
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

#include "lhpOpRemoveTracability.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVMEGroup.h"
#include "mafVMEOutput.h"
#include "vtkDataSet.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpRemoveTracability);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpRemoveTracability::lhpOpRemoveTracability(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = false;  
	m_bPropagate = false;
}
//----------------------------------------------------------------------------
lhpOpRemoveTracability::~lhpOpRemoveTracability( ) 
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpRemoveTracability::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpRemoveTracability(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpRemoveTracability::Accept(mafNode *node)
//----------------------------------------------------------------------------
{	
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpRemoveTracability::OpRun()   
//----------------------------------------------------------------------------
{ 	
	if (wxNO == wxMessageBox(_("This operation will remove any history of activity "
		"(mafAttributeTraceability) stored within the VME. Do you want to continue?"), 
		_("Confirmation required"),	wxYES_NO | wxICON_QUESTION)) 
	{
		mafEventMacro(mafEvent(this, OP_RUN_CANCEL));	
	}
	else
	{
		m_bPropagate = wxYES == wxMessageBox(_("Do you want to apply the operation to all VME children?"), 
		_("Confirmation required"),	wxYES_NO | wxICON_QUESTION);

		mafEventMacro(mafEvent(this, OP_RUN_OK));	
	}
}

//----------------------------------------------------------------------------
void lhpOpRemoveTracability::OpDo()
//----------------------------------------------------------------------------
{	
	static const char* ATTR_NAME = "TrialAttribute";

	if (!m_bPropagate)
	{
		if (m_Input->GetAttribute(ATTR_NAME) != NULL)	//This is HOTFIX
			m_Input->RemoveAttribute(ATTR_NAME); //will crash, if the attribute with ATTR_NAME does not exist
	}
	else
	{
		std::vector< mafNode* > stack;
		stack.push_back(m_Input);

		while (stack.size() > 0)
		{
			mafNode* node = stack[stack.size() - 1]; 
			stack.pop_back();

			if (node->GetAttribute(ATTR_NAME) != NULL)	//This is HOTFIX
				node->RemoveAttribute(ATTR_NAME); //will crash, if the attribute with ATTR_NAME does not exist

			const mafNode::mafChildrenVector* children = node->GetChildren();
			int m = (int)children->size();
			for (int i = 0; i < m; i++) {
				stack.push_back(children->at(i));
			}
		} //end while
	}
}


#endif