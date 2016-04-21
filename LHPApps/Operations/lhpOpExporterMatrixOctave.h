/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterMatrixOctave.h,v $
  Language:  C++
  Date:      $Date: 2012-03-08 15:51:56 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Youbing Zhao
==========================================================================
  Copyright (c) 2002/2012
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpExporterMatrixOctave_H__
#define __lhpExporterMatrixOctave_H__


#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;

class lhpOctaveMatrixType {
public:
	static const std::string matPoseNameSfx;
	static const std::string pointListNameSfx;
	static const std::string faceNameSfx;
};

//----------------------------------------------------------------------------
// lhpOpExporterMatrixOctave : Export VME matrices to an Octave format
//----------------------------------------------------------------------------
/** Write a VME matrix to an Octave format. */
class LHP_OPERATIONS_EXPORT lhpOpExporterMatrixOctave : public mafOp
{
public:
	/** Class constructor */
	lhpOpExporterMatrixOctave(const wxString &label = "lhpOpExporterMatrixOctave");
	/** Class destructor */
	~lhpOpExporterMatrixOctave(); 

	mafTypeMacro(lhpOpExporterMatrixOctave, mafOp);

	/** Export the abs matrix of the input VME to a file in octave format */
	int Write();

	/** Builds operation's interface. */
	void OpRun();

	/** event handling */
	virtual void OnEvent(mafEventBase *maf_event);

	/** Copy operation*/
	mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

protected:

	/** Create the dialog interface for the importer. */
	virtual void CreateGui();  

	void OpStop(int result);

	void OnOK();

	void ExportPoseMatrix(std::ofstream & outfile);
	void ExportPointList(std::ofstream & outfile);
	void ExportFaces(std::ofstream & outfile);

	wxString m_OutputDir;
	wxString m_OutputFileNameFullPath;

	/** the input VME */
	mafVME * m_pVme;
	/** the name of the input VME, also used as the name of the matrix */
	std::string  m_Name;

	/** export pose matrix */
	int m_bUsePoseMatrix;
	/** export point data */
	int m_bUsePointList;

};

#endif