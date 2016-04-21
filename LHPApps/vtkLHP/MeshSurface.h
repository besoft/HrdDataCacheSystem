/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshSurface.h,v $ 
  Language: C++ 
  Date: $Date: 2012-04-17 16:54:21 $ 
  Version: $Revision: 1.1.2.12 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef MeshSurface_h__
#define MeshSurface_h__
 
#pragma once

// std
#include <cmath>
#include <float.h>
#include <set>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER >= 1500
#pragma message("Make sure that the project is built with /openmp")
#include <omp.h> // compile with: /openmp 
#endif

#define ISFINITE(w) ((w) >= -DBL_MAX && (w) <= DBL_MAX)

//vtk
#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCell.h"
#include "vtkGenericCell.h"
#include "vtkProgressiveHull.h"
#include "vtkDecimatePro.h"
#include "vtkDecimate.h"
#include "vtkQuadricDecimation.h"
#include "vtkQuadricclustering.h"
#include "vtkPolyDataNormals.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPointData.h"
#include "vtkDataSetAttributes.h"
#include "vtkFloatArray.h"
#pragma warning(pop)

// my classes
#include "PKUtils.h"
#include "OoCylinder.h"
#include "MeshNavigator.h"

using namespace std;

//////
/// Maximum ignorable distance from mesh.
//////
#define MESH_DISTANCE_EPSILON 0.000000001 //0.000001

//////
/// Offset used to enlarge mesh in process of coarse mesh creation.
//////
#define COARSE_MESH_ENLARGEMENT_OFFSET 10

//////
/// Class accepts original mesh, creates coarse mesh a determines relations for transformation between them.
//////
class MeshSurface
{
public:
	//////
	/// Creates new instance with specified detail mesh. Generates coarse mesh
	/// with specific reduction ratio.
	/// @param original original detail mesh (input)
	/// @param coarse original coarse mesh (optional input)
	/// @reductionRatio 0 to 1 ratio of decimated vertices from original mesh (input)
	///  @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
	//////
	MeshSurface(vtkPolyData *original, bool muscle = false, vtkPolyData *coarse = NULL, float reductionRatio = 0.1, bool bUseProgressiveHull = false);

	//////
	/// Creates new instance with specified detail mesh. Generates coarse mesh
	/// with specific number of vertices.
	/// @param original original detail mesh (input)
	/// @param coarse original coarse mesh (optional input)
	/// @coarseVertexCount number of vertices for coarse mesh (input)
	///  @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
	//////
	MeshSurface(vtkPolyData *original, bool muscle = false, vtkPolyData *coarse = NULL, int coarseVertexCount = 300, bool bUseProgressiveHull = false);

	//////
	/// Destroys instance, deallocates memory.
	//////
	~MeshSurface(void);

	//////
	/// Initializes new instance with specified detail mesh. Generates coarse mesh
	/// with specific number of vertices.
	/// @param original original detail mesh (input)
	/// @param coarse original coarse mesh (optional input)
	/// @coarseVertexCount number of vertices for coarse mesh (input)
	///  @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
	//////
	void InitInstance(vtkPolyData *original, bool muscle, vtkPolyData *coarse = NULL, int coarseMeshSize = 300, bool bUseProgressiveHull = false);

	//////
	/// Deeply clones this instance.
	/// @return deep copy of this instance
	//////
	MeshSurface* Clone();

	//////
	/// Gets soft reference to original mesh.
	/// @return soft reference to original mesh
	//////
	vtkPolyData* GetOriginal();

	//////
	/// Gets soft reference to coarse mesh.
	/// @return soft reference to coarse mesh
	//////
	vtkPolyData* GetCoarse();

	//////
	/// Gets soft reference to object oriented bounding cylinder.
	/// @return soft reference to bounding cylinder
	//////
	OoCylinder* GetOoCylinder();

	//////
	/// Gets soft reference to mesh navigator
	/// @return soft reference to mesh navigator
	//////
	MeshNavigator* GetNavigator();

	//////
	/// Sets flag if mesh deformable.
	/// @param true for deformable
	//////
	void SetDeformable(bool deformable);

	//////
	/// Gets flag if mesh deformable.
	/// @return true for deformable
	//////
	bool GetDeformable();

	//////
	/// Distributes values specified for all points of coarse mesh to array of values for all vertices of original mesh
	/// according to its barycentric coordinates in respect to coarse mesh.
	/// @param coarseAttributes matrix with row for every coarse vertex and column for every distributed value of every vertex (input)
	/// @param origAttributes preallocated matrix with row for every vertex of original mesh and column for every distributed value (output)
	//////
	void DistributeCoarseAttributesToOriginal(PKMatrix *coarseAttributes, PKMatrix *origAttributes);

	//////
	/// Copies coordinates to mesh to change positions of vertices.
	/// @param mesh mesh with vertices to be repositioned (input/output)
	/// @param points nx3 matrix with new coordinates for vertices (input)
	//////
	void SetUpPointCoords(vtkPolyData *mesh, PKMatrix *points);

	//////
	/// Gets array of neigbouring vetices for one vertex of mesh sorted so they create fan around specified point.
	/// @param polyData mesh with topology (input)
	/// @param pointId id of central point (input)
	/// @param neighbourIds array of neigbouring vertices ids (not to be preallocated) (output)
	/// @param neighbourIdsCount length of output array => number of neighbours (output)
	/// @param bufferHigherResPoints if not NULL, this must be a matrix this->GetOriginal()->GetNumberOfPoints() x 3, 
	/// which is used to speed-up the process of repetitive calling of this method.
	//////
	void ApplyCoarseCoordsToOriginalMesh(PKMatrix *coarseCoords, PKMatrix *bufferHigherResPoints);

	/////////////////////
	/// Support static methods
	/////////////////////

	//////
	/// Calculates linear decomposition of point coordinates to mesh vertices.
	/// Refers to Mean Value Coordinates for Closed Triangular Meshes, Tao Ju, Scott Schaefer, Joe Warren, Rice University
	/// @param pPoly mesh with topology and coordinates (input)
	/// @param point 3D coordinates of decomposed vertex (input)
	/// @param coords preallocated array of length of number of vertices of mesh for final coefficients of linear decomposition (output)
	//////
	void static CalculateMeshCoordsForPoint(vtkPolyData* pPoly, double point[3], double* coords);

	//////
	/// Calculates normals for mesh vertices.
	/// @param mesh input mesh (input)
	/// @param normals preallocated nx3 matrix for normals (output)
	//////
	void static CalculateNormals(vtkPolyData *mesh, PKMatrix *normals);

	//////
	/// Gets array of neigbouring vetices for one vertex of mesh.
	/// @param polyData mesh with topology (input)
	/// @param pointId id of central point (input)
	/// @param neighbourIds array of neigbouring vertices ids (not to be preallocated) (output)
	/// @param neighbourIdsCount length of output array => number of neighbours (output)
	//////
	void static GetPointNeighbours(vtkPolyData* polyData, vtkIdType pointId, vtkIdType *&neighbourIds, vtkIdType &neighbourIdsCount);

	void static GetPointNeighboursSorted(vtkPolyData* polyData, vtkIdType pointId, vtkIdType *&neighbourIds, vtkIdType &neighbourIdsCount);

	//////
	/// Gets third apexes of common triangles of two mesh vertices. There is expectation of existence of exactly one
	/// edge between these vertices and therefore for "normal" closed meshed two incident triangles.
	/// @param polyData mesh with topology (input)
	/// @param pointId1 id of first point (input)
	/// @param pointId2 id of second point (input)
	/// @param cellId1 id of missing apex of first triangle (output)
	/// @param cellId2 id of missing apex of first triangle (output)
	//////
	void static GetPointsTriangleVertices(vtkPolyData* polyData, vtkIdType pointId1, vtkIdType pointId2, vtkIdType* vertexId1, vtkIdType* vertexId2);

	void static AddMeshOffset(vtkPolyData *mesh, PKMatrix *offset);

	double static FindIntersectionInDirection(double start[3], double direction[3], vtkPolyData* mesh, PKMatrix* points = NULL, vtkIdType* triangleId = NULL, vtkGenericCell* cellTemp = NULL);

private:
	MeshSurface(void);
	
	//////
	/// Creates coarse control mesh by reduction of original high detail mesh.
	/// @param source mesh to reduce (input)
	/// @param minimalizationFactor ratio of reduction from 0 to 1 (input)
	//////
	void CreateCoarseMesh(vtkPolyData* source, int coarseMeshSize);

	//////
	/// Enlarges mesh by shifting by normals.
	/// @param mesh mesh for enlargement (input/ouput)
	/// @param shiftLength length of shift for every vertex (input)
	//////
	static void EnlargeMesh(vtkPolyData* mesh, double shiftLength);
	static void ShiftPointSafely(vtkIdType pointId, vtkPolyData *mesh, double direction[3]);

	//////
	/// Calculates barycentric coordinates of all original vertices in coarse mesh and
	/// stores them to internal coarseToOrigRel matrix.
	/// Uses OpenMP for parallelization.
	//////
	void CalculateRelations();

	//**********************************************************************************************************************
	// attributes
	//**********************************************************************************************************************

	//////
	/// Original mesh.
	//////
	vtkPolyData *original;

	//////
	/// Auto generated coarse mesh.
	//////
	vtkPolyData *coarse;

	//////
	/// nOrig x nCoarse matrix of barycentric coordinates of the original mesh in the coarse one. Every row
	///	contains coordinates of one original vertex.
	//////
	PKMatrix *coarseToOrigRel; // orig x coarse

	OoCylinder *ooCylinder;
	MeshNavigator *navigator;
	bool deformable;

	bool m_UseProgressiveHull;	///<true, if progressive hull should be used

	bool muscle;  // true if MeshSurface represents muscle, false if bone
};

#endif

