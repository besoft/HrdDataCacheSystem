/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPSelectionVolumeGray.cpp,v $
Language:  C++
Date:      $Date: 2010-06-08 13:36:34 $
Version:   $Revision: 1.1.2.5 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkLHPSelectionVolumeGray.h"
#include "vtkLHPImageStencilData.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkLHPSelectionVolumeGray, "$Revision: 1.1.2.5 $");

vtkStandardNewMacro(vtkLHPSelectionVolumeGray);

//----------------------------------------------------------------------------
vtkLHPSelectionVolumeGray::vtkLHPSelectionVolumeGray()
//----------------------------------------------------------------------------
{
	m_BackgroundValue = 0;
}

//----------------------------------------------------------------------------
vtkLHPSelectionVolumeGray::~vtkLHPSelectionVolumeGray()
//----------------------------------------------------------------------------
{
	
}

//----------------------------------------------------------------------------
// allocate space for an empty selection volume with give dimension
void vtkLHPSelectionVolumeGray::Create(int dim[3], double spacing[3], double origin[3])
//----------------------------------------------------------------------------
{
	SetSpacing(spacing);
	SetOrigin(origin);
	SetDimensions(dim[0], dim[1], dim[2]);
	SetNumberOfScalarComponents(1);
	SetScalarType(VTK_UNSIGNED_CHAR);
	AllocateScalars();
	Clear();
	Update();
}

//----------------------------------------------------------------------------
// fill the stencil area with designated value
void vtkLHPSelectionVolumeGray::Select(vtkLHPImageStencilData * stencil, unsigned char label)
//----------------------------------------------------------------------------
{
	unsigned char * outPtr = GetDataPointer();
	int outInc[3];
	//m_Volume->GetDimensions(dim);
	//m_Volume->GetExtent(inExt);
	GetIncrements(outInc);
	stencil->Fill(outPtr, outInc, label, Extent);
	this->SetUpdateExtent(stencil->GetExtent());
	this->Modified();

}

//----------------------------------------------------------------------------
// select by thresholding
void vtkLHPSelectionVolumeGray::SelectByThreshold(vtkImageData * volume, double lower, double upper, unsigned char label)
{
	// what will happen if the input and output volume have different dimensions and origins ??

	Clear();

	int inExt[6], outInc[3], updateExtent[6];
	//m_Volume->GetIncrements(inInc);
	//m_Volume->GetDimensions(inDim);
	volume->GetExtent(inExt);
	GetIncrements(outInc);

	updateExtent[0] = inExt[1];
	updateExtent[1] = inExt[0];
	updateExtent[2] = inExt[3];
	updateExtent[3] = inExt[2];
	updateExtent[4] = inExt[5];
	updateExtent[5] = inExt[4];


	unsigned char * outPtr; //= GetDataPointer();
	double val;

	for (int idZ = inExt[4] ; idZ <= inExt[5] ; idZ++)
	{

		for (int idY = inExt[2]; idY <= inExt[3] ; idY++)
		{

			for (int idX = inExt[0]; idX <= inExt[1] ; idX++)
			{

				val = volume->GetScalarComponentAsDouble (idX, idY, idZ, 0);
				
				// select by threshold
				if (( val >= lower)  && ( val <= upper))
				{
					// update update extent
					if (idX < updateExtent[0])
						updateExtent[0] = idX;
					if (idX > updateExtent[1])
						updateExtent[1] = idX;
					if (idY < updateExtent[2])
						updateExtent[2] = idY;
					if (idY > updateExtent[3])
						updateExtent[3] = idY;
					if (idZ < updateExtent[4])
						updateExtent[4] = idZ;
					if (idZ > updateExtent[5])
						updateExtent[5] = idZ;

					outPtr = (unsigned char *) GetScalarPointer(idX, idY, idZ);
					* outPtr = label;
				}
			}
		}
	}

	this->SetUpdateExtent(updateExtent);
	this->Modified();

}

//----------------------------------------------------------------------------
// unselect is selecting with the background value
void vtkLHPSelectionVolumeGray::UnSelect(vtkLHPImageStencilData * stencil)
//----------------------------------------------------------------------------
{
	Select(stencil, m_BackgroundValue);
}

//----------------------------------------------------------------------------
// set all voxels in the volume to the background
void vtkLHPSelectionVolumeGray::Clear()
//----------------------------------------------------------------------------
{
	unsigned char * inPtr = GetDataPointer();
	// fill the memory with an unsigned char value
	int dim[3];
	GetDimensions(dim);
	lhpMemSet(inPtr, m_BackgroundValue, dim[0] * dim[1] * dim[2]);
	this->Modified();
}

//----------------------------------------------------------------------------
inline unsigned char * vtkLHPSelectionVolumeGray::GetDataPointer()
//----------------------------------------------------------------------------
{
	return (unsigned char *) GetScalarPointer();
}
