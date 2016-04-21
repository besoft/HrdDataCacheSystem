/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper_DBG.cpp,v $
Language:  C++
Date:      $Date: 2012-03-02 10:54:19 $
Version:   $Revision: 1.1.2.3 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2008-2012
University of Bedforshire, University of West Bohemia
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "medVMEMuscleWrapper.h"


#ifdef _DEBUG_VIS_

#include "vtkPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkCubeSource.h"
#include "vtkTubeFilter.h"
#include "vtkMAFSmartPointer.h"
#include "vtkCellArray.h"
#include "vtkGlyph3D.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"

#include "mafTransform.h"

#include <map>


void medVMEMuscleWrapper::Debug_Write(const char* strText, vtkPolyData* pData, int limit)
{
	int nPoints = pData->GetNumberOfPoints();
	_RPT2(_CRT_WARN, "############## %s - %d points\n", strText, nPoints);	
	for (int i = 0; i < nPoints && i < limit; i++)
	{
		const double* point = pData->GetPoint(i);
		_RPT4(_CRT_WARN, "[%d] = [%.0f, %.0f, %.0f]\n",
			i, point[0], point[1], point[2]);
	}

	if (nPoints > limit) {
		_RPT0(_CRT_WARN, "...\n");
	}
}

//Visualize deformation result
void medVMEMuscleWrapper::Debug_Visualize_DeformationData()
{	
	const int resolution = medVMEMusculoSkeletalModel::LOD::Lowest;	//requested resolution	
	typedef medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator vmeIter;
	typedef medMSMGraph::MSMGraphNodeList::const_iterator grnodeIter;

#pragma region WRAPPERS
	vtkMAFSmartPointer< vtkAppendPolyData > wrappersRP_CP[2];	//combines all wrappers and ref systems		
	for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); pItem != m_Wrappers.end(); pItem++)
	{    
		if ((*pItem)->pCurves[0] != NULL && (*pItem)->pCurves[1] != NULL) 
		{
			for (int i = 0; i < 2; i++)
			{
				//RP and CP
				vtkTubeFilter* tbsource = vtkTubeFilter::New();
				tbsource->SetInput((*pItem)->pCurves[i]);
				tbsource->SetRadius(2.5);
				tbsource->SetNumberOfSides(8);
				tbsource->SetCapping(1);
				tbsource->Update();
				wrappersRP_CP[i]->AddInput(tbsource->GetOutput());
				tbsource->Delete();

				if ((*pItem)->RefSysOrigin[i] != NULL)
				{
					vtkCubeSource* source = vtkCubeSource::New();
					source->SetCenter((*pItem)->RefSysOrigin[i]);
					source->SetXLength(10);
					source->SetYLength(10);
					source->SetZLength(10);
					source->Update();

					wrappersRP_CP[i]->AddInput(source->GetOutput());
					source->Delete();
				}
			}
		}       
	}

	vtkMAFSmartPointer< vtkPolyDataMapper > wrappersRP_CP_Mapper[2];
	vtkMAFSmartPointer< vtkActor > wrappersRP_CP_Actor[2];
	for (int i = 0; i < 2; i++) 
	{
		wrappersRP_CP_Mapper[i]->SetInput(wrappersRP_CP[i]->GetOutput());
		wrappersRP_CP_Actor[i]->SetMapper(wrappersRP_CP_Mapper[i].GetPointer());
	}

	wrappersRP_CP_Actor[0]->GetProperty()->SetColor(0.0, 0.4, 0.0);		//green lines
	wrappersRP_CP_Actor[1]->GetProperty()->SetColor(1.0, 0.8, 0.0);		//yellow lines			
#pragma endregion

#pragma region MUSCLE	
	vtkMAFSmartPointer< vtkPolyDataMapper > muscleRP_CP_Mapper[2];
	vtkMAFSmartPointer< vtkActor > muscleRP_CP_Actor[2];
	for (int i = 0; i < 2; i++) 
	{
		muscleRP_CP_Mapper[i]->SetInput(i == 0 ? m_pTransformedMuscle : m_pDeformedMuscle);
		muscleRP_CP_Actor[i]->SetMapper(muscleRP_CP_Mapper[i].GetPointer());
		muscleRP_CP_Actor[i]->GetProperty()->SetOpacity(0.6);
	}

	muscleRP_CP_Actor[0]->GetProperty()->SetColor(0.0, 0.4, 0.0);		//green lines
	muscleRP_CP_Actor[1]->GetProperty()->SetColor(1.0, 0.8, 0.0);		//yellow lines				
#pragma endregion MUSCLE

#pragma region BONES
	//this map contains bones that were used as obstacles
	typedef medMSMGraph::BoneType BoneType;
	typedef std::map< mafNode*, int >::iterator hashIter;
	std::map< mafNode*, int > bonesUsed;

#pragma region CURRENT_POSE		
	vtkMAFSmartPointer< vtkAppendPolyData > bonesCP_Active;					//combines all active bones together
	vtkMAFSmartPointer< vtkAppendPolyData > bonesCP_NonActive;			//combines all non-active bones together
	vtkMAFSmartPointer< vtkPolyDataMapper > bonesCP_ActiveMapper;
	vtkMAFSmartPointer< vtkPolyDataMapper > bonesCP_NonActiveMapper;
	vtkMAFSmartPointer< vtkActor > bonesCP_ActiveActor;
	vtkMAFSmartPointer< vtkActor > bonesCP_NonActiveActor;
	
	medVMEMusculoSkeletalModel* modelCP = GetMusculoSkeletalModel();	
	if (modelCP != NULL)
	{
		//1. get all active bones (i.e., already filtered)		
		medVMEMusculoSkeletalModel::mafVMENodeList bonesA;
		modelCP->GetBones(bonesA, resolution);
		
		for (vmeIter it = bonesA.begin();	it != bonesA.end(); it++)
		{
			bonesUsed[(*it)] = (int)BoneType::Invalid;	//unknown bone type

			//we have to get the VTK surface of the bone and transform it to our space
			vtkPolyData* pPoly = CreateTransformedPolyDataFromVME(mafVME::SafeDownCast(*it));
			bonesCP_Active->AddInput(pPoly);
			pPoly->Delete();
		}		

		//2. get all non-active bones
		medMSMGraph::MSMGraphNodeList bonesNA;
		modelCP->GetMSMGraph()->GetBones(resolution, bonesNA, true);
		for (grnodeIter it = bonesNA.begin(); it != bonesNA.end(); it++)
		{
			//check, if the bone is already in active list
			hashIter fnd = bonesUsed.find((*it)->m_Vme);
			if (fnd != bonesUsed.end())
			{
				//OK, so we have found it
				fnd->second = (int)(*it)->m_NodeDescriptor.m_BoneInfo;
			}
			else
			{
				//not found => add this bone to NA list
				//we have to get the VTK surface of the bone and transform it to our space
				vtkPolyData* pPoly = CreateTransformedPolyDataFromVME(mafVME::SafeDownCast((*it)->m_Vme));
				bonesCP_NonActive->AddInput(pPoly);
				pPoly->Delete();

				bonesUsed[(*it)->m_Vme] = -(int)(*it)->m_NodeDescriptor.m_BoneInfo;	//store bone type
			}
		}

		//visualization part
		bonesCP_ActiveMapper->SetInput(bonesCP_Active->GetOutput());
		bonesCP_NonActiveMapper->SetInput(bonesCP_NonActive->GetOutput());
		bonesCP_ActiveActor->SetMapper(bonesCP_ActiveMapper.GetPointer());
		bonesCP_NonActiveActor->SetMapper(bonesCP_NonActiveMapper.GetPointer());	
		
		bonesCP_ActiveActor->GetProperty()->SetDiffuseColor(255/255.0, 242/255.0, 186/255.0);	//0.949999988079071, 0.9399999976158142, 0.8100000023841858);			//bone
		bonesCP_NonActiveActor->GetProperty()->SetDiffuseColor(255/255.0, 242/255.0, 186/255.0);//0.949999988079071, 0.9399999976158142, 0.8100000023841858);	//bone
		bonesCP_NonActiveActor->GetProperty()->SetOpacity(0.2);
	}
#pragma endregion 
	
#pragma region REST_POSE
	vtkMAFSmartPointer< vtkAppendPolyData > bonesRP_Active;		//combines all active bones together
	vtkMAFSmartPointer< vtkAppendPolyData > bonesRP_NonActive;	//combines all active bones together
	vtkMAFSmartPointer< vtkPolyDataMapper > bonesRP_ActiveMapper;
	vtkMAFSmartPointer< vtkPolyDataMapper > bonesRP_NonActiveMapper;
	vtkMAFSmartPointer< vtkActor > bonesRP_ActiveActor;
	vtkMAFSmartPointer< vtkActor > bonesRP_NonActiveActor;

	//Find REST-POSE model
	medVMEMusculoSkeletalModel* modelRP = GetMusculoSkeletalModel_RP();
	if (modelRP != NULL)
	{
		const medMSMGraph* grp = modelRP->GetMSMGraph();	
		for (hashIter it = bonesUsed.begin(); it != bonesUsed.end(); it++)
		{
			medMSMGraph::MSMGraphNodeList boneList;
			grp->GetBone(abs((*it).second), resolution, boneList, true);	//get the RP bone

			vtkAppendPolyData* pAcList = ((*it).second >= 0) ? bonesRP_Active.GetPointer() : bonesRP_NonActive.GetPointer();
			for (grnodeIter nit = boneList.begin(); nit != boneList.end(); nit++)
			{					
				//we have to get the VTK surface of the bone and transform it to our space
				vtkPolyData* pPoly = CreateTransformedPolyDataFromVME(mafVME::SafeDownCast((*nit)->m_Vme));
				pAcList->AddInput(pPoly);
				pPoly->Delete();				
			}
		}

		//visualization part
		bonesRP_ActiveMapper->SetInput(bonesRP_Active->GetOutput());
		bonesRP_NonActiveMapper->SetInput(bonesRP_NonActive->GetOutput());
		bonesRP_ActiveActor->SetMapper(bonesRP_ActiveMapper.GetPointer());
		bonesRP_NonActiveActor->SetMapper(bonesRP_NonActiveMapper.GetPointer());	
		
		bonesRP_ActiveActor->GetProperty()->SetDiffuseColor(144/255.0, 206/255.0, 151/255.0);	//0.949999988079071, 0.9399999976158142, 0.8100000023841858);			//bone
		bonesRP_NonActiveActor->GetProperty()->SetDiffuseColor(144/255.0, 206/255.0, 151/255.0);//0.949999988079071, 0.9399999976158142, 0.8100000023841858);	//bone
		bonesRP_NonActiveActor->GetProperty()->SetOpacity(0.2);
	}
#pragma endregion 
#pragma endregion //BONES

#pragma region ATTACHMENT POINTS	
	vtkMAFSmartPointer< vtkAppendPolyData > attchpts_RP_CP[2][2];		//combines points together
	vtkMAFSmartPointer< vtkPolyDataMapper > attchpts_CP_Mapper[2][2];
	vtkMAFSmartPointer< vtkActor > attchpts_RP_CP_Actor[2][2];

	for (int iPose = 0; iPose < 2; iPose++)
	{
		for (int i = 0; i < 2; i++)
		{
			vtkPoints* points = NULL;
			if (iPose != 0)
			{
				if (NULL == (points = CreateTransformedPointsFromVME(m_OIVME[i]))) {	//if not specified, points must be derived from action lines end-points
					points = CreateTransformedPointsFromWrappers(m_Wrappers, i == 0);
				}
			}
			else
			{
				//we have to find RP landmark, if we are able to do so, they do not have tags, so we will have to rely on names				
				if (m_OIVME[i] == NULL)
					points = CreateTransformedPointsFromWrappers(m_Wrappers, i == 0, true);
				else if (modelRP != NULL)
				{

					mafString strName = m_OIVME[i]->GetName();
					const medMSMGraph* grp = modelRP->GetMSMGraph();	
					medMSMGraph::mafVMENodeList landList;
					grp->GetLandmarkClouds(landList, true);

					for (vmeIter it = landList.begin(); it != landList.end(); it++) 
					{
						if (strName.Compare((*it)->GetName()) == 0)
						{
							//we have found it
							points = CreateTransformedPointsFromVME(mafVME::SafeDownCast(*it));
							break;
						}
					}
				}					
			}

			if (points != NULL)
			{
				int nPoints = points->GetNumberOfPoints();

				vtkPolyData* pPoly = vtkPolyData::New();
				pPoly->SetPoints(points);
				points->Delete();	//no longer needed

				//create cells				
				vtkCellArray* lines = vtkCellArray::New();
				vtkIdType edge[2] = {nPoints - 1, 0};
				lines->InsertNextCell(2, edge);				

				for (int k = 1; k < nPoints; k++) {
					edge[0] = edge[1]; edge[1] = k;
					lines->InsertNextCell(2, edge);
				}

				pPoly->SetLines(lines);
				lines->Delete();	//no longer needed

				//create balls
				vtkSphereSource* source = vtkSphereSource::New();					
				source->SetRadius(2.5);
				//source->SetPhiResolution(12);
				//source->SetThetaResolution(12);

				vtkGlyph3D* filter = vtkGlyph3D::New();
				filter->SetSource(source->GetOutput());
				source->Delete();	//no longer needed

				filter->SetInput(pPoly);
				filter->Update();

				attchpts_RP_CP[iPose][i]->AddInput(filter->GetOutput());
				filter->Delete();	//no longer needed

				//create tubes
				vtkTubeFilter* filter2 = vtkTubeFilter::New();
				filter2->SetInput(pPoly);
				filter2->SetRadius(1.25);
				filter2->SetNumberOfSides(5);
				filter2->Update();

				attchpts_RP_CP[iPose][i]->AddInput(filter2->GetOutput());
				filter2->Delete();	//no longer needed
				
				pPoly->Delete();	//no longer needed
			} 		
		}
	}

	//now comes visualization
	for (int iPose = 0; iPose < 2; iPose++)
	{
		for (int i = 0; i < 2; i++)
		{
			attchpts_CP_Mapper[iPose][i]->SetInput(attchpts_RP_CP[iPose][i]->GetOutput());
			attchpts_RP_CP_Actor[iPose][i]->SetMapper(attchpts_CP_Mapper[iPose][i].GetPointer());
		}
	}

	//origin should be red, insertion blue,
	//RP should give it green tone, CP then yellow
	attchpts_RP_CP_Actor[0][0]->GetProperty()->SetColor(1.0, 0.4, 0.0);		//RP ori
	attchpts_RP_CP_Actor[0][1]->GetProperty()->SetColor(0.0, 0.4, 1.0);		//RP ins
	attchpts_RP_CP_Actor[1][0]->GetProperty()->SetColor(1.0, 0.6, 0.0);		//CP ori
	attchpts_RP_CP_Actor[1][1]->GetProperty()->SetColor(1.0, 0.8, 1.0);		//CP ins
#pragma endregion

#pragma region ALIGNED DATA	
#pragma region ACTORS and MAPPERS
	//WRAPPERS
	vtkMAFSmartPointer< vtkPolyDataMapper > tr_wrappersRP_Mapper;
	tr_wrappersRP_Mapper->SetInput(wrappersRP_CP[0]->GetOutput());

	vtkMAFSmartPointer< vtkActor > tr_wrappersRP_Actor;
	tr_wrappersRP_Actor->SetMapper(tr_wrappersRP_Mapper.GetPointer());
	
	vtkSmartPointer < vtkProperty > cpyProperty = wrappersRP_CP_Actor[0]->GetProperty()->NewInstance();
	cpyProperty->UnRegister(NULL);	//remove our reference (let vtkSmartPointer to deal with it)
	cpyProperty->DeepCopy(wrappersRP_CP_Actor[0]->GetProperty());
	cpyProperty->SetRepresentationToWireframe();

	tr_wrappersRP_Actor->SetProperty(cpyProperty.GetPointer());	

	//MUSCLE
	vtkMAFSmartPointer< vtkPolyDataMapper > tr_muscleRP_Mapper;
	tr_muscleRP_Mapper->SetInput(m_pTransformedMuscle);

	vtkMAFSmartPointer< vtkActor > tr_muscleRP_Actor;
	tr_muscleRP_Actor->SetMapper(tr_muscleRP_Mapper.GetPointer());
	
	cpyProperty = muscleRP_CP_Actor[0]->GetProperty()->NewInstance();
	cpyProperty->UnRegister(NULL);	//remove our reference (let vtkSmartPointer to deal with it)
	cpyProperty->DeepCopy(muscleRP_CP_Actor[0]->GetProperty());
	cpyProperty->SetRepresentationToWireframe();

	tr_muscleRP_Actor->SetProperty(cpyProperty.GetPointer());	

	//BONES
	vtkMAFSmartPointer< vtkPolyDataMapper > tr_bonesRP_ActiveMapper;
	vtkMAFSmartPointer< vtkPolyDataMapper > tr_bonesRP_NonActiveMapper;
	tr_bonesRP_ActiveMapper->SetInput(bonesRP_Active->GetOutput());
	tr_bonesRP_NonActiveMapper->SetInput(bonesRP_NonActive->GetOutput());

	vtkMAFSmartPointer< vtkActor > tr_bonesRP_ActiveActor;
	vtkMAFSmartPointer< vtkActor > tr_bonesRP_NonActiveActor;
	tr_bonesRP_ActiveActor->SetMapper(tr_bonesRP_ActiveMapper.GetPointer());
	tr_bonesRP_NonActiveActor->SetMapper(tr_bonesRP_NonActiveMapper.GetPointer());	

	cpyProperty = bonesRP_ActiveActor->GetProperty()->NewInstance();
	cpyProperty->UnRegister(NULL);	//remove our reference (let vtkSmartPointer to deal with it)
	cpyProperty->DeepCopy(bonesRP_ActiveActor->GetProperty());
	cpyProperty->SetRepresentationToWireframe();

	tr_bonesRP_ActiveActor->SetProperty(cpyProperty.GetPointer());	
	
	cpyProperty = bonesRP_NonActiveActor->GetProperty()->NewInstance();
	cpyProperty->UnRegister(NULL);	//remove our reference (let vtkSmartPointer to deal with it)
	cpyProperty->DeepCopy(bonesRP_NonActiveActor->GetProperty());
	cpyProperty->SetRepresentationToWireframe();

	tr_bonesRP_NonActiveActor->SetProperty(cpyProperty.GetPointer());	

	//ATTACHMENT POINTS
	vtkMAFSmartPointer< vtkPolyDataMapper > tr_attchpts_RP_Mapper[2];
	vtkMAFSmartPointer< vtkActor > tr_attchpts_RP_Actor[2];
	for (int i = 0; i < 2; i++)
	{
		tr_attchpts_RP_Mapper[i]->SetInput(attchpts_RP_CP[0][i]->GetOutput());
		tr_attchpts_RP_Actor[i]->SetMapper(tr_attchpts_RP_Mapper[i].GetPointer());

		cpyProperty = attchpts_RP_CP_Actor[0][i]->GetProperty()->NewInstance();
		cpyProperty->UnRegister(NULL);	//remove our reference (let vtkSmartPointer to deal with it)
		cpyProperty->DeepCopy(attchpts_RP_CP_Actor[0][i]->GetProperty());
		cpyProperty->SetRepresentationToWireframe();

		tr_attchpts_RP_Actor[i]->SetProperty(cpyProperty.GetPointer());	
	}

#pragma endregion ACTORS and MAPPERS

	//align rest-pose data so that pelvis centroid and its z-axis rotation match values of the current-pose
	if (modelRP != NULL && modelCP != NULL)
	{
		//get both pelvis regions
		medMSMGraph::mafVMENodeList listRegionRP, listRegionCP;
		modelRP->GetMSMGraph()->GetRegion(medMSMGraph::RegionType::Pelvis, listRegionRP, true);
		modelCP->GetMSMGraph()->GetRegion(medMSMGraph::RegionType::Pelvis, listRegionCP, true);

		if (listRegionRP.size() != 0 && listRegionCP.size() != 0)
		{
			//extract VMEs for both pelvis'
			mafVME* pelvisRP = mafVME::SafeDownCast(listRegionRP[0]);
			mafVME* pelvisCP = mafVME::SafeDownCast(listRegionCP[0]);			

			//get transformation matrices for both regions
			const mafMatrix* matRP = this->GetVMEAbsMatrix(pelvisRP);
			const mafMatrix* matCP = this->GetVMEAbsMatrix(pelvisCP);			

			double posRP[3], posCP[3];
			mafTransform::GetPosition(*matRP, posRP);
			mafTransform::GetPosition(*matCP, posCP);
			
			double oriRP[3], oriCP[3];
			mafTransform::GetOrientation(*matRP, oriRP);
			mafTransform::GetOrientation(*matCP, oriCP);

			//OK, we must build a transformation
			double delta_pos[3], delta_ori[3];
			for (int i = 0; i < 3; i++){
				delta_pos[i] = posCP[i] - posRP[i];
				delta_ori[i] = oriCP[i] - oriRP[i];
			}

			tr_wrappersRP_Actor->AddPosition(delta_pos);
			tr_wrappersRP_Actor->AddOrientation(delta_ori);
			tr_muscleRP_Actor->AddPosition(delta_pos);
			tr_muscleRP_Actor->AddOrientation(delta_ori);
			tr_bonesRP_ActiveActor->AddPosition(delta_pos);
			tr_bonesRP_ActiveActor->AddOrientation(delta_ori);
			tr_bonesRP_NonActiveActor->AddPosition(delta_pos);
			tr_bonesRP_NonActiveActor->AddOrientation(delta_ori);
			tr_attchpts_RP_Actor[0]->AddPosition(delta_pos);
			tr_attchpts_RP_Actor[0]->AddOrientation(delta_ori);
			tr_attchpts_RP_Actor[1]->AddPosition(delta_pos);
			tr_attchpts_RP_Actor[1]->AddOrientation(delta_ori);
		}
	}
#pragma endregion

	//----------------------------------------------------------------------------
	// Render
	//----------------------------------------------------------------------------
	vtkRenderer *ren1= vtkRenderer::New();
	ren1->AddActor(tr_bonesRP_ActiveActor.GetPointer());
	ren1->AddActor(tr_bonesRP_NonActiveActor.GetPointer());
	ren1->AddActor(tr_muscleRP_Actor.GetPointer());
	ren1->AddActor(tr_wrappersRP_Actor.GetPointer());
	ren1->AddActor(tr_attchpts_RP_Actor[0].GetPointer());
	ren1->AddActor(tr_attchpts_RP_Actor[1].GetPointer());

	ren1->AddActor( bonesCP_ActiveActor.GetPointer());
	ren1->AddActor( bonesCP_NonActiveActor.GetPointer());
	ren1->AddActor( bonesRP_ActiveActor.GetPointer());
	ren1->AddActor( bonesRP_NonActiveActor.GetPointer());

	for (int i = 0; i < 2; i++)
	{		
		ren1->AddActor(muscleRP_CP_Actor[i].GetPointer());
		ren1->AddActor(wrappersRP_CP_Actor[i].GetPointer());
		ren1->AddActor(attchpts_RP_CP_Actor[i][0].GetPointer());
		ren1->AddActor(attchpts_RP_CP_Actor[i][1].GetPointer());
	}

	//ren1->AddActor(pActorAL);
	ren1->SetBackground( 0, 0, 0 );

	vtkRenderWindow *renWin = vtkRenderWindow::New();
	renWin->AddRenderer( ren1 );	
	renWin->SetSize( 1280, 1024 );

	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);
	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	iren->Initialize();
	iren->Start();
	
	//Remove data
	iren->SetInteractorStyle(NULL);
	style->Delete();
	iren->Delete();
	renWin->Delete();
	//ren1->RemoveAllProps();
	ren1->Delete();

	
	/*_RPT0(_CRT_WARN, "#################################\n");
	Debug_Write("OC", oc);
	Debug_Write("DC", dc);
	Debug_Write("Input", input, 100);
	Debug_Write("Output", output, 100);	*/
}

#endif


#ifdef VTK2OBJ
#include "vtkOBJExporter.h"

static void	ExportToOBJ(vtkPolyData* PolyData, const char* m_MuscleVmeName)
{
	vtkPolyDataMapper* pMapper = vtkPolyDataMapper::New();
	pMapper->SetInput(PolyData) ;

	vtkActor* pActorInput = vtkActor::New();
	pActorInput->SetMapper( pMapper );
	pActorInput->GetProperty()->SetColor(0.7, 0, 0.4);
	pActorInput->GetProperty()->SetOpacity(0.5);
	

	//----------------------------------------------------------------------------
	// Render
	//----------------------------------------------------------------------------
	vtkRenderer *ren1= vtkRenderer::New();
	ren1->AddActor( pActorInput );


	//ren1->AddActor(pActorAL);
	ren1->SetBackground( 0, 0, 0 );

	vtkRenderWindow *renWin = vtkRenderWindow::New();
	renWin->AddRenderer( ren1 );
	renWin->SetSize( 640, 480 );


	vtkOBJExporter* exprt = vtkOBJExporter::New();
	exprt->SetInput(renWin);
	exprt->SetFilePrefix(m_MuscleVmeName);
	exprt->Update();

	
	renWin->Delete();
	//ren1->RemoveAllProps();
	ren1->Delete();

	pActorInput->Delete();
}
#endif

#pragma endregion //DEBUG_STUFF