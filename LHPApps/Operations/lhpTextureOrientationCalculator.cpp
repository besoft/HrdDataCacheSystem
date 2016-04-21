/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationCalculator.cpp,v $
Language:  C++
Date:      $Date: 2009-10-08 13:12:17 $
Version:   $Revision: 1.1.1.1.2.1 $
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


#include "lhpCooccurrenceMatrixFilter.h"
#include "lhpTextureOrientationUseful.h"
#include "lhpTextureOrientationUseful.h"
#include "lhpTextureOrientationCalculator.h"
#include "lhpIsotropicSampleTable.h"
#include "medOpMatrixVectorMath.h"

#include "vtkStructuredPoints.h"

#include <ostream>
#include <assert.h>

#ifndef M_PI
  #define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <algorithm>


//------------------------------------------------------------------------------
// Constructor
lhpTextureOrientationCalculator::lhpTextureOrientationCalculator() : m_inputImage(NULL),
m_textureStatistic(CooCorrelation), m_numberOfGreyLevels(0),
m_numberOfTheta(7), m_numberOfPhiMax(12), m_numberOfR(15),
m_voiDimsSet(false), m_voiSizeSet(false), m_voiPosSet(false)
//------------------------------------------------------------------------------
{
}


//------------------------------------------------------------------------------
// Deconstructor
lhpTextureOrientationCalculator::~lhpTextureOrientationCalculator()
//------------------------------------------------------------------------------
{
}




//----------------------------------------------------------------------------
// Set the volume of interest dimensions
void lhpTextureOrientationCalculator::SetVoiDimensions(int dimsx, int dimsy, int dimsz) 
//----------------------------------------------------------------------------
{
  m_voiDimensions[0] = dimsx ;
  m_voiDimensions[1] = dimsy ;
  m_voiDimensions[2] = dimsz ;

  m_voiDimsSet = true ;
  m_voiSizeSet = false ;
}


//----------------------------------------------------------------------------
// Set the volume of interest dimensions
void lhpTextureOrientationCalculator::SetVoiDimensions(const int dims[3]) 
//----------------------------------------------------------------------------
{
  SetVoiDimensions(dims[0], dims[1], dims[2]) ;
}



//----------------------------------------------------------------------------
// Set the volume of interest size
void lhpTextureOrientationCalculator::SetVoiSize(double size) 
//----------------------------------------------------------------------------
{
  m_voiSize = size ;

  m_voiSizeSet = true ;
  m_voiDimsSet = false ;
}


//----------------------------------------------------------------------------
// Set the position (start corner) of the volume of interest
void lhpTextureOrientationCalculator::SetVoiPosition(int x, int y, int z)
//----------------------------------------------------------------------------
{
  m_voiPos[0] = x ;
  m_voiPos[1] = y ;
  m_voiPos[2] = z ;

  m_voiPosSet = true ;
}


//----------------------------------------------------------------------------
// Set the position (start corner) of the volume of interest
void lhpTextureOrientationCalculator::SetVoiPosition(const int pos[3])
//----------------------------------------------------------------------------
{
  SetVoiPosition(pos[0], pos[1], pos[2]) ;
}




//------------------------------------------------------------------------------
// Process the image
void lhpTextureOrientationCalculator::Execute()
//------------------------------------------------------------------------------
{
  //----------------------------------------------------------------------------
  // Update the input image
  //----------------------------------------------------------------------------
  assert(m_inputImage != NULL) ;
  m_inputImage->Update() ;

  // Get dims, bounds and spacing
  int dims[3] ;
  double bounds[6], spacing[3] ;
  m_inputImage->GetDimensions(dims) ;
  m_inputImage->GetBounds(bounds) ;
  m_inputImage->GetSpacing(spacing) ;

  // Calculate no. of grey levels
  // Note that this assumes that the input image is not float type, and the range starts at zero.
  double range[2] ;
  m_inputImage->GetScalarRange(range) ;
  m_numberOfGreyLevels = range[1] + 1 ;





  // ---------------------------------------------------------------------------
  // Set up the cooccurrence matrix and the table of sample points 
  // ---------------------------------------------------------------------------

  lhpCooccurrenceMatrixFilter *coMat = lhpCooccurrenceMatrixFilter::New() ;
  coMat->SetInput(m_inputImage) ;
  coMat->SetOutputDimensions(m_numberOfGreyLevels) ;
  coMat->SetCalculationRequest(CooCorrelation, true) ;
  coMat->SetVoiToSoftBoundary() ;
  coMat->SetGreyLevelSubsetRange(range[0], range[1]) ;  // in case range starts above zero

  if (!this->m_voiDimsSet){
    if (this->m_voiSizeSet){
      int d0 = lhpTextureOrientationUseful::Round(this->m_voiSize / spacing[0]) ;
      int d1 = lhpTextureOrientationUseful::Round(this->m_voiSize / spacing[1]) ;
      int d2 = lhpTextureOrientationUseful::Round(this->m_voiSize / spacing[2]) ;
      this->SetVoiDimensions(d0,d1,d2) ;
    }
    else
      this->SetVoiDimensions(dims) ;
  }
  coMat->SetVoiDimensions(this->m_voiDimensions) ;


  if (!this->m_voiPosSet)
    this->SetVoiPosition(0,0,0) ;
  coMat->SetVoiPosition(this->m_voiPos) ;


  m_rSpacing = std::min(spacing[0], spacing[1]) ;
  m_rSpacing = std::min(spacing[2], m_rSpacing) ;
  lhpIsotropicSampleTable *table = new lhpIsotropicSampleTable(m_numberOfTheta, m_numberOfPhiMax, m_numberOfR, m_rSpacing) ;



  //----------------------------------------------------------------------------
  // First pass to calculate cooccurrence matrices and determine threshold value
  //----------------------------------------------------------------------------
  int i, j, k ;
  int n = 0 ;
  double meanK0 = 0.0 ;     // mean texture result at k = 0
  double meanKMax = 0.0 ;   // mean texture result at k = nr-1

  for (i = 0, n = 0 ;  i < table->GetNumberOfTheta() ;  i++){
    for (j = 0 ;  j < table->GetNumberOfPhi(i) ;  j++){
      for (k = 0 ;  k < table->GetNumberOfR() ;  k++){
        double xworld = table->GetX(i,j,k) ;
        double yworld = table->GetY(i,j,k) ;
        double zworld = table->GetZ(i,j,k) ;

        // calculate displacement in voxel indices
        int dx = lhpTextureOrientationUseful::Round(xworld / spacing[0]) ;
        int dy = lhpTextureOrientationUseful::Round(yworld / spacing[1]) ;
        int dz = lhpTextureOrientationUseful::Round(zworld / spacing[2]) ;

        // get the texture matrix and statistic
        coMat->SetDisplacement(dx,dy,dz) ;
        coMat->Update() ;
        double tex = coMat->GetCalculationResult(CooCorrelation) ;
        table->SetSampleValue(i,j,k,tex) ;
      }

      // increment means
      meanK0 += table->GetSampleValue(i, j, 0) ;
      meanKMax += table->GetSampleValue(i, j, m_numberOfR-1) ;
      n++ ;
    }
  }
  meanK0 /= (double)n ;
  meanKMax /= (double)n ;


  //----------------------------------------------------------------------------
  // Second pass with threshold to calculate covariance matrix
  //----------------------------------------------------------------------------

  double correlThold = 0.5*meanK0 + 0.5*meanKMax ;  // set thold half way between extreme values
  bool found ;

  // initialize covariance matrix to zero
  for (i = 0 ;  i < 3 ;  i++)
    for (j = 0 ;  j < 3 ;  j++)
      m_covariance[i][j] = 0.0 ;

  for (i = 0, n = 0 ;  i < table->GetNumberOfTheta() ;  i++){
    for (j = 0 ;  j < table->GetNumberOfPhi(i) ;  j++){
     for (k = 0, found = false ;  k < table->GetNumberOfR() ;  k++){
       double tex = table->GetSampleValue(i,j,k) ;
        if (((meanK0 > meanKMax) && (tex <= correlThold)) || ((meanK0 < meanKMax) && (tex >= correlThold))){
          double xworld = table->GetX(i,j,k) ;
          double yworld = table->GetY(i,j,k) ;
          double zworld = table->GetZ(i,j,k) ;

          // increment covariance matrix
          m_covariance[0][0] += xworld*xworld ;
          m_covariance[0][1] += xworld*yworld ;
          m_covariance[0][2] += xworld*zworld ;
          m_covariance[1][0] += yworld*xworld ;
          m_covariance[1][1] += yworld*yworld ;
          m_covariance[1][2] += yworld*zworld ;
          m_covariance[2][0] += zworld*xworld ;
          m_covariance[2][1] += zworld*yworld ;
          m_covariance[2][2] += zworld*zworld ;
          n++ ;

          found = true ;
          break ;
        }
      }
    }
  }

  // normalise the covariance matrix
  for (i = 0 ;  i < 3 ;  i++)
    for (j = 0 ;  j < 3 ;  j++)
      m_covariance[i][j] /= (double)n ;

  // TODO: check for zero or uniform matrix before calculating eigenvectors


  // calculate the eigenvalues and eigenvectors
  lhpTextureOrientationUseful::EigenVectors3x3(m_covariance, m_lambda, m_V) ;



  // ---------------------------------------------------------------------------
  // Delete allocated memory
  // ---------------------------------------------------------------------------
  coMat->Delete() ;
  delete table ;
}






//------------------------------------------------------------------------------
// Get the covariance matrix in 1D column-major form
void lhpTextureOrientationCalculator::GetCovarianceMatrix(double covarMat[9]) const
//------------------------------------------------------------------------------
{
  medOpMatrixVectorMath matmath;
  matmath.Copy2DArrayToMatrix3x3(m_covariance, covarMat) ;
}


//------------------------------------------------------------------------------
// Get the eigenvector matrix in 1D column-major form
void lhpTextureOrientationCalculator::GetEigenvectorMatrix(double eigenvectorMat[9]) const
//------------------------------------------------------------------------------
{
  medOpMatrixVectorMath matmath ;
  matmath.Copy2DArrayToMatrix3x3(m_V, eigenvectorMat) ;
}


//------------------------------------------------------------------------------
// Get the eigenvector matrix, with columns scaled by eigenvalues, in 1D column-major form
void lhpTextureOrientationCalculator::GetEigenvectorMatrixScaled(double eigenvectorMat[9]) const
//------------------------------------------------------------------------------
{
  medOpMatrixVectorMath matmath ;
  matmath.Copy2DArrayToMatrix3x3(m_V, eigenvectorMat) ;

  // convert eigenvalues from variances to sd's and scale the matrix colums
  double sd[3] ;
  sd[0] = sqrt(m_lambda[0]) ;
  sd[1] = sqrt(m_lambda[1]) ;
  sd[2] = sqrt(m_lambda[2]) ;
  matmath.MultiplyColumnsByScalars(sd, eigenvectorMat, eigenvectorMat) ;
}


//------------------------------------------------------------------------------
// Get the eigenvalues
void lhpTextureOrientationCalculator::GetEigenvalues(double eigenvalues[3]) const
//------------------------------------------------------------------------------
{
  medOpMatrixVectorMath matmath ;
  matmath.CopyVector(m_lambda, eigenvalues) ;
}


//------------------------------------------------------------------------------
// Get the ith eigenvector
void lhpTextureOrientationCalculator::GetEigenvector(double eigenvector[3], int i) const
//------------------------------------------------------------------------------
{
  eigenvector[0] = m_V[0][i] ;
  eigenvector[1] = m_V[1][i] ;
  eigenvector[2] = m_V[2][i] ;
}


//------------------------------------------------------------------------------
// Get the ith eigenvector, scaled by eigenvalue
void lhpTextureOrientationCalculator::GetEigenvectorScaled(double eigenvector[3], int i) const
//------------------------------------------------------------------------------
{
  double sd = sqrt(m_lambda[i]) ;
  eigenvector[0] = sd * m_V[0][i] ;
  eigenvector[1] = sd * m_V[1][i] ;
  eigenvector[2] = sd * m_V[2][i] ;
}




//------------------------------------------------------------------------------
// Print self
void lhpTextureOrientationCalculator::PrintSelf(std::ostream& os, int indent) const
//------------------------------------------------------------------------------
{
  os << "no. of grey levels = " << m_numberOfGreyLevels << std::endl ;
  os << "no. of theta = " << m_numberOfTheta << std::endl ;
  os << "max no. of phi = " << m_numberOfPhiMax << std::endl ;
  os << "no. of r = " << m_numberOfR << std::endl ;
  os << "r spacing = " << m_rSpacing << std::endl ;
  os << std::endl ;

  if (m_voiDimsSet)
    os << "voi dims: " << m_voiDimensions[0] << " " << m_voiDimensions[1] << " " << m_voiDimensions[2] << std::endl ;
  else
    os << "voi dims: not set" << std::endl ;

  if (m_voiPosSet)
    os << "voi pos: " << m_voiPos[0] << " " << m_voiPos[1] << " " << m_voiPos[2] << std::endl ;
  else
    os << "voi pos: not set" << std::endl ;
  os << std::endl ;


  os << "covariance matrix" << std::endl ;
  for (int i = 0 ;  i < 3 ;  i++)
    os << m_covariance[i][0] << "\t" << m_covariance[i][1] << "\t" << m_covariance[i][2] << std::endl ;
  os << std::endl ;

  os << "eigenvectors" << std::endl ;
  for (int i = 0 ;  i < 3 ;  i++)
    os << "eigenvector " << i << "\t" << m_V[0][i] << "\t" << m_V[1][i] << "\t" << m_V[2][i] << std::endl ;
  os << std::endl ;

  os << "eigenvalues" << std::endl ;
  os << m_lambda[0] << "\t" << m_lambda[1] << "\t" << m_lambda[2] << std::endl ;
  os << std::endl ;

  os << "principal directions (r, theta, phi degrees)" << std::endl ;
  double r, theta, phi ;
  for (int i = 0 ;  i < 3 ;  i++){
    lhpTextureOrientationUseful::CartToPolar(m_V[0][i], m_V[1][i], m_V[2][i], r, theta, phi) ;
    theta *= 180.0/M_PI ;
    phi *= 180.0/M_PI ;
    os << i << "\t" << r << "\t" << theta << "\t" << phi << std::endl ;
  }
  os << std::endl ;

  // error checking - should have covar*V = lambda*V
  medOpMatrixVectorMath *mvmath = new medOpMatrixVectorMath() ;
  double M[9], V[9], MV[9], LV[9] ;
  mvmath->Copy2DArrayToMatrix3x3(m_covariance, M) ;
  mvmath->Copy2DArrayToMatrix3x3(m_V, V) ;
  mvmath->MultiplyMatrixByMatrix(M, V, MV) ;
  mvmath->MultiplyColumnsByScalars(m_lambda, V, LV) ;
  os << "error checking, matrices should be identical:" << std::endl ;
  mvmath->PrintMatrix(os, MV) ;
  mvmath->PrintMatrix(os, LV) ;

  delete mvmath ;

}
