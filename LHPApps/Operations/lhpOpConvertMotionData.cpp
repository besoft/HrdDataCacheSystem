/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpConvertMotionData.cpp,v $
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

#include "lhpOpConvertMotionData.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "medVMEMusculoSkeletalModel.h"
#include "medVMEMuscleWrapper.h"
#include "medVMEJoint.h"
#include "medOpRegisterClusters.h"
#include "mafMatrixVector.h"
#include "mafTagArray.h"
#include "vtkMAFSmartPointer.h"
#include "vtkMath.h"

#include <wx/busyinfo.h>

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpConvertMotionData);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpConvertMotionData::lhpOpConvertMotionData(const wxString &label) : lhpOpCopyMusculoskeletalModel(label)
//----------------------------------------------------------------------------
{
	m_MusculoskeletalModelVME = NULL;
	m_bFilteringEnabled = true;

	m_bEstimateJoints = true;
	m_bPreserveEstimatedJoints = false;
	m_RegistrationMode = medOpRegisterClusters::SIMILARITY;

	m_JointsWeight = 5;	
}
//----------------------------------------------------------------------------
lhpOpConvertMotionData::~lhpOpConvertMotionData( )
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
mafOp* lhpOpConvertMotionData::Copy()
//----------------------------------------------------------------------------
{
	return new lhpOpConvertMotionData(m_Label);
}

//----------------------------------------------------------------------------
bool lhpOpConvertMotionData::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return AcceptLandmarkClouds(node);
}

//----------------------------------------------------------------------------
/*static*/ bool lhpOpConvertMotionData::AcceptMusculoskeletalAtlas(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(medVMEMusculoSkeletalModel));
}

//----------------------------------------------------------------------------
/*static*/ bool lhpOpConvertMotionData::AcceptLandmarkClouds(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(mafVMELandmarkCloud));
}

//----------------------------------------------------------------------------
/*Sets a new MotionData VME
NB. you need to call OpDo to confirm the changes. 
/*virtual*/ void lhpOpConvertMotionData::SetMotionDataVME(mafVME* vme) {	
	SetInput(m_MotionDataVME = vme);
}

//----------------------------------------------------------------------------
//Returns true, if the VME represented by the given msm_node should be DeepCopied
/*virtual*/ bool lhpOpConvertMotionData::CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node)
	//----------------------------------------------------------------------------
{
	// the filtering policy is:
	// deep copy all nodes whose descendant is a bone (regions, groups), or its ancestor is a bone (landmarks, attachment areas, ...) or nodes that are bones
	// and similarly deep copy all nodes related to wrappers, muscle wrappers and joints
	MSMGraphNodeDescriptor::DescFlags thisFlags = (MSMGraphNodeDescriptor::DescFlags)(
		MSMGraphNodeDescriptor::RegionNode | MSMGraphNodeDescriptor::BodyLandmarkCloudNode | MSMGraphNodeDescriptor::JointNode);

	if ((msm_node->m_NodeDescriptor.m_Flags & thisFlags) != 0 ||
			(msm_node->m_DescendantNodesDescriptor.m_Flags & thisFlags) != 0)
			return true;	//copy node, if either it is of the requested group, or some of its children is our target

	return false;
}

//----------------------------------------------------------------------------
void lhpOpConvertMotionData::OpRun()
//----------------------------------------------------------------------------
{
	//get the motion data
	mafString str = "Select Musculoskeletal atlas VME with the predefined joints";

	mafEvent e(this,VME_CHOOSE);
	e.SetString(&str);
	e.SetArg((long)&lhpOpConvertMotionData::AcceptMusculoskeletalAtlas);
	mafEventMacro(e);

	m_bEstimateJoints = wxMessageBox(_("Do you want to estimate joints centres?"),
			wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES;

	if (m_bEstimateJoints) {
		m_bPreserveEstimatedJoints = wxMessageBox(_("Do you want to preserve the estimated joints centres?"),
				wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES;
	}

	m_MusculoskeletalModelVME = medVMEMusculoSkeletalModel::SafeDownCast(e.GetVme());
  mafEventMacro(mafEvent(this, m_MusculoskeletalModelVME != NULL ? OP_RUN_OK : OP_RUN_CANCEL));
}

//----------------------------------------------------------------------------
void lhpOpConvertMotionData::OpDo()
//----------------------------------------------------------------------------
{
	mafVMELandmarkCloud* vmeMotion = mafVMELandmarkCloud::SafeDownCast(m_MotionDataVME);
	medVMEMusculoSkeletalModel* vmeModel = medVMEMusculoSkeletalModel::SafeDownCast(m_MusculoskeletalModelVME);

	//check input VMEs
	if (vmeModel == NULL || vmeMotion == NULL){
		mafLogMessage("Unable to continue. Some of inputs is invalid");
		return;
	}

	wxBusyInfo*  wait = (m_TestMode == 0) ?
	  new wxBusyInfo(_("Please wait, working...")) : NULL;	
	
	//now, deep copy the relevant parts of the musculoskeletal model
	medVMEMusculoSkeletalModel* vmeModel2 =
		medVMEMusculoSkeletalModel::SafeDownCast(
		CopyTree(vmeModel, vmeModel->GetParent())
		);

	FuseMotion(vmeModel2, vmeMotion);

	mafString name = vmeMotion->GetName();
	name.Append(" [JOINTS] ");
	vmeModel2->SetName(name);

	if (wait != NULL)
		delete wait;
}

//----------------------------------------------------------------------------
//Fuses the specified region with the motion data
/*virtual*/ void lhpOpConvertMotionData::FuseMotion(mafVMELandmarkCloud* vmeSource, mafVMELandmarkCloud* vmeMotion, mafVMESurface* vmeTarget)
{
	const JNT_INFO* pJntInfo = GetJointsDesc();

	medOpRegisterClusters opReg;
	opReg.SetSource(vmeSource);
	opReg.SetTarget(vmeMotion);		
	opReg.SetFollower(vmeTarget);

	opReg.SetRegistrationMode(m_RegistrationMode);
	opReg.SetMultiTime(1);

	//nan values in motion capture data denoting that the marker was occluded are converted to 0,0,0 during the import
	//so we should ignore these markers (0,0,0) is default value
	opReg.SetFilteringMode(medOpRegisterClusters::Invisible |
		medOpRegisterClusters::InfiniteOrNaN | medOpRegisterClusters::WithUserValue);

	//Charite motion capture data has different labeling
	const static char* AM[] = {
		"LCAB", "LHEEL",
		"LFM", "LMET01",
		"LGT", "LGTR",
		"LICT", "LPELV01",
		"LKC", "LSHA03",
		"LLE", "LLCO",
		"LLM", "LLMA",
		"LME", "LMCO",
		"LMM", "LMMA",
		"LPM", "LTOE",
		"LTT", "LTUTI",
		"LVM", "LMET05",
		"RCAB", "RHEEL",
		"RFM", "RMET01",
		"RGT", "RGTR",
		"RICT", "RPELV01",
		"RKC", "RSHA03",
		"RLE", "RLCO",
		"RLM", "RLMA",
		"RME", "RMCO",
		"RMM", "RMMA",
		"RPM", "RTOE",
		"RTT", "RTUTI",
		"RVM", "RMET05",
		NULL, NULL,
	};

	const char** pPtr = AM;
	while (*pPtr != NULL)
	{
		opReg.AddAlternativeMatching(pPtr[0], pPtr[1]);
		pPtr += 2;
	}

	//we should add also weights for joints
	if (m_bEstimateJoints)
	{
		int iJntPos = 0;
		while (pJntInfo[iJntPos].JointType >= 0)
		{
			opReg.SetSourceWeight(pJntInfo[iJntPos].JointTypeName, m_JointsWeight);	//all other landmarks have weight 1
			iJntPos++;
		}
	}

	opReg.OpDo();

	//now, we have to extract the results and copy transformations to our follower
	bool bFound = false;
	mafVMEGroup* group = opReg.GetResult();
	const mafNode::mafChildrenVector* children = group->GetChildren();
	for (mafNode::mafChildrenVector::const_iterator it2 = children->begin();
		it2 != children->end(); it2++)
	{
		mafVMESurface* vmeSource = mafVMESurface::SafeDownCast((*it2));
		if (vmeSource != NULL)
		{
			//we have here our VME => copy its timestamps and matrices to our vmeTarget
			mafMatrixVector* mvector = vmeSource->GetMatrixVector();
			vmeTarget->GetMatrixVector()->DeepCopy(mvector);
			//vmeTarget->DeepCopy(vmeSource);

			bFound = true; break;	//we are ready
		}
	}

	//group should be automatically destroyed when dtor of lhpOpRegisterClustersRefactor is called
	//since opReg has no listener, the results of the operation are not reparent to the VME tree
}

//----------------------------------------------------------------------------
//Fuses the specified atlas with the given motion. 
/*virtual*/ void lhpOpConvertMotionData::FuseMotion(medVMEMusculoSkeletalModel* vmeModel2, mafVMELandmarkCloud* vmeMotion)
//----------------------------------------------------------------------------
{	
	//estimates joints position and adds them into the motion data
	if (m_bEstimateJoints) {
		AddEstimatedJoints(vmeMotion);
	}

	//create MSMGraph for the new musculoskeletal model
	medMSMGraph graph;
	graph.BuildGraph(vmeModel2);

	//get all joints we have
	medMSMGraph::MSMGraphNodeList listJoints;
	graph.GetJoints(listJoints, true);

	//get all regions we have
	medMSMGraph::MSMGraphNodeList listBodyLandmarks;
	medMSMGraph::MSMGraphNodeList listRegions;
	graph.GetRegions(listRegions);

	//for each region, we have to find its motion (BodyLandmarks) and add joints into the landmark cloud 
	if (m_bEstimateJoints)
	{
		for (medMSMGraph::MSMGraphNodeList::const_iterator it = listRegions.begin(); it != listRegions.end(); it++)
		{
			//get the body landmark
			graph.GetBodyLandmarks(listBodyLandmarks, false, *it);
			
			//SO, we have in listBodyLandmarks our Source (m_Input for lhpOpRegisterClustersRefactor)
			//in m_MotionDataVME our Target and in *it our Follower
			if (listBodyLandmarks.size() != 1) {
				continue;	//skip this (without any warning, this will be done later
			}

			//add joint centroid for the current body landmarks
			mafVMELandmarkCloud* source = mafVMELandmarkCloud::SafeDownCast(listBodyLandmarks[0]->m_Vme);			
			AddJointsForRegion((*it)->m_NodeDescriptor.m_RegionInfo, listJoints, source);			
		}
	}

	//for each region, we have to find its motion (BodyLandmarks) and
	for (medMSMGraph::MSMGraphNodeList::const_iterator it = listRegions.begin(); it != listRegions.end(); it++)
	{
		//get the body landmark
		graph.GetBodyLandmarks(listBodyLandmarks, false, *it);

		//SO, we have in listBodyLandmarks our Source (m_Input for lhpOpRegisterClustersRefactor)
		//in m_MotionDataVME our Target and in *it our Follower
		if (listBodyLandmarks.size() != 1) {
			mafMessage("ERROR: Region '%s' does not have JUST ONE BodyLandmarks", (*it)->m_Vme->GetName());
			continue;	//try another region
		}

		//add joint centroid for the current body landmarks
		mafVMELandmarkCloud* source = mafVMELandmarkCloud::SafeDownCast(listBodyLandmarks[0]->m_Vme);
		mafVMESurface* vmeTarget = mafVMESurface::SafeDownCast((*it)->m_Vme);
				
		FuseMotion(source, vmeMotion, vmeTarget);
	}
		
	//removes source joints
	if (m_bEstimateJoints) 
	{
		for (medMSMGraph::MSMGraphNodeList::const_iterator it = listRegions.begin(); it != listRegions.end(); it++)
		{
			//get the body landmark
			graph.GetBodyLandmarks(listBodyLandmarks, false, *it);

			//SO, we have in listBodyLandmarks our Source (m_Input for lhpOpRegisterClustersRefactor)
			//in m_MotionDataVME our Target and in *it our Follower
			if (listBodyLandmarks.size() != 1) {
				continue;	//skip this (without any warning, this has been already done
			}

			//add joint centroid for the current body landmarks
			mafVMELandmarkCloud* source = mafVMELandmarkCloud::SafeDownCast(listBodyLandmarks[0]->m_Vme);			
			RemoveJointsForRegion((*it)->m_NodeDescriptor.m_RegionInfo, source);	
		}

		//remove the estimated joint positions (no longer required).
		if (!m_bPreserveEstimatedJoints) {
			RemoveEstimatedJoints(vmeMotion);
		}
	}	
}

#pragma region Joint Centres Estimation
//----------------------------------------------------------------------------
//Adds centres of all joints (from listJoint) connecting the given region with others as landmarks into source landmark cloud.
//	Call RemoveJointsForRegion method to remove these landmarks.
/*virtual*/ void lhpOpConvertMotionData::AddJointsForRegion(int region, medMSMGraph::MSMGraphNodeList& listJoints, mafVMELandmarkCloud* source)
	//----------------------------------------------------------------------------
{
	//the structure is: region Id, number of joints, jont type 1, ...
	const static int g_RegJnt[] = {
		medMSMGraph::RegionEnum::Pelvis, 3, medMSMGraph::JointEnum::CoccoSacral, medMSMGraph::JointEnum::RightCoxoFemoral, medMSMGraph::JointEnum::LeftCoxoFemoral,
		medMSMGraph::RegionEnum::RightThigh, 2, medMSMGraph::JointEnum::RightCoxoFemoral, medMSMGraph::JointEnum::RightFemoroTibial,
		medMSMGraph::RegionEnum::RightShank, 2, medMSMGraph::JointEnum::RightFemoroTibial, medMSMGraph::JointEnum::RightSubTalar,
		medMSMGraph::RegionEnum::RightFoot, 2, medMSMGraph::JointEnum::RightSubTalar, medMSMGraph::JointEnum::RightFootRay1Interphalangeal,
		medMSMGraph::RegionEnum::LeftThigh, 2, medMSMGraph::JointEnum::LeftCoxoFemoral, medMSMGraph::JointEnum::LeftFemoroTibial,
		medMSMGraph::RegionEnum::LeftShank, 2, medMSMGraph::JointEnum::LeftFemoroTibial, medMSMGraph::JointEnum::LeftSubTalar,
		medMSMGraph::RegionEnum::LeftFoot, 2, medMSMGraph::JointEnum::LeftSubTalar, medMSMGraph::JointEnum::LeftFootRay1Interphalangeal,
		-1, 0
	};

		const JNT_INFO* jnts = GetJointsDesc();

		int iRegPos = 0;
		while (g_RegJnt[iRegPos] >= 0)
		{
			if (g_RegJnt[iRegPos] == region)
			{
				//get joints
				for (int i = 0; i < g_RegJnt[iRegPos + 1]; i++)
				{
					int jnt_type = g_RegJnt[iRegPos + 2 + i];	//get joint type

					//find the joint in the list of joints
					for (medMSMGraph::MSMGraphNodeList::const_iterator it = listJoints.begin();
						it != listJoints.end(); it++)
					{
						if ((*it)->m_NodeDescriptor.m_JointInfo != jnt_type) {
							continue;	//no match
						}

						double jnt_pos[3];
						medVMEJoint* jnt = medVMEJoint::SafeDownCast((*it)->m_Vme);
						if (jnt == NULL || !jnt->GetJointAbsPosition(jnt_pos)){
							continue;	//invalid joint
						}

						//find the joint in our internal structure
						int iJntPos = 0;
						while (jnts[iJntPos].JointType >= 0)
						{
							if (jnts[iJntPos].JointType == jnt_type)
							{
								int lmidx = source->AppendLandmark(jnts[iJntPos].JointTypeName);
								if (lmidx < 0) {	//if the landmark already exists, get its index
									lmidx = source->FindLandmarkIndex(jnts[iJntPos].JointTypeName);
								}

								source->SetLandmark(lmidx, jnt_pos[0], jnt_pos[1], jnt_pos[2]);
								break;	//this joint is processed
							}

							iJntPos++;
						} //end while [name of joints]

						break;	//joint was found in the list
					} //end for joints in list
				}	//end for joints in descriptor

				break;	//we are ready
			} //end if region

			iRegPos += g_RegJnt[iRegPos + 1] + 2;	//skip the region description
		}
}

//----------------------------------------------------------------------------
//Removes landmarks created by AddJointsForRegion method.
/*virtual*/ void lhpOpConvertMotionData::RemoveJointsForRegion(int region, mafVMELandmarkCloud* source)
	//----------------------------------------------------------------------------
{
	//remove all joints (names are the same)
	RemoveEstimatedJoints(source);
}

//----------------------------------------------------------------------------
//Calculates the average distance (through all time frames) between a marker with ID m and a centroid.
//Coordinates of the centroid in every frame are given in c, the coordinates of marker are in pInfo.
/*static*/ double lhpOpConvertMotionData::CalcDCM(OPT_INFO* pInfo, const VCoord* c, int m)
	//----------------------------------------------------------------------------
{
	int index = m;

	int count = 0;
	double sum = 0.0;
	for (int f = 0; f < pInfo->numFrames; f++)
	{
		if (pInfo->validity[index]) {
			sum += sqrt(vtkMath::Distance2BetweenPoints(pInfo->markers[index], c[f]));
			count++;
		}

		index += pInfo->numMarkers;
	}

	return count != 0 ? sum / count : 0.0;
}

//----------------------------------------------------------------------------
//Calculates the partial derivative of CalcDCM function for c[i][j] variable of the centroid.
//Parameters i and j denote frame number and coordinate dimension (0-2), respectively.
/*static*/ double lhpOpConvertMotionData::CalcDCM_PD(OPT_INFO* pInfo, const VCoord* c, int m, int i, int j)
	//----------------------------------------------------------------------------
{
	int index = m;

	int count = 0;
	double sum = 0.0;
	for (int f = 0; f < pInfo->numFrames; f++)
	{
		if (pInfo->validity[index])
		{
			if (f == i)		//for f <> i, derivative is always zero
			{
				sum += (c[f][j] - pInfo->markers[index][j])	/
					sqrt(vtkMath::Distance2BetweenPoints(pInfo->markers[index], c[f]));
			}
			count++;
		}

		index += pInfo->numMarkers;
	}

	return count != 0 ? sum / count : 0.0;
}

//----------------------------------------------------------------------------
//Calculates the variance in distance calculated by CalcDCM method.
//	Coordinates of the centroid in every frame are given in c, the coordinates of marker are in pInfo.
//	Calculated average distance is passes in d_cm parameter.
/*static*/ double lhpOpConvertMotionData::CalcSigmaCM(OPT_INFO* pInfo, const VCoord* c, int m, double d_cm)
	//----------------------------------------------------------------------------
{
	int index = m;

	int count = 0;
	double sum = 0.0;
	for (int f = 0; f < pInfo->numFrames; f++)
	{
		if (pInfo->validity[index])
		{
			double dist = sqrt(vtkMath::Distance2BetweenPoints(pInfo->markers[index], c[f]));
			sum += (dist - d_cm)*(dist - d_cm);
			count++;
		}

		index += pInfo->numMarkers;
	}

	return count != 0 ? sum / count : 0.0;
}

//----------------------------------------------------------------------------
//Calculates the partial derivative of CalcSigmaCM function for c[i][j] variable of the centroid.
//	Parameters i and j denote frame number and coordinate dimension (0-2), respectively.
//	Partial derivative of CalcDCM function for c[i][j] is given in d_cm_PD.
/*static*/ double lhpOpConvertMotionData::CalcSigmaCM_PD(OPT_INFO* pInfo, const VCoord* c, int m, double d_cm, double d_cm_PD, int i, int j)
	//----------------------------------------------------------------------------
{
	int index = m;

	int count = 0;
	double sum = 0.0;
	for (int f = 0; f < pInfo->numFrames; f++)
	{
		if (pInfo->validity[index])
		{
			double dist = sqrt(vtkMath::Distance2BetweenPoints(pInfo->markers[index], c[f]));
			if (f != i) {
				sum += -d_cm_PD*(dist - d_cm);
			}
			else
			{
				sum += (dist - d_cm)*(
						((c[f][j] - pInfo->markers[index][j]) / dist) - d_cm_PD
					);
			}

			count++;
		}

		index += pInfo->numMarkers;
	}

	return count != 0 ? 2*sum / count : 0.0;
}

//----------------------------------------------------------------------------
//Calculates joint cost function and its gradient.
//This is a callback function called from alglib non-linear optimization solver.
/*static*/ void lhpOpConvertMotionData::JointCostFunction(const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad, void *ptr)
{
	const static double alfa = 0.125;

	OPT_INFO* pInfo = (OPT_INFO*)ptr;

	VCoord* c = (VCoord*)x.getcontent();	//coordinates of joint for each frame

	//1) calculate the function value
	func = 0.0;

	int nValidMarkers = 0;
	for (int m = 0; m < pInfo->numMarkers; m++)
	{
		if (pInfo->validframes[m] == 0) {
			continue;	//this marker must be ignored
		}

		double d_cm = CalcDCM(pInfo, c, m);
		double sigma_cm = CalcSigmaCM(pInfo, c, m, d_cm);

		func += sigma_cm + alfa*d_cm;
		nValidMarkers++;
	}

	func /= nValidMarkers;

	//2) calculate the gradient
	double* grad_Q =  grad.getcontent();

	int gradPos = 0;
	for (int i = 0; i < pInfo->numFrames; i++)
	{
		for (int j = 0; j < 3; j++, gradPos++)
		{
			//calculate partial derivative for C_i,j where i is the frame, j is coordinate in the frame
			int nValidMarkers = 0;
			grad_Q[gradPos] = 0.0;

			for (int m = 0; m < pInfo->numMarkers; m++)
			{
				if (pInfo->validframes[m] == 0) {
					continue;	//this marker must be ignored
				}

				double d_cm = CalcDCM(pInfo, c, m);
				double d_cm_PD = CalcDCM_PD(pInfo, c, m, i, j);

				double sigma_cm_PD = CalcSigmaCM_PD(pInfo, c, m, d_cm, d_cm_PD, i, j);

				grad_Q[gradPos] += sigma_cm_PD + alfa*d_cm_PD;
				nValidMarkers++;
			}

			grad_Q[gradPos] /= nValidMarkers;
		}
	}
}

//----------------------------------------------------------------------------
//Estimates joint positions (at each frame) for the given landmark cloud and adds these positions into the cloud.
//Call RemoveEstimatedJoints method to remove these positions.
/*virtual*/ void lhpOpConvertMotionData::AddEstimatedJoints(mafVMELandmarkCloud* target)
	//----------------------------------------------------------------------------
{
	std::vector<mafTimeStamp> timeStamps;
	target->GetTimeStamps(timeStamps);
	int numTimeStamps = target->GetNumberOfTimeStamps();

#ifdef _DEBUG
	numTimeStamps = 10;	//limit
#endif

	//get joints description
	const JNT_INFO* jnts = GetJointsDesc();

	OPT_INFO info;
	memset(&info, 0, sizeof(info));
	info.numFrames = numTimeStamps;
	info.solution = new VCoord[info.numFrames];
	memset(info.solution, 0, sizeof(VCoord)*info.numFrames);

	int iJointPos = 0;
	while (jnts[iJointPos].JointType >= 0)
	{
		//calculate number of markers defining the joint
		info.numMarkers = 0;
		const char** pNames = jnts[iJointPos].Landmarks;
		while (*pNames != NULL) {
			info.numMarkers++; pNames++;
		}

		info.validframes = new int[info.numMarkers];
		memset(info.validframes, 0, info.numMarkers*sizeof(int));

		info.validity = new bool[info.numMarkers*info.numFrames];
		memset(info.validity, false, info.numMarkers*info.numFrames*sizeof(bool));

		info.markers = new VCoord[info.numMarkers*info.numFrames];

		int* validmarkers = new int[info.numFrames];	//helper array to store number of markers valid for each frame
		memset(validmarkers, 0, info.numFrames*sizeof(int));

		//process all landmarks
		pNames = jnts[iJointPos].Landmarks;
		for (int markerIndex = 0; markerIndex < info.numMarkers; markerIndex++)
		{
			int lmidx = target->FindLandmarkIndex(pNames[markerIndex]);
			if (lmidx < 0) {	//invalid landmark, continue
				continue;
			}

			//get every frame
			for (int iFrame = 0, coordIndex = markerIndex; iFrame < info.numFrames;
				iFrame++, coordIndex += info.numMarkers)
			{
				double currTime = timeStamps[iFrame];
				target->GetLandmarkPosition(lmidx, info.markers[coordIndex], currTime);

				//check, if the coordinates are valid
				bool bIsFinite = true;
				for (int k = 0; k < 3; k++) {
					if (!(info.markers[coordIndex][k] >= -DBL_MAX && info.markers[coordIndex][k] <= DBL_MAX)) {
						bIsFinite = false;
					}
				}

				info.validity[coordIndex] = bIsFinite && (
					(info.markers[coordIndex][0] != 0.0 || info.markers[coordIndex][1] != 0.0 || info.markers[coordIndex][2] != 0.0)
					);

				if (info.validity[coordIndex])
				{
					//add the coordinate to sum, to estimate initial solution
					for (int k = 0; k < 3; k++) {
						info.solution[iFrame][k] += info.markers[coordIndex][k];
					}

					validmarkers[iFrame]++;

					info.validframes[markerIndex]++;
				}
			} //end for [each time frame]
		}//end for [each landmark]

		//calculate initial solution
		for (int iFrame = 0; iFrame < info.numFrames; iFrame++)
		{
			if (validmarkers[iFrame] != 0)
			{
				for (int k = 0; k < 3; k++) {
					info.solution[iFrame][k] /= validmarkers[iFrame];
				}
			}
		} //end for [each frame]

		delete[] validmarkers;	//no longer needed

		//run our non-linear solver (conjugate gradient minimization)
		alglib::real_1d_array x;
		x.setcontent(info.numFrames*3, info.solution[0]);

		alglib::mincgstate state;
		alglib::mincgcreate(x, state);
		alglib::mincgoptimize(state, JointCostFunction, NULL, &info);

		alglib::mincgreport rep;
		alglib::mincgresults(state, x, rep);

		//get solution
		VCoord* sol = (VCoord*)x.getcontent();

		int lmidx = target->AppendLandmark(jnts[iJointPos].JointTypeName);
		if (lmidx < 0) {	//if the landmark already exists, get its index
			lmidx = target->FindLandmarkIndex(jnts[iJointPos].JointTypeName);
		}

		for (int iFrame = 0; iFrame < info.numFrames; iFrame++)
		{
			double currTime = timeStamps[iFrame];
			target->SetLandmark(lmidx, sol[iFrame][0], sol[iFrame][1], sol[iFrame][2], currTime);
		}

		delete[] info.markers;
		delete[] info.validity;
		delete[] info.validframes;

		iJointPos++;
	}
}

//----------------------------------------------------------------------------
//Remove landmarks created by AddEstimatedJoints method.
/*virtual*/ void lhpOpConvertMotionData::RemoveEstimatedJoints(mafVMELandmarkCloud* target)
	//----------------------------------------------------------------------------
{
	//get joints description
	const JNT_INFO* jnts = GetJointsDesc();

	int iJointPos = 0;
	while (jnts[iJointPos].JointType >= 0)
	{
		int lmidx = target->FindLandmarkIndex(jnts[iJointPos].JointTypeName);
		if (lmidx >= 0) {
			target->RemoveLandmark(lmidx);
		}

		iJointPos++;
	}
}

//----------------------------------------------------------------------------
//Gets the array of joints ended by {-1, NULL} entry.
/*static*/ const lhpOpConvertMotionData::JNT_INFO* lhpOpConvertMotionData::GetJointsDesc()
//----------------------------------------------------------------------------
{
	//const static char* PELVIS[] = {"RASIS", "RPELV01", "RPELV02", "RPSIS", "LPSIS", "LPELV02", "LPELV01", "LASIS", NULL};
	//const static char* RIGHTTHIGH[] = {"RGTR", "RTHI01", "RTHI02", "RTHI03", "RTHI04", "RTHI05", "RTHI06", NULL};
	//const static char* LEFTTHIGH[] = {"LGTR", "LTHI01", "LTHI02", "LTHI03", "LTHI04", "LTHI05", "LTHI06", NULL};
	//const static char* RIGHTSHANK[] = {"RMCO", "RLCO", "RCAFI", "RTUTI", "RSHA01", "RSHA02", "RSHA03", "RSHA04", "RSHA05", "RSHA06", NULL};
	//const static char* LEFTSHANK[] = {"LMCO", "LLCO", "LCAFI", "LTUTI", "LSHA01", "LSHA02", "LSHA03", "LSHA04", "LSHA05", "LSHA06", NULL};
	//const static char* RIGHTFOOT[] = {"RMMA", "RLMA", "RMET01", "RTOE", "RMET05", "RHEEL", NULL};
	//const static char* LEFTFOOT[] = {"LMMA", "LLMA", "LMET01", "LTOE", "LMET05", "LHEEL", NULL};

	//const static char** JOINTS[] = { PELVIS, RIGHTTHIGH, RIGHTTHIGH, RIGHTSHANK, RIGHTSHANK, RIGHTFOOT,
	//	PELVIS, LEFTTHIGH, LEFTTHIGH, LEFTSHANK, LEFTSHANK, LEFTFOOT,
	//	NULL, NULL};

	const static char* CoccoSacral[] /*ROOT*/ = {"RASIS", "RPELV01", "RPELV02", "RPSIS", "LPSIS", "LPELV02", "LPELV01", "LASIS", /*Charite names*/
		/*LHDL names*/
		NULL,
	};

	const static char* RightCoxoFemoral[]/*RIGHT_HIP*/ = {/*"RASIS", "RPELV01", "RPELV02", "RPSIS", "RGTR", */
		"RASIS", "RPELV01", "RPELV02", "RPSIS", "LPSIS", "LPELV02", "LPELV01", "LASIS", "RGTR", "RTHI01", "RTHI02", "RTHI03", "RTHI04", "RTHI05", "RTHI06",
		/*Charite names*/
		"RGT", "RHC", /*LHDL names*/
		NULL,
	};

	const static char* LeftCoxoFemoral[]/*LEFT_HIP*/ = {/*"LPSIS", "LPELV02", "LPELV01", "LASIS", "LGTR", */
		"RASIS", "RPELV01", "RPELV02", "RPSIS", "LPSIS", "LPELV02", "LPELV01", "LASIS", "LGTR", "LTHI01", "LTHI02", "LTHI03", "LTHI04", "LTHI05", "LTHI06",
		/*Charite names*/
		"LGT", "LHC", /*LHDL names*/
		NULL,
	};

	const static char* RightFemoroTibial[]/*RIGHT_KNEE*/ = {/*"RTHI04", "RTHI05", "RTHI06", "RMCO", "RLCO", "RTUTTI",*/
		"RGTR", "RTHI01", "RTHI02", "RTHI03", "RTHI04", "RTHI05", "RTHI06", "RMCO", "RLCO", "RCAFI", "RTUTI", "RSHA01", "RSHA02", "RSHA03", "RSHA04", "RSHA05", "RSHA06",		/*Charite names*/
		"RHF", "RTT", "RLE", "RME", "RKC", /*LHDL names*/
		NULL,
	};

	const static char* LeftFemoroTibial[]/*LEFT_KNEE*/ = {/*"LTHI04", "LTHI05", "LTHI06", "LMCO", "LLCO", "LTUTTI", */
		"LGTR", "LTHI01", "LTHI02", "LTHI03", "LTHI04", "LTHI05", "LTHI06", "LMCO", "LLCO", "LCAFI", "LTUTI", "LSHA01", "LSHA02", "LSHA03", "LSHA04", "LSHA05", "LSHA06",		/*Charite names*/
		"LHF", "LTT", "LLE", "LME", "LKC", /*LHDL names*/
		NULL,
	};

	const static char* RightSubTalar[]/*RIGHT_ANKLE*/ = {"RSHA04", "RSHA05", "RSHA06", "RMMA", "RLMA", "RHEEL",
		"RMET01", "RTOE", "RMET05", /*Charite names*/
		"RLM", "RMM", "RAC", "RPT", "RVMB", "RCAB", "RST", "RTN",
		"RFMB", "RSMB",/*LHDL names*/
		NULL,
	};

	const static char* LeftSubTalar[]/*LEFT_ANKLE*/ = {"LSHA04", "LSHA05", "LSHA06", "LMMA", "LLMA", "LHEEL",
		"LMET01", "LTOE", "LMET05", /*Charite names*/
		"LLM", "LMM", "LAC", "LPT", "LVMB", "LCAB", "LST", "LTN",
		"LFMB", "LSMB",/*LHDL names*/
		NULL,
	};

	//const static char* RightFootRay1Interphalangeal[]/*RIGHT_TOE*/ = {"RMET01", "RTOE", "RMET05", /*Charite names*/
	//	"RFMB", "RSMB", /*LHDL names*/
	//	NULL,
	//};

	//const static char* LeftFootRay1Interphalangeal[]/*LEFT_TOE*/ = {"LMET01", "LTOE", "LMET05",
	//	"LFMB", "LSMB", /*LHDL names*/
	//	NULL,
	//};

	const static JNT_INFO g_Joints[] = {
		{ medMSMGraph::JointEnum::CoccoSacral, "Joint_CoccoSacral", CoccoSacral},
		{ medMSMGraph::JointEnum::RightCoxoFemoral, "Joint_RightCoxoFemoral", RightCoxoFemoral},
		{ medMSMGraph::JointEnum::LeftCoxoFemoral, "Joint_LeftCoxoFemoral", LeftCoxoFemoral},
		{ medMSMGraph::JointEnum::RightFemoroTibial, "Joint_RightFemoroTibial", RightFemoroTibial},
		{ medMSMGraph::JointEnum::LeftFemoroTibial, "Joint_LeftFemoroTibial", LeftFemoroTibial},
		{ medMSMGraph::JointEnum::RightSubTalar, "Joint_RightSubTalar", RightSubTalar},
		{ medMSMGraph::JointEnum::LeftSubTalar, "Joint_LeftSubTalar", LeftSubTalar},
		//{ medMSMGraph::JointEnum::RightFootRay1Interphalangeal, "Joint_RightFootRay1Interphalangeal", RightFootRay1Interphalangeal},
		//{ medMSMGraph::JointEnum::LeftFootRay1Interphalangeal, "Joint_LeftFootRay1Interphalangeal", LeftFootRay1Interphalangeal},

		{ -1, NULL, NULL},	//end entry
	};

	return g_Joints;
}
#pragma endregion