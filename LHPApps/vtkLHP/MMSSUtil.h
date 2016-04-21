/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSUtil.h,v $ 
  Language: C++ 
  Date: $Date: 2011-11-11 09:09:08 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#pragma once

#include "MMSSVector3d.h"
//using namespace std;

void glLookAt( MMSSVector3d eyePosition, MMSSVector3d center, MMSSVector3d upVector );

/*
rotation around userlike axis
- normalizedDirection is normalized direction vector of axis
- phi is angle in radians
- m is array of 16 elements for matrix stored in rows
*/
void RotationAroundAxis(MMSSVector3d normalizedDirection, double phi, double* m);

/*
Count barycentric coords of point VS polygon.
*/
void GetBarycentricCoord(MMSSVector3d* pointsOfPolygon, unsigned int pointsSize, MMSSVector3d point, float* coords);


/*
Get min-max box
*/
void GetMinMaxBox(MMSSVector3d* points, unsigned int pointsSize, MMSSVector3d* leftDownCorner, MMSSVector3d* rightUpCorner);