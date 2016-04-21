/*=========================================================================
Program:   lhp
Module:    $RCSfile: lhpOpMeshGeneratorExecutable.h,v $
Language:  C++
Date:      $Date: 2010-03-30 14:55:16 $
Version:   $Revision: 1.1.2.4 $
Authors:  Daniele Giunchi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpMeshGeneratorExecutable_H__
#define __lhpOpMeshGeneratorExecutable_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGUIRollOut;
#include <vector>

//----------------------------------------------------------------------------
// lhpOpMeshGeneratorExecutable :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpMeshGeneratorExecutable: public mafOp
{
public:
  lhpOpMeshGeneratorExecutable(wxString label = "Connected Threshold");
  ~lhpOpMeshGeneratorExecutable();

  mafTypeMacro(lhpOpMeshGeneratorExecutable, mafOp);

  enum ID_TETGEN_PARAMETERS
  {
    ID_ADVANCED_GUI_ROLLOUT = MINID,
    
    ID_PLC,
    ID_REFINE,
    ID_COARSEN,
    ID_NO_BOUNDARY_SPLIT,
    ID_QUALITY,
    ID_MIN_RATIO,
    ID_VAR_VOLUME,
    ID_FIXED_VOLUME,
    ID_MAX_VOLUME,
    ID_REMOVE_SLIVER,
    ID_MAX_DIHEDRAL,
    ID_REGION_ATTRIB,
    ID_EPSILON,
    ID_NO_MERGE,
    ID_DETECT_INTER,
    ID_ORDER,
    ID_DO_CHECK,
    ID_VERBOSE,
    ID_USE_SIZING_FUNCTION,

    ID_CELL_ENTITY_IDS_ARRAY_NAME,
    ID_TETRAHEDRON_VOLUME_ARRAY_NAME,
    ID_SIZING_FUNCTION_ARRAY_NAME,

    ID_OUTPUT_SURFACE_ELEMENTS,
    ID_OUTPUT_VOLUME_ELEMENTS,
  };

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /*virtual*/ mafOp* Copy();

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /*virtual*/ void OnEvent(mafEventBase *maf_event);

  /* Handle tetgen parameters, fill the string controlling the GUI*/
  void Fill_m_TetGenCommandLineParametersString_FromIVars();

protected:

  /*virtual*/ void OpStop(int result);

  void CreateGui();
  void Execute();

private:

  int m_PLC;
  int m_Refine;
  int m_Coarsen;
  int m_NoBoundarySplit;
  int m_Quality;
  double m_MinRatio;
  int m_VarVolume;
  int m_FixedVolume;
  double m_MaxVolume;
  int m_RemoveSliver;
  double m_MaxDihedral;
  int m_RegionAttrib;
  double m_Epsilon;
  int m_NoMerge;
  int m_DetectInter;
  int m_Order;
  int m_DoCheck;
  int m_Verbose;
  int m_UseSizingFunction;

  mafString m_CellEntityIdsArrayName;
  mafString m_TetrahedronVolumeArrayName;
  mafString m_SizingFunctionArrayName;

  int m_OutputSurfaceElements;
  int m_OutputVolumeElements;

  mafGUIRollOut *m_AdvancedGUIRollOut;
  mafString m_TetGenCommandLineParametersString;
};


#endif