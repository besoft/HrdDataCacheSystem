/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpDownloadVMERefactor.cpp,v $
Language:  C++
Date:      $Date: 2011-05-30 14:47:44 $
Version:   $Revision: 1.1.1.1.2.38 $
Authors:   Daniele Giunchi, Stefano Perticoni, Roberto Mucci
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

#include "medDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpBuilderDecl.h"

#include <wx/dir.h>
#include <wx/busyinfo.h>
#include <wx/tokenzr.h>

#include "lhpOpDownloadVMERefactor.h"

#include "lhpUtils.h"
#include "mafZipUtility.h"
#include "mafGUI.h"
#include "lhpUser.h"
#include "mafNode.h"
#include "mafVMEGenericAbstract.h"
#include "mafOpImporterMSF.h"
#include "mafVMELandmarkCloud.h"
#include "medVMEWrappedMeter.h"
#include "mafTagArray.h"

#include "mafVMEItem.h"
#include "mafDataVector.h"
#include "mafVMEStorage.h"
#include "mafVMERoot.h"
#include "mafVMEGroup.h"
#include "mafVMESurface.h"

#include "lhpFactoryTagHandler.h"
#include "vtkPolyData.h"
#include "vtkDirectory.h"
#include "mafMatrixVector.h"
#include "mafAbsMatrixPipe.h"
#include "lhpVMEPoseGroup.h"
#include "mafNodeIterator.h"

#include "lhpUploadDownloadVMEHelper.h"
#include "mafVMEExternalData.h"

#include "vtkMatrix4x4.h"
#include "mafTransform.h"
#include "mafCrypt.h"

mafCxxTypeMacro(lhpOpDownloadVMERefactor);

mafString lhpOpDownloadVMERefactor::m_CacheSubdir = "0";

lhpOpDownloadVMERefactor::lhpOpDownloadVMERefactor(wxString label, int fromSandbox) :
mafOp(label)
{ 
  // setting m_UseMockResourceFile to true will bypass the web service call for retrieving
  // the download list
  m_UseMockResourceFile = false;

  m_MockResourcesToDownloadLocalFileName = "";
  mafString mockResourcesToDownloadLocalFileName = LHP_DATA_ROOT;
  
  // msf with 1 volume and 1 mesh
  // mockResourcesToDownloadLocalFileName.Append("/lhpOpDownloadVMERefactorTest/msfWith1SmallMeshAnd1SmallVolume/msfWith1SmallMeshAnd1SmallVolume_dataresource.txt");
  
  // msf with 15 different vmes
  mockResourcesToDownloadLocalFileName.Append("/lhpOpDownloadVMERefactorTest/msf_test_import_export_vme/msf_test_import_export_vme_Dataresource.txt");

  m_MockResourcesToDownloadLocalFileName = mockResourcesToDownloadLocalFileName;
  
  m_OpType  = OPTYPE_OP;
	m_Canundo = false;
  m_FillLinkVector = false;
  m_FullMsfDownload = false;
  m_DebugMode = false;
  m_DerivedVMEsVector.clear();
  m_VMELinksVector.clear();
  m_DownloadedResourcesList.clear();
  m_DownloadedVMEsVector.clear();
  m_TriedToDownloadResourcesList.clear();
  m_Group = NULL;
  m_DownloadedMSFGroup = NULL;
  m_User = NULL;
  m_DownloadCounter = 0;
  m_FromSandbox = fromSandbox;     
  
  m_UploadDownloadVMEHelper = new lhpUploadDownloadVMEHelper(this);

  m_PythonInterpreterFullPath = "python.exe_UNDEFINED";
  m_PythonwInterpreterFullPath = "pythonw.exe_UNDEFINED";  

  m_IncomingDirectoryABSPath = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\Incoming\\").c_str();


  m_VMEUploaderDownloaderDirABSPath  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  //m_ServiceURL = "https://www.biomedtown.org/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_ServiceURL = "http://devel.fec.cineca.dataVectorIterator:12680/town/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_ServiceURL = "http://62.149.243.50:8081/town/biomed_town/LHDL/users/repository/lhprepository2";
  m_ServiceURL = "";
  m_Repository = "";
  
  m_MSFDirectoryABSPath = "";
  m_IncomingCompletePath = "";
  
  m_ResourcesToDownloadLocalFileName = "ToDownload.txt" ;
  
  m_BinaryRealName = "";
  m_URISRBFile = "";
  m_URISRBFileSize = "0";
  m_ProxyURL = "";
  m_ProxyPort = "0";

  m_ExecutedLXMLChecker = 0;

  m_PassPhrase = "";

}

lhpOpDownloadVMERefactor::~lhpOpDownloadVMERefactor()
{
  m_DerivedVMEsVector.clear();
  m_VMELinksVector.clear();
  m_DownloadedResourcesList.clear();
  m_DownloadedVMEsVector.clear();
  m_TriedToDownloadResourcesList.clear();
  mafDEL(m_Group);
  mafDEL(m_DownloadedMSFGroup);
  cppDEL(m_UploadDownloadVMEHelper);
}

bool lhpOpDownloadVMERefactor::Accept(mafNode* vme)
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
    // It doesn't work from ior:
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
    wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());

    mafLogMessage( _T("Searching lxmlCheckerForAccept in '%s'"), m_VMEUploaderDownloaderDirABSPath.GetCStr()); // Debug stuff
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

mafOp* lhpOpDownloadVMERefactor::Copy()
{
	/** return a copy of itself, needs to put dataVectorIterator into the undo stack */
	return new lhpOpDownloadVMERefactor(m_Label, m_FromSandbox);
}

void lhpOpDownloadVMERefactor::OpRun()
{
//   m_ProcessCounter = 0;
//   m_StopsCounter = 0;

  m_PythonInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath();
  m_PythonwInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath();

  m_User = m_UploadDownloadVMEHelper->GetUser();
  m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  if(m_ServiceURL.Equals("1"))
  {
    wxMessageBox("Error in GetServiceURL(). Please check your internet connection and retry operation.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    OpStop(OP_RUN_CANCEL);
    return;
  }

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

  int result = OP_RUN_CANCEL;

  mafString DebugPath = m_VMEUploaderDownloaderDirABSPath;
  DebugPath.Append("\\Debug.py");
  if (wxFileExists(DebugPath.GetCStr()))
  {
    ifstream debugFile;
    debugFile.open(DebugPath.GetCStr());
    if (!debugFile) 
    {
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


  bool upToDate = false;
  upToDate = m_UploadDownloadVMEHelper->IsClientSoftwareVersionUpToDate();

  if(upToDate)
  { 
    mafEventMacro(mafEvent(this, MENU_FILE_SAVE));

    if (m_TestMode == true)
    {
      result = OP_RUN_OK;
      mafEventMacro(mafEvent(this,result));
    }
    else if (m_UseMockResourceFile)
    {
      result = OP_RUN_OK;
      mafEventMacro(mafEvent(this,result));
    }
    else if(this->SelectResourcesToDownloadByGUI() == MAF_OK)
    {
      result = OP_RUN_OK;
      mafEventMacro(mafEvent(this,result));
    }
    else
    {
      OpStop(result);
      return;
    }
  }
  else
  {
    OpStop(result);
    return;
  }

}

void lhpOpDownloadVMERefactor::OnEvent(mafEventBase *maf_event) 
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case wxOK:
      {
        this->OpStop(OP_RUN_OK);
        return;
      }
      break;

    case wxCANCEL:
      {
        this->OpStop(OP_RUN_CANCEL);
        return;
      }
      break;

    default:
      mafEventMacro(*e);
      break;
    }	
  }
}

void lhpOpDownloadVMERefactor::OpDo()   
{
  // restore status

  m_NumberOfVMEWithBinary = 0;
  // If exists delete the download stack file: (This file is used to check download integrity)
  wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  path += "DownloadCallStack.txt";
  if(wxFileExists(path))
  {
    wxRemoveFile(path);
  }
   path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\DownloadCallStack\\").c_str();
   if(wxDirExists(path))
   {
      // Don't work
      // bool result = false;
      // while(result==false) // this dir must be deleted!
      // {
      //   result = wxRmdir(path);
      // }
      // Remove all file in DownloadCallStack
      wxString f = wxFindFirstFile(_T(path + _T("*.*")));
      while ( !f.empty() )
      {
        wxRemoveFile(f);
        f = wxFindNextFile();
      }
   }
   else
   {
      wxMkdir(path);
   }
   
   if(wxFileExists(lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\lhpDownloadThreads.txt"))
   {
      wxRemoveFile(lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\lhpDownloadThreads.txt");
   }

  if (m_UseMockResourceFile)
  {
    m_ResourcesToDownloadLocalFileName = m_MockResourcesToDownloadLocalFileName;
  }

  if(Fill_m_ResourcesToDownloadList_IVar() != MAF_OK)
  {
    wxMessageBox("Unable to read what are the choosen vme");
    return;
  }

  // write on a file the number of vme that will be downloaded
  path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  path += "lhpCommunication.txt";

  //if file exists , delete it
  if (wxFileExists(path))
  {
    wxRemoveFile(path);
  }

  ofstream commFile;
  commFile.open(path);

  commFile << m_ResourcesToDownloadList.size() << "\n";

  commFile.close();
  //

  //wxInfoFrame *wait;
  wxWindowDisabler *disableAll;
  wxBusyCursor *wait_cursor;
  if(!m_TestMode)
  {
//     wait = new wxInfoFrame(NULL, "Initializing download process. Please wait.");
//     wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
//     wait->Show(true);
//     wait->Refresh();
//     wait->Update();
    disableAll = new wxWindowDisabler();
    wait_cursor = new wxBusyCursor();
  }

  //Download VME form the basket
  if (DownloadVMEs(m_ResourcesToDownloadList, m_Group) != MAF_OK)
  {
    if(!m_TestMode)
    {
      //delete wait;
      cppDEL(disableAll);
      cppDEL(wait_cursor);
    }
    return;
  }

  if (m_ListLinkURIInTree.size() != 0)
  {
    for (int i = 0; i < m_ListLinkURIInTree.size(); i++)
    {
      //Check if VME has been already downloaded
      for (int c = 0; c < m_DownloadedResourcesList.size(); c++)
      {
        mafString vmeName = m_ListLinkURIInTree[i].c_str();
        if (m_DownloadedResourcesList[c].Equals(vmeName))
        {
          //Fill links vector with nodes downloaded already
          m_VMELinksVector.push_back(m_DownloadedVMEsVector[c]);
        }
      }
    } 
  }


 if (m_ListLinkURI.size() != 0 || m_ListLinkURIInTree.size() != 0)
  {
    m_FillLinkVector = true;

    //Download VME linkResourceName
    if (DownloadVMEs(m_ListLinkURI, m_Group) != MAF_OK)
    {
      if(!m_TestMode)
      {
        //delete wait;
        cppDEL(disableAll);
        cppDEL(wait_cursor);
      }
      return;
    }
    int counter = 0;
    for (int n = 0; n < m_DerivedVMEsVector.size(); n++)
    {
      int subId = -1;
      mafString linkName;
      for (mafNode::mafLinksMap::iterator i = m_DerivedVMEsVector[n]->GetLinks()->begin(); i != m_DerivedVMEsVector[n]->GetLinks()->end(); i++)
      {
        linkName = i->first;
        if (m_VMELinksVector[counter]->IsA("mafVMELandmarkCloud") && m_VMELinksVector[counter]->GetNumberOfChildren() == 1)
        {
          //set subId to 0, because dataVectorIterator is the first landmark of the cloud
          subId = 0;
          m_VMELinksVector[counter] = m_VMELinksVector[counter]->GetFirstChild();
          
          if (m_DerivedVMEsVector[n]->IsA("medVMEWrappedMeter") && linkName == m_VMELinksVector[counter]->GetName())
          {
            if(m_VMELinksVector[counter] != NULL && m_VMELinksVector[counter]->GetParent() != NULL)
            {
              ((medVMEWrappedMeter *)m_DerivedVMEsVector[n])->SetMeterLink(linkName.GetCStr(), m_VMELinksVector[counter]); 
              ((medVMEWrappedMeter *)m_DerivedVMEsVector[n])->AddMidPoint(m_VMELinksVector[counter]->GetParent());
            }
          }
          else
          {
            if(m_VMELinksVector[counter]->GetParent() != NULL)
            {
              m_DerivedVMEsVector[n]->SetLink(linkName.GetCStr(), m_VMELinksVector[counter]->GetParent(), subId);
            }
          }
          counter++;
        }
        else
        {
         if (m_DerivedVMEsVector[n]->IsA("medVMEWrappedMeter") && linkName == m_VMELinksVector[counter]->GetName())
          {
            if(m_VMELinksVector[counter] != NULL)
            {
              ((medVMEWrappedMeter *)m_DerivedVMEsVector[n])->SetMeterLink(linkName.GetCStr(), m_VMELinksVector[counter]); 
              ((medVMEWrappedMeter *)m_DerivedVMEsVector[n])->AddMidPoint(m_VMELinksVector[counter]);
            }
          }
          else
          {
            if(m_VMELinksVector[counter])
            {
              m_DerivedVMEsVector[n]->SetLink(linkName.GetCStr(), m_VMELinksVector[counter]/*->GetParent()*/);
            }
          }
          counter++;
        }
      }
    }
  }
  //All VME successfuly downloaded!!
  DownloadSuccessful(true);

  if(!m_TestMode)
  {
    //delete wait;
    cppDEL(disableAll);
    cppDEL(wait_cursor);
  }

  WaitForDownloadCompletition(m_NumberOfVMEWithBinary);

  wxInfoFrame *wait;
  if(!m_TestMode)
  {
    wait = new wxInfoFrame(NULL, "Saving! Please wait...");
    wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
    wait->Show(true);
    wait->Refresh();
    wait->Update();
  }

  mafEventMacro(mafEvent(this, MENU_FILE_SAVE));

  delete wait;

}

int lhpOpDownloadVMERefactor::DownloadFullMSF(mafNode *node)   
{
  int result = MAF_ERROR;
  wxArrayString childrenVMEs = GetChildrenVMEsResources(node);

  int numChild = childrenVMEs.size();

  if (numChild == 0)
  {
    result = MAF_OK;
    return result;
  }

  if (node->IsA("mafVMERoot"))
  {
    node = m_DownloadedMSFGroup;
  }

  result = DownloadVMEs(childrenVMEs, node);
  return result;
}

int lhpOpDownloadVMERefactor::DownloadVMEs( wxArrayString vmeResourcesToDownloadList, mafNode *outputParentVME /*= NULL*/ )
{
  if(m_ServiceURL.Compare("")==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  mafString isLast = "false";

  for (int i = 0; i < vmeResourcesToDownloadList.size(); i++)
  {

    for (int c = 0; c < m_ResourcesToDownloadList.size(); c++)
    {
      if (m_ResourcesToDownloadList[c].CompareTo(vmeResourcesToDownloadList[i].c_str()) == 0)
        m_DownloadCounter +=1;
    }
      
      m_TriedToDownloadResourcesList.push_back(vmeResourcesToDownloadList[i].c_str()); 
      
      wxString oldDir = wxGetCwd();
      if (m_DebugMode)
        mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
      wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
      if (m_DebugMode)
        mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

      m_UploadDownloadVMEHelper->CreateThreadedUploaderDownloader();
      
      // DEBUG
      mafLogMessage("");
      std::ostringstream stringStream;
      stringStream << std::endl << "-----------------------------------" << std::endl;
      stringStream << "Downloading: "  << vmeResourcesToDownloadList[i].c_str() << std::endl;
      stringStream << "-----------------------------------" << std::endl;
      mafLogMessage("");

      mafLogMessage(stringStream.str().c_str());
            
      if(!CreateIncomingDirectory())
      {
        wxMessageBox("Unable to create Incoming Directory", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        DownloadSuccessful(false);
        return MAF_ERROR;
      }

      if(!CreateIncomingChildDirectory())
      {
        wxMessageBox("Unable to create a temporary cache, remember that msf must be saved locally", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        DownloadSuccessful(false);
        return MAF_ERROR;
      }
      
      if(DownloadXMLDataResource(vmeResourcesToDownloadList[i]) != MAF_OK)
      {
        DownloadSuccessful(false);
        return MAF_ERROR;
      }


      //reconstruct msf
      if(BuildMSFFromXMLResource(vmeResourcesToDownloadList[i]) != MAF_OK)
      {
        wxMessageBox("Unable to reconstruct msf", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        DownloadSuccessful(false);
        return MAF_ERROR;
      }
      wxBusyCursor *wait = NULL;

      if (m_TestMode == false)
      {
        wait = new wxBusyCursor();
      }
      
      if (m_URISRBFile.Equals("NOT PRESENT"))
      {
        m_URISRBFile = ".";
      }
            
      wxString command2execute;
      command2execute.clear();
      command2execute = m_PythonwInterpreterFullPath.GetCStr();
      mafString pythonScriptName = "Client.py ";
      command2execute.Append(pythonScriptName.GetCStr());
      command2execute.Append("127.0.0.1 "); //server address (localhost)
      command2execute.Append("50000 "); //port address (50000)
      command2execute.Append(wxString::Format("DOWNLOAD ")); //Download command
      command2execute.Append(wxString::Format("%s ",m_URISRBFileSize)); //file size

      //workaround to understanding directory argument
      wxString directoryWorkAround = m_IncomingCompletePath;
      directoryWorkAround.Replace(" ", "?");
      command2execute.Append(wxString::Format("%s ",directoryWorkAround)); //cache directory
      command2execute.Append(wxString::Format("%s ",m_User->GetName())); //user
      command2execute.Append(wxString::Format("%s ",m_User->GetPwd())); //pwd
      command2execute.Append(wxString::Format("%s ",m_ServiceURL.GetCStr())); //dev repository
      command2execute.Append(wxString::Format("%s ",vmeResourcesToDownloadList[i].c_str())); //XML URI NAME
      command2execute.Append(wxString::Format("%s ",m_URISRBFile.GetCStr())); //SRB DATA NAME

      //Code to understand if this is the last VME to download
      if (!m_FullMsfDownload )
      {
        bool check = false;
        if (m_ResourcesToDownloadList.size() == 1 && \
            !m_ResourcesToDownloadTypeList[i].CmpNoCase(wxString("mafVMERoot"))==0 && !m_FillLinkVector)
          
          check = true;

        else if (outputParentVME != NULL && outputParentVME->Equals(m_Group) && \
          (i+1 == vmeResourcesToDownloadList.size()))
          
          check = true;

        else if ((i+1 == vmeResourcesToDownloadList.size()) && m_ListLinkURI.IsEmpty() \
          && !m_ResourcesToDownloadTypeList[i].CmpNoCase(wxString("mafVMERoot"))==0)
          
          check = true;

        //wxArrayString URI = GetLinksList(vmeResourcesToDownloadList[i].c_str());
        wxArrayString URI = GetLinksListLocal();
        if (check && URI.IsEmpty())
          isLast = "true";

      }
      else if (outputParentVME != NULL && outputParentVME->Equals(m_DownloadedMSFGroup) && (i+1 == vmeResourcesToDownloadList.size()) && m_DownloadCounter  == m_ResourcesToDownloadList.size())
      {
        wxArrayString URI = GetLinksListLocal();
        
        if (URI.IsEmpty() && !HasChildLocal())
          isLast = "true";
        else
        {
          bool alreadyDownloaded = false;
          isLast = "true";
          //Check if VME has been already downloaded
          for (int i = 0 ; i < URI.size(); i++)
          {
            alreadyDownloaded = false;
            for (int c = 0; c < m_DownloadedResourcesList.size(); c++)
            {
              mafString VMEname = URI[i].c_str();
              if (m_DownloadedResourcesList[c].Equals(VMEname))
              {
                alreadyDownloaded = true;
                break;
              }
            }
            if (alreadyDownloaded = false)
            {
              isLast = "false";
              break;
            }
          }
        }

      }
      else if (m_FillLinkVector && (i+1 == vmeResourcesToDownloadList.size()))
      {
        //wxArrayString URI = GetLinksList(vmeResourcesToDownloadList[i].c_str());
        wxArrayString URI = GetLinksListLocal();
        if (URI.IsEmpty())
          isLast = "true";
        else
        {
          bool alreadyDownloaded = false;
          isLast = "true";
          //Check if VME has been already downloaded
          for (int i = 0 ; i < URI.size(); i++)
          {
            alreadyDownloaded = false;
            for (int c = 0; c < m_DownloadedResourcesList.size(); c++)
            {
              mafString VMEname = URI[i].c_str();
              if (m_DownloadedResourcesList[c].Equals(VMEname))
              {
                alreadyDownloaded = true;
                break;
              }
            }
            if (alreadyDownloaded = false)
            {
              isLast = "false";
              break;
            }
          }
        }
      }

      command2execute.Append(wxString::Format("%s",isLast.GetCStr())); //is last VME
      
      wxString command2executeWithoutPassword = command2execute;
      command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

      // DEBUG
      // if (m_DebugMode)
      mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
      long processID = wxExecute(command2execute, wxEXEC_ASYNC);

      wxSetWorkingDirectory(oldDir);
      m_DownloadedResourcesList.push_back(vmeResourcesToDownloadList[i].c_str());

      if(ImportMSF(outputParentVME) != MAF_OK)
      {
        wxMessageBox("Unable to import msf", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        DownloadSuccessful(false);
        return MAF_ERROR;;
      }
      cppDEL(wait);
  }

  ImposeAbsMatrixFromTag();
  return MAF_OK;
}

void lhpOpDownloadVMERefactor::OpStop(int result)   
{
  //HideGui();
	mafEventMacro(mafEvent(this,result));
}

bool lhpOpDownloadVMERefactor::CreateIncomingChildDirectory()
{
  bool result = true;

  if (m_TestMode == false)
  {
    //create cache: logic comunicate the msf directory
    mafEvent event;
    event.SetSender(this);
    event.SetId(ID_MSF_DATA_CACHE);
    mafEventMacro(event);

    wxString temp;
    temp.Append((*event.GetString()).GetCStr());
    temp = temp.BeforeLast('/');
    m_MSFDirectoryABSPath = temp;
  }
  else
  {
    assert(m_TestMode == true);
    assert(m_MSFDirectoryABSPath != "");
  }

  wxDir dir(m_MSFDirectoryABSPath.GetCStr());
  wxString exist = m_MSFDirectoryABSPath.GetCStr();
  if ( !wxDirExists(exist) || !dir.IsOpened())
  {
    // deal with the error here - wxDir would already log an error message
    // explaining the exact reason of the failure
    return false;
  }

  //control cache subdir
  mafString currentSubdir;
  currentSubdir = m_IncomingDirectoryABSPath + m_CacheSubdir.GetCStr();
  while(wxDirExists(currentSubdir))
  {
    int number = atoi(m_CacheSubdir.GetCStr());
    number += 1;
    m_CacheSubdir = "";
    m_CacheSubdir << number;
    currentSubdir = m_IncomingDirectoryABSPath + m_CacheSubdir.GetCStr();
  }
  currentSubdir = currentSubdir + "\\";

  // DEBUG
  std::ostringstream stringStream;
  stringStream << "Created Incoming Dir: " << currentSubdir.GetCStr() << std::endl;
  mafLogMessage(stringStream.str().c_str());
        
  wxMkDir(currentSubdir);
  m_IncomingCompletePath = currentSubdir;

  return result;
  
}


bool lhpOpDownloadVMERefactor::CreateIncomingDirectory()
{
  bool resultIncoming = false;
  
  wxString existIncoming = m_IncomingDirectoryABSPath.GetCStr();
  if ( wxDirExists(existIncoming) )
  {
    resultIncoming = true;
  }
  else
  {
    wxMkDir(existIncoming);
    if ( wxDirExists(existIncoming) ) resultIncoming = true;
  }
  return resultIncoming;
}

int lhpOpDownloadVMERefactor::SelectResourcesToDownloadByGUI()
{
  if(m_FromSandbox)
    FillRepositoryVar();
  else m_Repository = m_User->GetName();

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  // gui for selecting vme
  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonwInterpreterFullPath.GetCStr();

  command2execute.Append(" downloadSelectorApp.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(wxString::Format("%d",m_FromSandbox));

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
  
  mafLogMessage("Waiting for remote repository to return resources list......");

  wxBusyInfo *wait;
  if(!m_TestMode)
  {
    wait = new wxBusyInfo("Please wait for your data resource listing...");
  }

  long pid = wxExecute(command2execute, wxEXEC_SYNC);

  if(!m_TestMode)
  { 
    delete wait;
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  
  mafLogMessage("done");

  return MAF_OK;
}

int lhpOpDownloadVMERefactor::Fill_m_ResourcesToDownloadList_IVar()
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  // open auto tags file and try to handle tags using tags factory 
  ifstream resourcesToDownloadFile;

  resourcesToDownloadFile.open(m_ResourcesToDownloadLocalFileName.GetCStr());
  if (!resourcesToDownloadFile) {
    mafLogMessage("Unable to open file");
    return MAF_ERROR; // terminate with error
  }

  char buffer[200];

  while (resourcesToDownloadFile.getline(buffer, 200))
  {

    char *pch;
    wxArrayString resourceInfoList;

    pch = strtok(buffer, " ");

    for(int i=0; i<4 && pch;i++)
    {
      resourceInfoList.Add(pch);
      pch = strtok(NULL, " ");
    }
    m_ResourcesToDownloadList.Add(resourceInfoList[0]);
    m_ResourcesToDownloadParentList.Add(resourceInfoList[1]);
    m_ResourcesToDownloadTypeList.Add(resourceInfoList[2]);
    m_ResourcesToDownloadHasChildList.Add(resourceInfoList[3]);

  }
  resourcesToDownloadFile.close();

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  return MAF_OK;
}

int lhpOpDownloadVMERefactor::DownloadXMLDataResource( mafString xmlDataResourceName )
{


//  mafSleep(1000);

  // workaround to avoid memory saturation
  // Every 50 download process started wait 10 seconds

//   if(m_ProcessCounter > 50)
//   {
//     m_StopsCounter++;
//     if(m_DebugMode)
//     {
//       mafLogMessage(wxString::Format(">>>>>>> Waiting %d seconds to avoid memory saturation... <<<<<<<<<",pow(2,m_StopsCounter) * 10).c_str());
//     }
//     //mafSleep(10000 * pow(2,m_StopsCounter));
//     m_ProcessCounter = 0;
//   }

  // workaround to avoid memory saturation
  int numberOfThreads = 0;
  do
  {
    wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
    path += "lhpDownloadThreads.txt";

    //This file must exist
    if (wxFileExists(path))

    {
      std::ifstream commFile(path,std::ios::in);
      std::string strNumberOfThreads;
      commFile >> strNumberOfThreads;
      numberOfThreads = atoi(strNumberOfThreads.c_str());
      commFile.close();
      if(numberOfThreads > 20)
      {
        mafSleep(15000);
      }
    }
    else
    {
      numberOfThreads = 0;
    }
  }
  while(numberOfThreads > 20);

  if(m_ServiceURL.Compare("")==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonInterpreterFullPath.GetCStr();
  command2execute.Append(" downloadSingleXML.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(xmlDataResourceName.GetCStr());
  command2execute.Append(" ");

  wxString directoryWorkAround = m_IncomingCompletePath;
  directoryWorkAround.Replace(" ", "?");
  command2execute.Append(directoryWorkAround);

  wxArrayString output;
  wxArrayString errors;

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  // DEBUG
  std::ostringstream stringStream;
  mafLogMessage("");
  stringStream << command2executeWithoutPassword.c_str() << std::endl;
  mafLogMessage(stringStream.str().c_str());
  mafLogMessage("Downloading XML resource file from repository, please Wait......");
  mafLogMessage("");

  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
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

    wxMessageBox(wxString::Format("Error in downloadSingleXML.py ! Check your internet connection '%s'.\nMSF download stopped.",xmlDataResourceName.GetCStr()), wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
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

  if(output.size() < 2)
  {
    return MAF_ERROR;
  }

  m_URISRBFileSize = output[output.size() - 1];
  m_URISRBFile = output[output.size() - 2];
  if (output.size() > 3)
  {
  	m_PassPhrase = output[output.size() - 4];
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  return MAF_OK;
}

int lhpOpDownloadVMERefactor::BuildMSFFromXMLResource( mafString incomingXMLResourceLocalFileName )
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonwInterpreterFullPath.GetCStr();
  command2execute.Append(" msfReconstructor.py ");

  wxString incomingDirABSPathWorkaround = m_IncomingCompletePath;
  incomingDirABSPathWorkaround.Replace(" ", "?");

  command2execute.Append(incomingDirABSPathWorkaround);
  command2execute.Append(" ");
  command2execute.Append(incomingXMLResourceLocalFileName);
  command2execute.Append(" ");

  wxString targetDirABSPathWorkaround = m_MSFDirectoryABSPath;
  targetDirABSPathWorkaround.Append("/");
  targetDirABSPathWorkaround.Replace(" ", "?");

  command2execute.Append(targetDirABSPathWorkaround);
  command2execute.Append(" ");
  
  // DEBUG
  // if (m_DebugMode)
  mafLogMessage("");
  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
  mafLogMessage("");

  long pid = wxExecute(command2execute, wxEXEC_SYNC);
  
  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  return MAF_OK;
}

wxArrayString lhpOpDownloadVMERefactor::GetChildrenVMEsResources( mafNode *parentVME )
{
  int result = MAF_ERROR;
  wxString name;
  int count;
  wxArrayString listChildURI;
  listChildURI.clear();
  mafTagItem *tagChild = parentVME->GetTagArray()->GetTag("L0000_resource_MAF_TreeInfo_VmeChildURI1");

  if (tagChild == NULL)
    return listChildURI;

  std::string listChild = tagChild->GetValue();

  if (listChild.rfind("dataresource") != std::string::npos)
  {
    while (listChild.find_first_of(' ') != -1)
    {
      count = listChild.find_first_of(' ');
      name = (listChild.substr(0, count)).c_str();
      if (!name.IsEmpty())
      {
        name.Trim(false);
        name.Trim();
        listChildURI.Add(name);
      }
      listChild.erase(0, count+1);
    }
  }
  // write on a file the number of vme that will be downloaded
  wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
  path += "lhpCommunication.txt";

  //if file exists , delete it
//   if (wxFileExists(path))
//     wxRemoveFile(path);

  ifstream commFileIn;
  commFileIn.open(path);


  std::string strNumOfDownload;
  int numOfDownload;

  commFileIn >> strNumOfDownload;

  numOfDownload = atoi(strNumOfDownload.c_str());

  commFileIn.close();

  ofstream commFile;
  commFile.open(path);

  commFile << (numOfDownload + listChildURI.size()) << "\n";

  commFile.close();
  //

  return listChildURI;
}

void lhpOpDownloadVMERefactor::GetLinkedVMEsResources( mafNode *masterVME )
{
  wxString linkResourceName;
  int count;

  mafTagItem *linksTagItem = masterVME->GetTagArray()->GetTag("L0000_resource_MAF_Procedural_VMElinkURI1");
  
  if (linksTagItem == NULL)
    return;

  std::string linksList = linksTagItem->GetValue();

  if (linksList.rfind("dataresource") != std::string::npos)
  {
    while (linksList.find_first_of(' ') != -1)
    {
      count = linksList.find_first_of(' ');
      linkResourceName = (linksList.substr(0, count)).c_str();
  
      if (!linkResourceName.IsEmpty())
      {
        linkResourceName.Trim(false);
        linkResourceName.Trim();

        if (m_FullMsfDownload)
          m_ListLinkURIInTree.Add(linkResourceName);

        else
        
          m_ListLinkURI.Add(linkResourceName);
      }

      linksList.erase(0, count+1);
    }

    if (!linksList.empty())
    {
      if (linksList.rfind("dataresource") != std::string::npos)
      {
        if (m_FullMsfDownload)
          m_ListLinkURIInTree.Add(linksList.c_str());
        else
          m_ListLinkURI.Add(linksList.c_str());
      }
    }
  }
}

wxArrayString lhpOpDownloadVMERefactor::GetLinksList( mafString xmlRemoteResource )
{
  if(m_ServiceURL.Compare("")==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  wxString oldDir = wxGetCwd();
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
  mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonInterpreterFullPath.GetCStr();

  command2execute.Append("lhpReadRemoteTag.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(xmlRemoteResource.GetCStr());
  command2execute.Append(",");
  command2execute.Append("L0000_resource_MAF_Procedural_VMElinkURI1");

  if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  wxArrayString output;
  wxArrayString errors;
  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in lhpReadRemoteTag.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    if (m_DebugMode)
      mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
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

  wxString name;
  int count;
  wxArrayString URIArray;
  wxString linkURI= output[output.size() - 1];
  if (linkURI.rfind("dataresource") != std::string::npos)
  {
    while (linkURI.find_first_of(' ') != -1)
    {
      count = linkURI.find_first_of(' ');
      name = (linkURI.substr(0, count)).c_str();
      if (!name.IsEmpty())
      {
        name.Trim(false);
        name.Trim();
        URIArray.Add(name);
      }
      linkURI.erase(0, count+1);
    }

    if (!linkURI.empty())
    {
      if (linkURI.rfind("dataresource") != std::string::npos)
      {
        URIArray.Add(linkURI.c_str());
      }
    }
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  return URIArray;
}

wxArrayString lhpOpDownloadVMERefactor::GetLinksListLocal()
{
  wxArrayString linksListResult;

  mafString reconstructedMSFLocalFileNameInCacheDir = "outputMAF.msf";

  mafString reconstructedMSFABSFileName;
  reconstructedMSFABSFileName.Append(m_IncomingCompletePath);
  reconstructedMSFABSFileName.Append(reconstructedMSFLocalFileNameInCacheDir);
  reconstructedMSFABSFileName.ParsePathName();

  if (!wxFileExists(reconstructedMSFABSFileName.GetCStr()))
  {
    return MAF_ERROR;
  }

  mafVMEStorage *reconstructedMSFStorage;
  reconstructedMSFStorage = mafVMEStorage::New();
  reconstructedMSFStorage->SetURL(reconstructedMSFABSFileName.GetCStr());

  mafVMERoot *reconstructedMSFRoot;
  reconstructedMSFRoot = reconstructedMSFStorage->GetRoot();
  reconstructedMSFRoot->Initialize();
  reconstructedMSFRoot->SetName("RootB");

  int res = reconstructedMSFStorage->Restore();
  if (res != MAF_OK)
  {
    std::ostringstream stringStream;
    stringStream << "Error during " << reconstructedMSFABSFileName.GetCStr() << 
      " loading! Exiting with MAF_ERROR"  << std::endl;
    mafLogMessage(stringStream.str().c_str());

    return MAF_ERROR;
  }

  mafNode *downloadedVME = NULL;

  downloadedVME = reconstructedMSFRoot->GetFirstChild();

  if (downloadedVME == NULL)
  {
    return linksListResult;
  }

  if (downloadedVME->GetNumberOfLinks() != 0)
  {
    wxString linkResourceName;
    int count;

    mafTagItem *linksTagItem = downloadedVME->GetTagArray()->GetTag("L0000_resource_MAF_Procedural_VMElinkURI1");

    if (linksTagItem == NULL)
      return linksListResult;

    std::string linksList = linksTagItem->GetValue();

    if (linksList.rfind("dataresource") != std::string::npos)
    {
      while (linksList.find_first_of(' ') != -1)
      {
        count = linksList.find_first_of(' ');
        linkResourceName = (linksList.substr(0, count)).c_str();

        if (!linkResourceName.IsEmpty())
        {
          linkResourceName.Trim(false);
          linkResourceName.Trim();

          linksListResult.Add(linkResourceName);
        }

        linksList.erase(0, count+1);
      }

      if (!linksList.empty())
      {
        if (linksList.rfind("dataresource") != std::string::npos)
        {
          linksListResult.Add(linksList.c_str());
        }
      }
    }

    return linksListResult;
  }
  return MAF_OK;
}



bool lhpOpDownloadVMERefactor::HasChild( mafString xmlRemoteResource )
{
  if(m_ServiceURL.Compare("") ==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  bool containChild = false;

  wxString oldDir = wxGetCwd();
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonInterpreterFullPath.GetCStr();

  command2execute.Append("lhpReadRemoteTag.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(xmlRemoteResource.GetCStr());
  command2execute.Append(",");
  command2execute.Append("L0000_resource_MAF_TreeInfo_VmeChildURI1");

  if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  wxArrayString output;
  wxArrayString errors;
  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in lhpReadRemoteTag.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    if (m_DebugMode)
      mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
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

  wxString child= output[output.size() - 1];
  if (child.Contains("dataresource"))
    containChild = true;

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  return containChild;
}

bool lhpOpDownloadVMERefactor::HasChildLocal()
{
  mafString reconstructedMSFLocalFileNameInCacheDir = "outputMAF.msf";

  mafString reconstructedMSFABSFileName;
  reconstructedMSFABSFileName.Append(m_IncomingCompletePath);
  reconstructedMSFABSFileName.Append(reconstructedMSFLocalFileNameInCacheDir);
  reconstructedMSFABSFileName.ParsePathName();

  if (!wxFileExists(reconstructedMSFABSFileName.GetCStr()))
  {
    return MAF_ERROR;
  }

  mafVMEStorage *reconstructedMSFStorage;
  reconstructedMSFStorage = mafVMEStorage::New();
  reconstructedMSFStorage->SetURL(reconstructedMSFABSFileName.GetCStr());

  mafVMERoot *reconstructedMSFRoot;
  reconstructedMSFRoot = reconstructedMSFStorage->GetRoot();
  reconstructedMSFRoot->Initialize();
  reconstructedMSFRoot->SetName("RootB");

  int res = reconstructedMSFStorage->Restore();
  if (res != MAF_OK)
  {
    std::ostringstream stringStream;
    stringStream << "Error during " << reconstructedMSFABSFileName.GetCStr() << 
      " loading! Exiting with MAF_ERROR"  << std::endl;
    mafLogMessage(stringStream.str().c_str());

    return MAF_ERROR;
  }

  mafNode *downloadedVME = NULL;

  downloadedVME = reconstructedMSFRoot->GetFirstChild();

  if (downloadedVME == NULL)
  {
    //if reconstructedMSFRoot has no child, node downloaded is a MSFRoot
    return false;
  }

  return true;
}

bool lhpOpDownloadVMERefactor::IsRoot( mafString xmlRemoteResource )
{
  if(m_ServiceURL.Compare("")==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  bool isRoot = false;
  wxString oldDir = wxGetCwd();
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonInterpreterFullPath.GetCStr();

  command2execute.Append("lhpReadRemoteTag.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(xmlRemoteResource.GetCStr());
  command2execute.Append(",");
  command2execute.Append("L0000_resource_MAF_VmeType");

  if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  wxArrayString output;
  wxArrayString errors;
  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in lhpReadRemoteTag.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    if (m_DebugMode)
      mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
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

  wxString child= output[output.size() - 1];
  if (child.CompareTo("mafVMERoot") == 0)
    isRoot = true;


  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  return isRoot;
}

//----------------------------------------------------------------------------
int lhpOpDownloadVMERefactor::ImportMSF(mafNode *parentNode)
//----------------------------------------------------------------------------
{
  mafString reconstructedMSFLocalFileNameInCacheDir = "outputMAF.msf";

  mafString reconstructedMSFABSFileName;
  reconstructedMSFABSFileName.Append(m_IncomingCompletePath);
  reconstructedMSFABSFileName.Append(reconstructedMSFLocalFileNameInCacheDir);
  reconstructedMSFABSFileName.ParsePathName();

  if (!wxFileExists(reconstructedMSFABSFileName.GetCStr()))
  {
    return MAF_ERROR;
  }

  mafVMEStorage *reconstructedMSFStorage;
  reconstructedMSFStorage = mafVMEStorage::New();
  reconstructedMSFStorage->SetURL(reconstructedMSFABSFileName.GetCStr());

  mafVMERoot *reconstructedMSFRoot;
  reconstructedMSFRoot = reconstructedMSFStorage->GetRoot();
  reconstructedMSFRoot->Initialize();
  reconstructedMSFRoot->SetName("RootB");

  int res = reconstructedMSFStorage->Restore();
  if (res != MAF_OK)
  {
    std::ostringstream stringStream;
    stringStream << "Error during " << reconstructedMSFABSFileName.GetCStr() << 
    " loading! Exiting with MAF_ERROR"  << std::endl;
    mafLogMessage(stringStream.str().c_str());
          
    return MAF_ERROR;
  }

  mafNode *downloadedVME = NULL;

  downloadedVME = reconstructedMSFRoot->GetFirstChild();
  
  if (downloadedVME == NULL)
  {
    //if reconstructedMSFRoot has no child, node downloaded is a MSFRoot
    m_FullMsfDownload = true;
    mafNEW(m_DownloadedMSFGroup);
    
    //copy tags from Root downloaded to new group.
    m_DownloadedMSFGroup->GetTagArray()->DeepCopy(reconstructedMSFRoot->GetTagArray());
    mafString label = "MSF from repository: ";
    label.Append(reconstructedMSFRoot->GetName());
    m_DownloadedMSFGroup->SetName(label.GetCStr());
    m_DownloadedMSFGroup->ReparentTo(m_Input);
    m_DownloadedVMEsVector.push_back(m_DownloadedMSFGroup);
    
    if (DownloadFullMSF(reconstructedMSFRoot) != MAF_OK)
    {
      return MAF_ERROR;
    }

    m_FullMsfDownload = false;
    mafDEL(reconstructedMSFStorage);
    return MAF_OK;
  }

  m_DownloadedVMEsVector.push_back(downloadedVME);

  if (downloadedVME->GetNumberOfLinks() != 0)
  {
    if (!m_FullMsfDownload)
      wxMessageBox(wxString::Format("Link found! VME linkResourceName will be downloaded"), wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    GetLinkedVMEsResources(downloadedVME);
    m_DerivedVMEsVector.push_back(downloadedVME);  
  }

  if (m_FillLinkVector)
  {
    m_VMELinksVector.push_back(downloadedVME);
  }

  if (m_Group == NULL && !m_FullMsfDownload)
  {
    mafNEW(m_Group);
    m_Group->SetName("Downloaded from repository");
    m_Group->ReparentTo(m_Input);
  }
   
  if (parentNode != NULL)
  {
    //////////////////////////////////////////////////////////////////////////
    //To optimize
    //////////////////////////////////////////////////////////////////////////
    std::vector<mafString> filesCreated;
    if (mafVMEGenericAbstract::SafeDownCast(downloadedVME) && mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector() && mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->GetArchiveName() != "")//If is a multi time VME
    {

      mafStorage *storage = mafVMERoot::SafeDownCast(downloadedVME->GetRoot())->GetStorage();
      mafString base_url = storage->GetURL();
      mafString tmp = wxPathOnly(base_url.GetCStr());
      tmp<<"/";
      tmp<<mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->GetArchiveName();
      while (!wxFileExists(tmp.GetCStr()));
      filesCreated = ZIPOpen(tmp);

      //mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->SetSingleFileMode(false);
      mafDataVector::DataMap::iterator it;
      mafVMEItem *item = NULL;
      it = mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->Begin();
      for (it = mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->Begin(); it != mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->End(); it++)
      {
        item = it->second;
        item->SetIOModeToDefault();
      }
    }
    // Loop on links and eliminate null ones
    // Upload operation if links are not present will fill links attributes to null: so for compatibility with already uploaded data null links will be deleted here
    mafNode::mafLinksMap::iterator lnk_it;
    std::vector<mafString> lnk_toBeRemoved;
    for (lnk_it = downloadedVME->GetLinks()->begin(); lnk_it != downloadedVME->GetLinks()->end(); lnk_it++)
    {
      if (lnk_it->second.m_Node == NULL)
      {
        lnk_toBeRemoved.push_back(lnk_it->first);
      }
    }
    for(int i = 0; i < lnk_toBeRemoved.size(); i++)
    {
      downloadedVME->RemoveLink(lnk_toBeRemoved.at(i));
    }
    //////////////////////////////////////////////////////////////////////////
    downloadedVME->ReparentTo(parentNode);
    //////////////////////////////////////////////////////////////////////////
    //To optimize - removing tmp files
    //////////////////////////////////////////////////////////////////////////
    if (mafVMEGenericAbstract::SafeDownCast(downloadedVME) && mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector() && mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->GetArchiveName() != "")//If is a multi time VME
    {
      mafDataVector::DataMap::iterator it;
      mafVMEItem *item = NULL;
      it = mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->Begin();
      for (it = mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->Begin(); it != mafVMEGenericAbstract::SafeDownCast(downloadedVME)->GetDataVector()->End(); it++)
      {
        item = it->second;
        item->UpdateData();
        item->SetIOModeToMemory();
      }

      mafLogMessage("<<<<<<<<<Removing temp data");
      for (int i=0;i<filesCreated.size();i++)
      {
        remove(filesCreated[i]);
      }

    }
    //////////////////////////////////////////////////////////////////////////    

    if (((mafVME *)downloadedVME)->IsAnimated())
    {
      mafVMEGenericAbstract *vme = mafVMEGenericAbstract::SafeDownCast(downloadedVME);
      if(vme)
      {
        if(vme->GetDataVector()->GetNumberOfItems() > 1)
        {
          CopyDownloadedAnimatedVMEDataInTargetMSFDir(downloadedVME);
        }
        else
        {
          CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
        }
      }
      else
      {
        CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
      }
    }
    else
    {
      CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
    }

    if (DownloadFullMSF(downloadedVME) != MAF_OK)
    {
      return MAF_ERROR;
    }
  }
  else
  {
    downloadedVME->ReparentTo(m_Group);

    if (((mafVME *)downloadedVME)->IsAnimated())
    {
      mafVMEGenericAbstract *vme = mafVMEGenericAbstract::SafeDownCast(downloadedVME);
      if(vme)
      {
        if(vme->GetDataVector()->GetNumberOfItems() > 1)
        {
          CopyDownloadedAnimatedVMEDataInTargetMSFDir(downloadedVME);
        }
        else
        {
          CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
        }
      }
      else
      {
        CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
      }
    }
    else
    {
      CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(downloadedVME);
    }
  }

  mafDEL(reconstructedMSFStorage);
  return MAF_OK;
}
//----------------------------------------------------------------------------
void lhpOpDownloadVMERefactor::ImposeAbsMatrixFromTag()
//----------------------------------------------------------------------------
{
  // Here start the method to impose always the same position the the vme
  // both if it's downloaded separately or with its parents

  mafNodeIterator *it;
  if(m_DownloadedMSFGroup) // for entire msf
  {
     it = m_DownloadedMSFGroup->NewIterator();
     ImposeAbsMatrixFromTagInternal(it);
     mafDEL(it);
  }
  
  if(m_Group) // for singles vmes
  {
    it = m_Group->NewIterator();
    ImposeAbsMatrixFromTagInternal(it);
    mafDEL(it);
  }
  
}
//---------------------------------------------------------------------------- 
void lhpOpDownloadVMERefactor::ImposeAbsMatrixFromTagInternal(mafNodeIterator *it)
//----------------------------------------------------------------------------
{

  it->SetTraversalModeToPreOrder();

  for(mafNode* downloadedVME = it->GetFirstNode(); downloadedVME; downloadedVME=it->GetNextNode())
  {

    lhpVMEPoseGroup *workaroundForNotVmeGeneric = NULL;

    if(downloadedVME && !downloadedVME->IsA("mafVMEGroup") && !downloadedVME->IsA("mafVMERoot") && !downloadedVME->IsA("lhpVMEPoseGroup") && !downloadedVME->IsA("mafVMESlicer"))
    {

      bool olderVersion = false;

      // -------------------------------------
      // first get the TimeStamps vector:

      // Tag
      // get the TimeStamps vector from L0000_resource_MAF_TimeSpace_TimeStampVector tag

      // This is used to know number and times of the matrices in the Matrix Vector

      mafTagItem *itemTimeStampsVectorFromTag = downloadedVME->GetTagArray()->GetTag("L0000_resource_MAF_TimeSpace_TimeStampVector"); // Item
      mafString strTimeStampsVectorFromTag = mafString(itemTimeStampsVectorFromTag->GetValue()); // String

      std::vector<mafTimeStamp> timeStampVectorFromTag; // Vector

      // parse the string to obtain a vector of double
      mafString notParsed;
      double timestamp;
      notParsed = mafString(strTimeStampsVectorFromTag.Duplicate());
      for(int c = 0; notParsed.Length()!=0 ; c++)
      {
        int spacePos = notParsed.FindFirst(" ");
        std::string matStr = notParsed.GetCStr();

        if(spacePos != -1) 
        {
          timestamp = atof((matStr.substr(0,spacePos)).c_str());
          notParsed = mafString(matStr.substr(spacePos+1,matStr.length()-spacePos).c_str());
        }
        else // no spaces found end of the vector!
        {
          timestamp = atof(matStr.c_str());
          notParsed = "";
        }
        timeStampVectorFromTag.push_back(timestamp);
      }

      // vme
      // get the timestamps vector from vme:

      // It's really needed?
      std::vector<mafTimeStamp> timeStampVectorFromVme;
      mafVME::SafeDownCast(downloadedVME)->GetAbsTimeStamps(timeStampVectorFromVme);

      // -------------------------------------

      // then get the matrix vector!!!

      // "L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose"

      mafTagItem *itemAbsMatrixFromTag = downloadedVME->GetTagArray()->GetTag("L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose"); // Get the absolute matrix from tags
      mafString strAbsMatrixFromTag = mafString(itemAbsMatrixFromTag->GetValue());
      notParsed = mafString(strAbsMatrixFromTag.Duplicate());

      std::vector<mafMatrix*> absMatrixVectorFromTag;

      wxStringTokenizer tkz(strAbsMatrixFromTag.GetCStr(),wxT(' '),wxTOKEN_RET_EMPTY_ALL);

      // For all timestamps!
      for(int i = 0; i < timeStampVectorFromTag.size(); i++)
      {

        // parsing for a single matrix
        // Parse the string to obtain a mafMatrix
        // If this tag is not specified the identity matrix is used

        vtkMatrix4x4 *identityMatrix;
        vtkNEW(identityMatrix);
        identityMatrix->Identity();

        mafMatrix *absMatrixFromTag = new mafMatrix(identityMatrix);

        // parse the string to obtain a vector of double
        double elements[16];

        if (!tkz.HasMoreTokens())
        {
          // Manage this error (not enough matrix!)
          // For example old uploaded vme!
          // For msf/vme uploaded with older version of the psl applications
          mafLogMessage(wxString::Format("\nError in %s download: Cannot restore parent matrix vector!\nProbably this is a vme that has been uploaded in a older version of this application (not enough matrix).\nPose matrix for this vme is not guaranteed for all timestamps!",downloadedVME->GetName()).c_str());
          olderVersion = true;
          break;
        }
        
//         if (notParsed.Length() == 0)
//         {
//           // Manage this error (not enough matrix!)
//           // For example old uploaded vme!
//           // For msf/vme uploaded with older version of the psl applications
//           mafLogMessage(wxString::Format("\nError in %s download: Cannot restore parent matrix vector!\nProbably this is a vme that has been uploaded in a older version of this application (not enough matrix).\nPose matrix for this vme is not guaranteed for all timestamps!",downloadedVME->GetName()).c_str());
//           olderVersion = true;
//           break;
//         }

        for(int c = 0;c < 16; c++)
        {
          if (!tkz.HasMoreTokens())
          {
            // Manage this error (not enough elements!)
            // This will happen? (a corrupted matrix inside the tag)
            mafLogMessage(wxString::Format("\nError in %s download: Cannot restore parent matrix vector!\nAbsolute matrix tag is corrupted (not enough elements).\nPose matrix for this vme is not guaranteed for all timestamps!",downloadedVME->GetName()).c_str());
            olderVersion = true;
            break;
          }
          elements[c] = atof(tkz.GetNextToken().c_str());
        }

//         for(int c = 0; (c < 16 && (notParsed.Length() != 0)); c++)
//         {
//           if (notParsed.Length() == 0)
//           {
//             // Manage this error (not enough elements!)
//             // This will happen? (a corrupted matrix inside the tag)
//             mafLogMessage(wxString::Format("\nError in %s download: Cannot restore parent matrix vector!\nAbsolute matrix tag is corrupted (not enough elements).\nPose matrix for this vme is not guaranteed for all timestamps!",downloadedVME->GetName()).c_str());
//             olderVersion = true;
//             break;
//           }
//           int spacePos = notParsed.FindFirst(" ");
//           std::string matStr = notParsed.GetCStr();
// 
//           if(spacePos != -1) 
//           {
//             elements[c] = atof((matStr.substr(0,spacePos)).c_str());
//             notParsed = mafString(matStr.substr(spacePos+1,matStr.length()-spacePos).c_str());
//           }
//           else // no spaces found end of the vector!
//           {
//             elements[c] = atof(matStr.c_str());
//             notParsed = "";
//           }
//         }

        if(olderVersion == true)
        {
          break;
        }

        // read the abs matrix from tag
        for(int z = 0; z < 4; z++)
        {
          for(int j = 0; j < 4; j++)
          {
            absMatrixFromTag->SetElement(z,j,elements[(z * 4) + j]);
          }
        }
        absMatrixFromTag->SetTimeStamp(timeStampVectorFromTag.at(i));
        absMatrixVectorFromTag.push_back(absMatrixFromTag);
        // parsing for a single matrix
      }

      // Here the same put a mechanism that compares all the absolute matrix vector!!! ToDo

      if(!olderVersion)
      {
        for(int i = 0; i < timeStampVectorFromTag.size(); i++)
        {
          mafMatrix absMatrix;

          mafVME::SafeDownCast(downloadedVME)->Update();
          mafVME::SafeDownCast(downloadedVME)->GetOutput()->GetAbsMatrix(absMatrix,(timeStampVectorFromTag.at(i)));
          mafMatrix *absMatrixFromTag = absMatrixVectorFromTag.at(i);
          // if the current absolute matrix is different from the one specified in the tag
          if(!absMatrixFromTag->Equals(&absMatrix))
          {

            // the method do not calculate a transform but directly set the absolute matrix!

            // WORKAROUND for parametric surface (and other vme that are not inherited from mafVMEGEneric):
            // Currently the parametric surface is not time-varying!!!
            // So for this type of vme we create a time-varying group and then we re-parent the parametric surface to it!

            mafVMEGeneric *vme = mafVMEGeneric::SafeDownCast(downloadedVME);
            
            if(!vme)
            {
              if(!workaroundForNotVmeGeneric)
              {
                mafNEW(workaroundForNotVmeGeneric);
                workaroundForNotVmeGeneric->SetName(mafString(downloadedVME->GetName()) + "_Pose");
                mafEventMacro(mafEvent(this,VME_ADD,workaroundForNotVmeGeneric));
                workaroundForNotVmeGeneric->ReparentTo(downloadedVME->GetParent());
                downloadedVME->ReparentTo(workaroundForNotVmeGeneric);

                mafMatrix mafMat;
                mafMat.DeepCopy(mafVME::SafeDownCast(downloadedVME)->GetOutput()->GetMatrix());
                mafMat.SetTimeStamp(0);
                workaroundForNotVmeGeneric->GetMatrixVector()->SetMatrix(mafMat);

                vtkMatrix4x4 *identityMatrix;
                vtkNEW(identityMatrix);
                identityMatrix->Identity();
                mafMatrix mafIdentityMatrix;
                mafIdentityMatrix.SetVTKMatrix(identityMatrix);
                mafIdentityMatrix.SetTimeStamp(0);
                mafVME::SafeDownCast(downloadedVME)->SetMatrix(mafIdentityMatrix);
                downloadedVME = it->GetFirstNode();
              }

              workaroundForNotVmeGeneric->SetAbsMatrix(*absMatrixFromTag); // do not use the transform!!!
              workaroundForNotVmeGeneric->GetAbsMatrixPipe()->Update();
              workaroundForNotVmeGeneric->Update();
            }
            else
            {
              vme->SetAbsMatrix(*absMatrixFromTag); // do not use the transform!!!
              vme->GetAbsMatrixPipe()->Update();
              vme->GetMatrixPipe()->Update();
              vme->Update();
            }
          }
        }
      }
    }
  }
}
//----------------------------------------------------------------------------
void lhpOpDownloadVMERefactor::CopyDownloadedNonAnimatedVMEDataInTargetMSFDir( mafNode *vmeAttachedAlreadyToTargetTree )
//----------------------------------------------------------------------------
{
 
  if (vmeAttachedAlreadyToTargetTree->IsA("mafVMEGeneric") ||
      vmeAttachedAlreadyToTargetTree->IsA("medVMEAnalog")  ||
      vmeAttachedAlreadyToTargetTree->IsA("mafVMELandmarkCloud") ||
      vmeAttachedAlreadyToTargetTree->IsA("mafVMEExternalData"))
  {
    m_NumberOfVMEWithBinary++;
    mafString savedMSFFileName = ((mafVMERoot *)m_Input->GetRoot())->GetStorage()->GetURL();
    wxString oldVMEDataLocalFileName, newVMEDataLocalFileName, tmpURL;
    wxString path, msfLocalFileNameWithoutExtension, ext;
    wxString oldVMEDataABSFolder, oldVMEDataLocalFileNameWithoutExtension, \
      oldVMEDataFileExtension;

    wxSplitPath(savedMSFFileName.GetCStr(), &path, &msfLocalFileNameWithoutExtension, &ext);
    if(!vmeAttachedAlreadyToTargetTree->IsA("mafVMEExternalData"))
    {
      mafDataVector *dataVector = ((mafVMEGeneric*)vmeAttachedAlreadyToTargetTree)->GetDataVector();
      mafDataVector::DataMap::iterator dataVectorIterator;

      
      dataVectorIterator = dataVector->Begin();
      mafVMEItem *vmeItem=dataVectorIterator->second;
      oldVMEDataLocalFileName = vmeItem->GetURL();
      wxSplitPath(oldVMEDataLocalFileName, &oldVMEDataABSFolder, \
        &oldVMEDataLocalFileNameWithoutExtension, &oldVMEDataFileExtension);

      dataVector->UpdateVectorId();
      vmeItem->UpdateItemId();
      int newVMEItemId = vmeItem->GetId();

      newVMEDataLocalFileName = msfLocalFileNameWithoutExtension << '.' << newVMEItemId << '.'\
        << oldVMEDataFileExtension;

      vmeItem->SetURL(newVMEDataLocalFileName);     
    }
    else
    {
      // Now external files are treated as common binary files. This is required for download integrity check.
      oldVMEDataLocalFileName = wxString(mafVMEExternalData::SafeDownCast(vmeAttachedAlreadyToTargetTree)->GetFileName())+"."+wxString(mafVMEExternalData::SafeDownCast(vmeAttachedAlreadyToTargetTree)->GetExtension());
    }
   
    wxString oldVMEDataABSFileName = m_IncomingCompletePath.GetCStr();
    oldVMEDataABSFileName += oldVMEDataLocalFileName.c_str();
    wxString newVMEDataABSFileName = path;
    newVMEDataABSFileName += "/";
    newVMEDataABSFileName += newVMEDataLocalFileName.c_str();

    //Call python module to copy binary data when downloaded
    wxString oldDir = wxGetCwd();
    wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
    if (m_DebugMode)
      mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

    wxString command2execute;
    command2execute.Clear();
    command2execute = m_PythonwInterpreterFullPath.GetCStr();

    command2execute.Append("binaryImporter.py ");
    command2execute.Append("false"); //false if it is not animated
    command2execute.Append(" ");
    
    oldVMEDataABSFileName.Replace(" ", "???");
    command2execute.Append(oldVMEDataABSFileName);

    command2execute.Append(" ");

    newVMEDataABSFileName.Replace(" ", "???");
    command2execute.Append(newVMEDataABSFileName);

    // DEBUG
    //if (m_DebugMode)
    mafLogMessage("");
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
    mafLogMessage("");

    if (m_PassPhrase != "")
    {
      oldVMEDataABSFileName.Replace("???", " ");
      DecryptFile(oldVMEDataABSFileName);
    }

    long processID = wxExecute(command2execute, wxEXEC_ASYNC);
    wxSetWorkingDirectory(oldDir);

    mafString fileToDencrypt = newVMEDataABSFileName;
 
    if (m_DebugMode)
      mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  }
  //
}
//----------------------------------------------------------------------------
void lhpOpDownloadVMERefactor::CopyDownloadedAnimatedVMEDataInTargetMSFDir( mafNode *node )
//----------------------------------------------------------------------------
{
  if (node->IsA("mafVMEGeneric") || node->IsA("mafVMEVector"))
  {
    m_NumberOfVMEWithBinary++;
    mafDataVector *dv = ((mafVMEGeneric*)node)->GetDataVector();
    mafDataVector::Iterator it;
    mafString newMSFFileName = ((mafVMERoot *)m_Input->GetRoot())->GetStorage()->GetURL();
    wxString path, name, ext;
    wxString oldArchiveURL, newArchiveURL, tmpURL;
    wxString oldItemURL, oldItemExt, oldItemPath, newItemURL;
    wxString oldArchivePath, oldArchiveName, oldArchiveExt;
    
    wxSplitPath(newMSFFileName.GetCStr(), &path, &name, &ext);
    it = dv->Begin();
    mafVMEItem *item=it->second;
    oldArchiveURL = item->GetArchiveFileName();
    wxSplitPath(oldArchiveURL, &oldArchivePath, &oldArchiveName, &oldArchiveExt);
    dv->UpdateVectorId();
    int newId = dv->GetVectorID();

    newArchiveURL = name;
    newArchiveURL += '.';
    newArchiveURL += mafString(newId);
    newArchiveURL += '.';
    newArchiveURL += oldArchiveExt;

    item->SetArchiveFileName(mafString(newArchiveURL.c_str()));
    

    wxString absOldArchiveURL = m_IncomingCompletePath.GetCStr();
    absOldArchiveURL += oldArchiveURL;

    wxString absNewArchiveURL = path;
    absNewArchiveURL += '/';
    absNewArchiveURL += newArchiveURL;

    for (it = dv->Begin(); it!= dv->End(); it++)
    {
      mafVMEItem *item=it->second;
      oldItemURL = item->GetURL();
      wxSplitPath(oldItemURL, &oldItemPath, &oldItemURL, &oldItemExt);
      item->UpdateItemId();
      newId = item->GetId();
      newItemURL = name;
      newItemURL += '.';
      newItemURL += mafString(newId);
      newItemURL += '.';
      newItemURL += oldItemExt;
      item->SetURL(newItemURL);
      item->SetArchiveFileName(mafString(newArchiveURL.c_str()));
    }

    //Call python module to copy binary data when downloaded
    wxString oldDir = wxGetCwd();
    wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
    if (m_DebugMode)
      mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

    wxString command2execute;
    command2execute.Clear();
    command2execute = m_PythonwInterpreterFullPath.GetCStr();

    command2execute.Append("binaryImporter.py ");
    command2execute.Append("true"); //true if dataVectorIterator is animated
    command2execute.Append(" ");


    absOldArchiveURL.Replace(" ", "???");
    command2execute.Append(absOldArchiveURL);
    command2execute.Append(" ");

    absNewArchiveURL.Replace(" ", "???");
    command2execute.Append(absNewArchiveURL);
    if (m_DebugMode)
      mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

    // if (m_DebugMode)
    mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

    if (m_PassPhrase != "")
    {
      DecryptFile(absOldArchiveURL);
    }

    long processID = wxExecute(command2execute, wxEXEC_ASYNC);
    wxSetWorkingDirectory(oldDir);

    if (m_DebugMode)
      mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  }
}

int lhpOpDownloadVMERefactor::DownloadSuccessful(bool successful)
{
  if(m_ServiceURL.Compare("")==0)
    m_ServiceURL = m_UploadDownloadVMEHelper->GetServiceURL(m_FromSandbox);
  bool failed = !successful;

  mafString checkParameter;
  for (int m = 0; m < m_TriedToDownloadResourcesList.size(); m++)
  {
    if (m != 0)
      checkParameter.Append(":");
    if (m == m_DownloadedResourcesList.size()-1 && failed)
    {
      //if failed, last VME genereted an error
      checkParameter.Append(m_TriedToDownloadResourcesList[m]);
      checkParameter.Append(",");
      checkParameter.Append("error");
    }
    else
    {
      checkParameter.Append(m_TriedToDownloadResourcesList[m]);
      checkParameter.Append(",");
      checkParameter.Append("success");
    }
  }

  wxString oldDir = wxGetCwd();
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
  if (m_DebugMode)
    mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

  wxString command2execute;
  command2execute.Clear();
  command2execute = m_PythonwInterpreterFullPath.GetCStr();

  command2execute.Append("lhpDownloadVmeCheck.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_ServiceURL.GetCStr());
  command2execute.Append(" ");
  command2execute.Append(checkParameter.GetCStr());
  
  // DEBUG
  // if (m_DebugMode)   
  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage("");
  mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
  mafLogMessage("");

  long pid = -1;
  if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in lhpDownloadVmeCheck.py", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
    return MAF_ERROR;
  }

  wxSetWorkingDirectory(oldDir);
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  return MAF_OK;
}

int lhpOpDownloadVMERefactor::KillThreadedUploaderDownloaderProcess()
{
  return m_UploadDownloadVMEHelper->KillThreadedUploaderDownloaderProcess();
}

void lhpOpDownloadVMERefactor::WaitForDownloadCompletition(int numOfDownloadToWait)
{

  wxInfoFrame *wait;
  if(!m_TestMode)
  {
    wait = new wxInfoFrame(NULL, "Download integrity check! Please wait...");
    wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
    wait->Show(true);
    wait->Refresh();
    wait->Update();
  }

  vtkMAFSmartPointer<vtkDirectory> vtkDir;
  mafString callsDirPath = lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\DownloadCallStack\\";
  //std::vector<mafString> callsPaths;
  int numOfCompletedDownload = 0;
  mafLogMessage(wxString::Format("Starting download integrity check on %d VME",numOfDownloadToWait));
  
  while(1)
  {
    vtkDir->Open(callsDirPath);
    numOfCompletedDownload  = vtkDir->GetNumberOfFiles() - 2; // the file in this directory are supposed to be only .call files (no control on file extension)
    
    if(numOfCompletedDownload >= numOfDownloadToWait) // Add a timeout
    {
      // write on a file the number of vme that will be downloaded
//       wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
//       path += "lhpCommunication.txt";

      //if file exists , delete it
//       if (wxFileExists(path))
//       {
//         wxRemoveFile(path);
//       }
// 
//       ofstream commFile;
//       commFile.open(path);
// 
//       commFile << "0" << "\n";
// 
//       commFile.close();
      //

      mafLogMessage("Download completed! Checking integrity...");
      int numOfFailedDownload = 0;
      numOfCompletedDownload = vtkDir->GetNumberOfFiles();
      for(int i = 0; i < numOfCompletedDownload; i++)
      {
        if(!m_TestMode)
        {
          delete wait;
          wait = new wxInfoFrame(NULL, wxString("Download in progress. Please wait..."));
          wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
          wait->Show(true);
          wait->Refresh();
          wait->Update();
        }

        mafLogMessage("=====================================================");

        std::string callFilePath = vtkDir->GetFile(i);
        
        mafLogMessage(wxString::Format("File: %s",callFilePath.c_str()));

        size_t foundPosition;

        foundPosition=callFilePath.find(".call");

        int nameLength = callFilePath.size();

        bool foundCallExtensionInName = (foundPosition!=std::string::npos);
        bool isExtensionAtTheEndOfName = (nameLength - foundPosition)  == 5 ? true : false ;

        bool isCallConfigFile = foundCallExtensionInName && isExtensionAtTheEndOfName;
        
        if(!isCallConfigFile) // only .call files
        {
          continue;
        }
        callFilePath = mafString(wxString(callsDirPath.GetCStr()) + wxString(callFilePath.c_str()));
        mafLogMessage(wxString::Format("Opening result call file %s",callFilePath.c_str()));
        std::string callResultStr;
        int callResult; // 1 for success 0 for failure
        std::ifstream callFile;

        callFile.open(callFilePath.c_str());
        
        callFile >> callResultStr;
        callResult = atoi(callResultStr.c_str());
        if(callResult == 0)
        {
          numOfFailedDownload++;

          //if file exists , delete it
//           if (wxFileExists(path))
//           {
//             wxRemoveFile(path);
//           }
// 
//           commFile.open(path);
// 
//           commFile << numOfFailedDownload << "\n";
// 
//           commFile.close();
          //

          mafLogMessage("Detected failed download!");

          // Read the call parameters from file

          std::string binarySize;
          std::string cacheDir;
          std::string userName;
          std::string repositoryURL;
          std::string xmlUri;
          std::string srbDataUri;
          std::string isLastResourceToDownload;

          std::string isAnimated;
          std::string binaryFileOldPath;
          std::string binaryFileNewPath;
          
          callFile >> binarySize;
          callFile >> cacheDir;
          callFile >> userName;
          callFile >> repositoryURL;
          callFile >> xmlUri;
          callFile >> srbDataUri;
          callFile >> isLastResourceToDownload;

          callFile >> isAnimated;
          callFile >> binaryFileOldPath;
          callFile >> binaryFileNewPath;

          callFile.close();

//           if (wxFileExists(callFilePath.c_str()))
//           {
          mafLogMessage(wxString::Format("Removing file %s ",callFilePath.c_str()));
          bool result = false;
          while (!result)
          {
            result = wxRemoveFile(callFilePath.c_str());
            //mafSleep(1000);
            mafLogMessage(wxString::Format("wxFileExists = %d || wxRemoveFile = %d || number of failed download = %d",wxFileExists(callFilePath.c_str()),result,numOfFailedDownload));
          }
//           }

          // Here call the download procedure another time

          wxString oldDir = wxGetCwd();
          if (m_DebugMode)
            mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
          wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
          if (m_DebugMode)
            mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

          // first call client.py
          wxString command2execute;
          command2execute.clear();
          command2execute = m_PythonwInterpreterFullPath.GetCStr();
          mafString pythonScriptName = "Client.py ";
          command2execute.Append(pythonScriptName.GetCStr());
          command2execute.Append("127.0.0.1 "); //server address (localhost)
          command2execute.Append("50000 "); //port address (50000)
          command2execute.Append(wxString::Format("DOWNLOAD ")); //Download command
          command2execute.Append(wxString::Format("%s ",binarySize.c_str())); //file size

          // Workaround to understanding directory argument
          wxString currentCacheChildABSFolderWorkaroundRep = wxString(cacheDir.c_str());
          currentCacheChildABSFolderWorkaroundRep.Replace(" ", "?");
          command2execute.Append(wxString::Format("%s ",currentCacheChildABSFolderWorkaroundRep.c_str())); //cache directory
          command2execute.Append(wxString::Format("%s ",m_User->GetName())); //user
          command2execute.Append(wxString::Format("%s ",m_User->GetPwd())); //pwd
          command2execute.Append(wxString::Format("%s ",repositoryURL.c_str())); //dev repository
          command2execute.Append(wxString::Format("%s ",xmlUri.c_str())); //XML URI NAME
          command2execute.Append(wxString::Format("%s ",srbDataUri.c_str())); //SRB DATA NAME
          command2execute.Append(wxString::Format("%s",isLastResourceToDownload.c_str())); //is last VME
          wxString command2executeWithoutPassword = command2execute;
          command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");
          // DEBUG
          //if (m_DebugMode)

          // Check the number of running threads
          int numberOfThreads = 0;
          do
          {
            wxString path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
            path += "lhpDownloadThreads.txt";

            //This file must exist
            if (wxFileExists(path))

            {
              std::ifstream commFile(path,std::ios::in);
              std::string strNumberOfThreads;
              commFile >> strNumberOfThreads;
              numberOfThreads = atoi(strNumberOfThreads.c_str());
              commFile.close();
              if(numberOfThreads > 20)
              {
                mafSleep(15000);
              }
            }
            else
            {
              numberOfThreads = 0;
            }
          }
          while(numberOfThreads > 20);

          mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
          long processID = wxExecute(command2execute, wxEXEC_ASYNC);

          mafSleep(1000);
          // then call binaryImporter.py
          command2execute.Clear();
          command2execute = m_PythonwInterpreterFullPath.GetCStr();

          command2execute.Append("binaryImporter.py ");

          command2execute.Append(wxString::Format("%s ",isAnimated.c_str()));
          command2execute.Append(wxString::Format("%s ",binaryFileOldPath.c_str()));
          command2execute.Append(wxString::Format("%s",binaryFileNewPath.c_str()));

          //if (m_DebugMode)
          mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
          processID = wxExecute(command2execute, wxEXEC_ASYNC);
          mafSleep(1000);
          wxSetWorkingDirectory(oldDir);
        }
        else
        {
          callFile.close();
//           if (wxFileExists(callFilePath.c_str()))
//           {
          mafLogMessage(wxString::Format("Removing file %s",callFilePath.c_str()));
          
          bool result = false;
          while (!result)
          {
            result = wxRemoveFile(callFilePath.c_str());
            //mafSleep(1000);
            mafLogMessage(wxString::Format("wxFileExists = %d || wxRemoveFile = %d || number of failed download = %d",wxFileExists(callFilePath.c_str()),result,numOfFailedDownload));
          }
          
//           }
          mafLogMessage("Successful download!");
        }
        //mafSleep(1000);
      }

      if(numOfFailedDownload == 0)
      {
        if(!m_TestMode)
        {
          delete wait;
        }
        wxMessageBox("Download completed!","Information!",wxICON_INFORMATION);
        break;
      }

      // Delete the directory and all its content
      // wxRmdir(callsDirPath.GetCStr());

      // write on a file the number of vme that will be downloaded
//       path = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();
//       path += "lhpCommunication.txt";

      //if file exists , delete it CAN BE DELETED?
//       if (wxFileExists(path))
//       {
//         wxRemoveFile(path);
//       }
// 
//       commFile.open(path);
// 
//       commFile << numOfFailedDownload << "\n";
// 
//       commFile.close();
      //

      // Call itself to check for the new downloads
      //mafSleep(2000);

      if(!m_TestMode)
      {
        delete wait;
      }

      WaitForDownloadCompletition(numOfFailedDownload);
      break; // exit the loop
    }
    if(!m_TestMode)
    {
      delete wait;
      wait = new wxInfoFrame(NULL, wxString("Download integrity check! Please wait...\n") + wxString::Format("Detected %d on %d completed downloads.",numOfCompletedDownload,numOfDownloadToWait));
      wait->SetWindowStyleFlag(wxSTAY_ON_TOP); //to keep wait message on top
      wait->Show(true);
      wait->Refresh();
      wait->Update();
    }
    mafLogMessage(wxString::Format("Detected %d on %d completed downloads. Waiting for download completition...",numOfCompletedDownload,numOfDownloadToWait));
    mafSleep(1000);
  }
  mafLogMessage("Download integrity check complete!");
  // Delete the directory and all its content ToDo: create the directory when starting the op and delete it here
  // wxRmdir(callsDirPath.GetCStr());
}

int lhpOpDownloadVMERefactor::FillRepositoryVar()
{
  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_VMEUploaderDownloaderDirABSPath.GetCStr());
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
//----------------------------------------------------------------------------
int lhpOpDownloadVMERefactor::DecryptFile( mafString fileName )
//----------------------------------------------------------------------------
{
  mafString fileToDencrypt = fileName;
  while (!::wxFileExists(fileToDencrypt.GetCStr()))
  {
    mafSleep(1000);
  }

  bool result = false;
  if (fileToDencrypt != "")
  {
    mafLogMessage("======================================");
    mafLogMessage("Decrypting %s",fileToDencrypt.GetCStr());
    mafLogMessage("======================================");
    mafString tmpfile = fileToDencrypt;
    tmpfile << ".tmp";

    int maxTry = 1000;
    int i = 0;
    do 
    {
      i++;
      mafSleep(1000);
      /*result = mafDecryptFile(fileToDencrypt.GetCStr(), tmpfile.GetCStr(), m_PassPhrase.GetCStr());*/

      wxArrayString output;
      wxArrayString errors;
      wxString command2execute;
      command2execute = mafGetApplicationDirectory().c_str();
      command2execute << "/bin/botanApp.exe D \"";
      command2execute << fileName.GetCStr();
      command2execute << "\" \"";
      command2execute << tmpfile.GetCStr();
      command2execute << "\" ";
      command2execute << m_PassPhrase.GetCStr();

      wxString command2executeWithoutPassword = command2execute;
      command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

      mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );

      long pid = -1;
      if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
      {
        wxMessageBox("Error in decrypting data. Dowload stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
        mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),command2execute.c_str(), pid);  

        return MAF_ERROR;
      }
      else
      {
        result = true;
      }

    } while (!result && i<maxTry);

    if (result == false)
    {
      mafLogMessage("======================================");
      mafLogMessage("Decrypting ERROR - DecryptFile");
      mafLogMessage("======================================");
      return MAF_ERROR;
    }

    result = ::wxRemoveFile(fileToDencrypt.GetCStr());

    if (result == false)
    {
      mafLogMessage("======================================");
      mafLogMessage("Decrypting ERROR - RemoveFile");
      mafLogMessage("======================================");
      return MAF_ERROR;
    }

    result = ::wxRenameFile(tmpfile.GetCStr(),fileToDencrypt.GetCStr());

    if (result == false)
    {
      mafLogMessage("======================================");
      mafLogMessage("Decrypting ERROR - RenameFile");
      mafLogMessage("======================================");
      return MAF_ERROR;
    }
    mafLogMessage("======================================");
    mafLogMessage("Decrypting DONE");
    mafLogMessage("======================================");
  }

  return MAF_OK;
}
