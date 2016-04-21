/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: IMassSpringSystem.h,v $ 
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

#include "MMSSVector3d.h"

/*
Interface for Mass Spring System classes
*/
class IMassSpringSystem
{
public:
	struct Spring{
		unsigned int p1_index, p2_index;	// end points of spring.
		float stiffness;     // stiffness of spring.
		float initLen;		// init spring length
	};

	struct Counts{
		unsigned int verticesCount;	// count of vertices
		unsigned int springsCount;    // count of springs.
		unsigned int meshPointsCount; // count of points which lies on bounded planes
	};
public:
   
	/* virtual dtor*/
	virtual ~IMassSpringSystem() {
	}
		/*
		Move points to new positions.
		- 'indices' means how vertices will be moved (index equal index from vertex array in constructor)
		- 'positions' is one dimensional array of position in indices order(x,y,z,x,y,z...)
		- 'size' is size of both arrays
		*/
		virtual void MovePoints(unsigned int* indices,MMSSVector3d* positions, unsigned int size) = 0;
		
		/*
		Set fixed points. Fixed points are points, which will not be actualized (none force will be applicate).
		- all points, which indices are set in parameter 'indices' will be set as fixed points, other points will be free.
		- 'positions' is array of 'size' elements -> it means fixed positions of points, which indices is set in param 'indices'
		*/
		virtual void SetFixedPoints(unsigned int* indices,MMSSVector3d* positions,unsigned int size) = 0;
		
		/*
		Count vertices position in next step
		- parameter 'time' means time diference between last state of MSS and new required state
		*/
		virtual void NextStep(float time) = 0;
		
		/*
		Get actual positions of all points.
		- parameter 'positions' is array of 'size' points, where positions will be stored.
		*/
		virtual void GetVertices(MMSSVector3d* positions, unsigned int size) = 0;
		
		/*
		Method for setting parameters of mass spring system
		- all parameters must be greater than 0
		*/
		virtual void SetCoefs(float damping, float weight) = 0;
		
		/*
		Method for getting some inner counters as iterations which were done and time spended in counting iterations
		*/
		virtual void GetTimeIteration(int* iteration, float* time) = 0;
};