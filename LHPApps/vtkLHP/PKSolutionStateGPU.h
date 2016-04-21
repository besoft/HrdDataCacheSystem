/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKSolutionStateGPU.h,v $ 
  Language: C++ 
  Date: $Date: 2012-05-09 $ 
  Version: $Revision: 1.0 $ 
  Authors: Ladislav Cmolik
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef PKSolutionStateGPU_h__
#define PKSolutionStateGPU_h__

#pragma once

#include "PKUtils.h"
#include "GPU_solver.h"

class PKSolutionStateGPU {
public:
	//nOriginalMeshPoints = number of points in the original high resolution mesh being deformed
	PKSolutionStateGPU(int matrixHeight, int matrixWidth, int numberOfDimensions, int nOriginalMeshPoints);
	~PKSolutionStateGPU(void);

	int matrixHeight;
	int matrixWidth;
	int numberOfDimensions;

	double originalVolume;
	double originalVolumeCoarse;
	double coarseToOrigVolRatio;
	double volumeLostByFix;

	PKMatrix *solutionK;
	PKMatrix *solutionKPlus1;
	PKMatrix *Lt;
	PKMatrix *LtL;
	PKMatrix *LtLInv;
	PKMatrix *Jg;
	PKMatrix *JgT;
	PKMatrix *h;

	PKMatrix* bufferHighResMatrix;

	gpu_double *gpu_L;
	gpu_double *gpu_B;
	gpu_double *gpu_solutionK;

	gpu_double *gpu_solutionKPlus1;
	gpu_double *gpu_LtLInv;
	gpu_double *gpu_Lx_minus_b;
	gpu_double *gpu_Lt_dot_Lx_minus_b;
	gpu_double *gpu_Lt_dot_Lx_minus_bT;
	gpu_double *gpu_JgT;
	gpu_double *gpu_Jg;
	gpu_double *gpu_Jg_LtLInv;
	gpu_double *gpu_Jg_LtLInvT;

	bool *blockedVertices;
};

#endif