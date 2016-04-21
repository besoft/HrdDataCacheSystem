/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterAnsysCommon.h,v $
  Language:  C++
  Date:      $Date: 2010-11-26 16:46:11 $
  Version:   $Revision: 1.1.1.1.2.1 $
  Authors:   Gianluigi Crimi   
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpExporterAnsysCommon_H__
#define __lhpOpExporterAnsysCommon_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

#include "vtkUnstructuredGrid.h"
#include "vtkIntArray.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;
class wxBusyInfo;

 #define MAX_ELEMENT_NODES 10

//----------------------------------------------------------------------------
// lhpOpExporterAnsysInputFile :
//----------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT lhpOpExporterAnsysCommon : public mafOp
{
public:

	lhpOpExporterAnsysCommon(const wxString &label = "lhpOpExporterAnsysCommon");
	~lhpOpExporterAnsysCommon(); 
	
  /** Apply vme abs matrix to data geometry */
  void ApplyABSMatrixOn() {m_ABSMatrixFlag = 1;};
  void ApplyABSMatrixOff() {m_ABSMatrixFlag = 0;};
  void SetApplyABSMatrix(int apply_matrix) {m_ABSMatrixFlag = apply_matrix;};

  /** Set/Get output file name*/
  void SetOutputFileName(const char *outputFileName) {m_AnsysOutputFileNameFullPath = outputFileName;};
  const char *GetOutputFileName() {return m_AnsysOutputFileNameFullPath.c_str();};

  /** Export the input mesh by writing it in Ansys .inp format */
  virtual int Write() = 0;

  virtual void OnEvent(mafEventBase *maf_event);

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode *node);

  /** Builds operation's interface. */
	void OpRun();

  /** Return the "pid" of the wxExecute() ansysWriter.py process; use only for debugging
  purposes */
  long GetPid();

protected:

  virtual mafString GetWildcard() = 0;

  /** Create the dialog interface for the importer. */
  virtual void CreateGui();  
  
  void OpStop(int result);

  void OnOK();

  static int compareElem(const void *p1, const void *p2);

  void InitProgressBar(wxString label);
  void UpdateProgressBar(long progress);
  void CloseProgressBar();

  wxBusyInfo *m_BusyInfo;

  int m_IntCharSize;

  float m_TotalElements;
  long m_CurrentProgress;
  long m_OperationProgress;

  wxString m_FileDir;
  wxString m_AnsysOutputFileNameFullPath;

  int m_ImporterType;
	mafVMEMesh *m_ImportedVmeMesh;

  /** Ansys input file name */
  mafString m_AnsysInputFileName;

  int m_ABSMatrixFlag;

  long m_Pid;  

  enum ANSYS_EXPORTER_ID
  {
    ID_ABS_MATRIX_TO_STL = MINID,  
  };

  struct ExportElement
  {
    int elementID;
    int matID;
    int elementType;
    int elementReal;
    int cellID;
  };
};
#endif
