/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkProgressiveHullCUDA.h,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:26 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Tomas Janak
  Notes:
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef __vtkProgressiveHullCUDA_h
#define __vtkProgressiveHullCUDA_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkProgressiveHull.h"
class VTK_vtkLHP_EXPORT vtkProgressiveHullCUDA : public vtkProgressiveHull
{
public:

	//retrieve instance of the class
  static vtkProgressiveHullCUDA* New();

	/** Get maximal tolerated valence of vertex */
	vtkGetMacro(MaxValence, int);

	/** Set maximal tolerated valence of vertex 
	If an edge collapse would cause appearance of vertex with larger valence than MaxValence,
	this edge will not be collapsed. This parameter influences the mesh quality. */
	vtkSetMacro(MaxValence, int);

protected:
	// constructor
	vtkProgressiveHullCUDA();          
	// destructor
	~vtkProgressiveHullCUDA(){
	}

	
	int MaxValence;	//max allowed valence of vertex

	
  /** Filter execution */
  /*virtual*/ void Execute();
};
#endif

