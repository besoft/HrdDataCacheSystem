/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkParametricCylinder.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkParametricCylinder.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE

#include "vtkObjectFactory.h"
#include "vtkMath.h"

vtkStandardNewMacro(vtkParametricCylinder);

//----------------------------------------------------------------------------
vtkParametricCylinder::vtkParametricCylinder() :
	Radius(1.0), Height(4.0)
{
	this->MinimumU = 0;
	this->MinimumV = 0;
	this->MaximumU = 1.0;
	this->MaximumV = 2 * vtkMath::DoublePi();

	this->JoinU = 0;
	this->JoinV = 1;
	this->TwistU = 0;
	this->TwistV = 0;
	this->ClockwiseOrdering = 0;
	this->DerivativesAvailable = 1;	
}

//----------------------------------------------------------------------------
vtkParametricCylinder::~vtkParametricCylinder()
{
}

#include <math.h>

//----------------------------------------------------------------------------
void vtkParametricCylinder::Evaluate(double uvw[3], double Pt[3])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	//x = r(u)*cosv
	//y = r(u)*sinv
	//z = h*u
	
	double cv = cos(v);
	double sv = sin(v);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	
	if (status == vtkParametricFunction::RegularPart)
	{
		//x = r*w*cos(v)
		//y = r*w*sin(v)
		//z = h*u

		// The point
		Pt[0] = this->Radius * w * cv;
		Pt[1] = this->Radius * w * sv;
		Pt[2] = this->Height * u;		
	}
	else
	{
		//we are in capped area
		if (u <= this->MinimumU || u >= this->MaximumU)
		{
			//singular cases
			// The point
			Pt[0] = 0;
			Pt[1] = 0;
			Pt[2] = this->Height * u;			
		}
		else
		{
			//x = r(u)*cos(v) = r*sin(acos(f(u))) * cos(v)
			//y = r(u)*sin(v) = r*sin(acos(f(u))) * sin(v)
			//z = h*f(u); f(u) is derived as follows: CapMin goes from Umin to Umin + tol, i.e., its parametric height is tol
			//for given u, we obtain u' from <0, 1> that tells us where we are: u' = t / tol where t = u - Umin (or Umax - u for CapMax)
			//as u' is from <0, 1> we may use some transfer function to get u'' from <0, 1>, in our case it is power to k
			//Hence f(u) = Umin + u''*tol = Umin + (t/tol)^k*tol
			//Naturally, for CapMax f(u) = Umax - u''*tol

			double t;
			if (status == vtkParametricFunction::CapMin) {
				t = u - this->MinimumU;				
			}
			else {
				//status == vtkParametricFunction::CapMax)
				t = this->MaximumU - u;				
			}		

			double k = 2.0;	//WHEN CHANGED, CHANGE ALSO Derivations
			double pw = pow(t / tol, k);
			double sroot = sqrt(1 - pow(pw - 1, 2));
			double csr = this->Radius * w * sroot;
				//sin(acos(pw - 1));
			
			// The point
			Pt[0] = csr * cv;
			Pt[1] = csr * sv;
			
			if (status == vtkParametricFunction::CapMin)
				Pt[2] = this->Height * (this->MinimumU + pw*tol);
			else
				Pt[2] = this->Height * (this->MaximumU - pw*tol);
		}
	}	
}

// Description:
// Performs the mapping \$Pt -> f(uvw)\$f, if available.  
//
// Pt is the Cartesian point for which u, v and w parameters are to be located,
// uvw are the parameters, with u corresponding to uvw[0], v to uvw[1] and w to uvw[2] respectively. 
// The method returns false, if uvw parameters could not be found
/*virtual*/ bool vtkParametricCylinder::EvaluateInverse(double Pt[3], double uvw[3])
{
	//x = r*w*cos(v)
	//y = r*w*sin(v)
	//z = h*u

	double uMin = this->MinimumU;
	double uMax = this->MaximumU;

	if (this->CapPortionAvailable != 0)
	{
		double uCap = (uMax - uMin)* this->CapPortion;	//in u parameter
		uMin += uCap;	//uMin is where the regular part starts
		uMax -= uCap;	//uMax is where the regular part ends
	}

	uvw[0] = Pt[2] / this->Height;
	if (uvw[0] < uMin || uvw[0] > uMax)
		return false;	//WE SUPPORT ONLY REGULAR PART

	//x = r*w*cos(v)
	//y = r*w*sin(v)	
	uvw[1] = SolveGoniometricAngle(Pt[0], Pt[1]);
	if (uvw[1] < this->MinimumV || uvw[1] > this->MaximumV)
		return false;

	if (this->OnionLayers == 0) {
		uvw[2] = 1.0;		
	}
	else
	{		
		uvw[2] = sqrt(Pt[0]*Pt[0] + Pt[1]*Pt[1]) / this->Radius;
		if (uvw[2] < 0.0 || uvw[2] > 1.0)
			return false;
	}
	
	return true;
}

//Computes partial derivatives at the point f(u,v,w).
 /*virtual*/ void vtkParametricCylinder::EvaluateDerivative(double uvw[3], double Duvw[9])
 {
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	double *Du = Duvw;
	double *Dv = Duvw + 3;
	double *Dw = Duvw + 6;

	double cv = cos(v);
	double sv = sin(v);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	if (status == vtkParametricFunction::RegularPart)
	{
		//x = r*w*cos(v)	=> dx/du = 0, dx/dv = -rw*sinv, dx/dw = r*cosv
		//y = r*w*sin(v)	=> dy/du = 0, dy/dv = rw*cosv, dy/dw = r*sinv
		//z = h*u			=> dz/du = h, dz/dv = 0, dz/dw = 0

		//The derivatives are:
		Du[0] = 0;
		Du[1] = 0;
		Du[2] = this->Height;

		Dv[0] = -this->Radius * w * sv;
		Dv[1] = this->Radius * w * cv;
		Dv[2] = 0;

		Dw[0] = this->Radius * cv;
		Dw[1] = this->Radius * sv;
		Dw[2] = 0;
	}
	else
	{
		//we are in capped area
		if (u <= this->MinimumU || u >= this->MaximumU)
		{
			//Singular case
			//As the derivatives u do not exist, we just make sure that normal will be calculated correctly:
			Dv[0] = -this->Radius * sv;
			Dv[1] = this->Radius * cv;
			Dv[2] = 0;

			Dw[0] = 0; Dw[1] = 0;
			if (status == vtkParametricFunction::CapMin)
				Dw[2] = this->Height * (u - tol);
			else
				Dw[2] = this->Height * (u - this->MaximumU + tol);						

			vtkMath::Cross(Dw, Dv, Du);	
		}
		else
		{
			//x = r(u)*cos(v) = r*sin(acos(f(u))) * cos(v)
			//y = r(u)*sin(v) = r*sin(acos(f(u))) * sin(v)
			//z = h*g(u); g(u) is derived as follows: CapMin goes from Umin to Umin + tol, i.e., its parametric height is tol
			//for given u, we obtain u' from <0, 1> that tells us where we are: u' = t / tol where t = u - Umin (or Umax - u for CapMax)
			//as u' is from <0, 1> we may use some transfer function to get u'' from <0, 1>, in our case it is power to k
			//Hence g(u) = Umin + u''*tol = Umin + (t/tol)^k*tol
			//Naturally, for CapMax g(u) = Umax - u''*tol

			//f(u): let f(x) = (x - p) / p where p = radius of the cap, i.e., h'*PortionCap and h'=available height = h*(vmax - vmin) => p = h*tol
			//as x is from <0, p> we must map cap u to this interval
			//to diminish the fact that delta_v is linear but the distance on the cap is not, the mapping is non-linear
			//For CapMin, u is from <Vmin, Vmin+tol> => u', which is from <0, 1>, = (u - vmin) / tol
			//u'', which is from <0, 1> but nonlinearly, = u'^2 = (u - vmin)^2/tol^2
			//=> x = p*u'' = h*(u - vmin)^2/tol => f(u) = (u - vmin)^2/tol^2 - 1
			//For CapMax, u is from <Vmax-tol, Vmax> => u' = (vmax-u) / tol => u'' = (vmax-u)^2 / tol^2
			//=> x = p*u'' = h*(vmax - u)^2/tol => f(u) = (vmax - u)^2/tol^2 - 1

			double t, sgn;
			if (status == vtkParametricFunction::CapMin)
			{
				t = u - this->MinimumU;
				sgn = -1.0;
			}
			else
			{
				//status == vtkParametricFunction::CapMax)
				t = this->MaximumU - u;
				sgn = 1.0;
			}		

			double a = this->Radius * w;

			double k = 2.0;
			double pw = pow(t / tol, k);			
			double sroot = sqrt(1 - pow(pw - 1, 2)); //this is sin(acos(pw - 1));
						
			Du[1] = Du[0] = sgn * k * a * (pw - 1) * pw / (t * sroot); 
			Du[0] *= cv; Du[1] *= sv;
			Du[2] = this->Height * k * pw * tol / t;

			Dv[0] = -a * sroot * sv; 
			Dv[1] = a * sroot * cv; 
			Dv[2] = 0;

			Dw[0] = this->Radius * sroot * cv; 
			Dw[1] = this->Radius * sroot * sv; 
			Dw[2] = 0;
		}
	}	
 }

 //Computes partial derivatives of 2nd order at the point f(u,v,w).
 /*virtual*/ void vtkParametricCylinder::EvaluateDerivative2(double uvw[3], double Duvw[9])
 {
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	memset(Duvw, 0, 9*sizeof(double));

	double *D2u = Duvw;
	double *D2v = Duvw + 3;
	double *D2w = Duvw + 6;	

	double cv = cos(v);
	double sv = sin(v);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	if (status == vtkParametricFunction::RegularPart)
	{
		//x = r*w*cos(v)	=> dx/du = 0, dx/dv = -rw*sinv, dx/dw = r*cosv
		//y = r*w*sin(v)	=> dy/du = 0, dy/dv = rw*cosv, dy/dw = r*sinv
		//z = h*u			=> dz/du = h, dz/dv = 0, dz/dw = 0
		//
		// => d2x/du2 = 0, d2x/dv2 = -rw*cosv, d2x/dw2 = 0
		// => d2y/du2 = 0, d2y/dv2 = -rw*sinv, d2y/dw2 = 0
		// => d2z/du2 = 0, d2z/dv2 = 0, d2z/dw2 = 0

		//The derivatives are:		
		D2v[0] = -this->Radius * w * cv;
		D2v[1] = -this->Radius * w * sv;		
	}
	else
	{
		//we are in capped area
		//x = r(u)*cos(v) = r*sin(acos(f(u))) * cos(v)
		//y = r(u)*sin(v) = r*sin(acos(f(u))) * sin(v)
		//z = h*g(u); g(u) is derived as follows: CapMin goes from Umin to Umin + tol, i.e., its parametric height is tol
		//for given u, we obtain u' from <0, 1> that tells us where we are: u' = t / tol where t = u - Umin (or Umax - u for CapMax)
		//as u' is from <0, 1> we may use some transfer function to get u'' from <0, 1>, in our case it is power to k
		//Hence g(u) = Umin + u''*tol = Umin + (t/tol)^k*tol
		//Naturally, for CapMax g(u) = Umax - u''*tol

		double t, sgn;
		if (status == vtkParametricFunction::CapMin)
		{
			t = u - this->MinimumU;
			sgn = -1.0;
		}
		else
		{
			//status == vtkParametricFunction::CapMax)
			t = this->MaximumU - u;
			sgn = 1.0;
		}		

		double a = this->Radius * w;

		double k = 2.0;
		double pw = pow(t / tol, k);			
		double sroot = sqrt(1 - pow(pw - 1, 2)); //this is sin(acos(pw - 1));

		//according to http://www.wolframalpha.com/
		D2u[1] = D2u[0] = sgn* k * a * pw * ((k - 1)*(pw*pw - pw*3 + 1) - 1) /
			(t*t * (pw - 2) * sroot);
		D2u[0] *= cv; D2u[1] *= sv;		
		D2u[2] = -sgn * this->Height * k * (k - 1) * pw * (tol / t) * (tol / t);
		
		D2v[0] = -a * sroot * cv; 
		D2v[1] = -a * sroot * sv; 
		D2v[2] = 0;

		//D2w = 0
	}	
 }

//----------------------------------------------------------------------------
double vtkParametricCylinder::EvaluateScalar(double* vtkNotUsed(uv[3]),
										  double* vtkNotUsed(Pt[3]),
										  double* vtkNotUsed(Duv[9]))
{
	return 0;
}

//----------------------------------------------------------------------------
void vtkParametricCylinder::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "Radius: " << this->Radius << "\n";
	os << indent << "Height: " << this->Height << "\n";
}
#endif