/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParametricMuscle.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParametricMuscle - Generate a torus.
// .SECTION Description
// vtkParametricMuscle generates a torus.
//
// For further information about this surface, please consult the
// technical description "Parametric surfaces" in http://www.vtk.org/documents.php
// in the "VTK Technical Documents" section in the VTk.org web pages.
//
// .SECTION Thanks
// Andrew Maclean a.maclean@cas.edu.au for
// creating and contributing the class.
//
#ifndef __vtkParametricMuscle_h
#define __vtkParametricMuscle_h

#include "vtkMAFMuscleDecomposition.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "vtkParametricFunction.h"
#include "solvers.h"

class vtkParametricMuscle : public vtkParametricFunction
{

public:
  vtkTypeMacro(vtkParametricMuscle,vtkParametricFunction);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct an object with the default parameters:  
  static vtkParametricMuscle *New();

  // Description:
  // Set/Get the radius X of the central curve.  The default value is 2.0.
  vtkSetMacro(RadiusX,double);
  vtkGetMacro(RadiusX,double);

  // Description:
  // Set/Get the radius Y of the central curve.  The default value is 4.0.
  vtkSetMacro(RadiusY,double);
  vtkGetMacro(RadiusY,double);

  // Description:
  // Set/Get the radius of the object.  The default value is 1.0.
  vtkSetMacro(CrossRadiusX1,double);
  vtkGetMacro(CrossRadiusX1,double);

  // Description:
  // Set/Get the radius of the object.  The default value is 1.5.
  vtkSetMacro(CrossRadiusX2,double);
  vtkGetMacro(CrossRadiusX2,double);

  // Description:
  // Set/Get the radius of the object.  The default value is 0.5.
  vtkSetMacro(CrossRadiusZ1,double);
  vtkGetMacro(CrossRadiusZ1,double);

  // Description:
  // Set/Get the radius of the object.  The default value is 0.75.
  vtkSetMacro(CrossRadiusZ2,double);
  vtkGetMacro(CrossRadiusZ2,double);

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

	typedef struct SOLVER_CONTEXT_DATA
	{
		vtkParametricMuscle* pThis;
		double Pt[3];
	} SOLVER_CONTEXT_DATA;

	static void EvaluateInverse_fvec(const alglib::real_1d_array &x, double &func, void *ptr);
	static void EvaluateInverse_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr);

	//objective function
	void EvaluateInverse_fvec(const alglib::real_1d_array &x, const double Pt[3], double &func);
	
	//jacobian and function values
	void EvaluateInverse_jac(const alglib::real_1d_array &x, const double Pt[3], alglib::real_1d_array &fi, alglib::real_2d_array &jac);

	enum InverseIntervalsState
	{	
		FirstAttempt,
		RestartedAttempt,

		AnalyticSolutionFoundRestartNotAllowed,
		AnalyticSolutionFoundRestartAllowed,
		NumericSolutionNeededRestartNotAllowed,
		NumericSolutionNeededRestartAllowed,		
	};

	/** Attempts to refine the intervals in which the solution u,v,w can be found. 
	Returns false, if the intervals could not be refined, true, otherwise. */
	bool FindEvaluateInverseIntervals(const double Pt[3], double uMinMax[2], double vMinMax[2], double wMinMax[2], InverseIntervalsState& state);	

	/** 
	Rx + a(u)*w*cos(v) = 0 and z = b(u)*w*sin(v), if xNotNull == false, or
	Ry + a(u)*w*cos(v) = 0 and z = b(u)*w*sin(v), if xNotNull == true*/
	bool FindEvaluateInverseIntervalsCase2(const double Pt[3], double uMinMax[2], double vMinMax[2], double wMinMax[2], bool xNotNull);

	/** x = (Rx + a(u)*w*cos(v))*cos(u),  Ry + a(u)*w*cos(v) = 0, and z = b(u)*w*sin(v) - xNotNull == true, or
	 (Rx + a(u)*w*cos(v))*cos(u) = 0,  y = (Ry + a(u)*w*cos(v))*sin(u) = 0, and z = b(u)*w*sin(v) - xNotNull == false*/
	bool FindEvaluateInverseIntervalsCase6(const double Pt[3], double uMinMax[2], double vMinMax[2], double wMinMax[2], bool xNotNull);

	//Calculates a(u)
	inline double GetAu(double u, ParametricFunctionPart status = vtkParametricFunction::Unknown, double tol = 0.0) {
		return LerpAB(u, status, tol, this->CrossRadiusX1, this->CrossRadiusX2);
	}
	
	//Calculates b(u)
	inline double GetBu(double u, ParametricFunctionPart status = vtkParametricFunction::Unknown, double tol = 0.0){
		return LerpAB(u, status, tol, this->CrossRadiusZ1, this->CrossRadiusZ2);
	}

	//Performs linear interpolation between minVal and maxVal according to u, status and tol.
	//If status is Unknown, it is automatically determined from u value
	double LerpAB(double u, ParametricFunctionPart status, double tol, double minVal, double maxVal);
	
protected:
  vtkParametricMuscle();
  ~vtkParametricMuscle();

  // Variables
  double RadiusX;
  double RadiusY;
  double CrossRadiusX1;
  double CrossRadiusZ1;
  double CrossRadiusX2;
  double CrossRadiusZ2;

private:
  vtkParametricMuscle(const vtkParametricMuscle&);  // Not implemented.
  void operator=(const vtkParametricMuscle&);  // Not implemented.
};
#endif
#endif
