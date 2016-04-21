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

#ifndef __vtkMassSpringMuscles_h
#define __vtkMassSpringMuscles_h

#include "vtkPolyData.h"
#include "vtkPolyDataToPolyDataFilter.h"
#include "vtkDoubleArray.h"
#include "MassSpringSystemCPU.h"
#include "medVMEMuscleWrapper.h"
#include "vtkCellArray.h"
#include "mafTransform.h"
#include <list>
#include <vector>

#include "vtkLHPConfigure.h"

#define NO_OF_CONTROL_PARTICLES 3 // the number of particles associated with each vertex of a mesh that control the movement of that vertex


/** Contains data needed for MSS processing. */
class VTK_vtkLHP_EXPORT vtkMSSDataSet : public vtkPolyData 
{
public:

	/** RTTI macro */
	vtkTypeMacro(vtkMSSDataSet, vtkPolyData);

protected:
	MMSSVector3d *particles; // an array of all the centers of particles in this dataset
	double *radiuses; // an array of radiuses of particles
	int *bone2IDs;   // an array of associated nearest two bone IDs of particles
	double *boneWeights; // an array of the associated nearest bone weights of particles
	int noOfParticles;
	IMassSpringSystem::Spring *springs; // an array of all springs in the dataset
	int noOfSprings;
	int noOfVertices; // number of vertices in the mesh, i.e. the length of individual arrays in closestParticles
	int **closestParticles; // Array of "NO_OF_CONTROL_PARTICLES" arrays, each containing "number-of-vertices" indices of particles
	//	i.e. on closestParticles[i][j] is the i-th control particle of j-th vertex
	unsigned int *fixedPointIndices; // indices of fixed points of the particle system
	int noOfFixedPoints;
	int *boundaryParticlesMap; // contains an index into the particle array of the i-th boundary particle
	int noOfBoundaryParticles; // number of boundary particles

	vtkDoubleArray* MVCWeights; // a 2-D array of mean value coordinate weights of the mesh, based on the boundary particles,
	// i.e. there is no-of-mesh-vertices entries, each noOfBoundaryParticles long
	vtkPolyData* BoundaryTriangulation; // a triangulation of the boundary particles, used for reconstruction of surface via MVC

	/** Constructor */
	vtkMSSDataSet();

	/** Desturctor */
	~vtkMSSDataSet();

	void DeallocateMemory();
public:

	/** Creates new instance of vtkMSSDataSet */
	static vtkMSSDataSet *New();

	/** Creates a deep copy of the object */
	void DeepCopy(vtkMSSDataSet *source);

	/** Returns the centers of particles associated with this object. */
	MMSSVector3d *GetParticles() { return this->particles; }

	/** Returns the radiuses of particles associated with this object. */
	double *GetParticleRadiuses() { return this->radiuses; }

	/** Returns the bone IDs of particles associated with this object. */
	int *GetParticleBoneIDs() { return this->bone2IDs; }

	/** Returns the nearest bone weights of particles associated with this object. */
	double *GetParticleBoneWeights() { return this->boneWeights; }

	/** Returns the indices of fixed points of the object. */
	unsigned int *GetFixedPoints() { return this->fixedPointIndices; }

	/** Returns the indices of the "control" particles which are closest to given vertex.
	It is an array of "NO_OF_CONTROL_PARTICLES" arrays, each containing "number-of-vertices" indices of particles
	i.e. on closestParticles[i][j] is the i-th control particle of j-th vertex. */
	int **GetClosestParticles() { return this->closestParticles; }

	/** Returns the springs associated with this object. */
	IMassSpringSystem::Spring *GetSprings() { return this->springs; }

	/** Returns the indices of boundary points of the object. */
	int *GetBoundaryParticles() { return this->boundaryParticlesMap; }

	/** Returns the mean value coordinates of the mesh of this object. */
	vtkGetObjectMacro(MVCWeights, vtkDoubleArray);

	/** Returns the triangulation of the boundary particles of this object. */
	vtkGetObjectMacro(BoundaryTriangulation, vtkPolyData);

	/** Sets the pointer to the spring array - the data are not copied! */
	void SetSpringArray(IMassSpringSystem::Spring *springs) { 
		if(this->springs) {
			delete []this->springs;
			this->springs = NULL;
		}
		this->springs = springs; 
	}

	/** Sets the pointer to the particles array (centers of particles) - the data are not copied! */
	void SetParticlesArray(MMSSVector3d *particles) { 
		if(this->particles) {
			delete []this->particles;
			this->particles = NULL;
		}
		this->particles = particles; 
	}

	/** Sets the pointer to the particle radiuses array - the data are not copied! */
	void SetParticleRadsArray(double *radiuses) { 
		if (this->radiuses) {
			delete []this->radiuses;
			this->radiuses = NULL;
		}
		this->radiuses = radiuses; 
	}

	/** Sets the pointer to the particle associated bone IDs array - the data are not copied! */
	void SetParticleBoneIDsArray(int *boneIDs) { 
		if(this->bone2IDs) {
			delete []this->bone2IDs;
			this->bone2IDs = NULL;
		}
		this->bone2IDs = boneIDs; 
	}

	/** Sets the pointer to the particle associated bone weights array - the data are not copied! */
	void SetParticleBoneWeightsArray(double *boneWeights) {
		if(this->boneWeights) {
			delete []this->boneWeights;
			this->boneWeights = NULL;
		}
		this->boneWeights = boneWeights;
	}

	/** Sets the pointer to the closest particles array - the data are not copied! */
	void SetClosestParticlesArray(int **closestParticles) { 
		if(this->closestParticles) {
			for (int i = 0; i < NO_OF_CONTROL_PARTICLES; i++)
				delete this->closestParticles[i];
			delete []this->closestParticles;
			this->closestParticles = NULL;
		}
		this->closestParticles = closestParticles; 
	}

	/** Sets the pointer to the fixed points array - the data are not copied! */
	void SetFixedPointsArray(unsigned int *fixedPointIndices) { 
		if(this->fixedPointIndices) {
			delete []this->fixedPointIndices;
			this->fixedPointIndices = NULL;
		}
		this->fixedPointIndices = fixedPointIndices; 
	}

	/** Sets the pointer to the boundary particles map array - the data are not copied! */
	void SetBoundaryParticlesMapArray(int *boundaryParticlesMap) { 
		if(this->boundaryParticlesMap) {
			delete []this->boundaryParticlesMap;
			this->boundaryParticlesMap = NULL;
		}
		this->boundaryParticlesMap = boundaryParticlesMap; 
	}

	/** Sets the pointer to the MVC weight array */
	vtkSetObjectMacro(MVCWeights, vtkDoubleArray);

	/** Sets the pointer to the triangulation of the boundary particles */
	vtkSetObjectMacro(BoundaryTriangulation, vtkPolyData);

	/** Returns the number of particles in this object */
	int GetNumberOfParticles() { 
		return this->noOfParticles; 
	}

	/** Returns the number of vertices of the mesh associated with this object */
	int GetNumberOfVertices() { 
		return this->noOfVertices; 
	}

	/** Returns the number of springs in this object. */
	int GetNumberOfSprings() { 
		return this->noOfSprings; 
	}

	/** Returns the number of fixed points in this object. */
	int GetNumberOfFixedPoints() { 
		return this->noOfFixedPoints; 
	}

	/** Returns the number of boundary points in this object. */
	int GetNumberOfBoundaryParticles() { 
		return this->noOfBoundaryParticles; 
	}

	/** Sets the number of particles. */
	void SetNumberOfParticles(int count) { 
		this->noOfParticles = count; 
	}

	/** Sets the number of vertices. */
	void SetNumberOfVertices(int count) { 
		this->noOfVertices = count; 
	}

	/** Sets the number of springs. */
	void SetNumberOfSprings(int count)  { 
		this->noOfSprings = count; 
	}

	/** Sets the number of fixed points. */
	void SetNumberOfFixedPoints(int count)  { 
		this->noOfFixedPoints = count; 
	}

	/** Sets the number of boundary points. */
	void SetNumberOfBoundaryParticles(int count)  { 
		this->noOfBoundaryParticles = count; 
	}
};

/** A class to represent a bounding box that can contain a set of spheres with a given center and radius.*/
class SphereBoundingBox
{

private:

	SphereBoundingBox *children[8]; // the children bounding boxes - max 8, none if it is leaf
	std::vector<int> content; // the indices of spheres stored in this bounding box - empty if it is not leaf
	int noOfSpheres; // the number of spheres this bounding box contains, i.e. the length of the "content" array
	MMSSVector3d *spheres; // an array of the actual spheres to which the "content" array points
	double *radiuses; // an array of radiuses of the spheres
	int queryNumber; // counts how many times the bounding box was involved in a collision query - it is used to determine which children are "active", i.e. used in prevoius query

	double cooMin[3];
	double cooMax[3];

	bool hasValidChildren; // true if the node is splitted correctly
	SphereBoundingBox *parent;

public:

	static const int  MIN_SPHERES_TO_SPLIT = 30;
	// Constructor
	// PARAM:
	// parent - A pointer to a parent bounding box
	// spheres - Array of spheres that belong to this bounding box 
	// radiuses - An array of radiuses of the spheres.
	SphereBoundingBox(SphereBoundingBox *parent, MMSSVector3d *spheres, double* radiuses);

	// Destructor
	~SphereBoundingBox();

	// Copies ids of all spheres from given array into the internal structure of the bounding box
	// PARAM:
	// contentArray - An array with all the spheres that should be inserted into this bounding box
	// radiuses - An array of radiuses of the spheres from content array.
	// boundary - Array with the indices of boundary particles. Only boundary particles will be used for collisions.
	// count - The number of items in the array
	void SetContentFromArray(MMSSVector3d *contentArray, double* radiuses, int* boundary, int count);

	// Copies ids of all spheres from given array into the internal structure of the bounding box
	// PARAM:
	// contentArray - An array with all the spheres that should be inserted into this bounding box
	// radiuses - An array of radiuses of the spheres from content array.
	// count - The number of items in the array
	void SetContentFromArray(MMSSVector3d *contentArray, double* radiuses, int count);

	// Splits the bounding box in 8 regular smaller bounding boxes and distributes the content among them
	void SplitRegularly();

	// Splitst the bounding box in such a way that the spliting lines go through the center of mass of all the spheres it contains
	// Not Implemented
	void SplitByContent();

	// Computes the boundary of this box based on its content.
	void ComputeBoundingBox();

	// Returns an array containing lower bounds of the bouning box.
	// RETURN:
	// 3-item array - x, y and z coordinate of the lowest corner of the bounding box
	double *GetBoundingBoxMin() { return this->cooMin; }

	// Returns an array containing higher bounds of the bouning box.
	// RETURN:
	// 3-item array - x, y and z coordinate of the highest corner of the bounding box
	double *GetBoundingBoxMax() { return this->cooMax; }

	// Returns the number of spheres contained in this bounding box.
	// RETURN:
	// The number of spheres
	int GetNumberOfSpheres() { return this->noOfSpheres; }

	// Returns whether this bounding box has valid children
	// RETURN:
	// True if the bounding box was already splitted
	bool HasValidChildren() { return this->hasValidChildren; }

	// Returns the indices of spheres contained in this bounding box.
	// RETURN:
	// A vector with sphere indices
	std::vector<int> GetContent() { return this->content; }

	// Returns whether this bounding box is active, i.e. it was used in the previous iteratiton.
	// PARAM:
	// queryNumber - how many queries were processed using this (or its root) bounding box
	// RETURN:
	// True if the bounding box was visited in the previous iteration of collision detection.
	bool IsActive(int queryNumber) { return this->queryNumber >= queryNumber; }

	// Sets whether this bounding box should be considered as active or not during next collision detection (i.e. if it was used in the last CD).
	// PARAM:
	// queryNumber - how many queries were processed using this (or its root) bounding box
	void SetActive(int queryNumber) { this->queryNumber = queryNumber; }

	// Sets whether this bounding box is splitted correctly
	// PARAM:
	// valid - true if this bounding box is splitted, false if the node should be re-split if needed
	void SetValid(bool valid) { this->hasValidChildren = valid; }

	// Returns the number of the last collision query in which this bounding box participated.
	// RETURN:
	// number of last collision query of this box
	int GetQueryNumber() { return this->queryNumber; }

	// Sets a new query number for this bounding box
	// PARAM:
	// the new query number (usually should be the current query that is being processed)
	void SetQueryNumber(int queryNumber) { this->queryNumber = queryNumber; }

	// Returns one of the eight children.
	// PARAM:
	// childrenID - a number from 0 to 7 (warning - the method does not check if the number is in bounds!!) which denotes the ID of the children. 
	// RETURN:
	// A pointer to the wanted children bounding box. Can be NULL (if children with that index does not exists)
	SphereBoundingBox *GetChildren(int childrenID) { return this->children[childrenID]; }

	// Inserts the ID of a given sphere into this node.
	// PARAM:
	// sphere - the ID of the sphere to be inserted
	void InsertSphere(int sphereID);

	// Tests whether given bounding box collides (i.e. overlaps) with this one 
	// PARAM:
	// bbox - the test subject
	// RETURN:
	// True if the boxes ovelap, false otherwise
	bool Collides(SphereBoundingBox *bbox);

	// Detect which parts of two bounding boxes collide with each other.
	// PARAM:
	// bbox - The bounding box that should be tested against this bounding box
	// currentQueryNumber - the number of the current collisiong query
	// reccursionCounter - Controls how many times can the bounding boxes be divided before piecewise (sphere vs. sphere) collision detection starts
	// RETURN:
	// List of pairs of indices of spheres that should be tested against each other
	std::vector<std::vector<int>> DetectCollision(SphereBoundingBox *bbox, int currentQueryNumber, int reccursionCounter = 5);

	// Traverse the children boudnig boxes, extracts the spheres stored in them.
	// PARAM:
	// completeContent - a pointer to a vector which will contain all the speheres stored in all the children bounding boxes
	void CollectContent(std::vector<int> *completeContent);

	// Remove all children it might have and refits its own bounding box
	void Rebuild();

	// Updates this bounding box and recursively its children based on the changes of the deformable object they encapsulate
	// PARAM:
	// queryNumber - how many queries were processed using this (or its root) bounding box
	void Update(int queryNumber);

	// Increments the query number of this bounding box.
	void IncreaseQueryNumber() { this->queryNumber++; }
};


/** A class used for simulation of the muscles as mass spring systems.
Contains methods for both preparation and processing of the data. */
class VTK_vtkLHP_EXPORT vtkMassSpringMuscle : public vtkPolyDataToPolyDataFilter
{
public:
	vtkTypeMacro(vtkMassSpringMuscle, vtkPolyDataToPolyDataFilter);

protected:
	vtkMSSDataSet *data;
	vtkPolyData *geometry; // mesh of the object
	MassSpringSystemCPU *mss;
	MMSSVector3d *vertices; // the current positions of the particle centers
	MMSSVector3d *verticesPrev; // the positions of the particle centers in previous simulation step (changes immediately after the step is done, used by the MSS)
	MMSSVector3d *positionPrev; // the positions of the particle centers in previous simulation step (does not change immediately, can be used for mesh update)
	MMSSVector3d* tempCollisionForces; //for storing forces during collision
	std::list<int> *collidingVertices;
	int* numberOfCollisions; // the number of colliding vertices of another system that collided with a given vertex of this system
	SphereBoundingBox *bbox;
	bool* verticesFlags;//1 means fixed, 0 means free


	/** Consturctor */
	vtkMassSpringMuscle();

	/** Desturctor */
	~vtkMassSpringMuscle();

public:

	static vtkMassSpringMuscle *New();


	/* methods for data processing */
private:

	static inline double ComputeDistanceSquared(double *vertexA, double *vertexB)
	{
		return (vertexA[0] - vertexB[0]) * (vertexA[0] - vertexB[0])
			+ (vertexA[1] - vertexB[1]) * (vertexA[1] - vertexB[1])
			+ (vertexA[2] - vertexB[2]) * (vertexA[2] - vertexB[2]);
	}

	/** Updates the mesh based on the changes in the mass spring system (represented by the Vector arrays) */
	void updateMesh();

	/** Updates particle positions of the object */
	void updatePositions();

	/** Adds a given force to a given vertex.
	vertexIndex - index of the vertex (particle) to which the force is applied
	force - the force vector */
	void AddCollisionForce(int vertexIndex, MMSSVector3d force);


public:

	/** Prepares for another batch of collision detection. */
	virtual void ResetCollisions();

	/** Processes the temporal collision forces accumulated during the resolution of a collision with various other objects. */
	virtual void ProcessCollisions();

	/** Computes the average difference of particle positions in between two simulation steps.
	Assumes the update method is called after each call of this method.
	RETURN:
	an average of lengths of "(postion[i] - previousPosition[i])" vectors*/
	float ComputeAverageDifference();

	/** Updates the output. */
	void Update();

	/** Detects and resolves collision of two MSS muscles
	otherObject - the second muscle to collide with this one.
	iterationNumber - the number of collision detection querries were made in the whole scene so far, i.e.
	how many times was completed the loop "for all objects ResetCollisions()->for all objects CollideWithAnotherObject()->
	for all objects ProcessCollisions()". */
	void CollideWithAnotherObject(vtkMassSpringMuscle *otherObject, int iterationNumber);

	/** Returns the vtkPolyData object conaining the (deformed) geometry of this SimObject. */
	vtkPolyData *GetGeometry() { return this->geometry; }

	/** Generates a vtkPolyData object containing a set of spheres that represents all the particles. */
	vtkPolyData *GetParticlesAsOneMesh();

	/** Get the deformed positions of particles. */
	void GetParticles(std::vector<medVMEMuscleWrapper::CParticle>& particles);

	/** Returns the mass spring system associated with this muscle.*/
	MassSpringSystemCPU *GetMSS() { return this->mss; }

protected:
	/** Returns the vertices (particles as a null-mass points, i.e. their center) associated with this object. */
	MMSSVector3d *GetVertices() { return this->vertices; }

	/** Returns the root bounding box of this object. */
	SphereBoundingBox* GetBoundingBox() { return this->bbox; }

	/** Returns true if a vertex with the given index is a fixed point.
	pointIndex - the index of the point of querry */
	virtual bool IsFixed(int pointIndex) { return verticesFlags[pointIndex]; }


	/* methods for data preparation */

public:

	/** Creates and returns the data structures required by the mass spring system.
	Call this method if the data are not already ready and stored somewhere. If they are, simply use SetInput.
	muscleParticles - vector with particles generated for the fibers of the muscle 
	muscleMesh - the surface mesh of the muscle */
	static vtkMSSDataSet *CreateData(std::vector<medVMEMuscleWrapper::CParticle>& muscleParticles, vtkPolyData *muscleMesh);

	/** Returns the MSS data. */
	vtkMSSDataSet *GetInput() { return (vtkMSSDataSet *)(this->vtkPolyDataToPolyDataFilter::GetInput()); }

	/** Returns the associated input mesh. */
	vtkPolyData *GetInputMesh() { return this->geometry; }

	/** Sets the input of the filter. Data must contain the particles, springs etc. of the system,
	geometry must contain the original mesh of the muscle that is to be deformed. */
	virtual void SetInput(vtkMSSDataSet *data, vtkPolyData *geometry);

	/** Prepares all data for processing. Returns true if succesful.
	transform - the transformation that should be applied to the fixed vertices
	(i.e. the transform between rest pose and current pose). */
	virtual bool PreprocessTransformFixedOnly(std::vector<mafTransform *>& transforms);
	virtual bool Preprocess(std::vector<mafTransform *>& transforms);

private:

	/** Generates the control "links" between particles and vertices of the mesh.
	Then updates the radius of each particle in order to prevent overlapping of particles
	and at the same time be as big as possible.
	Fills the vtkMSSDataSet object 'data' passed as argument.
	geometry = the mesh the data are bound to. */
	static void GenerateLinksUpdateRadiuses(vtkMSSDataSet *data, vtkPolyData *geometry);

	/** Computes the mean value coordinates of the input mesh based on the particle mesh.
	Only boundary particles are considered. They are triangularized with delaunay triangulation
	and then use the vtkMAFMeanvalueCoordinateInterpolation for the actual computation.
	Fills the vtkMSSDataSet object 'data' passed as argument.
	geometry = the mesh the data are bound to.
	*/
	static void ComputeMVCCoordinates(vtkMSSDataSet *data, vtkPolyData *geometry);

public:

	/** Creates a set of balls describing the given mesh.
	source - the mesh to be transformed into a set of balls. */
	static vtkMSSDataSet *GenerateSolidObject(vtkPolyData *source);
};

/** an object to represent hard constraints */
class VTK_vtkLHP_EXPORT vtkMassSpringBone : public vtkMassSpringMuscle
{
protected:

	/** Consturctor */
	vtkMassSpringBone(){}

	/** Desturctor */
	~vtkMassSpringBone();

public:

	static vtkMassSpringBone *New();

	/** Prepares all data for processing. Returns true if succesful.
	transform - the transformation that should be applied. */
	bool Preprocess(mafTransform *transform);

	/** Sets the input of the filter. */
	void SetInput(vtkMSSDataSet *data) { 
		this->data = data; this->vtkPolyDataToPolyDataFilter::SetInput(data); 
	}

	/** Returns true always - bones have all points fixed. */
	bool IsFixed(int pointIndex) { return true; }

	/** Prepares for another batch of collision detection. */
	void ResetCollisions();

	/** Processes the temporal collision forces accumulated during the resolution of a collision with another object
	- NOTHING for bones. */
	void ProcessCollisionForces() {}
};

#endif