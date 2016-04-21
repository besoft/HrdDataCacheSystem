/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpScaleMMA.cpp,v $
Language:  C++
Date:      $Date: 2012-02-08 07:23:07 $
Version:   $Revision: 1.1.2.3 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
// error : S032_R_B000_CTA_FootRay2P3
#include "lhpDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"

#include "lhpOpScaleMMA.h"
//#include "mafMemDbg.h"

#include "mafGUI.h"
#include "mafNodeIterator.h"

#include <vtkPolyData.h>

#include <vtkPointLocator.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkQuadricDecimation.h>
#include <vtkAppendPolyData.h>


#include "wx/busyinfo.h"

#include "vtkMAFSmartPointer.h"
#include "vtkMAFMeanValueCoordinatesInterpolation.h"
#include "medVMEMusculoSkeletalModel.h"
#include "mafTagArray.h"
#include "medMSMGraph.h"
#include "mafTransform.h"
//#include "MSMGraphNodeList.h"

#include <time.h>

#include <algorithm>
#include <limits>
//#include "csparse.h"
#include <vtkCleanPolyData.h>
#include <vtkDoubleArray.h>

mafCxxTypeMacro(lhpOpScaleMMA);


static cs *make_sym ( cs *A );
static int is_sym ( cs *A );


//----------------------------------------------------------------------------
// Constructor
lhpOpScaleMMA::lhpOpScaleMMA(wxString label):lhpOpCopyMusculoskeletalModel(label)
//----------------------------------------------------------------------------
{
	m_InputCopy = NULL;
	m_LandmarkPatient = NULL;

	m_nMuscleRun = 10;
	m_degen = 0;

	m_T = NULL;
	m_pProb = NULL;

	for (int i = 0; i < 2; i++)
	{
		m_pFemur[i] = NULL;
		m_pPatella[i] = NULL;
		m_pAnkle[i] = NULL;
	}
	
	m_LandmarkPatient = NULL;
	//m_OutSurface = NULL;

	// for debugging
	m_LdScale = 1.0;
	m_dupCount = 0;
	m_isolatedCount = 0;
	m_totalPoint = 0;;

}

//----------------------------------------------------------------------------
// Destructor
lhpOpScaleMMA::~lhpOpScaleMMA(void)
//----------------------------------------------------------------------------
{

	for (int i = 0; i < m_interSurfConstraintList.size(); i++)
		delete m_interSurfConstraintList[i];
}

mafOp * lhpOpScaleMMA::Copy()
{
	return new lhpOpScaleMMA();
}

//----------------------------------------------------------------------------
// Accept surface 
bool lhpOpScaleMMA::Accept(mafNode * node)
//----------------------------------------------------------------------------
{
	return (node != NULL && node->IsA("medVMEMusculoSkeletalModel")) ;
}


//----------------------------------------------------------------------------
// Static copy of accept function
bool lhpOpScaleMMA::AcceptStatic(mafNode* node)
//----------------------------------------------------------------------------
{
	return (node != NULL && node->IsA("medVMEMusculoSkeletalModel")) ;
}

/*
bool lhpOpScaleMMA::AcceptSurface(mafNode *node)
{
	return (node != NULL && (node->IsMAFType(mafVMESurface) ));
}
*/
bool lhpOpScaleMMA::AcceptLandmark(mafNode *node)
{
	return (node != NULL && (node->IsMAFType(mafVMELandmarkCloud) ));
}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::OpRun()
//----------------------------------------------------------------------------
{
	PrefetchDataInfo();

	CreateGUI();

	ShowGui();	
}  

// TODO: add matrix support
template<class T>  void lhpOpScaleMMA::LoadSurfaceFromList(medMSMGraph::MSMGraphNodeList & inList, 
	const char * parentName, const char * nodeName, std::vector<T *> & outList)
{
	for (int i = 0;  i < inList.size(); i++)
	{
			T* pSurf = T::SafeDownCast(inList[i]->m_Vme);
			mafNode * pParent = inList[i]->m_Vme->GetParent();

			// to check if the LandmarkCloud node is in a "Muscle Attachment Areas" VMEGroup
			if (pSurf && (strlen(parentName) ==0 || !strcmp(pParent->GetName(), parentName))  
				&& (strlen(nodeName) ==0 || !strcmp(pSurf->GetName(), nodeName))  )
			{
				mafString name = pSurf->GetName();
				outList.push_back(pSurf);

				if (pParent->IsA("mafVMESurface") && inList[i]->m_Vme->IsA("mafVMESurface") ) {
					m_GroupNodeMap.insert(std::pair<mafNode * , mafNode * > (inList[i]->m_Vme, pParent));
				}

				if (!strcmp(pParent->GetName(), "Bones"))
				{
					// find the mat of pParent
					mafMatrix mat;
					mafVMESurface * parentVme = mafVMESurface::SafeDownCast(pParent);
					parentVme->GetOutput()->GetAbsMatrix(mat, parentVme->GetTimeStamp() );
				}
				if (!strcmp(pParent->GetName(), "LeftFoot"))
					mafLogMessage(" ");
				
				if (!strcmp(pParent->GetName(), "RightShank"))
					mafLogMessage(" ");

			}	else	{
				if (!strcmp(pParent->GetName(), "LeftFoot"))
					mafLogMessage(" ");

				if (!strcmp(pParent->GetName(), "RightShank"))
					mafLogMessage(" ");
				/*
				if ( pSurf && pParent && !strcmp(pSurf->GetName(), "Bones")  &&  !strcmp(pParent->GetName(), "Pelvis") )
				{
					mafMatrix mat;
					pSurf->GetOutput()->GetAbsMatrix(mat, pSurf->GetTimeStamp() );
					//pSurf->
				}
				*/
			}
	}
	//mafLogMessage("%s : %d loaded", parentName, outList.size());
}


void lhpOpScaleMMA::LoadMotionLsFromList(medMSMGraph::MSMGraphNodeList & inList, 
	std::vector<mafVMELandmarkCloud*> & outList)
{
	for (int i = 0;  i < inList.size(); i++)
	{
			mafVMELandmarkCloud * pSurf = mafVMELandmarkCloud::SafeDownCast(inList[i]->m_Vme);
			mafNode * pParent = inList[i]->m_Vme->GetParent();
			
			mafNode * pGrandParent = NULL;
			if (pParent != NULL)
			{
				pGrandParent = pParent->GetParent();
			
				// to check if the LandmarkCloud node is in a "Muscle Attachment Areas" VMEGroup
				// TODO: remove foot
				if (pSurf && pGrandParent /* && strcmp(pParent->GetName(), "LeftFoot")  
					&& strcmp(pParent->GetName(), "RightFoot")  
					&& (strstr(pSurf->GetName(), "Foot") == NULL)
					/* && (!strcmp(pSurf->GetName(), "MotionLs")) */  )
					//&& (strcmp(pGrandParent->GetName(), "Muscle Geometry"))  )
				{
					mafString name = pSurf->GetName();

					/*
					if (! strcmp(name, "Pelvis Cloud registered on VP"))
					{
						mafLogMessage("Pelvis landmark found ");

					}
					*/
					
					mafMatrix mat;
					pSurf->GetOutput()->GetAbsMatrix(mat, pSurf->GetTimeStamp() );

					outList.push_back(pSurf);

				}	else	{
					//mafLogMessage(" ");
				}

			}
	}
	
}



void lhpOpScaleMMA::PrefetchDataInfo()
{
	medMSMGraph::MSMGraphNodeList  MuscleList; // muscle list
	medMSMGraph::MSMGraphNodeList  BoneList; // muscle list
	medMSMGraph::MSMGraphNodeList  AttachList; // attachment landmark clouds list
	medMSMGraph::MSMGraphNodeList  LigamentList; 
	medMSMGraph::MSMGraphNodeList  TendontList; 
	medMSMGraph::MSMGraphNodeList LandmarkList;

	// get input model
	medVMEMusculoSkeletalModel * pModel = medVMEMusculoSkeletalModel::SafeDownCast(m_Input);
	const medMSMGraph * pGraph = pModel->GetMSMGraph();

	

	// get muscles
	pGraph->GetMuscles(m_LodLevel, MuscleList , true);
	pGraph->GetBones(m_LodLevel, BoneList , true);
	pGraph->GetLandmarkClouds(AttachList , true);
	pGraph->GetLigaments(LigamentList,  true);

	std::vector<mafVMESurface* > LoadedMuscleList; // source template surface list
	std::vector<mafVMESurface* > LoadedBoneList; // source template surface list
	std::vector<mafVMESurface * > LoadedLigamentList;
	std::vector<mafVMELandmarkCloud * > LoadedAttachList;

	m_GroupNodeMap.clear();

	LoadSurfaceFromList(MuscleList, "Muscles", "", LoadedMuscleList);
	LoadSurfaceFromList(BoneList, "Bones", "", LoadedBoneList);
	LoadSurfaceFromList(LigamentList, "Muscles", "", LoadedLigamentList);
	LoadSurfaceFromList(AttachList, "Muscle Attachment Areas", "", LoadedAttachList);
	//LoadSurfaceFromList(AttachList, "", LoadedAttachList);

	//int s1 = AttachList.size();
	//int s2 = LoadedAttachList.size();

	m_nMuscle = LoadedMuscleList.size();
	m_nAttach = LoadedAttachList.size();
	m_nBone = LoadedBoneList.size();
	m_nLigaments = LoadedLigamentList.size();
}


void lhpOpScaleMMA::PrepareData()
{
	m_InputCopy = CopyTree(m_Input, m_Input->GetRoot());

	medMSMGraph::MSMGraphNodeList  MuscleList; // muscle list
	medMSMGraph::MSMGraphNodeList  BoneList; // muscle list
	medMSMGraph::MSMGraphNodeList  AttachList; // attachment landmark clouds list
	medMSMGraph::MSMGraphNodeList  LigamentList; 
	medMSMGraph::MSMGraphNodeList  TendontList; 
	medMSMGraph::MSMGraphNodeList  LeftFootList; 
	medMSMGraph::MSMGraphNodeList  RightFootList;
	
	// TODO : add motion landmarks
	medMSMGraph::MSMGraphNodeList LandmarkList;


	// get input model
	medVMEMusculoSkeletalModel * m_pModel = medVMEMusculoSkeletalModel::SafeDownCast(m_InputCopy);
	m_pGraph = m_pModel->GetMSMGraph();

	// get muscles
	m_pGraph->GetMuscles(m_LodLevel, MuscleList , true);
	m_pGraph->GetBones(m_LodLevel, BoneList , true);
	m_pGraph->GetLandmarkClouds(AttachList , true);
	m_pGraph->GetLigaments(LigamentList,  true);

	m_GroupNodeMap.clear();

	LoadSurfaceFromList(MuscleList, "Muscles", "",  m_MuscleList);
	LoadSurfaceFromList(BoneList, "Bones", "", m_BoneList);
	LoadSurfaceFromList(LigamentList, "Muscles", "", m_LigamentList);
	LoadSurfaceFromList(AttachList, "Muscle Attachment Areas", "", m_AttachList);
	//
	//LoadSurfaceFromList(AttachList, "", "MotionLs", m_MotionLsList);
	LoadMotionLsFromList(AttachList, m_MotionLsList);

	m_pGraph->GetRegion(medMSMGraph::RegionType::LeftFoot, LeftFootList); 
	m_pGraph->GetRegion(medMSMGraph::RegionType::RightFoot, RightFootList); 
	mafNode * pNode =  LeftFootList[0]->m_Vme;
	mafVMESurface * pSurf = mafVMESurface::SafeDownCast(pNode);
	//mafString name = pSurf->GetName();
	m_BoneList.push_back(pSurf);
	
	pNode =  RightFootList[0]->m_Vme;
	pSurf = mafVMESurface::SafeDownCast(pNode);
	m_BoneList.push_back(pSurf);



	int s1 = AttachList.size();
	int s2 = m_AttachList.size();

	m_nMuscle = m_MuscleList.size();
	m_nAttach = m_AttachList.size();
	m_nBone = m_BoneList.size();
	m_nLigaments = m_LigamentList.size();
	m_nMotionLs = m_MotionLsList.size();

	mafLogMessage("Loading data finished: %d muscles , %d bones, %d ligaments, %d attachments.", m_nMuscle, m_nBone, m_nLigaments, m_nAttach);

	//BES: 29.6.2012 - make sure that all landmark clouds are closed
	for (int i = 0; i < m_nAttach; i++) {
		if (m_AttachList[i]->IsOpen()) {
			m_AttachList[i]->Close();
		}
	}
}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::CreateGUI()
//----------------------------------------------------------------------------
{
	// setup Gui on the right panel
	m_Gui = new mafGUI(this);
	
	m_Gui->SetListener(this);

	//m_Gui->Label("Testing UI for ScaleMMA");
	//m_Gui->Divider();

	//m_Gui->Label("");
	m_Gui->Label(_("Choose Patient Landmarks:"),true);
	m_Gui->Label(&m_NameLandmarkPatient);
	m_Gui->Button(ID_CHOOSE_LANDMARK_PATIENT,_("choose patient landmark"));

	m_Gui->Label("");
	
	m_Gui->Label("Number of muscles to process");
	//m_Gui->Integer(ID_NUM_MUSCLE,_("Number"),&m_nMuscleRun,1,m_nMuscle);
	mafString msg;
	msg << "Total number of muscles: " << m_nMuscle;
	m_Gui->Label(msg);

	m_nBoneRun = m_nBone ;
	m_Gui->Label("Number of bones to process");
	//m_Gui->Integer(ID_NUM_BONE,_("Number"),&m_nBoneRun,1,m_nBone);
	msg.Erase(0);;
	msg << "Total number of bones: " << m_nBone;
	m_Gui->Label(msg);

	//m_nBoneRun = m_nBone ;
	m_Gui->Label("Number of ligaments to process");
	//m_Gui->Integer(ID_NUM_BONE,_("Number"),&m_nBoneRun,1,m_nBone);
	msg.Erase(0);;
	msg << "Total number of ligaments: " << m_nLigaments;
	m_Gui->Label(msg);
	
	m_Gui->Divider(2);
	m_Gui->OkCancel();

	m_Gui->Enable(wxOK,false);

}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::OpDo()
//----------------------------------------------------------------------------
{
	//Superclass::OpDo();
}

void lhpOpScaleMMA::CleanSurfaceList(std::vector<mafVMESurface* > & surfList, double rate,  
									 std::vector<vtkMAFSmartPointer<vtkPolyData> > & cleanList, 
									 std::vector<vtkMAFSmartPointer<vtkPolyData> > & reducedList,  
									 int & nSurf, double weight)
{

	int nPointOriginal, nPointCleaned, nPointReduced;

	for (int i = 0; i < surfList.size(); i++)
	{
		mafVMESurface * pSurf = surfList[i];

		//mafLogMessage("Reading muscle %d %s", i, pSurf->GetName());
		pSurf->GetOutput()->Update();
		vtkMAFSmartPointer<vtkPolyData>  pPolyData = vtkPolyData::SafeDownCast(pSurf->GetOutput()->GetVTKData());
		nPointOriginal = pPolyData->GetNumberOfPoints();
		vtkMAFSmartPointer<vtkPolyData> pCleanPoly = CleanPolyData(pPolyData);
		nPointCleaned = pCleanPoly->GetNumberOfPoints();
		vtkMAFSmartPointer<vtkPolyData> pLowResPoly = ReducePolyData(pCleanPoly, rate);
		
		if (pLowResPoly.GetPointer() )
		{
			nPointReduced = pLowResPoly->GetNumberOfPoints();
			mafLogMessage("%s, original size %d, cleaned %d, simplified %d.", pSurf->GetName(), nPointOriginal, nPointCleaned, nPointReduced);

			m_SurfVMEList.push_back(pSurf );
			cleanList.push_back(pCleanPoly);
			reducedList.push_back(pLowResPoly);
			m_SurfList.push_back(pLowResPoly);
			m_SurfListCleanHighRes.push_back(pCleanPoly);
			m_srcNameList.push_back(std::string(pSurf->GetName() ) );
			m_srcSizeList.push_back(pLowResPoly->GetNumberOfPoints());
			m_WeightList.push_back(weight);
		} else {
			mafLogMessage("error in simplifying %s", pSurf->GetName());
		}
	}

	nSurf = cleanList.size();
}

void lhpOpScaleMMA::CleanAllSurfaceList()
{
	m_srcSizeList.clear();

	CleanSurfaceList(m_MuscleList, 0.1, m_HighResMuscleListClean, m_MuscleListClean,  m_nMuscleClean, 1.0);
	CleanSurfaceList(m_BoneList, 0.05,  m_HighResBoneListClean, m_BoneListClean,  m_nBoneClean, 50);
	CleanSurfaceList(m_LigamentList, 0.5, m_HighResLigamentListClean, m_LigamentListClean, m_nLigamentClean, 1.0);

	assert(m_SurfList.size() == m_srcSizeList.size());
}

void lhpOpScaleMMA::FindAttachmentMap(std::vector< std::vector<tAttSurfDist>   >  & attachSurfMap, 
									  std::vector<std::vector<int> > & surfAttachMap, std::vector<vtkMAFSmartPointer<vtkPolyData> >  & surfList, double threshold)
{
	int nSurf = surfList.size();
	surfAttachMap.resize(nSurf);;
	
	double avgDist, lastDist;

	AttSurfDistComp compObj;

	attachSurfMap.resize(m_nAttach);

	for (int j = 0; j < m_nAttach; j++)	
	{
		lastDist = 1e6;

		mafVMELandmarkCloud * pLandmarkCloud =m_AttachList[j];

		for (int i = 0; i < surfList.size(); i++)
		{
			//mafVMESurface * pSurf = surfList[i];
			vtkPolyData * pSurf = surfList[i].GetPointer();
			//const char * name = pSurf->GetName();

			//if (strstr(name, "S032_R_M094_Quadriceps_Tendon_Superf") )
			//	mafLogMessage("S032_R_M094_Quadriceps_Tendon_Superfs");

			if (pLandmarkCloud && pSurf && IsClose(pLandmarkCloud, pSurf , threshold) )
			{
				avgDist = AverageDistance(pLandmarkCloud, pSurf);

				if ( avgDist <threshold)
				{
					lastDist = avgDist;
					tAttSurfDist pair(i, avgDist);
					attachSurfMap[j].push_back(pair);
				}
			}
		}

		std::sort(attachSurfMap[j].begin(), attachSurfMap[j].end(), compObj);

		if (!attachSurfMap[j].empty())
			surfAttachMap[attachSurfMap[j][0].first].push_back(j);

		mafLogMessage(_T("%s attached to  ------"), m_AttachList[j]->GetName() );
		for (int k = 0; k < attachSurfMap[j].size(); k++)
		{
			mafLogMessage(_T(" %s with distance %f") , m_SurfVMEList[attachSurfMap[j][k].first ]->GetName(), attachSurfMap[j][k].second);
		}
		
	}
}

void lhpOpScaleMMA::FindMotionLsMap(std::vector< LdMap>  & motionLsMap, 
									  /*std::vector<std::vector<int> > & surfAttachMap, */ std::vector<vtkMAFSmartPointer<vtkPolyData> >  & surfList, double threshold)
{
	int nSurf = surfList.size();
	//surfAttachMap.resize(nSurf);;
	
	//double avgDist, lastDist
	double closestDist, dist;
	int closestPtId, pointId, closestSurfId;

	//AttSurfDistComp compObj;
	//attachSurfMap.resize(m_nAttach);

	for (int j = 0; j < m_nMotionLs; j++)	
	{
		closestDist = 1e6;
		closestSurfId = -1;

		mafVMELandmarkCloud * pLandmarkCloud = m_MotionLsList[j];
		//if (  !pLandmarkCloud  || (strcmp(pLandmarkCloud->GetName(), "MotionLs")) )
		if (  !pLandmarkCloud )
			continue;
		
		/*
		if (! strcmp(pLandmarkCloud->GetName(), "MotionLs"))
		{
			//mafMatrix mat;
			//pLandmarkCloud->GetOutput()->GetAbsMatrix(mat, pLandmarkCloud->GetTimeStamp() );
			mafLogMessage("MotionLs");
		}
		*/

		int nLandmark = pLandmarkCloud->GetNumberOfLandmarks();
		int k;
		for ( k = 0; k < nLandmark; k++)
		{
			closestDist = 1e5;
			closestSurfId = -1;

			for (int i = 0; i < surfList.size(); i++)
			{
				vtkPolyData * pSurf = surfList[i].GetPointer();
				//const char * name = pSurf->GetName();

				if (pSurf && IsClose(pLandmarkCloud, pSurf , threshold) )
				{
					// need closest point id
					dist =  PtSurfDistance(pLandmarkCloud, k, pSurf, pointId);

					// <threshold
					if (( dist < closestDist) &&  (dist < threshold))
					{
						closestDist =  dist;
						closestSurfId = i;
						closestPtId = pointId;
					}
				}
			}

			if (closestSurfId != -1)
			{
				// add the landmark to the list
				motionLsMap.push_back(LdMap(j, k, closestSurfId, closestPtId));
			}
			else
			{
				mafLogMessage("closest point not found in MotionLs %d %d", j, k);
			}

			if (closestDist > 10.0)
				mafLogMessage("closest distance %f %d %d", closestDist, j, k);

		}


		/*
		std::sort(attachSurfMap[j].begin(), attachSurfMap[j].end(), compObj);

		if (!attachSurfMap[j].empty())
			surfAttachMap[attachSurfMap[j][0].first].push_back(j);

		mafLogMessage(_T("%s attached to  ------"), m_AttachList[j]->GetName() );
		for (int k = 0; k < attachSurfMap[j].size(); k++)
		{
			mafLogMessage(_T(" %s with distance %f") , m_SurfVMEList[attachSurfMap[j][k].first ]->GetName(), attachSurfMap[j][k].second);
		}
		*/
		
	}
}

// muscle , bone, ligament
void lhpOpScaleMMA::FindAttachmentMap()
{
	// establish correspondence between attachments and muscles
	//mafLogMessage("------------------ Muscles -----------------------");
	FindAttachmentMap(m_AttachMuscleMap, m_MuscleAttachMap, m_MuscleListClean , 4.0);
	//mafLogMessage("------------------ Bones -----------------------");
	FindAttachmentMap(m_AttachBoneMap,m_BoneAttachMap, m_BoneListClean, 3.0);
	//mafLogMessage("------------------ Ligaments -----------------------");
	FindAttachmentMap(m_AttachLigamentMap,m_LigamentAttachMap, m_LigamentListClean, 20.0);
	//mafLogMessage("------------------ MotionLs -----------------------");
	FindMotionLsMap(m_MotionLsMap, /* m_BoneAttachMap , */ m_BoneListClean, 10000.0);
	
	int nSize = m_MotionLsMap.size();

	mafLogMessage("Mapping attachments to muscles and bones finished");
}

void lhpOpScaleMMA::AddRegionToGroupMap(int parentId, std::vector<int> & childList)
{
	medMSMGraph::MSMGraphNodeList list;
	mafNode * pParent = NULL, * pChild = NULL;
	
	m_pGraph->GetRegion(parentId, list); 
	if (!list.empty())
	{
		pParent =  list[0]->m_Vme;
		for (int i = 0; i < childList.size(); i++)
		{
			list.clear();
			m_pGraph->GetBone(childList[i], m_LodLevel, list, true);
			if (!list.empty()) {
				pChild =  list[0]->m_Vme;
				m_GroupNodeMap.insert(std::pair<mafNode*, mafNode*>(pChild, pParent));
				list.clear();
			}
		}
	}
}

void lhpOpScaleMMA::MatSurfLdInput()
{
	// surfaces
	// std::vector<mafVMESurface* > m_MuscleList; // source template surface list
	// std::vector<mafVMESurface* > m_BoneList; // source template surface list
	 //std::vector<mafVMESurface * > m_LigamentList;

	// landmarks
	for (int i = 0; i <  m_AttachList.size(); i++)
		MatLdInput(m_AttachList[i]);
	for (int i = 0; i <  m_MotionLsList.size(); i++)
		MatLdInput(m_MotionLsList[i]);

}

void lhpOpScaleMMA::MatLdInput(mafVMELandmarkCloud * pVme)
{
	pVme->GetOutput()->Update();
	vtkPointSet * pPoints = vtkPointSet::SafeDownCast(pVme->GetOutput()->GetVTKData());

	if (!pPoints)
		return;

	pPoints->Update();

	mafMatrix mat;
	pVme->GetOutput()->GetAbsMatrix(mat, pVme->GetTimeStamp() );
	mafTransform transform;
	transform.SetMatrix(mat);  
	//transform.Invert();

	double pIn[3], pOut[3];

	int nPoint = pPoints->GetNumberOfPoints();
	//vtkMAFSmartPointer<vtkPoints> pOutPoints;
	//pOutPoints->SetNumberOfPoints(nPoint);

	for (int i = 0; i < nPoint; i++)
	{
		pPoints->GetPoint(i, pIn);
		transform.TransformPoint(pIn, pOut);
		//pOutPoints->SetPoint(i, pOut);
		pVme->SetLandmark(i, pOut[0], pOut[1], pOut[2], pVme->GetTimeStamp());

#ifdef _DEBUG
		if (fabs(pIn[0] - pOut[0]) > 1e-6)
			mafLogMessage("Points transformed");
#endif
	}

	//pPoints->SetPoints(pOutPoints);
	//pVme->SetData(pPoints, pVme->GetTimeStamp());

}

void lhpOpScaleMMA::MatSurfInput(mafVMESurface * pVme)
{

}
/*
void lhpOpScaleMMA::MatInput(vtkPointSet * pPoints, mafMatrix mat)
{
		//mafMatrix mat;
		//pGroupVme->GetOutput()->GetAbsMatrix(mat, pGroupVme->GetTimeStamp() );
		// mulitply the inverse matrix
		mafTransform transform;
		transform.SetMatrix(mat);  
		
		//transform.Invert();

		double p[3];
		int  nPoint =  pPoints->GetNumberOfPoints();

		vtkMAFSmartPointer<vtkPoints> pOutPoints;
		pOutPoints->SetNumberOfPoints(nPoint);
		for (int i = 0; i < nPoint; i++)
		{      
			transform.TransformPoint(pPoly->GetPoint(i), p);
			pOutPoints->SetPoint(i, p);
		}
		pPoly->SetPoints(pOutPoints);
		pGroupVme->SetData(pPoly, pGroupVme->GetTimeStamp());
}
*/
void lhpOpScaleMMA::MatSurfLdOutput()
{
	// surfaces
	// std::vector<mafVMESurface* > m_MuscleList; // source template surface list
	// std::vector<mafVMESurface* > m_BoneList; // source template surface list
	 //std::vector<mafVMESurface * > m_LigamentList;

	// landmarks
	for (int i = 0; i <  m_AttachList.size(); i++)
		MatLdOutput(m_AttachList[i]);

	for (int i = 0; i <  m_MotionLsList.size(); i++)
		MatLdOutput(m_MotionLsList[i]);

}


void lhpOpScaleMMA::MatLdOutput(mafVMELandmarkCloud * pVme)
{
	if (!strcmp(pVme->GetName(), "Pelvis Cloud registered on VP") )
		mafLogMessage("Pelvis Cloud found");

	pVme->GetOutput()->Update();
	vtkPointSet * pPoints = vtkPointSet::SafeDownCast(pVme->GetOutput()->GetVTKData());
	if (!pPoints)
		return;

	pPoints->Update();

	mafMatrix mat;
	pVme->GetOutput()->GetAbsMatrix(mat, pVme->GetTimeStamp() );
	mafTransform transform;
	transform.SetMatrix(mat);  
	transform.Invert();

	double pIn[3], pOut[3];

	int nPoint = pPoints->GetNumberOfPoints();
	//vtkMAFSmartPointer<vtkPoints> pOutPoints;
	//pOutPoints->SetNumberOfPoints(nPoint);

	for (int i = 0; i < nPoint; i++)
	{
		pPoints->GetPoint(i, pIn);
		transform.TransformPoint(pIn, pOut);
		//pOutPoints->SetPoint(i, pOut);
		pVme->SetLandmark(i, pOut[0], pOut[1], pOut[2], pVme->GetTimeStamp());
	}

	//pPoints->SetPoints(pOutPoints);
	//pVme->SetData(pPoints, pVme->GetTimeStamp());

}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::Execute()
//----------------------------------------------------------------------------
{
	wxBusyCursor wait_cursor;

	//Superclass::OpDo();
	m_maxN = -1;
	m_minN = 100;

	PrepareData();

	CleanAllSurfaceList();

	// TODO, add matrix handling here
	MatSurfLdInput();

	FindAttachmentMap();
	
	FindLandmarkBones();

	// add direct children surface to  m_GroupMapNode
	// pelvis
	std::vector<int> childList;
	childList.push_back(medMSMGraph::BoneType::LeftIliac);
	childList.push_back(medMSMGraph::BoneType::RightIliac);
	AddRegionToGroupMap(medMSMGraph::RegionType::Pelvis,  childList);

	// RightThigh
	childList.clear();
	childList.push_back(medMSMGraph::BoneType::RightFemur);
	AddRegionToGroupMap(medMSMGraph::RegionType::RightThigh,  childList);

	// RightShank
	childList.clear();
	childList.push_back(medMSMGraph::BoneType::RightTibia);
	childList.push_back(medMSMGraph::BoneType::RightFibula);
	AddRegionToGroupMap(medMSMGraph::RegionType::RightShank,  childList);

	// LeftThigh
	childList.clear();
	childList.push_back(medMSMGraph::BoneType::LeftFemur);
	AddRegionToGroupMap(medMSMGraph::RegionType::LeftThigh,  childList);

	// LeftShank
	childList.clear();
	childList.push_back(medMSMGraph::BoneType::LeftTibia);
	childList.push_back(medMSMGraph::BoneType::LeftFibula);
	AddRegionToGroupMap(medMSMGraph::RegionType::LeftShank,  childList);



	/*
	std::map<mafNode * , vtkMAFSmartPointer<  vtkAppendPolyData> > newGroupMap;
	vtkAppendPolyData * pNewGroupPoly = NULL;

	tGroupNodeMap::iterator iterGroupMap;
	for  (iterGroupMap = m_GroupNodeMap.begin(); iterGroupMap != m_GroupNodeMap.end(); iterGroupMap++)
	{
		mafNode * pChild = iterGroupMap->first;
		mafNode * pParent = iterGroupMap->second;

		if (!strcmp(pParent->GetName(), "Pelvis"))
			mafLogMessage("Pelvis region found");

		if (newGroupMap.find(pParent) == newGroupMap.end())
		{
			vtkMAFSmartPointer<vtkAppendPolyData> newGroupPoly;
			newGroupMap[pParent] = newGroupPoly;
			pNewGroupPoly = newGroupPoly.GetPointer();
		}
		else
		{
			pNewGroupPoly = newGroupMap[pParent].GetPointer();
		}

		mafVMESurface * pChildSurf = mafVMESurface::SafeDownCast(pChild);
		pChildSurf->GetOutput()->Update();
		vtkPolyData * pPoly = vtkPolyData::SafeDownCast(pChildSurf->GetOutput()->GetVTKData());

		pNewGroupPoly->AddInput(pPoly);

	}

	std::map<mafNode * , vtkMAFSmartPointer<  vtkAppendPolyData> >::iterator iterNewGroupMap;
	for (iterNewGroupMap = newGroupMap.begin(); iterNewGroupMap != newGroupMap.end(); iterNewGroupMap++)
	{
		mafNode * pGroup = iterNewGroupMap->first;
		vtkAppendPolyData * pAppend =  iterNewGroupMap->second;
		mafVMESurface * pGroupVme = mafVMESurface::SafeDownCast(pGroup);
		pGroupVme->SetData(pAppend->GetOutput(), 0);

	}
	*/


	// calculate attachment constraint points
	int nSurf = m_SurfList.size();
	// calculate the start index number ( need to * 3 for matrix entry row)
	m_srcStartIndexList.resize(nSurf);
	m_srcStartIndexList[0] = 0;
	for (int i = 1; i < nSurf; i++) {
		int s1 = m_srcSizeList[i-1];
		m_srcStartIndexList[i] = s1 + m_srcStartIndexList[i - 1];
	}

	int nLandmark;
	int  attachStart = m_srcStartIndexList.back() + m_srcSizeList.back();
	for (int i = 0; i <m_nAttach; i++)
	{
		mafVMELandmarkCloud * pLandmark = m_AttachList[i];
		vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pLandmark->GetOutput()->GetVTKData());
		
		m_AttachStartIndexList.push_back(attachStart);
		nLandmark =  pLandmarkPoints->GetNumberOfPoints();
		m_AttachSizeList.push_back(nLandmark);
		attachStart += nLandmark;
	}

	int  motionLsStart = m_AttachStartIndexList.back() + m_AttachSizeList.back();
	for (int i = 0; i <m_nMotionLs; i++)
	{
		mafVMELandmarkCloud * pLandmark = m_MotionLsList[i];
		vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pLandmark->GetOutput()->GetVTKData());
		
		m_MotionLsStartIndexList.push_back(motionLsStart);
		nLandmark =  pLandmarkPoints->GetNumberOfPoints();
		m_MotionLsSizeList.push_back(nLandmark);
		motionLsStart += nLandmark;
	}


	// including bones and muscles
	CalcInterSurfConstraint(m_SurfList);
	// calculate patient landmark constraint
	CalcLandmarkConstraint();
	
	// build matrix
	// number of rows
	m_nRow = 0;
	m_T = cs_spalloc (0, 0, 1, 1, 1) ;;
	m_bvec.clear();

	// 1
	for (int i = 0; i < m_SurfList.size() ; i++)
	{
		AddSourceSurface(i);
	}


	CalcAttachmentConstraint();

	 // 2
	AddAttachmentConstraint();

	AddMotionLsConstraint();

	// 3
	 for (int i = 0; i < m_SurfList.size() ; i++)
	 {
		 for (int j = i + 1; j < m_SurfList.size(); j++)
		 {
				AddInterMuscleConstraint(i, j);
		 }
	 }

	 AddLandmarkConstraint();

	

	


	wxBusyInfo wait(_("Calculating: Please wait"));

	 // sovle problem
	 m_pProb = GenProblem (m_T, m_bvec, 1e-6);

	 int nPoint1 = m_T->n / 3.0;
	 cs_spfree(m_T);

	 int successful = Solve(m_pProb);

	 if (successful)
		mafLogMessage(_T("problem solved successfully"));
	 else
	 {
		wxString msg = _("Problem solve failed probably caused by memory allocation failure\n");
		wxMessageBox(msg,_("Operation failed"),wxOK|wxICON_ERROR);
		 mafLogMessage(_T("problem solve failed"));

		 free_problem(m_pProb);

		return;
	 }

	//write output
	 std::string nameRootDeformed =  std::string(m_InputCopy->GetName()) + std::string(" Deformed");
	 m_InputCopy->SetName(nameRootDeformed.c_str());

 	//m_OutSurface.resize(nSurf);
	m_OutSurfaceHighRes.clear();
	m_OutAttach.clear();

	double coord[3], coord1[3];
	int startIdx = 0;

	/*
	mafNode * root = m_Input->GetRoot();

	mafSmartPointer<mafVMEGroup> gDeformed;
	gDeformed->SetName("Deformed Muscle");
	root->AddChild(gDeformed);
	gDeformed->SetParent(root);
	*/

	bool bOutRange;
	double bb[6];
	
	 for (int i = 0; i < nSurf; i++)
	 {
		 bOutRange = false;

		vtkMAFSmartPointer<vtkPolyData>  pOriginalMesh = m_SurfList[i];
		pOriginalMesh->Update();
		vtkMAFSmartPointer<vtkPolyData> pDeformedMesh;
		pDeformedMesh->DeepCopy(pOriginalMesh.GetPointer());
		pDeformedMesh->Update();

		int nPoint = m_srcSizeList[i];
		startIdx = m_srcStartIndexList[i]*3;

		vtkPoints * pPoints = pDeformedMesh->GetPoints();

		int bvecSize = m_bvec.size();

		vtkMAFSmartPointer<vtkPoints>  pAllPoints;
		pAllPoints->SetNumberOfPoints(nPoint);
	
		for (int j = 0; j < nPoint; j++)
		{
			pOriginalMesh->GetPoint(j, coord1);
			//if ((fabs (prob->x[i*3]) < 1e0 )&& (fabs (prob->x[i*3 + 1]) < 1e0 ) && (fabs (prob->x[i*3 + 2]) < 1e0 ))
			//	mafLogMessage(_T("singular point %d"), i);
			//pPoints->SetPoint(vtkIdType(i), prob->x[(nPoint0 + i)*3], prob->x[(nPoint0 + i)*3+1], prob->x[(nPoint0 + i)*3+2]);
			coord[0] = m_pProb->x[startIdx];
			coord[1] = m_pProb->x[startIdx+1];
			coord[2] = m_pProb->x[startIdx+2];

			if (fabs(coord[0]) > 1e4 || fabs(coord[1]) > 1e4 || fabs(coord[2]) > 1e4) {
				mafLogMessage("point coordinate value out of range");
				bOutRange = true;
			}

			if (fabs(coord[0]) < 1e-4 || fabs(coord[1]) < 1e-4 || fabs(coord[2]) < 1e-4)
				mafLogMessage("point coordinate value collapsed");

			pAllPoints->SetPoint(vtkIdType(j), coord);
			startIdx += 3;
			
		}
		
		pDeformedMesh->SetPoints(pAllPoints.GetPointer());
		pDeformedMesh->Update();
		//pAllPoints->Delete();

		m_OutSurfaceVtk.push_back(pDeformedMesh);

		if (bOutRange) {
			pDeformedMesh->GetBounds(bb);
		}

	 }

	// write output attachment landmarks
	//mafSmartPointer<mafVMEGroup> gDeformedAttach;
	//gDeformedAttach->SetName("Deformed Attachment");
	//root->AddChild(gDeformedAttach);
	//gDeformedAttach->SetParent(root);

	mafMatrix mat;
	double coordOut[3];


	for (int i = 0; i < m_nAttach; i++)
	{
		int startIdx = m_AttachStartIndexList[i] * 3;

		//mafSmartPointer<mafVMELandmarkCloud> outLdCloud;
		//outLdCloud->DeepCopy(m_AttachList[i]);
		mafSmartPointer<mafVMELandmarkCloud> outLdCloud =  m_AttachList[i];

		//outLdCloud->GetOutput()->GetAbsMatrix(mat, outLdCloud->GetTimeStamp() );
		//mafTransform transform;
		//transform.SetMatrix(mat);  
		//transform.Invert();

		
		for (int j = 0; j < m_AttachList[i]->GetNumberOfLandmarks(); j++)
		{
			coord[0] = m_pProb->x[startIdx];
			coord[1] = m_pProb->x[startIdx+1];
			coord[2] = m_pProb->x[startIdx+2];

			//transform.TransformPoint(coord, coordOut);
			//outLdCloud->SetLandmark(j, coordOut[0], coordOut[1], coordOut[2], 0);
			outLdCloud->SetLandmark(j, coord[0], coord[1], coord[2], 0);

			startIdx += 3;
		}
		
		//m_OutAttach.push_back(outLdCloud);
		//gDeformedAttach->AddChild(m_OutAttach[i].GetPointer());
	}
	
	// handle motion list
	for (int i = 0; i < m_nMotionLs; i++)
	{
		int startIdx = m_MotionLsStartIndexList[i] * 3;

		//mafSmartPointer<mafVMELandmarkCloud> outLdCloud;
		//outLdCloud->DeepCopy(m_AttachList[i]);
		mafSmartPointer<mafVMELandmarkCloud> outLdCloud =  m_MotionLsList[i];
		//outLdCloud->GetOutput()->GetMatrix(mat, outLdCloud->GetTimeStamp() );
		//mafMatrix eyeMat;
		//eyeMat.Identity();
		
		//if (!strcmp(outLdCloud->GetName(), "Pelvis Cloud registered on VP") )
		//	outLdCloud->SetAbsMatrix(eyeMat);

		//mafTransform transform;
		//transform.SetMatrix(mat);  
		//transform.Invert();

		for (int j = 0; j < m_MotionLsList[i]->GetNumberOfLandmarks(); j++)
		{
			coord[0] = m_pProb->x[startIdx];
			coord[1] = m_pProb->x[startIdx+1];
			coord[2] = m_pProb->x[startIdx+2];

			// handle matrix here
			//transform.TransformPoint(coord, coordOut);
			//outLdCloud->SetLandmark(j, coordOut[0], coordOut[1], coordOut[2], 0);
			outLdCloud->SetLandmark(j, coord[0], coord[1], coord[2], 0);
			
			startIdx += 3;
		}
		
		//m_OutAttach.push_back(outLdCloud);
		//gDeformedAttach->AddChild(m_OutAttach[i].GetPointer());
	}
	
	free_problem(m_pProb);
	 

	//mafSmartPointer<mafVMEGroup> gDeformedHighRes;
	//gDeformedHighRes->SetName("Deformed High Resolution Surface");
	//root->AddChild(gDeformedHighRes);
	//gDeformedHighRes->SetParent(root);
	
	// remapping to high resolution
	int nSize = m_OutSurfaceVtk.size();
	//m_GroupNodeMap.clear();
	
	int nOut = m_OutSurfaceVtk.size();
	for (int i = 0; i < m_OutSurfaceVtk.size(); i++)
	//for (int i = 0; i < 3; i++)
	{
		int nPointH = m_SurfListCleanHighRes[i]->GetNumberOfPoints();
		int nPointL =  m_SurfList[i]->GetNumberOfPoints();
		
		
		int nMem = nPointH * nPointL * 8  / 1e6;
/*
#ifdef _DEBUG		
		if (nMem > 500)
		{
			mafLogMessage("Mapping %d, Low %d, High %d Mem %d, skipping", i, nPointL, nPointH,  nMem);
			continue;
		} else 
#endif
		*/
		{
			mafLogMessage("Mapping %d, Low %d, High %d Mem %d, %d/%d", i, nPointL, nPointH,  nMem, i, nOut);
		}

		double *   pWeights = new double[nPointL];
		double ptRec[3];

		//vtkMAFSmartPointer<vtkDoubleArray>  relationship;
		//vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(m_SurfList[i].GetPointer(), 
		//	m_SurfListCleanHighRes[i].GetPointer(), relationship.GetPointer());

		//scale all pLowResPolyDataBeingScaled
		vtkMAFSmartPointer<vtkPolyData>  outSurfHighRes;
		outSurfHighRes->DeepCopy(m_SurfListCleanHighRes[i].GetPointer());
		vtkPoints * pHighResPoints = outSurfHighRes->GetPoints();

		for (int j = 0; j < nPointH; j++)
		{
			 vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(m_SurfList[i].GetPointer(),  m_SurfListCleanHighRes[i]->GetPoint(j), 1, pWeights);
     
			vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(m_OutSurfaceVtk[i].GetPointer(), 1, (const double*)pWeights, ptRec);
			pHighResPoints->SetPoint(j, ptRec);
		}

		delete [] pWeights;

		outSurfHighRes->SetPoints(pHighResPoints);

		//vtkPolyData * outPoly =  vtkPolyData::SafeDownCast(m_OutSurfaceVtk[i].GetPointer();
		//vtkPolyData * outPoly = m_OutSurfaceVtk[i].GetPointer();
		//vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(m_OutSurfaceVtk[i].GetPointer(),  relationship.GetPointer(), outSurfHighRes.GetPointer());

		int nPoint = outSurfHighRes->GetNumberOfPoints();

		mafSmartPointer<mafVMESurface> outVmeSurfHighRes = m_SurfVMEList[i];

		outVmeSurfHighRes->SetData(outSurfHighRes , outVmeSurfHighRes->GetTimeStamp());
		/*
		if (m_GroupNodeMap.count(pPartNode) == 0)
		{
				 mafSmartPointer<mafVMEGroup> gPartDeformed;
				 std::string nameGroupDeformed = std::string(pPartNode->GetName());
				gPartDeformed->SetName(nameGroupDeformed.c_str());
				gDeformedHighRes->AddChild(gPartDeformed);
				gPartDeformed->SetParent(gDeformedHighRes.GetPointer());
				m_GroupNodeMap[pPartNode] = gPartDeformed;
		} 

		m_GroupNodeMap[pPartNode]->AddChild(outVmeSurfHighRes);
		outVmeSurfHighRes->SetParent(m_GroupNodeMap[pPartNode]);
		m_OutSurfaceHighRes.push_back(outVmeSurfHighRes);
		*/

	}

	
	// generate group nodes
	mafLogMessage("Generating region nodes and group nodes");
	std::map<mafNode * , vtkMAFSmartPointer<  vtkAppendPolyData> > newGroupMap;
	vtkAppendPolyData * pNewGroupPoly = NULL;
	tGroupNodeMap::iterator iterGroupMap;

	for  (iterGroupMap = m_GroupNodeMap.begin(); iterGroupMap != m_GroupNodeMap.end(); iterGroupMap++)
	{
		mafNode * pChild = iterGroupMap->first;
		mafNode * pParent = iterGroupMap->second;

		if (! strcmp(pParent->GetName(), "LeftFoot") )
			mafLogMessage(" ");

		if (newGroupMap.find(pParent) == newGroupMap.end())
		{
			vtkMAFSmartPointer<vtkAppendPolyData> newGroupPoly;
			newGroupMap[pParent] = newGroupPoly;
			pNewGroupPoly = newGroupPoly.GetPointer();
		}
		else
		{
			pNewGroupPoly = newGroupMap[pParent].GetPointer();
		}

		mafVMESurface * pChildSurf = mafVMESurface::SafeDownCast(pChild);
		pChildSurf->GetOutput()->Update();
		vtkPolyData * pPoly = vtkPolyData::SafeDownCast(pChildSurf->GetOutput()->GetVTKData());

		pNewGroupPoly->AddInput(pPoly);

	}

	MatSurfLdOutput();
	
	std::map<mafNode * , vtkMAFSmartPointer<  vtkAppendPolyData> >::iterator iterNewGroupMap;
	for (iterNewGroupMap = newGroupMap.begin(); iterNewGroupMap != newGroupMap.end(); iterNewGroupMap++)
	{
		mafNode * pGroup = iterNewGroupMap->first;
		vtkAppendPolyData * pAppend =  iterNewGroupMap->second;
		mafVMESurface * pGroupVme = mafVMESurface::SafeDownCast(pGroup);
		
		pAppend->GetOutput()->Update();
		vtkPolyData * pPoly = vtkPolyData::SafeDownCast(pAppend->GetOutput());

		mafVME * pParent = pGroupVme->GetParent();
		
		// temp, add inverse mat handling to group nodes

		if (pPoly)
		//if (pPoly && pGroupVme && pParent && !strcmp(pGroupVme->GetName(), "Bones")  
		//	&&  !strcmp(pParent->GetName(), "Pelvis") )
		{
			mafMatrix mat;
			pGroupVme->GetOutput()->GetAbsMatrix(mat, pGroupVme->GetTimeStamp() );
			// mulitply the inverse matrix
			mafTransform transform;
			transform.SetMatrix(mat);  
			transform.Invert();

			double p[3];
			int  nPoint =  pPoly->GetNumberOfPoints();

			vtkMAFSmartPointer<vtkPoints> pOutPoints;
			pOutPoints->SetNumberOfPoints(nPoint);
			for (int i = 0; i < nPoint; i++)
			{      
				transform.TransformPoint(pPoly->GetPoint(i), p);
				pOutPoints->SetPoint(i, p);
			}
			pPoly->SetPoints(pOutPoints);
			pGroupVme->SetData(pPoly, pGroupVme->GetTimeStamp());

		}
		else 
		{
			//pGroupVme->SetData(pAppend->GetOutput(), pGroupVme->GetTimeStamp());
		}
		


	}

	

	
	mafEventMacro(mafEvent(this, CAMERA_UPDATE));

}




bool lhpOpScaleMMA::IsClose(mafVMELandmarkCloud * pCloudVme, vtkPolyData * pMuscle, double threshold)
{
	pCloudVme->GetOutput()->Update();
	//pMuscleVme->GetOutput()->Update();

	vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pCloudVme->GetOutput()->GetVTKData());

	pLandmarkPoints->Update();
	pMuscle->Update();

	// check if there are intersections of their bouding boxes
	double b1[6], b2[6];
	
	pLandmarkPoints->GetBounds(b1);
	pMuscle->GetBounds(b2);

	if ((b1[1] < b2[0] - threshold) || (b1[3] < b2[2] - threshold) || (b1[5] < b2[4] - threshold))
		return false;

	if ((b2[1] < b1[0] - threshold) || (b2[3] < b1[2] - threshold) || (b2[5] < b1[4] - threshold))
		return false;

	return true;

}




double lhpOpScaleMMA::AverageDistance(mafVMELandmarkCloud * pCloudVme, vtkPolyData* pMuscle)
{
	pCloudVme->GetOutput()->Update();
	//pMuscleVme->GetOutput()->Update();

	vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pCloudVme->GetOutput()->GetVTKData());
	// calculate average closest distance of the landmark points to the muscle
	double p1[3], p2[3], dist, avgDist = 0;

	vtkMAFSmartPointer<vtkPointLocator>  pLocator;

	pLocator->SetDataSet(pMuscle);
	pLocator->SetNumberOfPointsPerBucket(10);
	pLocator->BuildLocator();

	int nLandmarks = pLandmarkPoints->GetNumberOfPoints();

	for (int i = 0; i < nLandmarks; i++)
	{
		pLandmarkPoints->GetPoint(i, p1);

		vtkIdType pointId = pLocator->FindClosestPoint(p1);

		if (pointId == -1)
			dist = 1e5; // a large value
		else {
			pMuscle->GetPoint(pointId, p2);
			dist = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
		}
		avgDist +=  dist;
	}

	return avgDist / nLandmarks;
}

double lhpOpScaleMMA::PtSurfDistance(mafVMELandmarkCloud * pCloudVme, int ptIdx, vtkPolyData* pMuscle, int & pointId)
{
	pCloudVme->GetOutput()->Update();

	vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pCloudVme->GetOutput()->GetVTKData());
	// calculate average closest distance of the landmark points to the muscle
	double p1[3], p2[3], dist, avgDist = 0;

	vtkMAFSmartPointer<vtkPointLocator>  pLocator;

	pLocator->SetDataSet(pMuscle);
	pLocator->SetNumberOfPointsPerBucket(10);
	pLocator->BuildLocator();

	int nLandmarks = pLandmarkPoints->GetNumberOfPoints();

	//for (int i = 0; i < nLandmarks; i++)
	//{
		pLandmarkPoints->GetPoint(ptIdx, p1);

		pointId = pLocator->FindClosestPoint(p1);

		if (pointId == -1)
			dist = 1e5; // a large value
		else {
			pMuscle->GetPoint(pointId, p2);
			dist = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
		}
		//avgDist +=  dist;
	//}

	//return avgDist / nLandmarks;
	return dist;
}

/*
double lhpOpScaleMMA::ClosestDistance(mafVMELandmarkCloud * pCloudVme, vtkPolyData* pMuscle)
{
	pCloudVme->GetOutput()->Update();
	//pMuscleVme->GetOutput()->Update();

	vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pCloudVme->GetOutput()->GetVTKData());
	// calculate average closest distance of the landmark points to the muscle
	double p1[3], p2[3], dist, avgDist = 0;

	vtkMAFSmartPointer<vtkPointLocator>  pLocator;

	pLocator->SetDataSet(pMuscle);
	pLocator->SetNumberOfPointsPerBucket(10);
	pLocator->BuildLocator();

	int nLandmarks = pLandmarkPoints->GetNumberOfPoints();

	for (int i = 0; i < nLandmarks; i++)
	{
		pLandmarkPoints->GetPoint(i, p1);

		vtkIdType pointId = pLocator->FindClosestPoint(p1);

		if (pointId == -1)
			dist = 1e5; // a large value
		else {
			pMuscle->GetPoint(pointId, p2);
			dist = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
		}
		avgDist +=  dist;
	}

	return avgDist / nLandmarks;
}
*/

void lhpOpScaleMMA::AddSourceSurface(int srcIdx)
{

	int nIsolatedCount = 0;

	mafString name = mafString(m_srcNameList[srcIdx].c_str());;

	vtkPolyData * pMesh = m_SurfList[srcIdx].GetPointer();
	pMesh->BuildCells();
	pMesh->BuildLinks();

	int nPoints = pMesh->GetNumberOfPoints();
	int nCells = pMesh->GetNumberOfCells();
	
	vtkIdType numCells = 0;
	//vtkIdType * cells;
	int npts;
	vtkIdType *	pts;

	// neighbour points of a vertex
	std::vector<int> neighb;
	double coord[3];
	double v;
	double surfWeight = m_WeightList[srcIdx];

	// starting row, as there are multiple muscles in the model
	int startIdx = m_srcStartIndexList[srcIdx] * 3;

	// Laplacian deformation
	// create matrix
	// for every vertex create the entry in the matrix
	vtkIdList *cellIds = vtkIdList::New();
	
	for (int ptIdx = 0; ptIdx < nPoints; ptIdx++) 
	{
		//cellIds->Reset();
		//vtkIdType numCells = pLinks->GetNcells(ptIdx);
		pMesh->GetPointCells(ptIdx, cellIds);
		numCells = cellIds->GetNumberOfIds();

		neighb.clear();
		// for each cell
		for (int iCell=0; iCell < numCells; iCell++)
		{

			pMesh->GetCellPoints(cellIds->GetId(iCell),npts,pts);

		// for each cell point
			if (npts <=2 ) {
				m_degen++;
				//continue;
			}

			// npts 
			for (int iCellPoint=0; iCellPoint < npts; iCellPoint++)
			{
					// found the neighboring edge
					if ( pts[iCellPoint] == ptIdx) 
					{
						neighb.push_back(pts[(iCellPoint +1) % npts]);

						break;
					}

			}
		}
		

		// no duplicates
		//std::vector<int>::iterator end_unique = unique(neighb.begin(), neighb.end());
		//neighb.erase(end_unique, neighb.end());

		// the order of the neighours are not garanteed
		int nei_num = neighb.size();

		//assert (nei_num != 0);

		if (m_maxN < nei_num)
			m_maxN = nei_num;

		if (m_minN > nei_num)
			m_minN = nei_num;

		if (nei_num <=2)
			m_isolatedCount++;


		// nacer's code
		// v: weight for neighbouring vertices
		if (nei_num != 0)
			v= 1.0/nei_num * surfWeight;

		////////// DO THE A MATRIX ////////
		/////// create the x line ///
		///firstly, write the diagonal element 
		cs_entry (m_T, m_nRow,  startIdx + ptIdx * 3, -surfWeight);
		/// secondly, fill in the rest non-zero columns 
		for(int m=0;m<nei_num; m++)
		{
			cs_entry (m_T, m_nRow, startIdx + neighb[m]*3, v);
		}
		m_nRow++;

		//// create the y line ///
		///firstly, write the diagonal element 
		cs_entry (m_T, m_nRow,  startIdx + ptIdx * 3 + 1, -surfWeight);
		/// secondly, fill in the rest non-zero columns 
		for(int m=0; m<nei_num; m++){
			cs_entry (m_T, m_nRow, startIdx + neighb[m]*3+1, v);
		}
		m_nRow++;

		//// create the z line ///
		///firstly, write the diagonal element 
		cs_entry (m_T, m_nRow, startIdx + ptIdx * 3 + 2, -surfWeight);
		/// secondly, fill in the rest non-zero columns 
		for(int m=0; m<nei_num; m++)
		{
			cs_entry (m_T, m_nRow, startIdx + neighb[m]*3+2, v);
		}
		m_nRow++;


		//////////DO THE B MATRIX ////////////////////

		float bx=0, by=0, bz=0;
		double center[3];
		pMesh->GetPoint(vtkIdType(ptIdx), center);

		m_dupCount = 0; 
		for(int m=0; m<nei_num; m++)
		{
			int neighbId = neighb[m];
			pMesh->GetPoint(vtkIdType(neighb[m]), coord);

			double dist2 = vtkMath::Distance2BetweenPoints (coord, center);
			m_totalPoint++;
			if ( dist2< 1e-4)
			{
				//mafLogMessage("Close neighbour points found %d %d : %lf", ptIdx, neighbId, dist2 );
				 m_dupCount++;
			}

			bx += coord[0] * v;
			by += coord[1] * v;
			bz += coord[2] * v;
		}

		if (m_dupCount >=2)
		{
			mafLogMessage("%d close neighbour points found for point %d", m_dupCount, ptIdx );
		}

		pMesh->GetPoint(vtkIdType(ptIdx), coord);

		bx -= coord[0] * surfWeight;
		by -= coord[1] * surfWeight;
		bz -= coord[2] * surfWeight;

		bx*=m_Scale[0];
		by*=m_Scale[1];
		bz*=m_Scale[2];

		m_bvec.push_back(bx);
		m_bvec.push_back(by);
		m_bvec.push_back(bz);

	 } // for all points

	 cellIds->Delete();

	 if (nIsolatedCount)
		mafLogMessage(_T("Isolated points: %d"), nIsolatedCount);

}


// adding attachment constraint
// srdIdx: muscle Idx
void lhpOpScaleMMA::AddAttachmentConstraint()
{

	//int startIdx = m_srcStartIndexList[srcIdx] * 3;

	for (int i = 0; i <m_nAttach; i++)
	{
		// append landmarks to the muscles
		int attachStartIdx = m_AttachStartIndexList[i]* 3;
		mafVMELandmarkCloud * pLandmark = m_AttachList[i];
		vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pLandmark->GetOutput()->GetVTKData());

		// set starting row to the current row ( after lapaician rows for the current model)
		int rStart = m_nRow;
		double  point[3];
		POINT3 closestPoint;

		// landmark peer id list
		tPointPairList   closePairList = m_LandmarkPeerId[i];
		std::vector<POINT3> closePointList = m_LandmarkPeer[i];;

		double weight = 1.0;

		// add every point of the landmark
		for (int j = 0; j < closePairList.size(); j++)
		//for (int ptIdx = 0; ptIdx < 1; ptIdx++)
		{
			// get coordinates of closest point (landmark peers) on the muscle 
			//vtkIdType closestPointId = (vtkIdType) closestpId.at(ptIdx);
			int landmarkIdx = attachStartIdx + closePairList[j].first * 3;
			int closePointIdx = closePairList[j].second * 3;

			// get coordinates of landmark points on the bones
			pLandmarkPoints->GetPoint(closePairList[j].first, point);

			closestPoint = closePointList[j]; 

			cs_entry (m_T, m_nRow,  landmarkIdx, -1.0);
			cs_entry (m_T, m_nRow,  closePointIdx, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,   landmarkIdx  + 1, -1.0);
			cs_entry (m_T, m_nRow,   closePointIdx + 1, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,  landmarkIdx  + 2, -1.0);
			cs_entry (m_T, m_nRow,  closePointIdx + 2, 1.0 * weight);
			m_nRow++;

			double point1[3];
			point1[0] = closestPoint.x;
			point1[1] = closestPoint.y;
			point1[2] = closestPoint.z;

			double dist = vtkMath::Distance2BetweenPoints(point,point1);
			if (dist > 64.0)
				mafLogMessage("distance %d", (int) dist);
			
			m_bvec.push_back( (- point[0] + closestPoint.x* weight ) * m_Scale[0]);
			m_bvec.push_back( (- point[1] + closestPoint.y * weight ) * m_Scale[1]);
			m_bvec.push_back( (- point[2] + closestPoint.z * weight ) * m_Scale[2]);

		}
	} // for all attachment areas

}

 void lhpOpScaleMMA::AddMotionLsConstraint()
 {
	 int idxTotal = 0;

	 for (int i = 0; i <m_nMotionLs; i++)
	{
		// append landmarks to the muscles
		int startIdx = m_MotionLsStartIndexList[i]* 3;
		
		mafVMELandmarkCloud * pLandmark = m_MotionLsList[i];
		int nLandmark = pLandmark->GetNumberOfLandmarks();

		vtkPointSet * pLandmarkPoints = vtkPointSet::SafeDownCast(pLandmark->GetOutput()->GetVTKData());

		// set starting row to the current row ( after lapaician rows for the current model)
		int rStart = m_nRow;
		double  point[3];
		double closestPoint[3];

		double weight = 1.0;

		// add every point of the landmark
		for (int j = 0; j <  nLandmark; j++)
		//for (int ptIdx = 0; ptIdx < 1; ptIdx++)
		{
			LdMap motionLdMap = m_MotionLsMap[idxTotal];
			int idLd = motionLdMap.LdId;
			int idSurf = motionLdMap.SurfId;
			int  idPt = motionLdMap.PtId;

			assert(j == idLd);
			
			// get coordinates of landmark points on the bones
			pLandmarkPoints->GetPoint(j, point);

			// get coordinates of closest point (landmark peers) on the muscle 
			//vtkIdType closestPointId = (vtkIdType) closestpId.at(ptIdx);
			int landmarkIdx = startIdx + idLd * 3;
			
			// the bone surface point id in the matrix
			int closePointIdx =  (m_srcStartIndexList[m_nMuscle + idSurf] + idPt) * 3;


			// TODO
			vtkMAFSmartPointer<vtkPolyData>  pBone = m_SurfList[m_nMuscle + idSurf];
			pBone->GetPoint(idPt, closestPoint);
			// double * closestPoint = pBone->GetPoint(idPt);

			cs_entry (m_T, m_nRow,  landmarkIdx, -1.0);
			cs_entry (m_T, m_nRow,  closePointIdx, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,   landmarkIdx  + 1, -1.0);
			cs_entry (m_T, m_nRow,   closePointIdx + 1, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,  landmarkIdx  + 2, -1.0);
			cs_entry (m_T, m_nRow,  closePointIdx + 2, 1.0 * weight);
			m_nRow++;

			//double point1[3];
			//point1[0] = closestPoint.x;
			//point1[1] = closestPoint.y;
			//point1[2] = closestPoint.z;

			
			double dist = vtkMath::Distance2BetweenPoints(point,closestPoint);
			if (dist > 64.0)
				mafLogMessage("distance %d", (int) dist);
			
			m_bvec.push_back( (- point[0] + closestPoint[0]* weight ) * m_Scale[0]);
			m_bvec.push_back( (- point[1] + closestPoint[1] * weight ) * m_Scale[1]);
			m_bvec.push_back( (- point[2] + closestPoint[2] * weight ) * m_Scale[2]);

			idxTotal ++;

		}
	} // for all attachment areas

 }


void  lhpOpScaleMMA::AddLandmarkConstraint()
{
	static double weight = 100.0;
	static double ratio = 1.5;
	double point[3], peer[3], target[3];

	int ldSize = m_LdValid.size();
	//assert(ldSize == 6);
	assert( m_LdPeerSurfId.size() == ldSize);
	assert( m_LdPeerPointId.size() == ldSize);
	assert(m_LdSource->GetNumberOfPoints() == ldSize);
	assert(m_LdTarget->GetNumberOfPoints() == ldSize);
	assert(m_LdPeer->GetNumberOfPoints() == ldSize);

	for (int i = 0; i < ldSize; i++)
	{
		// add femur head landmark
		int surfId = m_LdPeerSurfId[i];
		int ptId = m_LdPeerPointId[i];
		int startIdx = m_srcStartIndexList[surfId] * 3;
		m_LdTarget->GetPoint(i, target);

		if (!m_LdValid[i] )
			continue;

		if (m_UseOuterLd[i])
		{
			// landmark pairs

			int landmarkIdx = m_nRow;

			// add landmark 
			cs_entry (m_T, m_nRow,  landmarkIdx, 1.0 * weight);
			m_nRow++;
			cs_entry (m_T, m_nRow,  landmarkIdx + 1, 1.0 * weight);
			m_nRow++;
			cs_entry (m_T, m_nRow,  landmarkIdx + 2, 1.0 * weight);
			m_nRow++;
			
			// add landmark peer
			// TODO: weight
			cs_entry (m_T, m_nRow,  landmarkIdx, -1.0 * weight);
			cs_entry (m_T, m_nRow,  startIdx + ptId * 3, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,   landmarkIdx  + 1, -1.0 * weight);
			cs_entry (m_T, m_nRow,  startIdx + ptId * 3 + 1, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,  landmarkIdx  + 2, -1.0 * weight);
			cs_entry (m_T, m_nRow,  startIdx + ptId * 3 + 2, 1.0 * weight);
			m_nRow++;

			m_LdSource->GetPoint(i, point);
			m_LdPeer->GetPoint(i, peer);

			// target position of the landmark
			m_bvec.push_back(target[0] * weight);
			m_bvec.push_back(target[1] * weight);
			m_bvec.push_back(target[2] * weight);

			m_bvec.push_back( (- point[0] + peer[0]) * weight);
			m_bvec.push_back( (- point[1] + peer[1]) * weight);
			m_bvec.push_back( (- point[2] + peer[2]) * weight);
		}
		else 
		{
			cs_entry (m_T, m_nRow,  startIdx + ptId * 3, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,  startIdx + ptId * 3 + 1, 1.0 * weight);
			m_nRow++;

			cs_entry (m_T, m_nRow,  startIdx + ptId * 3 + 2, 1.0 * weight);
			m_nRow++;

			m_bvec.push_back(target[0] * weight);
			m_bvec.push_back(target[1] * weight);
			m_bvec.push_back(target[2] * weight);
		}

	}
}

void lhpOpScaleMMA::CalcLandmarkConstraint()
{
	if (! m_pGraph)
		return;

	// femur head
	LocateFemurHead(LEFT);
	LocateFemurHead(RIGHT);

	// Patella
	CalcPatellaLandmark(LEFT);
	CalcPatellaLandmark(RIGHT);

	CalcAnkleLandmark(LEFT);
	CalcAnkleLandmark(RIGHT);

	// calculate scale factor
	double pointFemur[3], pointAnkle[3];;
	double pointFemur1[3], pointAnkle1[3];;
	double deltaSrc[3], deltaTarget[3];

	m_LdSource->GetPoint(0, pointFemur);
	m_LdSource->GetPoint(2, pointAnkle);
	deltaSrc[2] = pointFemur[2] - pointAnkle[2];

	m_LdSource->GetPoint(1, pointFemur1);
	m_LdSource->GetPoint(3, pointAnkle1);
	deltaSrc[2] += pointFemur1[2] - pointAnkle1[2];
	deltaSrc[0] = pointFemur[0] - pointFemur1[0];


	m_LdTarget->GetPoint(0, pointFemur);
	m_LdTarget->GetPoint(2, pointAnkle);
	deltaTarget[2] = pointFemur[2] - pointAnkle[2];

	m_LdTarget->GetPoint(1, pointFemur1);
	m_LdTarget->GetPoint(3, pointAnkle1);
	deltaTarget[2] += pointFemur1[2] - pointAnkle1[2];
	deltaTarget[0] = pointFemur[0] - pointFemur1[0];

	m_Scale[2] = fabs(deltaTarget[2] / deltaSrc[2]); 
	m_Scale[0] = fabs(deltaTarget[0] / deltaSrc[0]); 
	m_Scale[1] = (m_Scale[2] + m_Scale[0])/2.0;

}

void  lhpOpScaleMMA::LocateFemurHead( BODY_DIR dir)
{

	//pFemur->GetOutput()->Update();
	assert(m_FemurIdx[dir] >= 0);

	vtkMAFSmartPointer<vtkPolyData > pMesh = m_SurfList[m_FemurIdx[dir] ];
	
	if (NULL == pMesh)
		return;

	double bb[6];
	double center[3];
	double dim[3];


	pMesh->GetBounds(bb);
	pMesh->GetCenter(center);

	for (int i = 0; i < 3; i++)
		dim[i] = bb[i*2+1] - bb[i*2];

	double headPos[3];
	// top 10% and in the inner half

	const double eps = 1e-5;
	// brute force searching
	double point[3], upper[3], inner[3], front[3];
	vtkIdType nPoints = pMesh->GetNumberOfPoints();
	double delta[3], deltaOld[3];
	int id[3]; // upper inner front poit ids

	for (int i = 0; i < 3; i++)
		deltaOld[i] = std::numeric_limits<double>::max();

	for (int i = 0; i < nPoints; i++)
	{
		pMesh->GetPoint(i, point);
		// upper 
		delta[2] = abs(point[2] - bb[5]);

		if (delta[2] > dim[2] * 0.2)
			continue;

		delta[1]= abs(point[1] - bb[2]);

		// left is in the X direction
		if (LEFT== dir) 
			delta[0] = abs(point[0] - bb[0]);
		else
			delta[0]= abs(point[0] - bb[1]);

		
		if (delta[0] < deltaOld[0]) 
		{
			for (int j = 0; j < 3; j++)
				inner[j] = point[j];

			deltaOld[0] = delta[0];
			id[0] = i;
		}

		if (delta[1] < deltaOld[1]) 
		{
			for (int j = 0; j < 3; j++)
				front[j] = point[j];

			deltaOld[1] = delta[1];

			id[1] = i;
		}

		if (delta[2] < deltaOld[2]) 
		{
			for (int j = 0; j < 3; j++)
				upper[j] = point[j];

			deltaOld[2] = delta[2];

			id[2] = i;
		}
		
	}

	mafLogMessage("Delta: %f %f %f\n", delta[0], delta[1], delta[2]);
	headPos[0] = (upper[0] + front[0])/ 2.0;
	headPos[1] = (upper[1] + inner[1])/ 2.0;
	headPos[2] = (front[2] + inner[2])/ 2.0;


	m_UseOuterLd.push_back(true);
	m_LdSource->InsertNextPoint(headPos);
	m_LdPeer->InsertNextPoint(upper);
	m_LdPeerPointId.push_back(id[2]); // use the upper id

	m_LdPeerSurfId.push_back(m_FemurIdx[dir]);
	/*
	for (int i = 0; i < m_SurfList.size(); i++)
	{
		if (m_SurfVMEList[i] == pFemur)
		{
			m_LdPeerSurfId.push_back(i);
			break;
		}
	}
	*/

	// temporary
	
	double target[3];
	int ldFound = MAF_ERROR;

	if (m_LandmarkPatient )
	{
		if (dir == LEFT)
			ldFound = m_LandmarkPatient->GetLandmark("LeftFemur", target);
		else 
			ldFound = m_LandmarkPatient->GetLandmark("RightFemur", target);
	}
	
	if (ldFound == MAF_ERROR)
	{
			const double ratio = 1.0;
			for (int i = 0; i < 3; i++)
				target[i] = headPos[i] * ratio;
	}

	for (int i = 0; i < 3; i ++)
		target[i] *= m_LdScale;
	
	m_LdValid.push_back(ldFound == MAF_OK);

	m_LdTarget->InsertNextPoint(target);
	

}

void lhpOpScaleMMA::CalcAnkleLandmark(BODY_DIR dir)
{
	//mafVMESurface * pVme = m_pAnkle[dir];

	assert(m_AnkleIdx[dir] >= 0);

	vtkMAFSmartPointer<vtkPolyData > pMesh = m_SurfList[m_AnkleIdx[dir] ];
	
	if (NULL == pMesh)
		return;

	double bb[6];
	pMesh->GetBounds(bb);

	const double eps = 1e-5;
	double lower[3], point[3];
	double delta, deltaOld;
	int pointId; // upper inner front poit ids

	vtkIdType nPoints = pMesh->GetNumberOfPoints();

	deltaOld = std::numeric_limits<double>::max();

	// brute force searching
	for (int i = 0; i < nPoints; i++)
	{
		pMesh->GetPoint(i, point);

		delta = abs(point[2] - bb[4]);

		if (delta < deltaOld) 
		{
			for (int j = 0; j < 3; j++)
				lower[j] = point[j];

			deltaOld = delta;

			pointId = i;
		}
		
	}

	m_UseOuterLd.push_back(false);
	m_LdSource->InsertNextPoint(0, 0, 0);
	m_LdPeer->InsertNextPoint(lower);

	m_LdPeerSurfId.push_back(m_AnkleIdx[dir]);
	/*
	for (int i = 0; i < m_SurfList.size(); i++)
	{
		if (m_SurfVMEList[i] == pVme)
		{
			m_LdPeerSurfId.push_back(i);
			break;
		}
	}
	*/
	m_LdPeerPointId.push_back(pointId); // use the upper id

	// temporary
	double target[3];

	int ldFound = MAF_ERROR;

	if (m_LandmarkPatient )
	{
		if (dir == LEFT)
			ldFound = m_LandmarkPatient->GetLandmark("LeftAnkle", target);
		else 
			ldFound = m_LandmarkPatient->GetLandmark("RightAnkle", target);
	}
	
	if (ldFound == MAF_ERROR)
	{
			const double ratio = 1.0;
			for (int i = 0; i < 3; i++)
				target[i] = lower[i] * ratio;
	}
	
	for (int i =0; i < 3; i ++)
		target[i] *= m_LdScale;

	m_LdValid.push_back(ldFound == MAF_OK);
	m_LdTarget->InsertNextPoint(target);

}

void lhpOpScaleMMA::CalcPatellaLandmark(BODY_DIR dir)
{

	assert(m_PatellaIdx[dir] >= 0);

	// calc barycenter
	vtkMAFSmartPointer<vtkPolyData > pMesh = m_SurfList[m_PatellaIdx[dir] ];

	int nPoints = pMesh->GetNumberOfPoints();
	double point[3], center[3];
	for (int k = 0; k < 3; k++)
		center[k] = 0;

	for (int i = 0; i < nPoints; i++) 
	{
		pMesh->GetPoint(i, point);
		for (int k = 0; k < 3; k++)
			center[k] += point[k];
	}

	for (int k = 0; k < 3; k++)
		center[k] /= nPoints;

	m_UseOuterLd.push_back(true);
	m_LdSource->InsertNextPoint(center);

	m_LdPeerSurfId.push_back(m_PatellaIdx[dir]);

	/*
	for (int i = 0; i < m_SurfList.size(); i++)
	{
		if (m_SurfVMEList[i] == pVme)
		{
			m_LdPeerSurfId.push_back(i);
			break;
		}
	}
	*/
	// find the closest point as the peer

	vtkMAFSmartPointer<vtkPointLocator>  pLocator;

	pLocator->SetDataSet(pMesh.GetPointer());
	pLocator->SetNumberOfPointsPerBucket(10);
	pLocator->BuildLocator();

	vtkIdType pointId = pLocator->FindClosestPoint(center);

	if (pointId == -1)
		pointId = 0;

	double peer[3];
	pMesh->GetPoint(pointId, peer);

	m_LdPeer->InsertNextPoint(peer);
	m_LdPeerPointId.push_back(pointId); // use the upper id

	// temporary
	double target[3];
	int ldFound = MAF_ERROR;

	if (m_LandmarkPatient )
	{
		if (dir == LEFT)
			ldFound = m_LandmarkPatient->GetLandmark("LeftKnee", target);
		else 
			ldFound = m_LandmarkPatient->GetLandmark("RightKnee", target);
	}
	
	if (ldFound ==MAF_ERROR)
	{
			const double ratio = 1.0;
			for (int i = 0; i < 3; i++)
				target[i] = center[i] * ratio;
	}

	for (int i = 0; i < 3; i ++)
		target[i] *= m_LdScale;

	m_LdValid.push_back(ldFound == MAF_OK);
	 m_LdTarget->InsertNextPoint(target);
}

// todo

void lhpOpScaleMMA::CalcAttachmentConstraint(std::vector<std::vector<int> > & surfAttachMap, 
	std::vector<vtkMAFSmartPointer<vtkPolyData>  > & surfList, int startIdxStart)
{
	//Create the tree
	vtkMAFSmartPointer<vtkPointLocator>  pLocator;
	//vtkPointLocator *  pLocator = vtkPointLocator::New(); // = 	vtkMAFSmartPointer<vtkPointLocator>::New();

	vtkPolyData * pMesh = NULL;
	mafVMELandmarkCloud * pLandmark = NULL;
	vtkPointSet * pLandmarkPoints = NULL;
	vtkIdType pointId;
	double inPoint[3], outPoint[3];

	// one muscle may have muliple landmark sets
	for (int srcIdx = 0; srcIdx < surfList.size(); srcIdx++)
	{
		//surfList[srcIdx]->GetOutput()->Update();
		//pMesh = vtkPolyData::SafeDownCast(surfList[srcIdx]->GetOutput()->GetVTKData());
		pMesh = surfList[srcIdx].GetPointer();

		pLocator->SetDataSet(pMesh);
		pLocator->SetNumberOfPointsPerBucket(10);
		pLocator->BuildLocator();

		// add landmarks
		//vtkPoints *  closestp = vtkPoints::New();
		//std::vector<int> closestpId;
		int mapsize = surfAttachMap[srcIdx].size();
		int startIdx = m_srcStartIndexList[startIdxStart + srcIdx];

		for (int j = 0; j < surfAttachMap[srcIdx].size(); j++)
		{
			// source attachement landmark points
			
			int attachId = surfAttachMap[srcIdx].at(j);
			pLandmark = m_AttachList[ attachId ];

			// landmark points of srcIdx muscle
			pLandmarkPoints = vtkPointSet::SafeDownCast(pLandmark->GetOutput()->GetVTKData());

			//closestp->SetNumberOfPoints(pLandmarkPoints->GetNumberOfPoints());

			// for every landmark point, find its closest point on the muscle mesh
			for (int ptIdx = 0; ptIdx < pLandmarkPoints->GetNumberOfPoints(); ptIdx++)
			{
				pLandmarkPoints->GetPoint(ptIdx, inPoint);

				//pLocator->FindClosestPoint(inPoint, outPoint, cell, cell_id, sub_id, dist2);
				pointId = pLocator->FindClosestPoint(inPoint);
				if (-1 != pointId) 	{
					//closestpId.push_back(pointId);
					pMesh->GetPoint(pointId, outPoint);

					double dist = vtkMath::Distance2BetweenPoints(inPoint,outPoint);
					if (dist > 64.0)
						mafLogMessage("distance %d", (int)dist);
					//closestp->InsertNextPoint( outPoint);
					
					// startIndex is included TODO
					m_LandmarkPeerId[attachId].push_back(std::pair<int, int> (ptIdx, startIdx + pointId));
					m_LandmarkPeer[attachId].push_back(POINT3(outPoint));

				} else {
					// TODO
				}

			}
		}


	}


}


void lhpOpScaleMMA::CalcAttachmentConstraint()
{
	m_LandmarkPeerId.resize(m_nAttach);
	m_LandmarkPeer.resize(m_nAttach);

	CalcAttachmentConstraint(m_MuscleAttachMap, m_MuscleListClean, 0);
	CalcAttachmentConstraint(m_BoneAttachMap, m_BoneListClean, m_nMuscleClean);
	CalcAttachmentConstraint(m_LigamentAttachMap, m_LigamentListClean, m_nMuscleClean + m_nBoneClean);

}

void lhpOpScaleMMA::CalcInterSurfConstraint(const std::vector<vtkMAFSmartPointer<vtkPolyData> > & surfList)
{

	if (surfList.empty())
		return;

	int nList = surfList.size();

	// initialisation the index pair list array
	m_interSurfConstraintList.resize(nList * nList); 

	for (int i = 0; i < nList * nList; i++)
		m_interSurfConstraintList[i] = NULL;


	vtkPolyData * m1 = NULL, * m2 = NULL;


	for (int i = 0; i < nList; i++)
	{
		m1 = surfList[i].GetPointer();

		for (int j = i + 1; j < nList; j++)
		{
			m2 =  surfList[j].GetPointer();;

			tPointPairList * pIndexPairList = FindClosePoints(m1, m2, 10.0, 100);

			if (!pIndexPairList->empty())
				mafLogMessage(_T("Closest poitns: %d  %s %d %s %d"),  i, m_srcNameList[i].c_str(), j, m_srcNameList[j].c_str(), pIndexPairList->size());
			//if (!pIndexPairList->empty())
			//	mafLogMessage(_T("Closest poitns: %d  %d %d"), i, j, pIndexPairList->size());

			m_interSurfConstraintList[i * nList + j] = pIndexPairList;

		}
	}
}

void lhpOpScaleMMA::AddInterMuscleConstraint(int idx1, int idx2)
{

	int nSurf = m_SurfList.size();
	tPointPairList * pIndexPairList = m_interSurfConstraintList[idx1 * nSurf + idx2];

	vtkPolyData * m1 = m_SurfList[idx1].GetPointer();

	vtkPolyData * m2 = m_SurfList[idx2].GetPointer();

	double p1[3], p2[3];

	int startIdx1 = m_srcStartIndexList[idx1] * 3;
	int startIdx2 = m_srcStartIndexList[idx2] * 3;

	int pointIdx1, pointIdx2;

	int numPairs = pIndexPairList->size();

	for (int i = 0; i < numPairs ; i++)
	{
		//tPointPairList * pPairList = pIndexPairList[i];

		//for (int j = 0; j < pPairList->size(); j++)
		//{
			tIndexPair idxPair = (*pIndexPairList)[i];

			pointIdx1 = idxPair.first;
			pointIdx2 = idxPair.second;

			// adding one row in the matrix
			cs_entry (m_T, m_nRow,  startIdx1 + pointIdx1 * 3, -1.0);
			cs_entry (m_T, m_nRow,  startIdx2 + pointIdx2 * 3, 1.0);
			m_nRow++;

			cs_entry (m_T, m_nRow,  startIdx1 + pointIdx1 * 3 + 1, -1.0);
			cs_entry (m_T, m_nRow,  startIdx2 + pointIdx2 * 3 + 1, 1.0);
			m_nRow++;

			cs_entry (m_T, m_nRow,  startIdx1 + pointIdx1 * 3 + 2, -1.0);
			cs_entry (m_T, m_nRow,  startIdx2 + pointIdx2 * 3 + 2, 1.0);
			m_nRow++;

			m1->GetPoint(pointIdx1, p1);
			m2->GetPoint(pointIdx2, p2);

			// bvec
			m_bvec.push_back(( - p1[0] + p2[0]) * m_Scale[0]);
			m_bvec.push_back((- p1[1] + p2[1] ) * m_Scale[1]);
			m_bvec.push_back((- p1[2] + p2[2]) * m_Scale[2]);
		//}

	}

}

// find closest points between two neighbouring muscles within the given threshold
lhpOpScaleMMA::tPointPairList *  lhpOpScaleMMA::FindClosePoints(vtkPolyData * m1, vtkPolyData * m2, double dist, int step)
{
	double b1[6], b2[6];
	
	m1->GetBounds(b1);
	m2->GetBounds(b2);

	tPointPairList * pCloseIdPairs = new tPointPairList;

	// no intersection between two bounding box
	if ((b1[1] < b2[0]) || (b1[3] < b2[2]) || (b1[5] < b2[4]))
		return pCloseIdPairs;

	if ((b2[1] < b1[0]) || (b2[3] < b1[2]) || (b2[5] < b1[4]))
		return pCloseIdPairs;

	
	// find the closest point in threshold d in m2 for every "step" point in m1
	vtkMAFSmartPointer<vtkPointLocator>  pLocator; 

	double inPoint[3], dist2;

	int numPoint1 = m1->GetNumberOfPoints();
	int numPoint2 = m2->GetNumberOfPoints();

	bool bReverse = false;

	if (numPoint1 > numPoint2 )
	{
		vtkPolyData * pTemp = m1;
		m1 = m2; 
		m2 = pTemp;

		int temp = numPoint1;
		numPoint1 = numPoint2;
		numPoint2 = temp;

		bReverse = true;
	}
	
	pLocator->SetDataSet(m2);
	pLocator->SetNumberOfPointsPerBucket(10);
	pLocator->BuildLocator();

	int interval  = numPoint1 / step;

	if (interval < 1)
		interval = 1;


	// for every landmark point, find its closest point on the muscle mesh
	for (int ptIdx = 0; ptIdx < numPoint1; ptIdx+=interval)
	{

		m1->GetPoint(ptIdx, inPoint);

		vtkIdType pointId = pLocator->FindClosestPointWithinRadius(dist, inPoint, dist2);
		
		if ( -1 != pointId)
			if (bReverse)
				pCloseIdPairs->push_back(tIndexPair(pointId, ptIdx));
			else
				pCloseIdPairs->push_back(tIndexPair(ptIdx, pointId));

	}

	int numPair = pCloseIdPairs->size();

	return pCloseIdPairs;
}



//----------------------------------------------------------------------------
void lhpOpScaleMMA::OpUndo()
//----------------------------------------------------------------------------
{

}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::OpStop(int result)
//----------------------------------------------------------------------------
{
	if (m_Gui)
		HideGui();

	if (OP_RUN_OK == result)
	{

	}
	else
	{
		
	}

	mafEventMacro(mafEvent(this,result));
}


//----------------------------------------------------------------------------
void lhpOpScaleMMA::OnEvent(mafEventBase * event)
//----------------------------------------------------------------------------
{

	if (mafEvent *e = mafEvent::SafeDownCast(event))
	{
		switch(e->GetId())
		{
		// template endocardium
		case ID_CHOOSE_MUSCLE:
			//OnChooseMuscle();
			break;
		// template epicardium
		case ID_CHOOSE_LANDMARK_MUSCLE:
			//OnChooseLandmarkMuscle();
			break;
		// loading gadolinium txt file
		case ID_CHOOSE_LANDMARK_PATIENT:
			OnChooseLandmarkPatient();
			break;

		case wxOK:
			Execute();
			OpStop(OP_RUN_OK);           
			break;
		case wxCANCEL:
			OpStop(OP_RUN_CANCEL);        
			break;
		default:
			mafEventMacro(*e);
			break; 
		}
	}
	
}

void lhpOpScaleMMA::OnChooseLandmarkPatient()
{
	mafEvent e(this,VME_CHOOSE);
	mafEventMacro(e);
	mafNode *vme = e.GetVme();

	if(!vme) return; // the user choosed cancel - keep previous target
	if(!AcceptLandmark(vme)) // the user choosed ok     - check if it is a valid vme
	{
		wxString msg = _("target vme must be a non-empty Surface\n please choose another vme \n");
		wxMessageBox(msg,_("incorrect vme type"),wxOK|wxICON_ERROR);
		m_LandmarkPatient = NULL;
		m_NameLandmarkPatient		= _("none");
		m_Gui->Enable(wxOK,false);
		m_Gui->Update();
		return;
	}

	m_LandmarkPatient			= mafVMELandmarkCloud::SafeDownCast(vme);
	m_NameLandmarkPatient = m_LandmarkPatient->GetName();

	int num = m_LandmarkPatient->GetNumberOfLandmarks();

	m_targetLandmarkList.push_back(m_LandmarkPatient);

	// todo
	//if (InputReady())
	m_Gui->Enable(wxOK,true);

	m_Gui->Update();
}


// generate the problem
problem *lhpOpScaleMMA::GenProblem (cs * T, std::vector<double> & bvec, double tol)
{
    cs *A, *C;
    int sym, m, n, mn, nz1, nz2;
    problem *Prob;

    Prob = (problem *)cs_calloc (1, sizeof (problem));

    if (!Prob) 
		return (NULL);

    //T = cs_load (f);				/* load triplet matrix T from a file */
    Prob->A = A = cs_triplet (T);	/* A = compressed-column form of T */
   // cs_spfree (T);					/* clear T */

    if (!cs_dupl (A)) 
		return (free_problem (Prob)) ; /* sum up duplicates */
    
	Prob->sym = sym = is_sym (A);	/* determine if A is symmetric */
    m = A->m ; n = A->n;
    mn = CS_MAX (m,n);
    nz1 = A->p [n];
    cs_dropzeros (A);				/* drop zero entries */
    nz2 = A->p [n];

    if (tol > 0) 
		cs_droptol (A, tol);		/* drop tiny entries (just to test) */

    Prob->C = C = sym ? make_sym (A) : A;  /* C = A + triu(A,1)', or C=A */

    if (!C)
		return (free_problem (Prob));

    mafLogMessage(_T("\n--- Matrix: %d-by-%d, nnz: %d (sym: %d: nnz %d), norm: %8.2e\n"),
	    m, n, A->p [n], sym, sym ? C->p [n] : 0, cs_norm (C));

    if (nz1 != nz2) 
		mafLogMessage(_T("zero entries dropped: %d\n"), nz1 - nz2);

    if (nz2 != A->p [n]) 
		mafLogMessage (_T("tiny entries dropped: %d\n"), nz2 - A->p [n]);

    Prob->b = (double *)cs_malloc (mn, sizeof (double));
    Prob->x = (double *)cs_malloc (mn, sizeof (double));
    Prob->r = (double *)cs_malloc (mn, sizeof (double));

	if ((!Prob->b || !Prob->x || !Prob->r) )
	{
		free_problem (Prob);
		return NULL;
	}

	for (int i = 0; i < Prob->A->m; ++i)
	{
		Prob->b[i] = bvec[i];  //1 + double (i)/Prob->A->m; //x;
	}

    return Prob;
}


/* free a problem */
problem * lhpOpScaleMMA::free_problem (problem *Prob)
{
    if (!Prob) return (NULL) ;
    cs_spfree (Prob->A) ;
    if (Prob->sym) cs_spfree (Prob->C) ;
    cs_free (Prob->b) ;
    cs_free (Prob->x) ;
    cs_free (Prob->r) ;
    return ( (problem *) cs_free (Prob)) ;
}


/* 1 if A is square & upper tri., -1 if square & lower tri., 0 otherwise */
static int is_sym ( cs *A )
{
    int is_upper, is_lower, j, p, n = A->n, m = A->m, *Ap = A->p, *Ai = A->i ;
    if (m != n) 
		return (0);

    is_upper = 1 ;
    is_lower = 1 ;
    
	for (j = 0 ; j < n ; j++)
    {
		for (p = Ap [j] ; p < Ap [j+1] ; p++)
		{
			if (Ai [p] > j) 
				is_upper = 0;

			if (Ai [p] < j) 
				is_lower = 0 ;
		}
    }

    return (is_upper ? 1 : (is_lower ? -1 : 0)) ;
}

/* true for off-diagonal entries */

static int dropdiag ( int i, int j, double aij, void *other ) 
{ 
  return (i != j);
}


/* C = A + triu(A,1)' */
static cs *make_sym ( cs *A )
{
    cs *AT, *C ;
    AT = cs_transpose (A, 1) ;		/* AT = A' */
    cs_fkeep (AT, &dropdiag, NULL) ;	/* drop diagonal entries from AT */
    C = cs_add (A, AT, 1, 1) ;		/* C = A+AT */
    cs_spfree (AT);
    return (C);
}

static double tic (void) 
{ 
  return (clock () / (double) CLOCKS_PER_SEC) ; 
}

static double toc (double t) 
{ 
  double s = tic (); 
  return (CS_MAX (0, s-t));
}

/* create a right-hand-side */
static void rhs (double *x, double *b, int m)
{
 // int i ;
 // for (i = 0 ; i < m ; i++) 
	//b [i] = 1 + ((double) i) / m;

  for (int i = 0 ; i < m ; i++) 
	x [i] = b [i];
}

/* infinity-norm of x */
static double norm (double *x, int n)
{
    int i ;
    double normx = 0 ;
    for (i = 0 ; i < n ; i++) normx = CS_MAX (normx, fabs (x [i])) ;
    return (normx) ;
}

/* compute residual, norm(A*x-b,inf) / (norm(A,1)*norm(x,inf) + norm(b,inf)) */
static void resid (int ok, cs *A, double *x, double *b, double *r)
{
    int i, m, n ;
    if (!ok) { mafLogMessage (_T("    (failed)\n")) ; return ; }
    m = A->m ; n = A->n ;
    for (i = 0 ; i < m ; i++) r [i] = -b [i] ;	    /* r = -b */
    cs_gaxpy (A, x, r) ;			    /* r = r + A*x  */
    mafLogMessage (_T("resid: %8.2e\n"),
	norm (r,m) / ((n == 0) ? 1 : (cs_norm (A) * norm (x,n) + norm (b,m)))) ;
}

/* solve a linear system using Cholesky, LU, and QR, with various orderings */
int lhpOpScaleMMA::Solve (problem *Prob)
{
    cs *A, *C;
    double	*b, *x, *r, t, tol ;
    int	k, m, n, ok, order, nb, ns, *R, *S, *rr, sprank;
    csd *D ;

    if (!Prob) 
		return (0);

    A = Prob->A;
	C = Prob->C;
	b = Prob->b;
	x = Prob->x;
	r = Prob->r;

    m = A->m;
	n = A->n;

    tol = Prob->sym ? 0.001 : 1;	/* partial pivoting tolerance */
	// youbing : 
    D = cs_dmperm (C);				/* dmperm analysis */

    if (!D) 
		return (0);

    nb = D->nb; 
	R = D->R; 
	S = D->S; 
	rr = D->rr;
    sprank = rr[3];

    for (ns = 0, k = 0 ; k < nb ; k++)
    {
		ns += ((R [k+1] == R [k]+1) && (S [k+1] == S [k]+1));
    }

    mafLogMessage (_T("blocks: %d singletons: %d structural rank: %d\n"), nb, ns, sprank);
    cs_dfree (D);

	//printf ("\nnacer cs_qrsol\n");
	
    //for (order = -1 ; order <= 2 ; order += 3)	// natural and amd(A'*A) 
    //{
	//	if (order == -1 && m > 1000)
	//		continue;

		mafLogMessage (_T("QR   "));
		//print_order (order);

		rhs (x, b, m);							// compute right-hand-side 
		t = tic ();
		// Youbing : QR decomposition
		ok = cs_qrsol (C, x, 2);			// min norm(Ax-b) with QR 		
		mafLogMessage(_T("time: %8.2f "), toc (t));
		resid (ok, C, x, b, r);					// print residual 
    //}
	
	return ok;

    if (m != n || sprank < n) 
		return (0);								/* return if rect. or singular*/

	//printf ("\nnacer LU\n");
    for (order = -1 ; order <= 0 ; order++)		/* try all orderings */
    {
		if (order == -1 && m > 1000) 
			continue;

		mafLogMessage(_T("LU   "));
		//print_order (order);
		rhs (x, b, m);							/* compute right-hand-side */
		t = tic ();
		ok = cs_lusol (C, x, order, tol);		/* solve Ax=b with LU */		
		mafLogMessage (_T("time: %8.2f "), toc (t));
		resid (ok, C, x, b, r);					/* print residual */
    }

    //if (!Prob->sym) 
	//	return (1);

	//printf ("\nnacer Chol\n");
	/*
    for (order = -1 ; order <= 0 ; order++)		// natural and amd(A+A') 
    {
		if (order == -1 && m > 1000) 
			continue;

		mafLogMessage (_T("Chol "));
		//print_order (order);
		rhs (x, b, m);							// compute right-hand-side 
		t = tic ();
		// YOubing : core 
		ok = cs_cholsol (C, x, order);			// solve Ax=b with Cholesky 		
		mafLogMessage(_T("time: %8.2f "), toc (t));
		resid (ok, C, x, b, r);					// print residual 
    }
	*/

    return (ok);
} 

vtkMAFSmartPointer<vtkPolyData>  lhpOpScaleMMA::CleanPolyData(vtkMAFSmartPointer<vtkPolyData>  pPoly)
{
	vtkMAFSmartPointer<vtkCleanPolyData> cleanPolyData;	

	cleanPolyData->ToleranceIsAbsoluteOn ();
	cleanPolyData->SetAbsoluteTolerance(1e-10);
	cleanPolyData->PointMergingOn ();
	//cleanPolyData-> ConvertLinesToPointsOff();
	//cleanPolyData->ConvertPolysToLinesOff ();
	cleanPolyData->SetInput(pPoly.GetPointer());
	cleanPolyData->Update();

	vtkMAFSmartPointer<vtkPolyData>  pCleanPoly = vtkPolyData::SafeDownCast(cleanPolyData->GetOutput());

	return pCleanPoly;

}

// clear, simplify and check the input mesh to remove skinny edges in the processing step
vtkMAFSmartPointer<vtkPolyData>  lhpOpScaleMMA::ReducePolyData(vtkMAFSmartPointer<vtkPolyData> pCleanPoly, double rate)
{
	int nPoint1, nPoint2, nCells1, nCells2;

	//int nVert = pPoly->GetNumberOfVerts();
	//int nLine = pPoly->GetNumberOfLines();
	//int nPoly = pPoly->GetNumberOfPolys();
	nPoint1 = pCleanPoly->GetNumberOfPoints();
	nCells1 = pCleanPoly->GetNumberOfCells();
	//mafLogMessage("Before clear verts: %d, lines: %d, poly: %d, points: %d", nVert, nLine, nPoly, nPoint1);

	pCleanPoly->SetLines(NULL);
	pCleanPoly->SetVerts(NULL);
	pCleanPoly->BuildCells();


	vtkMAFSmartPointer<vtkQuadricDecimation> decimator;
	decimator->SetTargetReduction(1 - rate);
	decimator->SetInput(pCleanPoly.GetPointer() );
	decimator->Update();

	vtkMAFSmartPointer<vtkPolyData> pReducedPoly = decimator->GetOutput();

	nPoint2 = pCleanPoly->GetNumberOfPoints();
	nCells2 = pCleanPoly->GetNumberOfCells();
	//mafLogMessage("After decimation: verts: %d, lines: %d, poly: %d, points: %d", nVert, nLine, nPoly, nPoint2);


	// check skinny triangles
	/*
	double p1[3], p2[3], p0[3];
	double threshold = 1e-5;

	vtkMAFSmartPointer<vtkIdList>  ptIds = vtkIdList::New();
	
	for (int i = 0; i < nCells2; i++)
	{
		ptIds->Reset();
		pCleanPoly->GetCellPoints (i, ptIds.GetPointer());

		if (ptIds->GetNumberOfIds() == 3)
		{
			int id0, id1, id2;
			id0 = ptIds->GetId(0);
			id1 = ptIds->GetId(1);
			id2 = ptIds->GetId(2);

			pCleanPoly->GetPoint(ptIds->GetId(0) , p0);
			pCleanPoly->GetPoint(ptIds->GetId(1) , p1);
			pCleanPoly->GetPoint(ptIds->GetId(2) , p2);
			double 	determ = vtkMath::Determinant3x3 (p1, p2, p0);
			if (fabs(determ) < threshold)
			{
				mafLogMessage("degenerated triangle found: %d %d %d, %f", id0, id1, id2, determ);
			}

			double dist01, dist12, dist20;
			dist01 = sqrt(vtkMath::Distance2BetweenPoints(p0,p1));
			dist12 = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
			dist20 = sqrt(vtkMath::Distance2BetweenPoints(p2,p0));
			if (dist01 < threshold)
				mafLogMessage("close points : %d %d %f", id0, id1, dist01);
			
			if (dist12 < threshold)
				mafLogMessage("close points : %d %d %f", id1, id2, dist12);

			if (dist20 < threshold)
				mafLogMessage("close points : %d %d %f", id2, id0, dist20);
		}
		else
		{
			mafLogMessage("polygon not triangle with number of vertices %d" , ptIds->GetNumberOfIds());
		}
	}

	//double bb[6];
	vtkIdType inCellId = 0, outCellId = 0;
	vtkIdType npts=0;
	vtkIdType *pts=0;
	vtkIdType c = 0;
	vtkCellData * thisCellData = pCleanPoly->GetCellData();
	vtkCellData * newCellData = vtkCellData::New();
	newCellData->CopyAllocate(thisCellData, pCleanPoly->GetNumberOfCells());


	vtkIdList *cellIds = vtkIdList::New();
	int numCells = cellIds->GetNumberOfIds();

	for (int i = 0; i < nPoint2; i++)
	{
		ptIds->Reset();
		pCleanPoly->GetPointCells (i, cellIds);
		numCells = cellIds->GetNumberOfIds();
		
		
		if (numCells == 0)
			mafLogMessage("isolated point %d", i);
		else if (numCells == 1)
			mafLogMessage("1 neighbour point %d", i);
		else if (numCells == 2)
			mafLogMessage("2 neighbour point %d", i);
			

	}

	*/

	if (nPoint2 < nPoint1)
	{
		mafLogMessage("ClearPolyData: %d points removed, Input : %d, Output: %d ", nPoint1 - nPoint2, nPoint1, nPoint2);
	}

	return pReducedPoly;

}

void lhpOpScaleMMA::FindLandmarkBones()
{
	medMSMGraph::MSMGraphNodeList list;

	m_pGraph->GetBone(medMSMGraph::BoneType::LeftFemur, m_LodLevel, list, true);
	int len = list.size();
	if (!list.empty()) {
		m_pFemur[LEFT] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_FemurIdx[LEFT] = FindVMEIdx(m_pFemur[LEFT]);
	}
	else 
		mafLogMessage("Error: Left femur not found");

	m_pGraph->GetBone(medMSMGraph::BoneType::RightFemur, m_LodLevel, list, true);
	if (!list.empty()) {
		m_pFemur[RIGHT] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_FemurIdx[RIGHT] = FindVMEIdx(m_pFemur[RIGHT]);
	}
		else 
		mafLogMessage("Error: Right femur not found");

	// patella
	list.clear();
	m_pGraph->GetBone(medMSMGraph::BoneType::LeftPatella, m_LodLevel, list, true);
	if (!list.empty()) {
		m_pPatella[LEFT] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_PatellaIdx[LEFT]= FindVMEIdx(m_pPatella[LEFT]);
	}
	else 
		mafLogMessage("Error: Left patella not found");


	list.clear();
	m_pGraph->GetBone(medMSMGraph::BoneType::RightPatella, m_LodLevel, list, true);
	len = list.size();
	if (!list.empty()) {
		m_pPatella[RIGHT] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_PatellaIdx[RIGHT]= FindVMEIdx(m_pPatella[RIGHT]);
	}
	else 
		mafLogMessage("Error: Right patella not found");


	// ankle", "RightTibia
	list.clear();
	m_pGraph->GetBone(medMSMGraph::BoneType::LeftTibia, m_LodLevel, list, true);
	if (!list.empty()) {
		m_pAnkle[0] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_AnkleIdx[LEFT]= FindVMEIdx(m_pAnkle[LEFT]);
	}
	else 
		mafLogMessage("Error: Left tibia not found");

	list.clear();
	m_pGraph->GetBone(medMSMGraph::BoneType::RightTibia, m_LodLevel, list, true);
	if (!list.empty()) {
		m_pAnkle[RIGHT] = mafVMESurface::SafeDownCast(list[0]->m_Vme);
		m_AnkleIdx[RIGHT]= FindVMEIdx(m_pAnkle[RIGHT]);
	}
	else 
		mafLogMessage("Error: Right tibia not found");

}

int lhpOpScaleMMA::FindVMEIdx(mafVMESurface * pSurf)
{
	for (int i = 0; i < m_SurfVMEList.size(); i++)
		if (m_SurfVMEList[i] == pSurf) {
			return i;
		}

	return -1;
}