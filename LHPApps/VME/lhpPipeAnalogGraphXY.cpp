/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeAnalogGraphXY.cpp,v $
  Language:  C++
  Date:      $Date: 2010-09-10 15:45:52 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time_Array error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <vnl/vnl_vector.h>

#include "lhpPipeAnalogGraphXY.h"
#include "mafDecl.h"
#include "mafSceneNode.h"
#include "mafGUI.h"
#include "mafGUICheckListBox.h"

#include "mafVMEScalarMatrix.h"
#include "mafTagArray.h"
#include "mafTagItem.h"
#include "mafVMEOutputScalar.h"
#include "mafVMEOutputScalarMatrix.h"
#include "mafEventSource.h"

#include "vtkTextProperty.h"
#include "vtkDoubleArray.h"
#include "vtkXYPlotActor.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkLegendBoxActor.h"
#include "vtkMAFSmartPointer.h"
#include "mafString.h"

#include <list>
#include <sstream>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpPipeAnalogGraphXY);
//----------------------------------------------------------------------------

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
lhpPipeAnalogGraphXY::lhpPipeAnalogGraphXY()
:mafPipe()
//----------------------------------------------------------------------------
{
  m_PlotActor	= NULL;
 // m_CheckBox  = NULL;

  m_DataMax = 0;
  m_DataMin = 0;
  m_XMax = 0;
  m_XMin = 0;

  m_Xlabel		= 50;
  m_Ylabel		= 50;
//  m_NumberOfSignals = 0;

  m_TitileX		= "";
  m_TitileY		= "";
  m_Title     = "";
//  m_ItemName  = "analog_";
  m_FitPlot = 0;
  m_Legend = 0;

  m_NumX = 0;
  m_ItemId = 0;
  m_NumberOfDecimals = 2;
  m_VtkData.clear();
  m_ScalarArray.clear();
/*  instantiated.push_back(this);*/
}
//----------------------------------------------------------------------------
lhpPipeAnalogGraphXY::~lhpPipeAnalogGraphXY()
//----------------------------------------------------------------------------
{
  m_Vme->GetEventSource()->RemoveObserver(this);

  m_RenFront->RemoveActor2D(m_PlotActor);
  
  // m_RenFront->SetBackground(m_OldColour);

  for(int i=0;i<m_VtkData.size();i++)
  {
    vtkDEL(m_VtkData[i]);
  }
  m_VtkData.clear();

  for(int i=0;i<m_ScalarArray.size();i++)
  {
    vtkDEL(m_ScalarArray[i]);
  }
  m_ScalarArray.clear();

  vtkDEL(m_XArray);
  m_PlotActor->RemoveAllInputs();
  vtkDEL(m_PlotActor);

   mafEventMacro(mafEvent(this,PIPE_GRAH_DESTROY));
}
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::Create(mafSceneNode *n)
//----------------------------------------------------------------------------
{
  double xData;
  int counter = 0;
  Superclass::Create(n);

  m_EmgPlot = mafVMEScalarMatrix::SafeDownCast(m_Vme);

  m_NumX = m_EmgPlot->GetScalarOutput()->GetScalarData().columns();

  if(m_Vme->GetEventSource() != NULL)
    m_Vme->GetEventSource()->AddObserver(this);

  m_EmgPlot->Update();
  m_XArray = vtkDoubleArray::New();
  vnl_vector<double> rowX = m_EmgPlot->GetScalarOutput()->GetScalarData().get_row(0);

  for (int t = 0; t < m_NumX; t++)
  {
   xData = rowX.get(t);
   m_XArray->InsertValue(counter,xData);
   counter++;
  }

  m_XManualRange[0] = m_XMin;
  m_XManualRange[1] = m_XMax;

  m_ColorRGB[0] = 255;
  m_ColorRGB[1] = 0;
  m_ColorRGB[2] = 0;

  vtkNEW(m_PlotActor);
  
  m_PlotActor->GetProperty()->SetColor(0.02,0.06,0.62);	
  m_PlotActor->GetProperty()->SetLineWidth(1.4);
  m_PlotActor->SetLabelFormat("%-#6.2f");

  vtkTextProperty* tProp = m_PlotActor->GetTitleTextProperty();
  tProp->SetColor(0.02,0.06,0.62);
  tProp->SetFontFamilyToArial();
  tProp->ItalicOff();
  tProp->BoldOff();
  tProp->SetFontSize(5);

  m_PlotActor->SetAxisTitleTextProperty(tProp);
  m_PlotActor->SetAxisLabelTextProperty(tProp);
  m_PlotActor->SetTitleTextProperty(tProp);	

  m_LegendBox_Actor = m_PlotActor->GetLegendBoxActor();
  m_PlotActor->SetLegendPosition(0.75, 0.85); //Set position and size of the Legend Box
  m_PlotActor->SetLegendPosition2(0.35, 0.25);
  m_PlotActor->SetPosition(0.01,0.01);
  m_PlotActor->SetPosition2(0.9,0.9);
  m_PlotActor->SetVisibility(1);
  m_PlotActor->SetXValuesToValue();

  bool tagPresent = m_Vme->GetTagArray()->IsTagPresent("SIGNALS_COLOR");
  if (!tagPresent)
  {
    mafTagItem tag_Sig;
    tag_Sig.SetName("SIGNALS_COLOR");
    tag_Sig.SetNumberOfComponents(3); //3 color values each signal
    m_Vme->GetTagArray()->SetTag(tag_Sig);
  }

  mafTagItem *tag_Signals = m_Vme->GetTagArray()->GetTag("SIGNALS_COLOR");
//   for (long n = 0; n < m_NumberOfSignals; n++)
//   {
/*    long id = n * 3;*/
    if (tagPresent) //Retrieve signals colors from tag
    {
      m_ColorRGB[0] = tag_Signals->GetValueAsDouble(0);
      m_ColorRGB[1] = tag_Signals->GetValueAsDouble(1);
      m_ColorRGB[2] = tag_Signals->GetValueAsDouble(2);
      m_PlotActor->SetPlotColor(0, m_ColorRGB);
    }
    else //Create random colors
    {
      m_ColorRGB[0] = rand() % 255;
      m_ColorRGB[1] = rand() % 255;
      m_ColorRGB[2] = rand() % 255;
      m_PlotActor->SetPlotColor(0, m_ColorRGB);
      tag_Signals->SetValue(m_ColorRGB[0], 0);
      tag_Signals->SetValue(m_ColorRGB[1], 1);
      tag_Signals->SetValue(m_ColorRGB[2], 2);
    }
/*  }*/

  tagPresent = false;
  tagPresent = m_Vme->GetTagArray()->IsTagPresent("AXIS_TITLE");
  if (!tagPresent)
  {
    mafTagItem tag_Sig;
    tag_Sig.SetName("AXIS_TITLE");
    tag_Sig.SetNumberOfComponents(2);
    m_Vme->GetTagArray()->SetTag(tag_Sig);
  }

  mafTagItem *tagAxisTile = m_Vme->GetTagArray()->GetTag("AXIS_TITLE");
  if (tagPresent) //Retrieve axis title from tag
  {
    m_TitileX = tagAxisTile->GetValue(0);
    m_TitileY = tagAxisTile->GetValue(1);
  }
  else //set default axis title
  {
    tagAxisTile->SetValue(m_TitileX,0);
    tagAxisTile->SetValue(m_TitileY,1);
  }
  m_PlotActor->SetXTitle(m_TitileX);
  m_PlotActor->SetYTitle(m_TitileY);

  m_RenFront->GetBackground(m_OldColour); // Save the old Color so we can restore it
  m_RenFront->SetBackground(1,1,1);  
  m_AlwaysVisibleRenderer->SetBackground(1,1,1);
  CreateGui();
}

//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::UpdateGraph()
//----------------------------------------------------------------------------
{
  double scalarData = 0;
  int counter_array = 0;
  vtkDoubleArray *scalar;
  vnl_vector<double> row;

  if(m_XManualRange[0] < m_XMin)
    m_XManualRange[0] = m_XMin;
  if(m_XManualRange[1] > m_XMax)
    m_XManualRange[1] = m_XMax;

  for(int i=0;i<m_VtkData.size();i++)
  {
    vtkDEL(m_VtkData[i]);
  }
  m_VtkData.clear();

  for(int i=0;i<m_ScalarArray.size();i++)
  {
    vtkDEL(m_ScalarArray[i]);
  }
  m_ScalarArray.clear();

  m_RenFront->RemoveActor2D(m_PlotActor);

  m_EmgPlot = mafVMEScalarMatrix::SafeDownCast(m_Vme);

   vtkMAFSmartPointer<vtkDoubleArray> newXArray;
   vtkMAFSmartPointer<vtkDoubleArray> fakeXArray;

   //cycle to get a fake scalar value
//    for (int c = 0; c < m_NumberOfSignals ; c++)
//    {
//      if (m_CheckBox->IsItemChecked(c)) //fill the vector with vtkDoubleArray of signals checked
//      {
       scalar = vtkDoubleArray::New();
       row = m_EmgPlot->GetScalarOutput()->GetScalarData().get_row(1); //skip first row with X information

       if (m_FitPlot)
       {
         for (int t = 0; t < m_NumX; t++) 
         { 
           scalarData = row.get(t);
           break;
         }
       }
       scalar->Delete();
//      }
//    }

//   for (int c = 0; c < m_NumberOfSignals ; c++)
//   {
//     if (m_CheckBox->IsItemChecked(c)) //fill the vector with vtkDoubleArray of signals checked
//     {
      int counter = 0;
      scalar = vtkDoubleArray::New();
      row = m_EmgPlot->GetScalarOutput()->GetScalarData().get_row(1); //skip first row with x information
      
      if (m_FitPlot)
      {
        for (int t = 0; t < m_NumX; t++) 
        { 
          newXArray->InsertValue(counter, m_XArray->GetValue(t));
          scalarData = row.get(t);
          scalar->InsertValue(counter, scalarData);
          counter++;
        }
      }
      else //if not Autofit plot, get values inside m_XManualRange
      {
        for (int t = 0; t < m_NumX; t++) 
        { 
          if (m_XManualRange[0] <= m_XArray->GetValue(t) && m_XArray->GetValue(t) <= m_XManualRange[1])
          {
            newXArray->InsertValue(counter, m_XArray->GetValue(t));
            scalarData = row.get(t);
            scalar->InsertValue(counter, scalarData);
            counter++;
          }
        }
      }
      
      m_ScalarArray.push_back(scalar);

      vtkRectilinearGrid *rect_grid;
      rect_grid = vtkRectilinearGrid::New();
      rect_grid->SetDimensions(newXArray->GetNumberOfTuples(), 1, 1);
      rect_grid->SetXCoordinates(newXArray); 
      rect_grid->GetPointData()->SetScalars(m_ScalarArray.at(0)); 
      m_VtkData.push_back(rect_grid);
      m_PlotActor->AddInput(m_VtkData.at(0));
//     }
//     else
//     {
//       scalar = vtkDoubleArray::New();
//       fakeXArray->Resize(0);
//       scalar->InsertValue(0, scalarData);  //now scalarData is a fake value, already present in the plot
//       m_ScalarArray.push_back(scalar);
// 
//       vtkRectilinearGrid *rect_grid;
//       rect_grid = vtkRectilinearGrid::New();
//       rect_grid->SetDimensions(fakeXArray->GetNumberOfTuples(), 1, 1);
//       rect_grid->SetXCoordinates(fakeXArray); 
//       rect_grid->GetPointData()->SetScalars(m_ScalarArray.at(c)); 
//       m_VtkData.push_back(rect_grid);
//       m_PlotActor->AddInput(m_VtkData.at(c));
//     }
//   }

  row.clear();
  newXArray->GetRange(m_XRange);

  double minY = 0;
  double maxY = 0;

  for (int i = 0; i < m_ScalarArray.size(); i++)
  {
    double dataRange[2];
    m_ScalarArray.at(i)->GetRange(dataRange);

    if(dataRange[0] < minY)
      minY = dataRange[0];

    if(dataRange[1] > maxY)
      maxY = dataRange[1];
  }

  if (m_FitPlot)
  {
    m_PlotActor->SetPlotRange(m_XRange[0], minY, m_XRange[1], maxY);
  }
  else
  {
    m_PlotActor->SetPlotRange(m_XRange[0], m_DataManualRange[0], m_XRange[1], m_DataManualRange[1]);
  }

  m_PlotActor->SetNumberOfXLabels(m_XRange[1] - m_XRange[0]); 
  m_PlotActor->SetNumberOfYLabels(m_DataMax - m_DataMin);

  vtkDoubleArray *lineArray;
  vtkNEW(lineArray);
  lineArray->InsertNextTuple1(m_EmgPlot->GetTimeStamp());
  lineArray->InsertNextTuple1(m_EmgPlot->GetTimeStamp());

  vtkDoubleArray *scalarArrayLine;
  vtkNEW(scalarArrayLine);
  double scalarRange[2];
  if(m_FitPlot)
  {
    scalarRange[0]=minY+abs(minY*0.1);
    scalarRange[1]=maxY-abs(maxY*0.1);
  }
  else
  {
    scalarRange[0]=m_DataManualRange[0]+abs(m_DataManualRange[0]*0.1);
    scalarRange[1]=m_DataManualRange[1]-abs(m_DataManualRange[1]*0.1);
  }
  scalarArrayLine->InsertNextTuple1(scalarRange[0]);
  scalarArrayLine->InsertNextTuple1(scalarRange[1]);

  vtkDEL(lineArray);
  vtkDEL(scalarArrayLine);

  m_RenFront->AddActor2D(m_PlotActor);
  
  std::string format ("%-#");

	  std::stringstream frmt_stream;
	  frmt_stream << m_NumberOfDecimals;
    format += ".";
	  format += frmt_stream.str();
    format += "f";

  m_PlotActor->SetLabelFormat(format.c_str());
  CreateLegend();
}
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::CreateLegend()
//----------------------------------------------------------------------------
{
  int counter_legend = 0;
  mafString name; 
  mafTagItem *tag_Signals = m_Vme->GetTagArray()->GetTag("SIGNALS_COLOR");
  m_PlotActor->RemoveAllInputs();
//   for (int c = 0; c < m_NumberOfSignals ; c++)
//   {
    //int idx = c*3;
//     if (m_CheckBox->IsItemChecked(c))
//     { 
      m_PlotActor->AddInput(m_VtkData.at(0));
      m_LegendBox_Actor->SetNumberOfEntries(counter_legend + 1);
      name = m_Vme->GetName();
/*      name = m_CheckBox->GetItemLabel(0);*/
      m_LegendBox_Actor->SetEntryString(counter_legend,name);

      m_ColorRGB[0] = tag_Signals->GetValueAsDouble(0);
      m_ColorRGB[1] = tag_Signals->GetValueAsDouble(1);
      m_ColorRGB[2] = tag_Signals->GetValueAsDouble(2);
      m_LegendBox_Actor->SetEntryColor(counter_legend, m_ColorRGB);
      counter_legend++;
//     }
//   }
}
//----------------------------------------------------------------------------
//void lhpPipeAnalogGraphXY::ChangeItemName()
//--------------------------\--------------------------------------------------
//{
//   m_CheckBox->SetItemLabel(m_ItemId, (wxString)m_ItemName);
//   mafTagItem *t = m_Vme->GetTagArray()->GetTag("SIGNALS_NAME");
//   t->SetValue(m_ItemName, m_ItemId);
//   m_CheckBox->Update();
//}

//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::ChangeAxisTitle()
//----------------------------------------------------------------------------
{
  mafTagItem *t = m_Vme->GetTagArray()->GetTag("AXIS_TITLE");
  t->SetValue(m_TitileX, 0);
  t->SetValue(m_TitileY, 1);
}

//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::ChangeSignalColor()
//----------------------------------------------------------------------------
{
  m_PlotActor->SetPlotColor(m_ItemId, m_ColorRGB);
  mafTagItem *t = m_Vme->GetTagArray()->GetTag("SIGNALS_COLOR");

  long colorId = m_ItemId*3;
  t->SetValue(m_ColorRGB[0], colorId);
  t->SetValue(m_ColorRGB[1], colorId+1);
  t->SetValue(m_ColorRGB[2], colorId+2);
/*  m_CheckBox->Update();*/
}

//----------------------------------------------------------------------------
mafGUI* lhpPipeAnalogGraphXY::CreateGui()
//----------------------------------------------------------------------------
{
  if(m_Gui == NULL) 
    m_Gui = new mafGUI(this);

/*  m_Gui->String(ID_ITEM_NAME,_("Name :"), &m_ItemName,"");*/
  m_Gui->Color(ID_SIGNALS_COLOR, _("Color"), &m_SignalColor, _("Set signal color"));
/*  m_Gui->Divider(1);*/

//   m_Gui->String(ID_AXIS_NAME_X,_("X Title"), &m_TitileX,_("Set X axis name"));
//   m_Gui->String(ID_AXIS_NAME_Y,_("Y Title"), &m_TitileY,_("Set Y axis name"));
//   m_Gui->Divider(1);
// 
//   if (m_XMax == 0.0)
//   {
//     m_XMax = VTK_DOUBLE_MAX;
//   }
// 
//   m_Gui->VectorN(ID_RANGE_X, _("Range X"), m_XManualRange, 2, m_XMin, m_XMax); // TODO
//   m_Gui->VectorN(ID_RANGE_Y, _("Range Y"), m_DataManualRange, 2); // m_DataMin, m_DataMax);
// 
//   m_Gui->Enable(ID_RANGE_X, !m_FitPlot);
//   m_Gui->Enable(ID_RANGE_Y, !m_FitPlot);

  //m_Gui->Bool(ID_FIT_PLOT,_("Autofit plot"),&m_FitPlot,1,_("Fit bounds to display all graph"));
//   m_Gui->Divider();
// 
//   m_Gui->Divider();

  m_Gui->Bool(ID_LEGEND,_("Legend"),&m_Legend,0,_("Show legend"));
/*  m_Gui->Divider(1);*/

//   wxString name;
//   bool checked = FALSE;
// 
//   m_CheckBox = m_Gui->CheckList(ID_CHECK_BOX,_("Item"),150,_("Chose item to plot"));
//   m_Gui->Button(ID_DRAW,_("Plot"), "",_("Draw selected items"));
//   m_Gui->Divider();
// 
//   bool tagPresent = m_Vme->GetTagArray()->IsTagPresent("SIGNALS_NAME");
//   if (!tagPresent)
//   {
//     mafTagItem tag_Sig;
//     tag_Sig.SetName("SIGNALS_NAME");
//     tag_Sig.SetNumberOfComponents(m_NumberOfSignals);
//     m_Vme->GetTagArray()->SetTag(tag_Sig);
//   }
// 
//   mafTagItem *tag_Signals = m_Vme->GetTagArray()->GetTag("SIGNALS_NAME");
//   for (int n = 0; n < m_NumberOfSignals; n++)
//   {
//     if (tagPresent)
//     {
//       name = tag_Signals->GetValue(n);
//     }
//     else
//     {
//       name = m_ItemName + wxString::Format("%d", n);
//       tag_Signals->SetValue(name.c_str(), n);
//     }
//      m_CheckBox->AddItem(n , name, checked);
//   }
  return m_Gui;
}

//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
//     case ID_RANGE_X:
//     case ID_RANGE_Y:
//       {
//         if (m_XManualRange[0] >= m_XManualRange[1] || m_DataManualRange[0] >= m_DataManualRange[1])
//         {
//           mafErrorMessage("Invalid plot range!");
//           return;
//         }
//         m_PlotActor->RemoveAllInputs();
//         UpdateGraph();
//         mafEventMacro(mafEvent(this,CAMERA_UPDATE));
//       }
//       break;
//     case ID_FIT_PLOT:
//       {
//         m_Gui->Enable(ID_RANGE_X, !m_FitPlot);
//         m_Gui->Enable(ID_RANGE_Y, !m_FitPlot);
//         m_PlotActor->RemoveAllInputs();
//         UpdateGraph();
//         mafEventMacro(mafEvent(this,CAMERA_UPDATE));
//       }
//       break;
    case ID_SIGNALS_COLOR:
      {
        m_ColorRGB[0] = m_SignalColor.Red()/255.0;
        m_ColorRGB[1] = m_SignalColor.Green()/255.0;
        m_ColorRGB[2] = m_SignalColor.Blue()/255.0;
        ChangeSignalColor();
        m_PlotActor->RemoveAllInputs();
        UpdateGraph();
        mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
    case ID_DRAW:
      {
        m_PlotActor->RemoveAllInputs();
        UpdateGraph();
        mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
    case ID_LEGEND:
      {
        switch (m_Legend)
        {
        case TRUE:
          {
            m_PlotActor->LegendOn();
          }
          break;
        case FALSE:
          {
            m_PlotActor->LegendOff();
          } 
          break;
        }
        mafEventMacro(mafEvent(this,CAMERA_UPDATE));
      }
      break;
//     case ID_ITEM_NAME:
//       {
//         ChangeItemName();
//         CreateLegend();
//         mafEventMacro(mafEvent(this,CAMERA_UPDATE));
//       }
      break;
//     case ID_AXIS_NAME_X:
//         m_PlotActor->SetXTitle(m_TitileX);
//         ChangeAxisTitle();
//         mafEventMacro(mafEvent(this,CAMERA_UPDATE));
//       break;
//     case ID_AXIS_NAME_Y:
//         m_PlotActor->SetYTitle(m_TitileY);
//         ChangeAxisTitle();
//         mafEventMacro(mafEvent(this,CAMERA_UPDATE));
//       break;
//     case ID_CHECK_BOX:
//       {
//         m_ItemId = e->GetArg();
// /*        m_ItemName = m_CheckBox->GetItemLabel(m_ItemId);*/
//         m_Gui->Update();
//       }
//       break;
    default:
     mafEventMacro(*e);
    }
    mafEventMacro(*e);
  }
  else if(maf_event->GetId() == VME_TIME_SET)
  {
    m_PlotActor->RemoveAllInputs();
    UpdateGraph();
    mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }
}
//----------------------------------------------------------------------------
//void lhpPipeAnalogGraphXY::SetSignalToPlot(int index,bool plot)
//----------------------------------------------------------------------------
// {
// /*  m_CheckBox->CheckItem(index,plot);*/
// }
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::SetTitleX(mafString title)
//----------------------------------------------------------------------------
{
  m_TitileX = title.GetCStr();
  m_PlotActor->SetXTitle(m_TitileX);
  ChangeAxisTitle();
}
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::SetTitleY(mafString title)
//----------------------------------------------------------------------------
{
  m_TitileY = title.GetCStr();
  m_PlotActor->SetYTitle(m_TitileY);
  ChangeAxisTitle();
}
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::SetTitle(mafString title)
//----------------------------------------------------------------------------
{
  m_Title = title.GetCStr();
  m_PlotActor->SetTitle(m_Title);
}
//----------------------------------------------------------------------------
void lhpPipeAnalogGraphXY::UpdateCommonMembers( \
double      pDataMax, \
double      pDataMin, \
double      pXMax, \
double      pXMin, \
double      *pDataManualRange, \
double      *pXManualRange, \
wxString		pTitileX, \
wxString		pTitileY, \
int pNumberOfDecimals, \
int counter)
//----------------------------------------------------------------------------
{
  m_DataMax=pDataMax;
  m_DataMin=pDataMin;
  m_XMax=pXMax;
  m_XMin=pXMin;
  m_DataManualRange[0]=pDataManualRange[0];
  m_DataManualRange[1]=pDataManualRange[1];
  m_XManualRange[0]=pXManualRange[0];
  m_XManualRange[1]=pXManualRange[1];
  m_TitileX=pTitileX;
  m_TitileY=pTitileY;

  if(counter != -1)
  {
	  m_PlotActor->SetLegendPosition(0.75, 0.85 - 0.15 * counter); //Set position and size of the Legend Box
	  m_PlotActor->SetLegendPosition2(0.35, 0.25);
  }
  m_PlotActor->SetXTitle(m_TitileX);
  m_PlotActor->SetYTitle(m_TitileY);

  m_NumberOfDecimals = pNumberOfDecimals;
}