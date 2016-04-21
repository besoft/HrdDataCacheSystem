/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUIOpenSimSettings.cpp,v $
Language:  C++
Date:      $Date: 2011-09-29 14:25:19 $
Version:   $Revision: 1.1.2.1 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2008
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpGUIOpenSimSettings.h"
#include "mafCrypt.h"

#include "mafDecl.h"
#include "mafGUI.h"
#include "lhpUtils.h"

lhpGUIOpenSimSettings::lhpGUIOpenSimSettings(mafObserver *Listener, const mafString &label):
mafGUISettings(Listener, label)
{

  #ifdef USE_OPENSIM_API 
	m_OpenSimDirDefault = "c:\\OpenSim 3.0\\";
  #else
	m_OpenSimDirDefault = "c:\\OpenSim2.2.1\\";
  #endif
  
  m_OpenSimDir = "UNDEFINED_m_OpenSimDirDefault";

  m_OpenSimVersionId = -1; // UNDEFINED

  #ifdef USE_OPENSIM_API 
    m_OpenSimVersionIdDefault = OPENSIM_300;
  #else
	m_OpenSimVersionIdDefault = OPENSIM_221;
  #endif


  m_OpenSimVersionString = "UNDEFINED_m_OpenSimVersionString";
  // read the configuration file to fill the iVars
  InitializeSettings();
}

lhpGUIOpenSimSettings::~lhpGUIOpenSimSettings()
{
}

void lhpGUIOpenSimSettings::CreateGui()
{
	m_Gui = new mafGUI(this);
	m_Gui->Label(_("OpenSim Integration Settings"));
	m_Gui->Label(_(""));
	wxString choices[NUMBER_OF_SUPPORTED_OPENSIM_VERSIONS] = {"2.2.1" , "2.4.0" , "3.0.0"};
	m_Gui->Label("Supported OpenSim Version");
	m_Gui->Combo(ID_OPENSIM_VERSION, "" , &m_OpenSimVersionId, NUMBER_OF_SUPPORTED_OPENSIM_VERSIONS, choices );
	m_Gui->Label(_(""));

#ifdef USE_OPENSIM_API
	m_Gui->Enable(ID_OPENSIM_VERSION, false);
#else
	m_Gui->Enable(ID_OPENSIM_VERSION, true);
#endif

	m_Gui->Label("BEWARE: After changing the OpenSim");
	m_Gui->Label("installation directory:");
	m_Gui->Label("1) System PATH must be updated accordingly ");
	m_Gui->Label("for example to C:\\OpenSim_NewInstallDir\\bin");
	m_Gui->Label("if your new installation dir is");
	m_Gui->Label("C:\\OpenSim_NewInstallDir\\");
	m_Gui->Label("2) Application must be restarted ");
	m_Gui->Label("");
	m_Gui->Label(_("OpenSim installation directory"));
	m_Gui->String(ID_OPENSIM_DIR, "" , &m_OpenSimDir);
	m_Gui->Divider(2);
	EnableItems(true);
}

void lhpGUIOpenSimSettings::EnableItems( bool enable )
{
	m_Gui->Enable(ID_OPENSIM_DIR, enable);
}


void lhpGUIOpenSimSettings::InitializeSettings()
{
  wxString stringItem;
 
  // search for the registry value
  if(m_Config->Read("m_OpenSimDir", &stringItem))
  {
	// if found set the Ivar
	  
    m_OpenSimDir = stringItem;
  }
  else
  {
	// otherwise write the IVar default value
	m_OpenSimDir = m_OpenSimDirDefault;
    m_Config->Write("m_OpenSimDir",m_OpenSimDir);
  }

  #ifdef USE_OPENSIM_API 
     m_OpenSimVersionId=OPENSIM_300;
	 m_Config->Write("m_OpenSimVersionId",m_OpenSimVersionId);
  #else

    int intItem;

	if(m_Config->Read("m_OpenSimVersionId", &intItem))
	{
		// if found set the Ivar

		m_OpenSimVersionId = intItem;
	}
	else
	{
		// otherwise write the IVar default value
		m_OpenSimVersionId = m_OpenSimVersionIdDefault;
		m_Config->Write("m_OpenSimVersionId",m_OpenSimVersionId);
	}
  
  #endif

  m_Config->Flush();
}

void lhpGUIOpenSimSettings::OnEvent( mafEventBase *maf_event )
{
	switch(maf_event->GetId())
	{
	case ID_OPENSIM_DIR:
		if (IsOpenSimDirExistent())
		{
			m_Config->Write("m_OpenSimDir",m_OpenSimDir.GetCStr());
		}
		break;

	case ID_OPENSIM_VERSION:
		{
			m_Config->Write("m_OpenSimVersionId", m_OpenSimVersionId);
		}
		break;

	default:
		mafEventMacro(*maf_event);
		break; 
	}

	m_Config->Flush();

}

void lhpGUIOpenSimSettings::SetOpenSimDir( wxString val )
{
	m_OpenSimDir = val;
	m_Config->Write("m_OpenSimDir",m_OpenSimDir);
}

mafString &lhpGUIOpenSimSettings::GetOpenSimDir()
{
	wxString stringItem;

	// search for the registry value
	m_Config->Read("m_OpenSimDir", &stringItem);

	m_OpenSimDir = stringItem;
	return m_OpenSimDir;
}

mafString & lhpGUIOpenSimSettings::GetOpenSimVersionString()
{
	int opensimVersionId;

	// search for the registry value
	m_Config->Read("m_OpenSimVersionId", &opensimVersionId);

	mafString openSimVersionString = "OPENSIM_VERSION_UNDEFINED";

	if (opensimVersionId == OPENSIM_221)
	{
		openSimVersionString =  "OPENSIM_221";
	}
	else if (opensimVersionId == OPENSIM_240)
	{
		openSimVersionString = "OPENSIM_240";
	}
	else if (opensimVersionId == OPENSIM_300)
	{
		openSimVersionString = "OPENSIM_300";
	}
	
	m_OpenSimVersionString = openSimVersionString;
	
	return m_OpenSimVersionString;
}

bool lhpGUIOpenSimSettings::IsOpenSimDirExistent()
{
	bool openSimDirExists = wxDirExists(m_OpenSimDir.GetCStr());
	if (!openSimDirExists)
	{
		std::ostringstream stringStream;
		stringStream  << "OpenSim directory "<< m_OpenSimDir.GetCStr() << " not found. Setting directory to default: " << m_OpenSimDirDefault.GetCStr() << std::endl;
		mafLogMessage(stringStream.str().c_str());

		m_OpenSimDir = m_OpenSimDirDefault;

		assert(wxDirExists(m_OpenSimDir));

		m_Config->Write("m_OpenSimDir",m_OpenSimDir);
		if (m_Gui)
		{
			m_Gui->Update();
		}
	}

	return openSimDirExists;
}