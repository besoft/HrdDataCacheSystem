/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpUploadVMERefactor.h,v $
Language:  C++
Date:      $Date: 2011-03-30 12:24:48 $
Version:   $Revision: 1.1.1.1.2.14 $
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

#ifndef __lhpOpUploadVMERefactor_H__
#define __lhpOpUploadVMERefactor_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpUser;
class mafVME;
class lhpUploadDownloadVMEHelper;

//----------------------------------------------------------------------------
// lhpOpUploadVMERefactor :
//----------------------------------------------------------------------------
/**Upload Single VME */
// TODO: REFACTOR THIS 
// BEWARE Heavy Refactoring in progress!!!!!!!!!!!!

class LHP_OPERATIONS_EXPORT lhpOpUploadVMERefactor: public mafOp
{
public:

  /** Upload also vme children? Default to false */
  void SetWithChild(bool withChild) {m_WithChild = withChild;};
  bool GetWitChild() {return m_WithChild;};

  /** File to store uploaded XML resources URI for rollback */
  void SetXMLUploadedResourcesRollBackLocalFileName(mafString file) {m_XMLUploadedResourcesRollBackLocalFileName = file;};
  mafString GetXMLUploadedResourcesRollBackLocalFileName() {return m_XMLUploadedResourcesRollBackLocalFileName;};

  /** Is this the last resource that will feed the uploader in one instance of the 
  multiple upload operation? */
  void SetIsLast(bool isLast) {m_IsLast = isLast;};
  bool GetIsLast() {return m_IsLast;};

  /** Upload */
  int Upload(bool isRootInRepository = false);

  /** Return the remote URI where the XML resource has been stored after calling Upload()*/
  mafString GetRemoteXMLResourceURI() {return m_RemoteXMLResourceURI;};

  /** Set the python interpreter; to be used if the op is instantiated without gui */
  void SetPythonInterpreterFullPath(mafString pythonInterpreterFullPath) {m_PythonInterpreterFullPath = pythonInterpreterFullPath;};
  mafString GetPythonInterpreterFullPath() {return m_PythonInterpreterFullPath;};

  /** Set the pythonw interpreter; to be used if the op is instantiated without gui */
  void SetPythonwInterpreterFullPath(mafString pythonwInterpreterFullPath) {m_PythonwInterpreterFullPath = pythonwInterpreterFullPath;};
  mafString GetPythonwInterpreterFullPath() {return m_PythonwInterpreterFullPath;};

  /** Set the user; to be used if the op is instantiated without gui */
  void SetUser(lhpUser *user) {m_User = user;};
  lhpUser *GetUser(lhpUser *user) {return m_User;};
  
  /** Set the folder containing the input msf file; to be used if the op is instantiated without gui */
  void SetMSFFileABSFolder(mafString MSFFileABSFolder) {m_MSFFileABSFolder = MSFFileABSFolder;};
  mafString GetMSFFileABSFolder() {return m_MSFFileABSFolder;};;

  /** Set the input msf file name; to be used if the op is instantiated without gui */
  void SetMSFFileABSFileName(mafString MSFFileABSFileName) {m_MSFFileABSFileName = MSFFileABSFileName;};
  mafString GetMSFFileABSFileName() {return m_MSFFileABSFileName;};

  /** Set boolean variable to switch on/off groups choice widget. */
  void SetGroupsChoice(bool groupsChoice) {m_GroupsChoice = groupsChoice;};
  bool GetGroupsChoice() {return m_GroupsChoice;};

  /** Set repository URL to create XML resources and to upload. */
  void SetRepositoryURL(const char *repositoryURL) {m_RepositoryURL = mafString(repositoryURL); };
  mafString GetRepositoryURL() {return m_RepositoryURL; } ;

  lhpOpUploadVMERefactor(wxString label = "Upload Vme");
  ~lhpOpUploadVMERefactor(); 

  mafTypeMacro(lhpOpUploadVMERefactor, mafOp);

  /** Set debug modality */
  void SetDebugMode(bool debugMode){m_DebugMode = debugMode;};

  mafOp* Copy();

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode* vme);

  /** Builds operation's interface by calling CreateOpDialog() method. */
  void OpRun();

  /** Execute the operation. */
  virtual void OpDo();

  //////////////////////////////////////////////////////////////////////////
  int InitializeToUpload(bool isRootInRepository = false);
  int UploadAlreadyInitilized(bool isRootInRepository = false);
  void SetParserObtimizationOn(){m_ParserOptimization = true;};
  void SetParserObtimizationOff(){m_ParserOptimization = false;};
  wxString m_CurrentCacheChildABSFolder; //>current cache subdirectory
  //////////////////////////////////////////////////////////////////////////

  /** Generate a passphrase alphanumeric with a cutom number of characters */
  static mafString GeneratePassPhrase(int numOfCharacters = 32);

  /** set passphrase */
  void SetPassPhrase(mafString passPhrase){m_PassPhrase = passPhrase;};

protected:

  /** Create one upload thread */
  void CreateUploadThread( mafString &hasLink, mafString &uploadWithChildren, mafString &isLastResource );
  
  /** Create an XML Resource on the repository: return MAF_OK or MAF_ERROR */
  int CreateXMLResource();

  int BuildMSFForVMEToBeUploaded();

  /** Fill m_MSFFileABSFolder ivar through logic*/
  void FillMSFFileABSFolderIVar();

  /** Save information information about VME link*/
  void StoreInputVMELinksInfo();

  /** Generate auto tags and manual tags list files from XML lhdl dictionary:
  an automatic tag that the factory cannot handle will become manual*/
  int GeneratesHandledAndUnhandledPlusManualTagsFileFromXMLDictionary();

  static mafString GetXMLMasterDictionaryFileName(mafString  dictionaryAbsFolder, mafString dictionaryFileNamePrefix);
  
  static int FillAutoTagsAndManualTagsVARsFromXMLMasterDictionaryFile(\
    mafString pythonInterpreter, \
    mafString vmeUpDownDirAbsFolder, mafString xmlDictionaryLocalFilePrefix, \
    wxArrayString &outAutoTags, wxArrayString &outManualTags);

  /** Try to handle auto tags through tags factory and convert unhandled 
  to manual tags ie to be filled by the user*/
  void HandleAutoTagsTroughFactory();

	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	virtual void OpStop(int result);

  /* Check for Master Cache and Outgoing directories: create them if non existent*/
  bool MasterCacheFolderAndOutgoingFolderAvailable();

  /** This method creates on filesystem a cache with msf and binary data that must be uploaded.
  Return true if succeed */
  bool CreateChildCache();

  /** This method copies in cache the msf and binary data that must be uploaded */
  bool CopyInputVMEInCurrentChildCache();

  int CopyPythonEditedTagsIntoOriginalVME();

  /** Crypt the data to be uploaded */
  int CryptingData(mafString fileName);

  /** send passphrase crypted to the server */
  int SendPassPhrase();
   
  mafString m_CacheMasterFolderABSName; //>cache subfolders parent directory
  static mafString m_CacheChildFolderLocalName; //>cache subdirectory
  //wxString m_CurrentCacheChildABSFolder; //>current cache subdirectory
  
  mafString m_OutgoingFolderABSName; //directory for xml and binary to send

  mafString m_VMEUploaderDownloaderABSFolderName; //>directory where the scripts are
  
  mafString m_PythonInterpreterFullPath; //>python  executable
  mafString m_PythonwInterpreterFullPath; //>pythonw  executable
  mafString m_MSFFileABSFolder; //>directory of original msf
	mafString m_MSFFileABSFileName; //>absolute path of original msf
  
  mafString m_MasterXMLDictionaryFilePrefix; 
  mafString m_MasterXMLDictionaryLocalFileName;
  mafString m_SubXMLDictionaryFilePrefix;
  mafString m_SubXMLDictionaryFileName;
  mafString m_AssembledXMLDictionaryFileName;
  mafString m_SubDictionaryBuildingCommand;

  mafString m_HandledAutoTagsLocalFileName;
  mafString m_AutoTagsListFromXMLDictionaryLocalFileName; 
  mafString m_ManualTagsListFromXMLDictionaryLocalFileName;
  
  bool m_WithChild;
  bool m_IsLast;
  mafString m_XMLUploadedResourcesRollBackLocalFileName;
  bool m_IsBinaryDataPresent;
  mafString m_RemoteXMLResourceURI;

  lhpUser  *m_User; 

  mafString m_ConnectionConfigurationFileName;
  mafString m_ProxyURL;
  mafString m_ProxyPort;
  mafString m_RepositoryURL;
  bool m_GroupsChoice;

  lhpUploadDownloadVMEHelper *m_UploadDownloadVMEHelper;

private:

 
  int m_SubId;
  mafVME *m_CacheVme;

  int m_SubdictionaryId;
  wxArrayString m_AutoTagsList;
  wxArrayString m_ManualTagsList;
  wxArrayString m_UnhandledAutoTagsListFromFactory; ///< the factory was not able to handle these tags
  wxArrayString m_HandledAutoTagsListFromFactory; ///< tags handled by the factory  
  wxString m_UnhandledPlusManualTagsLocalFileName;
  wxString m_NodeName;
  FILE *m_ProxyFile;
  bool m_HasLink;
  bool m_HasChild;
  bool m_DebugMode;

  std::vector<mafNode*> m_LinkNode;
  std::vector<mafString> m_LinkName;

  /** test friend */
  friend class lhpOpUploadMultiVMERefactorTest;

  
  bool m_SendPassPhrase;
  bool m_TestThreadedUploaderDownloaderApplicationCreated;

  //////////////////////////////////////////////////////////////////////////
  mafString m_HasLinkStr;
  mafString m_IsLastResource;
  mafString m_UploadWithChildren;
  bool m_ParserOptimization;
  //////////////////////////////////////////////////////////////////////////

  mafString m_PassPhrase;
  mafString m_PublicKey[6];
  
};

#endif