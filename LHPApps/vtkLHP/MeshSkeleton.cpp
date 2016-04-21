/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshSkeleton.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-11-11 09:09:08 $ 
  Version: $Revision: 1.1.2.5 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#include "MeshSkeleton.h"
#include "mafDbg.h"

#define MAX_POINT_DIFF 0.01

//////
/// Creates new skeleton deposit from original and deformed skeleton.
/// @param original original skeleton (input)
/// @param deformed deformed skeleton (input)
//////
MeshSkeleton::MeshSkeleton(vtkPolyData *original, vtkPolyData *deformed)
{
	this->originalPoints = NULL;
	this->deformedPoints = NULL;
	this->controlPointCount = 0;
	this->mainSkeletonEnd = 0;	
	this->rsoAngle = 0;
	this->rsoValid = false;

	CONTROL_SKELETON skeleton;
	skeleton.pPolyLines[0] = original;
	skeleton.pPolyLines[1] = deformed;
	skeleton.pCCList = NULL;
	skeleton.RSOValid[0] = skeleton.RSOValid[1] = false;
	
	this->AnalyzeSkeletons(&skeleton, 1);
}

MeshSkeleton::MeshSkeleton(CONTROL_SKELETON* skeletons, int count) {
	this->originalPoints = NULL;
	this->deformedPoints = NULL;
	this->controlPointCount = 0;
	this->mainSkeletonEnd = 0;
	this->AnalyzeSkeletons(skeletons, count);	

	/*this->originalPoints->DebugOutputMathematica("originalPoints.nb");
	this->deformedPoints->DebugOutputMathematica("deformedPoints.nb");*/
}

//////
/// Disposes skeleton and resources.
//////
MeshSkeleton::~MeshSkeleton(void)
{
	if (this->originalPoints != NULL)
	{
		PKUtils::DisposeMatrix(&(this->originalPoints));
		this->originalPoints = NULL;
	}
	
	if (this->deformedPoints != NULL)
	{
		PKUtils::DisposeMatrix(&(this->deformedPoints));
		this->deformedPoints = NULL;
	}
}

//////
/// Gets number of bone segments in skeleton.
/// @return number of bones
//////
int MeshSkeleton::GetNumberOfBones()
{
	return this->controlPointCount - 1;
}


int MeshSkeleton::GetNumberOfMainBones() {
	return this->mainSkeletonEnd - 1;
}

//////
/// Gets start and end point of original bone specified by ID.
/// @param id selection of bone (input)
/// @param pointA pointer to starting point (output)
/// @param pointB pointer to ending point (output)
//////
void MeshSkeleton::GetOriginalEdge(int id, double **pointA, double **pointB)
{
	if (id >= this->GetNumberOfBones())
	{
		throw "Index out of range.";
	}

	*pointA = this->originalPoints->values[id];
	*pointB = this->originalPoints->values[id + 1];
}

//////
/// Gets start and end point of deformed bone specified by ID.
/// @param id selection of bone (input)
/// @param pointA pointer to starting point (output)
/// @param pointB pointer to ending point (output)
//////
void MeshSkeleton::GetDeformedEdge(int id, double **pointA, double **pointB)
{
	if (id >= this->GetNumberOfBones())
	{
		throw "Index out of range.";
	}

	*pointA = this->deformedPoints->values[id];
	*pointB = this->deformedPoints->values[id + 1];
}

void MeshSkeleton::AnalyzeSkeletons(CONTROL_SKELETON* skeletons, int count) {

	double temp[3];
	double *rsoOrig = NULL;
	double *rsoDef = NULL;
	int totalPtCount = 0;
	int mainPtCount = 0;
	
	// allocate temp
	PKMatrix** originalPointsTemp = new PKMatrix*[count];
	memset(originalPointsTemp, NULL, sizeof(PKMatrix*));
	PKMatrix** deformedPointsTemp = new PKMatrix*[count];
	memset(deformedPointsTemp, NULL, sizeof(PKMatrix*));
		
	// count points
	for (int i = 0; i < count; i++) {
		vtkPolyData* original = skeletons[i].pPolyLines[0];
		vtkPolyData* deformed = skeletons[i].pPolyLines[1];

		if (original == NULL || deformed == NULL) {
			continue;
		}

		original->BuildCells();
		deformed->BuildCells();

		vtkIdType origPointCount = original->GetNumberOfPoints();
		vtkIdType deformedPointCount = deformed->GetNumberOfPoints();

		int desiredPointCount = (max(origPointCount, deformedPointCount) - 1) * BONE_PART_COUNT + 1;

		// split and interpolate skeletons
		int realPointCount = this->InterpolateSkeletons(original, deformed, originalPointsTemp[i], deformedPointsTemp[i], desiredPointCount);

		// add gained points to point count
		totalPtCount += realPointCount;

		// add space for RSO if valid
		if (skeletons[i].RSOValid[0] && skeletons[i].RSOValid[1]) {
			totalPtCount++;
			if (rsoOrig == NULL) {
				rsoOrig = skeletons[i].RSO[0];
				rsoDef = skeletons[i].RSO[1];
			}
		}

		// remember size of first skeleton => will be main and will be part of main point structure (from 0 to mainPtCount)
		if (mainPtCount <= 0) {
			mainPtCount = realPointCount;
		}
	}


	// allocate final
	this->originalPoints = PKUtils::CreateMatrix(totalPtCount, 3);
	this->deformedPoints = PKUtils::CreateMatrix(totalPtCount, 3);
	this->controlPointCount = totalPtCount;
	this->mainSkeletonEnd = mainPtCount;
	this->rsoValid = false;

	double **origPtr = this->originalPoints->values;
	double **deformedPtr = this->deformedPoints->values;


	// merge point matrices (temp => final)
	for (int i = 0; i < count; i++) {
		vtkPolyData* original = skeletons[i].pPolyLines[0];
		vtkPolyData* deformed = skeletons[i].pPolyLines[1];

		if (original == NULL || deformed == NULL) {
			continue;
		}

		// copy interpolated matrices
		for (int j = 0; j < originalPointsTemp[i]->height; j++) 
		{
			PKUtils::CopyVertex(originalPointsTemp[i]->values[j], *origPtr);
			PKUtils::CopyVertex(deformedPointsTemp[i]->values[j], *deformedPtr);

			origPtr++;
			deformedPtr++;
		}

		// free temp matrices
		PKUtils::DisposeMatrix(originalPointsTemp + i);
		PKUtils::DisposeMatrix(deformedPointsTemp + i);

		// copy RSO
		if (skeletons[i].RSOValid[0] && skeletons[i].RSOValid[1]) {
			PKUtils::CopyVertex(skeletons[i].RSO[0], origPtr[0]);
			PKUtils::CopyVertex(skeletons[i].RSO[1], deformedPtr[0]);

			origPtr++;
			deformedPtr++;
		}
	}

	// free temp
	delete[] originalPointsTemp;
	originalPointsTemp = NULL;
	delete[] deformedPointsTemp;
	deformedPointsTemp = NULL;

	// count rso direction
	if (rsoOrig != NULL && rsoDef != NULL) {
		double* rso[2] = { rsoOrig, rsoDef };
		double directions[2][3];
		double nearBones[2][3];

		for (int i = 0; i < 2; i++) {
			double minDist = 1E99;
			int nearestBone = 0;

			// find nearest bone
			for (int boneId = 0; boneId < this->GetNumberOfMainBones(); boneId++) {
				double dist = this->MeasurePointDistanceToBone(boneId, rso[i], i == 1);
				if (dist < minDist) {
					minDist = dist;
					nearestBone = boneId;
				}
			}

			double *a, *b;
			if (i == 0) {
				this->GetOriginalEdge(nearestBone, &a, &b);
			} else {
				this->GetDeformedEdge(nearestBone, &a, &b);
			}

			// project rso to line
			double plane[4];
			PKUtils::SubtractVertex(b, a, nearBones[i]);
			PKUtils::NormalizeVertex(nearBones[i]);
			PKUtils::CopyVertex(nearBones[i], plane);
			plane[3] = -PKUtils::Dot(rso[i], plane);
			double t = - (PKUtils::Dot(plane, a) + plane[3]);
			
			double projection[3];
			PKUtils::MultiplyVertex(plane, t);
			PKUtils::AddVertex(a, plane, projection);

			// calculate direction
			PKUtils::SubtractVertex(rso[i], projection, directions[i]);
			PKUtils::NormalizeVertex(directions[i]);
		}

		// rotate deformed dir from coords of deformed bone to coords of original bone
		double boneAngle;
		double boneAxis[3];
		PKUtils::GetRotation(nearBones[1], nearBones[0], boneAxis, boneAngle);
		RotationMatrix rotMat = RotationMatrix();
		PKUtils::RotationMatrixGeneral(boneAxis, boneAngle, &rotMat);
		PKUtils::MultiplyMatrixVertex(&rotMat, directions[1], temp);
		PKUtils::CopyVertex(temp, directions[1]);

		// calculate rotation axis for original skeleton coord system
		PKUtils::GetRotation(directions[0], directions[1], this->rsoAxis, this->rsoAngle);
		this->rsoValid = true;
	}
}

int cmpDouble(const void *a, const void *b) {
	if (*(double*)a < *(double*)b) {
		return -1;
	} else if (*(double*)a == *(double*)b) {
		return 0;
	} else {
		return 1;
	}
}

int MeshSkeleton::InterpolateSkeletons(vtkPolyData *skeletonA, vtkPolyData *skeletonB, PKMatrix* &pointsA, PKMatrix* &pointsB, int desiredPointCount) 
{
	// measure both skeletons

	vtkCellArray *cells;

	// A
	vtkIdType *origPointsA;
	vtkIdType origPointCountA;
	cells = skeletonA->GetLines();
	cells->InitTraversal();
	cells->GetNextCell(origPointCountA, origPointsA);
	double *origLengthsA = new double[origPointCountA - 1];
	double totalLengthA = this->MeasureSkeleton(skeletonA->GetPoints(), origPointsA, origPointCountA, origLengthsA);

	// B
	vtkIdType *origPointsB;
	vtkIdType origPointCountB;
	cells = skeletonB->GetLines();
	cells->InitTraversal();
	cells->GetNextCell(origPointCountB, origPointsB);
	double *origLengthsB = new double[origPointCountB - 1];
	double totalLengthB = this->MeasureSkeleton(skeletonB->GetPoints(), origPointsB, origPointCountB, origLengthsB);



	// initialize point parametrization sets with original points from A
	double* params = new double[origPointCountA + origPointCountB - 2];
	params[0] = 0.0;
	params[origPointCountA - 1] = 1.0;
	origLengthsA[0] /= totalLengthA;
	double t = 0;
	for (int i = 0; i < origPointCountA - 2; i++) {
		origLengthsA[i] /= totalLengthA;
		t += origLengthsA[i];
		params[i + 1] = t;
	}
	int paramsCount = origPointCountA;

	// add points from B
	t = 0;
	for (int i = 0; i < origPointCountB - 2; i++) {
		origLengthsB[i] /= totalLengthB;
		t += origLengthsB[i];
		
		// offset in paramsA to skip obviously identical ends (0/1)
		int j = PKUtils::FindNearestElement(t, params + 1, origPointCountA - 2);
		double diff = abs(t - params[j]);
		if (diff > MAX_POINT_DIFF) {
			// insert new point to params
			params[paramsCount] = t;
			paramsCount++;
		}
	}
	origLengthsB[origPointCountB - 2] /= totalLengthB;

	// sort params ASC
	qsort(params, paramsCount, sizeof(double), cmpDouble);

	// split segments to get approcimately desired point count

	// count precise final point count
	int finalPointCount = paramsCount;
	for (int i = 0; i < paramsCount - 1; i++) {
		double len = params[i + 1] - params[i];
		int middlePtsCount = (int)(len * desiredPointCount + 0.5) - 1;
		if (middlePtsCount > 0) {
			finalPointCount += middlePtsCount;
		}
	}

	// allocate memory
	pointsA = PKUtils::CreateMatrix(finalPointCount, 3);
	pointsB = PKUtils::CreateMatrix(finalPointCount, 3);

	// calculate points for params on both skeletons
	double a[3], b[3];
	int segmentA = 0, segmentB = 0;
	double previousSegmentsSumA = 0.0, previousSegmentsSumB = 0.0;
	int pointIndex = 0;
	for (int i = 0; i < paramsCount - 1; i++) {

		// calculate subdivisions
		double len = params[i + 1] - params[i];
		int divisionsCount = (int)(len * desiredPointCount + 0.5);
		if (divisionsCount < 1) {
			divisionsCount = 1;
		}
		double step = len / divisionsCount;
		double t = params[i];

		// generate points <i...i+1)
		for (int j = 0; j < divisionsCount; j++, t += step, pointIndex++) {
			// skeleton A
			double inSegmentT = t - previousSegmentsSumA;
			
			while (inSegmentT > origLengthsA[segmentA] + MAX_POINT_DIFF) {
				// move to next segment
				previousSegmentsSumA += origLengthsA[segmentA];
				segmentA++;
				inSegmentT = t - previousSegmentsSumA;
			}

			// linear interpolation
			skeletonA->GetPoint((vtkIdType)segmentA, a);
			skeletonA->GetPoint((vtkIdType)segmentA + 1, b);

			PKUtils::SubtractVertex(b, a, b);
			PKUtils::MultiplyVertex(b, inSegmentT / origLengthsA[segmentA]);
			PKUtils::AddVertex(a, b, pointsA->values[pointIndex]);

			// skeleton B
			inSegmentT = t - previousSegmentsSumB;
			
			while (inSegmentT > origLengthsB[segmentB] + MAX_POINT_DIFF) {
				// move to next segment
				previousSegmentsSumB += origLengthsB[segmentB];
				segmentB++;
				inSegmentT = t - previousSegmentsSumB;
			}

			// linear interpolation
			skeletonB->GetPoint((vtkIdType)segmentB, a);
			skeletonB->GetPoint((vtkIdType)segmentB + 1, b);

			PKUtils::SubtractVertex(b, a, b);
			PKUtils::MultiplyVertex(b, inSegmentT / origLengthsB[segmentB]);
			PKUtils::AddVertex(a, b, pointsB->values[pointIndex]);
		}
	}

	// last points
	skeletonA->GetPoint(origPointCountA - 1, pointsA->values[pointIndex]);
	skeletonB->GetPoint(origPointCountB - 1, pointsB->values[pointIndex]);
	pointIndex++;

	_ASSERTE(pointIndex == finalPointCount);


	// free memory
	delete[] origLengthsA;
	delete[] origLengthsB;
	delete[] params;

	// return final point count
	return finalPointCount;
}

////////
///// Interpolates skeleton equidistantly to specified number of control points.
///// @param skeleton input skeleton (input)
///// @param finalPointCount number of control points, therefore number of final bone segments + 1 (input)
///// @param finalPoints array of final control points of length finalPointCount, must be preallocated (output)
////////
//void MeshSkeleton::InterpolateSkeletonToEquiDistPoints(vtkPolyData *skeleton, int finalPointCount, double **finalPoints)
//{
//	vtkCellArray *cells = skeleton->GetLines();
//	cells->InitTraversal();
//
//	vtkIdType *origPoints;
//	vtkIdType origPointCount;
//	cells->GetNextCell(origPointCount, origPoints);
//
//	double *origLengths = new double[origPointCount - 1];
//	double totalLength = this->MeasureSkeleton(skeleton->GetPoints(), origPoints, origPointCount, origLengths);
//	
//	double stepLength = totalLength / (double)(finalPointCount - 1);
//	double inPartLength = 0;
//	vtkIdType currentSkeletonPart = 0;
//
//	// start
//	double *pointA = new double[3];
//	double *pointB = new double[3];	
//	double dir[3];
//	double temp[3];
//	double *swapPointer;
//
//	skeleton->GetPoint(origPoints[0], pointA);
//	skeleton->GetPoint(origPoints[1], pointB);
//	PKUtils::CopyVertex(pointB, dir);
//	PKUtils::SubtractVertex(dir, pointA, dir);
//	PKUtils::MultiplyVertex(dir, stepLength / origLengths[0]);
//
//	// first point
//	double lastPoint[3];
//	skeleton->GetPoint(0, lastPoint);
//	PKUtils::CopyVertex(lastPoint, finalPoints[0]);
//
//	// follow skeletons step by step
//	for (int i = 1; i < finalPointCount; i++)
//	{
//		inPartLength += stepLength;
//
//		// jump to another part of original skeleton
//		while (inPartLength > origLengths[currentSkeletonPart] && currentSkeletonPart != origPointCount - 2)
//		{
//			inPartLength -= origLengths[currentSkeletonPart];
//			currentSkeletonPart++;
//
//			swapPointer = pointA;
//			pointA = pointB;
//			pointB = swapPointer;
//			skeleton->GetPoint(origPoints[currentSkeletonPart + 1], pointB);
//
//			PKUtils::CopyVertex(pointB, dir);
//			PKUtils::SubtractVertex(dir, pointA, dir);
//			PKUtils::MultiplyVertex(dir, stepLength / origLengths[currentSkeletonPart]);
//
//			skeleton->GetPoint(origPoints[currentSkeletonPart], lastPoint);
//
//			double skipped = stepLength - inPartLength;
//			PKUtils::CopyVertex(dir, temp);
//			PKUtils::MultiplyVertex(temp, skipped / PKUtils::CalculateVertexLength(temp));
//			PKUtils::SubtractVertex(lastPoint, temp, lastPoint);
//		}
//
//		PKUtils::AddVertex(lastPoint, dir, lastPoint);
//		PKUtils::CopyVertex(lastPoint, finalPoints[i]);
//	}
//
//	// hard last point
//	skeleton->GetPoint(origPoints[origPointCount - 1], lastPoint);
//	PKUtils::CopyVertex(lastPoint, finalPoints[finalPointCount - 1]);
//	
//	delete origLengths;
//	delete pointA;
//	delete pointB;
//}

//////
/// Measure lengths of uninterpolated skeleton parts.
/// @param points original point coordinates (input)
/// @param origPointIds array of input point ids (input)
/// @param origPointCount length of origPointIds (input)
/// @param lengths array of lengths of all parts (output)
/// @return total length of skeleton
//////
double MeshSkeleton::MeasureSkeleton(vtkPoints *points, vtkIdType *origPointIds, vtkIdType origPointCount, double *lengths)
{
	double pointA[3], pointB[3];
	double totalLength = 0;

	for (vtkIdType i = 0; i < origPointCount - 1; i++)
	{
		points->GetPoint(origPointIds[i], pointA);
		points->GetPoint(origPointIds[i + 1], pointB);

		PKUtils::SubtractVertex(pointA, pointB, pointA);

		lengths[i] = PKUtils::CalculateVertexLength(pointA);
		totalLength += lengths[i];
	}

	return totalLength;
}

//////
/// Measures distance of vertex from all bone segments.
/// @param point 3D coordinates of measured vertex (input)
/// @param distances preallocated array of length GetNumberOfBones() for distances for all bones (output)
/// @param minDist pointer to minimum measured distance, may be NULL (output)
/// @param maxDist pointer to maximum measured distance, may be NULL (output)
///	@param sumDist pointer to sum of all measured distances, may be NULL (output)
//////
void MeshSkeleton::MeasurePointDistances(double point[], double *distances, double *minDist, double *maxDist, double *sumDist, bool deformed)
{
	double min = 1E298;
	double max = -1E298;
	double sum = 0;
	double last;

	double *end = distances + (this->mainSkeletonEnd - 1);

	for (int i = 0; distances < end; distances++, i++)
	{
		last = MeasurePointDistanceToBone(i, point, deformed);
		(*distances) = last;

		sum += last;
		if (last > max)
		{
			max = last;
		}
		if (last < min)
		{
			min = last;
		}
	}

	if (minDist != NULL) (*minDist) = min;
	if (maxDist != NULL) (*maxDist) = max;
	if (sumDist != NULL) (*sumDist) = sum;
}

//////
/// Measure distance from vertex to bone.
/// @param boneId ID of bone (input)
/// @param point 3D coordinates of vertex (input)
/// @return minimum distance between point and bone
//////
double MeshSkeleton::MeasurePointDistanceToBone(int boneId, double point[], bool deformed)
{
	double dir[3];
	double *a = deformed ? this->deformedPoints->values[boneId] : this->originalPoints->values[boneId];
	double *b = deformed ? this->deformedPoints->values[boneId + 1] : this->originalPoints->values[boneId + 1];
	
	// direction of bone as dir
	PKUtils::CopyVertex(b, dir);
	PKUtils::SubtractVertex(dir, a, dir);

	// d parameter for plane with dir as normal containing point
	double d = -vtkMath::Dot(point, dir);

	// t parameter of parametric line AB for intersection with plane
	double t = -(vtkMath::Dot(dir, a) + d) / (vtkMath::Dot(dir, dir));

	if (t <= 0)
	{
		// point lies before point A of bone => get distance from A
		PKUtils::CopyVertex(a, dir);
		PKUtils::SubtractVertex(dir, point, dir);
		return PKUtils::CalculateVertexLength(dir);
	}
	else if (t >= 1)
	{
		// point lies beyond point B of bone => get distance from B
		PKUtils::CopyVertex(b, dir);
		PKUtils::SubtractVertex(dir, point, dir);
		return PKUtils::CalculateVertexLength(dir);
	}
	else 
	{
		// point lies in space between point A and B of bone => get perpendicular distance from bone line
		PKUtils::MultiplyVertex(dir, t);
		PKUtils::AddVertex(dir, a, dir);
		PKUtils::SubtractVertex(dir, point, dir);
		return PKUtils::CalculateVertexLength(dir);
	}
}

bool MeshSkeleton::GetRsoRotation(double axis[3], double &angle) {
	PKUtils::CopyVertex(this->rsoAxis, axis);
	angle = this->rsoAngle;
	return this->rsoValid;
}