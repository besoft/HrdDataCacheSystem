/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUIOpenSimSettings.h,v $
Language:  C++
Date:      $Date: 2012-02-07 09:29:43 $
Version:   $Revision: 1.1.2.2 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2008
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpGUIOpModifyOpenSimModelSettings_H__
#define __lhpGUIOpModifyOpenSimModelSettings_H__

#include "mafGUISettings.h"
#include "lhpGuiDefines.h"

//----------------------------------------------------------------------------
// lhpGUIOpenSimSettings :
//----------------------------------------------------------------------------

class LHP_GUI_EXPORT lhpGUIOpenSimSettings : public mafGUISettings
{
public:

  lhpGUIOpenSimSettings(mafObserver *Listener, const mafString &label = _("OpenSim Integration Settings"));
  ~lhpGUIOpenSimSettings(); 

  enum OPENSIM_SETTINGS_WIDGET_ID
  {
	  ID_OPENSIM_DIR = MINID,
	  ID_OPENSIM_VERSION,
  };

  enum m_OpenSimVersion 
  { 
	  OPENSIM_221 = 0 , 
	  OPENSIM_240 = 1 , 
	  OPENSIM_300 = 2, 
	  NUMBER_OF_SUPPORTED_OPENSIM_VERSIONS 
  };

  void OnEvent(mafEventBase *maf_event);

  mafString &GetOpenSimDir();
  mafString &GetOpenSimVersionString();

  void SetOpenSimDir(wxString val);

protected:

  /** Create the GUI for the setting panel.*/
  void CreateGui();

  /** Initialize the application settings.*/
  void InitializeSettings();
 
  void EnableItems( bool enable );
 
  bool IsOpenSimDirExistent();
  mafString m_OpenSimDir;
  mafString m_OpenSimDirDefault;

  int m_OpenSimVersionId;
  int m_OpenSimVersionIdDefault;

  mafString m_OpenSimVersionString;

};
#endif
