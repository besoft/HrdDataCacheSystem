/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMSModelCheckTags.cpp,v $
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

#include "lhpOpMSModelCheckTags.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "medVMEMusculoSkeletalModel.h"
#include "mafTagArray.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMSModelCheckTags);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMSModelCheckTags::lhpOpMSModelCheckTags(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = false;  	
}
//----------------------------------------------------------------------------
lhpOpMSModelCheckTags::~lhpOpMSModelCheckTags( ) 
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpMSModelCheckTags::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpMSModelCheckTags(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpMSModelCheckTags::Accept(mafNode *node)
//----------------------------------------------------------------------------
{	
  return (node && node->IsMAFType(medVMEMusculoSkeletalModel));
}
//----------------------------------------------------------------------------
void lhpOpMSModelCheckTags::OpRun()   
//----------------------------------------------------------------------------
{ 	
	if (wxMessageBox(_("This operation checks the VMEs in the selected musculoskeletal model, if they contain correct tags."),
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
void lhpOpMSModelCheckTags::OpDo()
//----------------------------------------------------------------------------
{	
	medVMEMusculoSkeletalModel* model = medVMEMusculoSkeletalModel::SafeDownCast(m_Input);
	if (model != NULL)
	{
		const medMSMGraph* graph = model->GetMSMGraph(true);		
		const medMSMGraph::MSMGraphNode* root = graph->GetRoot();
	
		CheckGraphR(root, 0);	
	}
}

//----------------------------------------------------------------------------
// Main routine to recursively check every node of the atlas. 
void lhpOpMSModelCheckTags::CheckGraphR(const medMSMGraph::MSMGraphNode* root, int indent)
//----------------------------------------------------------------------------
{
	//display information about this node
	wxString szAttr;
	if (root->m_HigherResNode != NULL)
		szAttr.Append("HR,");
	if (root->m_LowerResNode != NULL)
		szAttr.Append("LR,");
	if (root->m_HullNode != NULL)
		szAttr.Append("HN");
	if (root->m_HullUserNode != NULL)
		szAttr.Append("HUN");

	wxString szStr = wxString::Format("#%-5d (%-40s) %-20s TAG<%s> ATTR<%s>",
		(root->m_Vme == NULL ? -1 : root->m_Vme->GetId()),
		(root->m_Vme == NULL ? "<null>" : root->m_Vme->GetName()),
		(root->m_Vme == NULL ? "<null>" : root->m_Vme->GetTypeName()),
		root->m_NodeDescriptor.ToString().GetCStr(),
		szAttr
		);

	mafLogMessage(szStr);
#if defined(_MSC_VER)
	for (int i = 0; i < indent; i++) {
		_RPT0(_CRT_WARN, "\t");
	}

	_RPT0(_CRT_WARN, szStr.c_str());
	_RPT0(_CRT_WARN, "\n");
#endif

	//if we are not the highest resolution, we might have not set tags
	if (root->m_HigherResNode != NULL)
	{
		if ((root->m_NodeDescriptor.m_Flags &
			(
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::RegionNode |
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::BoneNode |
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::JointNode |
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::MuscleNode |
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::LigamentNode |
			medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::BodyLandmarkCloudNode			
			)) == 0)
		{
			mafLogMessage("!!! PROPER TAG MISSING !!! Reconstructed automatically");

			//find the highest resolution and copy tags
			const medMSMGraph::MSMGraphNode* hr = root->m_HigherResNode;
			while (hr->m_HigherResNode != NULL) {
				hr = hr->m_HigherResNode;
			}

			CopyTags(hr->m_Vme, root->m_Vme);
		}
	}

	//and process children (recursively)
	medMSMGraph::MSMGraphNodeList::const_iterator it;
	for (it = root->m_Children.begin(); it != root->m_Children.end(); it++) {
		CheckGraphR(*it, indent + 1);
	}
}

//----------------------------------------------------------------------------
//Copies tags from to to VME 
void lhpOpMSModelCheckTags::CopyTags(mafNode* from, mafNode* to)
//----------------------------------------------------------------------------
{
	to->GetTagArray()->DeepCopy(from->GetTagArray());
}

#endif