/*=========================================================================

 Program: MAF2Medical
 Module: lhpOpImporterOpenSimSimulationResults
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
#include "lhpOpImporterOpenSimIDSimulationResults.h"

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
lhpOpImporterOpenSimIDSimulationResults::lhpOpImporterOpenSimIDSimulationResults(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_IMPORTER;
	m_Canundo	= false;
	m_File		= "";
	m_ImportMultipleResults = false;
	
}
//----------------------------------------------------------------------------
lhpOpImporterOpenSimIDSimulationResults::~lhpOpImporterOpenSimIDSimulationResults()
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterOpenSimIDSimulationResults::Copy()   
//----------------------------------------------------------------------------
{
	lhpOpImporterOpenSimIDSimulationResults *cp = new lhpOpImporterOpenSimIDSimulationResults(m_Label);
	cp->m_File = m_File;
	cp->m_Matrix = m_Matrix;
	return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterOpenSimIDSimulationResults::OpRun()   
//----------------------------------------------------------------------------
{

	int result = OP_RUN_CANCEL;
	m_File = "";

	wxString ikWildcard = "OpenSim Inverse Dynamics Results File (*.sto)|*.sto";

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
void lhpOpImporterOpenSimIDSimulationResults::ImportInternal( wxString fileName , int timeId )
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
Inverse Dynamics
nRows=156
nColumns=11
This file contains inverse dynamics results.

endheader
time	R_ThighToPelvisCoordinateSetX_moment	R_ThighToPelvisCoordinateSetY_moment	R_ThighToPelvisCoordinateSetZ_moment	R_ShankToR_ThighCoordinateSet_moment	PelvisToground_xRotation_moment	PelvisToground_yRotation_moment	PelvisToground_zRotation_moment	PelvisToground_xTranslation_force	PelvisToground_yTranslation_force	PelvisToground_zTranslation_force
*/

  line = text.ReadLine(); 
  assert (line.Find("Inverse Dynamics") != -1);

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
  assert (line.Find("endheader") != -1);

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

  // assert(allColumnNames.size() == 8);
  assert(allColumnNames[0] == "time");
  // assert(allColumnNames[1] == "xRot_R_ThighToground_moment");

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
  // assert(m_Matrix(0,0) == 0.4);
  // assert(m_Matrix(dataRows - 1 , dataColumns - 1 ) == -1.66726083);

	int column = -1;

	int numFieldData = 0;

	// for each column ...
	for (column = 0; column < dataColumns ; column++)
	{
	    bool fillFieldData = false;

		// allColumnNames = (time	R_ThighToPelvisCoordinateSetX_moment	R_ThighToPelvisCoordinateSetY_moment	R_ThighToPelvisCoordinateSetZ_moment	R_ShankToR_ThighCoordinateSet_moment	PelvisToground_xRotation_moment	PelvisToground_yRotation_moment	PelvisToground_zRotation_moment	PelvisToground_xTranslation_force	PelvisToground_yTranslation_force	PelvisToground_zTranslation_force

		string columnLabel = allColumnNames[column]; // R_ThighToPelvisCoordinateSetX_moment	

		string jointName;
		vector<string> columnLabelVector; 
		vector<int> columnIdVector;

		bool foundCoordinateSet = false;
		wxString coordinateSetName = "";

		if (columnLabel.find("xRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "xRot";
		}
		else if (columnLabel.find("yRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "yRot";
		}
		else if (columnLabel.find("zRot_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "zRot";
		}
		else if (columnLabel.find("xTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "xTr";
		}
		else if (columnLabel.find("yTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "yTr";
		}
		else if (columnLabel.find("zTr_") != string::npos)
		{
			foundCoordinateSet = true;
			coordinateSetName = "zTr";
		}

		if (foundCoordinateSet == true)
		{
			int tmpColumn = column; 
			size_t pos;
			pos = columnLabel.find(coordinateSetName.c_str());
			
			jointName = columnLabel.substr(coordinateSetName.length()+1); 

			int last = jointName.find_last_of("_");
			jointName=jointName.substr(0, last); // R_ThighToground

			// search all other joint columns 
			string coordinateName;
			int firstUnderscore = columnLabel.find_first_of("_");
			coordinateName = columnLabel.substr(0, firstUnderscore); // xRot	

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

			vtkFieldData *fieldDataToAdd = vtkFieldData::New(); // create the field data for the simulation output

			wxString prefix = "OpenSimIDOutput_";

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
				doubleArray->SetNumberOfValues(dataColumns);

				for (int row = 0; row < dataRows; row++)
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
			stringStream << "  Adding ID simulation results " << std::endl;          
			stringStream << "//--------------------------------------------------------------------" << std::endl;          
			stringStream << "ID simulation CoordinateSet added to VME: " << surface->GetName() << std::endl;          
			
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

mafNode* lhpOpImporterOpenSimIDSimulationResults::GetRefSysInChild( mafNode* inNode, wxString &inVmeGroupNameToFind, mafNode* outRefsys )
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

//----------------------------------------------------------------------------
void lhpOpImporterOpenSimIDSimulationResults::Import()   
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
