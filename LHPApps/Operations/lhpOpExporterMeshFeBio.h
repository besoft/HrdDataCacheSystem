/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpOpExporterMeshFeBio.h,v $
Language:  C++
Date:      $Date: 2010-12-15 15:23:16 $
Version:   $Revision: 1.1.2.1 $
Authors:   Eleonora Mambrini   
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpExporterMeshFeBio_H__
#define __lhpOpExporterMeshFeBio_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;

class lhpMeshFeBioWriter;

//----------------------------------------------------------------------------
// lhpOpExporterMeshFeBio :
//----------------------------------------------------------------------------
/** Write a mafVMEMesh in FeBio .feb format. */
class LHP_OPERATIONS_EXPORT lhpOpExporterMeshFeBio : public mafOp
{
public:
  /** Class constructor */
  lhpOpExporterMeshFeBio(const wxString &label = "lhpOpExporterMeshFeBio");
  /** Class destructor */
  ~lhpOpExporterMeshFeBio(); 

  mafTypeMacro(lhpOpExporterMeshFeBio, mafOp);

  /** Set/Get output file name*/
  void SetOutputFileName(const char *outputFileName) {m_FebBioOutputFileNameFullPath = outputFileName;};
  const char *GetOutputFileName() {return m_FebBioOutputFileNameFullPath.c_str();};

  /** Export the input mesh by writing it in FeBio format */
  int Write();

  /** Builds operation's interface. */
  void OpRun();

  virtual void OnEvent(mafEventBase *maf_event);

  /** Copy operation*/
  mafOp* Copy();

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode *node);

protected:

  /** Create the dialog interface for the importer. */
  virtual void CreateGui();  

  void OpStop(int result);

  void OnOK();

  wxString m_FileDir;
  wxString m_FebBioOutputFileNameFullPath;

  mafVMEMesh *m_ImportedVmeMesh;

  mafString m_CacheDir;

  lhpMeshFeBioWriter *m_Writer;

};
#endif
