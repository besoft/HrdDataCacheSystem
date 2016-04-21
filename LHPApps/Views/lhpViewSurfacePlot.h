/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewSurfacePlot.h,v $
Language:  C++
Date:      $Date: 2009-11-03 12:58:12 $
Version:   $Revision: 1.1.2.1 $
Authors:   Alberto Losi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpViewSurfacePlot_H__
#define __lhpViewSurfacePlot_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafViewVTK.h"

#include <vnl\vnl_matrix.h>
#include <list>
//#include "lhpPipeAnalogGraphXY.h"
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpPipeSurfacePlot;
//----------------------------------------------------------------------------
// lhpViewSurfacePlot :
//----------------------------------------------------------------------------
/** */
class lhpViewSurfacePlot: public mafViewVTK
{
public:
  lhpViewSurfacePlot(wxString label = "lhpViewSurfacePlot", int camera_position = CAMERA_FRONT, bool show_axes = false, bool show_grid = false, bool show_ruler = false, int stereo = 0);
  virtual ~lhpViewSurfacePlot(); 

  mafTypeMacro(lhpViewSurfacePlot, mafViewVTK);

  virtual mafView*  Copy(mafObserver *Listener, bool lightCopyEnabled = false);
  
  /** event handling, pass events to corresponding VME pipes */
  virtual void OnEvent(mafEventBase *maf_event);

  /** do nothing but to call the parent's Create() */
  virtual void Create();

  /** show data vme */
  void VmeShow(mafNode *node, bool show);

  /** IDs for the view GUI */
  enum VIEW_GRAPH_GUI_WIDGETS
  {
    ID_SELECT_ID_IK = Superclass::ID_LAST,
	ID_SCALE_X,
	ID_SCALE_Y,
	ID_SCALE_Z,
	ID_CHANGE_SCALE,
	ID_RENDER_MODE,
    ID_LAST
  };

protected:
  mafGUI *CreateGui();
  /** pipe of the current VME */
  lhpPipeSurfacePlot * m_Pipe;
  /** node of the current VME */
  mafNode * m_Node;

  /** enable GUI */
  void EnableGUI(bool b);
  
  //int m_nRow;
  //int m_nCol;
  
  /** X scale of the plot */
  float m_ScaleX;
  /** Y scale of the plot */
  float m_ScaleY;
  /** Z scale of the plot */
  float m_ScaleZ;
  
  /** selecting Inverse Dynamics or Inverse Kinetics */
  //int m_SelectIDIK;
  /** render mode of surface, wireframe or points */
  int m_RenderMode;
 
};
#endif
