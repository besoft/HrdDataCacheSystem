/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MassSpringSystemCPU.h,v $ 
  Language: C++ 
  Date: $Date: 2012-04-29 20:37:13 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Ivo Zelený, Tomas Janák
  ========================================================================== 
  Copyright (c) 2011-2012 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#pragma once


#include "IMassSpringSystem.h"

#define DEFAULT_MSS_MIN_STEP 0.04f

class MassSpringSystemCPU : public IMassSpringSystem
{
public:
	
	/*
	Constructor
	- vertices is array of vertices positions
	- springs is array of springs
	- count contains count of springs and vertices
	*/
	MassSpringSystemCPU(MMSSVector3d* vertices, Spring* springs, Counts count);//default constructor
	
	/*
	Constructor
	- vertices is array of vertices positions
	- springs is array of springs
	- count contains count of springs and vertices
	- radiuses contains the radiuses of the arrays - used to determine maximum allowed displacement of particles per iteration
	- boundary contains information about which particles are on the boundary and therefore take part in collision detection - used to determine maximum allowed displacement of particles per iteration
	- number of particels on the boundary, i.e. the length of the "boundary" array
	*/
	MassSpringSystemCPU(MMSSVector3d* vertices, Spring* springs, Counts count, double *radiuses, int *boundary, int noOfBoundaryParticles);

private:
	/*
	Initializes the data
	*/
	void MassSpringSystemCPU::Initialize(MMSSVector3d* vertices, Spring* springs, Counts count);

public:

	/*
	Destructor. Release all alocated memory.
	*/
	~MassSpringSystemCPU(void);

	/*
	Move points to new positions.
	- 'indices' means how vertices will be moved (index equal index from vertex array in constructor)
	- 'positions' is one dimensional array of position in indices order(x,y,z,x,y,z...)
	- 'size' is size of both arrays
	*/
	virtual void MovePoints(unsigned int* indices,MMSSVector3d* positions, unsigned int size);
	
	/*
	Set fixed points. Fixed points are points, which will not be actualized (none force will be applicate).
	- all points, which indices are set in parameter 'indices' will be set as fixed points, other points will be free.
	- 'positions' is array of 'size' elements -> it means fixed positions of points, which indices is set in param 'indices'
	*/
	virtual void SetFixedPoints(unsigned int* indices,MMSSVector3d* positions,unsigned int size);
	
	/*
	Computes vertices position in next step
	- parameter 'time' means time diference between last state of MSS and new required state
	*/
	virtual void NextStep(float time);
	
	/*
	Computes forces that should be applied on the particles and computes maximal "safe" dt
	so that no particle will be moved by more than its radius.
	*/
	virtual void NextStepForces();

	/*
	Computes position for vertices. Do not compute the forces - make sure NextStepForces is called before this!
	*/
	virtual void NextStepPositions();
	
	/*
	Get actual positions of all points.
	- parameter 'positions' is array of 'size' points, where positions will be stored.
	*/
	virtual void GetVertices(MMSSVector3d* positions, unsigned int size);
	
	/*
	Method for setting parameters of mass spring system
	- all parameters must be greater than 0
	*/
	virtual void SetCoefs(float damping, float weight);
	
	/*
	Method for getting some inner counters as iterations which were done and time spended in counting iterations
	*/
	virtual void GetTimeIteration(int* iteration, float* time);

	/*
	Returns the array of vertex flags (denoting whether a given vertex is fixed or not)
	*/
	bool *getVerticesFlags() { return this->verticesFlags; }

	/*
	Retturns the pointer to internal array that contains positions of the vertices of the MSS.
	!Modifying its content can result in faulty simulation!
	*/
	MMSSVector3d *getVertexArrayPointer() { return this->vertices; }

	/*
	Returns the pointer to internal array that contains positions of the vertices of the MSS in previous simulation step.
	!Modifying its content can result in faulty simulation!
	*/
	MMSSVector3d *getVertexPrevArrayPointer() { return this->vertices_prev; }
	
	/*
	Returns the ifinetisimal time step dt used for integrating the system
	*/
	float getDt() { return this->minStep; }

	/*
	Sets the ifinetisimal time step dt used for integrating the system
	*/
	void setDt(float dt) { this->minStep = dt; }
	
private:
	float minStep;//minimal step of simulation
	
	float dampConst;//damping of springs
	float weight;//weight of points

	//for saving consumed time and iteration count
	float realTimebuffer;
	int iterationCount;

	MMSSVector3d* vertices;//actual positions
	MMSSVector3d* vertices_prev;//for storing previous positions
	double *radiuses; // stores the radiuses of the particles
	int *boundary; // holds indices of particles which are on the boundary
	int noOfBoundaryParticles;
	Spring* springs;//for storing springs
	MMSSVector3d* forces;//for storing forces
	bool* verticesFlags;//1 means fixed, 0 means free
	Counts count;//saving count of points, springs

	float timeReserve;
};
