/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleVectorMath.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleVectorMath_H__
#define __lhpMultiscaleVectorMath_H__

#include <iostream>

/*******************************************************************************
lhpMultiscaleVectorMath:
Namespace for simple vector math methods
Except for RenormalizeHomoVector(), these methods assume vector has dimension 3, 
so they also work where the dimension is 4 and the homo coord has been renormalised to 1.
*******************************************************************************/

namespace lhpMultiscaleVectorMath{
  void RenormalizeHomoVector(double *xh) ;                       ///< Divide through by homo coord, so x[3] = 1.  Dimension is homo 4.
  double MagnitudeOfVector(const double *x) ;                       ///< Magnitude of vector.
  void NormalizeVector(const double *x, double *xn) ;                  ///< Normalize vector
  void MultiplyVectorByScalar(double s, const double *a, double *b) ;   ///< Multiply vector by scalar.
  void DivideVectorByScalar(double s, const double *x, double *b) ;     ///< Divide vector by scalar.
  void AddVectors(const double *a, const double *b, double *c) ;              ///< Add vectors: a + b = c
  void SubtractVectors(const double *a, const double *b, double *c) ;         ///< Subtract vectors: a - b = c
  double DotProduct(const double *a, const double *b) ;                      ///< Dot product a.b
  void VectorProduct(const double *a, const double *b, double *c) ;         ///< Vector Product a^b = c
  void CopyVector(const double *a, double *b) ;                         ///< Copy vector: b = a
  void PrintVector(std::ostream& os, const double *a) ;                        ///< print vector
  bool Equals(const double *a, const double *b, double tol) ;            ///< Are vectors equal.  Dimension 3 only
}

#endif

