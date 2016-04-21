/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKMath.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:00:35 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#include "PKMath.h"
#include "MeshNavigator.h"
#include "mafDbg.h"

PKMath::PKMath(void)
{
}

PKMath::~PKMath(void)
{
}

 
double PKMath::GetDistanceSqToTriangle(const double A[3], const double B[3], const double C[3], const double point[3], double direction[3], double maxDistSq) {
	// extend dimensions
	double temp[4];
	double plane[4];
	double onPlane[3];
	double side[3];
	double pointRel[3];

	double A4[4];
	double B4[4];
	double C4[4];
	double point4[4];

	PKUtils::CopyVertex(A, A4);
	A4[3] = 1;
	PKUtils::CopyVertex(B, B4);
	B4[3] = 1;
	PKUtils::CopyVertex(C, C4);
	C4[3] = 1;
	PKUtils::CopyVertex(point, point4);
	point4[3] = 1;

	// perpendicular distance
	PKUtils::Cross4(A4, B4, C4, plane);
	PKUtils::MultiplyVertex4(plane, 1 / PKUtils::CalculateVertexLength(plane));
	double height = PKUtils::Dot4(plane, point4);
	double heightSq = height * height;

	// save us troubles if already too far
	if (heightSq > maxDistSq) {
		return maxDistSq;
	}

	// point on plane
	PKUtils::CopyVertex(plane, temp);
	PKUtils::MultiplyVertex(temp, - height);
	PKUtils::CopyVertex(temp, direction);
	PKUtils::AddVertex(point, temp, onPlane);

	// rotate to 2D
	double zAxis[] = { 0, 0, 1 };

	PKMath::RotateVertex(A4, plane, zAxis);
	A4[2] = 1;
	PKMath::RotateVertex(B4, plane, zAxis);
	B4[2] = 1;
	PKMath::RotateVertex(C4, plane, zAxis);
	C4[2] = 1;	
	PKMath::RotateVertex(onPlane, plane, zAxis);
	onPlane[2] = 1;

	// inside?
	PKUtils::SubtractVertex2(B4, A4, side);
	PKUtils::SubtractVertex2(onPlane, A4, pointRel);
	int sign1 = PKUtils::Dot2(side, pointRel);
	PKUtils::SubtractVertex2(C4, B4, side);
	PKUtils::SubtractVertex2(onPlane, B4, pointRel);
	int sign2 = PKUtils::Dot2(side, pointRel);
	PKUtils::SubtractVertex2(A4, C4, side);
	PKUtils::SubtractVertex2(onPlane, C4, pointRel);
	int sign3 = PKUtils::Dot2(side, pointRel);

	if (sign(sign1) == sign(sign2) && sign(sign1) == sign(sign3)) {
		return heightSq;
	}

	// find nearest edge
	double dirAB[2];
	double dirBC[2];
	double dirCA[2];

	double distAB = PKMath::GetDistanceSqToLinePart2D(onPlane, A4, B4, dirAB);
	double distBC = PKMath::GetDistanceSqToLinePart2D(onPlane, B4, C4, dirBC);
	double distCA = PKMath::GetDistanceSqToLinePart2D(onPlane, C4, A4, dirCA);

	double minDist = 0;

	if (distAB >= distBC && distAB >= distCA) {
		PKUtils::CopyVertex2(dirAB, temp);
		minDist = distAB;
	} else if (distBC >= distAB && distBC >= distCA) {
		PKUtils::CopyVertex2(dirBC, temp);
		minDist = distBC;
	} else {
		PKUtils::CopyVertex2(dirCA, temp);
		minDist = distCA;
	}

	// combine with vertical
	temp[2] = 0;
	PKMath::RotateVertex(temp, zAxis, plane);
	PKUtils::AddVertex(direction, temp, direction);
	return heightSq + minDist;
}

double PKMath::GetDistanceSqToLinePart2D(const double point[3], const double a[3], const double b[3], double direction[2]) {
	double line[3];
	double onLine[2];
	double temp[2];

	// create line
	vtkMath::Cross(a, b, line);
	PKUtils::DivideVertex(line, PKUtils::CalculateVertex2Length(line));

	// distance to line
	double toLine = PKUtils::Dot(line, point);
	PKUtils::MultiplyVertex(line, -toLine);
	PKUtils::CopyVertex2(line, direction);
	PKUtils::AddVertex2(point, line, onLine);
	
	// distance to A and B
	PKUtils::SubtractVertex2(onLine, a, temp);
	PKUtils::SubtractVertex2(b, a, line);
	int signA = sign(PKUtils::Dot2(temp, line));

	PKUtils::SubtractVertex2(onLine, b, temp);
	PKUtils::SubtractVertex2(a, b, line);
	int signB = sign(PKUtils::Dot2(temp, line));

	// between
	if (signA * signB >= 0) {
		return toLine * toLine;
	}

	// left from A
	if (signA < 0) {
		PKUtils::SubtractVertex2(point, a, direction);	
	}
	else {
		// right from B
		PKUtils::SubtractVertex2(point, b, direction);	
	}

	return PKUtils::CalculateVertex2LengthSq(direction);
}

void PKMath::RotateVertex(double vertex[3], const double oldDirection[3], const double newDirection[3]) {
	double oldDirLocal[3];
	double newDirLocal[3];

	PKUtils::CopyVertex(oldDirection, oldDirLocal);
	PKUtils::CopyVertex(newDirection, newDirLocal);

	double rotated[3];

	/////////////////////////////
	// get rotation parameters //
	/////////////////////////////
	
	double oldAngleZtoY = -atan2(oldDirLocal[2], oldDirLocal[1]);	

	// rotate old XYZ to XY
	rotated[0] = oldDirLocal[0];
	rotated[1] = oldDirLocal[1] * cos(oldAngleZtoY) - oldDirLocal[2] * sin(oldAngleZtoY);
	rotated[2] = oldDirLocal[1] * sin(oldAngleZtoY) + oldDirLocal[2] * cos(oldAngleZtoY);
	PKUtils::CopyVertex(rotated, oldDirLocal);

	double oldAngleYtoX = -atan2(oldDirLocal[1], oldDirLocal[0]);
	double newAngleYtoZ = atan2(newDirLocal[2], newDirLocal[1]);

	// rotate old XYZ to XY
	rotated[0] = newDirLocal[0];
	rotated[1] = newDirLocal[1] * cos(-newAngleYtoZ) - newDirLocal[2] * sin(-newAngleYtoZ);
	rotated[2] = newDirLocal[1] * sin(-newAngleYtoZ) + newDirLocal[2] * cos(-newAngleYtoZ);
	PKUtils::CopyVertex(rotated, newDirLocal);

	double newAngleXtoY = atan2(newDirLocal[1], newDirLocal[0]);	

	///////////////////////
	// rotate vertex //////
	///////////////////////

	// rotate old XYZ to XY
	rotated[0] = vertex[0];
	rotated[1] = vertex[1] * cos(oldAngleZtoY) - vertex[2] * sin(oldAngleZtoY);
	rotated[2] = vertex[1] * sin(oldAngleZtoY) + vertex[2] * cos(oldAngleZtoY);
	PKUtils::CopyVertex(rotated, vertex);

	// rotate old XY to X and back to XY
	rotated[0] = vertex[0] * cos(oldAngleYtoX + newAngleXtoY) - vertex[1] * sin(oldAngleYtoX + newAngleXtoY);
	rotated[1] = vertex[0] * sin(oldAngleYtoX + newAngleXtoY) + vertex[1] * cos(oldAngleYtoX + newAngleXtoY);
	rotated[2] = vertex[2];
	PKUtils::CopyVertex(rotated, vertex);

	// rotate XY back to XYZ
	rotated[0] = vertex[0];
	rotated[1] = vertex[1] * cos(newAngleYtoZ) - vertex[2] * sin(newAngleYtoZ);
	rotated[2] = vertex[1] * sin(newAngleYtoZ) + vertex[2] * cos(newAngleYtoZ);
	PKUtils::CopyVertex(rotated, vertex);
}

//////
/// HAJ: Calculates spin of point in specified direction, which is to be understood
/// as a number of intersections that ray sent in that direction makes with the mesh. If mesh is closed 
/// and result is odd number then point lies inside mesh. Otherwise it lies outside or on the mesh.
/// @param mesh tested mesh (input)
/// @param point starting point of casted ray (input)
/// @param direction direction of ray (input)
/// @param initSkip length of initial deaf space where intersections are ignored (to disallow self intersections for mesh vertices as point) (input)
/// @param navigator allows skipping cells that cannot be intersected
/// @return spin = number of ray intersections with mesh
//////
int PKMath::CalculatePointSpin(vtkPolyData *mesh, PKMatrix *points, double point[3], double direction[3], double initSkip, MeshNavigator *navigator)
{	
	const static double eps = 0.000001;

	//make sure that the direction is normalized
	//vtkMath::Normalize(direction);	 - not required
	
	vtkIdType nCells = mesh->GetNumberOfCells();

	//get the cells that makes sense to check (requires navigator)	
	PKHashTable<vtkIdType, vtkIdType> cellsMap;	
	const PKHashTableRecord<vtkIdType, vtkIdType>* cellsMapInner = NULL;

	if (navigator != NULL) 
	{		
		navigator->GetTriangleCandidatesForRay(point, direction, &cellsMap);
		nCells = cellsMap.GetCount();		
		cellsMapInner = cellsMap.GetValuesRef();
	}

	int spin = 0;	//number of triangles intersected in one direction

	//for each cell to be checked			
	for (vtkIdType i = 0; i < nCells; i++)
	{
		//get the cell indices
		vtkIdType nCellPoints, *pCellPointsIds;
		mesh->GetCellPoints((cellsMapInner != NULL ? cellsMapInner[i].value : i), nCellPoints, pCellPointsIds);

		//check, if it is triangle?
		_VERIFY_CMD(nCellPoints == 3, continue);

		//get the current position of points forming the triangle being tested
		double a[3], b[3], c[3];
		myGetPointDeep(mesh, points, pCellPointsIds[0], a);
		myGetPointDeep(mesh, points, pCellPointsIds[1], b);
		myGetPointDeep(mesh, points, pCellPointsIds[2], c);
		
		
		//calculate the intersection of the triangle ABC with the line with its specified origin (point) and direction 
		double t;
		if (GetTriangleLineIntersection(a, b, c, point, direction, &t) && t >= initSkip) {
			//we have here ray intersection :-)
			spin++;
		}
	}	
		 
	return spin;
}

//////
//// Finds the cell of the mesh that is intersected by the ray specified by point and direction and is closest to the origin of the ray.
/// @param mesh tested mesh (input)
/// @param point starting point of casted ray (input)
/// @param direction direction of ray (input), should be normalized
/// @param initSkip length of the area in which intersection is to be ignored 
/// @param navigator allows skipping cells that cannot be intersected
/// @param triangleId if not NULL, here is stored id of the cell intersected
/// @param t if not NULL, here is returned the parameterized position of the intersection
/// @return true, if there is any intersection; false, otherwise
///  Cartesian coordinates of the intersection are to be obtained as point + t*direction
///  Note that if t < initSkip, it is considered that there is no intersection
//////
bool PKMath::FindIntersectionInDirection(vtkPolyData *mesh, PKMatrix *points, 
										 const double point[3], const double direction[3], double initSkip, 
										 MeshNavigator *navigator, 
										 vtkIdType* triangleId,
										 double* t)
{	
	const static double eps = 0.000001;
	double bestT = DBL_MAX;
	
	vtkIdType nCells = mesh->GetNumberOfCells();

	//get the cells that makes sense to check (requires navigator)	
	PKHashTable<vtkIdType, vtkIdType> cellsMap;	
	const PKHashTableRecord<vtkIdType, vtkIdType>* cellsMapInner = NULL;

	if (navigator != NULL) 
	{		
		navigator->GetTriangleCandidatesForRay(point, direction, &cellsMap);
		nCells = cellsMap.GetCount();		
		cellsMapInner = cellsMap.GetValuesRef();
	}
	
	//for each cell to be checked			
	for (vtkIdType i = 0; i < nCells; i++)
	{
		//get the cell indices
		vtkIdType triId = (cellsMapInner != NULL ? cellsMapInner[i].value : i);
		vtkIdType nCellPoints, *pCellPointsIds;
		mesh->GetCellPoints(triId, nCellPoints, pCellPointsIds);

		//check, if it is triangle?
		_VERIFY_CMD(nCellPoints == 3, continue);

		//get the current position of points forming the triangle being tested
		double a[3], b[3], c[3];
		myGetPointDeep(mesh, points, pCellPointsIds[0], a);
		myGetPointDeep(mesh, points, pCellPointsIds[1], b);
		myGetPointDeep(mesh, points, pCellPointsIds[2], c);
		

		//calculate the intersection of the triangle ABC with the line with its specified origin (point) and direction 
		double lt;
		if (GetTriangleLineIntersection(a, b, c, point, direction, &lt)) 
		{
			if (lt >= initSkip && lt < bestT)
			{
				//we have here ray intersection :-)
				bestT = lt;

				if (triangleId != NULL) {
					*triangleId = triId;
				}
			}
		}
	}	
		 
	if (t != NULL) {
		*t = bestT;
	}

	return bestT != DBL_MAX && bestT >= initSkip;
}
	
//Determines if the line defined by one point  and direction intersects the given triangle ABC.
//Returns true, if the triangle is intersected; false, otherwise. If t is not NULL, it returns parameterized position of the intersection.
//Cartesian coordinates of the intersection are to be calculated as point + t*direction.	
/*static*/ bool PKMath::GetTriangleLineIntersection(const double a[3], const double b[3], const double c[3], 
													const double point[3], const double direction[3], double* t)
{
	//the calculation is done according to Scheiner PJ, Eberly DH: Geometric Tools for Computer Graphics, pg. 487
	const static double eps = 0.000001;

	double e1[3], e2[3], p[3];
	for (int k = 0; k < 3; k++) {
		e1[k] = b[k] - a[k]; e2[k] = c[k] - a[k];
	}

	//check, if the triangle could be intersected by the ray cast from the given point in the direction
	vtkMath::Cross(direction, e2, p);	//p will be (0,0,0), if the ray is parallel to e2
	double tmp = vtkMath::Dot(p, e1);	//if the ray is parallel to the triangle, p is the normal of the triangle, hence, temp will be 0					
	if (fabs(tmp) < eps){
		return false;	
	}

	tmp = 1.0 / tmp;	//tmp is not zero, so we divide

	double s[3];
	for (int k = 0; k < 3; k++) {
		s[k] = point[k] - a[k];
	}

	//compute the first barycentric coordinate of the intersection of the line with the plane
	double u = tmp*vtkMath::Dot(s, p);
	if (u < 0.0 || u > 1.0) {
		return false;	//does not lie inside the triangle
	}

	//compute the second barycentric coordinate
	double q[3];
	vtkMath::Cross(s, e1, q);
	double v = tmp*vtkMath::Dot(direction, q);
	if (v < 0.0 || v > 1.0 || (u + v) > 1.0) {
		return false;	//does not lie inside the triangle
	}

	//the line intersect the triangle, the third barycentric coordinate is 1 - u - v
	//now, we need to determine, if the intersection is in the positive or negative direction
	if (t != NULL) {
		*t = tmp*vtkMath::Dot(e2, q);
	}

	return true;
}

//http://www.blackpawn.com/texts/pointinpoly/default.html
void PKMath::CalculateBarycentricCoords(const double point[3], const double a[3], const double b[3], const double c[3], double coords[3]) {
	// Compute vectors        
	double v0[3], v1[3], v2[3];
	
	PKUtils::SubtractVertex(c, a, v0);
	PKUtils::SubtractVertex(b, a, v1);
	PKUtils::SubtractVertex(point, a, v2);

	// Compute dot products
	double dot00 = PKUtils::Dot(v0, v0);
	double dot01 = PKUtils::Dot(v0, v1);
	double dot02 = PKUtils::Dot(v0, v2);
	double dot11 = PKUtils::Dot(v1, v1);
	double dot12 = PKUtils::Dot(v1, v2);

	// Compute barycentric coordinates
	double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	coords[0] = 1 - u - v; // to A
	coords[1] = v; // to B
	coords[2] = u; // to C
}

// http://softsurfer.com/Archive/algorithm_0106/algorithm_0106.htm#dist3D_Segment_to_Segment
double PKMath::GetDistanceSqBetweenLineSegments3D(const double line1A[3], const double line1B[3], const double line2A[3], const double line2B[3]) {
	double u[3];
	PKUtils::SubtractVertex(line1B, line1A, u);
	double v[3];
	PKUtils::SubtractVertex(line2B, line2A, v);
	double w[3];
	PKUtils::SubtractVertex(line1A, line2A, w);
	float    a = PKUtils::Dot(u,u);        // always >= 0
	float    b = PKUtils::Dot(u,v);
	float    c = PKUtils::Dot(v,v);        // always >= 0
	float    d = PKUtils::Dot(u,w);
	float    e = PKUtils::Dot(v,w);
	float    D = a*c - b*b;       // always >= 0
	float    sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
	float    tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < PARALLEL_EPSILON) { // the lines are almost parallel
		sN = 0.0;        // force using point P0 on segment S1
		sD = 1.0;        // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else {                // get the closest points on the infinite lines
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) {       // sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD) {  // sc > 1 => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0) {           // tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {      // tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) < PARALLEL_EPSILON ? 0.0 : sN / sD);
	tc = (abs(tN) < PARALLEL_EPSILON ? 0.0 : tN / tD);

	// get the difference of the two closest points
	double dP[3];
	PKUtils::MultiplyVertex(u, sc);
	PKUtils::MultiplyVertex(v, tc);
	PKUtils::SubtractVertex(u, v, dP);
	PKUtils::AddVertex(w, dP, dP); //Vector   dP = w + (sc * u) - (tc * v);  // = S1(sc) - S2(tc)

	return PKUtils::CalculateVertex2LengthSq(dP);   // return the closest distance
}

bool PKMath::GetTriangleNormal(vtkPolyData *mesh, const PKMatrix *points, vtkIdType triangleId, double normal[3]) {
		double a[3], b[3], c[3];
		double sideAB[3], sideCA[3];

		vtkIdType nPtsIds, *pPtsIds;
		mesh->GetCellPoints(triangleId, nPtsIds, pPtsIds);

		if (nPtsIds != 3) {
			return false;
		}

		// load points
		if (points != NULL) {
			PKUtils::CopyVertex(points->values[pPtsIds[0]], a);
			PKUtils::CopyVertex(points->values[pPtsIds[1]], b);
			PKUtils::CopyVertex(points->values[pPtsIds[2]], c);
		} else {
			mesh->GetPoint(pPtsIds[0], a);
			mesh->GetPoint(pPtsIds[1], b);
			mesh->GetPoint(pPtsIds[2], c);
		}

		// calculate normal
		PKUtils::SubtractVertex(b, a, sideAB);
		PKUtils::SubtractVertex(c, a, sideCA);
		vtkMath::Cross(sideAB, sideCA, normal);
		
		return true;
	}

/*static*/ bool PKMath::GetPlaneLineIntersection(const double plane[4], const double point[3], const double dir[3], double intersection[3], double *t) {
	double denom = PKUtils::Dot(plane, dir);

	if (abs(denom) < PARALLEL_EPSILON) {
		return false;
	}

	double tLocal = -(PKUtils::Dot(plane, point) + plane[3]) / denom;

	if (t != NULL) {
		*t = tLocal;
	}

	double temp[3];
	PKUtils::CopyVertex(dir, temp);
	PKUtils::MultiplyVertex(temp, tLocal);
	PKUtils::AddVertex(point, temp, intersection);

	return true;
}

/*inline*/ bool PKMath::IntersectsRayBox(const double start[3], const double dir[3], 
										 const double planeXYa[4], const double planeXYb[4], const double planeYZa[4], const double planeYZb[4], const double planeXZa[4], const double planeXZb[4], 
										 double* intersection, double *distance) 
{

	double intersectionLocal[4];
	intersectionLocal[3] = 1;
	double t;
	bool found = false;
	double tMin = 1E99;

	// XY plane
	if (PKMath::GetPlaneLineIntersection(planeXYa, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeYZa, intersectionLocal) * PKUtils::Dot4(planeYZb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXZa, intersectionLocal) * PKUtils::Dot4(planeXZb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	if (PKMath::GetPlaneLineIntersection(planeXYb, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeYZa, intersectionLocal) * PKUtils::Dot4(planeYZb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXZa, intersectionLocal) * PKUtils::Dot4(planeXZb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	// XZ plane
	if (PKMath::GetPlaneLineIntersection(planeXZa, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeYZa, intersectionLocal) * PKUtils::Dot4(planeYZb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXYa, intersectionLocal) * PKUtils::Dot4(planeXYb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	if (PKMath::GetPlaneLineIntersection(planeXZb, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeYZa, intersectionLocal) * PKUtils::Dot4(planeYZb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXYa, intersectionLocal) * PKUtils::Dot4(planeXYb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	// YZ plane
	if (PKMath::GetPlaneLineIntersection(planeYZa, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeXYa, intersectionLocal) * PKUtils::Dot4(planeXYb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXZa, intersectionLocal) * PKUtils::Dot4(planeXZb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	if (PKMath::GetPlaneLineIntersection(planeYZb, start, dir, intersectionLocal, &t) && t >= -VERTEX_HIT_EPSILON) {

		// test inside?
		double signum1 = PKUtils::Dot4(planeXYa, intersectionLocal) * PKUtils::Dot4(planeXYb, intersectionLocal);
		double signum2 = PKUtils::Dot4(planeXZa, intersectionLocal) * PKUtils::Dot4(planeXZb, intersectionLocal);

		if (signum1 <= -VERTEX_HIT_EPSILON && signum2 <= -VERTEX_HIT_EPSILON) {
			if (intersection == NULL) {
				return true;
			}

			found = true;

			if (t < tMin) {
				tMin = t;
				PKUtils::CopyVertex(intersectionLocal, intersection);
			}
		}
	}

	if (!found) {
		return false;
	}

	if (intersection != NULL) {
		for (int j = 0; j < 3; j++) {
			intersection[j] = intersectionLocal[j];
		}
	}

	if (distance != NULL) {
		*distance = tMin;
	}

	return true;
}