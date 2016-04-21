/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpOpenRepository.cpp,v $
Language:  C++
Date:      $Date: 2011-04-12 15:02:24 $
Version:   $Revision: 1.1.2.2 $
Authors:   Alberto Losi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#include "lhpDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpBuilderDecl.h"
#include "lhpUser.h"

#include "lhpUtils.h"
#include <wx/process.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/busyinfo.h>

#include "lhpOpOpenRepository.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpOpenRepository);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpOpenRepository::lhpOpOpenRepository(wxString label) :
mafOp(label)
  //----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;
  m_DebugMode = true;

  m_User = NULL;
  m_PythonInterpreterFullPath = "python.exe_UNDEFINED";
  m_PythonwInterpreterFullPath = "pythonw.exe_UNDEFINED";  
  m_ScriptPath  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  m_UploadDownloadVMEHelper = new lhpUploadDownloadVMEHelper(this);
}

//----------------------------------------------------------------------------
lhpOpOpenRepository::~lhpOpOpenRepository()
//----------------------------------------------------------------------------
{
  cppDEL(m_UploadDownloadVMEHelper);
}
//----------------------------------------------------------------------------
mafOp* lhpOpOpenRepository::Copy()
  //----------------------------------------------------------------------------
{
  /** return a copy of itself, needs to put it into the undo stack */
  return new lhpOpOpenRepository(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpOpenRepository::Accept(mafNode* vme)
  //----------------------------------------------------------------------------
{
  lhpUser *user = NULL;
  //Get User values
  mafEvent event;
  event.SetSender(this);
  event.SetId(ID_REQUEST_USER);
  mafEventMacro(event);
  if(event.GetMafObject() != NULL) //if proxy string contains something != ""
  {
    user = (lhpUser*)event.GetMafObject();
  }
  return (user != NULL && user->IsAuthenticated() && vme != NULL);
}
//----------------------------------------------------------------------------
void lhpOpOpenRepository::OpRun()
 //----------------------------------------------------------------------------
{
  int result = OP_RUN_CANCEL;
  m_User = m_UploadDownloadVMEHelper->GetUser();
  m_PythonInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath();
  m_PythonwInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath();
  result = ExecuteScript();
  //mafEventMacro(mafEvent(this,result));
  OpStop(result);
  return;
}
//----------------------------------------------------------------------------
int lhpOpOpenRepository::ExecuteScript()   
//----------------------------------------------------------------------------
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_ScriptPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;

  command2execute = m_PythonwInterpreterFullPath.GetCStr();
  command2execute.Append(" openBrowser.py ");
  command2execute.Append(m_User->GetName());
  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
  long pid = wxExecute(command2execute, wxEXEC_SYNC);

//   Future implementation with Auto-login script:
//   command2execute = m_PythonwInterpreterFullPath.GetCStr();
//   command2execute.Append(" safeSaveSettings.py ");
//   long pid = wxExecute(command2execute, wxEXEC_SYNC);
// 
//   command2execute = m_PythonwInterpreterFullPath.GetCStr();
//   command2execute.Append(" autoLogin.py ");
//   command2execute.Append(m_User->GetName());
//   command2execute.Append(" ");
//   command2execute.Append(m_User->GetPwd());
//   wxString command2executeWithoutPassword = command2execute;
//   command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");
//   mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
//   pid = wxExecute(command2execute, wxEXEC_SYNC);
// 
//   command2execute = m_PythonwInterpreterFullPath.GetCStr();
//   command2execute.Append(" safeRestoreSettings.py ");
//   pid = wxExecute(command2execute, wxEXEC_SYNC);

  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  mafLogMessage("done");
  return OP_RUN_OK;
}
//----------------------------------------------------------------------------
// void lhpOpOpenRepository::SetDebugMode(bool debugMode)
//----------------------------------------------------------------------------
// {
//   m_DebugMode = debugMode;
// }
//----------------------------------------------------------------------------
void lhpOpOpenRepository::OpStop(int result)
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void lhpOpOpenRepository::OpDo()
//----------------------------------------------------------------------------
{
   //ExecuteScript();
}
//----------------------------------------------------------------------------
void lhpOpOpenRepository::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId()) 
    {
    default:
      mafEventMacro(*e);
    }
  }
}