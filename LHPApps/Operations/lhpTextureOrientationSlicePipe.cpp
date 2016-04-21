/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationSlicePipe.cpp,v $
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
#include "vtkCamera.h"

#include "lhpTextureOrientationSlicePipe.h"

#include <cstdlib>
#include <ostream>


//------------------------------------------------------------------------------
// using namespace 
//------------------------------------------------------------------------------
using namespace lhpTextureOrientation ;



//------------------------------------------------------------------------------
// Volume slice pipeline constructor
// This is based on mafPipeVolumeSlice::CreateSlice().
//
//               vtk_data------------------------------
//               /       \                             |
//              /         \                            |
//  vtkMAFVolumeSlicer      vtkMAFVolumeSlicer     bounding box
//   slicer_image   ..>  slicer_polygonal
//         |       .           |
//         |      .            |
//       image ...          polydata
//         |   (texture)       |
//         |                 mapper
//         |                   |
//       texture ...........> actor
//
lhpTextureOrientationSlicePipe::lhpTextureOrientationSlicePipe(mafVME* vme,  int viewId, double *pos, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  int m_TextureRes = 512 ;
  double xspc = 0.33, yspc = 0.33, zspc = 1.0;

  // get volume data from vme (could be struct pts or rect grid)
  vtkDataSet *vtk_data = vme->GetOutput()->GetVTKData() ;
  vtk_data->Update() ;
  if(vtk_data->IsA("vtkImageData") || vtk_data->IsA("vtkStructuredPoints"))
  {
    ((vtkImageData *)vtk_data)->GetSpacing(xspc,yspc,zspc);
  }

  // get pose matrix from vme
  vtkMatrix4x4 *mat = vme->GetOutput()->GetMatrix()->GetVTKMatrix() ;

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



  //----------------------------------------------------------------------------
  // Add actors to renderer
  //----------------------------------------------------------------------------
  renderer->AddActor(m_boxActor) ;
  renderer->AddActor(m_sliceActor) ;


  //----------------------------------------------------------------------------
  // Aim the camera at the centre of the image
  //----------------------------------------------------------------------------
  vtkCamera *camera = renderer->GetActiveCamera() ;
  camera->SetFocalPoint(vtk_data->GetCenter()) ;
}




//------------------------------------------------------------------------------
// Volume slice pipeline destructor
lhpTextureOrientationSlicePipe::~lhpTextureOrientationSlicePipe()
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
void lhpTextureOrientationSlicePipe::SetVisibility(int visibility)
//------------------------------------------------------------------------------
{
  m_boxActor->SetVisibility(visibility) ;
  m_sliceActor->SetVisibility(visibility) ;
}


//------------------------------------------------------------------------------
// Slice pipeline - set slice direction
void lhpTextureOrientationSlicePipe::SetSliceDirection(int viewId)
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
void lhpTextureOrientationSlicePipe::SetSlicePosition(double *pos)
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
void lhpTextureOrientationSlicePipe::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  os << indent ;

  os << "visibility: box = " << m_boxActor->GetVisibility() << " slice = " << m_sliceActor->GetVisibility() << std::endl ;

  double b[6] ;
  m_sliceActor->GetBounds(b) ;
  os << "slice bounds = " << b[0] << " " << b[1] << " " << b[2] << " " << b[3] << " " << b[4] << " " << b[5] << std::endl ;
}
