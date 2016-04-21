/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpFuseMotionData.cpp,v $
  Language:  C++
  Date:      $Date: 2012-01-24 14:28:06 $
  Version:   $Revision: 1.1.2.4 $
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

#include "lhpOpFuseMotionData.h"

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
mafCxxTypeMacro(lhpOpFuseMotionData);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpFuseMotionData::lhpOpFuseMotionData(const wxString &label) : lhpOpConvertMotionData(label)
//----------------------------------------------------------------------------
{  
	m_MotionDataVME = NULL;
	m_bFilteringEnabled = true;	

	m_RegistrationMode = medOpRegisterClusters::RIGID;
}
//----------------------------------------------------------------------------
lhpOpFuseMotionData::~lhpOpFuseMotionData( ) 
//----------------------------------------------------------------------------
{
  
}

//----------------------------------------------------------------------------
mafOp* lhpOpFuseMotionData::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpFuseMotionData(m_Label);
}

//----------------------------------------------------------------------------
bool lhpOpFuseMotionData::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return AcceptMusculoskeletalAtlas(node);
}

//----------------------------------------------------------------------------
//Sets a new musculoskeletal mode VME
//NB. you need to call OpDo to confirm the changes.
/*virtual*/ void lhpOpFuseMotionData::SetMusculoskeletalModelVME(mafVME* vme) {
	SetInput(m_MusculoskeletalModelVME = vme);
}

//----------------------------------------------------------------------------
void lhpOpFuseMotionData::OpRun()   
//----------------------------------------------------------------------------
{ 		
	//get the motion data
	mafString str = "Select landmark cloud VME with the motion data";

	mafEvent e(this,VME_CHOOSE);
	e.SetString(&str);
	e.SetArg((long)&lhpOpFuseMotionData::AcceptLandmarkClouds);
	mafEventMacro(e);

	m_MotionDataVME = mafVMELandmarkCloud::SafeDownCast(e.GetVme());  
	if (m_MotionDataVME != NULL)
	{
		EnableFiltering(wxMessageBox(
			_("Do you want to automatically filter out VMEs valid only in the rest-pose position (e.g., muscles, ...)?"),
			wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES);

		m_bEstimateJoints = wxMessageBox(_("Do you want to estimate joints centres?"),
			wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES;

		if (m_bEstimateJoints) {
			m_bPreserveEstimatedJoints = wxMessageBox(_("Do you want to preserve the estimated joints centres?"),
				wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES;
		}
	}

  mafEventMacro(mafEvent(this, m_MotionDataVME != NULL ? OP_RUN_OK : OP_RUN_CANCEL));
}

//----------------------------------------------------------------------------
void lhpOpFuseMotionData::OpDo()
//----------------------------------------------------------------------------
{	
	medVMEMusculoSkeletalModel* vmeModel = medVMEMusculoSkeletalModel::SafeDownCast(m_Input);
	mafVMELandmarkCloud* vmeMotion = mafVMELandmarkCloud::SafeDownCast(m_MotionDataVME);	

	//check input VMEs
	if (vmeModel == NULL || vmeMotion == NULL){
		mafMessage("Unable to fuse motion data. Some of inputs is invalid");
		return;
	}	
	
	wxBusyInfo*  wait = (m_TestMode == 0) ?
	  new wxBusyInfo(_("Please wait, working...")) : NULL;

	//first, deep copy everything storing pairs Source ID, New ID into a lookup table	
	medVMEMusculoSkeletalModel* vmeModel2 = 
		medVMEMusculoSkeletalModel::SafeDownCast(
		CopyTree(vmeModel, vmeModel->GetParent())
		);	

	FuseMotion(vmeModel2, vmeMotion);
	
	mafString name = vmeModel2->GetName();
	name.Append(" Fused by ");
	name.Append(vmeMotion->GetName());
	vmeModel2->SetName(name);	

	if (wait != NULL)
		delete wait;
}


//----------------------------------------------------------------------------
//Returns true, if the VME represented by the given msm_node should be DeepCopied
/*virtual*/ bool lhpOpFuseMotionData::CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node)
{	
	//BES 24.1.2012 - mafVMEShortcut does not work as expected in all cases
	//and, therefore, we will deep copy everything
	if (!m_bFilteringEnabled)
		return true;

	// the filtering policy is:
	// deep copy all nodes whose descendant is a bone (regions, groups), or its ancestor is a bone (landmarks, attachment areas, ...) or nodes that are bones
	// and similarly deep copy all nodes related to wrappers, muscle wrappers and joints
	MSMGraphNodeDescriptor::DescFlags thisFlags = (MSMGraphNodeDescriptor::DescFlags)(
		MSMGraphNodeDescriptor::RegionNode | MSMGraphNodeDescriptor::BoneNode | MSMGraphNodeDescriptor::BodyLandmarkCloudNode | 
		MSMGraphNodeDescriptor::JointNode |	MSMGraphNodeDescriptor::WrapperMeterNode | 
		MSMGraphNodeDescriptor::MuscleWrapperNode);
	
	if ((msm_node->m_NodeDescriptor.m_Flags & thisFlags) != 0 ||
			(msm_node->m_DescendantNodesDescriptor.m_Flags & thisFlags) != 0)
			return true;	//copy node, if either it is of the requested group, or some of its children is our target

	if ((msm_node->m_AncestorNodesDescriptor.m_Flags & MSMGraphNodeDescriptor::BoneNode) != 0)
		return true;	//copy parametric surfaces and landmarks associated with bones
		
	return false;
}

//----------------------------------------------------------------------------
//Returns true, if for the VME represented by the given msm_node a shortcut should be created*/
/*virtual*/ bool lhpOpFuseMotionData::CanBeLinkCopied(const medMSMGraph::MSMGraphNode* msm_node)
{		
	//BES 24.1.2012 - nothing is linked copied
	return false;	
}