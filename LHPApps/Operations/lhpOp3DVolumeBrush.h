/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpOp3DVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.11 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lphOp3DVolumeBrush_H__
#define __lphOp3DVolumeBrush_H__

#include "mafop.h"
#include "lhpOperationsDefines.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class mafGUIDialog;
class mafDeviceManager;
class mafVMEVolumeGray;
class mafAction;
class mafInteractor;
class mafInteractorPicker;
class mafInteractorSER;
class medDeviceButtonsPadMouseDialog;
class vtkLHPSelectionVolumeGray;
class lhpFrameVolumeBrush;
class lhpInteractorVolumeBrush;
class lhpViewVolumeBrush;


/** 
Operation to select voxels in a 3D paintbrush style
*/
class LHP_OPERATIONS_EXPORT lhpOp3DVolumeBrush : public mafOp
{
public:

	enum BRUSH_SHAPE
	{
		BRUSH_SHAPE_SPHERE = 0,
		BRUSH_SHAPE_CUBE,
	};

	enum RENDER_MODE
	{
		RENDER_MODE_NONE = 0,
		RENDER_MODE_ISOSURFACE,
		RENDER_MODE_VOLUME,
	};

	lhpOp3DVolumeBrush(wxString label = "Volume Brush");
	~lhpOp3DVolumeBrush(void);

	mafTypeMacro(lhpOp3DVolumeBrush, mafOp);

	mafOp* Copy();

	/** Class for handle mafEvent*/
	virtual void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* vme);

	/** 
	Static copy of Accept(), required so that we can pass the function
	pointer to the VME_CHOOSE event */
	static bool AcceptStatic(mafNode* vme);

	/** Builds operation's GUI by calling CreateOpDialog() method. */
	void OpRun();

	/** Execute the operation. */
	virtual void OpDo();

	/** Makes the undo for the operation. */
	void OpUndo();

	/** remove the panel on operation exit */
	void HideGui();

protected:
	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);

	/** create op GUI panel */
	void CreateGUI();
	
	/** Create the volume brush view window */
	void CreateOpFrame();

	/** Delete volume brush view window */
	void DeleteOpFrame();

	/** load an existing selection volume*/
	void OnLoadSelectionVolume();

	/** check if the selecion volume to be loaded can be accepted */
	bool AcceptSelectionVolume(mafNode * vme);

	mafVMEVolumeGray* m_Volume; //<Input volume

	mafInteractorSER *m_SER;                            //<Static event router

	mafDeviceManager *m_DeviceManager;        //<Device manager
	medDeviceButtonsPadMouseDialog* m_DialogMouse;
	mafInteractorPicker* m_Picker;                      //<Picker to allow placeholder interaction
	mafAction* m_Action;                      //<Action to allow placeholder interaction

	mafInteractor *m_OldBehavior;             //<Old volume behavior

	/** dimension of the input volume data */
	int m_Dimensions[3];
	/** spacing of the input volume data */
	double m_Spacing[3];
	/** origin of the input volume data */
	double m_Origin[3];
	
	/** brush shape: sphere or cube */
	int m_BrushShape;
	/** brush size (radius) */
	int m_BrushSize;
	/** 3D render mode : none, iso surface or volume */
	int m_RenderMode;

	/** lower threshold for threshold based selection */
	double m_LowerThreshold;
	/** upper threshold for threshold based selection */
	double m_UpperThreshold;

	/** colour for selection highlighting */
	wxColour * m_ColourHighlight;
	/** transprency of selection highlighting */
	int m_TransHighlight;
	
	/** interactor for handling mouse events */
	lhpInteractorVolumeBrush * m_PER;
	/** the window frame holding lhpViewVolumeBrush */
	lhpFrameVolumeBrush * m_Frame;

	/** the op window based on mafViewOrthoSlice */
	lhpViewVolumeBrush * m_View;


	/** mafVME holding the selection volume */
	mafVMEVolumeGray* m_SelectionVME;
	/** the vtkStructuredPoint selection volume */
	vtkLHPSelectionVolumeGray * m_SelectionVolume;
	/** pointer to the loaded selection VME */
	mafVMEVolumeGray* m_LoadedSelectionVME;


private:


};

#endif
