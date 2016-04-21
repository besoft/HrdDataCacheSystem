/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: IMultiMeshDeformer.h,v $ 
  Language: C++ 
  Date: $Date: 2012-02-08 14:27:41 $ 
  Version: $Revision: 1.1.2.4 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef IMULTIMESHDEFORMER_H_
#define IMULTIMESHDEFORMER_H_

#pragma once

using namespace std;

#include "vtkPolyData.h"

//////
/// Interface of filter able to deform multiple meshes at once according
/// their skeletons.
/// Can prevent intersections between meshes and also with another non-deformable obstacles.
//////
class IMultiMeshDeformer {
public :
	
	//*************************************************************************************************************************************
	// Main method
	//*************************************************************************************************************************************

	//////
	/// Synchronous execution of deformation.
	///
	/// @return true no error occurred, if false results may not be relevant
	//////
	virtual bool ExecuteMultiData() = 0;
	
	//*************************************************************************************************************************************
	// Deformable meshes
	//*************************************************************************************************************************************

	virtual void AddWrapper(vtkPolyData *w) = 0;
	virtual void FreeWrappers() = 0;

	//////
	/// Sets number of meshes for deformation (deformable inputs).
	///
	/// Used for memory preallocation.
	/// @param count number of meshes for deformation (input)
	//////
	virtual void SetNumberOfMeshes(int count) = 0;

	//////
	/// Gets number of meshes for deformation (deformable inputs).
	///
	/// Used for memory preallocation.
	/// @return number of meshes for deformation
	//////
	virtual int GetNumberOfMeshes() = 0;
	
	//////
	/// Sets input deformable mesh at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	virtual void SetInputMesh(int meshIndex, vtkPolyData *poly) = 0;
	
	//////
	/// Gets input deformable mesh at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return poly vtkPolyData - soft reference - must not be freed outside
	//////
	virtual vtkPolyData* GetInputMesh(int meshIndex) = 0;
	
	//////
	/// Sets output for result of deformable mesh at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
	//////
	virtual void SetOutputMesh(int meshIndex, vtkPolyData *poly) = 0;

	//////
	/// Sets output for result of deformable mesh coarse at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
	//////
	virtual void SetOutputMeshCoarse(int meshIndex, vtkPolyData *poly) = 0;

	//////
	/// Gets output for result of deformable mesh at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return poly vtkPolyData - soft reference
	//////
	virtual vtkPolyData* GetOutputMesh(int meshIndex) = 0;

	//*************************************************************************************************************************************
	// Skeletons of deformable meshes
	//*************************************************************************************************************************************

	//////
	/// Sets number of skeletons for deformable mesh at specified index.
	///
	/// Used for memory preallocation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param count number of skeletons (input)
	//////
	virtual void SetNumberOfMeshSkeletons(int meshIndex, int count) = 0;

	//////
	/// Gets number of skeletons for deformable mesh at specified index.
	///
	/// Used for memory preallocation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return count number of skeletons (input)
	//////
	virtual int GetNumberOfMeshSkeletons(int meshIndex) = 0;

	//////
	/// Sets skeleton on index for deformable mesh at specified index.
	///
	/// Used for memory preallocation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param skeletonIndex index of skeleton for specified mesh
	/// @param original original skeleton
	/// @param deformed modified skeleton
	/// ... others taken from original vtkMEDPolyDataDeformation interface and not much used
	/// and as they have default value, I think they can be left out, but I am not yet sure.
	///
	/// Original desc:
	/// 
	/// Specifies the n-th control skeleton. 
	/// If RSO points are specified, they are used during the computation of LFs
	/// of curves of both skeletons. A local fame is defined by its origin point 
	/// and three vectors u, v and w. Vector u is the tangent vector (it goes in
	/// the direction of polyline) and vectors v,w are perpendicular to this vector.
	/// As there is infinite number of u,v,w configurations, the algorithm uses the
	/// given RSO point to get a unique one (v lies in the plane defined by u and RSO). 
	/// If RSO is not specified, v is chosen to lie in the plane closest to the u vector.
	/// When RSO points are not specified (or they are specified incorrectly), 
	/// the deformed object might be unrealistically rotated against other objects 
	/// in the scene, if the skeleton of object to deform tends to rotate (simple edge, 
	/// or only one skeleton for object).
	/// 
	/// @return count number of skeletons (input)
	//////
	virtual void SetMeshSkeleton(int meshIndex, int skeletonIndex, vtkPolyData* original, vtkPolyData* modified, vtkIdList* correspondence = NULL, double* original_rso = NULL, double* modified_rso = NULL) = 0;
	
	//*************************************************************************************************************************************
	// Hard obstacles (bones)
	//*************************************************************************************************************************************
	
	//////
	/// Sets number of meshes used as hard obstacles (non-deformable inputs).
	///
	/// Used for memory preallocation.
	/// @param count number of meshes as obstacles (input)
	//////
	virtual void SetNumberOfObstacles(int count) = 0;

	//////
	/// Gets number of meshes used as hard obstacles (non-deformable inputs).
	///
	/// Used for memory preallocation.
	/// @return number of meshes as obstacles
	//////
	virtual int GetNumberOfObstacles() = 0;

	//////
	/// Sets input non-deformable mesh (obstacle) at specified index.
	///
	/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	virtual void SetObstacle(int obstacleIndex, vtkPolyData *poly) = 0;

	//////
	/// Gets input non-deformable mesh (obstacle) at specified index.
	///
	/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @return poly vtkPolyData - soft reference - must not be freed outside
	//////
	virtual vtkPolyData* GetObstacle(int obstacleIndex) = 0;

	//*************************************************************************************************************************************
	// Coarse meshes
	//*************************************************************************************************************************************

	//////
	/// Sets coarse mesh for deformable mesh at specified index.
	/// This method can be called to spped up preprocessing using stored cached values.
	/// If not set up, coarse mesh is constructed during deformation. This value
	/// can then be read from matching getter method.
	/// Internal in-memory caching is also used to avoid such behaviour during multiple deformations
	/// in same application instance.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param coarse vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	virtual void SetCoarseMesh(const int meshIndex, vtkPolyData* coarse) = 0;

	//////
	/// Gets coarse mesh for deformable mesh at specified index.
	/// This can be mesh from matching setter or calculated mesh.
	/// Make deep copy of returned pointer for your needs.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return vtkPolyData - soft reference - must not be freed outside
	//////
	virtual vtkPolyData* GetCoarseMesh(const int meshIndex) = 0;


	//////
	/// Sets coarse mesh for obstacle mesh at specified index.
	/// This method can be called to spped up preprocessing using stored cached values.
	/// If not set up, coarse mesh is constructed during deformation. This value
	/// can then be read from matching getter method.
	/// Internal in-memory caching is also used to avoid such behaviour during multiple deformations
	/// in same application instance.
	///
	/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @param coarse vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	virtual void SetCoarseObstacle(const int obstacleIndex, vtkPolyData* coarse) = 0;

	//////
	/// Gets coarse mesh for obstacle mesh at specified index.
	/// This can be mesh from matching setter or calculated mesh.
	/// Make deep copy of returned pointer for your needs.
	///
	/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @return vtkPolyData - soft reference - must not be freed outside
	//////
	virtual vtkPolyData* GetCoarseObstacle(const int obstacleIndex) = 0;

	//*************************************************************************************************************************************
	// Settings
	//*************************************************************************************************************************************

	//////
	/// Sets whether to prevent collisions.
	/// With true, intersections of two meshes should be prevented at the price
	/// of high computational demands.
	/// On false settings, all hard obstacles are ignored as well.
	///
	/// @param prevent true to prevent collisions
	//////
	virtual void SetPreventCollisions(bool prevent) = 0;

	//////
	/// Gets whether to prevent collisions.
	/// With true, intersections of two meshes should be prevented at the price
	/// of high computational demands.
	/// On false settings, all hard obstacles are ignored as well.
	///
	/// @return true if prevents collisions
	//////
	virtual bool GetPreventCollisions() = 0;

	//////
	/// Sets if progressive hulls should be used to represent coarse meshes
	/// With true, much slower calculation is used to produce progressive hull 
	/// that have a tendency to be more stable	
	///
	/// @param enabled true, if progressive hulls should be used
	//////
	virtual void SetUseProgressiveHulls(bool enabled) = 0;

	//////
	/// Gets if progressive hulls should be used to represent coarse meshes
	/// With true, much slower calculation is used to produce progressive hull 
	/// that have a tendency to be more stable	
	///
	/// @return true if progressive hulls should be used
	//////
	virtual bool GetUseProgressiveHulls() = 0;

	//////
	/// Sets if grid will be used to boost the ray casting.
	///
	/// @param enabled true, if grid will be used to boost the ray casting
	//////
	virtual void SetUseGrid(bool enabled) = 0;

	//////
	/// Gets if grid will be used to boost the ray casting.
	///
	/// @return true, if grid will be used to boost the ray casting
	//////
	virtual bool GetUseGrid() = 0;


	//////
	/// Sets if the method should visualize its progress
	///
	/// @param enabled true, if the method should visualize its progress
	//////
	virtual void SetDebugMode(bool enabled) = 0;

	//////
	/// Gets if grid will be used to boost the ray casting.
	///
	/// @return true, if debug mode is on and the method will visualize its progress
	//////
	virtual bool GetDebugMode() = 0;
};

#endif 