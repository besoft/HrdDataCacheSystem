/*=========================================================================
Program: Multimod Application Framework RELOADED
Module: $RCSfile: vtkMAFMuscleDecomposition.cpp,v $
Language: C++
Date: $Date: 2012-04-04 18:43:42 $
Version: $Revision: 1.1.1.1.2.5 $
Authors: Josef Kohout (Josef.Kohout *AT* beds.ac.uk)
==========================================================================
Copyright (c) 2008 University of Bedfordshire (www.beds.ac.uk)
See the COPYINGS file for license details
=========================================================================
*/

#include "vtkMAFMuscleDecomposition.h"
#include "vtkMAFVisualDebugger.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"
#include "vtkMath.h"
#ifdef ADV_KUKACKA
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkContourFilter.h"
#pragma warning(pop)
#endif

#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkCellLocator.h"
#include "vtkSmartPointer.h"
#include "vtkMAFSmartPointer.h"
#include "vtkGenericCell.h"
#include "vtkMAFMeanValueCoordinatesInterpolation.h"

#include <math.h>
#include <float.h>
#include <vector>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#define HARMFUNC_MIN	1		//boundaries of harmonic function
#define HARMFUNC_MAX	10

vtkCxxRevisionMacro(vtkMAFMuscleDecomposition, "$Revision: 1.1.1.1.2.5 $");
vtkStandardNewMacro(vtkMAFMuscleDecomposition);

#include "mafMemDbg.h"
#include "mafDbg.h"

#ifdef ADV_KUKACKA_TEST
__PROFILING_DECLARE_DEFAULT_PROFILER(true);
#endif

vtkMAFMuscleDecomposition::vtkMAFMuscleDecomposition()
{
	this->Resolution = 9;       //nine segments => 10 points
	this->NumberOfFibres = 50;

	this->FibersTemplate = NULL;
	this->OriginArea = NULL;
	this->InsertionArea = NULL;

	this->SmoothFibers = 1;
	this->SmoothSteps = 5;
	this->SmoothFactor = 4.0;
	this->DebugMode = dbgNone;

	this->AdvancedSlicing = 0;
	this->AdvancedKukacka = 0;
	this->UniformSampling = 0;

	this->InputTemplate = NULL;
}

vtkMAFMuscleDecomposition::~vtkMAFMuscleDecomposition()
{
	if (FibersTemplate != NULL) {
		FibersTemplate->Delete();
		FibersTemplate = NULL;
	}

	if (OriginArea != NULL) {
		OriginArea->Delete();
		OriginArea = NULL;
	}

	if (InsertionArea != NULL) {
		InsertionArea->Delete();
		InsertionArea = NULL;
	}
}

//------------------------------------------------------------------------
//Sets new template for muscle fibers
/*virtual*/ void vtkMAFMuscleDecomposition::SetFibersTemplate(vtkMAFMuscleFibers* pTemplate)
	//------------------------------------------------------------------------
{
	if (pTemplate != FibersTemplate)
	{
		if (FibersTemplate != NULL)
			FibersTemplate->Delete();

		if (NULL != (FibersTemplate = pTemplate))
			FibersTemplate->Register(this);

		this->Modified();
	}
}

//------------------------------------------------------------------------
//Sets new origin area points for the input muscle
/*virtual*/ void vtkMAFMuscleDecomposition::SetOriginArea(vtkPoints* pPoints)
	//------------------------------------------------------------------------
{
	if (pPoints != OriginArea)
	{
		if (OriginArea != NULL)
			OriginArea->Delete();

		if (NULL != (OriginArea = pPoints))
			OriginArea->Register(this);

		this->Modified();
	}
}

//------------------------------------------------------------------------
//Sets new insertion area points for the input muscle
/*virtual*/ void vtkMAFMuscleDecomposition::SetInsertionArea(vtkPoints* pPoints)
	//------------------------------------------------------------------------
{
	if (pPoints != InsertionArea)
	{
		if (InsertionArea != NULL)
			InsertionArea->Delete();

		if (NULL != (InsertionArea = pPoints))
			InsertionArea->Register(this);

		this->Modified();
	}
}

//------------------------------------------------------------------------
//Samples E2 space <0..1>x<0..1> storing samples into points buffer.
//N samples are created. Points buffer must be capable enough to hold
//2*N doubles (format is x1,y1,x2,y2...).
//The routine is based on the code by Frances Y. Kuo <f.kuo@unsw.edu.au>,
//School of Mathematics and Statistics, University of New South Wales,
//Sydney NSW 2052, Australia
void vtkMAFMuscleDecomposition::CreateSobolPoints(int N, double* points)
	//------------------------------------------------------------------------
{
	if (N == 1)
	{
		points[0] = points[1] = 0.5;
		return;
	}

	// L = max number of bits needed
	int L = (int)ceil(log((double)N)/log(2.0));

	//generate an array C, C[i] = index from the right of the first zero bit of i
	int* C = new int[N];
	for (int i = 0; i < N; i++)
	{
		C[i] = 1;
		int value = i;
		while (value & 1) {
			value >>= 1;
			C[i]++;
		}
	}

	// Compute direction numbers V[1] to V[L], scaled by pow(2,32)
	typedef unsigned long ULong2[2];
	ULong2* V = new ULong2[L+1];
	V[1][0] = V[1][1] = 1 << 31; //32 - 1
	for (int i = 2; i <= L; i++)
	{
		V[i][0] = 1 << (32 - i); // all m's = 1
		V[i][1] = V[i - 1][1] ^ (V[i - 1][1] >> 1);
	}

	//Compute points (from bits)
	ULong2 X = {0, 0};
	for (int i = 0; i < N; i++)
	{
		for (unsigned j = 0; j < 2; j++)
		{
			X[j] = X[j] ^ V[C[i]][j];
			points[2*i + j] = X[j] / 4294967296.0; //pow(2.0,32);
		}
	}

	// Clean up
	delete [] V;
	delete [] C;
}

//------------------------------------------------------------------------
//Samples E2 space <0..1>x<0..1> uniformly storing samples into points buffer.
//N samples are created. Points buffer must be capable enough to hold
//2*N doubles (format is x1,y1,x2,y2...).
//N.B: N must be k*k, where k is an integer.
void vtkMAFMuscleDecomposition::CreateUniformPoints(int N, double* points)
	//------------------------------------------------------------------------
{
	// uniform sampling
	int sqrtN = (int)sqrt((double)N);
	for(int i = 0; i < sqrtN; ++i) {
		for(int j = 0; j < sqrtN; ++j) {
			points[(i * sqrtN + j) * 2 + 0] = i / (double)sqrtN;
			points[(i * sqrtN + j) * 2 + 1] = j / (double)sqrtN;
		}
	}
}

//------------------------------------------------------------------------
//Sorts the given points according to their iCoord coordinate.
//The resulting order is returned and the user is responsible for its
//deallocation when it is no longer needed. pPoints array is not touched.
int* vtkMAFMuscleDecomposition::SortPoints(const VCoord* pPoints, int nPoints, int iCoord)
	//------------------------------------------------------------------------
{
	int* pOrder = new int[nPoints];
	for (int i = 0; i < nPoints; i++){
		pOrder[i] = i;
	}

	//heap-sort
	//construct heap by successive adding of points
	for (int i = 1; i < nPoints; i++)   //1 point is already sorted
	{
		int k = i;
		while (k > 0)
		{
			int kPar = ((k + 1) / 2) - 1; //get parent index
			if (pPoints[pOrder[kPar]][iCoord] >= pPoints[pOrder[k]][iCoord])
				break;  //heap restored

			int tmp = pOrder[k];
			pOrder[k] = pOrder[kPar];
			pOrder[kPar] = tmp;

			k = kPar;
		}
	}

	//successive sorting (swap maximum at first position with the last item)
	//and restore the heap considering one item less
	for (int i = nPoints - 1; i > 0; i--)
	{
		//swap
		int tmp = pOrder[0];
		pOrder[0] = pOrder[i];
		pOrder[i] = tmp;

		//move top down in the tree (while there is some child)
		int k = 0;
		while (2*(k + 1) < i)
		{
			int j = 2*k + 1;
			if (j + 1 < i && pPoints[pOrder[j]][iCoord] < pPoints[pOrder[j + 1]][iCoord])
				j++;  //goto the right child

			if (pPoints[pOrder[k]][iCoord] >= pPoints[pOrder[j]][iCoord])
				break; //heap restored

			tmp = pOrder[k];
			pOrder[k] = pOrder[j];
			pOrder[j] = tmp;

			k = j;
		}
	}

	return pOrder;
}

//------------------------------------------------------------------------
//Gets edges from the contour and sorts them to form continuous path. The
//format of the returned array (the caller is responsible for its
//deallocation) is s1,s2,s2,s3,s3,s4, ... sn,s1 - for instance:
//0,2,2,3,3,4,4,6,0;
int* vtkMAFMuscleDecomposition::GetSortedEdges(vtkPolyData* contour
	//, const double* orientationNormal, bool orientationCCW
	)
	//------------------------------------------------------------------------
{
	contour->BuildLinks();
	int nCells = contour->GetNumberOfCells();
	int nEntries = 2*nCells;

	int* pEdgeIds = new int[nEntries];
	for (int i = 0, j = 0; i < nCells; i++, j += 2)
	{
		vtkIdType nPts, *pPts;
		contour->GetCellPoints(i, nPts, pPts);
		_ASSERT(nPts == 2);

		pEdgeIds[j + 0] = pPts[0];
		pEdgeIds[j + 1] = pPts[1];
	}

	//pEdgeIds now contains edges whose indices are sorted
	int nSearchVal = pEdgeIds[1];
	int iChain = 0;
	int iStartPos = 2;

	while (iStartPos < nEntries)
	{
		//search for nSearchVal
		int iFoundPos = -1;
		for (int i = iStartPos; i < nEntries; i += 2)
		{
			if (pEdgeIds[i] == nSearchVal) {
				iFoundPos = i; break;
			}
		}

		_ASSERT(iFoundPos >= 0);
		if (iFoundPos != iStartPos)
		{
			//swap edges
			int tmp = pEdgeIds[iFoundPos];
			pEdgeIds[iFoundPos] = pEdgeIds[iStartPos];
			pEdgeIds[iStartPos] = tmp;

			tmp = pEdgeIds[iFoundPos + 1];
			pEdgeIds[iFoundPos + 1] = pEdgeIds[iStartPos + 1];
			pEdgeIds[iStartPos + 1] = tmp;
		}

		nSearchVal = pEdgeIds[iStartPos + 1]; //continue with this value
		iStartPos += 2;

		//if the contour is in several pieces we might have found the end
		if (nSearchVal == pEdgeIds[iChain])
		{
			//continue with the next polygon
			if (iStartPos < nEntries)
			{
				//if there is any other polygon
				iChain = iStartPos;
				nSearchVal = pEdgeIds[iStartPos + 1];
				iStartPos += 2;
			}
		}
	}

	/*
	//BES: 11.10.2013 - perform orientation - !!! THIS DOES NOT WORK
	int sgnPlus = 0, sgnNeg = 0;
	for (int i = 0; i < nEntries; i += 2)
	{
		int i0 = i;
		int i1 = i + 1;
		int i2 = (i + 3) % nEntries;

		VCoord P0, P1, P2;
		contour->GetPoint(pEdgeIds[i0], P0);
		contour->GetPoint(pEdgeIds[i1], P1);
		contour->GetPoint(pEdgeIds[i2], P2);

		double u[3], v[3], n[3];
		for (int j = 0; j < 3; j++)
		{
			u[j] = P1[j] - P0[j];
			v[j] = P2[j] - P0[j];
		}

		vtkMath::Cross(u, v, n);		
		if (vtkMath::Dot(n, orientationNormal) >= 0.0)
			sgnPlus++;		
		else 
			sgnNeg++;
	}

	if ((sgnPlus > sgnNeg) ^ (orientationCCW))
	{
		//we need to change the orientation
		int i = 1, j = nEntries - 2;
		while (i < j)
		{
			int tmp = pEdgeIds[i];
			pEdgeIds[i] = pEdgeIds[j];
			pEdgeIds[j] = tmp;
			i++; j--;
		}
	}
*/	
	return pEdgeIds;
}

//------------------------------------------------------------------------
//Returns the coordinates of surface point that is the closest to the given plane.
void vtkMAFMuscleDecomposition::FindClosestPoint(
	vtkPolyData* input, const double* origin, const double* normal, double* x)
	//------------------------------------------------------------------------
{
	//the distance between a point Q and plane P*n + d = 0 can be computed as
	//(Q-P)*n assuming that n is normalized or, according to Schneider:
	//Geometric Tools for Computer Graphics. pg. 376, also as Q*n + d
	//(assuming n is normalized). N.B. the latter formula is signed
	double d = -vtkMath::Dot(origin, normal);

	int iMinDist = 0;
	double dblMinDist = DBL_MAX;

	int nPoints = input->GetNumberOfPoints();
	for (int i = 0; i < nPoints; i++)
	{
		const double* pcoords = input->GetPoint(i);
		double dblDist = fabs(normal[0]*pcoords[0] + normal[1]*pcoords[1] +
			normal[2]*pcoords[2] + d);

		if (dblDist < dblMinDist)
		{
			dblMinDist = dblDist;
			iMinDist = i;
		}
	}

	//retrieve the point
	input->GetPoint(iMinDist, x);
}

//------------------------------------------------------------------------
//Adds new points into pContourPoints [in/out] so they form a polygon of at
//least 4 vertices. Returns the new number of points in the list. N.B,
//pContourPoints must be capable to hold at least 4 vertices
int vtkMAFMuscleDecomposition::FixPolygon(VCoord* pContourPoints, int nPoints)
	//------------------------------------------------------------------------
{
	if (nPoints >= 4)
		return nPoints;

	if (nPoints == 1)
	{
		//all four points are the same
		for (int k = 0; k < 3; k++){
			pContourPoints[3][k] = pContourPoints[2][k] = pContourPoints[1][k] = pContourPoints[0][k];
		}
	}
	else if (nPoints == 2)
	{
		//we just create additional two points to have rectangle collapsed into an edge
		for (int k = 0; k < 3; k++)
		{
			pContourPoints[2][k] = pContourPoints[1][k];
			pContourPoints[3][k] = pContourPoints[0][k];
		}
	}
	else
	{
		//just subdivide one edge (the longest one)
		int iMax = 0;
		double dblMax = 0.0;
		for (int k = 0; k < 3; k++)
		{
			double dblLen = vtkMath::Distance2BetweenPoints(
				pContourPoints[k], pContourPoints[(k + 1) % 3]);
			if (dblLen > dblMax)
			{
				dblMax = dblLen;
				iMax = 0;
			}
		}

		//we need to make space for our new point
		for (int i = 2; i > iMax; i--)
		{
			for (int k = 0; k < 3; k++){
				pContourPoints[i + 1][k] = pContourPoints[i][k];
			}
		}

		//add the point
		for (int k = 0; k < 3; k++){
			pContourPoints[iMax + 1][k] = 0.5*(pContourPoints[iMax][k] +
				pContourPoints[(iMax + 2) % 4][k]);
		}
	}

	return 4;
}

//#define _OLD_DR
//------------------------------------------------------------------------
//Divides the rectangle defined by one point and two vectors into
//nPoints edges such that the total square error between lengths of
//contour and rectangle edges is minimized. The routine stores beginning
//points of these rectangle edges into pOutRectPoints.
//N.B. nPoints should be >= 4 and pOutRectPoints MUST BE capable
//to store max(nPoints, 4) coordinates.
void vtkMAFMuscleDecomposition::DivideRectangle(double* origin, double* u, double* v,
	int nPoints, VCoord* pContourPoints, VCoord* pOutRectPoints)
	//------------------------------------------------------------------------
{
	PROFILE_THIS_FUNCTION();

	if (nPoints == 4)
	{
		//we won't do anything (makes a little sense to map 4 to 4)
		for (int i = 0; i < 3; i++)
		{
			pOutRectPoints[0][i] = origin[i];
			pOutRectPoints[1][i] = origin[i] + u[i];
			pOutRectPoints[2][i] = origin[i] + u[i] + v[i];
			pOutRectPoints[3][i] = origin[i] + v[i];
		}

		return;
	}

	//compute contour edges lengths + its perimeter
	double dblCLen = 0.0;
	double* pELens = new double[nPoints];
	for (int i = 0; i < nPoints; i++)
	{
		int i2 = (i + 1) % nPoints;
		pELens[i] = sqrt(vtkMath::Distance2BetweenPoints(pContourPoints[i], pContourPoints[i2]));
		dblCLen += pELens[i];
	}

	//compute sides and sizes of rectangle sides
	double Verts[4][3];
	double RSizes[4];
	for (int i = 0; i < 3; i++)
	{
		Verts[0][i] = u[i];
		Verts[1][i] = v[i];
	}

	RSizes[2] = RSizes[0] = vtkMath::Normalize(Verts[0]);
	RSizes[3] = RSizes[1] = vtkMath::Normalize(Verts[1]);
	for (int i = 0; i < 3; i++)
	{
		Verts[2][i] = -Verts[0][i];
		Verts[3][i] = -Verts[1][i];
	}

	//and the perimeter of rectangle
	double dblRLen = 2*(RSizes[0] + RSizes[1]);  

#ifndef _OLD_DR
	//parameterize contour points
	double* pCPars = new double[nPoints + 1];
	double dblCLen_1 = 1 / dblCLen, dblCLenS = 0.0;
	for (int i = 0; i < nPoints; i++)
	{
		double t = dblCLenS * dblCLen_1;	//current t
		dblCLenS += pELens[i];
		pCPars[i] = t;
	}

	pCPars[nPoints] = 1.0;				//to avoid round-off errors

	//parameterize rectangle points
	double RPars[5];
	double dblRLen_1 = 1 / dblRLen, dblRLenS = 0.0;
	for (int i = 0; i < 4; i++)
	{
		double t = dblRLenS * dblRLen_1;	//current t
		dblRLenS += RSizes[i];
		RPars[i] = t;
	}

	RPars[4] = 1.0;										//to avoid round-off errors

	//now perform mapping
	int nBestDivision[4];							//gives where the contour chain should be split (the first index of next segment)
	nBestDivision[0] = nBestDivision[1] = 0;

	int iCurRLim = 1;		
	for (int i = 1; i < nPoints; i++)
	{
		double tr_err = pCPars[i] - RPars[iCurRLim];
		if (tr_err > 0.0) //sign test
		{
			//the contour edge exceeds the limit 
			if (i - 1 == nBestDivision[iCurRLim - 1])
			{
				//but it is the very first edge of this segment, so we need to accept it
				nBestDivision[iCurRLim - 1] = nBestDivision[iCurRLim] = i;
			}			
			else
			{
				//check what would minimize the error
				double tl_err = RPars[iCurRLim] - pCPars[i - 1];	//i cannot be 0 here since it would be already captured by the first branch
				if (tl_err > tr_err){
					nBestDivision[iCurRLim - 1] = nBestDivision[iCurRLim] = i;
				}
				else{
					nBestDivision[iCurRLim - 1] = nBestDivision[iCurRLim] = i - 1;				
				}
			}

			if (++iCurRLim == 4)
				break; //no longer need to continue
		}
	}

	while (iCurRLim < 4)
	{
		nBestDivision[iCurRLim - 1] = nPoints - (4 - iCurRLim);	//
		iCurRLim++;
	}

	nBestDivision[3] = nPoints;

	_ASSERT(nBestDivision[0] < nPoints && nBestDivision[1] < nPoints && nBestDivision[2] < nPoints);
	_ASSERT(nBestDivision[0] < nBestDivision[1] && nBestDivision[1] < nBestDivision[2] && nBestDivision[2] < nBestDivision[3]);

	//we have detected best division strategy, now do it
	for (int i = 0; i < 3; i++)
	{
		pOutRectPoints[0][i] = origin[i];
		pOutRectPoints[nBestDivision[0]][i] = origin[i] + u[i];
		pOutRectPoints[nBestDivision[1]][i] = origin[i] + u[i] + v[i];
		pOutRectPoints[nBestDivision[2]][i] = origin[i] + v[i];
	}

	int nIndex = 1;
	int iStartPos = 0;	
	for (int i = 0; i < 4; i++)
	{
		int iEndPos = nBestDivision[i];
		double dblCoef = RSizes[i]/(dblCLen*(pCPars[iEndPos] - pCPars[iStartPos]));

		for (int j = iStartPos; j < iEndPos - 1; j++, nIndex++)
		{
			double dblFact = dblCoef*pELens[j];
			for (int k = 0; k < 3; k++)
			{
				pOutRectPoints[nIndex][k] =
					pOutRectPoints[nIndex - 1][k] + Verts[i][k]*dblFact;
			}
		}

		iStartPos = iEndPos;
		nIndex++;
	}

	_ASSERT(nIndex == nPoints + 1);

	delete[] pCPars;

#else
	double dblCoef = dblRLen / dblCLen;	//scaling 
	double dblBestERSizes[4] = {0.0, 0.0, 0.0, 0.0};
	double dblBestScore = DBL_MAX;
	int nBestDivision[4] = {0, 0, 0, nPoints};

	double ERSizes[4];
	ERSizes[0] = pELens[0];
	ERSizes[1] = pELens[1];
	ERSizes[2] = pELens[2];
	ERSizes[3] = dblCLen - (ERSizes[0] + ERSizes[1] + ERSizes[2]);

	//check all possible cases
	for (int i = 1; i < nPoints - 2; i++)
	{
		for (int j = i + 1; j < nPoints - 1; j++)
		{
			for (int k = j + 1; k < nPoints; k++)
			{
				//ERSizes contains the undeformed sizes
				double dblScore = 0.0;
				for (int m = 0; m < 4; m++)
				{
					double dblDiff = RSizes[m] - ERSizes[m]*dblCoef;
					dblScore += dblDiff*dblDiff;
				}

				if (dblScore < dblBestScore)
				{
					//we found a better solution
					dblBestScore = dblScore;
					for (int m = 0; m < 4; m++){
						dblBestERSizes[m] = ERSizes[m];
					}

					nBestDivision[0] = i;
					nBestDivision[1] = j;
					nBestDivision[2] = k;
				} //end  if (dblScore < dblBestScore)

				ERSizes[3] -= pELens[k]; //in the last round, this will be 0
				ERSizes[2] += pELens[k]; //in the last round this will be all
			} //end for k

			ERSizes[1] += pELens[j];
			ERSizes[2] -= pELens[j];
			ERSizes[3] = ERSizes[2] - pELens[j + 1];
			ERSizes[2] = pELens[j + 1];
			//in the last round, 0 is of unknown size, 1 is large, 2 contains last edge and 3 is zero
		}

		ERSizes[0] += pELens[i];
		ERSizes[1] = pELens[i + 1];
		ERSizes[2] = pELens[i + 2];
		ERSizes[3] = dblCLen - (ERSizes[0] + ERSizes[1] + ERSizes[2]);
	}

	//we have detected best division strategy, now do it
	for (int i = 0; i < 3; i++)
	{
		pOutRectPoints[0][i] = origin[i];
		pOutRectPoints[nBestDivision[0]][i] = origin[i] + u[i];
		pOutRectPoints[nBestDivision[1]][i] = origin[i] + u[i] + v[i];
		pOutRectPoints[nBestDivision[2]][i] = origin[i] + v[i];
	}

	int nIndex = 1;
	int iStartPos = 0;
	for (int i = 0; i < 4; i++)
	{
		int iEndPos = nBestDivision[i];
		double dblCoef = RSizes[i] / dblBestERSizes[i];

		for (int j = iStartPos; j < iEndPos - 1; j++, nIndex++)
		{
			double dblFact = dblCoef*pELens[j];
			for (int k = 0; k < 3; k++)
			{
				pOutRectPoints[nIndex][k] =
					pOutRectPoints[nIndex - 1][k] + Verts[i][k]*dblFact;
			}
		}

		iStartPos = iEndPos;
		nIndex++;
	}
#endif
	delete[] pELens;
}

//------------------------------------------------------------------------
//Computes new locations of given points (pPoints)lying inside the
//template polygon (pPolyTemplate) within the other polygon
//(pPolyTarget). Both polygons have the same number of edges. New
//coordinates are stored in pPoints buffer.
void vtkMAFMuscleDecomposition::MapPoints(VCoord* pPoints, int nPoints,
	VCoord* pPolyTemplate, VCoord* pPolyTarget, int nPolyPoints)
	//------------------------------------------------------------------------
{
	PROFILE_THIS_FUNCTION();

	const static double eps_zero = 1e-8;

	_RPT0(_CRT_WARN, "====== TEMPLATE POLYGON ======\n");
	for (int i = 0; i < nPolyPoints; i++)
	{
		_RPT3(_CRT_WARN, "%.2f,%.2f,%.2f\n",
			pPolyTemplate[i][0],  pPolyTemplate[i][1], pPolyTemplate[i][2]);
	}

	_RPT0(_CRT_WARN, "====== TARGET POLYGON ======\n");
	for (int i = 0; i < nPolyPoints; i++)
	{
		_RPT3(_CRT_WARN, "%.2f,%.2f,%.2f\n",
			pPolyTarget[i][0],  pPolyTarget[i][1], pPolyTarget[i][2]);
	}

	_RPT0(_CRT_WARN, "====== POINTS TO BE MAPPED ======\n");
	for (int i = 0; i < nPoints; i++)
	{
		_RPT3(_CRT_WARN, "%.2f,%.2f,%.2f\n",
			pPoints[i][0],  pPoints[i][1], pPoints[i][2]);
	}

	//#pragma omp parallel firstprivate(nPolyPoints, pPoints) --- N.B. overhead to launch a thread is too big to make this work
	{
		VCoord* s_i = new VCoord[nPolyPoints];   //hold vectors vi, v (vi = points on boundary, v = inner point to map)
		double* r_i = new double[nPolyPoints];   //lengths of s_i vectors
		double* D_i = new double[nPolyPoints];   //dot products of s_i vectors
		double* A_i = new double[nPolyPoints];   //area of i-th triangle

		//for every inner point
		//#pragma omp for
		for (int j = 0; j < nPoints; j++)
		{
			//compute mean value coordinates of the current point (v) within the source polygon
			//see: K. Hormann, MS. Floater: Mean Value Coordinates for Arbitrary Planar Polygons.
			//http://folk.uio.no/michaelf/papers/barycentric.pdf

			//first, compute s_i, r_i
			for (int i = 0; i < nPolyPoints; i++)
			{
				for (int k = 0; k < 3; k++){
					s_i[i][k] = pPolyTemplate[i][k] - pPoints[j][k];
				}

				r_i[i] = vtkMath::Norm(s_i[i]);
				if (r_i[i] <= eps_zero) //if r_i is zero
				{
					//v = vi => this will be mapped to target point directly
					for (int k = 0; k < 3; k++){
						pPoints[j][k] = pPolyTarget[i][k];
					}

					goto next_point;  //and proceed with the next point
				}
			}

			//next, compute D_i and A_i
			for (int i = 0; i < nPolyPoints; i++)
			{
				int i_plus = (i + 1) % nPolyPoints;
				D_i[i] = vtkMath::Dot(s_i[i], s_i[i_plus]);
				A_i[i] = fabs(0.5*(
					(s_i[i][1] - s_i[i][0])*(s_i[i_plus][2] - s_i[i_plus][0]) -
					(s_i[i][2] - s_i[i][0])*(s_i[i_plus][1] - s_i[i_plus][0])
					));   //it may happen that area is negative; this is caused by
				//a) points do not lie on a common plane - see the tolerance in ExecuteData
				//b) numeric reasons

				//if s_i and s_i+ does not form a triangle (colinear)
				//v lies on line supported by the edge vi, vi+1
				//if, moreover, D_i is negative, the angle is > 90 degrees
				//which logically implies that the point lies on the edge
				if (A_i[i] <= eps_zero && D_i[i] < 0.0)
				{
					//v lies on an edge of polygon
					double dblDen = r_i[i] + r_i[i_plus];
					for (int k = 0; k < 3; k++){
						pPoints[j][k] = (r_i[i]*pPolyTarget[i][k] + r_i[i_plus]*pPolyTarget[i_plus][k]) / dblDen;
					}

					goto next_point;
				}
			} //end for i

			//point is in a general position, we will blend it
			pPoints[j][0] = pPoints[j][1] = pPoints[j][2] = 0.0;

			double dblWTotal = 0.0;
			for (int i = 0; i < nPolyPoints; i++)
			{
				int i_plus = (i + 1) % nPolyPoints;
				int i_minus = (i + nPolyPoints - 1) % nPolyPoints;

				double w;
				if (fabs(A_i[i_minus]) <= eps_zero)
					w = 0.0;
				else
					w = (r_i[i_minus] - D_i[i_minus]/r_i[i]) / A_i[i_minus];

				if (fabs(A_i[i]) > eps_zero)
					w += (r_i[i_plus] - D_i[i]/r_i[i]) / A_i[i];

				for (int k = 0; k < 3; k++){
					pPoints[j][k] += w*pPolyTarget[i][k];
				}

				dblWTotal += w;
			}

			for (int k = 0; k < 3; k++){
				pPoints[j][k] /= dblWTotal;
			}

next_point:
			;
		} //end for j (every point)

		delete[] A_i;
		delete[] D_i;
		delete[] r_i;
		delete[] s_i;
	}

	_RPT0(_CRT_WARN, "====== MAPPED POINTS ======\n");
	for (int i = 0; i < nPoints; i++)
	{
		_RPT3(_CRT_WARN, "%.2f,%.2f,%.2f\n",
			pPoints[i][0],  pPoints[i][1], pPoints[i][2]);
	}
}

//------------------------------------------------------------------------
//Smooth the fiber defined by the given points.
void vtkMAFMuscleDecomposition::SmoothFiber(VCoord* pPoints, int nPoints)
	//------------------------------------------------------------------------
{
	if (nPoints <= 3)
		return; //cannot smooth

	double dblTotalW = this->SmoothFactor + 2;
	for (int i = 0; i < SmoothSteps; i++)
	{
		double x[3];  //buffer for one point
		for (int k = 0; k < 3; k++){
			x[k] = pPoints[0][k];
		}

		//for every inner point Pj of the curve, we set its coordinates into:
		//Pj' = 1/6*(Pj-1 + 4*Pj + Pj+1)  -- see Coons curve
		for (int j = 1; j < nPoints - 1; j++)
		{
			//j+1 is at iteration i-1
			//j-1 is at iteration i-1 and its iteration i is in x and should be saved now
			//j is at iteration i-1 and its iteration should be stored in x
			for (int k = 0; k < 3; k++)
			{
				double dblTmp = x[k];
				x[k] = (pPoints[j - 1][k] + this->SmoothFactor*pPoints[j][k] +
					pPoints[j + 1][k]) / dblTotalW;
				pPoints[j - 1][k] = dblTmp;
			}
		} //end for points

		//and alter the last point
		for (int k = 0; k < 3; k++) {
			pPoints[nPoints - 2][k] = x[k];
		}
	} //end for SmoothSteps
}

//------------------------------------------------------------------------
//Smooth the fiber defined by the given points - is used as "static", i.e. does not use the parametres set in this vtkMADMuscleDecomposition.
void vtkMAFMuscleDecomposition::SmoothFiber(VCoord* pPoints, int nPoints, double smoothFactor, int smoothSteps)
	//------------------------------------------------------------------------
{
	if (nPoints <= 3)
		return; //cannot smooth

	double dblTotalW = smoothFactor + 2;
	for (int i = 0; i < smoothSteps; i++)
	{ 
		double x[3];  //buffer for one point
		for (int k = 0; k < 3; k++){
			x[k] = pPoints[0][k];
		}

		//for every inner point Pj of the curve, we set its coordinates into:
		//Pj' = 1/6*(Pj-1 + 4*Pj + Pj+1)  -- see Coons curve
		for (int j = 1; j < nPoints - 1; j++)
		{
			//j+1 is at iteration i-1
			//j-1 is at iteration i-1 and its iteration i is in x and should be saved now
			//j is at iteration i-1 and its iteration should be stored in x
			for (int k = 0; k < 3; k++)
			{
				double dblTmp = x[k];
				x[k] = (pPoints[j - 1][k] + smoothFactor*pPoints[j][k] + 
					pPoints[j + 1][k]) / dblTotalW;
				pPoints[j - 1][k] = dblTmp;
			}
		} //end for points
		//and alter the last point
		for (int k = 0; k < 3; k++) {
			pPoints[nPoints - 2][k] = x[k];
		}
	} //end for SmoothSteps
}


//------------------------------------------------------------------------
//By default, UpdateInformation calls this method to copy information
//unmodified from the input to the output.
/*virtual*/void vtkMAFMuscleDecomposition::ExecuteInformation()
	//------------------------------------------------------------------------
{
	//check input
	vtkPolyData* input = GetInput();
	if (input == NULL)
	{
		vtkErrorMacro(<< "Invalid input for vtkMAFPolyDataDeformation.");
		return;   //we have no input
	}

	//check output
	vtkPolyData* output = GetOutput();
	if (output == NULL)
		SetOutput(vtkPolyData::New());

	if (this->FibersTemplate == NULL)
		this->FibersTemplate = vtkMAFParallelMuscleFibers::New();  //default is a parallel muscle

	//copy input to output
	Superclass::ExecuteInformation();
}

//------------------------------------------------------------------------
//This method is the one that should be used by subclasses, right now the
//default implementation is to call the backwards compatibility method
/*virtual*/void vtkMAFMuscleDecomposition::ExecuteData(vtkDataObject *output)
{
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#pragma message ("WARNING: PUBLICATION_TEST_GENERATED_DATA_OVERRIDE is ON - DO NOT COMMIT THIS!!!")
	this->AdvancedKukacka = 1;
	DataType testDataType = Ellipsoid;
	GenerateInputData(testDataType);	
#endif

#ifdef ADV_KUKACKA_TEST	
//	this->NumberOfFibres = 64;
//	this->Resolution = 30;
	this->AdvancedKukacka = 1;

	const int nMaxExpers = 1;
	for (int iExpNum = 0; iExpNum < nMaxExpers; iExpNum++) 
	{
#endif
	PROFILE_TIMER_START(_T(__FUNCTION__));

	PROFILE_TIMER_START(REST-POSE PREPROCESSING);
	PROFILE_TIMER_START(INPUT VALIDATION);
#pragma region Input Checks
	//check whether output is valid
	vtkPolyData* input = GetInput();
	if (input == NULL || input->GetNumberOfPoints() == 0)
		return;

	vtkPolyData* inputTemplate = GetInputTemplate();
	if (inputTemplate == NULL || inputTemplate->GetNumberOfPoints() == 0)
		inputTemplate = input;

	vtkPolyData* pPoly = vtkPolyData::SafeDownCast(output);
	if (pPoly == NULL)
	{
		vtkWarningMacro(<< "Invalid output for vtkMAFMuscleDecomposition.");
		return;   //we have no valid output
	}

	if (NumberOfFibres <= 0 || Resolution <= 0) {
		vtkWarningMacro(<< "NumberOfFibres or Resolution is invalid.");
		return;
	}

	//BES: 31.5.2012 - Uniform sampling requires square root
	if (this->UniformSampling != 0)
	{
		int nSqrt = (int)sqrt((double)this->NumberOfFibres);
		nSqrt *= nSqrt;

		if (this->NumberOfFibres != nSqrt) {
			vtkWarningMacro(<< "NumberOfFibres should be equal to N*N when UniformSampling is enabled.");
			this->NumberOfFibres = nSqrt;
		}
	}
#pragma endregion Input Checks
	PROFILE_TIMER_STOP(INPUT VALIDATION);

	vtkMAFVisualDebugger* vd = (this->DebugMode == dbgNone ? NULL : vtkMAFVisualDebugger::New());

	PROFILE_TIMER_START(ATTACHMENT AREAS PROCESSING);
#pragma region Attachment Areas Processing
	//first, project OI points onto the surface of muscle, so we could filter fibers
	//we will need a cell locator
	vtkCellLocator *cellLocator = NULL, *cellLocatorORI = NULL, *cellLocatorINS = NULL;
	vtkPoints *oriAreaLM = NULL, *insAreaLM = NULL;
	vtkPolyData *oriAreaSurf = NULL, *insAreaSurf = NULL;

	//create an octree based locator for cells of the input mesh
	cellLocator = vtkCellLocator::New();
	cellLocator->SetDataSet(inputTemplate);
	cellLocator->Update();

#ifdef ADV_KUKACKA	//KUKACKA method requires processing of attachment areas
	vtkMAFSmartPointer< vtkPolyData > oriContour, insContour;

	vtkSmartPointer< vtkPolyData > muscleCut = inputTemplate->NewInstance();
	muscleCut->UnRegister(this);	//to avoid memory leaks
	if (this->AdvancedKukacka != 0 || this->AdvancedSlicing != 0)
#else
	if (this->AdvancedSlicing != 0)
#endif
	{		
		//project attachment area landmarks on the muscle surface
		oriAreaLM = vtkPoints::New(); insAreaLM = vtkPoints::New();
		ProjectAttachmentArea(this->OriginArea, cellLocator, oriAreaLM);
		ProjectAttachmentArea(this->InsertionArea, cellLocator, insAreaLM);

		//detect attachment area surface (on the muscle surface)
		oriAreaSurf = vtkPolyData::New(); insAreaSurf = vtkPolyData::New();
#ifdef ADV_KUKACKA
		if (this->AdvancedKukacka == 0)
		{
#endif
			GetAttachmentAreaSurface(oriAreaLM, inputTemplate, oriAreaSurf);
			GetAttachmentAreaSurface(insAreaLM, inputTemplate, insAreaSurf);
#ifdef ADV_KUKACKA
		}
		else
		{
			GetAttachmentAreaSurface(oriAreaLM, inputTemplate, oriAreaSurf, muscleCut.GetPointer(), oriContour);
			GetAttachmentAreaSurface(insAreaLM, muscleCut.GetPointer(), insAreaSurf, muscleCut.GetPointer(), insContour);

			ComputeHarmonicScalarFunction(muscleCut.GetPointer(), oriContour, HARMFUNC_MIN, insContour, HARMFUNC_MAX);

			if ((this->DebugMode & dbgVisualizeHarmonicField) == dbgVisualizeHarmonicField)
				DebugVisualizeHarmonicWeights(muscleCut.GetPointer(), this->OriginArea, this->InsertionArea, oriAreaLM, insAreaLM);
		}
#endif

		//create an octree based locator for cells of the both areas
		cellLocatorORI = vtkCellLocator::New();
		cellLocatorORI->SetDataSet(oriAreaSurf);
		cellLocatorORI->Update();

		cellLocatorINS = vtkCellLocator::New();
		cellLocatorINS->SetDataSet(insAreaSurf);
		cellLocatorINS->Update();
	} //end if Advanced mode
#pragma endregion
	PROFILE_TIMER_STOP(ATTACHMENT AREAS PROCESSING);

	PROFILE_TIMER_START(CUBE CONSTRUCTION);
#pragma region Template Cube and Target Cube Construction
	//we now will create fiber points in the template cube (1x1x1)
	int nFVerts = (Resolution + 1)*NumberOfFibres;
	VCoord* pFVerts = new VCoord[nFVerts];

	double* r_s = new double[2*NumberOfFibres];
	if (this->UniformSampling == 0) {
		CreateSobolPoints(NumberOfFibres, r_s);
	}
	else {
		CreateUniformPoints(NumberOfFibres, r_s);
	}

	int nIndex = 0;
	double delta_t = 1.0 / Resolution;
	for (int i = 0; i < NumberOfFibres; i++)
	{
		double r = r_s[2*i];
		double s = r_s[2*i + 1];
		for (int j = 0; j <= Resolution; j++, nIndex++)
		{
			double x[3];
			FibersTemplate->GetPoint(r, s, j*delta_t, x);

			//fibers templates use different coordinate system (they use left-handed)
			//whilst the rest of our application uses right-handed
			pFVerts[nIndex][0] = x[0];
			pFVerts[nIndex][1] = x[2];
			pFVerts[nIndex][2] = x[1];
		}
	}

	delete[] r_s;   //no longer needed

	//we need to find minimal oriented box that fits the input point data
	//so that all points are inside of this box (or on its boundary)
	//and the total squared distance of template origin points from
	//the input mesh origin points and the total squared distance of
	//template insertion points from the input mesh origin points
	//are minimized
	LOCAL_FRAME lf, lfNorm;
	ComputeFittingOB(inputTemplate->GetPoints(), lf);

	//we are going to transform the template cube (1x1x1) into the target cube using
	//the local frame lf and by "projection" into the muscle volume, the
	//projection will be done in the plane perpendicular to the longest
	//direction => get this direction
	lfNorm = lf;
	int iPlane = 0;
	if (AdvancedSlicing != 0)
	{
		for (int i = 0; i < 3; i++) {
			vtkMath::Normalize(lfNorm.uvw[i]);	//normalization
		}

		iPlane = 2;	//axis 2 should be the one between both attachment areas
	}
	else
	{
		//BES: 11.10.2011 - I see no reason to detect the longest axis even for original slicing
		//but I keep it intact until an evidence that this is really unwanted thing is found
		double dblAxisLength = vtkMath::Normalize(lfNorm.uvw[0]);
		for (int i = 1; i < 3; i++)
		{
			double dblTmp = vtkMath::Normalize(lfNorm.uvw[i]);
			if (dblTmp > dblAxisLength)
			{
				dblAxisLength = dblTmp;
				iPlane = i;
			}
		}
	}

	int iPlane2 = (iPlane + 2) % 3; //fiber templates use different coordinate system
	int iPlane3 = (iPlane + 1) % 3;

	//lfNorm now contains the local frame with normalized axis
	//and iPlane is the index of longest axis
#pragma endregion Template Cube and Target Cube Construction
	PROFILE_TIMER_STOP(CUBE CONSTRUCTION);

	PROFILE_TIMER_START(FIBRES MAPPING);
#pragma region Computation of Target Fibres by Projection
	if ((this->DebugMode & dbgDoNotProjectFibres) == dbgDoNotProjectFibres)
	{
		//just transformation of point x (onto target cube)
		for (int i = 0; i < nFVerts; i++)
		{
			double x[3];
			for (int k = 0; k < 3; k++){
				x[k] = lf.O[k] + lf.uvw[0][k]*pFVerts[i][0] +
					lf.uvw[1][k]*pFVerts[i][1] + lf.uvw[2][k]*pFVerts[i][2];
			}

			for (int k = 0; k < 3; k++){
				pFVerts[i][k] = x[k];
			}
		}//end for i (points)
	}
	else
	{
		//we need to perform projection
		//these are for debug visualization
		vtkPoints* pMuscleContoursPoints = NULL;
		vtkPoints* pTargetContoursPoints = NULL;
		vtkPoints* pMappedFibresPoints = NULL;
		vtkCellArray* pMuscleContoursCells = NULL;
		vtkCellArray* pTargetContoursCells = NULL;
		vtkCellArray* pMappedFibresCells = NULL;

		vtkPolyData* pMuscleContours = NULL;
		vtkPolyData* pTargetContours = NULL;
		vtkPolyData* pMappedFibres = NULL;
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
		vtkPoints* pMuscleContoursFittingPlanePoints = NULL;
		vtkCellArray* pMuscleContoursFittingPlaneCells = NULL;
		vtkPolyData* pMuscleContourFittingPlane = NULL;
#endif

		if ((this->DebugMode & (dbgVisualizeSlicing | dbgVisualizeSlicingResult)) != 0)
		{
			pMuscleContoursPoints = vtkPoints::New();
			pTargetContoursPoints = vtkPoints::New();
			pMappedFibresPoints = vtkPoints::New();
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
			pMuscleContoursFittingPlanePoints = vtkPoints::New();
#endif
			pMuscleContoursCells = vtkCellArray::New();
			pTargetContoursCells = vtkCellArray::New();
			pMappedFibresCells = vtkCellArray::New();
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
			pMuscleContoursFittingPlaneCells = vtkCellArray::New();
#endif

			pMuscleContours = vtkPolyData::New();
			pTargetContours = vtkPolyData::New();
			pMappedFibres = vtkPolyData::New();
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
			pMuscleContourFittingPlane = vtkPolyData::New();
#endif

			pMuscleContours->SetPoints(pMuscleContoursPoints);
			pMuscleContours->SetLines(pMuscleContoursCells);
			pTargetContours->SetPoints(pTargetContoursPoints);
			pTargetContours->SetLines(pTargetContoursCells);
			pMappedFibres->SetPoints(pMappedFibresPoints);
			pMappedFibres->SetLines(pMappedFibresCells);
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
			pMuscleContourFittingPlane->SetPoints(pMuscleContoursFittingPlanePoints);
			pMuscleContourFittingPlane->SetPolys(pMuscleContoursFittingPlaneCells);
#endif
		}

		int* pOrder = NULL;

		vtkPlane* cutPlane = vtkPlane::New();
		vtkCutter* cutter = vtkCutter::New();

#ifdef ADV_KUKACKA
		vtkContourFilter* cutterKukacka = vtkContourFilter::New();
		if (this->AdvancedKukacka == 0)
		{
#endif
			//sort points according to their coordinate in this axis, which will help
			//us to reduce the number of required cuts (slow) by exploiting the
			//coherence of the data
			pOrder = SortPoints(pFVerts, nFVerts, iPlane);

			//construct mesh cutter    
			cutPlane->SetNormal(lfNorm.uvw[iPlane]);

			cutter->SetCutFunction(cutPlane);
			cutter->SetInput(inputTemplate);
#ifdef ADV_KUKACKA
		}
		else
		{
			//IDEA: Introduce a new attribute for each vertex, this vertex is the value of harmonic function.
			//Set the value to 0 for each vertex.	Insert contours of attachment areas into
			//the surface mesh of muscle and set the value of harmonic function at vertices
			//of origin contour to 1 and at vertices of insertion contour to 10. Calculate
			//harmonic function (Laplacian) that smoothly transforms values from one contour
			//to the other one. Now, get isolines instead of planar CUT.

			//First, create order of fibres vertices ([0, N, 2*N, ...][1, N + 1, 2*N +1 ...] .. [N-1,2*N-1, ...])
			pOrder = new int[nFVerts];

			nIndex = 0;
			for (int j = 0; j <= Resolution; j++)
			{
				for (int i = 0; i < NumberOfFibres; i++, nIndex++)
				{
					pOrder[nIndex] = i*(Resolution + 1) + j;
				}
			}

			//and initialize contour creator		
			cutterKukacka->SetNumberOfContours(1);
			cutterKukacka->SetInput(muscleCut.GetPointer());
		}
#endif

		std::vector< CONTOUR_MAPPING_PARAMETERIZATION > ParContours;

		VCoord* pPrevTargetPolyBuf = NULL;					//here is stored the previous buffer
		VCoord* pPrevTargetPoly = NULL;
		int iStartPos = 0;

		vtkMAFSmartPointer< vtkGenericCell > cell;
		while (iStartPos < nFVerts)
		{
			PROFILE_TIMER_START(FIBRES MAPPING: CUTTING SURFACE);

			//tolerance - points within this tolerance uses the same cutting plane
			const static double dblTolerance = 0.005;

			double dblAvgCoord = pFVerts[pOrder[iStartPos]][iPlane];

			int iEndPos;
#ifdef ADV_KUKACKA
			if (this->AdvancedKukacka == 0)
			{
#endif
				iEndPos = iStartPos + 1;  //exclusive
				while (iEndPos < nFVerts &&
					pFVerts[pOrder[iStartPos]][iPlane] + dblTolerance > pFVerts[pOrder[iEndPos]][iPlane])
				{
					dblAvgCoord +=  pFVerts[pOrder[iEndPos]][iPlane];
					iEndPos++;
				}
#ifdef ADV_KUKACKA
			}
			else
			{
				iEndPos = iStartPos;  //exclusive but our for will do this for us
				for (int i = 0; i < NumberOfFibres; i++)
				{
					dblAvgCoord +=  pFVerts[pOrder[iEndPos]][iPlane];
					iEndPos++;
				}
			}
#endif

			dblAvgCoord /= (iEndPos - iStartPos);

			//all points at iStartPos - iEndPos-1 will be projected using the same
			//plane (constructed in their average); find the origin of the
			//rectangle created by the intersection of target cube and plane with
			//normal lf.uvw[iPlane] going through transformed dblAvgCoord
			double origin[3];
			for (int j = 0; j < 3; j++){
				origin[j] = lf.O[j] + lf.uvw[iPlane][j]*dblAvgCoord;
			}

			vtkPolyData* contour = NULL;

#ifdef ADV_KUKACKA
			if (this->AdvancedKukacka == 0)
			{
#endif
				cutPlane->SetOrigin(origin);
				cutter->Update();   //cut the mesh by the plane

				contour = vtkPolyData::SafeDownCast(cutter->GetOutput());
#ifdef ADV_KUKACKA
			}
			else
			{
				int iCurIso = iStartPos / NumberOfFibres;
				//if (oriContour->GetNumberOfPoints() > insContour->GetNumberOfPoints())
				//	iCurIso = this->Resolution - iCurIso;	//let us process from smaller area to larger one

				if (iCurIso == 0)
					//contour = oriContour;
					cutterKukacka->SetValue(0, HARMFUNC_MIN + 1e-9); //eps
				else if (iCurIso == this->Resolution)
					//contour = insContour;
					cutterKukacka->SetValue(0, HARMFUNC_MAX - 1e-9);
				else
				{
					double t = iCurIso / (double)Resolution;
					cutterKukacka->SetValue(0, HARMFUNC_MIN * (1 - t) + HARMFUNC_MAX * t);				
				}

				cutterKukacka->Update(); //find the iso-line => we should have a contour

				contour = vtkPolyData::SafeDownCast(cutterKukacka->GetOutput());
			}
#endif
			PROFILE_TIMER_STOP(FIBRES MAPPING: CUTTING SURFACE);
			PROFILE_TIMER_START(FIBRES MAPPING: PARAMETERIZATION);

			//get the current contour			
			CONTOUR_MAPPING_PARAMETERIZATION curcnt;	
			memset(&curcnt, 0, sizeof(curcnt));
			curcnt.iStartPos = iStartPos;
			curcnt.iEndPos = iEndPos;


			int nPoints = curcnt.nPoints = contour->GetNumberOfPoints();

			//first, get the contour coordinates so that twists between adjacent contours are minimized
			if (nPoints < 2)
			{
				//due to some numeric problems, there is no intersection
				//find the closest point to the given plane and this will be our intersection
				if (nPoints == 0)
				{
					curcnt.TemplateMuscleContour = new VCoord[nPoints = curcnt.nPoints = 1];
					FindClosestPoint(inputTemplate, origin, lfNorm.uvw[iPlane], curcnt.TemplateMuscleContour[0]);
				}
				else
				{
					curcnt.TemplateMuscleContour = new VCoord[nPoints];
					contour->GetPoint(0, curcnt.TemplateMuscleContour[0]);
				}

				curcnt.BoxContour = new VCoord[nPoints];
				for (int k = 0; k < 3; k++) {
					curcnt.BoxContour[0][k] = origin[k];
				}
			}
			else
			{				
				//find the point on the contour that is the closest to our origin
				//from this point we will do our mapping
				VCoord* pTargetPolyBuf = new VCoord[2*(nPoints + 2)];  //twice because of mapping, +2 because of fix - see later
				VCoord* pTargetPoly = pTargetPolyBuf;
/*
				//unfortunately, points on the contour are not ordered
				//so we will need to do this first
				//BES: 11.10.2013 - and orient it consistently with the template
				double oriNormal[3];
				vtkMath::Cross(lf.uvw[iPlane2], lf.uvw[iPlane3], oriNormal);*/

				int* pEdgesOrder = GetSortedEdges(contour); //, lf.uvw[iPlane], vtkMath::Dot(oriNormal, lf.uvw[iPlane]) >= 0.0);				

				int iMapStartPos = 0;
				double* pFixedPoint = origin;

#ifdef ADV_KUKACKA
				if (this->AdvancedKukacka != 0 && pPrevTargetPoly != NULL)					//BES: 1.11.2011 - the original LHDL morphing method seems to twist fibres less than the new one
					pFixedPoint = pPrevTargetPoly[0];				
#endif
				
				//if the previous contour does not exist, the start of this contour is at the point closest to the
				//origin (typically, one corner of the template polygon), otherwise,  both contour are aligned in such a manner that
				//their start vertices are as close as possible
				double dblMinDist = DBL_MAX;
				for (int j = 0; j < nPoints; j++)
				{
					contour->GetPoint(pEdgesOrder[2*j], pTargetPoly[j]);
					double dblDist = vtkMath::Distance2BetweenPoints(pFixedPoint, pTargetPoly[j]);
					if (dblDist < dblMinDist)
					{
						iMapStartPos = j;
						dblMinDist = dblDist;
					}
				}

				//we need to shift pTargetPoly so, the closest point is the first one
				if (iMapStartPos != 0)
				{
					memcpy(pTargetPoly + nPoints, pTargetPoly, iMapStartPos*sizeof(VCoord));
					pTargetPoly += iMapStartPos;
				}

				//make sure that the muscle contour (targetpoly) has at least 4 points
				nPoints = FixPolygon(pTargetPoly, nPoints);
				delete[] pEdgesOrder; //no longer needed
				
				//store the template muscle contour
				curcnt.TemplateMuscleContour = new VCoord[nPoints];
				memcpy(curcnt.TemplateMuscleContour, pTargetPoly, nPoints*sizeof(pTargetPoly[0]));

				//divide the rectangle into nPoints segments, so we have the template polygon
				curcnt.BoxContour = new VCoord[nPoints < 4 ?  4 : nPoints];		
				DivideRectangle(origin, lf.uvw[iPlane2], lf.uvw[iPlane3],
					nPoints, pTargetPoly, curcnt.BoxContour);
				
				//clean-up
				delete[] pPrevTargetPolyBuf;

				pPrevTargetPolyBuf = pTargetPolyBuf;
				pPrevTargetPoly = pTargetPoly;
			} //end else [nPoints > 1]

			//curcnt contains global coordinates of template muscle contours
			//let us make them relative to inputTemplate
			//RELEASE NOTE: it is assumed that inputTemplate is a triangular mesh!
			curcnt.TemplateMuscleContourTriIds = new int[nPoints];
			for (int j = 0; j < nPoints; j++)
			{
				int subId;
				double x_cl[3], par_cor[3], dist2;
				cellLocator->FindClosestPoint(curcnt.TemplateMuscleContour[j], x_cl, cell, curcnt.TemplateMuscleContourTriIds[j], subId, dist2);
				cell->EvaluatePosition(x_cl, NULL, subId, par_cor, dist2, curcnt.TemplateMuscleContour[j]);	
			}

			ParContours.push_back(curcnt);
			PROFILE_TIMER_STOP(FIBRES MAPPING: PARAMETERIZATION);	

			iStartPos = iEndPos;
		} //end while

		delete[] pPrevTargetPolyBuf;

		input->BuildCells();	//make sure the cells are built
		PROFILE_TIMER_STOP(REST-POSE PREPROCESSING);

		PROFILE_TIMER_START(CURRENT-POSE PROCESSING);
		
		VCoord* pTrPoints = new VCoord[nFVerts];		//to avoid allocation and deallocation inside the loop
		int nContours = (int)ParContours.size();
		for (int iCurCnt = 0; iCurCnt < nContours; iCurCnt++)
		{
			PROFILE_TIMER_START(FIBRES MAPPING: PROJECTION ITSELF);
#pragma region Projection

			//vtkMAFSmartPointer< vtkMAFVisualDebugger > vd2;
			//vd2->AddOrUpdateLines("IsoLine", contour, GetInput()->GetLength() / 500, 1,0,0);
			//vd2->AddOrUpdateSurface("Muscle", GetInput(), 0.7, 0, 0.4, 0.5);
			//vd2->DebugStep();

			CONTOUR_MAPPING_PARAMETERIZATION& cnt = ParContours[iCurCnt];
			int nPoints = cnt.nPoints;

			//convert parametric coordinates to absolute
			VCoord* pTargetPoly = new VCoord[nPoints + 3];	//+3 because of FixContour 
			for (int j = 0; j < nPoints; j++)
			{
				vtkIdType nPts, *pPts;
				input->GetCellPoints(cnt.TemplateMuscleContourTriIds[j], nPts, pPts);

				VCoord triPts[3];
				for (int k = 0; k < 3; k++) 
				{
					input->GetPoint(pPts[k], triPts[k]);
					triPts[k][0] *= cnt.TemplateMuscleContour[j][k];
					triPts[k][1] *= cnt.TemplateMuscleContour[j][k];
					triPts[k][2] *= cnt.TemplateMuscleContour[j][k];
				}

				for (int k = 0; k < 3; k++) {
					pTargetPoly[j][k] = triPts[0][k] + triPts[1][k] + triPts[2][k];
				}
			}

			//OK, now we may perform the mapping
			if (nPoints == 1)
			{
				//singular case - to be handled separately        
				for (int i = cnt.iStartPos; i < cnt.iEndPos; i++)
				{
					for (int j = 0; j < 3; j++){
						pFVerts[pOrder[i]][j] = pTargetPoly[0][j];
					}
				}
			}
			else
			{
				//we have an expected case
				VCoord* pTemplatePoly = cnt.BoxContour;

				if ((this->DebugMode & (dbgVisualizeSlicing | dbgVisualizeSlicingResult)) != 0)
				{
					//save contour
					DebugAddContour(pTemplatePoly, nPoints, pTargetContoursPoints, pTargetContoursCells);
					DebugAddContour(pTargetPoly, nPoints, pMuscleContoursPoints, pMuscleContoursCells);
					DebugAddMorphDir(pTemplatePoly, pTargetPoly, nPoints, pMappedFibresPoints, pMappedFibresCells);

#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
#pragma message ( "PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY: ON - this should be off unless you are testing something for a publication" )
					
					DebugAddBestFittingPlane(pTargetPoly, nPoints, pMuscleContoursFittingPlanePoints, pMuscleContoursFittingPlaneCells);
#endif
				}

				//so we can now perform "warping" of points from template cube
				for (int i = cnt.iStartPos, nIndex = 0; i < cnt.iEndPos; i++, nIndex++)
				{
					//transformation of point x (onto target cube)
					for (int k = 0; k < 3; k++){
						pTrPoints[nIndex][k] = lf.O[k] +
							lf.uvw[0][k]*pFVerts[pOrder[i]][0] +
							lf.uvw[1][k]*pFVerts[pOrder[i]][1] +
							lf.uvw[2][k]*pFVerts[pOrder[i]][2];
					}
				} //end for i (points)

				MapPoints(pTrPoints, cnt.iEndPos - cnt.iStartPos,
					pTemplatePoly, pTargetPoly, nPoints);

				if ((this->DebugMode & (dbgVisualizeSlicing)) != 0) {
					DebugVisualizeSlicing(vd, lf, this->OriginArea, this->InsertionArea,
						pTargetContours, pMuscleContours, pMappedFibres
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
					, pMuscleContourFittingPlane
#endif
						);
				}

				//store results
				for (int i = cnt.iStartPos, nIndex = 0; i < cnt.iEndPos; i++, nIndex++)
				{
					for (int j = 0; j < 3; j++){
						pFVerts[pOrder[i]][j] = pTrPoints[nIndex][j];
					}
				}				    				
			}

			delete[] pTargetPoly;
#pragma endregion Projection
			PROFILE_TIMER_STOP(FIBRES MAPPING: PROJECTION ITSELF);
		}	//end for

		//clean up
		for (int iCurCnt = 0; iCurCnt < nContours; iCurCnt++)
		{
			CONTOUR_MAPPING_PARAMETERIZATION& cnt = ParContours[iCurCnt];						
			delete[] cnt.TemplateMuscleContourTriIds;
			delete[] cnt.TemplateMuscleContour;
			delete[] cnt.BoxContour;
		}
		ParContours.clear();

		delete[] pTrPoints;
		delete[] pOrder;  //no longer needed

#ifdef ADV_KUKACKA
		cutterKukacka->Delete();
#endif
		cutter->Delete();
		cutPlane->Delete();

		if ((this->DebugMode & (dbgVisualizeSlicing | dbgVisualizeSlicingResult)) != 0)
		{
			if ((this->DebugMode & (dbgVisualizeSlicingResult)) != 0) {
				DebugVisualizeSlicing(vd,lf, this->OriginArea, this->InsertionArea,
					pTargetContours, pMuscleContours, pMappedFibres
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
					, pMuscleContourFittingPlane
#endif
					);
			}

			pMappedFibres->Delete();
			pMappedFibresCells->Delete();
			pMappedFibresPoints->Delete();
			pTargetContours->Delete();
			pTargetContoursCells->Delete();
			pTargetContoursPoints->Delete();
			pMuscleContours->Delete();
			pMuscleContoursCells->Delete();
			pMuscleContoursPoints->Delete();

#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
			pMuscleContourFittingPlane->Delete();
			pMuscleContoursFittingPlaneCells->Delete();
			pMuscleContoursFittingPlanePoints->Delete();
#endif
		}
	} //end if (DebugMode)
#pragma endregion Computation of Target Fibres by Projection
	PROFILE_TIMER_STOP(FIBRES MAPPING);

	PROFILE_TIMER_START(STORING THE OUTPUT);
#pragma region Saving the Target Fibres into Output PolyData
	//save the result
#ifdef ADV_SLICING_ORTHOPLANES
	double dblTMin = 0.0,	dblTMax = DBL_MAX;
#else
	double OCPlaneO[3], ICPlaneO[3],	//origins of cutting planes
		OCPlaneN[3], ICPlaneN[3];	//normals of cutting planes
#endif

	if (this->AdvancedSlicing != 0)
	{
#ifdef ADV_SLICING_ORTHOPLANES
		double tmintmax[4];
#else
		double centroid[3];
#endif

		//both oriAreaLM and insAreaLM are related to inputTemplate but might not be related to input
		//we need to transform them so that we can use them for the filtering
		if (GetInputTemplate() != NULL)
		{
			TransformPointsFromInputTemplateToInput(oriAreaLM, inputTemplate, input);
			TransformPointsFromInputTemplateToInput(insAreaLM, inputTemplate, input);
			
			TransformPointsFromInputTemplateToInput(oriAreaSurf->GetPoints(), inputTemplate, input);
			TransformPointsFromInputTemplateToInput(insAreaSurf->GetPoints(), inputTemplate, input);

			oriAreaSurf->Modified(); cellLocatorORI->Update();
			insAreaSurf->Modified(); cellLocatorINS->Update();

#ifdef ADV_SLICING_ORTHOPLANES	
			//we need to update lfNorm as well, so that lfNorm.uvw[iPlane] goes from the transformed origin to insertion area			
			oriAreaSurf->GetCenter(lfNorm.O);
			insAreaSurf->GetCenter(lfNorm.uvw[iPlane]);
			for (int k = 0; k < 3; k++) {
				lfNorm.uvw[iPlane][k] -= lfNorm.O[k];
			}

			vtkMath::Normalize(lfNorm.uvw[iPlane]);
#else
			input->GetCenter(centroid);	//get centroid for the muscle, this might be different from box centroid but should be good enough
#endif
		}
#ifndef ADV_SLICING_ORTHOPLANES
		else
		{
			//compute centroid		
			for (int i = 0; i < 3; i++) {
				centroid[i] = lf.O[i] + 0.5*(lf.uvw[0][i] + lf.uvw[1][i] + lf.uvw[2][i]);
			}
		}
#endif

		//and finally, create some filtering stuff
#ifdef ADV_SLICING_ORTHOPLANES
		//project origin and insertion areas points onto the principal axis (iPlane) and
		//returns both extremes in times t				
		GetProjectedMinMaxT(oriAreaLM, lfNorm, iPlane, tmintmax[0], tmintmax[1]);
		GetProjectedMinMaxT(insAreaLM, lfNorm, iPlane, tmintmax[2], tmintmax[3]);

		if (tmintmax[1] < tmintmax[2]) {	//expected case
			dblTMin = tmintmax[1]; dblTMax = tmintmax[2];
		}
		else if (tmintmax[0] < tmintmax[3]) {	//it won't be best but we cannot do much more
			dblTMin = tmintmax[0]; dblTMax = tmintmax[3];
		}
		//else we do not filter anything
#else
		//compute cutting planes
		ComputeCuttingPlane(oriAreaLM, centroid, OCPlaneO, OCPlaneN);
		ComputeCuttingPlane(insAreaLM, centroid, ICPlaneO, ICPlaneN);
#endif
	} //end if AdvSlicing ON

	//lets go process fibers :-)
	vtkPoints* pPoints = vtkPoints::New();	
	vtkCellArray* pCells = vtkCellArray::New();
	pPoly->SetLines(pCells);
	pPoly->SetPoints(pPoints);

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
	vtkPolyData* pPolyCurve = vtkPolyData::New();
	vtkMAFSmartPointer< vtkPoints > curvePoints;
	vtkMAFSmartPointer< vtkCellArray > curveLines;
	pPolyCurve->SetPoints(curvePoints);
	pPolyCurve->SetLines(curveLines);
#endif

	pPoints->SetNumberOfPoints(nFVerts);
	vtkIdType* pIds = new vtkIdType[Resolution + 1];

	nIndex = 0;
	int nFiberPos = 0;

	for (int i = 0; i < this->NumberOfFibres; i++)
	{
		//Filter out the duplicities in the fiber and parts that are outside the area of interest
		int nValidIndex = 0, nValidPoints = this->Resolution + 1;
#ifdef ADV_SLICING_ORTHOPLANES
		FilterFiber(&pFVerts[nFiberPos], nValidIndex, nValidPoints,
			lfNorm, iPlane, dblTMin, dblTMax);	//in simple slicing, tmin-tmax are IGNORED
#else
		if (this->AdvancedSlicing == 0)
			FilterFiber(&pFVerts[nFiberPos], nValidIndex, nValidPoints,
			NULL, NULL, NULL, NULL);	//in simple slicing, there are no cutting planes
		else
			FilterFiber(&pFVerts[nFiberPos], nValidIndex, nValidPoints,
			OCPlaneO, OCPlaneN, ICPlaneO, ICPlaneN);
#endif

		if (nValidPoints != 0)
		{
			if (this->AdvancedSlicing != 0 && ((this->DebugMode & dbgDoNotProjectFibres) != dbgDoNotProjectFibres))
			{
				//if advanced slicing is enabled and we must project fibers, it is likely that we have shorter fibers
				//extrapolate  the fiber and find its intersection with the muscle surface
				if (nValidIndex > 0)
				{
					if (ExtrapolateFiber(&pFVerts[nFiberPos + nValidIndex], nValidPoints, -1, cellLocator,
						cellLocatorORI, cellLocatorINS, &pFVerts[nFiberPos + nValidIndex - 1]))
					{
						nValidPoints++;
						nValidIndex--;
					}
				}

				if ((nValidIndex + nValidPoints) < this->Resolution + 1)
				{
					if (ExtrapolateFiber(&pFVerts[nFiberPos + nValidIndex], nValidPoints, 1, cellLocator,
						cellLocatorORI, cellLocatorINS, &pFVerts[nFiberPos + nValidIndex + nValidPoints])) {
							nValidPoints++;
					}
				}
			}

			if (this->SmoothFibers != 0){
				SmoothFiber(&pFVerts[nFiberPos + nValidIndex], nValidPoints);
			}

			for (int j = 0; j < nValidPoints; j++)
			{
				pPoints->SetPoint(nIndex, pFVerts[nFiberPos + nValidIndex + j]);
				pIds[j] = nIndex++;
			} //end for j

			pCells->InsertNextCell(nValidPoints, pIds);
		}

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
		pPoly->BuildCells();
		double MSE = FitCurveToFibre(testDataType, pPoly, pCells->GetNumberOfCells() - 1, pPolyCurve);
		_RPT2(_CRT_WARN, "#%d: MSE = %.3f\n", pCells->GetNumberOfCells() - 1, MSE);
		
#endif

		if ((this->DebugMode & dbgVisualizeFibresPostprocessing) == dbgVisualizeFibresPostprocessing)
		{	
			vtkCellArray* pPoly2Cells = vtkCellArray::New();
			vtkPolyData* pPoly2 = vtkPolyData::New();
			pPoly2->SetPoints(pPoly->GetPoints());
			pPoly2->SetLines(pPoly2Cells);
			pPoly2Cells->UnRegister(this);
			
			vtkIdType nPts, *pPts;
			pPoly->GetCellPoints(pCells->GetNumberOfCells() - 1, nPts, pPts);
			pPoly2Cells->InsertNextCell(nPts, pPts);


#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
			vtkCellArray* pPolyCurve2Cells = vtkCellArray::New();

			vtkPolyData* pPolyCurve2 = vtkPolyData::New();
			pPolyCurve2->SetPoints(pPolyCurve->GetPoints());
			pPolyCurve2->SetLines(pPolyCurve2Cells);
			pPolyCurve2Cells->UnRegister(this);
			pPolyCurve->BuildCells();
			
			pPolyCurve->GetCellPoints(pCells->GetNumberOfCells() - 1, nPts, pPts);
			pPolyCurve2Cells->InsertNextCell(nPts, pPts);
#endif
			DebugVisualizeFibresPostprocessing(vd, this->OriginArea, this->InsertionArea, pPoly2
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
				, pPolyCurve2
#endif
				);
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
			pPolyCurve2->Delete();
#endif
			pPoly2->Delete();
		}

		nFiberPos += this->Resolution + 1;
	} //end for i

	pPoints->SetNumberOfPoints(nIndex);	//reduce the number of points

	delete[] pIds;

	pCells->Delete();
	pPoints->Delete();

	delete[] pFVerts;

	if ((this->DebugMode & dbgVisualizeFibresPostprocessingResult) == dbgVisualizeFibresPostprocessingResult)
	{
		DebugVisualizeFibresPostprocessing(vd, this->OriginArea, this->InsertionArea, pPoly
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
			, pPolyCurve
#endif
			);
	}

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
	pPolyCurve->Delete();
#endif

#pragma endregion Saving the Target Fibres into Output PolyData
	PROFILE_TIMER_STOP(STORING THE OUTPUT);	
	PROFILE_TIMER_START(CLEANUP);
#pragma region CleanUp	
	if (cellLocator != NULL) cellLocator->Delete();	//delete cellLocator
	if (oriAreaLM != NULL) oriAreaLM->Delete();
	if (insAreaLM != NULL) insAreaLM->Delete();
	if (oriAreaSurf != NULL) oriAreaSurf->Delete();
	if (insAreaSurf != NULL) insAreaSurf->Delete();
	if (cellLocatorORI != NULL) cellLocatorORI->Delete();
	if (cellLocatorINS != NULL) cellLocatorINS->Delete();

	if (vd != NULL)
		vd->Delete();
	PROFILE_TIMER_STOP(CLEANUP);
#pragma endregion
	PROFILE_TIMER_STOP(CURRENT-POSE PROCESSING);

#ifdef ADV_KUKACKA_TEST	
	PROFILE_TIMER_STOP(_T(__FUNCTION__));
	}
	CDefaultProfiler::g_pDefaultProfiler->SetValue(_T("MeshVertices"), this->GetInput()->GetNumberOfPoints());
	CDefaultProfiler::g_pDefaultProfiler->SetValue(_T("MeshTriangles"), this->GetInput()->GetNumberOfCells());
	CDefaultProfiler::g_pDefaultProfiler->SetValue(_T("Resolution"), this->Resolution);
	CDefaultProfiler::g_pDefaultProfiler->SetValue(_T("NumberOfFibres"), this->NumberOfFibres);
	CDefaultProfiler::g_pDefaultProfiler->SetValue(_T("Experiments"), nMaxExpers);

	_TCHAR szBuf[MAX_PATH];
	_stprintf(szBuf, _T("Kukacka_Profile_Log_%d_%d_%d.xml"), this->GetInput()->GetNumberOfPoints(), this->Resolution, this->NumberOfFibres);

	CDefaultProfiler::g_pDefaultProfiler->SaveProfile(szBuf);
	CDefaultProfiler::g_pDefaultProfiler->ResetProfile();
#endif
} //end: void vtkMAFMuscleDecomposition::ExecuteData(vtkDataObject *output)

//Routines related to matching template cube and input data bounding box
#pragma region Matching

//------------------------------------------------------------------------
//Computes the principal axis for the given point set.
//N.B. the direction is normalized
void vtkMAFMuscleDecomposition::ComputeAxisLine(vtkPoints* points,
	double* origin, double* direction)
	//------------------------------------------------------------------------
{
	//the line goes through the centroid
	ComputeCentroid(points, origin);

	//compute eigen vectors, the principal axis is the first one
	double eigenvects[3][3];
	ComputeEigenVects(points, origin, eigenvects);

	for (int i = 0; i < 3; i++) {
		direction[i] = eigenvects[0][i];
	}
}

//------------------------------------------------------------------------
//Computes 4*nFrames vectors by rotating u around r vector. All vectors are
//normalized and stored in the order A, B, C, D where B is the vector
//opposite to A, C is vector perpendicular to A and r and D is vector
//opposite to C. N.B. vectors u and r must be normalized and perpendicular!
//The buffer pVectors must be capable enough to hold all vectors.
void vtkMAFMuscleDecomposition::ComputeDirectionVectors(
	double* u, double* r, int nFrames, VCoord* pVectors)
	//------------------------------------------------------------------------
{
	//the matrix for the rotation can be computed by procedure given in
	//Bloomenthal J. Calculation of reference frames along a
	//space curve. Graphics Gems I, 1990, 567–571.
	//http://www.unchainedgeometry.com/jbloom/pdf/ref-frames.pdf
	//or also by Schneider: Geometric Tools for Computer Games
	double cos_delta = cos(M_PI_2 / (nFrames + 1));
	double sin_delta = sqrt(1 - cos_delta*cos_delta);
	double cos_theta = 1.0;

	double sqx = r[0]*r[0];
	double sqy = r[1]*r[1];
	double sqz = r[2]*r[2];

	double M[3][3]; //rotation matrix
	for (int i = 0, index = 0; i < nFrames; i++)
	{
		double sin_w = sqrt(1.0 - cos_theta*cos_theta);
		double cos_w1 = 1.0 - cos_theta;

		double xycos1 = r[0]*r[1]*cos_w1;
		double yzcos1 = r[1]*r[2]*cos_w1;
		double zxcos1 = r[2]*r[0]*cos_w1;

		double xsin = r[0]*sin_w;
		double ysin = r[1]*sin_w;
		double zsin = r[2]*sin_w;

		M[0][0] = sqx + (1.0 - sqx)*cos_theta;
		M[1][0] = xycos1 + zsin;
		M[2][0] = zxcos1 - ysin;

		M[0][1] = xycos1 - zsin;
		M[1][1] = sqy + (1.0 - sqy)*cos_theta;
		M[2][1] = yzcos1 + xsin;

		M[0][2] = zxcos1 + ysin;
		M[1][2] = yzcos1 - xsin;
		M[2][2] = sqz + (1 - sqz)*cos_theta;

		//by multiplying vector u by the matrix we get the new vector
		for (int j = 0; j < 3; j++)
		{
			pVectors[index][j] = 0.0;
			for (int k = 0; k < 3; k++) {
				pVectors[index][j] += u[k]*M[k][j];
			}
		}

		//get the other axis
		vtkMath::Cross(pVectors[index], r, pVectors[index + 2]);
		vtkMath::Normalize(pVectors[index + 2]);

		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 3; k++){
				pVectors[index + 1][k] = -pVectors[index][k];
			}

			index += 2;
		}

		//in the next loop we will need to create LFS for angle theta + delta
		//cos(theta + delta) = cos(theta)*cos(delta)-sin(theta)*sin(delta)
		cos_theta = cos_theta*cos_delta - sin_w*sin_delta;
	}
}

//------------------------------------------------------------------------
//Adjusts the length of direction vectors (in pVects) to fit the given point set.
//For each direction vector, the algorighm find a plane defined by the center
//(it should be the centroid of points) and a normal of non-unit size that is
//collinear with the input direction vector. This normal is chosen so that the
//no point lies in the positive halfspace of the plane (i.e. in the direction
//of normal from the plane). The computed normals are returned in pVects.
//N.B. the input vectors must be of unit size!
void vtkMAFMuscleDecomposition::FitDirectionVectorsToData(vtkPoints* points,
	double* center, int nVects, VCoord* pVects)
	//------------------------------------------------------------------------
{
	double* pLengths = new double[nVects];
	memset(pLengths, 0, nVects*sizeof(double));

	int N = points->GetNumberOfPoints();
	for (int i = 0; i < N; i++)
	{
		const double* pcoords = points->GetPoint(i);
		for (int j = 0; j < nVects; j++)
		{
			//compute dot product
			double dblDot = 0.0;
			for (int k = 0; k < 3; k++){
				dblDot += (pcoords[k] - center[k])*pVects[j][k];
			}

			//the point lies outside the area => enlarge the vector length
			if (dblDot > pLengths[j])
				pLengths[j] = dblDot;
		}
	}

	for (int i = 0; i < nVects; i++)
	{
		_ASSERT(pLengths[i] != 0.0);

		for (int j = 0; j < 3; j++){
			pVects[i][j] *= pLengths[i];
		}
	}

	delete[] pLengths;
}

//------------------------------------------------------------------------
//Computes local frame systems for various cubes defined by their center
//and two direction vectors in w and 4 direction vectors in u and v axis.
//Direction vectors in u and v are given in uv_dirs and have the structure
//compatible with the output of ComputeDirectionVectors method.
//The computed LFs are stored in pLFS buffer. The buffer must be capable to
//hold 8*nCubes (= nCubes in ComputeDirectionVectors) entries.
//------------------------------------------------------------------------
void vtkMAFMuscleDecomposition::ComputeLFS(double* center, VCoord* w_dir,
	int nCubes, VCoord* uv_dirs, LOCAL_FRAME* pLFS)
{
	double w01[3], cv0[3], cv1[3];  //temp vars
	for (int i = 0; i < 3; i++)
	{
		w01[i] = w_dir[0][i] - w_dir[1][i];
		cv0[i] = center[i] + w_dir[0][i];
		cv1[i] = center[i] + w_dir[1][i];
	}

	for (int i = 0, index = 0; i < 4*nCubes; i += 4, index += 8)
	{
		for (int j = 0; j < 3; j++)
		{
			pLFS[index + 0].O[j] = cv1[j] + uv_dirs[i + 1][j] + uv_dirs[i + 3][j];
			pLFS[index + 1].O[j] = cv1[j] + uv_dirs[i + 0][j] + uv_dirs[i + 3][j];
			pLFS[index + 2].O[j] = cv1[j] + uv_dirs[i + 0][j] + uv_dirs[i + 2][j];
			pLFS[index + 3].O[j] = cv1[j] + uv_dirs[i + 1][j] + uv_dirs[i + 2][j];
			pLFS[index + 4].O[j] = cv0[j] + uv_dirs[i + 1][j] + uv_dirs[i + 3][j];
			pLFS[index + 5].O[j] = cv0[j] + uv_dirs[i + 0][j] + uv_dirs[i + 3][j];
			pLFS[index + 6].O[j] = cv0[j] + uv_dirs[i + 0][j] + uv_dirs[i + 2][j];
			pLFS[index + 7].O[j] = cv0[j] + uv_dirs[i + 1][j] + uv_dirs[i + 2][j];

			pLFS[index + 0].uvw[0][j] = pLFS[index + 3].uvw[1][j] =
				pLFS[index + 4].uvw[1][j] = pLFS[index + 7].uvw[0][j] = uv_dirs[i + 0][j] - uv_dirs[i + 1][j];

			pLFS[index + 1].uvw[1][j] = pLFS[index + 2].uvw[0][j] =
				pLFS[index + 5].uvw[0][j] = pLFS[index + 6].uvw[1][j] = -pLFS[index + 0].uvw[0][j];

			pLFS[index + 0].uvw[1][j] = pLFS[index + 1].uvw[0][j] =
				pLFS[index + 4].uvw[0][j] = pLFS[index + 5].uvw[1][j] = uv_dirs[i + 2][j] - uv_dirs[i + 3][j];

			pLFS[index + 2].uvw[1][j] = pLFS[index + 3].uvw[0][j] =
				pLFS[index + 6].uvw[0][j] = pLFS[index + 7].uvw[1][j] = -pLFS[index + 0].uvw[1][j];

			for (int k = 0; k < 4; k++)
			{
				pLFS[index + k].uvw[2][j] = w01[j];
				pLFS[index + 4 + k].uvw[2][j] = -w01[j];
			}
		}
	}
}

//------------------------------------------------------------------------
//Finds the best local frame system from those passed in pLFS that best
//maps template origin and insertion points to target origin and insertion
//points. N.B. any point set can be NULL, if it is not needed. Special
//case is when both target sets or template sets are NULL, then the
//routine returns the first LF.
int vtkMAFMuscleDecomposition::FindBestMatch(vtkPoints* template_O, vtkPoints* template_I,
	int nLFS, LOCAL_FRAME* pLFS, vtkPoints* target_O, vtkPoints* target_I)
	//------------------------------------------------------------------------
{
	vtkMAFVisualDebugger* vd = (this->DebugMode == dbgNone ? NULL : vtkMAFVisualDebugger::New());

	//check, if the input is valid for matching
	vtkPoints* tmpPts[4] = { template_O, template_I, target_O, target_I };
	int nPoints[4];       //number of points in template_O, I, target_O, I

	for (int i = 0; i < 4; i++)
	{
		nPoints[i] = 0;

		if (tmpPts[i] != NULL) {
			nPoints[i] = tmpPts[i]->GetNumberOfPoints();
		}
	}

	//check, if the input is valid for matching
	if ((nPoints[0] == 0 || nPoints[2] == 0) &&
		(nPoints[1] == 0 || nPoints[3] == 0))
		return 0;

	//we have at least one point to match
	//extract coordinates
	VCoord* pPoints[4];
	for (int i = 0; i < 4; i++)
	{
		pPoints[i] = NULL;
		if (nPoints[i] != 0)
		{
			pPoints[i] = new VCoord[nPoints[i]];
			for (int j = 0; j < nPoints[i]; j++){
				tmpPts[i]->GetPoint(j, pPoints[i][j]);
			}
		}
	}

	//Algorithm:
	//Every point from template_X is transformed using the current LF
	//and the final Euclidian coordinates are compared to coordinates
	//of points from target_X in order to find the closest target point.

	//Distances between points from template_X and their closest counterparts
	//from target_X are sorted and M first smallest distances are summed to
	//get the matching score. As our fiber templates have no more than 10 points,
	//M = all points.The LF with the lowest score is chosen as the best.

	int iBestLF = 0;
	double dblBestLFScore = DBL_MAX;   //infinite

	for (int iSys = 0; iSys < nLFS; iSys++)
	{
		double dblScore = 0.0;
		for (int i = 0; i < 2; i++)
		{
			if (nPoints[i] == 0 || nPoints[i + 2] == 0)
				continue; //invalid matching, skip it

			for (int j = 0; j < nPoints[i]; j++)
			{
				//transform point j
				//NOTE: fiber templates use left hand oriented coordinate system
				double x[3];
				for (int k = 0; k < 3; k++){
					x[k] = pLFS[iSys].O[k] + pLFS[iSys].uvw[0][k]*pPoints[i][j][0] +
						pLFS[iSys].uvw[1][k]*pPoints[i][j][2] + pLFS[iSys].uvw[2][k]*pPoints[i][j][1];
				}

				//find the closest point
				//brute-force as the number of points should be lower than 10
				double dblMinDist = DBL_MAX;
				for (int m = 0; m < nPoints[i + 2]; m++)
				{
					double dblDist = vtkMath::Distance2BetweenPoints(x, pPoints[i + 2][m]);
					if (dblDist < dblMinDist)
						dblMinDist = dblDist;
				}

				dblScore += sqrt(dblMinDist);
			} //end for j (points in template)
		} //end for i (templates)
		
		if ((this->DebugMode & dbgVisualizeFitting) == dbgVisualizeFitting)
			DebugVisualizeFitting(vd, iSys, nLFS, pLFS[iSys],
			template_O, template_I, target_O, target_I, dblScore);

		if (dblScore < dblBestLFScore)
		{
			dblBestLFScore = dblScore;
			iBestLF = iSys;
		}
	} //end for iSys

	for (int i = 0; i < 4; i++) {
		delete[] pPoints[i];
	}

	if ((this->DebugMode & dbgVisualizeFittingResult) == dbgVisualizeFittingResult)
		DebugVisualizeFitting(vd, iBestLF, nLFS, pLFS[iBestLF],
		template_O, template_I, target_O, target_I, dblBestLFScore, true);

	if (vd != NULL)
		vd->Delete();

	return iBestLF;
}

//------------------------------------------------------------------------
//Computes the minimal oriented box that fits the input data so that all
//points are inside of this box (or on its boundary) and the total squared
//distance of template origin points from the input mesh origin points and
//the total squared distance of template insertion points from the input
//mesh origin points are minimized
//------------------------------------------------------------------------
void vtkMAFMuscleDecomposition::ComputeFittingOB(vtkPoints* points, LOCAL_FRAME& out_lf)
{
	//get the principal axis
	double l_pos[3], l_dir[3];
	if (this->AdvancedSlicing == 0)
		ComputeAxisLine(points, l_pos, l_dir);
	else
		ComputeAxisLine(points, OriginArea, InsertionArea, l_pos, l_dir);

	//compute initial vectors representing initial axis u (perpendicular to
	//l_dir) u is a projection into one of of XZ, XY or YZ plane + 90 degrees
	//rotation the optimal plane is the one closest to the plane where u lies
	int iPlane = 0;
	for (int i = 1; i < 3; i++) {
		if (fabs(l_dir[i]) < fabs(l_dir[iPlane]))
			iPlane = i; //new minimum
	}

	double u[3];
	int i1 = (iPlane + 1) % 3;
	int i2 = (iPlane + 2) % 3;
	u[i1] = l_dir[i2];
	u[i2] = -l_dir[i1];
	u[iPlane] = 0.0;

	vtkMath::Normalize(u);

	//create various systems by rotating them around w axis by a small angle (approx. 8 degrees)
	const int nCubes = 10;       //number of frames to be created in Pi/2 area
	int nVects = (4*nCubes + 2); //total number of vectors

	VCoord* pVects = new VCoord[nVects];
	for (int i = 0; i < 3; i++)
	{
		pVects[0][i] = l_dir[i];
		pVects[1][i] = -l_dir[i];
	}

	ComputeDirectionVectors(u, l_dir, nCubes, &pVects[2]);

	//for every direction, we now need to find a plane such that
	//all input points are in the negative half-space
	//the routine shorten (or prolong) input vectors so they
	//define the data bounding object
	FitDirectionVectorsToData(points, l_pos, nVects, pVects);

	//it is possible to construct cubes by linear combination
	//of the origin l_pos and vectors pVects[0], pVects[1],
	//pVects[i], pVects[i + 1], pVects[i + 2] and pVects[i + 3]
	//we will construct LFS
	LOCAL_FRAME* pLFS = new LOCAL_FRAME[8*nCubes];
	ComputeLFS(l_pos, pVects, nCubes, &pVects[2], pLFS);
	delete[] pVects;  //no longer needed

	//find the best system for the input data
	vtkPoints* tempO = vtkPoints::New();
	vtkPoints* tempI = vtkPoints::New();
	FibersTemplate->GetOriginLandmarks(tempO);
	FibersTemplate->GetInsertionLandmarks(tempI);

	out_lf = pLFS[FindBestMatch(tempO, tempI,
		8*nCubes, pLFS, OriginArea, InsertionArea)];

	tempI->Delete();
	tempO->Delete();
	delete[] pLFS;
	//and we have it here
}