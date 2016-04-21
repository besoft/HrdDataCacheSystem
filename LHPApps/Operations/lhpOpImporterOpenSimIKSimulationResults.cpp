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

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpOpImporterOpenSimIKSimulationResults.h"

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

using namespace std;
//----------------------------------------------------------------------------
lhpOpImporterOpenSimIKSimulationResults::lhpOpImporterOpenSimIKSimulationResults(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_IMPORTER;
	m_Canundo	= false;
	m_File		= "";
	m_ImportMultipleResults = false;
}
//----------------------------------------------------------------------------
lhpOpImporterOpenSimIKSimulationResults::~lhpOpImporterOpenSimIKSimulationResults()
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterOpenSimIKSimulationResults::Copy()   
//----------------------------------------------------------------------------
{
	lhpOpImporterOpenSimIKSimulationResults *cp = new lhpOpImporterOpenSimIKSimulationResults(m_Label);
	cp->m_File = m_File;
	cp->m_Matrix = m_Matrix;
	return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterOpenSimIKSimulationResults::OpRun()   
//----------------------------------------------------------------------------
{
	
	int result = OP_RUN_CANCEL;
	m_File = "";
	
	wxString ikWildcard = "OpenSim Inverse Kinematics Result File (*.mot)|*.mot";

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
void lhpOpImporterOpenSimIKSimulationResults::Import()   
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

mafNode* lhpOpImporterOpenSimIKSimulationResults::GetRefSysInChild( mafNode* inNode, wxString &inVmeGroupNameToFind, mafNode* outRefsys )
{
	mafNode *n = NULL;

	n = m_Input->GetRoot();
	n = n->FindInTreeByName(inVmeGroupNameToFind.c_str());
	assert(n != NULL);

	n = n->FindInTreeByName("Joints");
	assert(n != NULL);

	int numChildren = n->GetNumberOfChildren();

	for (int i = 0; i < numChildren; i++)
	{
		// get the vme containing _in_child
		wxString name = n->GetChild(i)->GetName();
		int found = name.Find("_in_child");

		if (found != -1)
		{
			outRefsys = n->GetChild(i);
			break;
		}
	}

	cout << " ";
	return outRefsys;
}

void lhpOpImporterOpenSimIKSimulationResults::ImportInternal( wxString fileName, int timeId )
{
	if (!m_TestMode)
	{
		wxSetCursor(wxCursor(wxCURSOR_WAIT));
		mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
	}

	/*wxString resultsName = m_File;

	if (m_ImportMultipleResults == true)
	{

	wxString path, localFileNameWithoutExtension, ext;

	wxSplitPath(resultsName, &path, &localFileNameWithoutExtension, &ext);

	int len = localFileNameWithoutExtension.Length();
	wxString localFileNameWithoutLastCharacter = localFileNameWithoutExtension.SubString(0, len-1);

	int fileId=0;
	wxString currentFileName = path + wxString('/') + localFileNameWithoutLastCharacter + wxString(fileId);

	ifstream inputFile( currentFileName );

	if (inputFile)
	{

	}
	else
	{

	}
	}
	*/

	ifstream inputFile( fileName );

	string line;
	std::vector<mafString> stringVec;

	/*
	/////////////////////////
	//OLD
	/////////////////////////
	first trial
	nRows=156
	nColumns=131

	# SIMM Motion File Header:
	name first trial
	datacolumns 131
	datarows 156
	otherdata 1
	range 0.400000 1.175000
	endheader
	time	R_ThighToPelvisCoordinateSetX	R_ThighToPelvisCoordinateSetY	R_ThighToPelvisCoordinateSetZ	R_ShankToR_ThighCoordinateSet	PelvisToground_xRotation	PelvisToground_yRotation	PelvisToground_zRotation	PelvisToground_xTranslation	PelvisToground_yTranslation	PelvisToground_zTranslation	RASIS_tx	RASIS_ty	RASIS_tz	LASIS_tx	LASIS_ty	LASIS_tz	SACRO_tx	SACRO_ty	SACRO_tz	RGT_tx	RGT_ty	RGT_tz	RLE_tx	RLE_ty	RLE_tz	RME_tx	RME_ty	RME_tz	RTT_tx	RTT_ty	RTT_tz	RLM_tx	RLM_ty	RLM_tz	RMM_tx	RMM_ty	RMM_tz	RHF_tx	RHF_ty	RHF_tz	RCA_tx	RCA_ty	RCA_tz	LGT_tx	LGT_ty	LGT_tz	LLE_tx	LLE_ty	LLE_tz	LME_tx	LME_ty	LME_tz	LTT_tx	LTT_ty	LTT_tz	LLM_tx	LLM_ty	LLM_tz	LMM_tx	LMM_ty	LMM_tz	LHF_tx	LHF_ty	LHF_tz	LCA_tx	LCA_ty	LCA_tz	RVM_tx	RVM_ty	RVM_tz	RFM_tx	RFM_ty	RFM_tz	LFM_tx	LFM_ty	LFM_tz	LVM_tx	LVM_ty	LVM_tz	RELBOW_tx	RELBOW_ty	RELBOW_tz	RWRIST_tx	RWRIST_ty	RWRIST_tz	LELBOW_tx	LELBOW_ty	LELBOW_tz	LWRIST_tx	LWRIST_ty	LWRIST_tz	C7_tx	C7_ty	C7_tz	RACROMIUM_tx	RACROMIUM_ty	RACROMIUM_tz	L5_tx	L5_ty	L5_tz	LACROMIUM_tx	LACROMIUM_ty	LACROMIUM_tz	MIDASIS_tx	MIDASIS_ty	MIDASIS_tz	RIE_tx	RIE_ty	RIE_tz	RIM_tx	RIM_ty	RIM_tz	RIMETATARSI_tx	RIMETATARSI_ty	RIMETATARSI_tz	LIE_tx	LIE_ty	LIE_tz	LIM_tx	LIM_ty	LIM_tz	LIMETATARSI_tx	LIMETATARSI_ty	LIMETATARSI_tz	RHIPCTR_tx	RHIPCTR_ty	RHIPCTR_tz	LHIPCTR_tx	LHIPCTR_ty	LHIPCTR_tz

	/////////////////////////
	//NEW
	/////////////////////////
	Coordinates
	version=1
	nRows=73
	nColumns=24
	inDegrees=yes

	Units are S.I. units (second, meters, Newtons, ...)
	Angles are in degrees.

	endheader
	time	pelvis_tilt ....
	*/

	while (line.find("nRows") == -1)
	{

		if (getline(inputFile , line))
		{
			continue;
		}
		else
		{
			std::ostringstream stringStream;
			stringStream << "Problems importing results: check " << m_File.c_str() << " results file" << std::endl;          
			wxMessageBox(stringStream.str().c_str());
			mafLogMessage(stringStream.str().c_str());
			return;
		}
	}

	int equal = line.find("=");
	wxString nRowsString = line.substr(equal+1,line.length()).c_str(); 

	int rows;
	rows = atoi(nRowsString.c_str());

	getline(inputFile , line); 

	equal = line.find("=");
	string nColumnsString = line.substr(equal+1,line.length()); 
	int columns;
	columns = atoi(nColumnsString.c_str());


	while (line.find("endheader") == -1)
	{
		if (getline(inputFile , line))
		{
			continue;
		}
	}

	wxString lineWx = line.c_str();

	//wxStringTokenizer tkz(lineWx,wxDEFAULT_DELIMITERS);
	//wxString token = tkz.GetNextToken();
	//assert(token == "range");

	//token = tkz.GetNextToken(); 
	//double low = atof(token.c_str());
	//// assert(low == 0.400000);

	//token = tkz.GetNextToken(); 
	//double high = atof(token.c_str());
	//// assert(high == 1.175000);

	//getline(inputFile , line); 
	//assert(line.find("endheader") != -1);

	getline(inputFile , line); 
	assert(line.find("time") != -1);

	lineWx = line.c_str();

	wxStringTokenizer tkz2(lineWx,wxDEFAULT_DELIMITERS);
	int num;
	num = tkz2.CountTokens();
	// assert(num == 131);

	vector<string> allColumnNames;
	while (tkz2.HasMoreTokens())
	{
		allColumnNames.push_back(tkz2.GetNextToken().c_str());
	}

	// assert(allColumnNames.size() == 131);
	// assert(allColumnNames[0] == "time");
	// assert(allColumnNames[1] == "R_ThighToPelvisCoordinateSetX");

	m_Matrix.clear();
	m_Matrix.set_size(rows , columns);

	// fill the data matrix
	for (int row = 0; row < rows; row++)
	{
		int column = 0;

		getline(inputFile , line);

		lineWx = line.c_str();

		wxStringTokenizer tkz3(lineWx,wxDEFAULT_DELIMITERS);

		assert(tkz3.CountTokens() == columns);

		while (tkz3.HasMoreTokens())
		{
			wxString token = tkz3.GetNextToken();

			std::istringstream stm;
			stm.str(token.c_str());
			double d;
			stm >> d;

			m_Matrix(row,column) = d; 
			column++;
		}

		assert(column == columns);

		if (!m_TestMode)
		{
			mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,(long)(((double) row)/((double) rows-1)*100.)));
		}
	} 

	double val = m_Matrix(0,0);
	// assert(m_Matrix(0,0) == 0.4);
	//assert(m_Matrix(columns - 1 , rows - 1 ) == 0.27010800);


	int column = -1;

	int numFieldData = 0;

	// for each column ...
	for (column = 0; column < columns ; column++)
	{
		bool fillFieldData = false;

		// allColumnNames = ("time","R_ThighToPelvisCoordinateSetX","R_ThighToPelvisCoordinateSetY","R_ThighToPelvisCoordinateSetZ","R_ShankToR_ThighCoordinateSet","PelvisToground_xRotation","PelvisToground_yRotation","PelvisToground_zRotation","PelvisToground_xTranslation","Pelv...
		string columnLabel = allColumnNames[column]; // R_ThighToPelvisCoordinateSetX

		string jointName;
		vector<string> columnLabelVector; 
		vector<int> columnIdVector;

		bool foundCoordinateSet = false;
		wxString coordinateSetName = "";

		if (columnLabel.find("xRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "xRot_";
		}
		else if (columnLabel.find("yRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "yRot_";
		}
		else if (columnLabel.find("zRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "zRot_";
		}
		else if (columnLabel.find("xTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "xTr_";
		}
		else if (columnLabel.find("yTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "yTr_";
		}
		else if (columnLabel.find("zTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "zTr_";
		}

		if (foundCoordinateSet == true)
		{
			int tmpColumn = column; 
			size_t pos;
			pos = columnLabel.find(coordinateSetName.c_str());
			jointName = columnLabel.substr(coordinateSetName.length()); // R_ThighToPelvis

			// search all other joint columns 
			string coordinateName;
			coordinateName = columnLabel.substr(0, coordinateSetName.length()); // CoordinateSetX

			assert(coordinateName == coordinateSetName.c_str());

			columnLabelVector.push_back(columnLabel);
			columnIdVector.push_back(tmpColumn);

			bool searchingForSameJoint = true;
			bool onLastColumn = false;

			int lastColumn = allColumnNames.size() - 1;

			if (tmpColumn == lastColumn)
			{
				onLastColumn = true;
			}

			if (!onLastColumn)
			{
				while (searchingForSameJoint) // R_ThighToPelvis
				{
					tmpColumn = tmpColumn + 1;

					// get the next coordinates name
					columnLabel = allColumnNames[tmpColumn];

					pos = columnLabel.find(jointName);

					if (pos != string::npos)
					{
						// found another column for the same joint
						searchingForSameJoint = true;

						columnLabelVector.push_back(columnLabel);
						columnIdVector.push_back(tmpColumn);

					}
					else
					{
						searchingForSameJoint = false;
					}
				}

				column = tmpColumn - 1;

			}

			// R_ThighToPelvis
			// time   CoordinateSetX	CoordinateSetY	CoordinateSetZ

			// create 4 field data
			// OpenSimIKResults_R_ThightToPelvisTimeStamp
			// 0.1 0.2 0.4 0.6 1 2 6 7.8
			// OpenSimIKResults_R_ThighToPelvisCoordinateSetX
			// 1 3 6 9 10 24 36q
			// OpenSimIKResults_R_ThighToPelvisCoordinateSetY
			// 2 59 9 94 4 6 8 10 55 67
			// OpenSimIKResults_R_ThighToPelvisCoordinateSetZ
			// 10 59 9 94 4 6 8 10 55 67

			vtkFieldData *fieldDataToAdd = vtkFieldData::New(); // create the field data for the simulation output

			wxString prefix = "OpenSimIKOutput_";

			// create the time array field data
			vtkDoubleArray *timeArray = vtkDoubleArray::New();

			wxString timeArrayName = prefix + "time";
			timeArray->SetName(timeArrayName);
			timeArray->SetNumberOfValues(rows);

			for (int row = 0 ; row < rows; row++)
			{
				timeArray->InsertValue(row, m_Matrix.get(row, 0));
			}

			fieldDataToAdd->AddArray(timeArray);
			timeArray->Delete();

			// create field data data array
			for (int i = 0; i < columnIdVector.size(); i++) // for each result column 
			{
				// create the ith data array
				vtkDoubleArray *doubleArray = vtkDoubleArray::New();

				wxString arrayName;
				arrayName = prefix;
				arrayName.Append(columnLabelVector[i].c_str());

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
				doubleArray->SetNumberOfValues(rows);

				for (int row = 0; row < rows; row++)
				{
					// fill ith data array with jth value 
					doubleArray->InsertValue(row, m_Matrix.get(row, columnIdVector[i]));
				}

				// add the ith data array to the field data
				fieldDataToAdd->AddArray(doubleArray);

				//clean up
				doubleArray->Delete();
			}

			pos = jointName.find("To");
			assert(pos != string::npos);

			wxString inVmeGroupNameToFind = jointName.substr(0, pos).c_str(); // R_ThighToPelvis

			// search R_Thigh
			// get the parent
			// get the joints child
			// get the _in_child ref sys vmeSurface
			// add the field data to the _in_child vmeSurface

			mafNode* outRefsys = NULL;

			outRefsys = GetRefSysInChild(m_Input, inVmeGroupNameToFind, outRefsys);

			mafVMESurface *surface = mafVMESurface::SafeDownCast(outRefsys);

			assert(surface != NULL);

			std::ostringstream stringStream;
			stringStream << "" << std::endl;
			stringStream << "//--------------------------------------------------------------------" << std::endl;          
			stringStream << "  Adding IK simulation results " << std::endl;          
			stringStream << "//--------------------------------------------------------------------" << std::endl;          
			stringStream << "IK simulation CoordinateSet added to VME: " << surface->GetName() << std::endl;          

			mafLogMessage(stringStream.str().c_str());

			surface->GetOutput()->GetVTKData()->Update();

			// if the array exists overwrite otherwise add
			vtkFieldData* oldFieldData = surface->GetOutput()->GetVTKData()->GetFieldData();
			if (oldFieldData == NULL)
			{
				// create field data
				surface->GetOutput()->GetVTKData()->SetFieldData(fieldDataToAdd);
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
