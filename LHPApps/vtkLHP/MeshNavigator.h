/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshNavigator.h,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:00:35 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef MeshNavigator_h__
#define MeshNavigator_h__

#pragma once

#include <set>
#include <stack>

//vtk
#include "vtkPolyData.h"
#include "vtkPoints.h"

// my classes
#include "PKUtils.h"
#include "PKMath.h"
#include "OoCylinder.h"
#include "PKHashTable.h"

using namespace std;

#define NAVIGATOR_CELL_TRIANGLE_COUNT 10

// absolute
#define NAVIGATOR_PADDING 0.1

#define NAVIGATOR_AXIS_X 1
#define NAVIGATOR_AXIS_Y 2
#define NAVIGATOR_AXIS_Z 4

typedef struct {
	int x;
	int y;
	int z;

	void FromArray(int coords[3]) {
		this->x = coords[0];
		this->y = coords[1];
		this->z = coords[2];
	}

	void ToArray(int coords[3]) {
		coords[0] = this->x;
		coords[1] = this->y;
		coords[2] = this->z;
	}

} tNavigatorCoords;


class MeshNavigator
{
public:
	MeshNavigator(vtkPolyData *mesh = NULL, PKMatrix *points = NULL, OoCylinder *ooc = NULL);
	~MeshNavigator(void);
			
	void SetUpByMesh(vtkPolyData* mesh, PKMatrix *points, OoCylinder *ooc = NULL);
	int GetTriangleCandidatesForRay(const double start[3], const double dir[3], PKHashTable<vtkIdType, vtkIdType> *triangleIds);

private:
	void CreateBox(OoCylinder *ooc);
	void MapCells();
	
	void DisposeCells();
	void ClearCellsVisited();
	void AllocateCells(int areaCount);

	PKHashTable<vtkIdType, vtkIdType>* GetTrianglesInCell(int xIndex, int yIndex, int zIndex);
	int FindPointCoords(const double point[3], int coords[3]) const;
	int GetCellIndexByCoords(int xIndex, int yIndex, int zIndex, bool checks = true, bool throws = true) const;
	int GetCellIndexByCoords(int coords[3], bool checks = true, bool throws = true) const;
	bool IntersectsTriangleCell(double a[3], double b[3], double c[3], int x, int y, int z);
	bool FindRayEntry(const double start[3], const double dir[3], int coords[3]);
	bool IntersectsRayCell(const double start[3], const double dir[3], int x, int y, int z) const;
	void GetPlaneForCell(int axis, int index, double plane[4]) const;
	bool CheckCell(const double start[3], const double dir[3], const tNavigatorCoords coords, int axis, bool positive) const;

	double xDir[3];
	double yDir[3];
	double zDir[3]; // ooc axis dir

	double planeXY[4];
	double planeYZ[4];
	double planeXZ[4];

	int xCount;
	int yCount;
	int zCount;

	double xStep;
	double yStep;
	double zStep;

	int gridCellCount;
	PKHashTable<vtkIdType, vtkIdType>** cells;
	bool* cellsVisited;

	// shallow copies!!!! DO NOT DISPOSE!!!!!!!!!!
	vtkPolyData *mesh;
	PKMatrix *points;
};

#endif