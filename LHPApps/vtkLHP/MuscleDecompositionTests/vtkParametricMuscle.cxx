/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkParametricMuscle.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkParametricMuscle.h"
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "vtkObjectFactory.h"
#include "vtkMath.h"

vtkStandardNewMacro(vtkParametricMuscle);

//----------------------------------------------------------------------------
vtkParametricMuscle::vtkParametricMuscle() :
	RadiusX(4.0), RadiusY(10.0), 
	CrossRadiusX1(1.0), CrossRadiusZ1(0.5), 
	CrossRadiusX2(2.5), CrossRadiusZ2(1.0)
{
	this->MinimumU = 0;
	this->MinimumV = 0;
	this->MaximumU = vtkMath::DoublePi() / 2;	//DO NOT CHANGE! Unless you wish to modify Inverse (10 hours of work to derive everything)
	this->MaximumV = 2 * vtkMath::DoublePi();

	this->JoinU = 0;
	this->JoinV = 1;
	this->TwistU = 0;
	this->TwistV = 0;
	this->ClockwiseOrdering = 1;
	this->DerivativesAvailable = 1;	
}

//----------------------------------------------------------------------------
vtkParametricMuscle::~vtkParametricMuscle()
{
}

#include <math.h>

//Calculates b(u)
//Performs linear interpolation between minVal and maxVal according to u, status and tol.
//If status is Unknown, it is automatically determined from u value
double vtkParametricMuscle::LerpAB(double u, ParametricFunctionPart status, double tol, double minVal, double maxVal)
{
	if (status == vtkParametricFunction::Unknown)
		status = GetParametricFunctionPart(u, tol);
	
	double t1, t2;	//for blending between CrossRadiusX1, CrossRadiusX2 and CrossRadiusZ1 and CrossRadiusZ2 
	switch (status)
	{
	case vtkParametricFunction::CapMin:
		t1 = (u - this->MinimumU) / tol; t2 = 0.0; break;
	case vtkParametricFunction::CapMax:
		t1 = 0.0; t2 = (this->MaximumU - u) / tol; break;
	case vtkParametricFunction::RegularPart:
		t2 = (u - this->MinimumU - tol) / (this->MaximumU - this->MinimumU - 2*tol); 
		t1 = 1.0 - t2; break;	
	}

	return t1*minVal + t2*maxVal;
}

//----------------------------------------------------------------------------
void vtkParametricMuscle::Evaluate(double uvw[3], double Pt[3])
{
	double u = uvw[0];
	double v = uvw[1];
	double w = this->OnionLayers != 0 ? uvw[2] : 1.0;

	//x = (Rx + a(u)*w*cos(v))*cos(u)
	//y = (Ry + a(u)*w*cos(v))*sin(u)
	//z = b(u)*w*sin(v)

	double cv = cos(v);
	double sv = sin(v);
	double cu = cos(u);
	double su = sin(u);

	double tol;	//tolerance for detecting caps
	ParametricFunctionPart status = GetParametricFunctionPart(u, tol);
	
	double a = GetAu(u, status, tol);
	double b = GetBu(u, status, tol);

	// The point
	Pt[0] = (this->RadiusX + a * w * cv) * cu;
	Pt[1] = (this->RadiusY + a * w * cv) * su;
	Pt[2] = b * w* sv;
}


#define MAXIMIZE_MIN(st, val)	{ double tmp = val; if (st[0] < tmp) { st[0] = tmp; retStatus = true; } }
#define MINIMIZE_MAX(st, val)	{ double tmp = val; if (st[1] > tmp) { st[1] = tmp; retStatus = true; } }
#define UPDATE_MINMAX(st, val)	{ double tmp = val; if (tmp < st[0]) { st[0] = tmp; } if (tmp > st[1]) { st[1] = tmp; }}
#define SWAP_MINMAX_IFNEEDED(st) { if (st[0] > st[1]) { double tmp = st[0]; st[0] = st[1]; st[1] = tmp; }}

//Rx + a(u)*w*cos(v) = 0 and z = b(u)*w*sin(v), if xNotNull == false, or
//Ry + a(u)*w*cos(v) = 0 and z = b(u)*w*sin(v), if xNotNull == true
bool vtkParametricMuscle::FindEvaluateInverseIntervalsCase2(const double Pt[3], double uMinMax[2], double vMinMax[2], double wMinMax[2], bool xNotNull)
{
	bool retStatus = false;
	MAXIMIZE_MIN(vMinMax, vtkMath::DoublePi() / 2);
	MINIMIZE_MAX(vMinMax, 3*vtkMath::DoublePi() / 2);

	double r = xNotNull ? this->RadiusY : this->RadiusX;

	//-z/r = b(u)/a(u)*tg(v) => -z/r*a(u)/b(u) = tg(v) =>
	//v = arctg(-z/r*a(u)/b(u));
	//Function a(u)/b(u) is monotonous (and positive) on the whole interval u, so extremes are from uMin, uMax
	double abMinMax[2];

	double tol;
	ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
	abMinMax[0] = GetAu(uMinMax[0], status, tol) / GetBu(uMinMax[0], status, tol);

	status = GetParametricFunctionPart(uMinMax[1], tol);
	abMinMax[1] = GetAu(uMinMax[1], status, tol) / GetBu(uMinMax[1], status, tol);
	SWAP_MINMAX_IFNEEDED(abMinMax);
	
	double vNewMinMax[2];
	double coef = -Pt[2]/r;
	//coef is positive, if z is negative, which is when sin(v) < 0  => v is <PI, 3/2 PI> III. quadrant [we have updated the interval according to this above]
	//coef is negative, if z is positive, which is when sin(v) > 0 => v is <PI/2 to PI>	 II. quadrant		
	//as arctg is also monotonous function (from -PI/2 to PI/2), we may now calculate boundaries
	//1) if coef is negative, the minimal value is coef*abMinMax[1] and maximal is coef*abMinMax[0]
	//and atan will return some negative value from -PI/2 to 0, so we need to add PI to II. quadrant
	//2) if coef is positive, the minimal value is coef*abMinMax[0] and maximal is coef*abMinMax[1]
	//and atan will return some positive value from 0 to PI/2, so we need to add PI to get to III. quadrant			
	if (coef < 0.0) {
		vNewMinMax[0] = atan(coef * abMinMax[1]); vNewMinMax[1] = atan(coef * abMinMax[0]);
	} else {
		vNewMinMax[0] = atan(coef * abMinMax[0]); vNewMinMax[1] = atan(coef * abMinMax[1]);
	}

	vNewMinMax[0] += vtkMath::DoublePi(); vNewMinMax[1] += vtkMath::DoublePi();
	MAXIMIZE_MIN(vMinMax, vNewMinMax[0]);
	MINIMIZE_MAX(vMinMax, vNewMinMax[1]);

	//z = b(u)*w*sin(v) and we know that we are in either II. or III. quadrant			
	double sinV[2] = { sin(vMinMax[0]), sin(vMinMax[1]) };
	//1) if z > 0, i.e., sin(v) > 0 => II. quadrant then sinV[0] > sinV[1]
	//w = z / (b(u)*sin(v)) => w_min is when b(u)*sin(v) is maximal, which is when b(u) is max and sin(v) = sinV[0]
	//b(u) is maximal for MAX(B1, B2), w_max is when b(u)*sin(v) is minimal, which is when b(u) is min and sin(v) = sinV[1]
	//b(u) is minimal for MAX(B1, B2)
	//2) if z < 0, i.e., sin(v) < 0 => III. quadrant, then sinV are negative and sinV[0] < sinV[1], as z is negative, however,
	//we are getting sinV[0]*(-1) > sinV[1]*(-1), which is the same case as in the first option
	double bMinMax[2];
	if (this->CrossRadiusZ1 < this->CrossRadiusZ2) {
		bMinMax[0] = this->CrossRadiusZ1; bMinMax[1] = this->CrossRadiusZ2; 
	}
	else {
		bMinMax[0] = this->CrossRadiusZ2; bMinMax[1] = this->CrossRadiusZ1; 
	}

	MAXIMIZE_MIN(wMinMax, Pt[2] / (sinV[0] * bMinMax[1]));
	MINIMIZE_MAX(wMinMax, Pt[2] / (sinV[1] * bMinMax[0]));
	return retStatus;
}

/** x = (Rx + a(u)*w*cos(v))*cos(u),  Ry + a(u)*w*cos(v) = 0, and z = b(u)*w*sin(v) - xNotNull == true, or
 (Rx + a(u)*w*cos(v))*cos(u) = 0,  y = (Ry + a(u)*w*cos(v))*sin(u) = 0, and z = b(u)*w*sin(v) - xNotNull == false*/
bool vtkParametricMuscle::FindEvaluateInverseIntervalsCase6(const double Pt[3], double uMinMax[2], double vMinMax[2], double wMinMax[2], bool xNotNull)
{
	bool retStatus = false;

	//Rx + a(u)*w*cos(v) = 0			or		Ry + a(u)*w*cos(v) = 0
	//y = (Ry + a(u)*w*cos(v))*sin(u)		or		x = (Rx + a(u)*w*cos(v))*cos(u)
	//z = b(u)*w*sin(v)				
	//and cos(v) must be negative, since everything else is positive, i.e., v must be PI/2 to 3/2 PI
	bool oldRetStauts = FindEvaluateInverseIntervalsCase2(Pt, uMinMax, vMinMax, wMinMax, xNotNull);

	//1) y = (Ry + a(u)*w*cos(v))*sin(u)  =>
	//u = arcsin(y / (Ry + a(u)*w*cos(v)))
	//arcsin is monotonous function for inputs from -1 to 1 resulting in -PI/2 to PI/2
	//but since u must be from 0 to PI/2, it must be that y / (Ry + a(u)*w*cos(v) is from <0, 1>	
	//2) x = (Rx + a(u)*w*cos(v))*cos(u)  =>
	//u = arccos(x / (Rx + a(u)*w*cos(v)))
	//arccos is monotonous function for inputs from -1 to 1 resulting in PI to 0
	//but since u must be from 0 to PI/2, it must be that x / (Rx + a(u)*w*cos(v) is from <0, 1>

	//Function F(u,v,w) = y / (Ry + a(u)*w*cos(v)) or x / (Rx + a(u)*w*cos(v)) has absolute extremes in stationary points or at boundaries
	//stationary points: dF(u,v,w)/du = 0, dF(u,v,w)/dv = 0, and dF(u,v,w)/dw = 0, but these points are difficult to calculate
	//However, it is clear that maximum happens when G(u,v,w) = (Ry + a(u)*w*cos(v))  or (Rx + a(u)*w*cos(v)) is minimal and vice versa	
	//Stationary points for G(u,v,w): are for w = 0 and cos(v) = 0, i.e., v = PI/2 or 3/2PI				
	double r = xNotNull ? this->RadiusX : this->RadiusY;
	double GMinMax[2] = { DBL_MAX, -DBL_MAX};
	//check, if these stationary points are in the valid interval
	if (IsZero(wMinMax[0]))
	{
		//we don't need to check v as later it is found that v may be anything and the result is the same

		//if ((vMinMax[0] <= vtkMath::DoublePi() / 2 && vMinMax[1] >= vtkMath::DoublePi() / 2) ||
		//	(vMinMax[0] <= 3*vtkMath::DoublePi() / 2 && vMinMax[1] >= 3* vtkMath::DoublePi() / 2))
		GMinMax[0] = GMinMax[1] = r;	//if yes, the extremes are the same: this->RadiusX or this->RadiusY
	}

	//Now we must check boundaries of G(u,v,w), i.e., get absolute extremes for H(v, w; u = uMin)
	//Ry + a(uMin)*w*cos(v) => dH/dv = -a(umin)*w*sin(v) = 0, dH/dw = a(umin)*cosv = 0
	//stationary points are w = 0 and v = PI/2 or 3/2PI, which we already had done
	//boundary of H(v,w) is extreme for I(w; u = uMin, v = vMin) => dI/dw = a(umin)*cos(vmin) = 0 => no stationary points
	//but we have also need to check w = wMin, wMax				
	double aMinMax[2] = { GetAu(uMinMax[0]), GetAu(uMinMax[1]) };
	double cosVMinMax[2] = { cos(vMinMax[0]), cos(vMinMax[1]) };

	UPDATE_MINMAX(GMinMax, r + aMinMax[0]*wMinMax[0]*cosVMinMax[0]);
	UPDATE_MINMAX(GMinMax, r + aMinMax[0]*wMinMax[1]*cosVMinMax[0]);
	//Now I(w; u = uMin, v = vMax)
	UPDATE_MINMAX(GMinMax, r + aMinMax[0]*wMinMax[0]*cosVMinMax[1]);
	UPDATE_MINMAX(GMinMax, r + aMinMax[0]*wMinMax[1]*cosVMinMax[1]);

	//H(v, w; u = uMax)
	UPDATE_MINMAX(GMinMax, r + aMinMax[1]*wMinMax[0]*cosVMinMax[0]);
	UPDATE_MINMAX(GMinMax, r + aMinMax[1]*wMinMax[1]*cosVMinMax[0]);				
	UPDATE_MINMAX(GMinMax, r + aMinMax[1]*wMinMax[0]*cosVMinMax[1]);
	UPDATE_MINMAX(GMinMax, r + aMinMax[1]*wMinMax[1]*cosVMinMax[1]);

	//Now H(u, w; v = vMin) => stationary point does not exist
	//I(u; v = vMin, w = vMin/vMax) => no stationary point and boundaries already checked
	//Now H(u, w; v = vMax) => stationary point does not exist
	//I(u; v = vMax, w = vMin/vMax) => no stationary point and boundaries already checked

	//Now H(u, v; w = wMin) => stationary point is (u, v, 0) if wMin is zero, no other exists
	//I(u; v = vMin/vMax, w = wMin) => no stationary point and boundaries already checked
	//Now H(u, v; w = wMax) => stationary point does not exist
	//I(u; v = vMin/vMax, w = wMax) => no stationary point and boundaries already checked

	//GminMax now contains extremes for G(u,v,w) function
	double FMinMax[2] = { Pt[1] / GMinMax[0], Pt[1] / GMinMax[1] };
	SWAP_MINMAX_IFNEEDED(FMinMax);

	//if MinMax do not have the same sign, the function G(u,v,w) crosses 0
	//So actually the extreme is +/-INF depending on sign of y = Pt[1]
	if (FMinMax[0] < 0)
	{
		FMinMax[0] = FMinMax[1]; 
		if (FMinMax[0] < 0.0)
			FMinMax[0] = 0.0;

		FMinMax[1] = 1.0;	
	}	

	if (FMinMax[1] > 1.0)
		FMinMax[1] = 1.0;

	double uNewMinMax[2];
	if (xNotNull) {
		uNewMinMax[1] = acos(FMinMax[0]), uNewMinMax[0] = acos(FMinMax[1]);
	}
	else { 
		uNewMinMax[0] = asin(FMinMax[0]), uNewMinMax[1] = asin(FMinMax[1]);
	}

	retStatus = oldRetStauts;

	//update intervals
	MAXIMIZE_MIN(uMinMax, uNewMinMax[0]);
	MINIMIZE_MAX(uMinMax, uNewMinMax[1]);
	return retStatus;
}

//Attempts to refine the intervals in which the solution u,v,w can be found. 
//Returns false, if the intervals could not be refined, true, otherwise.
bool vtkParametricMuscle::FindEvaluateInverseIntervals(const double Pt[3], double uMinMax[2], double vMinMax[2], 
													   double wMinMax[2], InverseIntervalsState& state)
{
	bool retStatus = false;

	//x = (Rx + a(u)*w*cos(v))*cos(u)
	//y = (Ry + a(u)*w*cos(v))*sin(u)
	//z = b(u)*w*sin(v)

	//a(u) > 0, b(u) > 0, u = <0, PI/2> => cos(u) >= 0 and sin(u) >= 0, v = <0, 2PI>, w = <0, 1>, Rx > 0, Ry > 0
	if (Pt[2] < 0)
	{
		//sin(v) < 0 => v = <PI, 2PI>
		MAXIMIZE_MIN(vMinMax, vtkMath::DoublePi());

		if (Pt[0] < 0 || Pt[1] < 0) {
			//cos(v) < 0
			MINIMIZE_MAX(vMinMax, 3*vtkMath::DoublePi() / 2);
		}		
	}
	else
	{
		//sin(v) > 0 => v = <0, PI>
		MINIMIZE_MAX(vMinMax, vtkMath::DoublePi());

		if (Pt[0] < 0 || Pt[1] < 0) {
			//cos(v) < 0
			MAXIMIZE_MIN(vMinMax, vtkMath::DoublePi() / 2);
		}
	}

	if (!IsZero(this->RadiusX - this->RadiusY))
	{
		if (!IsZero(Pt[0]))
		{
			//x != 0, Rx != Ry
			if (!IsZero(Pt[1]))
			{
				//x != 0, y != 0, Rx != Ry
				//the most general case (expected one)
				//y / x = (Ry + a(u)*w*cos(v))/(Rx + a(u)*w*cos(v))*tg(u) =>
				//u = atan( y / x * (Rx + a(u)*w*cos(v) / (Ry + a(u)*w*cos(v)))
				//as atan is monotonous function and u must be from 0 to PI/2
				//we have one constraint:  y / x * (Rx + a(u)*w*cos(v) / (Ry + a(u)*w*cos(v)) > 0
				//and with this constraint and over the current given intervals we need to find extremes for 
				//F(u,v,w) = y / x * (Rx + a(u)*w*cos(v) / (Ry + a(u)*w*cos(v)) =>
				//dF/du = a'(u)*w*cos(v) = 0, dF/dv = a(u)*w*sin(v), and dF/dw = a(u)*cos(v)
				double FMinMax[2] = { DBL_MAX, -DBL_MAX};
				double yr = Pt[1] / Pt[0];

				//stationary points for the function are, when w = 0 and cos(v) = 0:
				//(u, PI/2, 0), (u, 3/2PI, 0)
				if (IsZero(wMinMax[0]))
				{					
					if (
						(vMinMax[0] <= vtkMath::DoublePi() / 2 && vMinMax[1] >= vtkMath::DoublePi() / 2) ||
						(vMinMax[0] <= 3*vtkMath::DoublePi() / 2 && vMinMax[1] >= 3*vtkMath::DoublePi() / 2)
					)
						FMinMax[0] = FMinMax[1] = yr*this->RadiusX / this->RadiusY;	//cos(v) = 0
				}
				
				double aMinMax[2] = { GetAu(uMinMax[0]), GetAu(uMinMax[1]) };
				double cosVMinMax[2] = { cos(vMinMax[0]), cos(vMinMax[1]) };

				//Now we must check boundaries of F(u,v,w), i.e., get absolute extremes for six functions:
				//1) G(v, w; u = uMin, uMax), 2) G(u, w; v = vMin, vMax), and 3) G(u, v; w = wMin/wMax)
				//stationary points of these functions are:
				//For 1) when cosv = 0 and w = 0 => already checked above
				//For 2) do not exist
				//For 3) do not exist
				
				//For each of the functions G, boundaries must be also checked, i.e., we need to get absolute extremes for 24 functions:
				//1.1) H(v; w = wMin, wMax, u = uMin, uMax)
				//1.2) H(w; v = vMin, vMax, u = uMin, uMax)
				//2.1) H(u; w = wMin, wMax, v = vMin, vMax)
				//2.2) H(w; u = uMin, uMax, v = vMin, vMax)	--- but these are 1.2
				//3.1) H(u; v = vMin, vMax, w = wMin, wMax)	--- but these are 2.1
				//3.2) H(v; u = uMin, uMax, w = wMin, wMax)	--- but these are 1.1
				//Stationary points of these function are:
				//For 1.1) when sin(v) = 0, i.e., v = 0, PI, 2PI 
				//=> (uMin, 0, wMin), (uMin, 0, wMax), (uMax, 0, wMin), (uMax, 0, wMax),
				//	(uMin, PI, wMin), (uMin, PI, wMax), (uMax, PI, wMin), (uMax, PI, wMax),
				//	(uMin, 2*PI, wMin), (uMin, 2*PI, wMax), (uMax, 2*PI, wMin), (uMax, 2*PI, wMax),
				//For 1.2) do not exist
				//For 2.1) do not exist

				//For each of the functions H boundaries must be also checked and these are combinations of uMin/uMax,vMin/vMax,wMin/wMax

				//Start with 1.1
				if (vMinMax[0] <= 0.0 || vMinMax[1] >= 2*vtkMath::DoublePi()) 
				{	//cos(v) = 1	
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[0]) / (this->RadiusY + aMinMax[0]*wMinMax[0]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[1]) / (this->RadiusY + aMinMax[0]*wMinMax[1]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[0]) / (this->RadiusY + aMinMax[1]*wMinMax[0]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[1]) / (this->RadiusY + aMinMax[1]*wMinMax[1]));
				}

				if (vMinMax[0] <= vtkMath::DoublePi() && vMinMax[1] >= vtkMath::DoublePi()) 
				{	//cos(v) = -1	
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX - aMinMax[0]*wMinMax[0]) / (this->RadiusY - aMinMax[0]*wMinMax[0]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX - aMinMax[0]*wMinMax[1]) / (this->RadiusY - aMinMax[0]*wMinMax[1]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX - aMinMax[1]*wMinMax[0]) / (this->RadiusY - aMinMax[1]*wMinMax[0]));
					UPDATE_MINMAX(FMinMax, yr * (this->RadiusX - aMinMax[1]*wMinMax[1]) / (this->RadiusY - aMinMax[1]*wMinMax[1]));
				}

				//Boundaries of H
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[0]*cosVMinMax[0]) / (this->RadiusY + aMinMax[0]*wMinMax[0]*cosVMinMax[0]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[0]*cosVMinMax[1]) / (this->RadiusY + aMinMax[0]*wMinMax[0]*cosVMinMax[1]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[1]*cosVMinMax[0]) / (this->RadiusY + aMinMax[0]*wMinMax[1]*cosVMinMax[0]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[0]*wMinMax[1]*cosVMinMax[1]) / (this->RadiusY + aMinMax[0]*wMinMax[1]*cosVMinMax[1]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[0]*cosVMinMax[0]) / (this->RadiusY + aMinMax[1]*wMinMax[0]*cosVMinMax[0]));				
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[0]*cosVMinMax[1]) / (this->RadiusY + aMinMax[1]*wMinMax[0]*cosVMinMax[1]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[1]*cosVMinMax[0]) / (this->RadiusY + aMinMax[1]*wMinMax[1]*cosVMinMax[0]));
				UPDATE_MINMAX(FMinMax, yr * (this->RadiusX + aMinMax[1]*wMinMax[1]*cosVMinMax[1]) / (this->RadiusY + aMinMax[1]*wMinMax[1]*cosVMinMax[1]));

				//FminMax now contains extremes for F(u,v,w) function
				double uNewMinMax[2] = { atan(FMinMax[0]), atan(FMinMax[1]) };
				MAXIMIZE_MIN(uMinMax, uNewMinMax[0]);
				MINIMIZE_MAX(uMinMax, uNewMinMax[1]);

				//as cos is monotonous decreasing function and cos(u) >= 0 =>
				//x/cos(uMin) - Rx <=  x/cos(u) - Rx  = a(u)*w*cos(v) <= x/cos(uMax) - Rx, z = b(u)*w*sin(v) 
				//if z > 0, then  z/(x/cos(uMin) - Rx) <= b(u)/a(u)*tg(v) <= z/(x/cos(uMax) - Rx) =>
				//a(u)/b(u)*z/(x/(cos(uMin) - Rx)) <= tg(v) <= a(u)/b(u)*z/(x/(cos(uMax) - Rx))
				//otherwise, a(u)/b(u)*z/(x/(cos(uMin) - Rx)) >= tg(v) >= a(u)/b(u)*z/(x/(cos(uMax) - Rx))
				
				//Function a(u)/b(u) is monotonous (and positive) on the whole interval u, so extremes are from uMin, uMax
				double abMinMax[2], tol;	
				ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
				abMinMax[0] = GetAu(uMinMax[0], status, tol) / GetBu(uMinMax[0], status, tol);

				status = GetParametricFunctionPart(uMinMax[1], tol);
				abMinMax[1] = GetAu(uMinMax[1], status, tol) / GetBu(uMinMax[1], status, tol);
				SWAP_MINMAX_IFNEEDED(abMinMax);

				//if z > 0, then MIN(a(u)/b(u))*z/(x/(cos(uMin) - Rx)) <= tg(v) <= MAX(a(u)/b(u))*z/(x/(cos(uMax) - Rx)),
				//else MAX(a(u)/b(u))*z/(x/(cos(uMin) - Rx)) >= tg(v) >= MIN(a(u)/b(u))*z/(x/(cos(uMax) - Rx)),

				double xcosMinMax[2] = { Pt[0] / cos(uMinMax[0]) - this->RadiusX,  Pt[0] / cos(uMinMax[1]) - this->RadiusX, };													
				double coef[2] = { Pt[2] / xcosMinMax[0], Pt[2] / xcosMinMax[1] };

				//cos(u) is always positive, z*cos(u)/(x - Rx*cos(u)) is negative, if z < 0 and x > Rx*cos(u), or z > 0 and x < Rx*cos(u)				
				double vNewMinMax[2];
				if (Pt[2] >= 0.0) {
					vNewMinMax[0] = atan(coef[0] * abMinMax[0]); vNewMinMax[1] = atan(coef[1] * abMinMax[1]);
				} else {
					vNewMinMax[0] = atan(coef[1] * abMinMax[0]); vNewMinMax[1] = atan(coef[0] * abMinMax[1]);
				}

				if (vNewMinMax[0] >= 0.0 && vNewMinMax[1] >= 0.0)
				{
					//tg(v) is positive for I. or III. quadrant
					if (Pt[2] < 0.0) { //III. quadrant
						vNewMinMax[0] += vtkMath::DoublePi(); vNewMinMax[1] += vtkMath::DoublePi();
					}
				}
				else if (vNewMinMax[0] <= 0.0 && vNewMinMax[1] <= 0.0)
				{
					//tg(v) is negative for II. or IV. quadrant
					if (Pt[2] < 0.0) { //IV. quadrant
						vNewMinMax[0] += 2*vtkMath::DoublePi(); vNewMinMax[1] += 2*vtkMath::DoublePi();
					} else {
						vNewMinMax[0] += vtkMath::DoublePi(); vNewMinMax[1] += vtkMath::DoublePi();
					}
				}
				else
				{
					//Pt[0] must be > 0 since cos(u) influences the sign of tg(v), i.e., Pt[2] determines quadrants
					//possible quadrants are I. and II. or III. and IV.
					SWAP_MINMAX_IFNEEDED(vNewMinMax);	//negative will be the first
					if (Pt[2] >= 0.0) {  //I. and II. quadrant
						vNewMinMax[0] += vtkMath::DoublePi();
					}
					else { //III. and IV. quadrant
						vNewMinMax[0] += 2*vtkMath::DoublePi(); vNewMinMax[1] += vtkMath::DoublePi();
					}

					SWAP_MINMAX_IFNEEDED(vNewMinMax);	//order them
				}
	
				MAXIMIZE_MIN(vMinMax, vNewMinMax[0]);
				MINIMIZE_MAX(vMinMax, vNewMinMax[1]);

				//z = b(u)*w*sin(v) but we don't know quadrants in which sin(v) is, so we need to find extremes of sin over the interval
				double sinVMinMax[2] = { DBL_MAX, -DBL_MAX };
				UPDATE_MINMAX(sinVMinMax, sin(vMinMax[0]));
				UPDATE_MINMAX(sinVMinMax, sin(vMinMax[1]));

				if (vMinMax[0] <= 0 || vMinMax[1] >= 2*vtkMath::DoublePi()) {
					UPDATE_MINMAX(sinVMinMax, 0.0);
				}

				if (vMinMax[0] <= vtkMath::DoublePi() / 2 && vMinMax[1] >= vtkMath::DoublePi() / 2) {
					UPDATE_MINMAX(sinVMinMax, 1.0);
				}

				if (vMinMax[0] <= 3*vtkMath::DoublePi() / 2 && vMinMax[1] >= 3*vtkMath::DoublePi() / 2) {
					UPDATE_MINMAX(sinVMinMax, -1.0);
				}


				//1) if z > 0, i.e., sin(v) > 0 => sinVMinMax[1] > sinVMinMax[0]  > 0
				//w = z / (b(u)*sin(v)) => w_min is when b(u)*sin(v) is maximal, which is when b(u) is max and sin(v) = sinV[1]
				//b(u) is maximal for MAX(B1, B2), w_max is when b(u)*sin(v) is minimal, which is when b(u) is min and sin(v) = sinV[0]
				//b(u) is minimal for MIN(B1, B2)
				//2) if z < 0, i.e., sin(v) < 0 => sinVMinMax[0] < sinVMinMax[1] < 0, and we are getting sinV[0]*(-1) > sinV[1]*(-1), which is the same case as in the first option

				double bMinMax[2];
				if (this->CrossRadiusZ1 < this->CrossRadiusZ2) {
					bMinMax[0] = this->CrossRadiusZ1; bMinMax[1] = this->CrossRadiusZ2; 
				}
				else {
					bMinMax[0] = this->CrossRadiusZ2; bMinMax[1] = this->CrossRadiusZ1; 
				}

				double wNewMinMax[2] = { Pt[2] / (sinVMinMax[1] * bMinMax[1]), Pt[2] / (sinVMinMax[0] * bMinMax[0]) };
				SWAP_MINMAX_IFNEEDED(wNewMinMax);

				MAXIMIZE_MIN(wMinMax, wNewMinMax[0]);
				MINIMIZE_MAX(wMinMax, wNewMinMax[1]);

				state = NumericSolutionNeededRestartNotAllowed;
			}
			else
			{
				//x != 0, Rx != Ry but y = 0 => Ry + a(u)*w*cos(v) = 0 or sin(u) = 0
				if (state != RestartedAttempt)
				{
					//we will first try the solution with sin(u) = 0, if solution is not valid, we will use the other one
					//sin(u) = 0 for u = 0 => cos(u) = 1
					//x= Rx + a(u)*w*cos(v) 
					//z = b(u)*w*sin(v)
					uMinMax[0] = uMinMax[1] = 0.0;

					double tol;
					ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
					double a = GetAu(uMinMax[0], status, tol);
					double b = GetAu(uMinMax[0], status, tol);

					vMinMax[0] = vMinMax[1] = SolveGoniometricAngle((Pt[0]-this->RadiusX) / a, Pt[2] / b);	
					wMinMax[0] = wMinMax[1] = Pt[2] / (b*sin(vMinMax[0]));
					state = AnalyticSolutionFoundRestartAllowed;
					return true;
				}
				else
				{
					//x = (Rx + a(u)*w*cos(v))*cos(u)
					//Ry + a(u)*w*cos(v) = 0
					//z = b(u)*w*sin(v)						
					retStatus = FindEvaluateInverseIntervalsCase6(Pt, uMinMax, vMinMax, wMinMax, true);
					state = NumericSolutionNeededRestartNotAllowed;
				}			
			}
		}
		else if (!IsZero(Pt[1]))
		{
			//x = 0 but y != 0 => either Rx + a(u)*w*cos(v) = 0 or cos(u) = 0
			if (state != RestartedAttempt)
			{
				//we will first try the solution with cos(u) = 0, if solution is not valid, we will use the other one
				//cos(u) = 0 for u = PI/2 => sin(u) = 1
				//y= Ry + a(u)*w*cos(v) 
				//z = b(u)*w*sin(v)

				uMinMax[0] = uMinMax[1] = vtkMath::DoublePi() / 2;

				double tol;
				ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
				double a = GetAu(uMinMax[0], status, tol);
				double b = GetAu(uMinMax[0], status, tol);

				vMinMax[0] = vMinMax[1] = SolveGoniometricAngle((Pt[1]-this->RadiusY) / a, Pt[2] / b);	
				wMinMax[0] = wMinMax[1] = Pt[2] / (b*sin(vMinMax[0]));
				state = AnalyticSolutionFoundRestartAllowed;
				return true;
			}
			else
			{
				//Rx + a(u)*w*cos(v) = 0
				//y = (Ry + a(u)*w*cos(v))*sin(u)
				//z = b(u)*w*sin(v)								
				retStatus = FindEvaluateInverseIntervalsCase6(Pt, uMinMax, vMinMax, wMinMax, false);				
				state = NumericSolutionNeededRestartNotAllowed;
			}
		}
		else
		{
			//x = 0 and y = 0, Rx != Ry => Rx + a(u)*w*cos(v) = 0 or Ry + a(u)*w*cos(v) = 0 and either cos(u) or sin(u) is 0
			if (state != RestartedAttempt)
			{
				//we will first try the solution with sin(u) = 0, if solution is not valid, we will use the other one
				//sin(u) = 0 for u = 0
				//Rx + a(u)*w*cos(v) = 0
				//z = b(u)*w*sin(v)

				uMinMax[0] = uMinMax[1] = 0.0;
			}
			else 
			{
				//cos(u) = 0 for u = PI/2
				//Ry + a(u)*w*cos(v) = 0
				//z = b(u)*w*sin(v)

				uMinMax[0] = uMinMax[1] = vtkMath::DoublePi() / 2;
			}

			double tol;
			ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
			double a = GetAu(uMinMax[0], status, tol);
			double b = GetAu(uMinMax[0], status, tol);

			if (state != RestartedAttempt)
			{
				//Rx + a(u)*w*cos(v) = 0
				vMinMax[0] = vMinMax[1] = SolveGoniometricAngle(-this->RadiusX / a, Pt[2] / b);	
				state = AnalyticSolutionFoundRestartAllowed;
			}
			else 
			{		
				//Ry + a(u)*w*cos(v) = 0
				vMinMax[0] = vMinMax[1] = SolveGoniometricAngle(-this->RadiusY / a, Pt[2] / b);	
				state = AnalyticSolutionFoundRestartNotAllowed;	
			}

			wMinMax[0] = wMinMax[1] = Pt[2] / (b*sin(vMinMax[0]));
			return true;
		}
	}
	else
	{
		//Rx = Ry =>

		//x = (Rx + a(u)*w*cos(v))*cos(u)
		//y = (Rx + a(u)*w*cos(v))*sin(u)
		//z = b(u)*w*sin(v)
		if (!IsZero(Pt[0]))
		{
			//x != 0			
			uMinMax[1] = uMinMax[0] = SolveGoniometricAngle(Pt[0],Pt[1]);

			double tol;
			ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
			double au = GetAu(uMinMax[0], status, tol);
			double bu = GetBu(uMinMax[0], status, tol);

			vMinMax[1] = vMinMax[0] = SolveGoniometricAngle(
				(Pt[0] / cos(uMinMax[0]) - this->RadiusX) / au,	Pt[2] / bu);
			wMinMax[0] = wMinMax[1] = Pt[2] / (bu * sin(vMinMax[0]));
			state = AnalyticSolutionFoundRestartNotAllowed;
			return true;	//we have analytic solution :-)
		}
		else if (!IsZero(Pt[1]))
		{					
			//x = 0 but y != 0, Rx = Ry
			//=> cos(u) = 0 => u is PI/2 and sin(u) = 1, hence
			//y = Ry + a(u)*w*cos(v)
			//z = b(u)*w*sin(v)
			uMinMax[0] = uMinMax[1] = vtkMath::DoublePi() / 2;

			double tol;
			ParametricFunctionPart status = GetParametricFunctionPart(uMinMax[0], tol);
			double a = GetAu(uMinMax[0], status, tol);
			double b = GetAu(uMinMax[0], status, tol);

			vMinMax[0] = vMinMax[1] = SolveGoniometricAngle((Pt[1] - this->RadiusY) / a, Pt[2] / b);
			wMinMax[0] = wMinMax[1] = Pt[2] / (b*sin(vMinMax[0]));
			state = AnalyticSolutionFoundRestartNotAllowed;
			return true;	//singular case, one analytical solution		
		}
		else
		{
			//x = 0 and y = 0 =>
			//-Rx = a(u)*w*cos(v)
			//z = b(u)*w*sin(v)
			//and cos(v) must be negative, since everything else is positive, i.e., v must be PI/2 to 3/2 PI
			retStatus = FindEvaluateInverseIntervalsCase2(Pt, uMinMax, vMinMax, wMinMax, false);			
			state = NumericSolutionNeededRestartNotAllowed;
		}		
	}

	return retStatus;
}

// Description:
// Performs the mapping \$Pt -> f(uvw)\$f, if available.  
//
// Pt is the Cartesian point for which u, v and w parameters are to be located,
// uvw are the parameters, with u corresponding to uvw[0], v to uvw[1] and w to uvw[2] respectively. 
// The method returns false, if uvw parameters could not be found
/*virtual*/ bool vtkParametricMuscle::EvaluateInverse(double Pt[3], double uvw[3])
{
	//x = (Rx + a(u)*w*cos(v))*cos(u)
	//y = (Ry + a(u)*w*cos(v))*sin(u)
	//z = b(u)*w*sin(v)

	//This cannot be solved analytically except for singular cases
	//For numerical solution, we however needs a good initial solution, otherwise, the result will be poor	
	double uMin = this->MinimumU;
	double uMax = this->MaximumU;
	if (this->CapPortionAvailable != 0)
	{
		double uCap = (uMax - uMin)* this->CapPortion;	//in u parameter
		uMin += uCap;	//uMin is where the regular part starts
		uMax -= uCap;	//uMax is where the regular part ends
	}

	InverseIntervalsState state = FirstAttempt;
	while (true)
	{
		double	uMinMax[2] = {uMin, uMax},
				vMinMax[2] = {this->MinimumV, this->MaximumV},
				wMinMax[2] = {0.0, 1.0};
	
		FindEvaluateInverseIntervals(Pt, uMinMax, vMinMax, wMinMax, state);
		if (state == AnalyticSolutionFoundRestartNotAllowed || state == AnalyticSolutionFoundRestartAllowed) {
			uvw[0] = uMinMax[0]; uvw[1] = vMinMax[0]; uvw[2] = wMinMax[0];			
		}		

		bool solutionValid = true;
		if (state == NumericSolutionNeededRestartAllowed || state == NumericSolutionNeededRestartNotAllowed)		
		{	
			//x = (Rx + a(u)*w*cos(v))*cos(u)
			//y = (Ry + a(u)*w*cos(v))*sin(u)
			//z = b(u)*w*sin(v)
			SOLVER_CONTEXT_DATA ssFit;
			ssFit.pThis = this;
			memcpy(ssFit.Pt, Pt, 3*sizeof(double));		

			double epsf = 0;	
			alglib::ae_int_t maxits = 0;
			alglib::nleqstate state;
			alglib::nleqreport rep;

			alglib::real_1d_array x;		//set initial solution
			x.setlength(3);	

			x[0] = (uMinMax[0] + uMinMax[1]) / 2;
			x[1] = (vMinMax[0] + vMinMax[1]) / 2;
			x[2] = (wMinMax[0] + wMinMax[1]) / 2;

			alglib::nleqcreatelm(3, x, state);
			alglib::nleqsetcond(state, epsf, maxits);
			alglib::nleqsolve(state, EvaluateInverse_fvec, EvaluateInverse_jac, NULL, &ssFit);
			alglib::nleqresults(state, x, rep);	

			//-4    ERROR:  algorithm   has   converged   to   the
			//stationary point Xf which is local minimum  of
			//f=F[0]^2+...+F[m-1]^2, but is not solution  of
			//nonlinear system.

			if (rep.terminationtype < 0)
				solutionValid = false;	//sorry, unable to solve

			uvw[0] = x[0];
			uvw[1] = x[1];
			uvw[2] = x[2];	
		}
		
		if (uvw[0] < uMin || uvw[0] > uMax)
			solutionValid = false;	//WE SUPPORT ONLY REGULAR PART
	
		if (uvw[1] < this->MinimumV || uvw[1] > this->MaximumV)
			solutionValid = false;

		if (uvw[2] < 0.0 || uvw[2] > 1.0)
			solutionValid = false;

		if (solutionValid)
			return true;

		if (state == AnalyticSolutionFoundRestartAllowed || state == NumericSolutionNeededRestartAllowed)
			state = RestartedAttempt;
		else
			return false;
	}	
}

/*static*/ void vtkParametricMuscle::EvaluateInverse_fvec(const alglib::real_1d_array &x, double &func, void *ptr)
{
	((SOLVER_CONTEXT_DATA*)ptr)->pThis->EvaluateInverse_fvec(x, ((SOLVER_CONTEXT_DATA*)ptr)->Pt, func);
}

/*static*/ void vtkParametricMuscle::EvaluateInverse_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr)
{
	((SOLVER_CONTEXT_DATA*)ptr)->pThis->EvaluateInverse_jac(x, ((SOLVER_CONTEXT_DATA*)ptr)->Pt, fi, jac);
}

//objective function
void vtkParametricMuscle::EvaluateInverse_fvec(const alglib::real_1d_array &x, const double Pt[3], double &func)
{
	//x = (Rx + a(u)*w*cos(v))*cos(u)		=> F0(u, v, w) = (Rx + a(u)*w*cos(v))*cos(u) - x
	//y = (Ry + a(u)*w*cos(v))*sin(u)
	//z = b(u)*w*sin(v)

	double uvw[3] = {x[0], x[1], x[2] }, Pt2[3];
	Evaluate(uvw, Pt2);
	func =	(Pt2[0] - Pt[0])*(Pt2[0] - Pt[0]) +
			(Pt2[1] - Pt[1])*(Pt2[1] - Pt[1]) +
			(Pt2[2] - Pt[2])*(Pt2[2] - Pt[2]);
}

//jacobian and function values
void vtkParametricMuscle::EvaluateInverse_jac(const alglib::real_1d_array &x, const double Pt[3], alglib::real_1d_array &fi, alglib::real_2d_array &jac)
{
	//x = (Rx + a(u)*w*cos(v))*cos(u)		=> F0(u, v, w) = (Rx + a(u)*w*cos(v))*cos(u) - x
	//y = (Ry + a(u)*w*cos(v))*sin(u)
	//z = b(u)*w*sin(v)

	double uvw[3] = {x[0], x[1], x[2] }, Pt2[3], Duvw[9];
	Evaluate(uvw, Pt2);
	EvaluateDerivative(uvw, Duvw);

	for (int k = 0; k < 3; k++)
	{
		fi[k]		= (Pt2[k] - Pt[k]);
		jac[k][0]	= Duvw[k];
		jac[k][1]	= Duvw[3 + k];
		jac[k][2]	= Duvw[6 + k];
	}	
}

//Computes partial derivatives at the point f(u,v,w).
 /*virtual*/ void vtkParametricMuscle::EvaluateDerivative(double uvw[3], double Duvw[9])
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

	//x = (Rx + a(u)*w*cos(v))*cos(u)		=> dx/du = -(Rx Sin[u]) - w a[u] Cos[v] Sin[u] + w Cos[u] Cos[v] a'[u],	dx/dv = -(w a[u] Cos[u] Sin[v]),		dx/dw = a[u] Cos[u] Cos[v]
	//y = (Ry + a(u)*w*cos(v))*sin(u)		=>dy/du = Cos[u] (Ry + w a[u] Cos[v]) + w Cos[v] Sin[u] a'[u],			dy/dv = -(w a[u] Sin[u] Sin[v]),		dy/dw = a[u] Cos[v] Sin[u]
	//z = b(u)*w*sin(v)					=>dz/du = w Sin[v] b'[u],										dz/dv = w b[u] Cos[v],				dz/dw = b[u] Sin[v]
	//where a'(u) = A1*t1' + A2*t2' and b'(u) = B1*t1' + B2*t2'

	double dt1, dt2;	//for blending between CrossRadiusX1, CrossRadiusX2 and CrossRadiusZ1 and CrossRadiusZ2 
	switch (status)
	{
	case vtkParametricFunction::CapMin:		
		dt1 = 1.0 / tol;
		dt2 = 0.0; 
		break;
	case vtkParametricFunction::CapMax:
		dt1 = 0.0; 
		dt2 = -1 / tol;
		break;
	case vtkParametricFunction::RegularPart:		
		dt2 = 1 / (this->MaximumU - this->MinimumU - 2*tol); 
		dt1 = -dt2;
		break;	
	}

	double a = GetAu(u, status, tol);
	double b = GetBu(u, status, tol);
	double da = dt1*this->CrossRadiusX1 + dt2*this->CrossRadiusX2;
	double db = dt1*this->CrossRadiusZ1 + dt2*this->CrossRadiusZ2;


	//The derivatives are:
	Du[0] = -(this->RadiusX + a*w*cv)*su + w*cu*cv*da;
	Du[1] = (this->RadiusY + a*w*cv)*cu + w*su*cv*da;
	Du[2] = w*sv*db;

	Dv[0] = -(a*w*sv)*cu;
	Dv[1] = -(a*w*sv)*su;
	Dv[2] = b*w*cv;

	Dw[0] = a*cv*cu;
	Dw[1] = a*cv*su;
	Dw[2] = b*sv;	
 }

 //Computes partial derivatives of 2nd order at the point f(u,v,w).
 /*virtual*/ void vtkParametricMuscle::EvaluateDerivative2(double uvw[3], double Duvw[9])
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

	//x = (Rx + a(u)*w*cos(v))*cos(u)		=> dx/du = -(Rx Sin[u]) - w a[u] Cos[v] Sin[u] + w Cos[u] Cos[v] a'[u],	dx/dv = -(w a[u] Cos[u] Sin[v]),		dx/dw = a[u] Cos[u] Cos[v]
	//y = (Ry + a(u)*w*cos(v))*sin(u)		=>dy/du = Cos[u] (Ry + w a[u] Cos[v]) + w Cos[v] Sin[u] a'[u],		dy/dv = -(w a[u] Sin[u] Sin[v]),		dy/dw = a[u] Cos[v] Sin[u]
	//z = b(u)*w*sin(v)					=>dz/du = w Sin[v] b'[u],										dz/dv = w b[u] Cos[v],				dz/dw = b[u] Sin[v]
	//where a'(u) = A1*t1' + A2*t2' and b'(u) = B1*t1' + B2*t2'
	//
	//Hence, since a''(u) = 0 and b''(0), 
	//d2x/du2 =-(w a[u] Cos[u] Cos[v]) - 2 w Cos[v] Sin[u] a'[u] + Cos[u] (-Rx + w Cos[v] a''[u]) = -((Rx + w a[u] Cos[v]) Cos[u]) - 2 w Cos[v] Sin[u] a'[u]
	//d2y/du2 = -((Ry + w a[u] Cos[v]) Sin[u]) + 2 w Cos[u] Cos[v] a'[u] + w Cos[v] Sin[u] a''[u] = -((Ry + w a[u] Cos[v]) Sin[u]) + 2 w Cos[v] Cos[u] a'[u]
	//d2z/du2 = 0
	//d2x/dv2 = -(w a[u] Cos[u] Cos[v])
	//d2y/dv2 = -(w a[u] Sin[u] Cos[v])
	//d2z/dv2 = -(w b[u] Sin[v])
	//d2x/dw2 = d2y/dw2 = d2z/dw2 = 0


	double dt1, dt2;	//for blending between CrossRadiusX1, CrossRadiusX2 and CrossRadiusZ1 and CrossRadiusZ2 
	switch (status)
	{
	case vtkParametricFunction::CapMin:		
		dt1 = 1.0 / tol;
		dt2 = 0.0; 
		break;
	case vtkParametricFunction::CapMax:
		dt1 = 0.0; 
		dt2 = -1 / tol;
		break;
	case vtkParametricFunction::RegularPart:		
		dt2 = 1 / (this->MaximumU - this->MinimumU - 2*tol); 
		dt1 = -dt2;
		break;	
	}

	double a = GetAu(u, status, tol);
	double b = GetBu(u, status, tol);
	double da = dt1*this->CrossRadiusX1 + dt2*this->CrossRadiusX2;
	double db = dt1*this->CrossRadiusZ1 + dt2*this->CrossRadiusZ2;


	//The derivatives are:
	Du[0] = -(this->RadiusX + a*w*cv)*cu - 2*w*su*cv*da;	//-((Rx + w a[u] Cos[v]) Cos[u]) - 2 w Cos[v] Sin[u] a'[u]
	Du[1] = -(this->RadiusY + a*w*cv)*su + 2*w*cu*cv*da;	//-((Ry + w a[u] Cos[v]) Sin[u]) + 2 w Cos[v] Cos[u] a'[u]
	Du[2] = 0;

	Dv[0] = -(a*w*cv)*cu;
	Dv[1] = -(a*w*cv)*su;
	Dv[2] = -(b*w*sv);

	Dw[0] = Dw[1] = Dw[2] = 0;	
 }

//----------------------------------------------------------------------------
double vtkParametricMuscle::EvaluateScalar(double* vtkNotUsed(uv[3]),
										  double* vtkNotUsed(Pt[3]),
										  double* vtkNotUsed(Duv[9]))
{
	return 0;
}

//----------------------------------------------------------------------------
void vtkParametricMuscle::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "RadiusX: " << this->RadiusX << "\n";
	os << indent << "RadiusY: " << this->RadiusY << "\n";
	os << indent << "Cross Radius X1: " << this->CrossRadiusX1 << "\n";
	os << indent << "Cross Radius X2: " << this->CrossRadiusX2 << "\n";
	os << indent << "Cross Radius Z1: " << this->CrossRadiusZ1 << "\n";
	os << indent << "Cross Radius Z2: " << this->CrossRadiusZ2 << "\n";
}
#endif