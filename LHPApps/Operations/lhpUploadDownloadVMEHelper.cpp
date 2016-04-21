/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUploadDownloadVMEHelper.cpp,v $
Language:  C++
Date:      $Date: 2012-02-01 14:11:47 $
Version:   $Revision: 1.1.2.12 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2002/2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.dataVectorIterator - www.b3c.dataVectorIterator)

MafMedical Library use license agreement

The software named MafMedical Library and any accompanying documentation, 
manuals or data (hereafter collectively "SOFTWARE") is property of the SCS s.r.l.
This is an open-source copyright as follows:
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation and/or 
other materials provided with the distribution.
* Modified source versions must be plainly marked as such, and must not be misrepresented 
as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MafMedical is partially based on OpenMAF.
=========================================================================*/

#include "mafDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpBuilderDecl.h"
#include "lhpUploadDownloadVMEHelper.h"
#include "lhpUtils.h"
#include "lhpUser.h"

#include <wx/process.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/busyinfo.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/fs_zip.h>

#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <ctime>

mafCxxTypeMacro(lhpUploadDownloadVMEHelper);

// Static variables 
mafString lhpUploadDownloadVMEHelper::m_ConfigFileName = mafString("VMEUploadDownloader_config.ini");

using namespace std;

//----------------------------------------------------------------------------
lhpUploadDownloadVMEHelper::lhpUploadDownloadVMEHelper(mafObserver *listener)
//----------------------------------------------------------------------------
{ 
  //assert(listener);
  m_Listener = listener;

  m_DebugMode = false;
    
  m_VMEUploaderDownloaderDirABSPath  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  //m_ServiceURL = "https://www.biomedtown.org/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_ServiceURL = "http://62.149.243.50:8081/town/biomed_town/LHDL/users/repository/lhprepository2";
  m_ServiceURL = "";
  m_ConnectionConfigurationFileName = "vmeUploaderConnectionConfiguration.conf" ;
 
  m_ProxyURL = "";
  m_ProxyPort = "0";

  m_GroupIdFileName = "groupId.txt";
}
//----------------------------------------------------------------------------
lhpUploadDownloadVMEHelper::~lhpUploadDownloadVMEHelper()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void lhpUploadDownloadVMEHelper::RemoveConnectionConfigurationFile()
//----------------------------------------------------------------------------
{
  wxString oldDir = wxGetCwd();
  
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  if(wxFileExists(m_ConnectionConfigurationFileName.GetCStr()))
  {
    wxRemoveFile(m_ConnectionConfigurationFileName.GetCStr());
  }

  wxSetWorkingDirectory(oldDir);

}
//----------------------------------------------------------------------------
void lhpUploadDownloadVMEHelper::SaveConnectionConfigurationFile()
//----------------------------------------------------------------------------
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  if(wxFileExists(m_ConnectionConfigurationFileName.GetCStr()))
  {
    wxRemoveFile(m_ConnectionConfigurationFileName.GetCStr());
  }

  ofstream configurationFile;
  configurationFile.open(m_ConnectionConfigurationFileName.GetCStr());

  if (!configurationFile) {
    wxString message = m_ConnectionConfigurationFileName.GetCStr();
    message.Append(" not found! Unable to write configuration connection file");
    mafLogMessage(message.c_str());
  }
  else
  {
    m_ProxyURL = GetUser()->GetProxyHost();
    m_ProxyPort = GetUser()->GetProxyPort();

    configurationFile << m_ProxyURL;   
    configurationFile << "\n";
    configurationFile << m_ProxyPort;
     
    wxString message = m_ConnectionConfigurationFileName.GetCStr();
    message.Append("Found connection configuration file: using connection parameters");
    message.Append("m_ProxyURL: ");
    message.Append(m_ProxyURL.GetCStr());
    message.Append("m_ProxyPort: ");
    message.Append(m_ProxyPort.GetCStr());

    if (m_DebugMode)
      mafLogMessage(message.c_str());

    configurationFile.close();
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

}
//----------------------------------------------------------------------------
bool lhpUploadDownloadVMEHelper::ExistsThreadedUploaderDownloader()
//----------------------------------------------------------------------------
{
  long pid = GetThreadedUploaderDownloaderProcessID();

  if (pid == -1)
  {
    return false;
  } 
  else
  {
    return true;
  }
}
//----------------------------------------------------------------------------
bool lhpUploadDownloadVMEHelper::IsClientSoftwareVersionUpToDate()
//----------------------------------------------------------------------------
{
  wxBusyInfo("Checking if  your software is up-to-date in order to upload, please wait...");
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );
  
  // get application linkResourceName
  mafEvent eventGetApplicationName(this,ID_REQUEST_APPLICATION_NAME);
//   eventGetApplicationName.SetSender(this);
//   eventGetApplicationName.SetId(ID_REQUEST_APPLICATION_NAME);
  mafEventMacro(eventGetApplicationName);
  mafString appName = eventGetApplicationName.GetString()->GetCStr();  
  
  m_ProxyURL = GetUser()->GetProxyHost();
  m_ProxyPort = GetUser()->GetProxyPort();

  // get manual tags
  wxString command2execute;
  command2execute.Clear();
  command2execute = GetPythonwInterpreterFullPath().GetCStr();
  command2execute.Append(" lhpDictionaryVersionChecker.py ");
  command2execute.Append(appName.GetCStr());
  
  if( GetUser()->GetProxyFlag() )
  {
    command2execute.Append(" ");
    command2execute.Append(m_ProxyURL.GetCStr());
    command2execute.Append(" ");
    command2execute.Append(m_ProxyPort.GetCStr());
  }

  if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  wxArrayString output;
  wxArrayString errors;
  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in lhpDictionaryVersionChecker.py. Look at the log area for more infos.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  if (m_DebugMode)
  {
    mafLogMessage("Command Output Messages:");
    for (int i = 0; i < output.size(); i++)
    {
      mafLogMessage(output[i]);
    }

    mafLogMessage("Command Errors Messages:");
    for (int i = 0; i < errors.size(); i++)
    {
      mafLogMessage(errors[i]);
    }
  }
  
  wxString result = output[output.size() - 1];

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  if (result == "UpToDate")
  {
    return true;
  } 
  else
  {
    return false;
  }  
}
//----------------------------------------------------------------------------
bool lhpUploadDownloadVMEHelper::CreateThreadedUploaderDownloader()
//----------------------------------------------------------------------------
{
  bool existAlready = false;
  existAlready = ExistsThreadedUploaderDownloader();
  
  if (existAlready == true)
  {
    mafLogMessage("Threaded Uploader Downloader process found, skipping creation...");
    return false;
  }

  wxString command2execute;
  if (m_DebugMode)
    command2execute = GetPythonInterpreterFullPath().GetCStr();
  else
    command2execute = GetPythonwInterpreterFullPath().GetCStr();

  mafString pythonScriptName = "ThreadedUploaderDownloader.py ";
  command2execute.Append(pythonScriptName.GetCStr());
  command2execute.Append("50000");
  if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  long processID = wxExecute(command2execute, wxEXEC_ASYNC);

  mafSleep(10000);

  if (m_DebugMode)
    mafLogMessage(_T("ASYNC Command process '%s' terminated with exit code %d."),
    command2execute.c_str(), processID);
  
  int startTime = time(NULL);
  int timeOut = 100;

  while (ExistsThreadedUploaderDownloader() == false)
  {
    mafSleep(1);
    
    int elapsedTime = time(NULL) - startTime;
    
    if (elapsedTime > timeOut)
    {
      // DEBUG
      std::ostringstream stringStream;
      stringStream << "Problems creating Upload/Download server. Exiting after: " << timeOut << std::endl;
      mafLogMessage(stringStream.str().c_str());
            
      break;
    }
  }

  return true;
}
//----------------------------------------------------------------------------
int lhpUploadDownloadVMEHelper::KillThreadedUploaderDownloaderProcess()
//----------------------------------------------------------------------------
{
  int pid = GetThreadedUploaderDownloaderProcessID();

  if (pid == -1)
  {
    std::ostringstream stringStream;    
    stringStream << "process with ID: " << pid  << "does not exists" << std::endl;
    
    mafLogMessage(stringStream.str().c_str());
          
    return 0;
  } 
  else
  {
   
    int returnValue = wxKill(pid, wxSIGKILL);
    
    std::ostringstream stringStream;
    stringStream << "process with ID: " << pid  << "terminated with exit code"  << returnValue \
    << std::endl;
    
    mafLogMessage(stringStream.str().c_str());
    
    wxFile lockFile;
    wxString lockpath = m_VMEUploaderDownloaderDirABSPath;
    lockpath += "activeLock.lhp";

    if (wxFileExists(lockpath))
    {
      wxRemoveFile(lockpath);
    }
  
    return returnValue;
  }
}
//----------------------------------------------------------------------------
long lhpUploadDownloadVMEHelper::GetThreadedUploaderDownloaderProcessID()
//----------------------------------------------------------------------------
{
  long processID = -1;
  bool threadedUploaderDownloaderLockFileExists = false; 

  wxFile lockFile;
  wxString lockpath = m_VMEUploaderDownloaderDirABSPath;
  lockpath += "activeLock.lhp";
  if (wxFileExists(lockpath))
  {
    threadedUploaderDownloaderLockFileExists = lockFile.Open(lockpath);
  }

  if(threadedUploaderDownloaderLockFileExists)
  {
    char processIDCharArray[10];
    char *pointer;
    pointer = &processIDCharArray[0];

    lockFile.Read(pointer, 10);
    processID = atoi(processIDCharArray);

    lockFile.Close();

    bool threadedUploaderDownloaderProcessExists = false;
    threadedUploaderDownloaderProcessExists = wxProcess::Exists(processID);

    if(!threadedUploaderDownloaderProcessExists)
    {
      wxRemoveFile(lockpath);
    }
  }

  return processID;
}
//----------------------------------------------------------------------------
mafString lhpUploadDownloadVMEHelper::GetPythonInterpreterFullPath()
//----------------------------------------------------------------------------
{
  mafString defaultPythonExeABSPath = "C:\\Python25\\python.exe ";
  mafString pythonExe = defaultPythonExeABSPath;

  mafEvent eventGetPythonExe;
  eventGetPythonExe.SetSender(this);
  eventGetPythonExe.SetId(ID_REQUEST_PYTHON_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonExe);

  if(eventGetPythonExe.GetString())
  {
    pythonExe.Erase(0);
    pythonExe = eventGetPythonExe.GetString()->GetCStr();
    pythonExe.Append(" ");
  }
  else
  {
    //use defaultPythonExeABSPath
  }

  return pythonExe;
}
//----------------------------------------------------------------------------
mafString lhpUploadDownloadVMEHelper::GetPythonwInterpreterFullPath()
//----------------------------------------------------------------------------
{
  mafString defaultPythonwExeABSPath = "C:\\Python25\\pythonw.exe ";
  mafString pythonwExe = defaultPythonwExeABSPath;  

  mafEvent eventGetPythonwExe;
  eventGetPythonwExe.SetSender(this);
  eventGetPythonwExe.SetId(ID_REQUEST_PYTHONW_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonwExe);

  if(eventGetPythonwExe.GetString())
  {
    pythonwExe.Erase(0);
    pythonwExe = eventGetPythonwExe.GetString()->GetCStr();
    pythonwExe.Append(" ");
  }
  else
  {
    //use defaultPythonwExeABSPath
  }

  return pythonwExe;
}
//----------------------------------------------------------------------------
lhpUser *lhpUploadDownloadVMEHelper::GetUser()
//----------------------------------------------------------------------------
{
  lhpUser *user = NULL;

  mafEvent event(this,ID_REQUEST_USER);
  //event.SetSender(this);
  //event.SetId(ID_REQUEST_USER);
  mafEventMacro(event);
  if(event.GetMafObject() != NULL) //if proxy string contains something != ""
  {
    user = (lhpUser*)event.GetMafObject();
  }

  return user;
}
//----------------------------------------------------------------------------
mafString lhpUploadDownloadVMEHelper::GetServiceURL(bool groupChoice)
//----------------------------------------------------------------------------
{
  assert(m_VMEUploaderDownloaderDirABSPath);
  assert(m_ConfigFileName);
  mafString serviceURL("");

  mafString absConfigFilePath = m_VMEUploaderDownloaderDirABSPath + "//" + m_ConfigFileName;

  std::ifstream configFile;
  configFile.open(absConfigFilePath.GetCStr());

  while(!configFile.eof() && configFile.is_open())
  {
    std::string line;
    std::string lineRaw;
    std::getline(configFile,lineRaw);

    // remove spaces from the line
    for(int i=0; i < lineRaw.length(); i++)
    {
      if(lineRaw[i] != ' ')
      {
        line.push_back(lineRaw[i]);
      }
    }
        
    // remove the portion of string after the first '#' (comments)
    int commentPlace;
    commentPlace = line.find("#");
    if(commentPlace != -1)
    {
      line.erase(commentPlace, line.length() - commentPlace);
    }

    // Get the service url
    int servicePlace;
    servicePlace = line.find("ServiceURL=");
    if(servicePlace != -1) // the last win
    {
      line = line.substr(strlen("ServiceURL=") + servicePlace, line.length());
      
      // Added support for the new physiomespace instance that have multiple repositories:
      // the new service URL is the type "http://62.149.243.50:8091/Plone/Members/username"

      std::string usernameKeyWord = "{username}";
      int usernamePlace = line.find(usernameKeyWord);
      if(usernamePlace != -1)
      {
        mafString username = mafString("");
        if(groupChoice)
        {
          // gui for selecting group
          wxString command2execute;
          command2execute.Clear();

          wxString oldDir = wxGetCwd();
          if (m_DebugMode)
            mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
          wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
          if (m_DebugMode)
            mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

          if (m_DebugMode)
            command2execute = GetPythonInterpreterFullPath().GetCStr();
          else
            command2execute = GetPythonwInterpreterFullPath().GetCStr();

          command2execute.Append(" groupsSelectorApp.py ");
          lhpUser *user = this->GetUser();
          if(user)
          {
            command2execute.Append(user->GetName());
            command2execute.Append(" ");
            command2execute.Append(user->GetPwd());
          }

          wxString command2executeWithoutPassword = command2execute;
          command2executeWithoutPassword.Replace(user->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

          mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );

          long pid = wxExecute(command2execute, wxEXEC_SYNC);
          if(pid)
          {
            mafLogMessage("Error in groupsSelectorApp.");
            return MAF_ERROR;
          }

          mafLogMessage("done");

          // 
          ifstream groupIdFile;
          groupIdFile.open(m_GroupIdFileName.GetCStr());
          if (!groupIdFile) {
            mafLogMessage("Unable to open file");
            return MAF_ERROR; // terminate with error
          }

          char buffer[200];
          
          groupIdFile.getline(buffer, 200);
          char *pch;
          wxArrayString resourceInfoList;
          pch = strtok(buffer, " ");
          username = mafString(pch);

          groupIdFile.close();

          wxSetWorkingDirectory(oldDir);
          if (m_DebugMode)
            mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
        }
        else
        {
          lhpUser *user = this->GetUser();
          if(user)
          {
            username = user->GetName();
          }
        }
        
        if(username.Compare("") != 0)
        {
          line.replace(usernamePlace,usernameKeyWord.length(),username.GetCStr());
        }
      }

      serviceURL = mafString(line.c_str());
    }
  }
  if(serviceURL.Compare("") == 0) // if empty put the standard one
  {
    serviceURL = "https://www.biomedtown.org/biomed_town/LHDL/users/repository/lhprepository2";
  }
  mafLogMessage(serviceURL);
  m_ServiceURL = serviceURL;
  return serviceURL;
}