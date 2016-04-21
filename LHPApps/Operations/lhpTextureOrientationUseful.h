/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationUseful.h,v $
Language:  C++
Date:      $Date: 2009-06-26 14:03:04 $
Version:   $Revision: 1.1.1.1.2.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpTextureOrientationUseful_H__
#define __lhpTextureOrientationUseful_H__

#include "vtkDataSet.h"
#include "vtkImageData.h"
#include "vtkStructuredPoints.h"


//------------------------------------------------------------------------------
/// Helper namespace for lhpOpTextureOrientation. \n
/// Useful misc methods.
//------------------------------------------------------------------------------
namespace lhpTextureOrientationUseful
{
  /// Find eigenvalues and eigenvectors of 3x3 matrix
  /// eigenvalues are sorted in order of largest to smallest
  /// eigenvectors are the columns of V[row][col]
  /// Symmetric matrices only !
  void EigenVectors3x3(double A[3][3], double lambda[3], double V[3][3]) ;

  /// Round function which works with negative and positive numbers
  int Round(double x) ;

  /// Convert cartesian to polar coords
  void CartToPolar(double x, double y, double z, double &r, double &theta, double &phi) ;

  /// Convert polar to cartesian coords.
  /// Returns zero for undefined theta or phi.
  void PolarToCart(double r, double theta, double phi, double &x, double &y, double &z) ;

  void PrintAttributeData(ostream& os, vtkDataSet *dataset) ;  ///< Print Attribute data
  void PrintAttributeData(ostream& os, vtkImageData *imageData) ;  ///< Print Attribute data
  void PrintAttributeData(ostream& os, vtkStructuredPoints *structPts) ;  ///< Print Attribute data

}


#endif
