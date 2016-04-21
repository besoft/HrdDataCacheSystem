/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUploadDownloadVMEHelper.h,v $
Language:  C++
Date:      $Date: 2012-02-01 14:09:50 $
Version:   $Revision: 1.1.2.5 $
Authors:   Stefano Perticoni
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

#ifndef __lhpUploadDownloadVMEHelper_H__
#define __lhpUploadDownloadVMEHelper_H__

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

//----------------------------------------------------------------------------
// lhpUploadDownloadVMEHelper :
//----------------------------------------------------------------------------
/** 
VME Uploader and Downloader Helper Class
*/
class LHP_OPERATIONS_EXPORT lhpUploadDownloadVMEHelper : public mafObject
{
public:
  mafTypeMacro(lhpUploadDownloadVMEHelper,mafObject);

  lhpUploadDownloadVMEHelper(){lhpUploadDownloadVMEHelper(NULL);};
  lhpUploadDownloadVMEHelper(mafObserver *listener);
  ~lhpUploadDownloadVMEHelper();

  /** Return the vertical application user that is logged */
  lhpUser *GetUser();

  /** Get the application python.exe interpreter full path*/
  mafString GetPythonInterpreterFullPath();
  
  /** Get the application pythonw.exe interpreter full path*/
  mafString GetPythonwInterpreterFullPath();

  /** Destroy Threaded Uploader Downloader process (also destroy the owned Uploader
  Downloader GUI). Return 0 if ok, -1 on error.*/
  int KillThreadedUploaderDownloaderProcess();

  /** Check if the Upload/Download Manager python process exists already */
  bool ExistsThreadedUploaderDownloader();

  /** Create an instance of the threaded Upload Download manager if it not exists already,
  return True if the instance has been created otherwise False*/
  bool CreateThreadedUploaderDownloader();
  
  /** Return the threaded Uploader Downloader process ID or -1 if the process does not exist 
  (this is the server process which can create Upload and Download threads on demand through
  client requests)*/
  long GetThreadedUploaderDownloaderProcessID();

  /** Check if lhbbuilder software version is up to date in order to a allow vme uploading */
  bool IsClientSoftwareVersionUpToDate();

  /** Save connection configuration file for connection
  REFACTOR THIS: The generated vmeUploaderConnectionConfiguration.conf file is 
  used by Python but this dependency should be removed by passing parameters to the python
  script*/
  void SaveConnectionConfigurationFile();
 
  void RemoveConnectionConfigurationFile();

  void SetListener(mafObserver *listener) {m_Listener = listener;};
  mafObserver *GetListener() {return m_Listener;};

  mafString GetServiceURL(bool groupChoice);
private:

  mafString m_VMEUploaderDownloaderDirABSPath; //> python scripts directory 
  mafString m_ConnectionConfigurationFileName;
  mafString m_ProxyURL;
  mafString m_ProxyPort;
  mafString m_ServiceURL;
  mafObserver *m_Listener;
  static mafString m_ConfigFileName;

  mafString m_GroupIdFileName;
  
  bool m_DebugMode;
};
#endif