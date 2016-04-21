/*========================================================================= 
Program: Musculoskeletal Modeling (VPHOP WP10)
Module: $RCSfile: vtkProgressiveHullCPU.cxx,v $ 
Language: C++ 
Date: $Date: 2012-04-03 13:43:53 $ 
Version: $Revision: 1.1.2.2 $ 
Authors: David Cholt, Tomas Janak
Notes:   Done raw cleanup, Done final cleanup
========================================================================== 
Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details 
=========================================================================
*/
#ifdef _WINDOWS
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4996)	//VTK uses strcpy, which is considered unsafe by VS
#include "vtkProgressiveHullCPU.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkCellArray.h"
#pragma warning(pop)

#include <float.h>
#include <time.h> // for optimalisation

#ifdef _DEBUG
#define _VTKPROGRESSIVEHULL_LOG
#endif

#ifdef _VTKPROGRESSIVEHULL_LOG
#define Log					vtkProgressiveHull::Log
#define FLog				vtkProgressiveHull::FLog
#else
#define Log __noop
#define FLog __noop
#endif

#pragma region Nested classes

vtkInstantiatorNewMacro(vtkProgressiveHullCPU);

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CVertex::CVertex(double *pCoord)
	//----------------------------------------------------------------------------
{
	DCoord[0] = pCoord[0];
	DCoord[1] = pCoord[1];
	DCoord[2] = pCoord[2];
	BDeleted = BBoundary = false;
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CVertex::~CVertex()
	//----------------------------------------------------------------------------
{
	OneRingVertex.clear();
	OneRingTriangle.clear();
	OneRingEdge.clear();
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CTriangle::CTriangle()
	//----------------------------------------------------------------------------
{
	AVertex[0]=AVertex[1]=AVertex[2]=-1;
	AEdge[0]=AEdge[1]=AEdge[2]=-1;
	BDeleted = false;
	Modified = false;
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CTriangle::CTriangle(int v0,int v1,int v2)
	//----------------------------------------------------------------------------
{
	AVertex[0]=v0;
	AVertex[1]=v1;
	AVertex[2]=v2;

	AEdge[0]=AEdge[1]=AEdge[2]=-1;
	BDeleted = false;
	Modified = false;
}

//----------------------------------------------------------------------------
void vtkProgressiveHullCPU::CTriangle::SetEdge(int e0,int e1,int e2)
	//----------------------------------------------------------------------------
{
	AEdge[0]=e0;
	AEdge[1]=e1;
	AEdge[2]=e2; 
}

//----------------------------------------------------------------------------
void vtkProgressiveHullCPU::CTriangle::SetNormal(double Normal[3])
	//----------------------------------------------------------------------------
{
	DNormal[0] = Normal[0];
	DNormal[1] = Normal[1];
	DNormal[2] = Normal[2];
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CEdge::CEdge()
	//----------------------------------------------------------------------------
{
	AVertex[0]=AVertex[1]=AVertex[2]=AVertex[3]=-1;
	ATriangle[0]=ATriangle[1]=-1;
	BDeleted = BBoundary = false;
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CEdge::CEdge(int v0,int v1)
	//----------------------------------------------------------------------------
{
	AVertex[0] = v0;
	AVertex[1] = v1;
	AVertex[2] = AVertex[3] = -1;
	ATriangle[0]=ATriangle[1]=-1;
	BDeleted = BBoundary = BMarked = false;
}

//----------------------------------------------------------------------------
vtkProgressiveHullCPU::CEdge::CEdge(int v0,int v1,int v2,int v3)
	//----------------------------------------------------------------------------
{
	AVertex[0] = v0;
	AVertex[1] = v1;
	AVertex[2] = v2;
	AVertex[3] = v3;
	ATriangle[0]=ATriangle[1]=-1;
	BDeleted = BBoundary = BMarked = false;
}

//----------------------------------------------------------------------------
void vtkProgressiveHullCPU::CEdge::SetTriangle(int t0,int t1)
	//----------------------------------------------------------------------------
{
	ATriangle[0]=t0;
	ATriangle[1]=t1;
}
//----------------------------------------------------------------------------
void vtkProgressiveHullCPU::CEdge::SetDeciPoint(double point[3])
	//----------------------------------------------------------------------------
{
	DeciPoint[0] = point[0];
	DeciPoint[1] = point[1];
	DeciPoint[2] = point[2];
	BMarked = true;
}
#pragma endregion Nested classes

#pragma region Support methods


//----------------------------------------------------------------------------
//retrieve instance of the class
/*static*/ vtkProgressiveHullCPU* vtkProgressiveHullCPU::New()
{
	return new vtkProgressiveHullCPU();
}

//----------------------------------------------------------------------------
// vtkProgressiveHullCPU Constructor
vtkProgressiveHullCPU::vtkProgressiveHullCPU()
	//----------------------------------------------------------------------------
{
	targetPoints = 0;
	TargetReduction = 0;
	MeshRoughness = 0.5;
	MeshQuality = 1;
	EnlargeMeshAmount = -1;
	lp = NULL;
}

//----------------------------------------------------------------------------
// vtkProgressiveHullCPU Destructor
vtkProgressiveHullCPU::~vtkProgressiveHullCPU()
	//----------------------------------------------------------------------------
{
	ClearMesh();
}

//----------------------------------------------------------------------------
// Calculates unsigned volume of tetrahedron defined by three vertices of triangle 
// tID and 4th point v
double vtkProgressiveHullCPU::TetraVolume(int tID, double v[3])
	//----------------------------------------------------------------------------
{
	// (v1 - v) . (v2 - v) x (v3 - v)
	double oMV1[3];		// origin moved vector 1
	double oMV2[3];		// origin moved vector 2
	double oMV3[3];		// origin moved vector 3

	for (int i = 0; i < 3; ++i) {
		oMV1[i] = Vertexes[Triangles[tID]->AVertex[0]]->DCoord[i] - v[i];
		oMV2[i] = Vertexes[Triangles[tID]->AVertex[1]]->DCoord[i] - v[i];
		oMV3[i] = Vertexes[Triangles[tID]->AVertex[2]]->DCoord[i] - v[i];
	}

	double cross[3];
	vtkMath::Cross(oMV2,oMV3,cross);
	return abs(vtkMath::Dot(oMV1,cross));
}

//----------------------------------------------------------------------------
// Calculates the volume of tetrahedron defined by three vertices of triangle
// and origin point (0,0,0)
double vtkProgressiveHullCPU::TriangleVolume(double v1[3],double v2[3],double v3[3])
	//----------------------------------------------------------------------------
{
	// (v1 . (v2 x v3))
	double cross[3];
	vtkMath::Cross(v2,v3,cross);
	return vtkMath::Dot(v1,cross);
}

//----------------------------------------------------------------------------
// Calculates the volume of tetrahedron defined by three vertices of given triangle
//  and origin point (0,0,0) - CTriangle parameter overload
double vtkProgressiveHullCPU::TriangleVolume(CTriangle* T)
	//----------------------------------------------------------------------------
{
	return TriangleVolume(Vertexes[T->AVertex[0]]->DCoord, Vertexes[T->AVertex[1]]->DCoord, Vertexes[T->AVertex[2]]->DCoord);
}

//----------------------------------------------------------------------------
// Checks if proposed decimation can produce weird spike artifacts using 
// tolerance parameter [-1:1].
// Checks inner triangle angles if checkAngles is set to true, using angle parameter [Radians].
// Returns true, if decimation is alowed.
bool vtkProgressiveHullCPU::CheckSpikeAndTriangles(CEdge* e, double tolerance, double angle, bool checkAngles)
	//----------------------------------------------------------------------------
{
	vtkstd::vector<int> adjs;	// adjacent triangles
	double newNormal[3];
	unsigned int pos = 0;
	vtkstd::vector<vtkstd::vector<double> > newNormals; // array of new normals. i had some problems with double**.
	unsigned int i = 0;			// edge vertex iterator
	unsigned int j = 0;			// adjacent triangle iterator
	int k = 0;  // triangle vertex
	int ID1 = 0;				// id of triangle's still vertex
	int ID2 = 0;				// id of triangle's still vertex
	int ID3 = 0;				// id of triangle's movable vertex
	double difference;			// difference between normals

	// Count all possible normals
	int count = (Vertexes[e->AVertex[0]]->OneRingTriangle.size() + Vertexes[e->AVertex[1]]->OneRingTriangle.size());
	newNormals.resize(count);

	// We need to go trough both OneRingTriangles of the edge vertices 
	for (i = 0; i < 2; i++)
	{
		adjs = Vertexes[e->AVertex[i]]->OneRingTriangle;
		for(j = 0; j < adjs.size(); j++)
		{
			// we need to find which triangle's vertex will be substituted in process of decimation
			for	(k = 0; k < 3; k++) {
				if (adjs[j] == e->ATriangle[0] || adjs[j] == e->ATriangle[1] || Triangles[adjs[j]]->BDeleted)
					continue;

				if (Triangles[adjs[j]]->AVertex[k] == e->AVertex[i])
				{
					// we perform "virtual" decimation using 2 triangle's vertices and previously computed edge's decimation point
					ID1 = Triangles[adjs[j]]->AVertex[(k+1)%3];
					ID2 = Triangles[adjs[j]]->AVertex[(k+2)%3];
					ID3 = Triangles[adjs[j]]->AVertex[(k+3)%3];

					// check triangle angles.
					if (checkAngles)
					{
						// compute all triangle angles
						double angle1New = ComputeTriangleAngle(Vertexes[ID1]->DCoord, Vertexes[ID2]->DCoord, e->DeciPoint);
						double angle2New = ComputeTriangleAngle(Vertexes[ID2]->DCoord, e->DeciPoint, Vertexes[ID1]->DCoord);
						double angle3New = ComputeTriangleAngle(e->DeciPoint, Vertexes[ID1]->DCoord, Vertexes[ID2]->DCoord);
						double min1;
						double min2;
						// find maximum
						min1 = (angle2New < angle3New)? angle2New: angle3New;
						min2 = (angle1New < min1) ? angle1New : min1;

						if (min2 < angle) // if max angle is greater then tolerated
						{
							// we check if it atleast gets smaller by this decimation. This is slow, so we expect to do it less often.
							bool result = false;
							if (min2 == angle1New) // we can do this since its assigned
								result = angle > ComputeTriangleAngle(Vertexes[ID1]->DCoord, Vertexes[ID2]->DCoord, Vertexes[ID3]->DCoord);
							if (min2 == angle2New)
								result = angle > ComputeTriangleAngle(Vertexes[ID2]->DCoord, Vertexes[ID3]->DCoord, Vertexes[ID1]->DCoord);
							if (min2 == angle3New)
								result = angle > ComputeTriangleAngle(Vertexes[ID3]->DCoord, Vertexes[ID1]->DCoord, Vertexes[ID2]->DCoord);	
							if (result == false) // it doesnt, we don't decimate
								return false;
						}
					}


					// and compute normal of the new triangle
					ComputeTriangleNormal(Vertexes[ID1]->DCoord, Vertexes[ID2]->DCoord, e->DeciPoint, newNormal);
					// and we save the normal
					newNormals[pos].resize(3);
					newNormals[pos][0]=newNormal[0];
					newNormals[pos][1]=newNormal[1];
					newNormals[pos][2]=newNormal[2];
					pos++;
					break;
				}
			}

		}
	}

	// we will compare 2 normals in each iteration
	double first[3];
	double second[3];

	for (i = 0; i < pos; i++)
	{
		for (j = 0; j < pos; j++) {
			first[0] = newNormals[i][0];
			first[1] = newNormals[i][1];
			first[2] = newNormals[i][2];
			second[0] = newNormals[j][0];
			second[1] = newNormals[j][1];
			second[2] = newNormals[j][2];
			vtkMath::Normalize(first);
			vtkMath::Normalize(second);
			// now we compute difference between triangle normals. For spikes, this difference is big, 
			// for relatively flat surface it's small.
			difference = acos(vtkMath::Dot(first, second));
			double maxAngle = vtkMath::Pi() * tolerance;
			if (difference < 0)
			{
				return false;
			}	
			if (difference > maxAngle)
			{
				return false;
			}
		}
	}	

	return true;
}

//----------------------------------------------------------------------------
// Calculates the normal of triangle T and updates it
void vtkProgressiveHullCPU::UpdateTriangleNormal(CTriangle* T){
	//----------------------------------------------------------------------------
	double nCV1[3];		// normal computation vector 1
	double nCV2[3];		// normal computation vector 2
	double norm[3];		// normal

	int v0,v1,v2;			// Triangle vertices indices
	v0 = T->AVertex[0];
	v1 = T->AVertex[1];
	v2 = T->AVertex[2];

	// Create two vectors for cross product
	for (int i = 0; i < 3; ++i)
		nCV1[i] = Vertexes[v1]->DCoord[i] - Vertexes[v0]->DCoord[i];
	for (int i = 0; i < 3; ++i)
		nCV2[i] = Vertexes[v2]->DCoord[i] - Vertexes[v0]->DCoord[i];
	vtkMath::Cross(nCV1, nCV2, norm);
	vtkMath::Normalize(norm); // Normals need to be normalised = have length of 1
	T->SetNormal(norm);
}

//----------------------------------------------------------------------------
// Computes largest triangle's inner angle
double vtkProgressiveHullCPU::ComputeTriangleAngle(double a[3], double b[3], double c[3])
	//----------------------------------------------------------------------------
{
	double nCV1[3];		// angle computation vector 1
	double nCV2[3];		// angle computation vector 2
	for (int i = 0; i < 3; ++i)
		nCV1[i] = b[i] - a[i];
	for (int i = 0; i < 3; ++i)
		nCV2[i] = c[i] - a[i];
	vtkMath::Normalize(nCV1);
	vtkMath::Normalize(nCV2);
	double result = vtkMath::Dot(nCV1, nCV2);
	return acos(result);
}

//----------------------------------------------------------------------------
// Calculates the normal of triangle a,b,c
void vtkProgressiveHullCPU::ComputeTriangleNormal(double a[3], double b[3], double c[3], double normal[3]){
	//----------------------------------------------------------------------------
	double nCV1[3];		// normal computation vector 1
	double nCV2[3];		// normal computation vector 2

	// Create two vectors for cross product
	for (int i = 0; i < 3; ++i)
		nCV1[i] = b[i] - a[i];
	for (int i = 0; i < 3; ++i)
		nCV2[i] = c[i] - a[i];
	vtkMath::Cross(nCV1, nCV2, normal);
	vtkMath::Normalize(normal); // Normals need to be normalised = have length of 1
}

//----------------------------------------------------------------------------
// Makes Cross product of 2 triangle vertices that differ from reference point.
void vtkProgressiveHullCPU::MakeCross(int triangleID, int refPointID, double cross[3])
	//----------------------------------------------------------------------------
{
	int i;
	for	(i = 0; i < 3; i++)
	{
		if (Triangles[triangleID]->AVertex[i] == refPointID){
			int id1 = Triangles[triangleID]->AVertex[(i+1)%3];
			int id2 = Triangles[triangleID]->AVertex[(i+2)%3];
			vtkMath::Cross(Vertexes[id1]->DCoord, Vertexes[id2]->DCoord, cross);
		}
	}
}

//----------------------------------------------------------------------------
// Creates triangle instance from specified data.
vtkProgressiveHullCPU::CTriangle* vtkProgressiveHullCPU::CreateTriangle(vtkIdList *ptids, int v0ID,int v2ID, int v3ID, int tID)
	//----------------------------------------------------------------------------
{
	int v0,v1,v2;			// Triangle vertices indices
	CTriangle *pTriangle; // New triangle added to internal structure

	v0 = ptids->GetId(v0ID);
	v1 = ptids->GetId(v2ID);            
	v2 = ptids->GetId(v3ID);            
	pTriangle = new CTriangle(v0,v1,v2);
	pTriangle->Id = tID;
	pTriangle->Volume = TriangleVolume(Vertexes[v0]->DCoord,Vertexes[v1]->DCoord,Vertexes[v2]->DCoord);
	UpdateTriangleNormal(pTriangle);
	// Add this triangle's oriented volume to the volume of the whole mesh
	OriginalVolume += pTriangle->Volume;
	return pTriangle;
}
#pragma endregion Support methods

#pragma region Filter execution methods
//----------------------------------------------------------------------------
// Initializes mesh data from input, stores mesh data to STL structures
void vtkProgressiveHullCPU::InitMesh()
	//----------------------------------------------------------------------------
{
	int i;				// Vertex iteration
	int t;				// Triangle counter
	int CellCount;		// Cell count
	double pCoord[3];		// Triangle vertices coords
	CVertex *pVertex;		// New vertex added to internal structure
	vtkIdList *ptids;     // Vertex IDs

	InputMesh = this->GetInput();   // Link Input
	OutputMesh = this->GetOutput(); // Link Output

	OriginalVolume = 0;
	VolumeDifference = 0;

	NumOfVertex = InputMesh->GetNumberOfPoints();
	actualPoints = NumOfVertex;
	targetPoints = actualPoints - (int)(actualPoints * TargetReduction);

	FLog(TargetReduction, false);
	FLog(actualPoints, false);
	FLog(MeshRoughness, false);
	FLog(MeshQuality, false);


	Vertexes.resize(NumOfVertex);

	// Iterate trough all input vertices and store them in internal structure
	for(i=0;i<NumOfVertex;i++)
	{
		// Create vertex
		InputMesh->GetPoint(i,pCoord);
		pVertex = new CVertex(pCoord);
		pVertex->Id = i;

		//6 is here since vertex valence is usually about 6
		pVertex->OneRingTriangle.reserve(6);
		pVertex->OneRingEdge.reserve(6);
		pVertex->OneRingVertex.reserve(6);

		Vertexes[i] = pVertex;
	}

	InputMesh->BuildCells();
	CellCount = InputMesh->GetNumberOfCells(); 

	// Depending on format of cells, we need to resize Triangles vector.
	// This way is the vector resized only once, not with every additional triangle (faster)
	// I'm assuming that all cells are the same format as the first one (if not, can cause errors in data)
	NumOfTriangle = CellCount;

	if (InputMesh->GetCell(0)->GetCellType() != 5) 
		NumOfTriangle*=2;
	Triangles.resize(NumOfTriangle);
	t = 0;

	// Iterate trough all input cells and store their data in internal structure
	// Get vertices of all cells
	for(i=0; i < CellCount; i++)
	{
		ptids = InputMesh->GetCell(i)->GetPointIds();
		// Parse cell depending on CellType
		switch (InputMesh->GetCell(i)->GetCellType()) {
		case 5:  // Cell is triangle
			Triangles[t] = CreateTriangle(ptids, 0, 1, 2, t);
			t++;
			break;
		case 6:  // Cell contains striped triangles (used in vtk files)
			Triangles[t] = CreateTriangle(ptids, 0, 1, 2, t);
			t++;
			Triangles[t] = CreateTriangle(ptids, 0, 3, 2, t);
			t++;
			break;
		case 9:  // Cell contains quads (used in obj files)
			Triangles[t] = CreateTriangle(ptids, 0, 1, 2, t);
			t++;
			Triangles[t] = CreateTriangle(ptids, 0, 2, 3, t);
			t++;
			break;
		default:
			Log("Error. Unknown format in cell ", i);
			break;
		}
	}
	Log("  Num Of Triangles: ", NumOfTriangle);
	Log("  Num Of Vertex: ", NumOfVertex);
	Log("  Original Mesh Volume: ", OriginalVolume);
#ifdef _USE_LP_SOLVE
	lp = make_lp(0, 3); // create lp model with zero rows and 3 cols, used in SolveLP
#endif
}
//----------------------------------------------------------------------------
// Builds the internal relationship of a mesh, creates edges, finds neigbors for all primitives
void vtkProgressiveHullCPU::BuildMesh()
	//----------------------------------------------------------------------------
{
	int i,j,t;     // Iteration
	int sv1,sv2;   // New edge vertices
	int dv1,dv2;   // Neighbor vertices

	bool bflag;	 // Indicates boundary edge

	int *pVertexIndex,*pVertexIndex2,*pEdgeIndex;

	CEdge		*pEdge;
	CTriangle	*pTriangle, *pNeighTriangle;
	CVertex	*pVertex,*pVertex1,*pVertex2;

	// Locate one ring triangles for all vertices
	// Iterate trough triangles
	for( t=0; t<NumOfTriangle; t++)
	{
		pVertexIndex = Triangles[t]->AVertex;
		//each triangle has three vertices, each of these has the triangle in one ring
		for(i=0;i<3;i++)
		{
			pVertex = Vertexes[pVertexIndex[i]];
			//add the triangle to the vertex as an first degree ring triangle.
			pVertex->OneRingTriangle.push_back(t);
		}
	}

	NumOfEdge = 0;
	Edges.resize(3*NumOfTriangle);

	// Locate and create one ring edges and vertices
	for( t=0; t<NumOfTriangle; t++)
	{
		pTriangle = Triangles[t];		
		pVertexIndex = pTriangle->AVertex;
		pEdgeIndex = pTriangle->AEdge;
		//each triangle has three vertices;
		for(i=0;i<3;i++)
		{
			if( pEdgeIndex[i] >= 0 )				continue; // edge is already found

			pEdgeIndex[i] = NumOfEdge;

			sv1 = pVertexIndex[i]; // First edge vertex ID
			sv2 = pVertexIndex[(i+1)%3]; // Second edge vertex ID

			pVertex1 = Vertexes[sv1]; // First edge vertex
			pVertex2 = Vertexes[sv2]; // Second edge vertex 

			pVertex1->OneRingVertex.push_back(sv2);  // Add Second vertex to First Vertex's one ring
			pVertex2->OneRingVertex.push_back(sv1);  // Vice versa
			pVertex1->OneRingEdge.push_back(NumOfEdge); // Both vertices share
			pVertex2->OneRingEdge.push_back(NumOfEdge); // the same edge

			pEdge = new CEdge(sv1,sv2); // Create new found edge
			pEdge->Id = NumOfEdge;
			pEdge->AVertex[2] = pVertexIndex[(i+2)%3];
			pEdge->ATriangle[0] = t;
			Edges[NumOfEdge] = pEdge;

			bflag = true;
			// Now, each edge has 2 neighbor triangles, we must find the second one for each edge
			int tricount = (int)pVertex1->OneRingTriangle.size();      
			for(int k = 0; k < tricount; k++ )
			{
				int neight = pVertex1->OneRingTriangle[k];
				//as triangles are ascending ordered in vertices
				//any neighbouring triangle associated with pVertex1 
				//having lower id was already processed previously
				if( neight <= t) continue;
				pNeighTriangle = Triangles[neight];	
				pVertexIndex2 = pNeighTriangle->AVertex;

				for(j=0; j<3; j++)
				{
					// when we find the edge of the neighbor triangle, continue to find 
					// next edge of the neighbor triangle.
					if( pNeighTriangle->AEdge[j] >=0 ) continue;	

					// neighbor orientation of triangles should be consistent
					dv1 = pVertexIndex2[j];
					dv2 = pVertexIndex2[(j+1)%3];

					if (sv2 == dv2 && sv1 == dv1)
					{
						//the mesh is not properly oriented, the neighbouring triangle has orientation different
						//from the currently processed triangle => this edge is not decimated to prevent errors
						Log("  Warning: Inconsistent orientation of triangles on edge ", NumOfEdge);
						break;
					}

					if( sv2 == dv1 && sv1==dv2 )
					{
						pNeighTriangle->AEdge[j] = NumOfEdge;
						if( pEdge->ATriangle[1] < 0 )
						{
							// If the mesh is manifold, each edge only have two adjacent triangles.
							pEdge->AVertex[3] = pVertexIndex2[(j+2)%3];
							pEdge->ATriangle[1] = neight;
							bflag = false;
						}
						else
						{
							// If the mesh is non-manifold, each edge have more than two adjacent triangles.
							// This edge is not decimated, but since triangles do overlap now, errors may still occur
							Log("  Warning: More than two adjacent triangles on edge ", NumOfEdge);
						}
						break;						
					}          
				}
			}
			if( bflag == true )
			{
				//found a boundary edge (or inconsistent or else defected)
				pEdge->BBoundary = true;
				Vertexes[pEdge->AVertex[0]]->BBoundary = true;
				Vertexes[pEdge->AVertex[1]]->BBoundary = true;
			}
			NumOfEdge++;
		}
	}
	Edges.resize(NumOfEdge);
	Log("  Num Of Edges: ", NumOfEdge);
}

//----------------------------------------------------------------------------
// Executes filter
void vtkProgressiveHullCPU::Execute()
	//----------------------------------------------------------------------------
{
	clock_t whole,start; // benchmark

	whole = clock();

	Log("Filter executed\n\nInitializing mesh...");
	InitMesh();
	Log("Mesh initialized\n\nBuilding relationships...");
	BuildMesh();
	Log("Mesh built\n\nComputing progressive hull...");

	Log("Prioritizing edges...");
	start = clock();
	PrioritizeEdges();
	Log("Prioritizing time: ", (int)(clock() - start));
	FLog((int)(clock() - start), false);
	Log("Decimating...");
	Decimate();

	EnlargeMesh();
	DoneMesh();
	Log("Mesh done\n\nClearing...");
	ClearMesh();
	Log("Mesh cleared\n\nExecution successfull.");
	Log("Execution time: ", (int)(clock() - whole));
	FLog((int)(clock() - start), false);
	FLog(actualPoints, true);
}

//----------------------------------------------------------------------------
// Compute decimation priority for all edges in mesh.
// Puts edges to priority queue with this priority (low to high).
void vtkProgressiveHullCPU::PrioritizeEdges()
	//----------------------------------------------------------------------------
{
	int e;				// edge iterator
	double priority;	// edge priority

	// Go through all edges
	for (e = 0; e < NumOfEdge; e++)
	{
		priority = ComputePriorityFast(e);   // Too slow
		if (priority >= 0)
			Queue.Put(priority, Edges[e]);
	}
}

//----------------------------------------------------------------------------
// Tries to compute decimation point of specified edge. If decimation is possible,
// setups edge for decimation
void vtkProgressiveHullCPU::ComputeDecimationPoint(int edgeID)
	//----------------------------------------------------------------------------
{
	CEdge* pEdge;		// iterated edge

	int adjT;			// Adjacent triangles iterator
	vtkstd::vector<int> adjTs; // Triangles adjacent to edge vertex
	int numTriangles;	// Number of triangles adjacent to edge vertex
	double volumeGain = 0;   // Computed priority

	double holeVolume = 0;  // Volume of hole triangles together (old hole - F_i+1)

	double cross[3];	// Cross product of 2 of the triangle vertices

	vtkstd::vector<int> ATriangles; // All adjacent triangles in hole, for lp_solve
	double a = 0, b = 0, c = 0;		// Parameters of lp_solve objective
	double newHoleVolume;	// Result of ip_solve
	double newV[3];			// Position of vector V used for decimation (will substitute the edge)

	pEdge = Edges[edgeID];
	if (pEdge->BDeleted)
		// Edge is already deleted, skip
		return;

	if (pEdge->BBoundary || Vertexes[pEdge->AVertex[0]]->BBoundary || Vertexes[pEdge->AVertex[1]]->BBoundary)	
		//  Edge or its vertices are boundary, skip
		return;

	// First we iterate trough triangles adjacent to edge's [0] vertex
	adjTs = Vertexes[pEdge->AVertex[0]]->OneRingTriangle;
	numTriangles = adjTs.size();
	// Add triangle volume information to holeVolume and extract data for ip_solve
	// objective function (holeVolume in F_i)
	for (adjT = 0; adjT < numTriangles; adjT++)
	{
		// We skip adjacent edge triangles as they are not in F_i
		if (adjTs[adjT] == pEdge->ATriangle[0] || adjTs[adjT] == pEdge->ATriangle[1])
			continue;
		// We skip deleted triangles as they don't affect F_i+1 nor F_i
		if (Triangles[adjTs[adjT]]->BDeleted)
			continue;
		holeVolume += Triangles[adjTs[adjT]]->Volume;
		ATriangles.push_back(adjTs[adjT]);
		MakeCross(adjTs[adjT], pEdge->AVertex[0], cross);
		a += cross[0];
		b += cross[1];
		c += cross[2];
	}

	// Second we iterate trough triangles adjacent to edge's [1] vertex
	adjTs = Vertexes[pEdge->AVertex[1]]->OneRingTriangle;
	numTriangles = adjTs.size();
	// Add triangle volume information to old holeVolume and extract data for ip_solve
	// objective function (holeVolume in F_i)
	for (adjT = 0; adjT < numTriangles; adjT++)
	{
		// We skip adjacent edge triangles as they are not in F_i
		if (adjTs[adjT] == pEdge->ATriangle[0] || adjTs[adjT] == pEdge->ATriangle[1])
			continue;
		// We skip deleted triangles as they cant be in F_i
		if (Triangles[adjTs[adjT]]->BDeleted)
			continue;

		holeVolume += Triangles[adjTs[adjT]]->Volume;
		ATriangles.push_back(adjTs[adjT]);
		MakeCross(adjTs[adjT], pEdge->AVertex[1], cross);
		a += cross[0];
		b += cross[1];
		c += cross[2];
	}

	// Now we add edge's adjacent triangle volumes to old holeVolume. 
	// These will not be in new mesh if the edge is decimated,
	// so they will not have any role in new volume
	// but they are needed for computation in lp_solve, so we push them to list
	holeVolume += Triangles[pEdge->ATriangle[0]]->Volume;
	holeVolume += Triangles[pEdge->ATriangle[1]]->Volume;
	ATriangles.push_back(pEdge->ATriangle[0]);
	ATriangles.push_back(pEdge->ATriangle[1]);

	// Solve the LP, returning best new vertex V position and volume change
	if (SolveLP(ATriangles, a, b, c, newHoleVolume, newV) == 0){
		volumeGain = (newHoleVolume - holeVolume);
		pEdge->SetDeciPoint(newV); // Sets BMarked too
		pEdge->DeciVolumeChange = volumeGain;
	} else {
		pEdge->BMarked = false; // this edge is not ready for decimations
	}
}

//----------------------------------------------------------------------------
// Computes priority of edge decimation. This fast method is not using lp_solve,
// only average of edge star points to determine priority.
// This method computes only volume change part of priority, global previous volume
// change has to be added to result of this method!
double vtkProgressiveHullCPU::ComputePriorityFast(int edgeID) {
	//----------------------------------------------------------------------------
	CEdge* pEdge;		// iterated edge
	int adjT;			// Adjacent triangles iterator
	vtkstd::vector<int> adjTs; // Triangles adjacent to edge vertex

	unsigned int starV;	// Star vector iterator
	int starID;			// Vector ID for duplicity check
	vtkstd::vector<int> starVs; // Star vector of the edge

	double priority = 0;   // Computed priority

	int numTriangles;	// Number of triangles adjacent to edge vertex
	double average[3] = {0,0,0}; // Average position of all points in edge's star

	pEdge = Edges[edgeID];
	if (pEdge->BDeleted)
		// Edge is already deleted, skip
		return -1;

	if (pEdge->BBoundary || Vertexes[pEdge->AVertex[0]]->BBoundary || Vertexes[pEdge->AVertex[1]]->BBoundary)	
		//  Edge or its vertices are boundary, skip
		return -1;

	// We construct list of all star vertices
	starVs = Vertexes[pEdge->AVertex[0]]->OneRingVertex;
	starVs.reserve(starVs.size() + Vertexes[pEdge->AVertex[1]]->OneRingVertex.size());
	for (starV = 0; starV < Vertexes[pEdge->AVertex[1]]->OneRingVertex.size(); starV++)
	{
		starID = Vertexes[pEdge->AVertex[1]]->OneRingVertex[starV];
		// We need to exclude duplicate vertices in star as we need accurate vertex average in the end
		if (starVs.end() == vtkstd::find(starVs.begin(), starVs.end(), starID))
			starVs.push_back(starID); // not found in star, insert
	}

	for (starV = 0; starV < starVs.size(); starV++)
	{
		average[0] += Vertexes[starVs[starV]]->DCoord[0];
		average[1] += Vertexes[starVs[starV]]->DCoord[1];
		average[2] += Vertexes[starVs[starV]]->DCoord[2];
	}
	average[0] /= starVs.size();
	average[1] /= starVs.size();
	average[2] /= starVs.size();

	// First we iterate trough triangles adjacent to edge's [0] vertex
	adjTs = Vertexes[pEdge->AVertex[0]]->OneRingTriangle;
	numTriangles = adjTs.size();
	// We compute the volume of tetragedron of the triangle and average point
	for (adjT = 0; adjT < numTriangles; adjT++)
	{
		// We skip adjacent edge triangles to exclude dupliscityes
		if (adjTs[adjT] == pEdge->ATriangle[0] || adjTs[adjT] == pEdge->ATriangle[1])
			continue;
		// We skip deleted triangles as they don't affect F_i+1 nor F_i
		if (Triangles[adjTs[adjT]]->BDeleted)
			continue;
		priority += TetraVolume(adjTs[adjT], average);
	}

	// Second we iterate trough triangles adjacent to edge's [1] vertex
	adjTs = Vertexes[pEdge->AVertex[1]]->OneRingTriangle;
	numTriangles = adjTs.size();
	// We compute the volume of tetragedron of the triangle and average point
	for (adjT = 0; adjT < numTriangles; adjT++)
	{
		// We skip adjacent edge triangles as they are not in F_i
		if (adjTs[adjT] == pEdge->ATriangle[0] || adjTs[adjT] == pEdge->ATriangle[1])
			continue;
		// We skip deleted triangles as they cant be in F_i
		if (Triangles[adjTs[adjT]]->BDeleted)
			continue;

		priority += TetraVolume(adjTs[adjT], average);
	}
	// Now we consider edge adjacent triangles' volume
	priority += TetraVolume(pEdge->ATriangle[0], average);
	priority += TetraVolume(pEdge->ATriangle[1], average);

	return priority + VolumeDifference;
}

//----------------------------------------------------------------------------
// Solves the linear programming model to determine edge substitution vector 
// position. Returns 0 if solution is valid and optimal.
int vtkProgressiveHullCPU::SolveLP(vtkstd::vector<int>ATriangles, double a, double b, double c, double& outVolume, double* V)
	//----------------------------------------------------------------------------
{
#ifndef _USE_LP_SOLVE
	return 0;
#else
	int *colno = NULL, j, ret = 0;        // column pointer, column iterator, return value
	double *row = NULL;					  // row pointer
	CTriangle* ATriangle;				  // adjacent triangle from list, to get triangle normal
	double rh;							  // right side of the constrain

	if(lp == NULL)
		ret = 1; // lp not allocated

	/* We need to build the lp_solve model
	We start with cleaning a model to 0 rows and 3 columns as we use same model over and over again */
	if(ret == 0) {
		resize_lp(lp,0,3); // since lp_solve isn't ment to reuse, this is "equivalent" of lp_clear()
		resize_lp(lp, ATriangles.size(), 3); // reallocate memory for constrains, is faster for many constrains
	}

	if(ret == 0) {
		// let's name our variables. Not required, but can be useful for debugging
		set_col_name(lp, 1, "x");
		set_col_name(lp, 2, "y");
		set_col_name(lp, 3, "z");

		// create space large enough for one row
		colno = (int *) malloc(3 * sizeof(*colno));  // column (variavble) numbers, x = 1, y = 2, z = 3
		row = (double *) malloc(3 * sizeof(*row));   // row data
		if((colno == NULL) || (row == NULL))
			ret = 2;
	}

	// All variables have build-in lower bounds of zero constrain by default. We need to override that.
	set_bounds(lp, 1, -DBL_MAX, DBL_MAX);
	set_bounds(lp, 2, -DBL_MAX, DBL_MAX);
	set_bounds(lp, 3, -DBL_MAX, DBL_MAX);

	/* Constructs constrain	(n_x * x + n_y * y + n_z * z >= n*v) for each triangle. 
	This constrain says that the [x,y,z] point has to be above the triangle
	(in halfspace defined by this triangle, which is in triangle's normal direction)*/

	// First create the objective - faster (according to documentation)
	/* set the objective function (a * x + b * y + c * z) */
	j = 0;

	colno[j] = 1; // first column
	row[j++] = a;

	colno[j] = 2; // second column
	row[j++] = b;

	colno[j] = 3; // third column
	row[j++] = c;

	// set the objective in lpsolve
	if(!set_obj_fnex(lp, j, row, colno))
		ret = 4;

	set_add_rowmode(lp, TRUE);  // makes building the model faster if it is done row by row

	while(ret == 0 && !ATriangles.empty()) { // creates constrain for every adjacent triangle
		ATriangle = Triangles[ATriangles.back()];

		j = 0;

		colno[j] = 1; // first column  
		row[j++] = ATriangle->DNormal[0];
		colno[j] = 2; // second column  
		row[j++] = ATriangle->DNormal[1];
		colno[j] = 3; // third column  
		row[j++] = ATriangle->DNormal[2];

		rh = vtkMath::Dot(ATriangle->DNormal, Vertexes[ATriangle->AVertex[0]]->DCoord);

		// add this constrain row to lpsolve
		if(!add_constraintex(lp, j, row, colno, GE, rh))
			ret = 3;
		ATriangles.pop_back();
	}
	if(ret == 0) {
		set_add_rowmode(lp, FALSE); // rowmode should be turned off again when done building the model
	}

	if(ret == 0) {
		// Set the object direction to minimize
		set_minim(lp);

		// We only want to see critical messages on screen while solving
		set_verbose(lp, CRITICAL);

		// Now let lpsolve calculate a solution
		ret = solve(lp);
		if(ret == OPTIMAL)
			ret = 0;
		else
			ret = 5;
	}

	if(ret == 0) {
		// A solution is calculated, now lets get some results

		// objective value represents new volume, can be used as priority
		outVolume = get_objective(lp);

		// calculated x, y and z are coords of substituting vertex
		get_variables(lp, row);
		for(j = 0; j < 3; j++) {
			V[j] = row[j];
		}
	}

	// free allocated memory
	if(row != NULL)
		free(row);
	if(colno != NULL)
		free(colno);
	// LP solver itself will be deleted in ClearMesh();
	return(ret);
#endif
}

//----------------------------------------------------------------------------
// Picks up edges from the queue and sends them to decimation process
void vtkProgressiveHullCPU::Decimate() {
	//----------------------------------------------------------------------------
	while (targetPoints < actualPoints)
	{
		if (!Queue.IsEmpty())
		{
			Substitute(Queue.Get());  // Pickup edge from queue and decimate it
		} else {
			Log("Nothing left to decimate! Actual Points: ", actualPoints);
			break;
		}		
	} //end whle
}

//----------------------------------------------------------------------------
// Substitutes CEdge e with one point, preserving topology of the
// mesh in the process. Searches for edges, that are invalidated by this 
// substitution, and calls their priority change
void vtkProgressiveHullCPU::Substitute(CEdge* e) {
	//----------------------------------------------------------------------------
	int adj;					// Adjacent element iterator
	vtkstd::vector<int> adjs;	// Adjacent elements
	vtkstd::vector<int>::iterator it; // For erasing
	int numPrimitives;			// Number of triangles adjacent to edge vertex
	int delVertexID;
	int newVertexID;
	unsigned int i, j;
	CEdge *e1, *e2, *etmp;		// Edges for relationship reconstruction
	CEdge *skipE[2];			// Skipped edges
	int	moveID;					// ID of triangle to move from deleted edge

	ComputeDecimationPoint(e->Id); // We need decimation point prepared for this edge

	// Edge must be prepared for substitution
	if (!e->BMarked)
		return;
	// Edge must not be already processed
	if (e->BDeleted)
		return;

	// Decimation has to be... nice
	if (!CheckSpikeAndTriangles(e, MeshRoughness, MeshQuality * 10 / 180.0f * vtkMath::Pi(), true))
		return;

	// extract data from passed edge
	newVertexID = e->AVertex[0];
	delVertexID = e->AVertex[1];
	Vertexes[newVertexID]->DCoord[0] = e->DeciPoint[0];
	Vertexes[newVertexID]->DCoord[1] = e->DeciPoint[1];
	Vertexes[newVertexID]->DCoord[2] = e->DeciPoint[2];
	VolumeDifference += e->DeciVolumeChange;

	// recalculate volumes of e0 adjacent triangles
	adjs = Vertexes[newVertexID]->OneRingTriangle;
	numPrimitives = adjs.size();
	for (adj = 0; adj < numPrimitives; adj++)
	{
		// We skip edge adjacent triangles as they are not in F_i
		if (adjs[adj] == e->ATriangle[0] || adjs[adj] == e->ATriangle[1])
			continue;
		Triangles[adjs[adj]]->Volume = TriangleVolume(Triangles[adjs[adj]]);
		Triangles[adjs[adj]]->Modified = true;	 // And those are modified
		UpdateTriangleNormal(Triangles[adjs[adj]]);
	}

	// first we join triangles to newVertexID
	adjs = Vertexes[delVertexID]->OneRingTriangle;
	numPrimitives = adjs.size();
	for (adj = 0; adj < numPrimitives; adj++)
	{
		// We skip edge adjacent triangles as they are not in F_i
		if (adjs[adj] == e->ATriangle[0] || adjs[adj] == e->ATriangle[1])
			continue;
		for	(i = 0; i < 3; i++)
		{
			if (Triangles[adjs[adj]]->AVertex[i] == delVertexID)
				Triangles[adjs[adj]]->AVertex[i] = newVertexID;
		}
		// recalculate volumes of e1 adjacent triangles
		Triangles[adjs[adj]]->Volume = TriangleVolume(Triangles[adjs[adj]]);
		UpdateTriangleNormal(Triangles[adjs[adj]]);
		Triangles[adjs[adj]]->Modified = true;
		Vertexes[newVertexID]->OneRingTriangle.push_back(adjs[adj]);
	}

	// second we need to rebuild relationships in mesh excluding deleted edges and triangles
	for	(j = 0; j < 2; j++)
	{
		e1 = NULL;
		e2 = NULL;
		for	(i = 0; i < 3; i++)
		{
			// iterate though all edges of both adjacent triangles to edge e
			etmp = Edges[Triangles[e->ATriangle[j]]->AEdge[i]];
			if (etmp->Id != e->Id)
			{
				if (etmp->AVertex[0] == newVertexID || etmp->AVertex[1] == newVertexID)
					// This edge will remain in structure, but will have to be altered
					e1 = etmp;
				if (etmp->AVertex[0] == delVertexID || etmp->AVertex[1] == delVertexID)
					// This edge will be deleted, but we need the information it holds
					e2 = etmp;
			}
		}
		if (e1 == NULL || e2 == NULL)
		{
			Log("Error: Something went wrong on edge ", e->Id);
			Log("Error: Mesh will most probably be inconsistent");
			return;
		}
		moveID = (e2->ATriangle[0] == e->ATriangle[j]) ? e2->ATriangle[1] : e2->ATriangle[0];

		e2->BDeleted = true;
		Queue.Remove(e2);

		// rebuild triangle relations considering deleted edges
		if (e1->ATriangle[0] == e->ATriangle[j]) 
			e1->ATriangle[0] = moveID;
		else
			e1->ATriangle[1] = moveID;
		for (i = 0; i < 3; i++)
		{
			if (Triangles[moveID]->AEdge[i] == e2->Id) {
				Triangles[moveID]->AEdge[i] = e1->Id;
				break;
			}
		}
		Triangles[e->ATriangle[j]]->BDeleted = true;
		skipE[j] = e2;
	}

	// third we join edges from delVertexID to newVertexID
	adjs = Vertexes[delVertexID]->OneRingEdge;
	numPrimitives = adjs.size();
	for (adj = 0; adj < numPrimitives; adj++)
	{
		int testID = Edges[adjs[adj]]->Id;
		if (Edges[adjs[adj]]->AVertex[0] == delVertexID)
			Edges[adjs[adj]]->AVertex[0] = newVertexID;
		else
			Edges[adjs[adj]]->AVertex[1] = newVertexID;
		if (adjs[adj] != e->Id && adjs[adj] != skipE[0]->Id && adjs[adj] != skipE[1]->Id && !Edges[adjs[adj]]->BDeleted)
			Vertexes[newVertexID]->OneRingEdge.push_back(adjs[adj]);
	}

	// next, we redo the oneringvertexvertex for new vertex
	adjs = Vertexes[delVertexID]->OneRingVertex;
	numPrimitives = adjs.size();
	for (adj = 0; adj < numPrimitives; adj++)
	{
		// we need to remove duplicity
		it = find(Vertexes[adjs[adj]]->OneRingVertex.begin(), Vertexes[adjs[adj]]->OneRingVertex.end(), newVertexID);
		if (it != Vertexes[adjs[adj]]->OneRingVertex.end())
			Vertexes[adjs[adj]]->OneRingVertex.erase(it);
		// Change occurences of deleted vertex to new wertex
		vtkstd::replace(Vertexes[adjs[adj]]->OneRingVertex.begin(), Vertexes[adjs[adj]]->OneRingVertex.end(), delVertexID, newVertexID);
		// Insert vertex to newVertex onering (no duplicates)
		it = find(Vertexes[newVertexID]->OneRingVertex.begin(), Vertexes[newVertexID]->OneRingVertex.end(), adjs[adj]);
		if (it == Vertexes[newVertexID]->OneRingVertex.end()) 
			Vertexes[newVertexID]->OneRingVertex.push_back(adjs[adj]);
	}
	// remove selfreference and deleted vertex reference
	it = find(Vertexes[newVertexID]->OneRingVertex.begin(), Vertexes[newVertexID]->OneRingVertex.end(), newVertexID);
	if (it != Vertexes[newVertexID]->OneRingVertex.end()) 
		Vertexes[newVertexID]->OneRingVertex.erase(it);
	it = find(Vertexes[newVertexID]->OneRingVertex.begin(), Vertexes[newVertexID]->OneRingVertex.end(), delVertexID);
	if (it != Vertexes[newVertexID]->OneRingVertex.end()) 
		Vertexes[newVertexID]->OneRingVertex.erase(it);

	// delete delVertex
	Vertexes[delVertexID]->BDeleted = true;
	actualPoints--;
	Queue.Remove(e);
	e->BDeleted = true;

	// finally, recalculate neighbor edge priorities 

	adjs.clear();
	// one ring edges of one ring vertices are afected
	for (i = 0; i < Vertexes[newVertexID]->OneRingVertex.size(); i++)
	{
		int vID = Vertexes[newVertexID]->OneRingVertex[i];
		adjs.insert(adjs.end(), Vertexes[vID]->OneRingEdge.begin(), Vertexes[vID]->OneRingEdge.end());
	}
	vtkstd::sort(adjs.begin(), adjs.end());
	// remove duplicities as it would be slower to compute their priority later
	adjs.erase(vtkstd::unique(adjs.begin(), adjs.end()), adjs.end());

	numPrimitives = adjs.size();
	bool update = true;
	double priority;
	for (adj = 0; adj < numPrimitives; adj++)
	{	
		priority = ComputePriorityFast(adjs[adj]);  // compute priority for all adjacent triangles
		if (priority >= 0) {
			if (!Queue.ModifyPriority(Edges[adjs[adj]], priority)) // try to modify priority (trying is O(1))
				Queue.Put(priority, Edges[adjs[adj]]); // put to queue, if not in
		}
		else {
			Queue.Remove(Edges[adjs[adj]]); // remove from queue
			Edges[adjs[adj]]->BMarked = false; // and forbid it's decimation
		}
	}
	adjs.clear();
}

//----------------------------------------------------------------------------
// Enlarges mesh using vertex translation in direction of vertex normal
// (computed as average of all vertex djacent triangles' normals)
void vtkProgressiveHullCPU::EnlargeMesh()
	//----------------------------------------------------------------------------
{
	if (EnlargeMeshAmount > 0)
	{
		// for all
		for (int i = 0; i < NumOfVertex; i++)
			// undeleted vertices
			if (!Vertexes[i]->BDeleted)
			{
				double normal[3];
				normal[0] = 0;
				normal[1] = 0;
				normal[2] = 0;

				// compute vertex normal
				for (int j = 1; j < (int)Vertexes[i]->OneRingTriangle.size(); j++)
				{
					if (!Triangles[Vertexes[i]->OneRingTriangle[j]]->BDeleted)
					{
						normal[0] += Triangles[Vertexes[i]->OneRingTriangle[j]]->DNormal[0];
						normal[1] += Triangles[Vertexes[i]->OneRingTriangle[j]]->DNormal[1];
						normal[2] += Triangles[Vertexes[i]->OneRingTriangle[j]]->DNormal[2];
					}
				}
				// translate vertex
				vtkMath::Normalize(normal);
				Vertexes[i]->DCoord[0] += normal[0] * EnlargeMeshAmount;
				Vertexes[i]->DCoord[1] += normal[1] * EnlargeMeshAmount;
				Vertexes[i]->DCoord[2] += normal[2] * EnlargeMeshAmount;
			}
	}
}

//----------------------------------------------------------------------------
// Extract data from internal structure and put them to output
// Hands over only nondeleted data.
void vtkProgressiveHullCPU::DoneMesh()
	//----------------------------------------------------------------------------
{
	int i;
	int* idMap = new int[NumOfVertex];
	int* newTriangleIDs = new int [3];

	// set up polydata object and data arrays
	vtkPoints *points = vtkPoints::New() ;
	vtkCellArray *triangles = vtkCellArray::New() ;

	// insert data into point array
	// InsertPoint() can allocate its own memory if not enough pre-allocated
	// points->Allocate(NumOfVertex) ;   
	int j = 0;
	for (i = 0 ;  i < NumOfVertex ;  i++)
	{
		if (!Vertexes[i]->BDeleted)  // skip deleted
		{
			idMap[i] = points->InsertNextPoint(Vertexes[i]->DCoord); // map the point's new ID
		}
	}													  

	// insert data into cell array
	triangles->Allocate(NumOfTriangle) ;   
	for (i = 0 ;  i < NumOfTriangle ;  i++)
		if (!Triangles[i]->BDeleted)  // skip deleted
		{
			// we use the map to resolve triangle vertice id's
			newTriangleIDs[0] = idMap[Triangles[i]->AVertex[0]];
			newTriangleIDs[1] = idMap[Triangles[i]->AVertex[1]];
			newTriangleIDs[2] = idMap[Triangles[i]->AVertex[2]];
			triangles->InsertNextCell(3, newTriangleIDs) ;
		}
		// hand over
		OutputMesh->SetPoints(points) ;
		OutputMesh->SetPolys(triangles) ;

		Log("Num of Triangles After Decimation: ", triangles->GetNumberOfCells());
		Log("Num of Vertices After Decimation: ", points->GetNumberOfPoints());
		// clear memory
		points->Delete();
		points = NULL;
		triangles->Delete();
		triangles = NULL;
		delete[] idMap;
		idMap = NULL;
		delete[] newTriangleIDs;
		newTriangleIDs = NULL;
}

//----------------------------------------------------------------------------
// Clear data from memory
void vtkProgressiveHullCPU::ClearMesh()
	//----------------------------------------------------------------------------
{
	int i;
	for(i=0;i<NumOfEdge;i++)
	{
		delete  Edges[i];
	}
	for(i=0;i<NumOfTriangle;i++)
	{
		delete  Triangles[i];
	}
	for(i=0;i<NumOfVertex;i++)
	{
		delete  Vertexes[i];
	}
	Vertexes.clear();
	Triangles.clear();
	Edges.clear();

	NumOfVertex = 0;
	NumOfTriangle = 0;
	NumOfEdge = 0;
#ifdef _USE_LP_SOLVE
	if(lp != NULL) {
		delete_lp(lp);
		lp = NULL; // prevent futher errors
	}
#endif
}

#pragma endregion Filter execution methods
#endif