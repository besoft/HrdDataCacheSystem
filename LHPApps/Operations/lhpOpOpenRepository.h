/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpOpenRepository.h,v $
Language:  C++
Date:      $Date: 2009-11-03 13:00:35 $
Version:   $Revision: 1.1.2.1 $
Authors:   Alberto Losi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpOpenRepository_H__
#define __lhpOpOpenRepository_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpUploadDownloadVMEHelper.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpUser;

//----------------------------------------------------------------------------
// lhpOpDownloadVMERefactor :
//----------------------------------------------------------------------------
/** Open BiomedTown remote repository in the default browser
*/
class LHP_OPERATIONS_EXPORT lhpOpOpenRepository: public mafOp
{
public:

  lhpOpOpenRepository(wxString label = "Open repository");

  ~lhpOpOpenRepository(); 

  mafTypeMacro(lhpOpOpenRepository, mafOp);

  mafOp* Copy();

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode* vme);

  /** Builds operation's interface by calling CreateOpDialog() method. */
  void OpRun();

  //void SetDebugMode(bool debugMode);

  virtual void OpDo();
  void OnEvent(mafEventBase *maf_event);

protected:
  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  virtual void OpStop(int result);

  /** Execute the operation. */
  int ExecuteScript();

  lhpUser  *m_User;
  lhpUploadDownloadVMEHelper *m_UploadDownloadVMEHelper;
  mafString m_PythonInterpreterFullPath; 
  mafString m_PythonwInterpreterFullPath;
  mafString m_ScriptPath;
  bool m_DebugMode;
};
#endif