/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewAnalogGraphXY.cpp,v $
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

#include "lhpViewAnalogGraphXY.h"
#include "mafVMEScalarMatrix.h"
#include "vtkDoubleArray.h"
#include "mafEventSource.h"
#include "mafVMEOutputScalarMatrix.h"
#include "mafGUI.h"


#include <vnl/vnl_vector.h>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpViewAnalogGraphXY);
//----------------------------------------------------------------------------

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
lhpViewAnalogGraphXY::lhpViewAnalogGraphXY(wxString label, int camera_position, bool show_axes, bool show_grid, bool show_ruler, int stereo)
:mafViewVTK(label,camera_position,show_axes,show_grid, show_ruler, stereo)
//----------------------------------------------------------------------------
{
  m_DataMax = 0;
  m_DataMin = 0;
  m_XMax = 0;
  m_XMin = 0;
  m_DataManualRange[0] = 0;
  m_DataManualRange[1] = 0;
  m_XManualRange[0] = 0;
  m_XManualRange[1] = 0;
  m_NumberOfDecimals = 2;
}
//----------------------------------------------------------------------------
lhpViewAnalogGraphXY::~lhpViewAnalogGraphXY()
//----------------------------------------------------------------------------
{
  cppDEL(m_Gui);
  m_Instantiated.clear();
}
//----------------------------------------------------------------------------
mafView *lhpViewAnalogGraphXY::Copy(mafObserver *Listener)
//----------------------------------------------------------------------------
{
  lhpViewAnalogGraphXY *v = new lhpViewAnalogGraphXY(m_Label, m_CameraPositionId, m_ShowAxes,m_ShowGrid, m_ShowRuler, m_StereoType);
  v->m_Listener = Listener;
  v->m_Id = m_Id;
  v->m_PipeMap = m_PipeMap;
  v->Create();
  return v;
}
//----------------------------------------------------------------------------
void lhpViewAnalogGraphXY::Create()
//----------------------------------------------------------------------------
{
  Superclass::Create();
}
//-------------------------------------------------------------------------
mafGUI *lhpViewAnalogGraphXY::CreateGui()
//-------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->String(ID_AXIS_NAME_X,_("X Title"), &m_TitileX,_("Set X axis name"));
  m_Gui->String(ID_AXIS_NAME_Y,_("Y Title"), &m_TitileY,_("Set Y axis name"));
  m_Gui->Divider(1);

//   if (m_XMax == 0.0)
//   {
//     m_XMax = VTK_DOUBLE_MAX;
//   }

  m_Gui->VectorN(ID_RANGE_X, _("Range X"), m_XManualRange, 2); //, m_XMin, m_XMax);
  m_Gui->VectorN(ID_RANGE_Y, _("Range Y"), m_DataManualRange, 2); // m_DataMin, m_DataMax);

  m_Gui->Divider(1);
  
  m_Gui->Integer(ID_NUMOFDECIMALS,_("Precision"),&m_NumberOfDecimals,0);

  m_Gui->Divider(0);

  m_Gui->Enable(ID_RANGE_X, false);
  m_Gui->Enable(ID_RANGE_Y, false);
  m_Gui->Enable(ID_AXIS_NAME_X, false);
  m_Gui->Enable(ID_AXIS_NAME_Y, false);
  m_Gui->Enable(ID_NUMOFDECIMALS,false);
  return m_Gui;
}
//----------------------------------------------------------------------------
void lhpViewAnalogGraphXY::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case ID_AXIS_NAME_X:
    case ID_AXIS_NAME_Y:
    case ID_NUMOFDECIMALS:
    case ID_RANGE_X:
    case ID_RANGE_Y:
      {
        if(m_XManualRange[0] < m_XMin)
          m_XManualRange[0] = m_XMin;
        if(m_XManualRange[1] > m_XMax)
          m_XManualRange[1] = m_XMax;
        m_Gui->Update();
        if (m_XManualRange[0] >= m_XManualRange[1] || m_DataManualRange[0] >= m_DataManualRange[1])
        {
          mafErrorMessage("Invalid plot range!");
          return;
        }
        for(std::list<lhpPipeAnalogGraphXY*>::iterator it = m_Instantiated.begin(); it != m_Instantiated.end(); it++)
        {
          (*it)->UpdateCommonMembers( \
            m_DataMax, \
            m_DataMin, \
            m_XMax, \
            m_XMin, \
            m_DataManualRange, \
            m_XManualRange, \
            m_TitileX, \
            m_TitileY, \
            m_NumberOfDecimals);
          (*it)->UpdateGraph();
        }
        mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
    case PIPE_GRAH_DESTROY:
      {
        m_Instantiated.remove((lhpPipeAnalogGraphXY*)(e->GetSender()));
        m_DataMax = 0;
        m_DataMin = 0;
        m_XMax = 0;
        m_XMin = 0;
        m_DataManualRange[0] = 0;
        m_DataManualRange[1] = 0;
        m_XManualRange[0] = 0;
        m_XManualRange[1] = 0;
        for(std::list<lhpPipeAnalogGraphXY*>::iterator it = m_Instantiated.begin(); it != m_Instantiated.end(); it++)
        {
          double xData;
          int counter = 0;
          if((*it)->m_Vme->GetEventSource() != NULL)
            (*it)->m_Vme->GetEventSource()->RemoveObserver((*it));
          mafVMEScalarMatrix *emgPlot = mafVMEScalarMatrix::SafeDownCast((*it)->m_Vme);

          mafTimeStamp currTime = emgPlot->GetTimeStamp();
          std::vector<mafTimeStamp> kframes;
          emgPlot->GetTimeStamps(kframes);
          if(emgPlot->GetNumberOfTimeStamps() < 20) // this is added to not make the view too slow when there are a lot of timestamps
          {
            for(int t = 0; t < emgPlot->GetNumberOfTimeStamps(); t++) //
            {
              emgPlot->SetTimeStamp(kframes[t]);
              vnl_vector<double> datarow;
              datarow = emgPlot->GetScalarOutput()->GetScalarData().get_row(1);
              m_DataMin = min(datarow.min_value(),m_DataMin);
              m_DataMax = max(datarow.max_value(),m_DataMax);
            }
          }
          else
          {
            for(int t = 0; t < emgPlot->GetNumberOfTimeStamps(); t = t + (int)(emgPlot->GetNumberOfTimeStamps()/20)) // here are taken "only" 20 samples of timestamps
            {
              emgPlot->SetTimeStamp(kframes[t]);
              vnl_vector<double> datarow;
              datarow = emgPlot->GetScalarOutput()->GetScalarData().get_row(1);
              m_DataMin = min(datarow.min_value(),m_DataMin);
              m_DataMax = max(datarow.max_value(),m_DataMax);
            }
          }

          emgPlot->SetTimeStamp(currTime);

          m_DataManualRange[0] = m_DataMin; //Initialize max data range 
          m_DataManualRange[1] = m_DataMax;

          (*it)->m_Vme->GetEventSource()->AddObserver((*it));

          emgPlot->Update();
          vtkDoubleArray *xArray = vtkDoubleArray::New();
          vnl_vector<double> rowX = emgPlot->GetScalarOutput()->GetScalarData().get_row(0);

          for (int t = 0; t < rowX.size(); t++)
          {
            xData = rowX.get(t);
            xArray->InsertValue(counter,xData);
            counter++;
          }
          if(m_XMax == 0 && m_XMin == 0)
          {
            m_XMax = xArray->GetValue(counter - 1);
            m_XMin = xArray->GetValue(0);
          }
          else
          {
            m_XMax = floor(min(xArray->GetValue(xArray->GetNumberOfTuples()-1),m_XMax));
            m_XMin = floor(max(xArray->GetValue(0),m_XMin));
          }
          m_XManualRange[0] = m_XMin; //Initialize max data range 
          m_XManualRange[1] = m_XMax;

          vtkDEL(xArray);
        }
        int counter = 0;
        for(std::list<lhpPipeAnalogGraphXY*>::iterator it = m_Instantiated.begin(); it != m_Instantiated.end(); it++)
        {
          (*it)->UpdateCommonMembers( \
            m_DataMax, \
            m_DataMin, \
            m_XMax, \
            m_XMin, \
            m_DataManualRange, \
            m_XManualRange, \
            m_TitileX, \
            m_TitileY, \
            m_NumberOfDecimals, \
            counter);
          (*it)->UpdateGraph();
          counter++;
        }
        m_Gui->Update();
        mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
    }
  }
  mafEventMacro(*maf_event);
}
//----------------------------------------------------------------------------
void lhpViewAnalogGraphXY::VmeShow(mafNode *node, bool show)
//----------------------------------------------------------------------------
{
  //Superclass::VmeShow(node, show);
  if(node->IsA("mafVMEScalarMatrix"))
  {
	  if(show)
	  {
      Superclass::VmeShow(node, show);
	    m_Instantiated.push_back(lhpPipeAnalogGraphXY::SafeDownCast(lhpViewAnalogGraphXY::SafeDownCast(this)->GetNodePipe(node)));
	   }
    else
    {
      m_Instantiated.remove(lhpPipeAnalogGraphXY::SafeDownCast(lhpViewAnalogGraphXY::SafeDownCast(this)->GetNodePipe(node)));
      Superclass::VmeShow(node, show);
	  }
    m_DataMax = 0;
    m_DataMin = 0;
    m_XMax = 0;
    m_XMin = 0;
    m_DataManualRange[0] = 0;
    m_DataManualRange[1] = 0;
    m_XManualRange[0] = 0;
    m_XManualRange[1] = 0;
    for(std::list<lhpPipeAnalogGraphXY*>::iterator it = m_Instantiated.begin(); it != m_Instantiated.end(); it++)
    {
      double xData;
      int counter = 0;
      if((*it)->m_Vme->GetEventSource() != NULL)
        (*it)->m_Vme->GetEventSource()->RemoveObserver((*it));
      mafVMEScalarMatrix *emgPlot = mafVMEScalarMatrix::SafeDownCast((*it)->m_Vme);

      mafTimeStamp currTime = emgPlot->GetTimeStamp();
      std::vector<mafTimeStamp> kframes;
      emgPlot->GetTimeStamps(kframes);
      if(emgPlot->GetNumberOfTimeStamps() < 20) // this is added to not make the view too slow when there are a lot of timestamps
      {
        for(int t = 0; t < emgPlot->GetNumberOfTimeStamps(); t++) //
        {
          emgPlot->SetTimeStamp(kframes[t]);
          vnl_vector<double> datarow;
          datarow = emgPlot->GetScalarOutput()->GetScalarData().get_row(1);
          m_DataMin = min(datarow.min_value(),m_DataMin);
          m_DataMax = max(datarow.max_value(),m_DataMax);
        }
      }
      else
      {
        for(int t = 0; t < emgPlot->GetNumberOfTimeStamps(); t = t + (int)(emgPlot->GetNumberOfTimeStamps()/20)) // here are taken "only" 20 samples of timestamps
        {
          emgPlot->SetTimeStamp(kframes[t]);
          vnl_vector<double> datarow;
          datarow = emgPlot->GetScalarOutput()->GetScalarData().get_row(1);
          m_DataMin = min(datarow.min_value(),m_DataMin);
          m_DataMax = max(datarow.max_value(),m_DataMax);
        }
      }

      emgPlot->SetTimeStamp(currTime);

      m_DataManualRange[0] = m_DataMin; //Initialize max data range 
      m_DataManualRange[1] = m_DataMax;

      (*it)->m_Vme->GetEventSource()->AddObserver((*it));

      emgPlot->Update();
      vtkDoubleArray *xArray = vtkDoubleArray::New();
      vnl_vector<double> rowX = emgPlot->GetScalarOutput()->GetScalarData().get_row(0);

      for (int t = 0; t < rowX.size(); t++)
      {
        xData = rowX.get(t);
        xArray->InsertValue(counter,xData);
        counter++;
      }
      if(m_XMax == 0 && m_XMin == 0)
      {
        m_XMax = xArray->GetValue(counter - 1);
        m_XMin = xArray->GetValue(0);
      }
      else
      {
        m_XMax = (min(xArray->GetValue(xArray->GetNumberOfTuples()-1),m_XMax));
        m_XMin = (max(xArray->GetValue(0),m_XMin));
      }
      m_XManualRange[0] = m_XMin; //Initialize max data range 
      m_XManualRange[1] = m_XMax;

      vtkDEL(xArray);
    }
    int counter = 0;
    for(std::list<lhpPipeAnalogGraphXY*>::iterator it = m_Instantiated.begin(); it != m_Instantiated.end(); it++)
    {
      (*it)->UpdateCommonMembers( \
                    m_DataMax, \
                    m_DataMin, \
                    m_XMax, \
                    m_XMin, \
                    m_DataManualRange, \
                    m_XManualRange, \
              		  m_TitileX, \
              		  m_TitileY, \
                    m_NumberOfDecimals, \
                    counter);
      (*it)->UpdateGraph();
      counter++;
    }
    m_Gui->Enable(ID_RANGE_X, m_Instantiated.size() > 0);
    m_Gui->Enable(ID_RANGE_Y, m_Instantiated.size() > 0);
    m_Gui->Enable(ID_AXIS_NAME_X, m_Instantiated.size() > 0);
    m_Gui->Enable(ID_AXIS_NAME_Y, m_Instantiated.size() > 0);
    m_Gui->Enable(ID_NUMOFDECIMALS, m_Instantiated.size() > 0);

    m_Gui->Update();
    mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }
}