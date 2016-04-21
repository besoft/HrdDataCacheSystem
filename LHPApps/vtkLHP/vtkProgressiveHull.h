/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkProgressiveHull.h,v $ 
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


#ifndef __vtkProgressiveHull_h
#define __vtkProgressiveHull_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkstd/vector"
#include "vtkstd/algorithm"
#include "vtkPolyDataToPolyDataFilter.h"

#include "vtkLHPConfigure.h"

#ifdef _WINDOWS	
#define _USE_LP_SOLVE
#define _USE_CUDA
#endif

/** vtkProgressiveHull computes an outer hull of the input mesh.
It assumes that the input is a manifold triangular (surface)mesh. 
The filter is based on the paper by Nikos Platis, Theoharis Theoharis:
Progressive Hulls for Intersection Applications,
Computer Graphics Forum - CGF , vol. 22, no. 2, pp. 107-116, 2003.
This filter automatically detects HW and depending on the outcome,
it executes CPU or slightly faster CUDA implementation.
If you want to use CPU or CUDA version directly (not recommended), 
create instance of vtkProgressiveHullCPU or vtkProgressiveHullCUDA 
instead of instancing vtkProgressiveHull.
N.B. Using vtkProgressiveHullXXX directly opens new options. */
class VTK_vtkLHP_EXPORT vtkProgressiveHull : public vtkPolyDataToPolyDataFilter
{
public:
	/** static variable to control, if CUDA can be used
	Set this 0 to disable using CUDA at all. */
	static int useCuda;

  vtkTypeMacro(vtkProgressiveHull, vtkPolyDataToPolyDataFilter);  

public:
	/** Retrieve instance of the class */
	static vtkProgressiveHull *New();	

	/** Returns true if it is possible to use the cuda version of progressive hull */
	static bool IsCudaAvailable();

	/** Get ratio of points to be removed */
	vtkGetMacro(TargetReduction, double);

	/** Set ratio of points to be removed. 
	Value ranges from 0 to 1 (maximal decimation). */
	vtkSetMacro(TargetReduction, double);

	/** Get tolerance of decimation */
	vtkGetMacro(MeshRoughness, double);
	/** Set tolerance of edge decimation, from -1 to 1, 1 means all decimations are tolerated (even bad ones),
	-1 means only really flat decimation are allowed. Optimal value is around 0.5. Greater value means mode edges will be decimated overall */
	vtkSetMacro(MeshRoughness, double);


	///
	///  Used variables and structures, support methods 
	///
protected:

  /** constructor */
	vtkProgressiveHull(){}
  /** destructor */
	~vtkProgressiveHull(){}
	
	double TargetReduction;			///<Reduction factor: 1.0 means maximal decimation
	double MeshRoughness;           ///< Tolerance of edge decimation


	int targetPoints;				///<number of desirable points in the output mesh
									///this is set in Execute from TargetReduction and the number of points in the input mesh

	///
	/// Console logging methods
	///
  static void Log(char* what)
  {
#if defined(_MSC_VER) && _MSC_VER >= 1500
		_RPT1(_CRT_WARN, "%s\n", what);
#endif
	  printf(what);
	  printf("\n");
  }
  static void Log(char* what, int parameter)
  {
#if defined(_MSC_VER) && _MSC_VER >= 1500
		_RPT2(_CRT_WARN, "%s%d\n", what, parameter);
#endif
	  printf(what);
	  printf("%d", parameter);
	  printf("\n");
  }

static void Log(char* what, double parameter)
  {
#if defined(_MSC_VER) && _MSC_VER >= 1500
		_RPT2(_CRT_WARN, "%s%f\n", what, parameter);
#endif
	  printf(what);
	  printf("%f", parameter);
	  printf("\n");
  }

	///
	/// File logging methods
	///
protected:
	/** Use this method to create the log file with input filename parameter "what" from outside of the module! */
	static void FLog(const char* what, bool endline)
	{
		try {
		ifstream t;
		ofstream out;
		t.open("Log.txt", ios::in);
		out.open("Log.txt", ios::out | ios::app);
		if (t.is_open())
		{
			t.close();
		}
		else
		{
			out << setw(40) << "File;" << setw(6) << "R;" << setw(8) << "OldPts;" << setw(6) << "Rgh;" << setw(6) << "Q;" << setw(8) << "PriTime;" << setw(8) << "DecTime;"  << setw(8) << "DecPts;" << endl;
		}
		out << setw(40) << what<<";";
		if (endline)
			out << endl;
		out.close();
		}catch(...) {
		}
	}

	static void FLog(double what, bool endline)
	{
		try {
		ofstream out;
		out.open("Log.txt", ios::out | ios::app);
		out << setw(6) << what<<";";
		if (endline)
			out << endl;
		out.close();
		}catch(...) {
		}
	}

	static void FLog(int what, bool endline)
	{
		try {
		ofstream out;
		out.open("Log.txt", ios::out | ios::app);
		out << setw(8) << what<<";";
		if (endline)
			out << endl;
		out.close();
		}catch(...) {
		}
	}
};
#endif

