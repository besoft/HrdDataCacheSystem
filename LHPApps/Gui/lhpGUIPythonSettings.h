/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUIPythonSettings.h,v $
Language:  C++
Date:      $Date: 2010-02-23 15:53:07 $
Version:   $Revision: 1.1.1.1.2.2 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2008
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpPythonSettings_H__
#define __lhpPythonSettings_H__

#include "mafGUISettings.h"
#include "lhpGuiDefines.h"

//----------------------------------------------------------------------------
// lhpPythonSettings :
//----------------------------------------------------------------------------
/**
  This class represents the panel attached to settings dialog, regarding 
  python interpreter used at runtime
  It Contains:
  - Python.exe interpreter full path
  - Pythonw.exe interpreter full path

*/
class LHP_GUI_EXPORT lhpGUIPythonSettings : public mafGUISettings
{
public:
	lhpGUIPythonSettings(mafObserver *Listener, const mafString &label = _("Python Settings"));
	~lhpGUIPythonSettings(); 

  enum APP_SETTINGS_WIDGET_ID
  {
    ID_PYTHON_EXE = MINID,
    ID_PYTHONW_EXE,
    ID_FORCE_EMBEDDED_PYTHON,
  };

  /** Answer to the messages coming from interface. */
  void OnEvent(mafEventBase *maf_event);

  /** Set python.exe interpreter used at runtime*/
  void SetPythonExe(const char *pythonExe){m_PythonExe = pythonExe;};
  
  /** Get python.exe interpreter used at runtime*/
  mafString &GetPythonExe(){return m_PythonExe;};

  /** Set pythonw.exe interpreter used at runtime*/
  void SetPythonwExe(const char *pythonwExe){m_PythonwExe = pythonwExe;};

	/** Get pythonw.exe interpreter used at runtime*/
  mafString &GetPythonwExe() {return m_PythonwExe;};


protected:
  /** Create the GUI for the setting panel.*/
  void CreateGui();

  /** Initialize the application settings.*/
  void InitializeSettings();

  /** Used to enable/disable gui items*/
  void EnableItems(bool enable);

  mafString   m_PythonExe;///< python.exe full path
  mafString   m_PythonwExe;///< pythonw.exe full path
  
  bool Is_m_PythonwExe_Existent();
  bool Is_m_PythonExe_Existent();

  mafString m_PythonExeDefaultInterpreter;
  mafString m_PythonwExeDefaultInterpreter;
  
  bool m_EmbeddedPythonInterpreterAvailable;
  int m_ForceEmbeddedPythonRuntime;
};
#endif
