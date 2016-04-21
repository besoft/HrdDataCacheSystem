/*=========================================================================

 Program: MAF2Medical
 Module: lhpOpImporterOpenSimIKSimulationResults
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __lhpOpImporterOpenSimIKSimulationResults_H__
#define __lhpOpImporterOpenSimIKSimulationResults_H__

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
// lhpOpImporterOpenSimIKSimulationResults :
//----------------------------------------------------------------------------
/** Import OpenSim IK simulation results 

Input: 

1) A custom vme tree structure representing an OpenSim model for example:

testModel.msf
\UnitTestsData\lhpOpImporterOpenSimIKSimulationResultsTest\testModel\testModel.msf
(comment in cpp file and test are related to this sample data)

available in unit test data directory

2) An IK results file from an OpenSim simulation for example:

\UnitTestsData\lhpOpImporterOpenSimIKSimulationResultsTest\OutputIK\output_ik.mot
(comment in cpp file and test are related to this sample data)

available in unit test data directory

The IK result file is read and each CoordinateSet is added to the corresponding "ref sys in child" field data: look at the test
\Testing\Operations\lhpOpImporterOpenSimIKSimulationResultsTest.h

to see how the importer works in detail

VME's to which results are added will be written in the Log Area at the end of the operation.

*/
class LHP_OPERATIONS_EXPORT lhpOpImporterOpenSimIKSimulationResults : public mafOp
{
public:

	lhpOpImporterOpenSimIKSimulationResults(const wxString &label = "lhpOpImporterOpenSimIKSimulationResults");
	~lhpOpImporterOpenSimIKSimulationResults(); 

	/** Set the OpenSim results file for importing */
	void SetFileName(const char *file_name) {m_File = file_name;};

	/** Import results in VME tree*/
	void Import();

	void ImportInternal(wxString fileName, int timeId = -1);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* node) {return true;};

	/** Builds operation's interface. */
	void OpRun();

	mafOp* Copy();

protected:

	/** Get the target "ref sys in child" vme for results adding */
	mafNode* GetRefSysInChild( mafNode* inRoot, wxString &inVmeGroupNameToFind, mafNode* outRefsys );
    vnl_matrix<double> m_Matrix;
	wxString m_File;

	bool m_ImportMultipleResults;
};
#endif
