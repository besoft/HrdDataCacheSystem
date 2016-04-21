/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpInteractorVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.8 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpVolumeBrushInteractor_H__
#define __lhpVolumeBrushInteractor_H__

#include "mafInteractorPER.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class lhpViewVolumeBrush;
class lhpFrameVolumeBrush;
class vtkLHPImageStencilData;
class vtkLHPSelectionVolumeGray;
class vtkLHPVolumeTextureMapper3D;

class mafDeviceButtonsPadMouse;
class mafEventInteraction;
class vtkMAFContourVolumeMapper;

class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;



/**
  class for handling mouse click and move events in lhpViewVolumeBrush. 
  It also transfers event handling calls from lhpOp3DVolumeBrush to 
  lhpViewVolumeBrush, such as changing of brush shape and size.
*/
class lhpInteractorVolumeBrush : 	public mafInteractorPER
{
public:
	enum BRUSH_OP
	{
		BRUSH_OP_SELECT = 0,
		BRUSH_OP_UNSELECT,
	};

	mafTypeMacro(lhpInteractorVolumeBrush, mafInteractorPER);

	/** set brush shape */
	void SetBrushShape(int s);
	/** set brush size (radius) */
	void SetBrushSize(int s);
	/** set selection volume rendering mode */
	void SetSelectionVolumeRenderMode(int s);

	void SetHighlightColour(wxColour * colour);
	void SetHighlightTransparency(int trans);

	/** set the data volume */
	void SetVolumeData(vtkImageData * );
	/** set selection volume */
	void SetSelectionVolume(vtkLHPSelectionVolumeGray * );

	/** set VolumeBrush view */
	void SetView(lhpViewVolumeBrush * pView);
	void SetFrame(lhpFrameVolumeBrush * pFrame);

	/** show or hide the brush gizmo from the lhpOp3DVolumeBrush */
	void ShowBrush(bool b);

	// Stencil functions
	/** show selections */
	void ShowSelectionVolume();
	/** hide selections */
	void HideSelectionVolume();
	/** clear selections */
	void ClearSelectionVolume();
	/** select by threshold input by the user */
	void SelectByThreshold(double lower, double upper);

	void UpdateAllViews();

protected:
	lhpInteractorVolumeBrush();
	virtual ~lhpInteractorVolumeBrush();
	
	
	// brush updating
	
	/** calculate the brush postion from mouse clicks */
	void UpdateBrushPosition();
	
	/** update the bounding box of the brush after size or position change */
	void UpdateBrushBoundingBox();
	
	/** update the size and position of the brush gizmo */
	void UpdateBrushGizmoPositionAndSize();

	
	// stencil updating
	
	/** update the translation of the brush stencil */
	void UpdateBrushStencil();
	
	/** stroke generation: to get a smooth shape of brush painting */
	void UpdateStrokeStencil();
	
	/** test if the painting intersect with the volume */
	bool IsBrushIntersectWithVolume();

	/** update the selection volume according to the selection stencil */
	void UpdateSelectionVolume(int op, unsigned char label);
	

	// event handling
	virtual void OnLeftButtonDown   (mafEventInteraction *e);
	virtual void OnLeftButtonUp     (mafEventInteraction *e);
	//virtual void OnMiddleButtonDown (mafEventInteraction *e);
	//virtual void OnMiddleButtonUp   (mafEventInteraction *e);
	virtual void OnRightButtonDown  (mafEventInteraction *e);
	virtual void OnRightButtonUp    (mafEventInteraction *e);
	//virtual void OnButtonDown       (mafEventInteraction *e);
	//virtual void OnButtonUp         (mafEventInteraction *e); 

	virtual void OnMove             (mafEventInteraction *e);
	virtual void OnChar             (mafEventInteraction *e) {};

	
	
	vtkRenderer * m_CurrentRenderer;
	vtkRenderWindowInteractor * m_Interactor;
	mafDeviceButtonsPadMouse * m_Mouse;
	/** the view for volume brush operation */
	lhpViewVolumeBrush * m_View;
	
	/* the window frame holding lhpViewVolumeBrush */
	lhpFrameVolumeBrush * m_Frame;

	/** if the mouse is dragging with left button down */
	bool m_DraggingLeft;
	
	/** mouse click position in 2D */
	double  m_PickedPoint2D[2];
	/** mouse click position in 3D */
	double  m_PickedPoint3D[4];

	/** selection volume rendering mode : none, isosurface or volume */
	int m_StencilRenderMode;
	/** brush shape, sphere or cube */
	int m_BrushShape;
	/** 
	brush size(radius), currently only m_BrushSize[0] is used */
	double m_BrushSize[3]; 
	/** brush bounding box */
	double m_boundsBrush[6];
	/** brush position */
	double m_BrushCenter[3];
	/** last brush position */
	double m_LastBrushCenter[3];

	/** value for selection, currently is 200 */
	unsigned char m_CurrentSelectionLabel;


	// stencil stuff
	/** brush stencil, a sphere or cube */
	vtkLHPImageStencilData * m_pBrushStencilData;
	/** 
	translation from the default brush origin to the current 3D position,
	for reuse of the brush stencil
	*/
	int m_transBrushStencil[3];
	/** stencil of a stroke, which is formed by continuous moving of brush stencils */
	vtkLHPImageStencilData * m_pStrokeStencilData;

	/** volume data */
	vtkImageData * m_VolumeData;
	/** spacing of the volume data */
	double m_Spacing[3];
	/** origin of the volume data */
	double m_Origin[3];
	/** volume data extent */
	int m_Extent[6]; 

	/** selection volume */
	vtkLHPSelectionVolumeGray * m_SelectionVolumeData;
	
	// for volume rendering of selections
	/** 3d texture volume rendering mapper */
	vtkLHPVolumeTextureMapper3D * m_SelectionVolumeMapper;
	/** vtkVolume of the selection volume */
	vtkVolume * m_SelectionVolumeRendered;
	/** opacity transfer function for selection volume */
	vtkPiecewiseFunction * m_OpacityTransferFunction;
	/** color transfer function for selection volume */
	vtkColorTransferFunction * m_ColorTransferFunction;
	/** volume property for volume rendering of the selection volume */
	vtkVolumeProperty * m_VolumeProperty;

	/** maf contour mapper for isosurface rendering of a selection volume */
	vtkMAFContourVolumeMapper   *m_ContourMapper; 
	/** contour value for m_ContourMapper */
	double m_ContourValue;
	/** alpha value for m_ContourMapper */
	double m_AlphaValue;

private:

};


#endif