/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshSkeleton.h,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:48:04 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef MeshSkeleton_h__
#define MeshSkeleton_h__

#pragma once

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkMath.h"
#include "PKUtils.h"

//////
/// Number of segments to create from each original bones when interpolationg.
//////
#define BONE_PART_COUNT 10

//////
/// Ignorable distance from bone.
//////
#define EPSION_SKELETON 0.0000000001

typedef struct CONTROL_SKELETON    // structure for keeping of pure action lines before deformation itself
	{  
		vtkPolyData* pPolyLines[2];    //<polyline curves (0 = original, 1 = deformed)
		vtkIdList* pCCList;            //<list of correspondences between curves

		bool RSOValid[2];              //<specifies, if RSO is valid
		double RSO[2][3];              //<RSO point
	} CONTROL_SKELETON;

//////
/// Class for storage of interpolated original and deformed skeleton.
//////
class MeshSkeleton
{
public:
	//////
	/// Creates new skeleton deposit from original and deformed skeleton.
	/// @param original original skeleton (input)
	/// @param deformed deformed skeleton (input)
	//////
	MeshSkeleton(vtkPolyData *original, vtkPolyData *deformed);

	MeshSkeleton(CONTROL_SKELETON* skeletons, int count);

	//////
	/// Disposes skeleton and resources.
	//////
	~MeshSkeleton(void);

	//////
	/// Gets number of bone segments in skeleton.
	/// @return number of bones
	//////
	int GetNumberOfBones();

	int GetNumberOfMainBones();

	//////
	/// Gets start and end point of original bone specified by ID.
	/// @param id selection of bone (input)
	/// @param pointA pointer to starting point (output)
	/// @param pointB pointer to ending point (output)
	//////
	void GetOriginalEdge(int id, double **pointA, double **pointB);

	//////
	/// Gets start and end point of deformed bone specified by ID.
	/// @param id selection of bone (input)
	/// @param pointA pointer to starting point (output)
	/// @param pointB pointer to ending point (output)
	//////
	void GetDeformedEdge(int id, double **pointA, double **pointB);

	//////
	/// Measures distance of vertex from all bone segments.
	/// @param point 3D coordinates of measured vertex (input)
	/// @param distances preallocated array of length GetNumberOfBones() for distances for all bones (output)
	/// @param minDist pointer to minimum measured distance, may be NULL (output)
	/// @param maxDist pointer to maximum measured distance, may be NULL (output)
	///	@param sumDist pointer to sum of all measured distances, may be NULL (output)
	//////
	void MeasurePointDistances(double point[], double *distances, double *minDist, double *maxDist, double *sumDist, bool deformed = false);

	bool GetRsoRotation(double axis[3], double &angle);

private:
	//////
	/// Interpolates skeletons and stores their coordinates.
	/// @param CONTROL_SKELETON* skeletons skeletons (input by soft reference)
	/// @param int count (input)
	//////
	void AnalyzeSkeletons(CONTROL_SKELETON* skeletons, int count);

	//////
	/// Measure lengths of uninterpolated skeleton parts.
	/// @param points original point coordinates (input)
	/// @param origPointIds array of input point ids (input)
	/// @param origPointCount length of origPointIds (input)
	/// @param lengths array of lengths of all parts (output)
	/// @return total length of skeleton
	//////
	double MeasureSkeleton(vtkPoints *points, vtkIdType *origPointIds, vtkIdType origPointCount, double *lengths);

	//////
	/// Interpolates skeleton equidistantly to specified number of control points.
	/// @param skeleton input skeleton (input)
	/// @param finalPointCount number of control points, therefore number of final bone segments + 1 (input)
	/// @param finalPoints array of final control points of length finalPointCount, must be preallocated (output)
	//////
	//void InterpolateSkeletonToEquiDistPoints(vtkPolyData *skeleton, int finalPointCount, double **finalPoints);
	int InterpolateSkeletons(vtkPolyData *skeletonA, vtkPolyData *skeletonB, PKMatrix* &pointsA, PKMatrix* &pointsB, int finalPointCount);

	//////
	/// Measure distance from vertex to bone.
	/// @param boneId ID of bone (input)
	/// @param point 3D coordinates of vertex (input)
	/// @return minimum distance between point and bone
	//////
	double MeasurePointDistanceToBone(int boneId, double point[], bool deformed = false);



	//**********************************************************************************************************************
	// attributes
	//**********************************************************************************************************************

	//////
	/// List of original (undeformed) interpolated skeleton control points.
	//////
	PKMatrix *originalPoints;

	//////
	/// List of deformed interpolated skeleton control points.
	//////
	PKMatrix *deformedPoints;

	//////
	/// Number of control points (same for original and deformed skeleton).
	//////
	int controlPointCount;
	
	// in ORIGINAL skeleton coord system!!!
	double rsoAxis[3];
	double rsoAngle;
	bool rsoValid;

	int mainSkeletonEnd;
};

#endif