/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeMeshFake.h,v $
  Language:  C++
  Date:      $Date: 2009-11-03 12:58:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpPipeMeshFake_H__
#define __lhpPipeMeshFake_H__

#include "mafPipeMesh.h"
#include "mafEvent.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// forward refs :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// lhpPipeMeshFake :
//----------------------------------------------------------------------------
/** 
Visual pipe to visualize graphs of analog signals. */
class LHP_VME_EXPORT lhpPipeMeshFake : public mafPipeMesh
{
public:
	mafTypeMacro(lhpPipeMeshFake,mafPipeMesh);

	lhpPipeMeshFake();
	virtual     ~lhpPipeMeshFake ();
	void lhpPipeMeshFake::OnEvent(mafEventBase *maf_event); 

	void CopyGuiInfoFromPipe(mafPipeMesh *pipe);

	int GetScalarMapActive(){return m_ScalarMapActive;};

	int GetScalarIndex(){return m_ScalarIndex;};

	int GetWiredActorVisible(){return m_BorderElementsWiredActor;};

	/** Create the Gui for the visual pipe that allow the user to change the pipe's parameters.*/
	mafGUI *CreateGui();
protected:

private:

	wxComboBox *m_ScalarComboBox;

};  
#endif // __lhpPipeMeshFake_H__
