/*=========================================================================
Program: Multimod Application Framework RELOADED
Module: $RCSfile: vtkMAFMuscleDecompositionMMSS.cpp,v $
Language: C++
Date: $Date: 2011-26-06 09:45:53 $
Version: $Revision: 1.1.2.1 $
Authors: Jan Rus (originally I. Zelený)
==========================================================================
Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details
=========================================================================
*/

#include "vtkMAFMuscleDecompositionMMSS.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"
#include "vtkMath.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "IMassSpringSystem.h"
#include "MassSpringSystemCPU.h"
#include "MassSpringSystemGPU.h"
#include "MMSSMorphing.h"

#include <math.h>
#include <float.h>
#include <algorithm>
using namespace std;

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

vtkCxxRevisionMacro(vtkMAFMuscleDecompositionMMSS, "$Revision: 1.1.2.1 $");
vtkStandardNewMacro(vtkMAFMuscleDecompositionMMSS);

#include "mafMemDbg.h"
#include "mafDbg.h"
#include "mafDefines.h"

int meshTrianglesCount;
MMSSMorphing::Triangle* meshTriangles;
int meshVerticesCount;
MMSSVector3d* meshVertices;
MMSSVector3d* meshNormals;

vtkMAFMuscleDecompositionMMSS::vtkMAFMuscleDecompositionMMSS()
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
}

vtkMAFMuscleDecompositionMMSS::~vtkMAFMuscleDecompositionMMSS()
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
/*virtual*/ void vtkMAFMuscleDecompositionMMSS::SetFibersTemplate(vtkMAFMuscleFibers* pTemplate)
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
/*virtual*/ void vtkMAFMuscleDecompositionMMSS::SetOriginArea(vtkPoints* pPoints)
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
/*virtual*/ void vtkMAFMuscleDecompositionMMSS::SetInsertionArea(vtkPoints* pPoints)
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
//Smooth the fiber defined by the given points.
void vtkMAFMuscleDecompositionMMSS::SmoothFiber(VCoord* pPoints, int nPoints)
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
	} //end for SmoothSteps
}

//------------------------------------------------------------------------
//By default, UpdateInformation calls this method to copy information
//unmodified from the input to the output.
/*virtual*/void vtkMAFMuscleDecompositionMMSS::ExecuteInformation()
	//------------------------------------------------------------------------
{
	//check input
	vtkPolyData* input = GetInput();
	if (input == NULL)
	{
		mafLogMessage("Invalid input for vtkMAFPolyDataDeformation.");
		return;   //we have no input
	}

	//check output
	vtkPolyData* output = GetOutput();
	if (output == NULL)
		SetOutput(vtkPolyData::New());

	if (this->FibersTemplate == NULL)
		this->FibersTemplate = vtkMAFParallelMuscleFibers::New();  //default is a pennate muscle

	//copy input to output
	Superclass::ExecuteInformation();
}

template <typename T>
T **AllocateDynamicArray( int nRows, int nCols)
{
	T **dynamicArray;

	dynamicArray = new T*[nRows];
	for( int i = 0 ; i < nRows ; i++ )
		dynamicArray[i] = new T [nCols];

	return dynamicArray;
}

template <typename T>
void FreeDynamicArray(T** dArray)
{
	delete [] *dArray;
	delete [] dArray;
}

//---------------------------------------------------------------------------------
//vytvorime tabulku hran a trojuhelniku
//Table of edges and triangles
//N = triangle_count * 3 = number of oriented edges
//column 0 = firs index of each edge
//column 1 = second index of each edge
//table_index/3 = triangle index
GLuint **createEdgeTriTable(MMSSMorphing::Triangle *meshTriangles, int triangleCount)
{
	GLuint **edgeTriTable = AllocateDynamicArray<GLuint>(triangleCount * 3, 2);

	for(int i = 0; i < triangleCount * 3; i++)
	{
		edgeTriTable[i][0] = -1;
		edgeTriTable[i][1] = -1;
	}

	for(int i = 0; i < triangleCount; i++)
	{
		//edge 1
		edgeTriTable[3 * i + 0][0] = meshTriangles[i].p1_index;
		edgeTriTable[3 * i + 0][1] = meshTriangles[i].p2_index;

		//edge 2
		edgeTriTable[3 * i + 1][0] = meshTriangles[i].p2_index;
		edgeTriTable[3 * i + 1][1] = meshTriangles[i].p3_index;

		//edge 3
		edgeTriTable[3 * i + 2][0] = meshTriangles[i].p3_index;
		edgeTriTable[3 * i + 2][1] = meshTriangles[i].p1_index;
	}

	return edgeTriTable;
}

//----------------------------------------------------------------------------------------------
//returns true, is point P is inside triagle V1, V2, V3
bool isPointInsideTriangle(MMSSVector3d P, MMSSVector3d V1, MMSSVector3d V2, MMSSVector3d V3)
{
	// Compute vectors
	MMSSVector3d v0 = V3 - V1;
	MMSSVector3d v1 = V2 - V1;
	MMSSVector3d v2 = P - V1;

	// Compute dot products
	GLfloat dot00 = (v0 * v0);
	GLfloat dot01 = (v0 * v1);
	GLfloat dot02 = (v0 * v2);
	GLfloat dot11 = (v1 * v1);
	GLfloat dot12 = (v1 * v2);

	// Compute barycentric coordinates
	GLfloat invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	GLfloat u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	GLfloat v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	return ((u >= 0) && (v >= 0) && (u + v <= 1));
}

//----------------------------------------------------------------------------------------------
//Intersection of the given plane and segment
//points of segment and 3 points of plane
//intersection point p = A + t * (B-A)
GLfloat segmentPlaneIntersectionParameter(MMSSVector3d A, MMSSVector3d B, MMSSVector3d planeP0,
	MMSSVector3d planeP1, MMSSVector3d planeP2)
{
	MMSSVector3d l = (B-A); //segment direction

	//normal
	MMSSVector3d N = MMSSVector3d::VectorCross(planeP1-planeP0, planeP2-planeP0);
	N = MMSSVector3d::Normalize(N);

	//parameter "t" of the ray to the intersection
	return ((planeP0 - A) * N) / (l * N);
}

//-----------------------------------------------------------------------------------------------
//Calculates 2 intersection of the given plane and edges of the given triangle
//needs trinagle index and 3 plane vertices p0, p1, p2
//return 2 intersection points
//startEdge = -1 means unknown first intersected edge
Vector4D* get2TrianglePlaneIntersections(GLuint triangleIndex, GLuint **edgeTriTable,
	MMSSVector3d P0, MMSSVector3d P1, MMSSVector3d P2, int startEdge, bool reverse)
{
	//triangle vertex indices
	int aI = meshTriangles[triangleIndex].p1_index;
	int bI = meshTriangles[triangleIndex].p2_index;
	int cI = meshTriangles[triangleIndex].p3_index;

	//triangle vertices
	MMSSVector3d a = meshVertices[aI];
	MMSSVector3d b = meshVertices[bI];
	MMSSVector3d c = meshVertices[cI];

	//calculate, which edge is intersected first/second AB=1, BC=2, CA=3, unknown=0
	int inEdge = 0;
	vector<int> outEdges;
	outEdges.clear();
	if(startEdge >= 0)
	{
		if((edgeTriTable[startEdge][0] == aI && edgeTriTable[startEdge][1] == bI)
			|| (edgeTriTable[startEdge][0] == bI && edgeTriTable[startEdge][1] == aI))
		{ inEdge = 1; }
		else if((edgeTriTable[startEdge][0] == bI && edgeTriTable[startEdge][1] == cI)
			|| (edgeTriTable[startEdge][0] == cI && edgeTriTable[startEdge][1] == bI))
		{ inEdge = 2; }
		else if((edgeTriTable[startEdge][0] == cI && edgeTriTable[startEdge][1] == aI)
			|| (edgeTriTable[startEdge][0] == aI && edgeTriTable[startEdge][1] == cI))
		{ inEdge = 3; }
	}

	//sign test for each triangle vertex (above or under the plane)
	MMSSVector3d nPlane = MMSSVector3d::Normalize(MMSSVector3d::VectorCross(P1-P0, P2-P0));//cutting plane normal
	MMSSVector3d nTri = MMSSVector3d::Normalize(MMSSVector3d::VectorCross(b-a, c-a));//triangle normal
	float sigA = MMSSVector3d::Normalize(a-P0) * nPlane;
	float sigB = MMSSVector3d::Normalize(b-P0) * nPlane;
	float sigC = MMSSVector3d::Normalize(c-P0) * nPlane;

#pragma region Orientation test
	//at least one is in the plane
	//move them to different position
	if(sigA * sigB * sigC == 0)
	{
		sigA += numeric_limits<float>::epsilon( );
		sigB += numeric_limits<float>::epsilon( );
		sigC += numeric_limits<float>::epsilon( );
	}

	//all vertices on one side from the plane
	//get the vertex closest to the plane and change its sign
	if(sigA < 0 && sigB < 0 && sigC < 0) //under
	{
		if(sigA > sigB && sigA > sigC){ sigA *= -1; }
		else if(sigB > sigA && sigB > sigC){ sigB *= -1; }
		else if(sigC > sigA && sigC > sigB){ sigC *= -1; }
	}
	else if(sigA > 0 && sigB > 0 && sigC > 0) //above
	{
		if(sigA < sigB && sigA < sigC){ sigA *= -1; }
		else if(sigB < sigA && sigB < sigC){ sigB *= -1; }
		else if(sigC < sigA && sigC < sigB){ sigC *= -1; }
	}

#pragma endregion

	//one vertex above and two under (and vice versa)
	if(sigA*sigB < 0) //edge AB cut
	{
		if(sigB*sigC < 0)//edge AB and BC cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 1 || inEdge == 2)
			{
				if(inEdge == 1) outEdges.push_back(2);
				else outEdges.push_back(1);
			}
			//if none is "in-edge"
			else //(inEdge== 3/0)
			{
				outEdges.push_back(1);
				outEdges.push_back(2);
			}
		}
		else //edge AB and AC cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 1 || inEdge == 3)
			{
				if(inEdge == 1) outEdges.push_back(3);
				else outEdges.push_back(1);
			}
			//if none is "in-edge"
			else //(inEdge== 2/0)
			{
				outEdges.push_back(1);
				outEdges.push_back(3);
			}
		}
	}
	else if(sigB*sigC < 0) //edge BC cut
	{
		if(sigA*sigB < 0)//edge BC and AB cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 2 || inEdge == 1)
			{
				if(inEdge == 2) outEdges.push_back(1);
				else outEdges.push_back(2);
			}
			//if none is "in-edge"
			else //(inEdge== 3/0)
			{
				outEdges.push_back(2);
				outEdges.push_back(1);
			}
		}
		else //edge BC and CA cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 2 || inEdge == 3)
			{
				if(inEdge == 2) outEdges.push_back(3);
				else outEdges.push_back(2);
			}
			//if none is "in-edge"
			else //(inEdge== 1/0)
			{
				outEdges.push_back(2);
				outEdges.push_back(3);
			}
		}
	}
	else if(sigC*sigA < 0) //edge CA cut
	{
		if(sigA*sigB < 0)//edge CA and AB cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 3 || inEdge == 1)
			{
				if(inEdge == 3) outEdges.push_back(1);
				else outEdges.push_back(3);
			}
			//if none is "in-edge"
			else //(inEdge== 2/0)
			{
				outEdges.push_back(3);
				outEdges.push_back(1);
			}
		}
		else //edge CA and BC cut
		{
			//if one edge is "in-edge", other edge is "out-edge"
			if(inEdge == 3 || inEdge == 2)
			{
				if(inEdge == 3) outEdges.push_back(2);
				else outEdges.push_back(3);
			}
			//if none is "in-edge"
			else //(inEdge== 1/0)
			{
				outEdges.push_back(3);
				outEdges.push_back(2);
			}
		}
	}
	else
	{
		bool err = true;
	}

	MMSSVector3d* intersections = (MMSSVector3d*)malloc(2 * sizeof(MMSSVector3d));
	int indexOut = -1;
	int indexIn = -1;

	//the "out-edge" is known
	if(outEdges.size() == 1)
	{
		//calculate intersection
		if(outEdges[0] == 1)
		{
			GLfloat t = segmentPlaneIntersectionParameter(a, b, P0, P1, P2);
			intersections[0] = a + t * (b-a);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == aI && edgeTriTable[i][1] == bI)
					|| (edgeTriTable[i][0] == bI && edgeTriTable[i][1] == aI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[0] == 2)
		{
			GLfloat t = segmentPlaneIntersectionParameter(b, c, P0, P1, P2);
			intersections[0] = b + t * (c-b);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == bI && edgeTriTable[i][1] == cI)
					|| (edgeTriTable[i][0] == cI && edgeTriTable[i][1] == bI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[0] == 3)
		{
			GLfloat t = segmentPlaneIntersectionParameter(c, a, P0, P1, P2);
			intersections[0] = c + t * (a-c);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == cI && edgeTriTable[i][1] == aI)
					|| (edgeTriTable[i][0] == aI && edgeTriTable[i][1] == cI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}

		//the same for "in-edge"
		if(inEdge == 1)
		{
			GLfloat t = segmentPlaneIntersectionParameter(a, b, P0, P1, P2);
			intersections[1] = a + t * (b-a);
		}
		else if(inEdge == 2)
		{
			GLfloat t = segmentPlaneIntersectionParameter(b, c, P0, P1, P2);
			intersections[1] = b + t * (c-b);
		}
		else if(inEdge == 3)
		{
			GLfloat t = segmentPlaneIntersectionParameter(c, a, P0, P1, P2);
			intersections[1] = c + t * (a-c);
		}

		indexIn = startEdge;
	}
	//foose from in- and out- edges
	else if(outEdges.size() == 2)
	{
		//get both intersections
		if(outEdges[0] == 1)
		{
			GLfloat t = segmentPlaneIntersectionParameter(a, b, P0, P1, P2);
			intersections[0] = a + t * (b-a);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == aI && edgeTriTable[i][1] == bI)
					|| (edgeTriTable[i][0] == bI && edgeTriTable[i][1] == aI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[0] == 2)
		{
			GLfloat t = segmentPlaneIntersectionParameter(b, c, P0, P1, P2);
			intersections[0] = b + t * (c-b);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == bI && edgeTriTable[i][1] == cI)
					|| (edgeTriTable[i][0] == cI && edgeTriTable[i][1] == bI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[0] == 3)
		{
			GLfloat t = segmentPlaneIntersectionParameter(c, a, P0, P1, P2);
			intersections[0] = c + t * (a-c);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == cI && edgeTriTable[i][1] == aI)
					|| (edgeTriTable[i][0] == aI && edgeTriTable[i][1] == cI))
				{
					if(i/3 != triangleIndex)
					{
						indexOut = (GLfloat)i;
						break;
					}
				}
			}
		}

		//calculate intersection
		if(outEdges[1] == 1)
		{
			GLfloat t = segmentPlaneIntersectionParameter(a, b, P0, P1, P2);
			intersections[1] = a + t * (b-a);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == aI && edgeTriTable[i][1] == bI)
					|| (edgeTriTable[i][0] == bI && edgeTriTable[i][1] == aI))
				{
					if(i/3 != triangleIndex)
					{
						indexIn = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[1] == 2)
		{
			GLfloat t = segmentPlaneIntersectionParameter(b, c, P0, P1, P2);
			intersections[1] = b + t * (c-b);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == bI && edgeTriTable[i][1] == cI)
					|| (edgeTriTable[i][0] == cI && edgeTriTable[i][1] == bI))
				{
					if(i/3 != triangleIndex)
					{
						indexIn = (GLfloat)i;
						break;
					}
				}
			}
		}
		else if(outEdges[1] == 3)
		{
			GLfloat t = segmentPlaneIntersectionParameter(c, a, P0, P1, P2);
			intersections[1] = c + t * (a-c);

			//calculate index of the edge of the edge-incident triangle
			for(int i = 0; i < meshTrianglesCount * 3; i++)
			{
				if((edgeTriTable[i][0] == cI && edgeTriTable[i][1] == aI)
					|| (edgeTriTable[i][0] == aI && edgeTriTable[i][1] == cI))
				{
					if(i/3 != triangleIndex)
					{
						indexIn = (GLfloat)i;
						break;
					}
				}
			}
		}

		//get the order of intersections
		MMSSVector3d dir = MMSSVector3d::Normalize(MMSSVector3d::VectorCross(nPlane, nTri));//direction of the cut

		//zjistime uhel mezi smerem rezu a vektorem spojujicim pruseciky
		//angle between the cut direction and the vector connecting both intersections
		float angle = (intersections[0] - intersections[1]) * dir;
		if(angle <= 0)
		{
			MMSSVector3d tmp = intersections[0];
			intersections[0] = intersections[1];
			intersections[1] = tmp;

			int tmpI = indexOut;
			indexOut = indexIn;
			indexIn = tmpI;
		}
	}
	else
	{
		bool err = true;
	}

	//result = 2 intersections
	//intersection = x,y,z,edge_index
	Vector4D* result = (Vector4D*)malloc(3 * sizeof(Vector4D));
	Vector4D intersection1 = {intersections[1].x, intersections[1].y, intersections[1].z, indexIn};
	Vector4D intersection2 = {intersections[0].x, intersections[0].y, intersections[0].z, indexOut};

	if(startEdge >= 0)
	{
		result[0] = intersection1;
		result[1] = intersection2;
	}
	else
	{
		if(reverse)
		{
			result[0] = intersection2;
			result[1] = intersection1;
		}
		else
		{
			result[0] = intersection1;
			result[1] = intersection2;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
//closest point of segment AB to point X
MMSSVector3d closestPointOfSegment(MMSSVector3d X, MMSSVector3d A, MMSSVector3d B)
{
	//distances do triangle edges
	MMSSVector3d u = B - A;

	float t = (u * (X - A)) / (u * u);

	if(t >=1)
	{
		return B;
	}
	else if(t <= 0)
	{
		return A;
	}
	else
	{
		return A + t * u;
	}
}

//-----------------------------------------------------------------------------------------
//Projects given set of vertices on the surface of the given mesh -finds the closest point
//on the mesh surface.
vector<Vector4D> projectPointsOntoSurface(MMSSVector3d* projectedPoints, unsigned int pointCount, MMSSMorphing::Triangle* meshTriangles,
	unsigned int triangleCount, MMSSVector3d* meshVertices, int* clustering, int clusterIndex)
{
	//x,y,z = position of the vertex after projection
	//w = index of the triangle with the projected vertex
	vector<Vector4D> result;
	result.clear();

	int vertIndex = 0;
	int triangleIndex = -1;

	MMSSVector3d intersectionPoint = {0, 0, 0};
	//cycle thru all vertices
	for(unsigned int i = 0; i < pointCount; i++)
	{
		GLfloat minT = FLT_MAX;
		triangleIndex = -1;

		for(unsigned int j = 0; j < triangleCount; j++)
		{
			if(clustering != NULL)
			{
				if(clustering[meshTriangles[j].p1_index] != clusterIndex
					&& clustering[meshTriangles[j].p2_index] != clusterIndex
					&& clustering[meshTriangles[j].p3_index] != clusterIndex)
				{
					continue;
				}
			}

			//triangle vertices
			MMSSVector3d p0 = meshVertices[meshTriangles[j].p1_index];
			MMSSVector3d p1 = meshVertices[meshTriangles[j].p2_index];
			MMSSVector3d p2 = meshVertices[meshTriangles[j].p3_index];

			/*float dist1 = MMSSVector3d::VectorLength(p0 - projectedPoints[i]);
			float dist2 = MMSSVector3d::VectorLength(p1 - projectedPoints[i]);
			float dist3 = MMSSVector3d::VectorLength(p2 - projectedPoints[i]);

			//if triangle is too far
			if(dist1 > minT && dist2 > minT && dist3 > minT)
			{
				continue;
			}*/

			//normal
			MMSSVector3d N = meshNormals[j];

			//parameter "t" of the ray to the intersection
			//GLfloat t = ((p0 - s) * N) / (N * N); -normalized
			GLfloat t = ((p0 - projectedPoints[i]) * N);

			//intersection with the triangle plane
			//is 'p' inside the triangle?
			float triDistance = FLT_MAX;
			MMSSVector3d p = projectedPoints[i] + t * N;
			if(isPointInsideTriangle(p, p0, p1, p2))
			{
				triDistance = abs(t);
			}

			//closest point of triangle edges
			MMSSVector3d x1 = closestPointOfSegment(projectedPoints[i], p0, p1);
			float x1Distance = MMSSVector3d::VectorLength(projectedPoints[i]-x1);
			MMSSVector3d x2 = closestPointOfSegment(projectedPoints[i], p1, p2);
			float x2Distance = MMSSVector3d::VectorLength(projectedPoints[i]-x2);
			MMSSVector3d x3 = closestPointOfSegment(projectedPoints[i], p2, p0);
			float x3Distance = MMSSVector3d::VectorLength(projectedPoints[i]-x3);

			//we have distances tri, x1, x2, x3 -which one is the shortest?
			if(x1Distance <= x2Distance && x1Distance <= x3Distance && x1Distance <= triDistance)
			{
				triDistance = x1Distance;
				p = x1;
			}
			else if(x2Distance <= x1Distance && x2Distance <= x3Distance && x2Distance < triDistance)
			{
				triDistance = x2Distance;
				p = x2;
			}
			else if(x3Distance <= x1Distance && x3Distance <= x2Distance && x3Distance < triDistance)
			{
				triDistance = x3Distance;
				p = x3;
			}
			else if(triDistance <= x1Distance && triDistance <= x2Distance && triDistance < x3Distance)
			{
				triDistance = triDistance;
				p = p;
			}
			else
			{
				//not possible
			}

			if(abs(triDistance) < minT)
			{
				//point is inside the triangle!!!
				intersectionPoint = p;
				minT = abs(triDistance);
				triangleIndex = j;
			}
		}

		//test whether projection is close to the triangle we tip
		if(MMSSVector3d::VectorLength(intersectionPoint - meshVertices[meshTriangles[triangleIndex].p1_index]) < 0.0001)
		{
			//(centroid - corner) * 0.9
			MMSSVector3d centroid = (1.0f / 3) * (meshVertices[meshTriangles[triangleIndex].p1_index] + meshVertices[meshTriangles[triangleIndex].p2_index] + meshVertices[meshTriangles[triangleIndex].p3_index]);
			MMSSVector3d u = meshVertices[meshTriangles[triangleIndex].p1_index] - centroid;

			intersectionPoint = centroid + 0.9f * u;
		}

		if(MMSSVector3d::VectorLength(intersectionPoint - meshVertices[meshTriangles[triangleIndex].p2_index]) < 0.0001)
		{
			//(centroid - corner) * 0.9
			MMSSVector3d centroid = (1.0f / 3) * (meshVertices[meshTriangles[triangleIndex].p1_index] + meshVertices[meshTriangles[triangleIndex].p2_index] + meshVertices[meshTriangles[triangleIndex].p3_index]);
			MMSSVector3d u = meshVertices[meshTriangles[triangleIndex].p2_index] - centroid;

			intersectionPoint = centroid + 0.9f * u;
		}

		if(MMSSVector3d::VectorLength(intersectionPoint - meshVertices[meshTriangles[triangleIndex].p3_index]) < 0.0001)
		{
			//(centroid - corner) * 0.9
			MMSSVector3d centroid = (1.0f / 3) * (meshVertices[meshTriangles[triangleIndex].p1_index] + meshVertices[meshTriangles[triangleIndex].p2_index] + meshVertices[meshTriangles[triangleIndex].p3_index]);
			MMSSVector3d u = meshVertices[meshTriangles[triangleIndex].p3_index] - centroid;

			intersectionPoint = centroid + 0.9f * u;
		}

		Vector4D point = {intersectionPoint.x, intersectionPoint.y, intersectionPoint.z, (float)triangleIndex};
		result.push_back(point);

		vertIndex++;
	}

	return result;
}

//----------------------------------------------------------------------------------------
//Projects given set of vertices on the surface of the given mesh using -finds intersection
//of the ray from the centroid thru the given point
vector<Vector4D> projectPointsOntoSurface2(MMSSVector3d* projectedPoints, MMSSVector3d centroid, unsigned int pointCount,
	MMSSMorphing::Triangle* meshTriangles, unsigned int triangleCount,	MMSSVector3d* meshVertices)
{
	//x,y,z = position of the vertex after projection
	//w = index of the triangle with the projected vertex
	vector<Vector4D> result;
	vector<Vector4D> minPos;
	result.clear();

	vector<float> params;
	params.clear();

	int triangleIndexMin = -1;
	int triangleIndexMinPos = -1;
	MMSSVector3d intersectionPointMin = {0, 0, 0};
	int triangleIndexMax = -1;
	MMSSVector3d intersectionPointMax = {0, 0, 0};
	MMSSVector3d intersectionPointMinPos = {0, 0, 0};

	int positiveIntersections = 0;

	bool isInside = true;

	//cycle thru all vertices
	for(unsigned int i = 0; i < pointCount; i++)
	{
		GLfloat minT = FLT_MAX;
		GLfloat minTPos = FLT_MAX;
		GLfloat maxT = FLT_MIN;
		triangleIndexMin = -1;
		triangleIndexMinPos = -1;
		triangleIndexMax = -1;
		positiveIntersections = 0;
		//for each triangle
		for(unsigned int j = 0; j < triangleCount; j++)
		{
			//vertices of the tested triangle
			MMSSVector3d p0 = meshVertices[meshTriangles[j].p1_index];
			MMSSVector3d p1 = meshVertices[meshTriangles[j].p2_index];
			MMSSVector3d p2 = meshVertices[meshTriangles[j].p3_index];

			float dist1 = MMSSVector3d::VectorLength(p0 - projectedPoints[i]);
			float dist2 = MMSSVector3d::VectorLength(p1 - projectedPoints[i]);
			float dist3 = MMSSVector3d::VectorLength(p2 - projectedPoints[i]);

			//if triangle is too far
			if(dist1 > minT && dist2 > minT && dist3 > minT
				&& dist1 < maxT && dist2 < maxT && dist3 < maxT)
			{
				continue;
			}

			//normal
			MMSSVector3d N = meshNormals[j];

			//parameter "t" of the ray to the intersection
			//GLfloat t = ((p0 - s) * N) / (N * N); -normalized
			//GLfloat t = ((p0 - s) * N);
			GLfloat t = (N * (p0 - centroid)) / (N * (projectedPoints[i] - centroid));

			//intersection with the triangle plane
			//is 'p' inside the triangle?
			float triDistance = 0;
			//Vector p = s + t * N;
			MMSSVector3d p = centroid + t * (projectedPoints[i] - centroid);
			if(isPointInsideTriangle(p, p0, p1, p2))
			{
				if(t > 0)
				{
					positiveIntersections++;
				}

				if(t < minTPos && t > 0)
				{
					intersectionPointMinPos = p;
					minTPos = t;
					triangleIndexMinPos = j;
				}

				if(t < minT)
				{
					//point is inside the triangle!!!
					intersectionPointMin = p;
					minT = t;
					triangleIndexMin = j;
				}

				if(t > maxT)
				{
					//point is inside the triangle!!!
					intersectionPointMax = p;
					maxT = t;
					triangleIndexMax = j;
				}
			}
		}

		if(positiveIntersections % 2 != 1)
		{
			isInside = false;
		}

		if(triangleIndexMin != -1 && triangleIndexMax != -1)
		{
			Vector4D pointMin = {intersectionPointMin.x, intersectionPointMin.y, intersectionPointMin.z, (float)triangleIndexMin};
			Vector4D pointMax = {intersectionPointMax.x, intersectionPointMax.y, intersectionPointMax.z, (float)triangleIndexMax};
			result.push_back(pointMin);
			result.push_back(pointMax);

			params.push_back(minT);
			params.push_back(maxT);

			Vector4D pointMinPos = {intersectionPointMinPos.x, intersectionPointMinPos.y, intersectionPointMinPos.z, (float)triangleIndexMinPos};
			minPos.push_back(pointMinPos);
		}
	}

	if(isInside)
	{
		return minPos;
	}

	vector<Vector4D> res;
	res.clear();

	if( params.size() > 0)
		for(int i = 0; i <params.size()-1; i+=2)
		{
			if(params[i] > params[i+1]){res.push_back(result[i]);}
			else{res.push_back(result[i+1]);}
		}

		return res;
}

//-----------------------------------------------------------------
// Calculates points on the surface of the mesh which form the shape of the cut defined by the
// list of controlVertices
vector<MMSSVector3d> cutMuscleTip(GLuint *trianglesWithControlVertices, MMSSVector3d *controlVertices, int numOfControlVertices, GLuint **edgeTriTable)
{
	//cycle thru all triangles and remove duplicities
	vector<int> indices;
	indices.clear();

	for(int i = 0; i < numOfControlVertices-1; i++)
	{
		if(trianglesWithControlVertices[i] != trianglesWithControlVertices[i + 1])
		{
			indices.push_back(i);
		}
	}

	if(trianglesWithControlVertices[numOfControlVertices-1] != trianglesWithControlVertices[0])
	{
		indices.push_back(numOfControlVertices-1);
	}

	numOfControlVertices = indices.size();

	GLuint *trianglesWithControlVerticesTmp = (GLuint*)malloc((numOfControlVertices) * sizeof(GLuint));
	MMSSVector3d *controlVerticesTmp = (MMSSVector3d*)malloc((numOfControlVertices) * sizeof(MMSSVector3d));

	for(int i = 0; i < numOfControlVertices; i++)
	{
		trianglesWithControlVerticesTmp[i] = trianglesWithControlVertices[indices[i]];
		controlVerticesTmp[i] = controlVertices[indices[i]];
	}

	trianglesWithControlVertices = (GLuint*)malloc((numOfControlVertices) * sizeof(GLuint));
	controlVertices = (MMSSVector3d*)malloc((numOfControlVertices) * sizeof(MMSSVector3d));

	for(int i = 0; i < numOfControlVertices; i++)
	{
		trianglesWithControlVertices[i] = trianglesWithControlVerticesTmp[i];
		controlVertices[i] = controlVerticesTmp[i];
	}

	//centroid of the cut
	MMSSVector3d planeCentroid = {0, 0, 0};
	for(int i = 0; i < numOfControlVertices; i++)
	{
		planeCentroid = planeCentroid + controlVertices[i];
	}
	planeCentroid = (1.0 / numOfControlVertices) * planeCentroid;

	//points of the cut
	vector<MMSSVector3d> cutEdge;
	vector<int> cutTri;
	vector<vector<MMSSVector3d>> edges1;
	vector<vector<MMSSVector3d>> edges2;

	cutEdge.clear();
	int lastIntersectedEdge = -1;

	int triIndex = trianglesWithControlVertices[0];
	int actualCutIndex = 0;
	int increment = 1;

	MMSSVector3d standardCentroid = planeCentroid;

	MMSSVector3d a, b, c, d, e, f; //three vertices of two triangles (abc, def) and their normals(m, n)

	bool cutFailed = false; //if the cut searching fails

	while(true)
	{
		//plane vertices
		MMSSVector3d p0 = planeCentroid;
		MMSSVector3d p1 = controlVertices[actualCutIndex];
		MMSSVector3d p2;

		a = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p1_index];
		b = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p2_index];
		c = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p3_index];

		if(actualCutIndex + increment < numOfControlVertices-1)
		{
			p2 = controlVertices[actualCutIndex + increment];

			d = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p1_index];
			e = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p2_index];
			f = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p3_index];
		}
		else
		{
			p2 = controlVertices[(actualCutIndex + increment) % numOfControlVertices];

			d = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p1_index];
			e = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p2_index];
			f = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p3_index];
		}

		MMSSVector3d n = 0.5f * (MMSSVector3d::Normalize(MMSSVector3d::VectorCross(b-a, c-a)) + MMSSVector3d::Normalize(MMSSVector3d::VectorCross(e-d, f-d)));//avg. normala start a cil trojuhelniku

		planeCentroid = controlVertices[actualCutIndex] + n;
		p0 = planeCentroid;

		//get 2 intersections with the actual triangle
		//intersection = x,y,z,index_of_the_edge_in_the_table
		Vector4D* intersections;
		intersections = get2TrianglePlaneIntersections(triIndex, edgeTriTable,
			p0, p1, p2, lastIntersectedEdge, true);

		//two intersections but we need only one of them
		MMSSVector3d intersection1 = {intersections[1].x, intersections[1].y, intersections[1].z};
		int intersection1EdgeIndex = (int)intersections[1].w;

		cutEdge.push_back(intersection1);

		//get the index of the next triangle with edge common with the actual triangle
		triIndex = intersection1EdgeIndex / 3;
		lastIntersectedEdge = intersection1EdgeIndex;

		//if we reached the start triangle
		//cut finished!!!
		if(triIndex == trianglesWithControlVertices[0] && actualCutIndex >= numOfControlVertices - 1)
		{
			edges1.push_back(cutEdge);
			cutEdge.clear();
			increment = 1;
			cutFailed = false;
			break;
		}

		//if it is not possible to reach the target vertex to finish the cut
		//eg. two hills with two vertices on the top of each of the hills
		else if(triIndex == trianglesWithControlVertices[actualCutIndex] && cutEdge.size() > 1)
		{
			if(cutFailed == false)
			{
				cutEdge.clear();
				//increment++;
				MMSSVector3d a = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p1_index];
				MMSSVector3d b = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p2_index];
				MMSSVector3d c = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p3_index];
				planeCentroid = controlVertices[actualCutIndex] + MMSSVector3d::VectorCross(b-a, c-a);//normala rezaneho trojuhelniku
				lastIntersectedEdge = -1;
				cutFailed = true;
				continue;
			}
			else
			{
				cutEdge.clear();
				edges1.push_back(cutEdge);
				cutFailed = false;

				actualCutIndex++;
				lastIntersectedEdge = -1;
				increment = 1;
				planeCentroid = standardCentroid;
			}
		}

		//if(cutEdge.size() > edges1[actualCutIndex].size())
		if(false)
		{
			/*edges2.push_back(cutEdge);
			cutEdge.clear();
			
			actualCutIndex++;
			lastIntersectedEdge = -1;
			triIndex == trianglesWithControlVertices[actualCutIndex];

			if(actualCutIndex >= numOfControlVertices) break;*/
		}
		else
		{
			//if the actual triangle contains the start vertex, end of the cut
			//increment index of the cutting plane
			//"while" to skip all control vertices lying in one triangle
			while(actualCutIndex + 1 < numOfControlVertices && triIndex == trianglesWithControlVertices[actualCutIndex + 1])
			{
				edges1.push_back(cutEdge);
				cutEdge.clear();
				cutFailed = false;

				actualCutIndex++;
				lastIntersectedEdge = -1;
				increment = 1;
				planeCentroid = standardCentroid;
			}
		}
	}

	// start cutting with inverted direction

	cutEdge.clear();
	lastIntersectedEdge = -1;

	triIndex = trianglesWithControlVertices[0];
	actualCutIndex = 0;
	increment = 1;

	standardCentroid = planeCentroid;
	cutFailed = false;

	while(true)
	{
		//plane vertices
		MMSSVector3d p0 = planeCentroid;
		MMSSVector3d p1 = controlVertices[actualCutIndex];
		MMSSVector3d p2;

		a = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p1_index];
		b = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p2_index];
		c = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p3_index];

		if(actualCutIndex + increment < numOfControlVertices-1)
		{
			p2 = controlVertices[actualCutIndex + increment];

			d = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p1_index];
			e = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p2_index];
			f = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex + increment]].p3_index];
		}
		else
		{
			p2 = controlVertices[(actualCutIndex + increment) % numOfControlVertices];

			d = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p1_index];
			e = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p2_index];
			f = meshVertices[meshTriangles[trianglesWithControlVertices[(actualCutIndex + increment) % numOfControlVertices]].p3_index];
		}

		MMSSVector3d n = 0.5f * (MMSSVector3d::Normalize(MMSSVector3d::VectorCross(b-a, c-a)) + MMSSVector3d::Normalize(MMSSVector3d::VectorCross(e-d, f-d)));//avg. normala start a cil trojuhelniku

		planeCentroid = controlVertices[actualCutIndex] + n;
		p0 = planeCentroid;

		//get 2 intersections with the actual triangle
		//intersection = x,y,z,index_of_the_edge_in_the_table
		Vector4D* intersections;
		intersections = get2TrianglePlaneIntersections(triIndex, edgeTriTable,
			p0, p1, p2, lastIntersectedEdge, false);

		//two intersections but we need only one of them
		MMSSVector3d intersection1 = {intersections[1].x, intersections[1].y, intersections[1].z};
		int intersection1EdgeIndex = (int)intersections[1].w;

		cutEdge.push_back(intersection1);

		//get the index of the next triangle with edge common with the actual triangle
		triIndex = intersection1EdgeIndex / 3;
		lastIntersectedEdge = intersection1EdgeIndex;

		//if we reached the start triangle
		//cut finished!!!
		if(triIndex == trianglesWithControlVertices[0] && actualCutIndex >= numOfControlVertices - 1)
		{
			edges2.push_back(cutEdge);
			cutEdge.clear();
			increment = 1;
			cutFailed = false;
			break;
		}

		//if it is not possible to reach the target vertex to finish the cut
		//eg. two hills with two vertices on the top of each of the hills
		else if(triIndex == trianglesWithControlVertices[actualCutIndex] && cutEdge.size() > 1)
		{
			if(cutFailed == false)
			{
				cutEdge.clear();
				//increment++;
				MMSSVector3d a = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p1_index];
				MMSSVector3d b = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p2_index];
				MMSSVector3d c = meshVertices[meshTriangles[trianglesWithControlVertices[actualCutIndex]].p3_index];
				planeCentroid = controlVertices[actualCutIndex] + MMSSVector3d::VectorCross(b-a, c-a);//normala rezaneho trojuhelniku
				lastIntersectedEdge = -1;
				cutFailed = true;
				continue;
			}
			else
			{
				cutEdge.clear();
				edges2.push_back(cutEdge);
				cutFailed = false;

				actualCutIndex++;
				lastIntersectedEdge = -1;
				increment = 1;
				planeCentroid = standardCentroid;
			}
		}

		if(cutEdge.size() > edges1[actualCutIndex].size())
		//if(false)
		{
			triIndex = trianglesWithControlVertices[(actualCutIndex + 1)%numOfControlVertices];

			if(actualCutIndex >= numOfControlVertices-1)
			{
				edges2.push_back(cutEdge);
				cutEdge.clear();

				break;
			}

			while(actualCutIndex + 1 < numOfControlVertices && triIndex == trianglesWithControlVertices[actualCutIndex + 1])
			{
				edges2.push_back(cutEdge);
				cutEdge.clear();

				actualCutIndex++;
				lastIntersectedEdge = -1;
				increment = 1;
				planeCentroid = standardCentroid;
				cutFailed = false;
			}
		}
		else
		{
			//if the actual triangle contains the start vertex, end of the cut
			//increment index of the cutting plane
			//"while" to skip all control vertices lying in one triangle
			while(actualCutIndex + 1 < numOfControlVertices && triIndex == trianglesWithControlVertices[actualCutIndex + 1])
			{
				edges2.push_back(cutEdge);
				cutEdge.clear();

				actualCutIndex++;
				lastIntersectedEdge = -1;
				increment = 1;
				planeCentroid = standardCentroid;
				cutFailed = false;
			}
		}
	}

	//form the result for the output
	vector<MMSSVector3d> result;
	for(int i = 0; i< numOfControlVertices; i++)
	{
		if(edges1[i].size() < edges2[i].size() && edges1[i].size() != 0)
		{
			result.insert(result.end(), edges1[i].begin(), edges1[i].end());
		}
		else if(edges1[i].size() >= edges2[i].size() && edges2[i].size() != 0)
		{
			result.insert(result.end(), edges2[i].begin(), edges2[i].end());
		}
		else
		{
			result.insert(result.end(), edges1[i].begin(), edges1[i].end());
		}
	}

	edges1.clear();
	edges2.clear();

	return result;
}

//--------------------------------------------------------------------------------------------------
// Calculates points on the surface of the mesh which form the shape of the cut defined by the
// list of controlVertices
MMSSVector3d* cutMuscleTip(GLuint *trianglesWithControlVertices, MMSSVector3d *controlVertices,
	int numOfControlVertices, GLuint **edgeTriTable, MMSSVector3d startDirection,
	int numOfPoints)
{
	//list of points forming the cut shape
	vector<MMSSVector3d> cutEdge = cutMuscleTip(trianglesWithControlVertices, controlVertices, numOfControlVertices, edgeTriTable);

	//calculate centroid and length of the cut
	MMSSVector3d centroid = {0, 0, 0};
	float cutLength = 0;
	for(int i = 0; i < cutEdge.size(); i++)
	{
		centroid = centroid + cutEdge[i];

		if(i < cutEdge.size() - 1)
		{
			cutLength += MMSSVector3d::VectorLength(cutEdge[i + 1] - cutEdge[i]);
		}
	}
	cutLength += MMSSVector3d::VectorLength(cutEdge[0] - cutEdge[cutEdge.size() - 1]);
	centroid = (1.0f/cutEdge.size()) * centroid;

	//not used
	startDirection = MMSSVector3d::Normalize(startDirection);

	float minY = 99999; //angle very very close to 1
	int startIndex = 0;
	for(int i = 0; i < cutEdge.size(); i++)
	{
		if(cutEdge[i].y < minY)
		{
			minY = cutEdge[i].y;
			startIndex = i;
		}
	}

	//sort vertices to start with startIndex
	MMSSVector3d *cut = new MMSSVector3d[cutEdge.size()];
	for(int i = 0; i < cutEdge.size(); i++)
	{
		int ind = i + startIndex;
		if(ind >= cutEdge.size()) ind -= cutEdge.size();

		cut[i] = cutEdge[ind];
	}

	//calculate requested points
	float segmentLength = cutLength / numOfPoints;
	float lastLength = 0;
	float actualLength = 0;
	
	MMSSVector3d* interpolationList = new MMSSVector3d[numOfPoints];
	interpolationList[0] = cut[0];
	int interpolationPoints = 1;

	for(int i = 0; i < cutEdge.size() - 1; i++)
	{
		actualLength += MMSSVector3d::VectorLength(cut[i + 1] - cut[i]);

		//if one edge contains more than one interpolation point
		while(actualLength > segmentLength)
		{
			//interpolation and length on the last edge
			float len = segmentLength - lastLength;
			MMSSVector3d v = MMSSVector3d::Normalize(cut[i + 1] - cut[i]); //edge
			v = cut[i] + len * v; //position of the point in the space

			interpolationList[interpolationPoints] = v;
			interpolationPoints++;

			if(interpolationPoints >= numOfPoints) break;

			actualLength -= segmentLength;
			lastLength = -len;
		}
		if(interpolationPoints >= numOfPoints) break;

		lastLength = actualLength;
	}

	//test for the last edge
	if(interpolationPoints < numOfPoints)
	{
		actualLength += MMSSVector3d::VectorLength(cut[0] - cut[cutEdge.size()-1]);

		//if one edge contains more than one interpolation point
		while(actualLength > segmentLength)
		{
			//interpolation and length on the last edge
			float len = segmentLength - lastLength;
			MMSSVector3d v = MMSSVector3d::Normalize(cut[0] - cut[cutEdge.size()-1]); //edge
			v = cut[cutEdge.size()-1] + len * v; //position of the point in the space

			interpolationList[interpolationPoints] = v;
			interpolationPoints++;

			if(interpolationPoints >= numOfPoints) break;

			actualLength -= segmentLength;
			lastLength = -len;
		}
		lastLength = actualLength;
	}

	cutEdge.clear();
	delete[] cut;

	return interpolationList;
}

//----------------------------------------------------------------------------------------
//creates table of edge-vertex relationship, index = vertex index, entry = index of edge
vector<vector<unsigned int>> createPointEdgeTable(MMSSMorphing::Edge *edges, int edgeCount)
{
	//create table of edges and vertices
	vector<vector<unsigned int>> table;
	table.resize(meshVerticesCount);

	//for all vertices store the index
	for(unsigned int i = 0; i < edgeCount; i++)
	{
		table[edges[i].p1_index].push_back(i);
		table[edges[i].p2_index].push_back(i);
	}

	return table;
}

//-----------------------------------------------
// Vertex clustering of the muscle mesh vertices
// each vertex has its own cluster index, clustered are only
// vertices with the given masking index. Clusters the area of the mesh
// into two different clusters depending on the surface distance
// to the borders of the area A&B.
vector<vector<int>> clustering(int* clustering, int meshVertexCount, float* edgeLength, MMSSMorphing::Edge* edgeTable,
	vector<vector<unsigned int>> pointEdgeTable, int edgesCount, vector<int> A, vector<int> B, int maskingIndex, int newIndex)
{
	vector<vector<int>> results; //1. vector = cluster indices, 2. vector == left vertices, 3. vector == right vertices
	results.resize(3);

	//original clustering of the mesh vertices
	int* vertexIndex = clustering;

	/////////////////////////////////////////////////////////////////////////////
	// Go thru all edges of the mesh from A border to B border (forward)       //
	// then from B to A (backward). Edges, where the forward shortest distance //
	// and the backward shortest distance are switched lay between clusters.   //
	// They form edge between clusters - same distance to A and B as well.     //
	/////////////////////////////////////////////////////////////////////////////

	//distances for the forward
	float* vertexDistanceFor = (float*)malloc(meshVertexCount * sizeof(float));
	for(int i = 0; i < meshVertexCount; i++)
	{
		vertexDistanceFor[i] = FLT_MAX;
	}

	//distances for the backward
	float* vertexDistanceBack = (float*)malloc(meshVertexCount * sizeof(float));
	for(int i = 0; i < meshVertexCount; i++)
	{
		vertexDistanceBack[i] = FLT_MAX;
	}

	//used(processed) edges of the mesh
	bool* edgeUsed = (bool*)malloc(edgesCount * sizeof(bool));
	for(int i = 0; i < edgesCount; i++)
	{
		edgeUsed[i] = false;
	}

	//processed vertices of the mesh
	vector<int> lastVisitedVertices;
	vector<int> startVertices;
	vector<int> targetVertices;

	//////////////////////////////////
	// FORWARD DISTANCE CALCULATION //
	//////////////////////////////////
	for(int i = 0; i < A.size(); i++)
	{
		lastVisitedVertices.push_back(A[i]);
		vertexDistanceFor[A[i]] = 0;
	}

	for(int i = 0; i < B.size(); i++)
	{
		targetVertices.push_back(B[i]);
	}

	//start vrtices and target vertices, cycle thru edges
	while(lastVisitedVertices.size() > 0)
	{
		//all from lastVisited marked as visited
		//and moved into the startVertex field
		//clear the lastVisited
		startVertices.clear();
		for(int i = 0; i < lastVisitedVertices.size(); i++)
		{
			vertexIndex[lastVisitedVertices[i]] = newIndex;
			startVertices.push_back(lastVisitedVertices[i]);
		}

		lastVisitedVertices.clear();

		//for each start vertex find incident edges and store the distance to the edge
		for(int i = 0; i < startVertices.size(); i++)
		{
			//for each incident edge
			for(int j = 0; j < pointEdgeTable[startVertices[i]].size(); j++)
			{
				//which of two wertices is the visited and where we want to go
				int edgeIndex = pointEdgeTable[startVertices[i]][j];
				int targetVertex = edgeTable[edgeIndex].p1_index;
				int startVertex = edgeTable[edgeIndex].p2_index;
				if(edgeTable[edgeIndex].p1_index == startVertices[i])
				{
					targetVertex = edgeTable[edgeIndex].p2_index;
					startVertex = edgeTable[edgeIndex].p1_index;
				}

				//process only the masked area of the mesh
				if(vertexIndex[targetVertex] == maskingIndex)
				{
					if(!edgeUsed[edgeIndex])
					{
						edgeUsed[edgeIndex] = true;

						bool known = false;
						for(int k = 0; k < lastVisitedVertices.size(); k++)
						{
							if(lastVisitedVertices[k] == targetVertex) known = true;
						}

						if(!known) lastVisitedVertices.push_back(targetVertex);

						//store the shortest distance
						vertexDistanceFor[targetVertex] = std::min(vertexDistanceFor[targetVertex], vertexDistanceFor[startVertex] + edgeLength[edgeIndex]);
					}
				}
			}
		}
	}

	///////////////////////////////////
	// BACKWARD DISTANCE CALCULATION //
	///////////////////////////////////
	vector<int> contureIndicesA;
	vector<int> contureIndicesB;
	for(int i = 0; i < edgesCount; i++)
	{
		edgeUsed[i] = false;
	}

	for(int i = 0; i < B.size(); i++)
	{
		lastVisitedVertices.push_back(B[i]);
		vertexDistanceBack[B[i]] = 0;
	}

	for(int i = 0; i < A.size(); i++)
	{
		targetVertices.push_back(A[i]);
	}

	//start vertices and target vertices, cycle thru edges
	while(lastVisitedVertices.size() > 0)
	{
		//all from lastVisited marked as visited
		//and moved into the startVertex field
		//clear the lastVisited
		startVertices.clear();
		for(int i = 0; i < lastVisitedVertices.size(); i++)
		{
			vertexIndex[lastVisitedVertices[i]] = newIndex+1;
			startVertices.push_back(lastVisitedVertices[i]);
		}

		lastVisitedVertices.clear();

		//for each start vertex find incident edges and store the distance to the edge
		for(int i = 0; i < startVertices.size(); i++)
		{
			//for each incident edge
			for(int j = 0; j < pointEdgeTable[startVertices[i]].size(); j++)
			{
				//which of two wertices is the visited and where we want to go
				int edgeIndex = pointEdgeTable[startVertices[i]][j];
				int targetVertex = edgeTable[edgeIndex].p1_index;
				int startVertex = edgeTable[edgeIndex].p2_index;
				if(edgeTable[edgeIndex].p1_index == startVertices[i])
				{
					targetVertex = edgeTable[edgeIndex].p2_index;
					startVertex = edgeTable[edgeIndex].p1_index;
				}

				//process only the masked area of the mesh
				if(vertexIndex[targetVertex] == newIndex)
				{
					if(!edgeUsed[edgeIndex])
					{
						edgeUsed[edgeIndex] = true;

						//store the shortest distance
						vertexDistanceBack[targetVertex] = min(vertexDistanceBack[targetVertex], vertexDistanceBack[startVertex] + edgeLength[edgeIndex]);

						//if it is closer to the other border
						if(vertexDistanceBack[targetVertex] < vertexDistanceFor[targetVertex])
						{
							bool known = false;
							for(int k = 0; k < lastVisitedVertices.size(); k++)
							{
								if(lastVisitedVertices[k] == targetVertex) known = true;
							}

							if(!known) lastVisitedVertices.push_back(targetVertex);
						}
						else //store the vertex pair
						{
							bool known = false;
							for(int k = 0; k < contureIndicesA.size(); k++)
							{
								if(contureIndicesA[k] == startVertex) known = true;
							}

							if(!known) contureIndicesA.push_back(startVertex);

							known = false;
							for(int k = 0; k < contureIndicesB.size(); k++)
							{
								if(contureIndicesB[k] == targetVertex) known = true;
							}

							if(!known) contureIndicesB.push_back(targetVertex);
						}
					}
				}
			}
		}
	}

	results[1] = contureIndicesB;
	results[2] = contureIndicesA;

	return results;
}

//------------------------------------------------------------------------
//This method is the one that should be used by subclasses, right now the
//default implementation is to call the backwards compatibility method
/*virtual*/void vtkMAFMuscleDecompositionMMSS::ExecuteData(vtkDataObject *output)
{
#pragma region Input Checks
	//check whether output is valid
	vtkPolyData* input = GetInput();
	if (input == NULL || input->GetNumberOfPoints() == 0)
		return;

	vtkPolyData* pPoly = vtkPolyData::SafeDownCast(output);
	if (pPoly == NULL)
	{
		mafLogMessage("Invalid output for vtkMAFMuscleDecompositionMMSS.");
		return;   //we have no valid output
	}

	if (NumberOfFibres <= 0 || Resolution <= 0)
	{
		mafLogMessage("Invalid output for vtkMAFMuscleDecompositionMMSS.");
		return;
	}

	//check, whether we have the right number of fibers
	//we need odd number of fibers per axis (eg. 7x7xResolution)
	int fibresInAxis = (int)sqrt((double)NumberOfFibres);

	//if the number is not odd, increment the number of fibers
	if(fibresInAxis % 2 == 0)
	{
		fibresInAxis++;
		mafLogMessage("Number of fibres changed to %d.", fibresInAxis * fibresInAxis);
	}

	NumberOfFibres = fibresInAxis * fibresInAxis;

#pragma endregion

#pragma region Get the muscle mesh vertices and source/insertion areas

	vtkPoints* mesh = input->GetPoints();
	meshTrianglesCount = input->GetNumberOfCells();
	meshTriangles = new MMSSMorphing::Triangle[meshTrianglesCount];
	meshNormals = new MMSSVector3d[meshTrianglesCount];
	meshVerticesCount = mesh->GetNumberOfPoints();
	meshVertices = new MMSSVector3d[meshVerticesCount];

	double x[3];
	//convert points of mesh with openMaf representation to MMSSVector3d structure representation
	for(int i = 0; i < meshVerticesCount; i++)
	{
		mesh->GetPoint(i,x);

		meshVertices[i].x = (float)x[0];
		meshVertices[i].y = (float)x[1];
		meshVertices[i].z = (float)x[2];
	}

	for(int i = 0; i < meshTrianglesCount; i++)
	{
		vtkCell* c = GetInput()->GetCell(i);
		vtkIdList* l = c->GetPointIds();
		meshTriangles[i].p1_index = l->GetId(0);
		meshTriangles[i].p2_index = l->GetId(1);
		meshTriangles[i].p3_index = l->GetId(2);

		//calc normals
		MMSSVector3d N = MMSSVector3d::VectorCross(meshVertices[meshTriangles[i].p2_index]-meshVertices[meshTriangles[i].p1_index],
												   meshVertices[meshTriangles[i].p3_index]-meshVertices[meshTriangles[i].p1_index]);
		meshNormals[i] = MMSSVector3d::Normalize(N);
	}


	MMSSVector3d* targetOriginAreaVector = new MMSSVector3d[OriginArea->GetNumberOfPoints()];
	MMSSVector3d* targetInsertionAreaVector = new MMSSVector3d[InsertionArea->GetNumberOfPoints()];

	for(int i = 0; i < OriginArea->GetNumberOfPoints(); i++)
	{
		OriginArea->GetPoint(i,x);

		targetOriginAreaVector[i].x = (float)x[0];
		targetOriginAreaVector[i].y = (float)x[1];
		targetOriginAreaVector[i].z = (float)x[2];
	}

	for(int i = 0; i < InsertionArea->GetNumberOfPoints(); i++)
	{
		InsertionArea->GetPoint(i,x);

		targetInsertionAreaVector[i].x = (float)x[0];
		targetInsertionAreaVector[i].y = (float)x[1];
		targetInsertionAreaVector[i].z = (float)x[2];
	}

	MMSSVector3d zero = {0, 0, 0};//zero vector
	int layers = Resolution + 1;  //number of template layers

	//we need to find minimal oriented box that fits the input point data
	//so that all points are inside of this box (or on its boundary)
	//and the total squared distance of template origin points from
	//the input mesh origin points and the total squared distance of
	//template insertion points from the input mesh origin points
	//are minimized
	LOCAL_FRAME lf, lfNorm;
	ComputeFittingOB(input->GetPoints(), lf);

	lfNorm = lf;
	int iPlane = 0;
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
	
	int iPlane2 = (iPlane + 2) % 3; //fibre templates use different coordinate system
	int iPlane3 = (iPlane + 1) % 3;

	//lfNorm now contains the local frame with normalized axis
	//and iPlane is the index of longest axis

#pragma endregion
//mafLogMessage("Data loaded from input.");
#pragma region Decomposition of mesh triangles into edges -> edgeTriTable, edgeTable

	//decompose triangles to edges
	unsigned int edgesCount = (meshTrianglesCount*3)/2;
	MMSSMorphing::Edge* edgeTable = (MMSSMorphing::Edge*)malloc(edgesCount * sizeof(MMSSMorphing::Edge));
	float* edgeLength = (float*)malloc(edgesCount * sizeof(float));
	bool* hasPair = (bool*)malloc(edgesCount * sizeof(bool));
	for(int i = 0; i < edgesCount; i++) hasPair[i] = false;
	MMSSMorphing::Edge edge;
	MMSSVector3d v = {0, 0, 0};
	bool fnd = false;
	unsigned int ind = 0;

	for(unsigned int i = 0; i < meshTrianglesCount; i++)
	{
		//first edge of triangle
		edge.p1_index = meshTriangles[i].p1_index;
		edge.p2_index = meshTriangles[i].p2_index;
		fnd = false;
		for(unsigned int j = 0; j < ind; j++)
		{
			if(hasPair[j] == false)
			{
				if(edgeTable[j].p1_index == edge.p1_index && edgeTable[j].p2_index == edge.p2_index
					|| edgeTable[j].p1_index == edge.p2_index && edgeTable[j].p2_index == edge.p1_index)
				{
					fnd = true;
					hasPair[j] = true;
					break;
				}
			}
		}
		if(!fnd)
		{
			edgeTable[ind] = edge;
			v = meshVertices[edge.p1_index] - meshVertices[edge.p2_index];
			edgeLength[ind] = MMSSVector3d::VectorLength(v);
			ind++;
		}

		//second edge of triangle
		edge.p1_index = meshTriangles[i].p1_index;
		edge.p2_index = meshTriangles[i].p3_index;
		fnd = false;
		for(unsigned int j = 0; j < ind; j++)
		{
			if(hasPair[j] == false)
			{
				if(edgeTable[j].p1_index == edge.p1_index && edgeTable[j].p2_index == edge.p2_index
					|| edgeTable[j].p1_index == edge.p2_index && edgeTable[j].p2_index == edge.p1_index)
				{
					fnd = true;
					hasPair[j] = true;
					break;
				}
			}
		}
		if(!fnd)
		{
			edgeTable[ind] = edge;
			v = meshVertices[edge.p1_index] - meshVertices[edge.p2_index];
			edgeLength[ind] = MMSSVector3d::VectorLength(v);
			ind++;
		}

		//third edge of triangle
		edge.p1_index = meshTriangles[i].p2_index;
		edge.p2_index = meshTriangles[i].p3_index;
		fnd = false;
		for(unsigned int j = 0; j < ind; j++)
		{
			if(hasPair[j] == false)
			{
				if(edgeTable[j].p1_index == edge.p1_index && edgeTable[j].p2_index == edge.p2_index
					|| edgeTable[j].p1_index == edge.p2_index && edgeTable[j].p2_index == edge.p1_index)
				{
					fnd = true;
					hasPair[j] = true;
					break;
				}
			}
		}
		if(!fnd)
		{
			edgeTable[ind] = edge;
			v = meshVertices[edge.p1_index] - meshVertices[edge.p2_index];
			edgeLength[ind] = MMSSVector3d::VectorLength(v);
			ind++;
		}
	}


	//create table of edges and triangles
	//table index / 3 == triangle index
	GLuint **edgeTriTable = createEdgeTriTable(meshTriangles, meshTrianglesCount);
#pragma endregion
	
#pragma region RedCut
	//
	// possible cause bugs! need something more universal
	//
	MMSSVector3d down = {lfNorm.uvw[iPlane3][0], lfNorm.uvw[iPlane3][1], lfNorm.uvw[iPlane3][2]};

	//number of edges of the cut
	int numOfEdges = 2*fibresInAxis + 2*(fibresInAxis-2);

	//don't use more than 16 projected points
	vector<Vector4D> projections;
	int vertCount = OriginArea->GetNumberOfPoints();
	/*if(OriginArea->GetNumberOfPoints() > 16) vertCount = 16;
	
	if(OriginArea->GetNumberOfPoints() > 16)
	{
		float len = 0;
		for(int i = 1; i < OriginArea->GetNumberOfPoints(); i++)
		{
			len += MMSSVector3d::VectorLength(targetOriginAreaVector[i] - targetOriginAreaVector[i - 1]);
		}
		len += MMSSVector3d::VectorLength(targetOriginAreaVector[0] - targetOriginAreaVector[OriginArea->GetNumberOfPoints() - 1]);

		float step = len / 16.0f;
		MMSSVector3d* origVerts = (MMSSVector3d*)malloc(vertCount * sizeof(MMSSVector3d));

		origVerts[0] = targetOriginAreaVector[0];

		float nextStepDist = step;
		float lastPointDist = 0;
		float nextPointDist = 0;
		int index = 1;
		int lastUsed = 0;
		for(int i = 1; i < OriginArea->GetNumberOfPoints(); i++)
		{
			lastPointDist = nextPointDist;
			nextPointDist += MMSSVector3d::VectorLength(targetOriginAreaVector[i] - targetOriginAreaVector[i - 1]);

			if(nextPointDist > nextStepDist)
			{
				if(lastUsed < (i-1))
				{
					origVerts[index] = targetOriginAreaVector[i - 1];
					index++;
					nextStepDist += step;
					lastUsed = (i-1);
					if(index == 16) break;
				}

				///MMSSVector3d dir = MMSSVector3d::Normalize(targetOriginAreaVector[i] - targetOriginAreaVector[i - 1]);
				//MMSSVector3d newPos = targetOriginAreaVector[i - 1] + (nextStepDist - lastPointDist) * dir;
				//origVerts[index] = newPos;
				//index++;
				//nextStepDist += step;
			}
		}

		projections = projectPointsOntoSurface(origVerts, vertCount, meshTriangles, meshTrianglesCount, meshVertices);
	}
	else*/
	{
		projections = projectPointsOntoSurface(targetOriginAreaVector, vertCount, meshTriangles, meshTrianglesCount, meshVertices, NULL, -1);
	}

	//projection of vertices on the mesh surface
	
	//projected vertices controlling the cut shape
	MMSSVector3d* controlVertices = (MMSSVector3d*)malloc(vertCount * sizeof(MMSSVector3d));

	//triangles including the control vertices
	GLuint* trianglesWithControlVerticesR = (GLuint*)malloc(vertCount * sizeof(GLuint));

	int triCntR = projections.size();
	for(int i = 0; i < projections.size(); i++)
	{
		MMSSVector3d v = {0, 0, 0};
		v.x = projections[i].x;
		v.y = projections[i].y;
		v.z = projections[i].z;
		controlVertices[i] = v;
		trianglesWithControlVerticesR[i] = (GLuint)projections[i].w;
	}

	//vertices of the RED cut
	MMSSVector3d* redCut = cutMuscleTip(trianglesWithControlVerticesR, controlVertices, vertCount, edgeTriTable, down, numOfEdges);

#pragma endregion shape of the insertion area on the surface of the muscle mesh
	
#pragma region BlueCut
	//projection of vertices on the mesh surface
	projections = projectPointsOntoSurface(targetInsertionAreaVector, InsertionArea->GetNumberOfPoints(), meshTriangles, meshTrianglesCount, meshVertices, NULL, -1);

	//projected vertices controlling the cut shape
	controlVertices = (MMSSVector3d*)malloc((InsertionArea->GetNumberOfPoints()) * sizeof(MMSSVector3d)); //pozice vrcholu po projekci na nejblizsi trojuhelniky

	//triangles including the control vertices
	GLuint* trianglesWithControlVerticesB = (GLuint*)malloc((InsertionArea->GetNumberOfPoints()) * sizeof(GLuint)); //indexy trojuhelniku s projekci vrcholu

	int triCntB = projections.size();
	for(int i = 0; i < projections.size(); i++)
	{
		MMSSVector3d v = {0, 0, 0};
		v.x = projections[i].x;
		v.y = projections[i].y;
		v.z = projections[i].z;
		controlVertices[i] = v;
		trianglesWithControlVerticesB[i] = (GLuint)projections[i].w;
	}

	//vertices of the RED cut
	MMSSVector3d* blueCut = cutMuscleTip(trianglesWithControlVerticesB, controlVertices, InsertionArea->GetNumberOfPoints(), edgeTriTable, down, numOfEdges);
#pragma endregion shape of the origin area on the surface of the muscle mesh
	
#pragma region Red & Blue normal
	// WE SUPPOSE THE ORIENTATION OF THE INSERTION / SOURCE AREAS IS THE SAME

	//centroid of the RED cut
	MMSSVector3d redCentroid = {0, 0, 0};
	for(int i = 0; i < numOfEdges; i++)	redCentroid = redCentroid + redCut[i];
	redCentroid = (1.0f/(float)numOfEdges) * redCentroid;

	//get the avg. normal of the "cut-plane"
	MMSSVector3d redNormal = {0, 0, 0};
	for(int i = 0; i < numOfEdges-1; i++)
	{
		//normal of one triangle
		MMSSVector3d n = MMSSVector3d::VectorCross((redCut[i] - redCentroid), (redCut[i+1] - redCentroid));
		if(n * redNormal < 0)
		{
			redNormal = redNormal - MMSSVector3d::Normalize(n);
		}
		else redNormal = redNormal + MMSSVector3d::Normalize(n);
	}

	//normal of the last triangle
	MMSSVector3d n = MMSSVector3d::VectorCross((redCut[numOfEdges-1] - redCentroid), (redCut[0] - redCentroid));

	if(n * redNormal < 0)
	{
		redNormal = redNormal - MMSSVector3d::Normalize(n);
	}
	else redNormal = redNormal + MMSSVector3d::Normalize(n);

	//avg. RED "cut-plane" normal
	redNormal = MMSSVector3d::Normalize((1.0f / (float)numOfEdges) * redNormal);

	//centroid of the BLUE cut
	MMSSVector3d blueCentroid = {0, 0, 0};
	for(int i = 0; i < numOfEdges; i++)	blueCentroid = blueCentroid + blueCut[i];
	blueCentroid = (1.0f/(float)numOfEdges) * blueCentroid;

	//get the avg. normal of the "cut-plane"
	MMSSVector3d blueNormal = {0, 0, 0};
	for(int i = 0; i < numOfEdges-1; i++)
	{
		//normal of one triangle
		MMSSVector3d n = MMSSVector3d::VectorCross((blueCut[i] - blueCentroid), (blueCut[i+1] - blueCentroid));
		if(n * blueNormal < 0)
		{
			blueNormal = blueNormal - MMSSVector3d::Normalize(n);
		}
		else blueNormal = blueNormal + MMSSVector3d::Normalize(n);
	}

	//normal of the last triangle
	MMSSVector3d m = MMSSVector3d::VectorCross((blueCut[numOfEdges-1] - blueCentroid), (blueCut[0] - blueCentroid));

	if(m * blueNormal < 0)
	{
		blueNormal = blueNormal - MMSSVector3d::Normalize(m);
	}
	else blueNormal = blueNormal + MMSSVector3d::Normalize(m);

	//avg. BLUE "cut-plane" normal
	blueNormal = MMSSVector3d::Normalize((1.0f / (float)numOfEdges) * blueNormal);

	if((blueCentroid - redCentroid) * redNormal > 0)
	{
		redNormal = -1 * redNormal;
	}

	if((blueCentroid - redCentroid) * blueNormal < 0)
	{
		blueNormal = -1 * blueNormal;

	}
#pragma endregion normals of "cutting planes" cutting red&blue areas out of the mmuscle mesh

#pragma region Muscle mesh vertex clustering

	//create table of edges and vertices
	//index = vertex index, entry = edge index
	int* indices = (int*)malloc(meshVerticesCount * sizeof(int));
	vector<vector<unsigned int>> pointEdgeTable = createPointEdgeTable(edgeTable, edgesCount);
	vector<int> start;
	vector<int> end;

	//insert RED and BLUE cuts
	for(int i = 0; i < triCntR; i++)
	{
		start.push_back(meshTriangles[trianglesWithControlVerticesR[i]].p1_index);
		start.push_back(meshTriangles[trianglesWithControlVerticesR[i]].p2_index);
		start.push_back(meshTriangles[trianglesWithControlVerticesR[i]].p3_index);
	}

	for(int i = 0; i < triCntB; i++)
	{
		end.push_back(meshTriangles[trianglesWithControlVerticesB[i]].p1_index);
		end.push_back(meshTriangles[trianglesWithControlVerticesB[i]].p2_index);
		end.push_back(meshTriangles[trianglesWithControlVerticesB[i]].p3_index);
	}

	for(int i = 0; i < meshVerticesCount; i++)
	{
		indices[i] = -1;
	}

	vector<vector<int>> res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, start, end, -1, 0);//vzniknou clustery -1 -> 0,1

	//copy of 2 obtained contours between clusters
	vector<int> contour0;
	contour0.resize(res[1].size());
	for(int i = 0; i < contour0.size(); i++) contour0[i] = res[1][i];

	vector<int> contour1;
	contour1.resize(res[2].size());
	for(int i = 0; i < contour1.size(); i++) contour1[i] = res[2][i];

	//next clustering
	//we will get clusters 0 -> 2, 3
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, start, contour0, 0, 2);

	//copy of 2 obtained contours between clusters
	vector<int> contour2;
	contour2.resize(res[1].size());
	for(int i = 0; i < contour2.size(); i++) contour2[i] = res[1][i];

	vector<int> contour3;
	contour3.resize(res[2].size());
	for(int i = 0; i < contour3.size(); i++) contour3[i] = res[2][i];

	//next clustering
	//we will get clusters 1 -> 4, 5
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, contour1, end, 1, 4);

	//copy of 2 obtained contours between clusters
	vector<int> contour4;
	contour4.resize(res[1].size());
	for(int i = 0; i < contour4.size(); i++) contour4[i] = res[1][i];

	vector<int> contour5;
	contour5.resize(res[2].size());
	for(int i = 0; i < contour5.size(); i++) contour5[i] = res[2][i];

	//next clustering
	//we will get clusters 2 -> 6, 7
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, start, contour2, 2, 6);

	////copy of 2 obtained contours between clusters
	//vector<int> contour6;
	//contour6.resize(res[1].size());
	//for(int i = 0; i < contour6.size(); i++) contour6[i] = res[1][i];

	//vector<int> contour7;
	//contour7.resize(res[2].size());
	//for(int i = 0; i < contour7.size(); i++) contour7[i] = res[2][i];

	//next clustering
	//we will get clusters 3 -> 8, 9
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, contour3, contour0, 3, 8);

	////copy of 2 obtained contours between clusters
	//vector<int> contour8;
	//contour8.resize(res[1].size());
	//for(int i = 0; i < contour8.size(); i++) contour8[i] = res[1][i];

	//vector<int> contour9;
	//contour9.resize(res[2].size());
	//for(int i = 0; i < contour9.size(); i++) contour9[i] = res[2][i];

	//next clustering
	//we will get clusters 4 -> 10, 11
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, contour1, contour4, 4, 10);

	////copy of 2 obtained contours between clusters
	//vector<int> contour10;
	//contour10.resize(res[1].size());
	//for(int i = 0; i < contour10.size(); i++) contour10[i] = res[1][i];

	//vector<int> contour11;
	//contour11.resize(res[2].size());
	//for(int i = 0; i < contour11.size(); i++) contour11[i] = res[2][i];

	//next clustering
	//we will get clusters 5 -> 12, 13
	res = clustering(indices, meshVerticesCount, edgeLength, edgeTable,
		pointEdgeTable, edgesCount, contour5, end, 5, 12);

	////copy of 2 obtained contours between clusters
	//vector<int> contour12;
	//contour12.resize(res[1].size());
	//for(int i = 0; i < contour12.size(); i++) contour12[i] = res[1][i];

	//vector<int> contour13;
	//contour13.resize(res[2].size());
	//for(int i = 0; i < contour13.size(); i++) contour13[i] = res[2][i];

#pragma endregion surface-distance vertex clustering, basis for spine construction
	
#pragma region Create bones (spine) of the muscle from centroid clusters

	MMSSVector3d* centroids = (MMSSVector3d*)malloc(8 * sizeof(MMSSVector3d));
	int* sizes = (int*)malloc(8 * sizeof(int));

	//calculate centroids of clusters
	for(int i = 0; i < 8; i++)
	{
		centroids[i] = zero;
		sizes[i] = 0;
	}

	for(int i = 0; i < meshVerticesCount; i++)
	{
		centroids[indices[i]-6] = centroids[indices[i]-6] + meshVertices[i];
		sizes[indices[i]-6]++;
	}

	for(int i = 0; i < 8; i++)
	{
		centroids[i] = (1.0 / sizes[i]) * centroids[i];
	}

	//NEW! : smooth positions of the centroids -spin vertices
	//use the smoothed position only if the resulted spine is more "narrow"
	//centroids[0] = 0.5f * (centroids[0] + redCentroid);
	//centroids[7] = 0.5f * (centroids[7] + blueCentroid);

	MMSSVector3d v1, v2, v3, v4, v5, v6; //positions
	MMSSVector3d d0, d1, d2, d3, d4, d5, d6; //original directions

	//smoothed positions of vertices of the spine
	//only those position which makes the spine more narrow, will be preserved
	v1 = 0.5f * (centroids[0] + centroids[2]);
	v2 = 0.5f * (centroids[1] + centroids[3]);
	v3 = 0.5f * (centroids[2] + centroids[4]);
	v4 = 0.5f * (centroids[3] + centroids[5]);
	v5 = 0.5f * (centroids[4] + centroids[6]);
	v6 = 0.5f * (centroids[5] + centroids[7]);

	v1 = 0.5f * (centroids[1] + v1);
	v2 = 0.5f * (centroids[2] + v2);
	v3 = 0.5f * (centroids[3] + v3);
	v4 = 0.5f * (centroids[4] + v4);
	v5 = 0.5f * (centroids[5] + v5);
	v6 = 0.5f * (centroids[6] + v6);

	//original segment directions
	d0 = MMSSVector3d::Normalize(centroids[1] - centroids[0]);
	d1 = MMSSVector3d::Normalize(centroids[2] - centroids[1]);
	d2 = MMSSVector3d::Normalize(centroids[3] - centroids[2]);
	d3 = MMSSVector3d::Normalize(centroids[4] - centroids[3]);
	d4 = MMSSVector3d::Normalize(centroids[5] - centroids[4]);
	d5 = MMSSVector3d::Normalize(centroids[6] - centroids[5]);
	d6 = MMSSVector3d::Normalize(centroids[7] - centroids[6]);

	//angular changes for v2, v3, v4, v5
	// -- v2 -- //
	float origAngle1 = d0 * d1;
	float origAngle2 = d2 * d3;

	float newAngle1 = d0 * MMSSVector3d::Normalize(v2 - centroids[1]);
	float newAngle2 = MMSSVector3d::Normalize(centroids[3] - v2) * d3;

	//if new position makes the spine less narrow
	if(origAngle1 + origAngle2 > newAngle1 + newAngle1)
	{
		v2 = centroids[2];
	}

	// -- v3 -- //
	origAngle1 = d1 * d2;
	origAngle2 = d3 * d4;

	newAngle1 = d1 * MMSSVector3d::Normalize(v3 - centroids[2]);
	newAngle2 = MMSSVector3d::Normalize(centroids[4] - v3) * d4;

	//if new position makes the spine less narrow
	if(origAngle1 + origAngle2 > newAngle1 + newAngle1)
	{
		v3 = centroids[3];
	}

	// -- v4 -- //
	origAngle1 = d2 * d3;
	origAngle2 = d4 * d5;

	newAngle1 = d2 * MMSSVector3d::Normalize(v4 - centroids[3]);
	newAngle2 = MMSSVector3d::Normalize(centroids[5] - v4) * d5;

	//if new position makes the spine less narrow
	if(origAngle1 + origAngle2 > newAngle1 + newAngle1)
	{
		v4 = centroids[4];
	}

	// -- v5 -- //
	origAngle1 = d3 * d4;
	origAngle2 = d5 * d6;

	newAngle1 = d3 * MMSSVector3d::Normalize(v5 - centroids[4]);
	newAngle2 = MMSSVector3d::Normalize(centroids[6] - v5) * d6;

	//if new position makes the spine less narrow
	if(origAngle1 + origAngle2 > newAngle1 + newAngle1)
	{
		v5 = centroids[5];
	}

	//apply the spine smoothing
	centroids[1] = v1;
	centroids[2] = v2;
	centroids[3] = v3;
	centroids[4] = v4;
	centroids[5] = v5;
	centroids[6] = v6;

	//calculate length of the spine and coordinates of cutting planes
	//placed across the spine
	float skeletonLen = 0; //length of the whole spine
	for(int i = 0; i < 8-1; i++)
	{
		skeletonLen += MMSSVector3d::VectorLength(centroids[i] - centroids[i+1]);
	}
	float segmentLen = skeletonLen / ((float)layers - 1.0); //length of one segment of the spine

	float actualLen = segmentLen;
	float targetLen = 0;
	vector<MMSSVector3d> intersections; //intersections of the spine and cutting planes
	vector<MMSSVector3d> normals; //normals of the cutting planes

	for(int i = 0; i < 8-1; i++)
	{
		targetLen += MMSSVector3d::VectorLength(centroids[i] - centroids[i+1]);
		while(targetLen > actualLen)
		{
			float lastBoneLen = MMSSVector3d::VectorLength(centroids[i] - centroids[i+1]); //length of the actual bone
			float lenToStart = targetLen - lastBoneLen; //where the bone begins, lastLen = the residue from the foregoing bone

			float len = actualLen - lenToStart; //where we are now, on the bone

			//interpolation of "cut-plane" normals
			MMSSVector3d normalA, normalB, normalC;
			if(i>0)	normalA = MMSSVector3d::Normalize(centroids[i] - centroids[i-1]);
			else normalA = MMSSVector3d::Normalize(redNormal);

			normalB = MMSSVector3d::Normalize(centroids[i+1] - centroids[i]);

			if(i<6)	normalC = MMSSVector3d::Normalize(centroids[i+2] - centroids[i+1]);
			else normalC = MMSSVector3d::Normalize((-1)*blueNormal);

			normalA = 0.5f * (normalA + normalB);
			normalC = 0.5f * (normalC + normalB);

			float normalInterp = (len / lastBoneLen);
			MMSSVector3d normal;
			if(normalInterp > 0.5f) normal = (((normalInterp-0.5f)*2.0f) * normalC) + ((1 - (normalInterp-0.5f)*2.0f) * normalB);
			else  normal = ((normalInterp*2.0f) * normalB) + ((1 - normalInterp*2.0f) * normalA);

			MMSSVector3d point = centroids[i] + len * MMSSVector3d::Normalize(centroids[i+1] - centroids[i]);
			intersections.push_back(point);
			normals.push_back(normal);

			actualLen += segmentLen;

			if(intersections.size() >= layers - 2) break;
		}
	}

#pragma endregion

#pragma region RedNormal, RedFarPoint and BlueNormal, BlueFarPoint

	//+/- normal of the cap cutting plane normal
	MMSSVector3d redCapNormal = MMSSVector3d::Normalize((centroids[0]-centroids[1]));
	MMSSVector3d blueCapNormal = MMSSVector3d::Normalize((centroids[7]-centroids[6]));

	//find the point with angle closest to the vector between centroid and capCentroid
	float maxDist = 0;
	MMSSVector3d redFarPoint = {0, 0, 0};
	int redFarPointIndex = -1;
	float minAngle = -1;
	for(int i = 0; i < meshVerticesCount; i++)
	{
		float angle = MMSSVector3d::Normalize(meshVertices[i] - redCentroid) * redCapNormal;

		if(angle >= 0.999)
		{
			if(MMSSVector3d::VectorLength(meshVertices[i] - redCentroid) > maxDist)
			{
				maxDist = MMSSVector3d::VectorLength(meshVertices[i] - redCentroid);
				redFarPoint = meshVertices[i];
				redFarPointIndex = i;
			}
		}
	}
	if(redFarPointIndex == -1)
		for(int i = 0; i < meshVerticesCount; i++)
		{
			float angle = MMSSVector3d::Normalize(meshVertices[i] - redCentroid) * redCapNormal;

			if(angle > minAngle)
			{
				minAngle = angle;
				redFarPoint = meshVertices[i];
				redFarPointIndex = i;
			}
		}

		maxDist = 0;
		MMSSVector3d blueFarPoint = {0, 0, 0};
		int blueFarPointIndex = -1;
		minAngle = -1;
		for(int i = 0; i < meshVerticesCount; i++)
		{
			float angle = MMSSVector3d::Normalize(meshVertices[i] - blueCentroid) * blueCapNormal;

			if(angle >= 0.999)
			{
				if(MMSSVector3d::VectorLength(meshVertices[i] - blueCentroid) > maxDist)
				{
					maxDist = MMSSVector3d::VectorLength(meshVertices[i] - blueCentroid);
					blueFarPoint = meshVertices[i];
					blueFarPointIndex = i;
				}
			}
		}
		if(blueFarPointIndex == -1)
			for(int i = 0; i < meshVerticesCount; i++)
			{
				float angle = MMSSVector3d::Normalize(meshVertices[i] - blueCentroid) * blueCapNormal;

				if(angle > minAngle)
				{
					minAngle = angle;
					blueFarPoint = meshVertices[i];
					blueFarPointIndex = i;
				}
			}
#pragma endregion

#pragma region Inner slices

			MMSSVector3d* controlVertices2;         //control vertices of the cut (slice)
			GLuint* trianglesWithControlVertices2;  //triangles containing control vertices
			vector<MMSSVector3d*> cuts;             //list of all slices

			int usedSlices = 0;

			//list of slices begins by redCut -insertion area
			cuts.push_back(redCut);
			usedSlices++;

			//layers - 2 = all slices withour red & blue cut
			for(int i = 0; i < layers - 2; i++)
			{
				//cut - coordinate system construction
				MMSSVector3d forward = {0, 0, 0};
				if(i < (layers-3)/2)
				{
					float b = i + 1;
					float a = (((float)layers-3)/2-i);

					forward = (1 / (float)((layers-1)/2)) * ((a * redNormal) - b * normals[i]); //normal interpolation
					forward = 0.5f * (forward - normals[i]);
				}
				else if(i == (layers-3)/2)
				{
					forward = (-1) * normals[(normals.size()-1) / 2];
				}
				else
				{
					float a = i - ((float)layers-3)/2;
					float b = (layers - 2) - i;

					forward = (-1 / (float)((layers-1)/2)) * ((a * blueNormal) + b * normals[i]); //normal interpolation
					forward = 0.5f * (forward - normals[i]);
				}

				MMSSVector3d cutCenter = intersections[i];
				MMSSVector3d left = MMSSVector3d::Normalize(MMSSVector3d::VectorCross(forward, down));
				MMSSVector3d up = MMSSVector3d::Normalize(MMSSVector3d::VectorCross(forward, left));

				//create rays of the cutting plane in the coord. sys. of the plane
				MMSSVector3d* controlVertices2tmp = (MMSSVector3d*)malloc(16 * sizeof(MMSSVector3d));

				MMSSVector3d v1 = cutCenter + left;
				MMSSVector3d v2 = cutCenter + left + left+ up;
				MMSSVector3d v3 = cutCenter + left + up;
				MMSSVector3d v4 = cutCenter + left + up + up;
				MMSSVector3d v5 = cutCenter + up;
				MMSSVector3d v6 = cutCenter - left + up + up;
				MMSSVector3d v7 = cutCenter - left + up;
				MMSSVector3d v8 = cutCenter - left - left + up;
				MMSSVector3d v9 = cutCenter - left;
				MMSSVector3d v10 = cutCenter - left - left - up;
				MMSSVector3d v11 = cutCenter - left - up;
				MMSSVector3d v12 = cutCenter - left - up - up;
				MMSSVector3d v13 = cutCenter - up;
				MMSSVector3d v14 = cutCenter + left - up - up;
				MMSSVector3d v15 = cutCenter + left - up;
				MMSSVector3d v16 = cutCenter + left + left - up;

				controlVertices2tmp[0] = v1;
				controlVertices2tmp[1] = v2;
				controlVertices2tmp[2] = v3;
				controlVertices2tmp[3] = v4;
				controlVertices2tmp[4] = v5;
				controlVertices2tmp[5] = v6;
				controlVertices2tmp[6] = v7;
				controlVertices2tmp[7] = v8;
				controlVertices2tmp[8] = v9;
				controlVertices2tmp[9] = v10;
				controlVertices2tmp[10] = v11;
				controlVertices2tmp[11] = v12;
				controlVertices2tmp[12] = v13;
				controlVertices2tmp[13] = v14;
				controlVertices2tmp[14] = v15;
				controlVertices2tmp[15] = v16;

				//projection of all vertices on the mesh surface
				projections = projectPointsOntoSurface2(controlVertices2tmp, cutCenter, 16, meshTriangles, meshTrianglesCount, meshVertices);

				controlVertices2 = (MMSSVector3d*)malloc(projections.size() * sizeof(MMSSVector3d)); //pozice vrcholu po projekci na nejblizsi trojuhelniky
				trianglesWithControlVertices2 = (GLuint*)malloc(projections.size() * sizeof(GLuint)); //indexy trojuhelniku s projekci vrcholu

				for(int j = 0; j < projections.size(); j++)
				{
					MMSSVector3d v = {projections[j].x, projections[j].y, projections[j].z};
					controlVertices2[j] = v;
					trianglesWithControlVertices2[j] = (GLuint)projections[j].w;
				}
				MMSSVector3d* cut = cutMuscleTip(trianglesWithControlVertices2, controlVertices2, projections.size(), edgeTriTable, down, numOfEdges);

				//if the cutting plane not intersects the Red or Blue area
				//use the cut
				bool useIt = true;
				for(int p = 0; p < numOfEdges; p++)
				{
					if((forward * (redCut[p] - cutCenter)) <= 0) useIt = false;
					if((forward * (blueCut[p] - cutCenter)) > 0) useIt = false;
				}

				if(useIt)
				{
					cuts.push_back(cut);
					usedSlices++;
				}
			}

			//list of slices ends by blueCut source area
			cuts.push_back(blueCut);
			usedSlices++;

			Resolution = usedSlices - 1;
			layers = Resolution + 1;  //number of template layers

#pragma endregion slices between insertion and source area
//mafLogMessage("Muscle mesh slicing finished after %d slices.", usedSlices);
			
#pragma region Corrections of fibres
			//rotate cuts to make the fibers as short, as possible
			int *rotatedInd = (int *)malloc(numOfEdges * sizeof(int));
			MMSSVector3d *rotatedCut = (MMSSVector3d *)malloc(numOfEdges * sizeof(MMSSVector3d));
			MMSSVector3d *origCut = (MMSSVector3d *)malloc(numOfEdges * sizeof(MMSSVector3d));

			//calculate rotated indices
			float minDistA = FLT_MAX;
			float minDistB = FLT_MAX;
			int minInd = 0;

			//for each rotation
			for(int j = 0; j < numOfEdges; j++)
			{
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedInd[i] = i + j;
					if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
				}

				//rotate the cut
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedCut[i] = cuts[0][rotatedInd[i]];
				}

				//test distance (length) sum for each rotation
				float dist = 0;
				for(int i = 0; i < numOfEdges; i++)
				{
					dist += MMSSVector3d::VectorLength(cuts[1][i] - rotatedCut[i]);
				}

				//store the shortest distance rotation-index
				if(dist < minDistA)
				{
					minDistA = dist;
					minInd = j;
				}
			}

			//apply the rotation
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedInd[i] = i + minInd;
				if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
			}

			//rotate the cut
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedCut[i] = cuts[0][rotatedInd[i]];
			}

			//apply the rotated cut
			for(int i = 0; i < numOfEdges; i++)
			{
				cuts[0][i] = rotatedCut[i];
			}

			for(int i = 0; i < numOfEdges; i++)
			{
				origCut[i] = cuts[0][i];
			}


			//flip the cut
			for(int i = 0; i < numOfEdges; i++)rotatedCut[i] = cuts[0][numOfEdges - 1 - i];
			for(int i = 0; i < numOfEdges; i++)cuts[0][i] = rotatedCut[i];


			//for each rotation
			for(int j = 0; j < numOfEdges; j++)
			{
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedInd[i] = i + j;
					if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
				}

				//rotate the cut
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedCut[i] = cuts[0][rotatedInd[i]];
				}

				//test distance (length) sum for each rotation
				float dist = 0;
				for(int i = 0; i < numOfEdges; i++)
				{
					dist += MMSSVector3d::VectorLength(cuts[1][i] - rotatedCut[i]);
				}

				//store the shortest distance rotation-index
				if(dist < minDistB)
				{
					minDistB = dist;
					minInd = j;
				}
			}

			//apply the rotation
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedInd[i] = i + minInd;
				if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
			}

			//rotate the cut
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedCut[i] = cuts[0][rotatedInd[i]];
			}

			//apply the rotated cut
			for(int i = 0; i < numOfEdges; i++)
			{
				cuts[0][i] = rotatedCut[i];
			}

			if(minDistB > minDistA)
			{
				for(int i = 0; i < numOfEdges; i++)cuts[0][i] = origCut[i];
			}

			minDistA = FLT_MAX;
			minDistB = FLT_MAX;

			//for each rotation
			for(int j = 0; j < numOfEdges; j++)
			{
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedInd[i] = i + j;
					if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
				}

				//rotate the cut
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedCut[i] = cuts[cuts.size() - 1][rotatedInd[i]];
				}

				//test distance (length) sum for each rotation
				float dist = 0;
				for(int i = 0; i < numOfEdges; i++)
				{
					dist += MMSSVector3d::VectorLength(cuts[cuts.size() - 2][i] - rotatedCut[i]);
				}

				//store the shortest distance rotation-index
				if(dist < minDistA)
				{
					minDistA = dist;
					minInd = j;
				}
			}

			//apply the rotation
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedInd[i] = i + minInd;
				if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
			}

			//rotate the cut
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedCut[i] = cuts[cuts.size() - 1][rotatedInd[i]];
			}

			//apply the rotated cut
			for(int i = 0; i < numOfEdges; i++)
			{
				cuts[cuts.size() - 1][i] = rotatedCut[i];
			}

			for(int i = 0; i < numOfEdges; i++)
			{
				origCut[i] = cuts[cuts.size() - 1][i];
			}


			//flip the cut
			for(int i = 0; i < numOfEdges; i++)rotatedCut[i] = cuts[cuts.size() - 1][numOfEdges - 1 - i];
			for(int i = 0; i < numOfEdges; i++)cuts[cuts.size() - 1][i] = rotatedCut[i];

			//for each rotation
			for(int j = 0; j < numOfEdges; j++)
			{
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedInd[i] = i + j;
					if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
				}

				//rotate the cut
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedCut[i] = cuts[cuts.size() - 1][rotatedInd[i]];
				}

				//test distance (length) sum for each rotation
				float dist = 0;
				for(int i = 0; i < numOfEdges; i++)
				{
					dist += MMSSVector3d::VectorLength(cuts[cuts.size() - 2][i] - rotatedCut[i]);
				}

				//store the shortest distance rotation-index
				if(dist < minDistB)
				{
					minDistB = dist;
					minInd = j;
				}
			}

			//apply the rotation
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedInd[i] = i + minInd;
				if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
			}

			//rotate the cut
			for(int i = 0; i < numOfEdges; i++)
			{
				rotatedCut[i] = cuts[cuts.size() - 1][rotatedInd[i]];
			}

			//apply the rotated cut
			for(int i = 0; i < numOfEdges; i++)
			{
				cuts[cuts.size() - 1][i] = rotatedCut[i];
			}

			if(minDistB > minDistA)
			{
				for(int i = 0; i < numOfEdges; i++)cuts[cuts.size() - 1][i] = origCut[i];
			}

			//for each slice
			float minDist = 999999;
			for(int k = 1; k < cuts.size(); k++)
			{
				//for each rotation
				for(int j = 0; j < numOfEdges; j++)
				{
					for(int i = 0; i < numOfEdges; i++)
					{
						rotatedInd[i] = i + j;
						if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
					}

					//rotate the cut
					for(int i = 0; i < numOfEdges; i++)
					{
						rotatedCut[i] = cuts[k][rotatedInd[i]];
					}

					//test distance (length) sum for each rotation
					float dist = 0;
					for(int i = 0; i < numOfEdges; i++)
					{
						dist += MMSSVector3d::VectorLength(cuts[k-1][i] - rotatedCut[i]);
					}

					//store the shortest distance rotation-index
					if(dist < minDist)
					{
						minDist = dist;
						minInd = j;
					}
				}

				//apply the rotation
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedInd[i] = i + minInd;
					if(rotatedInd[i] >= numOfEdges) rotatedInd[i] -= numOfEdges;
				}

				//rotate the cut
				for(int i = 0; i < numOfEdges; i++)
				{
					rotatedCut[i] = cuts[k][rotatedInd[i]];
				}

				//apply the rotated cut
				for(int i = 0; i < numOfEdges; i++)
				{
					cuts[k][i] = rotatedCut[i];
				}
			}



			/*
			MMSSVector3d centr = {0, 0, 0};
			int count = 0;
			for(int i = 0; i < cuts.size(); i++)
			{
			for(int j = 0; j < numOfEdges; j++)
			{
			centr = centr + cuts[i][j];
			count++;
			}
			}

			centr = (1.0f / (float)count) * centr;

			//if there is a sharp edge on the fibre, calculate new cut
			int modifiedCuts = 0;
			for(int repeat = 0; repeat < 10; repeat++)
			{
			mafLogMessage("repeat %d", repeat);
			if(modifiedCuts > 50) break;

			//for each cut
			for(int i = 0; (i + 2) < cuts.size(); i++)
			{
			MMSSVector3d *lastDirections = (MMSSVector3d *)malloc(numOfEdges * sizeof(MMSSVector3d));
			MMSSVector3d *nextDirections = (MMSSVector3d *)malloc(numOfEdges * sizeof(MMSSVector3d));

			for(int j = 0; j < numOfEdges; j++)
			{
			lastDirections[j] = MMSSVector3d::Normalize(cuts[i + 1][j] - cuts[i][j]);
			}

			for(int j = 0; j < numOfEdges; j++)
			{
			nextDirections[j] = MMSSVector3d::Normalize(cuts[i + 2][j] - cuts[i + 1][j]);
			}

			//looking for the sharpest angle on the fibre
			float angle = 1;
			for(int j = 0; j < numOfEdges; j++)
			{
			angle = min(angle, lastDirections[j] * nextDirections[j]);
			}

			//if the sharp angle is found
			//calculate new cut
			if(angle <= 0.5f)
			{
			MMSSVector3d *avgPositions = (MMSSVector3d *)malloc(numOfEdges * sizeof(MMSSVector3d));
			MMSSVector3d centroid = {0, 0, 0};
			//new position is the average of its neighbours
			for(int j = 0; j < numOfEdges; j++)
			{
			avgPositions[j] = 0.5f * (cuts[i][j] + cuts[i + 2][j]);
			centroid = centroid + avgPositions[j];
			}

			centroid = (1.0f / (float)numOfEdges) * centroid;

			//projection of all vertices on the mesh surface
			projections = projectPointsOntoSurface2(avgPositions, centroid, numOfEdges, meshTriangles, meshTrianglesCount, meshVertices);

			if(projections.size() < numOfEdges) continue;

			//pozice vrcholu po projekci na nejblizsi trojuhelniky
			MMSSVector3d* cut = (MMSSVector3d*)malloc(projections.size() * sizeof(MMSSVector3d));

			bool goodForUse = true;

			for(int j = 0; j < projections.size(); j++)
			{
			MMSSVector3d v = {projections[j].x, projections[j].y, projections[j].z};

			//its out of the bounding sphere!! sth. is wrong, dont use this cut
			//probably the centroid is out of the triangle mesh

			if(MMSSVector3d::VectorLength(v - centr) > 9999)
			{
			goodForUse = false;
			break;
			}

			cut[j] = v;
			}

			if(goodForUse)
			{
			cuts[i + 1] = cut;
			modifiedCuts++;
			}
			}
			}
			}
			*/
#pragma endregion
			
#pragma region Outer slices (red&blue cuts)

			vector<MMSSVector3d *> redCuts;
			for(int i = 0; i < (fibresInAxis-3)/2; i++)
			{
				float planePos = (float)(i+1) * (1.0/(float)((fibresInAxis-1)/2)); // 1/2;1/3,2/3;...
				int numOfEdges2 = 2*(fibresInAxis-2)+2*(fibresInAxis-2-2) - i * 8; //number of edges of the cut

				//connect each point of the cut with related farPoint and use it for interpolation of the position of the next cut
				MMSSVector3d* cutPoints = (MMSSVector3d*)malloc(numOfEdges * sizeof(MMSSVector3d));
				for(int j = 0; j < numOfEdges; j++)
				{
					cutPoints[numOfEdges - 1 - j] = cuts[0][j] + (planePos * (redFarPoint - cuts[0][j]));
				}

				//we get more points than we need -> reduce (remove point forming corners of the square-mesh-model)
				MMSSVector3d* ctrlVertices = (MMSSVector3d*)malloc((numOfEdges - (i+1)*8) * sizeof(MMSSVector3d));
				int index = 0;
				for(int j = 0; j< numOfEdges; j++)
				{
					if(j%(fibresInAxis-1) == 0 //corners
						|| (j%(fibresInAxis-1) > 0 + (i+1) && j%(fibresInAxis-1) < (fibresInAxis-1) - (i+1)))
					{
						ctrlVertices[index] = cutPoints[j];
						index++;
					}
				}

				//get the cut on the surface
				projections = projectPointsOntoSurface(ctrlVertices, (numOfEdges - (i+1)*8), meshTriangles, meshTrianglesCount, meshVertices, indices, 6);
				MMSSVector3d* controlVertices2 = (MMSSVector3d*)malloc((projections.size()) * sizeof(MMSSVector3d));

				for(int i = 0; i < projections.size(); i++)
				{
					//if intersection found
					if(projections[i].w != -1)
					{
						MMSSVector3d v = {projections[i].x, projections[i].y, projections[i].z};
						controlVertices2[i] = v;
					}
					else //else find the closest point on the surface
					{
						MMSSVector3d* point = (MMSSVector3d*)malloc(sizeof(MMSSVector3d));
						point[0] = ctrlVertices[i];
						vector<Vector4D> oneProjected  = projectPointsOntoSurface(point, 1, meshTriangles, meshTrianglesCount, meshVertices, indices, 6);
						MMSSVector3d w = {oneProjected[0].x, oneProjected[0].y, oneProjected[0].z};
						controlVertices2[i] = w;
						free(point);
					}
				}

				redCuts.push_back(controlVertices2);
			}

			vector<MMSSVector3d*> blueCuts;
			for(int i = 0; i < (fibresInAxis-3)/2; i++)
			{
				float planePos = (float)(i+1) * (1.0/(float)((fibresInAxis-1)/2)); // 1/2;1/3,2/3;...
				int numOfEdges2 = 2*(fibresInAxis-2)+2*(fibresInAxis-2-2) - i * 8; //number of edges of the cut

				//connect each point of the cut with related farPoint and use it for interpolation of the position of the next cut
				MMSSVector3d* cutPoints = (MMSSVector3d*)malloc(numOfEdges * sizeof(MMSSVector3d));
				for(int j = 0; j < numOfEdges; j++)
				{
					cutPoints[numOfEdges - 1 - j] = cuts[cuts.size()-1][j] + (planePos * (blueFarPoint - cuts[cuts.size()-1][j]));
				}

				//we get more point than we need -> reduce (remove point forming corners of the square-mesh-model)
				MMSSVector3d* ctrlVertices = (MMSSVector3d*)malloc((numOfEdges - (i+1)*8) * sizeof(MMSSVector3d));
				int index = 0;
				for(int j = 0; j< numOfEdges; j++)
				{
					if(j%(fibresInAxis-1) == 0 //corners
						|| (j%(fibresInAxis-1) > 0 + (i+1) && j%(fibresInAxis-1) < (fibresInAxis-1) - (i+1)))
					{
						ctrlVertices[index] = cutPoints[j];
						index++;
					}
				}

				//get the cut on the surface
				projections = projectPointsOntoSurface(ctrlVertices, (numOfEdges - (i+1)*8), meshTriangles, meshTrianglesCount, meshVertices, indices, 13);
				MMSSVector3d* controlVertices2 = (MMSSVector3d*)malloc((projections.size()) * sizeof(MMSSVector3d)); //pozice vrcholu po projekci na nejblizsi trojuhelniky

				for(int i = 0; i < projections.size(); i++)
				{
					//if intersection found
					if(projections[i].w != -1)
					{
						MMSSVector3d v = {projections[i].x, projections[i].y, projections[i].z};
						controlVertices2[i] = v;
					}
					else //else find the closest point on the surface
					{
						MMSSVector3d* point = (MMSSVector3d*)malloc(sizeof(MMSSVector3d));
						point[0] = ctrlVertices[i];
						vector<Vector4D> oneProjected  = projectPointsOntoSurface(point, 1, meshTriangles, meshTrianglesCount, meshVertices, indices, 13);
						MMSSVector3d w = {oneProjected[0].x, oneProjected[0].y, oneProjected[0].z};
						controlVertices2[i] = w;
						free(point);
					}
				}

				blueCuts.push_back(controlVertices2);
			}

#pragma endregion slices of the source/insertion areas
			
#pragma region Template cube construction
			/////////////////////////////////////////////////////////////////////
			// Creates the template cube (1x1x1) for the desired spring system //
			/////////////////////////////////////////////////////////////////////

			//number of vertices used to form the spring system template
			int templateVerticesCount = (Resolution + 1) * NumberOfFibres;

			//cout of indices of the fixed template vertices
			int templateFixedCount = 2 * NumberOfFibres + (Resolution - 1) * 4 * (fibresInAxis - 1);

			//indices of the fixed template vertices
			unsigned int* templateFixedIndices = new unsigned int[templateFixedCount];			

			//center of template
			MMSSVector3d templateCenter = {0, 0, 0};
			int indexInFixed = 0;
			int templateSurfaceVerticesCount;

			int nIndex = 0;
			double delta_t = 1.0 / Resolution;      //step in t
			double delta_s = 1.0 / (fibresInAxis-1);//step in s
			double delta_r = 1.0 / (fibresInAxis-1);//step in r
			double s = 0;
			double r = 0;

			MMSSVector3d* templateVertices = new MMSSVector3d[templateVerticesCount];

			//calculate vertex positions of the SS template
			for (int i = 0; i < fibresInAxis; i++)
			{
				r = 0;
				for (int j = 0; j < fibresInAxis; j++)
				{
					for (int k = 0; k <= Resolution; k++, nIndex++)
					{
						double x[3];
						FibersTemplate->GetPoint(r, s, k*delta_t, x);

						//fibre templates use different coordinate system (they use left-handed)
						//whilst the rest of our application uses right-handed
						templateVertices[nIndex].x = (GLfloat)x[0];
						templateVertices[nIndex].y = (GLfloat)x[2];
						templateVertices[nIndex].z = (GLfloat)x[1];

						if(i == 0 || i == (fibresInAxis-1) || j == 0 || j == (fibresInAxis-1) || k == 0 || k == Resolution)
						{
							templateFixedIndices[indexInFixed] = nIndex;							
							indexInFixed++;
						}

						templateCenter = templateCenter + templateVertices[nIndex];
					}
					r += delta_r;
				}
				s += delta_s;
			}

			//template mesh center
			templateCenter = (1.0f/templateVerticesCount) * templateCenter;

			//number of vertices in the template
			templateSurfaceVerticesCount = indexInFixed;

#pragma endregion
			
#pragma region Create spring system
			//just transformation of point x (onto target cube)
			//system is 4x smaller, to cause initial tensions
			for (int i = 0; i < templateVerticesCount; i++)
			{
				double x[3];
				for (int k = 0; k < 3; k++){
					x[k] = lf.O[k] + 0.25f * lf.uvw[0][k]*templateVertices[i].x + lf.uvw[0][k]*(0.384f)
						+ 0.25f * lf.uvw[1][k]*templateVertices[i].y + lf.uvw[1][k]*(0.384f)
						+ 0.25f * lf.uvw[2][k]*templateVertices[i].z + lf.uvw[2][k]*(0.384f);
				}

				for (int k = 0; k < 3; k++){
					templateVertices[i].x = x[0];
					templateVertices[i].y = x[1];
					templateVertices[i].z = x[2];
				}
			}//end for i (points)

			//count springs count as sum of valence of all point
			int springsCount = (fibresInAxis-2)*(fibresInAxis-2)*(Resolution-1)*6//inner points springs count: fibersIn axis - 2 means fibers count without two fibers in bound, Resolution-1 is the same but in t-parameter,6 is valence of inside point
				+ 2*(fibresInAxis-2)*(fibresInAxis-2)*5//count of points in inside of origin and insertion planes, 5 is valence
				+ 4*(fibresInAxis-2)*(Resolution-1)*5//count of points in inside of other planes(without origin and insertion), 5 is valence
				+ 8*(fibresInAxis-2)*4//8 edges around insertion and origin areas without corners, 4 is valence
				+ 4*(Resolution-1)*4//4 edges between insertion and origin areas without ends, 4 is valence
				+ 8*3;//8 corners of template, 3 is valence

			//springs count is half of sum of valences
			springsCount = springsCount/2;

			//generate springs
			IMassSpringSystem::Spring* springs = new IMassSpringSystem::Spring[springsCount];

			//first generate springs on the surface (points in border planes of template)
			int indexInSprings = 0;
			int actIndex = 0;
			int meshSpringsCount;
			double averageStiffness = 0;

			for(int i = 0; i < fibresInAxis; i++)
			{
				for(int j = 0; j < fibresInAxis; j++)
				{
					for(int k = 0; k <= Resolution; k++)
					{
						//add spring to point up of actual (next in t)
						if((i==0 || i==fibresInAxis-1 || j==0 || j==fibresInAxis-1) && k<Resolution)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + 1 + j*(Resolution+1) + i*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}
						//add spring to point front of actual (next in s)
						if((k==0 || k==Resolution || i==0 || i==fibresInAxis-1) && j<fibresInAxis-1)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + (j+1)*(Resolution+1) + i*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}
						//add spring to point right of actual (next in r)
						if((k==0 || k==Resolution || j==0 || j==fibresInAxis-1) && i<fibresInAxis-1)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + j*(Resolution+1) + (i+1)*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}

						actIndex++;
					}
				}
			}

			meshSpringsCount = indexInSprings;

			//generate springs which at least one end is inner point
			actIndex = 0;
			for(int i = 0; i < fibresInAxis; i++)
			{
				for(int j = 0; j < fibresInAxis; j++)
				{
					for(int k = 0; k <= Resolution; k++)
					{
						//add spring to point up of actual (next in t)
						if((i>0 && i<fibresInAxis-1 && j>0 && j<fibresInAxis-1) && k<Resolution)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + 1 + j*(Resolution+1) + i*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}
						//add spring to point front of actual (next in s)
						if((k>0 && k<Resolution && i>0 && i<fibresInAxis-1) && j<fibresInAxis-1)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + (j+1)*(Resolution+1) + i*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}
						//add spring to point right of actual (next in r)
						if((k>0 && k<Resolution && j>0 && j<fibresInAxis-1) && i<fibresInAxis-1)
						{
							springs[indexInSprings].p1_index = actIndex;
							springs[indexInSprings].p2_index = k + j*(Resolution+1) + (i+1)*fibresInAxis*(Resolution+1);

							springs[indexInSprings].initLen = MMSSVector3d::VectorLength(templateVertices[springs[indexInSprings].p1_index],templateVertices[springs[indexInSprings].p2_index]);
							springs[indexInSprings].stiffness = 1/springs[indexInSprings].initLen;
							averageStiffness += springs[indexInSprings].stiffness;

							indexInSprings++;
						}

						actIndex++;
					}
				}
			}
			//springsCount = indexInSprings;
			averageStiffness = averageStiffness / springsCount;

#pragma endregion originally by I. Zelený, edited by J. Rus
			
#pragma region Mapping of slices for the final output

			//generate final positions of template vertices (just original template position now)
			MMSSVector3d* targetPositions = new MMSSVector3d[templateVerticesCount];
			for(int i = 0; i < templateVerticesCount; i++)
			{
				targetPositions[i].x = templateVertices[i].x;
				targetPositions[i].y = templateVertices[i].y;
				targetPositions[i].z = templateVertices[i].z;
			}

			int contourLen = 2 * fibresInAxis + 2 * (fibresInAxis-2);
			int *contourIndices = (int *)malloc(contourLen * sizeof(int));
			int contourIndex = 0;

			for(int i = 0; i < fibresInAxis; i++)
			{
				contourIndices[contourIndex] = i;
				contourIndex++;
			}

			for(int i = 0; i < fibresInAxis - 2; i++)
			{
				contourIndices[contourIndex] = (2 * fibresInAxis) - 1 + i * fibresInAxis;
				contourIndex++;
			}

			for(int i = 0; i < fibresInAxis; i++)
			{
				contourIndices[contourIndex] = (fibresInAxis * fibresInAxis) - 1 - i;
				contourIndex++;
			}

			for(int i = 0; i < fibresInAxis - 2; i++)
			{
				contourIndices[contourIndex] = (fibresInAxis * (fibresInAxis-2)) - i * fibresInAxis;
				contourIndex++;
			}

			for(int pom = 0; pom <= Resolution; pom++)
			{
				for(int i = 0; i < contourLen; i++)
				{
					targetPositions[contourIndices[i] * layers + pom] = cuts[pom][contourLen-1-i];
				}
			}

			//map the caps (blue and red cuts)
			int tipIndex = ((fibresInAxis * fibresInAxis) - 1) / 2 * layers;
			targetPositions[tipIndex] = redFarPoint;
			targetPositions[tipIndex + Resolution] = blueFarPoint;

			int cutLength = 4 * fibresInAxis - 4 - 8;
			int sideLength = fibresInAxis - 2;
			int startIndex = (fibresInAxis + 1) * layers;
			int endIndex = (fibresInAxis * fibresInAxis - 1) * layers - (fibresInAxis + 1) * layers;
			int cutIndex = 0;
			int inCutIndex = 0;

			while(startIndex < tipIndex)
			{
				for(int i = 0; i < sideLength; i++)
				{
					targetPositions[startIndex + i * layers] = redCuts[cutIndex][inCutIndex];
					targetPositions[startIndex + i * layers + Resolution] = blueCuts[cutIndex][inCutIndex];
					inCutIndex++;
				}

				for(int i = 0; i < sideLength-2; i++)
				{
					targetPositions[startIndex + ((sideLength-1) + (i+1) * fibresInAxis) * layers] = redCuts[cutIndex][inCutIndex];
					targetPositions[startIndex + ((sideLength-1) + (i+1) * fibresInAxis) * layers + Resolution] = blueCuts[cutIndex][inCutIndex];
					inCutIndex++;
				}

				for(int i = 0; i < sideLength; i++)
				{
					targetPositions[endIndex - i * layers] = redCuts[cutIndex][inCutIndex];
					targetPositions[endIndex - i * layers + Resolution] = blueCuts[cutIndex][inCutIndex];
					inCutIndex++;
				}

				for(int i = 0; i < sideLength-2; i++)
				{
					targetPositions[endIndex - ((sideLength-1) + (i+1) * fibresInAxis) * layers] = redCuts[cutIndex][inCutIndex];
					targetPositions[endIndex - ((sideLength-1) + (i+1) * fibresInAxis) * layers + Resolution] = blueCuts[cutIndex][inCutIndex];
					inCutIndex++;
				}

				startIndex += (fibresInAxis + 1) * layers;
				endIndex -= (fibresInAxis + 1) * layers;
				sideLength -=2;
				cutLength -= 8;
				cutIndex++;
				inCutIndex = 0;
			}

#pragma endregion
			
#pragma region MMSS (spring system calculations)

			//create the spring system (MMSS)
			IMassSpringSystem::Counts c;
			c.verticesCount = templateVerticesCount;
			c.springsCount = springsCount;

			//critical damping
			float weight = (float)sqrt(1.0f / dblAxisLength);

			float k6 = 2 + 4.0f / averageStiffness;
			float critDamping = (float)sqrt(4 * k6 * weight) * 0.15f;
			//critDamping = 6 * weight;

			//mafLogMessage("Creating and starting MSS, particle weight: %f, spring damping: %f.", weight, critDamping);

			IMassSpringSystem* mss = new MassSpringSystemGPU(templateVertices, springs, c);
			mss->SetCoefs(critDamping, weight);

			//move the vertices with fixed positions to the right position and create a tension
			mss->SetFixedPoints(templateFixedIndices,targetPositions,templateFixedCount);			
			
			//run the system for X iterations
			float diagLen = MMSSVector3d::VectorLength(blueFarPoint, redFarPoint);
			float getTime = 0;
			int getIteration = 0;
			int getIterationAfter = 0;
			float maxMove = 0;

			MMSSVector3d* prevPositions = new MMSSVector3d[templateVerticesCount];
			MMSSVector3d* positions = new MMSSVector3d[templateVerticesCount];

			mss->NextStep(6);

			do
			{
				mss->GetTimeIteration(&getIteration,&getTime);
				mss->NextStep(2);
				mss->GetTimeIteration(&getIterationAfter,&getTime);

				for(int i = 0; i < templateVerticesCount; i++)
				{
					prevPositions[i] = positions[i];
				}

				mss->GetVertices(positions, templateVerticesCount);
		
				maxMove = 0;
				float actMove;
				for(int i = 0; i < templateVerticesCount; i++)
				{
					actMove = MMSSVector3d::VectorLength(prevPositions[i], positions[i]);
					if(actMove > maxMove)maxMove = actMove;
				}
				
				if(maxMove < (diagLen * 0.001f))
				{
					break;
				}

			//while changing and itaration count is < 10k
			}while(getIterationAfter > getIteration && getIterationAfter < 10000);

			//mafLogMessage("System finished after %f seconds.", getTime);
		

			//request the result
			mss->GetVertices(templateVertices, templateVerticesCount);

#pragma endregion
			
			//contain all vertices of template
			//convert into the MAF format
			VCoord* pFVerts = new VCoord[templateVerticesCount];
			for(int index = 0; index < templateVerticesCount; index++)
			{
				pFVerts[index][0] = templateVertices[index].x;
				pFVerts[index][1] = templateVertices[index].y;
				pFVerts[index][2] = templateVertices[index].z;
			}

			delete templateVertices;
			delete meshVertices;
			delete meshTriangles;
			delete mss;
			delete springs;
			delete templateFixedIndices;			

#pragma region Saving the results
			//save the result
			vtkPoints* pPoints = vtkPoints::New();
			pPoints->SetNumberOfPoints(templateVerticesCount);

			vtkCellArray* pCells = vtkCellArray::New();
			vtkIdType* pIds = new vtkIdType[Resolution + 1];

			nIndex = 0;
			for (int i = 0; i < NumberOfFibres; i++)
			{
				if (this->SmoothFibers != 0){
					SmoothFiber(&pFVerts[nIndex], Resolution + 1);
				}

				for (int j = 0; j <= Resolution; j++)
				{
					pPoints->SetPoint(nIndex, pFVerts[nIndex]);
					pIds[j] = nIndex++;
				} //end for j

				pCells->InsertNextCell(Resolution + 1, pIds);
			} //end for i

			delete[] pIds;

			pPoly->SetLines(pCells);
			pPoly->SetPoints(pPoints);

			pCells->Delete();
			pPoints->Delete();

			delete[] pFVerts;
#pragma endregion
} //end: void vtkMAFMuscleDecompositionMMSS::ExecuteData(vtkDataObject *output)

//Routines related to matching template cube and input data bounding box
#pragma region Matching

//------------------------------------------------------------------------
//Computes the principal axis for the given point set.
//N.B. the direction is normalized
void vtkMAFMuscleDecompositionMMSS::ComputeAxisLine(vtkPoints* points,
	double* origin, double* direction)
	//------------------------------------------------------------------------
{
	//the line goes through the centroid
	int N = points->GetNumberOfPoints();
	origin[0] = origin[1] = origin[2] = 0.0;
	for (int i = 0; i < N; i++)
	{
		const double* pcoords = points->GetPoint(i);
		for (int j = 0; j < 3; j++){
			origin[j] += pcoords[j];
		}
	}

	for (int j = 0; j < 3; j++){
		origin[j] /= N;
	}

	//compute the covariance matrix, which is a symmetric matrix
	double A[3][3], eigenvals[3], eigenvects[3][3];

	//fill the matrix
	memset(A, 0, sizeof(A));
	for (int k = 0; k < N; k++)
	{
		const double* pcoords = points->GetPoint(k);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++) {
				A[i][j] += (pcoords[i] - origin[i])*(pcoords[j] - origin[j]);
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
		V[i] = eigenvects[i];
	}

	vtkMath::Jacobi(ATemp, eigenvals, V);

	//copy the result
	//N.B. Jacobi returns vectors in columns!
	for (int i = 0; i < 3; i++) {
		direction[i] = eigenvects[i][0];
	}
}

//------------------------------------------------------------------------
//Computes 4*nFrames vectors by rotating u around r vector. All vectors are
//normalized and stored in the order A, B, C, D where B is the vector
//opposite to A, C is vector perpendicular to A and r and D is vector
//opposite to C. N.B. vectors u and r must be normalized and perpendicular!
//The buffer pVectors must be capable enough to hold all vectors.
void vtkMAFMuscleDecompositionMMSS::ComputeDirectionVectors(
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
void vtkMAFMuscleDecompositionMMSS::FitDirectionVectorsToData(vtkPoints* points,
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
void vtkMAFMuscleDecompositionMMSS::ComputeLFS(double* center, VCoord* w_dir,
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
int vtkMAFMuscleDecompositionMMSS::FindBestMatch(vtkPoints* template_O, vtkPoints* template_I,
	int nLFS, LOCAL_FRAME* pLFS, vtkPoints* target_O, vtkPoints* target_I)
	//------------------------------------------------------------------------
{
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

		if (dblScore < dblBestLFScore)
		{
			dblBestLFScore = dblScore;
			iBestLF = iSys;
		}
	} //end for iSys

	for (int i = 0; i < 4; i++) {
		delete[] pPoints[i];
	}

	return iBestLF;
}

//------------------------------------------------------------------------
//Computes the minimal oriented box that fits the input data so that all
//points are inside of this box (or on its boundary) and the total squared
//distance of template origin points from the input mesh origin points and
//the total squared distance of template insertion points from the input
//mesh origin points are minimized
//------------------------------------------------------------------------
void vtkMAFMuscleDecompositionMMSS::ComputeFittingOB(vtkPoints* points, LOCAL_FRAME& out_lf)
{
	//get the principal axis
	double l_pos[3], l_dir[3];
	ComputeAxisLine(points, l_pos, l_dir);

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

	//create various systems by rotating them around w axis by small angle (5 degrees)
	const int nCubes = 10;       //number of frames to be created in Pi/4 area
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

#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper.h"
#include "vtkTextMapper.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkTubeFilter.h"
#include "vtkCubeSource.h"

#pragma endregion 