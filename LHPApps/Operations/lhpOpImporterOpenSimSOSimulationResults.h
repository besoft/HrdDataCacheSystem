/*=========================================================================

 Program: MAF2Medical
 Module: lhpOpImporterOpenSimSOSimulationResults
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __lhpOpImporterOpenSimSOSimulationResults_H__
#define __lhpOpImporterOpenSimSOSimulationResults_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------

#include "mafOp.h"
#include "lhpOperationsDefines.h"

#include <vnl\vnl_matrix.h>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMERoot;

//----------------------------------------------------------------------------
// lhpOpImporterOpenSimSOSimulationResults :
//----------------------------------------------------------------------------
/** 

	Import OpenSim Static Optimization simulation results 

*/
class LHP_OPERATIONS_EXPORT lhpOpImporterOpenSimSOSimulationResults : public mafOp
{
public:

	lhpOpImporterOpenSimSOSimulationResults(const wxString &label = "lhpOpImporterOpenSimSOSimulationResults");
	~lhpOpImporterOpenSimSOSimulationResults(); 
		
	/** Set the OpenSim results file for importing */
	void SetFileName(const char *file_name) {m_File = file_name;};

	/** Import results in VME tree*/
	void Import();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* node) {return true;};

	/** Builds operation's interface. */
	void OpRun();

	mafOp* Copy();

protected:

	void ImportInternal( wxString fileName, int timeId = -1);

    vnl_matrix<double> m_Matrix;
	wxString m_File;

	bool m_ImportMultipleResults;
};
#endif
