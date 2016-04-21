/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoINPExporter.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoINPExporter_H__
#define __mmoINPExporter_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafEvent.h"
#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include <iostream>


//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafNode;
class mafVMEOutputSurface;

//----------------------------------------------------------------------------
// mmoINPExporter :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mmoINPExporter: public mafOp
{
public:
  mmoINPExporter(const wxString &label = "INPExporter");
  ~mmoINPExporter(); 

  mafTypeMacro(mmoINPExporter, mafOp);

  mafOp* Copy();
  void OnEvent(mafEventBase *maf_event);

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode *node);

  /** Builds operation's interface. */
  void OpRun();

  /** Set the filename for the .stl to export */
  void SetFileName(const char *file_name) {m_File = file_name;};

  void ApplyABSMatrixOn() {m_ABSMatrixFlag = 1;};
  void ApplyABSMatrixOff() {m_ABSMatrixFlag = 0;};
  void SetApplyABSMatrix(int apply_matrix) {m_ABSMatrixFlag = apply_matrix;};

  void ExportAsBynaryOn() {m_Binary = 1;};
  void ExportAsBynaryOff() {m_Binary = 0;};
  void SetExportAsBynary(int binary_file) {m_Binary = binary_file;};

  /** Export the surface. */
  void ExportSurface();

protected:
  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  //void OpStop(int result);
  void ExportingTraverse(const char *dirName, mafNode* node);
  void ExportOneSurface(const char *filename, mafVMEOutputSurface* surf);

  mafString  m_File;
  mafString  m_FileDir;
  int        m_Binary;
  int        m_ABSMatrixFlag;
};
#endif
