/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper_Core.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.2.22 $
Authors:   Tomas Janak
==========================================================================
Copyright (c) 2012 University of University of West Bohemia
See the COPYINGS file for license details 
=========================================================================
Contains classes and structures needed for the mass spring system representation of muscles.
=========================================================================*/

#include "mafDefines.h" 
#include <limits>
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkMassSpringMuscle.h"
#include "vtkObjectFactory.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"
#include "medVMEMuscleWrapper.h"
#include "vtkDelaunay3D.h"
#include "vtkMAFMeanValueCoordinatesInterpolation.h"
#include "vtkGeometryFilter.h"
#include "vtkPointLocator.h"
#include "vtkGenericCell.h"
#include "vtkMath.h"

#include "mafMemDbg.h"
#include "mafDbg.h"

using namespace std;

#pragma region vtkMSSDataSet

vtkStandardNewMacro(vtkMSSDataSet);

/** Constructor */
vtkMSSDataSet::vtkMSSDataSet()
{
	closestParticles = new int*[NO_OF_CONTROL_PARTICLES];
	for (int i = 0; i < NO_OF_CONTROL_PARTICLES; i++)
		closestParticles[i] = NULL;
	particles = NULL;
	radiuses = NULL;
	bone2IDs = NULL;
	boneWeights = NULL;
	springs = NULL;
	fixedPointIndices = NULL;
	boundaryParticlesMap = NULL;
	noOfFixedPoints = 0;
	noOfParticles = 0;
	noOfSprings = 0;
	noOfVertices = 0;
	noOfBoundaryParticles = 0;
	MVCWeights = NULL;
	BoundaryTriangulation = NULL;
}

/** Desturctor */
vtkMSSDataSet::~vtkMSSDataSet()
{
	DeallocateMemory();
}

void vtkMSSDataSet::DeallocateMemory()
{
	delete []particles;
	delete []radiuses;
	delete []bone2IDs;
	delete []boneWeights;
	delete []springs;
	for (int i = 0; i < NO_OF_CONTROL_PARTICLES; i++)
		delete []closestParticles[i];
	delete []closestParticles;
	delete []fixedPointIndices;
	delete []boundaryParticlesMap;	
	vtkDEL(MVCWeights);
	vtkDEL(BoundaryTriangulation);
}

/** Creates a shallow copy of the object */
void vtkMSSDataSet::DeepCopy(vtkMSSDataSet *source)
{
	Superclass::DeepCopy(source);
	DeallocateMemory();
	this->noOfParticles = source->noOfParticles;
	this->noOfSprings = source->noOfSprings;
	this->noOfFixedPoints = source->noOfFixedPoints;
	this->noOfVertices = source->noOfVertices;
	this->noOfBoundaryParticles = source->noOfBoundaryParticles;
	this->particles = new MMSSVector3d[noOfParticles]; // an array of all the particles in this dataset
	this->radiuses = new double[noOfParticles];
	this->bone2IDs = new int[noOfParticles * 2];
	this->boneWeights = new double[noOfParticles];
	this->boundaryParticlesMap = new int[noOfParticles];
	for (int i = 0; i < noOfParticles; i++)
	{
		particles[i] = source->particles[i];
		radiuses[i] = source->radiuses[i];
		bone2IDs[i * 2] = source->bone2IDs[i * 2];
		bone2IDs[i * 2 + 1] = source->bone2IDs[i * 2 + 1];
		boneWeights[i] = source->boneWeights[i];
		boundaryParticlesMap[i] = source->boundaryParticlesMap[i];
	}
	this->closestParticles = new int*[NO_OF_CONTROL_PARTICLES]; // Array of "NO_OF_CONTROL_PARTICLES" arrays, each containing "number-of-vertices" indices of particles
	//	i.e. on closestParticles[i][j] is the i-th control particle of j-th vertex
	for (int i = 0; i < NO_OF_CONTROL_PARTICLES; i++)
	{
		closestParticles[i] = new int[noOfVertices];
		for (int j = 0; j < noOfVertices; j++)
			closestParticles[i][j] = source->closestParticles[i][j];
	}
	this->springs = new IMassSpringSystem::Spring[noOfSprings]; // an array of all springs in the dataset
	for (int i = 0; i < noOfSprings; i++)
		springs[i] = source->springs[i];
	this->fixedPointIndices = new unsigned int[noOfFixedPoints]; // indices of fixed points of the particle system;
	for (int i = 0; i < noOfFixedPoints; i++)
		fixedPointIndices[i] = source->fixedPointIndices[i];
}
#pragma endregion

#pragma region SphereBoundingBox
// Constructor
// PARAM:
// parent - A pointer to a parent bounding box
// spheres - Array of spheres that belong to this bounding box 
// radiuses - An array of radiuses of the spheres.
SphereBoundingBox::SphereBoundingBox(SphereBoundingBox *parent, MMSSVector3d *spheres, double* radiuses)
{
	for (int i = 0; i < 8; i++)
		children[i] = NULL;
	this->parent = parent;
	this->noOfSpheres = 0;
	this->spheres = spheres;
	this->radiuses = radiuses;
	if (parent != NULL)			// new children, set it as active
		this->queryNumber = parent->GetQueryNumber();
	else						// root, start counting from 0
		this->queryNumber = 0;
	this->hasValidChildren = false;
}

// Destructor
SphereBoundingBox::~SphereBoundingBox()
{
	for (int i = 0; i < 8; i++)
	{
		if (children[i] != NULL)
		{
			delete children[i];
			children[i] = NULL;
		}
	}
}

// Copies ids of all spheres from given array into the internal structure of the bounding box
// PARAM:
// contentArray - An array with all the spheres that should be inserted into this bounding box
// radiuses - An array of radiuses of the spheres from content array.
// boundary - Array with the indices of boundary particles. Only boundary particles will be used for collisions.
// count - The number of items in the array
void SphereBoundingBox::SetContentFromArray(MMSSVector3d *contentArray, double* radiuses, int* boundary, int count)
{
	this->noOfSpheres = count;
	this->content.resize(count);
	for (int i = 0; i < count; i++)
		content[i] = boundary[i];
	this->spheres = contentArray;
	this->radiuses = radiuses;
}

// Copies ids of all spheres from given array into the internal structure of the bounding box
// PARAM:
// contentArray - An array with all the spheres that should be inserted into this bounding box
// radiuses - An array of radiuses of the spheres from content array.
// count - The number of items in the array
void SphereBoundingBox::SetContentFromArray(MMSSVector3d *contentArray, double* radiuses, int count)
{
	this->noOfSpheres = count;
	this->content.resize(count);
	for (int i = 0; i < count; i++)
		content[i] = i;
	this->spheres = contentArray;
	this->radiuses = radiuses;
}


// Splits the bounding box in 8 regular smaller bounding boxes and distributes the content among them
void SphereBoundingBox::SplitRegularly()
{
	if (hasValidChildren) // if the bbox is marked as active, it is already splitted the way it should be
		return;
	// generate children bounding boxes in the following pattern (H mean high coordinate, L low coordinate)
	// 0 - LLL
	// 1 - LLH
	// 2 - LHL
	// 3 - LHH
	// 4 - HLL
	// 5 - HLH
	// 6 - HHL
	// 7 - HHH
	// create children
	double cooHalf[3];
	for (int i = 0; i < 3; i++)
		cooHalf[i] = cooMin[i] + ((cooMax[i] - cooMin[i]) / 2);
	for (int i = 0; i < 8; ++i) {
		delete children[i];
		children[i] = NULL;
	}
	for (int i = 0; i < 8; i++)
		children[i] = new SphereBoundingBox(this, this->spheres, radiuses);

	// distribute the spheres
	for (int i = 0; i < noOfSpheres; i++)
	{
		int childrenIndex = 0;
		// if they are in the upper half (H coordinates)
		// add bit '1' to appropriate place - will be index into the children array
		if (spheres[content[i]].x > cooHalf[0]) childrenIndex = 7;
		if (spheres[content[i]].y > cooHalf[1]) childrenIndex |= 2;
		if (spheres[content[i]].z > cooHalf[2]) childrenIndex |= 1;

		children[childrenIndex]->InsertSphere(content[i]);
	}

	//content.clear(); // delete all content from this node // comment to norebuild
	// delete children without any content, computes bounding boxes in the ones with content
	for (int i = 0; i < 8; i++)
	{
		if (children[i]->GetNumberOfSpheres() == 0)
		{
			delete children[i];
			children[i] = NULL;
		}
		else
			children[i]->ComputeBoundingBox();
	}
	hasValidChildren = true;
}

// Computes the boundary of this box based on its content.
void SphereBoundingBox::ComputeBoundingBox()
{
	// initialize the bounding box
	for (int i = 0; i < 3; i++)
	{
		cooMin[i] = std::numeric_limits<double>::max();
		cooMax[i] = -std::numeric_limits<double>::max();
	}

	// Compute the bounding box, taking into account the radius of the spheres
	double coos[3];
	for (int i = 0; i < noOfSpheres; i++)
	{
		coos[0] = spheres[content[i]].x; // coordinates of the center of the sphere
		coos[1] = spheres[content[i]].y; // coordinates of the center of the sphere
		coos[2] = spheres[content[i]].z; // coordinates of the center of the sphere
		for (int j = 0; j < 3; j++) // process x, y and z
		{
			if ((coos[j] + radiuses[content[i]]) > cooMax[j])
				cooMax[j] = coos[j] + radiuses[content[i]];
			if ((coos[j] - radiuses[content[i]]) < cooMin[j])
				cooMin[j] = coos[j] - radiuses[content[i]];
		}
	}
}

// Inserts the ID of a given sphere into this node.
// PARAM:
// sphere - the ID of the sphere to be inserted
void SphereBoundingBox::InsertSphere(int sphereID)
{
	this->content.push_back(sphereID);
	this->noOfSpheres++;
}

// Tests whether given bounding box collides (i.e. overlaps) with this one 
// PARAM:
// bbox - the test subject
// RETURN:
// True if the boxes ovelap, false otherwise
bool SphereBoundingBox::Collides(SphereBoundingBox *bbox)
{
	double *bboxMin = bbox->GetBoundingBoxMin();
	double *bboxMax = bbox->GetBoundingBoxMax();
	for (int i = 0; i < 3; i++)
	{
		if (bboxMax[i] < this->cooMin[i] || this->cooMax[i] < bboxMin[i])
			return false;
	}
	return true;
}

// Detect which parts of two bounding boxes collide with each other.
// PARAM:
// bbox - The bounding box that should be tested against this bounding box
// currentQueryNumber - the number of the current collisiong query
// reccursionCounter - Controls how many times can the bounding boxes be divided before piecewise (sphere vs. sphere) collision detection starts
// RETURN:
// List of pairs of indices of spheres that should be tested against each other
vector<vector<int>> SphereBoundingBox::DetectCollision(SphereBoundingBox *bbox, int currentQueryNumber, int reccursionCounter)
{
	vector<vector<int>> ret;

	if (currentQueryNumber > this->queryNumber)// norebuild
		this->ComputeBoundingBox();// norebuild
	if (bbox->GetQueryNumber() < currentQueryNumber)// norebuild
		bbox->ComputeBoundingBox();// norebuild
	// set as active
	this->queryNumber = currentQueryNumber;
	bbox->SetQueryNumber(currentQueryNumber);

	if (this->Collides(bbox)) // if the two bounding boxes collide with each other
	{
		reccursionCounter--;
		// if the reccursion is allowed to continue and there is still a lot of objects (so that the splitting can be useful), split the nodes
		if (reccursionCounter > 0 && this->noOfSpheres > MIN_SPHERES_TO_SPLIT && bbox->GetNumberOfSpheres() > MIN_SPHERES_TO_SPLIT)
		{
			// split the boxes
			this->SplitRegularly();
			bbox->SplitRegularly();
			// for each children
			for (int i = 0; i < 8; i++) 
			{
				SphereBoundingBox *children1 = this->children[i];
				if (children1 == NULL) continue;
				if (children1->GetQueryNumber() < currentQueryNumber) // norebuild
					children1->ComputeBoundingBox();// norebuild
				if (children1->Collides(bbox)) // if it does not collide with the parent box of "children2", it surely does not collide with any of the "children2"
				{
					for (int j = 0; j < 8; j++)
					{
						SphereBoundingBox *children2 = bbox->GetChildren(j);
						if (children2 == NULL) continue;
						// recursively detect collision
						vector<vector<int>> toAppend = children1->DetectCollision(children2, currentQueryNumber, reccursionCounter);
						// append the result to the vector that will be returned
						for (vector<vector<int>> ::const_iterator iter = toAppend.begin(); iter != toAppend.end(); iter++)
							ret.push_back(*iter);
					}
				}
			}
		}
		else // else set the content of these nodes to be tested piecewise, i.e. sphere vs sphere
		{
			ret.resize(2);
			// copy the "content" of both bounding boxes to the final vector
			if (this->content.size() == 0) // if there is no content in this node, it is in its children
				this->CollectContent(&(ret[0]));
			else
			{
				ret[0].resize(noOfSpheres);
				for (int i = 0; i < noOfSpheres; i++)
					ret[0][i] = this->content[i];
			}
			vector<int> bboxContent = bbox->GetContent();
			if (bboxContent.size() == 0)
				bbox->CollectContent(&(ret[1]));
			else
			{
				ret[1].resize(bbox->GetNumberOfSpheres());
				for (int i = 0; i < bbox->GetNumberOfSpheres(); i++)
					ret[1][i] = bboxContent[i];
			}
		}
	}

	return ret;
}

// Traverse the children bounding boxes, extracts the spheres stored in them.
// PARAM:
// completeContent - a pointer to a vector which will contain all the speheres stored in all the children bounding boxes
void SphereBoundingBox::CollectContent(std::vector<int> *completeContent)
{
	for (int i = 0; i < 8; i++)
	{
		if (children[i] != NULL)
			children[i]->CollectContent(completeContent);
	}
	// copy the elements in the content of this bbox to the wanted vector
	for (vector<int> ::iterator iter = this->content.begin(); iter != this->content.end(); iter++)
		completeContent->push_back(*iter);
}

// Remove all children it might have and refits its own bounding box
void SphereBoundingBox::Rebuild()
{
	for (int i = 0; i < 8; i++)
	{
		if (children[i] != NULL) // there might be inactive children - take their content and delete them
		{
			children[i]->CollectContent(&(this->content));
			delete children[i];
			children[i] = NULL;
		}
	}
	this->ComputeBoundingBox(); // refit the box
	this->hasValidChildren = false;
}

// Updates this bounding box and recursively its children based on the changes of the deformable object they encapsulate
// PARAM:
// queryNumber - how many queries were processed using this (or its root) bounding box
void SphereBoundingBox::Update(int queryNumber)
{
	// go to the bottom active node first - the update phase proceeds in a bottom-up fashion
	int noOfActiveChildren = 0;
	for (int i = 0; i < 8; i++)
	{
		if (children[i] != NULL && children[i]->IsActive(queryNumber))
		{
			children[i]->Update(queryNumber);
			noOfActiveChildren++;
		}
	}
	if (noOfActiveChildren == 0) // we are in the deepest active node
		Rebuild();
	else // compute the bounding box as a merge of the children boxes
	{
		double volumeOrig = (cooMax[0] - cooMin[0]) * (cooMax[1] - cooMin[1]) * (cooMax[2] - cooMin[2]);
		// initialize the bounding box
		for (int i = 0; i < 3; i++)
		{
			cooMin[i] = std::numeric_limits<double>::max();
			cooMax[i] = -std::numeric_limits<double>::max();
		}
		double *childBoxMin;
		double *childBoxMax;
		// merge the children boxes
		double volumeSumChildren = 0;
		for (int i = 0; i < 8; i++)
		{
			if (children[i] != NULL)
			{
				// if the children is not active, but its siblings are, invalidate it
				if (!children[i]->IsActive(queryNumber)) 
					children[i]->Rebuild();

				childBoxMax = children[i]->GetBoundingBoxMax();
				childBoxMin = children[i]->GetBoundingBoxMin();
				volumeSumChildren += (childBoxMax[0] - childBoxMin[0]) * (childBoxMax[1] - childBoxMin[1]) * (childBoxMax[2] - childBoxMin[2]);
				for (int j = 0; j < 3; j++)
				{
					if (childBoxMax[j] > cooMax[j])
						cooMax[j] = childBoxMax[j];
					if (childBoxMin[j] < cooMin[j])
						cooMin[j] = childBoxMin[j];
				}
			}
		}
		// check if the difference between original volume and the volume of the merged bbox is not too large
		// if it is, this node will be marked as invalid and therefore will be resplit if met in the collision detection process
		if ((volumeOrig / volumeSumChildren) < 0.9) // 0.9 set according to Larsson, Akenine-Moeller paper
			Rebuild();
	}
}
#pragma endregion


vtkStandardNewMacro(vtkMassSpringMuscle);

/** Constructor */
vtkMassSpringMuscle::vtkMassSpringMuscle()
{
	data                = NULL;
	geometry            = NULL;
	mss                 = NULL;
	vertices            = NULL;
	verticesPrev        = NULL;
	positionPrev        = NULL;
	tempCollisionForces = NULL;
	collidingVertices   = NULL;
	numberOfCollisions  = NULL;
	bbox                = NULL;
	verticesFlags       = NULL;
}

/** Destructor */
vtkMassSpringMuscle::~vtkMassSpringMuscle()
{
	delete mss;
	//delete []vertices;
	//delete []verticesPrev;
	delete []positionPrev;
	delete []tempCollisionForces;
	delete []collidingVertices;
	delete []numberOfCollisions; 
	delete bbox;
	//delete []verticesFlags;
}

/** Updates the mesh based on the changes in the mass spring system (represented by the Vector arrays) */
void vtkMassSpringMuscle::updateMesh()
{
	/*	OLD CODE - update mesh via n-closest particles
	MMSSVector3d temp;
	temp.x = 0;
	temp.y = 0;
	temp.z = 0;
	vtkPoints *points = geometry->GetPoints();
	int **closestParticles = data->GetClosestParticles();
	for (int i = 0; i < geometry->GetNumberOfPoints(); i++)
	{
	// make an average movement vector of all the control particles
	for (int j = 0; j < NO_OF_CONTROL_PARTICLES; j++)
	temp = MMSSVector3d::VectorAdd(temp, MMSSVector3d::VectorSub(vertices[closestParticles[j][i]], positionPrev[closestParticles[j][i]]));
	MMSSVector3d::ConstMult(&temp, 1 / (double)NO_OF_CONTROL_PARTICLES);

	//add the average to the given vertex of the mesh
	double *point = points->GetPoint(i);
	point[0] +=  temp.x;
	point[1] +=  temp.y;
	point[2] +=  temp.z;
	points->SetPoint(i, point);
	temp.x = 0;
	temp.y = 0;
	temp.z = 0;
	}*/
	// use MVCs to update the mesh
	vtkPolyData *tri = data->GetBoundaryTriangulation();
	int *boundaryMap = data->GetBoundaryParticles();
	for (int i = 0; i < data->GetNumberOfBoundaryParticles(); i++)
		tri->GetPoints()->SetPoint(i, vertices[boundaryMap[i]].x, vertices[boundaryMap[i]].y, vertices[boundaryMap[i]].z);
	tri->Update();
	vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(tri,
		data->GetMVCWeights(), geometry);
}

/** Updates the particle positions of the object */
void vtkMassSpringMuscle::updatePositions()
{
	mss->GetVertices(positionPrev, data->GetNumberOfParticles());
}

/** Adds a given force to a given vertex.
vertexIndex - index of the vertex (particle) to which the force is applied
force - the force vector */
void vtkMassSpringMuscle::AddCollisionForce(int vertexIndex, MMSSVector3d force)
{
	tempCollisionForces[vertexIndex] = tempCollisionForces[vertexIndex] + force;
	numberOfCollisions[vertexIndex]++;
}

/** Clears the buffer for collsion forces. */
void vtkMassSpringMuscle::ResetCollisions()
{
	//this->bbox->Update(this->bbox->GetQueryNumber()); // norebuild
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		tempCollisionForces[i].x = 0;
		tempCollisionForces[i].y = 0;
		tempCollisionForces[i].z = 0;
		numberOfCollisions[i] = 0;
	}
}

/** Processes the temporal collision forces accumulated during the resolution of a collision with various other objects. */
void vtkMassSpringMuscle::ProcessCollisions()
{
	/*for (unsigned int i = 0; i < count.verticesCount; i++)
	{
	//forces[i] = MMSSVector3d::vectorAdd(forces[i], tempCollisionForces[i]);
	velocities[i] = (1 / (double)(1 + numberOfCollisions[i])) * velocities[i];
	velocities[i] = MMSSVector3d::vectorAdd(velocities[i], tempCollisionForces[i]);		

	vertices_prev[i] = vertices[i];
	vertices[i] = vertices[i] + minStep * velocities[i];
	}*/

	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{	
		if (numberOfCollisions[i] > 0)
		{			
			vertices[i] = vertices[i] + tempCollisionForces[i];
			verticesPrev[i] = vertices[i]; // the particles that collided should not "bounce" after collision - they stop moving
		}
	}
}

// Computes the average difference of particle positions in between two Update() calls.
// RETURN:
// an average of lengths of "(postion[i] - previousPosition[i])" vectors
float vtkMassSpringMuscle::ComputeAverageDifference()
{
	float avg = 0;
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
		avg += MMSSVector3d::VectorLength(vertices[i], positionPrev[i]);
	return avg / data->GetNumberOfParticles();
}


/** Updates the output.  */
void vtkMassSpringMuscle::Update()
{
	this->updateMesh();
	this->updatePositions();
	if (this->GetOutput() == NULL)
		this->SetOutput(geometry);
	else if (this->GetOutput() != geometry)
	{
		this->GetOutput()->DeepCopy(geometry);
		geometry->Delete();
		geometry = this->GetOutput();
	}
	// else the output is already set to geometry
}

/** Detects and resolves collision of two MSS muscles
otherObject - the second muscle to collide with this one.
iterationNumber - the number of collision detection querries were made in the whole scene so far, i.e.
how many times was completed the loop "for all objects ResetCollisions()->for all objects CollideWithAnotherObject()->
for all objects ProcessCollisions()". */
void vtkMassSpringMuscle::CollideWithAnotherObject(vtkMassSpringMuscle *otherObject, int iterationNumber)
{
	MMSSVector3d *otherVertices = otherObject->GetVertices();
	MMSSVector3d force;
	double *otherParticlesRads = otherObject->data->GetParticleRadiuses();
	double *particlesRads = data->GetParticleRadiuses();
	vector<vector<int>> collidingPairs = this->bbox->DetectCollision(otherObject->GetBoundingBox(), iterationNumber, 20);

	for (vector<vector<int>> ::const_iterator iter = collidingPairs.begin(); iter != collidingPairs.end(); iter++)
	{
		vector<int> toCollideThis = *iter;
		iter++;
		vector<int> toCollideOther = *iter;
		for (vector<int> ::const_iterator iterThis = toCollideThis.begin(); iterThis != toCollideThis.end(); iterThis++)
		{
			for (vector<int> ::const_iterator iterOther = toCollideOther.begin(); iterOther != toCollideOther.end(); iterOther++)
			{
				float length = MMSSVector3d::VectorLength(vertices[*iterThis], otherVertices[*iterOther]);
				double radiusSum = particlesRads[*iterThis] + otherParticlesRads[*iterOther];
				if (length < radiusSum)
				{
					// if it is a fixed point, add the whole vector to the other point
					force = (radiusSum - length) * MMSSVector3d::Normalize(vertices[*iterThis] - otherVertices[*iterOther]);
					if (verticesFlags[*iterThis] && !(otherObject->IsFixed(*iterOther))) // if the first one is fixed and second is not
					{
						otherObject->AddCollisionForce(*iterOther,  -1 * force);
						collidingVertices[*iterThis].push_back(*iterOther);
					}
					else if (otherObject->IsFixed(*iterOther) && !verticesFlags[*iterThis]) // if the second is fixed and first is not
					{
						AddCollisionForce(*iterThis, force);
						collidingVertices[*iterThis].push_back(*iterOther);
					}
					else if (!verticesFlags[*iterThis] && !(otherObject->IsFixed(*iterOther)))
					{
						force = 0.5 * force; // if neither one is fixed, add half to one vertex and half to the other
						AddCollisionForce(*iterThis, force);
						otherObject->AddCollisionForce(*iterOther, -1 * force);
						collidingVertices[*iterThis].push_back(*iterOther);
					}
					// else if both are fixed, do nothing
				}
			}
		}
	}
}

/** Generates a vtkPolyData object containing a set of spheres that represents all the particles. */
vtkPolyData *vtkMassSpringMuscle::GetParticlesAsOneMesh()
{	
	vtkAppendPolyData *app = vtkAppendPolyData::New();
	vtkSphereSource **spheres = new vtkSphereSource *[data->GetNumberOfParticles()];
	double *radiuses = data->GetParticleRadiuses();
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		spheres[i] = vtkSphereSource::New();
		if (data->GetBoundaryParticles()[i])
			spheres[i]->SetRadius(radiuses[i]);
		else spheres[i]->SetRadius(0.1);
		spheres[i]->SetCenter(vertices[i].x, vertices[i].y, vertices[i].z);
		spheres[i]->SetThetaResolution(6); // 6 vertices along both longitude and latitude
		spheres[i]->SetPhiResolution(6);
		app->AddInput(spheres[i]->GetOutput());
	}
	app->Update();
	vtkPolyData *ret = vtkPolyData::New();
	ret->ShallowCopy(app->GetOutput());
	app->Delete();
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
		spheres[i]->Delete();
	delete []spheres;
	return ret;
}

/** Get the deformed positions of particles. */
void vtkMassSpringMuscle::GetParticles(std::vector<medVMEMuscleWrapper::CParticle>& particles)
{
	if (particles.size() == data->GetNumberOfParticles()) {
		for(size_t i = 0; i < particles.size(); ++i) {
			particles[i].position[0] = vertices[i].x;
			particles[i].position[1] = vertices[i].y;
			particles[i].position[2] = vertices[i].z;
		}
	}
}

/** Creates and returns the data structures required by the mass spring system.
Call this method if the data are not already ready and stored somewhere. If they are, simply use SetInput.
muscleParticles - vector with particles generated for the fibers of the muscle 
muscleMesh - the surface mesh of the muscle */
vtkMSSDataSet *vtkMassSpringMuscle::CreateData(std::vector<medVMEMuscleWrapper::CParticle>& muscleParticles, vtkPolyData *muscleMesh)
{
#pragma region oldcode
	// Generate particles from fibers
	/*	int partCount = 0;
	vtkCellArray *lines = fibers->GetLines();
	vtkPoints *points = fibers->GetPoints();
	lines->InitTraversal();
	vtkIdType *pts;	vtkIdType npts;
	double pointA[3], pointB[3], pointVector[3];
	vector<double> partList;
	// first count total length of all fibers
	double totalLength = 0;
	for (int i = 0; i < lines->GetNumberOfCells(); i++) // for each polyline
	{
	lines->GetNextCell(npts, pts);
	for (int j = 1; j < npts; j++) // count the total length of this polyline
	{
	points->GetPoint(pts[j - 1], pointA); // get the border points of the segment
	points->GetPoint(pts[j], pointB);
	totalLength += sqrt(ComputeDistanceSquared(pointA, pointB));
	}
	}
	// determine the size and spacing of particles
	int noOfPartsPerModel = 2000;
	double distBetweenParticles = totalLength / noOfPartsPerModel; // the distance between the particles on one fiber - it will be constant for all particles ?? how to determine --------- TODO??
	double radius = distBetweenParticles / 2; // the initial radius of particles set so that they touch
	// generate particles
	lines->InitTraversal();
	for (int i = 0; i < lines->GetNumberOfCells(); i++) // for each polyline
	{
	lines->GetNextCell(npts, pts);
	double currLength = 0; // the distance along the segment covered so far
	for (int j = 1; j < npts; j++) // for each segment of the polyline
	{		
	points->GetPoint(pts[j - 1], pointA); // get the border points of the segment
	points->GetPoint(pts[j], pointB);
	double segLength = sqrt(ComputeDistanceSquared(pointA, pointB));
	pointVector[0] = (pointB[0] - pointA[0]) / segLength; // normalized vector
	pointVector[1] = (pointB[1] - pointA[1]) / segLength;
	pointVector[2] = (pointB[2] - pointA[2]) / segLength;
	while (currLength < segLength) // put particles on the segment with given spacing (distBetweenParticles)
	{
	// create a new particle
	partList.push_back(pointA[0] + pointVector[0] * currLength);// add the length to the starting point of this segment while going along the segment
	partList.push_back(pointA[1] + pointVector[1] * currLength);
	partList.push_back(pointA[2] + pointVector[2] * currLength);
	partList.push_back(radius);
	// move to the place of another particle
	currLength += distBetweenParticles;
	partCount++; // we created a new particle
	}
	currLength -= segLength; // only the uncovered distance will remain in currLength
	}
	// if a reasonably large part of the line was not covered, insert one last particle on the place of the last point
	if (currLength > radius / 2)
	{
	partList.push_back(pointB[0]);
	partList.push_back(pointB[1]);
	partList.push_back(pointB[2]);
	partList.push_back(radius);
	partCount++;
	}
	}
	// copy the particles from vector to a fixed array		
	MMSSVector3d *particles = new MMSSVector3d[partCount];
	double *radiuses = new double[partCount];
	for (int i = 0, counter = 0; i < partCount; i++)
	{
	particles[i].x = partList[counter++];
	particles[i].y = partList[counter++];
	particles[i].z = partList[counter++];
	radiuses[i] = partList[counter++];
	}	
	partList.clear();*/
#pragma endregion

	// extract needed data from muscleParticles and store it in the format used by MassSpringSystemCPU
	int partCount = muscleParticles.size();
	MMSSVector3d *particles = new MMSSVector3d[partCount];
	double *radiuses = new double[partCount];
	int *boneIDs = new int[partCount * 2];
	double *boneWeights = new double[partCount];
	int noOfBoundaryParticles = 0;
	// copy the particles into a MMSSVector3d array - used by the MassSpringSystemCPU, so lets use it as well
	// also count fixed particles and springs so that we can allocate a fixed length array (MSSCPU does not use vectors) and fill them later
	int noOfFixedParticles = 0;
	int noOfSprings = 0;
	for (int i = 0; i < partCount; i++)
	{
		particles[i].x = muscleParticles[i].position[0];
		particles[i].y = muscleParticles[i].position[1];
		particles[i].z = muscleParticles[i].position[2];
		if (muscleParticles[i].fixed) noOfFixedParticles++; // count the fixed particles
		noOfSprings += muscleParticles[i].neighbors.size(); // count the number of springs
		boneIDs[i * 2] = muscleParticles[i].boneID[0];
		boneIDs[i * 2 + 1] = muscleParticles[i].boneID[1];
		boneWeights[i] = muscleParticles[i].boneWeight;
		if (muscleParticles[i].boundary) noOfBoundaryParticles++; // count boundary particles
	}
	noOfSprings = noOfSprings >> 1; // every spring was counted twice -> divide by two

	// set the initial radius to the distance between first two particles (usually they will be on one fibre) - it gets updated properly later
	double radius = 0;
	if (partCount > 1)
		radius = MMSSVector3d::VectorLength(particles[0], particles[1]);

	int *boundaryParticlesMap = new int[noOfBoundaryParticles];
	// set the same radius for each particle for now - will be checked later in the GeneratetLinksUpdateRadiuses method
	// also store the information about fixed particles into a 1-D array as used by the MassSpringSystemCPU
	// and also fill spring parametres
	IMassSpringSystem::Spring *springs = new IMassSpringSystem::Spring[noOfSprings];
	unsigned int *fixedParticles = new unsigned int[noOfFixedParticles];
	for (int i = 0, boundaryCount = 0, fixedCount = 0, springCount = 0; i < partCount; i++)
	{
		radiuses[i] = radius; // set the radius
		if (muscleParticles[i].fixed)
			fixedParticles[fixedCount++] = i; // indicate the fixed particle
		// create the information about the spring
		for (int k = 0; k < muscleParticles[i].neighbors.size(); k++)
		{
			if (muscleParticles[i].neighbors[k] > i) // else the spring was already created before
			{
				springs[springCount].p1_index = i;
				springs[springCount].p2_index = muscleParticles[i].neighbors[k];
				springs[springCount].initLen = MMSSVector3d::VectorLength(particles[springs[springCount].p1_index], 
					particles[springs[springCount].p2_index]);
				springs[springCount++].stiffness = (1 / springs[springCount].initLen);
			}
		}
		if (muscleParticles[i].boundary) boundaryParticlesMap[boundaryCount++] = i; // store index of the next boundary particle
	}

	// create the vtkMSSDataSet object and fill it with the extracted information
	vtkMSSDataSet *data = vtkMSSDataSet::New();
	data->SetParticlesArray(particles);
	data->SetParticleRadsArray(radiuses);
	data->SetParticleBoneIDsArray(boneIDs);
	data->SetParticleBoneWeightsArray(boneWeights);
	data->SetSpringArray(springs);
	data->SetFixedPointsArray(fixedParticles);
	data->SetNumberOfParticles(partCount);
	data->SetNumberOfVertices(muscleMesh->GetNumberOfPoints());
	data->SetNumberOfSprings(noOfSprings);
	data->SetNumberOfFixedPoints(noOfFixedParticles);
	data->SetBoundaryParticlesMapArray(boundaryParticlesMap);
	data->SetNumberOfBoundaryParticles(noOfBoundaryParticles);

	// create the links - the data object is now filled
	vtkMassSpringMuscle::GenerateLinksUpdateRadiuses(data, muscleMesh);
	// create MVC coordinates between the boundary particles and the muscle mesh
	vtkMassSpringMuscle::ComputeMVCCoordinates(data, muscleMesh);
	return data;
}

/** Sets the input of the filter. Data must contain the particles, springs etc. of the system,
geometry must contain the original mesh of the muscle that is to be deformed. */
void vtkMassSpringMuscle::SetInput(vtkMSSDataSet *data, vtkPolyData *geometry)
{
	this->data = data; // its not modified anywhere, no need to create a copy
	this->vtkPolyDataToPolyDataFilter::SetInput(data); 

	if (this->GetOutput() == NULL) {
		this->SetOutput(vtkPolyData::New());
		this->GetOutput()->UnRegister(this);	//decrease reference
	}

	this->geometry = this->GetOutput();
	this->geometry->DeepCopy(geometry); 
}

/** Prepares all data for processing. Returns true if succesful.
transform - the transformation that should be applied to the fixed vertices
(i.e. the transform between rest pose and current pose). */
bool vtkMassSpringMuscle::PreprocessTransformFixedOnly(std::vector<mafTransform *>& transforms)
{
	if (data == NULL || geometry == NULL) return false; // if input data were not specified

	if(this->tempCollisionForces)
		delete []this->tempCollisionForces;
	this->tempCollisionForces = new MMSSVector3d[data->GetNumberOfParticles()];
	if(this->collidingVertices)
		delete []this->collidingVertices;
	this->collidingVertices = new list<int>[data->GetNumberOfParticles()];
	if(this->numberOfCollisions)
		delete []this->numberOfCollisions;
	this->numberOfCollisions = new int[data->GetNumberOfParticles()];
	if(this->positionPrev)
		delete []this->positionPrev;
	this->positionPrev = new MMSSVector3d[data->GetNumberOfParticles()];
	MMSSVector3d *particles = data->GetParticles();
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		positionPrev[i].x = particles[i].x;
		positionPrev[i].y = particles[i].y;
		positionPrev[i].z = particles[i].z;
	}

	// create the mass spring system
	IMassSpringSystem::Counts count;
	count.verticesCount = data->GetNumberOfParticles();
	count.springsCount = data->GetNumberOfSprings();
	if(this->mss) delete this->mss;
	this->mss = new MassSpringSystemCPU(positionPrev, data->GetSprings(), count,
		data->GetParticleRadiuses(), data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles()); 
	this->verticesFlags = mss->getVerticesFlags();
	this->vertices = mss->getVertexArrayPointer();
	this->verticesPrev = mss->getVertexPrevArrayPointer();

	// transform the points
	double part[3];
	double result[3];
	unsigned int *fixedPoints = data->GetFixedPoints();
	int *boneIDs = data->GetParticleBoneIDs();
	double *boneWeights = data->GetParticleBoneWeights();
	for (int i = 0; i < data->GetNumberOfFixedPoints(); i++)
	{
		int particleID = fixedPoints[i];
		part[0] = vertices[particleID].x;
		part[1] = vertices[particleID].y;
		part[2] = vertices[particleID].z;
		if(boneIDs[particleID * 2 + 1] == -1) {
			transforms[boneIDs[particleID * 2]]->TransformPoint(part, part);
		}
		else {
			transforms[boneIDs[particleID * 2]]->TransformPoint(part, result);
			transforms[boneIDs[particleID * 2 + 1]]->TransformPoint(part, part);
			part[0] = result[0] * boneWeights[particleID] + part[0] * (1 - boneWeights[particleID]);
			part[1] = result[1] * boneWeights[particleID] + part[1] * (1 - boneWeights[particleID]);
			part[2] = result[2] * boneWeights[particleID] + part[2] * (1 - boneWeights[particleID]);
		}
		vertices[particleID].x = part[0];
		vertices[particleID].y = part[1];
		vertices[particleID].z = part[2];		
		verticesPrev[particleID].x = part[0];
		verticesPrev[particleID].y = part[1];
		verticesPrev[particleID].z = part[2];
		positionPrev[particleID].x = part[0]; // store the initial position of particles
		positionPrev[particleID].y = part[1];
		positionPrev[particleID].z = part[2];
	}
	mss->SetFixedPoints(data->GetFixedPoints(), vertices, data->GetNumberOfFixedPoints());	
	bool *flags = mss->getVerticesFlags();
	// transform the mesh by the same transformation as its closest particle if it is fixed
	// in the end (after spring simulation ends), the transformation will be completed 
	// in updateMesh to reflect the changes of particle positions (i.e. vector (vertices[i] - positionPrev[i]))
	double temp[3];
	for (int i = 0; i < geometry->GetNumberOfPoints(); i++)
	{
		temp[0] = 0; temp[1] = 0; temp[2] = 0;
		// tranform it by the matrix of each closest fixed particle and make an average
		for (int j = 0; j < NO_OF_CONTROL_PARTICLES; j++)
		{
			geometry->GetPoint(i, part);
			if (flags[data->GetClosestParticles()[j][i]])
				transforms[boneIDs[data->GetClosestParticles()[j][i]]]->TransformPoint(part, part); 
			temp[0] += part[0];
			temp[1] += part[1];
			temp[2] += part[2];

		}
		temp[0] /= NO_OF_CONTROL_PARTICLES;
		temp[1] /= NO_OF_CONTROL_PARTICLES;
		temp[2] /= NO_OF_CONTROL_PARTICLES;
		geometry->GetPoints()->SetPoint(i, temp);
	}

	// setup bounding box
	if(this->bbox) delete this->bbox;
	this->bbox = new SphereBoundingBox(NULL, NULL, NULL); // they are the root BBs, so no parent
	this->bbox->SetContentFromArray(vertices, data->GetParticleRadiuses(), data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles());
	this->bbox->ComputeBoundingBox();

	return true;
}

bool vtkMassSpringMuscle::Preprocess(std::vector<mafTransform *>& transforms)
{
	if (data == NULL || geometry == NULL) return false; // if input data were not specified

	if(this->tempCollisionForces)
		delete []this->tempCollisionForces;
	this->tempCollisionForces = new MMSSVector3d[data->GetNumberOfParticles()];
	if(this->collidingVertices)
		delete []this->collidingVertices;
	this->collidingVertices = new list<int>[data->GetNumberOfParticles()];
	if(this->numberOfCollisions)
		delete []this->numberOfCollisions;
	this->numberOfCollisions = new int[data->GetNumberOfParticles()];
	if(this->positionPrev)
		delete []this->positionPrev;
	this->positionPrev = new MMSSVector3d[data->GetNumberOfParticles()];
	MMSSVector3d *particles = data->GetParticles();
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		positionPrev[i].x = particles[i].x;
		positionPrev[i].y = particles[i].y;
		positionPrev[i].z = particles[i].z;
	}

	// create the mass spring system
	IMassSpringSystem::Counts count;
	count.verticesCount = data->GetNumberOfParticles();
	count.springsCount = data->GetNumberOfSprings();
	if(this->mss) delete this->mss;
	this->mss = new MassSpringSystemCPU(positionPrev, data->GetSprings(), count,
		data->GetParticleRadiuses(), data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles()); 
	this->verticesFlags = mss->getVerticesFlags();
	this->vertices = mss->getVertexArrayPointer();
	this->verticesPrev = mss->getVertexPrevArrayPointer();

	// transform the points
	double part[3];
	double result[3];
	int *boneIDs = data->GetParticleBoneIDs();
	double *boneWeights = data->GetParticleBoneWeights();
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		part[0] = vertices[i].x;
		part[1] = vertices[i].y;
		part[2] = vertices[i].z;
		if(boneIDs[i * 2 + 1] == -1) {
			transforms[boneIDs[i * 2]]->TransformPoint(part, part);
		}
		else {
			transforms[boneIDs[i * 2]]->TransformPoint(part, result);
			transforms[boneIDs[i * 2 + 1]]->TransformPoint(part, part);
			part[0] = result[0] * boneWeights[i] + part[0] * (1 - boneWeights[i]);
			part[1] = result[1] * boneWeights[i] + part[1] * (1 - boneWeights[i]);
			part[2] = result[2] * boneWeights[i] + part[2] * (1 - boneWeights[i]);
		}
		vertices[i].x = part[0];
		vertices[i].y = part[1];
		vertices[i].z = part[2];
		verticesPrev[i].x = part[0];
		verticesPrev[i].y = part[1];
		verticesPrev[i].z = part[2];

		positionPrev[i].x =  part[0]; // store the initial position of particles
		positionPrev[i].y = part[1];
		positionPrev[i].z = part[2];
	}
	mss->SetFixedPoints(data->GetFixedPoints(), vertices, data->GetNumberOfFixedPoints());

	// transform the mesh by the same transformation as its closest particle
	// in the end (after spring simulation ends), the transformation will be completed 
	// in updateMesh to reflect the changes of particle positions (i.e. vector (vertices[i] - positionPrev[i]))
	/* NOT NEEDED NOW WHEN THE MVCs ARE USED FOR MESH UPDATE
	double temp[3];
	for (int i = 0; i < geometry->GetNumberOfPoints(); i++)
	{
	temp[0] = 0; temp[1] = 0; temp[2] = 0;
	// tranform it by the matrix of each closest particle and make an average
	for (int j = 0; j < NO_OF_CONTROL_PARTICLES; j++)
	{
	geometry->GetPoint(i, part);
	int particleID = data->GetClosestParticles()[j][i];
	if(boneIDs[particleID * 2 + 1] == -1) {
	transforms[boneIDs[particleID * 2]]->TransformPoint(part, part); 
	} else {
	transforms[boneIDs[particleID * 2]]->TransformPoint(part, result);
	transforms[boneIDs[particleID * 2 + 1]]->TransformPoint(part, part);
	part[0] = result[0] * boneWeights[particleID] + part[0] * (1 - boneWeights[particleID]);
	part[1] = result[1] * boneWeights[particleID] + part[1] * (1 - boneWeights[particleID]);
	part[2] = result[2] * boneWeights[particleID] + part[2] * (1 - boneWeights[particleID]);
	}
	temp[0] += part[0];
	temp[1] += part[1];
	temp[2] += part[2];
	}
	temp[0] /= NO_OF_CONTROL_PARTICLES;
	temp[1] /= NO_OF_CONTROL_PARTICLES;
	temp[2] /= NO_OF_CONTROL_PARTICLES;
	geometry->GetPoints()->SetPoint(i, temp);
	}*/

	// setup bounding box
	if(this->bbox) delete this->bbox;
	this->bbox = new SphereBoundingBox(NULL, NULL, NULL); // they are the root BBs, so no parent
	this->bbox->SetContentFromArray(vertices, data->GetParticleRadiuses(), data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles());
	this->bbox->ComputeBoundingBox();

	return true;
}

/** Generates the control "links" between particles and vertices of the mesh.
Then updates the radius of each particle in order to prevent overlapping of particles
and at the same time be as big as possible.
Fills the vtkMSSDataSet object 'data' passed as argument.
geometry = the mesh the data are bound to. */
void vtkMassSpringMuscle::GenerateLinksUpdateRadiuses(vtkMSSDataSet *data, vtkPolyData *geometry)
{
	//@original version = 624 samples -> new none = 141 samples

	// update radiuses to match the reference mesh
	// first generate links between vertices of the mesh and closest particles to that vertex
	// then update the radiuses so that each particle touches or contains most of its linked vertices
	int **closestParticles = data->GetClosestParticles();
	MMSSVector3d *particles = data->GetParticles();
	int *boundaryMap = data->GetBoundaryParticles();

	int noOfVerts = geometry->GetNumberOfPoints();
	int *linkMapVertToPart = new int[noOfVerts]; // will contain indices of particles belonging to a given vertex
	list<int> *linkMapPartToVert = new list<int>[data->GetNumberOfParticles()]; // will contain lists of vertices belonging to a given particle

	double *distances = new double[noOfVerts]; // keeps the distance to the closest particle for a given vertex
	for (int i = 0; i < NO_OF_CONTROL_PARTICLES; i++) {
		closestParticles[i] = new int[noOfVerts];
	}

	//BES: 22.11.2012 - create Point locator for boundary particles	
	vtkPoints* bndParts = vtkPoints::New();	//coordinates are in GLfloats, i.e., floats
	int nBndParts = data->GetNumberOfBoundaryParticles();
	bndParts->SetNumberOfPoints(nBndParts);
	for (int i = 0; i < nBndParts; i++) {
		bndParts->SetPoint(i, particles[boundaryMap[i]].x, 
			particles[boundaryMap[i]].y, particles[boundaryMap[i]].z);
	}

	vtkPolyData* bndPartsPS = vtkPolyData::New();
	bndPartsPS->SetPoints(bndParts);

	vtkPointLocator* bndPartsLocator = vtkPointLocator::New();
	bndPartsLocator->SetDataSet(bndPartsPS);
	bndPartsLocator->Update();

	bndPartsPS->Delete();	//no longer needed
	bndParts->Delete();		//no longer needed

	vtkIdList* kNearestIds = vtkIdList::New();

	//for each mesh vertex, find NO_OF_CONTROL_PARTICLES nearest particles and creates links between these
	for (int i = 0; i < noOfVerts; i++)
	{
		double x[3];
		geometry->GetPoint(i, x);

		//as boundary particles should lie inside the mesh (or very close to it, this method should give us good results)
		bndPartsLocator->FindClosestNPoints(NO_OF_CONTROL_PARTICLES, x, kNearestIds);

		double minDistances[NO_OF_CONTROL_PARTICLES];
		for (int k = 0; k < NO_OF_CONTROL_PARTICLES; k++) {
			minDistances[k] = numeric_limits<double>::max();
		}

		int nFoundParts = kNearestIds->GetNumberOfIds();
		if (nFoundParts != NO_OF_CONTROL_PARTICLES) 
		{
			//if we could not find quickly the result, use the brute-force
			nFoundParts = data->GetNumberOfBoundaryParticles();	//we will need to process every particle
			vtkIdType* pIds = kNearestIds->WritePointer(0, nFoundParts);
			for (int j = 0; j < nFoundParts; j++) {
				pIds[j] = j;
			}
		}

		vtkIdType* pIds = kNearestIds->GetPointer(0);					
		for (int j = 0; j < nFoundParts; j++)
		{
			int index = boundaryMap[pIds[j]];				
			double distance = vtkMath::Distance2BetweenPoints(x, bndPartsPS->GetPoint(pIds[j]));
			if (distance < minDistances[0]) // minDistances[0] is always the shortest of minDistances
			{
				linkMapVertToPart[i] = index;
				distances[i] = distance; // so that we wont have to compute it again in the next step
			}

			// update which particles are the closest to this vertex
			int tempParticleID;
			int prevParticleID = index;
			double tempDistance;
			for (int k = 0; k < NO_OF_CONTROL_PARTICLES; k++)
			{
				if (distance < minDistances[k])
				{
					tempDistance = distance;
					distance = minDistances[k];
					minDistances[k] = tempDistance;
					tempParticleID = prevParticleID;
					prevParticleID = closestParticles[k][i];
					closestParticles[k][i] = tempParticleID;
				}
			}
		}
		distances[i] = sqrt(distances[i]);
		linkMapPartToVert[linkMapVertToPart[i]].push_back(i);
	}

	kNearestIds->Delete();			//no longer needed
	bndPartsLocator->Delete();	//no longer needed

	double *radiuses = data->GetParticleRadiuses();
	// the secon part - update of radiuses
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		// if it is not linked with any vertex, leave the radius as it is, it will be modified in the second loop
		if (!linkMapPartToVert[i].empty())
		{
			// find both nearest and the most distant vertex
			double maxDistance = 0;
			double minDistance = numeric_limits<double>::max();
			for (list<int>::const_iterator iter = linkMapPartToVert[i].begin(); iter != linkMapPartToVert[i].end(); iter++)
			{
				/*	if (distances[*iter] > maxDistance)
				maxDistance = distances[*iter];
				if (distances[*iter] < minDistance)
				minDistance = distances[*iter];*/
				maxDistance += distances[*iter];
			}		
			//double temp = minDistance + ((maxDistance - minDistance) / 2);			// set the radius in between the two
			double temp = maxDistance / linkMapPartToVert[i].size(); // set the radius as the average 
			if (temp > radiuses[i])
				radiuses[i] = temp;
		}
	}
	// chceck whether the particles do not overlap and if so, fix it
	// use the collision detection mechanism to speed this up (avoid each-with-each checks by colliding this with itself)
	SphereBoundingBox *boxa = new SphereBoundingBox(NULL, NULL, NULL);
	boxa->SetContentFromArray(particles, radiuses, data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles());
	boxa->ComputeBoundingBox();
	SphereBoundingBox *boxb = new SphereBoundingBox(NULL, NULL, NULL);
	boxb->SetContentFromArray(particles, radiuses, data->GetBoundaryParticles(), data->GetNumberOfBoundaryParticles());
	boxb->ComputeBoundingBox();
	vector<vector<int>> collidingPairs = boxa->DetectCollision(boxb, 1, 20);

	for (vector<vector<int>> ::const_iterator iter = collidingPairs.begin(); iter != collidingPairs.end(); iter++)
	{
		vector<int> toCollideThis = *iter;
		iter++;
		vector<int> toCollideOther = *iter;
		for (vector<int> ::const_iterator iterThis = toCollideThis.begin(); iterThis != toCollideThis.end(); iterThis++)
		{
			for (vector<int> ::const_iterator iterOther = toCollideOther.begin(); iterOther != toCollideOther.end(); iterOther++)
			{
				if (*iterThis == *iterOther) continue;
				double distance = MMSSVector3d::VectorLength(particles[*iterThis], particles[*iterOther]);
				if (radiuses[*iterThis] + radiuses[*iterOther] > distance)
				{
					if (radiuses[*iterThis] < distance / 2)
						radiuses[*iterOther] = distance - radiuses[*iterThis];
					else if (radiuses[*iterOther] < distance / 2)
						radiuses[*iterThis] = distance - radiuses[*iterOther];
					else
					{
						radiuses[*iterThis] = distance / 2;
						radiuses[*iterOther] = distance / 2;
					}
				}
			}
		}
	}

	delete boxa;
	delete boxb;
	delete []linkMapVertToPart;
	delete []linkMapPartToVert;
	delete []distances;
}

/** Computes the mean value coordinates of the input mesh based on the particle mesh.
Only boundary particles are considered. They are triangularized with delaunay triangulation
and then use the vtkMAFMeanvalueCoordinateInterpolation for the actual computation.
Fills the vtkMSSDataSet object 'data' passed as argument.
geometry = the mesh the data are bound to.
*/
void vtkMassSpringMuscle::ComputeMVCCoordinates(vtkMSSDataSet *data, vtkPolyData *geometry)
{
	//algorithm: create triangular surface for the boundary particles 
	//and then express the relationship between this surface and the passed geometry mesh

	int *boundaryMap = data->GetBoundaryParticles();
	int noOfBoundaryParticles = data->GetNumberOfBoundaryParticles();
	MMSSVector3d *vertices = data->GetParticles();	
	vtkPoints *points = vtkPoints::New();
	points->SetNumberOfPoints(noOfBoundaryParticles);
	for (int i = 0; i < noOfBoundaryParticles; i++) {
		points->SetPoint(i, vertices[boundaryMap[i]].x, vertices[boundaryMap[i]].y, vertices[boundaryMap[i]].z);
	}

	vtkMAFSmartPointer< vtkPolyData > bndPoly;
	bndPoly->SetPoints(points);	
	points->Delete(); //no longer needed


	vtkMAFSmartPointer< vtkDelaunay3D > del;
	del->SetAlpha(0.0);	//no alpha, get the DT
	del->SetInput(bndPoly);

	vtkMAFSmartPointer< vtkGeometryFilter > geof;
	geof->SetInput(del->GetOutput());	
	geof->Update();		//run the pipeline to get DT surface

	//set the boundary triangulation
	vtkPolyData* tri = geof->GetOutput();
	data->SetBoundaryTriangulation(tri);

	vtkMAFSmartPointer<vtkDoubleArray>  relationship;
	vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(tri, geometry, relationship.GetPointer());
	data->SetMVCWeights(relationship);	
}

/** Creates a set of balls describing the given mesh.
source - the mesh to be transformed into a set of balls. */
vtkMSSDataSet *vtkMassSpringMuscle::GenerateSolidObject(vtkPolyData *source)
{
	int noOfSpheres = 0;


	vector<MMSSVector3d> centers;
	vector<double> radiuses;
	vtkIdList *currentNeighbours = vtkIdList::New();
	currentNeighbours->SetNumberOfIds(2);
	vtkIdList *currentPoint = vtkIdList::New();
	currentPoint->SetNumberOfIds(1);
	vtkIdList *neighbourCells = vtkIdList::New();
	vtkIdList *neighbourCells2 = vtkIdList::New();
	MMSSVector3d normal;
	double pointC[3];
	double pointA[3];
	double pointB[3];
	MMSSVector3d edge1, edge2;
	int noOfPoints = source->GetNumberOfPoints();
	bool *usedPointsMap = new bool[noOfPoints]; // contains whether a ball was generated based on this point or its neighbours
	for (int i = 0; i < noOfPoints; i++)
		usedPointsMap[i] = false;
	MMSSVector3d center;
	for (int i = 0; i < noOfPoints; i+=10)
	{
		if (usedPointsMap[i]) continue; // continue to the next so far unused vertex
		usedPointsMap[i] = true;
		currentPoint->SetId(0, i);
		source->GetPoint(i, pointC);		
		center.x = pointC[0];
		center.y = pointC[1];
		center.z = pointC[2];
		source->GetCellNeighbors(-1, currentPoint, neighbourCells); // get all cells (triangles) that are connected with vertex j
		float maxDistance = 0;
		normal.x = 0;
		normal.y = 0;
		normal.z = 0;

		// compute vertex normal and find the longest edge
		for (int k = 0; k < neighbourCells->GetNumberOfIds(); k++) // for each found cell...
		{
			vtkCell *currCell = source->GetCell(neighbourCells->GetId(k));
			vtkIdType pointAid = currCell->GetPointId(0); // get the other two points of the triangle - A and B, C == points[i]
			vtkIdType pointBid = currCell->GetPointId(1);
			usedPointsMap[pointAid] = true; // mark the neighbour points as used
			usedPointsMap[pointBid] = true;
			// make sure the orientation is preserved
			if (pointAid == i) // if GetPointId(0) == C, it reset A to GetPointId(1) and B to GetPointId(2)
			{
				pointAid = pointBid;
				pointBid = currCell->GetPointId(2);
			}
			else if (pointBid == i) // if GetPointId(1) == C, it reset B to GetPointId(0) and A to GetPointId(2)
			{
				pointBid = pointAid;
				pointAid = currCell->GetPointId(2);
			} // else if GetPointId(2) == C, it is fine already
			source->GetPoint(pointAid, pointA);
			source->GetPoint(pointBid, pointB);
			edge1.x = pointA[0] - pointC[0];
			edge1.y = pointA[1] - pointC[1];
			edge1.z = pointA[2] - pointC[2];
			edge2.x = pointB[0] - pointC[0];
			edge2.y = pointB[1] - pointC[1];
			edge2.z = pointB[2] - pointC[2];
			MMSSVector3d currNormal = MMSSVector3d::VectorCross(edge1, edge2);
			// add the normalized surface normal of the triangle to the resulting vertex normal
			normal = normal + MMSSVector3d::Normalize(currNormal);
			// store the length of the longest edge - it will be used as a radius of the resulting sphere
			if (MMSSVector3d::VectorLength(edge1) > maxDistance)
				maxDistance = MMSSVector3d::VectorLength(edge1);
			if (MMSSVector3d::VectorLength(edge2) > maxDistance)
				maxDistance = MMSSVector3d::VectorLength(edge2);
			// use the 2-ring of the vertex as well
			currentNeighbours->SetId(0, pointAid);
			currentNeighbours->SetId(1, pointBid);
			source->GetCellNeighbors(-1, currentNeighbours, neighbourCells2); // get all cells (triangles) that are connected with the two neighbours->2-ring of vertex i
			for (int l = 0; l < neighbourCells2->GetNumberOfIds(); l++)
			{
				currCell = source->GetCell(neighbourCells2->GetId(l));
				for (int n = 0; n < 3; n++)
				{
					if (currCell->GetPointId(n) != pointAid && currCell->GetPointId(n) != pointBid) // if its not point A nor B
					{
						usedPointsMap[currCell->GetPointId(n)] = true;
						double distance = sqrt(ComputeDistanceSquared(pointC, source->GetPoint(currCell->GetPointId(n))));
						if (distance > maxDistance)
							maxDistance = distance;
					}
				}				
			}
		}
		// shift the center of the ball to the "inside" of the object, i.e. against its normal
		normal = MMSSVector3d::Normalize(normal);
		MMSSVector3d::ConstMult(&normal, maxDistance); // shift it by the length of the longest edge
		center = center - normal;
		radiuses.push_back(1.414 * maxDistance); // after the shift, the sphere will still touch the vertex that is most far - sqrt(2*maxD*maxD)
		centers.push_back(center);
		noOfSpheres++;
		neighbourCells->Reset();
	}
	vtkMSSDataSet *result = vtkMSSDataSet::New();
	MMSSVector3d *centerArray = new MMSSVector3d[noOfSpheres];
	double *radiusesArray = new double[noOfSpheres];
	for (int i = 0; i < noOfSpheres; i++)
	{
		centerArray[i] = centers[i];
		radiusesArray[i] = radiuses[i];
	}
	delete []usedPointsMap;
	radiuses.clear();
	centers.clear();
	currentPoint->Delete();
	currentNeighbours->Delete();
	neighbourCells->Delete();
	neighbourCells2->Delete();
	result->SetParticleRadsArray(radiusesArray);
	result->SetParticlesArray(centerArray);
	result->SetNumberOfParticles(noOfSpheres);
	return result;
}


vtkStandardNewMacro(vtkMassSpringBone);


/** Destructor */
vtkMassSpringBone::~vtkMassSpringBone()
{
	delete []vertices;
}

/** Prepares all data for processing. Returns true if succesful. */
bool vtkMassSpringBone::Preprocess(mafTransform *transform)
{
	if (data == NULL) return false;

	MMSSVector3d *particles = data->GetParticles();
	if(this->vertices) delete []this->vertices;
	this->vertices = new MMSSVector3d[data->GetNumberOfParticles()];
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		vertices[i].x = particles[i].x;
		vertices[i].y = particles[i].y;
		vertices[i].z = particles[i].z;
	}
	// transform the points
	double part[3];
	for (int i = 0; i < data->GetNumberOfParticles(); i++)
	{
		part[0] = vertices[i].x;
		part[1] = vertices[i].y;
		part[2] = vertices[i].z;
		transform->TransformPoint(part, part);
		vertices[i].x = part[0];
		vertices[i].y = part[1];
		vertices[i].z = part[2];
	}

	// setup bounding box
	if(this->bbox) delete this->bbox;
	this->bbox = new SphereBoundingBox(NULL, NULL, NULL); // they are the root BBs, so no parent
	this->bbox->SetContentFromArray(vertices, data->GetParticleRadiuses(), data->GetNumberOfParticles());
	this->bbox->ComputeBoundingBox();

	return true;
}

/** Clears the buffer for collsion forces - nothing for solid objects. */
void vtkMassSpringBone::ResetCollisions()
{	

}