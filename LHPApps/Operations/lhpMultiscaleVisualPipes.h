/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleVisualPipes.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleVisualPipes_H__
#define __lhpMultiscaleVisualPipes_H__

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


namespace lhpMultiscale{
  //----------------------------------------------------------------------------
  ///< Types of multiscale actor
  //----------------------------------------------------------------------------
  enum PipeType {
    MSCALE_SURFACE_PIPE = 0,
    MSCALE_TOKEN_PIPE,
    MSCALE_SLICE_PIPE
  } ;

  //----------------------------------------------------------------------------
  ///< Index of slice direction
  //----------------------------------------------------------------------------
  enum ViewId
  {
    ID_XY = 0,
    ID_XZ,
    ID_YZ
  } ;
}



/*******************************************************************************
lhpMultiscalePipeline: abstract base class for vtk visual pipeline.
Constructs vtk objects (actor, mapper) for this pipeline and connects vme to renderer.
*******************************************************************************/
class lhpMultiscalePipeline
{
public:
  /** Return the actor which defines the size, position etc of the pipe's actor or actors 
  Do not use this method to set the visibility, in case there is more than one actor in the pipe */
  virtual vtkActor* GetActor() = 0 ;

  /** get and set type of pipe */
  virtual void SetType(lhpMultiscale::PipeType pipeType) {m_pipeType = pipeType ;}
  virtual lhpMultiscale::PipeType GetType() {return m_pipeType ;}

  /** Get and set the visibility of the pipe */
  virtual int GetVisibility() = 0 ;
  virtual void SetVisibility(int visibility) = 0 ;

  /** Print self */
  virtual void PrintSelf(std::ostream& os, vtkIndent indent) = 0 ;

private:
  lhpMultiscale::PipeType m_pipeType ;
};




/*******************************************************************************
lhpMultiscaleSurfacePipeline: vtk visual pipeline for surface data
Constructs vtk objects (actor, mapper) for this pipeline and connects vme to renderer.
*******************************************************************************/
class lhpMultiscaleSurfacePipeline : public lhpMultiscalePipeline
{
public:
  lhpMultiscaleSurfacePipeline(mafVME* vme, vtkRenderer *renderer) ;
  ~lhpMultiscaleSurfacePipeline() ;
  vtkActor* GetActor() {return m_actor ;}
  int GetVisibility() {return m_actor->GetVisibility() ;}
  void SetVisibility(int visibility) {m_actor->SetVisibility(visibility);}
  void PrintSelf(std::ostream& os, vtkIndent indent) ;
private:
  vtkActor *m_actor ;
  vtkPolyDataMapper *m_mapper ;
 
};



/*******************************************************************************
lhpMultiscaleTokenPipeline: vtk visual pipeline for multiscale token
Constructs vtk objects (source, actor, mapper etc) and plugs pipe into renderer.
Note that the token pipeline has to be given a color.
*******************************************************************************/
class lhpMultiscaleTokenPipeline : public lhpMultiscalePipeline
{
public:
  lhpMultiscaleTokenPipeline(vtkRenderer *renderer, int colorId) ;
  ~lhpMultiscaleTokenPipeline() ;
  vtkActor* GetActor() {return m_actor ;}
  int GetVisibility() {return m_actor->GetVisibility() ;}
  void SetVisibility(int visibility) {m_actor->SetVisibility(visibility);}
  void PrintSelf(std::ostream& os, vtkIndent indent) ;
private:
  virtual void CalculateColor(double *a) ;
  int m_colorId ;
  vtkCubeSource* m_tokenSource ;
  vtkActor* m_actor ;
  vtkPolyDataMapper* m_mapper ;
};



/*******************************************************************************
lhpMultiscaleSurfacePipeline: vtk visual pipeline for slicing volume data
Constructs vtk objects (actor, mapper) for this pipeline and connects vme to renderer.
*******************************************************************************/
class lhpMultiscaleVolumeSlicePipeline : public lhpMultiscalePipeline
{
public:
  lhpMultiscaleVolumeSlicePipeline(mafVME* vme, int viewId, double *pos, vtkRenderer *renderer) ;
  ~lhpMultiscaleVolumeSlicePipeline() ;
  vtkActor* GetActor() {return m_boxActor ;}
  int GetVisibility() {return m_boxActor->GetVisibility() ;}  ///< get visibility of pipeline  
  void SetVisibility(int visibility) ;                        ///< set visibility of all actors in pipeline
  void SetSliceDirection(int viewId) ;                        ///< set view direction
  void SetSlicePosition(double *pos) ;                        ///< set position of slice
  void PrintSelf(std::ostream& os, vtkIndent indent) ;
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