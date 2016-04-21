/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpOpComputeHausdorffDistance.h,v $
Language:  C++
Date:      $Date: 2011-08-05 09:11:04 $
Version:   $Revision: 1.1.2.3 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpComputeHausdorffDistance_H__
#define __lhpOpComputeHausdorffDistance_H__

//----------------------------------------------------------------------------
// Includes :
//----------------------------------------------------------------------------
#include "mafNode.h"
#include "mafOp.h"
#include "mafVMESurface.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

class mafGUIDialog;
class mafOpImporterSTL;
class mafVMEMesh;
class mafVMEVolumeGray;

class vtkImageData;
class vtkMAFContourVolumeMapper;
class vtkPolyData;


class LHP_OPERATIONS_EXPORT lhpOpComputeHausdorffDistance: public mafOp
{
public:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  enum GUI_IDS
  {
    ID_OK = MINID, 
    ID_CANCEL,
  };

  /** constructor. */
  lhpOpComputeHausdorffDistance(const wxString &label = "Compute Hausdorff Distance");
  /** destructor. */
  ~lhpOpComputeHausdorffDistance(); 

  /** Precess events coming from other objects */
  /*virtual*/ void OnEvent(mafEventBase *maf_event);

  /** RTTI macro */
  mafTypeMacro(lhpOpComputeHausdorffDistance, mafOp);

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

  /** Makes the undo for the operation. */
  /*virtual*/ void OpUndo();

  static bool SurfaceAccept(mafNode* node) {return(node != NULL && node->IsMAFType(mafVMESurface));};

protected:

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  /*virtual*/ void OpStop(int result);

  /** Compute H. Distance. */
  int ComputeDistance();

  /** Create the operation GUI. */
  void CreateGui();

  mafString *m_FilenameSTL1, *m_FilenameSTL2;
  mafString *m_VMEName1, *m_VMEName2;
  mafString *m_OutputDir;

  int m_VmeOrSTL1, m_VmeOrSTL2;

  mafVMESurface *m_SurfaceInput1;
  mafVMESurface *m_SurfaceInput2;
  mafVMESurface *m_SurfaceOutput;

  mafOpImporterSTL *m_STLImporter;
  std::vector<mafVMESurface*> m_ImportedSurfaces;

};
#endif
