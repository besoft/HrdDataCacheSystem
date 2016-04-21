#ifndef __lhpTextureOrientationCalculator_H__
#define __lhpTextureOrientationCalculator_H__

#include "lhpIsotropicSampleTable.h"
#include "lhpCooccurrenceMatrixFilter.h"

#include "vtkImageData.h"

#include <ostream>


//------------------------------------------------------------------------------
/// Helper class for lhpOpTextureOrientation. \n
/// Class which calculates the orientation of the image texture using cooccurrence matrices. \n
/// The input image must be histogram equalized with a small number of grey levels. \n
/// Note that this is not a vtk filter - the user must call Execute() to run it.
//------------------------------------------------------------------------------
class lhpTextureOrientationCalculator
{
public:
  /// constructor
  lhpTextureOrientationCalculator() ;

  /// deconstructor
  ~lhpTextureOrientationCalculator() ;

  /// set the input image 
  void SetInputImage(vtkImageData *image) {m_inputImage = image ;}

  /// Set the texture statistic to use
  void SetTextureStatistic(CooCalculationId statistic) {m_textureStatistic = statistic ;}

  /// Set the number of sample directions
  void SetResolutionOfSamples(int numberOfTheta, int numberOfPhi, int numberOfR) 
  {m_numberOfTheta = numberOfTheta; m_numberOfPhiMax = numberOfPhi ;  m_numberOfR = numberOfR ;}

  /// Set the volume of interest dimensions
  void SetVoiDimensions(int dimsx, int dimsy, int dimsz) ;

  /// Set the volume of interest dimensions
  void SetVoiDimensions(const int dims[3]) ;

  /// Set the volume of interest size in world coords
  void SetVoiSize(double size) ;

  /// Set the position (start corner) of the volume of interest
  void SetVoiPosition(int x, int y, int z) ;

  /// Set the position (start corner) of the volume of interest
  void SetVoiPosition(const int pos[3]) ;

  /// Run the calculator
  void Execute() ;

  /// Get the covariance matrix in 1D column-major form
  void GetCovarianceMatrix(double covarMat[9]) const ;

  /// Get the eigenvector matrix in 1D column-major form
  void GetEigenvectorMatrix(double eigenvectorMat[9]) const ;

  /// Get the eigenvector matrix, with columns scaled by eigenvalues, in 1D column-major form
  void GetEigenvectorMatrixScaled(double eigenvectorMat[9]) const ;

  /// Get the eigenvalues
  void GetEigenvalues(double eigenvalues[3]) const ;

  /// Get the ith eigenvector
  void GetEigenvector(double eigenvector[3], int i) const ;
  
  /// Get the ith eigenvector, scaled by eigenvalue
  void GetEigenvectorScaled(double eigenvector[3], int i) const ;

  /// Print self
  void PrintSelf(std::ostream& os, int indent) const ;


private:
  vtkImageData *m_inputImage ;
  int m_numberOfGreyLevels ;
  CooCalculationId m_textureStatistic ; ///< which texture statistic to use

  // volume of interest
  bool m_voiDimsSet ;
  bool m_voiSizeSet ;
  bool m_voiPosSet ;
  int m_voiDimensions[3] ;
  double m_voiSize ;
  int m_voiPos[3] ;

 
  int m_numberOfTheta ;
  int m_numberOfPhiMax ;
  int m_numberOfR ;
  double m_rSpacing ;

  double m_covariance[3][3] ;
  double m_lambda[3] ;
  double m_V[3][3] ;

} ;

#endif
