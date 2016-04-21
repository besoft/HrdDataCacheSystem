/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpGUIHistogramWidget.h,v $
  Language:  C++
  Date:      $Date: 2011-11-11 14:16:13 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Paolo Quadrani, Gianluigi Crimi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpGUIHistogramWidget_H__
#define __lhpGUIHistogramWidget_H__


//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafGUIPanel.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "lhpGuiDefines.h"

//----------------------------------------------------------------------------
// forward refs
//----------------------------------------------------------------------------
class mafRWI;
class vtkDataArray;
class mafGUI;
class vtkImageData;
class vtkMAFXYPlotActor;
class vtkImageAccumulate;

//----------------------------------------------------------------------------
/** mafGUIHistogramWidget : widget that encapsulate render window into a gui*/
class LHP_GUI_EXPORT lhpGUIHistogramWidget: public mafGUIPanel, public mafObserver
{
public:

  /** Constructor */
  lhpGUIHistogramWidget(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, 
           const wxSize& size = wxSize(400,300), long style = wxTAB_TRAVERSAL /*| wxSUNKEN_BORDER */);
  /** Destructor. */
  virtual ~lhpGUIHistogramWidget();

  /** Set the listener for the widget */
  virtual void SetListener(mafObserver *Listener) {m_Listener = Listener;};
  
  /** Main Event handler */
  void OnEvent(mafEventBase *event);
     
  /** Input data from which generate histogram.*/
  void SetData(vtkDataArray *data);

  /** Set the number of the bin */
  void SetBinNumber(int nbin);


  /** Enable/Disable Percentage visualizzation */
  void ShowPercentage(bool show);

  /* Reset the Zoom */
  void ResetZoom();

 

protected:
  /* */
  void UpdatePlotAera();
    
  /** Reset to default histogram parameters.*/
  void ResetHistogram();

  /* Update scaled data to counter o percentage visualizzation */
  void UpdateScaled();

  mafObserver   *m_Listener;
  
  int           m_BinNumber;
  double        m_ScaleFactor;
  bool        m_ShowPercentage;
  double        m_ZoomX;
  double        m_ZoomY;
  double        m_PosX;
  double        m_PosY;
  
  bool          m_Dragging;
  bool          m_Move;
  double        m_DragStart[2];
  
  mafGUI        *m_Gui;

  
  vtkDataArray  *m_Data;
  mafRWI        *m_HistogramRWI;
  vtkImageAccumulate  *m_Histogram;
	vtkMAFXYPlotActor  *m_HistogramPlot;
  vtkImageData  *m_HistogramData;
  vtkImageData  *m_ScaledHistogram;
  
};
#endif
