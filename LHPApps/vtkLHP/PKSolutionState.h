/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKSolutionState.h,v $ 
  Language: C++ 
  Date: $Date: 2012-04-16 06:39:25 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef PKSolutionState_h__
#define PKSolutionState_h__

#pragma once

// my classes
#include "PKUtils.h"

class PKSolutionState
{
public:
	//nOriginalMeshPoints = number of points in the original high resolution mesh being deformed
	PKSolutionState(int matrixHeight, int matrixWidth, int numberOfDimensions, int nOriginalMeshPoints);
	~PKSolutionState(void);

public:	
	double originalVolume; // objem jemne site pred deformaci
	double originalVolumeCoarse; // objem obalky pred deformaci
	double coarseToOrigVolRatio; // pomer objemu originalu a obalky
	double volumeLostByFix;

	PKMatrix *solutionK; // matice urcujici soucasnou pozici vrcholu (relativne vyjadreno)
	PKMatrix *solutionKPlus1; // matice pro pozici vrcholu v nasledne iteraci
	PKMatrix *Lt;			  // transponovana laplacianovska matice (vyjadruje vztahy mezi vrcholy)
	PKMatrix *LtL;			  // transponovana * L
	PKMatrix *Jg;			  // Jacobiho matice (derivace) objemu
	PKMatrix *JgT;			  // Jg transponovana
	PKMatrix *Lx_minus_b; // possible to avoid
	PKMatrix *Lt_dot_Lx_minus_b; 
	PKMatrix *Jg_LtLInv; // could be avoided?
	PKMatrix *Jg_LtLInvLin;
	PKMatrix *Lt_dot_Lx_minus_b_Lin;
	PKMatrix* JgLin;
	PKMatrix *LtLInv;
	PKMatrix *h;			  // ??? h je vektor popisujici posun ???

	PKMatrix* bufferHighResMatrix;	///BES: 3.12.2012 - hotfix to speed up MeshSurface::ApplyCoarseCoordsToOriginalMesh

	bool *blockedVertices;
};




#endif