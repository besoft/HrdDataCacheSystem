/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewAnalogGraphXY.h,v $
Language:  C++
Date:      $Date: 2009-11-03 12:58:12 $
Version:   $Revision: 1.1.2.1 $
Authors:   Alberto Losi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpViewAnalogGraphXY_H__
#define __lhpViewAnalogGraphXY_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafViewVTK.h"

#include <vnl\vnl_matrix.h>
#include <list>
#include "lhpPipeAnalogGraphXY.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// lhpViewAnalogGraphXY :
//----------------------------------------------------------------------------
/** */
class lhpViewAnalogGraphXY: public mafViewVTK
{
public:
  lhpViewAnalogGraphXY(wxString label = "Analog graph", int camera_position = CAMERA_FRONT, bool show_axes = false, bool show_grid = false, bool show_ruler = false, int stereo = 0);
  virtual ~lhpViewAnalogGraphXY(); 

  mafTypeMacro(lhpViewAnalogGraphXY, mafViewVTK);

  /*virtual*/ mafView* Copy(mafObserver *Listener);
  /*virtual*/ void OnEvent(mafEventBase *maf_event);

  /*virtual*/ void Create();

  void VmeShow(mafNode *node, bool show);

  /** IDs for the view GUI */
  enum VIEW_GRAPH_GUI_WIDGETS
  {
    ID_AXIS_NAME_X = Superclass::ID_LAST, 
    ID_AXIS_NAME_Y,
    ID_RANGE_X,
    ID_RANGE_Y,
    ID_NUMOFDECIMALS,
  };

protected:
  /*virtual*/ mafGUI *CreateGui();
  std::list<lhpPipeAnalogGraphXY*> m_Instantiated;

  double      m_DataMax;
  double      m_DataMin;
  double      m_XMax;
  double      m_XMin;
  double      m_DataManualRange[2];
  double      m_XManualRange[2];
  wxString		m_TitileX;
  wxString		m_TitileY;
  int         m_NumberOfDecimals;
};
#endif
