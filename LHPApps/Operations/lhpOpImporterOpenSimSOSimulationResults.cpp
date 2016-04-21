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

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpOpImporterOpenSimSOSimulationResults.h"

#include <wx/busyinfo.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include "mafGUI.h"

#include <iostream>
#include "vtkMAFSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkFieldData.h"
#include "vtkDoubleArray.h"
#include "mafVMERoot.h"
#include "mafVMESurface.h"
#include "medVMEComputeWrapping.H"
#include "mafVMEGroup.h"
#include "mafVMEPolyline.h"

using namespace std;
//----------------------------------------------------------------------------
lhpOpImporterOpenSimSOSimulationResults::lhpOpImporterOpenSimSOSimulationResults(const wxString &label) :
mafOp(label)
	//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_IMPORTER;
	m_Canundo	= false;
	m_File		= "";
	m_ImportMultipleResults = false;
}
//----------------------------------------------------------------------------
lhpOpImporterOpenSimSOSimulationResults::~lhpOpImporterOpenSimSOSimulationResults()
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterOpenSimSOSimulationResults::Copy()   
	//----------------------------------------------------------------------------
{
	lhpOpImporterOpenSimSOSimulationResults *cp = new lhpOpImporterOpenSimSOSimulationResults(m_Label);
	cp->m_File = m_File;
	cp->m_Matrix = m_Matrix;
	return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterOpenSimSOSimulationResults::OpRun()   
	//----------------------------------------------------------------------------
{

	int result = OP_RUN_CANCEL;
	m_File = "";

	wxString ikWildcard = "OpenSim Static Optimization Results File (*.sto)|*.sto";

	wxString f = mafGetOpenFile(m_File , ikWildcard ).c_str(); 

	if(f != "")
	{
		m_File = f;
		this->Import();		
		result = OP_RUN_OK;

	}

	mafEventMacro(mafEvent(this,result));

}

//----------------------------------------------------------------------------
void lhpOpImporterOpenSimSOSimulationResults::Import()   
//----------------------------------------------------------------------------
{

	std::ostringstream stringStream;

	wxString path, localFileNameWithoutExtension, ext;

	wxSplitPath(m_File, &path, &localFileNameWithoutExtension, &ext);

	int len = localFileNameWithoutExtension.Length();
	wxString localFileNameLastTwoCharacters = localFileNameWithoutExtension.SubString(len-2,len);
	if (localFileNameLastTwoCharacters == "_0")
	{
		m_ImportMultipleResults = true;
		stringStream << "Importing multiple result files ..." << std::endl;          

	}
	else
	{
		m_ImportMultipleResults = false;
		stringStream << "Importing " << m_File.c_str() << std::endl;          
	}

	mafLogMessage(stringStream.str().c_str());

	wxString fileName = m_File;

	if (m_ImportMultipleResults == true)
	{
		wxString path, localFileNameWithoutExtension, ext;

		wxSplitPath(m_File, &path, &localFileNameWithoutExtension, &ext);

		int len = localFileNameWithoutExtension.Length();
		wxString localFileNameWithoutLastCharacter = localFileNameWithoutExtension.SubString(0, len-2);

		int fileId=0;

		wxString tmpAppend = path;
		std::string currentFileName = tmpAppend <<  '/' << localFileNameWithoutLastCharacter.c_str() << fileId << "." << ext;

		while (wxFileExists(currentFileName.c_str()))
		{
			stringStream.clear();
			stringStream << "Importing " << currentFileName.c_str() << std::endl;          
			mafLogMessage(stringStream.str().c_str());

			ImportInternal(currentFileName.c_str(), fileId);
			fileId++;
			tmpAppend = path;
			currentFileName = tmpAppend <<  '/' << localFileNameWithoutLastCharacter.c_str() << fileId << "." << ext;
		}
	}
	else if (m_ImportMultipleResults == false)
	{

		ImportInternal(fileName);	
	}
}

//----------------------------------------------------------------------------
void lhpOpImporterOpenSimSOSimulationResults::ImportInternal( wxString fileName, int timeId /*= -1*/ )
{

	if (!m_TestMode)
	{
		wxSetCursor(wxCursor(wxCURSOR_WAIT));
		mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
	}

	wxFileInputStream inputFile( fileName );
	wxTextInputStream text( inputFile );

	wxString line;
	std::vector<mafString> stringVec;

	/*
	Static Optimization
	version=1
	nRows=234
	nColumns=95
	inDegrees=no
	This file contains static optimization results.

	endheader
	time	quad_fem_r	quad_fem_l	peri_r	add_long_r	add_brev_r	glut_med1_r	glut_med2_r	glut_med3_r	glut_min1_r	glut_min2_r	glut_min3_r	add_mag1_r	add_mag2_r	add_mag3_r	pect_r	psoas_r	glut_max1_r	glut_max2_r	glut_max3_r	iliacus_r	gem_r	rect_fem_r	tfl_r	sar_r	grac_r	bifemlh_r	semiten_r	vas_int_r	vas_lat_r	vas_med_r	bifemsh_r	med_gas_r	lat_gas_r	soleus_r	tib_ant_r	tib_post_r	per_long_r	per_brev_r	per_tert_r	semimem_r	peri_l	pect_l	add_long_l	add_brev_l	add_mag1_l	add_mag2_l	add_mag3_l	gem_l	glut_min1_l	glut_min2_l	glut_min3_l	glut_med1_l	glut_med2_l	glut_med3_l	glut_max1_l	glut_max2_l	glut_max3_l	iliacus_l	psoas_l	rect_fem_l	tfl_l	semimem_l	semiten_l	sar_l	grac_l	vas_int_l	vas_lat_l	vas_med_l	bifemsh_l	bifemlh_l	lat_gas_l	med_gas_l	tib_ant_l	tib_post_l	per_tert_l	per_long_l	per_brev_l	soleus_l	FX	FY	FZ	MX	MY	MZ	hip_flexion_r_reserve	hip_adduction_r_reserve	hip_rotation_r_reserve	knee_angle_r_reserve	ankle_angle_r_reserve	hip_flexion_l_reserve	hip_adduction_l_reserve	hip_rotation_l_reserve	knee_angle_l_reserve	ankle_angle_l_reserve	Force R_transformedP	Force L_transformedP
	*/

	line = text.ReadLine(); 
	assert (line.Find("Static Optimization") != -1);

	line = text.ReadLine();
	assert(line.Contains("version="));

	line = text.ReadLine();
	assert(line.Contains("nRows="));
	int findPos = line.Find("=");
	wxString dataRowsString = line.SubString(findPos+1,line.Length()); 

	int dataRows;
	dataRows = atoi(dataRowsString.c_str());
	//  assert(dataRows == 156 );

	line = text.ReadLine();
	assert(line.Contains("nColumns="));
	findPos = line.Find("=");
	wxString dataColumnsString = line.SubString(findPos+1,line.Length()); 
	int dataColumns;
	dataColumns = atoi(dataColumnsString.c_str());
	//  assert(dataColumns == 11 );

	line = text.ReadLine();
	assert (line.Find("inDegrees=") != -1);

	line = text.ReadLine();

	while (line.Find("endheader") == -1)
	{
		line = text.ReadLine();
	}


	line = text.ReadLine(); 
	assert(line.find("time") != -1);

	wxStringTokenizer tkz2(line,wxDEFAULT_DELIMITERS);
	int num;
	num = tkz2.CountTokens();
	// assert(num == 8);

	vector<string> allColumnNames;
	while (tkz2.HasMoreTokens())
	{
		allColumnNames.push_back(tkz2.GetNextToken().c_str());
	}

	if (allColumnNames.size() != dataColumns)
	{
		if (m_TestMode == false)
		{
			std::ostringstream stringStream;
			stringStream << "Column names:  " << allColumnNames.size() << 
				" different from data columns: "<< dataColumns << std::endl;          
			
			stringStream << "Please fix your data: " <<
				fileName.c_str() << std::endl;          

			wxString message = stringStream.str().c_str();

			if (!m_TestMode)
			{
				mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
			}

			wxMessageBox(message);
			return;
		}
	}

	//// assert(allColumnNames.size() == 8);
	//assert(allColumnNames[0] == "time");
	//assert(allColumnNames[1] == "quad_fem_r");
	//assert(allColumnNames[2] == "quad_fem_l");
	//assert(allColumnNames[dataColumns - 1] == "ankle_angle_l_reserve");
	//// assert(allColumnNames[1] == "xRot_R_ThighToground_moment");

	m_Matrix.clear();
	m_Matrix.set_size(dataRows , dataColumns);

	wxString token;

	// fill the data matrix
	for (int row = 0; row < dataRows; row++)
	{
		int column = 0;

		line = text.ReadLine();
		wxStringTokenizer tkz3(line,wxDEFAULT_DELIMITERS);

		assert(tkz3.CountTokens() == dataColumns);

		while (tkz3.HasMoreTokens())
		{
			token = tkz3.GetNextToken();

			std::istringstream stm;
			stm.str(token.c_str());
			double d;
			stm >> d;

			m_Matrix(row,column) = d; 
			column++;
		}

		assert(column == dataColumns);

		if (!m_TestMode)
		{
			mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,(long)(((double) row)/((double) dataRows-1)*100.)));
		}
	} 

	double val = m_Matrix(0,0);
//	assert(m_Matrix(0,0) == -0.00000000);
	double tmp = m_Matrix(dataRows - 1 , dataColumns - 1 );
//	assert(m_Matrix(dataRows - 1 , dataColumns - 1 ) == -0.0016181000000000001);

	// for each muscle find the corresponding vme in the tree

	int numberOfMuscles = allColumnNames.size() - 1;

	for (int column = 1; column < numberOfMuscles; column++)
	{
		string muscleName = allColumnNames[column];

		// search the ith muscle in the tree
		mafNode *root = NULL;
		root = m_Input->GetRoot();

		mafNode* muscle = NULL;

		muscle = root->FindInTreeByName(muscleName.c_str());

		if (muscle != NULL)
		{
			assert(mafVMEPolyline::SafeDownCast(muscle));

			// if I found the muscle add the array

			vtkFieldData *fieldDataToAdd = vtkFieldData::New(); // create the field data for the simulation output

			wxString prefix = "OpenSimSOOutput_";

			// create the time array field data
			vtkDoubleArray *timeArray = vtkDoubleArray::New();

			wxString timeArrayName = prefix + "time";
			timeArray->SetName(timeArrayName);
			timeArray->SetNumberOfValues(dataColumns);

			for (int row = 0 ; row < dataRows; row++)
			{
				timeArray->InsertValue(row, m_Matrix.get(row, 0));
			}

			fieldDataToAdd->AddArray(timeArray);
			timeArray->Delete();

			// create field data data array
			vtkDoubleArray *doubleArray = vtkDoubleArray::New();

			wxString arrayName;
			arrayName = prefix;
			arrayName.Append(muscleName.c_str());

			if (m_ImportMultipleResults == false)
			{
				// single result
			}
			else
			{
				// append _0 , _1 , ... _N for multiple results
				arrayName.append('_');
				arrayName << timeId;
			}

			doubleArray->SetName(arrayName);
			doubleArray->SetNumberOfValues(dataRows);

			for (int row = 0; row < dataRows; row++) // for each result column 
			{
				doubleArray->InsertValue(row, m_Matrix.get(row, column));
			}

			// add the ith data array to the field data
			fieldDataToAdd->AddArray(doubleArray);

			//clean up
			doubleArray->Delete();

			// add the field data to the muscle

			////////////////////////

			std::ostringstream stringStream;
			stringStream << "" << std::endl;
			stringStream << "//--------------------------------------------------------------------" << std::endl;          
			stringStream << "  Adding SO simulation results " << std::endl;          
			stringStream << "//--------------------------------------------------------------------" << std::endl;          
			stringStream << "ID simulation CoordinateSet added to VME: " << muscle->GetName() << std::endl;          

			mafLogMessage(stringStream.str().c_str());

			mafVMEPolyline* muscleVME = mafVMEPolyline::SafeDownCast(muscle);

			muscleVME->GetOutput()->GetVTKData()->Update();

			// if the array exists overwrite otherwise add
			vtkFieldData* oldFieldData = muscleVME->GetOutput()->GetVTKData()->GetFieldData();
			if (oldFieldData == NULL)
			{
				// create field data
				muscleVME->GetOutput()->GetVTKData()->SetFieldData(fieldDataToAdd);
			}
			else
			{
				// add to existing field data
				int numArraysToAdd = fieldDataToAdd->GetNumberOfArrays();

				for (int i=0; i< numArraysToAdd; i++)
				{
					vtkDataArray *arrayToAdd = fieldDataToAdd->GetArray(i);
					oldFieldData->AddArray(arrayToAdd);				
				}
			}

			fieldDataToAdd->Delete();
		}


	}

	if (!m_TestMode)
	{
		mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
	}
}

