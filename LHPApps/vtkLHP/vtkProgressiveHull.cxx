/*========================================================================= 
Program: Musculoskeletal Modeling (VPHOP WP10)
Module: $RCSfile: vtkProgressiveHull.cxx,v $ 
Language: C++ 
Date: $Date: 2011-09-07 05:42:26 $ 
Version: $Revision: 1.1.2.4 $ 
Authors: Tomas Janak
Notes: 
========================================================================== 
Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details 
=========================================================================
*/
#ifdef _WINDOWS
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkProgressiveHull.h"
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

#ifdef _USE_CUDA
#include "vtkProgressiveHullCUDA.h"
#include "ProgressiveHullCUDALib.h"
#endif

#ifdef _USE_LP_SOLVE
#pragma warning(push)
#pragma warning(disable: 4996)	//VTK uses strcpy, which is considered unsafe by VS
#include "vtkProgressiveHullCPU.h"
#pragma warning(pop)
#include "lp_lib.h"
#pragma comment(lib, "lpsolve55")
#else
#pragma message("WARNING: vtkProgressiveHull will not work because LP_SOLVE library is not available.")
#endif

vtkInstantiatorNewMacro(vtkProgressiveHull);

int vtkProgressiveHull::useCuda = 2; // 2 == not tested yet

//----------------------------------------------------------------------------
//retrieve instance of the class
/*static*/ vtkProgressiveHull* vtkProgressiveHull::New()
{
	Log("Progressive Hull instance created");

#ifdef _USE_CUDA
	// first try to initialize the parallel CUDA version
	if (vtkProgressiveHull::useCuda == 2) // if the test havent been made yet, do it
		IsCudaAvailable();
	if (vtkProgressiveHull::useCuda)
	{
		Log("CUDA version will be used");
		return vtkProgressiveHullCUDA::New();
	}
	else
	{
		Log("CPU version will be used");
		return vtkProgressiveHullCPU::New();
	}
#else	
#ifdef _USE_LP_SOLVE
	return vtkCPUProgressiveHull::New();
#else
	return NULL;
#endif
#endif		
}

/** Returns true if it is possible to use the cuda version of progressive hull */
bool vtkProgressiveHull::IsCudaAvailable()
{
#ifdef _USE_CUDA
	bool ret = ProgressiveHullCUDA::IsCudaCapable();
	if (ret) vtkProgressiveHull::useCuda = 1;
	else vtkProgressiveHull::useCuda = 0;
	return ret;
#else	
	vtkProgressiveHull::useCuda = 0;
	return false;
#endif	
}

#endif