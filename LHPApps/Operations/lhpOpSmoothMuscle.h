/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpSmoothMuscle.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 12:57:24 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/



#ifndef __lhpOpSmoothMuscle_H__
#define __lhpOpSmoothMuscle_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"
#if defined(VPHOP_WP10)

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class vtkPolyData;

//----------------------------------------------------------------------------
// lhpOpSmoothMuscle :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpSmoothMuscle: public mafOp
{
protected:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  enum GUI_IDS
  {
    ID_POLYMENDER_SMFACTOR = MINID,    

    ID_OK,
    
    LAST_ID,
  };

public:
  lhpOpSmoothMuscle(const wxString &label = "Smooth Muscle");
  ~lhpOpSmoothMuscle(); 

  mafTypeMacro(lhpOpSmoothMuscle, mafOp);	

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

    /** Execute the operation. */
  /*virtual*/ void OpUndo();

#if 0
	/** Sets the number of Taubin smoothing steps.
	Zero value disables smoothing */
	inline void SetTaubinSmoothSteps(int steps) {
		m_TaubinSmoothSteps = steps;
	}

	/** Gets the number of Taubin smoothing steps.
	Zero value means disabled smoothing */
	inline int GetTaubinSmoothSteps() {
		return m_TaubinSmoothSteps;
	}

	/** Sets, if the Taubin smoothing should preserve the shape of input surface. */
	inline void SetTaubinSmoothPreserveShape(bool preserve) {
		m_TaubinSmoothPreserveShape = preserve;
	}

	/** Gets, if the Taubin smoothing should preserve the shape of input surface. */
	inline int GetTaubinSmoothPreserveShape() {
		return m_TaubinSmoothPreserveShape;
	}

	/** Sets the number of steps to remove spikes.
	Zero value disables removal */
	inline void SetSpikesRemoveSteps(int steps) {
		m_RemoveMaxSteps = steps;
	}

	/** Gets the number of steps to remove spikes.
	Zero value means disabled removal */
	inline int GetSpikesRemoveSteps() {
		return m_RemoveMaxSteps;
	}

	/** Sets the size of ring of vertices around a spike to be removed.
	Zero means that just one vertex (at the spike) is removed, one that
	one-ring is removed, etc. */
	inline void SetSpikesRemoveSize(int size) {
		m_RemoveRingSize = size;
	}
	
	/** Gets the size of ring of vertices around a spike to be removed.
	Zero means that just one vertex (at the spike) is removed, one that
	one-ring is removed, etc. */
	inline int GetSpikesRemoveSize() {
		return m_RemoveRingSize;
	}

	/** Sets the quality angle for spike detection. 
	A pair of triangles with dihedral angle larger than QualityDihedralAngle
	is considered to participate in spike. */
	inline void SetQualityDihedralAngle(double angle) {
		m_QualityDihedralAngle = angle;
	}

	/** Gets the quality angle for spike detection. */
	inline int GetQualityDihedralAngle() {
		return m_QualityDihedralAngle;
	}
#endif
	/** Runs the transformation - the core
	Creates m_OriginalPolydata and m_OriginalPolydata.
	OpDo / OpUndo sets m_Input, called from OpRun */
	virtual void Execute();

#if 0
protected:
	/** Removes spikes from m_ResultPolydata.
	N.B. called from Execute */
	virtual void RemoveSpikes();
#endif
protected:	
	int m_PolyMenderSmoothFactor;					///<smooth factor for PolyMender
#if 0
	int m_TaubinSmoothSteps;						///<number of Taubin smooth steps (500 by default)
	bool m_TaubinSmoothPreserveShape;		///<true, if Taubin should try to preserve shape of input surface	
	int m_RemoveMaxSteps;								///>maximum number of removal steps
	int m_RemoveRingSize;								///>size (in ring of vertices) to be removed and filled
	double m_QualityDihedralAngle;			///<angle, that is to filter out spikes	
#endif

	vtkPolyData*	m_OriginalPolydata;			///<the stored original input (when Execute was called)
  vtkPolyData*	m_ResultPolydata;				///<the output
};
#endif
#endif //__lhpOpSmoothMuscle_H__