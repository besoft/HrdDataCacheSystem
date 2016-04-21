/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpViewVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.4 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpVolumeBrushView_H__
#define __lhpVolumeBrushView_H__

#include "mafViewOrthoSlice.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class lhpGizmoVolumeBrush;
class vtkLHPSelectionVolumeGray;

class vtkRenderWindowInteractor;
class vtkVolume;
class vtkActor;
class vtkSphereSource;
class vtkCubeSource;

/**
  lhpViewVolumeBrush is based on mafViewOrthoSlice to reuse its four slice based
  subviews (PERSPECTIVE and X, Y, Z) and the slice gimzo.But special handling of
  VMEs need to be added for it is used by an operation   and cannot be managered 
  by the mafViewMananager. Besides that lhpViewVolumeBrush also changed the slice
  rendering pipe for it needs to draw brush paintings on the volume slice.
*/

class lhpViewVolumeBrush : public mafViewOrthoSlice
{
public:
	lhpViewVolumeBrush(wxString label = "View VolumeBrush");
	virtual ~lhpViewVolumeBrush(); 

	mafTypeMacro(lhpViewVolumeBrush, mafViewOrthoSlice);

	/** set the selecton volume data to the perspective view */
	void AddRenderingVolume(vtkVolume * volume);
	/** remove the volume data from the perspective view */
	void RemoveRenderingVolume(vtkVolume * volume);
	/** set selection volume for painting highlighting */
	void SetSelectionVolume(vtkLHPSelectionVolumeGray * volume);
	/** enable the selection volume, not used currently */
	void EnableSelectionVolume(bool b);

	/** add slice gizmo vme manully */
	void SliceGizmoAdd();
	/** remove slice gizmo vme manully */
	void SliceGizmoRemove();

	
	/** add brush gizmo vme manully */
	void BrushGizmoAdd();
	/** only VmeRemove the brush gizmo from the scenegraph */
	void BrushGizmoRemove();
	/** remvoe and delete the brush gizmo */
	void BrushGizmoDelete();
	
	/** show or hide brush gizmo */
	void ShowBrush(bool show);
	/** get the showing status of the brush gizmo */
	bool GetBrushShow();
	
	/** set brush suze */
	void SetBrushSize(double s[3]);
	/** set brush center */
	void SetBrushCenter(double pos[3]);
	/** set brush shape */
	void SetBrushShape(int t);

	void SetHighlightColour(wxColour * colour);
	void SetHighlightTransparency(int trans);

	/** get the gizmo source for stencil generation */
	vtkSphereSource * GetGizmoSphereSource();
	/** get the gizmo source for stencil generation */
	vtkCubeSource * GetGizmoCubeSource();
	
	/** get the gizmo for rendering */
	mafVME * GetGizmoOutput();

	/** update the pipes for slice generation and rendering */
	//void UpdateSlice();

	/** constraint the mouse clicked point to the slice plane*/
	void UpdatePointToSlicePlane(double Pick3D[], vtkRenderWindowInteractor * interactor);

	/**
	override mafViewOrthoSlice::PackageView to change the pipe 
	for slice rendering 
	*/
	void PackageView();

	/** if the interactor is on the perspective view */
	bool IsPerspectiveView(vtkRenderWindowInteractor * interactor);

protected:

	/** the selection volume */
	vtkLHPSelectionVolumeGray * m_SelectionVolume;

	/** the volume brush gizmo */
	lhpGizmoVolumeBrush * m_BrushGizmo;

	/** 
	if the selection volume has been enabled, not used currently
	*/
	bool m_SelVolEnabled;

	/** 
	override mafViewOrthoSlice::CreateGUI to redefine the GUI panel 
	for removing some controls not needed for VolumeBrush
	*/
	virtual mafGUI * CreateGui();

	/** get the subview id from the interactor */
	int GetSubviewId(vtkRenderWindowInteractor * interactor);


};


#endif

