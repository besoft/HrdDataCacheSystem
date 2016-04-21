/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSUtil.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:45:53 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#include "MMSSUtil.h"

//implemented function for changing view matrix (look from eye to center and upVector define rotation of camera)
void glLookAt( MMSSVector3d eyePosition, MMSSVector3d center, MMSSVector3d upVector, float* matrix2 )
{
	MMSSVector3d forward, forwardB, side, up;
   
   forwardB = center - eyePosition;
   
   //normalize
   forward = MMSSVector3d::Normalize(forwardB);

   side = MMSSVector3d::VectorCross(forward, upVector);
   side = MMSSVector3d::Normalize(side);
   
   //Recompute up as: up = side x forward
   up = MMSSVector3d::VectorCross(side, forward);
   
   //M = ( s [ 0 ] s [ 1 ] s [ 2 ] 0 u [ 0 ] u [ 1 ] u [ 2 ] 0 -f [ 0 ] -f [ 1 ] -f [ 2 ] 0 0 0 0 1 )
   //------------------
   matrix2[0] = side.x;
   matrix2[4] = side.y;
   matrix2[8] = side.z;
   matrix2[12] = 0.0;
   //------------------
   matrix2[1] = up.x;
   matrix2[5] = up.y;
   matrix2[9] = up.z;
   matrix2[13] = 0.0;
   //------------------
   matrix2[2] = -forward.x;
   matrix2[6] = -forward.y;
   matrix2[10] = -forward.z;
   matrix2[14] = 0.0;
   //------------------
   matrix2[3] = -eyePosition.x;
   matrix2[7] = -eyePosition.y;
   matrix2[11] = -eyePosition.z;
   matrix2[15] = 1.0;
   //------------------
   
}

/*
rotation around userlike axis
- normalizedDirection is normalized direction vector of axis
- phi is angle in radians
- m is array of 16 elements for matrix stored in rows
*/
void RotationAroundAxis(MMSSVector3d n, double phi, double* m)
{
	double c = cos(phi);
	double s = sin(phi);
	double oneSubC = 1 - c;

	double nXnY = n.x*n.y*oneSubC;
	double nXnZ = n.x*n.z*oneSubC;
	double nYnZ = n.y*n.z*oneSubC;

	//-------------------------
	m[0] = c + n.x*n.x*oneSubC;
	m[1] = nXnY - n.z*s;
	m[2] = nXnZ + n.y*s;
	m[3] = 0;
	//-------------------------
	m[4] = nXnY + n.z*s;
	m[5] = c + n.y*n.y*oneSubC;
	m[6] = nYnZ - n.x*s;
	m[7] = 0;
	//-------------------------
	m[8] = nXnZ - n.y*s;
	m[9] = nYnZ + n.x*s;
	m[10] = c + n.z*n.z*oneSubC;
	m[11] = 0;
	//-------------------------
	m[12] = m[13] = m[14] = 0;
	m[15] = 1;
}

/*
Count barycentric coords of point VS polygon.
*/
void GetBarycentricCoord(MMSSVector3d* pointsOfPolygon, unsigned int pointsSize, MMSSVector3d point, float* coords)
{
	if(pointsSize <= 2)return;

	float sum = 0;
	for(int i = 0; i < (int)pointsSize; i++)
	{
		coords[i] = MMSSVector3d::VectorLength(MMSSVector3d::VectorCross(pointsOfPolygon[i]-pointsOfPolygon[(i-1)%pointsSize],pointsOfPolygon[(i+1)%pointsSize]-pointsOfPolygon[(i-1)%pointsSize]));
		for(int j = (i + 2)%pointsSize; j != i; j = (j + 1)%pointsSize)
		{
			coords[i] *= MMSSVector3d::VectorLength(MMSSVector3d::VectorCross(pointsOfPolygon[j]-pointsOfPolygon[(j-1)%pointsSize],point-pointsOfPolygon[(j-1)%pointsSize]));
		}
		sum += coords[i];
	}

	for(int i = 0; i < (int)pointsSize; i++)
	{
		coords[i] = coords[i] / sum;
	}
}

/*
Get AABB box of data.
*/
void GetMinMaxBox(MMSSVector3d* points, unsigned int pointsSize, MMSSVector3d* leftDownCorner, MMSSVector3d* rightUpCorner)
{
	float xMin = points[0].x;
	float xMax = points[0].x;
	float yMin = points[0].y;
	float yMax = points[0].y;
	float zMin = points[0].z;
	float zMax = points[0].z;

	//go through vertices and find minmax box
	for(unsigned int i = 1; i < pointsSize; i++)
	{
		if(points[i].x < xMin)
		{
			xMin = points[i].x;
		}
		else if(points[i].x > xMax)
		{
			xMax = points[i].x;
		}

		if(points[i].y < yMin)
		{
			yMin = points[i].y;
		}
		else if(points[i].y > yMax)
		{
			yMax = points[i].y;
		}

		if(points[i].z < zMin)
		{
			zMin = points[i].z;
		}
		else if(points[i].z > zMax)
		{
			zMax = points[i].z;
		}
	}

	leftDownCorner->x = xMin;
	leftDownCorner->y = yMin;
	leftDownCorner->z = zMin;

	rightUpCorner->x = xMax;
	rightUpCorner->y = yMax;
	rightUpCorner->z = zMax;
}
