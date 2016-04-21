/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpGUIHistogramWidget.cpp,v $
Language:  C++
Date:      $Date: 2011-11-11 14:16:13 $
Version:   $Revision: 1.1.2.1 $
Authors:   Paolo Quadrani, Gianluigi Crimi
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpGUIHistogramWidget.h"
#include "mafDecl.h"
#include "mafRWI.h"
#include "mafGUI.h"
#include "mafGUIRangeSlider.h"
#include "mafGUILutSlider.h"

#include "mafDeviceButtonsPad.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafEventInteraction.h"
#include "mmuIdFactory.h"

#include "vtkDataSet.h"
#include "vtkDataArray.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkLineSource.h"
#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkImageAccumulate.h"
#include "vtkMAFXYPlotActor.h"
#include "vtkCamera.h"
#include "vtkProperty2D.h"
#include "vtkDataset.h"

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

//----------------------------------------------------------------------------
lhpGUIHistogramWidget::lhpGUIHistogramWidget(wxWindow* parent, wxWindowID id /* = -1 */, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxSize */,long style /* = wxTAB_TRAVERSAL */)
:mafGUIPanel(parent,id,pos,size,style)
//----------------------------------------------------------------------------
{
	m_Listener    = NULL;
  m_Histogram   = NULL;
	m_HistogramData = NULL;
  m_ScaledHistogram = NULL;
  m_HistogramPlot = NULL;
  m_Gui         = NULL;
  m_Data        = NULL;
  vtkMAFXYPlotActor *test;
  
  m_DragStart[0]       = 0;
  m_DragStart[1]       = 0;
  m_Dragging        = false;
  //////////////////////////////////////////////////////////////////////////

  m_ZoomX=1.0;
  m_ZoomY=1.0;
  m_PosX=0.0;
  m_PosY=0.0;

  m_BinNumber=100;
  m_ShowPercentage=false;

  vtkNEW(m_Histogram);
  vtkNEW(m_HistogramPlot);

  m_HistogramData  = vtkImageData::New();
  m_HistogramData->Initialize();
  m_HistogramData->SetSpacing(1.0,0.0,0.0);

  m_ScaledHistogram  = vtkImageData::New();


  m_HistogramPlot->SetLabelFormat( "%g" );
  m_HistogramPlot->SetXTitle( "" );
  m_HistogramPlot->SetYTitle( "" );
  m_HistogramPlot->SetWidth(1);
  m_HistogramPlot->SetHeight(1);
  m_HistogramPlot->SetPosition(0.0, 0.0);
  m_HistogramPlot->GetProperty()->SetLineStipplePattern(0xaaaa);
  m_HistogramPlot->GetProperty()->SetLineWidth(1);
  m_HistogramPlot->GetProperty()->SetPointSize(4.0);
  m_HistogramPlot->PlotPointsOn();
  m_HistogramPlot->SetPlotColor(0,0.41,0.7,0.95);
  m_HistogramPlot->SetXValuesToValue();

  wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);

  m_HistogramRWI = new mafRWI(mafGetFrame());
  m_HistogramRWI->SetListener(this);
  m_HistogramRWI->m_RenFront->AddActor(m_HistogramPlot);
  m_HistogramRWI->m_RenFront->SetBackground(0.28,0.28,0.28);
  m_HistogramRWI->SetSize(pos.x,pos.y,size.GetWidth(),size.GetHeight());
  ((wxWindow *)m_HistogramRWI->m_RwiBase)->SetSize(size.GetWidth(),size.GetHeight());
  ((wxWindow *)m_HistogramRWI->m_RwiBase)->SetMinSize(wxSize(size.GetWidth(),size.GetHeight()));
  m_HistogramRWI->m_RenFront->GetActiveCamera();
  m_HistogramRWI->m_RwiBase->Reparent(this);
  m_HistogramRWI->m_RwiBase->SetListener(this);
  m_HistogramRWI->m_RwiBase->Show(true);
  
 
  SetMinSize(size);
}
//----------------------------------------------------------------------------
lhpGUIHistogramWidget::~lhpGUIHistogramWidget() 
//----------------------------------------------------------------------------
{
  m_HistogramRWI->m_RenFront->RemoveActor(m_HistogramPlot);
  vtkDEL(m_HistogramPlot);
  vtkDEL(m_Histogram);
  cppDEL(m_HistogramRWI);
}


//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::OnEvent( mafEventBase *event )
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(event))
  {
    switch(e->GetId())
    {
      default:
        e->Log();
      break; 
    }
    m_HistogramRWI->CameraUpdate();
  }
  else if (mafEventInteraction *ei = mafEventInteraction::SafeDownCast(event))
  {
    if (ei->GetId() == mafDeviceButtonsPadMouse::GetMouse2DMoveId())
    {
      if(m_Histogram->GetOutput() == NULL) return;
      double pos[2];
      ei->Get2DPosition(pos);
      if (m_Dragging)
      {
        double dx,dy;
        if (m_Move)
        {
          dx = m_DragStart[0]-pos[0];
          m_PosX += (  dx/200.0 );
          m_PosX = min (m_PosX,1);
          m_PosX = max (m_PosX, 0.0);

          dy = m_DragStart[1]-pos[1];
          m_PosY += (  dy/200.0 );
          m_PosY = min (m_PosY,1);
          m_PosY = max (m_PosY, 0.0);
        }
        else 
        {
          dx = m_DragStart[0]-pos[0];
          m_ZoomX += ( dx/100.0 );
          m_ZoomX = min (m_ZoomX,1);
          m_ZoomX = max (m_ZoomX, 0.1);

          dy = m_DragStart[1]-pos[1];
          m_ZoomY += ( dy/100.0 );
          m_ZoomY = min (m_ZoomY,1);
          m_ZoomY = max (m_ZoomY, 0.1);
        }
       
        m_PosX=min(m_PosX,1.0-m_ZoomX);       
        m_PosY=min(m_PosY,1.0-m_ZoomY);       

        if(m_Gui)
        {
          m_Gui->Update();
        }
        m_DragStart[0]=pos[0];
        m_DragStart[1]=pos[1];

        UpdatePlotAera();
      }
    }
    else if (ei->GetId() == mafDeviceButtonsPad::GetButtonDownId())
    {
      if (ei->GetButton() == MAF_LEFT_BUTTON)
      {
        m_Dragging = true;
        m_Move = true;
        ei->Get2DPosition(m_DragStart);         
      }
      else if (ei->GetButton() == MAF_RIGHT_BUTTON)
      {
        m_Dragging = true;
        m_Move = false;
        ei->Get2DPosition(m_DragStart);         
      }
    }
    else if (ei->GetId() == mafDeviceButtonsPad::GetButtonUpId())
    {
        m_Dragging = false;
    }
  }
}


//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::UpdatePlotAera() 
  //----------------------------------------------------------------------------
{
  double range[2], dataRange[2];

  m_HistogramData->GetScalarRange( dataRange );

  range[0]=dataRange[0]+(dataRange[1]-dataRange[0])*m_PosX;
  range[1]=range[0]+(dataRange[1]-dataRange[0])*m_ZoomX;
  m_HistogramPlot->SetXRange(range);

  m_ScaledHistogram->GetScalarRange(dataRange);
  
  range[0]=dataRange[0]+(dataRange[1]-dataRange[0])*m_PosY;
  range[1]=range[0]+(dataRange[1]-dataRange[0])*m_ZoomY;
  m_HistogramPlot->SetYRange(range);
  
  m_HistogramRWI->CameraUpdate();
}


/** Set the number of the bin */
void lhpGUIHistogramWidget::SetBinNumber(int nbin)
{
  m_BinNumber=nbin;

  double range[2];
  m_HistogramData->GetScalarRange( range );

  double xSpacing =  (range[1]-range[0])/(double)m_BinNumber;;
  m_Histogram->SetComponentExtent(0,m_BinNumber,0,0,0,0 );
  m_Histogram->SetComponentSpacing(xSpacing, 0, 0 );
  m_Histogram->Update();

  UpdateScaled();
  m_HistogramPlot->RemoveAllInputs();
  m_HistogramPlot->AddInput( vtkDataSet::SafeDownCast(m_ScaledHistogram));
  m_HistogramRWI->CameraUpdate();
}

//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::ResetZoom()
  //----------------------------------------------------------------------------
{
  m_ZoomX=1.0;
  m_ZoomY=1.0;
  m_PosX=0.0;
  m_PosY=0.0;
  UpdatePlotAera();
}

//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::SetData(vtkDataArray *data)
//----------------------------------------------------------------------------
{
  m_Data = data;
  
  m_HistogramData->SetDimensions(m_Data->GetNumberOfTuples(),1,1);
  m_HistogramData->SetScalarType(m_Data->GetDataType());

  m_HistogramData->GetPointData()->SetScalars(m_Data);
  m_HistogramData->Update();

  double range[2];
  m_HistogramData->GetScalarRange( range );

 
  m_Histogram->SetInput(m_HistogramData);
  m_Histogram->SetComponentOrigin( range[0],0,0 );
  
  double xSpacing =  (range[1]-range[0])/(double)m_BinNumber;;
  m_Histogram->SetComponentExtent(0,m_BinNumber,0,0,0,0 );
  m_Histogram->SetComponentSpacing(xSpacing, 0, 0 );
  m_Histogram->Update();

  UpdateScaled();
  m_HistogramPlot->RemoveAllInputs();
  m_HistogramPlot->AddInput( vtkDataSet::SafeDownCast(m_ScaledHistogram));
  ResetZoom();
   
}

//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::ShowPercentage( bool show )
//----------------------------------------------------------------------------
{
  m_ShowPercentage=show;

  UpdateScaled();
  UpdatePlotAera();
  m_HistogramPlot->RemoveAllInputs();
  m_HistogramPlot->AddInput( vtkDataSet::SafeDownCast(m_ScaledHistogram));
  m_HistogramRWI->CameraUpdate();
}

//----------------------------------------------------------------------------
void lhpGUIHistogramWidget::UpdateScaled()
//----------------------------------------------------------------------------
{
  m_ScaledHistogram->DeepCopy(m_Histogram->GetOutput());
  if (m_ShowPercentage)
  {
    
    int *values,type,nPoints,total=0;
    type=m_ScaledHistogram->GetScalarType();
    if (type==VTK_INT)
    {
      values=(int *)m_ScaledHistogram->GetScalarPointer();
      nPoints=m_ScaledHistogram->GetNumberOfPoints();
      for (int i=0;i<nPoints;i++)
        total+=values[i];
      for (int i=0;i<nPoints;i++)
        values[i]=(values[i]*100)/total;
    }
    else mafErrorMacro("Wrong Type");
  }
  m_ScaledHistogram->Update();
}


