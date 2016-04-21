/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSVector3d.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:45:53 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
////////////////////////////////////////////////////////////////  
// This file contains:
// MMSSVector3d class -> contain definition of vetor struct(header file)
// all useful operation with it
////////////////////////////////////////////////////////////////
#include "MMSSVector3d.h"

/*
count distance between two vectors
*/
float MMSSVector3d::VectorLength(MMSSVector3d v1, MMSSVector3d v2)
{
	float dx,dy,dz;
	dx = v2.x - v1.x;
	dy = v2.y - v1.y;
	dz = v2.z - v1.z;

	return sqrtf(dx*dx + dy*dy + dz*dz);
}

/*
count length of vector
*/
float MMSSVector3d::VectorLength(MMSSVector3d v)
{
	return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

/*
count sub of vectors
*/
MMSSVector3d MMSSVector3d::VectorSub(MMSSVector3d subFrom, MMSSVector3d sub)
{
	MMSSVector3d v;
	v.x = subFrom.x - sub.x;
	v.y = subFrom.y - sub.y;
	v.z = subFrom.z - sub.z;

	return v;
}

/*
count add of two vectors
*/
MMSSVector3d MMSSVector3d::VectorAdd(MMSSVector3d v1, MMSSVector3d v2)
{
	MMSSVector3d v;
	v.x = v1.x + v2.x;
	v.y = v1.y + v2.y;
	v.z = v1.z + v2.z;

	return v;
}

/*
scale vector by mul
*/
void MMSSVector3d::ConstMult(MMSSVector3d* vector,float mul)
{
	vector->x *= mul;
	vector->y *= mul;
	vector->z *= mul;
}

/*
count cross product of two vectors
*/
MMSSVector3d MMSSVector3d::VectorCross(MMSSVector3d v1, MMSSVector3d v2)
{
	MMSSVector3d v;
	
	v.x = v1.y*v2.z - v1.z*v2.y;
	v.y = v1.z*v2.x - v1.x*v2.z;
	v.z = v1.x*v2.y - v1.y*v2.x;

	return v;
}

/*
return normalized vector
*/
MMSSVector3d MMSSVector3d::Normalize(MMSSVector3d v1)
{
	MMSSVector3d v;
	
	v = (1/MMSSVector3d::VectorLength(v1)) * v1;

	return v;
}

MMSSVector3d MMSSVector3d::Transform(MMSSVector3d v, GLdouble* m)
{
	MMSSVector3d r;
	float h;
	r.x = (GLfloat)(v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12]);
	r.y = (GLfloat)(v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13]);
	r.z = (GLfloat)(v.x * m[2] + v.y * m[6] + v.z * m[10] + m[14]);
	h = (GLfloat)(v.x * m[3] + v.y * m[7] + v.z * m[11] + m[15]);

	return r;
}

/*
scale vector by mul, overloaded operator *
*/
MMSSVector3d operator*(float mul, const MMSSVector3d &v)
{
	MMSSVector3d vector;
	vector.x = mul * v.x;
	vector.y = mul * v.y;
	vector.z = mul * v.z;
   
	return vector;
}

/*
Dot product of two vectors
*/
float operator*(const MMSSVector3d &v1, const MMSSVector3d &v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

/*
count add of two vectors, overloaded operator +
*/
MMSSVector3d operator+(const MMSSVector3d &v1, const MMSSVector3d &v2)
{
   MMSSVector3d result;
   result.x = v1.x + v2.x;
   result.y = v1.y + v2.y;
   result.z = v1.z + v2.z;
   
   return result;
}

/*
count sub of two vectors, overloaded operator -
*/
MMSSVector3d operator-(const MMSSVector3d &v1, const MMSSVector3d &v2)
{
   MMSSVector3d result;
   result.x = v1.x - v2.x;
   result.y = v1.y - v2.y;
   result.z = v1.z - v2.z;
   
   return result;
}

/*
If vectors have same vals as coordinates return true, otherwise false.
*/
bool operator==(const MMSSVector3d &v1, const MMSSVector3d &v2)
{
	if(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)return true;
	else return false;
}
 
