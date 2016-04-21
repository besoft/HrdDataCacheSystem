/*========================================================================= 
Program: Musculoskeletal Modeling (VPHOP WP10)
Module: $RCSfile: vtkMAFMeanValueCoordinatesInterpolation.cpp,v $ 
Language: C++ 
Date: $Date: 2012-04-19 12:05:36 $ 
Version: $Revision: 1.1.2.2 $ 
Authors: Josef Kohout, Petr Kellnhofer
========================================================================== 
Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details 
=========================================================================
*/


#include "vtkMAFMeanValueCoordinatesInterpolation.h"
#include "vtkObjectFactory.h"
#include "vtkCellArray.h"
#include "vtkMath.h"
#include "mafDbg.h"

#include <float.h>

vtkCxxRevisionMacro(vtkMAFMeanValueCoordinatesInterpolation, "$Revision: 1.1.2.2 $");
vtkStandardNewMacro(vtkMAFMeanValueCoordinatesInterpolation);


#pragma region MVC Calculation
#pragma region OneDimensional Weights
/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const vtkPoints* inPoints, vtkDoubleArray* outWeights)
{
	int nBufSize = const_cast<vtkPolyData*>(inputMesh)->GetNumberOfPoints()*const_cast<vtkPoints*>(inPoints)->GetNumberOfPoints();
	ComputeMVCCoordinates(inputMesh, inPoints, outWeights->WritePointer(0, nBufSize));	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */	
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, vtkDoubleArray* outWeights)
{
	int nBufSize = const_cast<vtkPolyData*>(inputMesh)->GetNumberOfPoints()*numPoints;
	ComputeMVCCoordinates(inputMesh, inPoints, numPoints, outWeights->WritePointer(0, nBufSize));	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, vtkDoubleArray* outWeights)
{
	int nBufSize = const_cast<vtkPolyData*>(inputMesh)->GetNumberOfPoints()*numPoints;
	ComputeMVCCoordinates(inputMesh, inPoints, numPoints, outWeights->WritePointer(0, nBufSize));	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints()*inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh,  const vtkPoints* inPoints, double* outWeights)
{
	vtkPoints* pts = const_cast<vtkPoints*>(inPoints);
	int nPoints = pts->GetNumberOfPoints();

	// call template function to compute the weights. Note that we do not 
	//use VTK's template macro because we are limiting usage to floats and doubles.  	
	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ComputeMVCCoordinatesT(inputMesh, (const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, outWeights));			
	}	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, double* outWeights)
{
	ComputeMVCCoordinatesT(inputMesh, inPoints, numPoints, outWeights);	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, double* outWeights)
{
	ComputeMVCCoordinatesT(inputMesh, inPoints, numPoints, outWeights);	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const double* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const double* inPoints, int numPoints, double* outWeights)
{
	ComputeMVCCoordinatesT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const float* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const float* inPoints, int numPoints, double* outWeights)
{
	ComputeMVCCoordinatesT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinatesT(const vtkPolyData* inputMesh, const T* inPoints, int numPoints, double* outWeights)
{
	vtkPoints* pts = const_cast<vtkPolyData*>(inputMesh)->GetPoints();
	vtkIdType nPoints = pts->GetNumberOfPoints();

	vtkCellArray* cells = const_cast<vtkPolyData*>(inputMesh)->GetPolys();
	vtkIdType nTriangles = cells->GetNumberOfCells();

	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ComputeMVCCoordinatesT((const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, 
			cells->GetPointer(), nTriangles, inPoints, numPoints, outWeights));			
	}
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
template < typename T1, typename T2 >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinatesT(const T1* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const T2* inPoints, int numPoints, double* outWeights)
{
#pragma omp parallel shared(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights)
	{
#pragma omp for
		for (int i = 0; i < numPoints; i++)
		{			
			//get the point in the original mesh			
			const T2* point = inPoints + i*3;
			double* ptWeights = outWeights + i*numMeshPoints;			

			//and calculate its mean value coordinates (aka barycentric) in the coarse mesh
			ComputeMVCCoordinatesForPointT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, point, ptWeights);
		}
	}	
}

#ifndef ISFINITE
#define ISFINITE(w) ((w) >= -DBL_MAX && (w) <= DBL_MAX)
#endif

//----------------------------------------------------------------------------------------------------------------------
/// Calculates Mean Value Coordinates (MVC) weights of triangle mesh whose vertices are specified in pts array (and may be doubles or floats)
/// and triangles in pcells (format is: num of idx, idx1, idx2, idx3, num of idx, idx1, ...), i.e., these can be extracted directly from vtkPolyData
/// Refers to Mean Value Coordinates for Closed Triangular Meshes, Tao Ju, Scott Schaefer, Joe Warren, Rice University
/// @param pts coordinates of mesh (input)
/// @param npts number of points (input)
/// @param pcells cells (assumingly triangles: (3, id1, id2, id3), (3, id1, id2, id3) ...) (input)
/// @param ncells number of cells (input)
/// @param point 3D coordinates of vertex to be decomposed (input)
/// @param coords preallocated array of length of number of vertices of mesh for final coefficients of linear decomposition (output)
//----------------------------------------------------------------------------------------------------------------------
template < typename T1, typename T2 >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinatesForPointT(const T1* pts, vtkIdType npts, const vtkIdType* pcells, 
	vtkIdType ncells, const T2* point, double* coords)
{	
	//Points are organized {(x,y,z), (x,y,z), ....}
	//Tris are organized {(i,j,k), (i,j,k), ....}
	//Weights per point are computed
	_VERIFY_RET(npts != 0);

	const static int IDXMOD[] = {0, 1, 2, 0, 1, 2};	//to avoid % 3
	const static double MESH_DISTANCE_EPSILON = 1e-16; //0.000000001;


	// Begin by initializing weights. 
	memset(coords, 0, sizeof(double) * npts);

	// create local array for storing point-to-vertex vectors and distances
	double *u = new double[npts * 3];		//vectors
	double *d = new double[npts];				//distances

	//first, calculate vectors u and distances d
	//while checking, if point isn't too close to a vertex of mesh
	const T1* inPts = pts;
	double *outU = u;
	vtkIdType i, j;
	for (i = 0; i < npts; i++, outU += 3)
	{	
		//initialize distance
		d[i] = 0.0;

		//create point-to-vertex vector
		for (j = 0; j < 3; j++, inPts++) {
			outU[j] = ((double)*inPts) - point[j];			
		}

		//calculate distance (length of the vector)
		d[i] = vtkMath::Norm(outU);								
		if (d[i] < MESH_DISTANCE_EPSILON)
		{	
			//we are too close to the point
			coords[i] = 1.0;
			delete[] u;
			delete[] d;
			return;
		}

		// project onto unit sphere
		for (j = 0; j < 3; j++) {
			outU[j] /= d[i];
		}		
	}


	//It is not a simple case => the point either lies on some triangle or inside the object
	//Now loop over all triangle to compute weights
	const vtkIdType* triangleIds = pcells;		
	double totalW = 0;

	// for every triangle
	vtkIdType cellsize;
	for (i = 0; i < ncells; i++, triangleIds += cellsize )
	{	
		cellsize = triangleIds[0];
		triangleIds++;		//move to the first index of cell

		// filter out non triangles
		_VERIFY_CMD(cellsize == 3, continue);


		//get u vectors at vertices
		double* uVec[3];
		for (j = 0; j < 3; j++) {
			uVec[j] = u + 3*triangleIds[j];
		}


		double l[3], o[3];					//edge lengths and theta angles				
		double sin_o[3], cos_o[3];	//sin(theta) and cos(theta)
		for (j = 0; j < 3; j++)
		{						
			//calculate the length l_j of opposite side of triangle			
			// top apex theta angle o_j of tetrahedron (created from the point and triangle) on opposite side to j apex
			//is calculated as follows + calculate also sin(o_j) and cos(o_j)
			//x = l_j / 2; o_j = 2*asin(x) => o_j / 2 = asin (x) => x = sin(o_j / 2)
			//sin(o_j) = 2*sin(o_j / 2)*cos(o_j / 2) = 2*x*sqrt(1 - sin^2(o_j/2))
			//cos(o_j) = cos^2(o_j/2) - sin^2(o_j/2) = (1 - sin^2(o_j/2)) - sin^2(o_j/2) = 1-2*x^2			

			double dist = vtkMath::Distance2BetweenPoints(uVec[IDXMOD[j + 1]], uVec[IDXMOD[j + 2]]);
			double x2 = dist * (1 / 4.0);
			if (x2 > 1.0)
				x2 = 1.0;			//fix numerical problems

			l[j] = sqrt(dist);
			double x = l[j] * 0.5;
			if (x > 1.0)
				x = 1.0;		//fix numerical problems

			o[j] = 2 * asin(x);
			cos_o[j] = 1 - 2*x2;
			sin_o[j] = 2*x*sqrt(1 - x2);
		}

		double h = (o[0] + o[1] + o[2]) / 2;	//half sum theta angle

		//check, if the point does not lie on the triangle
		if (vtkMath::DoublePi() - h < MESH_DISTANCE_EPSILON)
		{			
			//the point lies on the triangle => use 2D barycentric coordinates
			memset(coords, 0, sizeof(double) * npts);	//zero all coordinates filled so far

			double w[3], sumW = 0;
			for (j = 0; j < 3; j++)
			{
				w[j] = sin_o[j] * d[IDXMOD[j + 1]] * d[IDXMOD[j + 2]];
				sumW += w[j];
			}

			for (j = 0; j < 3; j++)
			{
				*(coords + triangleIds[j]) = w[j] / sumW;
			}

			//and we are ready
			delete[] u;
			delete[] d;
			return;
		}

		//the point is outside the current triangle, let us compute weights for each its vertex
		double c[3], s[3];
		bool siLtEpsilonExists = false;

		double det = vtkMath::Determinant3x3(uVec[0], uVec[1], uVec[2]);
		if (fabs(det) < MESH_DISTANCE_EPSILON) {			
			continue;	// x lies outside t on the same plane, ignore t
		}

		if (det > 0.0)
			det = 1.0; 
		else if (det < 0.0) 
			det = -1.0;
		else 
			det = 0.0;

		// cache sin(h) and cos(h), hopefully, the compiler will use sincos function to compute it
		double sin_h = sin(h), cos_h = cos(h);				
		for (j = 0; j < 3; j++)
		{
			//calculate coefficient c_j for each vertex:
			//c[j] = 2 * sin(h) * sin(h - o[j]) / (sin(o[(j + 1) % 3]) * sin(o[(j + 2) % 3])) - 1;
			//using goniometric formulas sin(h-o[j]) can be rewritten as sin(h)*cos(o[j]) - cos(h)*sin(o[j])
			c[j] = 2*sin_h*(sin_h*cos_o[j] - cos_h*sin_o[j]) / (sin_o[IDXMOD[j + 1]] * sin_o[IDXMOD[j + 2]]) - 1;
			if (c[j] > 1.0)					//numeric stability
				c[j] = 1.0;
			else if (c[j] < -1.0)
				c[j] = 1.0;

			s[j] = sqrt(1 - c[j] * c[j]);
			if (s[j] < MESH_DISTANCE_EPSILON)
			{
				siLtEpsilonExists = true;
				break;
			}

			s[j] *= det;	//sign of determinant
		}

		if (siLtEpsilonExists)
		{
			// x lies outside t on the same plane, ignore t
			continue;
		}

		//calculate the final weights
		for (j = 0; j < 3; j++)
		{
			//weights[pid0] += (o[0]-c[1]*o[2]-c[2]*o[1]) / (dist[pid0]*sin_o[1]*s[2]);
			//weights[pid1] += (o[1]-c[2]*o[0]-c[0]*o[2]) / (dist[pid1]*sin_o[2]*s[0]);
			//weights[pid2] += (o[2]-c[0]*o[1]-c[1]*o[0]) / (dist[pid2]*sin_o[0]*s[1]);

			double w = (o[j] - c[IDXMOD[j + 1]] * o[IDXMOD[j + 2]] - c[IDXMOD[j + 2]] * o[IDXMOD[j + 1]]) / 
				(d[triangleIds[j]] * sin_o[IDXMOD[j + 1]] * s[IDXMOD[j + 2]]);

			_VERIFY_CMD(ISFINITE(w), continue);

			*(coords + triangleIds[j]) += w;
			totalW += w;
		}
	}

	// normalize (unless the sum is too small)
	if (fabs(totalW) >= MESH_DISTANCE_EPSILON)
	{
		for (i = 0; i < npts; i++) {
			coords[i] /= totalW;
		}
	}

	delete[] u;
	delete[] d;
}
#pragma endregion

#pragma region TwoDimensional Weights
/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh,  const vtkPoints* inPoints, double** outWeights)
{
	vtkPoints* pts = const_cast<vtkPoints*>(inPoints);
	int nPoints = pts->GetNumberOfPoints();

	// call template function to compute the weights. Note that we do not 
	//use VTK's template macro because we are limiting usage to floats and doubles.  	
	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ComputeMVCCoordinatesT(inputMesh, (const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, outWeights));			
	}	
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.  */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, double** outWeights)
{
	ComputeMVCCoordinatesT(inputMesh, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, double** outWeights)
{
	ComputeMVCCoordinatesT(inputMesh, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const double* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const double* inPoints, int numPoints, double** outWeights)
{
	ComputeMVCCoordinatesT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.  */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(const float* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const float* inPoints, int numPoints, double** outWeights)
{
	ComputeMVCCoordinatesT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights);
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.*/
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinatesT(const vtkPolyData* inputMesh, const T* inPoints, int numPoints, double** outWeights)
{
	vtkPoints* pts = const_cast<vtkPolyData*>(inputMesh)->GetPoints();
	vtkIdType nPoints = pts->GetNumberOfPoints();

	vtkCellArray* cells = const_cast<vtkPolyData*>(inputMesh)->GetPolys();
	vtkIdType nTriangles = cells->GetNumberOfCells();

	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ComputeMVCCoordinatesT((const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, 
			cells->GetPointer(), nTriangles, inPoints, numPoints, outWeights));			
	}
}

/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
template < typename T1, typename T2 >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinatesT(const T1* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
	int numMeshCells, const T2* inPoints, int numPoints, double** outWeights)
{
#pragma omp parallel shared(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, inPoints, numPoints, outWeights)
	{
#pragma omp for schedule(guided)
		for (int i = 0; i < numPoints; i++)
		{			
			//get the point in the original mesh			
			const T2* point = inPoints + i*3;
			double* ptWeights = outWeights[i];			

			//and calculate its mean value coordinates (aka barycentric) in the coarse mesh
			ComputeMVCCoordinatesForPointT(inMeshPoints, numMeshPoints, inMeshCells, numMeshCells, point, ptWeights);
		}
	}	
}

#pragma endregion
#pragma endregion

#pragma region MVC Reconstruction
#pragma region OneDimensional Weights
/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, vtkPoints* points)
{
	points->SetNumberOfPoints(numPoints);
	// call template function to compute the weights. Note that we do not 
	//use VTK's template macro because we are limiting usage to floats and doubles.  
	switch (points->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, (VTK_TT *)(points->GetVoidPointer(0))));			
	}	
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const vtkDoubleArray* inWeights, double* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, const_cast<vtkDoubleArray*>(inWeights)->GetPointer(0), points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, double* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const vtkDoubleArray* inWeights, float* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, const_cast<vtkDoubleArray*>(inWeights)->GetPointer(0), points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, float* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, 
	int numPoints, const double* inWeights, double* points)
{
	ReconstructAttributesT(inMeshPoints, numMeshPoints, 3, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, 
	int numPoints, const double* inWeights, float* points)
{
	ReconstructAttributesT(inMeshPoints, numMeshPoints, 3, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer points must be capable to store numPoints points. */
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinatesT(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, T* points)
{
	vtkPoints* pts = const_cast<vtkPolyData*>(inputMesh)->GetPoints();
	vtkIdType nPoints = pts->GetNumberOfPoints();

	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT((const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, 3, numPoints, inWeights, points));			
	}
}
#pragma endregion

#pragma region TwoDimensional Weights
/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, vtkPoints* points)
{
	points->SetNumberOfPoints(numPoints);
	// call template function to compute the weights. Note that we do not 
	//use VTK's template macro because we are limiting usage to floats and doubles.  
	switch (points->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, (VTK_TT *)(points->GetVoidPointer(0))));			
	}	
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, double* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, float* points)
{
	ReconstructCartesianCoordinatesT(inputMesh, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, 
	int numPoints, const double** inWeights, double* points)
{
	ReconstructAttributesT(inMeshPoints, numMeshPoints, 3, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer outpoints must be capable to store numPoints points. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, 
	int numPoints, const double** inWeights, float* points)
{
	ReconstructAttributesT(inMeshPoints, numMeshPoints, 3, numPoints, inWeights, points);
}

/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
This is a more memory-friendly version of the method that consumes less continuous memory.
N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
Buffer points must be capable to store numPoints points. */
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructCartesianCoordinatesT(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, T* points)
{
	vtkPoints* pts = const_cast<vtkPolyData*>(inputMesh)->GetPoints();
	vtkIdType nPoints = pts->GetNumberOfPoints();

	switch (pts->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT((const VTK_TT *)(pts->GetVoidPointer(0)), nPoints, 3, numPoints, inWeights, points));			
	}
}
#pragma endregion

#pragma endregion

#pragma region MVC Reconstruction of Attributes
#pragma region OnDimensional Weights
//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 
//@param inputAttrs attributes of vertices of mesh topologically compatible with the mesh specified in the call of corresponding CalculateMVCCoordinates method
//	@param numPoints number of points whose relationship to inputMesh was previously computed by CalculateMVCCoordinates
//	@inWeights MVC coordinates of points whose attributes are to be reconstructed
//	@outAttr where the reconstructed attributes should be stored
//	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated.
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const vtkDataArray* inputAttrs, 
	int numPoints, const double* inWeights, vtkDataArray* outAttrs)
{
	outAttrs->SetNumberOfComponents(const_cast<vtkDataArray*>(inputAttrs)->GetNumberOfComponents());
	outAttrs->SetNumberOfTuples(numPoints);

	switch (outAttrs->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT(inputAttrs, numPoints, inWeights, ( VTK_TT *)(outAttrs->GetVoidPointer(0))));
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
@param numAttrs number of attributes in one tuple (e.g., 3)
@param numPoints number of points whose attributes are to be reconstructed.
@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
@param outAttrs where attributes are to be stored (arranged in tuples)
N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const double* inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double* inWeights, double* outAttrs)
{
	ReconstructAttributesT(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights, outAttrs);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
//	@param inMeshAttrs an array of attributes (e..g, (x1, y1, z1), (x2, y2, z2), ...(xn, yn, zn))
//	@param numMeshPoints number of entries in inMeshAttr, i.e., number of vertices of the mesh
//	@param numAttrs number of attributes in one row of inMeshAttrs (e.g., 3)
//	@param numPoints number of points whose attributes are to be reconstructed.
//	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//	@param outAttrs where attributes are to be stored (one row for each point)
//	N.B., buffer outAttrs must be capable of holding numPoints entries of numAttrs values. 
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const double** inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double* inWeights, double** outAttrs)
{
	// for each original vertex
#pragma omp parallel shared(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights)
	{
#pragma omp for
		for (int i = 0; i < numPoints; i++)
		{
			//get the point in the original mesh						
			const double* ptWeights = inWeights + i*numMeshPoints;

			//and reconstructs the coordinates (attributes)
			ReconstructAttributesForPointT_2(inMeshAttrs, numMeshPoints, numAttrs, ptWeights, outAttrs[i]);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
//@param inputAttrs attributes of the original mesh
//@param numPoints number of points whose attributes are to be reconstructed.
//@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//@param outAttrs where attributes are to be stored (arranged in tuples)
//N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values.
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesT(const vtkDataArray* inputAttrs, 
	int numPoints, const double* inWeights, T* outAttrs)
{
	vtkDataArray* inAttrs = const_cast<vtkDataArray*>(inputAttrs);	
	switch (inAttrs->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT((const VTK_TT*)(inAttrs->GetVoidPointer(0)), 
			inAttrs->GetNumberOfTuples(), inAttrs->GetNumberOfComponents(), numPoints, inWeights, outAttrs));	
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
//@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
//@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one tuple (e.g., 3)
//@param numPoints number of points whose attributes are to be reconstructed.
//@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//@param outAttrs where attributes are to be stored (arranged in tuples)
//N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. 
template < typename T1, typename T2 >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesT(const T1* inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double* inWeights, T2* outAttrs)
{
	// for each original vertex
#pragma omp parallel shared(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights)
	{
#pragma omp for
		for (int i = 0; i < numPoints; i++)
		{
			//get the point in the original mesh			
			T2* point = outAttrs + i*numAttrs;
			const double* ptWeights = inWeights + i*numMeshPoints;

			//and reconstructs the coordinates (attributes)
			ReconstructAttributesForPointT(inMeshAttrs, numMeshPoints, numAttrs, ptWeights, point);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs attributes of a single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates of this point.
//@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
//@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one tuple (e.g., 3)
//@param inWeights MVC coordinates of the point whose attributes are to be reconstructed.
//@param outAttrs where attributes are to be stored
//N.B., buffer outAttrs must be capable of holding numAttrs values. 
template < typename T1, typename T2 >
/*static*/  void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesForPointT(
	const T1* inMeshAttrs, int numMeshPoints, int numAttrs, const double* inWeights, T2* outAttrs)
{
	const T1* pCurMeshAttr = inMeshAttrs;
	for (int iAttr = 0; iAttr < numAttrs; iAttr++)
	{
		outAttrs[iAttr] = (T2)((*pCurMeshAttr)*inWeights[0]);
		pCurMeshAttr++;
	}

	for (int i = 1; i < numMeshPoints; i++)
	{
		for (int iAttr = 0; iAttr < numAttrs; iAttr++)
		{
			outAttrs[iAttr] += (T2)((*pCurMeshAttr)*inWeights[i]);
			pCurMeshAttr++;
		}		
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs attributes of a single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates of this point.
//@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
//@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one tuple (e.g., 3)
//@param inWeights MVC coordinates of the point whose attributes are to be reconstructed.
//@param outAttrs where attributes are to be stored
//N.B., buffer outAttrs must be capable of holding numAttrs values. 
template < typename T1, typename T2 >
/*static*/  void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesForPointT_2(
	const T1** inMeshAttrs, int numMeshPoints, int numAttrs, const double* inWeights, T2* outAttrs)
{
	const T1* pCurMeshAttr = inMeshAttrs[0];
	for (int iAttr = 0; iAttr < numAttrs; iAttr++)
	{
		outAttrs[iAttr] = (T2)((*pCurMeshAttr)*inWeights[0]);
		pCurMeshAttr++;
	}

	for (int i = 1; i < numMeshPoints; i++)
	{
		pCurMeshAttr = inMeshAttrs[i];

		for (int iAttr = 0; iAttr < numAttrs; iAttr++)
		{
			outAttrs[iAttr] += (T2)((*pCurMeshAttr)*inWeights[i]);
			pCurMeshAttr++;
		}		
	}
}
#pragma endregion

#pragma region TwoDimensional Weights
//---------------------------------------------------------------------------------------------------------------------------------------------------------
///Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
//@param inputAttrs attributes of vertices of mesh topologically compatible with the mesh specified in the call of corresponding CalculateMVCCoordinates method
//@param numPoints number of points whose relationship to inputMesh was previously computed by CalculateMVCCoordinates
//@inWeights MVC coordinates of points whose attributes are to be reconstructed
//@outAttr where the reconstructed attributes should be stored
//N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const vtkDataArray* inputAttrs, 
	int numPoints, const double** inWeights, vtkDataArray* outAttrs)
{
	outAttrs->SetNumberOfComponents(const_cast<vtkDataArray*>(inputAttrs)->GetNumberOfComponents());
	outAttrs->SetNumberOfTuples(numPoints);

	switch (outAttrs->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT(inputAttrs, numPoints, inWeights, ( VTK_TT *)(outAttrs->GetVoidPointer(0))));
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
//@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
//@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one tuple (e.g., 3)
//@param numPoints number of points whose attributes are to be reconstructed.
//@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//@param outAttrs where attributes are to be stored (arranged in tuples)
//N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values.
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const double* inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double** inWeights, double* outAttrs)
{
	ReconstructAttributesT(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights, outAttrs);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
//@param inMeshAttrs an array of attributes (e..g, (x1, y1, z1), (x2, y2, z2), ...(xn, yn, zn))
//@param numMeshPoints number of entries in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one row of inMeshAttrs (e.g., 3)
//@param numPoints number of points whose attributes are to be reconstructed.
//@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//@param outAttrs where attributes are to be stored (one row for each point)
//N.B., buffer outAttrs must be capable of holding numPoints entries of numAttrs values.
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes(const double** inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double** inWeights, double** outAttrs)
{
	// for each original vertex
#pragma omp parallel shared(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights)
	{
#pragma omp for
		for (int i = 0; i < numPoints; i++)
		{
			//get the point in the original mesh						
			const double* ptWeights = inWeights[i];

			//and reconstructs the coordinates (attributes)
			ReconstructAttributesForPointT_2(inMeshAttrs, numMeshPoints, numAttrs, ptWeights, outAttrs[i]);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
/** Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
@param inputAttrs attributes of the original mesh
@param numPoints number of points whose attributes are to be reconstructed.
@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
@param outAttrs where attributes are to be stored (arranged in tuples)
N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
template < typename T >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesT(const vtkDataArray* inputAttrs, 
	int numPoints, const double** inWeights, T* outAttrs)
{
	vtkDataArray* inAttrs = const_cast<vtkDataArray*>(inputAttrs);	
	switch (inAttrs->GetDataType())  
	{ 
		vtkTemplateMacro( ReconstructAttributesT((const VTK_TT*)(inAttrs->GetVoidPointer(0)), 
			inAttrs->GetNumberOfTuples(), inAttrs->GetNumberOfComponents(), numPoints, inWeights, outAttrs));	
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
//@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
//@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
//@param numAttrs number of attributes in one tuple (e.g., 3)
//@param numPoints number of points whose attributes are to be reconstructed.
//@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
//@param outAttrs where attributes are to be stored (arranged in tuples)
//N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values.
template < typename T1, typename T2 >
/*static*/ void vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributesT(const T1* inMeshAttrs, int numMeshPoints, int numAttrs, 
	int numPoints, const double** inWeights, T2* outAttrs)
{
		// for each original vertex
#pragma omp parallel shared(inMeshAttrs, numMeshPoints, numAttrs, numPoints, inWeights)
	{
#pragma omp for
		for (int i = 0; i < numPoints; i++)
		{
			//get the point in the original mesh			
			T2* point = outAttrs + i*numAttrs;
			const double* ptWeights = inWeights[i];

			//and reconstructs the coordinates (attributes)
			ReconstructAttributesForPointT(inMeshAttrs, numMeshPoints, numAttrs, ptWeights, point);
		}
	}
}

#pragma endregion
#pragma endregion