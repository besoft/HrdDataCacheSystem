/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCopyMusculoskeletalModel.cpp,v $
  Language:  C++
  Date:      $Date: 2012-01-24 14:28:06 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011-2012
  University of West Bohemia
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpCopyMusculoskeletalModel.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "medVMEMusculoSkeletalModel.h"
#include "medVMEMuscleWrapper.h"
#include "mafVMEShortcut.h"
#include "medOpRegisterClusters.h"
#include "mafMatrixVector.h"
#include "mafVMEMeter.h"
#include "mafTagArray.h"

#include <wx/busyinfo.h>

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpCopyMusculoskeletalModel);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpCopyMusculoskeletalModel::lhpOpCopyMusculoskeletalModel(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
	m_bFilteringEnabled = false;
	m_Canundo = false;  	
}
//----------------------------------------------------------------------------
lhpOpCopyMusculoskeletalModel::~lhpOpCopyMusculoskeletalModel( ) 
//----------------------------------------------------------------------------
{
  
}

//----------------------------------------------------------------------------
mafOp* lhpOpCopyMusculoskeletalModel::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpCopyMusculoskeletalModel(m_Label);
}

//----------------------------------------------------------------------------
bool lhpOpCopyMusculoskeletalModel::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(medVMEMusculoSkeletalModel));
}

//----------------------------------------------------------------------------
void lhpOpCopyMusculoskeletalModel::OpRun()   
//----------------------------------------------------------------------------
{ 		
	//get the confirmation
	if (wxMessageBox(_("This operation clones the musculoskeletal model. Do you want to continue?"),
		wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) != wxYES)
	{
		mafEventMacro(mafEvent(this, OP_RUN_CANCEL));
	}

  mafEventMacro(mafEvent(this, OP_RUN_OK));
}


//------------------------------------------------------------------------
//Enables or disables filtering of VMEs that are irrelevant to motion data and, therefore,
//its presence in Current-Pose atlas is meaningless, e.g., muscles, muscle geometry, ...
//to make it run, you need to call OpDo
/*virtual*/ void lhpOpCopyMusculoskeletalModel::EnableFiltering(bool bEnabled)
//------------------------------------------------------------------------
{	
	m_bFilteringEnabled = bEnabled;
}


#include "vtkSmartPointer.h"
//----------------------------------------------------------------------------
void lhpOpCopyMusculoskeletalModel::OpDo()
//----------------------------------------------------------------------------
{	
	medVMEMusculoSkeletalModel* vmeModel = medVMEMusculoSkeletalModel::SafeDownCast(m_Input);
	
	//check input VMEs
	if (vmeModel == NULL){
		mafMessage("Unable to proceed. Some of inputs is invalid");
		return;
	}	
	
	wxBusyInfo*  wait = (m_TestMode == 0) ?
	  new wxBusyInfo(_("Please wait, working...")) : NULL;

	//first, deep copy everything storing pairs Source ID, New ID into a lookup table	
	medVMEMusculoSkeletalModel* vmeModel2 = 
		medVMEMusculoSkeletalModel::SafeDownCast(
		CopyTree(vmeModel, vmeModel->GetParent())
		);		
	
	mafString name = vmeModel2->GetName();
	name.Append(" (Copy)");	
	vmeModel2->SetName(name);	

	if (wait != NULL)
		delete wait;
}

//----------------------------------------------------------------------------
//Creates a copy of the whole subtree identified by node and places it as a child of parent.
/*virtual*/ mafNode* lhpOpCopyMusculoskeletalModel::CopyTree(mafNode* node, mafNode* parent)
//----------------------------------------------------------------------------
{
	//create MSMGraph for the given tree
	medMSMGraph graph;
	graph.BuildGraph(node);

	lhpVMEIds hash;	
	const medMSMGraph::MSMGraphNode* msm_root = graph.GetRoot();	
	mafNode* root = CopyTreeR(msm_root, parent, hash);
	
	//everything is copied now, update VMEs
	UpdateLinks(root, hash);
	return root;	
}

//----------------------------------------------------------------------------
//Recursive method called from CopyTree that creates a copy of VMEs described by the subtree msm_node
//and places the node into parent and returns the instance of created subtree or NULL, if some constraints
//disallow its construction. In mapIds are returned Ids of VMEs so the links could be updated after CopyTree.
//BEWARE: if a node (e.g., with ID=2198) is linked copied (to a shortcut VME with ID= 2789 referring to ID 2198),
//mapIDs will contain a pair (2198, 2789) and, if links are update without taking care
//we will end up with a shortcut referring to itself
mafNode* lhpOpCopyMusculoskeletalModel::CopyTreeR(const medMSMGraph::MSMGraphNode* msm_node, mafNode* parent, lhpVMEIds& mapIds)
{
	bool bDeepCopy = CanBeDeepCopied(msm_node);
	bool bLinkCopy = !bDeepCopy && CanBeLinkCopied(msm_node);
	
	if (bLinkCopy && msm_node->m_NodeDescriptor.m_Flags & MSMGraphNodeDescriptor::LinkNode)
		bDeepCopy = true;	//linking already linked node is nonsence, so perform a deep copy

	if (!bLinkCopy && !bDeepCopy)
		return NULL;	//we will not process this anymore, skip the node

	mafNode* copy = NULL;
	if (bDeepCopy) 
	{
		//if (mafVMEMeter::SafeDownCast(msm_node->m_Vme) != NULL)
		//	_RPT0(_CRT_WARN, "KUK");

		copy = msm_node->m_Vme->MakeCopy();
		copy->Register(this);			//prevents deleting 
	}
	else
	{
		mafVMEShortcut* vmeShortcut;
		mafNEW(vmeShortcut);

		vmeShortcut->SetTargetVME(mafVME::SafeDownCast(msm_node->m_Vme));
		vmeShortcut->SetName(msm_node->m_Vme->GetName());

		//now copy tags
		mafTagArray* src_tags = msm_node->m_Vme->GetTagArray();
		mafTagArray* dst_tags = vmeShortcut->GetTagArray();

		dst_tags->DeepCopy(src_tags);

		copy = vmeShortcut;
	}

	//put the created copy it into the parent		
	parent->AddChild(copy);	//this deletes unregistered copy
	copy->UnRegister(this);		//now, it should be OK to unregister it	
		
	//if not skipped, store IDs	
	mapIds[msm_node->m_Vme->GetId()] = copy->GetId();

	//and let us process children	
	for (medMSMGraph::MSMGraphNode::MSMGraphChildren::const_iterator it = msm_node->m_Children.begin();
		it != msm_node->m_Children.end(); it++){		
		CopyTreeR(*it, copy, mapIds);	
	}	

	return copy;
}

//----------------------------------------------------------------------------
// Update links of VMEs present in subtree identified by node in such a manner that
//1) if a linked VME was also copied, the link is updated to refer to the copy of linked VME
//2) if the VME is medVMEMuscleWrapper, current pose links are processed as 1), rest pose untouched. 
//Original IDs and IDs of their copies are given in hash. 
/*virtual*/ void lhpOpCopyMusculoskeletalModel::UpdateLinks(mafNode* node, const lhpVMEIds& hash)
{
	std::stack< mafNode* > stack;
	stack.push(node);

	while (!stack.empty())
	{
		mafNode* source = stack.top();
		stack.pop();

		//check, if the source is shortcut		
		if (NULL != mafVMEShortcut::SafeDownCast(source))
			continue;	//we cannot continue, it would lead 

		//check, if source is not medVMEMuscleWrapper
		medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(source);		
		
		//copy links 
		mafNode::mafLinksMap* links = source->GetLinks();
		for (mafNode::mafLinksMap::iterator it = links->begin(); it != links->end(); it++)
		{
			lhpVMEIds::const_iterator found = hash.find(it->second.m_NodeId);
			if (found != hash.end())
			{
				//we have found it => update the information
				if (wrapper != NULL)
				{
					//OK, check, if this should be updated
					if (it->first.StartsWith(medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[medVMEMuscleWrapper::LNK_RESTPOSE_MUSCLE]) ||
						it->first.StartsWith(medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[medVMEMuscleWrapper::LNK_RESTPOSE_REFSYSx]) ||
						it->first.StartsWith(medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[medVMEMuscleWrapper::LNK_RESTPOSE_WRAPPERx])// ||
						//it->first.StartsWith(medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[medVMEMuscleWrapper::LNK_FIBERS_ORIGIN]) ||			//BES: 24.1.2012 - origin and insertion are relative to bones => must be updated
						//it->first.StartsWith(medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[medVMEMuscleWrapper::LNK_FIBERS_INSERTION])
						)
						continue;	
				}

				//the link must be updated												
				source->SetLink(it->first, node->FindInTreeById(found->second), it->second.m_NodeSubId);
			}
		}		

		if (wrapper != NULL)	//make sure the cached data is reloaded
			wrapper->RestoreMeterLinks();

		//process children
		const mafNode::mafChildrenVector* children = source->GetChildren();
		int count = (int)children->size();

		for (int i = 0; i < count; i++) {			
			stack.push(children->at(i));
		}
	}
}