/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkParametricTorus.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkParametricTorus.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "vtkObjectFactory.h"
#include "vtkMath.h"

vtkStandardNewMacro(vtkParametricTorus);

//----------------------------------------------------------------------------
vtkParametricTorus::vtkParametricTorus() :
	Radius(1.0), CrossRadius(0.5)
{
	this->MinimumU = 0;
	this->MinimumV = 0;
	this->MaximumU = 2 * vtkMath::DoublePi();
	this->MaximumV = 2 * vtkMath::DoublePi();

	this->JoinU = 1;
	this->JoinV = 1;
	this->TwistU = 0;
	this->TwistV = 0;
	this->ClockwiseOrdering = 1;
	this->DerivativesAvailable = 1;	
}

//----------------------------------------------------------------------------
vtkParametricTorus::~vtkParametricTorus()
{
}

#include <math.h>

//----------------------------------------------------------------------------
void vtkParametricTorus::Evaluate(double uvw[3], double Pt[3])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;
		
	double cv = cos(v);
	double sv = sin(v);
	double cu = cos(u);
	double su = sin(u);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	
	if (status == vtkParametricFunction::RegularPart)
	{
		//x = (r + c*w cosv) cos u
		//y = (r + c*w cosv) sin u
		//z = c*w sin v

		// The point
		Pt[0] = (this->Radius + this->CrossRadius * w * cv) * cu;
		Pt[1] = (this->Radius + this->CrossRadius * w * cv) * su;
		Pt[2] = this->CrossRadius * w* sv;
	}
	else
	{
		//we are in capped area
		//x = (r + c(u)*w cosv) cos u
		//y = (r + c(u)*w cosv) sin u 
		//z = c(u)*w*sin v
		//c(u) = c*sin(acos(f(u)), where f(u) maps u non-linearly to <0..1> as follows.
		//CapMin: f(u) = ((u - Umin) / ((Umax - Umin)*CapPortion))^k - 1 = (u - Umin) / tol)^k - 1 
		//CapMax: f(u) = ((Umax - u)/ ((Umax - Umin)*CapPortion))^k - 1 = (Umax - u) / tol)^k - 1 

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
		double sroot = sqrt(1 - pow(pw - 1, 2));	//this is actually sin(acos(pw - 1));
		double csr = this->CrossRadius * sroot;				

		// The point
		Pt[0] = (this->Radius + csr * w *  cv) * cu;
		Pt[1] = (this->Radius + csr * w * cv) * su;
		Pt[2] = csr * w * sv;
	}	
}

// Description:
// Performs the mapping \$Pt -> f(uvw)\$f, if available.  
//
// Pt is the Cartesian point for which u, v and w parameters are to be located,
// uvw are the parameters, with u corresponding to uvw[0], v to uvw[1] and w to uvw[2] respectively. 
// The method returns false, if uvw parameters could not be found
/*virtual*/ bool vtkParametricTorus::EvaluateInverse(double Pt[3], double uvw[3])
{
	//x = (r + c*w cosv) cos u
	//y = (r + c*w cosv) sin u
	//z = c*w sin v

	double uMin = this->MinimumU;
	double uMax = this->MaximumU;

	if (this->CapPortionAvailable != 0)
	{
		double uCap = (uMax - uMin)* this->CapPortion;	//in u parameter
		uMin += uCap;	//uMin is where the regular part starts
		uMax -= uCap;	//uMax is where the regular part ends
	}
			
	if (!IsZero(Pt[0]))
	{
		//x != 0		
		//x = (r + c*w cosv)cos(u) = const*cos(u)
		//y= (r + c*w cosv)sin(u) = const*sin(u), where const > 0 because r>c, w*cosv <= 1 => r*1 > c*w*cosv
		uvw[0] = SolveGoniometricAngle(Pt[0], Pt[1]);

		//x/cosu - r = c*w*cos(v)
		//y/sinu - r = c*w*cos(v)
		//z = c*w*sin(v)
		double xr = Pt[0]/cos(uvw[0]) - this->Radius;
		if (IsZero(xr))
		{
			//either w = 0 or cos(v) = 0
			if (IsZero(Pt[2])) {
				uvw[1] = (this->MaximumV + this->MinimumV) / 2; uvw[2] = 0.0;
			}
			else 
			{
				//v = PI/2 or 3/2PI => sin(v) = +/-1
				uvw[1] = vtkMath::DoublePi() / 2;
				uvw[2] = Pt[2] / this->CrossRadius;
				if (uvw[2] < 0){
					uvw[1] *= 3; uvw[2] = -uvw[2];
				}
			}
		}
		else
		{
			uvw[1] = SolveGoniometricAngle(xr, Pt[2]);
			uvw[2] = Pt[2] / (this->CrossRadius * sin(uvw[1]));
		}
	}
	else
	{
		//x = 0
		if (IsZero(Pt[1]))
		{
			//y = 0 => r + c*w cosv = 0 => u may be anything
			uvw[0] = (uMax + uMin) / 2;

			//r + c*w cosv = 0	=> cosv = -r/cw
			//z = c*w sin v		=> sinv = z/cw
			//as SolveGoniometricAngle uses atan to compute v and cw > 0, it is enough -r,z
			uvw[1] = SolveGoniometricAngle(-this->Radius, Pt[2]);
			if (IsZero(Pt[2]))	//sinv = 0
				uvw[2] = -this->Radius / (this->CrossRadius * cos(uvw[1]));
			else
				uvw[2] = Pt[2] / (this->CrossRadius * sin(uvw[1]));			
		}
		else
		{
			//cos(u) = 0, then u is either PI/2 or 3*PI/2 => check, what is more appropriate
			double su = 1.0;
			uvw[0] = vtkMath::DoublePi() / 2;
			if (uvw[0] < uMin || uvw[0] > uMax) {
				su = -1.0; uvw[0] *= 3; //if PI/2 is not valid, choose 3*PI/2 for u
			}

			//y = (r + c*w*cos(v))*(+/-1)	
			double yr = Pt[1]/su - this->Radius;
			if (IsZero(yr))
			{
				//y +/-r = 0 => c*w*cos(v) = 0			
				//=> either w = 0 or cos(v) = 0
				if (IsZero(Pt[2]))
				{					
					uvw[1] = (this->MaximumV + this->MinimumV) / 2;	//v is anything
					uvw[2] = 0.0;
				}
				else
				{
					//v = PI/2 or 3/2 PI => z = c*w*(+/-1)
					uvw[1] = vtkMath::DoublePi() / 2;
					uvw[2] = Pt[2] / this->CrossRadius;
					if (uvw[2] < 0.0) {
						uvw[2] = -uvw[2]; uvw[1] *= 3;
					}
				}
			}
			else
			{
				//x = 0, y != 0, y +/-r != 0 =>				
				//(+/-1)*y - r = c*w*cos(v) and z = c*w*sin(v)
				//as SolveGoniometricAngle uses atan to compute v and cw > 0, it is enough yr,z
				uvw[1] = SolveGoniometricAngle(yr, Pt[2]);
				uvw[2] = yr / (this->CrossRadius * cos(uvw[1]));
				if (uvw[2] < 0.0 || uvw[1] < this->MinimumV || uvw[1] > this->MaximumV) {
					//either u must be 3/2*PI or point is invalid
					yr = -Pt[1]/su - this->Radius;
					uvw[0] = 3*vtkMath::DoublePi() / 2;
					uvw[1] = SolveGoniometricAngle(yr, Pt[2]);
					uvw[2] = yr / (this->CrossRadius * cos(uvw[1]));
				}
			}
		}
	}	
			
	if (uvw[0] < uMin || uvw[0] > uMax)
		return false;	//WE SUPPORT ONLY REGULAR PART
	
	if (uvw[1] < this->MinimumV || uvw[1] > this->MaximumV)
		return false;

	if (uvw[2] < 0.0 || uvw[2] > 1.0)
		return false;

	return true;	
}

//Computes partial derivatives at the point f(u,v,w).
 /*virtual*/ void vtkParametricTorus::EvaluateDerivative(double uvw[3], double Duvw[9])
 {		
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	double *Du = Duvw;
	double *Dv = Duvw + 3;
	double *Dw = Duvw + 6;

	double cv = cos(v);
	double sv = sin(v);
	double cu = cos(u);
	double su = sin(u);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	if (status == vtkParametricFunction::RegularPart)
	{		
		//x = (r + c*w*cos(v)) cos(u)	=> dx/du = -(r + c*w cosv)*sin u, dx/dv = -c*w*sin v * cosu, dx/dw = c*cosv*cosu
		//y = (r + c*w cosv) sin u	=> dx/du = (r + c*w cosv)*cos u, dx/dv = -c*w*sin v * sinu, dx/dw = c*cosv*sinu
		//z = c*w sin v			=> dx/du = 0, dx/dv = cw*cosv, dx/dw = c*sinv

		//The derivatives are:
		Du[1] = Du[0] = (this->Radius + this->CrossRadius*w*cv);
		Du[0] *= -su; Du[1] *= cu;
		Du[2] = 0;

		Dv[0] = Dv[1] = -this->CrossRadius * w * sv;
		Dv[0] *= cu; Dv[1] *= su;
		Dv[2] = this->CrossRadius * w * cv;
		
		Dw[0] = this->CrossRadius * cv * cu;
		Dw[1] = this->CrossRadius * cv * su;
		Dw[2] = this->CrossRadius * sv;
	}
	else
	{
		//we are in capped area
		//x = (r + c(u)*w cosv) cos u	=> dx/du = w cos(u) cos(v) c'(u)-sin(u) (w c(u) cos(v)+r),	dx/dv = -w c(u) cos(u) sin(v),		dx/dw = c(u) cos(u) cos(v)
		//y = (r + c(u)*w cosv) sin u  => dy/du = w sin(u) cos(v) c'(u)+cos(u) (w c(u) cos(v)+r),	dy/dv = -w c(u) sin(u) sin(v),		dy/dw = c(u) sin(u) cos(v)
		//z = c(u)*w sin v			=> dz/du = w sin(v) c'(u),							dz/dv = w c(u) cos(v),			dz/dw = c(u) sin(v)
		//c(u) = c*sin(acos(f(u))						=> c'(u) = -(c f(u) f'(u))/sqrt(1-f(u)^2)				
		//CapMin: f(u) = ((u - Umin) / tol)^k - 1		=> f'(u) = (k ((u-Umin)/tol)^(k-1))/tol
		//CapMax: f(u) = ((Umax - u) / tol)^k - 1 		=> f'(u) = -(k ((Umax - u)/tol)^(k-1))/tol	

		if (u <= this->MinimumU || u >= this->MaximumU)
		{
			//Singular case
			//As the c'(u) does not exist since (f(u) -> 1) => c(u) = 0, c'(u) -> INF, we just make sure that normal will be calculated correctly:
			//Actually, the case degenerates to:
			//x = r*cos(u), y = r*sin(u), and z = 0, hence
			//dx/du = -r*sin(u),	dx/dv = 0,		dx/dw = 0
			//dy/du = r*cos(u),		dy/dv = 0,		dy/dw = 0
			//dz/du = -0,			dz/dv = 0,		dz/dw = 0			
			//Du is the normal => we need some trick:
			Dw[0] = -this->Radius * su;
			Dw[1] =  this->Radius * cu;
			Dw[2] = 0;

			if (status == vtkParametricFunction::CapMax)
				vtkMath::Perpendiculars(Dw, Du, Dv, 0.0);			
			else
				vtkMath::Perpendiculars(Dw, Dv, Du, 0.0);			
		}
		else
		{
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
			double f = pw - 1;	//f(u)
			double df = (k / tol) * pow(t / tol, k - 1);			//f'(u)
			if (status == vtkParametricFunction::CapMax)			
				df = -df;

			double sroot = sqrt(1 - f*f);							//this is actually sin(acos(f(u)));
			double c = this->CrossRadius * sroot;					//c(u)
			double dc = -(this->CrossRadius* f* df) / sroot;		//c'(u)

			double a = w*c*cv + this->Radius;						// w c(u) cos(v)+r;
			Du[0] = w*cu*cv*dc - su*a;								//w cos(u) cos(v) c'(u)-sin(u) (w c(u) cos(v)+r)
			Du[1] = w*su*cv*dc + cu*a;								//w sin(u) cos(v) c'(u)+cos(u) (w c(u) cos(v)+r)
			Du[2] = w*sv*dc;										//w sin(v) c'(u)

			Dv[0] = -w*c*cu*sv;										// -w c(u) cos(u) sin(v)
			Dv[1] = -w*c*su*sv;										//-w c(u) sin(u) sin(v)
			Dv[2] = w*c*cv;											//w c(u) cos(v)

			Dw[0] = c*cu*cv;										//c(u) cos(u) cos(v)
			Dw[1] = c*su*cv;										//c(u) sin(u) cos(v)
			Dw[2] = c*sv;											//c(u) sin(v)
		}
	}
 }

 //Computes partial derivatives of 2nd order at the point f(u,v,w).
 /*virtual*/ void vtkParametricTorus::EvaluateDerivative2(double uvw[3], double Duvw[9])
 {	 
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	memset(Duvw, 0, 9*sizeof(double));	//reset 

	double *Du = Duvw;
	double *Dv = Duvw + 3;
	double *Dw = Duvw + 6;

	double cv = cos(v);
	double sv = sin(v);
	double cu = cos(u);
	double su = sin(u);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	if (status == vtkParametricFunction::RegularPart)
	{		
		//x = (r + c*w*cos(v)) cos(u)	=> dx/du = -(r + c*w cosv)*sin u, dx/dv = -c*w*sin v * cosu, dx/dw = c*cosv*cosu
		//y = (r + c*w cosv) sin u	=> dy/du = (r + c*w cosv)*cos u, dy/dv = -c*w*sin v * sinu, dy/dw = c*cosv*sinu
		//z = c*w sin v			=> dz/du = 0, dz/dv = cw*cosv, dz/dw = c*sinv

		//d2x/du2 = -(r + c*w*cos(v))*cos(u),		d2x/dv2 = -c*w*cos(v)*cos(u),		d2x/dw2 = 0
		//d2y/du2 = -(r + c*w*cos(v))*sin(u),		d2y/dv2 = -c*w*cos(v)*sin(u),		d2y/dw2 = 0
		//d2z/du2 = 0,						d2z/dv2 = -c*w*sin(v),				d2z/dw2 = 0

		//The derivatives are:
		Du[1] = Du[0] = -(this->Radius + this->CrossRadius*w*cv);
		Du[0] *= cu; Du[1] *= su;		

		Dv[0] = Dv[1] = -this->CrossRadius * w * cv;
		Dv[0] *= cu; Dv[1] *= su;
		Dv[2] = -this->CrossRadius * w * sv;				
	}
	else
	{
		//we are in capped area
		//x = (r + c(u)*w cosv) cos u	=> dx/du = w cos(u) cos(v) c'(u)-sin(u) (w c(u) cos(v)+r),	dx/dv = -w c(u) cos(u) sin(v),		dx/dw = c(u) cos(u) cos(v)
		//y = (r + c(u)*w cosv) sin u  => dy/du = w sin(u) cos(v) c'(u)+cos(u) (w c(u) cos(v)+r),	dy/dv = -w c(u) sin(u) sin(v),		dy/dw = c(u) sin(u) cos(v)
		//z = c(u)*w sin v			=> dz/du = w sin(v) c'(u),							dz/dv = w c(u) cos(v),			dz/dw = c(u) sin(v)
		//c(u) = c*sin(acos(f(u))						=> c'(u) = -(c f(u) f'(u))/sqrt(1-f(u)^2)				
		//CapMin: f(u) = ((u - Umin) / tol)^k - 1		=> f'(u) = (k ((u-Umin)/tol)^(k-1))/tol
		//CapMax: f(u) = ((Umax - u) / tol)^k - 1 		=> f'(u) = -(k ((Umax - u)/tol)^(k-1))/tol	

		//d2x/du2 = w cos(v) (cos(u) (c''(u)-c(u))-2 sin(u) c'(u))-r cos(u),		d2x/dv2 = -w c(u) cos(u) cos(v),		d2x/dw2 = 0
		//d2y/du2 = w cos(v) (sin(u) (c''(u)-c(u))+2 cos(u) c'(u))-r sin(u),		d2y/dv2 = -w c(u) sin(u) cos(v) ,		d2y/dw2 = 0
		//d2z/du2 = w sin(v) c''(u),									d2z/dv2 = -w c(u) sin(v),						d2z/dw2 = 0
		//c''(u) = -(c (f(u) (1-f(u)^2) f''(u)+f'(u)^2))/(1-f(u)^2)^(3/2)
		//CapMin: f''(u) = -((k-1) k ((Umax-u)/tol)^(k-2))/tol^2
		//CapMax: f''(u) = ((k-1) k ((Umax-u)/tol)^(k-2))/tol^2

		double t;
		if (status == vtkParametricFunction::CapMin) {
			t = u - this->MinimumU;				
		}
		else {
			//status == vtkParametricFunction::CapMax)
			t = this->MaximumU - u;				
		}

		double k = 2.0;	//WHEN CHANGED, CHANGE ALSO Derivations!! If k ==1 the following code will not work!!
		double pw = pow(t / tol, k);
		double f = pw - 1;	//f(u)
		double df = (k / tol) * pow(t / tol, k - 1);					//f'(u)
		double d2f = (k / (tol*tol))*(k - 1) * pow(t / tol, k - 2);		//f''(u)

		if (status == vtkParametricFunction::CapMax) {
			df = -df; d2f = -d2f;
		}
		
		double sroot = sqrt(1 - f*f);							//this is actually sin(acos(f(u)));
		double c = this->CrossRadius * sroot;					//c(u)
		double dc = -(this->CrossRadius* f* df) / sroot;		//c'(u)
		double d2c = -(c*f*(1-f*f)*d2f + df*df) / 
			(sroot*sroot*sroot);								//c''(u) = -(c (f(u) (1-f(u)^2) f''(u)+f'(u)^2))/(1-f(u)^2)^(3/2)
		
		Du[0] = w*cv*(cu*(d2c - c) - 2*su*dc) - this->Radius*cu;	//w cos(v) (cos(u) (c''(u)-c(u))-2 sin(u) c'(u))-r cos(u)
		Du[1] = w*cv*(su*(d2c - c) + 2*cu*dc) - this->Radius*su;	//w cos(v) (sin(u) (c''(u)-c(u))+2 cos(u) c'(u))-r sin(u)
		Du[2] = w*sv*d2c;											//w sin(v) c''(u)

		Dv[0] = -w*c*cu*cv;										//-w c(u) cos(u) cos(v)
		Dv[1] = -w*c*su*cv;										//-w c(u) sin(u) cos(v)
		Dv[2] = -w*c*sv;										//-w c(u) sin(v)	
	}	
 }

//----------------------------------------------------------------------------
double vtkParametricTorus::EvaluateScalar(double* vtkNotUsed(uv[3]),
										  double* vtkNotUsed(Pt[3]),
										  double* vtkNotUsed(Duv[9]))
{
	return 0;
}

//----------------------------------------------------------------------------
void vtkParametricTorus::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "Radius: " << this->Radius << "\n";
	os << indent << "Cross Radius: " << this->CrossRadius << "\n";
}
#endif