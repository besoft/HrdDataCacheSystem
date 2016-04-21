/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpInteractorPicker.h,v $
Language:  C++
Date:      $Date: 2010-06-24 12:39:14 $
Version:   $Revision: 1.1.2.2 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2010 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpInteractorPicker_h
#define __lhpInteractorPicker_h

#include "mafInteractorCameraMove.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------

class lhpInteractorPicker : public mafInteractorCameraMove
{
public:
  mafTypeMacro(lhpInteractorPicker,mafInteractorCameraMove);

  /** Redefined to send picking events if continuous picking is enabled */
  virtual void OnEvent(mafEventBase *event);

   /** Enable/disable continuous picking in OnEvent. */
  void EnableContinuousPicking(bool enable);

protected:
  virtual void OnButtonDown   (mafEventInteraction *e);
  virtual void OnButtonUp     (mafEventInteraction *e);

  /** 
  Send to the listener picked point coordinate through vtkPoint and the corresponding scalar value found in that position. */
  void SendPickingInformation(mafView *v, double *mouse_pos = NULL, int msg_id = VME_PICKED, mafMatrix *tracker_pos = NULL, bool mouse_flag = true);

  bool m_ContinuousPickingFlag;

  lhpInteractorPicker();
  virtual ~lhpInteractorPicker();

private:
  lhpInteractorPicker(const lhpInteractorPicker&);  // Not implemented.
  void operator=(const lhpInteractorPicker&);  // Not implemented.
};
#endif 
