/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCreateOpenSimModel.cpp,v $
  Language:  C++
  Date:      $Date: 2011-06-22 17:42:30 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpOpCreateOpenSimModel.h"
#include "mafDecl.h"
#include "mafEvent.h"

#include "mafVMERoot.h"
#include "mafOpImporterExternalFile.h"
#include "vtkIOStream.h"
#include "lhpUtils.h"

#include <list>
#include "wx\tokenzr.h"
#include "wx\filename.h"
#include "lhpOpModifyOpenSimModel.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpCreateOpenSimModel);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpCreateOpenSimModel::lhpOpCreateOpenSimModel(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;
}
//----------------------------------------------------------------------------
lhpOpCreateOpenSimModel::~lhpOpCreateOpenSimModel()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
mafOp* lhpOpCreateOpenSimModel::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpCreateOpenSimModel(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpCreateOpenSimModel::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModel::OpRun()
//----------------------------------------------------------------------------
{
  wxString applicationDirectory = lhpUtils::lhpGetApplicationDirectory();
  wxString inTemplateModelABSName = applicationDirectory + "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\CreateOpenSimComponents\\templateCreateModel.cpp";

  GenerateOpenSimModelFromFile(inTemplateModelABSName , m_Input);
  mafEventMacro(mafEvent(this,OP_RUN_OK));
}
//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModel::OpDo()
//----------------------------------------------------------------------------
{
  
}

std::string lhpOpCreateOpenSimModel::GenerateRandomString(int length, bool letters, bool numbers, bool symbols) {
    // the shortest way to do this is to create a string, containing
    // all possible values. Then, simply add a random value from that string
    // to our return value
    std::string allPossible; // this will contain all necessary characters
    std::string str; // the random string
    if (letters == true) { // if you passed true for letters, we'll add letters to the possibilities
        for (int i = 65; i <= 90; i++) {
            allPossible += static_cast<char>(i);
            allPossible += static_cast<char>(i+32); // add a lower case letter, too!
        }
    } if (numbers == true) { // if you wanted numbers, we'll add numbers
        for (int i = 48; i <= 57; i++) {
            allPossible += static_cast<char>(i);
        }
    } if (symbols == true) { // if you want symbols, we'll add symbols (note, their ASCII values are scattered)
        for (int i = 33; i <= 47; i++) {
            allPossible += static_cast<char>(i);
        } for (int i = 58; i <= 64; i++) {
            allPossible += static_cast<char>(i);
        } for (int i = 91; i <= 96; i++) {
            allPossible += static_cast<char>(i);
        } for (int i = 123; i <= 126; i++) {
            allPossible += static_cast<char>(i);
        }
    }
    // get the number of characters to use (used for rand())
    int numberOfPossibilities = allPossible.length();
    for (int i = 0; i < length; i++) {
        str += allPossible[rand() % numberOfPossibilities];
    }

    return str;
}

bool lhpOpCreateOpenSimModel::wxMakePath(const wxString& path) 
{ 
	// wxMkpath is similar to wxMkdir, except it can create 
	// a complete path without requiring all but the last to already exist. 
	// The return value is true when the path already exists or is created 
	// if the path already exists, there is nothing to do 
	if(!wxDirExists(path)) { 
		std::list<wxString> tokens; 
		wxStringTokenizer tkz(path, wxT("/\\")); 
		while ( tkz.HasMoreTokens() )       { 
			wxString token = tkz.GetNextToken(); 
			tokens.push_back(token); 
		} 
		wxString sub_path; 
		for(std::list<wxString>::iterator it = tokens.begin(); it != 
			tokens.end(); it++) { 
				sub_path += *it + wxFileName::GetPathSeparator(); 
				if(!wxDirExists(sub_path)) { 
					if(!wxMkdir(sub_path))return false; 
				} 
		} 
	} 
	return true;
}

wxString lhpOpCreateOpenSimModel::GenerateOpenSimModelFromText( wxString inText, mafNode *inputVme )
{
	wxString curDir = wxGetCwd();
	wxString fileName = "GenerateOpenSimModelFromText.txt";

	wxString absFileName = curDir + fileName;

	lhpOpModifyOpenSimModel::StringToFile(inText , absFileName);

	assert(wxFileExists(absFileName.c_str()));

	wxString osimModelAbsFileName = lhpOpCreateOpenSimModel::GenerateOpenSimModelFromFile(absFileName , inputVme);

	wxRemoveFile(absFileName);
	assert(wxFileExists(absFileName.c_str()) == false);

	return osimModelAbsFileName;
}

wxString lhpOpCreateOpenSimModel::GenerateOpenSimModelFromFile( wxString &inModelFileABSName , mafNode *inputVme )
{
	wxString cacheDir = lhpUtils::lhpGetApplicationDirectory() + "\\Data\\OpenSimCache\\";

	// check for the cache dir existence
	bool cacheDirExist = wxDirExists(cacheDir.c_str());

	if (cacheDirExist == false)
	{
		std::ostringstream stringStream;
		stringStream << "creating cache dir: " << cacheDir.c_str() << std::endl;
		mafLogMessage(stringStream.str().c_str());          
		lhpOpCreateOpenSimModel::wxMakePath(cacheDir.c_str());

		assert(wxDirExists(cacheDir.c_str()));
	}
	else
	{
		std::ostringstream stringStream;
		stringStream << "found cache dir: " << cacheDir.c_str() << std::endl;
		mafLogMessage(stringStream.str().c_str());
	}

	bool openSimFileNameExists = true;

	wxString openSimModelABSFileName = "UNDEFINED_OPENSIM_MODEL_ABS_FILENAME";

	// Generate the new random OpenSim model file name
	wxString openSimModelLocalFileNameWithoutExtension = "UNDEFINED_OPENSIM_MODEL_LOCAL_FILENAME";
	while (openSimFileNameExists)
	{
		wxString baseFileName = "OpenSimModel_";
		wxString randomString = GenerateRandomString(10,true,true,false).c_str();

		openSimModelLocalFileNameWithoutExtension = baseFileName + randomString;

		wxString openSimExtension = ".cpp";

		openSimModelABSFileName = cacheDir + openSimModelLocalFileNameWithoutExtension + openSimExtension;

		openSimFileNameExists = wxFileExists(openSimModelABSFileName);
	}

	assert(wxFileExists(openSimModelABSFileName) == false);

	assert(wxFileExists(inModelFileABSName.c_str()));	

	// copy the template empty model into target openSim model file
	wxCopyFile(inModelFileABSName  , openSimModelABSFileName);

	assert(wxFileExists(openSimModelABSFileName) == true);

	// import the model text into a vme
	mafOpImporterExternalFile *externalFileImporter = new mafOpImporterExternalFile();

	externalFileImporter->SetInput(inputVme);
	externalFileImporter->SetFileName(openSimModelABSFileName.c_str());
	mafLogMessage(openSimModelABSFileName.c_str());
	externalFileImporter->ImportExternalFile();
	externalFileImporter->GetOutput()->SetName(openSimModelLocalFileNameWithoutExtension.c_str());
	mafDEL(externalFileImporter);

	return openSimModelABSFileName;
}
