/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationFilter.h,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.1.1.2.2 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpTextureOrientationFilter_H__
#define __lhpTextureOrientationFilter_H__

#include "vtkStructuredPointsToPolyDataFilter.h"
#include "vtkPolyData.h"
#include "vtkCommand.h"

#include <ostream>


namespace lhpTextureOrientation
{
  //----------------------------------------------------------------------------
  /// output format of filter
  //----------------------------------------------------------------------------
  enum OutputFormat{
    VectorFormat = 0,
    TensorFormat
  };

  //----------------------------------------------------------------------------
  /// this defines the id of the vtk user callback 
  //----------------------------------------------------------------------------
  enum UserEvent{
    ProgressEventId = vtkCommand::UserEvent+0
  };
}


//------------------------------------------------------------------------------
/// Helper class for lhpOpTextureOrientation. \n
/// Texture orientation filter. \n
/// \n
/// Texture is not measured at every voxel, but at sparse sample points on a grid. \n
/// The user sets the sample spacing either in voxels or in scene units. \n
/// \n
/// Texture is a statistical property of a volume - not a single voxel. \n
/// Hence each measurement is of a small cuboidal volume about the nominal sampling point \n
/// which is here called a "probe" (for want of a better word). \n
/// The user sets the probe size either in voxels or in scene units. \n
/// \n
/// The filter returns a set of polydata points in the grid of sample positions across the image. \n
/// The attributes of the polydata are tensors containing the texture direction. \n
/// The user can choose the output attributes to be tensor or vector only. \n
/// There is also a scalar attribute in the range [0-1] which is mapped by vtk to the range [red - blue] \n
/// The scalar is the ratio of the principal eigenvalue to the total eigenvalues, \n
/// and indicates how strongly directional the tensor is.
//------------------------------------------------------------------------------
class lhpTextureOrientationFilter : public vtkStructuredPointsToPolyDataFilter
{
public:
  static lhpTextureOrientationFilter *New();
  vtkTypeRevisionMacro(lhpTextureOrientationFilter, vtkStructuredPointsToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent); ///< print self (CSV format)

  /// print results (CSV format)
  void PrintResults(ostream& os);

  /// set/get which scalar component to use (0, 1 or 2)
  void SetTargetComponent(int) ;
  int GetTargetComponent() const {return m_target_component ;}

  /// set output to vector format (this is the default). \n
  /// output attributes are normals containing principal direction and scalars containing magnitude.
  void SetOutputToVectorFormat() {m_outputFormat = lhpTextureOrientation::VectorFormat ;  this->Modified() ;}

  /// set output to tensor format. \n
  /// output attributes are tensors containing all direction information.
  void SetOutputToTensorFormat() {m_outputFormat = lhpTextureOrientation::TensorFormat ;  this->Modified() ;}

  /// set output format to vector or tensor
  void SetOutputFormat(lhpTextureOrientation::OutputFormat outputFormat) {m_outputFormat = outputFormat ;  this->Modified() ;}

  /// Set the dimensions of the probe in voxels
  void SetProbeSizeVoxels(int dimsx, int dimsy, int dimsz) ;

  /// Set the dimensions of the probe in voxels
  void SetProbeSizeVoxels(const int dims[3]) ;

  /// Set the size of the probe in world coords
  void SetProbeSizeSceneUnits(double size) ;

  /// Set the step increment of the probe in voxels
  void SetSampleSpacingVoxels(int stepx, int stepy, int stepz) ;

  /// Set the step increment of the probe in voxels
  void SetSampleSpacingVoxels(const int step[3]) ;

  /// Set the step increment of the probe in world coords
  void SetSampleSpacingSceneUnits(double step) ;

  /// Execute Method
  void Execute() ;

protected:
  /// constructor and destructor
  lhpTextureOrientationFilter();
  ~lhpTextureOrientationFilter();

  /// refresh the filter
  void Initialize() ;

  /// run the filter
  void FilterExecute() ;


  /// format of output
  lhpTextureOrientation::OutputFormat m_outputFormat ;

  int m_target_component ; // input image component to use

  // probe size and spacing
  bool m_ProbeSizeVoxelsSet ;
  bool m_ProbeSizeSceneUnitsSet ;
  bool m_SampleSpacingVoxelsSet ;
  bool m_SampleSpacingSceneUnitsSet ;

  int m_ProbeSizeVoxels[3] ;         // size of probe in voxels
  double m_ProbeSizeSceneUnits ;     // size of probe in scene units
  int m_SampleSpacingVoxels[3] ;      // spacing of probes in voxels
  double m_SampleSpacingSceneUnits ;  // spacing of probes in scene units

  int m_numx, m_numy, m_numz ;       // number of probes in x, y and z

  /// progress tracker
  double m_progress ;

  vtkImageData *m_input ;
  vtkPolyData *m_output ;

} ;

#endif