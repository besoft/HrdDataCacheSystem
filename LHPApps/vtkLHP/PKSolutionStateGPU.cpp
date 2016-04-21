/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKSolutionStateGPU.cpp,v $ 
  Language: C++ 
  Date: $Date: 2012-05-09 $ 
  Version: $Revision: 1.0 $ 
  Authors: Ladislav Cmolik
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "PKSolutionStateGPU.h"


PKSolutionStateGPU::PKSolutionStateGPU(int matrixHeight, int matrixWidth, int numberOfDimensions, int nOriginalMeshPoints) {
	
	this->matrixHeight = matrixHeight;
	this->matrixWidth = matrixWidth;
	this->numberOfDimensions = numberOfDimensions;

	this->originalVolume = 0;
	this->originalVolumeCoarse = 0;
	this->coarseToOrigVolRatio = 0;
	this->volumeLostByFix = 0;

	this->solutionK = NULL;
	this->solutionKPlus1 = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
	this->Lt = PKUtils::CreateMatrix(matrixWidth, matrixHeight);
	this->LtL = PKUtils::CreateMatrix(matrixWidth, matrixWidth);
	this->LtLInv = PKUtils::CreateMatrix(matrixWidth, matrixWidth);
	this->Jg = PKUtils::CreateMatrix(numberOfDimensions, matrixWidth);
	this->JgT = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
	this->h = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
	
	this->gpu_L = NULL;
	this->gpu_B = NULL;

	this->gpu_LtLInv = NULL;
	this->gpu_Lx_minus_b = NULL;

	this->gpu_solutionK = NULL;
	this->gpu_solutionKPlus1 = NULL;
	this->gpu_Lt_dot_Lx_minus_b = NULL;
	this->gpu_Lt_dot_Lx_minus_bT = NULL;
	this->gpu_JgT = NULL;
	this->gpu_Jg = NULL;
	this->gpu_Jg_LtLInv = NULL;
	this->gpu_Jg_LtLInvT = NULL;

	this->blockedVertices = new bool[matrixWidth];
	memset(this->blockedVertices, 0, sizeof(bool) * matrixWidth);

	this->bufferHighResMatrix = PKUtils::CreateMatrix(nOriginalMeshPoints, 3);
}


PKSolutionStateGPU::~PKSolutionStateGPU(void)
{
	PKUtils::DisposeMatrix(&(this->solutionKPlus1));
	PKUtils::DisposeMatrix(&(this->Lt));
	PKUtils::DisposeMatrix(&(this->LtL));
	PKUtils::DisposeMatrix(&(this->LtLInv));
	PKUtils::DisposeMatrix(&(this->Jg));
	PKUtils::DisposeMatrix(&(this->JgT));
	PKUtils::DisposeMatrix(&(this->h));

	PKUtils::DisposeMatrix(&(this->bufferHighResMatrix));
	
	delete[] this->blockedVertices;
	this->blockedVertices = NULL;
}
