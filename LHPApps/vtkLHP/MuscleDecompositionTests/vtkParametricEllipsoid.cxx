/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkParametricEllipsoid.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkParametricEllipsoid.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE

#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include <math.h>

vtkStandardNewMacro(vtkParametricEllipsoid);

//----------------------------------------------------------------------------
vtkParametricEllipsoid::vtkParametricEllipsoid() :
	XRadius(1)
	, YRadius(1)
	, ZRadius(1)
{
	// Preset triangulation parameters
	this->MinimumV = 0;
	this->MinimumU = 0;
	this->MaximumV = 2.0 * vtkMath::Pi();
	this->MaximumU = vtkMath::Pi();

	this->JoinU = 0;
	this->JoinV = 1;
	this->TwistU = 0;
	this->TwistV = 0;
	this->ClockwiseOrdering = 1;
	this->DerivativesAvailable = 1;
}

//----------------------------------------------------------------------------
vtkParametricEllipsoid::~vtkParametricEllipsoid()
{
}

//----------------------------------------------------------------------------
void vtkParametricEllipsoid::Evaluate(double uvw[3], double Pt[3])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;  	

	double cu = cos(u);
	double su = sin(u);
	double cv = cos(v);
	double sv = sin(v);

	// The point
	Pt[0] = this->XRadius*w*su*cv;	//
	Pt[1] = this->YRadius*w*su*sv;	//
	Pt[2] = this->ZRadius*cu;		//
}

/*virtual*/ bool vtkParametricEllipsoid::EvaluateInverse(double Pt[3], double uvw[3])
{
	if (!IsZero(Pt[0]))
	{
		//x != 0, as SolveGoniometricAngle uses arctg, we may simply use:
		uvw[0] = acos(Pt[2] / this->ZRadius);
		uvw[1] = SolveGoniometricAngle(Pt[0] / this->XRadius, Pt[1] / this->YRadius);		
		uvw[2] = Pt[0] / (this->XRadius * sin(uvw[0]) * cos(uvw[1]));
	}
	else
	{
		uvw[0] = acos(Pt[2] / this->ZRadius);
		if (!IsZero(Pt[1]))
		{
			//x = 0, y != 0 => sin(v) != 0 and w != 0 => cos(u) = 0 => u  = PI/2 or 3/2 PI
			double ysv = Pt[1] / sin(uvw[0]);
			if (ysv > 0) {
				uvw[1] = vtkMath::DoublePi() / 2; uvw[2] = ysv / this->YRadius;
			} 
			else {
				uvw[1] = 3*vtkMath::DoublePi() / 2; uvw[2] = -ysv / this->YRadius;
			}
		}
		else
		{
			//x = 0, y = 0 => w = 0 or sin(v) = 0
			if (IsZero(uvw[0]) || IsZero(uvw[0] - vtkMath::DoublePi())) {				
				uvw[2] = 0.5;	//sin(v) = 0 => w may by anything
			} 
			else {
				uvw[2] = 0.0;
			}

			uvw[1] = (this->MaximumV + this->MinimumV) / 2;
		}
	}

	if (uvw[1] < this->MinimumV || uvw[1] > this->MaximumV)
		return false;
	
	if (uvw[0] < this->MinimumU || uvw[0] > this->MaximumU)
		return false;

	if (uvw[2] < 0.0 || uvw[2] > 1.0)
		return false;

	return true;
}

//Computes partial derivatives at the point f(u,v,w).
/*virtual*/ void vtkParametricEllipsoid::EvaluateDerivative(double uvw[3], double Duvw[9])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;  

	double *Du = Duvw;
	double *Dv = Duvw + 3;
	double *Dw = Duvw + 6;

	double cu = cos(u);
	double su = sin(u);
	double cv = cos(v);
	double sv = sin(v);

	//The derivatives are:
	Du[0] = this->XRadius*w*cu*cv;
	Du[1] = this->YRadius*w*cu*sv;
	Du[2] = -this->ZRadius*su;

	Dv[0] = -this->XRadius*w*cu*sv;
	Dv[1] = this->YRadius*w*su*cv;
	Dv[2] = 0;

	Dw[0] = this->XRadius*cv*su;
	Dw[1] = this->XRadius*sv*su;
	Dw[2] = 0;
}

//Computes partial derivatives of 2nd order at the point f(u,v,w).
/*virtual*/ void vtkParametricEllipsoid::EvaluateDerivative2(double uvw[3], double Duvw[9])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;  

	double *Du = Duvw;
	double *Dv = Duvw + 3;
	double *Dw = Duvw + 6;

	double cu = cos(u);
	double su = sin(u);
	double cv = cos(v);
	double sv = sin(v);

	//The derivatives are:
	Du[0] = -this->XRadius*w*su*cv;
	Du[1] = -this->YRadius*w*su*sv;
	Du[2] = -this->ZRadius*cu;

	Dv[0] = -this->XRadius*w*cu*cv;
	Dv[1] = -this->YRadius*w*su*sv;
	Dv[2] = 0;

	Dw[0] = 0;
	Dw[1] = 0;
	Dw[2] = 0;
}

//----------------------------------------------------------------------------
double vtkParametricEllipsoid::EvaluateScalar(double*, double*, double*)
{
	return 0;
}

//----------------------------------------------------------------------------
void vtkParametricEllipsoid::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "X scale factor: " << this->XRadius << "\n";
	os << indent << "Y scale factor: " << this->YRadius << "\n";
	os << indent << "Z scale factor: " << this->ZRadius << "\n";
}

#endif