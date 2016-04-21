/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationTensorGlyphPipe.cpp,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.2.3 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafVME.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkStructuredPoints.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkCamera.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTensorGlyph.h"
#include "vtkPointData.h"
#include "vtkCleanPolyData.h"

#include "lhpTextureOrientationTensorGlyphPipe.h"
#include "lhpHistogramEqualizationFilter.h"
#include "lhpTextureOrientationFilter.h"
#include "lhpTextureOrientationUseful.h"
#include "lhpTextureOrientationCallbacks.h"

#include <ostream>
#include <fstream>


//------------------------------------------------------------------------------
using namespace lhpTextureOrientation ;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Constructor
lhpTextureOrientationTensorGlyphPipe::lhpTextureOrientationTensorGlyphPipe(mafVME* vme, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  // get volume data from vme (could be struct pts or rect grid)
  vtkDataSet *vtk_data = vme->GetOutput()->GetVTKData() ;
  vtk_data->Update() ;

  // get pose matrix from vme
  vtkMatrix4x4 *mat = vme->GetOutput()->GetMatrix()->GetVTKMatrix() ;


  //----------------------------------------------------------------------------
  // Equalize the histogram
  //----------------------------------------------------------------------------
  m_histEq = lhpHistogramEqualizationFilter::New() ;
  m_histEq->SetInput((vtkImageData*)vtk_data) ;
  m_histEq->SetNumberOfGreyLevels(6) ;

  // --------------------------------------------------------------
  // Calculate the texture direction at points in image
  // --------------------------------------------------------------
  m_texFilter = lhpTextureOrientationFilter::New() ;
  m_texFilter->SetInput(m_histEq->GetOutput()) ;
  m_texFilter->SetOutputToTensorFormat() ;  // set output attributes to tensor
  m_texFilter->SetProbeSizeSceneUnits(5) ;
  m_texFilter->SetSampleSpacingSceneUnits(5) ;


  // ---------------------------------------------------------------------------
  // Set up the glyphing pipeline to display vectors
  // ---------------------------------------------------------------------------
  m_ellipsoid = vtkSphereSource::New() ;  // output is centred about (0,0,0)
  m_ellipsoid->SetThetaResolution(10) ;
  m_ellipsoid->SetPhiResolution(10) ;
  m_ellipsoid->SetRadius(1.0) ;

  m_transform = vtkTransform::New() ;
  m_transform->Translate(0,0,0) ;
  m_transform->Scale(0.5, 0.5, 0.5) ;

  m_transformPD = vtkTransformPolyDataFilter::New() ;
  m_transformPD->SetInput((vtkPolyData *)m_ellipsoid->GetOutput());
  m_transformPD->SetTransform(m_transform);

  // Glyph the tensors with vtkTensorGlyph
  // The centre of the source glyph should be at (0,0,0) if symmetric flag is false.
  // NB vtkTensorGlyph throws up an annoying warning screen if the slice has no polydata point to glyph
  m_tensorGlyph = vtkTensorGlyph::New() ;
  m_tensorGlyph->SetInput((vtkDataSet *) m_texFilter->GetOutput()); // sets the polydata mask points
  m_tensorGlyph->SetExtractEigenvalues(0) ;                 // tells the filter that the tensor is already in eigenvector/eigenvalue form
  m_tensorGlyph->SetSource(m_transformPD->GetOutput());     // transform of glyph
  m_tensorGlyph->ScalingOn() ;                              // scale by eigenvalues
  m_tensorGlyph->SetScaleFactor(1);                         // scale factor 
  m_tensorGlyph->SetColorModeToScalars() ;                  // uses zero component only and maps (0.0, 1.0) to (red, blue)
  m_tensorGlyph->SetSymmetric(false) ;                      // Don't reflect glyph.

  // Calculate the normals properly because vtkTensorGlyph gets it wrong,
  // causing some ellipsoids to appear black.
  m_polyNormals = vtkPolyDataNormals::New() ;
  m_polyNormals->SetInput(m_tensorGlyph->GetOutput()) ;

  m_glyphMapper = vtkPolyDataMapper::New() ;
  m_glyphMapper->SetInput(m_polyNormals->GetOutput()) ;
  m_glyphMapper->SetScalarVisibility(1) ;

  m_glyphActor = vtkActor::New() ;
  m_glyphActor->SetMapper(m_glyphMapper) ;
  m_glyphActor->SetUserMatrix(mat) ;


  //----------------------------------------------------------------------------
  // Add actors to renderer
  //----------------------------------------------------------------------------
  renderer->AddActor(m_glyphActor) ;


  //----------------------------------------------------------------------------
  // Aim the camera at the centre of the bounding box
  //----------------------------------------------------------------------------
  vtkCamera *camera = renderer->GetActiveCamera() ;
  camera->SetFocalPoint(vtk_data->GetCenter()) ;

}




//------------------------------------------------------------------------------
// Destructor
lhpTextureOrientationTensorGlyphPipe::~lhpTextureOrientationTensorGlyphPipe()
//------------------------------------------------------------------------------
{
  m_histEq->Delete() ;
  m_texFilter->Delete() ;
  m_ellipsoid->Delete() ;
  m_transform->Delete() ;
  m_transformPD->Delete() ;
  m_tensorGlyph->Delete() ;
  m_polyNormals->Delete() ;
  m_glyphMapper->Delete() ;
  m_glyphActor->Delete() ;
}





//------------------------------------------------------------------------------
// Set visibility of pipeline
void lhpTextureOrientationTensorGlyphPipe::SetVisibility(int visibility)
//------------------------------------------------------------------------------
{
  m_glyphActor->SetVisibility(visibility) ;
}



//------------------------------------------------------------------------------
// Get pointer to output polydata from pipe
vtkPolyData* lhpTextureOrientationTensorGlyphPipe::GetPolydata() 
//------------------------------------------------------------------------------
{
  m_texFilter->GetOutput()->Update() ;
  vtkPolyData *poly = m_texFilter->GetOutput() ;

  // debugging
  //std::fstream thing2 ;
  //thing2.open("C:/Documents and Settings/Nigel/My Documents/Visual Studio Projects/MAF2/TextureOrientation1/thing2.txt", thing2.out) ;
  //lhpTextureOrientationUseful::PrintAttributeData(thing2, poly) ;
  //thing2.close() ;

  return m_texFilter->GetOutput() ;
}



//------------------------------------------------------------------------------
// Add observer to texture filter to catch user progress event
// NB The calling program is still responsible for deleting its reference count to the callback
void lhpTextureOrientationTensorGlyphPipe::AddProgressObserver(int vtkEventID, lhpTextureOrientationProgressCallback *callback)
//------------------------------------------------------------------------------
{
  m_texFilter->AddObserver(vtkEventID, callback) ;
}




//------------------------------------------------------------------------------
// Print self
void lhpTextureOrientationTensorGlyphPipe::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  m_polyNormals->GetOutput()->Update() ;
  vtkPolyData *poly = m_polyNormals->GetOutput() ;
  lhpTextureOrientationUseful::PrintAttributeData(os, poly) ;
}
