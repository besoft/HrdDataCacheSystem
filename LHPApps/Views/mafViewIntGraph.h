/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafViewIntGraph.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mafViewIntGraph_H__
#define __mafViewIntGraph_H__


#ifdef __GNUG__
    #pragma interface "mafViewIntGraph.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "mafView.h"
#include "mafDecl.h"
#include "mafViewIntGraphWindow.h"

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------
#define mafINTG_SAVEINFO_TAG  "DM_PLOT_INFORMATION_TAG"
#define mafINTGG_SAVEINFO_TAG "DM_GENERAL_PLOT_INFORMATION_TAG"

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafView.h"
#include "mafRWIBase.h"
#include "mafRWI.h"
#include "mafSceneGraph.h"
#include "mafGraphIDDesc.h"
#include "mafSceneNode.h" //used in subclasses
#include <map>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafSceneGraph;
class mafIntGraphHyer;

WX_DEFINE_ARRAY(mafVME *, VMEArray);

//----------------------------------------------------------------------------
// class mafViewIntGraph
//----------------------------------------------------------------------------
class mafViewIntGraph: public mafView
{
public:
  mafViewIntGraph(const wxString &label = "Biomechanical graph");
  virtual ~mafViewIntGraph(); 

  mafTypeMacro(mafViewIntGraph, mafView);

  virtual void      OnEvent(mafEventBase *maf_event);
  virtual mafView*  Copy(mafObserver *Listener);
  virtual void      Create();
  /** Add the vme to the view's scene-graph*/
  virtual void VmeAdd(mafNode *vme);
  /** Remove the vme from the view's scene-graph*/
  virtual void VmeRemove(mafNode *vme);
  virtual void VmeSelect(mafNode *vme, bool select);
  void	VmeSelect(const IDType& nGraphID, bool select)	;								
  /** Called to show/hide vme*/
  virtual void VmeShow(mafNode *vme, bool show);
  /** 
  Called to update visual pipe properties of the vme passed as argument. If the 'fromTag' flag is true,
  the update is done by reading the visual parameters from tags.*/
  virtual void VmeUpdateProperty(mafNode *vme, bool fromTag = false);
  /** 
  Create the visual pipe for the node passed as argument. 
  To create visual pipe first check in m_PipeMap if custom visual pipe is defined, 
  otherwise ask to vme which is its visual pipe. */
  virtual void VmeCreatePipe(mafNode *vme);
  /** Delete vme's visual pipe. It is called when vme is removed from visualization.*/
  virtual void VmeDeletePipe(mafNode *vme);

  virtual void CameraUpdate();
  virtual mafSceneGraph *GetSceneGraph()    {return m_Sg;}; 
  virtual mafRWIBase    *GetRWI()           {return m_Rwi->m_RwiBase;};
  /** Return a pointer to the image of the renderwindow.*/
  void GetImage(wxBitmap &bmp, int magnification = 1);
  /** Called to update all components that depends on Application Options.*/
  virtual void OptionsUpdate();
  /** 
  Set the visualization status for the node (visible, not visible, mutex, ...) \sa mafSceneGraph*/
  virtual int GetNodeStatus(mafNode *vme);
  /** 
  Return a pointer to the visual pipe of the node passed as argument. 
  It is used in mafSideBar to plug the visual pipe's GUI in the tabbed vme panel. \sa mafSideBar*/
  virtual mafPipe* GetNodePipe(mafNode *vme);


  /** Access function. See name. */
  mafViewIntGraphWindow *GetRenderWindow()    {return m_RenderWindow;}
  /** Access function. See name. */
  int                   GetPlotFrameStart()   {return m_PlotFrameStart;}
  /** Access function. See name. */
  int                   GetPlotFrameStop()    {return m_PlotFrameStop;}
  /** Access function. See name. */
  int                   GetReferenceFrame()    {return m_ReferenceFrame;}
  /** Access function. See name. */
  mafMemoryGraph        *GetGraph()     {return m_Graph;}
  /** Pass information inside window. Set graph and stuff */
  void                  setWindowGraph();
  /** save information about current plot configuration into tag */
  void                  savePlotInfo(mafVME *vme);
  /** restore information about current plot configuration from tag */
  void                  loadPlotInfo(mafVME *vme);
  /** save information about current plot into VME*/
  void                  savePlot(void);
  /** restore information about current plot from VME tree*/
  void                  loadPlot(void);
  /** save information about current plot into VME: general settings*/
  void                  savePlotGen(void);
  /** restore information about current plot from VME tree: general settings*/
  void                  loadPlotGen(void);

  /** Show view settings into the tabbed sidebar. */
  //void	ShowSettings();
  /** track modification of VME */
  //void VmeModified(mafVME *vme);
  /** Access function. See name. */
  VMEArray * GetVMEs() {return m_VMEs;}
  /** Access function. See name. */
  PipeArray *GetPipes() {return m_Pipes;}
  /** Update all gui widgets*/
  void      UpdateGui();
  /** Update all gui widgets*/
  bool&     ModifyMode() {return m_ModifyMode;}

  /** Set the vtk RenderWindow size. Used only for Linux (not necessary for Windows) */
  void SetWindowSize(int w, int h);

  /** Print this view.*/
  virtual void Print(wxDC *dc, wxRect margins);
  /** print a dump of this object */
  virtual void Print(std::ostream& os, const int tabs=0);// const;

  enum VIEWINTGRAPH_WIDGET_ID
  {
    ID_DEFAULT = Superclass::ID_LAST,
    ID_FREEZE_GRAPH,
    ID_ZOOM_START_STOP,
    ID_REFERENCE_FRAME,
    ID_SAVE_PLOT,
    ID_LOAD_PLOT,
    ID_ROLLOUT_RENDER,
    ID_SHOW_LAST
  };
  mafRWI *m_Rwi;



protected:
  //Actual window: same as m_win but with correct type
  mafViewIntGraphWindow   *m_RenderWindow;
  int                     m_IsFrozen;
  int                     m_ReferenceFrame;
  union
  {
    struct
    {
      int                  m_PlotFrameStart;
      int                  m_PlotFrameStop;
    };
    int                    m_FreezeFrames[2];
  };

  mafMemoryGraph           *m_Graph;
  mafGraphIDDesc           *m_Descriptions;
  bool                     m_LoadMode;
  bool                     m_ModifyMode;

  VMEArray                 *m_VMEs;
  PipeArray                *m_Pipes;
  mafSceneGraph            *m_Sg;

  //int   m_NumberOfVisibleVme; ///< perform ResetCamera only for the first vme shown into the view

  virtual mafGUI *CreateGui();

  /** Return the visual pipe's name.*/
  void GetVisualPipeName(mafNode *node, mafString &pipe_name);
};

#endif
