/*========================================================================= 
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: medPipeJoint.cpp,v $ 
  Language: C++ 
  Date: $Date: 2012-03-02 10:52:31 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Josef Kohout
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafDecl.h"

#include "medPipeJoint.h"
#include "medVMEJoint.h"
#include "mafSceneNode.h"
#include "mafVME.h"
#include "mafGUI.h"
#include "mafGUIValidator.h"
#include "mafTransformBase.h"
#include "mafEventSource.h"

#include "vtkMAFAssembly.h"
#include "vtkMAFSmartPointer.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkTubeFilter.h"
#include "vtkDataSet.h"
#include "vtkLinearTransform.h"
#include "vtkMath.h"
#include "vtkOutlineFilter.h"
#include "vtkArrowSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"

#include "wx/busyinfo.h"
#include <float.h>

#include "mafDbg.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(medPipeJoint);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
medPipeJoint::medPipeJoint() : mafPipe()
//----------------------------------------------------------------------------
{	
	m_JointPositionActor = m_MainActorCentreActor = m_SecondActorCentreActor = NULL;
	m_JointAxisActor[0] = m_JointAxisActor[1] = m_JointAxisActor[2] = NULL;

	//everything is supposed to be displayed
	m_EnableMABBox = m_EnableSABBox = m_EnableMAC = m_EnableSAC = 0; 
	m_EnableJoint = m_EnableAxis = 1;

	m_bSizesInitialized = false;
}

//----------------------------------------------------------------------------
medPipeJoint::~medPipeJoint()
//----------------------------------------------------------------------------
{
	DestroyPipe();	

	if (m_Vme != NULL)	//no longer observer
		m_Vme->GetEventSource()->RemoveObserver(this);
}

//----------------------------------------------------------------------------
void medPipeJoint::Create(mafSceneNode *n)
//----------------------------------------------------------------------------
{
  wxCursor busy;
	Superclass::Create(n);	//creates m_AssemblyFront and other stuff	

	//m_AssemblyFront is constructed to automatically use the transformation of the joint VME output 
	//m_AssemblyFront->GetUserTransform(); It should be Identity matrix by the default
	
	if (m_Vme != NULL)
		m_Vme->GetEventSource()->AddObserver(this);

	InitSizes();
	CreatePipe();
}

//----------------------------------------------------------------------------
//Initializes sizes for various parts of the pipe using the sizes of joint actors.
//If joint actors are unknown, this routine sets the sizes to the default values and
//m_bSizesInitialized is left unchanged, otherwise they are set to be appropriate to 
//the sizes of joint actors and the method sets m_bSizesInitialized to true.
//N.B. The method terminates without any change, if m_bSizesInitialized is already true.
/*virtual*/ void medPipeJoint::InitSizes()
//----------------------------------------------------------------------------
{
	if (m_bSizesInitialized)
		return;	//already initialized => do not change it

	medVMEJoint* joint = medVMEJoint::SafeDownCast(m_Vme);
	_VERIFY_RET(joint != NULL);
	
	mafVME* mainActor = joint->GetMainActorVME();
	mafVME* secondActor = joint->GetSecondActorVME();

	//first, set the default sizes
	double dblLength = 1.0;
	if (mainActor == NULL && secondActor == NULL)
	{			
		m_JointRadius = m_CentroidRadius = 1.0;	//default radius				
		m_BBoxTubeRadius = 0.5;
		m_AxisLength = 5.0;
	}
	else
	{
		//OK, at least one actor does exist => derive the sizes from its size
		if (mainActor != NULL) 
		{
			mainActor->GetOutput()->Update();	//make sure it is updated

			vtkDataSet* ds = mainActor->GetOutput()->GetVTKData();
			if (ds != NULL) {
				dblLength = ds->GetLength();
			}
		}

		if (secondActor != NULL) 
		{
			secondActor->GetOutput()->Update();	//make sure it is updated

			vtkDataSet* ds = secondActor->GetOutput()->GetVTKData();
			if (ds != NULL)
			{
				double dblTmp = ds->GetLength();
				if (dblTmp >  dblLength)
					dblLength = dblTmp;
			}
		}
		
		m_AxisLength = dblLength * 0.25;					//1/4
		m_JointRadius = dblLength * (1.0 / 16);	
		m_CentroidRadius = dblLength * (1.0 / 32);		
		m_BBoxTubeRadius = dblLength * (1.0 / 128);					

		m_bSizesInitialized = true;
	}
}

//----------------------------------------------------------------------------
//Creates all visual pipes to reflect the changes in VME 
/*virtual*/ void medPipeJoint::CreatePipe()
//----------------------------------------------------------------------------
{	
	medVMEJoint* joint = medVMEJoint::SafeDownCast(m_Vme);
	_VERIFY_RET(joint != NULL);

	joint->GetOutput()->Update();	//make sure we have everything up to date
	
	mafVME* mainActor = joint->GetMainActorVME();
	mafVME* secondActor = joint->GetSecondActorVME();
	
	
	//create bounding boxes
	if (NULL != (m_MainActorBBoxActor = CreateBBoxPipe(mainActor, m_BBoxTubeRadius)))
	{
		m_MainActorBBoxActor->GetProperty()->SetColor(0,0,1);	//blue
		m_MainActorBBoxActor->SetVisibility(m_EnableMABBox);		
		m_AssemblyFront->AddPart(m_MainActorBBoxActor);
	}

	if (NULL != (m_SecondActorBBoxActor = CreateBBoxPipe(secondActor, m_BBoxTubeRadius)))
	{
		m_SecondActorBBoxActor->GetProperty()->SetColor(1,0,0);	//red
		m_SecondActorBBoxActor->SetVisibility(m_EnableSABBox);		
		m_AssemblyFront->AddPart(m_SecondActorBBoxActor);
	}
	

	//create mass centres
	double tmpPos[3];
	if (joint->GetMainActorCentroid(tmpPos))
	{
		if (NULL != (m_MainActorCentreActor = CreateSpherePipe(mainActor, tmpPos, m_CentroidRadius)))
		{
			m_MainActorCentreActor->GetProperty()->SetColor(0,0,1);	//blue
			m_MainActorCentreActor->SetVisibility(m_EnableMAC);		
			m_AssemblyFront->AddPart(m_MainActorCentreActor);
		}
	}

	if (joint->GetSecondActorCentroid(tmpPos))
	{
		if (NULL != (m_SecondActorCentreActor = CreateSpherePipe(secondActor, tmpPos, m_CentroidRadius)))
		{
			m_SecondActorCentreActor->GetProperty()->SetColor(1,0,0);	//red
			m_SecondActorCentreActor->SetVisibility(m_EnableSAC);		
			m_AssemblyFront->AddPart(m_SecondActorCentreActor);
		}
	}

	//create the joint	 position
	if (joint->GetJointPosition(tmpPos))
	{				 
		if (NULL != (m_JointPositionActor = CreateSpherePipe(mainActor, tmpPos, m_JointRadius)))
		{
			m_JointPositionActor->GetProperty()->SetColor(1,0,1);	//purple
			m_JointPositionActor->SetVisibility(m_EnableJoint);		
			m_AssemblyFront->AddPart(m_JointPositionActor);

			//create axis system
			double tmpVec[3];
			for (int i = 0; i < 3; i++)
			{
				if (joint->GetJointRotationAxis(i, tmpVec))
				{
					if (NULL != (m_JointAxisActor[i] = CreateArrowPipe(mainActor, tmpPos, tmpVec, m_AxisLength)))
					{						
						m_JointAxisActor[i]->GetProperty()->SetColor(i == 0, i == 1, i == 2);
						m_JointAxisActor[i]->SetVisibility(m_EnableAxis);		
						m_AssemblyFront->AddPart(m_JointAxisActor[i]);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//Destroys already existing visual pipes
/*virtual*/ void medPipeJoint::DestroyPipe()
//----------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		if (m_JointAxisActor[i] != NULL)
			m_AssemblyFront->RemovePart(m_JointAxisActor[i]);

		vtkDEL(m_JointAxisActor[i]);
	}

	if (m_MainActorBBoxActor != NULL)
		m_AssemblyFront->RemovePart(m_MainActorBBoxActor);
	
	if (m_SecondActorBBoxActor != NULL)
		m_AssemblyFront->RemovePart(m_SecondActorBBoxActor);

	if (m_MainActorCentreActor != NULL)
		m_AssemblyFront->RemovePart(m_MainActorCentreActor);
	
	if (m_SecondActorCentreActor != NULL)
		m_AssemblyFront->RemovePart(m_SecondActorCentreActor);

	if (m_JointPositionActor != NULL)
		m_AssemblyFront->RemovePart(m_JointPositionActor);

	vtkDEL(m_MainActorBBoxActor);
	vtkDEL(m_SecondActorBBoxActor);

	vtkDEL(m_MainActorCentreActor);
	vtkDEL(m_SecondActorCentreActor);
	vtkDEL(m_JointPositionActor);
}


//----------------------------------------------------------------------------
//Creates visual pipe for visualization of sphere with the given local centre (relative to actorVME) 
/*virtual*/ vtkActor* medPipeJoint::CreateSpherePipe(mafVME* actorVME, const double* centre, double radius)
//----------------------------------------------------------------------------
{
	if (actorVME == NULL || actorVME->GetOutput() == NULL ||
		actorVME->GetOutput()->GetVTKData() == NULL)
		return NULL;

	vtkMAFSmartPointer< vtkSphereSource > ball;
	ball->SetCenter(const_cast<double*>(centre));
	ball->SetRadius(radius);
	ball->SetPhiResolution(12);				//smooth ball
	ball->SetThetaResolution(12);

	vtkMAFSmartPointer< vtkPolyDataMapper > ballMapper;
	ballMapper->SetInput(ball->GetOutput());	

	vtkActor* ballActor = vtkActor::New();
	ballActor->SetMapper(ballMapper);	
	ballActor->PickableOff();
	ballActor->SetUserTransform(actorVME->GetOutput()->GetTransform()->GetVTKTransform());
	
  return ballActor;
}

//----------------------------------------------------------------------------
//Creates visual pipe for visualization of bounding box of actorVME 
/*virtual*/ vtkActor* medPipeJoint::CreateBBoxPipe(mafVME* actorVME, double tuberadius)
//----------------------------------------------------------------------------
{
	if (actorVME == NULL || actorVME->GetOutput() == NULL ||
		actorVME->GetOutput()->GetVTKData() == NULL)
		return NULL;

	vtkMAFSmartPointer< vtkOutlineFilter > outlineBox;
	outlineBox->SetInput(actorVME->GetOutput()->GetVTKData());	

	vtkMAFSmartPointer< vtkTubeFilter > tubeFilter;
	tubeFilter->SetInput(outlineBox->GetOutput());
	tubeFilter->SetNumberOfSides(6);										//6 sides
	tubeFilter->SetRadius(tuberadius);	
	
	vtkMAFSmartPointer< vtkPolyDataMapper > outlineMapper;
	outlineMapper->SetInput(tubeFilter->GetOutput());	

	vtkActor* outlineActor = vtkActor::New();
	outlineActor->SetMapper(outlineMapper);	
	outlineActor->PickableOff();
	outlineActor->SetUserTransform(actorVME->GetOutput()->GetTransform()->GetVTKTransform());
	
  return outlineActor;
}

//----------------------------------------------------------------------------
//Creates visual pipe for visualization of arrow started at the centre position directing in the given direction dir and having the given length
/*virtual*/ vtkActor* medPipeJoint::CreateArrowPipe(mafVME* actorVME, const double* centre, const double* dir, double length)
//----------------------------------------------------------------------------
{
	const double EPS = 1e-8;

	//the arrow is from (0,0,0) to (1,0,0), i.e., it lies on x-axis
	vtkMAFSmartPointer< vtkArrowSource > arrow;
			
	//we want to rotate the axis so that it has the specified direction,
	//which actually means to rotate the coordinate system around y-axis
	//and around z-axis such that x-axis corresponds with dir direction
	vtkMAFSmartPointer< vtkTransform > trans;	
	trans->PostMultiply();		

	//first, rotate it around z axis
	//project the vector dir onto the plane XY	
	double rproj[3] = {dir[0], dir[1], 0.0};
	double r = vtkMath::Norm(rproj);
	if (r >= EPS)	//vector collinear with z-axis does not have projection
		trans->RotateZ( medVMEJoint::ComputeAngle(dir[0] / r, dir[1] / r));
	
	//now, rotate it around y axis
	//project the vector dir onto the plane XZ	
	rproj[1] = 0.0; rproj[2] = dir[2];	
	r = vtkMath::Norm(rproj);
	if (r >= EPS)	//vector collinear with y-axis does not have projection
		trans->RotateY( - medVMEJoint::ComputeAngle(dir[0] / r, dir[2] / r));

	trans->Scale(length, length, length);
	trans->Translate(centre);

	vtkMAFSmartPointer< vtkTransformPolyDataFilter > PDF;   
	PDF->SetInput(arrow->GetOutput());
	PDF->SetTransform(trans);  

	
	vtkMAFSmartPointer< vtkPolyDataMapper > arrowMapper;
	arrowMapper->SetInput(PDF->GetOutput());	

	vtkActor* arrowActor = vtkActor::New();
	arrowActor->SetMapper(arrowMapper);	
	arrowActor->PickableOff();
	arrowActor->SetUserTransform(actorVME->GetOutput()->GetTransform()->GetVTKTransform());
	
  return arrowActor;
}

//----------------------------------------------------------------------------
//Updates the radius of vtkSphereSource at the beginning of the pipe ending with the given actor. 
/*virtual*/ void medPipeJoint::UpdateSphereRadius(vtkActor* actor, double radius)
//----------------------------------------------------------------------------
{
	if (actor != NULL)
	{
		vtkMapper* mapper = actor->GetMapper();		
		vtkSphereSource* source = vtkSphereSource::SafeDownCast(mapper->GetInput()->GetSource());
		_VERIFY_RET(source != NULL);

		source->SetRadius(radius);
		mapper->Update();
	}
}

//----------------------------------------------------------------------------
//Updates the radius of vtkTubeFilter in the pipe ending with the given actor. 
/*virtual*/ void medPipeJoint::UpdateTubeRadius(vtkActor* actor, double radius)
	//----------------------------------------------------------------------------
{	
	if (actor != NULL)
	{
		vtkMapper* mapper = actor->GetMapper();		
		vtkTubeFilter* source = vtkTubeFilter::SafeDownCast(mapper->GetInput()->GetSource());
		_VERIFY_RET(source != NULL);

		source->SetRadius(radius);
		mapper->Update();
	}
}

//----------------------------------------------------------------------------
mafGUI *medPipeJoint::CreateGui()
//----------------------------------------------------------------------------
{
	mafGUI* gui = Superclass::CreateGui();

	gui->Bool(ID_JOINT_DISPLAY, "Show joint", &m_EnableJoint, 1, "Show the joint.");
	gui->Double(ID_JOINT_RADIUS, "Joint radius", &m_JointRadius);

	gui->Bool(ID_AXIS_DISPLAY, "Show axis", &m_EnableAxis, 1, "Show the rotation axis at the joint.");
	gui->Double(ID_AXIS_LENGTH, "Axis length", &m_AxisLength);

	gui->Bool(ID_MABOX_DISPLAY, "Show MA bounding box", &m_EnableMABBox, 1, "Show the bounding box of the main actor.");
	gui->Bool(ID_SABOX_DISPLAY, "Show SA bounding box", &m_EnableSABBox, 1, "Show the bounding box of the second actor.");
	gui->Double(ID_MASABOX_RADIUS, "BBox edge radius", &m_BBoxTubeRadius);

	gui->Bool(ID_MAC_DISPLAY, "Show MA centroid", &m_EnableMAC, 1, "Show the centroid of the main actor.");
	gui->Bool(ID_SAC_DISPLAY, "Show SA centroid", &m_EnableSAC, 1, "Show the centroid of the second actor.");
	gui->Double(ID_MASAC_RADIUS, "Centroid radius", &m_CentroidRadius);
  
	return gui;
}

//----------------------------------------------------------------------------
void medPipeJoint::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{	
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch (e->GetId())
		{
		case ID_JOINT_RADIUS:			
			UpdateSphereRadius(m_JointPositionActor, m_JointRadius);
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			m_bSizesInitialized = true;
			break;

		case ID_AXIS_LENGTH:
			DestroyPipe();	//this is easier to reconstruct completely
			CreatePipe();
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			m_bSizesInitialized = true;
			break;

		case ID_MASABOX_RADIUS:
			UpdateTubeRadius(m_MainActorBBoxActor, m_BBoxTubeRadius);
			UpdateTubeRadius(m_SecondActorBBoxActor, m_BBoxTubeRadius);
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			m_bSizesInitialized = true;
			break;

		case ID_MASAC_RADIUS:
			UpdateSphereRadius(m_MainActorCentreActor, m_CentroidRadius);
			UpdateSphereRadius(m_SecondActorCentreActor, m_CentroidRadius);
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			m_bSizesInitialized = true;
			break;		

		case ID_JOINT_DISPLAY:
			if (m_JointPositionActor != NULL)
			{
				m_JointPositionActor->SetVisibility(m_EnableJoint);			
				mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			}
			break;

		case ID_AXIS_DISPLAY:
			for (int i = 0; i < 3; i++)
			{
				if (m_JointAxisActor[i] != NULL)
					m_JointAxisActor[i]->SetVisibility(m_EnableAxis);			
			}
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			break;

		case ID_MABOX_DISPLAY:
			if (m_MainActorBBoxActor != NULL)
			{
				m_MainActorBBoxActor->SetVisibility(m_EnableMABBox);			
				mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			}
			break;

		case ID_SABOX_DISPLAY:
			if (m_SecondActorBBoxActor != NULL)
			{
				m_SecondActorBBoxActor->SetVisibility(m_EnableSABBox);			
				mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			}
			break;

		case ID_MAC_DISPLAY:
			if (m_MainActorCentreActor != NULL)
			{
				m_MainActorCentreActor->SetVisibility(m_EnableMAC);			
				mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			}
			break;

		case ID_SAC_DISPLAY:
			if (m_SecondActorCentreActor != NULL)
			{
				m_SecondActorCentreActor->SetVisibility(m_EnableSAC);
				mafEventMacro(mafEvent(this,CAMERA_UPDATE));
			}
			break;
		}
	}

	if (maf_event->GetId() == VME_OUTPUT_DATA_CHANGED) 
	{
		//recreate pipe to get it
		InitSizes();	//init sizes, if needed
		DestroyPipe();
		CreatePipe();

		mafEventMacro(mafEvent(this,CAMERA_UPDATE));
	}	
	else if (maf_event->GetId() == VME_TIME_SET)
	{
		//make sure we have everything up to date
		//because the current time has changed
		if (m_Vme != NULL) 
			m_Vme->GetOutput()->Update();				
	}
    
  Superclass::OnEvent(maf_event);
}