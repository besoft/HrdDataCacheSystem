/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleVectorMath.cpp,v $
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

#include "lhpMultiscaleVectorMath.h"
#include <cmath>
#include <iostream>


namespace lhpMultiscaleVectorMath{

  //------------------------------------------------------------------------------
  // Renormalize homgeneous vector x[4], so that homo coord x[3] = 1.
  void RenormalizeHomoVector(double *xh)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      xh[i] /= xh[3] ;
    xh[3] = 1.0 ;
  }


  //------------------------------------------------------------------------------
  // Magnitude of vector.
  // Vector can be x[3] or x[4] with homo coord = 1
  double MagnitudeOfVector(const double *x)
    //------------------------------------------------------------------------------
  {
    int i ;
    double sumsq ;

    for (i = 0, sumsq = 0.0 ;  i < 3 ;  i++)
      sumsq += x[i]*x[i] ;
    sumsq = sqrt(sumsq) ;
    return sumsq ;
  }


  //------------------------------------------------------------------------------
  // Normalise vector
  // Vector can be x[3] or x[4] with homo coord = 1
  void NormalizeVector(const double *x, double *xn)
    //------------------------------------------------------------------------------
  {
    double norm = MagnitudeOfVector(x) ;
    for (int i = 0 ;  i < 3 ;  i++)
      xn[i] = x[i] / norm ;
  }


  //------------------------------------------------------------------------------
  // Multiply vector by scalar
  // Vector can be x[3] or x[4] with homo coord = 1
  void MultiplyVectorByScalar(double s, const double *a, double *b)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      b[i] = s*a[i] ;
  }


  //------------------------------------------------------------------------------
  // Divide vector by scalar
  // Vector can be x[3] or x[4] with homo coord = 1
  void DivideVectorByScalar(double s, const double *a, double *b)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      b[i] = a[i] / s ;
  }

  //------------------------------------------------------------------------------
  // Add vectors: a + b = c 
  // Vectors can be x[3] or x[4] with homo coord = 1
  void AddVectors(const double *a, const double *b, double *c)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      c[i] = a[i] + b[i] ;
  }

  //------------------------------------------------------------------------------
  // Subtract vectors: a - b = c 
  // Vectors can be x[3] or x[4] with homo coord = 1
  void SubtractVectors(const double *a, const double *b, double *c)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      c[i] = a[i] - b[i] ;
  }



  //------------------------------------------------------------------------------
  // Dot product
  // Vectors can be x[3] or x[4] with homo coord = 1
  double DotProduct(const double *a, const double *b)
    //------------------------------------------------------------------------------
  {
    int i ;
    double dotprod ;

    for (i = 0, dotprod = 0.0 ;  i < 3 ;  i++)
      dotprod += a[i]*b[i] ;

    return dotprod ;
  }


  //------------------------------------------------------------------------------
  // Vector Product a^b = c
  // Vectors can be x[3] or x[4] with homo coord = 1
  void VectorProduct(const double *a, const double *b, double *c)
    //------------------------------------------------------------------------------
  {
    c[0] =   a[1]*b[2] - a[2]*b[1] ;
    c[1] = -(a[0]*b[2] - a[2]*b[0]) ;
    c[2] =   a[0]*b[1] - a[1]*b[0] ;
  }


  //------------------------------------------------------------------------------
  // Copy vector: b = a
  // Vectors can be x[3] or x[4] with homo coord = 1
  void CopyVector(const double *a, double *b)
    //------------------------------------------------------------------------------
  {
    for (int i = 0 ;  i < 3 ;  i++)
      b[i] = a[i] ;
  }


  //------------------------------------------------------------------------------
  // Print vector
  // Vectors can be x[3] or x[4] with homo coord = 1
  void PrintVector(std::ostream& os, const double *a)
    //------------------------------------------------------------------------------
  {
    os << a[0] << " " << a[1] << " " << a[2] << std::endl ;
  }


  //------------------------------------------------------------------------------
  // Are vectors equal.  Dimension 3 only
  bool Equals(const double *a, const double *b, double tol)
    //------------------------------------------------------------------------------
  {
    bool eq0 = ((a[0] > b[0]-tol) && (a[0] < b[0]+tol)) ;
    bool eq1 = ((a[1] > b[1]-tol) && (a[1] < b[1]+tol)) ;
    bool eq2 = ((a[2] > b[2]-tol) && (a[2] < b[2]+tol)) ;
    return (eq0 && eq1 && eq2) ;
  }


}