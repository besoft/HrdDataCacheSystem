/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpGizmoVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2010-04-14 16:27:45 $
Version:   $Revision: 1.1.2.6 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpGizmoVolumeBrush.h"
#include "lhpOp3DVolumeBrush.h"

#include "mafVMEGizmo.h"
#include "mmaMaterial.h"

#include "vtkCubeSource.h"
#include "vtkSphereSource.h"
#include "vtkProperty.h"
#include "vtkPolyData.h"
//#include "vtkTransformPolyDataFilter.h"

//----------------------------------------------------------------------------
lhpGizmoVolumeBrush::lhpGizmoVolumeBrush(mafVME *input, mafObserver *listener)
//----------------------------------------------------------------------------
{

	assert(input != NULL);
	m_InputVme = input;

	////-----------------
	// create pipeline stuff
	////-----------------
	CreatePipeline();

	mafNEW(m_Gizmo);
	m_Gizmo->SetName("VolumeBrushGizmo");
	m_Gizmo->ReparentTo(m_InputVme);


	SetBrushShape(lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE);

	// hide gizmo at creation
	this->Show(false);

	// ask the manager to create the pipeline
	mafEventMacro(mafEvent(this,VME_SHOW,m_Gizmo,true));
}

//----------------------------------------------------------------------------
lhpGizmoVolumeBrush::~lhpGizmoVolumeBrush(void)
//----------------------------------------------------------------------------
{
	vtkDEL(m_Cube);
	vtkDEL(m_Sphere);
	// not used currently
	//vtkDEL(m_PolyDataFilter);
	mafDEL(m_Gizmo);
	
}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::CreatePipeline()
//----------------------------------------------------------------------------
{
	// create pipeline for XMIN box gizmo 
	// calculate diagonal of m_InputVme space bounds 
	//double b[6];
	//m_InputVme->GetOutput()->GetBounds(b);

	//vector<double> dim;
	//dim.push_back(b[1] - b[0]);
	//dim.push_back(b[3] - b[2]);
	//dim.push_back(b[5] - b[4]);

	vtkNEW(m_Sphere);
	m_Sphere->SetThetaResolution (16);
	m_Sphere->SetPhiResolution (16);
	vtkNEW(m_Cube);

	// not used currently
	//vtkNEW(m_PolyDataFilter);

	//SetCenter((b[1] + b[0])/2,
	//	(b[3] + b[2])/2, (b[5] - b[4])/2);

	SetCenter(0, 0, 0);

	//vector<double>::iterator result;
	//result = min_element(dim.begin(), dim.end());

	//double min_dim = *result;
	//= min_dim / 8;
	m_Size[0] = m_Size[1] = m_Size[2] = 10;


}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::Show(bool show)
//----------------------------------------------------------------------------
{
	// use VTK opacity instead of VME_SHOW to speed up the render
	double opacity = show ? 1.0 : 0;
	m_Gizmo->GetMaterial()->m_Prop->SetOpacity(opacity);
	m_Shown = show;
}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::SetBrushShape(int brushShape)
//----------------------------------------------------------------------------
{
	vtkPolyData * data = m_Gizmo->GetData();

	if (lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE == brushShape)
	{
		m_Gizmo->SetData(m_Sphere->GetOutput());
		m_Gizmo->Update();
		vtkDEL(data);
		//m_PolyDataFilter->SetInput(m_Cube->GetOutput());
	}
	else if (lhpOp3DVolumeBrush::BRUSH_SHAPE_CUBE == brushShape)
	{
		m_Gizmo->SetData(m_Cube->GetOutput());
		m_Gizmo->Update();
		vtkDEL(data);
		//m_PolyDataFilter->SetInput(m_Cube->GetOutput());
	}
}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::SetSize(double s[3])
//----------------------------------------------------------------------------
{
	for(int i=0;i<3;i++)
		m_Size[i] = s[i];

	m_Sphere->SetRadius(m_Size[0]);

	m_Cube->SetXLength(m_Size[0] * 2);
	m_Cube->SetYLength(m_Size[1] * 2);
	m_Cube->SetZLength(m_Size[2] * 2);
}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::SetCenter(double coord[3])
//----------------------------------------------------------------------------
{
	SetCenter(coord[0], coord[1], coord[2]);
}

//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::SetCenter(double x,  double y, double z)
//----------------------------------------------------------------------------
{
	m_Center[0] = x;
	m_Center[1] = y;
	m_Center[2] = z;

	m_Cube->SetCenter(m_Center);
	m_Sphere->SetCenter(m_Center);
}


//----------------------------------------------------------------------------
void lhpGizmoVolumeBrush::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	// forward events to the listener
	maf_event->SetSender(this);
	mafEventMacro(*maf_event);
}

//----------------------------------------------------------------------------
mafVME * lhpGizmoVolumeBrush::GetOutput() 
//----------------------------------------------------------------------------
{
	return m_Gizmo; 
};

/*
  m_PolyDataFilter = vtkTransformPolyDataFilter::New();
  m_PolyDataFilter->SetInput(m_Cube->GetOutput());
  m_Filter->SetTransform(m_TranslateBoxTr);
  */