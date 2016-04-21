/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: OoCylinder.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-03-30 06:54:03 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "OoCylinder.h"


OoCylinder::OoCylinder(vtkPolyData *mesh, PKMatrix *points)
{
	this->direction[0] = this->direction[1] = this->direction[2] = 0;
	this->start[0] = this->start[1] = this->start[2] = 0;
	this->length = 0;
	this->radius = 0;

	if (mesh != NULL || points != NULL) {
		this->SetUpByMesh(mesh, points);
	}
}


OoCylinder::~OoCylinder(void)
{
}

void OoCylinder::SetUpByMesh(vtkPolyData* mesh, PKMatrix *points) {
	//the line goes through the centroid
	int nPoints = mesh->GetNumberOfPoints();
	this->start[0] = this->start[1] = this->start[2] = 0.0;

	for (int i = 0; i < nPoints; i++)
	{
		const double* pcoords = myGetPoint(mesh, points, i);
		for (int j = 0; j < 3; j++){
			this->start[j] += pcoords[j];
		}
	}

	for (int j = 0; j < 3; j++){
		this->start[j] /= nPoints;
	}

	//compute the covariance matrix, which is a symmetric matrix
	double A[3][3], eigenvals[3], eigenvects[3][3];

	//fill the matrix
	memset(A, 0, sizeof(A));
	for (int k = 0; k < nPoints; k++)
	{
		const double* pcoords = myGetPoint(mesh, points, k);

		for (int i = 0; i < 3; i++) 
		{
			for (int j = 0; j < 3; j++) {        
				A[i][j] += (pcoords[i] - this->start[i])*(pcoords[j] - this->start[j]);
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++){
			A[i][j] /= (nPoints - 1);
		}
	}  

	//compute eigen vectors, the principal axis is the first one
	double *ATemp[3], *V[3];
	for (int i = 0; i < 3; i++)
	{
		ATemp[i] = A[i];
		V[i] = eigenvects[i];
	}

	vtkMath::Jacobi(ATemp, eigenvals, V);

	//copy the result
	//N.B. Jacobi returns vectors in columns!
	for (int i = 0; i < 3; i++) {    
		this->direction[i] = eigenvects[i][0];
	}

	PKUtils::DivideVertex(this->direction, PKUtils::CalculateVertexLength(this->direction));

	// find radius and length
	double point[3];
	double onAxis[3];
	double d = - PKUtils::Dot(this->direction, this->start);
	double maxT = 0;
	double minT = 0;
	double radiusSq = 0;

	for (int i = 0; i < nPoints; i++) {
		myGetPointDeep(mesh, points, i, point);
		double t = PKUtils::Dot(this->direction, point) + d;

		maxT = max(t, maxT);
		minT = min(t, minT);

		// projection to axis
		PKUtils::CopyVertex(this->direction, onAxis);
		PKUtils::MultiplyVertex(onAxis, t);
		PKUtils::AddVertex(this->start, onAxis, onAxis);

		// radius
		PKUtils::SubtractVertex(onAxis, point, onAxis);
		radiusSq = max(radiusSq, PKUtils::CalculateVertexLengthSq(onAxis));
	}

	// go to start
	PKUtils::CopyVertex(this->direction, onAxis);
	PKUtils::MultiplyVertex(onAxis, minT);
	PKUtils::AddVertex(this->start, onAxis, this->start);

	this->length = maxT - minT;
	this->radius = sqrt(radiusSq);
}

bool OoCylinder::intersectsRadius(OoCylinder *ooc1, OoCylinder *ooc2) {
	double temp[3];
	double dStart[3];

	vtkMath::Cross(ooc1->direction, ooc2->direction, temp);
	PKUtils::DivideVertex(temp, PKUtils::CalculateVertexLength(temp));

	PKUtils::SubtractVertex(ooc1->start, ooc2->start, dStart);

	double distance = PKUtils::Dot(temp, dStart);

	if (distance > ooc1->radius + ooc2->radius) {
		return false;
	}

	return true;
}

void OoCylinder::GetDirection(double direction[3]) {
	PKUtils::CopyVertex(this->direction, direction);
}

void OoCylinder::GetStart(double start[3]) {
	PKUtils::CopyVertex(this->start, start);
}

double OoCylinder::GetLength() {
	return this->length;
}

double OoCylinder::GetRadius() {
	return this->radius;
}

bool OoCylinder::intersectsCapsule(OoCylinder *ooc1, OoCylinder *ooc2) {
	double end1[3];
	PKUtils::CopyVertex(ooc1->direction, end1);
	PKUtils::MultiplyVertex(end1, ooc1->length);
	PKUtils::AddVertex(ooc1->start, end1, end1);

	double end2[3];
	PKUtils::CopyVertex(ooc2->direction, end2);
	PKUtils::MultiplyVertex(end2, ooc2->length);
	PKUtils::AddVertex(ooc2->start, end2, end2);

	double distanceSq = PKMath::GetDistanceSqBetweenLineSegments3D(ooc1->start, end1, ooc2->start, end2);
	double radiusSumSq = ooc1->radius + ooc2->radius;
	radiusSumSq *= radiusSumSq;
	
	if (distanceSq > radiusSumSq) {
		return false;
	}

	return true;
}

#ifdef PROSTE_NE

//http://www.geometrictools.com/Documentation/IntersectionOfCylinders.pdf
bool OoCylinder::intersects(OoCylinder *ooc1, OoCylinder *ooc2) {
	
	throw "TODO: Unimplemented";

	double delta[3];
	PKUtils::SubtractVertex(ooc2->start, ooc1->start, delta);

	double w0xw1[3];
	vtkMath::Cross(ooc1->direction, ooc2->direction, w0xw1);
	
	double lenW0xW1 = PKUtils::CalculateVertexLength(w0xw1);
	double h0Div2 = ooc1->length / 2;
	double h1Div2 = ooc2->length / 2;
	double rSum = ooc1->radius + ooc2->radius;
	
	if (lenW0xW1 > 0)
	{
		// Test for separation by W0.
		if (ooc2->radius*lenW0xW1 + h0Div2 + h1Div2*abs(PKUtils::Dot(ooc1->direction,ooc2->direction)) - abs(PKUtils::Dot(ooc1->direction,delta)) < 0) return false;
		// Test for separation by W1.
		if (ooc1->radius*lenW0xW1 + h0Div2*abs(PKUtils::Dot(ooc1->direction,ooc2->direction)) + h1Div2 - abs(PKUtils::Dot(ooc2->direction,delta)) < 0) return false;
		// Test for separation by W0xW1.
		if (rSum*lenW0xW1 - abs(PKUtils::Dot(w0xw1, delta)) < 0) return false;
		// Test for separation by directions perpendicular to W0.
		if (OoCylinder::SeparatedByCylinderPerpendiculars(ooc1->start,ooc1->direction,ooc1->radius,ooc1->length,ooc2->start,ooc2->direction,ooc2->radius,ooc2->length)) return false;
		// Test for separation by directions perpendicular to W1.
		if (OoCylinder::SeparatedByCylinderPerpendiculars(ooc2->start,ooc2->direction,ooc2->radius,ooc2->length, ooc1->start,ooc1->direction,ooc1->radius,ooc1->length)) return false;
		// Test for separation by other directions.
		if (OoCylinder::SeparatedByOtherDirections(ooc1->direction,ooc1->radius,ooc1->length,ooc2->direction,ooc2->radius,ooc2->length,delta)) return false;
		}
	else
	{
		// Test for separation by height.
		if (h0Div2 + h1Div2 - abs(PKUtils::Dot(ooc1->direction,delta)) < 0) return false;
		// Test for separation radially.
		double dot = PKUtils::Dot(ooc1->direction, delta);
		PKUtils::CopyVertex(ooc1->direction, w0xw1);
		PKUtils::MultiplyVertex(w0xw1, dot);
		PKUtils::SubtractVertex(delta, w0xw1, w0xw1);
		if (rSum - PKUtils::CalculateVertexLength(w0xw1) < 0) return false;
		// If parallel cylinders are not separated by height or radial distance,
		// then the cylinders must overlap.
	}
	return true;



}

double OoCylinder::F (double t, double r0, double r1, double h1b1Div2, double c1sqr, double a2, double b2)
{
	double omt = 1 - t;
	double tsqr = t*t;
	double omtsqr = omt*omt;
	double term0 = r0*sqrt(omtsqr + tsqr);
	double term1 = r1*sqrt(omtsqr + c1sqr*tsqr);
	double term2 = h1b1Div2*t;
	double term3 = abs(omt*a2 + t*b2);
	return term0 + term1 + term2 - term3;
}

double OoCylinder::FDer (double t, double r0, double r1, double h1b1Div2, double c1sqr, double a2, double b2)
{
	double omt = 1 - t;
	double tsqr = t*t;
	double omtsqr = omt*omt;
	double term0 = r0*(2*t-1)/sqrt(omtsqr + tsqr);
	double term1 = r1*((1+c1sqr)*t - 1)/sqrt(omtsqr + c1sqr*tsqr);
	double term2 = h1b1Div2;
	double term3 = (b2 - a2)*sign(omt*a2 + t*b2);
	return term0 + term1 + term2 - term3;
}

bool OoCylinder::SeparatedByCylinderPerpendiculars (double C0[3], double W0[3], double r0, double h0, double C1[3], double W1[3], double r1, double h1)
{
	double Delta[3];
	PKUtils::SubtractVertex(C1, C0, Delta);
	double c1 = PKUtils::Dot(W0,W1);
	double b1 = sqrt(1 - c1*c1);
	
	double V0[3];
	PKUtils::CopyVertex(W0, V0);
	PKUtils::MultiplyVertex(V0, c1);
	PKUtils::SubtractVertex(W1, V0, V0);
	PKUtils::DivideVertex(V0, b1);
	
	double U0[3];
	vtkMath::Cross(V0, W0, U0);
	
	double a2 = PKUtils::Dot(Delta,U0);
	double b2 = PKUtils::Dot(Delta,V0);

	// Test directions (1-t)*U0 + t*V0.
	if (OoCylinder::F(0) <= 0) return true; // U0 is a separating direction
	if (OoCylinder::F(1) <= 0) return true; // V0 is a separating direction
	if (OoCylinder::FDer(0) >= 0) return false; // no separation by perpendicular directions
	if (OoCylinder::FDer(1) <= 0) return false; // no separation by perpendicular directions

	// Use bisection to locate t-bar for which F(t-bar) is a minimum. The upper
	// bound maxIterations may be chosen to guarantee a specified number of digits
	// of precision in the t-variable.
	double t0, t1, fd0, fd1, tmid, fdmid;
	int i;
	t0 = 0;
	t1 = 1;
	for (i = 0; i < OOC_MAX_SOLVE_ITER; i++)
	{
		tmid = 0.5*(t0 + t1);
		if (OoCylinder::F(tmid) <= 0) return true; // (1-t)*U0 + t*V0 is a separating direction
		fdmid = OoCylinder::FDer(tmid);
		if (fdmid > 0) { t1 = tmid; } else if (fdmid < 0) { t0 = tmid; } else { break; }
	}

	// Test directions (1-t)*(-U0) + t*V0.
	a2 = -a2;
	if (OoCylinder::F(0) <= 0) return true; // U0 is a separating direction
	if (OoCylinder::F(1) <= 0) return true; // V0 is a separating direction
	if (OoCylinder::FDer(0) >= 0) return false; // no separation by perpendicular directions
	if (OoCylinder::FDer(1) <= 0) return false; // no separation by perpendicular directions
	// Use bisection to locate t-bar for which F(t-bar) is a minimum. The upper
	// bound maxIterations may be chosen to guarantee a specified number of digits
	// of precision in the t-variable.
	t0 = 0;
	t1 = 1;
	for (i = 0; i < OOC_MAX_SOLVE_ITER; i++)
	{
		tmid = 0.5*(t0 + t1);
		if (OoCylinder::F(tmid) <= 0) return true; // (1-t)*U0 + t*V0 is a separating direction
		fdmid = FDer(tmid);
		if (fdmid > 0) { t1 = tmid; } else if (fdmid < 0) { t0 = tmid; } else { break; }
	}
}

double OoCylinder::G (double s, double t, double r0, double h0Div2, double r1, double h1Div2,
	double a0, double b0, double c0, double a1, double b1, double c1, double lenDelta)
{
	double omsmt = 1 - s - t, ssqr = s*s, tsqr = t*t, omsmtsqr = omsmt*omsmt;
	double temp = ssqr + tsqr + omsmtsqr;
	double L0 = a0*s + b0*t + c0*omsmt, L1 = a1*s + b1*t + c1*omsmt;
	double Q0 = temp - L0*L0, Q1 = temp - L1*L1;
	return r0*sqrt(Q0) + r1*sqrt(Q1) + h0Div2*abs(L0) + h1Div2*abs(L1) - omsmt*lenDelta;
}

void OoCylinder::GDer (double s, double t, double r0, double h0Div2, double r1, double h1Div2,
	double a0, double b0, double c0, double a1, double b1, double c1, double lenDelta, double gradient[2])
{
	double omsmt = 1 - s - t, ssqr = s*s, tsqr = t*t, omsmtsqr = omsmt*omsmt;
	double temp = ssqr + tsqr + omsmtsqr;
	double L0 = a0*s + b0*t + c0*omsmt, L1 = a1*s + b1*t + c1*omsmt;
	double Q0 = temp - L0*L0, Q1 = temp - L1*L1;
	double diffS = s - omsmt, diffT = t - omsmt;
	double diffa0c0 = a0 - c0, diffa1c1 = a1 - c1, diffb0c0 = b0 - c0, diffb1c1 = b1 - c1;
	double halfQ0s = diffS - diffa0c0*L0, halfQ1s = diffS - diffa1c1*L1;
	double halfQ0t = diffT - diffb0c0*L0, halfQ1t = diffT - diffb1c1*L1;
	double factor0 = r0/sqrt(Q0), factor1 = r1/sqrt(Q1);
	double signL0 = sign(L0), signL1 = sign(L1);

	gradient[0] = gradient[1] = 0;
	gradient[0] += halfQ0s*factor0;
	gradient[0] += halfQ1s*factor1;
	gradient[0] += h0Div2*diffa0c0*signL0;
	gradient[0] += h1Div2*diffa1c1*signL1;
	gradient[0] += lenDelta;
	gradient[1] += halfQ0t*factor0;
	gradient[1] += halfQ1t*factor1;
	gradient[1] += h0Div2*diffb0c0*signL0;
	gradient[1] += h1Div2*diffb1c1*signL1;
	gradient[1] += lenDelta;
}

bool SeparatedByOtherDirections (double W0[3], double r0, double h0, double W1[3], double r1, double h1, double Delta[3])
{
	// Minimize G(s,t) subject to s >= 0, t >= 0, and s+t <= 1. If at
	// any iterate you find a value for which G <= 0, return 'true'.
	// If no separating directions have been found at the end of the
	// minimization, return 'false'.
}

#endif