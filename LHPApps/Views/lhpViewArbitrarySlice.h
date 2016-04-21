/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewArbitrarySlice.h,v $
Language:  C++
Date:      $Date: 2009-11-03 12:58:12 $
Version:   $Revision: 1.1.2.1 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpViewArbitrarySlice_H__
#define __lhpViewArbitrarySlice_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafViewArbitrarySlice.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpPipeMeshFake;
class mafVMEMesh;

//----------------------------------------------------------------------------
// lhpViewArbitrarySlice :
//----------------------------------------------------------------------------
class lhpViewArbitrarySlice: public mafViewArbitrarySlice
{
public:
  lhpViewArbitrarySlice(wxString label = "lhpViewArbitrarySlice");
  virtual ~lhpViewArbitrarySlice(); 

  mafTypeMacro(lhpViewArbitrarySlice, mafViewArbitrarySlice);

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
