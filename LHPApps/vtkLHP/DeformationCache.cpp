/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: DeformationCache.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-04-11 07:16:03 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "DeformationCache.h"

//////
/// Only instance of this singleton.
//////
/*static*/ DeformationCache DeformationCache::g_instance;


//////
/// Disposes memory used by this instance.
//////
void DeformationCache::Dispose()
{
	for (int i = 0; i < (int)this->records.size(); i++) {
		DeformationCacheRecord* record = this->records[i];
		delete record;
	}

	this->records.clear();
}

//////
/// Detects whetter data in this instance of cache are valid for specified transformation task.
/// @param originalMesh original vtkPolyData to be deformed (input)
/// @param originalSkeleton original polygonal skeleton of the mesh (input)
/// @returns true if parameters of deformation matches parameters of deformation kept in this cache
//////
int DeformationCache::ContainsDataForTransformation(vtkPolyData *originalMesh, vtkPolyData *originalSkeleton)
{
	for (int i = 0; i < (int)this->records.size(); i++) {
		DeformationCacheRecord* record = this->records[i];
		if (record->meshSurface == NULL || !this->ArePolyDataEqual(originalMesh, record->meshSurface->GetOriginal()))
		{
			continue;
		}

		if (originalSkeleton == NULL || record->origSkeletonCopy == NULL || !this->ArePolyDataEqual(originalSkeleton, record->origSkeletonCopy))
		{
			continue;
		}

		if (record->origSkeletonCopy == NULL && originalSkeleton == NULL && record->meshSurface == NULL) {
			continue;
		}

		return i;
	}
	
	
	return -1;
}

//////
/// Compares two vtkPolyData instances. Accepts them as equals if they have the same number of vertices
/// and same coordinates for all of them. It means that it doesn't compare all aspects of polyDatas, but
/// should be safe enough for distinguishing between two totally different meshes. Handles NULL values.
/// @param polyA first vtkPolyData to compare (input)
/// @param polyB second vtkPolyData to compare (input)
/// @returns true on match by internal points or false otherwise
//////
bool DeformationCache::ArePolyDataEqual(vtkPolyData *polyA, vtkPolyData *polyB)
{
	if (polyA == polyB)
	{
		return true;
	}

	if (polyA == NULL || polyB == NULL)
	{
		return false;
	}

	if (polyA->GetNumberOfPoints() != polyB->GetNumberOfPoints())
	{
		return false;
	}

	if (polyA->GetNumberOfCells() != polyB->GetNumberOfCells())
	{
		return false;
	}

	vtkIdType nPoints = polyA->GetNumberOfPoints();

	double pointA[3], pointB[3];

	// compare all points
	for (vtkIdType i = 0; i < nPoints; i++)
	{
		polyA->GetPoint(i, pointA);
		polyB->GetPoint(i, pointB);

		if (pointA[0] != pointB[0] || pointA[1] != pointB[1] || pointA[2] != pointB[2])
		{
			return false;
		}
	}

	return true;
}
	
//////
/// Disposes all content of  this cache and prepares it for new deformation.
/// @param originalMesh original mesh to be deformed
/// @param originalSkeleton original skeleton before deformation
//////
int DeformationCache::AddNewTransformation(vtkPolyData *originalMesh, vtkPolyData *originalSkeleton)
{
	DeformationCacheRecord *record = new DeformationCacheRecord();
	record->origSkeletonCopy = NULL;

	if (originalSkeleton != NULL) {
		record->origSkeletonCopy = vtkPolyData::New();
		record->origSkeletonCopy->DeepCopy(originalSkeleton);
	}	

	this->records.push_back(record);

	return (int)this->records.size() - 1;
}

#pragma region //getters and setters

//////
/// Gets deep copy of MeshSurface instance in cache.
/// It is up to user to dispose it afterwards.
/// @return deep copy of MeshSurface
//////
MeshSurface* DeformationCache::GetMeshSurfaceDeepCopy(int transformationId)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	return record->meshSurface->Clone();
}

//////
/// Sets deep copy of mesh surface. Copy of instance remains in 
/// the cache even after disposal of original and is not affected by changes in it.
/// @meshSurface original of mesh surface for deep copy to cache
//////
void DeformationCache::SetMeshSurfaceDeepCopy(int transformationId, MeshSurface* meshSurface)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (record->meshSurface != NULL)
	{
		delete record->meshSurface;
		record->meshSurface = NULL;
	}

	record->meshSurface = meshSurface;

	if (record->meshSurface != NULL)
	{
		record->meshSurface = record->meshSurface->Clone();
	}
}

//////
/// Gets deep copy array of nearest bone parts for every mesh vertex.
/// It is up to user to dispose it afterwards.
/// @param length of array (output)
/// @return deep copy of nearest bones per vertex
//////
int* DeformationCache::GetNearestBonesPerVertexDeepCopy(int transformationId, int *length)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (length != NULL)
	{
		*length = record->nearestBonesPerVertexLength;
	}

	int *output = new int[record->nearestBonesPerVertexLength];
	memcpy(output, record->nearestBonesPerVertex, sizeof(int) * (record->nearestBonesPerVertexLength));
	return output;
}

//////
/// Sets deep copy of array of nearest bone parts for every mesh vertex. Copy of instance remains in 
/// the cache even after disposal of original and is not affected by changes in it.
/// @nearestBonesPerVertex original of array of nearest bone parts for every mesh vertex for deep copy to cache
/// @param length of array (input)
//////
void DeformationCache::SetNearestBonesPerVertexDeepCopy(int transformationId, int* nearestBonesPerVertex, int length)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (record->nearestBonesPerVertex != NULL)
	{
		delete record->nearestBonesPerVertex;
		record->nearestBonesPerVertex = NULL;
	}

	record->nearestBonesPerVertexLength = length;
	
	record->nearestBonesPerVertex = new int[length];
	memcpy(record->nearestBonesPerVertex, nearestBonesPerVertex, sizeof(int) * length);
}

//////
/// Gets deep copy of weights of bone realtions for every vertex of mesh in cache.
/// It is up to user to dispose it afterwards.
/// @return deep copy of weights of bone realtions for every vertex of mesh
//////
PKMatrix* DeformationCache::GetVertexWeightsToBonesDeepCopy(int transformationId)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	return PKUtils::CloneMatrix(record->vertexWeightsToBones);
}

//////
/// Sets deep copy of weights of bone realtions for every vertex of mesh. Copy of instance remains in 
/// the cache even after disposal of original and is not affected by changes in it.
/// @vertexWeightsToBones original of weights of bone realtions for every vertex of mesh for deep copy to cache
//////
void DeformationCache::SetVertexWeightsToBonesDeepCopy(int transformationId, PKMatrix* vertexWeightsToBones)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (record->vertexWeightsToBones != NULL)
	{
		PKUtils::DisposeMatrix(&(record->vertexWeightsToBones));
	}

	record->vertexWeightsToBones = PKUtils::CloneMatrix(vertexWeightsToBones);
}

//////
/// Gets deep copy of laplacian matrix in cache.
/// It is up to user to dispose it afterwards.
/// @return deep copy of laplacian matrix
//////
PKMatrix* DeformationCache::GetLaplacianMatrixDeepCopy(int transformationId)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	return PKUtils::CloneMatrix(record->laplacianMatrix);
}

//////
/// Sets deep copy of laplacian matrix. Copy of instance remains in 
/// the cache even after disposal of original and is not affected by changes in it.
/// @laplacianMatrix original of laplacian matrix for deep copy to cache
//////
void DeformationCache::SetLaplacianMatrixDeepCopy(int transformationId, PKMatrix* laplacianMatrix)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (record->laplacianMatrix != NULL)
	{
		PKUtils::DisposeMatrix(&(record->laplacianMatrix));
	}

	record->laplacianMatrix = PKUtils::CloneMatrix(laplacianMatrix);
}

//////
/// Gets deep copy of laplacian operator in cache.
/// It is up to user to dispose it afterwards.
/// @return deep copy of laplacian operator
//////
PKMatrix* DeformationCache::GetLaplacianOperatorDeepCopy(int transformationId)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	return PKUtils::CloneMatrix(record->laplacianOperator);
}

//////
/// Sets deep copy of laplacian operator. Copy of instance remains in 
/// the cache even after disposal of original and is not affected by changes in it.
/// @laplacianOperator original of laplacian operator for deep copy to cache
//////
void DeformationCache::SetLaplacianOperatorDeepCopy(int transformationId, PKMatrix* laplacianOperator)
{
	if (transformationId < 0 || transformationId >= (int)this->records.size()) {
		throw "Invalid transformation ID.";
	}

	DeformationCacheRecord *record = this->records[transformationId];

	if (record == NULL) {
		throw "Cache record not initialized.";
	}

	if (record->laplacianOperator != NULL)
	{
		PKUtils::DisposeMatrix(&(record->laplacianOperator));
	}

	record->laplacianOperator = PKUtils::CloneMatrix(laplacianOperator);
}

#pragma endregion //getters and setters