/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParametricFunction.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notice for more information.

	Modified by: Josef Kohout to make it work with VTK 4.2 and to incorporate additional functionality

=========================================================================*/
#include "vtkParametricFunction.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "vtkMath.h"


//----------------------------------------------------------------------------
vtkParametricFunction::vtkParametricFunction() :
	MinimumU(0.0)
  , MaximumU(1.0)
  , MinimumV(0.0)
  , MaximumV(1.0)
  , MinimumW(0.0)
  , MaximumW(1.0)
  , JoinU(0)
  , JoinV(0)
  , JoinW(0)
  , TwistU(0)
  , TwistV(0)
  , TwistW(0)
  , ClockwiseOrdering(0)
  , DerivativesAvailable(1)
  , CapPortion(0.2)
  , CapPortionAvailable(0)
  , OnionLayers(0)
{
}


//----------------------------------------------------------------------------
vtkParametricFunction::~vtkParametricFunction()
{
}

//Determines the part of the function according to the given parameter u.
//Returns tolerance (tol) that could be used for determining the boundaries of different parts:
//CapMin = <MinumumU, MinumumU + tol)
//RegularPart = <MinumumU + tol, MaxiumU - tol>
//CapMax = (MaxiumU, MaxiumU - tol>
vtkParametricFunction::ParametricFunctionPart vtkParametricFunction::GetParametricFunctionPart(double u, double& tol)
{	
	if (this->CapPortionAvailable == 0) {
		tol = 0.0; 
	}
	else	
	{	
		tol = (this->MaximumU - this->MinimumU) * this->CapPortion;

		if (u < this->MinimumU + tol)	//first part
			return vtkParametricFunction::CapMin;
		else if (u > this->MaximumU - tol) //last part
			return vtkParametricFunction::CapMax;
	}

	return vtkParametricFunction::RegularPart;
}

//Solves the following equations:
//I.	x = c*cos(u)
//II.	y = c*sin(u) , where c is common constant
//returning u <0 .. 2*PI> 
double vtkParametricFunction::SolveGoniometricAngle(double x, double y)
{	
	double u;

	//y / x = tan(u), if x != 0 and cos(u) != 0 but cos(u) = 0 <==> x = 0
	if (IsZero(x))
	{
		//cos(u) = 0 => u = pi/2 or 3/2pi
		u = vtkMath::DoublePi() / 2;
		if (y < 0)	//sin(3/2pi) < 0
			u *= 3;
	}
	else
	{
		u = atan(y / x);	//atan return -PI/2 to PI/2
		if (u > 0.0)
		{
			//1st or 3rd quadrant
			if (x < 0)
				u += vtkMath::DoublePi();	//3rd quadrant
		}
		else
		{
			//2nd or 4th quadrant		
			if (x < 0.0)					
				u += vtkMath::DoublePi(); //2nd quadrant
			else												
				u += 2*vtkMath::DoublePi();	//4th quadrant				
		}					
	}

	return u;
}

//----------------------------------------------------------------------------
void vtkParametricFunction::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Minimum U: " << this->MinimumU << "\n";
  os << indent << "Maximum U: " << this->MaximumU << "\n";

  os << indent << "Minimum V: " << this->MinimumV << "\n";
  os << indent << "Maximum V: " << this->MaximumV << "\n";

  os << indent << "Minimum W: " << this->MinimumW << "\n";
  os << indent << "Maximum W: " << this->MaximumW << "\n";

  os << indent << "JoinU: " << this->JoinU << "\n";
  os << indent << "JoinV: " << this->JoinV << "\n";
  os << indent << "JoinW: " << this->JoinV << "\n";

  os << indent << "TwistU: " << this->TwistU << "\n";
  os << indent << "TwistV: " << this->TwistV << "\n";
  os << indent << "TwistW: " << this->TwistV << "\n";

  os << indent << "ClockwiseOrdering: " << this->ClockwiseOrdering << "\n";
  os << indent << "Derivatives Available: " << this->DerivativesAvailable << "\n";

  os << indent << "CapPortion Available: " << this->CapPortionAvailable << "\n";
  os << indent << "CapPortion: " << this->CapPortion << "\n";
}
#endif