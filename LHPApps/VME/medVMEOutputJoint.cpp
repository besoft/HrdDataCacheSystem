/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medVMEOutputJoint.cpp,v $
  Language:  C++
  Date:      $Date: 2012-03-20 15:32:05 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011-2012
  University of West Bohemia
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "mafDbg.h"
#include "medVMEOutputJoint.h"
#include "medVMEJoint.h"
#include "mafGUI.h"

#include "vtkPolyData.h"


//-------------------------------------------------------------------------
mafCxxTypeMacro(medVMEOutputJoint)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
medVMEOutputJoint::medVMEOutputJoint()
//-------------------------------------------------------------------------
{
	m_JointOutput = vtkPolyData::New();	//empty dummy output
	//this is required since we want to visualize the joint but MAF won't call our
	//visual pipe without VTK data to be visualized

	for (int i = 0; i < 3; i++)
	{
		m_AbsPosition[i] = 
			m_RotateAxis[0][i] = m_RotateAxis[1][i] = m_RotateAxis[2][i] = 
			m_ActorsAxis[0][i] = m_ActorsAxis[1][i] =
			m_CurrentAngles[i] = 0.0;
		m_LimitAngles[i][0] = m_LimitAngles[i][1] = 0.0;
	}
	
	m_ValidRotateAxes = 0;
}

//-------------------------------------------------------------------------
medVMEOutputJoint::~medVMEOutputJoint()
//-------------------------------------------------------------------------
{
	vtkDEL(m_JointOutput);
}

//-------------------------------------------------------------------------
mafGUI* medVMEOutputJoint::CreateGui()
//-------------------------------------------------------------------------
{
	assert(m_Gui == NULL);
	m_Gui = mafVMEOutput::CreateGui();

	m_Gui->Divider(1);
	m_Gui->Label(_("Position: "), &m_szPosition, true);
	m_Gui->Label(_("Axes: "), &m_szAxes, true);
	m_Gui->Label(_("Actors: "), &m_szActors, true);
	m_Gui->Divider(1);
	m_Gui->Label(_("Angles: "), &m_szAngles, true);	
	m_Gui->Label(_("Limits: "), &m_szLimits, false);	

	this->Update();	//update labels
	return m_Gui;
}

//-------------------------------------------------------------------------
void medVMEOutputJoint::Update()
//-------------------------------------------------------------------------
{
	_VERIFY_RET(m_VME != NULL);
	m_VME->Update();	//this should change our current state

	m_szPosition = wxString::Format(_("[%.2f, %.2f, %.2f]"), 
		m_AbsPosition[0], m_AbsPosition[1], m_AbsPosition[2]);

	m_szAxes.Erase(0);
	for (int i = 0; i < m_ValidRotateAxes; i++) {
		m_szAxes.Append(wxString::Format(_("[%.2f, %.2f, %.2f], "), 
			m_RotateAxis[i][0], m_RotateAxis[i][1], m_RotateAxis[i][2]));
	}		

	m_szActors = wxString::Format(_("[%.2f, %.2f, %.2f], [%.2f, %.2f, %.2f]"), 
		m_ActorsAxis[0][0], m_ActorsAxis[0][1], m_ActorsAxis[0][2],
		m_ActorsAxis[1][0], m_ActorsAxis[1][1], m_ActorsAxis[1][2]);

	m_szAngles.Erase(0);
	for (int i = 0; i < m_ValidRotateAxes; i++) {
		m_szAngles.Append(wxString::Format(_("%.2f°, "), m_CurrentAngles[i]));
	}

	m_szLimits.Erase(0);
	for (int i = 0; i < m_ValidRotateAxes; i++) {
		m_szLimits.Append(wxString::Format(_("[%.2f°, %.2f°], "), m_LimitAngles[i][0], m_LimitAngles[i][1]));
	}

	if (m_Gui != NULL)
		m_Gui->Update();
}

//-------------------------------------------------------------------------
// Returns a VTK dataset corresponding to the current time. 
/*virtual*/ vtkDataSet* medVMEOutputJoint::GetVTKData()
//-------------------------------------------------------------------------
{
	return m_JointOutput;
}