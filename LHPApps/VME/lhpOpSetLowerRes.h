/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpSetLowerRes.h,v $
  Language:  C++
  Date:      $Date: 2011-03-25 11:47:55 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/



#ifndef __lhpOpSetLowerRes_H__
#define __lhpOpSetLowerRes_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGUIDialog;
class mafVME;

//----------------------------------------------------------------------------
// lhpOpSetLowerRes :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_VME_EXPORT  lhpOpSetLowerRes: public mafOp
{
public:

  static const char* GetLinkNameLR();
  static const char* GetLinkNameHULL();

protected:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  enum GUI_IDS
  {
    ID_LOWER_RES_VME = MINID,
    ID_UPDATE_LR,
    ID_REMOVE_LR,
    ID_HULL_RES_VME,
    ID_UPDATE_HULL,
    ID_REMOVE_HULL,   

    ID_OK,
    ID_CANCEL,

    LAST_ID,
  };


public:
  lhpOpSetLowerRes(const wxString &label = "Set Lower Resolution VME");
  ~lhpOpSetLowerRes(); 

  mafTypeMacro(lhpOpSetLowerRes, mafOp);

	/*virtual*/ void OnEvent(mafEventBase *maf_event);  

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

  /** Makes the undo for the operation. */
  /*virtual*/ void OpUndo();

	/*Sets a new lower resolution VME and updates GUI (if available).
	NB. you need to call OpDo to confirm the changes. */
	virtual void SetNewLowerResVME(mafVME* vme);

	/*Sets a new hull VME and updates GUI (if available). 
	NB. you need to call OpDo to confirm the changes. */
	virtual void SetNewHullVME(mafVME* vme);

protected:
	 /** Creates GUI including renderer window */
  virtual void CreateOpDialog();

  /** Destroys GUI */
  virtual void DeleteOpDialog(); 

	/** Callback for VME_CHOOSE that accepts any VME */
  static bool SelectVMECallback(mafNode *node);    

  //GUI handlers
protected:
	/** Handles the selection of  lower resolution or hull VME */
	virtual void OnUpdateLinkedVME(bool bLowResVME);

	/** Handles the removal of  lower resolution or hull VME */
	virtual void OnRemoveLinkedVME(bool bLowResVME);

protected:
	mafVME* m_SelectedLowResVME;			///< Pointer to the selected  VME (if exists) with lower resolution
	mafVME* m_SelectedHullVME;				///< Pointer to the selected  VME (if exists) with the hull	
	
	mafID m_UndoLowResVMEId;				///< ID of the previous VME (for Undo)
	mafID m_UndoHullVMEId;					///< ID of the previous VME (for Undo)

	mafGUIDialog* m_Dialog;									///<dialog - GUI
	mafString m_SelectedLowResVMEName;			///< Pointer to the selected  VME (if exists) with lower resolution
	mafString m_SelectedHullVMEName;				///< Pointer to the selected  VME (if exists) with the hull
};
#endif //__lhpOpSetLowerRes_H__