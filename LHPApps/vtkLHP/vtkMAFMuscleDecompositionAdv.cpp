/*=========================================================================
Program: Multimod Application Framework RELOADED
Module: $RCSfile: vtkMAFMuscleDecompositionAdv.cpp,v $
Language: C++
Date: $Date: 2012-03-19 12:45:22 $
Version: $Revision: 1.1.2.2 $
Authors: Josef Kohout
==========================================================================
Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details
=========================================================================
*/

#include "vtkMAFMuscleDecomposition.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"
#include "vtkMath.h"
#include "vtkCellLocator.h"
#include "vtkGenericCell.h"
#include "vtkMAFSmartPointer.h"
#include "vtkMAFPolyDataCutOutFilterEx.h"
#include "vtkDoubleArray.h"
#include "vtkMAFVisualDebugger.h"

#include <math.h>
#include <float.h>

#include "mafMemDbg.h"
#include "mafDbg.h"

//------------------------------------------------------------------------
//Computes centroid of points
void vtkMAFMuscleDecomposition::ComputeCentroid(const vtkPoints* points, double* centroid)
//------------------------------------------------------------------------
{
	int N = const_cast<vtkPoints*>(points)->GetNumberOfPoints();
  centroid[0] = centroid[1] = centroid[2] = 0.0;
  for (int i = 0; i < N; i++)
  {
	const double* pcoords = const_cast<vtkPoints*>(points)->GetPoint(i);
	for (int j = 0; j < 3; j++){
	  centroid[j] += pcoords[j];
	}
  }

  for (int j = 0; j < 3; j++){
	centroid[j] /= N;
  }
}

//------------------------------------------------------------------------
// Computes three eigen vectors of the given point set.
//Centroid may be NULL, if it should be calculated automatically, otherwise, it must be centroid of points.
//The returned eigen vectors are ordered by their lengths. The longest (i.e., the principal axis) is denoted by the first one.
void vtkMAFMuscleDecomposition::ComputeEigenVects(const vtkPoints* points, const double* centroid, double eigenvects[3][3])
//------------------------------------------------------------------------
{
	double origin[3];
	if (centroid == NULL) {
		ComputeCentroid(points, origin);
		centroid = origin;
	}

	//compute the covariance matrix, which is a symmetric matrix
	double A[3][3], eigenvals[3], eigenvectsT[3][3];

	//fill the matrix
	memset(A, 0, sizeof(A));
	int N = const_cast<vtkPoints*>(points)->GetNumberOfPoints();
	for (int k = 0; k < N; k++)
	{
		const double* pcoords = const_cast<vtkPoints*>(points)->GetPoint(k);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++) {
				A[i][j] += (pcoords[i] - centroid[i])*(pcoords[j] - centroid[j]);
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++){
			A[i][j] /= (N - 1);
		}
	}

	//compute eigen vectors, the principal axis is the first one
	double *ATemp[3], *V[3];
	for (int i = 0; i < 3; i++)
	{
		ATemp[i] = A[i];
		V[i] = eigenvectsT[i];
	}

	vtkMath::Jacobi(ATemp, eigenvals, V);

	//copy the result
	//N.B. Jacobi returns vectors in columns!
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			eigenvects[i][j] = eigenvectsT[j][i];
		}
	}
}

//------------------------------------------------------------------------
//Computes the principal axis for the given muscle point set and origin and insertion areas.
//N.B. the direction is normalized
void vtkMAFMuscleDecomposition::ComputeAxisLine(vtkPoints* points, vtkPoints* ori_points, vtkPoints* ins_points,
	double* origin, double* direction)
	//------------------------------------------------------------------------
{
	//the line goes through the centroid
	ComputeCentroid(points, origin);

	double oc[3], ic[3];
	ComputeCentroid(ori_points, oc);
	ComputeCentroid(ins_points, ic);

	for (int i = 0; i < 3; i++) {
		direction[i] = ic[i] - oc[i];
	}

	vtkMath::Normalize(direction);
}

#ifndef ADV_SLICING_ORTHOPLANES
//------------------------------------------------------------------------
//Computes the plane for the given set of points that can be used to cut unwanted space.
//The returned plane C is parallel with the best fitting plane R of the given points, whilst passing through
//the point P from the point set that lies in the same half space (defined by C) as the given centroid
//and its distance from the plane R is maximal. The returned normal directs in the opposite direction than
//the given centroid. N.B. the method assumes that centroid does not lie on the plane R.
void vtkMAFMuscleDecomposition::ComputeCuttingPlane(const vtkPoints* pAreaPoints, const double* pMuscleCentroid,
	double* planeOrigin, double* planeNormal)
//------------------------------------------------------------------------
{
	//1. Calculate plane R
	//compute the origin of plane R as centroid of pAreaPoints
	double originR[3];
	ComputeCentroid(pAreaPoints, originR);

	//the normal is the smallest of eigen vectors
	double eigenvects[3][3];
	ComputeEigenVects(pAreaPoints, originR, eigenvects);

	double dblNorm = vtkMath::Norm(eigenvects[2]);	//the length of normal

	//change the direction of normal such that it points outward of pMuscleCentroid
	double tmp[3];
	for (int i = 0; i < 3; i++) {
		tmp[i] = pMuscleCentroid[i] - originR[i];
	}

	double t = vtkMath::Dot(tmp, eigenvects[2]);
	if (t > 0) {
	 dblNorm = -dblNorm; //must change direction
	}

	for (int i = 0; i < 3; i++) {
		planeNormal[i] = eigenvects[2][i] / dblNorm;
	}

	//2. find the plane C
	//plane R is defined now as X*planeNormal + d = 0, compute d
	double d = -vtkMath::Dot(originR, planeNormal);

	double dblMinDist = DBL_MAX;	//invalid distance
	int N = const_cast<vtkPoints*>(pAreaPoints)->GetNumberOfPoints();
	for (int i = 0; i < N; i++)
	{
		double x[3];
		const_cast<vtkPoints*>(pAreaPoints)->GetPoint(i, x);

		//compute the signed distance from the plane R
		//see Geometric Tools for Computer Graphics, pg. 375
		double dblDist = vtkMath::Dot(x, planeNormal) + d;
		if (dblDist < dblMinDist)	//we need to most negative value (it is in the opposite direction of plane
		{
			for (int j = 0; j < 3; j++) {
				planeOrigin[j] = x[j];
			}

			dblMinDist = dblDist;
		}
	}
}

//------------------------------------------------------------------------
//Filter the fiber defined by the point sequence pPoints.
//N.B. pPoints is damaged!!!
//It filters out a) redundant points (two points at the same location)
//and b) if cutting planes are specified (not NULL), then also points that
//lies in positive half spaces defined by both cutting planes.
//It returns the start position of fibers in nValidIndex and the number
//of points in the fiber in nValidPoints. If the result would be a single point,
//this is also filtered out, i.e., possible results are nValidPoints == 0 or >=2.
void vtkMAFMuscleDecomposition::FilterFiber(VCoord* pPoints, int& nValidIndex, int& nValidPoints,
		const double* OCutPlaneOrigin, const double* OCutPlaneNormal,
		const double* ICutPlaneOrigin, const double* ICutPlaneNormal)
	//------------------------------------------------------------------------
{
	//first, remove duplicated points
	int nDstIndex = nValidIndex, nSrcIndex = nValidIndex + 1;
	int nEndIndex = nValidIndex + nValidPoints;	//out index
	while (nSrcIndex < nEndIndex)
	{
		if (pPoints[nSrcIndex][0] != pPoints[nDstIndex][0] ||
			pPoints[nSrcIndex][1] != pPoints[nDstIndex][1] ||
			pPoints[nSrcIndex][2] != pPoints[nDstIndex][2])
		{
			//we have here a new point
			if ((++nDstIndex) != nSrcIndex)
			{
				pPoints[nDstIndex][0] = pPoints[nSrcIndex][0];
				pPoints[nDstIndex][1] = pPoints[nSrcIndex][1];
				pPoints[nDstIndex][2] = pPoints[nSrcIndex][2];
			}
		}

		nSrcIndex++;
	}

	nEndIndex = nDstIndex + 1;
	if (OCutPlaneOrigin != NULL && OCutPlaneNormal != NULL)
	{
		//trim the beginning of the fiber
		double d =  -vtkMath::Dot(OCutPlaneOrigin, OCutPlaneNormal);
		while (nValidIndex < nEndIndex)
		{
			//compute signed distance from plane
			double t = vtkMath::Dot(pPoints[nValidIndex], OCutPlaneNormal) + d;
			if (t <= 0.0)
				break;

			nValidIndex++;
		}
	}

	if (ICutPlaneOrigin == NULL && ICutPlaneNormal == NULL)
		--nEndIndex;
	else
	{
		//trim the end of the fiber
		double d =  -vtkMath::Dot(ICutPlaneOrigin, ICutPlaneNormal);
		while (nValidIndex < (--nEndIndex))
		{
			double t = vtkMath::Dot(pPoints[nEndIndex], ICutPlaneNormal) + d;
			if (t <= 0.0)
				break;
		}
	}

	//nIndex is now the last valid
	if (nValidIndex >= nEndIndex)
		nValidPoints = 0;
	else
		nValidPoints = nEndIndex - nValidIndex + 1;
}
#else
//------------------------------------------------------------------------
//Filter the fiber defined by the point sequence pPoints.
//N.B. pPoints is damaged!!!
//It filters out a) redundant points (two points at the same location)
//and b) points whose projection onto the principal axis (lf, iPlane) is
//not in the interval tmin-tmax (see also GetProjectedT).
//It returns the start position of fibers in nValidIndex and the number
//of points in the fiber in nValidPoints. If the result would be a single point,
//this is also filtered out, i.e., possible results are nValidPoints == 0 or >=2.
void vtkMAFMuscleDecomposition::FilterFiber(VCoord* pPoints, int& nValidIndex, int& nValidPoints,
	const LOCAL_FRAME& lf, int iPlane, double tmin, double tmax)
	//------------------------------------------------------------------------
{
	//first, remove duplicated points
	int nDstIndex = nValidIndex, nSrcIndex = nValidIndex + 1;
	int nEndIndex = nValidIndex + nValidPoints;	//out index
	while (nSrcIndex < nEndIndex)
	{
		if (pPoints[nSrcIndex][0] != pPoints[nDstIndex][0] ||
			pPoints[nSrcIndex][1] != pPoints[nDstIndex][1] ||
			pPoints[nSrcIndex][2] != pPoints[nDstIndex][2])
		{
			//we have here a new point
			if ((++nDstIndex) != nSrcIndex)
			{
				pPoints[nDstIndex][0] = pPoints[nSrcIndex][0];
				pPoints[nDstIndex][1] = pPoints[nSrcIndex][1];
				pPoints[nDstIndex][2] = pPoints[nSrcIndex][2];
			}
		}

		nSrcIndex++;
	}

	//trim the beginning of the fiber
	nEndIndex = nDstIndex + 1;
	if (this->AdvancedSlicing == 0)
		--nEndIndex;
	else
	{
		while (nValidIndex < nEndIndex)
		{
			double t = GetProjectedT(pPoints[nValidIndex], lf, iPlane);
			if (t >= tmin)
				break;

			nValidIndex++;
		}

		//trim the end of the fiber
		while (nValidIndex < (--nEndIndex))
		{
			double t = GetProjectedT(pPoints[nEndIndex], lf, iPlane);
			if (t <= tmax)
				break;
		}
	}

	//nIndex is now the last valid
	if (nValidIndex >= nEndIndex)
		nValidPoints = 0;
	else
		nValidPoints = nEndIndex - nValidIndex + 1;
}

//------------------------------------------------------------------------
//Projects all points onto the iPlane axis of the local frame lf and returns the extreme distances from the origin of lf.
//	Extremes are returned in a fraction of  iPlane vector length, i.e., 0.0 = origin, 1.0 = the end point of O + vector[iPlane]
void vtkMAFMuscleDecomposition::GetProjectedMinMaxT(const vtkPoints* points,
	const LOCAL_FRAME& lf, int iPlane, double& tmin, double& tmax)
	//------------------------------------------------------------------------
{
	int N = const_cast< vtkPoints* >(points)->GetNumberOfPoints();
	if (N == 0) {
		tmin = DBL_MAX;
		tmax  = DBL_MIN;
	}
	else
	{
		tmin = tmax = GetProjectedT(const_cast< vtkPoints* >(points)->GetPoint(0), lf, iPlane);
		for (int i = 1; i < N; i++)
		{
			double t = GetProjectedT(const_cast< vtkPoints* >(points)->GetPoint(i), lf, iPlane);
			if (t < tmin)
				tmin = t;
			else if (t > tmax)
				tmax = t;
		}
	}
}
#endif

//------------------------------------------------------------------------
//Transforms the given points corresponding to vertices of the source mesh so that they correspond to vertices of the target mesh.
//N.B. the target mesh must be compatible with the source one, i.e., everything must be the same except for vertex coordinates.
void vtkMAFMuscleDecomposition::TransformPointsFromInputTemplateToInput(vtkPoints* points, vtkPolyData* source, vtkPolyData* target)
	//------------------------------------------------------------------------
{
	int nPoints = points->GetNumberOfPoints();
	for (int i = 0; i < nPoints; i++)
	{
		int ptIndex = source->FindPoint(points->GetPoint(i));
		_VERIFY_CMD(ptIndex >= 0, continue);

		points->SetPoint(i, target->GetPoint(ptIndex));
	}
}

//------------------------------------------------------------------------
//Extrapolates the fiber defined by the point sequence in the given direction.
//If nDir is positive, the method computes fiber successor, otherwise its predecessor and
//stores the coordinates of the computed point into pOut. The method performs a simple
//linear extrapolation.It is guaranteed that the point lies on the surface of input muscle.
//N.B.  cellLocator is a locator built upon input muscle surface and may not be NULL!!
//The method returns false, if the calculation fails.
bool vtkMAFMuscleDecomposition::ExtrapolateFiber(const VCoord* pPoints, int nPoints, int nDir,
	const vtkCellLocator* cellLocator, VCoord* pOut)
	//------------------------------------------------------------------------
{
	_ASSERTE(cellLocator != NULL);

	if (nPoints < 2)
		return false;	//cannot continue

	double ptStart[3], ptEnd[3];
	if (nDir > 0)
	{
		//successor
		for (int i = 0; i < 3; i++) {
			ptStart[i] = pPoints[nPoints - 1][i];
			ptEnd[i] = pPoints[nPoints - 1][i] - pPoints[nPoints - 2][i];
		}
	}
	else
	{
		//predecessor
		for (int i = 0; i < 3; i++) {
			ptStart[i] = pPoints[0][i];
			ptEnd[i] = pPoints[0][i] - pPoints[1][i];
		}
	}

	//make sure that ptEnd intersects with the mesh
	double dblCoef = (const_cast<vtkCellLocator*>(cellLocator)->GetDataSet())->GetLength() /
		vtkMath::Norm(ptEnd);	//distance
	for (int i = 0; i < 3; i++) {
		ptEnd[i] = ptStart[i] + ptEnd[i]*dblCoef;
	}

	//this is just to speed-up the search for intersection
	vtkMAFSmartPointer< vtkGenericCell > cellLoc;

	int nCellId, nSubId;
	double par_x[3];
	double t = 0.0;

	//checks the intersection of that line
	//IntersectWithLine is not well documented, I will try it
	if (0 == const_cast<vtkCellLocator*>(cellLocator)->IntersectWithLine(
		ptStart, ptEnd, 0.0,  //IN: end-points of line segment and tolerance
		t, *pOut, par_x,          //OUT: time of intersection and position
		//of intersection in global coordinates and parametric coordinates
		nSubId, nCellId,      //id of intersected cell and its face
		(vtkGenericCell*)cellLoc)
		)
	{
		//no intersection  => find the closest point, it will be slow!
		vtkPoints* points = vtkPolyData::SafeDownCast
			(const_cast<vtkCellLocator*>(cellLocator)->GetDataSet())->GetPoints();

		double u[3];
		for (int i = 0; i < 3; i++) {
			u[i] = ptEnd[i] - ptStart[i];
		}
		vtkMath::Normalize(u);	//line direction vector

		int N = points->GetNumberOfPoints();
		double dblMinDist = DBL_MAX;
		int iMinDist = -1;

		for (int i = 0; i < N; i++)
		{
			double x[3], xp[3], v[3];
			points->GetPoint(i, x);

			//project x onto the line
			for (int k = 0; k < 3; k++) {
				v[k] = x[k] - ptStart[k];
			}

			double t = vtkMath::Dot(u, v);
			if (t < 0.0)
				continue;		//it is before the start point

			for (int k = 0; k < 3; k++) {
				xp[k] = ptStart[k] + t*u[k];
			}

			//get the distance between the projected point xp and x
			double dblDist = vtkMath::Distance2BetweenPoints(x, xp);
			if (dblDist < dblMinDist)
			{
				dblMinDist = dblDist;
				iMinDist = i;
			}
		} //end for i

		//we have found it
		if (iMinDist < 0)
			return false;

		points->GetPoint(iMinDist, *pOut);
	}

	return true;
}

//------------------------------------------------------------------------
//Extrapolates the fiber defined by the point sequence in the given direction.
//If nDir is positive, the method computes fiber successor, otherwise its predecessor and
//stores the coordinates of the computed point into pOut. It is guaranteed that the point
//lies in the area represented by cellLocatorORI (nDIr < 0) or by cellLocatorINS (nDir > 0).
//N.B.  cellLocator is a locator built upon input muscle surface and may not be NULL!!
//The method returns false, if the calculation fails.
bool vtkMAFMuscleDecomposition::ExtrapolateFiber(const VCoord* pPoints, int nPoints, int nDir,
	const vtkCellLocator* cellLocator,
	const vtkCellLocator* cellLocatorORI, const vtkCellLocator* cellLocatorINS, VCoord* pOut)
	//------------------------------------------------------------------------
{
	//this is just to speed-up the search for intersection
	vtkMAFSmartPointer< vtkGenericCell > cellLoc;
	int nCellId, nSubId;
	double dblDist;

	//first, calculate linearly extrapolated end-point
	VCoord ptExtra, ptInternal;
	bool bSimpleExOK = ExtrapolateFiber(pPoints, nPoints, nDir, cellLocator, &ptExtra);

	if (!bSimpleExOK && nPoints < 1)
			return false;

	//get the closest point on the attachment area
	if (nDir > 0)
	{
		const_cast<vtkCellLocator*>(cellLocatorINS)->FindClosestPoint(
			const_cast<double*>(pPoints[nPoints - 1]), bSimpleExOK ? ptInternal : *pOut,
			cellLoc, nCellId, nSubId, dblDist);
	}
	else
	{
		const_cast<vtkCellLocator*>(cellLocatorORI)->FindClosestPoint(
			const_cast<double*>(pPoints[0]), bSimpleExOK ? ptInternal : *pOut,
			cellLoc, nCellId, nSubId, dblDist);
	}

	if (bSimpleExOK)
	{
		//compute average point of both points
		for (int i = 0; i < 3; i++) {
			ptInternal[i] = ptExtra[i]; //0.5*(ptExtra[i] + ptInternal[i]);
		}

		if (nDir > 0)
		{
			const_cast<vtkCellLocator*>(cellLocatorINS)->FindClosestPoint(
				ptInternal, *pOut,
				cellLoc, nCellId, nSubId, dblDist);
		}
		else
		{
			const_cast<vtkCellLocator*>(cellLocatorORI)->FindClosestPoint(
				ptInternal, *pOut,
				cellLoc, nCellId, nSubId, dblDist);
		}
	}

	return true;
}

//------------------------------------------------------------------------
//Project the points onto the surface and stores the projected points into out_points.
void vtkMAFMuscleDecomposition::ProjectAttachmentArea(const vtkPoints* points,
	const vtkCellLocator* cellLocator, vtkPoints* out_points)
//------------------------------------------------------------------------
{
	PROFILE_THIS_FUNCTION();

	//this is just to speed-up the search for intersection
	vtkMAFSmartPointer< vtkGenericCell > cellLoc;

	int N = const_cast<vtkPoints*>(points)->GetNumberOfPoints();
	out_points->SetNumberOfPoints(N);

	for (int i = 0; i < N; i++)
	{
		double x[3], xout[3], dblDist;
		const_cast<vtkPoints*>(points)->GetPoint(i, x);

		int nCellId, nSubId;
		const_cast<vtkCellLocator*>(cellLocator)->FindClosestPoint(x, xout,
			cellLoc, nCellId, nSubId, dblDist);

		out_points->SetPoint(i, xout);
	}
}

//#include "vtkSTLWriter.h"

//------------------------------------------------------------------------
//Creates the surface polydata that represents the attachment area on the muscle.
//Attachment area points (projpts) must lie on the muscle surface (surface), i.e., use ProjectAttachmentArea method prior to this one.
//If ADV_KUKACKA conditional is specified, the method may optionally return also the rest of muscle surface after the attachment area is cut-off
void vtkMAFMuscleDecomposition::GetAttachmentAreaSurface(const vtkPoints* projpts,
	const vtkPolyData* surface, vtkPolyData* output
#ifdef ADV_KUKACKA
		, vtkPolyData* cutSurface, vtkPolyData* cutPolyLine
#endif
		)
//------------------------------------------------------------------------
{
	PROFILE_THIS_FUNCTION();

	//create cells for projpts
	int N = const_cast<vtkPoints*>(projpts)->GetNumberOfPoints();
	vtkIdType ptIds[2] = {0, 1};

	vtkCellArray* cells = vtkCellArray::New();
	for (int i = 1; i < N; i++)
	{
		cells->InsertNextCell(2, ptIds);
		ptIds[0]++; ptIds[1]++;
	}

	ptIds[0] = N - 1;
	ptIds[1] = 0;
	cells->InsertNextCell(2, ptIds);

	//now we have here a contour
	vtkMAFSmartPointer< vtkPolyData > contour;
	contour->SetPoints(const_cast<vtkPoints*>(projpts));
	contour->SetLines(cells);

	cells->Delete();

	//create cutter
	vtkMAFSmartPointer< vtkMAFPolyDataCutOutFilterEx > cutter;
	cutter->SetInput(const_cast<vtkPolyData*>(surface));
	cutter->SetCuttingPolyline(contour);

	vtkMAFSmartPointer< vtkPolyData > clippedPart;
	cutter->SetOutputClippedPart(clippedPart);

#ifdef ADV_KUKACKA
	if (this->AdvancedKukacka != 0)
		cutter->SetOutputCuttingPolyline(cutPolyLine);
#endif
	
	cutter->SetDebug((this->DebugMode & dbgVisualizeAttachmentConstruction) == dbgVisualizeAttachmentConstruction);
	cutter->Update();

	vtkPolyData* outputL = cutter->GetOutput();
	vtkPolyData* outputR = cutter->GetOutputClippedPart();

	//the smaller of both parts is the desired attachment area (surface)
	int numL = outputL->GetNumberOfPoints();
	int numR = outputR->GetNumberOfPoints();
	//BES 15.3.2012 - cutter may fail, if the input is corrupted, producing an empty set, which later causes problems
	//hence, here is a fix: we accept smaller part unless it is empty
	if (numL == 0)
		output->ShallowCopy(outputR);
	else if (numR == 0)
		output->ShallowCopy(outputL);
	else
		output->ShallowCopy((numL < numR ? outputL : outputR));

#ifdef ADV_KUKACKA
	if (this->AdvancedKukacka != 0 && cutSurface != NULL)
		cutSurface->ShallowCopy((numL < numR ? outputR : outputL));
#endif

	//vtkMAFSmartPointer< vtkSTLWriter > writer;
	//writer->SetFileName("surface.stl");
	//writer->SetInput(const_cast<vtkPolyData*>(surface));
	//writer->Update();

	//writer->SetFileName("attachment1.stl");
	//writer->SetInput(output);
	//writer->Update();
	//writer->SetFileName("contour.vtk");
	//writer->SetInput(contour);
	//writer->Update();
}

#ifdef ADV_KUKACKA
//#define _OPENCL_SOLVER //--- too slow and incorrect

extern "C" 
{
	#include "csparse.h"
}
//#include <omp.h>
#ifdef _OPENCL_SOLVER
//#define VIENNACL_WITH_OPENMP
#define VIENNACL_WITH_OPENCL
#include "viennacl/compressed_matrix.hpp"
#include "viennacl/linalg/bicgstab.hpp"
#endif

//------------------------------------------------------------------------
//Calculates harmonic scalar function for the given surface.
//The surface may not contain attachment areas, i.e., it is an open mesh corresponding to the body of muscle.
//oriContour and insContour contains points (should geometrically correspond to some vertices of the surface mesh)
//that are on origin and insertion attachment area. oriVal and insVal are values of harmonic function in these points.
//Values for other points will be calculated and the final scalar filed stored into surface.
//IMPORTANT: neither oriVal nor insVal may be 0!!!
//
//See Also: Dong S, Kircher S, Garland M. Harmonic Functions for Quadrilateral Remeshing of Arbitraty Manifolds.
//Elsevier Science, 2004.
void vtkMAFMuscleDecomposition::ComputeHarmonicScalarFunction(vtkPolyData* surface,
	vtkPolyData* oriContour, double oriVal, vtkPolyData* insContour, double insVal)
	//------------------------------------------------------------------------
{
	PROFILE_THIS_FUNCTION();
//	PROFILE_TIMER_START(ComputeHarmonicScalarFunction::MatrixConstruction);

//	 omp_set_num_threads(8);	//8 threads --- OMP is too slow, this method is already too fast


	//scalar field u is calculated so that Laplacian u = 0, i.e., we need to solve linear equations
	//A*u = b, where bi = oriVal for points from oriContour, insVal for points from insContour, and 0 otherwise
	//A is NxN matrix such that it has 1 on the main diagonal and rows for points neither from oriContour nor insContour
	//contain cotan Laplacian description.	
	int nPoints = surface->GetNumberOfPoints();
	bool* processed = new bool[nPoints];
	memset(processed, 0, nPoints*sizeof(bool));	//set all points to zero

	//initialize scalars for both contours
	double* pValues = new double[nPoints];
	memset(pValues, 0, nPoints*sizeof(double));	//set all points to zero

	vtkPolyData* contours[2] = { oriContour, insContour };
	double contourHarmVal[2] = { oriVal, insVal };

	for (int j = 0; j < 2; j++)
	{
		int nCntPoints = contours[j]->GetNumberOfPoints();
		for (int i = 0; i < nCntPoints; i++)
		{
			int index = surface->FindPoint(contours[j]->GetPoint(i));	//find point in the surface
			_VERIFY_CMD(index >= 0, continue);

			pValues[index] = contourHarmVal[j];
			processed[index] = true;
		}
	}
		
	//and now let us create the matrix, which means that we will have to calculate Laplacian	
	typedef std::map< unsigned int, double> RowMap;
	std::vector< RowMap > cpu_sparse_matrix(nPoints);

	surface->BuildCells(); //surface->BuildLinks();

	int nCells = surface->GetNumberOfCells();		
//#pragma omp parallel for shared(cpu_sparse_matrix, nCells, surface, processed, nPoints)
	for (int i = 0; i < nCells; i++)
	{
		vtkIdType nPts, *pPts;
		surface->GetCellPoints(i, nPts, pPts);				
		
		//get coordinates we will need for our calculation
		typedef double VCoord[3];
		VCoord Coords[3];

		for (int j = 0; j < 3; j++) {
			surface->GetPoint(pPts[j], Coords[j]);
		}

		//compute cotans and add them to cpu_sparse_matrix
		for (int j = 0; j < 3; j++)
		{
			//cotg(alfa) = cos(alfa)/sin(alfa)
			//cos(alfa) = u*v /(|u|*|v|), where u and v are vectors of the triangle
			//sin(alfa) = |u x v| / (|u|*|v|)
			//thus cotg(alfa) = u*v / |u x v|
			
			vtkIdType iv1 = (j + 1) % 3;
			vtkIdType iv2 = (j + 2) % 3;

			double u[3], v[3];
			for (int k = 0; k < 3; k++)
			{
				u[k] = Coords[iv1][k] - Coords[j][k];
				v[k] = Coords[iv2][k] - Coords[j][k];
			}

			double uxv[3];
			vtkMath::Cross(u, v, uxv);
			double cotan_2 = 0.5*vtkMath::Dot(u, v) / vtkMath::Norm(uxv);

			//the calculated cotan is used in iv1-iv2 edge
			if (processed[pPts[iv1]]) {
//#pragma omp critical 
				{
				cpu_sparse_matrix[pPts[iv1]][pPts[iv1]] = 1.0;
				}
			}
			else {
//#pragma omp critical 
				{
				cpu_sparse_matrix[pPts[iv1]][pPts[iv2]] += cotan_2;
				}
			}

			if (processed[pPts[iv2]]) {
//#pragma omp critical 
				{
				cpu_sparse_matrix[pPts[iv2]][pPts[iv2]] = 1.0;
				}
			}
			else {
//#pragma omp critical 
				{
				cpu_sparse_matrix[pPts[iv2]][pPts[iv1]] += cotan_2;
				}
			}
		}
	}

	//normalization and adding 1.0 on diagonals
//#pragma omp parallel for shared(cpu_sparse_matrix)
	for (int i = 0; i < nPoints; i++)
	{
		double dblSum = 0.0;		
		for (RowMap::const_iterator it = cpu_sparse_matrix[i].begin();
			it != cpu_sparse_matrix[i].end(); it++) {
			dblSum += (*it).second;
		}

		for (RowMap::iterator it = cpu_sparse_matrix[i].begin();
			it != cpu_sparse_matrix[i].end(); it++) {			
				(*it).second /= -dblSum;	//weight			
		}

		cpu_sparse_matrix[i][i] = 1.0;	//add diagonal
	}

#ifdef _OPENCL_SOLVER
	

	//set up a sparse ViennaCL matrix on device:
	typedef viennacl::compressed_matrix<double> SparseMatrix;
	SparseMatrix vcl_sparse_matrix(nPoints, nPoints);	
	copy(cpu_sparse_matrix, vcl_sparse_matrix);

	//set up vector of right side (on device)
	viennacl::vector<double> vcl_rhs(nPoints);	
	copy(pValues, pValues + nPoints, vcl_rhs.begin());		

	//PROFILE_TIMER_STOP(ComputeHarmonicScalarFunction::MatrixConstruction);

	//PROFILE_TIMER_START(ComputeHarmonicScalarFunction::Solving);
	
	//solution using conjugate gradient solver:		--- does not work since CG cannot be used on non-symmetric matrices
	//because it actually solves [(At + A) / 2] *x = b and the term  [(At + A) / 2]  equals to A only for symmetric matrices
	//GMRES works fine but it is too slow, slower than CPU version
	//viennacl::linalg::cg_tag custom_tag; //(1e-8, 2000U);
	viennacl::linalg::bicgstab_tag custom_tag;
	viennacl::vector<double> vcl_result = viennacl::linalg::solve(vcl_sparse_matrix, vcl_rhs, custom_tag); 

	copy(vcl_result.begin(), vcl_result.end(), pValues);

	//PROFILE_TIMER_STOP(ComputeHarmonicScalarFunction::Solving);
#else
	//allocate a sparse matrix (in compressed column format) with reserved memory
	//in any triangulation, there is 2*Vb + 3*Vi - 3 edges, where Vb is number of vertices on the boundary
	//and Vi is the number of inner vertices, see http://www.math.utah.edu/~pa/MDS/triangulation.html
	//for a boundary vertex, we need just 1 entry in the matrix and for each inner vertex we need 
	//one entry for each of its edges + 1and since one edge belongs to two vertices, the number of entries
	//required should be Vb + 7*Vi = 7*(nPoints - Vb) + Vb  = 7*nPoints  - 6*Vb < 7*nPoints
	cs* A = cs_spalloc (nPoints, nPoints, nPoints*7, 1, 0);
	
	//since cs is column ordered, let us find the number of nnz in every column
	memset(A->p, 0, (nPoints + 1)*sizeof(A->p[0]));

//#pragma omp parallel for shared(cpu_sparse_matrix, A)
	for (int i = 0; i < nPoints; i++)
	{		
		for (RowMap::const_iterator it = cpu_sparse_matrix[i].begin();
			it != cpu_sparse_matrix[i].end(); it++) {
			 A->p[(*it).first + 1]++;
		}
	}

	//calculate column pointers (0, n1, n1+n2, n1+n2+n3, ...)
	for (int j = 1; j <= nPoints; j++) {
		A->p[j] += A->p[j - 1];
	}

	//store values
	for (int i = 0; i < nPoints; i++)
	{		
		for (RowMap::const_iterator it = cpu_sparse_matrix[i].begin();
			it != cpu_sparse_matrix[i].end(); it++) 
		{
			int index = A->p[(*it).first];	//column index ptr
			A->i[index] = i;								//row index
			A->x[index] = (*it).second;			//value
			A->p[(*it).first]++;						//increase column index
		}
	}

	//column pointers must be shifted down (n1, n1+n2, n1+n2+n3, ...) -> (0, n1, n1+n2, ...)
	for (int j = nPoints - 1; j > 0; j--) {
		A->p[j] = A->p[j - 1];
	}

	A->p[0] = 0;

			
	//PROFILE_TIMER_STOP(ComputeHarmonicScalarFunction::MatrixConstruction);
	
	//FILE* fA = fopen("d:\\matrix_A.txt", "wt");
	//FILE* fB = fopen("d:\\vector_B.txt", "wt");	
	//fprintf(fA, "%d\n", nPoints);
	//fprintf(fB, "%d\n", nPoints);


	//for (int i = 0; i < nPoints; i++) 
	//{
	//	fprintf(fA, "%d,", cpu_sparse_matrix[i].size());
	//	for (RowMap::const_iterator it = cpu_sparse_matrix[i].begin();
	//		it != cpu_sparse_matrix[i].end(); it++) 
	//	{
	//		fprintf(fA, "(%d, %f), ", (*it).first, (*it).second);
	//	}

	//	fprintf(fA, "\n");

	//	fprintf(fB, "%f\n", pValues[i]);

	//}

	//fclose(fA);
	//fclose(fB);

	//PROFILE_TIMER_START(ComputeHarmonicScalarFunction::Solving);
	int result = cs_lusol(A, pValues, 0, 1e-8);
	//PROFILE_TIMER_STOP(ComputeHarmonicScalarFunction::Solving);
	
	cs_spfree(A);	//free the matrix
#endif
	delete[] processed;

	vtkDoubleArray* scalars = vtkDoubleArray::New();
	scalars->SetName("HARM_SCALAR_FUNC");
	scalars->SetArray(pValues, nPoints, 0);
	surface->GetPointData()->SetScalars(scalars);	
	scalars->UnRegister(this);	//no longer needed



	//FILE* fX = fopen("d:\\solution_x.txt", "wt");
	//fprintf(fX, "%d\n", nPoints);
	//for (int i = 0; i < nPoints; i++) 
	//{		
	//	fprintf(fX, "%f\n", pValues[i]);

	//}
	//fclose(fX);

}
#endif

#pragma region Debug Stuff
//------------------------------------------------------------------------
//Adds a contour into VTK data sets.
void vtkMAFMuscleDecomposition::DebugAddContour(const VCoord* pPoints, int nPoints,
	vtkPoints* pOutPts, vtkCellArray* pOutCells)
	//------------------------------------------------------------------------
{
	int nShift = pOutPts->GetNumberOfPoints();
	for (int i = 0; i < nPoints; i++){
		pOutPts->InsertNextPoint(pPoints[i]);
	}

	vtkIdType ptIds[2] = {nShift, nShift + 1};
	for (int i = 1; i < nPoints; i++)
	{
		pOutCells->InsertNextCell(2, ptIds);
		ptIds[0]++; ptIds[1]++;
	}
}

//------------------------------------------------------------------------
//Adds links between two points into VTK data sets.
void vtkMAFMuscleDecomposition::DebugAddMorphDir(const VCoord* pPointsFrom, const VCoord* pPointsTo, int nPoints,
	vtkPoints* pOutPts, vtkCellArray* pOutCells)
//------------------------------------------------------------------------
{
	int nShift = pOutPts->GetNumberOfPoints();
	for (int i = 0; i < nPoints; i++){
		pOutPts->InsertNextPoint(pPointsFrom[i]);
		pOutPts->InsertNextPoint(pPointsTo[i]);
	}

	vtkIdType ptIds[2] = {nShift, nShift + 1};
	for (int i = 1; i < 2*nPoints; i++)
	{
		pOutCells->InsertNextCell(2, ptIds);
		ptIds[0]++; ptIds[1]++;
	}
}

#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
#include "vtkDelaunay3D.h"
#include "vtkGeometryFilter.h"
#include "vtkUnstructuredGrid.h"

//Computes the best fitting plane for the given points and adds it into VTK data sets. 
void vtkMAFMuscleDecomposition::DebugAddBestFittingPlane(const VCoord* pPoints, int nPoints, vtkPoints* pOutPts, vtkCellArray* pOutCells)
{
	//N.B. we want to find the plane such that the maximal distance to this plane is minimal, i.e., it is not least square problem
	//It seems that the problem is related to finding minimum bounding box http://en.wikipedia.org/wiki/Minimum_bounding_box_algorithms
	//Hence, what we will do is to compute the convex hull and for each triangle of the convex hull we will find two parallel planes passing through extreme points
	//The mid-plane for these planes having the minimum distance is the sought plane
	vtkPoints* points = vtkPoints::New();
	points->SetNumberOfPoints(nPoints);
	for (int i = 0; i < nPoints; i++) {
		points->SetPoint(i, pPoints[i]);
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
	tri->BuildCells();

	double dblBestError = DBL_MAX;
	double bestPlaneOrigin[3], bestPlaneNormal[3];
	
	int nCells = tri->GetNumberOfCells();
	for (int i = 0; i < nCells; i++)
	{
		vtkIdType nPts, *pPts;
		tri->GetCellPoints(i, nPts, pPts);

		VCoord ABC[3];
		for (int k = 0; k < 3; k++) {
			tri->GetPoint(pPts[k], ABC[k]);
		}

		double u[3], v[3];
		for (int k = 0; k < 3; k++) {
			u[k] = ABC[1][k] - ABC[0][k];
			v[k] = ABC[2][k] - ABC[0][k];
		}

		VCoord n[2];
		vtkMath::Cross(u, v, n[0]);
		vtkMath::Normalize(n[0]);
		for (int k = 0; k < 3; k++) {
			n[1][k] = -n[0][k];
		}
	
		FitDirectionVectorsToData(points, ABC[0], 2, n);
		double d1 = vtkMath::Norm(n[0]);
		double d2 = vtkMath::Norm(n[1]);
		double d = d1 + d2;
		if (d < dblBestError)
		{
			vtkMath::Cross(u, v, bestPlaneNormal);
			for (int k = 0; k < 3; k++) {
				bestPlaneOrigin[k] = ABC[0][k] + 0.5*(n[0][k] + n[1][k]);
			}	

			dblBestError = d;			
		}
	}	

			
	dblBestError = 0.0;
	FILE* f = fopen("coplanarity.txt", "at");
	
	vtkMath::Normalize(bestPlaneNormal);
	double d = -vtkMath::Dot(bestPlaneOrigin, bestPlaneNormal);
	double dblError = 0.0;

	for (int kkkk = 0; kkkk < nPoints; kkkk++)
	{			
		double dd = fabs(vtkMath::Dot(pPoints[kkkk], bestPlaneNormal) + d);
		//dblError += dd*dd;
		if (dd > dblError)
			dblError = dd;
	}

	fprintf(f, "%f\n", dblError);	
	fclose(f);

	VCoord eigenvect[4];
	//ComputeEigenVects(points, bestPlaneOrigin, eigenvect);
	eigenvect[0][0] = bestPlaneNormal[1];
	eigenvect[0][1] = -bestPlaneNormal[0];
	eigenvect[0][2] = 0.0;
	
	vtkMath::Cross(bestPlaneNormal, eigenvect[0], eigenvect[1]);
	
	vtkMath::Normalize(eigenvect[0]);
	vtkMath::Normalize(eigenvect[1]);

	for (int k = 0; k < 3; k++) {
		eigenvect[2][k] = -eigenvect[0][k];
		eigenvect[3][k] = -eigenvect[1][k];
	}
	FitDirectionVectorsToData(points, bestPlaneOrigin, 4, eigenvect);

	int orderX[4] = {0, 0, 2, 2};
	int orderY[4] = {1, 3, 3, 1};

	int nShift = pOutPts->GetNumberOfPoints();
	for (int i = 0; i < 4; i++)
	{
		pOutPts->InsertNextPoint(
			bestPlaneOrigin[0] + 1.125*(eigenvect[orderX[i]][0] + eigenvect[orderY[i]][0]),
			bestPlaneOrigin[1] + 1.125*(eigenvect[orderX[i]][1] + eigenvect[orderY[i]][1]),
			bestPlaneOrigin[2] + 1.125*(eigenvect[orderX[i]][2] + eigenvect[orderY[i]][2])
			);
	}
	
	vtkIdType ptIds[3] = {nShift, nShift + 1, nShift + 2};
	pOutCells->InsertNextCell(3, ptIds);
	ptIds[1]++; ptIds[2]++;
	pOutCells->InsertNextCell(3, ptIds);
	



	/*
	//compute the covariance matrix, which is a symmetric matrix
	double A[3][3], eigenvals[3], eigenvectsT[3][3];

	double origin[3] = {pTargetPoly[0][0], pTargetPoly[0][1], pTargetPoly[0][2] };	
	for (int kkkk = 0; kkkk < nPoints; kkkk++) 
	{
		origin[0] += pTargetPoly[kkkk][0];
		origin[1] += pTargetPoly[kkkk][1];
		origin[2] += pTargetPoly[kkkk][2];
	}

	origin[0] /= nPoints; origin[1] /= nPoints; origin[2] /= nPoints;						

	//fill the matrix
	memset(A, 0, sizeof(A));	
	for (int kkkk = 0; kkkk < nPoints; kkkk++)
	{		
		for (int ii = 0; ii < 3; ii++)
		{
			for (int jj = 0; jj < 3; jj++) {
				A[ii][jj] += (pTargetPoly[kkkk][ii] - origin[ii])*(pTargetPoly[kkkk][jj] - origin[jj]);
			}
		}
	}

	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < 3; jj++){
			A[ii][jj] /= (nPoints - 1);
		}
	}

	//compute eigen vectors, the principal axis is the first one
	double *ATemp[3], *V[3];
	for (int i = 0; i < 3; i++)
	{
		ATemp[i] = A[i];
		V[i] = eigenvectsT[i];
	}

	vtkMath::Jacobi(ATemp, eigenvals, V);

	double maxErr[3] = {0.0, 0.0, 0.0};
	for (int iAxis = 0; iAxis < 3; iAxis++)
	{
		//normal of best fitting plane is
		double normal[3] = {V[iAxis][0], V[iAxis][1], V[iAxis][2]};
		double dddd = vtkMath::Norm(normal);

		double d = -vtkMath::Dot(origin, normal);


		for (int kkkk = 0; kkkk < nPoints; kkkk++)
		{
			//	double u[3] = {pTargetPoly[kkkk][0] - origin[0], pTargetPoly[kkkk][1] - origin[1], pTargetPoly[kkkk][2] - origin[2]};
			double dd = fabs(vtkMath::Dot(pTargetPoly[kkkk], normal) + d);
			if (dd > maxErr[iAxis])
				maxErr[iAxis] = dd;
		}
	}


	FILE* f = fopen("coplanarity.txt", "at");
	fprintf(f, "%f, %f, %f, %f, %f, %f\n", eigenvals[0], eigenvals[1], eigenvals[2], 
		maxErr[0], maxErr[1], maxErr[2]);
	fclose(f);
	*/
}
#endif

//------------------------------------------------------------------------
//Creates an external rendering window and displays the fitted cube
void vtkMAFMuscleDecomposition::DebugVisualizeFitting(
	vtkMAFVisualDebugger* vd, 
  int nIndex, int nCount, LOCAL_FRAME& lfs,
  vtkPoints* template_O, vtkPoints* template_I,
  vtkPoints* target_O, vtkPoints* target_I, double dblScore, bool bBestOne)
//------------------------------------------------------------------------
{
	vd->AddOrUpdateSurface("Muscle", GetInput(), 0.7, 0, 0.4, 0.5);
	vd->AddOrUpdatePoints("OALandmarks", target_O, GetInput()->GetLength() / 200, 1, 0, 0);
	vd->AddOrUpdatePoints("IALandmarks", target_I, GetInput()->GetLength() / 200, 0, 0, 1);

 #pragma region Target Cube
  vtkPoints* pCubePts = vtkPoints::New();

  double x[3];
  pCubePts->InsertNextPoint(lfs.O);
  for (int k = 0; k < 3; k++){
	x[k] = lfs.O[k] + lfs.uvw[0][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] += lfs.uvw[1][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] += lfs.uvw[2][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] -= lfs.uvw[1][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] -= lfs.uvw[0][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] += lfs.uvw[1][k];
  }

  pCubePts->InsertNextPoint(x);
  for (int k = 0; k < 3; k++){
	x[k] -= lfs.uvw[2][k];
  }
  pCubePts->InsertNextPoint(x);

  vtkCellArray* pCells = vtkCellArray::New();
  vtkIdType CELLS[6*4] = {
	0,1,4,5,
	1,2,3,4,
	2,7,6,3,
	7,0,5,6,
	5,4,3,6,
	0,7,2,1,
  };

  for (int k = 0; k < 6; k++){
	pCells->InsertNextCell(4, &CELLS[k*4]);
  }

  vtkPolyData* pCubeSrc = vtkPolyData::New();
  pCubeSrc->SetPoints(pCubePts);
  pCubeSrc->SetLines(pCells);
  pCubePts->Delete();
  pCells->Delete();

	vd->AddOrUpdateLines("Cube", pCubeSrc, GetInput()->GetLength() / 600, 1, 1, 1);  
	pCubeSrc->Delete();
#pragma endregion Target Cube

#pragma region Template OI Areas
  vtkPoints* tmpIn[2] = { template_O, template_I };

  const char* FibTemplNames[] = {
	"vtkMAFPennateMuscleFibers", "vtkMAFFannedMuscleFibers", NULL,
  };

  int nFibTempl = 0;
  while (FibTemplNames[nFibTempl] != NULL)
  {
	if (strcmp(this->FibersTemplate->GetClassName(), FibTemplNames[nFibTempl]) == 0)
	  break;

	nFibTempl++;
  }

  vtkIdType Pennate_O[] = {5, 0, 1, 3, 2, 0};
  vtkIdType Pennate_I[] = {5, 0, 1, 3, 2, 0};
  vtkIdType Fanned_O[] = {5, 0, 1, 3, 2, 0};
  vtkIdType Fanned_I[] = {
	14, 0, 4, 6, 1, 3, 9, 7, 2, 0,
	4, 6, 9, 7, 4
	//5, 0, 1, 3, 2, 0
  };
  vtkIdType* Connectivity[] = {
	Pennate_O, Pennate_I, Fanned_O, Fanned_I, NULL, NULL };

  for (int i = 0; i < 2; i++)
  {    
	int nPoints = tmpIn[i]->GetNumberOfPoints();
	if (nPoints == 0)
	  continue; //invalid matching, skip it

	pCubePts = vtkPoints::New();
	for (int j = 0; j < nPoints; j++)
	{
	  //transform point j
	  //NOTE: fiber templates use left hand oriented coordinate system
	  double x[3];
	  double* pCoords = tmpIn[i]->GetPoint(j);
	  for (int k = 0; k < 3; k++){
		x[k] = lfs.O[k] + lfs.uvw[0][k]*pCoords[0] +
		  lfs.uvw[1][k]*pCoords[2] + lfs.uvw[2][k]*pCoords[1];
	  }

	  pCubePts->InsertNextPoint(x);
	}

	pCells = vtkCellArray::New();
	vtkIdType* pCell = Connectivity[2*nFibTempl + i];
	if (pCell != 0){
	  pCells->InsertNextCell(*pCell, &pCell[1]);
	}
	else
	{
	  vtkIdType cell[2];
	  for (int j = 0; j < nPoints / 2 - 1; j++) //front
	  {
		cell[0] = j; cell[1] = j + 1;
		pCells->InsertNextCell(2, cell);
	  }

	  for (int j = nPoints / 2; j < nPoints - 1; j++) //back
	  {
		cell[0] = j; cell[1] = j + 1;
		pCells->InsertNextCell(2, cell);
	  }

	  cell[0] = nPoints / 2 - 1; cell[1] = nPoints - 1;
	  pCells->InsertNextCell(2, cell);

	  cell[0] = nPoints / 2; cell[1] = 0;
	  pCells->InsertNextCell(2, cell);
	}

	pCubeSrc = vtkPolyData::New();
	pCubeSrc->SetPoints(pCubePts);
	pCubeSrc->SetLines(pCells);
	pCubePts->Delete();
	pCells->Delete();

		if (i == 0)
		{
			vd->AddOrUpdateLines("CubeOA", pCubeSrc, 
				GetInput()->GetLength() / 800, 1, 0, 0);  //origin = red
			vd->AddOrUpdatePoints("CubeO", pCubeSrc,
				GetInput()->GetLength() / 100, 1, 0, 0, 1.0, false, true);
		}
		else
		{
			vd->AddOrUpdateLines("CubeIA", pCubeSrc, 
				GetInput()->GetLength() / 800, 0, 0, 1);  //insertion = blue
			vd->AddOrUpdatePoints("CubeI", pCubeSrc,
				GetInput()->GetLength() / 100, 0, 0, 1, 1.0, false, true);
		}
		pCubeSrc->Delete();
	}
#pragma endregion Template OI Areas

  char szText[MAX_PATH];
#pragma warning(suppress: 4996)
  sprintf(szText, "#%d (out of %d) - score = %.2f", nIndex, nCount, dblScore);
  if (bBestOne)
#pragma warning(suppress: 4996)
	strcat(szText, " (BEST ONE)");

	if (bBestOne)
		vd->AddOrUpdateLabel("Score", szText, 12, 1, 0, 0, 0.08, 0.01, -1, -1);
	else
		vd->AddOrUpdateLabel("Score", szText, 12, 0.7, 0.7, 0.7, 0.08, 0.01, -1, -1);

  vd->SetBackgroundColor(1, 1, 1);
	vd->DebugStep();
	vd->RemoveAll();
}

//------------------------------------------------------------------------
//Creates an external rendering window and displays the slicing process
void vtkMAFMuscleDecomposition::DebugVisualizeSlicing(
	vtkMAFVisualDebugger* vd, LOCAL_FRAME& lfs,
	vtkPoints* target_O, vtkPoints* target_I,
	vtkPolyData* template_Contours, vtkPolyData* target_Contours,
	vtkPolyData* mapping_Links
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
		, vtkPolyData* target_Countours_Planes
#endif
	)
//------------------------------------------------------------------------
{
	//RELEASE NOTE: as vtkPoints stores the points in floats it may happen that two different points 
	//will coincide in vtkPoints and as a result vtkTubeFilter may fail - it is not problem.

	//Prepare Target
#ifndef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
		vd->AddOrUpdateSurface("Muscle", GetInput(), 
			0.9599999785423279, 0.2899999916553497, 0.2899999916553497,	
		0.5);
#else
	vd->AddOrUpdateSurface("Muscle", GetInput(), 0.7, 0, 0.4, 0.5);
#endif	

	vd->AddOrUpdatePoints("OALandmarks", target_O, GetInput()->GetLength() / 200, 1, 0, 0);
	vd->AddOrUpdatePoints("IALandmarks", target_I, GetInput()->GetLength() / 200, 0, 0, 1);
//	vd->AddOrUpdateLines("Template", template_Contours, GetInput()->GetLength() / 600, 0, 0, 1);
	vd->AddOrUpdateLines("Target", target_Contours, GetInput()->GetLength() / 600, 1, 0, 0);
#ifndef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY	
//	vd->AddOrUpdateLines("Links", mapping_Links,GetInput()->GetLength() / 1800, 1, 1, 0);
#endif

#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
	vd->AddOrUpdateSurface("TargetPlanes", target_Countours_Planes, 0.5, 0.5, 0.5, 0.5);
#endif

	vd->SetBackgroundColor(1, 1, 1);
	vd->DebugStep();
	vd->RemoveAll();
}

//------------------------------------------------------------------------
//Creates an external rendering window and displays the fibres produced
void vtkMAFMuscleDecomposition::DebugVisualizeFibresPostprocessing(vtkMAFVisualDebugger* vd, 
																   vtkPoints* target_O, vtkPoints* target_I, vtkPolyData* fibres
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
																   , vtkPolyData* analyticalFibres
#endif
																   )
{
	fibres->BuildCells();
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
	analyticalFibres->BuildCells();
#endif

	//Prepare Target
	vd->AddOrUpdateSurface("Muscle", GetInput(), 
		0.9599999785423279, 0.2899999916553497, 0.2899999916553497,		
		0.25);
	vd->AddOrUpdatePoints("OALandmarks", target_O, GetInput()->GetLength() / 200, 1, 0, 0);
	vd->AddOrUpdatePoints("IALandmarks", target_I, GetInput()->GetLength() / 200, 0, 0, 1);

	vd->AddOrUpdateLines("Fibres", fibres, GetInput()->GetLength() / 600, 1, 1, 0);
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
	vd->AddOrUpdateLines("AnalyticalFibres", analyticalFibres, GetInput()->GetLength() / 800, 0, 1, 0);
#endif
	vd->SetBackgroundColor(1, 1, 1);
	vd->DebugStep();
	vd->RemoveAll();
}

#ifdef ADV_KUKACKA
//------------------------------------------------------------------------
//Creates an external rendering window and displays the harmonic weights. 
void vtkMAFMuscleDecomposition::DebugVisualizeHarmonicWeights(vtkPolyData* surface, vtkPoints* target_O, vtkPoints* target_I,
															  vtkPoints* proj_O, vtkPoints* proj_I)
//------------------------------------------------------------------------
{
	vtkMAFSmartPointer< vtkMAFVisualDebugger > vd;
	vd->SetBackgroundColor(1, 1, 1);

	vd->AddOrUpdateScalarField("HarmFce", surface, true);
	vd->AddOrUpdateSurface("Muscle", GetInput(), 
		//0.9599999785423279, 0.2899999916553497, 0.2899999916553497
		0, 0, 0, 1, true);
	vd->AddOrUpdatePoints("OALandmarks", target_O, GetInput()->GetLength() / 200, 1, 0, 0);
	vd->AddOrUpdatePoints("IALandmarks", target_I, GetInput()->GetLength() / 200, 0, 0, 1);

	if (proj_O != NULL) {
		vd->AddOrUpdatePoints("OALandmarks_PRJ", proj_O, GetInput()->GetLength() / 300, 0.5, 0, 0);
	}

	if (proj_I != NULL) {
		vd->AddOrUpdatePoints("IALandmarks_PRJ", proj_I, GetInput()->GetLength() / 300, 0, 0, 0.5);
	}

	vd->DebugStep();
}
#endif

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "MuscleDecompositionTests/vtkParametricFunctionSource.h"
#include "MuscleDecompositionTests/vtkParametricCylinder.h"
#include "MuscleDecompositionTests/vtkParametricTorus.h"
#include "MuscleDecompositionTests/vtkParametricEllipsoid.h"
#include "MuscleDecompositionTests/vtkParametricMuscle.h"
#include "vtkCleanPolyData.h"

#define CAP_PORTION		0.15
#define U_SAMPLES		100
#define V_SAMPLES		20

#define U_DELTA_MM		0.1	//1/10 mm

//------------------------------------------------------------------------
//Creates parametric function.
vtkParametricFunction* vtkMAFMuscleDecomposition::CreateParametricFunction(enum DataType dataType)
{
	vtkParametricFunction* function;
	switch (dataType)
	{
	case vtkMAFMuscleDecomposition::Cylinder:
		{
			vtkParametricCylinder* cyl = vtkParametricCylinder::New();
			cyl->SetRadius(12.5);		//12.5 mm => 2.5 cm 
			cyl->SetHeight(100);		//and 10 cm, a good muscle shape
			function = cyl;
			break;
		}
	case vtkMAFMuscleDecomposition::Torus:
		{
			vtkParametricTorus* cyl = vtkParametricTorus::New();
			cyl->SetCrossRadius(12.5);	//12.5 mm => 2.5 cm 
			cyl->SetRadius(100);		//and 10 cm, a good muscle shape
			cyl->SetMaximumU(cyl->GetMaximumU()*3/5.0);
			function = cyl;
			break;
		}

	case vtkMAFMuscleDecomposition::Ellipsoid:
		{
			vtkParametricEllipsoid* cyl = vtkParametricEllipsoid::New();
			cyl->SetXRadius(10);	//2 cm 
			cyl->SetYRadius(15);	//3 cm
			cyl->SetZRadius(50);	//10 cm
			function = cyl;
			break;
		}

	case vtkMAFMuscleDecomposition::Muscle1:
		{
			vtkParametricMuscle* cyl = vtkParametricMuscle::New();
			cyl->SetRadiusX(50);			//5 cm 
			cyl->SetRadiusY(150);			//15 cm
			cyl->SetCrossRadiusX1(12.5);	//2.5 cm
			cyl->SetCrossRadiusZ1(0.5);		//1 cm
			cyl->SetCrossRadiusX2(40);		//8 cm
			cyl->SetCrossRadiusZ2(12.5);	//2.5 cm
			function = cyl;
			break;
		}

	default:
		break;
	}

	function->CapPortionAvailableOn();
	function->SetCapPortion(CAP_PORTION);	//1/3 are caps
	function->OnionLayersOn();
	return function;
}


//------------------------------------------------------------------------
//Generates the input data for the testing. 
void vtkMAFMuscleDecomposition::GenerateInputData(enum DataType dataType)
{
	vtkSmartPointer< vtkParametricFunction > function = CreateParametricFunction(dataType);
	function->UnRegister(this);	//decrease the reference from CreateParametricFunction

	double uCap = function->GetCapPortion(); //in portion
	
	vtkMAFSmartPointer< vtkParametricFunctionSource > source;	
	source->SetParametricFunction(function.GetPointer());
	source->Update();
	
	//this->SetNumberOfFibres(20);	//20 Fibres is enough
	this->SetResolution(35);
	/*
	switch (dataType)
	{
	case vtkMAFMuscleDecomposition::Cylinder:
		{			
			break;
		}
	case vtkMAFMuscleDecomposition::Torus:
		{
			vtkParametricTorus* cyl = (vtkParametricTorus*)function.GetPointer();
			double per = cyl->GetRadius()*(cyl->GetMaximumU() - cyl->GetMinimumU());
			this->SetResolution(35);//(int)(per / (100*U_DELTA_MM)));	//100* less then U_DELTA_MM
			
			break;
		}

	default:
		break;
	}
	*/

	/*vtkMAFSmartPointer< vtkCleanPolyData > cleanPoly;
	cleanPoly->SetInput(destrips->GetOutput());
	cleanPoly->Update();*/

	this->SetInput(source->GetOutput());

	//NOW we need to specify the attachment areas
	vtkMAFSmartPointer< vtkPoints > oriPoints;
	vtkMAFSmartPointer< vtkPoints > insPoints;

	int MaxPts = V_SAMPLES;
	oriPoints->SetNumberOfPoints(MaxPts);
	insPoints->SetNumberOfPoints(MaxPts);
	
	double uMin = function->GetMinimumU();
	double uMax = function->GetMaximumU();
	uCap *= (uMax - uMin);	//in u parameter
	uMin += uCap;	//uMin is where the regular part starts
	uMax -= uCap;	//uMax is where the regular part ends

	double vMin = function->GetMinimumV();
	double vMax = function->GetMaximumV();
	double vStep = (vMax - vMin) / MaxPts;

	double uvw[3];	
	uvw[2] = 1.0;	//w is not used (it is onion effect)
	for (int i = 0; i < MaxPts; i++)
	{
		uvw[1] = vMin + vStep*i;

		double Pt[3];
		uvw[0] = uMin;
		function->Evaluate(uvw, Pt);
		oriPoints->SetPoint(i, Pt);
		
		uvw[0] = uMax;
		function->Evaluate(uvw, Pt);
		insPoints->SetPoint(i, Pt);
	}

	this->SetOriginArea(oriPoints);
	this->SetInsertionArea(insPoints);	
}

//------------------------------------------------------------------------
//Fit the analytical curve to the constructed fibre and sample this curve to a new fibre in pOut.
//The fibre is described by the cell in pFibres with Id = fibreId. The method returns MSE error of fitting.
//Fitting is done by non-linear least-square fitting. 
double vtkMAFMuscleDecomposition::FitCurveToFibre(enum DataType dataType, vtkPolyData* pFibres, int fibreId, vtkPolyData* pOut)
{
	FIT_CONTEXT_DATA ssFit;	
	ssFit.pSamples = SampleFibre(pFibres, fibreId, ssFit.nSamples);
	
	vtkSmartPointer< vtkParametricFunction > function = CreateParametricFunction(dataType);
	function->UnRegister(this);	//decrease the reference from CreateParametricFunction
	ssFit.pFunction = function.GetPointer();

	//find the initial solution
	alglib::real_1d_array x;
	x.setlength(2);	

	double Pt[3], uvw[3];
	vtkIdType nPts, *pPts;
	pFibres->GetCellPoints(fibreId, nPts, pPts);
	pFibres->GetPoint(pPts[nPts / 2], Pt);


	/*const double testCases[] = 
	{
		0.6, 1.6, 0.02,
		0.6, 3.2, 0.02,
		0, 0, 0.5,
		0.3, vtkMath::DoublePi() / 2, 0,
		0.3, vtkMath::DoublePi() / 2, 0.2,
		vtkMath::DoublePi() / 2, vtkMath::DoublePi() / 2, 1.0,
		-1.0, -1.0, -1.0,
	};

	for (int exp = 0; exp < 2; exp++)
	{
	int index = 0;
	while (testCases[index] >= 0)
	{
		double uvwOrig[3] = {testCases[index], testCases[index + 1], testCases[index + 2], };

		function->Evaluate(uvwOrig, Pt);
		function->EvaluateInverse(Pt, uvw);

		double err = (uvw[0] - uvwOrig[0])*(uvw[0] - uvwOrig[0]) + 
			(uvw[1] - uvwOrig[1])*(uvw[1] - uvwOrig[1]) + 
			(uvw[2] - uvwOrig[2])*(uvw[2] - uvwOrig[2]);
		if (err >= 1e-7) {
			_RPT1(_CRT_WARN, "ERROR: %f\n", err);
		}
		index += 3;
	}

	((vtkParametricMuscle*)function.GetPointer())->SetRadiusY(((vtkParametricMuscle*)function.GetPointer())->GetRadiusX());
	}*/






	if (!function->EvaluateInverse(Pt, uvw)) {
		x[0] = 0.5*(function->GetMaximumV() + function->GetMinimumV()); x[1] = 0.5;
	}
	else {
		x[0] = uvw[1]; x[1] = uvw[2];
	}
	
	double epsg = 0.0000000001;
	double epsf = 0;
	double epsx = 0;
	alglib::ae_int_t maxits = 100;	//no more than 100 iterations
	alglib::minlmstate state;
	alglib::minlmreport rep;

	alglib::minlmcreatevj(ssFit.nSamples, x, state);
	alglib::minlmsetcond(state, epsg, epsf, epsx, maxits);
	alglib::minlmoptimize(state, FitCurveToFibre_fvec, FitCurveToFibre_jac, NULL, &ssFit);
	alglib::minlmresults(state, x, rep);

	//sample the fitted curve and measure error
	vtkPoints* points = pOut->GetPoints();
	vtkCellArray* cells = pOut->GetLines();
	
	//ERROR must be calculated differently
	double dblMSE = 0.0;

	vtkIdType* pCurve = new vtkIdType[ssFit.nSamples];
	int nValidPoints = 0;
	
	uvw[0] = 0.0; uvw[1] = x[0]; uvw[2] = x[1];	
	for (int i = 0; i < ssFit.nSamples; i++)
	{
		double Fu[3];
		FitCurveToFibre_Optimize_U(&ssFit, i, uvw);
		function->Evaluate(uvw, Fu);	//calculate Fu	

		for (int k = 0; k < 3; k++) {
			double dk = Fu[k] - ssFit.pSamples[i][k];
			dblMSE += dk*dk;
		}

		pCurve[nValidPoints] = points->InsertNextPoint(Fu);

		//points stores data in floats => there might be now duplicity
		if (nValidPoints == 0)
			nValidPoints++;
		else
		{
			double Fu2[3];
			points->GetPoint(pCurve[nValidPoints], Fu);
			points->GetPoint(pCurve[nValidPoints - 1], Fu2);
			if (Fu[0] != Fu2[0] || Fu[1] != Fu2[1] || Fu[2] != Fu2[2])
				nValidPoints++;
		}
	}

	cells->InsertNextCell(nValidPoints, pCurve);

	delete[] pCurve;
	delete[] ssFit.pSamples;
	return dblMSE / ssFit.nSamples;
}

//------------------------------------------------------------------------
//Calculate the length of fibre
double vtkMAFMuscleDecomposition::MeassureFibreLength(vtkPolyData* pFibres, int fibreId)
{
	vtkIdType nPts, *pPts;
	pFibres->GetCellPoints(fibreId, nPts, pPts);
	
	double ptBuf[6];
	double* Points[2] = {&ptBuf[0], &ptBuf[3]};
	pFibres->GetPoint(*pPts, Points[0]);

	double dblLength = 0.0; 
	for (int i = 1; i < nPts; i++)
	{		
		pFibres->GetPoint(*(++pPts), Points[1]);
		dblLength += sqrt(vtkMath::Distance2BetweenPoints(Points[0], Points[1]));
		
		//swap pointers to points
		double* tmp = Points[0];
		Points[0] = Points[1];
		Points[1] = tmp;
	}

	return dblLength;
}

//------------------------------------------------------------------------
//Samples uniformly the fibre identified by the cell with Id = fibreId. The number of samples is in nSamples.
vtkMAFMuscleDecomposition::VCoord* vtkMAFMuscleDecomposition::SampleFibre(vtkPolyData* pFibres, int fibreId, int& nSamples)
{		
	vtkIdType nPts, *pPts, *pPtsIds;
	pFibres->GetCellPoints(fibreId, nPts, pPtsIds);
	
	double ptBuf[6];
	double* Points[2] = {&ptBuf[0], &ptBuf[3]};
	pFibres->GetPoint(*pPtsIds, Points[0]);

	double dblLength = 0.0; 
	double* knot = new double[nPts];
	double dblTotalLength = MeassureFibreLength(pFibres, fibreId);	

	pPts = pPtsIds;
	for (int i = 1; i < nPts; i++)
	{		
		pFibres->GetPoint(*(++pPts), Points[1]);
		dblLength += sqrt(vtkMath::Distance2BetweenPoints(Points[0], Points[1]));
		knot[i] = dblLength / dblTotalLength;
		
		//swap pointers to points
		double* tmp = Points[0];
		Points[0] = Points[1];
		Points[1] = tmp;
	}

	knot[0] = 0.0; knot[nPts - 1] = 1.0;
	
	nSamples = (int)(dblTotalLength / U_DELTA_MM + 0.5);	

	VCoord* output = new VCoord[nSamples];
	double uStep = 1.0 / (nSamples - 1);
	for (int i = 1, j = 0; i < nSamples - 1; i++)
	{
		double u = uStep*i;
		while (knot[j] < u) {
			++j;
		}

		//interpolate points [j - 1] and [j]
		VCoord ptA, ptB;
		pFibres->GetPoint(pPtsIds[j - 1], ptA);
		pFibres->GetPoint(pPtsIds[j], ptB);

		double t = (u - knot[j - 1]) / (knot[j] - knot[j - 1]);
		for (int k = 0; k < 3; k++)
		{
			output[i][k] = ptA[k]*(1 - t) + ptB[k]*(t);
		}
	}

	pFibres->GetPoint(pPtsIds[0], output[0]);
	pFibres->GetPoint(pPtsIds[nPts - 1], output[nSamples - 1]);

	delete[] knot;
	return output;
}

//------------------------------------------------------------------------
//Calculates objective function 
/*static*/ void vtkMAFMuscleDecomposition::FitCurveToFibre_fvec(const alglib::real_1d_array &x, alglib::real_1d_array &fi, void *ptr)
{
	alglib::real_2d_array jac;
	jac.setlength(((FIT_CONTEXT_DATA*)ptr)->nSamples, 2);	//2 = number of parameters to optimize
	FitCurveToFibre_jac(x, fi, jac, ptr);
}

//------------------------------------------------------------------------
//Calculates Jacobian function
/*static*/ void vtkMAFMuscleDecomposition::FitCurveToFibre_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr)
{	
	//fi = ||F(u_i; x[0], x[1]) - P(u_i)||^2, i.e., square distance between point P(u_i) from the produced fibre and point F(u_i) from the analytical curve with parameters x[0] and x[1] (v, w)
	//Jacobian matrix J = [dfi/dxj], i.e.,
	//J[i, j] = 2*(F(u_i, x[0], x[1]) - P(u_i))*(dF_x(u_i, x[0], x[1])/dxj, dF_y(u_i, x[0], x[1])/dxj, dF_z(u_i, x[0], x[1])/dxj)

	FIT_CONTEXT_DATA* pData = (FIT_CONTEXT_DATA*)ptr;
	
	double uvw[3] = {0.0, x[0], x[1]};	
	for (int i = 0; i < pData->nSamples; i++)
	{
		FitCurveToFibre_Optimize_U(pData, i, uvw);	//calculate uvw

		double Fu[3];		
		pData->pFunction->Evaluate(uvw, Fu);	//calculate Fu	

		double dx = Fu[0] - pData->pSamples[i][0];
		double dy = Fu[1] - pData->pSamples[i][1];
		double dz = Fu[2] - pData->pSamples[i][2];

		fi[i] = dx*dx + dy*dy + dz*dz;
				
		double Duvw[9];
		pData->pFunction->EvaluateDerivative(uvw, Duvw);	//calculate derivative of Fu	

		double* Dv = Duvw + 3;
		double* Dw = Duvw + 6;
		jac[i][0] = 2*dx*Dv[0] + 2*dy*Dv[1] + 2*dz*Dv[1];
		jac[i][1] = 2*dx*Dw[0] + 2*dy*Dw[1] + 2*dz*Dw[1];
	}

}


#include "solvers.h"
//------------------------------------------------------------------------
//Finds the optimal parameter u so that F(u_optimal; v, w) is the closest to the point pData->pSamples[ptIdx],
//where v = uvw[1], w = uvw[2] are fixed parameters of the curve and uvw[0] = u_optimal upon the exit of this method.
/*static*/ void vtkMAFMuscleDecomposition::FitCurveToFibre_Optimize_U(FIT_CONTEXT_DATA* pData,  int ptIdx, double* uvw)
{	
	//get the initial solution assuming that u is length parameter of the curve, i.e., ||F(u + const1) - F(u)|| = const2 for any u
	double uMin = pData->pFunction->GetMinimumU();
	double uMax = pData->pFunction->GetMaximumU();
	double uCap = (uMax - uMin)* CAP_PORTION;	//in u parameter
	uMin += uCap;	//uMin is where the regular part starts
	uMax -= uCap;	//uMax is where the regular part ends
		
	double uStep = (uMax - uMin) / pData->nSamples;
	uvw[0] = uMin + ptIdx*uStep;

	FIT_OPTIMIZE_U_CONTEXT_DATA ssFit;
	ssFit.pFunction = pData->pFunction;
	memcpy(ssFit.uvw, uvw, 3*sizeof(double));

	memcpy(ssFit.Pt, pData->pSamples[ptIdx], sizeof(ssFit.Pt));
	

	//we are looking for the point Q lying on the curve F(u; v, w) such that ||Q - P|| is minimal
	//If the distance is to be minimal, vector (Q-P) must be perpendicular to tangent vector of F(u; v, w) at Q	
	//=> grad F(u; v, w)*(F(u; v, w) - P) = 0 and we are looking for solution of this equation
	
	double epsf = 0;	
	alglib::ae_int_t maxits = 0;
	alglib::nleqstate state;
	alglib::nleqreport rep;

	alglib::real_1d_array x;
	x.setlength(1);	
	x[0] = uvw[0];

	alglib::nleqcreatelm(1, x, state);
	alglib::nleqsetcond(state, epsf, maxits);
	alglib::nleqsolve(state, FitCurveToFibre_Optimize_U_fvec, FitCurveToFibre_Optimize_U_jac, NULL, &ssFit);
	alglib::nleqresults(state, x, rep);	

	uvw[0] = x[0];
}


/** Calculates objective function */
/*static*/ void vtkMAFMuscleDecomposition::FitCurveToFibre_Optimize_U_fvec(const alglib::real_1d_array &x, double &func, void *ptr)
{
	 //FUNC = dF(u; v, w)/du*(F(u; v, w) - P)
	FIT_OPTIMIZE_U_CONTEXT_DATA* pData = (FIT_OPTIMIZE_U_CONTEXT_DATA*)ptr;
	
	double Duvw[9], Q[3];
	pData->uvw[0] = x[0];
	pData->pFunction->Evaluate(pData->uvw, Q);
	pData->pFunction->EvaluateDerivative(pData->uvw, Duvw);
	
	//merit may be the same as fi
	func =	Duvw[0]*(Q[0] - pData->Pt[0]) + 
			Duvw[1]*(Q[1] - pData->Pt[1]) +
			Duvw[2]*(Q[2] - pData->Pt[2]);
	func *= func;	//objective function must be  f=F[0]^2+...+F[m-1]^2
}

/** Calculates Jacobian function */
/*static*/ void vtkMAFMuscleDecomposition::FitCurveToFibre_Optimize_U_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr)
{
	//FUNC = dF(u; v, w)/du*(F(u; v, w) - P)
	FIT_OPTIMIZE_U_CONTEXT_DATA* pData = (FIT_OPTIMIZE_U_CONTEXT_DATA*)ptr;
	
	double Duvw[9], Q[3];
	pData->uvw[0] = x[0];
	pData->pFunction->Evaluate(pData->uvw, Q);
	pData->pFunction->EvaluateDerivative(pData->uvw, Duvw);
	
	fi[0] =	Duvw[0]*(Q[0] - pData->Pt[0]) + 
			Duvw[1]*(Q[1] - pData->Pt[1]) +
			Duvw[2]*(Q[2] - pData->Pt[2]);

	//Jacobian FUNC = d/du [ dF(u; v, w)/du * ( F(u; v, w) - P) ] =>
	//d/du [ dFx(u; v, w) / du * (Fx(u; v, w) - Px) + dFy(u; v, w) / du * (Fy(u; v, w) - Py) + dFz(u; v, w) / du * (Fz(u; v, w) - Pz) ] =>
	//d/du [ dFx(u; v, w) / du * Fx(u; v, w) - dFx(u; v, w) / du * Px +  ... ] =>
	//d/du [ dFx(u; v, w) / du * Fx(u; v, w) + ... ] - d/du [ dFx(u; v, w) / du * Px + ... ] =>
	//d2Fx(u; v, w) / du2 * Fx(u; v, w) + dFx(u; v, w) / du * dFx(u; v, w) / du + ... - [d2Fx(u; v, w) / du2 * Px + ...] =>
	//F''(u; v, w)*(F(u; v, w) - P) + F'(u; v, w)*F'(u; v, w)

	double D2uvw[9];
	pData->pFunction->EvaluateDerivative2(pData->uvw, D2uvw);
	jac[0][0] = D2uvw[0]*(Q[0] - pData->Pt[0]) + 
				D2uvw[1]*(Q[1] - pData->Pt[1]) +
				D2uvw[2]*(Q[2] - pData->Pt[2]) +
				Duvw[0]*Duvw[0] +
				Duvw[1]*Duvw[1] +
				Duvw[2]*Duvw[2];
}
#endif
#pragma endregion 