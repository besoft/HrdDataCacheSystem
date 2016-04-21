/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpOpShowHistogram.h,v $
Language:  C++
Date:      $Date: 2012-02-07 09:29:44 $
Version:   $Revision: 1.1.2.3 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpOpShowHistogram_H__
#define __lhpOpShowHistogram_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class mafGUIDialog;
class mafDeviceManager;
class lhpGUIHistogramWidget;
class mafVME;
class vtkPointData;


/** 
Operation to show Histogram from VME scalars
*/
class LHP_OPERATIONS_EXPORT lhpOpShowHistogram : public mafOp
{
public:
  enum SHOW_HISTOGRAM_ID
  {
    ID_SCALARS_ARRAY_SELECTION = MINID,
    ID_BIN_NUMBER,
    ID_RESET_ZOOM,
    ID_SHOW_PERCENTAGE,
  };


  mafTypeMacro(lhpOpShowHistogram, mafOp);

	lhpOpShowHistogram(wxString label = "Show Histogram");
	~lhpOpShowHistogram(void);

	mafOp* Copy();

	/** Class for handle mafEvent*/
	virtual void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* vme);

	
	/** Builds operation's GUI by calling CreateOpDialog() method. */
	void OpRun();

	/** Execute the operation. */
	virtual void OpDo();

	/** Makes the undo for the operation. */
	void OpUndo();

	
protected:
  mafGUIDialog* m_Dialog;             //<Dialog - GUI
  mafGUI* m_GuiDialog;                //Dialog - GUI
  mafVME* m_VME;
  vtkPointData *m_PointData;

  lhpGUIHistogramWidget *m_Histogram;
  int m_SelectedScalarsArray;
  int m_BinNumber;
  int m_ShowPercentage;


  /** This method is called to update the histogram with current scalars */
  void UpdateHistogram();

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);

	/** create op GUI panel */
	void CreateOpDialog();
	
private:

};

#endif
