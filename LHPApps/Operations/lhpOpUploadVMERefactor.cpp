/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpUploadVMERefactor.cpp,v $
Language:  C++
Date:      $Date: 2011-05-30 14:47:44 $
Version:   $Revision: 1.1.1.1.2.34 $
Authors:   Daniele Giunchi, Stefano Perticoni, Roberto Mucci
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

#ifndef _DEBUG  
  const bool DEBUG_MODE = false;
#else
  const bool DEBUG_MODE = true;
#endif


#include "medDefines.h"
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
#include <wx/busyinfo.h>

#include "lhpOpUploadVMERefactor.h"
#include "lhpTagHandler.h"

#include "mafZipUtility.h"
#include "mafGUI.h"
#include "lhpUser.h"
#include "mafNode.h"
#include "mafVMEGenericAbstract.h"
#include "mafTagArray.h"
#include "mafVMEStorage.h"
#include "mafVMERoot.h"
#include "mafVMEFactory.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafVMESurface.h"
#include "mafVMEExternalData.h"
#include "mafDataVector.h"
#include "mafSmartPointer.h"
#include "mafCrypt.h"

#include "lhpFactoryTagHandler.h"
#include "vtkPolyData.h"

#include "lhpUploadDownloadVMEHelper.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpUploadVMERefactor);
//----------------------------------------------------------------------------

mafString lhpOpUploadVMERefactor::m_CacheChildFolderLocalName = "0";

enum  m_SubdictionaryId_VALUES
{
  NO_SUBDICTIONARY = 0,
  MOTION_ANALYSIS_SUBDICTIONARY = 1,
  DICOM_SUBDICTIONARY = 2,
};

enum lhpOpUploadVME_ID
{
  ID_SUBDICTIONARY = MINID, 
};

//----------------------------------------------------------------------------
lhpOpUploadVMERefactor::lhpOpUploadVMERefactor(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  
  m_TestThreadedUploaderDownloaderApplicationCreated = false;
  
  m_OpType  = OPTYPE_OP;
	m_Canundo = false;
  m_HasLink = false;
  m_HasChild = false;
  m_LinkNode.clear();
  m_LinkName.clear();
  m_SubId = -1;
  m_SendPassPhrase = true;

  m_UploadDownloadVMEHelper = new lhpUploadDownloadVMEHelper(this);
  m_User = NULL;
  m_PythonInterpreterFullPath = "python.exe_UNDEFINED";
  m_PythonwInterpreterFullPath = "pythonw.exe_UNDEFINED";  

  m_CacheMasterFolderABSName = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\UploadCache\\").c_str();
  m_OutgoingFolderABSName = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\Outgoing\\").c_str();

  m_VMEUploaderDownloaderABSFolderName  = (lhpUtils::lhpGetApplicationDirectory() + "\\VMEUploaderDownloaderRefactor\\").c_str();

  m_MSFFileABSFolder = "";
  m_MSFFileABSFileName = "";

  m_UnhandledPlusManualTagsLocalFileName = "manualTagFile.csv";

  //m_RepositoryURL = "http://devel.fec.cineca.it:12680/town/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_RepositoryURL ="https://www.biomedtown.org/biomed_town/LHDL/users/repository/lhprepository2/";
  //m_RepositoryURL = "http://62.149.243.50:8081/town/biomed_town/LHDL/users/repository/lhprepository2";
  m_RepositoryURL = "";
  m_GroupsChoice = true;
  
  m_MasterXMLDictionaryFilePrefix = "lhpXMLDictionary_";
  m_MasterXMLDictionaryLocalFileName = "UNDEFINED";
  m_SubXMLDictionaryFilePrefix = "UNDEFINED" ;
  m_SubXMLDictionaryFileName = "UNDEFINED";
  m_AssembledXMLDictionaryFileName = "assembledXMLDictionary.xml";
  m_SubDictionaryBuildingCommand = "UNDEFINED";

  m_AutoTagsListFromXMLDictionaryLocalFileName = "autoTagsList.txt";
  m_ManualTagsListFromXMLDictionaryLocalFileName = "manualTagsList.txt";
  m_HandledAutoTagsLocalFileName = "handledAutoTagsList.csv";
 
  m_HandledAutoTagsListFromFactory.Clear();
  m_UnhandledAutoTagsListFromFactory.Clear();
  m_AutoTagsList.Clear();
  m_ManualTagsList.Clear();
  
  m_SubdictionaryId = NO_SUBDICTIONARY; // default to none
  m_ConnectionConfigurationFileName = "vmeUploaderConnectionConfiguration.conf" ;

  m_ProxyURL = "";
  m_ProxyPort = "0";

  m_WithChild = false;
  m_IsLast = true;
  m_XMLUploadedResourcesRollBackLocalFileName = "m_InputXMLDataResourcesRollBackFile_UNDEFINED.txt";
  m_IsBinaryDataPresent = false;
  m_RemoteXMLResourceURI = "m_RemoteXMLResourceURI_UNDEFINED.txt: you need to call Upload to fill this ivar!";

  m_ParserOptimization = false;

}


//----------------------------------------------------------------------------
lhpOpUploadVMERefactor::~lhpOpUploadVMERefactor()
//----------------------------------------------------------------------------
{
  //cppDEL(m_User);
  cppDEL(m_UploadDownloadVMEHelper);
}
//----------------------------------------------------------------------------
mafOp* lhpOpUploadVMERefactor::Copy()
//----------------------------------------------------------------------------
{
	/** return a copy of itself, needs to put it into the undo stack */
	return new lhpOpUploadVMERefactor(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpUploadVMERefactor::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
	return (vme != NULL && (!vme->IsMAFType(mafVMERoot)));
}
//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::OpRun()
//----------------------------------------------------------------------------
{
  
  int result = OP_RUN_CANCEL;

  bool upToDate = false;

  upToDate = m_UploadDownloadVMEHelper->IsClientSoftwareVersionUpToDate();
  if(upToDate)
  {
    mafEventMacro(mafEvent(this, MENU_FILE_SAVE));
    this->OpStop(OP_RUN_OK);
  }

  m_PythonInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonInterpreterFullPath();
  m_PythonwInterpreterFullPath = m_UploadDownloadVMEHelper->GetPythonwInterpreterFullPath();
  m_User = m_UploadDownloadVMEHelper->GetUser();
  if(m_RepositoryURL.Compare("") == 0)
    m_RepositoryURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);
  if(m_RepositoryURL.Equals("1"))
  {
    wxMessageBox("Error in GetServiceURL(). Please check your internet connection and retry operation.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    OpStop(OP_RUN_CANCEL);
    return;
  }

  if(!m_User->GetProxyFlag())
  {
    m_ProxyURL = m_User->GetProxyHost();
    m_ProxyPort = m_User->GetProxyPort();
    m_UploadDownloadVMEHelper->SaveConnectionConfigurationFile();
  }
  else
  {
    m_UploadDownloadVMEHelper->RemoveConnectionConfigurationFile();
  }

}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::Upload(bool isRootInRepository)
//----------------------------------------------------------------------------
{
  //check if vme has a inputVMEName
  if(strcmp(m_Input->GetName(), "") == 0)
  {
    wxMessageBox("Can not upload VME without inputVMEName.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  mafString hasLink = "false";
  mafString isLastResource = "true";
  mafString uploadWithChildren = "false";
  m_HasLink = false;

  if (m_WithChild)
  {
    uploadWithChildren = "true";
  }
  if (!m_IsLast)
  {
    isLastResource = "false";
  }  

  if (m_Input->GetNumberOfLinks() != 0)
  {
    m_HasLink = true;
    hasLink = "true";
    StoreInputVMELinksInfo();

    //remove links that will be linked again after
    m_Input->RemoveAllLinks();
    mafEventMacro(mafEvent(this, MENU_FILE_SAVE));
  }

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str());
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolderName.GetCStr());

  //if already exist file with binary URI, remove it
  wxString fileName = m_Input->GetName();
  fileName << wxString::Format("%d", m_Input->GetId());
  wxString lockPath = m_VMEUploaderDownloaderABSFolderName;
  lockPath += fileName;
  if (wxFileExists(lockPath))
  {
    wxRemoveFile(lockPath); //fileName
  }
  
  if (GetTestMode() == false)
  { 
    FillMSFFileABSFolderIVar();
  }
  

  if (m_MSFFileABSFolder == "")
  {
    wxMessageBox("Can't edit VME tags: msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  if(!MasterCacheFolderAndOutgoingFolderAvailable())
  {
    wxMessageBox("Unable to create Cache Base Directory. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  if(!CreateChildCache())
  {
    wxMessageBox("Unable to create a temporary cache, remember that msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }
  int result;

  if(m_Input->IsA("mafVMERoot") || isRootInRepository) // bug #2068 fix
  {
    // create the xml resource to fill the tag
    result = CreateXMLResource();

    if (result != MAF_ERROR && m_SendPassPhrase)
    {
      result = SendPassPhrase();
    }

    mafTagItem rootTag;
    rootTag.SetName("RootURI");
    rootTag.SetValue(m_RemoteXMLResourceURI.GetCStr());
    m_Input->GetTagArray()->SetTag(rootTag);
  }
  else
  {
    mafTagItem rootTag;
    rootTag.SetName("RootURI");
    rootTag.SetValue("-1");
    m_Input->GetTagArray()->SetTag(rootTag);
  }

  int ret = this->GeneratesHandledAndUnhandledPlusManualTagsFileFromXMLDictionary();
  if (ret == MAF_ERROR)
  {
    wxMessageBox("Problems generating tags list! Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return ret;
  } 

  if (m_Input->IsA("mafVMERoot") && m_Input->GetTagArray() == NULL)
  {
    mafTagItem rootTag;
    rootTag.SetName("ROOT_TAG");
    m_Input->GetTagArray()->SetTag(rootTag);
  }

   result = BuildMSFForVMEToBeUploaded();
   if (result == MAF_ERROR)
   {
     return result;
   }

  result = CopyPythonEditedTagsIntoOriginalVME();
  if (result == MAF_ERROR)
  {
    return result;
  }

  mafEventMacro(mafEvent(this, MENU_FILE_SAVE));

  if(!CopyInputVMEInCurrentChildCache())
  {
    wxMessageBox("Unable to create a temporary cache, remember that msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  m_UploadDownloadVMEHelper->CreateThreadedUploaderDownloader();
  

  if(!(m_Input->IsA("mafVMERoot") || isRootInRepository))
  {
    result = CreateXMLResource();

    if (result != MAF_ERROR && m_SendPassPhrase)
    {
      result = SendPassPhrase();
    }
  }

  if (result == MAF_ERROR)
  { 

    return result;
  }

  CreateUploadThread(hasLink, uploadWithChildren, isLastResource);
  
  wxSetWorkingDirectory(oldDir);

  
  return MAF_OK;
}

//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::OpDo()   
//----------------------------------------------------------------------------
{
  // nothing to do for the moment...
}

//-------------------------------------------------------------------
int lhpOpUploadVMERefactor::CopyPythonEditedTagsIntoOriginalVME()
//-------------------------------------------------------------------
{

  mafString MSFCreatedByPythonABSFileName;
  mafString MSFToRestoreABSFileName;
  if(m_ParserOptimization)
  {
    MSFCreatedByPythonABSFileName.Append(m_CurrentCacheChildABSFolder.c_str());
    //MSFCreatedByPythonABSFileName.Append("/");
  }
  else
  {
    MSFCreatedByPythonABSFileName.Append(m_MSFFileABSFolder.GetCStr());
    MSFCreatedByPythonABSFileName.Append("/");
  }
  MSFCreatedByPythonABSFileName.Append("OutputMSF");
  int fileNumber = 0;
  MSFToRestoreABSFileName = MSFCreatedByPythonABSFileName;

  while(wxFileExists(MSFToRestoreABSFileName.Append(".msf").GetCStr()))
  {
    MSFToRestoreABSFileName = MSFCreatedByPythonABSFileName;
    MSFToRestoreABSFileName << fileNumber;
    fileNumber++;   
  }

  MSFCreatedByPythonABSFileName.Append(".lhp");
  int result = rename(MSFCreatedByPythonABSFileName, MSFToRestoreABSFileName);
  mafLogMessage("<<<<<renamed %s in %s",MSFCreatedByPythonABSFileName.GetCStr(),MSFToRestoreABSFileName.GetCStr());
  if ( result != 0 )
    return MAF_ERROR;

  mafVMEStorage *storage;
  storage = mafVMEStorage::New();
  storage->SetURL(MSFToRestoreABSFileName.GetCStr());

  mafVMERoot *root;
  root = storage->GetRoot();
  root->Initialize();
  root->SetName("RootB");

  int res = storage->Restore();
  if (res != MAF_OK)
  {
    // if some problems occurred during import give feedback to the user
    if (!m_TestMode)
      mafErrorMessage(_("Errors during file parsing! Look the log area for error messages."));
    return MAF_ERROR;
  }
  if (m_Input->IsA("mafVMERoot"))
  {
    //copy tags from MSF genereted by python editor, to orginal MSF.
    m_Input->GetTagArray()->DeepCopy(root->GetTagArray());
  }
  else
  {
    mafNode *temporaryNode = root->GetFirstChild();
    if (temporaryNode != NULL)
    {
      //copy tags from MSF genereted by python editor, to orginal MSF.
      m_Input->GetTagArray()->DeepCopy(temporaryNode->GetTagArray());
    }
    mafDEL(temporaryNode);
  }

  //attach links previously removed
  if (m_HasLink)
  {
    for (int i = 0; i < m_LinkNode.size(); i++)
    {
      m_Input->SetLink(m_LinkName[i].GetCStr(), m_LinkNode[i], m_SubId);
    }
  }

  remove(MSFToRestoreABSFileName);
  mafDEL(storage);
  return MAF_OK;
}

//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::StoreInputVMELinksInfo()   
//----------------------------------------------------------------------------
{
  m_LinkNode.clear();
  m_LinkName.clear();
  for (mafNode::mafLinksMap::iterator i = m_Input->GetLinks()->begin(); i != m_Input->GetLinks()->end(); i++)
  {
    if (i->second.m_Node != NULL)
    {
      mafNode *link = i->second.m_Node;
      if (link->IsA("mafVMELandmarkCloud") && i->second.m_NodeSubId != -1)
      {
        m_SubId = i->second.m_NodeSubId;
      }
      m_LinkNode.push_back(link);
      m_LinkName.push_back(i->first);
    }
  }
}


//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::OpStop(int result)   
//----------------------------------------------------------------------------
{
  if(m_Gui) HideGui();
	mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
bool lhpOpUploadVMERefactor::CreateChildCache()
//----------------------------------------------------------------------------
{
  mafLogMessage("");
  mafLogMessage("<<<<<Creating Upload Cache Dir>>>>>");
  mafLogMessage("");

  bool result = false;
  //control cache subdir
  wxString currentSubdir;
  currentSubdir = m_CacheMasterFolderABSName + m_CacheChildFolderLocalName.GetCStr();
  while(wxDirExists(currentSubdir))
  {
    int number = atoi(m_CacheChildFolderLocalName.GetCStr());
    number += 1;
    m_CacheChildFolderLocalName = "";
    m_CacheChildFolderLocalName << number;
    currentSubdir = m_CacheMasterFolderABSName + m_CacheChildFolderLocalName.GetCStr();
  }

  currentSubdir = currentSubdir + "\\";
  if(wxMkDir(currentSubdir) == 0)
    result = true;

  m_CurrentCacheChildABSFolder = currentSubdir;

  // DEBUG
  std::ostringstream stringStream;
  stringStream << "Created Cache Subdir " << m_CurrentCacheChildABSFolder << " for " << m_Input->GetName() << " uploading" << std::endl;
  mafLogMessage(stringStream.str().c_str());
        
  mafLogMessage("");
  mafLogMessage("<<<<<END Creating Upload Cache Dir>>>>>");
  mafLogMessage("");

  return result;
}
//----------------------------------------------------------------------------
bool lhpOpUploadVMERefactor::CopyInputVMEInCurrentChildCache()
//----------------------------------------------------------------------------
{
  mafLogMessage("");
  mafLogMessage("<<<<<Copy Input VME in Upload Cache>>>>>");
  mafLogMessage("");

  std::ostringstream stringStream;
  stringStream << "Creating Upload Cache for: " << m_Input->GetName() << std::endl;
  mafLogMessage(stringStream.str().c_str());
        
  bool copied = false;
  //If VME is ExternalData, copy the external file
  if (m_Input->IsA("mafVMEExternalData"))
  {
    wxString externalFileName = ((mafVMEExternalData *)m_Input)->GetFileName();
    externalFileName.append(".");
    externalFileName.append(((mafVMEExternalData *)m_Input)->GetExtension());
    wxString externalFilePath = ((mafVMEExternalData *)m_Input)->GetAbsoluteFileName();
    wxString externalCopiedName =  m_CurrentCacheChildABSFolder + "\\" + externalFileName;
    if(wxFileExists(externalFilePath))
    {
      copied = wxCopyFile(externalFilePath, externalCopiedName); // It's really needed? (already storage->Store())
      //((mafVMEExternalData *)m_Input)->SetFileName();
    }
  }

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  wxSetWorkingDirectory(m_CurrentCacheChildABSFolder.c_str());

  wxString msfname = m_CacheChildFolderLocalName;
  msfname.Append(".msf");

  //////////////////////////////////////////////////////////////////////////
  //To optimize
  //////////////////////////////////////////////////////////////////////////
  std::vector<mafString> filesCreated;
  if (mafVMEGenericAbstract::SafeDownCast(m_Input) && mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector() && mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->GetArchiveName() != "")//If is a multi time VME
  {
    
    mafStorage *storage = mafVMERoot::SafeDownCast(m_Input->GetRoot())->GetStorage();
    mafString base_url = storage->GetURL();
    mafString tmp = wxPathOnly(base_url.GetCStr());
    tmp<<"/";
    tmp<<mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->GetArchiveName();    
    mafLogMessage("<<<<<<<<<Opening zip file : %s",tmp.GetCStr());
    filesCreated = ZIPOpen(tmp);

    //mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->SetSingleFileMode(false);
    mafDataVector::DataMap::iterator it;
    mafVMEItem *item = NULL;
    it = mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->Begin();
    for (it = mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->Begin(); it != mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->End(); it++)
    {
      item = it->second;
      item->SetIOModeToDefault();
    }
  }
  //////////////////////////////////////////////////////////////////////////

  // restore due attributes
  mafString typeVme;
  typeVme = m_Input->GetTypeName();

  if (typeVme.Equals("mafVMELandmark"))
  {
    mafSmartPointer<mafVMEFactory> factory;
    m_CacheVme = factory->CreateVMEInstance("mafVMELandmarkCloud");
    if (!m_CacheVme)
      return false;

    m_CacheVme->DeepCopy(m_Input->GetParent());
    ((mafVMELandmarkCloud *)m_CacheVme)->SetNumberOfLandmarks(0);
    mafSmartPointer<mafVMELandmark> landmark;
    landmark->DeepCopy(m_Input);
    mafVMEGenericAbstract *vmeGeneric = mafVMEGenericAbstract::SafeDownCast(m_Input);
    //landmark->SetMatrix(*vmeGeneric->GetOutput()->GetAbsMatrix()); // This is commented in order to upload the vme with its relative matrix
    landmark->SetMatrix(*vmeGeneric->GetOutput()->GetMatrix());
    m_CacheVme->AddChild(landmark);
  }
  else
  {
    mafSmartPointer<mafVMEFactory> factory;
    m_CacheVme = factory->CreateVMEInstance(typeVme);
    if (!m_CacheVme)
      return false;

    try
    {
      m_CacheVme->DeepCopy(m_Input);
    }
    catch (...)
    {
      mafLogMessage("<<<<<WARNING!! VME Too large! DeepCopyVmeLarge is used!  ");
    	mafVMEGenericAbstract::SafeDownCast(m_CacheVme)->DeepCopyVmeLarge(m_Input);
    }
    
    mafVMEGenericAbstract *vmeGeneric = mafVMEGenericAbstract::SafeDownCast(m_Input);
    if (vmeGeneric != NULL)
    {
      //m_CacheVme->SetMatrix(*vmeGeneric->GetOutput()->GetAbsMatrix()); // This is commented in order to upload the vme with its relative matrix
      m_CacheVme->SetMatrix(*vmeGeneric->GetOutput()->GetMatrix());
    }
  }

  mafSmartPointer<mafVMESurface> fakeLinkNode;
  //link a fake node, in order to have link with Id = -1. So when the VME will be downloaded
  //it will be imported even if the linked VME are still to be downloaded
  if (m_HasLink)
  {
    for (int i = 0; i < m_LinkNode.size(); i++)
    {
      m_CacheVme->SetLink(m_LinkName[i].GetCStr(), fakeLinkNode);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //To optimize - removing tmp files
  //////////////////////////////////////////////////////////////////////////
  if (mafVMEGenericAbstract::SafeDownCast(m_Input) && mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector() && mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->GetArchiveName() != "")//If is a multi time VME
  {

    mafLogMessage("<<<<<<<<<Removing temp data");
    for (int i=0;i<filesCreated.size();i++)
    {
      remove(filesCreated[i]);
    }

    mafDataVector::DataMap::iterator it;
    mafDataVector::DataMap::iterator itCache;
    mafVMEItem *item = NULL;
    mafVMEItem *itemCache = NULL;
    it = mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->Begin();
    itCache = mafVMEGenericAbstract::SafeDownCast(m_CacheVme)->GetDataVector()->Begin();
    for (it = mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->Begin(), itCache = mafVMEGenericAbstract::SafeDownCast(m_CacheVme)->GetDataVector()->Begin(); it != mafVMEGenericAbstract::SafeDownCast(m_Input)->GetDataVector()->End(); it++,itCache++)
    {
      item = it->second;
      item->SetIOModeToMemory();

      itemCache = itCache->second;
      itemCache->SetIOModeToMemory();
    }
  }
  //////////////////////////////////////////////////////////////////////////

  mafVMEStorage *storage;
  storage = mafVMEStorage::New();
  //Substitute character

  // Bug #1977 Fix (IOR users haven't write permission on c:\)
  storage->SetURL((m_CurrentCacheChildABSFolder + "//" + msfname).c_str()); // Set an absolute path! otherwise an external file will be copied inside the root directory (C:\)

  mafVMERoot *root;
  root = storage->GetRoot();
  root->Initialize();
  root->SetName("Root");

  if (m_CacheVme->IsA("mafVMERoot"))
    root->DeepCopy(m_CacheVme);
  else
    root->AddChild(m_CacheVme);

  storage->Store();
  wxSetWorkingDirectory(oldDir);

  std::ostringstream stringStream1;
  stringStream1 << "Upload Cache for " << m_Input->GetName() << " successfully created in: " << m_CurrentCacheChildABSFolder << std::endl;
  mafLogMessage(stringStream1.str().c_str());
  
  mafLogMessage("");
  mafLogMessage("<<<<<END Copy Input VME in Upload Cache>>>>>");
  mafLogMessage("");

  mafDEL(storage); // Remove Leaks

  mafString fileToCrypt = "";
  wxString dirCache = m_CurrentCacheChildABSFolder.c_str();
  wxArrayString files;
  wxDir::GetAllFiles(dirCache,&files);
  for (int i=0;i<files.size();i++)
  {
    wxString path,name,ext;
    wxSplitPath(files[i],&path,&name,&ext);
    if (ext != "msf" && ext != "csv")
    {
      fileToCrypt << files[i];
    }
  }
  int result;
  m_SendPassPhrase = true;
  if (fileToCrypt != "")
  {
    m_SendPassPhrase = true;
    result = CryptingData(fileToCrypt);

//     if (result != MAF_ERROR)
//     {
//       result = SendPassPhrase();
//     }
    if (result == MAF_ERROR)
    { 
      return false;
    }
  }
  else
  {
    result = MAF_OK;
  }
  

  copied = true;
  return copied && result == MAF_OK;
}

//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::GeneratesHandledAndUnhandledPlusManualTagsFileFromXMLDictionary()
//----------------------------------------------------------------------------
{
  mafLogMessage("");
  mafLogMessage("<<<<<Running tags factory>>>>>");
  mafLogMessage("");

  std::ostringstream stringStream;
  stringStream << "Running tags factory for " << m_Input->GetName() << " metadata: trying to fill automatic tags..." << std::endl;
  mafLogMessage(stringStream.str().c_str());
        
  mafString masterXMLDictionaryFilePrefix = "lhpXMLDictionary_";

  FillAutoTagsAndManualTagsVARsFromXMLMasterDictionaryFile(m_PythonwInterpreterFullPath,
  m_VMEUploaderDownloaderABSFolderName, masterXMLDictionaryFilePrefix, m_AutoTagsList, m_ManualTagsList  );

  wxString oldDir = wxGetCwd();

  // ------- handledAutoTagsFile -------

  mafString tagName = "";
  
  lhpTagHandlerInputOutputParametersCargo *parametersCargo = lhpTagHandlerInputOutputParametersCargo::New();
  parametersCargo->SetInputVme(mafVME::SafeDownCast(m_Input));
  parametersCargo->SetInputUser(m_User);
	parametersCargo->SetInputMSF(m_MSFFileABSFileName);

  lhpFactoryTagHandler *tagsFactory  = lhpFactoryTagHandler::GetInstance();
  for (int i = 0; i < m_AutoTagsList.size(); i++)
  {
    tagName = m_AutoTagsList[i].c_str();

    if (tagName != "")
    {
      
      assert(tagsFactory!=NULL);
      lhpTagHandler *tagHandler = NULL;
      tagHandler = tagsFactory->CreateTagHandlerInstance("lhpTagHandler_" + tagName);
      
      if (tagHandler)
      {
        tagHandler->SetPythonExe(m_PythonInterpreterFullPath.GetCStr());
        tagHandler->SetPythonwExe(m_PythonwInterpreterFullPath.GetCStr());
        tagHandler->HandleAutoTag(parametersCargo);
        wxString tagValue = "\"";
        tagValue.Append(tagName.GetCStr());
        tagValue.Append("\",\"");
        tagValue.Append(parametersCargo->GetTagHandlerGeneratedString());
        tagValue.Append('\"');
        m_HandledAutoTagsListFromFactory.Add(tagValue.c_str());
        mafDEL(tagHandler); // Remove Leaks
      }
      else
      {
        m_UnhandledAutoTagsListFromFactory.Add(tagName.GetCStr());
        if (m_DebugMode)
          mafLogMessage(_("Cannot handle \"%s\" tag!, this tag will become manual"),tagName.GetCStr());
      }
      
    }
  }
  // clean up
  parametersCargo->Delete();

  // open auto tags file and try to handle tags using tags factory 
  ofstream handledAutoTagsFile;

  mafString handledAutoTagsLocalFileName = "";
  if(m_ParserOptimization)
  {
    handledAutoTagsLocalFileName = m_CurrentCacheChildABSFolder + m_HandledAutoTagsLocalFileName;
    handledAutoTagsFile.open(handledAutoTagsLocalFileName.GetCStr());
  }
  else
  {
    handledAutoTagsLocalFileName = m_HandledAutoTagsLocalFileName;
    handledAutoTagsFile.open(m_HandledAutoTagsLocalFileName.GetCStr());
  }
  for (int i = 0; i < m_HandledAutoTagsListFromFactory.size(); i++)
  {
    tagName = m_HandledAutoTagsListFromFactory[i].c_str();
    handledAutoTagsFile << tagName.GetCStr() << std::endl ;
  }
  
  handledAutoTagsFile.close();

  // ------- unhandledPlusManualTagsFile -------

  // open auto tags file and try to handle tags using tags factory 
  ofstream unhandledPlusManualTagsFile;

  mafString unhandledPlusManualTagsABSFileName = m_CurrentCacheChildABSFolder + m_UnhandledPlusManualTagsLocalFileName;
  unhandledPlusManualTagsFile.open(unhandledPlusManualTagsABSFileName.GetCStr());

  if (!unhandledPlusManualTagsFile) {
    mafLogMessage("Unable to create file");
    return MAF_ERROR; // terminate with error
  }

  std::vector<std::string> tagList;
  m_Input->GetTagArray()->GetTagList(tagList);

  bool tagFound;
  mafString tagValue = "";

  mafLogMessage("<<<<<<Write unhandled tags");
  // write unhandled tags
  for (int i = 0; i < m_UnhandledAutoTagsListFromFactory.size(); i++)
  {
    tagFound = false;
    tagName = m_UnhandledAutoTagsListFromFactory[i].c_str();

    for (int n = 0; n < tagList.size(); n++)
    {
      if (tagName.Equals(tagList[n].c_str()))
      {
        tagValue =  m_Input->GetTagArray()->GetTag(tagList[n].c_str())->GetValue();
        unhandledPlusManualTagsFile << "\"" << tagName.GetCStr() << "\",\"" << tagValue.GetCStr() << "\"" << std::endl ;
        tagFound = true;
        break;
      }
    }
    if (!tagFound)
      unhandledPlusManualTagsFile << "\"" << tagName.GetCStr() << "\",\"enter a value\"" << std::endl ;
  }

  mafLogMessage("<<<<<<Write manual tags");
  // write manual tags
  for (int i = 0; i < m_ManualTagsList.size(); i++)
  {
    tagFound = false;
    tagName = m_ManualTagsList[i].c_str();

    for (int n = 0; n < tagList.size(); n++)
    {
      if (tagName.Equals(tagList[n].c_str()))
      {
        tagValue =  m_Input->GetTagArray()->GetTag(tagList[n].c_str())->GetValue();
        unhandledPlusManualTagsFile << "\"" << tagName.GetCStr() << "\",\"" << tagValue.GetCStr() << "\"" << std::endl ;
        tagFound = true;
        break; 
      }
    }
    if (!tagFound)
      unhandledPlusManualTagsFile << "\"" << tagName.GetCStr() << "\",\"enter a value\"" << std::endl ;
  }

  unhandledPlusManualTagsFile.close();

  wxSetWorkingDirectory(oldDir);
  
  
  std::ostringstream stringStream1;
  stringStream1 << "Tags factory exited correctly: generated " << unhandledPlusManualTagsABSFileName\
   << " and " << handledAutoTagsLocalFileName << " in " << m_VMEUploaderDownloaderABSFolderName <<  std::endl;
  
  mafLogMessage(stringStream1.str().c_str());
  
  mafLogMessage("");
  mafLogMessage("<<<<<END Running tags factory>>>>>");
  mafLogMessage("");

  if (m_DebugMode)
    mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
  return MAF_OK;

}


//----------------------------------------------------------------------------
bool lhpOpUploadVMERefactor::MasterCacheFolderAndOutgoingFolderAvailable()
//----------------------------------------------------------------------------
{
  bool cacheMasterFolderCreated = false, outgoingFolderCreated = false;

  wxString cacheMasterFolderABSName = m_CacheMasterFolderABSName.GetCStr();
  if ( wxDirExists(cacheMasterFolderABSName) )
  {
     cacheMasterFolderCreated = true;
  }

  else
  {
    wxMkDir(cacheMasterFolderABSName);
    if ( wxDirExists(cacheMasterFolderABSName) ) cacheMasterFolderCreated = true;
  }

  wxString outgoingFolderABSName = m_OutgoingFolderABSName.GetCStr();
  if ( wxDirExists(outgoingFolderABSName) )
  {
    outgoingFolderCreated = true;
  }
  else
  {
    wxMkDir(outgoingFolderABSName);
    if ( wxDirExists(outgoingFolderABSName) ) outgoingFolderCreated = true;
  }

  return cacheMasterFolderCreated && outgoingFolderCreated;
}
//----------------------------------------------------------------------------
mafString lhpOpUploadVMERefactor::GetXMLMasterDictionaryFileName( mafString dictionaryAbsFolder, mafString dictionaryFileNamePrefix )
//----------------------------------------------------------------------------
{
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(dictionaryAbsFolder.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);
  
  if (files.size() == 0)
  {
    std::ostringstream stringStream;
    mafLogMessage(stringStream.str().c_str());
    mafLogMessage("lhpXMLDictionary_*.xml not found! exiting");

    return dictionaryFileName;
  }
  if (files.size() != 1)
  {
    std::ostringstream stringStream;
    stringStream << "Found " << files.size() << " dictionaries!"  << std::endl;    
    mafLogMessage(stringStream.str().c_str());
    mafLogMessage("Too many lhpXMLDictionary_*.xml! exiting");

    return dictionaryFileName;
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
    mafLogMessage("Found dictionary!");
    mafLogMessage(dictionaryFileName.GetCStr());
  }
  
  wxSetWorkingDirectory(oldDir);
  
  return dictionaryFileName;
}
//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::FillMSFFileABSFolderIVar()
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
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::BuildMSFForVMEToBeUploaded()
//----------------------------------------------------------------------------
{
  wxString command2execute;

  if (m_DebugMode)
    command2execute = m_PythonInterpreterFullPath.GetCStr();
  else
    command2execute = m_PythonwInterpreterFullPath.GetCStr();

  mafString pythonScriptName = "lhpEditVMETag.py ";

  wxString msfCurrentCacheChildWorkaroundABSFolder = m_CurrentCacheChildABSFolder;
  msfCurrentCacheChildWorkaroundABSFolder.Replace(" ", "??");

  wxString msfFileDirectoryWorkaroundABSFolder = m_MSFFileABSFolder;
  msfFileDirectoryWorkaroundABSFolder.Replace(" ", "??");

  command2execute.Append(pythonScriptName.GetCStr());
  command2execute.Append(wxString::Format\
    ("%s ",msfCurrentCacheChildWorkaroundABSFolder)); //Input cache child directory
  command2execute.Append(wxString::Format\
    ("%s ",msfFileDirectoryWorkaroundABSFolder)); //Input MSF directory
  command2execute.Append(wxString::Format\
    ("%d ",m_Input->GetId())); //Input VME Id
  command2execute.Append(wxString::Format\
    ("%s ", m_UnhandledPlusManualTagsLocalFileName.c_str())); //Input manualTagFile
  command2execute.Append(wxString::Format\
    ("%s", m_HandledAutoTagsLocalFileName.GetCStr())); //Input autoTagFile

  
  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
  long pid = -1;
  if (pid = wxExecute(command2execute, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Can't edit MSF. Uploading stopped!", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("ASYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
    return MAF_ERROR;
  }

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::CreateXMLResource()
//----------------------------------------------------------------------------
{
  if(m_RepositoryURL.Compare("") == 0)
    m_RepositoryURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);

  mafLogMessage("");
  mafLogMessage("<<<<<Creating XML Resource on repository>>>>>");
  mafLogMessage("");

  // Get XML URI upload target
  wxString command2execute;
  command2execute = m_PythonInterpreterFullPath.GetCStr();
  command2execute.Append("XMLResourceCreator.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_RepositoryURL.GetCStr());

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );
  
  mafLogMessage("Waiting for remote repository to create XML resource.....");

  wxArrayString output;
  wxArrayString errors;

  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in XMLResourceCreator.py. Cannot create XML resource! Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
    command2executeWithoutPassword.c_str(), pid);  

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

  mafLogMessage("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

  for (int i=0;i<output.size();i++)
  {
    mafString test;
    test = output[i];
    mafLogMessage(test);
  }

  mafLogMessage("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  m_PublicKey[0] = output[output.size() - 6];
  m_PublicKey[1] = output[output.size() - 5];
  m_PublicKey[2] = output[output.size() - 4];
  m_PublicKey[3] = output[output.size() - 3];
  m_PublicKey[4] = output[output.size() - 2];
  m_PublicKey[5] = output[output.size() - 1];
  m_RemoteXMLResourceURI = output[output.size() - 7];

  mafLogMessage("");
  std::ostringstream stringStream;
  stringStream  << "Created Resource: " << m_RemoteXMLResourceURI.GetCStr() << std::endl << std::endl;
  
  mafLogMessage(stringStream.str().c_str());

  if (m_RemoteXMLResourceURI == "OverQuota")
  {
    wxMessageBox("Over Quota!. Uploading stopped.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }
  

  mafLogMessage("");
  mafLogMessage("<<<<<END Creating XML Resource on repository>>>>>");
  mafLogMessage("");

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::FillAutoTagsAndManualTagsVARsFromXMLMasterDictionaryFile( mafString pythonInterpreter, mafString vmeUpDownDirAbsFolder, mafString xmlDictionaryLocalFilePrefix, wxArrayString &outAutoTags, wxArrayString &outManualTags )
//----------------------------------------------------------------------------
{  
  wxString oldDir = wxGetCwd();
  wxSetWorkingDirectory(vmeUpDownDirAbsFolder.GetCStr());

  xmlDictionaryLocalFilePrefix = GetXMLMasterDictionaryFileName(vmeUpDownDirAbsFolder, xmlDictionaryLocalFilePrefix);
  if (xmlDictionaryLocalFilePrefix == "NOT FOUND")
  {
	wxSetWorkingDirectory(oldDir);
    return MAF_ERROR;
  }

  mafString dictionaryToProcessLocalFileName = xmlDictionaryLocalFilePrefix;

  // generate auto tags file
  mafString autoTagsListFromXMLDictionaryLocalFileName = "autoTagsList.txt";

  wxString command2execute;
  command2execute = pythonInterpreter.GetCStr();
  command2execute.Append(" lhpXMLDictionaryParser.py ");
  command2execute.Append(dictionaryToProcessLocalFileName.GetCStr());
  command2execute.Append(" auto_tags ");
  command2execute.Append(autoTagsListFromXMLDictionaryLocalFileName.GetCStr());

  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  long pid = -1;

  pid = wxExecute(command2execute, wxEXEC_SYNC);

  if (pid == -1)
  {
    wxMessageBox("Error in lhpXMLDictionaryParser.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
      command2execute.c_str(), pid);
	wxSetWorkingDirectory(oldDir);
    return MAF_ERROR;
  }
  if ( !command2execute ){
	wxSetWorkingDirectory(oldDir);
    return MAF_ERROR;
	}

  mafString manualTagsListFromXMLDictionaryLocalFileName = "manualTagsList.txt";

  // generate manual tags file
  command2execute.Clear();
  command2execute = pythonInterpreter.GetCStr();
  command2execute.Append(" lhpXMLDictionaryParser.py ");
  command2execute.Append(dictionaryToProcessLocalFileName.GetCStr());
  command2execute.Append(" manual_tags ");
  command2execute.Append(manualTagsListFromXMLDictionaryLocalFileName.GetCStr());

  mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

  pid = -1;
  pid = wxExecute(command2execute, wxEXEC_SYNC);
  if (pid == -1)
  {
    wxMessageBox("Error in lhpXMLDictionaryParser.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),
    command2execute.c_str(), pid);
    wxSetWorkingDirectory(oldDir);
	return MAF_ERROR;
  }
  if ( !command2execute ){
	wxSetWorkingDirectory(oldDir);
    return MAF_ERROR;
	}

  outAutoTags.Clear();
  outManualTags.Clear();

  // fill output manualTagsArrayString
  ifstream inManualTagsFile;

  inManualTagsFile.open(manualTagsListFromXMLDictionaryLocalFileName.GetCStr());
  if (!inManualTagsFile) {
    wxString message = manualTagsListFromXMLDictionaryLocalFileName.GetCStr();
    message.Append(" not found! Unable to open XML dictionary file");
    mafLogMessage(message.c_str());
    wxSetWorkingDirectory(oldDir);
	return MAF_ERROR; // terminate with error
  }

  std::string mtag;

  while (inManualTagsFile >> mtag) 
  {
    outManualTags.Add(mtag.c_str());
  }

  inManualTagsFile.close();

  // fill output autoTagsArrayString
  ifstream inAutoTagsFile;

  inAutoTagsFile.open(autoTagsListFromXMLDictionaryLocalFileName.GetCStr());
  if (!inAutoTagsFile) {

    wxSetWorkingDirectory(oldDir);
    mafLogMessage("Unable to open file");
    return MAF_ERROR; // terminate with error
  }

  std::string atag;
  while (inAutoTagsFile >> atag) 
  {
    outAutoTags.Add(atag.c_str());
  }
  inAutoTagsFile.close();

  wxSetWorkingDirectory(oldDir);

  return MAF_OK;
}
//----------------------------------------------------------------------------
void lhpOpUploadVMERefactor::CreateUploadThread( mafString &hasLink, mafString &uploadWithChildren, mafString &isLastResource )
//----------------------------------------------------------------------------
{
  if(m_RepositoryURL.Compare("") == 0)
    m_RepositoryURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);

  mafLogMessage("");
  mafLogMessage("<<<<<Calling Uploader Server>>>>>");
  mafLogMessage("");

  wxString command2execute;
  if (m_DebugMode)
    command2execute = m_PythonInterpreterFullPath.GetCStr();
  else
    command2execute = m_PythonwInterpreterFullPath.GetCStr();

  mafString pythonScriptName = "Client.py ";
  command2execute.Append(pythonScriptName.GetCStr());
  command2execute.Append("127.0.0.1 "); //server address (localhost)
  command2execute.Append("50000 "); //port address (50000)
  command2execute.Append(wxString::Format("UPLOAD ")); //UPLOAD command

  //Get the right id
  if (m_Input->IsA("mafVMELandmark"))
  {
    command2execute.Append(wxString::Format("%d ", m_CacheVme->GetFirstChild()->GetId())); //vme id
  }
  else
  {
    command2execute.Append(wxString::Format("%d ",m_CacheVme->GetId())); //cache vme Id
  }

  //workaround for spaces handling
  wxString currentCacheChildABSFolderWarkaround = m_CurrentCacheChildABSFolder;
  currentCacheChildABSFolderWarkaround.Replace(" ", "??");
  command2execute.Append(wxString::Format("%s ",currentCacheChildABSFolderWarkaround)); //cache directory
  command2execute.Append(wxString::Format("%s ",m_User->GetName())); //user
  command2execute.Append(wxString::Format("%s ",m_User->GetPwd())); //pwd
  command2execute.Append(wxString::Format("%s ",m_RepositoryURL.GetCStr())); //dev repository
  command2execute.Append(wxString::Format("%d ",m_Input->GetId())); //input VME Id in original tree
  command2execute.Append(wxString::Format("%s ", hasLink.GetCStr())); //has link?
  command2execute.Append(wxString::Format("%s ", uploadWithChildren.GetCStr())); //upload with children?
  command2execute.Append(wxString::Format("%s ", \
  m_XMLUploadedResourcesRollBackLocalFileName.GetCStr())); //file to be used for rollback operation, in case of error in msf upload
  command2execute.Append(wxString::Format("%s ", m_RemoteXMLResourceURI.GetCStr())); //XML resource URI
  command2execute.Append(wxString::Format("%s ", isLastResource.GetCStr())); //"true" if is last VME to be uploaded
  wxString inputVMENameWorkaround = m_Input->GetName();
  //workaround for spaces handling
  inputVMENameWorkaround.Replace(" ", "??");
  command2execute.Append(wxString::Format("%s ", inputVMENameWorkaround.c_str())); //vme name

  long processID = wxExecute(command2execute, wxEXEC_ASYNC);

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage(_T("ASYNC Command process '%s' terminated with exit code %d."),
  command2executeWithoutPassword.c_str(), processID);
  
  mafLogMessage("");
  mafLogMessage("<<<<<END Calling Uploader Server>>>>>");
  mafLogMessage("");
  mafDEL(m_CacheVme); // Remove Leaks
  assert(true);
}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::InitializeToUpload(bool isRootInRepository /* = false */)
//----------------------------------------------------------------------------
{
  //check if vme has a inputVMEName
  if(strcmp(m_Input->GetName(), "") == 0)
  {
    wxMessageBox("Can not upload VME without inputVMEName.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  mafString hasLink = "false";
  mafString isLastResource = "true";
  mafString uploadWithChildren = "false";
  m_HasLink = false;

  if (m_WithChild)
  {
    uploadWithChildren = "true";
  }
  if (!m_IsLast)
  {
    isLastResource = "false";
  }  

  if (m_Input==NULL)
  {
    mafLogMessage("L'input non esiste!!!");
  }
  if (m_Input->GetNumberOfLinks() != 0)
  {
    m_HasLink = true;
    hasLink = "true";
    StoreInputVMELinksInfo();

    //remove links that will be linked again after
    m_Input->RemoveAllLinks();
    mafEventMacro(mafEvent(this, MENU_FILE_SAVE));
  }

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
  {
    mafLogMessage( _T("Current working directory is: '%s' <<<<FUNCTION lhpOpUploadVMERefactor::Upload()"), wxGetCwd().c_str());
  }
  mafLogMessage("<<<<<<<<<<m_VMEUploaderDownloaderABSFolderName = %s ",m_VMEUploaderDownloaderABSFolderName);
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolderName.GetCStr());

  //if already exist file with binary URI, remove it
  wxString fileName = m_Input->GetName();
  fileName << wxString::Format("%d", m_Input->GetId());
  wxString lockPath = m_VMEUploaderDownloaderABSFolderName;
  lockPath += fileName;
  if (wxFileExists(lockPath))
    wxRemoveFile(lockPath); //fileName

  if (GetTestMode() == false)
  { 
    FillMSFFileABSFolderIVar();
  }

  if (m_MSFFileABSFolder == "")
  {
    wxMessageBox("Can't edit VME tags: msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  if(!MasterCacheFolderAndOutgoingFolderAvailable())
  {
    wxMessageBox("Unable to create Cache Base Directory. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }

  /*if(!CreateChildCache())
  {
  wxMessageBox("Unable to create a temporary cache, remember that msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
  return MAF_ERROR;
  }*/
  int result;

  if(m_Input->IsA("mafVMERoot") || isRootInRepository) // bug #2068 fix
  {
    // create the xml resource to fill the tag
    result = CreateXMLResource();

//     if (result != MAF_ERROR)
//     {
//       result = SendPassPhrase();
//     }
    if (result == MAF_ERROR)
    { 

      return result;
    }
    mafTagItem rootTag;
    rootTag.SetName("RootURI");
    rootTag.SetValue(m_RemoteXMLResourceURI.GetCStr());
    m_Input->GetTagArray()->SetTag(rootTag);
  }
  else
  {
    mafTagItem rootTag;
    rootTag.SetName("RootURI");
    rootTag.SetValue("-1");
    m_Input->GetTagArray()->SetTag(rootTag);
  }

  int ret = this->GeneratesHandledAndUnhandledPlusManualTagsFileFromXMLDictionary();
  if (ret == MAF_ERROR)
  {
    mafLogMessage(">>>>>>>>>>>>>>>>ERROR INSIDE INIT_UPLOAD");
    wxMessageBox("Problems generating tags list! Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return ret;
  } 

  if (m_Input->IsA("mafVMERoot") && m_Input->GetTagArray() == NULL)
  {
    mafTagItem rootTag;
    rootTag.SetName("ROOT_TAG");
    m_Input->GetTagArray()->SetTag(rootTag);
  }

  //   result = BuildMSFForVMEToBeUploaded();
  //   if (result == MAF_ERROR)
  //   {
  //     return result;
  //   }

//   mafLogMessage("<<<<<<<<<<<Creo il thread : InitializeToUpload");
   m_UploadDownloadVMEHelper->CreateThreadedUploaderDownloader();
// 
// 
//   if(!(m_Input->IsA("mafVMERoot") || isRootInRepository))
//   {
//     result = CreateXMLResource(); 
//   }
// 
//   if (result == MAF_ERROR)
//   { 
// 
//     return result;
//   }

  m_HasLinkStr = hasLink;
  m_IsLastResource = isLastResource;
  m_UploadWithChildren = uploadWithChildren;

  wxSetWorkingDirectory(oldDir);

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::UploadAlreadyInitilized(bool isRootInRepository /* = false */)
//----------------------------------------------------------------------------
{
  int result = MAF_OK;

  wxString oldDir = wxGetCwd();
  if (m_DebugMode)
  {
    mafLogMessage( _T("Current working directory is: '%s'"), wxGetCwd().c_str());
  }
  wxSetWorkingDirectory(m_VMEUploaderDownloaderABSFolderName.GetCStr());

  result = CopyPythonEditedTagsIntoOriginalVME();
  if (result == MAF_ERROR)
  {
    mafLogMessage("<<<<<<<<<<<<<ERROR INSIDE CopyPythonEditedTagsIntoOriginalVME");
    return result;
  }

  mafEventMacro(mafEvent(this, MENU_FILE_SAVE));

  if(!CopyInputVMEInCurrentChildCache())
  {
    wxMessageBox("Unable to create a temporary cache, remember that msf must be saved locally. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }



  //m_UploadDownloadVMEHelper->CreateThreadedUploaderDownloader();


  if(!(m_Input->IsA("mafVMERoot") || isRootInRepository))
  {
    result = CreateXMLResource();

//     if (result != MAF_ERROR)
//     {
//       result = SendPassPhrase();
//     }
  }

  if (result == MAF_ERROR)
  { 

    return result;
  }

  CreateUploadThread(m_HasLinkStr, m_UploadWithChildren, m_IsLastResource);

  wxSetWorkingDirectory(oldDir);

  return MAF_OK;
}
//----------------------------------------------------------------------------
mafString lhpOpUploadVMERefactor::GeneratePassPhrase( int numOfCharacters /*= 8*/ )
//----------------------------------------------------------------------------
{
  /* initialize random seed: */
  srand ( time(NULL) );

  mafString passPhrase  = "";
  for (int i=0;i<numOfCharacters;i++)
  {
    unsigned short iSecret;
    do 
    {
      iSecret = (unsigned short)rand() % 74 + 48;

    } while ( !((iSecret<=57 && iSecret>=48) || (iSecret<=70 && iSecret>=65)) );
    /* generate secret number: */
    
    char *a = (char*)&iSecret;


    passPhrase.Append(a);
  }
  return passPhrase;
}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::CryptingData(mafString fileName)
//----------------------------------------------------------------------------
{
  bool result;
  if (m_PassPhrase.IsEmpty())
  {
    m_PassPhrase = lhpOpUploadVMERefactor::GeneratePassPhrase();
  }

  mafString tmpfile = fileName;
  tmpfile << ".cry";

  wxArrayString output;
  wxArrayString errors;
  wxString command2execute;
  command2execute = mafGetApplicationDirectory().c_str();
  command2execute << "/bin/botanApp.exe E ";
  command2execute << fileName.GetCStr();
  command2execute << " ";
  command2execute << tmpfile.GetCStr();
  command2execute << " ";
  command2execute << m_PassPhrase.GetCStr();

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );

  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in crypting data. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),command2execute.c_str(), pid);  

    return MAF_ERROR;
  }

//   result = mafEncryptFile(fileName.GetCStr(), tmpfile.GetCStr(), m_PassPhrase.GetCStr());
// 
//   if (result == false)
//   {
//     return MAF_ERROR;
//   }

  result = ::wxRemoveFile(fileName.GetCStr());

  if (result == false)
  {
    return MAF_ERROR;
  }

  result = ::wxRenameFile(tmpfile.GetCStr(),fileName.GetCStr());

  if (result == false)
  {
    return MAF_ERROR;
  }

  return MAF_OK;

}
//----------------------------------------------------------------------------
int lhpOpUploadVMERefactor::SendPassPhrase()
//----------------------------------------------------------------------------
{
  if(m_RepositoryURL.Compare("") == 0)
    m_RepositoryURL = m_UploadDownloadVMEHelper->GetServiceURL(m_GroupsChoice);

  mafLogMessage("");
  mafLogMessage("<<<<<Sending PassPhrase crypted>>>>>");
  mafLogMessage("");

  /*m_PassPhrase = lhpOpUploadVMERefactor::GeneratePassPhrase();*/

  // Get XML URI upload target
  wxString command2execute;
  command2execute = m_PythonInterpreterFullPath.GetCStr();
  command2execute.Append("SendPassPhrase.py ");
  command2execute.Append(m_User->GetName());
  command2execute.Append(" ");
  command2execute.Append(m_User->GetPwd());
  command2execute.Append(" ");
  command2execute.Append(m_RepositoryURL.GetCStr());
  command2execute.Append(" \"");
  command2execute.Append(m_PassPhrase.GetCStr());
  command2execute.Append("\" \"");
  for (int i=0;i<6;i++)
  {
    command2execute.Append(m_PublicKey[i].GetCStr());
    if (i != 5)
    {
    	command2execute.Append("###");
    }
  }
  command2execute.Append("\" ");
  command2execute.Append(m_RemoteXMLResourceURI.GetCStr());

  wxString command2executeWithoutPassword = command2execute;
  command2executeWithoutPassword.Replace(m_User->GetPwd().GetCStr(), "**YOUR_BIOMEDTOWN_PASSWORD**");

  mafLogMessage( _T("Executing command: '%s'"), command2executeWithoutPassword.c_str() );

  mafLogMessage("Waiting for sending passphrase.....");

  wxArrayString output;
  wxArrayString errors;

  long pid = -1;
  if (pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC) != 0)
  {
    wxMessageBox("Error in SendPassPhrase.py. Uploading stopped", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    mafLogMessage(_T("SYNC Command process '%s' terminated with exit code %d."),command2executeWithoutPassword.c_str(), pid);  

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

  mafString returnMessage = output[output.size() - 1];

  mafLogMessage("");
  std::ostringstream stringStream;
  stringStream  << "Uploaded PassPhrase: " << returnMessage.GetCStr() << std::endl << std::endl;

  mafLogMessage(stringStream.str().c_str());

  if (returnMessage == "ERROR")
  {
    wxMessageBox("ERROR!. Uploading stopped.", wxMessageBoxCaptionStr, wxSTAY_ON_TOP | wxOK);
    return MAF_ERROR;
  }


  mafLogMessage("");
  mafLogMessage("<<<<<END Sending PassPhrase crypted>>>>>");
  mafLogMessage("");

  return MAF_OK;
}
