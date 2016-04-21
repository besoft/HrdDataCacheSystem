/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpGUISettingsOpUploadMultiVMERefactor.h,v $
Language:  C++
Date:      $Date: 2009-07-17 12:59:19 $
Version:   $Revision: 1.1.2.1 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpGUISettingsOpUploadMultiVMERefactor_H__
#define __lhpGUISettingsOpUploadMultiVMERefactor_H__

#include "mafGUISettings.h"
#include "lhpGuiDefines.h"

class mafGUICheckListBox;

/**
  Class Name : lhpGUISettingsOpUploadMultiVMERefactor.
  Class that contain specific variables regarding  multiple upload operation\
  that can be changed from Application Settings.
*/
class LHP_GUI_EXPORT lhpGUISettingsOpUploadMultiVMERefactor : public mafGUISettings
{
public:

  /** constructor.*/
	lhpGUISettingsOpUploadMultiVMERefactor(mafObserver *Listener, \
  const mafString &label = _("Upload Multi VME"));

  /** destructor.*/
	~lhpGUISettingsOpUploadMultiVMERefactor(); 

  /** GUI IDs*/
	enum SETTINGS_WIDGET_ID
	{
		ID_STEP = MINID,
	};


	/** Answer to the messages coming from interface. */
	void OnEvent(mafEventBase *maf_event);

  /** Retrieve the sleep time */
  int GetSleepTimeBetweenTwoVMEUploadInSeconds(){return m_SleepTimeBetweenTwoVMEUploadInSeconds;};


protected:
	/** Create the GUI for the setting panel.*/
	void CreateGui();

	/** Initialize the application settings.*/
	void InitializeSettings();

	double m_SleepTimeBetweenTwoVMEUploadInSeconds;
};
#endif
