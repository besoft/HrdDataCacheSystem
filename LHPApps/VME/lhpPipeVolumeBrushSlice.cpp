/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpPipeVolumeBrushSlice.cpp,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:55 $
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

#include "lhpPipeVolumeBrushSlice.h"
#include "vtkLHPSelectionVolumeGray.h"

#include "mafVME.h"
#include "vtkMAFVolumeSlicer_BES.h"

#include "vtkImageData.h"
#include "vtkImageBlend.h"
#include "vtkTexture.h"

// currently color highlighting not supported
///*
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkPointData.h"
#include "vtkActor.h"
//*/

mafCxxTypeMacro(lhpPipeVolumeBrushSlice);

//----------------------------------------------------------------------------
lhpPipeVolumeBrushSlice::lhpPipeVolumeBrushSlice()
//----------------------------------------------------------------------------
{
	m_SelectionVolume = NULL;
	for (int i = 0; i < 3; i++)
	{
		m_SelVolSlicer[i] = NULL;
		m_SelVolSliceImage[i] = NULL;
		m_DataImage[i] = NULL;
		m_imageBlend[i] = NULL;

		// color highlighting not supported
		m_SelVolColorMap[i] = NULL;
		m_SelVolColorImage[i] = NULL;
		m_DataVolColorMap[i] = NULL;
		m_DataVolColorImage[i] = NULL;
	}

	// color highlighting
	vtkNEW(m_SelVolColorLUT);
	m_SelVolColorLUT->SetRange( 0, 255 );
	m_SelVolColorLUT->SetNumberOfColors( 256 ); 
	for (int i = 0; i < 256; i++)
		m_SelVolColorLUT->SetTableValue (i, 0.0, 0.0, 0.0, 0.0);

	// TODO: remove the hard coded number
	m_SelVolColorLUT->SetTableValue (200, 1.0, 1.0, 0.0, 1.0);
	m_SelVolColorLUT->SetTableValue (199, 1.0, 1.0, 0.0, 1.0);
	
	
}

//----------------------------------------------------------------------------
lhpPipeVolumeBrushSlice::~lhpPipeVolumeBrushSlice()
//----------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		vtkDEL(m_SelVolSlicer[i]);
		vtkDEL(m_SelVolSliceImage[i]);
		vtkDEL(m_DataImage[i]);
		vtkDEL(m_imageBlend[i]);

		vtkDEL(m_SelVolColorMap[i]);
		vtkDEL(m_SelVolColorImage[i]);
		vtkDEL(m_DataVolColorMap[i]);
		vtkDEL(m_DataVolColorImage[i]);
	}

	vtkDEL(m_SelVolColorLUT);

	m_SelectionVolume = NULL;

}

//----------------------------------------------------------------------------
void lhpPipeVolumeBrushSlice::SetSelectionVolume(vtkLHPSelectionVolumeGray * selectionVolume)
//----------------------------------------------------------------------------
{
	if (NULL == selectionVolume)
		return;

	m_SelectionVolume = selectionVolume;

	// for x, y and z, only one slice is initialized in the original slice view
	if (m_SliceDirection == SLICE_X || m_SliceDirection == SLICE_ORTHO)
	{
		CreateComposedSlice(0);
	}
	if (m_SliceDirection == SLICE_Y || m_SliceDirection == SLICE_ORTHO)
	{
		CreateComposedSlice(1);
	}
	if (m_SliceDirection == SLICE_Z || m_SliceDirection == SLICE_ORTHO)
	{
		CreateComposedSlice(2);
	}

}

//----------------------------------------------------------------------------
void lhpPipeVolumeBrushSlice::CreateComposedSlice(int direction)
//----------------------------------------------------------------------------
{
	
	// get spacing
	double selVolSpacing[3], dataVolSpacing[3];
	m_SelectionVolume->GetSpacing(selVolSpacing);

	vtkDataSet *vtk_data = m_Vme->GetOutput()->GetVTKData();
	vtk_data->Update();
	
	if(vtk_data->IsA("vtkImageData") || vtk_data->IsA("vtkStructuredPoints"))
	{
		((vtkImageData *)vtk_data)->GetSpacing(dataVolSpacing);
	}
	else
		return; // input data is not vtkImageData
	

	// create selection volume slice
	if (m_SelVolSlicer[direction])
		vtkDEL(m_SelVolSlicer[direction]);

	vtkNEW(m_SelVolSlicer[direction]);
	m_SelVolSlicer[direction]->SetPlaneOrigin(m_Origin[0], m_Origin[1], m_Origin[2]);
	m_SelVolSlicer[direction]->SetPlaneAxisX(m_XVector[direction]);
	m_SelVolSlicer[direction]->SetPlaneAxisY(m_YVector[direction]);
	m_SelVolSlicer[direction]->SetInput(m_SelectionVolume);
	// disable gpu to accelerate slicing
	m_SelVolSlicer[direction]->SetGPUEnabled(0);

	// images from selectionVolume
	// may be there is no need to use this slicer ?
	vtkNEW(m_SelVolSliceImage[direction]);
	m_SelVolSliceImage[direction]->SetScalarType(VTK_UNSIGNED_CHAR);
	m_SelVolSliceImage[direction]->SetNumberOfScalarComponents(1);
	//m_SelVolSliceImage[direction]->SetNumberOfScalarComponents(3);
	m_SelVolSliceImage[direction]->SetExtent(0, m_TextureRes - 1, 0, m_TextureRes - 1, 0, 0);
	m_SelVolSliceImage[direction]->SetSpacing(selVolSpacing);
	m_SelVolSlicer[direction]->SetOutput(m_SelVolSliceImage[direction]);
	m_SelVolSlicer[direction]->Update();

	// volume slice
	//m_SlicerImage[direction]->SetOutput(m_Image[direction]);
	//m_SlicerImage[direction]->Update();

	vtkNEW(m_DataImage[direction]);
	//m_DataImage[direction]->SetScalarType(vtk_data->GetPointData()->GetScalars()->GetDataType());
	m_DataImage[direction]->SetScalarTypeToUnsignedChar();
	//m_DataImage[direction]->SetNumberOfScalarComponents(vtk_data->GetPointData()->GetScalars()->GetNumberOfComponents());
	m_DataImage[direction]->SetNumberOfScalarComponents(1);
	m_DataImage[direction]->SetExtent(0, m_TextureRes - 1, 0, m_TextureRes - 1, 0, 0);
	m_DataImage[direction]->SetSpacing(dataVolSpacing);
	m_SlicerImage[direction]->SetOutput(m_DataImage[direction]);

	// color highlighting
	vtkNEW(m_DataVolColorMap[direction]);
	vtkNEW(m_DataVolColorImage[direction]);
	m_DataVolColorMap[direction]->SetLookupTable(m_ColorLUT);
	m_DataVolColorMap[direction]->SetOutputFormatToRGBA();
	m_DataVolColorMap[direction]->SetInput(m_DataImage[direction]);
	m_DataVolColorImage[direction]->SetExtent(0, m_TextureRes - 1, 0, m_TextureRes - 1, 0, 0);
	m_DataVolColorImage[direction]->SetSpacing(dataVolSpacing);
	m_DataVolColorMap[direction]->Update();

	// m_SelVolColorMap is an RGB image
	vtkNEW(m_SelVolColorMap[direction]);
	vtkNEW(m_SelVolColorImage[direction]);
	m_SelVolColorMap[direction]->SetLookupTable(m_SelVolColorLUT);
	m_SelVolColorMap[direction]->SetOutputFormatToRGBA();
	m_SelVolColorMap[direction]->SetInput(vtkImageData::SafeDownCast(m_SelVolSlicer[direction]->GetOutput()));
	m_SelVolColorImage[direction]->SetExtent(0, m_TextureRes - 1, 0, m_TextureRes - 1, 0, 0);
	m_SelVolColorImage[direction]->SetSpacing(selVolSpacing);
	m_SelVolColorMap[direction]->Update();

	// vtkImageBlend to blend 2 images from data volume and selection volume
	vtkNEW(m_imageBlend[direction]);
	m_imageBlend[direction]->SetBlendModeToNormal();
	m_imageBlend[direction]->SetOpacity(0, 0.382);
	m_imageBlend[direction]->SetOpacity(1, 0.618);
	m_imageBlend[direction]->SetInput(0, vtkImageData::SafeDownCast(m_DataVolColorMap[direction]->GetOutput()));
	m_imageBlend[direction]->SetInput(1, vtkImageData::SafeDownCast(m_SelVolColorMap[direction]->GetOutput()));
	// m_Image is the texture
	m_imageBlend[direction]->SetOutput(m_Image[direction]);
	m_imageBlend[direction]->Update();

	// as m_Texture is the blended image, the original color LUT cannot be used on it
	m_Texture[direction]->MapColorScalarsThroughLookupTableOff();
	
}

//----------------------------------------------------------------------------
//Set the origin and normal of the slice
void lhpPipeVolumeBrushSlice::SetSlice(double* Origin, double* Normal)
//----------------------------------------------------------------------------
{
	Superclass::SetSlice(Origin, Normal);

	for(int i=0;i<3;i++)
	{
		if(m_SelVolSlicer[i])
		{
			m_SelVolSlicer[i]->SetPlaneOrigin(m_Origin[0], m_Origin[1], m_Origin[2]);
			m_SelVolSlicer[i]->SetPlaneAxisX(m_XVector[i]);
			m_SelVolSlicer[i]->SetPlaneAxisY(m_YVector[i]);
		}
	}

}


//----------------------------------------------------------------------------
void lhpPipeVolumeBrushSlice::SetHighlightColour(wxColour * colour)
//----------------------------------------------------------------------------
{
	m_SelVolColorLUT->SetTableValue (200, colour->Red()/ 255.0, colour->Green()/ 255.0, colour->Blue()/ 255.0, 1.0);
	m_SelVolColorLUT->SetTableValue (199, colour->Red()/ 255.0, colour->Green()/ 255.0, colour->Blue()/ 255.0, 1.0);
}

//----------------------------------------------------------------------------
void lhpPipeVolumeBrushSlice::SetHighlightTransparency(int iTrans)
//----------------------------------------------------------------------------
{
	double trans = iTrans / 255.0;

	if ( m_SliceDirection == SLICE_ORTHO)
	{
		for (int direction = 0; direction < 3; direction++)
		{
			m_imageBlend[direction]->SetOpacity(0, 1 - trans);
			m_imageBlend[direction]->SetOpacity(1, trans);
			//m_Texture[direction]->Update();
		}
	}
	else
	{
		m_imageBlend[m_SliceDirection]->SetOpacity(0, 1 - trans);
		m_imageBlend[m_SliceDirection]->SetOpacity(1, trans);
		//m_Image[m_SliceDirection]->Update();
	}
}
