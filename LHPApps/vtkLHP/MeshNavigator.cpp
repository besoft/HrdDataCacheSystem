/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: MeshNavigator.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:00:35 $ 
  Version: $Revision: 1.1.2.4 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#include "MeshNavigator.h"


MeshNavigator::MeshNavigator(vtkPolyData *mesh, PKMatrix *points, OoCylinder *ooc)
{
	this->xCount = this->yCount = this->zCount = 0;
	this->xStep = this->yStep = this->zStep = 0.0;
	this->gridCellCount = 0;
	this->cells = NULL;
	this->cellsVisited = NULL;

	this->mesh = NULL;
	this->points = NULL;
	
	if (mesh != NULL) {
		this->SetUpByMesh(mesh, points, ooc);
	} else {
		this->DisposeCells();
	}
}


MeshNavigator::~MeshNavigator(void)
{
	this->DisposeCells();
}

int MeshNavigator::GetTriangleCandidatesForRay(const double start[3], const double dir[3], PKHashTable<vtkIdType, vtkIdType> *triangleIds) {
	tNavigatorCoords coords, coordsStart;
	int coordsArray[3];

	triangleIds->Clear();

	if (!this->FindRayEntry(start, dir, coordsArray)) {
		// no intersection AT ALL
		return 0;
	}

	coordsStart.FromArray(coordsArray);
		
	this->ClearCellsVisited();
	stack<tNavigatorCoords> cellsToExpand = stack<tNavigatorCoords>();
	cellsToExpand.push(coordsStart);

	while (true) {
		if (cellsToExpand.empty()) {
			break;
		}

		coordsStart = cellsToExpand.top();
		cellsToExpand.pop();

		// output results
		int index = this->GetCellIndexByCoords(coordsStart.x, coordsStart.y, coordsStart.z, false);
		if (index >= this->gridCellCount) {
			continue; // should not happen
		}

		triangleIds->Union(this->cells[index]);

		// YZ
		coords = coordsStart;
		coords.x += 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_X, true)) {
			cellsToExpand.push(coords);
		}

		coords = coordsStart;
		coords.x -= 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_X, false)) {
			cellsToExpand.push(coords);
		}

		// XZ
		coords = coordsStart;
		coords.y += 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_Y, true)) {
			cellsToExpand.push(coords);
		}

		coords = coordsStart;
		coords.y -= 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_Y, false)) {
			cellsToExpand.push(coords);
		}

		// XY
		coords = coordsStart;
		coords.z += 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_Z, true)) {
			cellsToExpand.push(coords);
		}

		coords = coordsStart;
		coords.z -= 1;
		if (this->CheckCell(start, dir, coords, NAVIGATOR_AXIS_Z, false)) {
			cellsToExpand.push(coords);
		}
		
	}

	return triangleIds->GetCount();
}

inline bool MeshNavigator::CheckCell(const double start[3], const double dir[3], const tNavigatorCoords coords, int axis, bool positive) const {	
	int coordIndex = 0;
	const double *axisDir;
	
	switch (axis) {
		case NAVIGATOR_AXIS_X:
			coordIndex = 0;
			axisDir = this->xDir;
			break;
		case NAVIGATOR_AXIS_Y:
			coordIndex = 1;
			axisDir = this->yDir;
			break;
		case NAVIGATOR_AXIS_Z:
			coordIndex = 2;
			axisDir = this->zDir;
			break;
		default:
			throw "Invalid axis specifier.";
			break;
	}

	bool res = false;
	
	// should I go there?
	int index = this->GetCellIndexByCoords(coords.x, coords.y, coords.z, true, false);

	double axisMatch = PKUtils::Dot(dir, axisDir);
	if (!positive) {
		axisMatch *= -1;
	}

	if (axisMatch > 0 && index >= 0 && !this->cellsVisited[index]) {
			
		// do I go there?
		if (this->IntersectsRayCell(start, dir, coords.x, coords.y, coords.z)) {
			res = true;
		}

		this->cellsVisited[index] = true;
	}

	return res;
}

void MeshNavigator::SetUpByMesh(vtkPolyData* mesh, PKMatrix *points, OoCylinder *ooc) {
	this->mesh = mesh;
	this->points = points;
	
	// first discover position and dimensions of mesh
	this->CreateBox(ooc);
	this->MapCells();
}

void MeshNavigator::MapCells() {
	vtkIdType nCells = this->mesh->GetNumberOfCells();
	vtkGenericCell* cell = vtkGenericCell::New();
	double point[9];
	int coords[3];
	
	this->AllocateCells(this->xCount * this->yCount * this->zCount);

	for (vtkIdType cellId = 0; cellId < nCells; cellId++) {
		mesh->GetCell(cellId, cell);
		
		if (cell->GetNumberOfPoints() < 3) {
			continue;
		}

		int minX, minY, minZ, maxX, maxY, maxZ;
		minX = minY = minZ = 99999;
		maxX = maxY = maxZ = -1;

		// find possible quadrant
		for (vtkIdType j = 0; j < 3; j++) {
			myGetPointDeep(mesh, points, cell->GetPointId(j), point + 3 * j);
			int index = this->FindPointCoords(point + 3 * j, coords);
			if (index < 0) {
				continue;
			}

			minX = min(minX, coords[0]);
			minY = min(minY, coords[1]);
			minZ = min(minZ, coords[2]);

			maxX = max(maxX, coords[0]);
			maxY = max(maxY, coords[1]);
			maxZ = max(maxZ, coords[2]);
		}	

		if (maxX < 0 || maxY < 0 || maxZ < 0) {
			continue;
		}

		// check cells in quadrant
		for (int x = minX; x <= maxX; x++) {
			for (int y = minY; y <= maxY; y++) {
				for (int z = minZ; z <= maxZ; z++) {

					if (!this->IntersectsTriangleCell(point, point + 3, point + 6, x, y, z)) {
						continue;
					}

					int index = this->GetCellIndexByCoords(x, y, z, false);

					if (index < 0) {
						continue;
					}

					this->cells[index]->Add(cellId, cellId);
				}
			}
		}

		
	}

	cell->Delete();
}

void MeshNavigator::CreateBox(OoCylinder *ooc) {
	bool disposeOoc = false;

	if (ooc == NULL) {
		ooc = new OoCylinder(this->mesh, this->points);
		disposeOoc = true;
	}
		
	// get axis
	ooc->GetDirection(this->zDir);
	if (abs(this->zDir[1]) < 0.8) {
		double yAxis[] = {0, 1, 0};
		vtkMath::Cross(this->zDir, yAxis, this->xDir);
		vtkMath::Cross(this->zDir, this->xDir, this->yDir);
	} else {
		double xAxis[] = {1, 0, 0};
		vtkMath::Cross(this->zDir, xAxis, this->yDir);
		vtkMath::Cross(this->zDir, this->yDir, this->xDir);
	}

	PKUtils::NormalizeVertex(this->xDir);
	PKUtils::NormalizeVertex(this->yDir);

	// planes
	double start[3];
	ooc->GetStart(start);
	
	PKUtils::CopyVertex(this->zDir, planeXY);
	PKUtils::CopyVertex(this->yDir, planeXZ);
	PKUtils::CopyVertex(this->xDir, planeYZ);
	this->planeXY[3] = - PKUtils::Dot(this->planeXY, start);
	this->planeXZ[3] = - PKUtils::Dot(this->planeXZ, start);
	this->planeYZ[3] = - PKUtils::Dot(this->planeYZ, start);
	
	// discover x and y
	double maxX = 0, minX = 0;
	double maxY = 0, minY = 0;
	//double maxZ = 0, minZ = 0;

	double point[4];
	point[3] = 1;
	int nPoints = this->mesh->GetNumberOfPoints();
	for (vtkIdType i = 0; i < nPoints; i++) {
		myGetPointDeep(this->mesh, this->points, i, point);

		double x = PKUtils::Dot4(this->planeYZ, point);
		double y = PKUtils::Dot4(this->planeXZ, point);
		//double z = PKUtils::Dot4(this->planeXY, point);

		minX = min(minX, x);
		maxX = max(maxX, x);

		minY = min(minY, y);
		maxY = max(maxY, y);

		/*minZ = min(minZ, z);
		maxZ = max(maxZ, z);*/
	}

	// padding
	minX -= NAVIGATOR_PADDING;
	maxX += NAVIGATOR_PADDING;
	minY -= NAVIGATOR_PADDING;
	maxY += NAVIGATOR_PADDING;

	// shift planes
	this->planeYZ[3] -= minX;
	this->planeXZ[3] -= minY;
	this->planeXY[3] += NAVIGATOR_PADDING;

	// calculate cell divisions
	double xSize = maxX - minX;
	double ySize = maxY - minY;
	double zSize = ooc->GetLength() + 2 * NAVIGATOR_PADDING;

	// try to keep desired average number of triangles in cells and rectangular shape
	double zCount = pow(nPoints * zSize * zSize / (NAVIGATOR_CELL_TRIANGLE_COUNT * xSize * ySize), 1.0 / 3.0); 

	this->xCount = (int) ceil(zCount * xSize / zSize);
	this->yCount = (int) ceil(zCount * ySize / zSize);
	this->zCount = (int) ceil(zCount);

	this->xStep = xSize / this->xCount;
	this->yStep = ySize / this->yCount;
	this->zStep = zSize / this->zCount;

	if (disposeOoc) {
		delete ooc;
	}	
}

void MeshNavigator::AllocateCells(int gridCellCount) {
	if (gridCellCount < 0) {
		throw "Number of cells must be >= 0";
	}

	this->DisposeCells();

	if (gridCellCount == 0) {
		return;
	}

	PKHashTable<vtkIdType, vtkIdType>** cells = new PKHashTable<vtkIdType, vtkIdType>*[gridCellCount];
	this->cells = cells;
	this->gridCellCount = gridCellCount;
	
	for (int i = 0; i < gridCellCount; i++, cells++) {
		(*cells) = new PKHashTable<vtkIdType, vtkIdType>();
	}

	this->cellsVisited = new bool[gridCellCount];
}

void MeshNavigator::ClearCellsVisited() {
	if (this->cellsVisited == NULL) {
		return;
	}

	memset(this->cellsVisited, 0, sizeof(bool) * this->gridCellCount);
}

void MeshNavigator::DisposeCells() {
	if (this->cells == NULL) {
		return;
	}
	
	PKHashTable<vtkIdType, vtkIdType>** cells = this->cells;
	for (int i = 0; i < this->gridCellCount; i++, cells++) {
		if ((*cells) != NULL) {
			delete (*cells);
			*cells = NULL;
		}		
	}

	delete[] this->cells;
	this->cells = NULL;
	this->gridCellCount = 0;
	delete[] this->cellsVisited;
	this->cellsVisited = NULL;
}

inline int MeshNavigator::GetCellIndexByCoords(int coords[3], bool checks, bool throws) const {
	return this->GetCellIndexByCoords(coords[0], coords[1], coords[2], checks, throws);
}

inline int MeshNavigator::GetCellIndexByCoords(int xIndex, int yIndex, int zIndex, bool checks, bool throws) const {
	if (checks) {
		if (xIndex < 0 || xIndex >= this->xCount) {
			if (throws) {
				throw "Invalid x index.";
			} else {
				return -1;
			}
		}

		if (yIndex < 0 || yIndex >= this->yCount) {
			if (throws) {
				throw "Invalid y index.";
			} else {
				return -1;
			}
		}

		if (zIndex < 0 || zIndex >= this->zCount) {
			if (throws) {
				throw "Invalid z index.";
			} else {
				return -1;
			}
		}
	}

	int coords = xIndex * this->yCount * this->zCount + yIndex * this->zCount + zIndex;

	if (checks) {
		if (coords >= this->gridCellCount) {
			if (throws) {
				throw "Invalid coords!";
			} else {
				return -1;
			}
		}
	}

	return coords;
}

inline PKHashTable<vtkIdType, vtkIdType>* MeshNavigator::GetTrianglesInCell(int xIndex, int yIndex, int zIndex) {
	return this->cells[this->GetCellIndexByCoords(xIndex, yIndex, zIndex)];
}

inline int MeshNavigator::FindPointCoords(const double point[3], int coords[3]) const {
	double temp[4];
	PKUtils::CopyVertex(point, temp);
	temp[3] = 1;

	double x = PKUtils::Dot4(this->planeYZ, temp);
	double y = PKUtils::Dot4(this->planeXZ, temp);
	double z = PKUtils::Dot4(this->planeXY, temp);

	coords[0] = (int)floor(x / this->xStep);
	coords[1] = (int)floor(y / this->yStep);
	coords[2] = (int)floor(z / this->zStep);

	return this->GetCellIndexByCoords(coords, true, false);
}

inline bool MeshNavigator::IntersectsTriangleCell(double a[3], double b[3], double c[3], int x, int y, int z) {
	// TODO: 
	// znamenkovy test zda vsechny body lezi uvnitr nebo ruzne => return true
	// prusecnice se vsemi stenami

	return true;
}

inline bool MeshNavigator::FindRayEntry(const double start[3], const double dir[3], int coords[3]) {
	double planeXYa[4], planeXYb[4], planeYZa[4], planeYZb[4], planeXZa[4], planeXZb[4];
	double intersection[3];

	// is start inside? => then OK, use it
	if (this->FindPointCoords(start, coords) >= 0) {
		return true;	
	}

	// start outside => find boundary intersection
	
	// intersection
	this->GetPlaneForCell(NAVIGATOR_AXIS_X, 0, planeYZa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_X, this->xCount, planeYZb);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Y, 0, planeXZa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Y, this->yCount, planeXZb);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Z, 0, planeXYa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Z, this->zCount, planeXYb);

	bool res = PKMath::IntersectsRayBox(start, dir, planeXYa, planeXYb, planeYZa, planeYZb, planeXZa, planeXZb, intersection);
	if (!res) {
		return false;
	}

	int index = this->FindPointCoords(intersection, coords);

	if (index >= 0) {
		return true;
	}

	// try move a little bit to fix numerical errors
	double step[3];
	double temp[3];
	PKUtils::CopyVertex(dir, step);
	PKUtils::MultiplyVertex(step, VERTEX_HIT_EPSILON);
	
	PKUtils::AddVertex(intersection, step, temp);	
	index = this->FindPointCoords(temp, coords);

	if (index >= 0) {
		return true;
	}

	PKUtils::SubtractVertex(intersection, step, temp);	
	index = this->FindPointCoords(temp, coords);

	if (index >= 0) {
		return true;
	}

	return false;
}

inline bool MeshNavigator::IntersectsRayCell(const double start[3], const double dir[3], int x, int y, int z) const {
	double planeXYa[4], planeXYb[4], planeYZa[4], planeYZb[4], planeXZa[4], planeXZb[4];
	
	// intersection
	this->GetPlaneForCell(NAVIGATOR_AXIS_X, x, planeYZa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_X, x + 1, planeYZb);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Y, y, planeXZa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Y, y + 1, planeXZb);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Z, z, planeXYa);
	this->GetPlaneForCell(NAVIGATOR_AXIS_Z, z + 1, planeXYb);

	return PKMath::IntersectsRayBox(start, dir, planeXYa, planeXYb, planeYZa, planeYZb, planeXZa, planeXZb);
}

inline void MeshNavigator::GetPlaneForCell(int axis, int index, double plane[4]) const {
	const double *sourcePlane;
	double step = 0;

	switch (axis) {
		case NAVIGATOR_AXIS_X:
			sourcePlane = this->planeYZ;
			step = this->xStep;
			break;
		case NAVIGATOR_AXIS_Y:
			sourcePlane = this->planeXZ;
			step = this->yStep;
			break;
		case NAVIGATOR_AXIS_Z:
			sourcePlane = this->planeXY;
			step = this->zStep;
			break;
		default:
			throw "Invalid axis specifier.";
			break;
	}

	PKUtils::CopyVertex4(sourcePlane, plane);
	plane[3] -= index * step;
}

