/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MassSpringSystemCPU.cpp,v $ 
  Language: C++ 
  Date: $Date: 2012-04-11 19:12:42 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
////////////////////////////////////////////////////////////////  
// This file contains:
// MassSpringSystemCPU class, render points and springs
// This is the CPU counting way to count particle system
////////////////////////////////////////////////////////////////

#include "MassSpringSystemCPU.h"
#include <stdio.h>
#include <time.h>

/*
Create and initialize new vertices and springs for spring system
- vertices are all coordinates in one dimensional field x,y,z,x,y,z...
- springs contain all springs
- count contain count of vertices and springs
*/
MassSpringSystemCPU::MassSpringSystemCPU(MMSSVector3d* vertices, Spring* springs, Counts count)
{
	Initialize(vertices, springs, count);
}


/*
Create and initialize new vertices and springs for spring system
	- vertices is array of vertices positions
	- springs is array of springs
	- count contains count of springs and vertices
	- radiuses contains the radiuses of the arrays - used to determine maximum allowed displacement of particles per iteration
	- boundary contains information about which particles are on the boundary and therefore take part in collision detection - used to determine maximum allowed displacement of particles per iteration
	- number of particels on the boundary, i.e. the length of the "boundary" array
	*/
MassSpringSystemCPU::MassSpringSystemCPU(MMSSVector3d* vertices, Spring* springs, Counts count, double *radiuses, int *boundary, int noOfBoundaryParticles)
{
	Initialize(vertices, springs, count);
	this->radiuses = radiuses;
	this->boundary = boundary;
	this->noOfBoundaryParticles = noOfBoundaryParticles;
}

/*
Initializes the data
*/
void MassSpringSystemCPU::Initialize(MMSSVector3d* vertices, Spring* springs, Counts count)
{
	minStep = DEFAULT_MSS_MIN_STEP;
	timeReserve = 0;
	
	dampConst = 1;
	weight = 1;
	
	realTimebuffer = 0;
	iterationCount = 0;

	this->springs = springs;
	this->vertices = new MMSSVector3d[count.verticesCount];
	this->forces = new MMSSVector3d[count.verticesCount];
	this->verticesFlags = new bool[count.verticesCount];
	this->count = count;

	//set previous vertices position to the same value as now
	this->vertices_prev = new MMSSVector3d[count.verticesCount];
	for(unsigned int i = 0; i < count.verticesCount; i++)
	{
		vertices_prev[i].x = vertices[i].x;
		vertices_prev[i].y = vertices[i].y;
		vertices_prev[i].z = vertices[i].z;

		this->vertices[i].x = vertices[i].x;
		this->vertices[i].y = vertices[i].y;
		this->vertices[i].z = vertices[i].z;

		verticesFlags[i] = 0;
	}
}

/*
release alocated memory
*/
MassSpringSystemCPU::~MassSpringSystemCPU(void)
{
	delete forces;
	delete vertices_prev;
	delete verticesFlags;
	delete vertices;
}

/*
move points to new positions
-indices means how vertices will be moved (index equal index from vertex array in constructor)
-positions is one dimensional array of position in indices order(x,y,z,x,y,z...)
*/
void  MassSpringSystemCPU::MovePoints(unsigned int* indices,MMSSVector3d* positions, unsigned int size)
{
	//set new positions to all gived vertices indices			
	for(unsigned int i = 0; i < size; i++)
	{
		if(indices[i] < this->count.verticesCount)
		{
			vertices[indices[i]] = positions[indices[i]];
			vertices_prev[indices[i]] = positions[indices[i]];
		}
	}
}

/*
count vertices position in next step
- time means time diference between last time calling and now calling
*/
void  MassSpringSystemCPU::NextStep(float time)
{
	float force;
	float len;
	MMSSVector3d temp;
	float actTime = this->minStep - this->timeReserve;
	
	clock_t init = clock();

	//step with minStep 
	while(actTime < time)
	{
		//set zero in force array
		for(unsigned int i = 0; i < count.verticesCount; i++)
		{
			forces[i].x = 0;
			forces[i].y = 0;
			forces[i].z = 0;
		}

		for(unsigned int i = 0; i < count.springsCount; i++)
		{
			//count force in spring
			len = MMSSVector3d::VectorLength(vertices[springs[i].p1_index],vertices[springs[i].p2_index]);
			force = springs[i].stiffness * (len - springs[i].initLen)/len; 
			temp = force * (1/len) * (vertices[springs[i].p2_index] - vertices[springs[i].p1_index]);
			
			//add force to one end point
			forces[springs[i].p1_index] = forces[springs[i].p1_index] + temp;
			
			//add negative force to second end point
			temp = (-1) * temp;
			forces[springs[i].p2_index] = forces[springs[i].p2_index] + temp;
		
		}

		float dt = minStep;
		
		//apply forces to point
		for(unsigned int i = 0; i < count.verticesCount; i++)
		{
			if(verticesFlags[i]==0)
			{
				temp = vertices[i];
				//time integration 
				vertices[i] = 2*vertices[i] - vertices_prev[i] + ((dt * dt / this->weight) * (forces[i] -  ((dampConst/dt) * (vertices[i] - vertices_prev[i]))));
				//set prev position to the position before applying forces
				vertices_prev[i] = temp;
			}
			else
			{
				vertices_prev[i] = vertices[i];
			}
		}

		actTime += minStep;
		iterationCount++;
	}

	//if there is some time reserve save it
	timeReserve = time - (actTime - minStep);
	realTimebuffer += ((float)(clock()-init))/((float)CLOCKS_PER_SEC);
}

/*
Computes forces that should be applied on the particles and computes maximal "safe" dt
so that no particle will be moved by more than its radius.
*/
void  MassSpringSystemCPU::NextStepForces()
{
	float force;
	float len;
	MMSSVector3d temp;
	//set zeros in the force array
	for(unsigned int i = 0; i < count.verticesCount; i++)
	{
		forces[i].x = 0;
		forces[i].y = 0;
		forces[i].z = 0;
	}

	for(unsigned int i = 0; i < count.springsCount; i++)
	{
		//count force in spring
		len = MMSSVector3d::VectorLength(vertices[springs[i].p1_index],vertices[springs[i].p2_index]);
		force = springs[i].stiffness * (len - springs[i].initLen)/len; 
		temp = force * (1/len) * (vertices[springs[i].p2_index] - vertices[springs[i].p1_index]);
			
		//add force to one end point
		forces[springs[i].p1_index] = forces[springs[i].p1_index] + temp;
			
		//add negative force to second end point
		temp = (-1) * temp;
		forces[springs[i].p2_index] = forces[springs[i].p2_index] + temp;		
	}

	// computes maximal allowed radius
	minStep = 999999;
	for (unsigned int i = 0; i < noOfBoundaryParticles; i++)
	{
		 // if it is not fixed, it must not move by more than its radius in order to prevent "tunelling" during collisions
		if (!verticesFlags[i] && radiuses[i] > 0)
		{
			int index = boundary[i];
			// newPos = 2*vertices[i] - vertices_prev[i] + ((dt * dt / this->weight) * (forces[i] -  ((dampConst/dt) * (vertices[i] - vertices_prev[i]))));
			// it must not move more than radius -> Length(newPos - vertices[i]) <= radius
			// -> radius >= Length(vertices[i] - vertices_prev[i]) + ((dt * dt / this->weight) * (Length(forces[i]) -  ((dampConst/dt) * Length(vertices[i] - vertices_prev[i]))));
			// ->set up quadratic equation and solve for dt
			float c = (float)((MMSSVector3d::VectorLength(vertices[index] - vertices_prev[index]) - radiuses[index])  * this->weight);
			float a = MMSSVector3d::VectorLength(forces[index]);
			float b = -dampConst * MMSSVector3d::VectorLength(vertices[index] - vertices_prev[index]);
			float discsqrt = sqrt(b*b - 4 * a *c);
			float t = (-b + discsqrt) / (2 * a); // 2*a will always be positive -> this is the higher root
			if (t < minStep)
				minStep = t;
		}
	}
	if (minStep < DEFAULT_MSS_MIN_STEP || minStep == 999999)
		minStep = DEFAULT_MSS_MIN_STEP;
}

/*
Computes position for vertices. Do not compute the forces - make sure NextStepForces is called before this!
*/
void  MassSpringSystemCPU::NextStepPositions()
{
	float dt = minStep;
	MMSSVector3d temp;
	//apply forces to point
	for(unsigned int i = 0; i < count.verticesCount; i++)
	{
		if(verticesFlags[i]==0)
		{
			temp = vertices[i];
			//time integration 
			vertices[i] = 2*vertices[i] - vertices_prev[i] + ((dt * dt / this->weight) * (forces[i] -  ((dampConst/dt) * (vertices[i] - vertices_prev[i]))));
			//set prev position to the position before applying forces
			vertices_prev[i] = temp;
		}
		else
		{
			vertices_prev[i] = vertices[i];
		}
	}
}

/*
Method for giving actual positions of vertices.
*/
void MassSpringSystemCPU::GetVertices(MMSSVector3d* positions, unsigned int size)
{
	for(unsigned int i = 0; i < size; i++)
	{
		positions[i] = this->vertices[i];
	}
}

/*
Set fixed points, which will not be actualized, none force will be applicate.
All points, which indices in parameter will be set as fixed points, other points will be free.
Actualize whole array of point and spring -> better option for lot of points.
*/
void MassSpringSystemCPU::SetFixedPoints(unsigned int* indices,MMSSVector3d* positions,unsigned int size)
{
	for(unsigned int i = 0; i < count.verticesCount; i++)
	{
		verticesFlags[i] = 0;
	}

	for(unsigned int i = 0; i < size; i++)
	{
		verticesFlags[indices[i]] = 1;
	}

	//in the end move points because we need set prev positions and act positions to same values
	//if not and prev positions is diferent -> moving equation will be continue
	MovePoints(indices,positions,size);
}

/*
Method for setting coefficients
*/
void MassSpringSystemCPU::SetCoefs(float damping, float weight)
{
	if(damping <= 0 || weight <= 0)return;
	this->dampConst = damping;
	this->weight = weight;
}

/*
Get iteration count and time of all iterations
*/
void MassSpringSystemCPU::GetTimeIteration(int* iteration, float* time)
{
	*iteration = this->iterationCount;
	*time =	this->realTimebuffer;
}