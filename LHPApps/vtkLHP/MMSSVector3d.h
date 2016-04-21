/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSVector3d.h,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:45:53 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include "glew.h"
#include <math.h>

struct Vector{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct Vector4D{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
};

class MMSSVector3d {
public:
	GLfloat x;
	GLfloat y;
	GLfloat z;

public :	
	/*
	Get distance between v1 and v2
	*/
	static float VectorLength(MMSSVector3d v1, MMSSVector3d v2);
	/*
	Get length of vector v
	*/
	static float VectorLength(MMSSVector3d v);
	/*
	Substract vector sub from vector subFrom
	*/
	static MMSSVector3d VectorSub(MMSSVector3d subFrom, MMSSVector3d sub);
	/*
	Scale vector by mult value
	*/
	static void ConstMult(MMSSVector3d* vector, float mul);
	/*
	Add vector v2 to vector v1
	*/
	static MMSSVector3d VectorAdd(MMSSVector3d v1, MMSSVector3d v2);
	/*
	Cross product of two vectors.
	*/
	static MMSSVector3d VectorCross(MMSSVector3d v1, MMSSVector3d v2);
	/*
	Get normalize coordinates of vector v1
	*/
	static MMSSVector3d Normalize(MMSSVector3d v1);
	/*
	Transform vector v by transformation matrix (16 double values by cols)
	*/
	static MMSSVector3d Transform(MMSSVector3d v, GLdouble* m);
};

//define operators for easier computing with Vectors

//scale vector by value
MMSSVector3d operator*(const float mult, const MMSSVector3d &v);

//dot product
float operator*(const MMSSVector3d &v1, const MMSSVector3d &v2);

MMSSVector3d operator+(const MMSSVector3d &v1, const MMSSVector3d &v2);

MMSSVector3d operator-(const MMSSVector3d &v1, const MMSSVector3d &v2);

//comparing
bool operator==(const MMSSVector3d &v1, const MMSSVector3d &v2);