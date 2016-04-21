/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: mafVMEShortcut.h,v $
Language:  C++
Date:      $Date: 2011-04-11 07:17:59 $
Version:   $Revision: 1.1.2.2 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2011
University of West Bohemia
=========================================================================*/
#ifndef __mafVMEShortcut_h
#define __mafVMEShortcut_h
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVMEGroup.h"
#include "lhpVMEDefines.h"
#include "wx/event.h"
#include <assert.h>
//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafGUIScrolledPanel;

/** mafVMEShortcut - a VME that provides a shortcut to another VME, using GUIs, Output, etc. 
of this other VME. Link may be removed, or created at any time. */
class LHP_VME_EXPORT mafVMEShortcut : public mafVMEGroup
{
public:	
	mafTypeMacro(mafVMEShortcut,mafVMEGroup);
	
public:
	static const char* ShortcutVME_Link;
	enum SHORTCUT_WIDGET_ID
	{
		ID_UPDATE_TARGETLINK = Superclass::ID_LAST,	//updates the link
		ID_DELETE_TARGETLINK,												//removes the link
		ID_GOTO_TARGETLINK,													//goto the link
		
		ID_LAST,
	};


protected:
	wxTextCtrl* m_targetVMECtrl;	
	wxButton* m_bttnUpdate;
	wxButton* m_bttnDelete;
	wxButton* m_bttnGoto;
	mafGUIScrolledPanel* m_GUI_VME;	

public:

	/** Gets the VME that is currently linked by this shortcut */
	inline mafVME* GetTargetVME() {		
		return mafVME::SafeDownCast(GetLink(ShortcutVME_Link));
	}

	/** Sets the VME that is currently linked by this shortcut */
	inline void SetTargetVME(mafVME* vme) {		
		assert(mafVMEShortcut::SafeDownCast(vme) == NULL);	//may NOT create LINKS to LINKS

		SetLink(ShortcutVME_Link, vme);		
	}

	/** Sets the VME that is currently linked by this shortcut  to NULL*/
	inline void RemoveTargetVME() {
		RemoveLink(ShortcutVME_Link);		
	}

	/** This routine should be called after the target link has changed to make sure
	that GUIs, visual pipes, etc. update to  reflect the change. */
	void UpdateMAFGUIs();
	

	/** return the right type of output */
	/*virtual*/ mafVMEOutput *GetOutput();

	
	/** Return the suggested pipe-typename for the visualization of this vme */
	/*virtual*/ mafString GetVisualPipe() {

		mafVME* vme = GetTargetVME();		
		return (vme == NULL) ? 
			__super::GetVisualPipe() :
			/*mafString("mafPipeBox") : */vme->GetVisualPipe();
	};

	/** Process events coming from other objects */ 
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	/** return icon */
	static char** GetIcon();
protected:
	/** Accepts target VME */
	static bool VMEAcceptTargetVME(mafNode *node);	

protected:
	mafVMEShortcut();
	virtual ~mafVMEShortcut();	
	

	/** Create GUI for the VME */
	/*virtual*/ mafGUI *CreateGui();

	/** Called when TargetLink is requested to be updated */
	virtual void OnUpdateTargetLink();

	/** Called when TargetLink is requested to be deleted */
	virtual void OnDeleteTargetLink();
	
	/** Called when TargetLink is requested to be selected */
	virtual void OnGotoTargetLink();

	/** Called to update VME GUI using the GUI provided by the passed vme (may be NULL)*/
	virtual void UpdateGui(mafVME* vme);

	/** This method is called upon redrawing the GUI. 
	It is here to ensure that the target VME GUI will be correctly displayed*/
	void OnPaintGui();
	friend class mafVMEShortcutPaintGuiEvnt;

private:
	mafVMEShortcut(const mafVMEShortcut&); // Not implemented
	void operator=(const mafVMEShortcut&); // Not implemented		
};
#endif
