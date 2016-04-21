/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationSlicePipe.h,v $
Language:  C++
Date:      $Date: 2009-06-26 14:03:04 $
Version:   $Revision: 1.1.1.1.2.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpTextureOrientationSlicePipe_H__
#define __lhpTextureOrientationSlicePipe_H__

#include "mafVME.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkCubeSource.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkMAFVolumeSlicer.h"  // bug: must include vtkImageData and vtkPolyData first
#include "vtkTexture.h"
#include <ostream>


namespace lhpTextureOrientation{
  //----------------------------------------------------------------------------
  /// Enum: Index of slice direction
  //----------------------------------------------------------------------------
  enum ViewId
  {
    ID_XY = 0,
    ID_XZ,
    ID_YZ
  } ;
}





//------------------------------------------------------------------------------
/// Helper class for lhpOpTextureOrientation. \n
/// vtk visual pipeline for slicing volume data. \n
/// Constructs vtk objects (actor, mapper) for this pipeline and connects vme to renderer.
//------------------------------------------------------------------------------
class lhpTextureOrientationSlicePipe
{
public:
  lhpTextureOrientationSlicePipe(mafVME* vme, int viewId, double *pos, vtkRenderer *renderer) ;
  ~lhpTextureOrientationSlicePipe() ;
  vtkActor* GetActor() {return m_boxActor ;}                  ///< return bounding box actor
  int GetVisibility() {return m_boxActor->GetVisibility() ;}  ///< get visibility of pipeline  
  void SetVisibility(int visibility) ;                        ///< set visibility of all actors in pipeline
  void SetSliceDirection(int viewId) ;                        ///< set view direction
  void SetSlicePosition(double *pos) ;                        ///< set position of slice
  void PrintSelf(std::ostream& os, vtkIndent indent) ; ///< print self
private:
  int m_viewId ;
  vtkActor *m_boxActor ;
  vtkPolyDataMapper *m_boxMapper ;
  vtkOutlineCornerFilter *m_ocf ;
  vtkActor *m_sliceActor ;
  vtkPolyDataMapper *m_sliceMapper ;
  vtkMAFVolumeSlicer	*m_SlicerPolygonal ;
  vtkPolyData *m_SlicePolydata ;
  vtkMAFVolumeSlicer	*m_SlicerImage ;
  vtkImageData *m_Image ;
  vtkTexture *m_Texture ;  
};

#endif