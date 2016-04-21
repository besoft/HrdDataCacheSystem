/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleVisualPipes.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
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
#include "vtkCubeSource.h"
#include "vtkProperty.h"
#include "vtkStructuredPoints.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkMatrix4x4.h"
#include "vtkMAFVolumeSlicer.h"
#include "vtkImageData.h"
#include "vtkTexture.h"

#include "lhpMultiscaleVisualPipes.h"

#include <cstdlib>
#include <ostream>

//------------------------------------------------------------------------------
using namespace lhpMultiscale ;
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Surface pipeline constructor
lhpMultiscaleSurfacePipeline::lhpMultiscaleSurfacePipeline(mafVME* vme, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  this->SetType(MSCALE_SURFACE_PIPE) ;

  // get polydata from vme
  vtkPolyData* polydata = vtkPolyData::SafeDownCast(vme->GetOutput()->GetVTKData());

  m_mapper = vtkPolyDataMapper::New() ;
  m_mapper->SetInput(polydata);
  m_mapper->ScalarVisibilityOn();

  m_actor = vtkActor::New() ;
  m_actor->SetMapper(m_mapper);

  // get pose matrix from vme
  vtkMatrix4x4 *mat = vme->GetOutput()->GetAbsMatrix()->GetVTKMatrix() ;
  m_actor->SetUserMatrix(mat) ;

  renderer->AddActor(m_actor) ;
}


//------------------------------------------------------------------------------
// Surface pipeline destructor
lhpMultiscaleSurfacePipeline::~lhpMultiscaleSurfacePipeline()
//------------------------------------------------------------------------------
{
  m_actor->Delete() ;
  m_mapper->Delete() ;
}


//------------------------------------------------------------------------------
// Surface pipeline print
void lhpMultiscaleSurfacePipeline::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  os << indent ;

  os << "surface pipe: type = " ;
  switch(this->GetType()){
    case MSCALE_SURFACE_PIPE:
      os << "SURFACE " ;
      break ;
    case MSCALE_TOKEN_PIPE:
      os << "TOKEN" ;
      break ;
    case MSCALE_SLICE_PIPE:
      os << "SLICE" ;
      break ;
    default:
      os << "?????" ;
      break ;
  }
  os << "\t" ;

  os << "visibility = " << m_actor->GetVisibility() << std::endl ;
}


//------------------------------------------------------------------------------
// Token pipeline constructor
lhpMultiscaleTokenPipeline::lhpMultiscaleTokenPipeline(vtkRenderer *renderer, int colorId) : m_colorId(colorId)
//------------------------------------------------------------------------------
{
  this->SetType(MSCALE_TOKEN_PIPE) ;

  // Set up source for sphere polydata
  m_tokenSource = vtkCubeSource::New() ;
  m_tokenSource->SetXLength(1.0) ;
  m_tokenSource->SetYLength(1.0) ;
  m_tokenSource->SetZLength(1.0) ;

  m_mapper	= vtkPolyDataMapper::New();
  m_mapper->SetInput(m_tokenSource->GetOutput());
  m_mapper->ScalarVisibilityOn();

  m_actor = vtkActor::New();
  m_actor->SetMapper(m_mapper);

  // set visibility and color
  m_actor->SetVisibility(0) ;

  double a[3] ;
  CalculateColor(a) ;
  m_actor->GetProperty()->SetColor(a) ;

  renderer->AddActor(m_actor) ;
}


//------------------------------------------------------------------------------
// Token pipeline destructor
lhpMultiscaleTokenPipeline::~lhpMultiscaleTokenPipeline()
//------------------------------------------------------------------------------
{
  m_actor->Delete() ;
  m_mapper->Delete() ;
  int i = m_tokenSource->GetReferenceCount() ;
  m_tokenSource->Delete() ;
}



//------------------------------------------------------------------------------
// Token pipeline print
void lhpMultiscaleTokenPipeline::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  os << indent ;

  os << "token pipe: type = " ;
  switch(this->GetType()){
    case MSCALE_SURFACE_PIPE:
      os << "SURFACE " ;
      break ;
    case MSCALE_TOKEN_PIPE:
      os << "TOKEN" ;
      break ;
    case MSCALE_SLICE_PIPE:
      os << "SLICE" ;
      break ;
    default:
      os << "?????" ;
      break ;
  }
  os << "\t" ;

  os << "visibility = " << m_actor->GetVisibility() << std::endl ;
}



//---------------------------------------------------------------------
// Calculate color
void lhpMultiscaleTokenPipeline::CalculateColor(double *a)
//------------------------------------------------------------------------------
{
  static double lut[15][3] =
  {
    {1,0,0}, {1,1,0}, {0,1,0}, {0,1,1}, {0,0,1}, {1,0,1}, 
    {1, 0.5, 0}, {0.5, 1, 0}, {0, 1, 0.5}, {0, 0.5, 1}, {0.5, 0, 1}, {1, 0, 0.5},
    {1, 0.5, 0.5}, {0.5, 1, 0.5},  {0.5, 0.5, 1}
  } ;

  if (m_colorId < 15){
    // look up color in lut
    a[0] = lut[m_colorId][0] ;
    a[1] = lut[m_colorId][1] ;
    a[2] = lut[m_colorId][2] ;
  }
  else{
    // ran out of lut values, so set to random colour
    a[0] = 0.5 * (double)std::rand() / (double)(RAND_MAX+1) ;
    a[1] = 0.5 * (double)std::rand() / (double)(RAND_MAX+1) ;
    a[2] = 0.5 * (double)std::rand() / (double)(RAND_MAX+1) ;

    // set max component to 1.0
    if (a[0] >= a[1] && a[0] >= a[2])
      a[0] = 1.0 ;
    else if (a[1] > a[0] && a[1] >= a[2])
      a[1] = 1.0 ;
    else
      a[2] = 1.0 ;
  }
}




//------------------------------------------------------------------------------
// Volume slice pipeline constructor
// This is based on mafPipeVolumeSlice::CreateSlice().
//
//               vtk_data------------------------
//               /       \                       |
//              /         \                      |
//  vtkMAFVolumeSlicer      vtkMAFVolumeSlicer       bounding box
//   slicer_image   ..>  slicer_polygonal
//         |       .           |
//         |      .            |
//       image ...          polydata
//         |   (texture)       |
//         |                 mapper
//         |                   |
//       texture ...........> actor
//
lhpMultiscaleVolumeSlicePipeline::lhpMultiscaleVolumeSlicePipeline(mafVME* vme,  int viewId, double *pos, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  int m_TextureRes = 512 ;
  double xspc = 0.33, yspc = 0.33, zspc = 1.0;

  this->SetType(MSCALE_SLICE_PIPE) ;

  // get volume data from vme (could be struct pts or rect grid)
  vtkDataSet *vtk_data = vme->GetOutput()->GetVTKData() ;
  vtk_data->Update() ;
  if(vtk_data->IsA("vtkImageData") || vtk_data->IsA("vtkStructuredPoints"))
  {
    ((vtkImageData *)vtk_data)->GetSpacing(xspc,yspc,zspc);
  }

  // get pose matrix from vme
  vtkMatrix4x4 *mat = vme->GetOutput()->GetAbsMatrix()->GetVTKMatrix() ;

  // save the view id
  m_viewId = viewId ;


  // set up pipeline to visualize bounding box
  m_ocf = vtkOutlineCornerFilter::New() ;
  m_ocf->SetInput(vtk_data) ;
  m_boxMapper = vtkPolyDataMapper::New() ;
  m_boxMapper->SetInput(m_ocf->GetOutput()) ;
  m_boxMapper->ScalarVisibilityOn() ;
  m_boxActor = vtkActor::New() ;
  m_boxActor->SetMapper(m_boxMapper) ;
  m_boxActor->SetUserMatrix(mat) ;
  renderer->AddActor(m_boxActor) ;

 
  // Set up look up table
  double range[2] ;
  vtkLookupTable *lut = vtkLookupTable::New() ;
  vtkPointData *PD = vtk_data->GetPointData() ;
  PD->GetArray(0)->GetRange(range) ;
  lut->SetTableRange(range[0], range[1]) ;
  lut->SetNumberOfTableValues(256) ;
  lut->SetNumberOfColors(256) ;
  for (int i = 0 ;  i < 255 ;  i++){
    double val = (double)i / 255.0 ;
    lut->SetTableValue(i, val, val, val) ;
  }
  lut->Build() ;



  // set up the image and polydata slicers
  m_SlicerPolygonal = vtkMAFVolumeSlicer::New() ;
  m_SlicerImage = vtkMAFVolumeSlicer::New() ;
  this->SetSlicePosition(pos) ;
  this->SetSliceDirection(viewId) ;
  m_SlicerImage->SetInput(vtk_data);
  m_SlicerPolygonal->SetInput(vtk_data);

 
  // set up image to be output of image slicer
  m_Image = vtkImageData::New() ;
  m_Image->SetScalarType(vtk_data->GetPointData()->GetScalars()->GetDataType());
  m_Image->SetNumberOfScalarComponents(vtk_data->GetPointData()->GetScalars()->GetNumberOfComponents());
  m_Image->SetExtent(0, m_TextureRes - 1, 0, m_TextureRes - 1, 0, 0);
  m_Image->SetSpacing(xspc, yspc, zspc);

  m_SlicerImage->SetOutput(m_Image);
  m_SlicerImage->Update();


  // set texture to receive image
  m_Texture = vtkTexture::New() ;
  m_Texture->RepeatOff();
  m_Texture->InterpolateOn();
  m_Texture->SetQualityTo32Bit();
  m_Texture->SetLookupTable(lut);
  m_Texture->MapColorScalarsThroughLookupTableOn();
  m_Texture->SetInput(m_Image);


  // Set up polydata slice and add texture
  m_SlicePolydata = vtkPolyData::New() ;
  m_SlicerPolygonal->SetOutput(m_SlicePolydata);
  m_SlicerPolygonal->SetTexture(m_Image);
  m_SlicerPolygonal->Update();


  // Set the mapper with the lut
  m_sliceMapper = vtkPolyDataMapper::New() ;
  m_sliceMapper->SetInput(m_SlicePolydata);
  lut->Delete() ;

  m_sliceActor = vtkActor::New() ;
  m_sliceActor->SetMapper(m_sliceMapper);
  m_sliceActor->SetTexture(m_Texture) ;
  m_sliceActor->SetUserMatrix(mat) ;
  renderer->AddActor(m_sliceActor) ;
}




//------------------------------------------------------------------------------
// Volume slice pipeline destructor
lhpMultiscaleVolumeSlicePipeline::~lhpMultiscaleVolumeSlicePipeline()
//------------------------------------------------------------------------------
{
  m_boxActor->Delete() ;
  m_boxMapper->Delete() ;
  m_ocf->Delete() ;
  m_sliceActor->Delete() ;
  m_sliceMapper->Delete() ;
  m_SlicerPolygonal->Delete() ;
  m_SlicePolydata->Delete() ;
  m_SlicerImage->Delete() ;
  m_Image->Delete() ;
  m_Texture->Delete() ;
}


//------------------------------------------------------------------------------
// Volume slice pipeline set visibility
void lhpMultiscaleVolumeSlicePipeline::SetVisibility(int visibility)
//------------------------------------------------------------------------------
{
  m_boxActor->SetVisibility(visibility) ;
  m_sliceActor->SetVisibility(visibility) ;
}


//------------------------------------------------------------------------------
// Slice pipeline - set slice direction
void lhpMultiscaleVolumeSlicePipeline::SetSliceDirection(int viewId)
//------------------------------------------------------------------------------
{
  // x and y axes of slice for each view direction
  float XVector[3][3] = {{0.0001,1,0}, {0,0,1}, {1,0,0}} ; // nb. 0.0001 is not zero because of vtk bug
  float YVector[3][3] = {{0,     0,1}, {1,0,0}, {0,1,0}} ;

  // save the new view id
  m_viewId = viewId ;

  int direction ;

  switch(viewId){
    case ID_XY:
      direction = 2 ;
      break ;
    case ID_XZ:
      direction = 1 ;
      break ;
    case ID_YZ:
      direction = 0 ;
      break ;
  }

  m_SlicerImage->SetPlaneAxisX(XVector[direction]);
  m_SlicerImage->SetPlaneAxisY(YVector[direction]);
  m_SlicerPolygonal->SetPlaneAxisX(XVector[direction]);
  m_SlicerPolygonal->SetPlaneAxisY(YVector[direction]);

}

//------------------------------------------------------------------------------
// Slice pipeline - set slice position
void lhpMultiscaleVolumeSlicePipeline::SetSlicePosition(double *pos)
//------------------------------------------------------------------------------
{
  // Tweak the position slightly so that slices are not exactly in the same plane
  // We move the slice away by a fraction of the total range,
  // so big slices move away further, leaving the small ones on top.
  double bounds[6], posn[3] ;
  this->GetActor()->GetBounds(bounds) ;

  posn[0] = pos[0] ;
  posn[1] = pos[1] ;
  posn[2] = pos[2] ;
  switch(m_viewId){
    case ID_XY:
      posn[2] -= 0.001*(bounds[5] - bounds[4]) ;
      if (posn[2] < bounds[4])
        posn[2] = bounds[4] ;
      break ;
    case ID_XZ:
      posn[1] -= 0.001*(bounds[3] - bounds[2]) ;
      if (posn[1] < bounds[2])
        posn[1] = bounds[2] ;
      break ;
    case ID_YZ:
      posn[0] -= 0.001*(bounds[1] - bounds[0]) ;
      if (posn[0] < bounds[0])
        posn[0] = bounds[0] ;
      break ;
  }

  // set the slice to the new position
  m_SlicerImage->SetPlaneOrigin(posn);
  m_SlicerPolygonal->SetPlaneOrigin(posn);
}


//------------------------------------------------------------------------------
// Slice pipeline print
void lhpMultiscaleVolumeSlicePipeline::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  os << indent ;

  os << "slice pipe: type = " ;
  switch(this->GetType()){
    case MSCALE_SURFACE_PIPE:
      os << "SURFACE " ;
      break ;
    case MSCALE_TOKEN_PIPE:
      os << "TOKEN" ;
      break ;
    case MSCALE_SLICE_PIPE:
      os << "SLICE" ;
      break ;
    default:
      os << "?????" ;
      break ;
  }
  os << "\t" ;

  os << "visibility: box = " << m_boxActor->GetVisibility() << " slice = " << m_sliceActor->GetVisibility() << std::endl ;

  double b[6] ;
  m_sliceActor->GetBounds(b) ;
  os << "slice bounds = " << b[0] << " " << b[1] << " " << b[2] << " " << b[3] << " " << b[4] << " " << b[5] << std::endl ;
}
