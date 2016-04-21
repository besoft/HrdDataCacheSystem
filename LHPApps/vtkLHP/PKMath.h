/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKMath.h,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:00:35 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef PKMath_h__
#define PKMath_h__
 
#pragma once

// std
#include <cmath>
#include <set>

// VTK
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCell.h"
#include "vtkGenericCell.h"

// my
#include "PKUtils.h"
#include "PKHashTable.h"

#ifndef MeshNavigator
class MeshNavigator;
#endif;

using namespace std;

#ifndef VERTEX_HIT_EPSILON
#define VERTEX_HIT_EPSILON 0.0001
#endif

#ifndef PARALLEL_EPSILON
#define PARALLEL_EPSILON 0.000001
#endif

//////
/// 
//////
class PKMath
{
public:
	static double GetDistanceSqToTriangle(const double A[3], const double B[3], const double C[3], const double point[3], double direction[3], double maxDistSq);
	static double GetDistanceSqToLinePart2D(const double point[3], const double a[3], const double b[3], double direction[2]);
	static double GetDistanceSqBetweenLineSegments3D(const double line1A[3], const double line1B[3], const double line2A[3], const double line2B[3]);
	static void RotateVertex(double vertex[3], const double oldDirection[3], const double newDirection[3]);
	//////
	/// Calculates spin of point in specified direction, which is to be understanded 
	/// as a number of intersections that ray sent in that direction makes with the mesh. If mesh is closed 
	/// and result is odd number then point lies inside mesh. Otherwise it lies outside or on the mesh.
	/// @param mesh tested mesh (input)
	/// @param point starting point of casted ray (input)
	/// @param direction direction of ray (input)
	/// @param initSkip length of initial deaf space where intersections are ingored (to disallow self intersections for mesh vertices as point) (input)
	/// @return spin = number of ray intersections with mesh
	//////
	static int CalculatePointSpin(vtkPolyData *mesh, PKMatrix *points, double point[3], double direction[3], double initSkip, MeshNavigator *navigator = NULL);
	static void CalculateBarycentricCoords(const double point[3], const double a[3], const double b[3], const double c[3], double coords[3]);
	static bool GetTriangleNormal(vtkPolyData *mesh, const PKMatrix *points, vtkIdType triangleId, double normal[3]);
	static bool GetPlaneLineIntersection(const double plane[4], const double point[3], const double dir[3], double intersection[3], double *t = NULL);
	static bool IntersectsRayBox(const double start[3], const double dir[3], 
		const double planeXYa[4], const double planeXYb[4], const double planeYZa[4], const double planeYZb[4], const double planeXZa[4], const double planeXZb[4],
		double* intersection = NULL, double *distance = NULL);

	/** 
	Determines if the line defined by one point  and direction intersects the given triangle ABC.
	Returns true, if the triangle is intersected; false, otherwise. If t is not NULL, it returns parameterized position of the intersection.
	Cartesian coordinates of the intersection are to be calculated as point + t*direction.
	*/
	static bool GetTriangleLineIntersection(const double a[3], const double b[3], const double c[3], 
		const double point[3], const double direction[3], double* t = NULL);

	/**
	Finds the cell of the mesh that is intersected by the ray specified by point and direction and is closest to the origin of the ray.
	@param mesh tested mesh (input)
	@param point starting point of casted ray (input)
	@param direction direction of ray (input), should be normalized
	@param initSkip length of the area in which intersection is to be ignored 
	@param navigator allows skipping cells that cannot be intersected (may be NULL)
	@param triangleId if not NULL, here is stored id of the cell intersected
	@param t if not NULL, here is returned the parameterized position of the intersection
	@return true, if there is any intersection; false, otherwise
	Cartesian coordinates of the intersection are to be obtained as point + t*direction
	Note that if t < initSkip, it is considered that there is no intersection */
	static bool FindIntersectionInDirection(vtkPolyData *mesh, PKMatrix *points, 
		const double point[3], const double direction[3], double initSkip, 
		MeshNavigator *navigator = NULL, vtkIdType* triangleId = NULL, double* t = NULL);

	static inline double SameSide(const double p1[3] , const double p2[3], const double a[3], const double b[3]) {
		double temp1[3];
		double temp2[3];
		double cp1[3];
		double cp2[3];
		PKUtils::SubtractVertex(b, a, temp1);
		PKUtils::SubtractVertex(p1, a, temp2);
		vtkMath::Cross(temp1, temp2, cp1);
		PKUtils::SubtractVertex(p2, a, temp2);
		vtkMath::Cross(temp1, temp2, cp2);

		if (PKUtils::Dot(cp1, cp2) >= 0) {
			return true;
		}
	    
		return false;
	}

	// 3D version
	static inline double PointInTriangle(const double p[3], const double a[3], const double b[3], const double c[3]) {
		if (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b)) {
			return true;
		}
		return false;
	}

	


private:
	//////
	/// Hidden contructor of static class.
	//////
	PKMath(void);

	//////
	/// Hidden destructor of static class.
	//////
	~PKMath(void);
};

#endif
