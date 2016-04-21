/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpGizmoVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-04-16 12:20:09 $
Version:   $Revision: 1.1.2.6 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpGizmoVolumeBrush_H__
#define __lhpGizmoVolumeBrush_H__


#include "mafGizmoInterface.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class lhpOp3DVolumeBrush;

class mafVMEGizmo;
class mafVME;

class vtkCubeSource;
class vtkSphereSource;
//class vtkTransformPolyDataFilter;

/**
  gizmo class for VolumeBrush, currently it supports sphere and cube shape
*/

class lhpGizmoVolumeBrush :	public mafGizmoInterface
{
public:

	lhpGizmoVolumeBrush(mafVME *input, mafObserver *listener);
	virtual ~lhpGizmoVolumeBrush(void);



	/** show the gizmo */
	void Show(bool show);
	/** get the showing status of the gizmo */
	bool GetShow() { return m_Shown; }
	
	/** set the gizmo shape to sphere or cube */
	void SetBrushShape(int brushShape);

	/** set the size (radius) of the gizmo */
	void SetSize(double s[3]);
	/** set the gizmo center */
	void SetCenter(double coord[3]);
	/** set the gizmo center */
	void SetCenter(double x, double y, double z);

	/** get gizmo vme */
	mafVME * GetOutput();

	/** get sphere source for rendering */
	vtkSphereSource * GetSphereSource() { return m_Sphere; }
	/** get cube source for rendering */
	vtkCubeSource * GetCubeSource() { return m_Cube; };

protected:
	/** 	Set input vme for the gizmo*/
	void SetInput(mafVME *vme) { m_InputVme = vme; } 

	/** Events handling */
	void OnEvent(mafEventBase *maf_event);

	/** create cube and sphere sources */
	void CreatePipeline();


	mafVME *m_InputVme;///<Register input vme

	/** brush shape, currently supporting sphere and cube */
	int m_Shape;
	/** brush size, currently only m_Size[0] used as the raidius */
	int m_Size[3];
	/** brush center */
	double m_Center[3];
	/** if the brush is shown */
	bool m_Shown;
	
	/** mafVMEGizmo brush gizmo */
	mafVMEGizmo *m_Gizmo; 

	/** sphere source */
	vtkSphereSource * m_Sphere;
	/** cube source */
	vtkCubeSource * m_Cube;

	// not used currently
	//vtkTransformPolyDataFilter * m_PolyDataFilter;

};

#endif