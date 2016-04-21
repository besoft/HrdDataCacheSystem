/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKSolutionState.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-04-11 07:16:04 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "PKSolutionState.h"


PKSolutionState::PKSolutionState(int matrixHeight, int matrixWidth, int numberOfDimensions, int nOriginalMeshPoints)
{
	this->solutionK = NULL;
	this->originalVolume = 0;
	this->originalVolumeCoarse = 0;
	this->coarseToOrigVolRatio = 0;
	this->volumeLostByFix = 0;

	this->solutionKPlus1 = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
	this->Lt = PKUtils::CreateMatrix(matrixWidth, matrixHeight);
	this->LtL = PKUtils::CreateMatrix(matrixWidth, matrixWidth);
	this->Jg = PKUtils::CreateMatrix(numberOfDimensions, matrixWidth);
	this->JgT = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);
	this->Lx_minus_b = PKUtils::CreateMatrix(matrixHeight, numberOfDimensions); // possible to avoid
	this->Lt_dot_Lx_minus_b = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions); 
	this->Jg_LtLInv = PKUtils::CreateMatrix(numberOfDimensions, matrixWidth); // could be avoided?
	this->Jg_LtLInvLin = PKUtils::CreateMatrix(1, this->Jg_LtLInv->height * this->Jg_LtLInv->width);
	this->Lt_dot_Lx_minus_b_Lin = PKUtils::CreateMatrix(1, this->Lt_dot_Lx_minus_b->height * this->Lt_dot_Lx_minus_b->width);
	this->JgLin = PKUtils::CreateMatrix(1, this->Jg->width * this->Jg->height);
	this->LtLInv = PKUtils::CreateMatrix(matrixWidth, matrixWidth);
	this->h = PKUtils::CreateMatrix(matrixWidth, numberOfDimensions);

	this->blockedVertices = new bool[matrixWidth];
	memset(this->blockedVertices, 0, sizeof(bool) * matrixWidth);

	this->bufferHighResMatrix = PKUtils::CreateMatrix(nOriginalMeshPoints, 3);
}


PKSolutionState::~PKSolutionState(void)
{
	PKUtils::DisposeMatrix(&(this->solutionKPlus1));
	PKUtils::DisposeMatrix(&(this->Lt));
	PKUtils::DisposeMatrix(&(this->LtL));
	PKUtils::DisposeMatrix(&(this->Jg));
	PKUtils::DisposeMatrix(&(this->JgT));
	PKUtils::DisposeMatrix(&(this->Lx_minus_b));
	PKUtils::DisposeMatrix(&(this->Lt_dot_Lx_minus_b));
	PKUtils::DisposeMatrix(&(this->Jg_LtLInv));
	PKUtils::DisposeMatrix(&(this->Jg_LtLInvLin));
	PKUtils::DisposeMatrix(&(this->Lt_dot_Lx_minus_b_Lin));
	PKUtils::DisposeMatrix(&(this->JgLin));
	PKUtils::DisposeMatrix(&(this->LtLInv));
	PKUtils::DisposeMatrix(&(this->h));

	PKUtils::DisposeMatrix(&(this->bufferHighResMatrix));

	delete[] this->blockedVertices;
	this->blockedVertices = NULL;
}
