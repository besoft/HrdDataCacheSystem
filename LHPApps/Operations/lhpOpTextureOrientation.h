/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpTextureOrientation.h,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.1.1.2.3 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpTextureOrientation_H__
#define __lhpOpTextureOrientation_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "mafEventBase.h"
#include "mafNode.h"
#include "mafGUIDialog.h"
#include "mafGUIFloatSlider.h"
#include "mafRWI.h"

#include "vtkPolyData.h"

#include "lhpHistogramEqualizationFilter.h"
#include "lhpTextureOrientationFilter.h"
#include "lhpTextureOrientationSlicePipe.h"
#include "lhpTextureOrientationVectorGlyphPipe.h"
#include "lhpTextureOrientationTensorGlyphPipe.h"
#include "lhpTextureOrientationCallbacks.h"


//------------------------------------------------------------------------------
// TODO
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
/// Operation which finds the texture direction. \n
/// The input is an image VME and the output is a surface VME. \n
/// The surface is a set of polydata points with the direction vector or tensor as attributes. \n
/// There is also an option of text output in spreadsheet-friendly csv format. \n
/// \n
/// There is also a scalar attribute in the range [0-1] which is mapped by vtk to the range [red - blue] \n
/// The scalar is the ratio of the principal eigenvalue to the total eigenvalues, \n
/// and indicates how strongly directional the tensor is.
/// \n
/// The user can select a vector or tensor view. \n
/// The vector view shows ellipsoids oriented along the principal direction. \n
/// The tensor view shows ellipsoids oriented by the eigenvectors and scaled by eigenvalues. \n
/// \n
/// Texture is not measured at every voxel, but at sparse sample points on a grid. \n
/// The user sets the sample spacing in scene units. \n
/// \n
/// Texture is a statistical property of a volume - not a single voxel. \n
/// Hence each measurement is of a small cuboidal volume about the nominal sampling point \n
/// which is here called a "probe" (for want of a better word). \n
/// The user sets the probe size in scene units. \n
//
// 
// The operation creates two similar visual pipes: one for vector display and the other for tensor display.
// Both visual pipes use lhpTextureOrientationFilter to calculate the output polydata and its attributes.
//------------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT lhpOpTextureOrientation : public mafOp
{
public:
  lhpOpTextureOrientation(wxString label = "Texture Orientation");
  ~lhpOpTextureOrientation(); 

  mafTypeMacro(lhpOpTextureOrientation, mafOp);

  mafOp* Copy();

  void OnEvent(mafEventBase *maf_event);

  /// Return true for the acceptable vme type.
  bool Accept(mafNode* vme);

  /// Builds operation's interface by calling CreateOpDialog() method.
  void OpRun();

  /// Execute the operation.
  void OpDo();

  /// Makes the undo for the operation.
  void OpUndo();

protected:
  //----------------------------------------------------------------------------
  // methods for operation's workflow
  //----------------------------------------------------------------------------

  /// Builds operation's interface and visualization pipeline.
  void CreateOpDialog();

  /// Remove operation's interface.
  void DeleteOpDialog();

  /// Create the vtk pipe
  void CreateVisualPipe() ;

  /// Create the output vme and reparent
  void CreateOutputVME() ;

  /// Update the camera
  void UpdateCamera() ;

  /// Get renderer
  vtkRenderer* GetRenderer() ;


  //----------------------------------------------------------------------------
  // event handlers
  //----------------------------------------------------------------------------
  void OnUpdate() ;
  void OnPrint() ;



  //----------------------------------------------------------------------------
  // methods for controlling the slice
  //----------------------------------------------------------------------------

  /// Initialize the parameters of the slice (origin and view index). \n
  /// This only sets the parameters - it does not set or change the visual pipes !
  void InitSliceParams(int viewIndex, double *bounds) ;

  /// Move the global slice
  void UpdateSlicePosition() ;

  /// Update the view direction. \n
  /// This also recalculates the slice position
  void UpdateViewAxis(double *bounds) ;

  /// Set the slider range to fit the given bounds
  void SetSliderRange(double *bounds) ;



  //----------------------------------------------------------------------------
  // member variables
  //----------------------------------------------------------------------------
  mafGUIDialog		*m_Dialog;        // dialog and interactor
  mafRWI          *m_Rwi;

  double m_SliceOrigin[3] ;           // position of slice
  int m_ViewIndex ;                   // view direction validator (don't assume that 0,1,2 = x,y,z !!)
  int m_ViewIndex_old ;
  double m_SliderOrigin ;             // position slider validator
  double m_SliderOrigin_old ;
  double m_ProbeSizeSceneUnits ;
  double m_SampleSpacingSceneUnits ;
  int m_outputFormat ;
  wxString m_outputFilename ;         // filename for output

  // These widgets are destroyed with the dialog
  // don't try to delete in deconstructor
  mafGUIFloatSlider *m_PosSlider ;
  mafGUIButton *m_printButton ;
  mafGUIButton *m_okButton ;
  wxGauge *m_progressGauge ;


  lhpTextureOrientationProgressCallback *m_progressCallback ;

  vtkRenderer     *m_externalRenderer ;

  // flags indicating whether pipes are updated with current inputs
  bool m_vectorPipeUpdated ;
  bool m_tensorPipeUpdated ;

  lhpTextureOrientationSlicePipe *m_slicePipe ;             // visual pipe for slice
  lhpTextureOrientationVectorGlyphPipe *m_vectorGlyphPipe ; // visual pipe for vector glyphs
  lhpTextureOrientationTensorGlyphPipe *m_tensorGlyphPipe ; // visual pipe for tensor glyphs
  vtkPolyData *m_polydata ;       // output polydata
  mafVMESurface *m_polydataVME ;  // output polydata vme
};


#endif