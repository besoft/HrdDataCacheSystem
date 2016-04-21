/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUser.cpp,v $
Language:  C++
Date:      $Date: 2009-09-09 15:01:46 $
Version:   $Revision: 1.1.1.1.2.3 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpUtils.h"

#include "lhpBuilderDecl.h"
#include "lhpUser.h"
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#include "mafDecl.h"
#include "mafGUIDialogLogin.h"
#include "mafCrypt.h"

#include <fstream>
#include <wx/busyinfo.h>

//----------------------------------------------------------------------------
lhpUser::lhpUser(mafObserver *listener)
//----------------------------------------------------------------------------
{
  m_Listener = listener;
  m_PythonExe = "python.exe_UNDEFINED";
  m_PythonwExe = "pythonw.exe_UNDEFINED";
  m_VMEUploaderDownloaderDir  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  
  m_IsAuthenticated = false;
 
}
//----------------------------------------------------------------------------
lhpUser::~lhpUser()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
bool lhpUser::CheckUserCredentials()
//----------------------------------------------------------------------------
{
  if (!m_Initialized)
  {
    InitializeUserInformations();
  }
 
  int res = false;
  if(ShowLoginDialog() != wxID_CANCEL)
    res = !m_Username.IsEmpty() && !m_Password.IsEmpty() && ExecuteAuthenticationScript();

  if(!res)
  {
    int result = wxMessageBox("\
No server side authentication!\n\
Do you want to Retry?\n\
Pressing Cancel information about\n\
the user will not be stored on MSF and network enabled\n\
facilities will be disabled",wxMessageBoxCaptionStr, wxOK | wxCANCEL);
    //returns 4 for OK, 16 for CANCEL
    if (result == 16)
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  m_IsAuthenticated = true;

  return false;
}

//----------------------------------------------------------------------------
bool lhpUser::IsAuthenticated()
//----------------------------------------------------------------------------
{
  return m_IsAuthenticated;
}

//----------------------------------------------------------------------------
bool lhpUser::ExecuteAuthenticationScript()
//----------------------------------------------------------------------------
{
  mafEvent event;
  event.SetSender(this);
  event.SetId(ID_REQUEST_PYTHONW_EXE_INTERPRETER);
  mafEventMacro(event);

  if(event.GetString())
  {
    m_PythonwExe.Erase(0);
    m_PythonwExe = event.GetString()->GetCStr();
  }
  
  bool pythonwInterpreterExists = wxFileExists(m_PythonwExe.GetCStr());
  if (!pythonwInterpreterExists)
  {
    std::ostringstream stringStream;
    stringStream  << "Pythonw Settings " << m_PythonwExe.GetCStr() << " interpreter not found. Cannot authenticate! Please check your Tools -> Options -> Python Settings " << std::endl;
    mafLogMessage(stringStream.str().c_str());
    return false;
  }

  wxString oldDir = wxGetCwd();
  mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());
  mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  
  command2execute = m_PythonwExe.GetCStr();
  command2execute.Append(" lhpAuthenticationControl.py ");
  command2execute.Append(m_Username.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(m_Password.GetCStr());
  
  if(m_ProxyFlag != 0 && !m_ProxyHost.Equals(""))
  {
    command2execute.Append(" ");
    command2execute.Append(m_ProxyHost.GetCStr());
    command2execute.Append(" ");
    command2execute.Append(wxString::Format("%d ",m_ProxyPort));
  }
  
  mafLogMessage("Authenticating user. Please wait...");
  
  wxBusyInfo *wait = new wxBusyInfo("Authenticating user. Please wait...");
  
  wxArrayString output;
  wxArrayString errors;

  long pid = wxExecute(command2execute, output, errors);

  if (output.size() == 0)
  {
    return false;
  }


  mafLogMessage("Command Output Messages:");
  for (int i = 0; i < output.size(); i++)
  {
    mafLogMessage(output[i]);
  }

  mafLogMessage("Command Errors Messages (if any...):");
  for (int i = 0; i < errors.size(); i++)
  {
    mafLogMessage(errors[i]);
  }

  wxString result = output[output.size() - 1];

  wxSetWorkingDirectory(oldDir);
  mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  if (result == "Authenticated")
  {
    cppDEL(wait);
    return true;
  } 
  else 
  {
    cppDEL(wait);
    int result = wxMessageBox("\
Problems during server side authentication!\n\
Please check the Log Area for more details on the error \n"\
,wxMessageBoxCaptionStr, wxOK);
    return false;
  }  
}



