/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafViewIntGraphWindow.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mafViewIntGraphWindow_H___
#define __mafViewIntGraphWindow_H___
    
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------
#include "mafString.h"
#include "mafMemGraph.h"
#include "mafStringSet.h"
#include "mafGUI.h"

//----------------------------------------------------------------------------
// forward references
//----------------------------------------------------------------------------
class mafViewIntGraph;

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------
#define mafDEFINE_GET_SET_MEMBER(type, name)       \
protected:                                         \
  type  m_##name;                                  \
public:                                            \
  type const& Get##name() const {return m_##name;}             \
  type  Set##name(type const& name) {type old_##name = m_##name; m_##name = name; return old_##name;}

enum  mafViewIntGraphFontFamily
{
  mafFF_DEFAULT = 0,
  mafFF_DECORATIVE ,
  mafFF_ROMAN      ,
  mafFF_SCRIPT     ,
  mafFF_SWISS      ,
  mafFF_MODERN     ,
  mafFF_LAST     ,
  mafFF_FORCED_DWORD = 0x7fffffff     
};

class mafIDDesc
{
public:
  virtual void   GetIDDesc(const IDType& nID,char *sDescript,unsigned int nLength) = 0;
  virtual wxMenu *GenerateMenu(mafMemoryGraph *pmgGraph, unsigned int nIDBase) = 0;
  virtual bool   ProcessMenu(mafMemoryGraph *pmgGraph, unsigned int nIDCommand) = 0;
};

struct mafViewIntGraphCreate
{
  wxWindow   *hNotifiedWnd;
  mafIDDesc  *pidDesc;
};

typedef mafViewIntGraphCreate *mafPViewIntGraphCreate;

struct mafViewIntSetGraph
{
  mafMemoryGraph *pmgGraph;
  mafIDDesc      *pidDesc;
};

//----------------------------------------------------------------------------
// mafViewIntGraphWindow
//----------------------------------------------------------------------------
class mafViewIntGraphWindow: public wxWindow
{
  DECLARE_DYNAMIC_CLASS(mafViewIntGraphWindow)
public:
  mafViewIntGraphWindow(const wxString& label);
  //default do nothing constructor
  mafViewIntGraphWindow(void);
  ~mafViewIntGraphWindow(void);

  int            &GetRoughGrid(void)   {return m_RoughGrid;}
  int            &GetPreciseGrid(void) {return m_PreciseGrid;}
  mafMemoryGraph *GetGraph(void) const;
  wxBitmap       GetBitmap();
  void           SetGraphData(mafViewIntSetGraph *pSetGrpah);
  void           Update(void);
  bool           SaveGraphAsCSV(wxString const &sFileName);
  mafGUI         *GetGUI(mafGUI *pGUI, mafObserver *listener, wxInt32 nBaseID);
  bool           OnEvent(mafEvent& e);
  static wxInt32 GetFontFamily(mafViewIntGraphFontFamily fFamily);
  /** Tune curve parameters */
  void           AdjustCurveAppearance(unsigned int nCurve);
  /** Save window settings in text form*/
  mafStringSet   *SaveSettings(void) const;
  /** Restore window settings from text form*/
  void           LoadSettings(mafStringSet const *pSettings);

protected:
  mafMemoryGraph  *m_Graph;
  mafIDDesc       *m_Desc;
  int             m_RoughGrid;
  int             m_PreciseGrid;
  wxInt32         m_TitleFntHeight;
  unsigned        m_pointed;

  void           OnCommand(wxCommandEvent& event);
  void           OnPaint(wxPaintEvent &event);
  void           OnSize(wxSizeEvent &event);
  void           OnCloseWindow  (wxCloseEvent& event);
  void           OnRightMouseButtonDown(wxMouseEvent &event);
  void           OnLeftMouseButtonDown(wxMouseEvent &event);

  //must have for table hook
  DECLARE_EVENT_TABLE();

  wxInt32  m_BaseID;           
  //tuning parameters
  mafDEFINE_GET_SET_MEMBER(wxInt32, TitleFontSize)
  mafDEFINE_GET_SET_MEMBER(mafViewIntGraphFontFamily, TitleFontFamily)
  mafDEFINE_GET_SET_MEMBER(wxInt32, TitleFontStyle)
  mafDEFINE_GET_SET_MEMBER(wxInt32, TitleFontWeight)

  mafDEFINE_GET_SET_MEMBER(wxInt32, TickFontSize)
  mafDEFINE_GET_SET_MEMBER(mafViewIntGraphFontFamily, TickFontFamily)
  mafDEFINE_GET_SET_MEMBER(wxInt32, TickFontStyle)
  mafDEFINE_GET_SET_MEMBER(wxInt32, TickFontWeight)

  mafDEFINE_GET_SET_MEMBER(wxInt32, CurveThickness)
  mafDEFINE_GET_SET_MEMBER(wxInt32, AxisThickness )
  mafDEFINE_GET_SET_MEMBER(wxInt32, GridThickness )
  mafDEFINE_GET_SET_MEMBER(IDType,  SelectedID);
  mafDEFINE_GET_SET_MEMBER(mafViewIntGraph*, NotifiedView);
  
private:
  int       GetIntX(const wxRect *prc, double rX);
  int       GetIntY(const wxRect *prc, double rY);
  int       Log10Abs(double rValue);
  int       GetOptimalSplits(double rMin, double rMax, double rCoef);
  void      AdoptMinMaxRange(double& vMin, double& vMax, double& vPow10Marks);
  void      DrawGraph(wxDC *pCompatDC);
  void      DrawAxes(wxDC *pDC, wxRect *prc, const mafMemoryGraph::mafMemGrTotalRange *cpgtrRanges, double rXCoef, double rYCoef, bool bGrid, bool bPreciseGrid);
  void      DrawXAxis(wxDC *pDC, wxRect *prc, double rXMin, double rXMax, double rXCoef, bool bGrid, bool bPreciseGrid);
  void      DrawYAxis(wxDC *pDC, wxRect *prc, double rXMin, double rXMax, double rXCoef, bool bGrid, bool bPreciseGrid);
  void      Init();
  wxColor   *m_ColorTable;
  wxInt32   m_ColorTableNumber;
};

wxChar const *GetListSeparator();
wxChar const *GetDecimalSeparator();

#endif