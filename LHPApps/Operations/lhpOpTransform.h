/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpTransform.h,v $
  Language:  C++
  Date:      $Date: 2011-03-03 11:37:46 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Nicola Vanella
  ==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
  =========================================================================*/

#ifndef __lhpOpTransform_H__
#define __lhpOpTransform_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "mafOpTransformInterface.h"
#include "mafGUIRollOut.h"


//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGizmoTranslate;
class mafGizmoRotate;
class mafGizmoScale;
class mafNode;
class mafGui;
class mafEvent;
class mafVMEPolyline;
class mafVMESurface;

//----------------------------------------------------------------------------
// lhpOpTransform :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpTransform: public mafOpTransformInterface
{
public:
  lhpOpTransform(const wxString &label = "Transform\tCtrl+T");
  ~lhpOpTransform(); 

  virtual void OnEvent(mafEventBase *maf_event);

  mafTypeMacro(lhpOpTransform, mafOp);

  mafOp* Copy();

  /** Return true for the acceptable vme type. */
  bool Accept(mafNode* vme);

  /** Builds operation's interface. */
  void OpRun();
  
  /** Execute the operation. */
  void OpDo();
  
  /** Makes the undo for the operation. */
  void OpUndo();
	
  void Reset();

protected:
  /** Create the GUI */
  virtual void CreateGui();
  
  void OnEventTransformGizmo(mafEventBase *maf_event);
  void OnEventTransformTextEntries(mafEventBase *maf_event);

  void OpStop(int result);

  void ShowActiveGizmo();
  void SelectRefSys();
  void ChooseRelativeRefSys();

  void UpdateTransformTextEntries();

  // Plugged objects
  mafGizmoTranslate           *m_GizmoTranslate;  
  mafGizmoRotate              *m_GizmoRotate;
  mafGizmoScale               *m_GizmoScale; 

  int m_UpdateAfterRelease;
  int m_RefSystemMode;

  mafVMEPolyline *m_TransformVME;
  mafVMEPolyline *m_LocalRefSysVME;
  mafVMEPolyline *m_LocalCenterRefSysVME;
  mafVMEPolyline *m_RelativeRefSysVME;
  mafVMEPolyline *m_RelativeCenterRefSysVME;
  mafVMEPolyline *m_ArbitraryRefSysVME;

  double m_Position[3];
  double m_Orientation[3];
  double m_Scaling[3];

  double m_OriginRefSysPosition[3];

  // Override superclass
  void RefSysVmeChanged();

  /** Postmultiply event matrix to vme abs matrix; also update Redo ivar m_NewAbsMatrix */;
  virtual void PostMultiplyEventMatrix(mafEventBase *maf_event);
};
#endif //__lhpOpTransform_H__
