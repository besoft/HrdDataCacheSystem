/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonemat.cpp,v $
  Language:  C++
  Date:      $Date: 2011-12-12 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.4 $
  Authors:   Daniele Giunchi, Stefano Perticoni, Gianluigi Crimi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpBonemat.h"
#include "lhpProceduralElements.h"
#include "lhpVolumeInterpolator.h"

#include "mafGUI.h"
#include "mafDecl.h"
#include "mafVMERoot.h"
#include "mafVMEMesh.h"
#include "mafString.h"
#include "mafAbsMatrixPipe.h"

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "vtkMAFSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkImageData.h"
#include "vtkDoubleArray.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"


//Static data
lhpVolumeInterpolator *staticVolumeInterpolatorPointer;
Calibration staticCalibration;


//----------------------------------------------------------------------------
lhpOpBonemat::lhpOpBonemat(wxString label) : lhpOpBonematCommon(label)
//----------------------------------------------------------------------------
{  
}
//----------------------------------------------------------------------------
lhpOpBonemat::~lhpOpBonemat()
//----------------------------------------------------------------------------
{  
}

//----------------------------------------------------------------------------
mafOp* lhpOpBonemat::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpBonemat *cp = new lhpOpBonemat(m_Label);
  return cp;
}

//----------------------------------------------------------------------------
int lhpOpBonemat::Execute()
//----------------------------------------------------------------------------
{
  FILE  *freq_fp;
  vtkIdType  numElements;
  lhpVolumeInterpolator volumeInterpolator;

	CreateMeshCopy();
  
  if ( (freq_fp = fopen(m_FrequencyFileName.GetCStr(), "w")) == NULL)
  {
    if (GetTestMode() == false)
    {
      wxMessageBox("Frequency file can't be opened");
    }
   
    assert(false);
    return MAF_ERROR;
  }
  
	mafMatrix inputMeshABSMatrix = mafVMEMesh::SafeDownCast(m_OutputMesh)->GetAbsMatrixPipe()->GetMatrix();
	mafMatrix inputVolumeABSMatrix = mafVMEVolumeGray::SafeDownCast(m_InputVolume)->GetAbsMatrixPipe()->GetMatrix();
	vtkUnstructuredGrid *inputUG =mafVMEMesh::SafeDownCast(m_OutputMesh)->GetUnstructuredGridOutput()->GetUnstructuredGridData();
  vtkUnstructuredGrid *inputTrasformedUG;
	bool meshTranformed=GetTrasformdedMesh(inputUG,&inputTrasformedUG,inputMeshABSMatrix,inputVolumeABSMatrix);
  
	volumeInterpolator.SetVolume(m_InputVolume->GetOutput()->GetVTKData());

	//setting static data for integration
	staticVolumeInterpolatorPointer=&volumeInterpolator;
	staticCalibration=m_Calibration;
  
  numElements = inputTrasformedUG->GetNumberOfCells();

  // Array containing properties for each element from 0 to numElements - 1
	ElementProp *elProps;
  elProps = new ElementProp[numElements];
 
	if (m_YoungModuleCalculationModality == HU_INTEGRATION)
		HUIntegration(inputTrasformedUG, elProps);
	else 
		EIntegration(inputTrasformedUG, elProps);
	  
	GenerateArraysAndFieldData(freq_fp, numElements, elProps, inputUG);

	//Memory Clean
	if (meshTranformed)
		 vtkDEL(inputTrasformedUG);
  delete [] elProps;

	return MAF_OK;
}

//----------------------------------------------------------------------------
void lhpOpBonemat::HUIntegration(vtkUnstructuredGrid *inputTrasformedUG, ElementProp *elProps)
//----------------------------------------------------------------------------
{
	long progress = 0;
	double elVolume,elIntegral;
	double HU;
	Element *element;  
	vtkIdType  numElements;

	numElements = inputTrasformedUG->GetNumberOfCells();

	InitProgressBar("Computing elements data...");

	for (vtkIdType elementNumber=0; elementNumber < numElements; elementNumber++) 
	{ 
		element = CreateElementFromCell(inputTrasformedUG->GetCell(elementNumber));
		element->Integrate(ComputeVolumeIntegrand, m_StepsNumber,elIntegral,elVolume);
		delete element;

		HU=elIntegral/elVolume;

		elProps[elementNumber].rhoQCT = RhoQCTFromHU(HU, &m_Calibration);
		elProps[elementNumber].rhoAsh = RhoAshFromRhoQCT(elProps[elementNumber].rhoQCT, &m_Calibration);
		elProps[elementNumber].E = YoungModuleFromRho(elProps[elementNumber].rhoAsh, &m_Calibration);
		elProps[elementNumber].elementID = elementNumber;

		progress = (elementNumber + 1) * 100 / numElements;
		UpdateProgressBar(progress);
	}

	CloseProgressBar();
}

//----------------------------------------------------------------------------
void lhpOpBonemat::EIntegration(vtkUnstructuredGrid * inputTrasformedUG, ElementProp * elProps)
//----------------------------------------------------------------------------
{
	long progress = 0;
	double elVolume,elEintegral,elIntegral;
	double HU;
	Element *element;  
	vtkIdType  numElements;

	numElements = inputTrasformedUG->GetNumberOfCells();

	InitProgressBar("Computing elements data...");

	for (vtkIdType elementNumber=0; elementNumber < numElements; elementNumber++) 
	{ 
		element = CreateElementFromCell(inputTrasformedUG->GetCell(elementNumber));
		element->Integrate(ComputeVolumeIntegrand, m_StepsNumber,elIntegral,elVolume);
		element->Integrate(ComputeVolumeYoungIntegrand, m_StepsNumber,elEintegral,elVolume);
		delete element;

		HU=elIntegral/elVolume;

		elProps[elementNumber].rhoQCT = RhoQCTFromHU(HU, &m_Calibration);
		elProps[elementNumber].rhoAsh = RhoAshFromRhoQCT(elProps[elementNumber].rhoQCT, &m_Calibration);
		elProps[elementNumber].E = elEintegral/elVolume;
		elProps[elementNumber].elementID = elementNumber;

		progress = (elementNumber + 1) * 100 / numElements;
		UpdateProgressBar(progress);
	}
	
	CloseProgressBar();
}

//----------------------------------------------------------------------------
void lhpOpBonemat::CreateBins(FILE *freq_fp, int numElements, ElementProp *elProps, std::vector<ElementProp> *materialProperties, std::vector <int> *frequences)
//----------------------------------------------------------------------------
{
	typedef std::vector<int> idVectorType;
	idVectorType idVector;
	vtkIdType freq, numMats;
	double rhoQCTAccumulator,rhoAshAccumulator,rhoQCT,rhoAsh, E;

	// COMPUTE MATERIALS & WRITE FREQUENCY FILE
	qsort(elProps, numElements, sizeof(ElementProp), compareE);

	mafLogMessage( "-- Writing frequency file\n");

	fprintf(freq_fp, "rho \t\t E \t\t NUMBER OF ELEMENTS\n\n");

	freq = 1;
	numMats = 1;
	materialProperties->push_back(elProps[0]);
	E = elProps[0].E;  
	rhoQCT = rhoQCTAccumulator = elProps[0].rhoQCT;
	rhoAsh = rhoAshAccumulator = elProps[0].rhoAsh;

	// grouping materials according to E value
	for (int id = 1; id < numElements; id++) 
	{
		if (E - elProps[id].E > m_Egap) // generate statistics for old group and create a new group
		{
			if(m_DensitySelection == USE_MEAN_DENSISTY)
			{
				rhoQCT = (*materialProperties)[numMats-1].rhoQCT = rhoQCTAccumulator / freq;
				rhoAsh = (*materialProperties)[numMats-1].rhoAsh = rhoAshAccumulator / freq;
			}

			// print statistics
			fprintf(freq_fp, "%f \t %f \t %d\n", (m_RhoSelection == USE_RHO_QCT) ? rhoQCT : rhoAsh, E, freq); 

			rhoQCT = rhoQCTAccumulator = elProps[id].rhoQCT;
			rhoAsh = rhoAshAccumulator = elProps[id].rhoAsh;

			materialProperties->push_back(elProps[id]);
			frequences->push_back(freq);

			E = elProps[id].E;
			numMats++;
			freq = 1;
		}
		else
		{
			rhoQCTAccumulator+= elProps[id].rhoQCT;
			rhoAshAccumulator+= elProps[id].rhoAsh;
			freq++;
		}
	}

	if(m_DensitySelection == USE_MEAN_DENSISTY)
	{
		rhoQCT = (*materialProperties)[numMats-1].rhoQCT = rhoQCTAccumulator / freq;
		rhoAsh = (*materialProperties)[numMats-1].rhoAsh = rhoAshAccumulator / freq;
	}
	frequences->push_back(freq);

	// print statistics
	fprintf(freq_fp, "%f \t %f \t %d\n", (m_RhoSelection == USE_RHO_QCT) ? rhoQCT : rhoAsh, E, freq); 
}

//----------------------------------------------------------------------------
bool lhpOpBonemat::GetTrasformdedMesh(vtkUnstructuredGrid *inputUG, vtkUnstructuredGrid **outputUG, mafMatrix meshAbsMatrix, mafMatrix volumeAbsMatrix)
//----------------------------------------------------------------------------
{
	mafMatrix identityMatrix;

	bool isMeshMatrixIdentity = meshAbsMatrix.Equals(&identityMatrix);
	bool isVolumeMatrixIdentity = volumeAbsMatrix.Equals(&identityMatrix);

	// this will feed the algorithm...
	inputUG->Update();

	if (isMeshMatrixIdentity && isVolumeMatrixIdentity)
		*outputUG=inputUG;
	else
	{		
		// if VME matrix is not identity apply it to dataset
		vtkTransform *transform = NULL;
		vtkTransformFilter *transformFilter = NULL;
		mafMatrix meshVolumeAlignMatrix;

		//Calculate align matrix 
		volumeAbsMatrix.Invert();
		mafMatrix::Multiply4x4(volumeAbsMatrix,meshAbsMatrix,meshVolumeAlignMatrix);

		// apply abs matrix to geometry
		transform = vtkTransform::New();
		transform->SetMatrix(meshVolumeAlignMatrix.GetVTKMatrix());
	
		// to delete
		transformFilter = vtkTransformFilter::New();

		*outputUG = vtkUnstructuredGrid::New();

		transformFilter->SetInput(inputUG);
		transformFilter->SetTransform(transform);
		transformFilter->Update();

		(*outputUG)->DeepCopy(transformFilter->GetUnstructuredGridOutput());

		vtkDEL(transform);
		vtkDEL(transformFilter);
	}
	
	return !isMeshMatrixIdentity;
}

//----------------------------------------------------------------------------
Element* lhpOpBonemat::CreateElementFromCell(vtkCell *cell)
//----------------------------------------------------------------------------
{
	Element *element;
	vtkIdType numElementNodes;
	numElementNodes = cell->GetNumberOfPoints();

	//element control
	if (numElementNodes == 4) 
		element = Tetra::New();
	else if (numElementNodes == 10) 
		element = Tetra10::New();
	else if (numElementNodes == 8 || numElementNodes == 20) 
		element = Hexa::New();
	else
		element = Wedge::New();

	for(int num = 0; num < numElementNodes; num++)
	{
		Node *current = new Node;
		current->key = cell->GetPointId(num);
		double coordinates[3];
		cell->GetPoints()->GetPoint(num, coordinates);
		current->x[0] = coordinates[0];
		current->x[1] = coordinates[1];
		current->x[2] = coordinates[2];

		element->SetNode(num, current);
	}
	return element;
}

//----------------------------------------------------------------------------
double lhpOpBonemat::RhoQCTFromHU(double HU, Calibration *calibration)
//----------------------------------------------------------------------------
{
	// rho = a + b * HU
	double rhoQCT = calibration->rhoIntercept + calibration->rhoSlope * HU;
	return (rhoQCT <= 0) ? 1e-6: rhoQCT;
}

//----------------------------------------------------------------------------
double lhpOpBonemat::RhoAshFromRhoQCT(double rhoQCT, Calibration *calibration)
//----------------------------------------------------------------------------
{
	//RHO CORRECTION///////////////////////////////////////////////////////////////
	if(calibration->rhoCalibrationCorrectionIsActive)
	{
		if(calibration->rhoCalibrationCorrectionType == SINGLE_INTERVAL)
			rhoQCT = calibration->a_CalibrationCorrection + calibration->b_CalibrationCorrection * rhoQCT;
		else if (calibration->rhoCalibrationCorrectionType == THREE_INTERVALS)
		{
			if (rhoQCT < calibration->rhoQCT1)
				rhoQCT = calibration->a_RhoQCTLessThanRhoQCT1 + calibration->b_RhoQCTLessThanRhoQCT1 * rhoQCT;
			else if (calibration->rhoQCT1 <= rhoQCT  && rhoQCT <= calibration->rhoQCT2)
				rhoQCT = calibration->a_RhoQCTBetweenRhoQCT1AndRhoQCT2 + calibration->b_RhoQCTBetweenRhoQCT1AndRhoQCT2 * rhoQCT;
			else if (rhoQCT > calibration->rhoQCT2)
				rhoQCT = calibration->a_RhoQCTBiggerThanRhoQCT2 + calibration->b_RhoQCTBiggerThanRhoQCT2 * rhoQCT;
		}
	}
	return rhoQCT;
}

//----------------------------------------------------------------------------
double lhpOpBonemat::YoungModuleFromRho(double rho, Calibration *calibration)
//----------------------------------------------------------------------------
{
	// E = a + b * rho ^ c
	double youngModule;

	if (calibration->densityIntervalsNumber == SINGLE_INTERVAL)
		youngModule = calibration->a_OneInterval + calibration->b_OneInterval * pow(rho, calibration->c_OneInterval); 
	else if (calibration->densityIntervalsNumber == THREE_INTERVALS)
	{
		if (rho < calibration->rhoAsh1)
			youngModule = calibration->a_RhoAshLessThanRhoAsh1 + calibration->b_RhoAshLessThanRhoAsh1 * pow(rho, calibration->c_RhoAshLessThanRhoAsh1);
		else if (calibration->rhoAsh1 <= rho  && rho <= calibration->rhoAsh2)
			youngModule = calibration->a_RhoAshBetweenRhoAsh1andRhoAsh2 + calibration->b_RhoAshBetweenRhoAsh1andRhoAsh2 * pow(rho, calibration->c_RhoAshBetweenRhoAsh1andRhoAsh2);
		else if (rho > calibration->rhoAsh2)
			youngModule = calibration->a_RhoAshBiggerThanRhoAsh2 + calibration->b_RhoAshBiggerThanRhoAsh2 * pow(rho, calibration->c_RhoAshBiggerThanRhoAsh2);
	}
	
	return (youngModule <= 0) ? 1e-6 : youngModule;
}

//----------------------------------------------------------------------------
void lhpOpBonemat::UpdateOutputMesh(vtkUnstructuredGrid *inputUG, vtkIntArray *arrayMaterial, vtkDoubleArray *arrayE, vtkDoubleArray *arrayPoisson, vtkDoubleArray *arrayRho, vtkFieldData *fdata)
//----------------------------------------------------------------------------
{
	vtkMAFSmartPointer<vtkUnstructuredGrid> outputUG;
	outputUG->SetPoints(inputUG->GetPoints());
	outputUG->SetCells(inputUG->GetCellTypesArray(),inputUG->GetCellLocationsArray(),inputUG->GetCells());
	outputUG->Update();

	outputUG->GetPointData()->DeepCopy(inputUG->GetPointData());
	outputUG->GetFieldData()->DeepCopy(inputUG->GetFieldData());

	vtkCellData *outCellData = outputUG->GetCellData();
	outCellData->DeepCopy(inputUG->GetCellData());
	outCellData->AddArray(arrayMaterial);
	outCellData->AddArray(arrayE);
	outCellData->AddArray(arrayPoisson);
	outCellData->AddArray(arrayRho);
	outputUG->SetFieldData(fdata);

	outputUG->Modified();
	outputUG->Update();

	mafVMEMesh::SafeDownCast(m_OutputMesh)->SetData(outputUG, 0);
}

//----------------------------------------------------------------------------
void lhpOpBonemat::FillDataArrays(int numElements, std::vector<ElementProp> materialProperties, std::vector<int> frequences, ElementProp *elemProperties, vtkFieldData *fdata,vtkDoubleArray *arrayRho, vtkDoubleArray *arrayE, vtkIntArray *arrayMaterial,vtkDoubleArray *arrayPoisson)
//----------------------------------------------------------------------------
{
	arrayRho->SetNumberOfTuples(numElements);
	arrayE->SetNumberOfTuples(numElements);
	arrayPoisson->SetNumberOfTuples(numElements);
	arrayMaterial->SetNumberOfTuples(numElements);

	int elemPos=0;
	int materialIndex=0;
	int numMaterial=materialProperties.size();

	for(int i=0;i<numMaterial;i++)
	{
		for(int j=0;j<frequences[i];j++)
		{
			vtkIdType elemIndex=elemProperties[elemPos].elementID;
			if (m_RhoSelection==USE_RHO_ASH)
				arrayRho->SetValue(elemIndex,materialProperties[materialIndex].rhoAsh);
			else 
				arrayRho->SetValue(elemIndex,materialProperties[materialIndex].rhoQCT);
			arrayE->SetValue(elemIndex,materialProperties[materialIndex].E);
			arrayMaterial->SetValue(elemIndex,materialIndex+1);
			arrayPoisson->SetValue(elemIndex,0.3);
			elemPos++;
		}
		materialIndex++;
	}
}

//----------------------------------------------------------------------------
vtkFieldData* lhpOpBonemat::GenerateFieldData(std::vector <ElementProp>materialProperties)
//----------------------------------------------------------------------------
{
	vtkIdType numMats=materialProperties.size();
	char *vectorNames[3] = {"EX","NUXY","DENS"};
	
	vtkFieldData * fdata=vtkFieldData::New();

	vtkDoubleArray *iarr = vtkDoubleArray::New();
	iarr->SetName("material_id");
	iarr->SetNumberOfValues(numMats);
	for (int j = 0; j < numMats; j++)
		iarr->InsertValue(j,  j + 1);

	// add the ith data array to the field data
	fdata->AddArray(iarr);
	//clean up
	iarr->Delete();
	
	// create field data data array
	for (int i = 0; i < 3; i++)
	{
		// create the ith data array
		vtkDoubleArray *darr = vtkDoubleArray::New();
		darr->SetName(vectorNames[i]);
		darr->SetNumberOfValues(numMats);

		for (int j = 0; j < numMats; j++)
		{
			// fill ith data array with jth value 
			// cycle on materials
			if(i==0)
				darr->InsertValue(j, materialProperties[j].E);
			else if(i==1)
				darr->InsertValue(j, 0.3);
			else if(i==2 && m_RhoSelection == USE_RHO_ASH)
				darr->InsertValue(j, materialProperties[j].rhoAsh);
			else if(i==2 && m_RhoSelection == USE_RHO_QCT)
				darr->InsertValue(j, materialProperties[j].rhoQCT);
		}
		// add the ith data array to the field data
		fdata->AddArray(darr);

		//clean up
		darr->Delete();
	}

	return fdata;
}

//----------------------------------------------------------------------------
void lhpOpBonemat::GenerateArraysAndFieldData(FILE *freq_fp, vtkIdType numElements, ElementProp *elProps, vtkUnstructuredGrid *inputUG)
//----------------------------------------------------------------------------
{
	std::vector <ElementProp> materialProperties;
	std::vector <int> frequences;

	CreateBins(freq_fp, numElements, elProps, &materialProperties, &frequences);

	fclose(freq_fp);

	//fielddata materials
	vtkFieldData *fdata = GenerateFieldData(materialProperties);
	vtkIntArray *arrayMaterial;
	vtkDoubleArray *arrayE, *arrayPoisson, *arrayRho;
	arrayMaterial=NULL;
	arrayE=arrayPoisson=arrayRho=NULL;

	vtkNEW(arrayMaterial);
	arrayMaterial->SetName("material");
	vtkNEW(arrayE);
	arrayE->SetName("EX");
	vtkNEW(arrayPoisson);
	arrayPoisson->SetName("NUXY");
	vtkNEW(arrayRho);
	arrayRho->SetName("DENS");

	FillDataArrays(numElements,materialProperties,frequences,elProps,fdata,arrayRho,arrayE,arrayMaterial,arrayPoisson);

	UpdateOutputMesh(inputUG, arrayMaterial, arrayE, arrayPoisson, arrayRho, fdata);

	fdata->Delete();
	vtkDEL(arrayMaterial);
	vtkDEL(arrayRho);
	vtkDEL(arrayE);
	vtkDEL(arrayPoisson);
	materialProperties.clear();
}

//----------------------------------------------------------------------------
double lhpOpBonemat::YoungModuleFromHu(double HU)
//----------------------------------------------------------------------------
{
	double rhoQCT = RhoQCTFromHU(HU, &staticCalibration);
	double rhoAsh = RhoAshFromRhoQCT(rhoQCT, &staticCalibration);
	return YoungModuleFromRho(rhoAsh, &staticCalibration);
}

//----------------------------------------------------------------------------
int lhpOpBonemat::compareE(const void *p1, const void *p2) 
//----------------------------------------------------------------------------
{
	double result;
	// decreasing order 
	result = ((ElementProp *) p2)->E - ((ElementProp *) p1)->E;  
	if (result < 0)
		return -1;
	if (result > 0)
		return 1;
	return 0;  
}
//----------------------------------------------------------------------------
double lhpOpBonemat::ComputeVolumeIntegrand(double pos[3])
//----------------------------------------------------------------------------
{
	return staticVolumeInterpolatorPointer->Interpolate(pos);
}
//----------------------------------------------------------------------------
double lhpOpBonemat::ComputeVolumeYoungIntegrand(double pos[3])
//----------------------------------------------------------------------------
{
	return staticVolumeInterpolatorPointer->Interpolate(pos,lhpOpBonemat::YoungModuleFromHu);
}
