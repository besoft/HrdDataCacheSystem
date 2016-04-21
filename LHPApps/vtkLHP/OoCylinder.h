/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: OoCylinder.h,v $ 
  Language: C++ 
  Date: $Date: 2011-03-30 06:54:03 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef OoCylinder_h__
#define OoCylinder_h__

#pragma once

//vtk
#include "vtkPolyData.h"
#include "vtkPoints.h"

// my classes
#include "PKUtils.h"
#include "PKMath.h"

using namespace std;

#ifndef OOC_MAX_SOLVE_ITER
#define OOC_MAX_SOLVE_ITER 100
#endif

class OoCylinder
{
public:
	OoCylinder(vtkPolyData *mesh = NULL, PKMatrix *points = NULL);
	~OoCylinder(void);

	double direction[3]; // keep normalized!
	double start[3];
	double radius;
	double length;
	
	void SetUpByMesh(vtkPolyData* mesh, PKMatrix *points);

	void GetDirection(double direction[3]);
	void GetStart(double start[3]);
	double GetLength();
	double GetRadius();

	static bool intersectsRadius(OoCylinder *ooc1, OoCylinder *ooc2);
	static bool intersectsCapsule(OoCylinder *ooc1, OoCylinder *ooc2);
	//static bool intersects(OoCylinder *ooc1, OoCylinder *ooc2);

private:
	/*static double F (double t, double r0, double r1, double h1b1Div2, double c1sqr, double a2, double b2);
	static double FDer (double t, double r0, double r1, double h1b1Div2, double c1sqr, double a2, double b2);
	static bool SeparatedByCylinderPerpendiculars (double C0[3], double W0[3], double r0, double h0, double C1[3], double W1[3], double r1, double h1);
	static double G (double s, double t, double r0, double h0Div2, double r1, double h1Div2, double a0, double b0, double c0, double a1, double b1, double c1, double lenDelta);
	static void GDer (double s, double t, double r0, double h0Div2, double r1, double h1Div2, double a0, double b0, double c0, double a1, double b1, double c1, double lenDelta, double gradient[2]);
	static bool SeparatedByOtherDirections (double W0[3], double r0, double h0, double W1[3], double r1, double h1, double Delta[3]);*/
};

#endif