/*=========================================================================

 Program: MAF2Medical
 Module: lhpOpImporterOpenSimIDSimulationResults
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __lhpOpImporterOpenSimIDSimulationResults_H__
#define __lhpOpImporterOpenSimIDSimulationResults_H__

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
// lhpOpImporterOpenSimIDSimulationResults :
//----------------------------------------------------------------------------
/** Import OpenSim Inverse Dynamics simulation results 

Input: 

1) A custom vme tree structure representing an OpenSim model for example:

testModel.msf
\UnitTestsData\lhpOpImporterOpenSimIDSimulationResultsTest\testModel\testModel.msf
(comment in cpp file and test are related to this sample data)

available in unit test data directory

2) An ID results file from an OpenSim simulation for example:

\UnitTestsData\lhpOpImporterOpenSimIDSimulationResultsTest\InputID\InverseDynamics_Analysis1.sto
(comment in cpp file and test are related to this sample data)

available in unit test data directory

The ID result file is read and each CoordinateSet_moment is added to the corresponding "ref sys in child" field data: look at the test
\Testing\Operations\lhpOpImporterOpenSimIDSimulationResultsTest.h

to see how the importer works in detail

VME's to which results are added will be written in the Log Area at the end of the operation.

*/
class LHP_OPERATIONS_EXPORT lhpOpImporterOpenSimIDSimulationResults : public mafOp
{
public:

	lhpOpImporterOpenSimIDSimulationResults(const wxString &label = "lhpOpImporterOpenSimIDSimulationResults");
	~lhpOpImporterOpenSimIDSimulationResults(); 
		
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

	/** Import results in VME tree*/
	void ImportInternal(wxString fileName , int timeId = -1);

	/** Get the target "ref sys in child" vme for results adding */
	mafNode* GetRefSysInChild( mafNode* inRoot, wxString &inVmeGroupNameToFind, mafNode* outRefsys );

    vnl_matrix<double> m_Matrix;
	wxString m_File;

	bool m_ImportMultipleResults ;
};
#endif
