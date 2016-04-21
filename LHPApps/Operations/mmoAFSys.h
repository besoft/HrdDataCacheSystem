/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoAFSys.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoAFSys_H__
#define __mmoAFSys_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEAFRefSys;
class mafGui;
class mafEvent;


#include <strstream>
#include <vector>
#include <map>
#include <string>

//----------------------------------------------------------------------------
// class mmoAFSys
//----------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT mmoAFSys: public mafOp
{
public:
  mmoAFSys(wxString label = "AFSys");
 ~mmoAFSys(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);
  void OpRun();
  void OpDo();
  void OpUndo();
  void CreateGui();

protected: 
  struct PredefinedScripts
  {
    PredefinedScripts(const mafString& name, const std::vector<mafString>& script, int boneID):m_Name(name),m_Script(script),m_BoneID(boneID){}
    mafString              m_Name;
    std::vector<mafString> m_Script;
    int                    m_BoneID;
  };
  bool ReadScript(const mafString& filename, std::vector<mafString>& output);
  void InitPredefined();
  void OpStop(int result);

  int                            m_Radio;
  mafVMEAFRefSys                 *m_RefSys;
  mafString                       m_ScriptFName;
  std::vector<PredefinedScripts>  m_predefinedScripts;
};
#endif
