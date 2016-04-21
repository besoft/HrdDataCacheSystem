/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: DeformationCache.h,v $ 
  Language: C++ 
  Date: $Date: 2011-04-11 07:16:04 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#ifndef DeformationCache_h__
#define DeformationCache_h__

#pragma once
#include <vector>

// VTK
#include "vtkPolyData.h"

// My code
#include "PKUtils.h"
#include "MeshSurface.h"
#include "vtkLHPConfigure.h"


using namespace std;

//////
/// Matrix structure for algebraic functions.
//////
typedef struct DeformationCacheRecord
{
	//////
	/// Cached instance of mesh surface.
	//////
	MeshSurface *meshSurface;

	//////
	/// List of nearest bone ids, one value for each mesh vertex.
	//////
	int *nearestBonesPerVertex;

	//////
	/// Lenght of list of nearest bone ids, one value for each mesh vertex.
	//////
	int nearestBonesPerVertexLength;

	//////
	/// Matrix of weights of bones for each vertex of mesh.
	/// Each row contains weights to all bones for single mesh vertex.
	/// Weights are normed to range <0; 1>, where 1 is nearest vertex,
	/// 0 is furthest, and values between are lineary interpolated.
	//////
	PKMatrix *vertexWeightsToBones;

	//////
	/// Cached laplacian matrix.
	//////
	PKMatrix *laplacianMatrix;

	//////
	/// Cached laplacian operator.
	//////
	PKMatrix *laplacianOperator;

	//////
	/// Control copy of original skeleton for deformation inputs match validation.
	//////
	vtkPolyData *origSkeletonCopy;

	DeformationCacheRecord::DeformationCacheRecord() {
		this->meshSurface = NULL;
		this->nearestBonesPerVertex = NULL;
		this->nearestBonesPerVertexLength = 0;
		this->vertexWeightsToBones = NULL;
		this->laplacianMatrix = NULL;
		this->laplacianOperator = NULL;
		this->origSkeletonCopy = NULL;
	}

	DeformationCacheRecord::~DeformationCacheRecord() {
		if (this->meshSurface != NULL)
		{
			delete this->meshSurface;
			this->meshSurface = NULL;
		}

		if (this->nearestBonesPerVertex != NULL)
		{
			delete this->nearestBonesPerVertex;
			this->nearestBonesPerVertex = NULL;
		}

		this->nearestBonesPerVertexLength = 0;

		if (this->vertexWeightsToBones != NULL)
		{
			PKUtils::DisposeMatrix(&(this->vertexWeightsToBones));
			this->vertexWeightsToBones = NULL;
		}

		if (this->laplacianMatrix != NULL)
		{
			PKUtils::DisposeMatrix(&(this->laplacianMatrix));
			this->laplacianMatrix = NULL;
		}

		if (this->laplacianOperator != NULL)
		{
			PKUtils::DisposeMatrix(&(this->laplacianOperator));
			this->laplacianOperator = NULL;
		}

		if (this->origSkeletonCopy != NULL)
		{
			this->origSkeletonCopy->Delete();
			this->origSkeletonCopy = NULL;
		}
	}
	
} 
DeformationCacheRecord;

//////
/// Class for caching of deformation parameters to speed up preprocessing.
//////
class VTK_vtkLHP_EXPORT DeformationCache
{
public:
	//////
	/// Optionally creates and returns instance of this singleton.
	/// RELEASE NOTE: the caller must call Dispose method when the data is no longer needed!
	/// @return instance of this singleton
	//////
	inline static DeformationCache* GetInstance() {
		return &g_instance;
	}

	//////
	/// Disposes memory used by this instance.
	//////
	void Dispose();

	//////
	/// Detects whetter data in this instance of cache are valid for specified transformation task.
	/// @param originalMesh original vtkPolyData to be deformed (input)
	/// @param originalSkeleton original polygonal skeleton of the mesh (input)
	/// @returns true if parameters of deformation matches parameters of deformation kept in this cache
	//////
	int ContainsDataForTransformation(vtkPolyData *originalMesh, vtkPolyData *originalSkeleton = NULL);

	//////
	/// Disposes all content of  this cache and prepares it for new deformation.
	/// @param originalMesh original mesh to be deformed
	/// @param originalSkeleton original skeleton before deformation
	//////
	int AddNewTransformation(vtkPolyData *originalMesh, vtkPolyData *originalSkeleton = NULL);


		/********************** /
		/ getters and setters   /
		/***********************/

	//////
	/// Gets deep copy of MeshSurface instance in cache.
	/// It is up to user to dispose it afterwards.
	/// @return deep copy of MeshSurface
	//////
	MeshSurface* GetMeshSurfaceDeepCopy(int transformationId);

	//////
	/// Sets deep copy of mesh surface. Copy of instance remains in 
	/// the cache even after disposal of original and is not affected by changes in it.
	/// @meshSurface original of mesh surface for deep copy to cache
	//////
	void SetMeshSurfaceDeepCopy(int transformationId, MeshSurface* meshSurface);

	//////
	/// Gets deep copy array of nearest bone parts for every mesh vertex.
	/// It is up to user to dispose it afterwards.
	/// @param length of array (output)
	/// @return deep copy of nearest bones per vertex
	//////
	int* GetNearestBonesPerVertexDeepCopy(int transformationId, int *length);

	//////
	/// Sets deep copy of array of nearest bone parts for every mesh vertex. Copy of instance remains in 
	/// the cache even after disposal of original and is not affected by changes in it.
	/// @nearestBonesPerVertex original of array of nearest bone parts for every mesh vertex for deep copy to cache
	/// @param length of array (input)
	//////
	void SetNearestBonesPerVertexDeepCopy(int transformationId, int* nearestBonesPerVertex, int length);

	//////
	/// Gets deep copy of weights of bone realtions for every vertex of mesh in cache.
	/// It is up to user to dispose it afterwards.
	/// @return deep copy of weights of bone realtions for every vertex of mesh
	//////
	PKMatrix* GetVertexWeightsToBonesDeepCopy(int transformationId);

	//////
	/// Sets deep copy of weights of bone realtions for every vertex of mesh. Copy of instance remains in 
	/// the cache even after disposal of original and is not affected by changes in it.
	/// @vertexWeightsToBones original of weights of bone realtions for every vertex of mesh for deep copy to cache
	//////
	void SetVertexWeightsToBonesDeepCopy(int transformationId, PKMatrix* vertexWeightsToBones);

	//////
	/// Gets deep copy of laplacian matrix in cache.
	/// It is up to user to dispose it afterwards.
	/// @return deep copy of laplacian matrix
	//////
	PKMatrix* GetLaplacianMatrixDeepCopy(int transformationId);

	//////
	/// Sets deep copy of laplacian matrix. Copy of instance remains in 
	/// the cache even after disposal of original and is not affected by changes in it.
	/// @laplacianMatrix original of laplacian matrix for deep copy to cache
	//////
	void SetLaplacianMatrixDeepCopy(int transformationId, PKMatrix* laplacianMatrix);

	//////
	/// Gets deep copy of laplacian operator in cache.
	/// It is up to user to dispose it afterwards.
	/// @return deep copy of laplacian operator
	//////
	PKMatrix* GetLaplacianOperatorDeepCopy(int transformationId);

	//////
	/// Sets deep copy of laplacian operator. Copy of instance remains in 
	/// the cache even after disposal of original and is not affected by changes in it.
	/// @laplacianOperator original of laplacian operator for deep copy to cache
	//////
	void SetLaplacianOperatorDeepCopy(int transformationId, PKMatrix* laplacianOperator);

private:
	//////
	/// Creates new instance of cache. Initalizes internal attributes to zero values.
	//////
	DeformationCache() {
	}

	//////
	///RELEASE NOTE: this cannot call Dispose since it would cause crash of the application because
	///several VTK static objects are already destroyed when the static singleton is being removed
	///to avoid memory leaks, the used has to call Dispose
	//////
	~DeformationCache() {
	}

	//////
	/// Compares two vtkPolyData instances. Accepts them as equals if they have the same number of vertices
	/// and same coordinates for all of them. It means that it doesn't compare all aspects of polyDatas, but
	/// should be safe enough for distinguishing between two totally different meshes. Handles NULL values.
	/// @param polyA first vtkPolyData to compare (input)
	/// @param polyB second vtkPolyData to compare (input)
	/// @returns true on match by internal points or false otherwise
	//////
	bool ArePolyDataEqual(vtkPolyData *polyA, vtkPolyData *polyB);

		//***********//
		// variables //
		// **********//

	//////
	/// Only instance of this singleton.
	//////
	static DeformationCache g_instance;

	vector<DeformationCacheRecord*> records;
};

#endif