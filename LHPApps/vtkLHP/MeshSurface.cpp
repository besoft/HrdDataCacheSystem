/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshSurface.cpp,v $ 
  Language: C++ 
  Date: $Date: 2012-04-17 16:54:21 $ 
  Version: $Revision: 1.1.2.13 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
 
#include "MeshSurface.h"
#include "vtkCellArray.h"
#include "mafDbg.h"
#include "vtkMath.h" //HAJ:

#include "vtkMAFMeanValueCoordinatesInterpolation.h"

//////
/// Creates new empty instance. Used privately in Clone() method.
//////
MeshSurface::MeshSurface(void)
{
	this->original = NULL;
	this->coarse = NULL;
	this->coarseToOrigRel = NULL;

	this->ooCylinder = new OoCylinder();
	this->navigator = new MeshNavigator();
	this->deformable = false;
	this->m_UseProgressiveHull = false;
}

//////
/// Creates new instance with specified detail mesh. Generates coarse mesh
/// with specific number of vertices.
/// @param original original detail mesh (input)
/// @param muscle if true, muscle is expected and coarse is computed as a hull, if false, coase is the same as original mesh
/// @param coarse original coarse mesh (optional input)
/// @coarseVertexCount number of vertices for coarse mesh (input)
/// @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
//////
MeshSurface::MeshSurface(vtkPolyData *original, bool muscle, vtkPolyData *coarse, int coarseVertexCount, bool bUseProgressiveHull)
{
	this->ooCylinder = new OoCylinder();
	this->navigator = new MeshNavigator();
	this->deformable = false;
	this->m_UseProgressiveHull = false;
	this->muscle = muscle;

	this->InitInstance(original, muscle, coarse, coarseVertexCount, bUseProgressiveHull);
}

//////
/// Creates new instance with specified detail mesh. Generates coarse mesh
/// with specific reduction ratio.
/// @param original original detail mesh (input)
/// @param muscle if true, muscle is expected and coarse is computed as a hull, if false, coase is the same as original mesh
/// @param coarse original coarse mesh (optional input)
/// @reductionRatio 0 to 1 ratio of decimated vertices from original mesh (input)
/// @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
//////
MeshSurface::MeshSurface(vtkPolyData *original, bool muscle, vtkPolyData *coarse, float reductionRatio, bool bUseProgressiveHull)
{
	this->ooCylinder = new OoCylinder();
	this->navigator = new MeshNavigator();
	this->deformable = false;
	this->m_UseProgressiveHull = false;
	this->muscle = muscle;

	vtkIdType nOrigPoints = original->GetNumberOfPoints();
	this->InitInstance(original, muscle, coarse, (int)(nOrigPoints * (1 - reductionRatio)), bUseProgressiveHull);
}

//////
/// Initializes new instance with specified detail mesh. Generates coarse mesh
/// with specific number of vertices.
/// @param original original detail mesh (input)
/// @param muscle if true, muscle is expected and coarse is computed as a hull, if false, coase is the same as original mesh
/// @coarseVertexCount number of vertices for coarse mesh (input)
///  @bUseProgressiveHull is true, if progressive hull should be calculated for coarse mesh (if this calculation is available)
//////
void MeshSurface::InitInstance(vtkPolyData *original, bool muscle, vtkPolyData *coarse, int coarseMeshSize, bool bUseProgressiveHull)
{
	vtkIdType nOrigPoints = original->GetNumberOfPoints();
	coarseMeshSize = min(nOrigPoints, coarseMeshSize);

#ifdef _USE_LP_SOLVE	//this is defined by vtkProgressiveHull, if it compiles correctly
	this->m_UseProgressiveHull = bUseProgressiveHull;
#else
#pragma message("WARNING: MeshSurface will ignore UseProgressiveHull options because ProgressiveHull is probably not available.")
	this->m_UseProgressiveHull = false;
#endif
	this->original = vtkPolyData::New();
	this->original->DeepCopy(original);
	this->original->BuildCells();	//make sure that we have Cells and Links available (for intersection tests);
	this->original->BuildLinks();

	this->coarse = NULL;
	this->coarseToOrigRel = NULL;

	if (coarse == NULL) 
	{
		if (muscle) this->CreateCoarseMesh(original, coarseMeshSize); // for muscle compute coarse mesh
		else {
			this->coarse = vtkPolyData::New(); // for bone use original mesh as a coarse
			this->coarse->DeepCopy(original);
		}
	}
	else 
	{
		this->coarse = vtkPolyData::New();
		if (muscle) this->coarse->DeepCopy(coarse);		// for muscle compute coarse mesh
		else this->coarse->DeepCopy(original);		    // for bone use original mesh as a coarse
	}

	this->coarse->BuildCells(); 	//make sure that we have Cells and Links available (for intersection tests);
	this->coarse->BuildLinks();

	this->CalculateRelations();
	
	//this->coarseToOrigRel->DebugOutputMathematica("baryc.nb");
}

//////
/// Destroys instance, deallocates memory.
//////
MeshSurface::~MeshSurface(void)
{
	if (this->coarse != NULL)
	{
		this->coarse->Delete();
		this->coarse = NULL;
	}

	if (this->original != NULL)
	{
		this->original->Delete();
		this->original = NULL;
	}

	if (this->coarseToOrigRel != NULL)
	{
		PKUtils::DisposeMatrix(&(this->coarseToOrigRel));
		this->coarseToOrigRel = NULL;
	}

	delete this->ooCylinder;
	delete this->navigator;
}

//////
/// Deeply clones this instance.
/// @return deep copy of this instance
//////
MeshSurface* MeshSurface::Clone()
{
	MeshSurface *clone = new MeshSurface();
	
	clone->original = vtkPolyData::New();
	clone->original->DeepCopy(this->original);

	clone->coarse = vtkPolyData::New();
	clone->coarse->DeepCopy(this->coarse);

	clone->coarseToOrigRel = PKUtils::CloneMatrix(this->coarseToOrigRel);
	clone->m_UseProgressiveHull = this->m_UseProgressiveHull;

	clone->deformable = this->deformable;

	return clone;
}

//////
/// Gets soft reference to original mesh.
/// @return soft reference to original mesh
//////
vtkPolyData* MeshSurface::GetOriginal()
{
	return this->original;
}

//////
/// Gets soft reference to coarse mesh.
/// @return soft reference to coarse mesh
//////
vtkPolyData* MeshSurface::GetCoarse()
{
	return this->coarse;
}

//////
/// Gets soft reference to object oriented bounding cylinder.
/// @return soft reference to bounding cylinder
//////
OoCylinder* MeshSurface::GetOoCylinder()
{
	return this->ooCylinder;
}

//////
/// Gets soft reference to mesh navigator
/// @return soft reference to mesh navigator
//////
MeshNavigator* MeshSurface::GetNavigator()
{
	return this->navigator;
}

//////
/// Sets flag if mesh deformable.
/// @param true for deformable
//////
void MeshSurface::SetDeformable(bool deformable) {
	this->deformable = deformable;
}

//////
/// Gets flag if mesh deformable.
/// @return true for deformable
//////
bool MeshSurface::GetDeformable() {
	return this->deformable;
}

#pragma region // distribution

#pragma region // coarse to orig

//////
/// Distributes values specified for all points of coarse mesh to array of values for all vertices of original mesh
/// according to its barycentric coordinates in respect to coarse mesh.
/// @param coarseAttributes matrix with row for every coarse vertex and column for every distributed value of every vertex (input)
/// @param origAttributes preallocated matrix with row for every vertex of original mesh and column for every distributed value (output)
//////
void MeshSurface::DistributeCoarseAttributesToOriginal(PKMatrix *coarseAttributes, PKMatrix *origAttributes)
{
	if (origAttributes->height != this->coarseToOrigRel->height)
	{
		throw "Original attributes height must match original mesh.";
	}

	if (coarseAttributes->height != this->coarseToOrigRel->width)
	{
		throw "Coarse attributes height must match coarse mesh.";
	}

	vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes((const double**)coarseAttributes->values,
		coarseAttributes->height, coarseAttributes->width, origAttributes->height,
		(const double**)this->coarseToOrigRel->values, origAttributes->values);
}

#pragma endregion // coarse to orig

#pragma endregion // distribution

//////
/// Creates coarse control mesh by reduction of original high detail mesh.
/// @param source mesh to reduce (input)
/// @param coarseMeshSize number of vertices required on the output
//////
void MeshSurface::CreateCoarseMesh(vtkPolyData* source, int coarseMeshSize)
{
	if (source == NULL)
	{
		throw "Source mesh must not be NULL.";
	}

	if (this->coarse != NULL)
	{
		this->coarse->Delete();
		this->coarse = NULL;
	}
	
	//create the output polydata
	this->coarse = vtkPolyData::New();

	//calculate the 
	vtkIdType nOrigPoints = source->GetNumberOfPoints();
	
	if (m_UseProgressiveHull)
	{	
		vtkProgressiveHull::useCuda = 0;	//BES: 2.3.2012 - CUDA version produces some undesirable spikes

		vtkProgressiveHull* decimator = vtkProgressiveHull::New();
		decimator->SetTargetReduction(1.0 - (double)coarseMeshSize / nOrigPoints);
		//decimator->SetMeshRoughness(0.4);						
		decimator->SetInput(source);
		decimator->SetOutput(this->coarse);
		decimator->Update();

		decimator->SetOutput(NULL);	//detach this->coarse from its sources		
		decimator->Delete();		
	}
	else
	{		
		int firstTarget = max(min(300, nOrigPoints), coarseMeshSize);
		float alpha = (nOrigPoints - firstTarget) / (float) nOrigPoints;
		float beta = (firstTarget - coarseMeshSize) / (float) firstTarget;
		
		vtkQuadricDecimation *decimator = vtkQuadricDecimation::New();
		decimator->SetTargetReduction(alpha);
		decimator->SetInput(original);
		decimator->SetOutput(this->coarse);
		decimator->Update();
		decimator->Delete();
		this->coarse->BuildCells();
		this->coarse->BuildLinks();

		this->EnlargeMesh(this->coarse, 10);

		vtkPolyData *secondCoarse = vtkPolyData::New();
		decimator = vtkQuadricDecimation::New();
		decimator->SetTargetReduction(beta);
		decimator->SetInput(this->coarse);	
		decimator->SetOutput(secondCoarse);
		decimator->Update();
		decimator->Delete();
		
		this->coarse->Delete();
		this->coarse = secondCoarse;	
	}
}

//////
/// Enlarges mesh by shifting by normals.
/// @param mesh mesh for enlargement (input/ouput)
/// @param shiftLength length of shift for every vertex (input)
//////
void MeshSurface::EnlargeMesh(vtkPolyData* mesh, double shiftLength)
{
	// move points 50 units in direction of their normals to enlarge coarse mesh
	PKMatrix *normals = PKUtils::CreateMatrix(mesh->GetNumberOfPoints(), 3);

	MeshSurface::CalculateNormals(mesh, normals);

	for (vtkIdType i = 0; i < normals->height; i++)
	{
		double *normal = normals->values[i];
		PKUtils::MultiplyVertex(normal, shiftLength);
		MeshSurface::ShiftPointSafely(i, mesh, normal);
	}

	PKUtils::DisposeMatrix(&normals);
}

void MeshSurface::ShiftPointSafely(vtkIdType pointId, vtkPolyData *mesh, double direction[3])
{
	double originalPoint[3], newPoint[3];
	vtkPoints *points = mesh->GetPoints();
	points->GetPoint(pointId, originalPoint);
	PKUtils::AddVertex(originalPoint, direction, newPoint);
	
	/*if (this->CalculatePointSpin(mesh, newPoint, direction, 0.001) % 2 != 0)
	{
		return;
	}*/

	points->SetPoint(pointId, newPoint);
}

//////
/// Calculates barycentric coordinates of all original vertices in coarse mesh and
/// stores them to internal coarseToOrigRel matrix.
/// Uses OpenMP for parallelization.
//////
void MeshSurface::CalculateRelations()
{
	vtkIdType origCount = this->original->GetNumberOfPoints();
	vtkIdType coarseCount = this->coarse->GetNumberOfPoints();

	if (this->coarseToOrigRel != NULL)
	{
		PKUtils::DisposeMatrix(&(this->coarseToOrigRel));
	}

	this->coarseToOrigRel = PKUtils::CreateMatrix(origCount, coarseCount);
	vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(this->coarse,
		this->original->GetPoints(), this->coarseToOrigRel->values);
}

//////
/// Copies coordinates to mesh to change positions of vertices.
/// @param mesh mesh with vertices to be repositioned (input/output)
/// @param points nx3 matrix with new coordinates for vertices (input)
//////
void MeshSurface::SetUpPointCoords(vtkPolyData *mesh, PKMatrix *points)
{
	if (points == NULL)
	{
		throw "Point coordinates must be provided as PKMatrix structure.";
	}

	if (mesh == NULL)
	{
		throw "Mesh must be provided valid vtkPolyData instance.";
	}
	
	vtkPoints *meshPoints = mesh->GetPoints();
	vtkIdType nPoints = mesh->GetNumberOfPoints();

	if (nPoints != points->height || points->width != 3)
	{
		throw "Given point coord matrix has invalid dimension(s).";
	}

	// copy coords to mesh
	for (vtkIdType i = 0; i < nPoints; i++) 
	{
		meshPoints->SetPoint(i, points->values[i]);
	}
}


#pragma region //support methods

//////
/// Calculates normals for mesh vertices.
/// @param mesh input mesh (input)
/// @param normals preallocated nx3 matrix for normals (output)
//////
void MeshSurface::CalculateNormals(vtkPolyData *mesh, PKMatrix *normals)
{
	if (normals == NULL)
	{
		throw "Normals must be preallocated.";
	}

	if (mesh == NULL)
	{
		throw "Mesh must be provided valid vtkPolyData instance.";
	}

	vtkIdType nPoints = mesh->GetNumberOfPoints();

	if (nPoints != normals->height || normals->width != 3)
	{
		throw "Given normal matrix has invalid dimension(s).";
	}

	vtkPolyDataNormals *normalizer = vtkPolyDataNormals::New();
	normalizer->ComputeCellNormalsOff();
	normalizer->ComputePointNormalsOn();
	normalizer->SetInput(mesh);
	normalizer->Update();
	vtkDataSetAttributes *atributes = normalizer->GetOutput()->GetPointData();

	vtkFloatArray *normArray = vtkFloatArray::SafeDownCast(atributes->GetNormals());
	int i;

	double normal[3];
	for (i = 0; i < normArray->GetNumberOfTuples() && i < nPoints; i++)
	{
		PKUtils::CopyVertex(normArray->GetTuple(i), normal);
		/*double length = PKUtils::CalculateVertexLength(normal);
		PKUtils::DivideVertex(normal, length);*/
		PKUtils::CopyVertex(normal, normals->values[i]);

		// Do whatever you need to do to write this value to a file here.
	}

	normalizer->Delete();
}
/*
//////
/// Calculates normals for mesh vertices.
/// @param mesh input mesh (input)
/// @param normals preallocated nx3 matrix for normals (output)
//////
void MeshSurface::CalculateNormals(vtkPolyData *mesh, PKMatrix *normals)
{
	if (normals == NULL)
	{
		throw "Normals must be preallocated.";
	}

	if (mesh == NULL)
	{
		throw "Mesh must be provided valid vtkPolyData instance.";
	}

	vtkIdType nPoints = mesh->GetNumberOfPoints();

	if (nPoints != normals->height || normals->width != 3)
	{
		throw "Given normal matrix has invalid dimension(s).";
	}
	
	double **normalRow = normals->values;
	double point[3];
	double neighA[3];
	double neighB[3];
	double normalPart[3];

	for (vtkIdType i = 0; i < nPoints; i++, normalRow++)
	{
		int normalCount = 0;
		double *normal = *normalRow;
		mesh->GetPoint(i, point);

		// get neighbours
		vtkIdType *neighbours = NULL;
		vtkIdType neighboursCount = -1;
		this->GetPointNeighboursSorted(mesh, i, neighbours, neighboursCount); 

		// for each neigbour
		for (vtkIdType j = 0; j < neighboursCount; j++)
		{
			mesh->GetPoint(neighbours[j], neighA);
			mesh->GetPoint(neighbours[(j + 1) % neighboursCount], neighB);
			
			// count normal of triangle and add to total normal
			PKUtils::SubtractVertex(neighA, point, neighA);
			PKUtils::SubtractVertex(neighB, point, neighB);
			vtkMath::Cross(neighA, neighB, normalPart);
			
			double length = PKUtils::CalculateVertexLength(normalPart);			

			if (length <= 0)
			{
				continue;
			}

			PKUtils::DivideVertex(normalPart, length);
			PKUtils::AddVertex(normal, normalPart, normal);
		}

		double totalLen = PKUtils::CalculateVertexLength(normal);
		if (totalLen > 0)
		{
			PKUtils::DivideVertex(normal, totalLen);

			// check direction
			int spin = this->CalculatePointSpin(mesh, point, normal, 0.001);
			if (spin % 2 == 1)
			{
				PKUtils::MultiplyVertex(normal, -1);
			}
		}

		delete neighbours;
	}
	
}*/

//////
/// Gets array of neigbouring vetices for one vertex of mesh sorted so they create fan around specified point.
/// @param polyData mesh with topology (input)
/// @param pointId id of central point (input)
/// @param neighbourIds array of neigbouring vertices ids (not to be preallocated) (output)
/// @param neighbourIdsCount length of output array => number of neighbours (output)
//////
void MeshSurface::GetPointNeighboursSorted(vtkPolyData* polyData, vtkIdType pointId, vtkIdType *&neighbourIds, vtkIdType &neighbourIdsCount) 
{
	vtkIdType sortedNeighCache[20];
	MeshSurface::GetPointNeighbours(polyData, pointId, neighbourIds, neighbourIdsCount);
	
	if (neighbourIdsCount <= 2)
	{
		return;
	}

	vtkIdType *sortedNeighbours = sortedNeighCache;
	if (neighbourIdsCount > 20)
	{
		sortedNeighbours = new vtkIdType[neighbourIdsCount];
	}

	vtkIdType *thirdAppexes = new vtkIdType[neighbourIdsCount * 2];
	
	for (vtkIdType i = 0; i < neighbourIdsCount; i++)
	{
		MeshSurface::GetPointsTriangleVertices(polyData, pointId, neighbourIds[i], &(thirdAppexes[2 * i]), &(thirdAppexes[2 * i + 1]));
	}

	// first one is on correct place=> find another
	sortedNeighbours[0] = neighbourIds[0];
	neighbourIds[0] = -1;
	vtkIdType openEndId = sortedNeighbours[0];
	
	for (vtkIdType i = 1; i < neighbourIdsCount; i++)
	{
		bool found = false;

		vtkIdType j;
		for (j = 1; j < neighbourIdsCount; j++)
		{
			if (neighbourIds[j] < 0)
			{
				continue;
			}

			if (thirdAppexes[2 * j] == openEndId || thirdAppexes[2 * j + 1] == openEndId)
			{
				openEndId = neighbourIds[j];
				sortedNeighbours[i] = neighbourIds[j];
				neighbourIds[j] = -1;
				found = true;
				break;
			}		
		}

		// error
		if (!found)
		{
			neighbourIdsCount = i;
			break;
		}		
	}

	memcpy(neighbourIds, sortedNeighbours, sizeof(vtkIdType) * neighbourIdsCount);
	
	if (neighbourIdsCount > 20)
	{
		delete[] sortedNeighbours;
	}

	delete[] thirdAppexes;
}

//////
/// Gets array of neigbouring vetices for one vertex of mesh.
/// @param polyData mesh with topology (input)
/// @param pointId id of central point (input)
/// @param neighbourIds array of neigbouring vertices ids (not to be preallocated) (output)
/// @param neighbourIdsCount length of output array => number of neighbours (output)
//////
void MeshSurface::GetPointNeighbours(vtkPolyData* polyData, vtkIdType pointId, vtkIdType *&neighbourIds, vtkIdType &neighbourIdsCount) 
{
	set<vtkIdType>::iterator it;
	vtkIdType *pCellsIds;
	unsigned short nCellsIds;
	polyData->GetPointCells(pointId, nCellsIds, pCellsIds);

	set<vtkIdType> neighbours;// (nCellsIds * ESTIMATED_PT_NEIGH_COUNT);

	// for all triangles of pointId
	for (int i = 0; i < nCellsIds; i++)
	{
		vtkIdType nPtsIds, *pPtsIds;
		polyData->GetCellPoints(pCellsIds[i], nPtsIds, pPtsIds);
		for (int j = 0; j < nPtsIds; j++)
		{
			// insert all negbours except pointId itself
			if (pointId == pPtsIds[j]) 
			{
				continue;
			}
			neighbours.insert(pPtsIds[j]);
		}
	}

	neighbourIdsCount = neighbours.size();
	neighbourIds = new vtkIdType[neighbourIdsCount];
	vtkIdType *pointer = neighbourIds;

	// copy to regular array
	for (it = neighbours.begin(); it != neighbours.end(); it++)
	{
		(*pointer) = *it;
		pointer++;
	}

	neighbours.clear();
}

#if defined(_DEBUG) && _MSC_VER >= 1600
#pragma warning(push)
#pragma warning(disable:4996)
#include "vtkSTLWriter.h"
#pragma warning(pop)
#endif

//////
/// Gets third apexes of common triangles of two mesh vertices. There is expectation of existence of exactly one
/// edge between these vertices and therefore for "normal" closed meshed two incident triangles.
/// @param polyData mesh with topology (input)
/// @param pointId1 id of first point (input)
/// @param pointId2 id of second point (input)
/// @param cellId1 id of missing apex of first triangle (output)
/// @param cellId2 id of missing apex of first triangle (output)
//////
void MeshSurface::GetPointsTriangleVertices(vtkPolyData* polyData, vtkIdType pointId1, vtkIdType pointId2, vtkIdType* vertexId1, vtkIdType* vertexId2) 
{
	*vertexId1 = -1;
	*vertexId2 = -1;

	// find triangles of pointId1
	vtkIdType *pCellsIds1;
	unsigned short nCellsIds1;
	polyData->GetPointCells(pointId1, nCellsIds1, pCellsIds1);

	// find triangles of pointId2
	vtkIdType *pCellsIds2;
	unsigned short nCellsIds2;
	polyData->GetPointCells(pointId2, nCellsIds2, pCellsIds2);

	if (nCellsIds1 <= 1 || nCellsIds2 <= 1)
	{
		//"Cannot find neighbours of isolated vertex.");
		_ASSERTE(false);
		return;
	}

	int cellFoundCount = 0;
		
	// for all pairs of 1 triangle from pointId1 and one from pointId2
	// until both common triangles found
	for (vtkIdType i = 0; i < nCellsIds1 && cellFoundCount < 2; i++) 
	{
		for (vtkIdType j = 0; j < nCellsIds2 && cellFoundCount < 2; j++) 
		{
			// if i and j are common triangles
			if (pCellsIds1[i] == pCellsIds2[j]) 
			{
				vtkCell* cell = polyData->GetCell(pCellsIds1[i]);

				vtkIdType third = -1;
				
				vtkIdList* cellPoints = cell->GetPointIds();
				vtkIdType cellPointsCount = cellPoints->GetNumberOfIds();

				// find third vertex of triangle (not pointId1 nor pointId2)
				for (vtkIdType k = 0; k < cellPointsCount; k++) 
				{
					if (cellPoints->GetId(k) == pointId1 || cellPoints->GetId(k) == pointId2) 
					{
						continue;
					}
					third = cellPoints->GetId(k);
					break;
				}

				if (third >= 0) 
				{
					// output result 1 or result 2
					if (cellFoundCount == 0) 
					{
						(*vertexId1) = third;
					} 
					else 
					{
						(*vertexId2) = third;
					}
					cellFoundCount++;
				}
				break;	//if we have found it, go on
			}
		}		
	}

#if defined(_DEBUG) && _MSC_VER >= 1600
	if (*vertexId1 < 0 || *vertexId2 < 0 || *vertexId1 == *vertexId2) 
	{
		double pt1[3], pt2[3];
		polyData->GetPoint(pointId1, pt1);
		polyData->GetPoint(pointId2, pt2);

		_RPT4(_CRT_WARN, "Invalid edge: %d (%f,%f, %f), ", pointId1, pt1[0], pt1[1], pt1[2]);
		_RPT4(_CRT_WARN, "%d (%f,%f, %f)\n", pointId2, pt2[0], pt2[1], pt2[2] );

		vtkSTLWriter* wr = vtkSTLWriter::New();
		wr->SetFileName("D:\\temp\\coarse.stl");
		wr->SetInput(polyData);
		wr->Update();
		wr->SetInput(NULL);
		wr->Delete();
	}
#endif	
}

//////
/// Calculates linear decomposition of point coordinates to mesh vertices.
/// Refers to Mean Value Coordinates for Closed Triangular Meshes, Tao Ju, Scott Schaefer, Joe Warren, Rice University
/// @param pPoly mesh with topology and coordinates (input)
/// @param point 3D coordinates of decomposed vertex (input)
/// @param coords preallocated array of length of number of vertices of mesh for final coefficients of linear decomposition (output)
//////
void MeshSurface::CalculateMeshCoordsForPoint(vtkPolyData* pPoly, double point[3], double* coords)
{	
	vtkMAFMeanValueCoordinatesInterpolation::ComputeMVCCoordinates(pPoly, point, 1, coords);
}


void MeshSurface::ApplyCoarseCoordsToOriginalMesh(PKMatrix *coarseCoords, PKMatrix *bufferHigherResPoints)
{
	if (coarseCoords->height != this->GetCoarse()->GetNumberOfPoints() || coarseCoords->width != 3)
	{
		throw "Invalid coordinate matrix size.";
	}

	PKMatrix *highResPoints = bufferHigherResPoints;
	if (highResPoints != NULL) {
		_ASSERTE(bufferHigherResPoints->height == this->GetOriginal()->GetNumberOfPoints() && bufferHigherResPoints->width == 3);
	}
	else {
		highResPoints = PKUtils::CreateMatrix(this->GetOriginal()->GetNumberOfPoints(), 3);
	}

	vtkMAFMeanValueCoordinatesInterpolation::ReconstructAttributes((const double**)coarseCoords->values,
		coarseCoords->height, 3, highResPoints->height, 
		(const double**)this->coarseToOrigRel->values, highResPoints->values);

	this->SetUpPointCoords(this->GetOriginal(), highResPoints);

	if (bufferHigherResPoints == NULL) {
		PKUtils::DisposeMatrix(&highResPoints);
	}
}

void MeshSurface::AddMeshOffset(vtkPolyData *mesh, PKMatrix *offset) {
	vtkPoints *points = mesh->GetPoints();
	vtkIdType nPoints = points->GetNumberOfPoints();

	if (offset->height < (int)nPoints || offset->width < 3) {
		throw "Offset matrix to small.";
	}

	double point[3];
	
	double** row = offset->values;
	for (vtkIdType i = 0; i < nPoints; i++, row++) {
		points->GetPoint(i, point);
		PKUtils::AddVertex(point, *row, point);
		points->SetPoint(i, point);
	}
}

//////
/// Computes the shortest distance od the point to a cell of the passive object in a given direction
/// @param start computed point (vertex)
/// @param direction direction in which the distance should be computed
/// @param mesh passive object in which the cell is searched
/// @param points
/// @param triangleId
/// @param cellTemp
//////
double MeshSurface::FindIntersectionInDirection( double start[3], double direction[3], vtkPolyData* mesh, PKMatrix* points /*= NULL*/, vtkIdType* triangleId /*= NULL*/, vtkGenericCell* cellTemp)
{
	// find intersection triangle
	double bestT = -1;

	double cellIsLocal = false;
	if (cellTemp == NULL) {
		cellTemp = vtkGenericCell::New();
		cellIsLocal = true;
	}

	
	double a[4]; // vertices of the cell
	double b[4];
	double c[4];
	double temp[4];
	double point[4]; // coordinates of the starting vertex (homogenous)

	
	/*double ab[3] = {0,0,0};
	double ac[3] = {0,0,0};
	double normal[3];
	*/
	PKUtils::CopyVertex(start, point);

	vtkIdType nCells = mesh->GetNumberOfCells();

	for (vtkIdType j = 0; j < nCells; j++) { // for each cell of the passive object...
		mesh->GetCell(j, cellTemp);

		if (cellTemp->GetNumberOfPoints() < 3) {
			continue;
		}

		myGetPointDeep(mesh, points, cellTemp->GetPointId(0), a); // save coordinates of cell points
		myGetPointDeep(mesh, points, cellTemp->GetPointId(1), b);
		myGetPointDeep(mesh, points, cellTemp->GetPointId(2), c);
		
		a[3] = b[3] = c[3] = point[3] = 1; // homogenous coordinates are used

		// normal equation
		PKUtils::Cross4(a, b, c, temp); 
		PKUtils::MultiplyVertex4(temp, PKUtils::CalculateVertexLength(temp)); 

		// project P
		double t = - (PKUtils::Dot4(temp, point) / PKUtils::Dot(temp, direction)); // computed distance of the point to the plane going through the cell
		if (t < 0 || (t > bestT && bestT >= 0)) {
			// other direction or not near enough
			continue;
		}

		double projection[3]; // projection point
		PKUtils::CopyVertex(direction, temp);
		PKUtils::MultiplyVertex(temp, t);
		PKUtils::AddVertex(point, temp, projection);
		
		/// HAJ: alternative computation algorithm (same results, more understandable, but not so quick)
		/*
		PKUtils::SubtractVertex(b,a,ab);
		PKUtils::SubtractVertex(c,a,ac);

		vtkMath::Cross(ab, ac, normal);
		PKUtils::DivideVertex(normal, PKUtils::CalculateVertexLength(normal));
		double d = -vtkMath::Dot(normal, a);

		double temp1[3] = {0,0,0};
		double x = (-d - vtkMath::Dot(normal, point))/vtkMath::Dot(normal, direction);
		
		PKUtils::CopyVertex(direction, temp1);
		PKUtils::MultiplyVertex(temp1, x);
		
		double projection1[3];
		PKUtils::AddVertex(point, temp1, projection1);
		
		// is inside triangle?
		if (!PKMath::PointInTriangle(projection1, a, b, c)) {
			// bad
			continue;
		}
		
		if (x < 0 || (x > bestT && bestT >= 0)) {
			// other direction or not near enough
			continue;
		}

		bestT = x;
		/**/
		// test, if the projection point lies inside the triangle (cell)?
		if (!PKMath::PointInTriangle(projection, a, b, c)) {
			// bad
			continue; // if not, continue with another cell
		}

		// if the projection lies in the cell, t is saved
		bestT = t;

		if (triangleId != NULL) {
			*triangleId = j;
		}
	}

	if (cellIsLocal) {
		cellTemp->Delete();
		cellTemp = NULL;
	}

	return bestT;
}


#pragma endregion //support methods