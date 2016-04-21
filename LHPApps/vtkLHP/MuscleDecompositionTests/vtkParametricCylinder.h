/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParametricCylinder.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParametricCylinder - Generate a torus.
// .SECTION Description
// vtkParametricCylinder generates a torus.
//
// For further information about this surface, please consult the
// technical description "Parametric surfaces" in http://www.vtk.org/documents.php
// in the "VTK Technical Documents" section in the VTk.org web pages.
//
// .SECTION Thanks
// Andrew Maclean a.maclean@cas.edu.au for
// creating and contributing the class.
//
#ifndef __vtkParametricCylinder_h
#define __vtkParametricCylinder_h

#include "vtkMAFMuscleDecomposition.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE

#include "vtkParametricFunction.h"

class vtkParametricCylinder : public vtkParametricFunction
{

public:
  vtkTypeMacro(vtkParametricCylinder,vtkParametricFunction);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct a cylinder with the following parameters:
  // MinimumU = 0, MaximumU = 1, //U is along the height
  // MinimumV = 0, MaximumV = 2*Pi,
  // JoinU = 1, JoinV = 1,
  // TwistU = 0, TwistV = 0,
  // ClockwiseOrdering = 1,
  // DerivativesAvailable = 1,
  // Radius = 1, Height = 4.
  static vtkParametricCylinder *New();

  // Description:
  // Set/Get the radius.  The default value is 1.0.
  vtkSetMacro(Radius,double);
  vtkGetMacro(Radius,double);

  // Description:
  // Set/Get the height.  The default value is 4.
  vtkSetMacro(Height,double);
  vtkGetMacro(Height,double);

  // Description
  // Return the parametric dimension of the class.
  /*virtual*/ int GetDimension() {return 2;}

  // Description:
  // A torus.
  //
  // This function performs the mapping \f$f(u,v) \rightarrow (x,y,x)\f$, returning it as Pt    
  /*virtual*/ void Evaluate(double uvw[3], double Pt[3]);
  
  // Description:
  // Performs the mapping \$Pt -> f(uvw)\$f, if available.    
  // Pt is the Cartesian point for which u, v and w parameters are to be located,
  // uvw are the parameters, with u corresponding to uvw[0], v to uvw[1] and w to uvw[2] respectively. 
  // The method returns false, if uvw parameters could not be found
  /*virtual*/ bool EvaluateInverse(double Pt[3], double uvw[3]);

   //Computes partial derivatives at the point f(u,v,w).
  /*virtual*/ void EvaluateDerivative(double uvw[3], double Duvw[9]);

  //Computes partial derivatives of 2nd order at the point f(u,v,w).
  /*virtual*/ void EvaluateDerivative2(double uvw[3], double Duvw[9]);

  // Description:
  // Calculate a user defined scalar using one or all of uvw, Pt, Duvw.
  //
  // uvw are the parameters with Pt being the the Cartesian point,
  // Duvw are the derivatives of this point with respect to u, v and w.
  // Pt, Duvw are obtained from Evaluate().
  //
  // This function is only called if the ScalarMode has the value
  // vtkParametricFunctionSource::SCALAR_FUNCTION_DEFINED
  //
  // If the user does not need to calculate a scalar, then the
  // instantiated function should return zero.
  //
  /*virtual*/ double EvaluateScalar(double uvw[3], double Pt[3], double Duvw[9]);  

protected:
  vtkParametricCylinder();
  ~vtkParametricCylinder();

  // Variables
  double Radius;
  double Height;

private:
  vtkParametricCylinder(const vtkParametricCylinder&);  // Not implemented.
  void operator=(const vtkParametricCylinder&);  // Not implemented.
};

#endif
#endif
