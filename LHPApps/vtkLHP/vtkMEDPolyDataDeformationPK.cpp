/*=========================================================================
Program: Musculoskeletal Modeling (VPHOP WP10)
Module: $RCSfile: vtkMEDPolyDataDeformationPK.cpp,v $
Language: C++
Date: $Date: 2012-04-16 06:42:25 $
Version: $Revision: 1.1.2.18 $
Authors: Petr Kellnhofer
==========================================================================
Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details
=========================================================================
*/

#include "vtkMEDPolyDataDeformationPK.h"
#include "mafDbg.h"
#include "time.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#include <vtkOBJReader.h>
//#include <vtkOBJExporter.h>
#pragma warning(pop)
//#include "collisiondetection.h"

//#include "../GPU_solver/Include/GPU_solver.h"
//GPU_Solver gpu_solver;

#define MAX_ITER_ON_COARSE 100
#define MIN_ITERS_BEFORE_INTERSECTION 0
#define ALLOWED_INTERSECTION_MM 0

#pragma region //copied from vtkMEDPolyDataDeformation

vtkCxxRevisionMacro(vtkMEDPolyDataDeformationPK, "$Revision: 1.1.2.18 $");
vtkStandardNewMacro(vtkMEDPolyDataDeformationPK);

volatile int vtkMEDPolyDataDeformationPK::cdErrors;
volatile int vtkMEDPolyDataDeformationPK::cdTests;

//////
/// Creates new empty deformation filter.
/// Should not be called directly, use New() instead.
//////
vtkMEDPolyDataDeformationPK::vtkMEDPolyDataDeformationPK()
{
	this->modelCount = 0;
	this->m_Skeletons = NULL;
	this->m_NumberOfSkeletons = 0;
	this->skeletons = NULL;
	this->meshes = NULL;
	this->nearestBonesPerVertex = NULL;
	this->vertexWeightsToBones = NULL;

	this->skeletonCounts = NULL;
	this->obstacleCount = 0;
	this->inputMeshes = NULL;
	this->inputMeshesCoarse = NULL;
	this->outputMeshes = NULL;
	this->outputMeshesCoarse = NULL;
	this->multiM_Skeletons = NULL;
	this->obstacles = NULL;
	this->obstaclesCoarse = NULL;
	this->preventCollisions = true;
	this->useProgresiveHulls = false;
	this->useGrid = true;
	this->debugMode = false;
	this->nWrappers = 0;

#ifdef DEBUG_vtkMEDPolyDataDeformationPK
	m_MATCHED_CC = NULL; m_MATCHED_FULLCC = NULL;
	m_MATCHED_POLYS[0] = NULL; m_MATCHED_POLYS[1] = NULL;
#endif
}

//////
/// Disposes filter and its resources.
/// Should not be called directly, use Delete() instead.
//////
vtkMEDPolyDataDeformationPK::~vtkMEDPolyDataDeformationPK()
{
	//destroy skeletons	 for all meshes
	for (int i = 0; i < this->modelCount; i++) {
		this->SetNumberOfMeshSkeletons(i, 0);
	}

	if (this->skeletons != NULL)
	{
		for (int i = 0; i < this->modelCount; i++) {
			if (this->skeletons[i] != NULL) {
				delete this->skeletons[i];
			}
		}
		delete this->skeletons;
		this->skeletons = NULL;
	}

	if (this->nearestBonesPerVertex != NULL)
	{
		for (int i = 0; i < this->modelCount; i++) {
			if (this->nearestBonesPerVertex[i] != NULL) {
				delete this->nearestBonesPerVertex[i];
			}
		}
		delete this->nearestBonesPerVertex;
		this->nearestBonesPerVertex = NULL;
	}

	if (this->vertexWeightsToBones != NULL)
	{
		for (int i = 0; i < this->modelCount; i++) {
			if (this->vertexWeightsToBones[i] != NULL) {
				PKUtils::DisposeMatrix(this->vertexWeightsToBones + i);
			}
		}

		delete this->vertexWeightsToBones;
		this->vertexWeightsToBones = NULL;
	}

	if (this->meshes != NULL)
	{
		for (int i = 0; i < this->modelCount + this->obstacleCount; i++) {
			if (this->meshes[i] != NULL) {
				delete this->meshes[i];
			}
		}
		delete[] this->meshes;
		this->meshes = NULL;
	}

	this->SetNumberOfMeshes(0);
	this->SetNumberOfObstacles(0);

#ifdef DEBUG_vtkMEDPolyDataDeformationPK
	DestroyMATCHEDData();
#endif
}

//------------------------------------------------------------------------
//Sets the number of control skeletons.
//Old skeletons are copied (and preserved)
/*virtual*/ void vtkMEDPolyDataDeformationPK::SetNumberOfSkeletons( int nCount )
	//------------------------------------------------------------------------
{
	//	_VERIFY_RET(nCount >= 0);

	if (nCount == m_NumberOfSkeletons)
		return; //nothing to be changed

	//if the number of curves is going to be decreased, we
	//need to delete some curves
	while (m_NumberOfSkeletons > nCount)
	{
		--m_NumberOfSkeletons;
		if (m_Skeletons[m_NumberOfSkeletons].pPolyLines[0] != NULL)
			m_Skeletons[m_NumberOfSkeletons].pPolyLines[0]->Delete();

		if (m_Skeletons[m_NumberOfSkeletons].pPolyLines[1] != NULL)
			m_Skeletons[m_NumberOfSkeletons].pPolyLines[1]->Delete();

		if (m_Skeletons[m_NumberOfSkeletons].pCCList != NULL)
			m_Skeletons[m_NumberOfSkeletons].pCCList->Delete();
	}

	CONTROL_SKELETON* pNewArr = NULL;
	if (nCount > 0)
	{
		pNewArr = new CONTROL_SKELETON[nCount];
		memset(pNewArr, 0, sizeof(CONTROL_SKELETON)*nCount);

		//copy existing curves
		for (int i = 0; i < m_NumberOfSkeletons; i++) {
			pNewArr[i] = m_Skeletons[i];
		}

		m_NumberOfSkeletons = nCount;
	}

	delete[] m_Skeletons;
	m_Skeletons = pNewArr;

	this->Modified();
}

//------------------------------------------------------------------------
//Specifies the n-th control skeleton.
//If RSO points are specified, they are used during the computation of LFs
//of curves of both skeletons. A local fame is defined by its origin point
//and three vectors u, v and w. Vector u is the tangent vector (it goes in
//the direction of polyline) and vectors v,w are perpendicular to this vector.
//As there is infinite number of u,v,w configurations, the algorithm uses the
//given RSO point to get a unique one (v lies in the plane defined by u and RSO).
//If RSO is not specified, v is chosen to lie in the plane closest to the u vector.
//When RSO points are not specified (or they are specified incorrectly),
//the deformed object might be unrealistically rotated against other objects
//in the scene, if the skeleton of object to deform tends to rotate (simple edge,
//or only one skeleton for object).
/*virtual*/ void vtkMEDPolyDataDeformationPK::SetNthSkeleton( int idx,
	vtkPolyData* original, vtkPolyData* modified, vtkIdList* correspondence,
	double* original_rso, double* modified_rso)
	//------------------------------------------------------------------------
{
	//	_VERIFY_RET(idx >= 0);

	if (idx >= GetNumberOfSkeletons())
		SetNumberOfSkeletons(idx + 1);

	if (m_Skeletons[idx].pPolyLines[0] != original)
	{
		if (NULL != m_Skeletons[idx].pPolyLines[0])
			m_Skeletons[idx].pPolyLines[0]->Delete();

		if (NULL != (m_Skeletons[idx].pPolyLines[0] = original))
			m_Skeletons[idx].pPolyLines[0]->Register(this);

		this->Modified();
	}

	if (m_Skeletons[idx].pPolyLines[1] != modified)
	{
		if (NULL != m_Skeletons[idx].pPolyLines[1])
			m_Skeletons[idx].pPolyLines[1]->Delete();

		if (NULL != (m_Skeletons[idx].pPolyLines[1] = modified))		
			m_Skeletons[idx].pPolyLines[1]->Register(this);

		this->Modified();
	}

	if (m_Skeletons[idx].pCCList != correspondence)
	{
		if (NULL != m_Skeletons[idx].pCCList)
			m_Skeletons[idx].pCCList->Delete();

		if (NULL != (m_Skeletons[idx].pCCList = correspondence))
			m_Skeletons[idx].pCCList->Register(this);

		this->Modified();
	}

	if (m_Skeletons[idx].RSOValid[0] != (original_rso != NULL))
	{
		if (m_Skeletons[idx].RSOValid[0] = (original_rso != NULL))
		{
			m_Skeletons[idx].RSO[0][0] = original_rso[0];
			m_Skeletons[idx].RSO[0][1] = original_rso[1];
			m_Skeletons[idx].RSO[0][2] = original_rso[2];
		}

		this->Modified();
	}

	if (m_Skeletons[idx].RSOValid[1] != (modified_rso != NULL))
	{
		if (m_Skeletons[idx].RSOValid[1] = (modified_rso != NULL))
		{
			m_Skeletons[idx].RSO[1][0] = modified_rso[0];
			m_Skeletons[idx].RSO[1][1] = modified_rso[1];
			m_Skeletons[idx].RSO[1][2] = modified_rso[2];
		}

		this->Modified();
	}
}

//------------------------------------------------------------------------
//By default, UpdateInformation calls this method to copy information
//unmodified from the input to the output.
/*virtual*/void vtkMEDPolyDataDeformationPK::ExecuteInformation()
	//------------------------------------------------------------------------
{
	//check input
	vtkPolyData* input = GetInput();
	if (input == NULL)
	{
		vtkErrorMacro(<< "Invalid input for vtkMEDPolyDataDeformation.");
		return;   //we have no input
	}

	//check output
	vtkPolyData* output = GetOutput();
	if (output == NULL)
		SetOutput(vtkPolyData::New());

	//copy input to output
	Superclass::ExecuteInformation();
}

//------------------------------------------------------------------------
//Return this object's modified time.
/*virtual*/ unsigned long int vtkMEDPolyDataDeformationPK::GetMTime()
	//------------------------------------------------------------------------
{
	unsigned long mtime = Superclass::GetMTime();
	for (int i = 0; i < m_NumberOfSkeletons; i++)
	{
		unsigned long t1;
		if (m_Skeletons[i].pPolyLines[0] != NULL)
		{
			t1 = m_Skeletons[i].pPolyLines[0]->GetMTime();
			if (t1 > mtime)
				mtime = t1;
		}

		if (m_Skeletons[i].pPolyLines[1] != NULL)
		{
			t1 = m_Skeletons[i].pPolyLines[1]->GetMTime();
			if (t1 > mtime)
				mtime = t1;
		}

		if (m_Skeletons[i].pCCList != NULL)
		{
			t1 = m_Skeletons[i].pCCList->GetMTime();
			if (t1 > mtime)
				mtime = t1;
		}
	}

	return mtime;
}

#pragma endregion // copied from original

#pragma region //my code

//*************************************************************************************************************************************
// Main method
//*************************************************************************************************************************************

//////
/// Synchronous execution of deformation.
///
/// @return true no error occured, if false results may not be relevant
//////
bool vtkMEDPolyDataDeformationPK::ExecuteMultiData() {
	PKMatrix **points = NULL;
	PKMatrix **completeMatrices = NULL;
	PKMatrix **completeVertices = NULL;
	bool allOk = true;

	try
	{
		dirTemp.clear();
		// get resources
		//bool useCache = true;	//TODO: reimplement caching policy so it does not exceed resources
		bool useCache = false;
		//omp_set_num_threads(1);

		completeMatrices = new PKMatrix*[this->modelCount];
		completeVertices = new PKMatrix*[this->modelCount];
		points = new PKMatrix*[this->modelCount];

		this->SetUpResources(&completeMatrices, &completeVertices, useCache);

		if (this->debugMode) {
			Debug_Visualize_Progress(-1, 0.0, NULL, &dirTemp);//0
		}

		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			//// predeform by M3
			//vtkPolyData* predeformed = vtkPolyData::New();
			//this->DeformMeshM3(modelIndex, true, predeformed);

			// extract points

			vtkPoints* vtkPts = this->meshes[modelIndex]->GetCoarse() /*predeformed*/->GetPoints();
			vtkIdType nPoints = vtkPts->GetNumberOfPoints();

			points[modelIndex] = PKUtils::CreateMatrix(nPoints, 3);
			PKMatrix *modelPoints = points[modelIndex];

			for (vtkIdType i = 0; i < nPoints; i++)
			{
				vtkPts->GetPoint(i, modelPoints->values[i]);
			}

			//predeformed->Delete();
		}

		cd = new CollisionDetection();
		gpu_solver = new GPU_Solver(); 
		cd->enableOpenGL();
		bool openglAvailable = cd->extensionsAvailable();
		bool cudaAvailable = gpu_solver->initCuda();
		if(openglAvailable && cudaAvailable) {
			allOk &= this->FindEnergyMinimumGPU(completeMatrices, completeVertices, &points);
		}
		else {
			allOk &= this->FindEnergyMinimumGN(completeMatrices, completeVertices, &points);
		}
		cd->disableOpenGL();
		delete cd;
		delete gpu_solver;

		// project to original meshes
		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			this->meshes[modelIndex]->SetUpPointCoords(this->meshes[modelIndex]->GetCoarse(), points[modelIndex]);
			this->meshes[modelIndex]->ApplyCoarseCoordsToOriginalMesh(points[modelIndex], NULL);
		}

		if (this->debugMode) {
			Debug_Visualize_Progress(-1, 0.0, NULL, &dirTemp); // testing
		}


		if (this->preventCollisions) {
			this->cdTests = 0;
			this->cdErrors = 0;
			this->FixFinalIntersections(this->meshes, this->modelCount + this->obstacleCount);
		}
		///
		if (this->debugMode) {
			Debug_Visualize_Progress(-1, 0.0, NULL, &dirTemp); // final state
		}

		// output results
		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			if (this->outputMeshes[modelIndex] == NULL || this->meshes[modelIndex] == NULL) {
				continue;
			}
			this->outputMeshes[modelIndex]->DeepCopy(this->meshes[modelIndex]->GetOriginal());

			if (this->outputMeshesCoarse[modelIndex] == NULL || this->meshes[modelIndex] == NULL) {
				continue;
			}
			this->outputMeshesCoarse[modelIndex]->DeepCopy(this->meshes[modelIndex]->GetCoarse());
		}
		//this->debugMode = true;

	}
	catch (...)
	{
		vtkErrorMacro(<< "Deformation failed.");
		allOk = false;
	}

	// tidy up

	// locals

	if (completeMatrices != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(completeMatrices + i);
		}

		delete[] completeMatrices;
	}

	if (completeVertices != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(completeVertices + i);
		}

		delete[] completeVertices;
	}

	if (points != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(points + i);
		}

		delete[] points;
	}

	// globals

	if (this->skeletons != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			delete this->skeletons[i];
		}

		delete this->skeletons;
		this->skeletons = NULL;
	}

	if (this->meshes != NULL) {
		for (int i = 0; i < this->modelCount + this->obstacleCount; i++) {
			delete this->meshes[i];
		}

		delete[] this->meshes;
		this->meshes = NULL;
	}

	return allOk;
}

//------------------------------------------------------------------------
//This method is the one that should be used by subclasses, right now the
//default implementation is to call the backwards compatibility method
/*virtual*/void vtkMEDPolyDataDeformationPK::ExecuteData(vtkDataObject *output)
{
	//check input
	vtkPolyData* input = this->GetInput();
	if (input == NULL)
	{
		return;
	}

	//check output
	vtkPolyData* pPoly = vtkPolyData::SafeDownCast(output);

	//if (ppoly == null || ppoly->getpoints() == null)
	//{
	//	vtkwarningmacro(<< "invalid output for vtkmedpolydatadeformation.");
	//	return;   //we have no valid output
	//}

	PKMatrix **points = NULL;
	PKMatrix **completeMatrices = NULL;
	PKMatrix **completeVertices = NULL;

	/*this->m_Skeletons->pPolyLines[1]->GetPoints()->SetPoint(this->m_Skeletons->pPolyLines[1]->GetNumberOfPoints() - 1,
	this->m_Skeletons->pPolyLines[0]->GetPoint(this->m_Skeletons->pPolyLines[0]->GetNumberOfPoints() - 1));
	double *end = this->m_Skeletons->pPolyLines[1]->GetPoint(this->m_Skeletons->pPolyLines[1]->GetNumberOfPoints() - 1);*/

	try
	{
		// get resources
		bool useCache = true;
		//omp_set_num_threads(1);

		// forge multi data
		vtkPolyData** inputMeshes = NULL;
		CONTROL_SKELETON** inputSkeletons = NULL;
		int *skeletonCounts = NULL;

		int modelCount = this->GetInputs(&inputMeshes, &inputSkeletons, &skeletonCounts);

		this->SetNumberOfMeshes(0);
		this->SetNumberOfObstacles(0);
		this->inputMeshes = inputMeshes;
		this->multiM_Skeletons = inputSkeletons;
		this->modelCount = modelCount;
		this->skeletonCounts = skeletonCounts;

		this->inputMeshesCoarse = new vtkPolyData*[this->modelCount];
		memset(this->inputMeshesCoarse, 0, sizeof(vtkPolyData*) * this->modelCount);

		// set up resource
		completeMatrices = new PKMatrix*[this->modelCount];
		completeVertices = new PKMatrix*[this->modelCount];
		points = new PKMatrix*[this->modelCount];

		this->useProgresiveHulls = false;

		this->SetUpResources(&completeMatrices, &completeVertices, useCache);

#pragma region // temp
		/*double *a, *b, *c, *d;

		this->skeletons[0]->GetOriginalEdge(0, &a, &b);
		this->skeletons[0]->GetDeformedEdge(0, &c, &d);

		double offset[3];
		PKUtils::SubtractVertex(c, a, offset);*/
		/*
		double prvni[3], druhy[3], diff[3];
		vtkIdType nPoints = this->meshes[0]->GetCoarse()->GetNumberOfPoints();

		PKMatrix* distances = PKUtils::CreateMatrix(nPoints, nPoints);
		for (vtkIdType i = 0; i < nPoints; i++) {
		for (vtkIdType j = 0; j < nPoints; j++) {
		this->meshes[0]->GetCoarse()->GetPoint(i, prvni);
		this->meshes[0]->GetCoarse()->GetPoint(j, druhy);
		PKUtils::SubtractVertex(prvni, druhy, diff);

		distances->values[i][j] = PKUtils::CalculateVertexLength(diff);

		if (i == j) {
		distances->values[i][j] = 1E99;
		}

		if (i != j && distances->values[i][j] < 0.1) {
		cout << "hmmm";
		}
		}
		}

		distances->DebugOutputMathematica("distances.nb");
		*/
#pragma endregion // end temp

		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			// extract points
			vtkPoints* vtkPts = this->meshes[modelIndex]->GetCoarse()->GetPoints();
			vtkIdType nPoints = vtkPts->GetNumberOfPoints();

			points[modelIndex] = PKUtils::CreateMatrix(nPoints, 3);
			PKMatrix *modelPoints = points[modelIndex];

			for (vtkIdType i = 0; i < nPoints; i++) {
				vtkPts->GetPoint(i, modelPoints->values[i]);				
			}			
		}

		//// solve
		//omp_set_num_threads(1);
		this->preventCollisions = true;
		this->useGrid = true;
		this->FindEnergyMinimumGN(completeMatrices, completeVertices, &points);

		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			// output results
			this->meshes[modelIndex]->SetUpPointCoords(this->meshes[modelIndex]->GetCoarse(), points[modelIndex]);
			this->meshes[modelIndex]->ApplyCoarseCoordsToOriginalMesh(points[modelIndex], NULL);
		}

		if (this->preventCollisions) {
			//this->FixFinalIntersections(this->meshes, this->modelCount + this->obstacleCount);
		}

		this->ExportMeshes(pPoly);
	}
	catch (...)
	{
		vtkErrorMacro(<< "Deformation failed.");
	}

	// tidy up

	// locals

	if (completeMatrices != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(completeMatrices + i);
		}

		delete[] completeMatrices;
	}

	if (completeVertices != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(completeVertices + i);
		}

		delete[] completeVertices;
	}

	if (points != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(points + i);
		}

		delete[] points;
	}

	// globals

	if (this->skeletons != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			delete this->skeletons[i];
		}

		delete this->skeletons;
		this->skeletons = NULL;
	}

	if (this->meshes != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			delete this->meshes[i];
		}

		delete this->meshes;
		this->meshes = NULL;
	}
}

int vtkMEDPolyDataDeformationPK::GetInputs(vtkPolyData ***inputMeshes, CONTROL_SKELETON*** inputSkeletons, int **skeletonCounts) {
	vtkPolyData *poly = NULL;
	double point[3];
	double temp[3];

	//check input
	vtkPolyData* input = this->GetInput();
	if (input == NULL)
	{
		return 0;
	}

#ifdef SEMIM
	int modelCount = 1;
#else
	int modelCount = 2;

	this->m_Skeletons[0].RSO[0][0] = 185 + 10; // x
	this->m_Skeletons[0].RSO[0][1] = 150; // y
	this->m_Skeletons[0].RSO[0][2] = 1820; // lentgh

	this->m_Skeletons[0].RSO[1][0] = 185; // x
	this->m_Skeletons[0].RSO[1][1] = 150 + 10; // y
	this->m_Skeletons[0].RSO[1][2] = 1820; // length

	//this->m_Skeletons[0].RSOValid[0] = this->m_Skeletons[0].RSOValid[1] = true;
#endif
	double offset[] = { 0, 40, 0 };
	//double offset[] = { 0, 72, 0 };

	// meshes
	*inputMeshes = new vtkPolyData*[modelCount];
	for (int i = 0; i < modelCount; i++) {
		// clone mesh
		(*inputMeshes)[i] = vtkPolyData::New();
		poly = (*inputMeshes)[i];
		poly->DeepCopy(input);

		// move mesh
		PKUtils::CopyVertex(offset, temp);
		PKUtils::MultiplyVertex(temp, i);

		vtkPoints *points = poly->GetPoints();
		vtkIdType pointCount = points->GetNumberOfPoints();
		for (vtkIdType j = 0; j < pointCount; j++) {
			points->GetPoint(j, point);
			PKUtils::AddVertex(point, temp, point);
			points->SetPoint(j, point);
		}

		poly->BuildCells();
		poly->BuildLinks();
	}

	// skeletons
	*inputSkeletons = new CONTROL_SKELETON*[modelCount];
	*skeletonCounts = new int[modelCount];
	CONTROL_SKELETON *originalSkeleton = NULL;

	for (int i = 0; i < modelCount; i++) {
		(*inputSkeletons)[i] = new CONTROL_SKELETON[this->m_NumberOfSkeletons];

		(*skeletonCounts)[i] = 0;

		for (int skeletonIndex = 0; skeletonIndex < this->m_NumberOfSkeletons; skeletonIndex++) {
			originalSkeleton = this->m_Skeletons + skeletonIndex;

			// validate
			if (originalSkeleton->pPolyLines[0] == NULL || originalSkeleton->pPolyLines[1] == NULL) {
				continue;
			}

			// clone skeleton
			CONTROL_SKELETON *skeleton = (*inputSkeletons)[i] + (*skeletonCounts)[i];
			memcpy(skeleton, originalSkeleton, sizeof(CONTROL_SKELETON));

			skeleton->pCCList = NULL;
			if (originalSkeleton->pCCList != NULL) {
				skeleton->pCCList = vtkIdList::New();
				skeleton->pCCList->DeepCopy(originalSkeleton->pCCList);
			}

			skeleton->pPolyLines[0] = vtkPolyData::New();
			skeleton->pPolyLines[0]->DeepCopy(originalSkeleton->pPolyLines[0]);
			skeleton->pPolyLines[1] = vtkPolyData::New();
			skeleton->pPolyLines[1]->DeepCopy(originalSkeleton->pPolyLines[1]);

			// move skeleton
			PKUtils::CopyVertex(offset, temp);
			PKUtils::MultiplyVertex(temp, i);

			vtkPoints *points = skeleton->pPolyLines[0]->GetPoints();
			vtkIdType pointCount = points->GetNumberOfPoints();
			for (vtkIdType j = 0; j < pointCount; j++) {
				points->GetPoint(j, point);
				PKUtils::AddVertex(point, temp, point);
				points->SetPoint(j, point);
			}

			points = skeleton->pPolyLines[1]->GetPoints();
			pointCount = points->GetNumberOfPoints();
			for (vtkIdType j = 0; j < pointCount; j++) {
				points->GetPoint(j, point);
				PKUtils::AddVertex(point, temp, point);
				points->SetPoint(j, point);
			}

			// move rso
			PKUtils::AddVertex(skeleton->RSO[0], temp, skeleton->RSO[0]);
			PKUtils::AddVertex(skeleton->RSO[1], temp, skeleton->RSO[1]);

			(*skeletonCounts)[i]++;
		}
	}

	return modelCount;
}

//////
/// Creates or loads resources for current deformation.
/// @param matrixL pointer to pointer to matrix of linear conditions of deformation, not to be preallocated (output)
/// @param vectorB pointer to pointer to vector of right side of linear conditions of deformation, not to be preallocated (output)
/// @param useCache flag of cache usage, on true tries to retrieve data from cache from previous deformation first or
///                 calculates new and then stores copy to cache. On false always calculates all resources from scratch.
//////
void vtkMEDPolyDataDeformationPK::SetUpResources(PKMatrix ***matricesL, PKMatrix ***verticesB, bool useCache)
{
	this->meshes = new MeshSurface*[this->modelCount + this->obstacleCount];
	this->skeletons = new MeshSkeleton*[this->modelCount];
	/*this->nearestBonesPerVertex = new int*[modelCount];
	this->vertexWeightsToBones = new PKMatrix*[modelCount];*/

	int *cacheIds = new int[this->modelCount + this->obstacleCount];
	bool *cacheFilled = new bool[this->modelCount + this->obstacleCount];
	memset(cacheIds, 0, sizeof(int) * this->modelCount + this->obstacleCount);

	// get cache singleton instance
	DeformationCache *cache = DeformationCache::GetInstance();

	// mesh skeletons extraction
	// temporary skeleton extracts - find first valids
	CONTROL_SKELETON *skeletons = new CONTROL_SKELETON[this->modelCount];
	for (int i = 0; i < this->modelCount; i++) {
		for (int j = 0; j < this->skeletonCounts[i]; j++) {
			if (this->multiM_Skeletons[i][j].pPolyLines[0] != NULL) {
				skeletons[i] = this->multiM_Skeletons[i][j];
				break;
			}
		}
	}

#pragma omp parallel
	{
		// meshes
#pragma omp for nowait	//nowait => threads may continue without being synchronized at the end of the loop
		for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
			// update action lines
			skeletons[modelIndex].pPolyLines[0]->BuildCells();
			skeletons[modelIndex].pPolyLines[1]->BuildCells();
			this->skeletons[modelIndex] = new MeshSkeleton(this->multiM_Skeletons[modelIndex], this->skeletonCounts[modelIndex]);

			// load from cache
			if (!useCache) 
			{
				cacheIds[modelIndex] = -1;
				cacheFilled[modelIndex] = false;
			}
			else
			{
#pragma omp critical
				{	//cache is not thread-safe => its access must be synchronized
					cacheIds[modelIndex] = cache->ContainsDataForTransformation(this->inputMeshes[modelIndex], skeletons[modelIndex].pPolyLines[0]);
					cacheFilled[modelIndex] = cacheIds[modelIndex] >= 0 ? true : false;

					if (cacheFilled[modelIndex]) {
						this->meshes[modelIndex] = cache->GetMeshSurfaceDeepCopy(cacheIds[modelIndex]);
					}
				}
			}

			//if not loaded from a cache, we need to construct it			
			if (!cacheFilled[modelIndex])
			{
				// prepare mesh
				this->meshes[modelIndex] = new MeshSurface(this->inputMeshes[modelIndex],true, this->inputMeshesCoarse[modelIndex], COARSE_MESH_SIZE, this->useProgresiveHulls);
				this->meshes[modelIndex]->SetDeformable(true);

				// extract coarse mesh
				if (this->inputMeshesCoarse[modelIndex] == NULL) {
					this->inputMeshesCoarse[modelIndex] = vtkPolyData::New();
					this->inputMeshesCoarse[modelIndex]->DeepCopy(this->meshes[modelIndex]->GetCoarse());
				}

				// store to cache
				if (useCache)
				{
#pragma omp critical
					{	//cache is not thread-safe => its access must be synchronized
						cacheIds[modelIndex] = cache->AddNewTransformation(this->inputMeshes[modelIndex], skeletons[modelIndex].pPolyLines[0]);
						cache->SetMeshSurfaceDeepCopy(cacheIds[modelIndex], this->meshes[modelIndex]);
					}
				}
			}
		}

		// obstacles
#pragma omp for //all threads must be synchronized at the end of this loop
		for (int obstacleIndex = 0; obstacleIndex < this->obstacleCount; obstacleIndex++) {

			if (!useCache)
			{
				cacheIds[this->modelCount + obstacleIndex] = -1;
				cacheFilled[this->modelCount + obstacleIndex] = false;
			}
			else
			{
				// load from cache
#pragma omp critical
				{	//cache is not thread-safe => its access must be synchronized
					cacheIds[this->modelCount + obstacleIndex] = cache->ContainsDataForTransformation(this->obstacles[obstacleIndex]);
					cacheFilled[this->modelCount + obstacleIndex] = cacheIds[this->modelCount + obstacleIndex] >= 0 ? true : false;
			
					if (cacheFilled[this->modelCount + obstacleIndex]) {
						this->meshes[this->modelCount + obstacleIndex] = cache->GetMeshSurfaceDeepCopy(cacheIds[this->modelCount + obstacleIndex]);
					}
				}
			}

			//if not loaded from a cache, we need to construct it			
			if (!cacheFilled[this->modelCount + obstacleIndex])
			{
				// prepare mesh
				this->meshes[this->modelCount + obstacleIndex] = new MeshSurface(this->obstacles[obstacleIndex], false, this->obstaclesCoarse[obstacleIndex], COARSE_MESH_SIZE, this->useProgresiveHulls);
				this->meshes[this->modelCount + obstacleIndex]->SetDeformable(false);

				// extract coarse mesh
				if (this->obstaclesCoarse[obstacleIndex] == NULL) {
					this->obstaclesCoarse[obstacleIndex] = vtkPolyData::New();
					this->obstaclesCoarse[obstacleIndex]->DeepCopy(this->meshes[this->modelCount + obstacleIndex]->GetCoarse());
				}

				// store to cache
				//BES: 11.2.2013 - obstacles change every time => caching obstacles leaks memory
/*				if (useCache)
				{
#pragma omp critical
					{	//cache is not thread-safe => its access must be synchronized
						cacheIds[this->modelCount + obstacleIndex] = cache->AddNewTransformation(this->obstacles[obstacleIndex]);
						cache->SetMeshSurfaceDeepCopy(cacheIds[this->modelCount + obstacleIndex], this->meshes[this->modelCount + obstacleIndex]);
					}
				}
				*/
			}
		}
	}	// end parallel block

		// attach vertices to skeleton for all meshes
		this->CalculateVertexWeightsToSkeleton();

#pragma omp parallel
	{
		// calculate matrices
#pragma omp for
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
			PKMatrix *laplacMatrix = NULL;
			PKMatrix *laplacOperator = NULL;
			PKMatrix *skeletonMatrix = NULL;
			PKMatrix *skeletonVector = NULL;

			// load from cache
			if (useCache && cacheFilled[modelIndex])
			{
#pragma omp critical 
				{
					laplacMatrix = cache->GetLaplacianMatrixDeepCopy(cacheIds[modelIndex]);
					laplacOperator = cache->GetLaplacianOperatorDeepCopy(cacheIds[modelIndex]);
				}
			}
			else
			{
				// calculate laplacian
				this->CalculateLaplacian(&laplacMatrix, &laplacOperator, modelIndex);

				// store to cache
				if (useCache)
				{
#pragma omp critical
					{
						cache->SetLaplacianMatrixDeepCopy(cacheIds[modelIndex], laplacMatrix);
						cache->SetLaplacianOperatorDeepCopy(cacheIds[modelIndex], laplacOperator);
					}
				}
			}

			// calculate skeleton matrix
			this->CalculateSkeletonMatrix(&skeletonMatrix, &skeletonVector, modelIndex);

			// update laplac operator
			this->RotateLaplacOperator(laplacOperator, laplacOperator, modelIndex);

			// merge linear conditions
			(*matricesL)[modelIndex] = PKUtils::MergeMatricesVerticallySoft(laplacMatrix, skeletonMatrix);
			(*verticesB)[modelIndex] = PKUtils::MergeMatricesVerticallySoft(laplacOperator, skeletonVector);

			// tidy up
			PKUtils::DisposeMatrixContainer(&laplacMatrix);
			PKUtils::DisposeMatrixContainer(&laplacOperator);
			PKUtils::DisposeMatrixContainer(&skeletonMatrix);
			PKUtils::DisposeMatrixContainer(&skeletonVector);
		}
	}

	delete[] skeletons;
	delete[] cacheFilled;
	delete[] cacheIds;	
}

void vtkMEDPolyDataDeformationPK::ExportMeshes(vtkPolyData *output) {
	if (output == NULL || this->modelCount == 0 || this->meshes == NULL || this->meshes[0] == NULL) {
		return;
	}

	vtkAppendPolyData* merger = vtkAppendPolyData::New();

	for (int i = 0; i < this->modelCount; i++) {
		merger->AddInput(this->meshes[i]->GetOriginal());
		//merger->AddInput(this->meshes[i]->GetCoarse());
	}

	merger->Update();

	output->DeepCopy(merger->GetOutput());
	//output->DeepCopy(this->meshes[0]->GetCoarse());
	//output->DeepCopy(this->meshes[0]->GetOriginal());

	merger->Delete();
}

#pragma region // point-skeleton weights

//////
/// Calculates vertex relation weight for bone segments of skeleton.
/// They are used for correct rotation of laplacian operator.
/// Results kept in internal structures of this class.
//////
void vtkMEDPolyDataDeformationPK::CalculateVertexWeightsToSkeleton()
{
	// disposal
	if (this->vertexWeightsToBones != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			PKUtils::DisposeMatrix(this->vertexWeightsToBones + i);
		}

		delete this->vertexWeightsToBones;
		this->vertexWeightsToBones = NULL;
	}

	if (this->nearestBonesPerVertex != NULL) {
		for (int i = 0; i < this->modelCount; i++) {
			delete this->nearestBonesPerVertex[i];
		}

		delete this->nearestBonesPerVertex;
		this->nearestBonesPerVertex = NULL;
	}

	// allocation
	this->vertexWeightsToBones = new PKMatrix*[this->modelCount];
	for (int i = 0; i < this->modelCount; i++) {
		vtkPolyData *workingMesh = this->meshes[i]->GetCoarse();
		vtkIdType nPoints = workingMesh->GetNumberOfPoints();
		int nBones = this->skeletons[i]->GetNumberOfMainBones();
		this->vertexWeightsToBones[i] = PKUtils::CreateMatrix(nPoints, nBones);
	}

	this->nearestBonesPerVertex = new int*[this->modelCount];
	for (int i = 0; i < this->modelCount; i++) {
		vtkPolyData *workingMesh = this->meshes[i]->GetCoarse();
		vtkIdType nPoints = workingMesh->GetNumberOfPoints();
		this->nearestBonesPerVertex[i] = new int[nPoints];
	}

	double point[3];

	for (int modelIndex = 0; modelIndex < this->modelCount; modelIndex++) {
		// for every vertex of mesh
		vtkPolyData *workingMesh = this->meshes[modelIndex]->GetCoarse();
		vtkIdType nPoints = workingMesh->GetNumberOfPoints();

		MeshSkeleton *skeleton = this->skeletons[modelIndex];
		int nBones = skeleton->GetNumberOfMainBones();

		PKMatrix *vertexWeightsToBones = this->vertexWeightsToBones[modelIndex];
		int *nearestBonesPerVertex = this->nearestBonesPerVertex[modelIndex];

		for (vtkIdType i = 0; i < nPoints; i++)
		{
			double *row = vertexWeightsToBones->values[i];

			workingMesh->GetPoint(i, point);

			// measure distances of vertex to all parts of skeleton
			double min;
			double max;
			skeleton->MeasurePointDistances(point, row, &min, &max, NULL);

			double minDistance = min + 1;
			double currDistance;
			int *currentNearIndex = nearestBonesPerVertex + i;

			// lineary map to fit: max  = 0, min = 1
			double *end = row + nBones;
			for (int j = 0; row != end; row++, j++)
			{
				currDistance = (*row);

				// find nearest bone (part of skeleton)
				if (currDistance < minDistance)
				{
					minDistance = currDistance;
					(*currentNearIndex) = j;
				}

				(*row) = 1 - (currDistance - min) / (max - min);
			}
		}
	}
}

#pragma endregion // point-skeleton weights

#pragma region //skeleton

//////
/// Calculates skeleton matrix for defining of skeleton using mesh vertices and final position for deformed skeleton points.
/// Output are barycentric coordinates of skeleton vertices in coarse mesh and new Euclid coordinates of the same vertices.
/// @param skeletonMatrix pointer to skeleton matrix with decomposition of bones to linear combination coeficient of mesh vertices - not to be preallocated (output)
/// @param laplOperator pointer to vector of deformed coordinates for skeleton points - not to be preallocated (output)
//////
void vtkMEDPolyDataDeformationPK::CalculateSkeletonMatrix(PKMatrix **skeletonMatrix, PKMatrix **skeletonVector, int modelIndex)
{
	vtkPolyData *workingMesh = this->meshes[modelIndex]->GetCoarse();
	MeshSkeleton *skeleton = this->skeletons[modelIndex];

	vtkIdType nPoints = workingMesh->GetNumberOfPoints();
	int numberOfBones = skeleton->GetNumberOfBones();

	// allocation of memory
	*skeletonMatrix = PKUtils::CreateMatrix(numberOfBones + 1, workingMesh->GetNumberOfPoints());
	*skeletonVector = PKUtils::CreateMatrix(numberOfBones + 1, 3);

	double *a;
	double *b;

	// all next segments (expected to be continuous, so olny b vertex is decomposed for every segment)
	for (int i = -1; i < numberOfBones; i++)
	{
		// condition for special treatment of first vertex (calculates first point of line instead of last as usualy)
		if (i >= 0)
		{
			skeleton->GetOriginalEdge(i, &a, &b);
		}
		else
		{
			skeleton->GetOriginalEdge(0, &b, &a);
		}

		// retrieve original barycentric coordinates
		this->meshes[modelIndex]->CalculateMeshCoordsForPoint(workingMesh, b, (*skeletonMatrix)->values[i + 1]);

		if (i >= 0)
		{
			skeleton->GetDeformedEdge(i, &a, &b);
		}
		else
		{
			skeleton->GetDeformedEdge(0, &b, &a);
		}

		// copy final coordinates to output
		PKUtils::CopyVertex(b, (*skeletonVector)->values[i + 1]);
	}
}

#pragma endregion //skeleton

#pragma region // laplacian

//////
/// Calculates area of triangle determined by three points.
/// @param a 3 coordinates of first apex (input)
/// @param b 3 coordinates of second apex (input)
/// @param c 3 coordinates of first apex (input)
/// @return area of triangle
//////
double vtkMEDPolyDataDeformationPK::CalculateTriangleArea(double a[3], double b[3], double c[3])
{
	double sideA[3];
	double sideB[3];
	double cross[3];

	PKUtils::SubtractVertex(b, a, sideA);
	PKUtils::SubtractVertex(c, a, sideB);

	vtkMath::Cross(sideA, sideB, cross);

	double length = PKUtils::CalculateVertexLength(cross);

	return length / 2;
}

//////
/// Rotates differential vertices of mesh according to change of skeleton shape.
/// Uses skeleton attribute to determine parameters of rotation. Keeps lengths of original vertices.
/// @param laplOperator differetial coordinates to rotate (input and output)
/// @param result of rotation (rotated laplacian operator) - must be preallocated, can be same as laplOperator (output)
//////
void vtkMEDPolyDataDeformationPK::RotateLaplacOperator(PKMatrix *laplOperator, PKMatrix *result, int modelIndex)
{
	double start[3];
	double end[3];
	double oldDir[3];
	double newDir[3];

	int* nearestBones = this->nearestBonesPerVertex[modelIndex];
	MeshSkeleton *skeleton = this->skeletons[modelIndex];

	// rotate by RSO
	double rsoAxis[3];
	double rsoAngle = 0;
	bool rsoValid = skeleton->GetRsoRotation(rsoAxis, rsoAngle);

	RotationMatrix rsoRotation = RotationMatrix();
	if (rsoValid) {
		PKUtils::RotationMatrixGeneral(rsoAxis, rsoAngle, &rsoRotation);
	}

	int count = laplOperator->height;

	// rotate laplac operator
	for (vtkIdType i = 0; i < count; i++)
	{
		double diffVector[3];
		PKUtils::CopyVertex(laplOperator->values[i], diffVector);

		/////////////////
		// choose bone //
		/////////////////

		int boneId = nearestBones[i];

		/////////////////////////////
		// get rotation parameters //
		/////////////////////////////

		double *a;
		double *b;

		skeleton->GetOriginalEdge(boneId, &a, &b);
		PKUtils::CopyVertex(a, start);
		PKUtils::CopyVertex(b, end);
		PKUtils::SubtractVertex(end, start, oldDir);
		PKUtils::NormalizeVertex(oldDir);

		skeleton->GetDeformedEdge(boneId, &a, &b);
		PKUtils::CopyVertex(a, start);
		PKUtils::CopyVertex(b, end);
		PKUtils::SubtractVertex(end, start, newDir);
		PKUtils::NormalizeVertex(newDir);

		// calculate rotation axis
		double axis[3];
		double angle = 0;
		PKUtils::GetRotation(oldDir, newDir, axis, angle);

		// rotate by change of skeleton
		RotationMatrix rotMat = RotationMatrix();
		PKUtils::RotationMatrixGeneral(axis, angle, &rotMat);

		// combine
		RotationMatrix rotFinal = RotationMatrix();
		PKUtils::MultiplyMatrices(&rotMat, &rsoRotation, &rotFinal);

		PKUtils::MultiplyMatrixVertex(&rotFinal, diffVector, result->values[i]);
	}
}

//////
/// Calculates laplacian matrix and vertex of differential coordinates for every vertex of coarse mesh.
/// @param laplMatrix pointer to laplacian matrix - not to be preallocated (output)
/// @param laplOperator pointer to vector of differential coordinates - not to be preallocated (output)
//////
void vtkMEDPolyDataDeformationPK::CalculateLaplacian(PKMatrix **laplMatrix, PKMatrix **laplOperator, int modelIndex)
{
	double sideA[3];
	double sideB[3];

	vtkPolyData *workingMesh = this->meshes[modelIndex]->GetCoarse();

	vtkIdType* neighbours;
	vtkIdType neighboursCount;

	int nPoints = workingMesh->GetNumberOfPoints();
	int nCoarsePoints = this->meshes[modelIndex]->GetCoarse()->GetNumberOfPoints();

	// allocate memory
	PKMatrix *laplMatrixLocal = PKUtils::CreateMatrix(nPoints, nCoarsePoints);
	(*laplMatrix) = laplMatrixLocal;

	PKMatrix *laplOperatorLocal = PKUtils::CreateMatrix(nPoints, 3);
	(*laplOperator) = laplOperatorLocal;

	double *matrixRow;

	// for every point
	for (vtkIdType i = 0; i < nPoints; i++)
	{
		matrixRow = laplMatrixLocal->values[i];
		double normal[3] = {0, 0, 0};

		matrixRow[i] = -1;

		double center[3];	//center point, point i
		workingMesh->GetPoint(i, center);

		this->meshes[modelIndex]->GetPointNeighbours(workingMesh, i, neighbours, neighboursCount);

		vtkIdType* pValidNeighbours = new vtkIdType[neighboursCount];
		int nValidNeighbours = 0;

		double* weights = new double[neighboursCount];
		double weightSum = 0;

		// calculate weights
		for (vtkIdType j = 0; j < neighboursCount; j++)
		{
			// read all relevant vertices
			vtkIdType leftId, rightId;
			this->meshes[modelIndex]->GetPointsTriangleVertices(workingMesh, i, neighbours[j], &leftId, &rightId);

			if (leftId < 0 || rightId < 0) {
				vtkErrorMacro(<< "Edge with less than two cells detected in CalculateLaplacian().");
				continue;
			}

			//we have here a quadrilateral => the edge i - neighbours[j] is valid
			if (leftId == rightId) {
				vtkErrorMacro(<< "Edge with less than two touching cells detected in CalculateLaplacian().");
				continue;
			}

			double opposit[3];
			workingMesh->GetPoint(neighbours[j], opposit);

			double left[3];
			workingMesh->GetPoint(leftId, left);

			double right[3];
			workingMesh->GetPoint(rightId, right);

			// calculate both cotangens
			double cotan = 0;

			PKUtils::SubtractVertex(opposit, left, sideA);
			PKUtils::SubtractVertex(center, left, sideB);
			cotan += PKUtils::CalculateVertexCotan(sideA, sideB);
			_ASSERTE(ISFINITE(cotan));

			PKUtils::SubtractVertex(opposit, right, sideA);
			PKUtils::SubtractVertex(center, right, sideB);
			cotan += PKUtils::CalculateVertexCotan(sideA, sideB);
			_ASSERTE(ISFINITE(cotan));

			pValidNeighbours[nValidNeighbours] = neighbours[j];
			weights[nValidNeighbours] = cotan;
			weightSum += weights[nValidNeighbours];
			nValidNeighbours++;
		}

		_ASSERTE(nValidNeighbours == neighboursCount);

		// calculate final values
		for (vtkIdType j = 0; j < nValidNeighbours; j++)
		{
			double weight = weights[j] / weightSum;

			double opposit[3];
			workingMesh->GetPoint(pValidNeighbours[j], opposit);

			matrixRow[pValidNeighbours[j]] = weight;
			_ASSERTE(ISFINITE(weight));

			// calculate normal change
			double direction[3];
			PKUtils::SubtractVertex(opposit, center, direction);
			PKUtils::MultiplyVertex(direction, weight);
			PKUtils::AddVertex(direction, normal, normal);
		}

		// output differential coordinates
		PKUtils::CopyVertex(normal, laplOperatorLocal->values[i]);

		delete[] weights;
		delete[] neighbours;
		delete[] pValidNeighbours;
	}
}

#pragma endregion // laplacian

#pragma region // volume

//////
/// Calculates oriented volume of mesh defined by topology and external point coordinates.
/// @param polyData information about topology of mesh and optionally coordinates of vertices (input)
/// @param alternativeCoords array of actual point coordinates to use instead of coords in mesh or NULL to use data in polyData (input)
/// @return oriented volume of mesh
//////
double vtkMEDPolyDataDeformationPK::CalculateVolume(vtkPolyData *polyData, PKMatrix *alternativeCoords)
{
	if (alternativeCoords != NULL && (alternativeCoords->width < 3 || alternativeCoords->height < polyData->GetNumberOfPoints()))
	{
		throw "Alternative coordinates have invalid size.";
	}

	double sum = 0;
	double temp[3];
	double A[3], B[3], C[3];
	double **coords = alternativeCoords != NULL ? alternativeCoords->values : NULL;

	for (vtkIdType i = 0; i < polyData->GetNumberOfCells(); i++)
	{
		vtkIdType nCellPoints, *pCellPtsIds;
		polyData->GetCellPoints(i, nCellPoints, pCellPtsIds);

		//check, if it is triangle?
		_VERIFY_CMD(nCellPoints == 3, continue);

		if (coords != NULL)
		{
			vtkMath::Cross(coords[pCellPtsIds[0]], coords[pCellPtsIds[1]], temp);
			sum += vtkMath::Dot(temp, coords[pCellPtsIds[2]]);
		}
		else
		{
			polyData->GetPoint(pCellPtsIds[0], A);
			polyData->GetPoint(pCellPtsIds[1], B);
			polyData->GetPoint(pCellPtsIds[2], C);
			vtkMath::Cross(A, B, temp);
			sum += vtkMath::Dot(temp, C);
		}
	}

	sum /= 6;
	return sum;
}

//////
/// Calculates transposed Jacobi matrix of volume function.
/// Matrix is transposed for faster access to memory. It means, that in every row, there are three derivations of volume function,
/// one for each coordinate of specified vertex. There are as many rows as vertices in mesh. As a result, you get
/// nPoints x 3 matrix. To get well defined Jacobi matrix of volume, you may need to linearize content of this matrix row by row,
/// as many calculations may require this to be gradient vector instead, as volume function is scalar in the first place.
/// @param alternativeCoords optional array of actual point coordinates used instead of coords in mesh. NULL accepted, then mesh coords are used. (input)
/// @param jacobiMatrix preallocated Jacobi matrix (output)
//////
void vtkMEDPolyDataDeformationPK::CalculateVolumeJacobiMatrixTransposed(vtkPolyData *mesh, PKMatrix *alternativeCoords, PKMatrix *jacobiMatrix)
{
	double sum[3];
	double temp[3];
	double aCoords[3], bCoords[3];
	double **coords = alternativeCoords != NULL ? alternativeCoords->values : NULL;

	vtkIdType nPoints = mesh->GetNumberOfPoints();

	// for every vertex of mesh
	for (vtkIdType i = 0; i < nPoints; i++)
	{
		sum[0] = sum[1] = sum[2] = 0;
		unsigned short nCells;
		vtkIdType *cells;

		mesh->GetPointCells(i, nCells, cells);

		// for all his cells (triangles expected)
		for (vtkIdType j = 0; j < nCells; j++)
		{
			vtkIdType nCellPoints, *pCellPtsIds;
			mesh->GetCellPoints(cells[j], nCellPoints, pCellPtsIds);

			//check, if it is triangle?
			_VERIFY_CMD(nCellPoints == 3, continue);

			// find two neighbours
			vtkIdType a = pCellPtsIds[0];
			vtkIdType b = pCellPtsIds[1];

			int coef = 1;

			// express point i outside, so you have (a . b) x i
			if (a == i)
			{
				a = pCellPtsIds[2];
				coef = - 1;
			}
			else if (b == i)
			{
				b = pCellPtsIds[2];
				coef = - 1;
			}

			// calculates a . b after derivation by i
			if (coords != NULL)
			{
				vtkMath::Cross(coords[a], coords[b], temp);
			}
			else
			{
				mesh->GetPoint(a, aCoords);
				mesh->GetPoint(b, bCoords);
				vtkMath::Cross(aCoords, bCoords, temp);
			}

			// apply negation from expressing step
			PKUtils::MultiplyVertex(temp, coef);
			// add to result
			PKUtils::AddVertex(temp, sum, sum);
		}

		// divide result of derivation by single vertex to match volume equation
		PKUtils::MultiplyVertex(sum, 1.0 / 6.0);

		// copy to output
		PKUtils::CopyVertex(sum, jacobiMatrix->values[i]);
	}
}

#pragma endregion // volume

#pragma region // intersections

void vtkMEDPolyDataDeformationPK::FixFinalIntersections(MeshSurface **meshes, int meshCount) {
	// update OOC and navigator

	for (int modelIndex = 0; modelIndex < meshCount; modelIndex++) {
		OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
		MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

		ooc->SetUpByMesh(this->meshes[modelIndex]->GetOriginal(), NULL);

		if (this->useGrid) {
			navigator->SetUpByMesh(this->meshes[modelIndex]->GetOriginal(), NULL, ooc);
		}
	}

	int meshMaxSize = 0;

	for (int i = 0; i < meshCount; i++) {
		meshMaxSize = max(meshes[i]->GetOriginal()->GetNumberOfPoints(), meshMaxSize);
	}

	// solve original mesh intersections
	PKMatrix *fixA = PKUtils::CreateMatrix(meshMaxSize, 3);
	PKMatrix *fixB = PKUtils::CreateMatrix(meshMaxSize, 3);

	tIntersectionContext contextA;
	tIntersectionContext contextB;

	//TODO perform collision detection
	int numberOfObjects = meshCount;
	gpuModel* gpuModels = NULL;	
	unsigned int*** verticesInside = NULL;
	unsigned int*** verticesUndecided = NULL;
	char** meshesIntersect = NULL;
	//bool** meshesUndecided = NULL;

	// enable OpenGL
	cd = new CollisionDetection();
	cd->enableOpenGL();
	bool openglAvailable = cd->extensionsAvailable();


	if(openglAvailable) 
	{
		gpuModels = new gpuModel[numberOfObjects];

		verticesInside = new unsigned int**[numberOfObjects];
		verticesUndecided = new unsigned int**[numberOfObjects];
		meshesIntersect = new char*[numberOfObjects];

		//alocate memory
		for (int i = 0; i < numberOfObjects; i++) {
			verticesInside[i] = new unsigned int*[numberOfObjects];
			verticesUndecided[i] = new unsigned int*[numberOfObjects];
			meshesIntersect[i] = new char[numberOfObjects];
			for (int j = 0; j < numberOfObjects; j++) {
				verticesInside[i][j] = NULL;
				verticesUndecided[i][j] = NULL;
				meshesIntersect[i][j] = 'u';
			}
		}

		//Initializing data
		GLuint numberOfPoints = 0;
		for(int i = 0; i < numberOfObjects; i++) {
			gpuModels[i] = cd->generateBuffers(meshes[i]->GetOriginal(), meshes[i]->GetDeformable());
			if(numberOfPoints < gpuModels[i].numberOfPoints)
				numberOfPoints = gpuModels[i].numberOfPoints;
		}

		cd->generateIdBuffers(numberOfPoints);

		//allocate memory for vertices
		for(int i = 0; i < numberOfObjects; i++) {
			for (int j = 0; j < numberOfObjects; j++) {					
				int bufferSize = gpuModels[j].numberOfPoints / 32;
				if(gpuModels[j].numberOfPoints % 32 != 0) bufferSize++;
				verticesInside[i][j] = new unsigned int[bufferSize];
				verticesUndecided[i][j] = new unsigned int[bufferSize];					
			}
		}

		cd->init();

		//boundary test - if vertex is on boundary we cannot decide if it is inside or outside
		for (int i = 0; i < numberOfObjects; i++) {
			//voxelize data[i]
			cd->prepareVoxelization();
			cd->voxelize(gpuModels[i]);
			cd->prepareBoundaryVoxelization();
			cd->voxelize(gpuModels[i]);
			cd->prepareTest();
			for (int j = 0; j < numberOfObjects; j++) {
				if(i == j) continue;
				//if (!meshes[j]->GetDeformable()) continue;
				//TODO if j > i and i is deformable and j i was already tested and no collision was detected then continue
				//if (!OoCylinder::intersectsCapsule(meshes[i]->GetOoCylinder(), meshes[j]->GetOoCylinder())) continue;

				//test data[j]
				meshesIntersect[i][j] = cd->test(gpuModels[j], verticesInside[i][j], verticesUndecided[i][j]);
			}
		}

		// shutdown OpenGL
		cd->destroy();
		for(int i = 0; i < numberOfObjects; i++) {
			glDeleteBuffers(1, &gpuModels[i].vbo);
			glDeleteBuffers(1, &gpuModels[i].ebo);
		}
		delete[] gpuModels;	
	}

	cd->disableOpenGL();
	delete cd;


	for (int i = 0; i < meshCount; i++) {
		for (int j = i + 1; j < meshCount; j++) {
			MeshSurface *meshA = meshes[i];
			MeshSurface *meshB = meshes[j];

			// rough estimate using bounding cylinder
			if (!OoCylinder::intersectsCapsule(meshes[i]->GetOoCylinder(), meshes[j]->GetOoCylinder())) {				
				continue;
			}

			contextA.mesh = meshA->GetOriginal();
			contextA.points = NULL;
			contextA.fix = fixA;
			contextA.movement = NULL;
			contextA.deformable = meshA->GetDeformable();
			if (this->useGrid) {
				contextA.navigator = meshA->GetNavigator();
			} else {
				contextA.navigator = NULL;
			}
			if(openglAvailable) {
				contextA.verticesInside = verticesInside[j][i];
				contextA.verticesUndecided = verticesUndecided[j][i];
			} else {
				contextA.verticesInside = contextA.verticesUndecided = NULL;
			}

			contextB.mesh = meshB->GetOriginal();
			contextB.points = NULL;
			contextB.fix = fixB;
			contextB.movement = NULL;
			contextB.deformable = meshB->GetDeformable();
			if (this->useGrid) {
				contextB.navigator = meshB->GetNavigator();
			} else {
				contextB.navigator = NULL;
			}
			if(openglAvailable) {
				contextB.verticesInside = verticesInside[i][j];
				contextB.verticesUndecided = verticesUndecided[i][j];
			} else {
				contextB.verticesInside = contextB.verticesUndecided = NULL;
			}

			double intersectedVolume = this->FindAndFixIntersection(&contextA, &contextB);

			if (meshA->GetDeformable()) {
				MeshSurface::AddMeshOffset(meshA->GetOriginal(), fixA);
			}

			if (meshB->GetDeformable()) {
				MeshSurface::AddMeshOffset(meshB->GetOriginal(), fixB);
			}
		}
	}

	// disposing
	PKUtils::DisposeMatrix(&fixA);
	PKUtils::DisposeMatrix(&fixB);

	
	if (openglAvailable) 
	{
		//dealocate memory
		for (int i = 0; i < numberOfObjects; i++) {
			for (int j = 0; j < numberOfObjects; j++) {
				delete[] verticesInside[i][j];
				delete[] verticesUndecided[i][j];			
			}

			delete[] verticesInside[i];
			delete[] verticesUndecided[i];
			delete[] meshesIntersect[i];
		}
		delete[] verticesInside;
		delete[] verticesUndecided;
		delete[] meshesIntersect;		
	}		
}

//////
/// Fixes the intersection of muscles and bones according to the state of both objects - if any motion was provided, 
/// the dynamic intersection is used, otherwise the static one is called
/// @param contextA object A (muscle/bone)
/// @param contextB object B (muscle/bone)
//////
double vtkMEDPolyDataDeformationPK::FindAndFixIntersection(tIntersectionContext *contextA, tIntersectionContext *contextB) {
	if (contextA->movement != NULL || contextB->movement != NULL) {
		return this->FindAndFixIntersectionDynamic(contextA, contextB);
	} else {
		return this->FindAndFixIntersectionStatic(contextA, contextB);
	}
}


//////
/// Fixes the static intersection (intersection without any movement)
/// @param contextA object A (muscle/bone)
/// @param contextB object B (muscle/bone)
//////
double vtkMEDPolyDataDeformationPK::FindAndFixIntersectionStatic( tIntersectionContext *contextA, tIntersectionContext *contextB )
{
#pragma region init locals		
	std::vector<vtkIdType> intersectedVertices[2]; 
	double volume = 0.0;
#pragma endregion init locals
	
#pragma region detect intersections per vertex
	for (vtkIdType side = 0; side < 2; side++) 
	{ 
		tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
		tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

		PKUtils::EraseMatrix(contextActive->fix); 		

		//find the intersection
		FindIntersectingVertices(contextActive, contextPassive, intersectedVertices[side]);
		PKHashTable< vtkIdType, vtkIdType >* cellsA = GetIntersectionSurfaceCells(contextActive->mesh, intersectedVertices[side]);
		
		// calculate the volume in the intersection
		volume += CalculateIntersectedVolume(contextActive->mesh, contextActive->points, cellsA);
		delete cellsA;
	}
#pragma endregion detect intersections per vertex

#pragma region fix static solution
	std::vector<Vector3> centers[2]; // centers of intersected groups
	std::vector<Vector3> centersDirections[2]; // direction of the group (average normal of all triangles in the group)
#define vectorListIds std::vector< std::vector<vtkIdType> >
	double temp[3];
	vectorListIds groups[2]; // groups of intersected vertices (contextA to contextB and vise versa)

#pragma region group to areas
	for (int side = 0; side < 2; side++) {
		tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
		tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

		int vertACount = intersectedVertices[side].size(); // number of intersected vertices
		bool* vertChecked = new bool[vertACount]; // for each intersected vertex one array cell
		memset(vertChecked, 0, sizeof(bool) * vertACount);

#pragma region create basic groups 
		int m = 0;
		// for each intersected vertex...
		for (std::vector<vtkIdType>::iterator itMain = intersectedVertices[side].begin(); itMain != intersectedVertices[side].end(); itMain++, m++) {
			if (vertChecked[m]) { // ignore checked vertices
				continue;
			}

			std::vector<vtkIdType> group;

			// insert first
			group.push_back(*itMain); // insert vertex into the group
			vertChecked[m] = true;    // marked it as checked

			// find all neighbours
			int n = 0;
			for (std::vector<vtkIdType>::iterator itFollow = intersectedVertices[side].begin(); itFollow != intersectedVertices[side].end(); itFollow++, n++) {
				if (vertChecked[n]) { // ignore checked vertices (they already have their neighbours)
					continue;
				}

				// tests each intersected vertex with all vertices placed in the list after it (with non-checked vertices)

				// test against any current in area	// 
				for (std::vector<vtkIdType>::iterator itOld = group.begin(); itOld != group.end(); itOld++) {
					// itFollow - vertex, which is not in the group yet
					// itOld - vertex, that is already in the group (is checked)
					if (contextActive->mesh->IsEdge(*itOld, *itFollow)) { // if exists edge between non-checked vertex and vertex in the group?
						group.push_back(*itFollow); // yes -> checked the vertex and add it into the group
						vertChecked[n] = true;
						break;
					}
				} // after the i-th pass all points, which are in group with vertex i, are checked
				// if some vertex is in any group, it is skipped
			}

			groups[side].push_back(group);
		}
		delete[] vertChecked;
#pragma endregion create basic groups

#pragma region union joined groups 
		while (true) {
			bool changed = false;

			// area vs area
			for (vectorListIds::iterator itA = groups[side].begin(); itA != groups[side].end(); itA++) {
				for (vectorListIds::iterator itB = itA + 1; itB != groups[side].end(); itB++) {
					bool doJoin = false;

					// vertices vs vertices
					for (std::vector<vtkIdType>::iterator itVA = itA->begin(); itVA != itA->end(); itVA++) {
						for (std::vector<vtkIdType>::iterator itVB = itB->begin(); itVB != itB->end(); itVB++) {
							if (contextActive->mesh->IsEdge(*itVA, *itVB)) {
								doJoin = true;
								break;
							}
						}
					}

					if (doJoin) {
						// copy vertices from group B to A
						for (std::vector<vtkIdType>::iterator itVB = itB->begin(); itVB != itB->end(); itVB++) {
							itA->push_back(*itVB);
						}

						// remove group B
						groups[side].erase(itB); // WARNING: VERY BAD WITH VECTOR as container

						changed = true;
						break;
					}
				}

				if (changed) {
					break;
				}
			}

			if (!changed) {
				break;
			}
		}
#pragma endregion union joined groups

#pragma region calculate centers
		int centerIndex = 0;
		for (vectorListIds::iterator it = groups[side].begin(); it != groups[side].end(); it++) {
			Vector3 center; //center of the group
			Vector3 centerDirection; // direction of the group (average normal of all triangles in the group)
			centerDirection.data[0] = 0;
			centerDirection.data[1] = 0;
			centerDirection.data[2] = 0;

			for (std::vector<vtkIdType>::iterator itVert = it->begin(); itVert != it->end(); itVert++) { // for all vertices of the group
				myGetPointDeep(contextActive->mesh, contextActive->points, *itVert, temp); 
				PKUtils::AddVertex(center.data, temp, center.data); // computes center point (by adding positions of all vertices)

				/* added by HAJ */
				double normal[3] = {0,0,0};
				
				unsigned short nFanCells; vtkIdType* pFanCellsIds;
				contextActive->mesh->GetPointCells(*itVert, nFanCells, pFanCellsIds); // all cells neighbouring with the vertex				
				for (int j = 0; j < nFanCells; j++) { // get the normal of the cell and add it to the total normal
					PKMath::GetTriangleNormal(contextActive->mesh, contextActive->points, pFanCellsIds[j], temp);
					PKUtils::DivideVertex(temp, PKUtils::CalculateVertexLength(temp));
					PKUtils::AddVertex(normal, temp, normal);
				}
				PKUtils::DivideVertex(normal, PKUtils::CalculateVertexLength(normal)); // normalize the total normal
				PKUtils::AddVertex(normal, centerDirection.data, centerDirection.data);
			}

			PKUtils::DivideVertex(center.data, it->size()); // final center coordinates
			PKUtils::DivideVertex(centerDirection.data, it->size()); // final center coordinates
			centers[side].push_back(center); // saving center position into the group
			centersDirections[side].push_back(centerDirection); // saving center direction

		}
#pragma endregion calculate centers
	}

#pragma endregion group to areas

#pragma region match areas and find fixes
	bool* matched[2]; // for each group center one boolean (checked), 2 rows of the array can be differently long
	for (int side = 0; side < 2; side++) {
		matched[side] = new bool[centers[side].size()];
		memset(matched[side], 0, sizeof(bool) * centers[side].size());
	}


	PKMatrix* distancesSq = PKUtils::CreateMatrix(centers[0].size() > 0 ? centers[0].size() : 1, centers[1].size() > 0 ? centers[1].size() : 1); // matrix of group centers distances

	// measure distances
	int m = 0;
	for (std::vector<Vector3>::iterator itA = centers[0].begin(); itA != centers[0].end(); itA++, m++) {
		int n = 0;
		for (std::vector<Vector3>::iterator itB = centers[1].begin(); itB != centers[1].end(); itB++, n++) {
			PKUtils::SubtractVertex(itB->data, itA->data, temp);
			double distSq = PKUtils::CalculateVertexLengthSq(temp);
			distancesSq->values[m][n] = distSq; // distances of groups centers among each other
		}
	}

#pragma region fix matching groups
	// pick nearest
	int pairCount = min(centers[0].size(), centers[1].size()); // object with less intersected groups

	for (int pair = 0; pair < pairCount; pair++) { 


		double minDistSq = DBL_MAX;
		int minI, minJ;

		for (int i = 0; i < distancesSq->height; i++) { // for each group of the selected object the nearest group from the other object is selected
			if (matched[0][i]) {
				continue;
			}

			for (int j = 0; j < distancesSq->width; j++) {
				if (matched[1][j]) {
					continue;
				}

				if (minDistSq > distancesSq->values[i][j]) {
					minDistSq = distancesSq->values[i][j];
					minI = i;
					minJ = j;
				}
			}
		}

		// too far => not relevant
		if (minDistSq > 10000) {
			break;
		}

		// note
		matched[0][minI] = true; // both groups are marked as matched (coupled)
		matched[1][minJ] = true;

		// do fix - vertices which have their partner in the other object
		for (int side = 0; side < 2; side++) {
			tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
			tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

			if (!contextActive->deformable) {
				continue;
			}

			double dir[3];
			PKUtils::SubtractVertex(centers[1][minJ].data, centers[0][minI].data, dir); // distance of two centres
			PKUtils::DivideVertex(dir, PKUtils::CalculateVertexLength(dir)); // dir ... vector going from active to passive object (normalized)
			if (side == 1) {
				PKUtils::MultiplyVertex(dir, -1);
			}

			int localArea = side == 0 ? minI : minJ;


			double *dirCenter = centersDirections[side][localArea].data; // direction of the group from the active object
			// all vertices from the group
			for (std::vector<vtkIdType>::iterator it = groups[side][localArea].begin(); it != groups[side][localArea].end(); it++) {
				double fn = vtkMath::Dot(dirCenter, dir);
				/* added by HAJ */
				// if directions of active object group center and the active to passive direction are almost orthogonal (angles between cca 78-102° are ignored),
				// ignore that couple od groups and use the non-group solving

				/*HAJ - comment to get the old version*/
				if (abs(fn) < DIRECTION_ANGLE_LIMIT) { 
					matched[side][localArea] = false; 
					continue;
				}

				// take the point, save its data for visualization
				myGetPointDeep(contextActive->mesh, contextActive->points, *it, temp);
				DirDesc pointVisualizationData;
				PKUtils::CopyVertex(temp, pointVisualizationData.point.data);
				PKUtils::CopyVertex(dir, pointVisualizationData.dir.data);

				// compute the distance of the point to the nearest triangle of the passive mesh in the dir direction				
				double bestT;				
				if (!PKMath::FindIntersectionInDirection(contextPassive->mesh, contextPassive->points, 
					temp, dir, ALLOWED_INTERSECTION_MM, contextPassive->navigator, NULL, &bestT)) 
				{
					continue;	//no intersection has been found (within the limit ALLOWED_INTERSECTION_MM)
				}

				// do not go too far (if both objects are muscles)
				if (contextPassive->deformable) {
					bestT *= 0.6;
				}

				pointVisualizationData.color.x = 0.5; // visualize such points with the violet color
				pointVisualizationData.color.y = 0.5;
				pointVisualizationData.color.z = 1.0;
				pointVisualizationData.bestT = bestT;
				dirTemp.push_back(pointVisualizationData);

				PKUtils::MultiplyVertex(dir, bestT); // compute the vector for moving the point in a given direction about the distance of bestT

				// fix the intersection
				PKUtils::CopyVertex(dir, contextActive->fix->values[*it]); // fix changes
				PKUtils::DivideVertex(dir, PKUtils::CalculateVertexLength(dir)); // 
			}
		}
	} // all vertices which have their partner in the other object are moved
#pragma endregion fix matching groups

#pragma region fix unmatched groups
	// fix the rest (those which cant be matched because of distance, angle or dispairity of count)
	for (int side = 0; side < 2; side++) {
		tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
		tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

		if (!contextActive->deformable) {
			continue;
		}

		double dir[3] = {0,0,0};

		// all remaining groups
		int groupId = 0;

		// for all vertices of the active part
		for (vectorListIds::iterator it = groups[side].begin(); it != groups[side].end(); it++, groupId++) { // for all groups
			if (matched[side][groupId]) { //solved centers are ingnored
				continue;
			}

			// all vertices of group (groups, which were not fixed yet)
			for (std::vector<vtkIdType>::iterator itV = it->begin(); itV != it->end(); itV++) {
				/* added by HAJ */
				// direction of the whole group
				/*HAJ - comment to get the old version*/
				dir[0] = centersDirections[side][groupId].x;
				dir[1] = centersDirections[side][groupId].y;
				dir[2] = centersDirections[side][groupId].z;

				// fix
				myGetPointDeep(contextActive->mesh, contextActive->points, *itV, temp);

				PKUtils::MultiplyVertex(dir, -1); // direction of the group leads outside from the object (we need the direction of reparation - into the centre of the object)

				// compute the distance of the point to the nearest triangle of the passive mesh in the dir direction
				double bestT;				
				if (!PKMath::FindIntersectionInDirection(contextPassive->mesh, contextPassive->points, 
					temp, dir, ALLOWED_INTERSECTION_MM, contextPassive->navigator, NULL, &bestT) ||
					bestT > 40) //HAJ: hack!!! 
				{
					continue;	//no intersection has been found
				}

				// do not go too far if two muscled next to each other are solved
				if (contextPassive->deformable) {
					bestT *= 0.6;
				}

				PKUtils::MultiplyVertex(dir, bestT); // compute the vector for moving the point in a given direction about the distance of bestT

				// fix the intersection
				PKUtils::CopyVertex(dir, contextActive->fix->values[*itV]);

				
				// data for debug visualization
				if (this->GetDebugMode())
				{
					DirDesc pointVisualizationData;
					PKUtils::CopyVertex(temp, pointVisualizationData.point.data);
					PKUtils::CopyVertex(dir, pointVisualizationData.dir.data);
					pointVisualizationData.color.x = 1.0; // visualize such points vith the magenta color
					pointVisualizationData.color.y = 0.5;
					pointVisualizationData.color.z = 1.0;
					pointVisualizationData.bestT = bestT;
#pragma omp critical
					dirTemp.push_back(pointVisualizationData);
				}
			}

			matched[side][groupId] = true;
		}
	}
#pragma endregion fix unmatched groups

	PKUtils::DisposeMatrix(&distancesSq);

	for (int side = 0; side < 2; side++) {
		delete[] matched[side];
	}
#pragma endregion match areas and find fixes

#pragma endregion fix static solution

	return volume;
}

//////
/// Fixes the dynamic intersection (intersection after providing of some movement)
/// @param contextA object A (muscle/bone)
/// @param contextB object B (muscle/bone)
//////
double vtkMEDPolyDataDeformationPK::FindAndFixIntersectionDynamic( tIntersectionContext *contextA, tIntersectionContext *contextB )
{
#pragma region init locals
	std::vector<vtkIdType> intersectedVertices[2]; 
	double volume = 0;	
#pragma endregion init locals

#pragma region detect intersections per vertex
	for (vtkIdType side = 0; side < 2; side++) 
	{ 
		tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
		tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

		PKUtils::EraseMatrix(contextActive->fix); 		

		if (!contextActive->deformable) {	// intersection is computed only if the contectActive is deformable
			continue;
		}

		//find the intersection
		FindIntersectingVertices(contextActive, contextPassive, intersectedVertices[side]);
		PKHashTable< vtkIdType, vtkIdType >* cellsA = GetIntersectionSurfaceCells(contextActive->mesh, intersectedVertices[side]);
		
		// calculate the volume in the intersection
		volume += CalculateIntersectedVolume(contextActive->mesh, contextActive->points, cellsA);
		delete cellsA;
	}
#pragma endregion detect intersections per vertex

#pragma region fix intersections per vertex
	for (vtkIdType side = 0; side < 2; side++) 
	{ 
		tIntersectionContext *contextActive = side == 0 ? contextA : contextB;
		tIntersectionContext *contextPassive = side == 0 ? contextB : contextA;

		if (!contextActive->deformable || contextActive->movement == NULL) { 
			//if the contextActive is not deformable, i.e., it is a bone, or no movement was made, there is nothing to to fix and we are ready
			continue;
		}

		// test all vertices		
#pragma omp parallel shared(intersectedVertices, contextActive, contextPassive, cdTests, cdErrors)
		{			
			int nPoints = (int)intersectedVertices[side].size();
#pragma omp for schedule(guided)	//each threat will process some iterations
			for (int i = 0; i < nPoints; i++)
			{
				vtkIdType ptIndex = intersectedVertices[side][i];

				double point[3], dir[3];
				contextActive->mesh->GetPoint(ptIndex, point);
				PKUtils::CopyVertex(contextActive->movement->values[ptIndex], dir); // direction of motion of the i-th vertex				
				double maxT = PKUtils::CalculateVertexLength(dir);
				PKUtils::DivideVertex(dir, -maxT); // normalize and reverse the direction
				
				// compute the distance of the point to the nearest triangle of the passive mesh in the dir direction
				//double bestT = MeshSurface::FindIntersectionInDirection(point, dir, contextPassive->mesh, contextPassive->points, NULL, cell); 
				double bestT;				
				if (!PKMath::FindIntersectionInDirection(contextPassive->mesh, contextPassive->points, 
					point, dir, ALLOWED_INTERSECTION_MM, contextPassive->navigator, NULL, &bestT)) 
				{
					continue;	//no intersection has been found
				}

				// do not go too far if two muscles are processed
				if (contextPassive->deformable) {
					bestT *= 0.6;
				}
				bestT = min(maxT, bestT); // do not move further than the motion was made

				// fix the intersection
				PKUtils::MultiplyVertex(dir, bestT); // vector of reparation
				PKUtils::CopyVertex(dir, contextActive->fix->values[ptIndex]); 

				/* added by HAJ : debug support*/
				if (this->GetDebugMode())
				{
					DirDesc pointVisualizationData; // data for processed point visualization
					PKUtils::CopyVertex(point, pointVisualizationData.point.data);
					PKUtils::CopyVertex(dir, pointVisualizationData.dir.data);
					pointVisualizationData.color.x = 1.0; // points fixed by dunamic part are figured by yellow color
					pointVisualizationData.color.y = 1.0;
					pointVisualizationData.color.z = 0.5;
					pointVisualizationData.bestT = bestT;
#pragma omp critical
					dirTemp.push_back(pointVisualizationData);
				}
			} //end for (i = 0; i < nPointsA; i++)
		} //#pragma omp parallel private

	} //end for both sides (contextA -> contextB and contextB -> contextA)
#pragma endregion fix intersections per vertex

	return volume;
}


//------------------------------------------------------------------------
//Finds the vertices of contextActiveMesh mesh that lies inside the volume of the contextPassive mesh 
//Indices of the detected vertices are stored into the output array
void vtkMEDPolyDataDeformationPK::FindIntersectingVertices(const tIntersectionContext *contextActive, const tIntersectionContext *contextPassive, 
														   std::vector<vtkIdType>& intersectedVertices)
{
	intersectedVertices.clear();	//make sure there is nothing in the output buffer

	// test all vertices		
#pragma omp parallel shared(contextActive, contextPassive, cdTests, cdErrors)
	{
		//this block runs in parallel with contextActive, contextPassive variable being shared by all threads
		//IMPORTANT NOTE: access to these variables is not automatically synchronized and as these are not marked as volatile
		//changing values of these require flush (fortunately, we do not modify them)			
		//cdTests and cdErrors variables are also shared but these are volatile and atomically incremented

		vtkIdType nPointsA = contextActive->mesh->GetNumberOfPoints();
		vtkIdType nPointsB = contextPassive->mesh->GetNumberOfPoints();

		vtkIdType nCellsA = contextActive->mesh->GetNumberOfCells();
		vtkIdType nCellsB = contextPassive->mesh->GetNumberOfCells();

#pragma omp for schedule(guided)	//each threat will process some iterations
		for (int i = 0; i < nPointsA; i++) 
		{
			int pos = i / 32;
			int shift = i % 32;

#pragma omp atomic
			this->cdTests++;

			bool inside = false;
			bool undecided = true;
			if(contextActive->verticesInside != NULL && contextActive->verticesUndecided != NULL) {
				unsigned int mask = 1u << shift;
				unsigned int value = contextActive->verticesInside[pos];
				inside = (value & mask) == mask;
				value = contextActive->verticesUndecided[pos];
				undecided = (value & mask) == mask;
			}

			//shift++;
			//if(shift == 32) {
			//	shift = 0;
			//	pos++;
			//}

#pragma region check spin
			double point[4];
			double direction[3];
			direction[0] = 0.86602540378443864676372317075294;//sqrt(3.0) / 2;
			direction[1] = 1.4142135623730950488016887242097; //sqrt(2.0);
			direction[2] = 1.1180339887498948482045868343656; //sqrt(5.0) / 2;

			myGetPointDeep(contextActive->mesh, contextActive->points, i, point);

			// inside test
			int spin;
			if(undecided) {
				spin = PKMath::CalculatePointSpin(contextPassive->mesh, contextPassive->points, point, direction, 0.000001, contextPassive->navigator);
			}
			else {
				spin = inside ? 1 : 2;
			}

			if (spin % 2 == 0) {
				//outside
				continue;
			}

#pragma omp critical
			{
				if(!inside && !undecided) {
					this->cdErrors++;
				}

				// remeber point  
				intersectedVertices.push_back(i); 										
			} //end omp critical
#pragma endregion check spin
		} //end for --- parallel
	} //end omp parallel
}


//------------------------------------------------------------------------
//Returns cells of the input mesh that have at least one vertex from intersectedVertices.
//The method retrieves a fan of triangles around every vertex from the input array intersectedVertices and stores their cell ids
//into the return buffer. Any duplicity is automatically filtered out, i.e., it is guaranteed that the returned triangles are unique.
//This method is useful (and designed) to return cells of the mesh that lie in the volume of another mesh. Vertices that lie in the
//volume of another mesh are typically obtained by FindIntersectingVertices method.
//N.B.: the caller is responsible for deletion of the returned object once it is no longer needed. 
PKHashTable<vtkIdType, vtkIdType>* vtkMEDPolyDataDeformationPK::GetIntersectionSurfaceCells(
	vtkPolyData* mesh, const std::vector<vtkIdType>& intersectedVertices)
{
	PKHashTable<vtkIdType, vtkIdType>* cellsA = new PKHashTable<vtkIdType, vtkIdType>();

	int nPoints = (int)intersectedVertices.size();
	for (int i = 0; i < nPoints; i++)
	{
		unsigned short nFanCells; vtkIdType* pFanCellsIds;
		mesh->GetPointCells(intersectedVertices[i], nFanCells, pFanCellsIds); // takes all cells with the point				
		for (int j = 0; j < nFanCells; j++) { // for each cell					
			cellsA->Add(pFanCellsIds[j], pFanCellsIds[j]); // add the cell into the list of intersected area
		}
	}

	return cellsA;
}

//////
/// Calculates the oriented volume of intersected part of the mesh identified by cellsA.
/// @param polyData information about topology of mesh and optionally coordinates of vertices (input)
/// @param points array of actual point coordinates to use instead of coords in mesh or NULL to use data in polyData (input)
/// @param cellsA contains cells of the input mesh that lie inside the volume of the other mesh
/// @return oriented volume of mesh (of the part  identified by intersectedVertices)
/// The method computes the oriented volume of the object composed of tetrahedras, 
/// each of them formed from one triangle of the surface that intersects and point (0,0,0). 
//////
double vtkMEDPolyDataDeformationPK::CalculateIntersectedVolume(vtkPolyData* mesh, PKMatrix* points, const PKHashTable<vtkIdType, vtkIdType>* cellsA)
{		
	//now calculate the volume
	double volume = 0.0;
	const PKHashTableRecord<vtkIdType, vtkIdType>* cellsAValues = cellsA->GetValuesRef();
	int cellACount = cellsA->GetCount();
	for (int cellsAIndex = 0; cellsAIndex < cellACount; cellsAIndex++) 
	{			
		vtkIdType id = cellsAValues[cellsAIndex].value;
		vtkIdType nCellPoints, *pCellPtsIds;
		mesh->GetCellPoints(id, nCellPoints, pCellPtsIds);

		//check, if it is triangle?
		_VERIFY_CMD(nCellPoints == 3, continue);

		double a[3], b[3], c[3], temp[3];
		myGetPointDeep(mesh, points, pCellPtsIds[0], a);
		myGetPointDeep(mesh, points, pCellPtsIds[1], b);
		myGetPointDeep(mesh, points, pCellPtsIds[2], c);

		vtkMath::Cross(a, b, temp);
		volume += vtkMath::Dot(temp, c);
	}

	return volume;
}

#pragma endregion // intersections

#pragma region // solving

//#define _DEBUG_PROCESS

#if defined(_DEBUG_OUTPUT_PKMATRIX) || defined(_DEBUG_PROCESS)
#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkSTLWriter.h"
#include "vtkMAFSmartPointer.h"
#pragma warning(pop)
#endif

double diffclock(clock_t clock1, clock_t clock2) {
	double diffticks = clock1 - clock2;
	double diffms = (diffticks * 1000) / CLOCKS_PER_SEC;
	return diffms;
}

//////
/// Minimizes linear system with hard constraint using Gauss-Newton iterative method with Lagrange multiplicators.
/// Uses modified approach from Huang 2006, Subspace Gradient Domain Mesh Deformation
/// @param matrixL matrix L of linear system (input)
/// @param vectorB vector of right sides of system (input)
/// @param solutionX pointer to vector of starting X (input) and then solution (output) - must be preallocated and should be prefilled (input and output)
//////
bool vtkMEDPolyDataDeformationPK::FindEnergyMinimumGN(PKMatrix **matricesL, PKMatrix **verticesB, PKMatrix ***solutionsX)
{
	double timeSlot[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	//this->preventCollisions = false;
	ofstream myfile;
	myfile.open("C:/Temp/times.txt", ios::out);
	myfile << "START" << endl;
	myfile.close();

	clock_t start = clock();

#pragma region INITIALIZATION	
	int iterNum = 0;
	double lastDiff = 9E99;
	double volumeDiff = 0;
	double alpha = 0.1;
	bool onFullMesh = false;
	bool isPrecisionFinal = false;

	bool failed = false;
	int modelCount = this->modelCount;
	PKSolutionState **states = new PKSolutionState*[modelCount];
#pragma region Initialization of deformation state
	for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
		PKMatrix *matrixL = matricesL[modelIndex];
		PKMatrix *vectorB = verticesB[modelIndex];

		int matrixHeight = matrixL->height;
		int matrixWidth = matrixL->width;
		int numberOfDimensions = vectorB->width;

		if (matrixWidth > matrixHeight)
		{
			throw "System is underconstrained.";
		}

		/* allocations */
		PKSolutionState *state = new PKSolutionState(matrixHeight, matrixWidth, numberOfDimensions,
			this->meshes[modelIndex]->GetOriginal()->GetNumberOfPoints());

		/* precalculations */
		state->solutionK = (*solutionsX)[modelIndex];
		state->originalVolume = this->CalculateVolume(this->meshes[modelIndex]->GetOriginal(), NULL);
		state->originalVolumeCoarse = this->CalculateVolume(this->meshes[modelIndex]->GetCoarse(), NULL);
		state->coarseToOrigVolRatio = state->originalVolumeCoarse / state->originalVolume;

		PKUtils::TransposeMatrix(matrixL, state->Lt); // Lt
		PKUtils::MultiplyMatrices(state->Lt, matrixL, state->LtL); // LtL

		if (!PKUtils::InvertMatrix(state->LtL, state->LtLInv)) // LtL-1
		{
			vtkErrorMacro(<< "Cannot invert LtL. Probably singular or pseudosingular.\n");

#ifdef _DEBUG_OUTPUT_PKMATRIX
			matrixL->DebugOutput("matrixL");
			vectorB->DebugOutput("vectorB");
			state->Lt->DebugOutput("state->Lt");
			state->LtL->DebugOutput("state->LtL");
#endif

#ifdef _DEBUG_PROCESS
			vtkMAFSmartPointer< vtkSTLWriter > stlw;
			stlw->SetInput(this->meshes[modelIndex]->GetOriginal());
			stlw->SetFileName("original.stl");
			stlw->SetFileTypeToBinary();
			stlw->Update();

			stlw->SetInput(this->meshes[modelIndex]->GetCoarse());
			stlw->SetFileName("coarse.stl");
			stlw->SetFileTypeToBinary();
			stlw->Update();

			stlw->SetInput(NULL);
#endif
			failed = true;
		}

		states[modelIndex] = state;
	} //end for models
#pragma endregion Initialization of deformation state

#pragma region Initialization of penetreation check mechanism
	/* intersection pairs allocation */
	PKMatrix*** intersectionFixes = NULL;
	tIntersectionContext contextA;
	tIntersectionContext contextB;

	if (this->preventCollisions) {
		intersectionFixes = new PKMatrix**[modelCount + this->obstacleCount];

		for (int i = 0; i < modelCount + this->obstacleCount; i++) {
			intersectionFixes[i] = new PKMatrix*[modelCount + this->obstacleCount];

			int matrixWidth = i < modelCount ? matricesL[i]->width : this->meshes[i]->GetCoarse()->GetNumberOfPoints();
			int numberOfDimensions = i < modelCount ? verticesB[i]->width : 3;

			for (int j = 0; j < modelCount + this->obstacleCount; j++) {
				if (i == j) {
					continue;
				}

				// contains repair vertices for i-th mesh to avoid collision with j-th mesh
				intersectionFixes[i][j] = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
			}
		}
	}
#pragma endregion Initialization of penetreation check mechanism
#pragma endregion INITIALIZATION

	clock_t end = clock();
	timeSlot[0] += diffclock(end, start);

	myfile.open("C:/Temp/times.txt", ios::out | ios::app);
	myfile << "initialization: " << timeSlot[0] << "ms" << endl;
	myfile.close();

	start = clock();

#pragma region STATIC PENETRATION
	if (this->preventCollisions) 
	{
		if (this->debugMode) {
			Debug_Visualize_Progress(iterNum, lastDiff, states, NULL); //1
		}

		// create OOC
		for (int modelIndex = 0; modelIndex < modelCount + this->obstacleCount; modelIndex++) {
			// per model variables
			OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
			MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

			//BES: 31.10.2011 - what should be here????
			ooc->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL);

			if (this->useGrid) {
				navigator ->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL, ooc);
			}
		}

#if MIN_ITERS_BEFORE_INTERSECTION == 0
		// solve static intersections
		for (int i = 0; i < modelCount + this->obstacleCount; i++) { 
			for (int j = i + 1; j < modelCount + this->obstacleCount; j++) {
				PKMatrix *fixA = intersectionFixes[i][j];
				PKMatrix *fixB = intersectionFixes[j][i];

				OoCylinder *oocA = this->meshes[i]->GetOoCylinder();
				OoCylinder *oocB = this->meshes[j]->GetOoCylinder();

				// rough estimate using bounding cylinder (if the bonding cylinders do not intersect, ignore both objects
				if (!OoCylinder::intersectsCapsule(oocA, oocB)) {
					PKUtils::EraseMatrix(fixA);
					PKUtils::EraseMatrix(fixB);
					continue;
				}

				contextA.mesh = this->meshes[i]->GetCoarse();
				contextA.fix = fixA;
				contextA.movement = NULL;
				if (this->useGrid) {
					contextA.navigator = this->meshes[i]->GetNavigator();
				} else {
					contextA.navigator = NULL;
				}
				contextA.deformable = this->meshes[i]->GetDeformable();
				if (contextA.deformable) {
					contextA.points = states[i]->solutionK;
				}
				else {
					contextA.points = NULL;
				}

				contextB.mesh = this->meshes[j]->GetCoarse();
				contextB.fix = fixB;
				contextB.movement = NULL;
				if (this->useGrid) {
					contextB.navigator = this->meshes[j]->GetNavigator();
				} else {
					contextB.navigator = NULL;
				}
				contextB.deformable = this->meshes[j]->GetDeformable();
				if (contextB.deformable) {
					contextB.points = states[j]->solutionK;
				}
				else {
					contextB.points = NULL;
				}
				/* cmolik: static colision detection is off. Why???
				double intersectedVolume = this->FindIntersection(&contextA, &contextB);

				if (i < modelCount) {
				PKUtils::AddMatrices(states[i]->solutionK, fixA, states[i]->solutionK);
				}

				if (j < modelCount) {
				PKUtils::AddMatrices(states[j]->solutionK, fixB, states[j]->solutionK);
				}
				*/
			}
		}
#endif
	} //end if [this->preventCollisions]
#pragma endregion STATIC PENETRATION

	end = clock();
	timeSlot[1] += diffclock(end, start);

	myfile.open("C:/Temp/times.txt", ios::out | ios::app);
	myfile << "Static penetration: " << timeSlot[1] << "ms" << endl;
	myfile.close();

#pragma region MAIN ITERATIVE LOOP
#ifdef _DEBUG_PROCESS
	double volRat;
	bool bCorseMeshProcessing;
#endif

	while (!failed)
	{
		iterNum++;

		myfile.open("C:/Temp/times.txt", ios::out | ios::app);
		myfile << "ITERATION: " << iterNum << endl;
		myfile.close();

		if (this->debugMode) {
			Debug_Visualize_Progress(iterNum, lastDiff, states, &dirTemp);//2
		}

#ifdef _DEBUG_PROCESS
		vtkMAFSmartPointer< vtkSTLWriter > stlw;
		stlw->SetFileTypeToBinary();

		char bufname[32];

		if (iterNum == 1)
		{
			for (int obstacleIndex = modelCount; obstacleIndex < modelCount + this->obstacleCount; obstacleIndex++)
			{
				stlw->SetInput(this->meshes[obstacleIndex]->GetOriginal());
#if _MSC_VER >= 1400 // grater than vs 2005
				sprintf_s<32>(bufname, "BN_ORIG_#000_%d.stl", obstacleIndex);
#else
				sprintf(bufname, "BN_ORIG_#000_%d.stl", obstacleIndex);
#endif

				stlw->SetFileName(bufname);
				stlw->Update();

				stlw->SetInput(this->meshes[obstacleIndex]->GetCoarse());

#if _MSC_VER >= 1400 // grater than vs 2005
				sprintf_s<32>(bufname, "BN_COAR_#000_%d.stl", obstacleIndex);
#else
				sprintf(bufname, "BN_COAR_#000_%d.stl", obstacleIndex);
#endif

				stlw->SetFileName(bufname);
				stlw->Update();
			}
		}

		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++)
		{
			vtkPolyData* poly = this->meshes[modelIndex]->GetCoarse()->NewInstance();
			poly->DeepCopy(this->meshes[modelIndex]->GetCoarse());

			this->meshes[modelIndex]->SetUpPointCoords(poly, states[modelIndex]->solutionK);

			stlw->SetInput(poly);

#if _MSC_VER >= 1400 // grater than vs 2005
			sprintf_s<32>(bufname, "MU_COAR_#%03d_%d.stl", iterNum, modelIndex);
#else
			sprintf(bufname, "MU_COAR_#%03d_%d.stl", iterNum, modelIndex);
#endif

			stlw->SetFileName(bufname);
			stlw->Update();

			poly->Delete();
		}
#endif

		// sequential decreasing of step length enhances precision in later phases
		// and speeds up first phases
		if (lastDiff > 5 * SOLUTION_EPSILON)
		{
			alpha = min(0.5, alpha);
		}
		else if (lastDiff > 1.5 * SOLUTION_EPSILON)
		{
			alpha = min(0.1, alpha);
		}
		else
		{
			alpha = min(0.025, alpha);
			isPrecisionFinal = true;
		}

		// switch to full mesh
		if (lastDiff <= 5 * SOLUTION_EPSILON || iterNum > MAX_ITER_ON_COARSE) {
			onFullMesh = true;
			//this->debugMode = true;
		}

		double maxDiff = 0;

		start = clock();

		// step calculation
#pragma region STEP CalculationdirTemp
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) 
		{
			// per model variables
			PKSolutionState *state = states[modelIndex];
			PKMatrix *matrixL = matricesL[modelIndex];
			PKMatrix *vectorB = verticesB[modelIndex];
			state->volumeLostByFix = 0;

			/* volume measurement */
			if (onFullMesh)
			{
				// precise measurement of volume error in the end (coords distribution takes a lot of time)
				this->meshes[modelIndex]->ApplyCoarseCoordsToOriginalMesh(state->solutionK, state->bufferHighResMatrix);
				double currentVolume = this->CalculateVolume(this->meshes[modelIndex]->GetOriginal(), NULL);
				volumeDiff = state->originalVolume - currentVolume;
				volumeDiff *= state->coarseToOrigVolRatio;

#ifdef _DEBUG_PROCESS
				volRat = 100 * fabs(currentVolume / state->originalVolume);	//volume may be negative!
				bCorseMeshProcessing = false;
#endif
			}
			else
			{
				// rough estimation of volume error in the beginning
				double currentVolume = this->CalculateVolume(this->meshes[modelIndex]->GetCoarse(), state->solutionK);
				volumeDiff = state->originalVolumeCoarse - currentVolume;

#ifdef _DEBUG_PROCESS
				volRat = 100 * fabs(currentVolume / state->originalVolumeCoarse);	//volume may be negative!
				bCorseMeshProcessing = true;
#endif
			}

			/* calculate jacobian of nonlinear conditions which is ultimately just a gradient of volume function */
			this->CalculateVolumeJacobiMatrixTransposed(this->meshes[modelIndex]->GetCoarse(), state->solutionK, state->JgT);  // JgT

			if (this->preventCollisions && iterNum > MIN_ITERS_BEFORE_INTERSECTION) {
				// prevent repeating intersections by volume correction
				if (volumeDiff > 0) {
					for (int i = 0; i < state->JgT->height; i++) {
						if (state->blockedVertices[i]) {
							state->JgT->values[i][0] = state->JgT->values[i][1] = state->JgT->values[i][2] = 0;
						}
					}
				}
			}

			PKUtils::TransposeMatrix(state->JgT, state->Jg);												 // Jg
			PKUtils::LinearizeMatrix(state->Jg, state->JgLin);

			/* common precalculations */
			PKUtils::MultiplyMatrices(matrixL, state->solutionK, state->Lx_minus_b);						// Lx
			PKUtils::SubtractMatrices(state->Lx_minus_b, vectorB, state->Lx_minus_b);						// Lx - b
			PKUtils::MultiplyMatrices(state->Lt, state->Lx_minus_b, state->Lt_dot_Lx_minus_b);					// Lt * (Lx - b)

			//char buf1[50] = { 0 };
			//sprintf(buf1, "C:/Temp/Lt_dot_Lx_minus_b_cpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->Lt_dot_Lx_minus_b, buf1);

			/* calculate lambda */
			PKUtils::MultiplyMatrices(state->Jg, state->LtLInv, state->Jg_LtLInv);								// Jg * (LtL)-1

			PKUtils::LinearizeMatrix(state->Jg_LtLInv, state->Jg_LtLInvLin);
			PKUtils::LinearizeMatrix(state->Lt_dot_Lx_minus_b, state->Lt_dot_Lx_minus_b_Lin);

			double rightSideOfLambdaLin = PKUtils::DotN(state->Jg_LtLInvLin->values[0], state->Lt_dot_Lx_minus_b_Lin->values[0], state->Lt_dot_Lx_minus_b_Lin->width);
			// Jg * (LtL)-1 * Lt * (Lx - b)
			double Jg_LtLInv_JgT_Lin = PKUtils::DotN(state->Jg_LtLInvLin->values[0], state->JgLin->values[0], state->JgLin->width);
			// Jg * (LtL)-1 * JgT
			double lambda = - (1 / Jg_LtLInv_JgT_Lin) * (volumeDiff / alpha - rightSideOfLambdaLin);
			// lambda = -[Jg * (LtL)-1 * JgT]-1 * {g - [Jg * (LtL)-1 * Lt * (Lx - b)]}

			//ofstream myfile;
			//char buf2[50] = { 0 };
			//sprintf(buf2, "C:/Temp/lambda_cpu_%u_%u.txt", iterNum, modelIndex);
			//myfile.open(buf2);
			//myfile << "rightSideOfLambdaLin: " << rightSideOfLambdaLin << endl;
			//myfile << "Jg_LtLInv_JgT_Lin: " << Jg_LtLInv_JgT_Lin << endl;
			//myfile << "Lambda: " << lambda << endl;
			//myfile.close();

			/* calculate h */
			PKUtils::MultiplyMatrixByScalar(state->JgT, lambda, state->h);								// JgT * lambda => h
			PKUtils::AddMatrices(state->Lt_dot_Lx_minus_b, state->h, state->Lt_dot_Lx_minus_b);		// Lt * (Lx - b) + [JgT * lambda <= h]
			PKUtils::MultiplyMatrices(state->LtLInv, state->Lt_dot_Lx_minus_b, state->h);			// (LtL-1) * (Lt * (Lx - b) + JgT * lambda) = - h
			PKUtils::MultiplyMatrixByScalar(state->h, -alpha, state->h);						// (-h) * (-alpha) = alpha * h

			//char buf3[50] = { 0 };
			//sprintf(buf3, "C:/Temp/h_cpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->h, buf3);

			// apply step
			// xk+1 = xk + alpha * h
			PKUtils::AddMatrices(state->solutionK, state->h, state->solutionKPlus1);

			char buf[50] = { 0 };
#pragma warning(suppress: 4996)
			sprintf(buf, "C:/Temp/solutionKPlus1_cpu_%u_%u.txt", iterNum, modelIndex);
			PrintPKMatrix(state->solutionKPlus1, buf);

			// update OOC
			if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
				OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
				MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

				ooc->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1);
				if (this->useGrid) {
					navigator->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1, ooc);
				}
			}
		} //end for models
#pragma endregion STEP Calculation

		end = clock();
		double deformation = diffclock(end, start);
		//timeSlot[2] += diffclock(end, start);

		myfile.open("C:/Temp/times.txt", ios::out | ios::app);
		myfile << "deformation: " << deformation << "ms" << endl;
		myfile.close();

		start = clock();

#pragma region PENETRATION CHECK
		if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
			// dynamic intersection fix
			dirTemp.clear();

			for (int i = 0; i < modelCount + this->obstacleCount; i++) {
				for (int j = i + 1; j < modelCount + this->obstacleCount; j++) {
					PKMatrix *fixA = intersectionFixes[i][j];
					PKMatrix *fixB = intersectionFixes[j][i];

					OoCylinder *oocA = this->meshes[i]->GetOoCylinder();
					OoCylinder *oocB = this->meshes[j]->GetOoCylinder();

					// rough estimate using bounding cylinder
					if (!OoCylinder::intersectsCapsule(oocA, oocB)) {
						PKUtils::EraseMatrix(fixA);
						PKUtils::EraseMatrix(fixB);
						continue;
					}

					contextA.mesh = this->meshes[i]->GetCoarse();
					contextA.points = i < modelCount ? states[i]->solutionKPlus1 : NULL;
					contextA.fix = fixA;
					contextA.movement =  i < modelCount ? states[i]->h : NULL;
					contextA.deformable = this->meshes[i]->GetDeformable();
					if (this->useGrid) {
						contextA.navigator = this->meshes[i]->GetNavigator();
					} else {
						contextA.navigator = NULL;
					}
					contextA.verticesInside = NULL;
					contextA.verticesUndecided = NULL;

					contextB.mesh = this->meshes[j]->GetCoarse();
					contextB.points = j < modelCount ? states[j]->solutionKPlus1 : NULL;
					contextB.fix = fixB;
					contextB.movement = j < modelCount ? states[j]->h : NULL;
					contextB.deformable = this->meshes[j]->GetDeformable();
					if (this->useGrid) {
						contextB.navigator = this->meshes[j]->GetNavigator();
					} else {
						contextB.navigator = NULL;
					}
					contextB.verticesInside = NULL;
					contextB.verticesUndecided = NULL;

					if ((contextA.deformable || contextB.deformable) && onFullMesh)
					{
						double intersectedVolume = this->FindAndFixIntersection(&contextA, &contextB);

#ifdef _DEBUG
						bool fixAOk = i < modelCount ? PKUtils::IsSubMatrix(states[i]->h, fixA) : true;
						bool fixBOk = j < modelCount ? PKUtils::IsSubMatrix(states[j]->h, fixB) : true;
						if (!fixAOk || !fixBOk) {
							cout << "Error";
						}
#endif
						if (i < modelCount) {
							states[i]->volumeLostByFix += abs(intersectedVolume) / 2;
						}

						if (j < modelCount) {
							states[j]->volumeLostByFix += abs(intersectedVolume) / 2;
						}

						// apply changes NOW because you cannot add fixes together as their sum might easily be bigger than the original change made in this step
						// also limit the correction space for another pair


						if (i < modelCount) {
							PKUtils::AddMatrices(states[i]->solutionKPlus1, fixA, states[i]->solutionKPlus1);
							PKUtils::AddMatrices(states[i]->h, fixA, states[i]->h);
						}

						if (j < modelCount) {
							PKUtils::AddMatrices(states[j]->solutionKPlus1, fixB, states[j]->solutionKPlus1);
							PKUtils::AddMatrices(states[j]->h, fixB, states[j]->h);
						}
					}

					//PKUtils::EraseMatrix(fixA);
					//PKUtils::EraseMatrix(fixB);
				}
			}
		} //end if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION)
#pragma endregion PENETRATION CHECK

		end = clock();
		double penetrationCheck = diffclock(end, start); 
		//timeSlot[3] += diffclock(end, start);

		myfile.open("C:/Temp/times.txt", ios::out | ios::app);
		myfile << "penetration check: " << penetrationCheck << "ms" << endl;
		myfile.close();

		start = clock();

#pragma region PENETRATION FIX
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) 
		{
			PKSolutionState *state = states[modelIndex];

			// intersection fix application
			if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
				//// update JgT
				//this->CalculateVolumeJacobiMatrixTransposed(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1, state->JgT);  // JgT

				// prevent volume correction on intersection vertices
				memset(state->blockedVertices, 0, sizeof(bool) * state->JgT->height);

				for (int i = 0; i <  modelCount + this->obstacleCount; i++) {
					if (i == modelIndex) {
						continue;
					}

					//PKMatrix *fix = intersectionFixes[min(i, modelIndex)][max(i, modelIndex)];
					PKMatrix *fix = intersectionFixes[modelIndex][i];

					for (int i = 0; i < fix->height; i++) {
						if (!state->blockedVertices[i] && PKUtils::CalculateVertexLengthSq(fix->values[i]) > 0) {
							state->blockedVertices[i] = true;
						}
					}
				}

				//// add volume correction
				//PKUtils::LinearizeMatrix(state->JgT, state->JgLin);
				//double JgTSize = sqrt(PKUtils::DotN(state->JgLin->values[0], state->JgLin->values[0], state->JgLin->width));
				//PKUtils::MultiplyMatrixByScalar(state->JgT, state->volumeLostByFix / JgTSize, state->JgT);
				//PKUtils::AddMatrices(state->solutionKPlus1, state->JgT, state->solutionKPlus1);
			}

			// max norm of difference vector between this and previous solution
			double diff = PKUtils::MeasureMatrixDifference(state->solutionK, state->solutionKPlus1);

			maxDiff = max(diff, maxDiff);

			// switch solutions instead of copying its values
			PKMatrix *temp = state->solutionK;
			state->solutionK = state->solutionKPlus1;
			state->solutionKPlus1 = temp;
		} //end for [models]
#pragma endregion PENETRATION FIX

		end = clock();
		double penetrationFix = diffclock(end, start);
		//timeSlot[4] += diffclock(end, start);

		myfile.open("C:/Temp/times.txt", ios::out | ios::app);
		myfile << "penetration fix: " << penetrationFix << "ms" << endl;
		myfile.close();

		timeSlot[2] += deformation;
		timeSlot[3] += penetrationCheck;
		timeSlot[4] += penetrationFix;

		// stop condition by minimum error
		if (maxDiff < SOLUTION_EPSILON && onFullMesh && isPrecisionFinal)
		{
			break;
		}

		// stop condition by max iter reached
		if (iterNum == 200) {
			break;
		}

		lastDiff = maxDiff;

		//#ifdef _DEBUG_PROCESS


		//fprintf(fLog, "%d\t%f\t%f\t%f\t%d\n", iterNum, lastDiff, volRat, alpha, (int)bCorseMeshProcessing);

		//#endif

	}
#pragma endregion MAIN ITERATIVE LOOP

	start = clock();

#pragma region FINALIZATION
	if (this->debugMode) {
		Debug_Visualize_Progress(iterNum, lastDiff, states, &dirTemp);
	}

	// output result
	for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
		(*solutionsX)[modelIndex] = states[modelIndex]->solutionK;
	}

	// lots of disposing - possible space for memory usage optimisation
	if (states != NULL) {
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
			if (states[modelIndex] != NULL) {
				delete states[modelIndex];
			}
		}

		delete states;
		states= NULL;
	}

	if (intersectionFixes != NULL) {
		for (int i = 0; i < modelCount + this->obstacleCount; i++) {
			for (int j = 0; j < modelCount + this->obstacleCount; j++) {
				if (i == j) {
					continue;
				}

				PKUtils::DisposeMatrix(intersectionFixes[i] + j);
			}

			delete[] intersectionFixes[i];
		}

		delete[] intersectionFixes;
		intersectionFixes = NULL;
	}
#pragma endregion FINALIZATION

	end = clock();
	timeSlot[5] += diffclock(end, start);

	// print times
	myfile.open("C:/Temp/times.txt", ios::out | ios::app);
	myfile << "SUMARY" << endl;
	myfile << "Initialization: " << timeSlot[0] << endl;
	myfile << "Static penetration: " << timeSlot[1] << endl;
	myfile << "Deformation: " << timeSlot[2] << endl;
	myfile << "Penetration check: " << timeSlot[3] << endl;
	myfile << "Penetration fix: " << timeSlot[4] << endl;
	myfile << "Finalization: " << timeSlot[5] << endl;
	myfile.close();

	return !failed;
}


//////
/// Minimizes linear system with hard constraint using Gauss-Newton iterative method with Lagrange multiplicators.
/// Uses modified approach from Huang 2006, Subspace Gradient Domain Mesh Deformation
/// The behaviour is the same as in FindEnergyMinimumGN, but this implementation uses GPU for paralelization of the computations
/// @param matrixL matrix L of linear system (input)
/// @param vectorB vector of right sides of system (input)
/// @param solutionX pointer to vector of starting X (input) and then solution (output) - must be preallocated and should be prefilled (input and output)
//////
bool vtkMEDPolyDataDeformationPK::FindEnergyMinimumGPU(PKMatrix **matricesL, PKMatrix **verticesB, PKMatrix ***solutionsX)
{
#ifdef MEASURE_TIME
	double timeSlot[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	//this->preventCollisions = false;
	ofstream myfile;
	myfile.open("C:/Temp/times_gpu.txt", ios::out);
	myfile << "START" << endl;
	myfile.close();

	clock_t start = clock();
#endif

#pragma region INITIALIZATION	
	int iterNum = 0;
	double lastDiff = 9E99;
	double volumeDiff = 0;
	double alpha = 0.1;
	bool onFullMesh = false;
	bool isPrecisionFinal = false;

	bool failed = false;
	int modelCount = this->modelCount;

	gpuModel* gpuModels = NULL;
	//CollisionDetection* cd = NULL;
	int numberOfObjects = 0;
	unsigned int*** verticesInside = NULL;
	unsigned int*** verticesUndecided = NULL;
	char** meshesIntersect = NULL;
	this->cdTests = 0;
	this->cdErrors = 0;
	//bool** meshesUndecided = NULL;
	if (this->preventCollisions) {
		numberOfObjects = this->modelCount + this->obstacleCount;
		gpuModels = new gpuModel[numberOfObjects];
		//cd = new CollisionDetection();

		// enable OpenGL
		//cd->enableOpenGL();

		//if(!cd->extensionsAvailable()) {
		//	return false;
		//}

		//Initializing data
		GLuint numberOfPoints = 0;
		for(int i = 0; i < numberOfObjects; i++) {
			gpuModels[i] = cd->generateBuffers(meshes[i]->GetCoarse(), meshes[i]->GetDeformable());
			if(numberOfPoints < gpuModels[i].numberOfPoints)
				numberOfPoints = gpuModels[i].numberOfPoints;
		}
		cd->generateIdBuffers(numberOfPoints);

		cd->init();

		verticesInside = new unsigned int**[numberOfObjects];
		verticesUndecided = new unsigned int**[numberOfObjects];
		meshesIntersect = new char*[numberOfObjects];

		//alocate memory
		for (int i = 0; i < numberOfObjects; i++) {
			verticesInside[i] = new unsigned int*[numberOfObjects];
			verticesUndecided[i] = new unsigned int*[numberOfObjects];
			meshesIntersect[i] = new char[numberOfObjects];
			for (int j = 0; j < numberOfObjects; j++) {
				int bufferSize = gpuModels[j].numberOfPoints / 32;
				if(gpuModels[j].numberOfPoints % 32 != 0) bufferSize++;
				verticesInside[i][j] = new unsigned int[bufferSize];
				verticesUndecided[i][j] = new unsigned int[bufferSize];
				meshesIntersect[i][j] = 'u';
			}
		}
	}

	//gpu_solver.initCuda();

	PKSolutionStateGPU **states = new PKSolutionStateGPU*[modelCount];
#pragma region Initialization of deformation state
	for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
		PKMatrix *matrixL = matricesL[modelIndex];
		PKMatrix *vectorB = verticesB[modelIndex];

		int matrixHeight = matrixL->height;
		int matrixWidth = matrixL->width;
		int numberOfDimensions = vectorB->width;

		if (matrixWidth > matrixHeight)
		{
			throw "System is underconstrained.";
		}

		/* allocations */
		PKSolutionStateGPU *state = new PKSolutionStateGPU(matrixHeight, matrixWidth, numberOfDimensions,
			this->meshes[modelIndex]->GetOriginal()->GetNumberOfPoints());

		state->gpu_L = gpu_solver->allocateMemD(matrixL->height * matrixL->width);
		state->gpu_B = gpu_solver->allocateMemD(vectorB->height * vectorB->width);
		state->gpu_LtLInv = gpu_solver->allocateMemD(matrixWidth * matrixWidth);
		state->gpu_Lx_minus_b = gpu_solver->allocateMemD(matrixHeight * numberOfDimensions);
		state->gpu_solutionK = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		//TODO register vbo 
		//gpu_solver.registerGLBufferD(gpuModels[modelIndex].vbo);
		state->gpu_solutionKPlus1 = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);

		state->gpu_Lt_dot_Lx_minus_b = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		state->gpu_Lt_dot_Lx_minus_bT = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		state->gpu_JgT = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		state->gpu_Jg = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		state->gpu_Jg_LtLInv = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);
		state->gpu_Jg_LtLInvT = gpu_solver->allocateMemD(matrixWidth * numberOfDimensions);

		/* precalculations */
		state->solutionK = (*solutionsX)[modelIndex];
		state->originalVolume = this->CalculateVolume(this->meshes[modelIndex]->GetOriginal(), NULL);
		state->originalVolumeCoarse = this->CalculateVolume(this->meshes[modelIndex]->GetCoarse(), NULL);
		state->coarseToOrigVolRatio = state->originalVolumeCoarse / state->originalVolume;

		// Lt
		PKUtils::TransposeMatrix(matrixL, state->Lt); 
		// LtL
		PKUtils::MultiplyMatrices(state->Lt, matrixL, state->LtL); 
		// LtL-1
		if (!PKUtils::InvertMatrix(state->LtL, state->LtLInv)) {
			vtkErrorMacro(<< "Cannot invert LtL. Probably singular or pseudosingular.\n");

#ifdef _DEBUG_OUTPUT_PKMATRIX
			matrixL->DebugOutput("matrixL");
			vectorB->DebugOutput("vectorB");
			state->Lt->DebugOutput("state->Lt");
			state->LtL->DebugOutput("state->LtL");
#endif

#ifdef _DEBUG_PROCESS
			vtkMAFSmartPointer< vtkSTLWriter > stlw;
			stlw->SetInput(this->meshes[modelIndex]->GetOriginal());
			stlw->SetFileName("original.stl");
			stlw->SetFileTypeToBinary();
			stlw->Update();

			stlw->SetInput(this->meshes[modelIndex]->GetCoarse());
			stlw->SetFileName("coarse.stl");
			stlw->SetFileTypeToBinary();
			stlw->Update();

			stlw->SetInput(NULL);
#endif
			failed = true;
		}

		/* transfer of input data to GPU */
		double *arr = PKUtils::PKMatrixToArray(matrixL);
		gpu_solver->transferMatrixDataD_hd(arr, state->gpu_L, matrixWidth, matrixHeight);
		delete[] arr; arr = PKUtils::PKMatrixToArray(vectorB);
		gpu_solver->transferMatrixDataD_hd(arr, state->gpu_B, vectorB->width, vectorB->height);
		delete[] arr; arr = PKUtils::PKMatrixToArray(state->solutionK);
		gpu_solver->transferMatrixDataD_hd(arr, state->gpu_solutionK, state->solutionK->width, state->solutionK->height);
		delete[] arr; arr = PKUtils::PKMatrixToArray(state->LtLInv);
		gpu_solver->transferMatrixDataD_hd(arr, state->gpu_LtLInv, state->LtLInv->width, state->LtLInv->height);
		delete[] arr;

		states[modelIndex] = state;
	} //end for models
#pragma endregion Initialization of deformation state

#pragma region Initialization of penetreation check mechanism
	/* intersection pairs allocation */
	PKMatrix*** intersectionFixes = NULL;
	tIntersectionContext contextA;
	tIntersectionContext contextB;

	if (this->preventCollisions) {
		intersectionFixes = new PKMatrix**[modelCount + this->obstacleCount];

		for (int i = 0; i < modelCount + this->obstacleCount; i++) {
			intersectionFixes[i] = new PKMatrix*[modelCount + this->obstacleCount];

			int matrixWidth = i < modelCount ? matricesL[i]->width : this->meshes[i]->GetCoarse()->GetNumberOfPoints();
			int numberOfDimensions = i < modelCount ? verticesB[i]->width : 3;

			for (int j = 0; j < modelCount + this->obstacleCount; j++) {
				if (i == j) {
					continue;
				}

				// contains repair vertices for i-th mesh to avoid collision with j-th mesh
				intersectionFixes[i][j] = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
			}
		}
	}
#pragma endregion Initialization of penetreation check mechanism
#pragma endregion INITIALIZATION

#ifdef MEASURE_TIME
	clock_t end = clock();
	timeSlot[0] += diffclock(end, start);

	myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
	myfile << "initialization: " << timeSlot[0] << "ms" << endl;
	myfile.close();

	start = clock();
#endif
	/*
	#pragma region STATIC PENETRATION
	if (this->preventCollisions) 
	{
	//if (this->debugMode) {
	//	Debug_Visualize_Progress(iterNum, lastDiff, states, NULL); //1
	//}

	// create OOC
	for (int modelIndex = 0; modelIndex < modelCount + this->obstacleCount; modelIndex++) {
	// per model variables
	OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
	MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

	//BES: 31.10.2011 - what should be here????
	ooc->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL);

	if (this->useGrid) {
	navigator ->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL, ooc);
	}
	}

	#if MIN_ITERS_BEFORE_INTERSECTION == 0
	// solve static intersections
	for (int i = 0; i < modelCount + this->obstacleCount; i++) {
	for (int j = i + 1; j < modelCount + this->obstacleCount; j++) {
	PKMatrix *fixA = intersectionFixes[i][j];
	PKMatrix *fixB = intersectionFixes[j][i];

	OoCylinder *oocA = this->meshes[i]->GetOoCylinder();
	OoCylinder *oocB = this->meshes[j]->GetOoCylinder();

	// rough estimate using bounding cylinder (if the bonding cylinders do not intersect, ignore both objects)
	if (!OoCylinder::intersectsCapsule(oocA, oocB)) {
	PKUtils::EraseMatrix(fixA);
	PKUtils::EraseMatrix(fixB);
	continue;
	}

	contextA.mesh = this->meshes[i]->GetCoarse();
	contextA.fix = fixA;
	contextA.movement = NULL;
	if (this->useGrid) {
	contextA.navigator = this->meshes[i]->GetNavigator();
	} else {
	contextA.navigator = NULL;
	}
	contextA.deformable = this->meshes[i]->GetDeformable();
	if (contextA.deformable) {
	contextA.points = states[i]->solutionK;
	}
	else {
	contextA.points = NULL;
	}

	contextB.mesh = this->meshes[j]->GetCoarse();
	contextB.fix = fixB;
	contextB.movement = NULL;
	if (this->useGrid) {
	contextB.navigator = this->meshes[j]->GetNavigator();
	} else {
	contextB.navigator = NULL;
	}
	contextB.deformable = this->meshes[j]->GetDeformable();
	if (contextB.deformable) {
	contextB.points = states[j]->solutionK;
	}
	else {
	contextB.points = NULL;
	}
	//if ((contextA.deformable || contextB.deformable) && onFullMesh)
	//{
	//	double intersectedVolume = this->FindIntersection(&contextA, &contextB);
	//
	//	if (i < modelCount) {
	//		PKUtils::AddMatrices(states[i]->solutionK, fixA, states[i]->solutionK);
	//	}
	//
	//	if (j < modelCount) {
	//		PKUtils::AddMatrices(states[j]->solutionK, fixB, states[j]->solutionK);
	//	}
	//}
	}
	}
	#endif
	} //end if [this->preventCollisions]
	#pragma endregion STATIC PENETRATION
	*/
#ifdef MEASURE_TIME
	end = clock();
	timeSlot[1] += diffclock(end, start);

	myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
	myfile << "Static penetration: " << timeSlot[1] << "ms" << endl;
	myfile.close();
#endif

#pragma region MAIN ITERATIVE LOOP
#ifdef _DEBUG_PROCESS
	double volRat;
	bool bCorseMeshProcessing;
#endif

	for (int modelIndex = 0; modelIndex < modelCount + this->obstacleCount; modelIndex++) {
		// per model variables
		OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
		MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

		//BES: 31.10.2011 - what should be here????
		ooc->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL);

		if (this->useGrid) {
			navigator ->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), modelIndex < modelCount ? (*solutionsX)[modelIndex] : NULL, ooc);
		}
	}


	while (!failed)
	{
		iterNum++;

#ifdef MEASURE_TIME
		myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
		myfile << "ITERATION: " << iterNum << endl;
		myfile.close();
#endif

		if (this->debugMode) {
			cd->releaseOpenGLContext();
			Debug_Visualize_ProgressGPU(iterNum, lastDiff, states, &dirTemp);//2
			cd->setOpenGLContext();
		}

#ifdef _DEBUG_PROCESS
		vtkMAFSmartPointer< vtkSTLWriter > stlw;
		stlw->SetFileTypeToBinary();

		char bufname[32];

		if (iterNum == 1)
		{
			for (int obstacleIndex = modelCount; obstacleIndex < modelCount + this->obstacleCount; obstacleIndex++)
			{
				stlw->SetInput(this->meshes[obstacleIndex]->GetOriginal());
#if _MSC_VER >= 1400 // grater than vs 2005
				sprintf_s<32>(bufname, "BN_ORIG_#000_%d.stl", obstacleIndex);
#else
				sprintf(bufname, "BN_ORIG_#000_%d.stl", obstacleIndex);
#endif

				stlw->SetFileName(bufname);
				stlw->Update();

				stlw->SetInput(this->meshes[obstacleIndex]->GetCoarse());

#if _MSC_VER >= 1400 // grater than vs 2005
				sprintf_s<32>(bufname, "BN_COAR_#000_%d.stl", obstacleIndex);
#else
				sprintf(bufname, "BN_COAR_#000_%d.stl", obstacleIndex);
#endif

				stlw->SetFileName(bufname);
				stlw->Update();
			}
		}

		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++)
		{
			vtkPolyData* poly = this->meshes[modelIndex]->GetCoarse()->NewInstance();
			poly->DeepCopy(this->meshes[modelIndex]->GetCoarse());

			this->meshes[modelIndex]->SetUpPointCoords(poly, states[modelIndex]->solutionK);

			stlw->SetInput(poly);

#if _MSC_VER >= 1400 // grater than vs 2005
			sprintf_s<32>(bufname, "MU_COAR_#%03d_%d.stl", iterNum, modelIndex);
#else
			sprintf(bufname, "MU_COAR_#%03d_%d.stl", iterNum, modelIndex);
#endif

			stlw->SetFileName(bufname);
			stlw->Update();

			poly->Delete();
		}
#endif

		// sequential decreasing of step length enhances precision in later phases
		// and speeds up first phases
		if (lastDiff > 5 * SOLUTION_EPSILON)
		{
			alpha = min(0.5, alpha);
		}
		else if (lastDiff > 1.5 * SOLUTION_EPSILON)
		{
			alpha = min(0.1, alpha);
		}
		else
		{
			alpha = min(0.025, alpha);
			isPrecisionFinal = true;
		}

		// switch to full mesh
		if (lastDiff <= 5 * SOLUTION_EPSILON || iterNum > MAX_ITER_ON_COARSE) {
			onFullMesh = true;
			//this->debugMode = true;
		}

		double maxDiff = 0;

#ifdef MEASURE_TIME
		start = clock();
#endif

		// step calculation
#pragma region STEP CalculationdirTemp
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) 
		{
			// per model variables
			PKSolutionStateGPU *state = states[modelIndex];
			//TODO map vertex buffer to cuda
			//state->gpu_solutionKPlus1 = gpu_solver.mapGLBufferD(gpuModels[modelIndex].vbo);
			PKMatrix *matrixL = matricesL[modelIndex];
			PKMatrix *vectorB = verticesB[modelIndex];
			state->volumeLostByFix = 0;

			/* volume measurement */
			if (onFullMesh)
			{
				// precise measurement of volume error in the end (coords distribution takes a lot of time)
				this->meshes[modelIndex]->ApplyCoarseCoordsToOriginalMesh(state->solutionK, state->bufferHighResMatrix);
				double currentVolume = this->CalculateVolume(this->meshes[modelIndex]->GetOriginal(), NULL);
				volumeDiff = state->originalVolume - currentVolume;
				volumeDiff *= state->coarseToOrigVolRatio;

#ifdef _DEBUG_PROCESS
				volRat = 100 * fabs(currentVolume / state->originalVolume);	//volume may be negative!
				bCorseMeshProcessing = false;
#endif
			}
			else
			{
				// rough estimation of volume error in the beginning
				double currentVolume = this->CalculateVolume(this->meshes[modelIndex]->GetCoarse(), state->solutionK);
				volumeDiff = state->originalVolumeCoarse - currentVolume;

#ifdef _DEBUG_PROCESS
				volRat = 100 * fabs(currentVolume / state->originalVolumeCoarse);	//volume may be negative!
				bCorseMeshProcessing = true;
#endif
			}

			/* calculate jacobian of nonlinear conditions which is ultimately just a gradient of volume function */
			this->CalculateVolumeJacobiMatrixTransposed(this->meshes[modelIndex]->GetCoarse(), state->solutionK, state->JgT);  // JgT

			if (this->preventCollisions && iterNum > MIN_ITERS_BEFORE_INTERSECTION) {
				// prevent repeating intersections by volume correction
				if (volumeDiff > 0) {
					for (int i = 0; i < state->JgT->height; i++) {
						if (state->blockedVertices[i]) {
							state->JgT->values[i][0] = state->JgT->values[i][1] = state->JgT->values[i][2] = 0;
						}
					}
				}
			}

			//PKUtils::TransposeMatrix(state->JgT, state->Jg);												 // Jg
			//PKUtils::LinearizeMatrix(state->Jg, state->JgLin);

			//JgT - transfer to GPU
			double *arr = PKUtils::PKMatrixToArray(state->JgT);
			gpu_solver->transferMatrixDataD_hd(arr, state->gpu_JgT, state->JgT->width, state->JgT->height);
			delete[] arr;

			// Jg
			gpu_solver->matrixTransposeD(state->gpu_JgT, state->gpu_Jg, state->JgT->height, state->JgT->width);

			/* common precalculations */
			//PKUtils::MultiplyMatrices(matrixL, state->solutionK, state->Lx_minus_b);						// Lx
			//PKUtils::SubtractMatrices(state->Lx_minus_b, vectorB, state->Lx_minus_b);						// Lx - b
			//PKUtils::MultiplyMatrices(state->Lt, state->Lx_minus_b, state->Lt_dot_Lx_minus_b);					// Lt * (Lx - b)

			//char buf13[50] = { 0 };
			//sprintf(buf13, "C:/Temp/L_cpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(matrixL, buf13);

			//char buf14[50] = { 0 };
			//sprintf(buf14, "C:/Temp/solutionK_cpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->solutionK, buf14);

			//char buf11[50] = { 0 };
			//sprintf(buf11, "C:/Temp/L_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_L, state->matrixWidth, state->matrixHeight, buf11);

			//char buf12[50] = { 0 };
			//sprintf(buf12, "C:/Temp/solutionK_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_solutionK, state->numberOfDimensions, state->solutionK->height, buf12);

			// Lx
			gpu_solver->matrixMulD(state->gpu_L, state->matrixWidth, state->matrixHeight, 'n', state->gpu_solutionK, state->numberOfDimensions, state->matrixWidth, 'n', state->gpu_Lx_minus_b); 
			// Lx - b 
			gpu_solver->vectorSubD(state->gpu_Lx_minus_b, state->gpu_B, state->numberOfDimensions * state->matrixHeight); 
			// Lt * (Lx - b)
			gpu_solver->matrixMulD(state->gpu_L, state->matrixWidth, state->matrixHeight, 't', state->gpu_Lx_minus_b, state->numberOfDimensions, state->matrixHeight, 'n', state->gpu_Lt_dot_Lx_minus_b); 

			//char buf15[50] = { 0 };
			//sprintf(buf15, "C:/Temp/Lx_minus_b_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_Lx_minus_b, state->numberOfDimensions, state->matrixHeight, buf15);

			//char buf1[50] = { 0 };
			//sprintf(buf1, "C:/Temp/Lt_dot_Lx_minus_b_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_Lt_dot_Lx_minus_b, state->numberOfDimensions, state->matrixWidth, buf1);

			/* calculate lambda */
			//PKUtils::MultiplyMatrices(state->Jg, state->LtLInv, state->Jg_LtLInv);								// Jg * (LtL)-1
			//PKUtils::LinearizeMatrix(state->Jg_LtLInv, state->Jg_LtLInvLin);
			//PKUtils::LinearizeMatrix(state->Lt_dot_Lx_minus_b, state->Lt_dot_Lx_minus_b_Lin);
			//double rightSideOfLambdaLin = PKUtils::DotN(state->Jg_LtLInvLin->values[0], state->Lt_dot_Lx_minus_b_Lin->values[0], state->Lt_dot_Lx_minus_b_Lin->width);
			// Jg * (LtL)-1 * Lt * (Lx - b)
			//double Jg_LtLInv_JgT_Lin = PKUtils::DotN(state->Jg_LtLInvLin->values[0], state->JgLin->values[0], state->JgLin->width);
			// Jg * (LtL)-1 * JgT
			//double lambda = - (1 / Jg_LtLInv_JgT_Lin) * (volumeDiff / alpha - rightSideOfLambdaLin);
			// lambda = -[Jg * (LtL)-1 * JgT]-1 * {g - [Jg * (LtL)-1 * Lt * (Lx - b)]}

			//char buf16[50] = { 0 };
			//sprintf(buf16, "C:/Temp/JgT_cpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->JgT, buf16);

			//char buf17[50] = { 0 };
			//sprintf(buf17, "C:/Temp/JgT_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_JgT, state->JgT->width, state->JgT->height, buf17);

			//char buf18[50] = { 0 };
			//sprintf(buf18, "C:/Temp/Jg_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintGPUMatrix(state->gpu_Jg, state->JgT->height, state->JgT->width, buf18);

			// Jg * (LtL)-1
			gpu_solver->matrixMulD(state->gpu_Jg, state->matrixWidth, state->numberOfDimensions, 'n', state->gpu_LtLInv, state->matrixWidth, state->matrixWidth, 'n', state->gpu_Jg_LtLInv);

			gpu_solver->matrixTransposeD(state->gpu_Jg_LtLInv, state->gpu_Jg_LtLInvT, state->numberOfDimensions, state->matrixWidth);
			gpu_solver->matrixTransposeD(state->gpu_Lt_dot_Lx_minus_b, state->gpu_Lt_dot_Lx_minus_bT, state->matrixWidth, state->numberOfDimensions);

			// Jg * (LtL)-1 * JgT	
			double gpu_Jg_LtLInv_JgT_Lin = gpu_solver->dotProductD(state->gpu_Jg_LtLInvT, state->gpu_JgT, state->matrixWidth * state->numberOfDimensions);
			// Jg * (LtL)-1 * Lt * (Lx - b)
			double gpu_rightSideOfLambdaLin = gpu_solver->dotProductD(state->gpu_Jg_LtLInvT, state->gpu_Lt_dot_Lx_minus_bT, state->matrixWidth * state->numberOfDimensions);
			// lambda = -[Jg * (LtL)-1 * JgT]-1 * {g - [Jg * (LtL)-1 * Lt * (Lx - b)]}
			double lambda = - (1 / gpu_Jg_LtLInv_JgT_Lin) * (volumeDiff / alpha - gpu_rightSideOfLambdaLin); 

			//ofstream myfile;
			//char buf2[50] = { 0 };
			//sprintf(buf2, "C:/Temp/lambda_gpu_%u_%u.txt", iterNum, modelIndex);
			//myfile.open(buf2);
			//myfile << "rightSideOfLambdaLin: " << gpu_rightSideOfLambdaLin << endl;
			//myfile << "Jg_LtLInv_JgT_Lin: " << gpu_Jg_LtLInv_JgT_Lin << endl;
			//myfile << "Lambda: " << lambda << endl;
			//myfile.close();


			/* calculate h */
			//PKUtils::MultiplyMatrixByScalar(state->JgT, lambda, state->h);								// JgT * lambda => h
			//PKUtils::AddMatrices(state->Lt_dot_Lx_minus_b, state->h, state->Lt_dot_Lx_minus_b);		// Lt * (Lx - b) + [JgT * lambda <= h]
			//PKUtils::MultiplyMatrices(state->LtLInv, state->Lt_dot_Lx_minus_b, state->h);			// (LtL-1) * (Lt * (Lx - b) + JgT * lambda) = - h
			//PKUtils::MultiplyMatrixByScalar(state->h, -alpha, state->h);						// (-h) * (-alpha) = alpha * h

			// Lt * (Lx - b) + JgT * lambda
			gpu_solver->vectorKoefAddD(state->gpu_Lt_dot_Lx_minus_b, lambda, state->gpu_JgT, state->JgT->width * state->JgT->height); 
			// -alpha * (LtL-1) * (Lt * (Lx - b) + JgT * lambda) = h
			gpu_solver->matrixKoefMulD(
				state->gpu_LtLInv, state->matrixWidth, state->matrixWidth, 'n', -alpha,
				state->gpu_Lt_dot_Lx_minus_b, state->numberOfDimensions, state->matrixWidth, 'n', 
				state->gpu_solutionKPlus1);
			GPUMatrixToPKMatrix(state->gpu_solutionKPlus1, state->h);
			//gpu_solver.matrixTransposeD(state->gpu_solutionKPlus1, state->gpu_solutionKPlus1, state->solutionKPlus1->height, state->solutionKPlus1->width);

			//char buf3[50] = { 0 };
			//sprintf(buf3, "C:/Temp/h_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->h, buf3);

			// apply step
			// xk+1 = xk + alpha * h
			PKUtils::AddMatrices(state->solutionK, state->h, state->solutionKPlus1);
			//double diff = gpu_solver.vectorAbsMax(state->gpu_solutionKPlus1, state->numberOfDimensions * state->matrixWidth);
			//diff = abs(diff * alpha);
			gpu_solver->vectorAddD(state->gpu_solutionK, state->gpu_solutionKPlus1, state->numberOfDimensions * state->matrixWidth);
			//GPUMatrixToPKMatrix(state->gpu_solutionK, state->solutionKPlus1);

			//char buf[50] = { 0 };
			//sprintf(buf, "C:/Temp/solutionKPlus1_gpu_%u_%u.txt", iterNum, modelIndex);
			//PrintPKMatrix(state->solutionKPlus1, buf);

			// update OOC
			if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
				OoCylinder *ooc = this->meshes[modelIndex]->GetOoCylinder();
				MeshNavigator *navigator = this->meshes[modelIndex]->GetNavigator();

				ooc->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1);
				if (this->useGrid) {
					navigator->SetUpByMesh(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1, ooc);
				}
			}

			//TODO unmap vertex buffer from cuda
			//gpu_solver.unmapGLBufferD(gpuModels[modelIndex].vbo);
		} //end for models
#pragma endregion STEP Calculation

#ifdef MEASURE_TIME
		end = clock();
		double deformation = diffclock(end, start);
		//timeSlot[2] += diffclock(end, start);

		myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
		myfile << "deformation: " << deformation << "ms" << endl;
		myfile.close();

		start = clock();
#endif

#pragma region PENETRATION CHECK
		if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
			// dynamic intersection fix
			dirTemp.clear();

			if(onFullMesh) {

				for(int i = 0; i < modelCount; i++) {
					double* arr = PKUtils::PKMatrixTransposedToArray(states[i]->solutionKPlus1);
					cd->updateBuffers(&gpuModels[i], arr);
					delete[] arr;
				}

				// inside/outside test of all vertices on GPU
				for (int i = 0; i < numberOfObjects; i++) {
					//voxelize data[i]
					cd->prepareVoxelization();
					cd->voxelize(gpuModels[i]);
					cd->prepareBoundaryVoxelization();
					cd->voxelize(gpuModels[i]);
					cd->prepareTest();
					for (int j = 0; j < numberOfObjects; j++) {
						if(i == j) continue;
						//if(meshesIntersect[i][j] == 'o') continue;
						//if (!meshes[j]->GetDeformable()) {
						//	meshesIntersect[i][j] = 'o';
						//	continue;
						//}
						//if (!OoCylinder::intersectsCapsule(meshes[i]->GetOoCylinder(), meshes[j]->GetOoCylinder())) {
						//	meshesIntersect[i][j] = 'o';
						//	meshesIntersect[j][i] = 'o';
						//	continue;
						//}

						//test data[j]
						meshesIntersect[i][j] = cd->test(gpuModels[j], verticesInside[i][j], verticesUndecided[i][j]);
					}
				}

			}


			// intersection volume and intersection fixes are calculated 
			for (int i = 0; i < modelCount + this->obstacleCount; i++) {
				for (int j = i + 1; j < modelCount + this->obstacleCount; j++) {

					PKMatrix *fixA = intersectionFixes[i][j];
					PKMatrix *fixB = intersectionFixes[j][i];

					OoCylinder *oocA = this->meshes[i]->GetOoCylinder();
					OoCylinder *oocB = this->meshes[j]->GetOoCylinder();

					//if(meshesIntersect[i][j] == 'o' && meshesIntersect[j][i] == 'o') {
					// rough estimate using bounding cylinder
					if (!OoCylinder::intersectsCapsule(oocA, oocB)) {
						PKUtils::EraseMatrix(fixA);
						PKUtils::EraseMatrix(fixB);
						continue;
					}

					contextA.mesh = this->meshes[i]->GetCoarse();
					contextA.points = i < modelCount ? states[i]->solutionKPlus1 : NULL;
					contextA.fix = fixA;
					contextA.movement =  i < modelCount ? states[i]->h : NULL;
					contextA.deformable = this->meshes[i]->GetDeformable();
					if (this->useGrid) {
						contextA.navigator = this->meshes[i]->GetNavigator();
					} else {
						contextA.navigator = NULL;
					}
					contextA.verticesInside = verticesInside[j][i];
					contextA.verticesUndecided = verticesUndecided[j][i];

					contextB.mesh = this->meshes[j]->GetCoarse();
					contextB.points = j < modelCount ? states[j]->solutionKPlus1 : NULL;
					contextB.fix = fixB;
					contextB.movement = j < modelCount ? states[j]->h : NULL;
					contextB.deformable = this->meshes[j]->GetDeformable();
					if (this->useGrid) {
						contextB.navigator = this->meshes[j]->GetNavigator();
					} else {
						contextB.navigator = NULL;
					}
					contextB.verticesInside = verticesInside[i][j];
					contextB.verticesUndecided = verticesUndecided[i][j];

					if ((contextA.deformable || contextB.deformable) && onFullMesh)
					{
						double intersectedVolume = this->FindAndFixIntersection(&contextA, &contextB);

#ifdef _DEBUG
						bool fixAOk = i < modelCount ? PKUtils::IsSubMatrix(states[i]->h, fixA) : true;
						bool fixBOk = j < modelCount ? PKUtils::IsSubMatrix(states[j]->h, fixB) : true;
						if (!fixAOk || !fixBOk) {
							cout << "Error";
						}
#endif

						if (i < modelCount) {
							states[i]->volumeLostByFix += abs(intersectedVolume) / 2;
						}

						if (j < modelCount) {
							states[j]->volumeLostByFix += abs(intersectedVolume) / 2;
						}

						// apply changes NOW because you cannot add fixes together as their sum might easily be bigger than the original change made in this step
						// also limit the correction space for another pair
						if (i < modelCount) {
							PKUtils::AddMatrices(states[i]->solutionKPlus1, fixA, states[i]->solutionKPlus1);
							PKUtils::AddMatrices(states[i]->h, fixA, states[i]->h);
						}

						if (j < modelCount) {
							PKUtils::AddMatrices(states[j]->solutionKPlus1, fixB, states[j]->solutionKPlus1);
							PKUtils::AddMatrices(states[j]->h, fixB, states[j]->h);
						}
					}
					//PKUtils::EraseMatrix(fixA);
					//PKUtils::EraseMatrix(fixB);
				}
			}
		} //end if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION)
#pragma endregion PENETRATION CHECK

#ifdef MEASURE_TIME
		end = clock();
		double penetrationCheck = diffclock(end, start); 
		//timeSlot[3] += diffclock(end, start);

		myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
		myfile << "penetration check: " << penetrationCheck << "ms" << endl;
		myfile.close();

		start = clock();
#endif

#pragma region PENETRATION FIX
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) 
		{
			PKSolutionStateGPU *state = states[modelIndex];

			// intersection fix application
			if (this->preventCollisions && iterNum >= MIN_ITERS_BEFORE_INTERSECTION) {
				//// update JgT
				//this->CalculateVolumeJacobiMatrixTransposed(this->meshes[modelIndex]->GetCoarse(), state->solutionKPlus1, state->JgT);  // JgT

				// prevent volume correction on intersection vertices
				memset(state->blockedVertices, 0, sizeof(bool) * state->JgT->height);

				for (int i = 0; i < modelCount + this->obstacleCount; i++) {
					if (i == modelIndex) {
						continue;
					}

					//PKMatrix *fix = intersectionFixes[min(i, modelIndex)][max(i, modelIndex)];
					PKMatrix *fix = intersectionFixes[modelIndex][i];

					for (int i = 0; i < fix->height; i++) {
						if (!state->blockedVertices[i] && PKUtils::CalculateVertexLengthSq(fix->values[i]) > 0) {
							state->blockedVertices[i] = true;
						}
					}
				}

				//// add volume correction
				//PKUtils::LinearizeMatrix(state->JgT, state->JgLin);
				//double JgTSize = sqrt(PKUtils::DotN(state->JgLin->values[0], state->JgLin->values[0], state->JgLin->width));
				//PKUtils::MultiplyMatrixByScalar(state->JgT, state->volumeLostByFix / JgTSize, state->JgT);
				//PKUtils::AddMatrices(state->solutionKPlus1, state->JgT, state->solutionKPlus1);
			}

			// max norm of difference vector between this and previous solution
			double diff = PKUtils::MeasureMatrixDifference(state->solutionK, state->solutionKPlus1);

			maxDiff = max(diff, maxDiff);

			// switch solutions instead of copying its values
			PKMatrix *temp = state->solutionK;
			state->solutionK = state->solutionKPlus1;
			state->solutionKPlus1 = temp;
		} //end for [models]
#pragma endregion PENETRATION FIX

#ifdef MEASURE_TIME
		end = clock();
		double penetrationFix = diffclock(end, start);
		//timeSlot[4] += diffclock(end, start);

		myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
		myfile << "penetration fix: " << penetrationFix << "ms" << endl;
		myfile.close();

		timeSlot[2] += deformation;
		timeSlot[3] += penetrationCheck;
		timeSlot[4] += penetrationFix;
#endif

		// stop condition by minimum error
		if (maxDiff < SOLUTION_EPSILON && onFullMesh && isPrecisionFinal)
		{
			break;
		}

		// stop condition by max iter reached
		if (iterNum == 200) {
			break;
		}

		lastDiff = maxDiff;

#ifdef _DEBUG_PROCESS
		FILE* fLog = NULL;
		if (iterNum != 1)
#pragma warning(suppress: 4996)
			fLog = fopen("Energy.txt", "at");
		else {
#pragma warning(suppress: 4996)
			fLog = fopen("Energy.txt", "wt+");
			fprintf(fLog, "#Iter\tEnDiff\tVolume[%%]\talpha\tcoarseMesh\n");
		}

		fprintf(fLog, "%d\t%f\t%f\t%f\t%d\n", iterNum, lastDiff, volRat, alpha, (int)bCorseMeshProcessing);
		fclose(fLog);
#endif
	}
#pragma endregion MAIN ITERATIVE LOOP

#ifdef MEASURE_TIME
	start = clock();
#endif

#pragma region FINALIZATION
	if (this->debugMode) {
		cd->releaseOpenGLContext();
		Debug_Visualize_ProgressGPU(iterNum, lastDiff, states, &dirTemp);
		cd->setOpenGLContext();
	}

	// output result
	for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
		(*solutionsX)[modelIndex] = states[modelIndex]->solutionK;
	}

	// lots of disposing - possible space for memory usage optimisation
	if (states != NULL) {
		for (int modelIndex = 0; modelIndex < modelCount; modelIndex++) {
			if (states[modelIndex] != NULL) {
				gpu_solver->freeMemD(states[modelIndex]->gpu_L);
				gpu_solver->freeMemD(states[modelIndex]->gpu_B);
				gpu_solver->freeMemD(states[modelIndex]->gpu_solutionK);
				gpu_solver->freeMemD(states[modelIndex]->gpu_solutionKPlus1);
				gpu_solver->freeMemD(states[modelIndex]->gpu_LtLInv);
				gpu_solver->freeMemD(states[modelIndex]->gpu_JgT);
				gpu_solver->freeMemD(states[modelIndex]->gpu_Lx_minus_b);
				gpu_solver->freeMemD(states[modelIndex]->gpu_Lt_dot_Lx_minus_b);
				gpu_solver->freeMemD(states[modelIndex]->gpu_Jg_LtLInv);
				delete states[modelIndex];
			}
		}

		delete states;
		states= NULL;
	}

	if (intersectionFixes != NULL) {
		for (int i = 0; i < modelCount + this->obstacleCount; i++) {
			for (int j = 0; j < modelCount + this->obstacleCount; j++) {
				if (i == j) {
					continue;
				}

				PKUtils::DisposeMatrix(intersectionFixes[i] + j);
			}

			delete[] intersectionFixes[i];
		}

		delete[] intersectionFixes;
		intersectionFixes = NULL;
	}

	if(this->preventCollisions) {
		//dealocate memory
		for (int i = 0; i < modelCount; i++) {
			for (int j = 0; j < modelCount; j++) {
				delete[] verticesInside[i][j];
			}
			delete[] verticesInside[i];
			delete[] meshesIntersect[i];
		}
		delete[] verticesInside;
		delete[] meshesIntersect;

		// shutdown OpenGL
		cd->destroy();
		for(int i = 0; i < numberOfObjects; i++) {
			glDeleteBuffers(1, &gpuModels[i].vbo);
			glDeleteBuffers(1, &gpuModels[i].ebo);
		}
		delete[] gpuModels;

		//cd->disableOpenGL();
		//delete cd;
	}
#pragma endregion FINALIZATION

#ifdef MEASURE_TIME
	end = clock();
	timeSlot[5] += diffclock(end, start);

	// print times
	myfile.open("C:/Temp/times_gpu.txt", ios::out | ios::app);
	myfile << "SUMARY" << endl;
	myfile << "Initialization: " << timeSlot[0] << endl;
	myfile << "Static penetration: " << timeSlot[1] << endl;
	myfile << "Deformation: " << timeSlot[2] << endl;
	myfile << "Penetration check: " << timeSlot[3] << endl;
	myfile << "Penetration fix: " << timeSlot[4] << endl;
	myfile << "Finalization: " << timeSlot[5] << endl;
	myfile.close();
#endif

	return !failed;
}

void vtkMEDPolyDataDeformationPK::PrintPKMatrix(PKMatrix *m, char *path) {
	ofstream myfile;
	myfile.open(path);
	myfile << m->height << endl;
	myfile << m->width << endl << endl;
	for(int i = 0; i < m->height; i++) {
		for(int j = 0; j < m->width; j++) {
			myfile << m->values[i][j] << ", ";
		}
		myfile << endl;
	}
	myfile.close();
}

void vtkMEDPolyDataDeformationPK::PrintGPUMatrix(double *gp, int width, int height, char *path) {
	double* arr = new double[height * width];
	gpu_solver->transferVectorDataD_dh(gp, arr, height * width); 
	ofstream myfile;
	myfile.open(path);
	myfile << height << endl;
	myfile << width << endl << endl;
	for(int i = 0; i < height; i++) {		
		for(int j = 0; j < width ; j++) {
			myfile << arr[i + height*j] << ", ";
		}
		myfile << endl;
	}		
	myfile.close();
	delete[] arr;
}

void vtkMEDPolyDataDeformationPK::GPUMatrixToPKMatrix(double *gp, PKMatrix *matrix) {
	double* arr = new double[matrix->height * matrix->width];
	gpu_solver->transferVectorDataD_dh(gp, arr, matrix->height * matrix->width); 
	PKUtils::ArrayToPKMatrix(arr, matrix);
	delete[] arr;
}

#pragma endregion // solving

#pragma region //IMultiMeshDeformer

//*************************************************************************************************************************************
// Deformable meshes
//*************************************************************************************************************************************

//////
/// Sets number of meshes for deformation (deformable inputs).
///
/// Used for memory preallocation.
/// @param count number of meshes for deformation (input)
//////
void vtkMEDPolyDataDeformationPK::SetNumberOfMeshes(int count) {
	if (count < 0) {
		throw "Mesh count must be >= 0.";
	}

	// free old

	if (this->inputMeshes != NULL)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			if (this->inputMeshes[i] != NULL)
				this->inputMeshes[i]->UnRegister(this);
		}

		delete[] this->inputMeshes;
		this->inputMeshes = NULL;
	}

	if (this->inputMeshesCoarse != NULL)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			if (this->inputMeshesCoarse[i] != NULL) {
				this->inputMeshesCoarse[i]->Delete();
			}
		}

		delete[] this->inputMeshesCoarse;
		this->inputMeshesCoarse = NULL;
	}

	if (this->outputMeshes != NULL)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			if (this->outputMeshes[i] != NULL)
				this->outputMeshes[i]->UnRegister(this);
		}

		delete[] this->outputMeshes;
		this->outputMeshes = NULL;
	}

	if (this->outputMeshesCoarse != NULL)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			if (this->outputMeshesCoarse[i] != NULL)
				this->outputMeshesCoarse[i]->UnRegister(this);
		}

		delete[] this->outputMeshesCoarse;
		this->outputMeshesCoarse = NULL;
	}

	if (this->skeletonCounts != NULL) {
		delete[] this->skeletonCounts;
		this->skeletonCounts = NULL;
	}

	if (this->multiM_Skeletons != NULL) {
		delete[] this->multiM_Skeletons;
		this->multiM_Skeletons = NULL;
	}

	// allocate

	if ((this->modelCount = count) != 0)
	{
		this->inputMeshes = new vtkPolyData*[count];
		memset(this->inputMeshes, 0, count*sizeof(vtkPolyData*));

		this->inputMeshesCoarse = new vtkPolyData*[count];
		memset(this->inputMeshesCoarse, 0, count*sizeof(vtkPolyData*));

		this->outputMeshes = new vtkPolyData*[count];
		memset(this->outputMeshes, 0, count*sizeof(vtkPolyData*));

		this->outputMeshesCoarse = new vtkPolyData*[count];
		memset(this->outputMeshesCoarse, 0, count*sizeof(vtkPolyData*));

		this->skeletonCounts = new int[count];
		memset(this->skeletonCounts, 0, count*sizeof(int));

		this->multiM_Skeletons = new CONTROL_SKELETON*[count];
		memset(this->multiM_Skeletons, 0, count* sizeof(CONTROL_SKELETON*));
	}
}

//////
/// Gets number of meshes for deformation (deformable inputs).
///
/// Used for memory preallocation.
/// @return number of meshes for deformation
//////
int vtkMEDPolyDataDeformationPK::GetNumberOfMeshes() {
	return this->modelCount;
}

//////
/// Sets input deformable mesh at specified index.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
//////
void vtkMEDPolyDataDeformationPK::SetInputMesh(int meshIndex, vtkPolyData *poly) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (this->inputMeshes[meshIndex] != poly)
	{
		if (this->inputMeshes[meshIndex] != NULL)
			this->inputMeshes[meshIndex]->UnRegister(this);

		if (NULL != (this->inputMeshes[meshIndex] = poly))
			this->inputMeshes[meshIndex]->Register(this);
		this->Modified();
	}
}

//////
/// Gets input deformable mesh at specified index.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @return poly vtkPolyData - soft reference - must not be freed outside
//////
vtkPolyData* vtkMEDPolyDataDeformationPK::GetInputMesh(int meshIndex) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	return this->inputMeshes[meshIndex];
}

//////
/// Sets output for result of deformable mesh at specified index deformation.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
//////
void vtkMEDPolyDataDeformationPK::SetOutputMesh(int meshIndex, vtkPolyData *poly) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (this->outputMeshes[meshIndex] != poly)
	{
		if (this->outputMeshes[meshIndex] != NULL)
			this->outputMeshes[meshIndex]->UnRegister(this);

		if (NULL != (this->outputMeshes[meshIndex] = poly))
			this->outputMeshes[meshIndex]->Register(this);

		this->Modified();
	}
}


//////
/// Sets output for result of deformable mesh coarse at specified index deformation.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @param poly vtkPolyData - soft reference to preallocated instance - will not be freed nor allocated inside filter
//////
void vtkMEDPolyDataDeformationPK::SetOutputMeshCoarse(int meshIndex, vtkPolyData *poly) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (this->outputMeshesCoarse[meshIndex] != poly)
	{
		if (this->outputMeshesCoarse[meshIndex] != NULL)
			this->outputMeshesCoarse[meshIndex]->UnRegister(this);

		if (NULL != (this->outputMeshesCoarse[meshIndex] = poly))
			this->outputMeshesCoarse[meshIndex]->Register(this);

		this->Modified();
	}
}


//////
/// Gets output for result of deformable mesh at specified index deformation.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @return poly vtkPolyData - soft reference
//////
vtkPolyData* vtkMEDPolyDataDeformationPK::GetOutputMesh(int meshIndex) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	return this->outputMeshes[meshIndex];
}

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
void vtkMEDPolyDataDeformationPK::SetNumberOfMeshSkeletons(int meshIndex, int count) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (count < 0) {
		throw "Skeleton count must be >= 0.";
	}

	// expose old
	this->m_Skeletons = this->multiM_Skeletons[meshIndex];
	this->m_NumberOfSkeletons = this->skeletonCounts[meshIndex];

	// set new
	this->SetNumberOfSkeletons(count);

	// copy back
	this->multiM_Skeletons[meshIndex] = this->m_Skeletons;
	this->skeletonCounts[meshIndex] = this->m_NumberOfSkeletons;

	//clear exposition
	this->m_Skeletons = NULL;
	this->m_NumberOfSkeletons = 0;
}

//////
/// Gets number of skeletons for deformable mesh at specified index.
///
/// Used for memory preallocation.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @return count number of skeletons (input)
//////
int vtkMEDPolyDataDeformationPK::GetNumberOfMeshSkeletons(int meshIndex) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	return this->skeletonCounts[meshIndex];
}

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
void vtkMEDPolyDataDeformationPK::SetMeshSkeleton(int meshIndex, int skeletonIndex, vtkPolyData* original, vtkPolyData* modified, vtkIdList* correspondence, double* original_rso, double* modified_rso) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (skeletonIndex < 0 || skeletonIndex >= this->skeletonCounts[meshIndex]) {
		throw "Skeleton index out of range.";
	}

	// expose old
	this->m_Skeletons = this->multiM_Skeletons[meshIndex];
	this->m_NumberOfSkeletons = this->skeletonCounts[meshIndex];

	// set new
	this->SetNthSkeleton(skeletonIndex, original, modified, correspondence, original_rso, modified_rso);

	// copy back
	this->multiM_Skeletons[meshIndex] = this->m_Skeletons;
	this->skeletonCounts[meshIndex] = this->m_NumberOfSkeletons;

	//clear exposition
	this->m_Skeletons = NULL;
	this->m_NumberOfSkeletons = 0;
}

//*************************************************************************************************************************************
// Hard obstacles (bones)
//*************************************************************************************************************************************

//////
/// Sets number of meshes used as hard obstacles (non-deformable inputs).
///
/// Used for memory preallocation.
/// @param count number of meshes as obstacles (input)
//////
void vtkMEDPolyDataDeformationPK::SetNumberOfObstacles(int count) {
	if (count < 0) {
		throw "Obstacle count must be >= 0.";
	}

	// free

	if (this->obstacles != NULL)
	{
		for (int i = 0; i < this->obstacleCount; i++)
		{
			if (this->obstacles[i] != NULL) {
				this->obstacles[i]->UnRegister(this);
			}
		}

		delete[] this->obstacles;
	}

	if (this->obstaclesCoarse != NULL)
	{
		for (int i = 0; i < this->obstacleCount; i++)
		{
			if (this->obstaclesCoarse[i] != NULL) {
				this->obstaclesCoarse[i]->Delete();
			}
		}

		delete[] this->obstaclesCoarse;
	}

	// allocate

	if ((this->obstacleCount = count) != 0)
	{
		this->obstacles = new vtkPolyData*[count];
		memset(this->obstacles, 0, this->obstacleCount * sizeof(vtkPolyData*));

		this->obstaclesCoarse = new vtkPolyData*[count];
		memset(this->obstaclesCoarse, 0, this->obstacleCount * sizeof(vtkPolyData*));
	}
}

//////
/// Gets number of meshes used as hard obstacles (non-deformable inputs).
///
/// Used for memory preallocation.
/// @return number of meshes as obstacles
//////
int vtkMEDPolyDataDeformationPK::GetNumberOfObstacles() {
	return this->obstacleCount;
}

//////
/// Sets input non-deformable mesh (obstacle) at specified index.
///
/// @param meshIndex index from 0 to GetNumberOfObstacles() - 1 (input)
/// @param poly vtkPolyData - soft reference - will not be freed inside filter (input)
//////
void vtkMEDPolyDataDeformationPK::SetObstacle(int obstacleIndex, vtkPolyData *poly) {
	if (obstacleIndex < 0 || obstacleIndex >= this->obstacleCount) {
		throw "Obstacle index out of range.";
	}
	if (this->obstacles[obstacleIndex] != poly)
	{
		if (this->obstacles[obstacleIndex] != NULL)
			this->obstacles[obstacleIndex]->UnRegister(this);

		if (NULL != (this->obstacles[obstacleIndex] = poly))
			this->obstacles[obstacleIndex]->Register(this);

		this->Modified();
	}
}

//////
/// Gets input non-deformable mesh (obstacle) at specified index.
///
/// @param meshIndex index from 0 to GetNumberOfObstacles() - 1 (input)
/// @return poly vtkPolyData - soft reference - must not be freed outside
//////
vtkPolyData* vtkMEDPolyDataDeformationPK::GetObstacle(int obstacleIndex) {
	if (obstacleIndex < 0 || obstacleIndex >= this->obstacleCount) {
		throw "Obstacle index out of range.";
	}

	return this->obstacles[obstacleIndex];
}

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
void vtkMEDPolyDataDeformationPK::SetCoarseMesh(const int meshIndex, vtkPolyData* coarse) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	if (this->inputMeshesCoarse[meshIndex] != NULL) {
		this->inputMeshesCoarse[meshIndex]->Delete();
		this->inputMeshesCoarse[meshIndex] = NULL;
	}

	if (coarse != NULL) {
		this->inputMeshesCoarse[meshIndex] = vtkPolyData::New();
		this->inputMeshesCoarse[meshIndex]->DeepCopy(coarse);
	}
}

//////
/// Gets coarse mesh for deformable mesh at specified index.
/// This can be mesh from matching setter or calculated mesh.
/// Make deep copy of returned pointer for your needs.
///
/// @param meshIndex index from 0 to GetNumberOfMeshes() - 1 (input)
/// @return vtkPolyData - soft reference - must not be freed outside
//////
vtkPolyData* vtkMEDPolyDataDeformationPK::GetCoarseMesh(const int meshIndex) {
	if (meshIndex < 0 || meshIndex >= this->modelCount) {
		throw "Invalid mesh index.";
	}

	return this->inputMeshesCoarse[meshIndex];
}

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
void vtkMEDPolyDataDeformationPK::SetCoarseObstacle(const int obstacleIndex, vtkPolyData* coarse) {
	if (obstacleIndex < 0 || obstacleIndex >= this->obstacleCount) {
		throw "Obstacle index out of range.";
	}

	if (this->obstaclesCoarse[obstacleIndex] != NULL) {
		this->obstaclesCoarse[obstacleIndex]->Delete();
		this->obstaclesCoarse[obstacleIndex] = NULL;
	}

	if (coarse != NULL) {
		this->obstaclesCoarse[obstacleIndex] = vtkPolyData::New();
		this->obstaclesCoarse[obstacleIndex]->DeepCopy(coarse);
	}
}

//////
/// Gets coarse mesh for obstacle mesh at specified index.
/// This can be mesh from matching setter or calculated mesh.
/// Make deep copy of returned pointer for your needs.
///
/// @param obstacleIndex index from 0 to GetNumberOfObstacles() - 1 (input)
/// @return vtkPolyData - soft reference - must not be freed outside
//////
vtkPolyData* vtkMEDPolyDataDeformationPK::GetCoarseObstacle(const int obstacleIndex) {
	if (obstacleIndex < 0 || obstacleIndex >= this->obstacleCount) {
		throw "Obstacle index out of range.";
	}

	return this->obstaclesCoarse[obstacleIndex];
}

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
void vtkMEDPolyDataDeformationPK::SetPreventCollisions(bool prevent)
{
	if (this->preventCollisions != prevent)
	{
		this->preventCollisions = prevent;
		this->Modified();
	}
}

//////
/// Gets whetter to prevent collisions.
/// With true, intersections of two meshes should be prevented at the price
/// of high computational demands.
/// On false settings, all hard obstacles are ignored as well.
///
/// @return true if prevents collisions
//////
bool vtkMEDPolyDataDeformationPK::GetPreventCollisions() {
	return this->preventCollisions;
}

//////
/// Sets if progressive hulls should be used to represent coarse meshes
/// With true, much slower calculation is used to produce progressive hull
/// that have a tendency to be more stable
///
/// @param enabled true, if progressive hulls should be used
//////
/*virtual*/ void vtkMEDPolyDataDeformationPK::SetUseProgressiveHulls(bool enabled)
{
	if (this->useProgresiveHulls != enabled)
	{
		this->useProgresiveHulls = enabled;
		this->Modified();
	}
}

//////
/// Gets if progressive hulls should be used to represent coarse meshes
/// With true, much slower calculation is used to produce progressive hull
/// that have a tendency to be more stable
///
/// @return true if progressive hulls should be used
//////
/*virtual*/ bool vtkMEDPolyDataDeformationPK::GetUseProgressiveHulls() {
	return this->useProgresiveHulls;
}

//////
/// Sets if grid will be used to boost the ray casting.
///
/// @param enabled true, if grid will be used to boost the ray casting
//////
void vtkMEDPolyDataDeformationPK::SetUseGrid(bool enabled) {
	if (this->useGrid != enabled)
	{
		this->useGrid = enabled;
		this->Modified();
	}
}

//////
/// Gets if grid will be used to boost the ray casting.
///
/// @return true, if grid will be used to boost the ray casting
//////
bool vtkMEDPolyDataDeformationPK::GetUseGrid() {
	return this->useGrid;
}

//////
/// Sets if the method should visualize its progress
///
/// @param enabled true, if the method should visualize its progress
//////
void vtkMEDPolyDataDeformationPK::SetDebugMode(bool enabled) {
	if (this->debugMode != enabled)
	{
		this->debugMode = enabled;
		this->Modified();
	}
}

//////
/// Gets if grid will be used to boost the ray casting.
///
/// @return true, if debug mode is on and the method will visualize its progress
//////
bool vtkMEDPolyDataDeformationPK::GetDebugMode() {
	return this->debugMode;
}

#pragma endregion //IMultiMeshDeformer
#pragma endregion //my code

void vtkMEDPolyDataDeformationPK::AddWrapper(vtkPolyData* w) {
	if (this->nWrappers > 0) {
		vtkPolyData** temp = this->wrapper;
		this->wrapper = new vtkPolyData*[nWrappers+1];
		for(int i = 0; i < nWrappers; i++) {
			this->wrapper[i] = temp[i];
		}
		delete[] temp;
	}
	else {
		this->wrapper = new vtkPolyData*[1];
	}
	this->wrapper[nWrappers] = vtkPolyData::New();
	this->wrapper[nWrappers]->DeepCopy(w);
	this->nWrappers++;
}

void vtkMEDPolyDataDeformationPK::FreeWrappers() {
	if (this->nWrappers > 0 || this->wrapper == NULL) {
		for(int i = 0; i < this->nWrappers; i++) {
			this->wrapper[i]->Delete();
		}
		delete[] this->wrapper;
		this->nWrappers = 0;
	}
}