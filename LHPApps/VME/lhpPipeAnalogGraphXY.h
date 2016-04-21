/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeAnalogGraphXY.h,v $
  Language:  C++
  Date:      $Date: 2009-11-03 12:58:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpPipeAnalogGraphXY_H__
#define __lhpPipeAnalogGraphXY_H__

#include "mafPipe.h"
#include "mafEvent.h"
#include "lhpVMEDefines.h"
//----------------------------------------------------------------------------
// forward refs :
//----------------------------------------------------------------------------
class mafVMEScalarMatrix;
class vtkActor;
class mafGUICheckListBox;
class vtkLegendBoxActor;
class vtkXYPlotActor;
class vtkDoubleArray;
class vtkRectilinearGrid;

#define PIPE_GRAH_DESTROY -10

//----------------------------------------------------------------------------
// lhpPipeAnalogGraphXY :
//----------------------------------------------------------------------------
/** 
Visual pipe to visualize graphs of analog signals. */
class LHP_VME_EXPORT lhpPipeAnalogGraphXY : public mafPipe
{
public:
  mafTypeMacro(lhpPipeAnalogGraphXY,mafPipe);

  lhpPipeAnalogGraphXY();
  virtual     ~lhpPipeAnalogGraphXY ();

  void lhpPipeAnalogGraphXY::OnEvent(mafEventBase *maf_event); 

  /*virtual*/ void Create(mafSceneNode *n);

  //Create plots of scalar data
  void UpdateGraph();

  //Change the name of the selected item in the legend box
/*  void ChangeItemName();*/

  //Change the title of the axis
  void ChangeAxisTitle();

  //Change signal color
  void ChangeSignalColor();

  /** Set if visualize or not a particular signal */
/*  void SetSignalToPlot(int index,bool plot);*/

  void SetTitleX(mafString title);
  void SetTitleY(mafString title);
  void SetTitle(mafString title);
  void UpdateCommonMembers( \
    double      pDataMax, \
    double      pDataMin, \
    double      pXMax, \
    double      pXMin, \
    double      *pDataManualRange, \
    double      *pXManualRange, \
    wxString		pTitileX, \
    wxString		pTitileY, \
    int pNumberOfDecimals, \
    int counter = -1);
  
protected:

  mafGUI* CreateGui();

  enum PIPE_GRAPH_GUI_WIDGETS
  {
    ID_X_MIN = Superclass::ID_LAST, 
    ID_DRAW,
/*    ID_CHECK_BOX,*/
    ID_LEGEND,
/*    ID_ITEM_NAME,*/
//     ID_AXIS_NAME_X,
//     ID_AXIS_NAME_Y,
//     ID_RANGE_X,
//     ID_RANGE_Y,
    ID_FIT_PLOT,
    ID_SIGNALS_COLOR,
    ID_LAST
  };

/*  mafGUICheckListBox* m_CheckBox;*/
  vtkLegendBoxActor *m_LegendBox_Actor;

private:
  //create the legend
  void CreateLegend();

  double m_OldColour[3];
  double m_ColorRGB[3];
  wxColor m_SignalColor;
/*  int       m_NumberOfSignals;*/
  int       m_NumX;
  long      m_ItemId;

  int m_Legend;
  int m_FitPlot;

  double      m_DataMax;
  double      m_DataMin;
  double      m_XMax;
  double      m_XMin;
  double      m_XRange[2];
  double      m_DataManualRange[2];
  double      m_XManualRange[2];
/*  mafString   m_ItemName;*/
  wxString    m_Title;
  int				  m_Xlabel;
  int				  m_Ylabel;
  wxString		m_TitileX;
  wxString		m_TitileY;

  std::vector<vtkRectilinearGrid*> m_VtkData;

  std::vector<vtkDoubleArray*> m_ScalarArray;
 
  vtkDoubleArray *m_XArray;
  std::vector<double> m_XVector;  

  mafVMEScalarMatrix *m_EmgPlot;

  vtkXYPlotActor *m_PlotActor;

  int m_NumberOfDecimals;
};  
#endif // __lhpPipeAnalogGraphXY_H__
