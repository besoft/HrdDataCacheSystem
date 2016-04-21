/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpUploadMultiVMERefactor.cpp,v $
Language:  C++
Date:      $Date: 2011-10-26 12:19:17 $
Version:   $Revision: 1.1.1.1.2.42 $
Authors:   Roberto Mucci , Stefano Perticoni , Matteo Giacomoni
==========================================================================
Copyright (c) 2002/2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)

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
#include "lhpUtils.h"

#include <wx/process.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/busyinfo.h>
#include <wx/html/htmlwin.h>

#include "lhpOpUploadMultiVMERefactor.h"
#include "lhpOpUploadVMERefactor.h"

#include "mafVMERoot.h"
#include "mafVMELandmarkCloud.h"
#include "mafGUI.h"

#include "lhpUser.h"

#include "mafNode.h"
#include "mafStorageElement.h"
#include "mafNodeIterator.h"
#include "mafTagArray.h"

#include "lhpFactoryTagHandler.h"
#include "lhpGUISettingsOpUploadMultiVMERefactor.h"
#include "vtkPolyData.h"

#include "mafNodeIterator.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"

#include "mafEventIO.h"
#include "mafStorage.h"
#include "mafDataVector.h"

#include <string>
#include <istream>
#include <ostream>
#include <fstream>

#include "lhpUploadDownloadVMEHelper.h"
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpUploadMultiVMERefactor);
//----------------------------------------------------------------------------
//static variables
mafString lhpOpUploadMultiVMERefactor::m_CacheSubdir = "0";

enum lhpOpUploadMultiVME_ID
{
  ID_SUBDICTIONARY = MINID, 
};

//----------------------------------------------------------------------------
lhpOpUploadMultiVMERefactor::lhpOpUploadMultiVMERefactor(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType  = OPTYPE_OP;
	m_Canundo = false;
  m_WithChild = false;
  m_DebugMode = false;
  m_AlreadyUploadedXMLURIVector.clear();
  m_AlreadyUploadedVMEVector.clear();
  m_EmptyNodeVector.clear();
  m_FileCreatedVector.clear();
  m_DerivedVMEsIdVector.clear();
  m_VMEsToUploadIdsVector.clear();
  m_OpUploadVME = NULL;
  m_PassPhrase = "";

  m_UploadDownloadVMEHelper = new lhpUploadDownloadVMEHelper(this);
  m_ServiceURL = "";
  //m_ServiceURL = "http://devel.fec.cineca.it:12680/town/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_ServiceURL = "https://www.biomedtown.org/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_ServiceURL = "http://62.149.243.50:8081/town/biomed_town/LHDL/users/repository/lhprepository2";

  m_PythonExe = "python.exe_UNDEFINED";

  m_VMEUploaderDownloaderABSFolder  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();

  m_MasterXMLDictionaryFileName = "UNDEFINED";
  m_SubXMLDictionaryFilePrefix = "UNDEFINED" ;
  m_SubXMLDictionaryFileName = "UNDEFINED";
  m_AssembledXMLDictionaryFileName = "assembledXMLDictionary.xml";
  m_SubDictionaryBuildingCommand = "UNDEFINED";

  m_SubdictionaryId = 0; // NO_SUBDICTIONARY; 

  SetListener(this);

  m_ExecutedLXMLChecker = 0;

  m_GroupsChoice = true;
  m_Repository = "";

  m_DontStopAfterOpRun = false;
}
//----------------------------------------------------------------------------
lhpOpUploadMultiVMERefactor::~lhpOpUploadMultiVMERefactor()
//----------------------------------------------------------------------------
{
  m_AlreadyUploadedXMLURIVector.clear();
  m_AlreadyUploadedVMEVector.clear();
  m_EmptyNodeVector.clear();
  m_FileCreatedVector.clear();
  m_DerivedVMEsIdVector.clear();

  cppDEL(m_UploadDownloadVMEHelper);
}

//----------------------------------------------------------------------------
bool lhpOpUploadMultiVMERefactor::Accept(mafNode* vme)
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
   
  if (user != NULL && user->IsAuthenticated() && vme != NULL) // If user is authenticated Python and wxPython are installed
  {
    // It doesen't work from ior:
    // Run a Python script that determine if lxml library is installed (only once per execution)
    if(m_ExecutedLXMLChecker == 1)
    {
      return true;
    }
//     if(m_ExecutedLXMLChecker == -1)
//     {
//       return false;
//     }

    wxString oldDir = wxGetCwd();
    wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());

    mafLogMessage( _T("Searching lxmlCheckerForAccept in '%s'"), m_VMEUploaderDownloaderABSFolder.GetCStr()); // Debug stuff
//     wxString command2execute;
//     command2execute.Clear();
//     command2execute = "pythonw.exe";
//     command2execute.Append(" lxmlCheckerForAccept.py ");

//    long pid = -1;
//     mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
//     if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
//     {
//       wxSetWorkingDirectory(oldDir);
//       m_ExecutedLXMLChecker = -1;
//         return false;
//     }
    wxSetWorkingDirectory(oldDir);
    m_ExecutedLXMLChecker = 1;
    return true;
  }
  return false;
}
//----------------------------------------------------------------------------
mafOp* lhpOpUploadMultiVMERefactor::Copy()
//----------------------------------------------------------------------------
{
	/** return a copy of itself, needs to put it into the undo stack */
	return new lhpOpUploadMultiVMERefactor(m_Label);
}
//----------------------------------------------------------------------------
void lhpOpUploadMultiVMERefactor::OpRun()
//----------------------------------------------------------------------------
{
  m_OpRunResult = OP_RUN_OK;
  // If exists delete the upload stack file: (This file is used to check upload integrity)
  wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  path += "UploadCallStack.txt";
  if(wxFileExists(path))
  {
    wxRemoveFile(path);
  }

  if (GetTestMode() == true)
  {
    m_OpUploadVME = new lhpOpUploadVMERefactor("vmeUploader");
  } 
  else
  {

    m_PassPhrase = lhpOpUploadVMERefactor::GeneratePassPhrase();
    m_PythonExe = m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath();
  
    m_User = m_UploadDownloadVMEHelper->GetUser();

    if(m_User->GetProxyFlag())
    {
      m_ProxyURL = m_User->GetProxyHost();
      m_ProxyPort = m_User->GetProxyPort();
      m_UploadDownloadVMEHelper->SaveConnectionConfigurationFile();
    }
    else
    {
      m_UploadDownloadVMEHelper->RemoveConnectionConfigurationFile();
    }

    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);
    if(m_ServiceURL.Equals("1"))
    {
      wxMessageBox("Error in GetServiceURL(). Upload stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
      m_OpRunResult = OP_RUN_CANCEL;
      if ( !m_DontStopAfterOpRun )
      {
        OpStop(OP_RUN_CANCEL);
      }
      
      return;
    }
    FillRepositoryVar();

    int result = OP_RUN_CANCEL;

    mafString DebugPath = m_VMEUploaderDownloaderABSFolder;
    DebugPath.Append("\\Debug.py");
    if (wxFileExists(DebugPath.GetCStr()))
    {
      ifstream debugFile;
      debugFile.open(DebugPath.GetCStr());
      if (!debugFile) {
        mafLogMessage("Unable to open Debug.py file");
      }

      std::string isDebug;
      debugFile >> isDebug;
      int pos = isDebug.find_last_of('=');
      isDebug = isDebug.substr(pos+1);
      if (!isDebug.compare("1") || !isDebug.compare("True"))
      {
        m_DebugMode = true;
      }
      debugFile.close();
    }

    int res = wxMessageBox("By continuing the upload you are agreeing to comply with and be bound\nby the terms of use that can be found at the following link (please review it carefully)\n\nhttps://www.biomedtown.org/biomed_town/LHDL/users/repository/public/TermsOfUse\n", "Terms of Use Agreement", wxOK | wxCANCEL);
    //returns 4 for OK, 16 for CANCEL
    if (res == 16)
    {
      //cppDEL(m_OpUploadVME);
      m_OpRunResult = result;
      if ( !m_DontStopAfterOpRun )
      {
        OpStop(result);
      }
      return;
    }

    // Collapse all LanmdarkClouds in the tree
    mafNodeIterator *iter = m_Input->GetRoot()->NewIterator();
    for (mafNode *node = iter->GetFirstNode(); node; node = iter->GetNextNode())
    {
      if(node->IsA("mafVMELandmarkCloud"))
      {
        mafVMELandmarkCloud *tempLM = mafVMELandmarkCloud::SafeDownCast(node);
        if(tempLM != NULL)
        {
          if(tempLM->IsOpen())
          {
            tempLM->Close();
          }
        }
      }
    }
    mafDEL(iter);
    mafString s(_("Upload VMEs"));
    mafEvent e(this,VME_CHOOSE, &s);
    e.SetBool(true); //true to create dialog with VME multiselect
    mafEventMacro(e);

    // iterate over children and remove vme not visible to traverse
    // BUG #2066
    std::vector<mafNode*> vmeToBeUploadedVectorTmp;
    vmeToBeUploadedVectorTmp = e.GetVmeVector();

    for(int j = 0; j < vmeToBeUploadedVectorTmp.size(); j++)
    {
      mafNode *node = vmeToBeUploadedVectorTmp.at(j);
      if(node->IsVisible() && node->IsInTree(node))
        m_VMEToBeUploadedVector.push_back(node);
    }
    vmeToBeUploadedVectorTmp.clear();

    int size = m_VMEToBeUploadedVector.size();

    if (size == 0)
    {
      m_OpRunResult = result;
      //cppDEL(m_OpUploadVME);
      if (!m_DontStopAfterOpRun)
      {
        OpStop(result);
      }
      return;
    } 
    
    std::vector<const char*> badVmeNames;
    std::vector<wxString> badChrMessage;
    // Check on vme names
    // Added to avoid UnicodeEncodeError exception in the python module minidom
    for (std::vector<mafNode*>::iterator it = m_VMEToBeUploadedVector.begin(); it != m_VMEToBeUploadedVector.end(); it++)
    {
      const char *vmeName = (*it)->GetName();
      for(int i = 0; i<strlen(vmeName); i++)
      {
        if(!((vmeName[i] < 127)&& (vmeName[i]>=0)))// vme name contains special characters
        {
          badVmeNames.push_back(vmeName);
          badChrMessage.push_back(wxString::Format("(character %c not allowed)", vmeName[i]));
          break;
        }
      }
    }
    if(badVmeNames.size()>0)
    {
      mafString errorMessage = "Upload stopped!\nSpecial characters in vme names. Please rename the following vme:\n";
      int n = 0;
      for(std::vector<const char*>::iterator it = badVmeNames.begin(); it != badVmeNames.end(); it++)
      {
        errorMessage = errorMessage + (*it) + " - " + badChrMessage.at(n).c_str() + "\n";
        n++;
      }
      badVmeNames.clear();
      badChrMessage.clear();
      m_VMEToBeUploadedVector.clear();
      wxMessageBox(wxString(errorMessage.GetCStr()),"Error",wxICON_ERROR);
      result = OP_RUN_CANCEL;
      m_OpRunResult = result;
      //cppDEL(m_OpUploadVME);
      if (!m_DontStopAfterOpRun)
      {
        OpStop(result);
      }
      return;
    }

    m_OpUploadVME = new lhpOpUploadVMERefactor("vmeUploader");
    m_OpUploadVME->SetDebugMode(m_DebugMode);
    m_OpUploadVME->SetPassPhrase(m_PassPhrase);
    m_OpUploadVME->SetListener(this->GetListener());
    m_OpUploadVME->SetPythonInterpreterFullPath(m_PythonExe);
    m_OpUploadVME->SetPythonwInterpreterFullPath(m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath());
    assert(m_User);
    m_OpUploadVME->SetUser(m_User);
    m_OpUploadVME->SetGroupsChoice(m_GroupsChoice);
    m_OpUploadVME->SetRepositoryURL(m_ServiceURL.GetCStr());

    bool upToDate = false;
    upToDate = m_UploadDownloadVMEHelper->IsClientSoftwareVersionUpToDate();

    // ask user if user will overwrite the existing file or will save it with another name

    if (upToDate)
    {

      mafEvent event;
      event.SetSender(this);
      event.SetId(ID_MSF_DATA_CACHE);
      mafEventMacro(event);

      wxString temp;
      temp.Append((*event.GetString()).GetCStr());
      m_OpenMSFFileNameFullPath = temp;
      //logic comunicate the msf directory
      temp = temp.BeforeLast('/');
      mafString openMsfABSFolder = temp;  

      // Bug #2055 fix
      bool overwrite;

      if(m_OpenMSFFileNameFullPath == "") // if filename is empty ask for it
      {
        overwrite = false;
      }
      else
      {
        overwrite = (wxMessageBox("The upload process will save the current msf. Would you like overwrite the existing one?","Warning" ,wxYES_NO | wxICON_QUESTION) == wxYES);
      }
      if(overwrite)
      {
        mafEventMacro(mafEvent(this, MENU_FILE_SAVE)); // filename correct and will overwrite 
      }
      else
      {
        mafEventMacro(mafEvent(this, MENU_FILE_SAVEAS)); // filename empty or will not overwrite
      }
      
      temp = "";
      temp.Append((*event.GetString()).GetCStr());
      m_OpenMSFFileNameFullPath = temp;
      //logic comunicate the msf directory
      temp = temp.BeforeLast('/');
      openMsfABSFolder = temp;  // do it again to check if user has saved the msf properly

      if (openMsfABSFolder == "")
        wxMessageBox("Msf must be saved locally. Upload stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
      else
        result = OP_RUN_OK;

      m_OpRunResult = result;
      //cppDEL(m_OpUploadVME);
      if (!m_DontStopAfterOpRun)
      {
        OpStop(result); 
      }
    }
    else
    {
      m_OpRunResult = result;
      //cppDEL(m_OpUploadVME);
      if (!m_DontStopAfterOpRun)
      {
        OpStop(result);
      }
    }
  }
}
//----------------------------------------------------------------------------
void lhpOpUploadMultiVMERefactor::OpDo()   
//----------------------------------------------------------------------------
{
  // write on a file the number of vme that will be uploaded
  wxString path = m_VMEUploaderDownloaderABSFolder;
  path += "lhpCommunication.txt";

  //if file exists , delete it
  if (wxFileExists(path))
    wxRemoveFile(path);

  ofstream commFile;
  commFile.open(path);

  commFile << m_VMEToBeUploadedVector.size() << "\n";

  commFile.close();

// Bug 1988 fix (to be completed)
// first check the if there is enough free space on server
// calculate total binary file size
  long neededSpace = 0;

  wxString openMsfABSFolder = m_OpenMSFFileNameFullPath.GetCStr();
  openMsfABSFolder = openMsfABSFolder.BeforeLast('/');

  for(int i = 0; i < m_VMEToBeUploadedVector.size(); i++)
  {
    mafVMEGeneric *vme = mafVMEGeneric::SafeDownCast(m_VMEToBeUploadedVector.at(i));
    if(vme)
    {
      // get vme binary filename
      mmuTimeVector kframes;
      vme->GetDataVector()->GetTimeStamps(kframes);
      mafString archivename = mafString("");
      std::vector<mafString> fileNames;
      for(int i = 0; i < kframes.size(); i++)
      {
        mafString filename = vme->GetDataVector()->GetItem(kframes.at(i))->GetURL();
        archivename = vme->GetDataVector()->GetArchiveName();
        if(archivename.Compare("") == 1)
        {
          break;
        }
        else
        {
          bool isInVector = false;
          for(int j = 0; j < fileNames.size(); j++)
          {
            if(filename.Compare(fileNames.at(j)) == 0)
            {
              isInVector = true;
              break;
            }
          }
          if(!isInVector)
            fileNames.push_back(filename);
        }
      }
      if(archivename.Compare("") == 1)
      {
        if(!wxFileExists(openMsfABSFolder + "/" + archivename)) // BUG #1954 fix
        {
          // if don't exist raise an error!
          wxMessageBox(wxString::Format("Binary file %s not found!\nCannot start upload.", mafString(openMsfABSFolder + "/" + archivename).GetCStr()),"Error!",wxICON_ERROR);
          return;
        }
        neededSpace = neededSpace + GetBinaryFileSize(openMsfABSFolder + "/" + archivename);
      }
      else
      {
        for(int j = 0; j < fileNames.size(); j++)
        {
          // first check if file exists

          if(!wxFileExists(openMsfABSFolder + "/" + fileNames.at(j))) // BUG #1954 fix
          {
            // if don't exist raise an error!
            wxMessageBox(wxString::Format("Binary file %s not found!\nCannot start upload.", mafString(openMsfABSFolder + "/" + fileNames.at(j)).GetCStr()),"Error!",wxICON_ERROR);
            return;
          }
          neededSpace = neededSpace + GetBinaryFileSize(openMsfABSFolder + "/" + fileNames.at(j));
        }
      }
    }
  }
// 
  if(neededSpace>0)
  {
    mafLogMessage(wxString::Format("Required space is %d bytes", neededSpace).c_str());
    
    wxString oldDir = wxGetCwd();
    wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());

    wxString command2execute;
    command2execute.Clear();
    command2execute = m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath();

    // ask the server for the free space
    command2execute.Append(" GetUserAvailableSpace.py ");
    command2execute.Append(m_User->GetName());
    command2execute.Append(" ");
    command2execute.Append(m_User->GetPwd());
    command2execute.Append(" ");
    command2execute.Append(m_Repository.GetCStr());
    command2execute.Append(" ");

//     if (m_DebugMode)
//       mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

    long pid = -1;
    pid = wxExecute(command2execute, wxEXEC_SYNC);
    mafSleep(10000);

    path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
    path += "lhpAvailableFreeSpaceOnServer.txt";

    //if file exists , delete it
    if (wxFileExists(path))
    {
      std::string strAvailableSpace;
      ifstream commFile;
      commFile.open(path);

      commFile >> strAvailableSpace;
      commFile.close();
      float availableSpace = atof(strAvailableSpace.c_str());

      
      mafLogMessage(wxString::Format("Available space is %f bytes", availableSpace).c_str());

      if(neededSpace > availableSpace)
      {
        wxMessageBox(wxString::Format("Not enough space on server! cannot start upload!\nRequired space is: %d bytes\nAvailable space is: %d bytes",neededSpace,availableSpace),"Error Over Quota!",wxICON_ERROR);
        wxRemoveFile(path);
        wxSetWorkingDirectory(oldDir);      
        return;
      }
      
      wxRemoveFile(path);
      wxSetWorkingDirectory(oldDir);
    }
//     if (pid != 0)
//     {
//         mafLogMessage(_T("ASYNC Command process '%s' returns %d."), command2execute.c_str(), pid);
//         wxMessageBox(wxString::Format("Not enough space on server! cannot start upload!\nRequired space is: %d byte",neededSize),"Error Over Quota!",wxICON_ERROR);
//         return;
//     }
  }
  Upload();
  return;
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::UploadVMEWithItsChildren( mafNode *vme )
//----------------------------------------------------------------------------
{
  int number = 0;
  mafString rollBackLocalFileName = "msfList";
  mafString xmlDataResourcesRollBackLocalFileName = rollBackLocalFileName;

  while(wxFileExists(\
  m_VMEUploaderDownloaderABSFolder + xmlDataResourcesRollBackLocalFileName.GetCStr() + ".lhp"))
  {
    xmlDataResourcesRollBackLocalFileName = rollBackLocalFileName;
    xmlDataResourcesRollBackLocalFileName << number;
    number++;
  }

  mafString msfFilePath = m_VMEUploaderDownloaderABSFolder + xmlDataResourcesRollBackLocalFileName.GetCStr() + ".lhp";
  FILE *file = fopen(msfFilePath,"w");
  fclose(file);
  xmlDataResourcesRollBackLocalFileName << ".lhp";

  std::vector<const mafNode::mafChildrenVector*> childrenVector;
  childrenVector.clear();
  bool hasBinaryData = false;
  bool alreadyUploaded = false;
  bool emptyNode = false;

  int numVmeChild = 0;
  int i = 0;
  mafNode *childToUpload = NULL;
  
  const mafNode::mafChildrenVector *childrenAll;
  const mafNode::mafChildrenVector *children;
  mafNode::mafChildrenVector *childrenTmp;

  childrenTmp = new mafNode::mafChildrenVector();

  childrenAll = vme->GetChildren();

  // iterate over children and remove vme not visible to traverse
  // BUG #2066
  for(int j = 0; j < childrenAll->size(); j++)
  {
    mafNode *node = childrenAll->at(j);
    if(node->IsVisible() && node->IsInTree(node))
      childrenTmp->push_back(node);
  }

  children = childrenTmp;

  childrenVector.push_back(children);
  numVmeChild = children->size();
  if (numVmeChild != 0)
  {
    childToUpload = children->at(0);

    std::ostringstream stringStream;
    stringStream << "childToUpload:" << childToUpload->GetName() << std::endl;
    mafLogMessage(stringStream.str().c_str());
  }
          
  while (numVmeChild != 0)
  {
    i = 0;
    for ( i ; i < children->size() ; i++)
    {
      numVmeChild = children->size();
      alreadyUploaded = false;
      emptyNode = false;
      
      //Check if VME children has been already uploaded
      for (int c = 0; c < m_EmptyNodeVector.size(); c++)
      {
        if (m_EmptyNodeVector[c]->Equals(children->at(i)) && m_EmptyNodeVector[c]->GetId() == children->at(i)->GetId())
        {
          emptyNode = true;
          break;
        }
      }

      // avoid bad children count bug #2070 fix
      children = children->at(i)->GetChildren();

      childrenTmp = new mafNode::mafChildrenVector();
      for(int j = 0; j < children->size(); j++)
      {
        mafNode *node = children->at(j);
        if(node->IsVisible() && node->IsInTree(node))
          childrenTmp->push_back(node);
      }
      children = childrenTmp;

      if (children->size() == 0 || emptyNode)
      {
        children = childrenVector.back();
        childToUpload = children->at(i);

        for (int c = 0; c < m_AlreadyUploadedVMEVector.size(); c++) 
        {
          if (m_AlreadyUploadedVMEVector[c]->Equals(children->at(i)) && \
              m_AlreadyUploadedVMEVector[c]->GetId() == children->at(i)->GetId())
          {
            alreadyUploaded = true;
            break;
          }
        }
        
        if (!alreadyUploaded)
        {
          if (childToUpload->GetNumberOfLinks() != 0)
          {
            m_DerivedVMEsIdVector.push_back(childToUpload->GetId());
          }

          if(childToUpload->IsVisible()) // avoid upload gizmos :) bug #2066 fix
          {
            m_OpUploadVME->SetInput(childToUpload);

            if (GetUploadError())
            {
              RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
              return MAF_ERROR;
            }
            // Iterate over children to discard gizmos
            int numchildren = 0;
            const mafNode::mafChildrenVector *cVector = childToUpload->GetChildren();
            for(int j = 0; j < cVector->size(); j++)
            {
              mafNode *node = cVector->at(j);
              if(node->IsVisible() && node->IsInTree(node))
                numchildren++;
            }

            m_OpUploadVME->SetWithChild(numchildren!=0);
            m_OpUploadVME->SetXMLUploadedResourcesRollBackLocalFileName(xmlDataResourcesRollBackLocalFileName);
            m_OpUploadVME->SetIsLast(false);

            if (m_OpUploadVME->Upload()\
              == MAF_ERROR)
            {
              RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
              return MAF_ERROR;
            }
          }

          m_AlreadyUploadedVMEVector.push_back(childToUpload);
          m_EmptyNodeVector.push_back(childToUpload);
          m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());
          if (SaveChildrenURIFile(childToUpload, m_OpUploadVME->GetRemoteXMLResourceURI()) \
          == MAF_ERROR)
          {
            
            return MAF_ERROR;
          }
        }
      }
      else
      {
        childrenVector.push_back(children);
        i = -1;
      }
    }
    m_EmptyNodeVector.push_back(childToUpload->GetParent());

    if (childrenVector.size() > 1)
      childrenVector.pop_back();
    else
      break;

    children = childrenVector.back();
    numVmeChild = children->size();

    mafSleep(5000);
  }

  
  if (GetUploadError())
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }
  
  // finally upload VME Root
  m_OpUploadVME->SetInput(vme);
  m_OpUploadVME->SetWithChild(vme->GetNumberOfChildren() != 0);
  m_OpUploadVME->SetXMLUploadedResourcesRollBackLocalFileName(xmlDataResourcesRollBackLocalFileName);
  m_OpUploadVME->SetIsLast(true);

  if (m_OpUploadVME->Upload() == MAF_ERROR)
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }

  m_AlreadyUploadedVMEVector.push_back(vme);
  m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());
  
  if (AddLinksURIToDerivedVMEMetadata(vme) == MAF_ERROR)
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }
  
  return MAF_OK;
  
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::UploadVMEWithItsLinks( mafNode *vme, bool isLast )
//----------------------------------------------------------------------------
{  
  bool hasBinaryData = false;
  mafString xmlURI;

  // first upload the links...
  if (vme->GetNumberOfLinks() != 0)
  {
    if (UploadVMELinks(vme) == MAF_ERROR)
    {
      return MAF_ERROR;
    }
  }

  m_OpUploadVME->SetInput(vme);
  xmlURI = "";

  m_OpUploadVME->SetWithChild(false);
  m_OpUploadVME->SetXMLUploadedResourcesRollBackLocalFileName("noMSF");
  m_OpUploadVME->SetIsLast(isLast); // Changed

  // ...then upload the master vme
  if (m_OpUploadVME->Upload(true) == MAF_ERROR)
  {
    m_AlreadyUploadedXMLURIVector.clear();
    m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }

  return MAF_OK;
}
//----------------------------------------------------------------------------
bool lhpOpUploadMultiVMERefactor::GetUploadError()
//----------------------------------------------------------------------------
{
  bool errorFound = false;
  if (wxFileExists(m_VMEUploaderDownloaderABSFolder + "ErrorFound.lhp"))
  {
    mafString errorMessage;
    std::ifstream errorFile(m_VMEUploaderDownloaderABSFolder + "ErrorFound.lhp", std::ios::in);
	if (!errorFile.fail())
    {
      std::string buf;
      getline(errorFile, buf);
      errorMessage.Append(buf.c_str());
      errorMessage.Append("\n");
      while (!errorFile.eof())
      {
        getline(errorFile, buf);
        errorMessage.Append(buf.c_str());
      }

      wxMessageBox(wxString::Format("Error in MSF upload.\n%s\nUpload MSF stopped.\nVME already uploaded \
will be removed from repository.",errorMessage.GetCStr()), wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
      errorFile.close();
      wxRemoveFile(m_VMEUploaderDownloaderABSFolder + "ErrorFound.lhp");
      errorFound = true;
    }
  }
  return errorFound;
}
//----------------------------------------------------------------------------
bool lhpOpUploadMultiVMERefactor::RemoveAlreadyUploadedXMLResources(std::vector<mafString> xmlURIVector)   
//----------------------------------------------------------------------------
{
  if(m_ServiceURL.Compare("") == 0)
  {
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);
    FillRepositoryVar();
  }

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());

  wxString command2execute;

  //WAIT MESSAGE:
  wxInfoFrame *wait;
  if(!m_TestMode)
  {
    wait = new wxInfoFrame(NULL, "Please wait, removing uploaded VME");
    wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
    wait->Show(true);
    wait->Refresh();
    wait->Update();
  }

  for (int i = 0; i < xmlURIVector.size(); i++)
  {
    command2execute.Clear();
    command2execute = "pythonw.exe ";

    // remove XML resource from repository
    command2execute.Append(" lhpRemoveXMLResource.py ");
    command2execute.Append(m_User->GetName());
    command2execute.Append(" ");
    command2execute.Append(m_User->GetPwd());
    command2execute.Append(" ");
    command2execute.Append(m_ServiceURL.GetCStr());
    command2execute.Append(" ");
    command2execute.Append(xmlURIVector[i].GetCStr());

    //if (m_DebugMode)
    mafLogMessage( _T("[TEST] - Executing command: '%s'"), command2execute.c_str() );

    long pid = -1;
    if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
    {
      wxMessageBox("Error in lhpRemoveXMLResource. Can not remove uploaded resource", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
      if (m_DebugMode)
        mafLogMessage(_T("ASYNC Command process '%s' terminated with exit code %d."),
        command2execute.c_str(), pid);
      if(!m_TestMode)
      {
        delete wait;
      }
      return MAF_ERROR;
    }
  } 
  if(!m_TestMode)
  {
    delete wait;
  }

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::AddLinksURIToDerivedVMEMetadata(mafNode *derived )
//----------------------------------------------------------------------------
{
  if(m_ServiceURL.Compare("") == 0)
  {
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);
    FillRepositoryVar();
  }
  wxString vmeURI;
  mafString listURI;
  mafNode *currentDerivedVME;

  for (int i = 0; i < m_DerivedVMEsIdVector.size(); i++)
  {
    currentDerivedVME = derived->FindInTreeById(m_DerivedVMEsIdVector[i]);

    //find URI of this VME
    for (int c = 0; c < m_AlreadyUploadedVMEVector.size(); c++)
    {
      if (m_AlreadyUploadedVMEVector[c]->Equals(currentDerivedVME) && \
          m_AlreadyUploadedVMEVector[c]->GetId() == currentDerivedVME->GetId())
      {
        vmeURI = m_AlreadyUploadedXMLURIVector[c];
        int count = vmeURI.find_first_of("'");
        vmeURI.erase(0, count+1);
        count = vmeURI.find_first_of("'");
        vmeURI = (vmeURI.substr(0, count)).c_str();
        break;
      }
    }

    for (mafNode::mafLinksMap::iterator i = currentDerivedVME->GetLinks()->begin(); \
      i != currentDerivedVME->GetLinks()->end(); i++)
    {
      if (i->second.m_Node != NULL)
      {
        mafNode *link = i->second.m_Node;
        for (int c = 0; c < m_AlreadyUploadedVMEVector.size(); c++)
        {
          if (m_AlreadyUploadedVMEVector[c]->Equals(link) && \
              m_AlreadyUploadedVMEVector[c]->GetId() == link->GetId())
          {
            m_AlreadyUploadedXMLURIVector[c];
            listURI.Append(m_AlreadyUploadedXMLURIVector[c]);
            listURI.Append(" ");
            break;
          }
        }
      }
    }

    wxString oldDir = wxGetCwd();
    wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());
    if (m_DebugMode)
      mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

    //Add URI tag to link VME uploaded
    wxString command2execute;
    command2execute.Clear();
    command2execute = m_PythonExe.GetCStr();

    command2execute.Append("lhpEditRemoteTag.py ");
    command2execute.Append(m_User->GetName());
    command2execute.Append(" ");
    command2execute.Append(m_User->GetPwd());
    command2execute.Append(" ");
    command2execute.Append(m_ServiceURL.GetCStr());
    command2execute.Append(" ");
    command2execute.Append(vmeURI.c_str());
    command2execute.Append(",");
    command2execute.Append("L0000_resource_MAF_Procedural_VMElinkURI1");
    command2execute.Append(",");
    command2execute.Append(listURI.GetCStr());
    if (m_DebugMode)
      mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

    long pid = -1;
    if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
    {
      wxMessageBox("Error in lhpEditRemoteTag.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
      mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
        command2execute.c_str(), pid);
      return MAF_ERROR;
    }

    wxSetWorkingDirectory(oldDir);
    if (m_DebugMode)
      mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  }
  return MAF_OK;

}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::UploadVMELinks( mafNode *derived )
//----------------------------------------------------------------------------
{  
  bool emptyNode = false;
  bool alreadyUploaded = false;
  bool hasBinaryData = false;
  std::vector<mafString> linkURI;
  linkURI.clear();


  for (mafNode::mafLinksMap::iterator i = derived->GetLinks()->begin(); i != derived->GetLinks()->end(); i++)
  {
    hasBinaryData = false;
    alreadyUploaded = false;
    mafString xmlURI;
    if (i->second.m_Node != NULL)
    {
      mafNode *link = i->second.m_Node;
      if (link->IsA("mafVMELandmarkCloud") && i->second.m_NodeSubId != -1)
      {
        ((mafVMELandmarkCloud *)link)->Open();
        link = (mafNode*)((mafVMELandmarkCloud *)link)->GetLandmark(i->second.m_NodeSubId);
      }

      wxMessageBox(wxString::Format("Link found! Upload VME: %s", link->GetName()), wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);

      //Verify if the link has some link!!
      if (link->GetNumberOfLinks() != 0)
      {
        if (UploadVMELinks(link) == MAF_ERROR)
        {
          return MAF_ERROR;
        }
      }
      m_OpUploadVME->SetInput(link);

      xmlURI = "";

      m_OpUploadVME->SetWithChild(false);
      m_OpUploadVME->SetXMLUploadedResourcesRollBackLocalFileName("noMsf");
      m_OpUploadVME->SetIsLast(false);

      if (m_OpUploadVME->Upload(true) == MAF_ERROR)
      { 

        m_AlreadyUploadedXMLURIVector.clear();
        m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());
        RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
        return MAF_ERROR;
      }
      m_AlreadyUploadedVMEVector.push_back(link);
      linkURI.push_back(xmlURI);
    }
  }
  if (SaveLinksURIFile(derived, linkURI) == MAF_ERROR)
  {
    wxMessageBox("Unable to write list of link binary URI. Uploading stopped.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }
  return MAF_OK;
}
//------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::SaveLinksURIFile(mafNode *node, std::vector<mafString> linkURI)
//------------------------------------------------------------
{
  wxString listURIFileName;
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  listURIFileName = node->GetName();
  listURIFileName.Append(wxString::Format("%d",node->GetId()));
  listURIFileName.Replace("/","_");
  listURIFileName.Replace("\\","_");
  listURIFileName.Replace(":","_");
  listURIFileName.Replace("*","_");
  listURIFileName.Replace("?","_");
  listURIFileName.Replace("\"","_");
  listURIFileName.Replace("<","_");
  listURIFileName.Replace(">","_");
  listURIFileName.Replace("!","_");
  listURIFileName.Append(".linkURI");
  wxString lockPath = m_VMEUploaderDownloaderABSFolder;
  lockPath += listURIFileName.c_str();

  //if file exists , delete it
  if (wxFileExists(lockPath))
    wxRemoveFile(lockPath);

  ofstream listURIFile;
  listURIFile.open(lockPath);
  if (!listURIFile)
  {
    return MAF_ERROR;
  }
  else
  {
    for (int n = 0; n < linkURI.size(); n++)
    {
      listURIFile << linkURI[n];
      listURIFile << "\n";
    }
    listURIFile.close();
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  return MAF_OK;
}
//------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::SaveChildrenURIFile(mafNode* node, mafString URI)
//------------------------------------------------------------
{
  wxString listURIFileName;
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  listURIFileName = node->GetParent()->GetName();
  listURIFileName.Append(wxString::Format("%d",node->GetParent()->GetId()));
  listURIFileName.Replace("/","_");
  listURIFileName.Replace("\\","_");
  listURIFileName.Replace(":","_");
  listURIFileName.Replace("*","_");
  listURIFileName.Replace("?","_");
  listURIFileName.Replace("\"","_");
  listURIFileName.Replace("<","_");
  listURIFileName.Replace(">","_");
  listURIFileName.Replace("!","_");
  listURIFileName.Replace("|","_");
  listURIFileName.Append(".childURI");

  wxString lockPath = m_VMEUploaderDownloaderABSFolder;
  lockPath += listURIFileName.c_str();
  //Check if file named "lockPath" has been created by this operation
  bool myFile = false;
  for (int n = 0; n < m_FileCreatedVector.size(); n++)
  {
    if (m_FileCreatedVector[n].Equals(lockPath))
    {
      myFile = true;
      break;
    }
  }

   //if file exists and is not created by this operation, delete it
  if (!myFile)
  {
    if (wxFileExists(lockPath.c_str()))
      wxRemoveFile(lockPath.c_str());
  }

  m_FileCreatedVector.push_back(lockPath);

  // open auto tags file and try to handle tags using tags factory 
  ofstream listURIFile;

  listURIFile.open(lockPath, fstream::in | fstream::out | fstream::app);
  if (!listURIFile)
  {
    return MAF_ERROR;
  }
  else
  {
    listURIFile << URI;
    listURIFile << "\n";
  }
    listURIFile.close();

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  return MAF_OK;
}
//----------------------------------------------------------------------------
void lhpOpUploadMultiVMERefactor::OpStop(int result)   
//----------------------------------------------------------------------------
{
	mafEventMacro(mafEvent(this,result));
}
//--------------------------------------------------------------------------------------------
mafString lhpOpUploadMultiVMERefactor::GetXMLDictionaryFileName( mafString dictionaryFileNamePrefix )
//--------------------------------------------------------------------------------------------
{
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);
  
  if (files.size() != 1)
  {
    mafLogMessage("lhpXMLDictionary_*.xml not found! exiting");
    return dictionaryFileName;
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
    if (m_DebugMode)
    {
      mafLogMessage("Found dictionary!");
      mafLogMessage(dictionaryFileName.GetCStr());
    }
  }
  
  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  
  return dictionaryFileName;
}
//----------------------------------------------------------------------------
void lhpOpUploadMultiVMERefactor::Upload()
//----------------------------------------------------------------------------
{
  
  bool rootSelected = false;

  if (GetTestMode() == true)
  {
    mafLogMessage("All Vmes will be uploaded with their metadata.\nPlease check before uploading. Completion of curation can be done in the sandbox.");
  }
  else
  {
    int res = wxMessageBox("All Vmes will be uploaded with their metadata.\nPlease check before uploading. Completion of curation can be done in the sandbox.", wxMessageBoxCaptionStr, wxOK | wxCANCEL);
    //returns 4 for OK, 16 for CANCEL
    if (res == 16)
    {
      cppDEL(m_OpUploadVME);
      return;
    }
  }
  wxWindowDisabler *disableAll;
  wxBusyCursor *wait_cursor;
  //wxInfoFrame *wait;
  if(!m_TestMode)
  {
//     wait = new wxInfoFrame(NULL, "Initializing upload process. Please wait.");
//     wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
//     wait->Show(true);
//     wait->Refresh();
//     wait->Update();
    disableAll = new wxWindowDisabler();
    wait_cursor = new wxBusyCursor();
  }

  //Check if VMERoot has been chosen, than upload only the root with its children
  
  // root selected?
  for (int i = 0; i < m_VMEToBeUploadedVector.size(); i++)
  {
    if (m_VMEToBeUploadedVector[i]->IsA("mafVMERoot"))
    {
      rootSelected = true;
      if (wxFileExists(m_VMEUploaderDownloaderABSFolder + "ErrorFound.lhp"))
      {
        wxRemoveFile(m_VMEUploaderDownloaderABSFolder + "ErrorFound.lhp");
      }

      mafNode *vmeWithNoName = NULL;

      vmeWithNoName = m_VMEToBeUploadedVector[i]->FindInTreeByName("");

      if  (vmeWithNoName != NULL)
      {
        wxMessageBox("Tree contains VME without name. Upload Stopped.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        cppDEL(m_OpUploadVME);
        return;
      }

      wxString rootName = m_VMEToBeUploadedVector[i]->GetName();
      rootName = rootName.Lower();
 
      if  (rootName.CompareTo("root") == 0)
      {
        wxString path, newRootName, ext;
        wxString msfFullPathName = m_OpenMSFFileNameFullPath;
        wxSplitPath(msfFullPathName, &path, &newRootName, &ext);
        wxMessageBox(wxString::Format("Root name will be modified with the name as the MSF: %s "\
        ,newRootName), wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        m_VMEToBeUploadedVector[i]->SetName(newRootName.c_str());
        m_VMEToBeUploadedVector[i]->Modified();
        ((mafVMERoot *)m_VMEToBeUploadedVector[i])->Update();

        mafEvent ev(this,VME_MODIFIED,m_VMEToBeUploadedVector[i]);
        mafEventMacro(ev);
      }
      
      // upload root with its children vmes
      if (UploadVMEWithItsChildren(m_VMEToBeUploadedVector[i]) == MAF_ERROR)
      {
        if(!m_TestMode)
        {
          //delete wait;
          cppDEL(disableAll);
          cppDEL(wait_cursor);
        }
        cppDEL(m_OpUploadVME);
        return;
      }
      break;
    }
  }

  // root not selected  =>
  if (!rootSelected)
  {
    bool isLast = false;
    // for each selected vme:
    for (int i = 0; i < m_VMEToBeUploadedVector.size(); i++)
    {
      if (strcmp(m_VMEToBeUploadedVector[i]->GetName(), "") == 0)
      {
        wxMessageBox("Can not upload VME without name.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        cppDEL(m_OpUploadVME);
        return;
      }

      //check if is last VME
      if (i+1 == m_VMEToBeUploadedVector.size())
        isLast = true;

      if (UploadVMEWithItsLinks(m_VMEToBeUploadedVector[i], isLast) == MAF_ERROR)
      {
        if(!m_TestMode)
        {
          //delete wait;
          cppDEL(disableAll);
          cppDEL(wait_cursor);
        }
        cppDEL(m_OpUploadVME);
        return;
      }
    }
  }

  if(!m_TestMode)
  {
    //delete wait;
    cppDEL(disableAll);
    cppDEL(wait_cursor);
  }

  cppDEL(m_OpUploadVME);
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::LoadVMEsToUploadIdsVectorFromFile(const char *vmeIdsFileName)
//----------------------------------------------------------------------------
{
  std::ifstream inputFile(vmeIdsFileName, std::ios::in);

  if (inputFile.fail()) {
    std::cerr << "Error opening " << vmeIdsFileName << "\n";
    assert(false);
    return MAF_ERROR;
  }

  int buf;
  
  m_VMEsToUploadIdsVector.clear();

  while(inputFile >> buf)
  {
    m_VMEsToUploadIdsVector.push_back(buf);
  }

  inputFile.close();

  assert(m_Input);

  mafNode *root = m_Input->GetRoot();
  assert(root);

  m_VMEToBeUploadedVector.clear();

  for (int i = 0; i < m_VMEsToUploadIdsVector.size(); i++) 
  {
    mafNode *node = root->FindInTreeById(m_VMEsToUploadIdsVector[i]);
    m_VMEToBeUploadedVector.push_back(node);
    assert(node);
    if (node == NULL)
    {
      std::ostringstream stringStream;
      stringStream << "Node with id: " << m_VMEToBeUploadedVector[i] << \
        " does not exist in VME tree. Exiting with MAF_ERROR" << std::endl;
      mafLogMessage(stringStream.str().c_str());
      m_VMEToBeUploadedVector.clear();
      return MAF_ERROR;
    }
  }
  return MAF_OK ;
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::SetVMEsToUploadIdsVector( std::vector<int> vmeIDsVector )
//----------------------------------------------------------------------------
{

  m_VMEsToUploadIdsVector.clear();

  m_VMEsToUploadIdsVector = vmeIDsVector;

  assert(m_Input);

  mafNode *root = m_Input->GetRoot();
  assert(root);

  m_VMEToBeUploadedVector.clear();

  for (int i = 0; i < m_VMEsToUploadIdsVector.size(); i++) 
  {
    mafNode *node = root->FindInTreeById(m_VMEsToUploadIdsVector[i]);
    m_VMEToBeUploadedVector.push_back(node);
    assert(node);
    if (node == NULL)
    {
        std::ostringstream stringStream;
        stringStream << "Node with id: " << m_VMEToBeUploadedVector[i] << \
        " does not exist in VME tree. Exiting with MAF_ERROR" << std::endl;
        mafLogMessage(stringStream.str().c_str());
        m_VMEToBeUploadedVector.clear();
        return MAF_ERROR;
    }
  }

  return MAF_OK ;
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::KillThreadedUploaderDownloaderProcess()
//----------------------------------------------------------------------------
{
  return m_UploadDownloadVMEHelper->KillThreadedUploaderDownloaderProcess();
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::GetBinaryFileSize(mafString filename)
//----------------------------------------------------------------------------
{
  FILE *pFile = NULL;

  // get the file stream
  pFile = fopen( filename.GetCStr(), "rb" );

  // set the file pointer to end of file
  fseek( pFile, 0, SEEK_END );

  // get the file size
  int Size = ftell( pFile );

  // return the file pointer to begin of file if you want to read it
  // rewind( pFile );

  // close stream and release buffer
  fclose( pFile );

  return Size;
}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::UploadVMEWithItsChildrenObtimized( mafNode *vme )
//----------------------------------------------------------------------------
{
  int number = 0;
  mafString rollBackLocalFileName = "msfList";
  mafString xmlDataResourcesRollBackLocalFileName = rollBackLocalFileName;

  while(wxFileExists(\
    m_VMEUploaderDownloaderABSFolder + xmlDataResourcesRollBackLocalFileName.GetCStr() + ".lhp"))
  {
    xmlDataResourcesRollBackLocalFileName = rollBackLocalFileName;
    xmlDataResourcesRollBackLocalFileName << number;
    number++;
  }

  mafString msfFilePath = m_VMEUploaderDownloaderABSFolder + xmlDataResourcesRollBackLocalFileName.GetCStr() + ".lhp";
  FILE *file = fopen(msfFilePath,"w");
  fclose(file);
  xmlDataResourcesRollBackLocalFileName << ".lhp";

  std::vector<const mafNode::mafChildrenVector*> childrenVector;
  childrenVector.clear();
  bool hasBinaryData = false;
  bool alreadyUploaded = false;
  bool emptyNode = false;

  int numVmeChild = 0;
  int i = 0;
  mafNode *childToUpload = NULL;

  const mafNode::mafChildrenVector *childrenAll;
  const mafNode::mafChildrenVector *children;
  mafNode::mafChildrenVector *childrenTmp;

  childrenTmp = new mafNode::mafChildrenVector();

  childrenAll = vme->GetChildren();

  //////////////////////////////////////////////////////////////////////////
  //Create the cache directory for the builder session
  mafString cacheDirectory = CreateCacheDirectories();
  m_CurrentCacheChildABSFolder = cacheDirectory;
  mafString ids = "\"";
  mafString vmeCacheDirectory = cacheDirectory;
  vmeCacheDirectory<<vme->GetId();
  vmeCacheDirectory<<"\\";
  wxMkDir(vmeCacheDirectory.GetCStr());
  ids<<vme->GetId();
  //////////////////////////////////////////////////////////////////////////

  // iterate over children and remove vme not visible to traverse
  // BUG #2066
  for(int j = 0; j < childrenAll->size(); j++)
  {
    ids<<" ";
    mafNode *node = childrenAll->at(j);
    if(node->IsVisible() && node->IsInTree(node))
    {
      childrenTmp->push_back(node);

      //Create the cache directory for the current vme
      mafString vmeCacheDirectory = cacheDirectory;
      vmeCacheDirectory<<node->GetId();
      vmeCacheDirectory<<"\\";
      wxMkDir(vmeCacheDirectory.GetCStr());

      ids<<node->GetId();
      //////////////////////////////////////////////////////////////////////////
    }
  }
  //////////////////////////////////////////////////////////////////////////

  //   int res = BuildMSFForVMEToBeUploaded(ids);
  //   if (res == MAF_ERROR)
  //   {
  //     return res;
  //   }

  std::vector<lhpOpUploadVMERefactor*> listOfUpload;
  //////////////////////////////////////////////////////////////////////////

  children = childrenTmp;

  childrenVector.push_back(children);
  numVmeChild = children->size();
  if (numVmeChild != 0)
  {
    childToUpload = children->at(0);
  }

  std::ostringstream stringStream;
  stringStream << "childToUpload:" << childToUpload->GetName() << std::endl;
  mafLogMessage(stringStream.str().c_str());

  while (numVmeChild != 0)
  {
    i = 0;
    for ( i ; i < children->size() ; i++)
    {
      numVmeChild = children->size();
      alreadyUploaded = false;
      emptyNode = false;

      //Check if VME children has been already uploaded
      for (int c = 0; c < m_EmptyNodeVector.size(); c++)
      {
        if (m_EmptyNodeVector[c]->Equals(children->at(i)) && m_EmptyNodeVector[c]->GetId() == children->at(i)->GetId())
        {
          emptyNode = true;
          break;
        }
      }

      // avoid bad children count bug #2070 fix
      children = children->at(i)->GetChildren();

      childrenTmp = new mafNode::mafChildrenVector();
      for(int j = 0; j < children->size(); j++)
      {
        mafNode *node = children->at(j);
        if(node->IsVisible() && node->IsInTree(node))
          childrenTmp->push_back(node);
      }
      children = childrenTmp;

      if (children->size() == 0 || emptyNode)
      {
        children = childrenVector.back();
        childToUpload = children->at(i);

        for (int c = 0; c < m_AlreadyUploadedVMEVector.size(); c++) 
        {
          if (m_AlreadyUploadedVMEVector[c]->Equals(children->at(i)) && \
            m_AlreadyUploadedVMEVector[c]->GetId() == children->at(i)->GetId())
          {
            alreadyUploaded = true;
            break;
          }
        }

        if (!alreadyUploaded)
        {
          if (childToUpload->GetNumberOfLinks() != 0)
          {
            m_DerivedVMEsIdVector.push_back(childToUpload->GetId());
          }

          if(childToUpload->IsVisible()) // avoid upload gizmos :) bug #2066 fix
          {

            lhpOpUploadVMERefactor *opUploadVME = lhpOpUploadVMERefactor::SafeDownCast(m_OpUploadVME->Copy());
            opUploadVME->SetParserObtimizationOn();
            opUploadVME->SetDebugMode(m_DebugMode);
            opUploadVME->SetInput(childToUpload);
            opUploadVME->SetListener(this->GetListener());
            opUploadVME->SetPythonwInterpreterFullPath(m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath());
            opUploadVME->SetPythonInterpreterFullPath(m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath());
            opUploadVME->SetUser(m_User);

            if (GetUploadError())
            {
              RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
              return MAF_ERROR;
            }
            // Iterate over children to discard gizmos
            int numchildren = 0;
            const mafNode::mafChildrenVector *cVector = childToUpload->GetChildren();
            for(int j = 0; j < cVector->size(); j++)
            {
              mafNode *node = cVector->at(j);
              if(node->IsVisible() && node->IsInTree(node))
                numchildren++;
            }

            opUploadVME->SetWithChild(numchildren!=0);
            opUploadVME->SetXMLUploadedResourcesRollBackLocalFileName(xmlDataResourcesRollBackLocalFileName);
            opUploadVME->SetIsLast(false);

            opUploadVME->m_CurrentCacheChildABSFolder = cacheDirectory;
            opUploadVME->m_CurrentCacheChildABSFolder<<childToUpload->GetId();
            opUploadVME->m_CurrentCacheChildABSFolder<<"\\";

            if (!wxDirExists(opUploadVME->m_CurrentCacheChildABSFolder))
            {
              wxMkDir(opUploadVME->m_CurrentCacheChildABSFolder);
              ids<<" ";
              ids<<childToUpload->GetId();
            }

            int ret = opUploadVME->InitializeToUpload();
            if (ret == MAF_ERROR)
            {
              wxMessageBox("Problems generating tags list! Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
              return ret;
            }

            listOfUpload.push_back(opUploadVME);
            //////////////////////////////////////////////////////////////////////////

            //             if (m_OpUploadVME->Upload()\
            //               == MAF_ERROR)
            //             {
            //               RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
            //               return MAF_ERROR;
            //             }
          }

          m_AlreadyUploadedVMEVector.push_back(childToUpload);
          m_EmptyNodeVector.push_back(childToUpload);
          //m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());
          /*if (SaveChildrenURIFile(childToUpload, m_OpUploadVME->GetRemoteXMLResourceURI()) \
          == MAF_ERROR)
          {

          return MAF_ERROR;
          }*/
        }
      }
      else
      {
        childrenVector.push_back(children);
        i = -1;
      }
    }
    m_EmptyNodeVector.push_back(childToUpload->GetParent());

    if (childrenVector.size() > 1)
      childrenVector.pop_back();
    else
      break;

    children = childrenVector.back();
    numVmeChild = children->size();
  }

  //////////////////////////////////////////////////////////////////////////
  // finally upload VME Root
  lhpOpUploadVMERefactor *uploadRoot = lhpOpUploadVMERefactor::SafeDownCast(m_OpUploadVME->Copy());
  uploadRoot->SetParserObtimizationOn();
  uploadRoot->SetDebugMode(m_DebugMode);
  uploadRoot->SetWithChild(vme->GetNumberOfChildren() != 0);
  uploadRoot->SetInput(vme);
  uploadRoot->SetWithChild(vme->GetNumberOfChildren() != 0);
  uploadRoot->SetXMLUploadedResourcesRollBackLocalFileName(xmlDataResourcesRollBackLocalFileName);
  uploadRoot->SetIsLast(true);
  uploadRoot->SetListener(this->GetListener());
  uploadRoot->SetPythonwInterpreterFullPath(m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath());
  uploadRoot->SetPythonInterpreterFullPath(m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath());
  uploadRoot->SetUser(m_User);

  uploadRoot->m_CurrentCacheChildABSFolder = cacheDirectory;
  uploadRoot->m_CurrentCacheChildABSFolder<<vme->GetId();
  uploadRoot->m_CurrentCacheChildABSFolder<<"\\";

  int ret = uploadRoot->InitializeToUpload();
  if (ret == MAF_ERROR)
  {
    wxMessageBox("Problems generating tags list! Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return ret;
  }

  ids << "\"";

  int res = BuildMSFForVMEToBeUploaded(ids);
  if (res == MAF_ERROR)
  {
    return res;
  }


  for (int i=0;i<listOfUpload.size();i++)
  {
    if (listOfUpload.at(i)->UploadAlreadyInitilized() == MAF_ERROR)
    {
      RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
      return MAF_ERROR;
    }

    m_AlreadyUploadedXMLURIVector.push_back(listOfUpload.at(i)->GetRemoteXMLResourceURI());

    if (SaveChildrenURIFile(listOfUpload.at(i)->GetInput(), listOfUpload.at(i)->GetRemoteXMLResourceURI()) == MAF_ERROR)
    {
      return MAF_ERROR;
    }
  }

  if (GetUploadError())
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }
  //////////////////////////////////////////////////////////////////////////

  if (uploadRoot->UploadAlreadyInitilized() == MAF_ERROR)
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }

  m_AlreadyUploadedVMEVector.push_back(vme);
  m_AlreadyUploadedXMLURIVector.push_back(m_OpUploadVME->GetRemoteXMLResourceURI());

  /*if (SaveChildrenURIFile(listOfUpload.at(i)->GetInput(), listOfUpload.at(i)->GetRemoteXMLResourceURI()) \
  == MAF_ERROR)
  {

  return MAF_ERROR;
  }*/


  mafLogMessage("<<<<<<<<<<<Saving original MSF");
  mafEventMacro(mafEvent(this, MENU_FILE_SAVE));

  if (AddLinksURIToDerivedVMEMetadata(vme) == MAF_ERROR)
  {
    RemoveAlreadyUploadedXMLResources(m_AlreadyUploadedXMLURIVector);
    return MAF_ERROR;
  }

  return MAF_OK;

}
//----------------------------------------------------------------------------
int lhpOpUploadMultiVMERefactor::BuildMSFForVMEToBeUploaded(mafString ids)
//----------------------------------------------------------------------------
{
  FillMSFFileABSFolderIVar();

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());

  wxString command2execute;

  mafString pythonScriptName = "lhpEditVMETag.py ";

  wxString msfCurrentCacheChildWorkaroundABSFolder = m_CurrentCacheChildABSFolder;
  msfCurrentCacheChildWorkaroundABSFolder.Replace(" ", "??");

  wxString msfFileDirectoryWorkaroundABSFolder = m_MSFFileABSFolder;
  msfFileDirectoryWorkaroundABSFolder.Replace(" ", "??");

  command2execute = "pythonw.exe ";
  command2execute.Append(pythonScriptName.GetCStr());
  command2execute.Append(wxString::Format\
    ("%s ",msfCurrentCacheChildWorkaroundABSFolder)); //Input cache child directory
  command2execute.Append(wxString::Format\
    ("%s ",msfFileDirectoryWorkaroundABSFolder)); //Input MSF directory
  command2execute.Append(wxString::Format\
    ("%s ",ids.GetCStr())); //Input VME Id
  command2execute.Append(wxString::Format\
    ("%s ", "manualTagFile.csv")); //Input manualTagFile
  command2execute.Append(wxString::Format\
    ("%s", "handledAutoTagsList.csv")); //Input autoTagFile


  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
  long pid = -1;
  if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
  {
    wxString wdir=wxGetCwd();
    mafLogMessage("Current wrking directory : %s",wdir.c_str());
    wxMessageBox("Can't edit MSF. Uploading stopped!", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("ASYNC Command process '%s' terminated with exit code %d."),command2execute.c_str(), pid);
    wxSetWorkingDirectory(oldDir);
    return MAF_ERROR;
  }

  wxSetWorkingDirectory(oldDir);
  return MAF_OK;
}
//----------------------------------------------------------------------------
mafString lhpOpUploadMultiVMERefactor::CreateCacheDirectories()   
//----------------------------------------------------------------------------
{
  mafString cacheChildFolderLocalName = "0";
  mafString cacheMasterFolderABSName = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\UploadCache\\").c_str();

  bool result = false;
  //control cache subdir
  wxString currentSubdir;
  currentSubdir = cacheMasterFolderABSName + cacheChildFolderLocalName.GetCStr();
  while(wxDirExists(currentSubdir))
  {
    int number = atoi(cacheChildFolderLocalName.GetCStr());
    number += 1;
    cacheChildFolderLocalName = "";
    cacheChildFolderLocalName << number;
    currentSubdir = cacheMasterFolderABSName + cacheChildFolderLocalName.GetCStr();
  }
  currentSubdir = currentSubdir + "\\";
  wxMkDir(currentSubdir);
  return currentSubdir;
}
//----------------------------------------------------------------------------
void lhpOpUploadMultiVMERefactor::FillMSFFileABSFolderIVar()
//----------------------------------------------------------------------------
{
  //logic comunicate the msf directory
  mafEvent event;
  event.SetSender(this);
  event.SetId(ID_MSF_DATA_CACHE);
  mafEventMacro(event);

  wxString temp;
  temp.Append((*event.GetString()).GetCStr());
  m_MSFFileABSFileName = temp;
  temp = temp.BeforeLast('/');
  m_MSFFileABSFolder = temp;
  assert(m_MSFFileABSFolder != "");
}

int lhpOpUploadMultiVMERefactor::FillRepositoryVar()
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolder.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  // 
  ifstream groupIdFile;
  groupIdFile.open("groupId.txt");
  if (!groupIdFile) {
    mafLogMessage("Unable to open file");
    return MAF_ERROR; // terminate with error
  }

  char buffer[200];

  groupIdFile.getline(buffer, 200);
  char *pch;
  wxArrayString resourceInfoList;
  pch = strtok(buffer, " ");
  m_Repository = mafString(pch);

  groupIdFile.close();

  wxSetWorkingDirectory(oldDir);

  return MAF_OK;
}