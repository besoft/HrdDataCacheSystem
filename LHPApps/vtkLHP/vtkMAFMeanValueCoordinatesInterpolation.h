/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkMAFMeanValueCoordinatesInterpolation.h,v $ 
  Language: C++ 
  Date: $Date: 2012-04-17 16:54:21 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Josef Kohout, Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef _vtkMAFMeanValueCoordinatesInterpolation
#define _vtkMAFMeanValueCoordinatesInterpolation

#pragma once

#include "vtkObject.h"
#include "vtkPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkLHPConfigure.h"

/**
	This class provides static methods for mean-value-coordinates (MVC) interpolation.
	Generally, there is one triangular mesh (closed and manifold) and one or more points whose MVC coordinates
	in the mesh are computed. Once computed, these coordinates can be used to restored Cartesian coordinates
	of these points using any topologically compatible mesh.

	IMPORTANT NOTE: representing M points in a mesh of N vertices using MVC requires N*M memory
	If N and M are large, it is recommended to compute MVC coordinates for individual points and store MVC coordinates onto disk.

	See also: Tao Ju, Scot Schaefer and Joe Warren. Mean Value Coordinates for Closed Triangular Meshes. SIGGRAPH 2005.
*/
class VTK_vtkLHP_EXPORT vtkMAFMeanValueCoordinatesInterpolation : public vtkObject
{
public:
  static vtkMAFMeanValueCoordinatesInterpolation *New();
  vtkTypeRevisionMacro(vtkMAFMeanValueCoordinatesInterpolation,vtkObject);

#pragma region MVC Relationship for Two Meshes
	/** Calculates MVC coordinates of every point from inputHiResMesh in inputLowResMesh. 
	Both meshes must be closed manifold triangular meshes. The best results are achieved, if inputLowResMesh is an outer hull of inputHiResMesh
	having the same major shape. Use this method to express relationship between both meshes. When inputLowResMesh geometry changes, 
	the geometry of inputHiResMesh can be updated easily using the returned relationship by ReconstructCartesianCoordinates method.
	N.B. This method may crash if, both meshes are too big. */
	inline static void ComputeMVCCoordinates(const vtkPolyData* inputLowResMesh, const vtkPolyData* inputHiResMesh, vtkDoubleArray* outWeights) {
		ComputeMVCCoordinates(inputLowResMesh, const_cast<vtkPolyData*>(inputHiResMesh)->GetPoints(), outWeights);
	}

	/** Reconstruct Cartesian coordinates of  inoutHiResMesh from its MVC coordinates outWeights in inputLowResMesh.
	Both meshes must be valid closed manifold triangular meshes, i.e., inoutHiResMesh should be contain both initial geometry and topology. 
	N.B. This method may crash if, both meshes are too big. */
	inline static void ReconstructCartesianCoordinates(const vtkPolyData* inputLowResMesh, const vtkDoubleArray* inWeights, vtkPolyData* inoutHiResMesh) {
		ReconstructCartesianCoordinates(inputLowResMesh, const_cast<vtkPolyData*>(inoutHiResMesh)->GetNumberOfPoints(), 
			inWeights, const_cast<vtkPolyData*>(inoutHiResMesh)->GetPoints());
	}
#pragma endregion

#pragma region MVC Calculation
#pragma region OneDimensional Weights
public:
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const vtkPoints* inPoints, vtkDoubleArray* outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints()*inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh,  const vtkPoints* inPoints, double* outWeights);
	
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */	
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, vtkDoubleArray* outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, vtkDoubleArray* outWeights);
	
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, double* outWeights);
	
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, double* outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
	static void ComputeMVCCoordinates(const double* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const double* inPoints, int numPoints, double* outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
	static void ComputeMVCCoordinates(const float* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const float* inPoints, int numPoints, double* outWeights);

private:
		/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
	template < typename T >
	static void ComputeMVCCoordinatesT(const vtkPolyData* inputMesh, const T* inPoints, int numPoints, double* outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	N.B., buffer outWeights must be capable of holding numPoints*numMeshPoints values. */
	template < typename T1, typename T2 >
	static void ComputeMVCCoordinatesT(const T1* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const T2* inPoints, int numPoints, double* outWeights);

	/** 
	Calculates Mean Value Coordinates (MVC) weights of triangle mesh whose vertices are specified in pts array (and may be doubles or floats)
	and triangles in pcells (format is: num of idx, idx1, idx2, idx3, num of idx, idx1, ...), i.e., these can be extracted directly from vtkPolyData
	Refers to Mean Value Coordinates for Closed Triangular Meshes, Tao Ju, Scott Schaefer, Joe Warren, Rice University
	@param pts coordinates of mesh (input)
	@param npts number of points (input)
	@param pcells cells (assumingly triangles: (3, id1, id2, id3), (3, id1, id2, id3) ...) (input)
	@param ncells number of cells (input)
	@param point 3D coordinates of vertex to be decomposed (input)
	@param coords preallocated array of length of number of vertices of mesh for final coefficients of linear decomposition (output)
	*/
	template < typename T1, typename T2 >
	static void ComputeMVCCoordinatesForPointT(const T1* pts, vtkIdType npts, const vtkIdType* pcells, 
		vtkIdType ncells, const T2* point, double* coords);
#pragma endregion

#pragma region TwoDimensional Weights For Larger Meshes
public:
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh,  const vtkPoints* inPoints, double** outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.  */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const double* inPoints, int numPoints, double** outWeights);
	
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const vtkPolyData* inputMesh, const float* inPoints, int numPoints, double** outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
	static void ComputeMVCCoordinates(const double* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const double* inPoints, int numPoints, double** outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.  */
	static void ComputeMVCCoordinates(const float* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const float* inPoints, int numPoints, double** outWeights);

private:
	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values. */
	template < typename T >
	static void ComputeMVCCoordinatesT(const vtkPolyData* inputMesh, const T* inPoints, int numPoints, double** outWeights);

	/** Calculates MVC coordinates of every point from inPoints set in the closed, manifold, triangular input mesh. 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., buffer outWeights must be capable of holding inPoints->GetNumberOfPoints() entries and each entry must hold inputMesh->GetNumberOfPoints() values.*/
	template < typename T1, typename T2 >
	static void ComputeMVCCoordinatesT(const T1* inMeshPoints, int numMeshPoints, const vtkIdType* inMeshCells, 
		int numMeshCells, const T2* inPoints, int numPoints, double** outWeights);
#pragma endregion

#pragma region Safe Disk Stored Weights For Really Large Meshes
public:
	//TODO: to be done
#pragma endregion
#pragma endregion

#pragma region MVC Reconstruction
#pragma region OneDimensional Weights
public:
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	inline static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const vtkDoubleArray* inWeights, vtkPoints* points){
		ReconstructCartesianCoordinates(inputMesh, numPoints, const_cast<vtkDoubleArray*>(inWeights)->GetPointer(0), points);
	}

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, vtkPoints* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const vtkDoubleArray* inWeights, double* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, double* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const vtkDoubleArray* inWeights, float* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, float* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, int numPoints, const double* inWeights, double* points);
		
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, int numPoints, const double* inWeights, float* points);
private:
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer points must be capable to store numPoints points. */
	template < typename T >
	static void ReconstructCartesianCoordinatesT(const vtkPolyData* inputMesh, int numPoints, const double* inWeights, T* points);	
#pragma endregion OneDimensional Weights

#pragma region TwoDimensional Weights
	public:	
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, vtkPoints* points);
	
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, double* points);
	
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, float* points);

	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, int numPoints, const double** inWeights, double* points);
		
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer outpoints must be capable to store numPoints points. */
	static void ReconstructCartesianCoordinates(const double* inMeshPoints, int numMeshPoints, int numPoints, const double** inWeights, float* points);
private:
	/** Reconstructs the Cartesian coordinates of points in the input mesh from MVC coordinates (inWeight). 
	This is a more memory-friendly version of the method that consumes less continuous memory.
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. 
	Buffer points must be capable to store numPoints points. */
	template < typename T >
	static void ReconstructCartesianCoordinatesT(const vtkPolyData* inputMesh, int numPoints, const double** inWeights, T* points);	
#pragma endregion
#pragma endregion

#pragma region MVC Reconstruction of Attributes
#pragma region OneDimensional Weights
public:
	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inputAttrs attributes of vertices of mesh topologically compatible with the mesh specified in the call of corresponding CalculateMVCCoordinates method
	@param numPoints number of points whose relationship to inputMesh was previously computed by CalculateMVCCoordinates
	@inWeights MVC coordinates of points whose attributes are to be reconstructed
	@outAttr where the reconstructed attributes should be stored
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	inline static void ReconstructAttributes(const vtkDataArray* inputAttrs, 
		int numPoints, const vtkDoubleArray* inWeights, vtkDataArray* outAttrs) {
			ReconstructAttributes(inputAttrs, numPoints, const_cast<vtkDoubleArray*>(inWeights)->GetPointer(0), outAttrs);
	}
	
	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inputAttrs attributes of vertices of mesh topologically compatible with the mesh specified in the call of corresponding CalculateMVCCoordinates method
	@param numPoints number of points whose relationship to inputMesh was previously computed by CalculateMVCCoordinates
	@inWeights MVC coordinates of points whose attributes are to be reconstructed
	@outAttr where the reconstructed attributes should be stored
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	static void ReconstructAttributes(const vtkDataArray* inputAttrs, 
		int numPoints, const double* inWeights, vtkDataArray* outAttrs);
	
	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	static void ReconstructAttributes(const double* inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double* inWeights, double* outAttrs);

	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inMeshAttrs an array of attributes (e..g, (x1, y1, z1), (x2, y2, z2), ...(xn, yn, zn))
	@param numMeshPoints number of entries in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one row of inMeshAttrs (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (one row for each point)
	N.B., buffer outAttrs must be capable of holding numPoints entries of numAttrs values. */
	static void ReconstructAttributes(const double** inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double* inWeights, double** outAttrs);

private:
	/** Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
	@param inputAttrs attributes of the original mesh
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	template < typename T >
	static void ReconstructAttributesT(const vtkDataArray* inputAttrs, 
		int numPoints, const double* inWeights, T* outAttrs);	
	
	/** Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	template < typename T1, typename T2 >
	static void ReconstructAttributesT(const T1* inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double* inWeights, T2* outAttrs);			

	/** Reconstructs attributes of a single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates of this point.
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param inWeights MVC coordinates of the point whose attributes are to be reconstructed.
	@param outAttrs where attributes are to be stored
	N.B., buffer outAttrs must be capable of holding numAttrs values. */
	template < typename T1, typename T2 >
	static void ReconstructAttributesForPointT(
		const T1* inMeshAttrs, int numMeshPoints, int numAttrs, const double* inWeights, T2* outAttrs);	

	/** Reconstructs attributes of a single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates of this point.
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param inWeights MVC coordinates of the point whose attributes are to be reconstructed.
	@param outAttrs where attributes are to be stored
	N.B., buffer outAttrs must be capable of holding numAttrs values. */
	template < typename T1, typename T2 >
	static void ReconstructAttributesForPointT_2(const T1** inMeshAttrs, int numMeshPoints, 
		int numAttrs, const double* inWeights, T2* outAttrs);	
#pragma endregion

#pragma region TwoDimensional Weights
public:		
	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inputAttrs attributes of vertices of mesh topologically compatible with the mesh specified in the call of corresponding CalculateMVCCoordinates method
	@param numPoints number of points whose relationship to inputMesh was previously computed by CalculateMVCCoordinates
	@inWeights MVC coordinates of points whose attributes are to be reconstructed
	@outAttr where the reconstructed attributes should be stored
	N.B., inputMesh must be topologically compatible with the mesh from which inWeights were calculated. */
	static void ReconstructAttributes(const vtkDataArray* inputAttrs, 
		int numPoints, const double** inWeights, vtkDataArray* outAttrs);
	
	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	static void ReconstructAttributes(const double* inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double** inWeights, double* outAttrs);

	/** Reconstructs the attributes of points in the input mesh from input mesh attributes and MVC coordinates (inWeight). 	
	@param inMeshAttrs an array of attributes (e..g, (x1, y1, z1), (x2, y2, z2), ...(xn, yn, zn))
	@param numMeshPoints number of entries in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one row of inMeshAttrs (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (one row for each point)
	N.B., buffer outAttrs must be capable of holding numPoints entries of numAttrs values. */
	static void ReconstructAttributes(const double** inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double** inWeights, double** outAttrs);

private:
	/** Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
	@param inputAttrs attributes of the original mesh
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	template < typename T >
	static void ReconstructAttributesT(const vtkDataArray* inputAttrs, 
		int numPoints, const double** inWeights, T* outAttrs);	
	
	/** Reconstructs attributes of every single point from attributes of vertices of the mesh used as input for calculation of MVC coordinates.
	@param inMeshAttrs an array of attributes arranged into tuples (e..g, x1, y1, z1, x2, y2, z2, ...xn, yn, zn)
	@param numMeshPoints number of tuples in inMeshAttr, i.e., number of vertices of the mesh
	@param numAttrs number of attributes in one tuple (e.g., 3)
	@param numPoints number of points whose attributes are to be reconstructed.
	@param inWeights MVC coordinates of all points whose attributes are to be reconstructed arranged as ("MVC for 1st point", "MVC for 2nd point" ...)
	@param outAttrs where attributes are to be stored (arranged in tuples)
	N.B., buffer outAttrs must be capable of holding numPoints*numAttrs values. */
	template < typename T1, typename T2 >
	static void ReconstructAttributesT(const T1* inMeshAttrs, int numMeshPoints, int numAttrs, 
		int numPoints, const double** inWeights, T2* outAttrs);			

#pragma endregion
#pragma endregion

protected:
	vtkMAFMeanValueCoordinatesInterpolation() {}
	~vtkMAFMeanValueCoordinatesInterpolation() {}

private:
  vtkMAFMeanValueCoordinatesInterpolation(const vtkMAFMeanValueCoordinatesInterpolation&);  // Not implemented.
  void operator=(const vtkMAFMeanValueCoordinatesInterpolation&);  // Not implemented.
};

#endif // _vtkMAFMeanValueCoordinatesInterpolation
