/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationUseful.cpp,v $
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


#include "lhpTextureOrientationUseful.h"
#include "vtkMath.h"
#include "vtkPointData.h"

namespace lhpTextureOrientationUseful
{
  //----------------------------------------------------------------------------
  // Find eigenvalues and eigenvectors of 3x3 matrix
  // eigenvalues are sorted in order of largest to smallest
  // eigenvectors are the columns of V[row][col]
  // Symmetric matrices only !
  void EigenVectors3x3(double A[3][3], double lambda[3], double V[3][3]) 
    //----------------------------------------------------------------------------
  {
    vtkMath *mth = vtkMath::New() ;

    // vtk function finds eigenvalues and eigenvectors of a symmetric matrix
    mth->Diagonalize3x3(A, lambda, V) ;

    // sort into order of increasing eigenvalue
    // irritating that the vtk function does not do this
    if (lambda[1] < lambda[2]){
      std::swap(lambda[1], lambda[2]) ;
      std::swap(V[0][1], V[0][2]) ;
      std::swap(V[1][1], V[1][2]) ;
      std::swap(V[2][1], V[2][2]) ;
    }

    if (lambda[0] < lambda[1]){
      std::swap(lambda[0], lambda[1]) ;
      std::swap(V[0][0], V[0][1]) ;
      std::swap(V[1][0], V[1][1]) ;
      std::swap(V[2][0], V[2][1]) ;
    }

    if (lambda[1] < lambda[2]){
      std::swap(lambda[1], lambda[2]) ;
      std::swap(V[0][1], V[0][2]) ;
      std::swap(V[1][1], V[1][2]) ;
      std::swap(V[2][1], V[2][2]) ;
    }
  }



  //------------------------------------------------------------------------------
  // Round function which works with negative and positive numbers
  int Round(double x)
    //------------------------------------------------------------------------------
  {
    if (x >= 0.0)
      return (int)(x + 0.5) ;
    else
      return (int)(x - 0.5) ;
  }



  //------------------------------------------------------------------------------
  // Convert cartesian to polar coords
  void CartToPolar(double x, double y, double z, double &r, double &theta, double &phi)
    //------------------------------------------------------------------------------
  {
    double rr2 = x*x + y*y ;
    r = sqrt(rr2 + z*z) ;

    if (rr2 > 0.0)
      phi = atan2(y, x) ;
    else
      phi = 0.0 ;

    if (r > 0.0)
      theta = acos(z/r) ;
    else
      theta = 0.0 ;
  }


  //------------------------------------------------------------------------------
  // Convert polar to cartesian coords
  void PolarToCart(double r, double theta, double phi, double &x, double &y, double &z)
    //------------------------------------------------------------------------------
  {
    x = r * cos(phi) * sin(theta) ;
    y = r * sin(phi) * sin(theta) ;
    z = r * cos(theta) ;
  }


  //------------------------------------------------------------------------------
  // Print Attribute data, including bounds
  void PrintAttributeData(ostream& os, vtkDataSet *dataset)
    //------------------------------------------------------------------------------
  {
    dataset->Update() ;
    vtkPointData *PD = dataset->GetPointData() ;

    int na = PD->GetNumberOfArrays() ;
    os << "no. of arrays = " << na << std::endl ;

    for (int i = 0 ;  i < na ;  i++){
      os << "array " << i << std::endl ;

      const char *name = PD->GetArray(i)->GetName() ;
      if (name != NULL)
        os << "name = " << name << std::endl ;

      int nc = PD->GetArray(i)->GetNumberOfComponents() ;
      os << "no. of components = " << nc << std::endl ;

      int dtype = PD->GetArray(i)->GetDataType() ;
      os << "data type = " << vtkImageScalarTypeNameMacro(dtype) << std::endl ;

      int ntup = PD->GetArray(i)->GetNumberOfTuples() ;
      os << "no. of tuples = " << ntup << std::endl ;

      double range[2] ;
      PD->GetArray(i)->GetRange(range) ;
      os << "range = " << range[0] << " " << range[1] << std::endl ;

      os << std::endl ;
    }

    double bnds[6] ;
    dataset->GetBounds(bnds) ;
    os << "bounds " << bnds[0] << " " << bnds[1] << " " << bnds[2] << " " << bnds[3] << " " << bnds[4] << " " << bnds[5] << std::endl ; 

    os << std::endl ;

  }


  //------------------------------------------------------------------------------
  // Print Attribute data, including image dimensions, etc
  void PrintAttributeData(ostream& os, vtkImageData *imageData)
    //------------------------------------------------------------------------------
  {
    PrintAttributeData(os, (vtkDataSet*)imageData) ;

    int dims[3] ;
    imageData->GetDimensions(dims) ;
    os << "dims " << dims[0] << " " << dims[1] << " " << dims[2] << std::endl ; 

    int extents[6] ;
    imageData->GetExtent(extents) ;
    os << "extents " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3] << " " << extents[4] << " " << extents[5] << std::endl ; 

    double spacing[3] ;
    imageData->GetSpacing(spacing) ;
    os << "spacing " << spacing[0] << " " << spacing[1] << " " << spacing[2] << std::endl ; 

    double origin[3] ;
    imageData->GetOrigin(origin) ;
    os << "origin " << origin[0] << " " << origin[1] << " " << origin[2] << std::endl ; 

    os << std::endl ;
  }



  //------------------------------------------------------------------------------
  // Print Attribute data, including image dimensions, etc
  void PrintAttributeData(ostream& os, vtkStructuredPoints *structPts)
    //------------------------------------------------------------------------------
  {
    PrintAttributeData(os, (vtkImageData*)structPts) ;
  }

}