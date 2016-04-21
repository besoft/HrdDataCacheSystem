/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewSurfacePlot.cpp,v $
Language:  C++
Date:      $Date: 2011-05-27 07:52:38 $
Version:   $Revision: 1.1.2.2 $
Authors:   Alberto Losi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#include "lhpDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpViewSurfacePlot.h"
#include "mafVMEScalarMatrix.h"
#include "vtkDoubleArray.h"
#include "mafEventSource.h"
#include "mafVMEOutputScalarMatrix.h"
#include "mafGUI.h"

#include "lhpPipeSurfacePlot.h"

#include <vtkCamera.h>


#include <string>
//#include <vnl/vnl_vector.h>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpViewSurfacePlot);
//----------------------------------------------------------------------------
/*
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
*/


//----------------------------------------------------------------------------
lhpViewSurfacePlot::lhpViewSurfacePlot(wxString label, int camera_position, bool show_axes, bool show_grid, bool show_ruler, int stereo)
:mafViewVTK(label,camera_position,show_axes,show_grid, show_ruler, stereo)
//----------------------------------------------------------------------------
{
  //m_nRow = 0;
  //m_nCol = 0;

  //m_SelectIDIK = 0;
  
	m_ScaleX = 10000;
  m_ScaleY = 300;
  m_ScaleZ = 5;
  m_RenderMode = 0;
  //ReadTestFile("D:/proj/NMS/infovis/test/1_ID.sto");
}
//----------------------------------------------------------------------------
lhpViewSurfacePlot::~lhpViewSurfacePlot()
//----------------------------------------------------------------------------
{
  cppDEL(m_Gui);
  //m_Instantiated.clear();
}
//----------------------------------------------------------------------------
//mafView *lhpViewSurfacePlot::Copy(mafObserver *Listener)
mafView*  lhpViewSurfacePlot::Copy(mafObserver *Listener, bool lightCopyEnabled)
//----------------------------------------------------------------------------
{
  lhpViewSurfacePlot *v = new lhpViewSurfacePlot(m_Label, m_CameraPositionId, m_ShowAxes,m_ShowGrid, m_ShowRuler, m_StereoType);
  v->m_Listener = Listener;
  v->m_Id = m_Id;
  v->m_PipeMap = m_PipeMap;
  v->Create();
  return v;
}
//----------------------------------------------------------------------------
void lhpViewSurfacePlot::Create()
//----------------------------------------------------------------------------
{
  Superclass::Create();
  //m_Rwi->m_Camera->ParallelProjectionOn();
  //mafLogMessage("in lhpViewSurfacePlot Create()");
}

/*
void lhpViewSurfacePlot::ReadTestFile(char * fname)
{
	std::ifstream inputfile;
	inputfile.open(fname, std::ifstream::in);
	if (!inputfile)
	{
		throw std::exception("Open file failed");
	}

	std::string line;
	std::string header;

	 while (!inputfile.eof())
	 {
		  std::getline(inputfile, line);

		  if (line.find("nRows=") != std::string::npos)
		  {
			  std::string sub = line.substr(6,line.length() - 6);
			  m_nRow = atoi(sub.c_str());
		  }
		  else if (line.find("nColumns=") != std::string::npos)
		  {
			  std::string sub = line.substr(9,line.length() - 9);
			  m_nCol = atoi(sub.c_str());
		  }
		  else if (line=="endheader")
		  {
			   std::getline(inputfile, header);
			   break;
		  }
	  }

	 mat = new double[m_nRow * m_nCol];
	 double d;
	 // read matrix
	 for (int i = 0; i < m_nRow; i++) 
	{
		 for (int j = 0; j < m_nCol; j++ )
		 {
			  inputfile >> d;
			  mat[ i * m_nCol + j ] = d;
		 }
		 //mafLogMessage("%d ", i);
	 }
	 //mafLogMessage("\n");

	inputfile.close();

}
*/

//-------------------------------------------------------------------------
mafGUI *lhpViewSurfacePlot::CreateGui()
//-------------------------------------------------------------------------
{
 
	m_Gui = mafView::CreateGui(); //new mafGUI(this);

	//radio button for selecting Inverse Dynamics or Inverse Kinetics
	//wxString IDIK_Choices[] = { _("Dynamics"),  _("Kinetics")  };
	//m_Gui->Radio(ID_SELECT_ID_IK,_(""), & m_SelectIDIK, 2, IDIK_Choices, 2, "Select ID or IK");

	m_Gui->Divider(1);
	m_Gui->Float(ID_SCALE_X,_("Scale Time"),&m_ScaleX);
	m_Gui->Float(ID_SCALE_X,_("Scale Variable Intervals"),&m_ScaleY);
	m_Gui->Float(ID_SCALE_X,_("Scale Values"),&m_ScaleZ);
	m_Gui->Button(ID_CHANGE_SCALE,"Update Scale","", "");
	m_Gui->Divider(1);

	wxString RenderMode[] = {  _("Surface"),   _("Wireframe"), _("Points")  };
	m_Gui->Radio(ID_RENDER_MODE,_(""), & m_RenderMode, 
		3, RenderMode, 2, "Choose Render Mode");

	m_Gui->Divider(2);
	m_Gui->AddGui(m_Rwi->GetGui());

	return m_Gui;
}
void lhpViewSurfacePlot::EnableGUI(bool b)
{
	//m_Gui->Enable(ID_SELECT_ID_IK, b);
	m_Gui->Enable(ID_SCALE_X, b);
	m_Gui->Enable(ID_SCALE_Y, b);
	m_Gui->Enable(ID_SCALE_Z, b);
	m_Gui->Enable(ID_CHANGE_SCALE, b);

	m_Gui->Update();
}

//----------------------------------------------------------------------------
void lhpViewSurfacePlot::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
	case ID_SELECT_ID_IK:
		//m_Pipe->OnChangeDataType(m_SelectIDIK);
		//mafEventMacro(mafEvent(this,CAMERA_UPDATE));
		//mafEventMacro(*maf_event);
		break;

    //case ID_SCALE_X:
    //case ID_SCALE_Y:
	//case ID_SCALE_Z:
	case ID_CHANGE_SCALE:
      {
		  m_Pipe->OnChangeAxisScale(m_ScaleX, m_ScaleY, m_ScaleZ);
		  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
	case ID_RENDER_MODE:
		m_Pipe->OnChangeRenderMode(m_RenderMode);
		mafEventMacro(mafEvent(this,CAMERA_UPDATE));
		break;

	case PIPE_PLOT_DESTROY:
		m_Pipe = NULL;
		break;

    }
  }
  mafEventMacro(*maf_event);
}

// show data VME
//----------------------------------------------------------------------------
void lhpViewSurfacePlot::VmeShow(mafNode *node, bool show)
//----------------------------------------------------------------------------
{
	// only handle mafVMESurface
	if(node->IsA("mafVMESurface"))
	{
		if(show)
		{
			Superclass::VmeShow(node, show);
			if (NULL != m_Pipe) {
				//m_Pipe->RemoveAll();
				VmeRemove(m_Node);
			}

			m_Node = node;
			m_Pipe = lhpPipeSurfacePlot::SafeDownCast(
				lhpViewSurfacePlot::SafeDownCast(this)->GetNodePipe(node));
			
			// set default parameters
			//m_Pipe->SetDataType(m_SelectIDIK);
			m_Pipe->SetRenderMode(m_RenderMode);
			m_Pipe->SetAxisScale(m_ScaleX, m_ScaleY, m_ScaleZ);
			// create plotting
			m_Pipe->UpdatePlot();

			EnableGUI(true);
		}
		else
		{
			//m_Pipe->RemoveAll();
			Superclass::VmeShow(node, show);
		}
		mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }
}