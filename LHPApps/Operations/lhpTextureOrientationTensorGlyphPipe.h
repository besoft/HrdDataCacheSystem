/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationTensorGlyphPipe.h,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.2.3 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpTextureOrientationTensorGlyphPipe_H__
#define __lhpTextureOrientationTensorGlyphPipe_H__

#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTensorGlyph.h"
#include "vtkPolyDataNormals.h"

#include "mafVME.h"
#include "lhpTextureOrientationTensorGlyphPipe.h"
#include "lhpHistogramEqualizationFilter.h"
#include "lhpTextureOrientationFilter.h"
#include "lhpTextureOrientationCallbacks.h"

#include <ostream>



//------------------------------------------------------------------------------
/// Helper class for lhpOpTextureOrientation. \n
/// Visual pipeline which finds and displays texture orientation. \n
/// Constructs vtk objects (actor, mapper) for this pipeline and connects vme to renderer. \n
/// GetPolydata() method returns pointer to polydata which contains the orientation results.
//------------------------------------------------------------------------------
class lhpTextureOrientationTensorGlyphPipe
{
public:
  lhpTextureOrientationTensorGlyphPipe(mafVME* vme, vtkRenderer *renderer) ;
  ~lhpTextureOrientationTensorGlyphPipe() ;
  vtkActor* GetActor() {return m_glyphActor ;}
  int GetVisibility() {return m_glyphActor->GetVisibility() ;}          ///< get visibility of pipeline  
  void SetVisibility(int visibility) ;                                  ///< set visibility of all actors in pipeline

  void SetProbeSizeSceneUnits(double probeSize) {m_texFilter->SetProbeSizeSceneUnits(probeSize) ;} ///< set size of probe
  void SetSampleSpacingSceneUnits(double sampleSpacing) {m_texFilter->SetSampleSpacingSceneUnits(sampleSpacing) ;} ///< set step size of probe

  /// set the output format
  void SetOutputFormat(lhpTextureOrientation::OutputFormat outputFormat) {m_texFilter->SetOutputFormat(outputFormat) ;}

  /// add observer to texture filter to catch and rethrow user progress event. \n
  /// NB The calling program is still responsible for deleting its reference count to the callback
  void AddProgressObserver(int vtkEventID, lhpTextureOrientationProgressCallback *callback) ;

  vtkPolyData* GetPolydata() ;                                          ///< return polydata containing results
  void PrintSelf(std::ostream& os, vtkIndent indent) ;                  ///< print self
  void PrintResults(std::ostream& os) {m_texFilter->PrintResults(os) ;} ///< print orientation results
private:
  lhpHistogramEqualizationFilter *m_histEq ;
  lhpTextureOrientationFilter *m_texFilter ;
  vtkSphereSource *m_ellipsoid ;
  vtkTransform *m_transform ;
  vtkTransformPolyDataFilter *m_transformPD ;
  vtkTensorGlyph *m_tensorGlyph ;
  vtkPolyDataNormals *m_polyNormals ;
  vtkPolyDataMapper *m_glyphMapper ;
  vtkActor *m_glyphActor ;
};

#endif