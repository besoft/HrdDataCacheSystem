/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpPipeVolumeBrushSlice.h,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:55 $
Version:   $Revision: 1.1.2.4 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpPipeVolumeBrushSlice_H__
#define __lhpPipeVolumeBrushSlice_H__

#include "mafPipeVolumeSlice_BES.h"
#include "lhpVMEDefines.h"

//------------------------------------------------------------------------------
// Forward declarations 
//------------------------------------------------------------------------------
class vtkMAFVolumeSlicer_BES;
class vtkLHPSelectionVolumeGray;
class vtkImageData;
class vtkLookupTable;
class vtkImageMapToColors;
class vtkImageBlend;

/**
 This class is for highlighting selections (brush paintings) on the volume
 slices in lhpVolumeBrushView.
*/
class LHP_VME_EXPORT lhpPipeVolumeBrushSlice : public mafPipeVolumeSlice_BES
{
public:
	mafTypeMacro(lhpPipeVolumeBrushSlice, mafPipeVolumeSlice_BES);

	lhpPipeVolumeBrushSlice();
	virtual ~lhpPipeVolumeBrushSlice();

	/** set selection volume input for the pipe */
	void SetSelectionVolume(vtkLHPSelectionVolumeGray * selectionVolume);
	
	/** set slice origin of the volume data and the selection volume */
	void SetSlice(double* Origin, double* Normal);

	/** create the pipeline for selection highlighting */
	void CreateComposedSlice(int direction);

	void SetHighlightColour(wxColour * colour);
	void SetHighlightTransparency(int trans);

protected:

	vtkLHPSelectionVolumeGray * m_SelectionVolume;

	/** gray slice from the selection volume */
	vtkImageData * m_SelVolSliceImage[3];
	/** gray slice from the data volume */
	vtkImageData * m_DataImage[3];
	
	/**slicer for the selection volume */
	vtkMAFVolumeSlicer_BES * m_SelVolSlicer[3]; 

	/** blender for selection highlighting */
	vtkImageBlend * m_imageBlend[3]; 

	// for color highlighting
	/** selection volume lut */
	vtkLookupTable * m_SelVolColorLUT;
	/** mapper to convert selection volume slice from gray scale to color */
	vtkImageMapToColors * m_SelVolColorMap[3];

	/** mapper to convert data volume slice from gray scale to color */
	vtkImageMapToColors * m_DataVolColorMap[3];

	/** RGBA slice from the data volume */
	vtkImageData * m_DataVolColorImage[3];
	/** RGBA slice from the selection volume */
	vtkImageData * m_SelVolColorImage[3];



};

#endif