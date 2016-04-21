/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpKeyczarIntegrationTest.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpBuilderDecl.h"
#include "lhpOpKeyczarIntegrationTest.h"

#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafGUI.h"

#include <iostream>
#include <fstream>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpKeyczarIntegrationTest);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpKeyczarIntegrationTest::lhpOpKeyczarIntegrationTest(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;
  
  m_PythonExe = "python.exe_UNDEFINED";
  m_PythonwExe = "pythonw.exe_UNDEFINED";  
}

//----------------------------------------------------------------------------
lhpOpKeyczarIntegrationTest::~lhpOpKeyczarIntegrationTest()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
bool lhpOpKeyczarIntegrationTest::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return true;
}
//----------------------------------------------------------------------------
mafOp* lhpOpKeyczarIntegrationTest::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpKeyczarIntegrationTest *cp = new lhpOpKeyczarIntegrationTest(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpKeyczarIntegrationTest::OpRun()   
//----------------------------------------------------------------------------
{
  // get python interpreters
  mafEvent eventGetPythonExe;
  eventGetPythonExe.SetSender(this);
  eventGetPythonExe.SetId(ID_REQUEST_PYTHON_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonExe);

  if(eventGetPythonExe.GetString())
  {
    m_PythonExe.Erase(0);
    m_PythonExe = eventGetPythonExe.GetString()->GetCStr();
    m_PythonExe.Append(" ");
  }

  mafEvent eventGetPythonwExe;
  eventGetPythonwExe.SetSender(this);
  eventGetPythonwExe.SetId(ID_REQUEST_PYTHONW_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonwExe);

  if(eventGetPythonwExe.GetString())
  {
    m_PythonwExe.Erase(0);
    m_PythonwExe = eventGetPythonwExe.GetString()->GetCStr();
    m_PythonwExe.Append(" ");
  }


  CreateGui();
  Execute();
}
//----------------------------------------------------------------------------
int lhpOpKeyczarIntegrationTest::Execute()
//----------------------------------------------------------------------------
{
  
  // This is for deploy: need to work on PYTHONPATH to solve issues with Python modules execution path...
  wxString keyCZarToolFullPath = "NONE";
  keyCZarToolFullPath = mafGetApplicationDirectory().c_str();
  keyCZarToolFullPath.Append("\\Security\\keyczar\\Keyczar-Python\\src\\keyczar");

  wxArrayString output;
  wxArrayString errors;

  wxString command2execute;

  wxString oldDir = wxGetCwd();

  mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  wxSetWorkingDirectory(keyCZarToolFullPath);

  mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  command2execute = "cmd.exe /K \"";
  command2execute.Append(m_PythonExe.GetCStr());
  command2execute.Append(" keyczart.py\"");
  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  long pid = wxExecute(command2execute, wxEXEC_SYNC);

  for (int i = 0; i < output.GetCount(); i++)
  {
    mafLogMessage(output[i].c_str());
  }
  
  if ( !command2execute )
    return MAF_ERROR;

  
  mafLogMessage(_T("Command process '%s' terminated with exit code %d."),
    command2execute.c_str(), pid);

  wxSetWorkingDirectory(oldDir);
  int result = OP_RUN_OK;
  mafEventMacro(mafEvent(this,result));
  
  return MAF_OK;
}

enum Mesh_Importer_ID
{
  ID_FIRST = MINID,
  ID_OK,
  ID_CANCEL,
};
//----------------------------------------------------------------------------
void lhpOpKeyczarIntegrationTest::CreateGui()
//----------------------------------------------------------------------------
{
  mafString wildcard = "inp files (*.inp)|*.inp|All Files (*.*)|*.*";

}
//----------------------------------------------------------------------------
void lhpOpKeyczarIntegrationTest::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case wxOK:
      {
        this->OpStop(OP_RUN_OK);
      }
      break;
      case wxCANCEL:
      {
        this->OpStop(OP_RUN_CANCEL);
      }
      break;
      default:
        mafEventMacro(*e);
      break;
    }	
  }
}
