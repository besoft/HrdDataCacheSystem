/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpDownloadVMERefactor.h,v $
Language:  C++
Date:      $Date: 2011-04-13 10:44:28 $
Version:   $Revision: 1.1.1.1.2.17 $
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

#ifndef __lhpOpDownloadVMERefactor_H__
#define __lhpOpDownloadVMERefactor_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpUser;
class mafVMEGroup;
class lhpUploadDownloadVMEHelper;
class mafNodeIterator;

//----------------------------------------------------------------------------
// lhpOpDownloadVMERefactor :
//----------------------------------------------------------------------------
/** Download VME from BiomedTown remote repository

IMPORTANT: use lhpOpDownloadVMERefactorTest in \Testing\Operations to debug and validate this 
component along with plugged python component tests in VMEUploaderDownloaderRefactor dir

DEBUG/DEVELOPMENT:
to debug this component along with the ThreadedUploaderDownloader server (ie Upload/Download manager)
run ThreadedUploaderDownloader.py in Eclipse in Debug mode on port 50000

REFACTOR THIS: we need to put lhpOpDownloadVMERefactorTest in automatic regression (Parabuild) 
on both development and production biomedtown as soon as possible. Prerequisites for this:

1: configuration file for webservices
extract all webservices uri/stuff in order to confine
devel/production ws different URIs to a single file. In this way both devel and production 
tests can be run by loading only the different configuration file

2: production server speed enhancement
At the present time this test cannot be run in production since it overloads the server

*/
class LHP_OPERATIONS_EXPORT lhpOpDownloadVMERefactor: public mafOp
{
public:

  enum REPOSITORY_VALUES
  {
    FROM_BASKET = 0,
    FROM_SANDBOX = 1,
  };

  lhpOpDownloadVMERefactor(wxString label = "Download VMEs", int downloadSourceID = FROM_BASKET);

  /** Set Current Working MSF Directory*/
  void SetMSFDirectoryABSPath(mafString msfDirectoryABSPath){m_MSFDirectoryABSPath = msfDirectoryABSPath;};

  /** Get Current Working MSF Directory*/
  mafString GetMSFDirectoryABSPath() {return m_MSFDirectoryABSPath;};

  /** Set the file containing XML resources to be downloaded (default to "ToDownload.txt")*/
  void SetResourcesToDownloadLocalFileName(mafString basketListFileName) {m_ResourcesToDownloadLocalFileName = basketListFileName;};
  
  /** Get the file containing XML resources to be downloaded (default to "ToDownload.txt")*/
  mafString GetResourcesToDownloadLocalFileName() {return m_ResourcesToDownloadLocalFileName;};

  /** Download resources */
  void Download() {this->OpDo();};

  ~lhpOpDownloadVMERefactor(); 

  mafTypeMacro(lhpOpDownloadVMERefactor, mafOp);

  mafOp* Copy();

  /** Class for handle mafEvent*/
  virtual void OnEvent(mafEventBase *maf_event);

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode* vme);

  /** Builds operation's interface by calling CreateOpDialog() method. */
  void OpRun();

  /** Execute the operation. */
  virtual void OpDo();

protected:

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  virtual void OpStop(int result);

  /* Creates the Incoming directory */
  bool CreateIncomingDirectory();

  /** Creates a cache with the downloaded msf to be imported
  in the current tree: this is an Incoming child dir with name 0, 1, ...
  ie Incoming/0 , Incoming/1*/
  bool CreateIncomingChildDirectory();

  /** Launch a GUI to select remote VME resources to download and write their 
  names in ToDownload.txt file */
  int SelectResourcesToDownloadByGUI();

  /** Fill m_ResourcesToDownloadList IVar from m_ResourcesToDownloadLocalFileName file (default to
  ToDownload.txt generated by SelectResourcesToDownloadByGUI() gui app) */
  int Fill_m_ResourcesToDownloadList_IVar();

  /** Download XML data resource given its name */
  int DownloadXMLDataResource(mafString xmlResourceName);

  /** Build an MSF from downloaded XML data resource and binary data*/
  int BuildMSFFromXMLResource(mafString incomingXMLResourceLocalFileName);

  /** Import Reconstructed MSF*/
  int ImportMSF(mafNode *parentNode = NULL);

  /** Fill linked remote resources IVars*/
  void GetLinkedVMEsResources(mafNode *masterVME);

  /** Get the list of children vmes xml remote resources */
  wxArrayString GetChildrenVMEsResources(mafNode *parentVME);

  /** Download a list of VME and reparent them under outputParentVME in the current tree*/
  int DownloadVMEs(wxArrayString vmeResourcesToDownloadList, mafNode *outputParentVME = NULL);  

  /** Download the whole MSF */
  int DownloadFullMSF(mafNode *node);

  /** Call WS to send to the repository information about VMEs correctly downloaded */
  int DownloadSuccessful(bool successful = true);

  /** Call WS to get xmlRemoteResource linked resources list */
  wxArrayString GetLinksList(mafString xmlRemoteResource);
  /** Retrieve links list from local copy of XML resource. */
  wxArrayString GetLinksListLocal();

  /** Call WS to check if resource that will be downloaded contains child */
  bool HasChild(mafString xmlRemoteResource);
  /** Check if resource that will be downloaded has a child, retrieving info from local copy of XML resource. */
  bool HasChildLocal();

  /** Call WS to check if resource that will be downloaded is a mafVMERoot*/
  bool IsRoot(mafString xmlRemoteResource);

  /** Rename old binary file into new name taking care of new VME id*/
  void CopyDownloadedNonAnimatedVMEDataInTargetMSFDir(mafNode *vmeAttachedAlreadyToTargetTree);

  /** Rename old binary file into new name taking care of new VME id for animated VME*/
  void CopyDownloadedAnimatedVMEDataInTargetMSFDir(mafNode *node);
  
  /** proxy for lhpUploadDownloadVMEHelper->KillThreadedUploaderDownloaderProcess() */
  int KillThreadedUploaderDownloaderProcess();

  int FillRepositoryVar();

  void WaitForDownloadCompletition(int numOfDownloadToWait);
  
  void ImposeAbsMatrixFromTag();
  void ImposeAbsMatrixFromTagInternal(mafNodeIterator *it);

  /** Decrypt file with m_PassPhrase */
  int DecryptFile(mafString fileName);

  std::vector<mafNode*> m_DerivedVMEsVector;
  std::vector<mafNode*> m_VMELinksVector;
  std::vector<mafString> m_DownloadedResourcesList;
  std::vector<mafString> m_TriedToDownloadResourcesList;
  std::vector<mafNode*> m_DownloadedVMEsVector;
  mafVMEGroup *m_DownloadedMSFGroup;
  mafVMEGroup *m_Group;

  lhpUser  *m_User;

  bool m_FillLinkVector;
  bool m_FullMsfDownload;
  bool m_DebugMode;

  int m_DownloadCounter;
  int m_FromSandbox;

  static mafString m_CacheSubdir; //>cache subdirectory
  
  mafString m_IncomingDirectoryABSPath; //directory for xml and binary to send
  mafString m_IncomingCompletePath; //directory for xml and binary to send

  mafString m_VMEUploaderDownloaderDirABSPath; //>directory where the scripts are
  mafString m_PythonInterpreterFullPath; 
  mafString m_PythonwInterpreterFullPath;
  mafString m_MSFDirectoryABSPath; //>directory of original msf

  mafString m_ResourcesToDownloadLocalFileName; 

  mafString m_MockResourcesToDownloadLocalFileName;
  bool m_UseMockResourceFile;

  wxArrayString m_ResourcesToDownloadList;
  wxArrayString m_ResourcesToDownloadParentList;
  wxArrayString m_ResourcesToDownloadTypeList;
  wxArrayString m_ResourcesToDownloadHasChildList;
  wxArrayString m_ListLinkURI;
  wxArrayString m_ListLinkURIInTree;

  mafString m_BinaryRealName;

  mafString m_URISRBFile;
  mafString m_URISRBFileSize;

  mafString m_ProxyURL;
  mafString m_ProxyPort;
  mafString m_ServiceURL;
  mafString m_Repository;
  
  lhpUploadDownloadVMEHelper *m_UploadDownloadVMEHelper;

  int m_ExecutedLXMLChecker;

  int m_NumberOfVMEWithBinary;

  mafString m_PassPhrase;
//   int m_ProcessCounter; // memory saturation workaround
//   in m_StopsCounter; 

  /** friend test class */
  friend class lhpOpDownloadVMERefactorTest;

};
#endif