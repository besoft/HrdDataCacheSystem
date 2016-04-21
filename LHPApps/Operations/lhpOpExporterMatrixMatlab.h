/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterMatrixMatlab.h,v $
  Language:  C++
  Date:      $Date: 2012-03-08 15:51:56 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Youbing Zhao
==========================================================================
  Copyright (c) 2002/2012
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpExporterMatrixMatlab_H__
#define __lhpExporterMatrixMatlab_H__


#include "mafOp.h"
#include "lhpOperationsDefines.h"

#include "matio.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;

class lhpMatlabMatrixType {
public:
	static const std::string matPoseNameSfx;
	static const std::string pointListNameSfx;
	static const std::string faceNameSfx;
};

//----------------------------------------------------------------------------
// lhpOpExporterMatrixMatlab : Export VME matrices to an Octave format
//----------------------------------------------------------------------------
/** Write a VME matrix to an Octave format. */
class LHP_OPERATIONS_EXPORT lhpOpExporterMatrixMatlab : public mafOp
{
public:
	/** Class constructor */
	lhpOpExporterMatrixMatlab(const wxString &label = "lhpOpExporterMatrixMatlab");
	/** Class destructor */
	~lhpOpExporterMatrixMatlab(); 

	mafTypeMacro(lhpOpExporterMatrixMatlab, mafOp);

	/** Export the abs matrix of the input VME to a file in octave format */
	void Write();

	/** Builds operation's interface. */
	void OpRun();

	/** event handling */
	virtual void OnEvent(mafEventBase *maf_event);

	/** Copy operation*/
	mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

protected:

	class MatlabExportException : public std::exception {
	public:
		MatlabExportException(const char * str) : std::exception(str) { }
	};

	/** Create the dialog interface for the importer. */
	virtual void CreateGui();  

	void OpStop(int result);

	int OnOK();

	void ExportPoseMatrix(mat_t * mat);
	void ExportPointList(mat_t * mat);
	void ExportFaces(mat_t * mat);

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