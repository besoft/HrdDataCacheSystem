/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUser.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:52 $
Version:   $Revision: 1.1 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpUser_H__
#define __lhpUser_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafString.h"
#include "mafUser.h"
#include "mafObserver.h"
#include "lhpCoreDefines.h"

//----------------------------------------------------------------------------
// forward declarations
//----------------------------------------------------------------------------

/** lhpUser - Used to manage username and password; store them and give basic function to check user's credentials.
This is intended as starting point for user managing. Method for Check User Credentials can be redefined to be
customized at application level according to custom requests.
*/
class LHP_CORE_EXPORT lhpUser : public mafUser
{
public:
  lhpUser(mafObserver *listener = NULL);
  virtual ~lhpUser();

  void SetListener(mafObserver *Listener)	{m_Listener = Listener;};
  mafObserver *GetListener() {return m_Listener;};

  /** Function to be customized at application level.
  By default open Login Dialog if the user did not inserted any information.*/
  bool CheckUserCredentials();

  /** return true if user is authenticated on BiomedTown*/
  bool IsAuthenticated();

protected:
  bool ExecuteAuthenticationScript();

  bool m_IsAuthenticated;

  mafString m_VMEUploaderDownloaderDir; //>directory where the scripts are
  mafString m_FileName; //>script file name
  mafString m_PythonExe; //>python  executable
  mafString m_PythonwExe; //>python  executable
  mafObserver *m_Listener;

};
#endif
