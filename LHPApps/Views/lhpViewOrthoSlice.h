/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewOrthoSlice.h,v $
Language:  C++
Date:      $Date: 2009-11-03 12:58:12 $
Version:   $Revision: 1.1.2.1 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpViewOrthoSlice_H__
#define __lhpViewOrthoSlice_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafViewOrthoSlice.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpPipeMeshFake;
class mafVMEMesh;

//----------------------------------------------------------------------------
// lhpViewOrthoSlice :
//----------------------------------------------------------------------------
class lhpViewOrthoSlice: public mafViewOrthoSlice
{
public:
  lhpViewOrthoSlice(wxString label = "lhpViewOrthoSlice");
  virtual ~lhpViewOrthoSlice(); 

  mafTypeMacro(lhpViewOrthoSlice, mafViewOrthoSlice);

  virtual mafView*  Copy(mafObserver *Listener, bool lightCopyEnabled = false);
  
  /** event handling, pass events to corresponding VME pipes */
  virtual void OnEvent(mafEventBase *maf_event);

	void onLutChange(mafEvent * e);

	void OnWiredActorVisibility(mafEvent * e);

	void OnBorderChange(mafEvent * e);

	void OnScalarMapActive(mafEvent * e);

	void OnUseVtkProperty(mafEvent * e);

	void OnScalars(mafEvent * e);

	void OnWireframe(mafEvent * e);

	virtual void VmeShow(mafNode *node, bool show);

	void CopyPipeInfoFromPerspectiveToSlices(mafNode *vme);

	 /** 
  Return a pointer to the visual pipe of the node passed as argument. 
  It is used in mafSideBar to plug the visual pipe's GUI in the tabbed vme panel. \sa mafSideBar*/
  virtual mafPipe* GetNodePipe(mafNode *vme);
  
protected:

	lhpPipeMeshFake *m_MeshFakePipe;
	mafVMEMesh *m_CurrentMesh;
	bool m_IgnoreCameraUpdate; 
};
#endif
