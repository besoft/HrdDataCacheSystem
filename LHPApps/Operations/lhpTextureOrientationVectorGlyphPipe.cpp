/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationVectorGlyphPipe.cpp,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.1.1.2.3 $
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
#include "vtkGlyph3D.h"
#include "vtkPointData.h"

#include "lhpTextureOrientationVectorGlyphPipe.h"
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
lhpTextureOrientationVectorGlyphPipe::lhpTextureOrientationVectorGlyphPipe(mafVME* vme, vtkRenderer *renderer)
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
  m_texFilter->SetOutputToVectorFormat() ;  // set output attributes to vector
  m_texFilter->SetProbeSizeSceneUnits(5) ;
  m_texFilter->SetSampleSpacingSceneUnits(5) ;


  // -----------------------------------
  // Set up the glyphing pipeline to display vectors
  // -----------------------------------
  m_ellipsoid = vtkSphereSource::New() ;
  m_ellipsoid->SetThetaResolution(10) ;
  m_ellipsoid->SetPhiResolution(10) ;
  m_ellipsoid->SetRadius(1.0) ;

  m_transform = vtkTransform::New() ;
  m_transform->Translate(0,0,0) ;
  m_transform->Scale(1.0, 0.2, 0.2) ; // glyphing axis is x, so make this the long axis

  m_transformPD = vtkTransformPolyDataFilter::New() ;
  m_transformPD->SetInput((vtkPolyData *)m_ellipsoid->GetOutput());
  m_transformPD->SetTransform(m_transform);

  m_glyph3D = vtkGlyph3D::New() ;
  m_glyph3D->SetInput((vtkDataSet *) m_texFilter->GetOutput());
  m_glyph3D->SetNumberOfSources(1);
  m_glyph3D->SetSource(m_transformPD->GetOutput());
  m_glyph3D->SetScaleFactor(1);
  m_glyph3D->SetScaleModeToScaleByVector() ;
  m_glyph3D->OrientOn() ;
  m_glyph3D->SetVectorModeToUseVector() ;
  m_glyph3D->SetColorModeToColorByScalar() ;  // uses zero component only and maps (0.0, 1.0) to (red, blue)

  m_glyphMapper = vtkPolyDataMapper::New() ;
  m_glyphMapper->SetInput(m_glyph3D->GetOutput()) ;
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
lhpTextureOrientationVectorGlyphPipe::~lhpTextureOrientationVectorGlyphPipe()
//------------------------------------------------------------------------------
{
  m_histEq->Delete() ;
  m_texFilter->Delete() ;
  m_ellipsoid->Delete() ;
  m_transform->Delete() ;
  m_transformPD->Delete() ;
  m_glyph3D->Delete() ;
  m_glyphMapper->Delete() ;
  m_glyphActor->Delete() ;
}





//------------------------------------------------------------------------------
// Set visibility of pipeline
void lhpTextureOrientationVectorGlyphPipe::SetVisibility(int visibility)
//------------------------------------------------------------------------------
{
  m_glyphActor->SetVisibility(visibility) ;
}



//------------------------------------------------------------------------------
// Get pointer to output polydata from pipe
vtkPolyData* lhpTextureOrientationVectorGlyphPipe::GetPolydata() 
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
void lhpTextureOrientationVectorGlyphPipe::AddProgressObserver(int vtkEventID, lhpTextureOrientationProgressCallback *callback)
//------------------------------------------------------------------------------
{
  m_texFilter->AddObserver(vtkEventID, callback) ;
}


//------------------------------------------------------------------------------
// Print self
void lhpTextureOrientationVectorGlyphPipe::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  m_glyph3D->GetOutput()->Update() ;
  vtkPolyData *poly = m_glyph3D->GetOutput() ;
  lhpTextureOrientationUseful::PrintAttributeData(os, poly) ;
}
