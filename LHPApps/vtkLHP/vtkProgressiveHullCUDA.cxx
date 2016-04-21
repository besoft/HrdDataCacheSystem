/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkProgressiveHullCUDA.cxx,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:26 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Tomas Janak
  Notes:
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include <list>
#include "vtkPolyData.h"
#include <vtkProperty.h>
#include "vtkTriangle.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkProgressiveHullCUDA.h"
#include "ProgressiveHullCUDALib.h"
using namespace std;

vtkInstantiatorNewMacro(vtkProgressiveHullCUDA);

//retrieve instance of the class
/*static*/ vtkProgressiveHullCUDA* vtkProgressiveHullCUDA::New()
{
	return new vtkProgressiveHullCUDA();
}

//CUDA version requires a bit larger MeshRoughness parameter to get the same result as CPU version
#define MESHROUGHNESS_ADDCOEFF 0.8
#define MESHROUGHNESS_MULTCOEFF (1 / (1  + MESHROUGHNESS_ADDCOEFF))

vtkProgressiveHullCUDA::vtkProgressiveHullCUDA()
{
	targetPoints = 0;
	TargetReduction = 0;	
	MeshRoughness = 0.5;
	MaxValence = 15;
}

// Creates a progressive hull of a given mesh (prepares the data then calls
// a CUDA kernell which actually creates the hull).
void vtkProgressiveHullCUDA::Execute()
{
	vtkPolyData *originalMesh = this->GetInput();
	originalMesh->BuildLinks();

	//create an array of vertices
	vtkPoints *origPoints = originalMesh->GetPoints();
	int numberOfVertices = originalMesh->GetNumberOfPoints();
	int numberOfTriangles = originalMesh->GetNumberOfCells();
	Vertex *verts = new Vertex[numberOfVertices];
	double points[3];
	for (int i = 0; i < numberOfVertices; i++)
	{
		origPoints->GetPoint(i, points);
		verts[i].SetCoordinates((float)points[0], (float)points[1], (float)points[2]);
	}	

	Log("Starting mesh processing.\nInput mesh information:\nNumber of vertices: ", numberOfVertices);
	Log("\nNumber of triangles: ", numberOfTriangles);

	// create an array of triangles and edges
	Triangle *tris = new Triangle[numberOfTriangles];
	vtkCellArray *polys = originalMesh->GetPolys();
	polys->InitTraversal();
	vtkIdType *pts, npts;
	int vids[3]; 
	// structures for generating neighbours
	int neighbours[3];
	vtkIdList *listE = vtkIdList::New();
	vtkIdList *listResults = vtkIdList::New();
	// structures for edge extracting
	int numberOfEdges = 0;
	list<Edge> edgeList;
	Edge newEdge;
	for (int i = 0; i < numberOfTriangles; i++)
	{
		polys->GetNextCell(npts, pts);
		vids[0] = pts[0]; vids[1] = pts[1];	vids[2] = pts[2];
		tris[i].SetVertices(vids);
		// generate neighbours and edges
		for (int j = 0; j < 3; j++)
		{
			// empty the lists
			listE->Reset();
			listResults->Reset();
			// add the next edge
			listE->InsertNextId(pts[j]);
			listE->InsertNextId(pts[(j + 1) % 3]);
			// search for neighbours of this cell sharing the edge that is currently in "list"
			originalMesh->GetCellNeighbors(i, listE, listResults);
			if (listResults->GetNumberOfIds() > 1) // more than one neighbour - not a manifold
				Log("\nWarning, input mesh is not manifold, ouput might be erroneous.\n");
				//return 2;
			if (listResults->GetNumberOfIds() == 0) // no neighbour - the mesh is not closed
				Log("\nWarning, input mesh is not closed, ouput might be erroneous.\n");
			else
				neighbours[j] = listResults->GetId(0);
			// add edge if the neighbour was not processed yet (has higher ID). if it was, the edge is already in the list
			if (neighbours[j] > i)
			{
				newEdge = Edge::Edge();
				newEdge.SetTriangles(i, neighbours[j]);
				// ensure that the vertex with ID 0, if present in the edge, will be as the first vertex - 
				// during gpu processing, negative values are used to denote links to other location in the edge array,
				// therefore theres a complication when 0 is present, because it can mean vertex 0 but also a link to the 0th index
				// this ordering solves these issues
				if (pts[j] == 0) 
					newEdge.SetVertices(pts[j], pts[(j + 1) % 3]);
				else
					newEdge.SetVertices(pts[(j + 1) % 3], pts[j]);
				edgeList.push_back(newEdge);
				// register the edge in its vertices
				verts[pts[j]].AddEdge(numberOfEdges);
				verts[pts[(j + 1) % 3]].AddEdge(numberOfEdges);
				numberOfEdges++;
			}
			else // check if the edge has different orientation in the neighbouring triangle to ensure normals pointing in the same direction
			{
				int *neighbVerts = tris[neighbours[j]].GetVertices();
				for (int ed = 0; ed < 3; ed++)
				{
					if (neighbVerts[ed] == pts[j]) // the first vertex found, the other should be on ed - 1
					{
						if (neighbVerts[(ed + 2) % 3] != pts[(j+1) % 3]) // if its not...
						{
							Log("\nWarning - inconsistent vertex ordering in input, repairing...\n");
							Log("", tris[neighbours[j]].GetVertices()[0]);
							Log(" ", tris[neighbours[j]].GetVertices()[1]);
							Log(" ", tris[neighbours[j]].GetVertices()[2]);
							int swap = neighbVerts[ed];	// ...swap vertices
							neighbVerts[ed] = neighbVerts[(ed + 2) % 3];
							neighbVerts[(ed + 2) % 3] = swap;
							Log("\n", tris[neighbours[j]].GetVertices()[0]);
							Log(" ", tris[neighbours[j]].GetVertices()[1]);
							Log(" ", tris[neighbours[j]].GetVertices()[2]);
						}
					}
				}
			}
		}
		tris[i].SetNeighbours(neighbours);
	}

	// convert the list with edges into an array
	Edge *edges = new Edge[numberOfEdges];
	for (int i  = 0; i < numberOfEdges; i++)
	{
		edges[i] = edgeList.front();
		edgeList.pop_front();
	}

	//compute the number of desirable points 
	targetPoints = numberOfVertices - (int)(numberOfVertices * TargetReduction);

	// call to the library that creates the hull
	ProgressiveHullCUDA::BuildResult* result = ProgressiveHullCUDA::BuildHull(
		verts, numberOfVertices, edges, numberOfEdges, tris, numberOfTriangles,
		targetPoints, (MeshRoughness + MESHROUGHNESS_ADDCOEFF) * MESHROUGHNESS_MULTCOEFF, MaxValence
		);	

	if (result->errorCode != 0)
	{
		Log("FATAL ERROR: ", result->errorCode);
		Log(const_cast< char* > (result->errorMsg));

		vtkPolyData* progHull = this->GetOutput();
		progHull->SetPoints(NULL);
		progHull->SetPolys(NULL);
	}
	else
	{
		////// create new mesh
		int edgeDif = numberOfEdges - result->newNumberOfEdges;
		// one collapse == removal of three edges and two triangles and one vertex
		int newNumberOfVerts = numberOfVertices - (edgeDif / 3);
		// prepare structures for the new mesh
		vtkPoints *newPoints = vtkPoints::New();	
		newPoints->SetNumberOfPoints(newNumberOfVerts);
		// fill the vertices
		int *vertIndexMap = new int[numberOfVertices]; // to map the old indices to the new indices
		for (int i = 0, toInsert = 0; i < numberOfVertices; i++)
		{
			if (result->newVertEdgeIndices[i * 2] != -1) // if this vertex was not deleted, add it
			{
				newPoints->SetPoint(toInsert, result->newVertCoordinates[i * 3], result->newVertCoordinates[i * 3 + 1], result->newVertCoordinates[i * 3 + 2]);
				vertIndexMap[i] = toInsert;
				toInsert++;
			}
		}
		// fill the triangles
		vtkCellArray *newTris = vtkCellArray::New();
		newTris->Initialize();
		vtkIdType t[3];
		for (int i = 0; i < numberOfTriangles; i++)
		{
			if (result->newTriangles[i * 3] != -1) // if this triangle was not deleted, add it
			{
				// remap the indices
				t[0] = vertIndexMap[result->newTriangles[i * 3]];
				t[1] = vertIndexMap[result->newTriangles[i * 3 + 1]];
				t[2] = vertIndexMap[result->newTriangles[i * 3 + 2]];
				newTris->InsertNextCell(3, t);
			}
		}
		// add vertices and triangles to the output object
		vtkPolyData* progHull = this->GetOutput();
		progHull->SetPoints(newPoints);
		progHull->SetPolys(newTris);

		Log("\nNew mesh information:\nNumber of vertices: ", newNumberOfVerts);
		Log("\nNumber of triangles: ", newTris->GetNumberOfCells());

		// free remaining cpu memory
		delete[] vertIndexMap;
	}

	delete result;
	
	delete[] tris;
	delete[] verts;
	delete[] edges;
}