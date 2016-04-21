/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpMultiReparentTo.cpp,v $
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

#include "lhpOpMultiReparentTo.h"
#if defined(VPHOP_WP10)

#include "mafOpReparentTo.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVMERoot.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMultiReparentTo);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMultiReparentTo::lhpOpMultiReparentTo(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType    = OPTYPE_OP;
	m_Canundo   = false;	
	m_TargetVme = NULL;
}

//----------------------------------------------------------------------------
lhpOpMultiReparentTo::~lhpOpMultiReparentTo( ) 
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
bool lhpOpMultiReparentTo::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node != NULL && node->IsMAFType(mafVME)); // && !node->IsMAFType(mafVMERoot));
}

//----------------------------------------------------------------------------
mafOp* lhpOpMultiReparentTo::Copy()   
//----------------------------------------------------------------------------
{
	lhpOpMultiReparentTo *cp = new lhpOpMultiReparentTo(m_Label);
	cp->m_TargetVme = m_TargetVme;
	return cp;
}

//----------------------------------------------------------------------------
void lhpOpMultiReparentTo::OpRun()   
//----------------------------------------------------------------------------
{
	if (m_InputVmes.size() == 0)
	{
		mafString str = "Select VMEs to reparent";

		mafEvent e(this,VME_CHOOSE);
		e.SetString(&str);
		e.SetBool(true);	//multiselect
		mafEventMacro(e);

		std::vector< mafNode* > nd = e.GetVmeVector();		
		
		m_InputVmes.clear();
		int nCount = (int)nd.size();
		for (int i = 0; i < nCount; i++)
		{
			mafVME* inp = mafVME::SafeDownCast(nd[i]);
			if (inp != NULL)
				m_InputVmes.push_back(inp);
		}
	}
	
	if (m_TargetVme == NULL)
	{
		mafString str = "Select VME where to reparent";

		mafEvent e(this,VME_CHOOSE);
		e.SetString(&str);
		mafEventMacro(e);
		m_TargetVme = mafVME::SafeDownCast(e.GetVme());
	}

	int result = OP_RUN_CANCEL;
	if (m_InputVmes.size() != 0 && m_TargetVme != NULL)
	{	
		result = OP_RUN_OK;	
	}

	mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
// Sets a new input VME.
// If vme is NULL, it removes VME at the given index 
void lhpOpMultiReparentTo::SetInputVME(mafVME* vme, int index)
//----------------------------------------------------------------------------
{
	if (index < 0)
		index = 0;

	int nCount = (int)m_InputVmes.size();	
	if (index >= nCount)
	{
		//add a new one
		if (vme != NULL)
			m_InputVmes.push_back(vme);
	}
	else
	{
		if (vme != NULL)
			m_InputVmes[index] = vme;
		else
			m_InputVmes.erase(m_InputVmes.begin() + index);
	}
}

//----------------------------------------------------------------------------
void lhpOpMultiReparentTo::SetTargetVme(mafVME *target)
//----------------------------------------------------------------------------
{
	m_TargetVme = target;

	if((m_TargetVme == NULL) || !m_Input->CanReparentTo(m_TargetVme))
	{
		mafMessage(_("Cannot re-parent to specified node"),_("Error"),wxICON_ERROR);
		mafEventMacro(mafEvent(this,OP_RUN_CANCEL));
	}
}

//----------------------------------------------------------------------------
void lhpOpMultiReparentTo::OpDo()
//----------------------------------------------------------------------------
{
	//store the previous parent
	mafNode* parent = m_Input->GetParent();

	//create mafOpReparentTo
	mafOpReparentTo* op = new mafOpReparentTo();		
	
	int nCount = (int)m_InputVmes.size();
	for (int i = 0; i < nCount; i++)
	{
		op->SetInput(m_InputVmes[i]);
		op->SetTargetVme(m_TargetVme);	//N.B. must be called after SetInput!

		if (m_InputVmes[i]->CanReparentTo(m_TargetVme))		
			op->OpDo();	//reparent it		
	}
	
	delete op;
}

#endif