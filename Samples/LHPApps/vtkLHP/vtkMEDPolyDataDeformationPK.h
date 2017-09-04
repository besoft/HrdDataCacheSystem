/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkMEDPolyDataDeformationPK.h,v $ 
  Language: C++ 
  Date: $Date: 2012-04-16 06:42:25 $ 
  Version: $Revision: 1.1.2.11 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef vtkMEDPolyDataDeformationPK_h__
#define vtkMEDPolyDataDeformationPK_h__
 
#pragma once

// std lib
#include <cmath>
#include <ctime>

// VTK
#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkPolyData.h"
#include "vtkCell.h"
#include "vtkMath.h"
#include "vtkPolyDataToPolyDataFilter.h"
#include "vtkObjectFactory.h"
#include "vtkAppendPolyData.h"
#pragma warning(pop)

// My code - utility classes
#include "IMultiMeshDeformer.h"
#include "PKUtils.h"
#include "PKMath.h"
#include "MeshSurface.h"
#include "MeshSkeleton.h"
#include "DeformationCache.h"
#include "PKSolutionState.h"
#include "PKSolutionStateGPU.h"
#include "OoCylinder.h"
#include "collisiondetection.h"
#include "GPU_Solver.h"

#include "mafDllMacros.h"
#include "vtkLHPConfigure.h"

typedef struct { // structure for vertex visualization data representation
  Vector3_ point; // original vertex coordinates
  Vector3_ dir;   // direction of motion
  double bestT;   // distance of motion
  Vector3_ color; // color for new position visualization
} DirDesc;

EXPORT_STL_VECTOR(VTK_vtkLHP_EXPORT,DirDesc)


using namespace std;

//////
/// Estimated count of neighbours per vertex for std::vector initialisation.
//////
#define ESTIMATED_PT_NEIGH_COUNT 6

//////
/// Number of vertices in coarse mesh. Upper boundary.
//////
#define COARSE_MESH_SIZE 250

//////
/// Maximum difference between two steps of GN iteration process, that is accepted
/// for termination.
//////
#define SOLUTION_EPSILON 0.01

typedef struct {
	vtkPolyData *mesh;
	PKMatrix *points;
	PKMatrix *fix;
	PKMatrix *movement;
	bool deformable;
	MeshNavigator *navigator;
	unsigned int* verticesInside;
	unsigned int* verticesUndecided;
} tIntersectionContext;


//////
/// Custom made VTK filter for skeleton driven mesh deformation.
//////
class VTK_vtkLHP_EXPORT vtkMEDPolyDataDeformationPK : public vtkPolyDataToPolyDataFilter, public IMultiMeshDeformer
{

#pragma region // my code
#pragma region //methods
public:
	
	//*************************************************************************************************************************************
	// Main method
	//*************************************************************************************************************************************

	//////
	/// Synchronous execution of deformation.
	///
	/// @return true no error occured, if false results may not be relevant
	//////
	/*virtual*/ bool ExecuteMultiData();
	
	//*************************************************************************************************************************************
	// Deformable meshes
	//*************************************************************************************************************************************

	/*virtual*/ void AddWrapper(vtkPolyData* w);
	/*virtual*/ void FreeWrappers();

	//////
	/// Sets number of meshes for deformation (deformable inputs).
	///
	/// Used for memory preallocation.
	/// @param count number of meshes for deformation (input)
	//////
	/*virtual*/ void SetNumberOfMeshes(int count);

	//////
	/// Gets number of meshes for deformation (deformable inputs).
	///
	/// Used for memory preallocation.
	/// @return number of meshes for deformation
	//////
	/*virtual*/ int GetNumberOfMeshes();
	
	//////
	/// Sets input deformable mesh at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	/*virtual*/ void SetInputMesh(int meshIndex, vtkPolyData *poly);
	
	//////
	/// Gets input deformable mesh at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return poly vtkPolyData - soft reference - must not be freed outside
	//////
	/*virtual*/ vtkPolyData* GetInputMesh(int meshIndex);
	
	//////
	/// Sets output for result of deformable mesh at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
	//////
	/*virtual*/ void SetOutputMesh(int meshIndex, vtkPolyData *poly);

	//////
	/// Sets output for result of deformable mesh coarse at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
	//////
	/*virtual*/ void SetOutputMeshCoarse(int meshIndex, vtkPolyData *poly);


	//////
	/// Gets output for result of deformable mesh at specified index deformation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return poly vtkPolyData - soft reference
	//////
	/*virtual*/ vtkPolyData* GetOutputMesh(int meshIndex);

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
	/*virtual*/ void SetNumberOfMeshSkeletons(int meshIndex, int count);

	//////
	/// Gets number of skeletons for deformable mesh at specified index.
	///
	/// Used for memory preallocation.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return count number of skeletons (input)
	//////
	/*virtual*/ int GetNumberOfMeshSkeletons(int meshIndex);

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
	/*virtual*/ void SetMeshSkeleton(int meshIndex, int skeletonIndex, vtkPolyData* original, vtkPolyData* modified, vtkIdList* correspondence = NULL, double* original_rso = NULL, double* modified_rso = NULL);
	
	//*************************************************************************************************************************************
	// Hard obstacles (bones)
	//*************************************************************************************************************************************
	
	//////
	/// Sets number of meshes used as hard obstacles (non-deformable inputs).
	///
	/// Used for memory preallocation.
	/// @param count number of meshes as obstacles (input)
	//////
	/*virtual*/ void SetNumberOfObstacles(int count);

	//////
	/// Gets number of meshes used as hard obstacles (non-deformable inputs).
	///
	/// Used for memory preallocation.
	/// @return number of meshes as obstacles
	//////
	/*virtual*/ int GetNumberOfObstacles();

	//////
	/// Sets input non-deformable mesh (obstacle) at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
	//////
	/*virtual*/ void SetObstacle(int obstacleIndex, vtkPolyData *poly);

	//////
	/// Gets input non-deformable mesh (obstacle) at specified index.
	///
	/// @param meshIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @return poly vtkPolyData - soft reference - must not be freed outside
	//////
	/*virtual*/ vtkPolyData* GetObstacle(int obstacleIndex);

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
	void SetCoarseMesh(const int meshIndex, vtkPolyData* coarse);

	//////
	/// Gets coarse mesh for deformable mesh at specified index.
	/// This can be mesh from matching setter or calculated mesh.
	/// Make deep copy of returned pointer for your needs.
	///
	/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
	/// @return vtkPolyData - soft reference - must not be freed outside
	//////
	vtkPolyData* GetCoarseMesh(const int meshIndex);


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
	void SetCoarseObstacle(const int obstacleIndex, vtkPolyData* coarse);

	//////
	/// Gets coarse mesh for obstacle mesh at specified index.
	/// This can be mesh from matching setter or calculated mesh.
	/// Make deep copy of returned pointer for your needs.
	///
	/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
	/// @return vtkPolyData - soft reference - must not be freed outside
	//////
	vtkPolyData* GetCoarseObstacle(const int obstacleIndex);


	//*************************************************************************************************************************************
	// Settings
	//*************************************************************************************************************************************

	//////
	/// Sets whetter to prevent collisions.
	/// With true, intersections of two meshes should be prevented at the price
	/// of high computational demands.
	/// On false settings, all hard obstacles are ignored as well.
	///
	/// @param prevent true to prevent collisions
	//////
	/*virtual*/ void SetPreventCollisions(bool prevent);

	//////
	/// Gets whetter to prevent collisions.
	/// With true, intersections of two meshes should be prevented at the price
	/// of high computational demands.
	/// On false settings, all hard obstacles are ignored as well.
	///
	/// @return true if prevents collisions
	//////
	/*virtual*/ bool GetPreventCollisions();

	//////
	/// Sets if progressive hulls should be used to represent coarse meshes
	/// With true, much slower calculation is used to produce progressive hull 
	/// that have a tendency to be more stable	
	///
	/// @param enabled true, if progressive hulls should be used
	//////
	/*virtual*/ void SetUseProgressiveHulls(bool enabled);

	//////
	/// Gets if progressive hulls should be used to represent coarse meshes
	/// With true, much slower calculation is used to produce progressive hull 
	/// that have a tendency to be more stable	
	///
	/// @return true if progressive hulls should be used
	//////
	/*virtual*/ bool GetUseProgressiveHulls();

	//////
	/// Sets if grid will be used to boost the ray casting.
	///
	/// @param enabled true, if grid will be used to boost the ray casting
	//////
	void SetUseGrid(bool enabled);

	//////
	/// Gets if grid will be used to boost the ray casting.
	///
	/// @return true, if grid will be used to boost the ray casting
	//////
	bool GetUseGrid();

	
	//////
	/// Sets if the method should visualize its progress
	///
	/// @param enabled true, if the method should visualize its progress
	//////
	void SetDebugMode(bool enabled);

	//////
	/// Gets if grid will be used to boost the ray casting.
	///
	/// @return true, if debug mode is on and the method will visualize its progress
	//////
	bool GetDebugMode();

protected:
	//*************************************************************************************************************************************
	// kernel
	//*************************************************************************************************************************************

	//////
	/// Calculates laplacian matrix and vertex of differential coordinates for every vertex of coarse mesh.
	/// @param laplMatrix pointer to laplacian matrix - not to be preallocated (output)
	/// @param laplOperator pointer to vector of differential coordinates - not to be preallocated (output)
	//////
	void CalculateLaplacian(PKMatrix **laplMatrix, PKMatrix** laplOperator, int modelIndex);

	//////
	/// Calculates skeleton matrix for defining of skeleton using mesh vertices and final position for deformed skeleton points.
	/// Output are barycentric coordinates of skeleton vertices in coarse mesh and new Euclid coordinates of the same vertices.
	/// @param skeletonMatrix pointer to skeleton matrix with decomposition of bones to linear combination coeficient of mesh vertices - not to be preallocated (output)
	/// @param laplOperator pointer to vector of deformed coordinates for skeleton points - not to be preallocated (output)
	//////
	void CalculateSkeletonMatrix(PKMatrix **skeletonMatrix, PKMatrix **skeletonVector, int modelIndex);

	//////
	/// Minimizes linear system with hard constraint using Gauss-Newton iterative method with Lagrange multiplicators.
	/// Uses modified approach from Huang 2006, Subspace Gradient Domain Mesh Deformation
	/// @param matrixL matrix L of linear system (input)
	/// @param vectorB vector of right sides of system (input)
	/// @param solutionX pointer to vector of starting X (input) and then solution (output) - must be preallocated and should be prefilled (input and output)
	//////
	bool FindEnergyMinimumGN(PKMatrix **matricesL, PKMatrix **verticesB, PKMatrix ***solutionsX);
	void PrintPKMatrix(PKMatrix *matrix, char* path);
	void PrintGPUMatrix(double *matrix, int width, int height, char* path);
	void GPUMatrixToPKMatrix(double *gp, PKMatrix *matrix);
	bool FindEnergyMinimumGPU(PKMatrix **matricesL, PKMatrix **verticesB, PKMatrix ***solutionsX);
	
	//////
	/// Calculates oriented volume of mesh defined by topology and external point coordinates.
	/// @param polyData information about topology of mesh and optionally coordinates of vertices (input)
	/// @param alternativeCoords array of actual point coordinates to use instead of coords in mesh or NULL to use data in polyData (input)
	/// @return oriented volume of mesh
	//////
	double CalculateVolume(vtkPolyData *polyData, PKMatrix *alternativeCoords);

	/// Calculates the oriented volume of intersected part of the mesh identified by cellsA.
	/// @param polyData information about topology of mesh and optionally coordinates of vertices (input)
	/// @param points array of actual point coordinates to use instead of coords in mesh or NULL to use data in polyData (input)
	/// @param cellsA contains cells of the input mesh that lie inside the volume of the other mesh
	/// @return oriented volume of mesh (of the part  identified by intersectedVertices)
	/// The method computes the oriented volume of the object composed of tetrahedras, 
	/// each of them formed from one triangle of the surface that intersects and point (0,0,0). 
	double CalculateIntersectedVolume(vtkPolyData* mesh, PKMatrix* points, const PKHashTable<vtkIdType, vtkIdType>* cellsA);


	//////
	/// Calculates transposed Jacobi matrix of volume function.
	/// Matrix is transposed for faster access to memory. It means, that in every row, there are three derivations of volume function,
	/// one for each coordinate of specified vertex. There are as many rows as vertices in mesh. As a result, you get
	/// nPoints x 3 matrix. To get well defined Jacobi matrix of volume, you may need to linearize content of this matrix row by row, 
	/// as many calculations may require this to be gradient vector instead, as volume function is scalar in the first place.
	/// @param alternativeCoords optional array of actual point coordinates used instead of coords in mesh. NULL accepted, then mesh coords are used. (input)
	/// @param jacobiMatrix preallocated Jacobi matrix (output)
	//////
	void CalculateVolumeJacobiMatrixTransposed(vtkPolyData *mesh, PKMatrix *alternativeCoords, PKMatrix *jacobiMatrix);

	//////
	/// Calculates vertex relation weight for bone segments of skeleton.
	/// They are used for correct rotation of laplacian operator.
	/// Results kept in internal structures of this class.
	//////
	void CalculateVertexWeightsToSkeleton();

	//////
	/// Creates or loads resources for current deformation.
	/// @param originalMesh original mesh to be deformed (input)
	/// @param matrixL pointer to pointer to matrix of linear conditions of deformation, not to be preallocated (output)
	/// @param vectorB pointer to pointer to vector of right side of linear conditions of deformation, not to be preallocated (output)
	/// @param useCache flag of cache usage, on true tries to retrieve data from cache from previous deformation first or
	///                 calculates new and then stores copy to cache. On false always calculates all resources from scratch.
	//////
	void SetUpResources(PKMatrix ***matricesL, PKMatrix ***verticesB, bool useCache);

	
	/** Finds the vertices of contextActiveMesh mesh that lies inside the volume of the contextPassive mesh 
	Indices of the detected vertices are stored into the output array */
	void FindIntersectingVertices(const tIntersectionContext *contextA, const tIntersectionContext *contextB, 
		std::vector<vtkIdType>& intersectedVertices);

	/** Returns cells of the input mesh that have at least one vertex from intersectedVertices.
	The method retrieves a fan of triangles around every vertex from the input array intersectedVertices and stores their cell ids
	into the return buffer. Any duplicity is automatically filtered out, i.e., it is guaranteed that the returned triangles are unique.
	This method is useful (and designed) to return cells of the mesh that lie in the volume of another mesh. Vertices that lie in the
	volume of another mesh are typically obtained by FindIntersectingVertices method.
	N.B.: the caller is responsible for deletion of the returned object once it is no longer needed. */
	PKHashTable<vtkIdType, vtkIdType>* GetIntersectionSurfaceCells(vtkPolyData* mesh, const std::vector<vtkIdType>& intersectedVertices);

	double FindAndFixIntersection(tIntersectionContext *contextA, tIntersectionContext *contextB);
	double FindAndFixIntersectionStatic(tIntersectionContext *contextA, tIntersectionContext *contextB);
	double FindAndFixIntersectionDynamic(tIntersectionContext *contextA, tIntersectionContext *contextB);

	void FixFinalIntersections(MeshSurface **meshes, int meshCount);	

	//*************************************************************************************************************************************
	// utility methods
	//*************************************************************************************************************************************	

	//////
	/// Gets common cells of two vertices of mesh. There is expectation of existence of exactly one
	/// edge between these vertices and therefore for "normal" closed meshed two incident polygons (cells).
	/// @param polyData mesh with topology (input)
	/// @param pointId1 id of first point (input)
	/// @param pointId2 id of second point (input)
	/// @param cellId1 id of first common cell (output)
	/// @param cellId2 id of second common cell (output)
	//////
	void GetPointsCells(vtkPolyData* polyData, vtkIdType pointId1, vtkIdType pointId2, vtkIdType* cellId1, vtkIdType* cellId2);

	//////
	/// Calculates area of triangle determined by three points.
	/// @param a 3 coordinates of first apex (input)
	/// @param b 3 coordinates of second apex (input)
	/// @param c 3 coordinates of first apex (input)
	/// @return area of triangle
	//////
	double CalculateTriangleArea(double a[3], double b[3], double x[3]);

	//////
	/// Rotates differential vertices of mesh according to change of skeleton shape.
	/// Uses skeleton attribute to determine parameters of rotation. Keeps lengths of original vertices.
	/// @param laplOperator differetial coordinates to rotate (input and output)
	/// @param result of rotation (rotated laplacian operator) - must be preallocated, can be same as laplOperator (output)
	//////
	void RotateLaplacOperator(PKMatrix *laplOperator, PKMatrix *result, int modelIndex);

	void ExportMeshes(vtkPolyData *output);
	int GetInputs(vtkPolyData ***inputMeshes, CONTROL_SKELETON*** inputSkeletons, int **skeletonCounts);

#pragma region // inlines
	
#pragma endregion // inlines
#pragma endregion //methods

#pragma region //variables
protected:
	//////
	/// Currently processed mesh. May be NULL.
	//////
	MeshSurface** meshes;

	//////
	/// Currently processed skeleton. May be NULL.
	//////
	MeshSkeleton** skeletons;

	//////
	/// List of nearest bone ids, one value for each mesh vertex.
	//////
	int** nearestBonesPerVertex;
	
	//////
	/// Matrix of weights of bones for each vertex of mesh.
	/// Each row contains weights to all bones for single mesh vertex.
	/// Weights are normed to range <0; 1>, where 1 is nearest vertex,
	/// 0 is furthest, and values between are lineary interpolated.
	//////
	PKMatrix** vertexWeightsToBones;

	int modelCount;

	volatile static int cdErrors;
	volatile static int cdTests;

#pragma region //IMultiMeshDeformer
	int *skeletonCounts;
	int obstacleCount;

	vtkPolyData **inputMeshes;
	vtkPolyData **inputMeshesCoarse;
	vtkPolyData **outputMeshes;
	vtkPolyData **outputMeshesCoarse;
	CONTROL_SKELETON** multiM_Skeletons;

	vtkPolyData **obstacles;
	vtkPolyData **obstaclesCoarse;

	bool preventCollisions;
	bool useProgresiveHulls;
	bool useGrid;
	bool debugMode;	

#pragma endregion //IMultiMeshDeformer


#pragma endregion //variables
#pragma endregion // my code


#pragma region // copied code
#pragma region //methods
public:
	static vtkMEDPolyDataDeformationPK *New();

	vtkTypeRevisionMacro(vtkMEDPolyDataDeformationPK, vtkPolyDataToPolyDataFilter); 

	/** Get the number of control curves. */
	inline virtual int GetNumberOfSkeletons() {
		return m_NumberOfSkeletons;
	}

	/** Sets the number of control skeletons.  
	Old skeletons are copied (and preserved) */
	virtual void SetNumberOfSkeletons(int nCount);

	/** Specifies the n-th control skeleton. 
	If RSO points are specified, they are used during the computation of LFs
	of curves of both skeletons. A local fame is defined by its origin point 
	and three vectors u, v and w. Vector u is the tangent vector (it goes in
	the direction of polyline) and vectors v,w are perpendicular to this vector.
	As there is infinite number of u,v,w configurations, the algorithm uses the
	given RSO point to get a unique one (v lies in the plane defined by u and RSO). 
	If RSO is not specified, v is chosen to lie in the plane closest to the u vector.
	When RSO points are not specified (or they are specified incorrectly), 
	the deformed object might be unrealistically rotated against other objects 
	in the scene, if the skeleton of object to deform tends to rotate (simple edge, 
	or only one skeleton for object). */
	virtual void SetNthSkeleton(int idx, vtkPolyData* original, 
		vtkPolyData* modified, vtkIdList* correspondence = NULL, 
		double* original_rso = NULL, double* modified_rso = NULL);

	/** Return this object's modified time. */  
	/*virtual*/ unsigned long int GetMTime();

protected:
	vtkMEDPolyDataDeformationPK(void);
	virtual ~vtkMEDPolyDataDeformationPK(void);

	/** 
	By default, UpdateInformation calls this method to copy information
	unmodified from the input to the output.*/
	/*virtual*/void ExecuteInformation();

	/**
	This method is the one that should be used by subclasses, right now the 
	default implementation is to call the backwards compatibility method */
	/*virtual*/void ExecuteData(vtkDataObject *output);

	typedef std::vector<DirDesc> dirList; // list of visualized points
	/**
	 This method visualizes the progress of the method for debugging purposes.
	 It is supposed to be called from FindEnergyMinimumGN method */
	void Debug_Visualize_Progress( int iterNum, double energy, PKSolutionState ** states, dirList* directions);
	/**
	 This method visualizes the progress of the method for debugging purposes.
	 It is supposed to be called from FindEnergyMinimumGPU method */
	void Debug_Visualize_ProgressGPU( int iterNum, double energy, PKSolutionStateGPU ** states, dirList* directions);
	/**
	Keypress callback function for vtkMEDPolyDataDeformationPK_DBG window*/
	static void KeypressCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData ) ;
	/**
	Mouse callback function for vtkMEDPolyDataDeformationPK_DBG window*/
	static void MouseCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData ) ;

	dirList dirTemp;   // list of moved (fixed) vertices
	#define DIRECTION_ANGLE_LIMIT 0.3 // limit for ignoration of almost orthogonal vertices of direction (= acos(DIRECTION_ANGLE_LIMIT) ... angles between 72-108° are ignored)

	vtkPolyData** wrapper;
	int nWrappers;


private:
	vtkMEDPolyDataDeformationPK(const vtkMEDPolyDataDeformationPK&);  // Not implemented.
	void operator = (const vtkMEDPolyDataDeformationPK&);  // Not implemented.  	
#pragma endregion //methods

#pragma region //variables

protected:

	CONTROL_SKELETON* m_Skeletons;   //<input array of skeletons
	int m_NumberOfSkeletons;         //<number of skeletons in this array
	CollisionDetection* cd;
	GPU_Solver* gpu_solver;

#pragma endregion //variables
#pragma endregion // copied code
};


#endif