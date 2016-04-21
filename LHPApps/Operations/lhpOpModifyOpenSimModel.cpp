/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModel.cpp,v $
Language:  C++
Date:      $Date: 2012-04-20 10:58:02 $
Version:   $Revision: 1.1.2.16 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModel.h"

#include <wx/mimetype.h>

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafNode.h"

#include "mafVMEExternalData.h"
#include "lhpUtils.h"
#include "mafGUI.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include "mafOpExporterMSF.h"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"
#include <iosfwd>
#include <string>
#include "xercesc/dom/DOMNodeIterator.hpp"
#include "xercesc/dom/DOMDocumentTraversal.hpp"
#include "xercesc/dom/DOMWriter.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "wx/dirdlg.h"
#include "mafVMESurface.h"
#include "mafDataVector.h"
#include "mafTagArray.h"
#include "mafVMERefSys.h"
#include "mafVMELandmarkCloud.h"
#include "wx/busyinfo.h"
#include "medVMEComputeWrapping.H"

using namespace xercesc;
using namespace std;

const string UNDEFINED_OUTPUT_MODEL_DIRECTORY = "UNDEFINED_m_OutputModelDirectory";
#define UNDEFINED_STRING "UNDEFINED";

const wxString OPENSIM_COMPONENTS_LOCAL_DIR = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\CreateOpenSimComponents\\";

const wxString NOTEPADPP_LOCAL_FILENAME = "\\ExternalTools\\NotepadPlusPlus5_9\\notepad++.exe";

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModel);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModel::lhpOpModifyOpenSimModel(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL;
	m_Debug = 0;

	m_NotepadPlusPlusOSIMStuffLocalDir = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\";

	m_GeneratedOpenSimModelLocalDir = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\GeneratedOpenSimModel\\";

	m_SinglePassMultipleReplaceFileName = "SinglePassMultipleReplace.py";

	m_DictionaryToConfigureFileName = UNDEFINED_STRING;
	m_DataFromNMSBuilderFileName = UNDEFINED_STRING;
	m_ConfiguredDictionaryFileName = UNDEFINED_STRING;

	m_ComponentToConfigureFileName = UNDEFINED_STRING;
	m_ConfiguredComponentFileName = UNDEFINED_STRING;

	m_OpenSimDir = "UNDEFINED_m_OpenSimDir";
	m_OpenSimVersion = "UNDEFINED_m_OpenSimVersion";
}
//----------------------------------------------------------------------------
lhpOpModifyOpenSimModel::~lhpOpModifyOpenSimModel( )
//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModel::Copy()
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModel(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModel::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	mafVMEExternalData *inputVMEExternalData = NULL;
	inputVMEExternalData = mafVMEExternalData::SafeDownCast(node);

	bool externalDataIsOpenSimInputFile = false;

	if (inputVMEExternalData != NULL)
	{
		wxString externalDataABSFileName = inputVMEExternalData->GetAbsoluteFileName();

		wxString path, localFileNameWithoutExtension, ext;

		wxSplitPath(externalDataABSFileName, &path, &localFileNameWithoutExtension, &ext);

		wxString openSimExtension = "cpp";

		if (ext == openSimExtension)
		{
			externalDataIsOpenSimInputFile = true;
		}
	}

	return (node != NULL && node->IsMAFType(mafVMEExternalData) && externalDataIsOpenSimInputFile);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModel::OpRun()
//----------------------------------------------------------------------------
{
	m_OutputModelDirectory = GetOutputModelDirectory();

	CreateGui();
	ShowGui();
}
//----------------------------------------------------------------------------
long lhpOpModifyOpenSimModel::GetPid()
//----------------------------------------------------------------------------
{
	return m_Pid;
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModel::CreateGui()
//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

	mafEvent buildHelpGui;
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	if (buildHelpGui.GetArg() == true)
	{
		m_Gui->Button(ID_HELP, "Help","");	
	}

	m_Gui->Bool(ID_DEBUG_MODE , "Debug", &m_Debug);
	m_Gui->Divider(2);
	m_Gui->Label("Generate Model", true);
	m_Gui->Label("");
	m_Gui->Button(ID_EDIT_MODEL_SOURCE, "Edit Model Source");
//	m_Gui->Label("");
//	m_Gui->Button(ID_VME_REFERENCE_IN_MODEL_EDITOR, "VME Reference in Model Editor");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_MODEL, "Generate Model");
	m_Gui->Label("");
	m_Gui->Divider(2);
	m_Gui->Label("Post processing facilities", true);
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_OUTPUT_MODEL_DIRECTORY, "Open output model files dir");
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_MSF_DIRECTORY, "Open MSF directory");
	m_Gui->Label("");

	//	m_Gui->Button(ID_VIEW_MODEL_IN_OPENSIM, "View Model in OpenSIM");
	//	m_Gui->Label("");
	m_Gui->OkCancel();
}

void lhpOpModifyOpenSimModel::EditOpenSIMFile()
{
	wxString openSimCPPModelABSFileName;
	openSimCPPModelABSFileName = ((mafVMEExternalData *)(this->m_Input))->GetAbsoluteFileName().GetCStr();

	OpenFileWithNotepadPP(openSimCPPModelABSFileName);
}

void lhpOpModifyOpenSimModel::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{
		
		case ID_HELP:
		{
			mafEvent helpEvent;
			helpEvent.SetSender(this);
			mafString operationLabel = this->m_Label;
			helpEvent.SetString(&operationLabel);
			helpEvent.SetId(OPEN_HELP_PAGE);
			mafEventMacro(helpEvent);
		}
		break;

		case ID_EDIT_MODEL_SOURCE:
			{
				EditOpenSIMFile();
			}
			break;

		case ID_GENERATE_MODEL:
			{
				BuildOpenSIMFile();
			}
			break;

		case ID_VME_REFERENCE_IN_MODEL_EDITOR:
			{
				OpenVmeXMLInModelEditor();
			}
			break;

		case ID_VIEW_MODEL_IN_OPENSIM:
			{
				ViewModelInOpenSim();
			}
			break;

		case ID_OPEN_OUTPUT_MODEL_DIRECTORY:
			{
				OnOpenOutputModelDirectory();
			}
			break;

		case ID_OPEN_MSF_DIRECTORY:
			{
				OnOpenMSFDirectory();
			}
			break;

		case wxOK:
			{
				OpStop(OP_RUN_OK);
			}
			break;

		case wxCANCEL:
			{
				OpStop(OP_RUN_CANCEL);
			}
			break;
		}
	}
}

void lhpOpModifyOpenSimModel::BuildOpenSIMFile()
{
	mafEvent eventRequestOpenSimDir;
	eventRequestOpenSimDir.SetSender(this);
	eventRequestOpenSimDir.SetId(ID_REQUEST_OPENSIM_DIR);
	mafEventMacro(eventRequestOpenSimDir);

	if(eventRequestOpenSimDir.GetString())
	{
		m_OpenSimDir.Erase(0);
		m_OpenSimDir = eventRequestOpenSimDir.GetString()->GetCStr();
	}

	bool openSimDirExists = wxDirExists(m_OpenSimDir.GetCStr());
	if (!openSimDirExists)
	{
		std::ostringstream stringStream;
		stringStream  << "OpenSim dir " << m_OpenSimDir.GetCStr() << " not found. Cannot build OpenSim model! Please check your Tools -> Options -> OpenSim Settings " << std::endl;
		wxMessageBox(stringStream.str().c_str());
		mafLogMessage(stringStream.str().c_str());
		return;
	}

	mafEvent eventRequestOpenSimVersion;
	eventRequestOpenSimVersion.SetSender(this);
	eventRequestOpenSimVersion.SetId(ID_REQUEST_OPENSIM_VERSION);
	mafEventMacro(eventRequestOpenSimVersion);

	if(eventRequestOpenSimVersion.GetString())
	{
		m_OpenSimVersion.Erase(0);
		m_OpenSimVersion = eventRequestOpenSimVersion.GetString()->GetCStr();
	}

	wxBusyInfo wait("Please wait...");

	bool prerequisitesOk = CheckPrerequisites();

	if (prerequisitesOk == true)
	{
		// continue
	}
	else
	{
		return;
	}

	wxString applicationDirectory = lhpUtils::lhpGetApplicationDirectory();

	//-----------
	// 	Create backup copy of TugOfWar1_CreateModel.cpp
	// 	copy "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel.cpp" "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel_BAK.cpp"

	wxString openSIMExampleABSFileName = m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel.cpp";
	assert(wxFileExists(openSIMExampleABSFileName.c_str()));

	wxString openSIMExampleBackupABSFileName = m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModelCpp_BAK";

	wxString preBuildBatchFileABSName = applicationDirectory + m_NotepadPlusPlusOSIMStuffLocalDir + "PreBuild.bat";

	wxFileExists(preBuildBatchFileABSName);

	std::ofstream preBuildBatchFile(preBuildBatchFileABSName.c_str(), std::ios::out);

	if (preBuildBatchFile.fail())
	{
		wxMessageBox("Error creating file");
		return;
	}

	preBuildBatchFile << "REM " << std::endl;
	preBuildBatchFile
		<< "COPY " << '"' << openSIMExampleABSFileName.c_str() << '"' << " " << '"' << openSIMExampleBackupABSFileName.c_str() << '"' << " /Y" << std::endl;

	//-----------
	// 	Copy current OpenSim API file to openSiM example dir renamed as TugOfWar1_CreateModel.cpp (default openSim example)
	// 	copy "c:\\b3c_software\\nmsBuilder_Today\\data\\OpenSimCache\\OpenSimModel_2B31NhDeF9.cpp" "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel.cpp"

	wxString myModelABSFileName = ((mafVMEExternalData *)(this->m_Input))->GetAbsoluteFileName().GetCStr();
	assert(wxFileExists(myModelABSFileName.c_str()));

	preBuildBatchFile << "REM " << std::endl;
	preBuildBatchFile
		<< "COPY " << '"' << myModelABSFileName.c_str() << '"' << " " << '"' << openSIMExampleABSFileName.c_str() << '"' << " /Y" << std::endl;

	if (m_Debug == true)
	{
		preBuildBatchFile
			<< "pause" << std::endl;
	}

	preBuildBatchFile.close();

	assert(wxFileExists(preBuildBatchFileABSName.c_str()));

	wxArrayString output;
	wxArrayString errors;
	long pid = -1;

	if (m_Debug == false)
	{
		pid = wxExecute(preBuildBatchFileABSName, output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute(preBuildBatchFileABSName, wxEXEC_SYNC);
	}

	wxBusyInfo wait1("Please wait...");

	assert(wxFileExists(openSIMExampleBackupABSFileName.c_str()));
	assert(wxFileExists(openSIMExampleABSFileName.c_str()));

	//-----------
	// pre build clean up
	//-----------

	wxString outputModelInOpenSimDir = m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain\\outputModel.osim";
	wxRemoveFile(outputModelInOpenSimDir);
	assert(wxFileExists(outputModelInOpenSimDir) == false);

	wxString outputModelGeneratorEXEInOpenSimDir = "UNDEFINED_m_OpenSimVersion";

	if (m_OpenSimVersion == "OPENSIM_221" || m_OpenSimVersion == "OPENSIM_240")
	{
		outputModelGeneratorEXEInOpenSimDir =  m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain\\RelWithDebInfo\\TugOfWar1_CreateModel.exe";
	}
	else if (m_OpenSimVersion == "OPENSIM_300")
	{
		outputModelGeneratorEXEInOpenSimDir =  m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain\\RelWithDebInfo\\exampleMain.exe";
	}
	else
	{
		assert(false);
	}

	wxRemoveFile(outputModelGeneratorEXEInOpenSimDir);
	assert(wxFileExists(outputModelGeneratorEXEInOpenSimDir) == false);

	wxString copiedOutputModelGeneratorEXEInNMSDir = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\generatedExe.exe";
	wxString copiedGeneratorExeABSFileName = applicationDirectory + copiedOutputModelGeneratorEXEInNMSDir;

	wxRemoveFile(copiedGeneratorExeABSFileName);
	assert(wxFileExists(copiedGeneratorExeABSFileName) == false);

	//-----------
	// build the openSim api exe
	//-----------

	wxString ext, mime, command2execute;
	wxString buildOsimCppBatchLocalFileName = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\BuildOpenSimSourceFile.bat";
	wxString buildOsimCppBatchABSFileName = applicationDirectory + buildOsimCppBatchLocalFileName;

	std::ofstream buildOsimCppBatchFile(buildOsimCppBatchABSFileName.c_str(), std::ios::out);

	if (buildOsimCppBatchFile.fail())
	{
		wxMessageBox("Error creating file");
		return;
	}

	if (m_OpenSimVersion == "OPENSIM_221")
	{
		CreateBuildScriptOpenSim221_240_VisualStudio2008(buildOsimCppBatchFile);
	}
	else if (m_OpenSimVersion == "OPENSIM_240")
	{
		CreateBuildScriptOpenSim221_240_VisualStudio2008(buildOsimCppBatchFile);
	}
	else if (m_OpenSimVersion == "OPENSIM_300")
	{
		CreateBuildScriptOpenSim300_VisualStudio2010(buildOsimCppBatchFile);
	}

	if (m_Debug == true)
	{
		buildOsimCppBatchFile
			<< "pause" << std::endl;
	}

	buildOsimCppBatchFile.close();

	assert(wxFileExists(buildOsimCppBatchABSFileName.c_str()));

	command2execute = buildOsimCppBatchABSFileName;

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	if (m_Debug == false)
	{
		pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute(command2execute, wxEXEC_SYNC);
	}

	wxBusyInfo wait2("Please wait...");

	//----------------------------------

	//----------------------------------
	// post build
	//----------------------------------

	wxString postBuildBatchFileABSName = applicationDirectory + "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\postBuild.bat";

	std::ofstream postBuildBatchFile(postBuildBatchFileABSName.c_str(), std::ios::out);

	if (postBuildBatchFile.fail())
	{
		wxMessageBox("Error creating file");
		return;
	}

	//-----------
	//  REM Restore backup file
	// 	COPY "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel_BAK.cpp" "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\TugOfWar1_CreateModel.cpp"

	postBuildBatchFile << "REM " << std::endl;
	postBuildBatchFile
		<< "COPY " << '"' << openSIMExampleBackupABSFileName.c_str() << '"' << " " << '"' << openSIMExampleABSFileName.c_str() << '"' << " /Y" << std::endl;

	//-----------
	// 		REM copy the generated exe file to the nmsBuilder data dir
	// 		COPY "c:\\OpenSim2.2.1\\sdk\\APIExamples\\ExampleMain\\RelWithDebInfo\\TugOfWar1_CreateModel.exe" "c:\\b3c_software\\nmsBuilder_Today\\data\\OpenSimCache\\OpenSimModel_2B31NhDeF9.exe"

	assert(wxFileExists(outputModelGeneratorEXEInOpenSimDir) == true);

	// debug mode

	postBuildBatchFile << "REM " << std::endl;
	postBuildBatchFile
		<< "COPY " << '"' << outputModelGeneratorEXEInOpenSimDir.c_str() << '"' << " " << '"' << copiedGeneratorExeABSFileName.c_str() << '"' << " /Y" << std::endl;

	if (m_Debug == true)
	{
		postBuildBatchFile
			<< "pause" << std::endl;
	}

	//	wxRemoveFile(openSIMExampleBackupABSFileName);
	//	assert(wxFileExists(openSIMExampleBackupABSFileName) == false);

	postBuildBatchFile.close();

	assert(wxFileExists(postBuildBatchFileABSName.c_str()));

	if (m_Debug == false)
	{
		pid = wxExecute(postBuildBatchFileABSName, output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute(postBuildBatchFileABSName, wxEXEC_SYNC);
	}

	wxBusyInfo wait3("Please wait...");

	assert(wxFileExists(openSIMExampleABSFileName.c_str()));
	assert(wxFileExists(copiedGeneratorExeABSFileName.c_str()));

	// generate testModel.osim
	wxString OUTPUT_MODEL_FILE_NAME= GetOutputModelFileName();
	wxString generatedModelLocalFileName = "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\" + OUTPUT_MODEL_FILE_NAME;
	wxString generatedModelABSFileName = applicationDirectory + generatedModelLocalFileName;

	bool generatedModelExists = false;
	generatedModelExists = wxFileExists(generatedModelABSFileName);

	if (generatedModelExists)
	{
		wxRemoveFile(generatedModelABSFileName.c_str());
	}

	generatedModelExists = wxFileExists(generatedModelABSFileName);

	assert(generatedModelExists == false);

	wxString oldWD = wxGetCwd();
	wxString npposstuff = applicationDirectory + "\\ExternalTools\\NotepadPlusPlusOSIMStuff\\";
	wxSetWorkingDirectory(npposstuff);

	// run the generatedExe.exe executable to create the outputModel.osim file

	if (m_Debug == false)
	{
		pid = wxExecute("generatedExe.exe", output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute("generatedExe.exe", wxEXEC_SYNC);
	}

	wxBusyInfo wait4("Please wait...");

	wxSetWorkingDirectory(oldWD);

	generatedModelExists = wxFileExists(generatedModelABSFileName);

	if (generatedModelExists)
	{
		// continue
	}
	else
	{
		std::ostringstream stringStream;
		stringStream << "Canot generate the model " << generatedModelABSFileName.c_str() << std::endl;
		stringStream << "To debug your model goto \"Editing Tools\" and check the" << std::endl;
		stringStream << "\"Debug\" option then press \"Generate Model\" to check for errors" << std::endl;
		stringStream << "Remember that osim output model name written with the osimModel.print API must be " <<
			OUTPUT_MODEL_FILE_NAME.c_str() << std::endl;
		string txt = stringStream.str().c_str();
		wxMessageBox(txt.c_str());
		return;
	}

	assert(wxFileExists(generatedModelABSFileName));

	wxString notepadPPLocalFileName = "\\ExternalTools\\NotepadPlusPlus5_9\\notepad++.exe";
	wxString notepadPPExeABSFileName = applicationDirectory + notepadPPLocalFileName;

	assert(wxFileExists(notepadPPExeABSFileName.c_str()));
	assert(wxFileExists(generatedModelABSFileName.c_str()));

	wxString modelFileToOpenInEditor = "UNDEFINED";

	if (m_OutputModelDirectory != UNDEFINED_OUTPUT_MODEL_DIRECTORY.c_str())
	{
		wxString copyFileName = GetOutputModelDirectory() + "\\" + OUTPUT_MODEL_FILE_NAME;
		wxCopyFile(generatedModelABSFileName, copyFileName);
		assert(wxFileExists(copyFileName));
		modelFileToOpenInEditor = copyFileName;
	}
	else
	{
		modelFileToOpenInEditor = generatedModelABSFileName;
	}

	// open the generated openSim model in Notepad++
	command2execute = notepadPPExeABSFileName + " " + modelFileToOpenInEditor;

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	// open model in editor
	m_Pid = wxExecute(command2execute); //,FALSE
}

void lhpOpModifyOpenSimModel::ViewModelInOpenSim()
{
	throw std::exception("The method or operation is not implemented.");
}

bool lhpOpModifyOpenSimModel::CheckPrerequisites()
{
	wxString nmsBuilderInstallationDocumentURL = "https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit?hl=en_US";

	wxString openSimExampleDir = m_OpenSimDir + "\\sdk\\APIExamples\\ExampleMain";
	bool openSimInstallationOk = wxDirExists(openSimExampleDir);
	assert(openSimInstallationOk);

	if (openSimInstallationOk)
	{
		// continue ...
	}
	else
	{
		wxString message = m_OpenSimDir + " Examples Installation Dir: " + openSimExampleDir + " not found. You will now be redirected to the NMSBuilder installation guide. Please \
																							follow the" + m_OpenSimDir + " Installation section";
		wxMessageBox(message);

		wxLaunchDefaultBrowser(nmsBuilderInstallationDocumentURL);
		return false;
	}

	wxString cmakeExePath = "c:\\CMake 2.8\\bin\\cmake.exe";
	bool cmakeInstallationOk = wxFileExists(cmakeExePath);

	assert(cmakeInstallationOk);

	if (cmakeInstallationOk)
	{
		// continue ...
	}
	else
	{
		wxString message = "CMake 2.8.3 Executable: " + cmakeExePath + " not found. You will now be redirected to the NMSBuilder installation guide. Please \
																	   follow the CMake 2.8.3 Installation section";

		wxMessageBox(message);

		wxLaunchDefaultBrowser(nmsBuilderInstallationDocumentURL);
		return false;
	}

	#ifndef USE_OPENSIM_API

	wxString envVariable = "PROGRAMFILES";
	wxString value;
	wxGetEnv(envVariable, &value);

	value.Append("\\Microsoft Visual Studio 9.0\\Common7\\Tools\\vsvars32.bat");

	wxString vsvars32 = value;
	bool vs9InstallationOk = wxFileExists(vsvars32);
	assert(vs9InstallationOk);

	if (vs9InstallationOk)
	{
		// continue ...
	}
	else
	{
		wxString message = "Visual Studio 2008 vsvars32: " + vsvars32 + " not found. You will now be redirected to the NMSBuilder installation guide. Please \
																		follow the Visual Studio 2008 Express Installation section";

		wxMessageBox(message);

		wxLaunchDefaultBrowser(nmsBuilderInstallationDocumentURL);
		return false;
	}

	#endif

	return true;
}

void lhpOpModifyOpenSimModel::OpenVmeXMLInModelEditor()
{
	mafString title = mafString("Select a VME:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	mafEventMacro(e);
	mafVME *vme = (mafVME *)e.GetVme();

	if (vme == NULL)
	{
		return;
	}

	assert(vme);

	int selectedVmeId = vme->GetId();

	mafEvent event;
	event.SetSender(this);
	event.SetId(ID_MSF_DATA_CACHE);
	mafEventMacro(event);

	wxString msfABSFileName;
	msfABSFileName.Append((*event.GetString()).GetCStr());

	string msfToParse = msfABSFileName.c_str();

	wxString  xmlFileNameOut = lhpUtils::lhpGetApplicationDirectory();
	xmlFileNameOut.Append(m_NotepadPlusPlusOSIMStuffLocalDir);
	xmlFileNameOut.Append("\\");
	xmlFileNameOut.Append(vme->GetName());
	xmlFileNameOut.Append(".xml");

	XercesDOMParser *parser = new XercesDOMParser;
	parser->parse(msfToParse.c_str());

	DOMNode *vmeNode = NULL;
	DOMNode *vmesParentNode = NULL;
	DOMElement *selectedVMENode = NULL;

	GetVMENodeById(parser->getDocument(), selectedVmeId, selectedVMENode);

	assert(selectedVMENode);

	DOMNode *selectedVMEChildrenNode = NULL;

	// remove the Children vme node since it is not needed for editing
	GetChildrenByTagName(selectedVMENode, "Children", selectedVMEChildrenNode);

	assert(selectedVMEChildrenNode != NULL);

	selectedVMENode->removeChild(selectedVMEChildrenNode);

	// DOMImplementationLS contains factory methods for creating objects
	// that implement the DOMBuilder and the DOMWriter interfaces
	static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
	DOMImplementation *impl =
		DOMImplementationRegistry::getDOMImplementation(gLS);

	// construct the DOMWriter
	DOMWriter* myWriter = ((DOMImplementationLS*)impl)->createDOMWriter();

	// construct the LocalFileFormatTarget
	XMLFormatTarget *myFormatTarget = new LocalFileFormatTarget(xmlFileNameOut.c_str());

	// serialize a DOMNode to the local file "myXMLFile.xml"
	myWriter->writeNode(myFormatTarget, *selectedVMENode);

	// optionally, you can flush the buffer to ensure all contents are written
	myFormatTarget->flush();

	// release the memory
	myWriter->release();

	// clean up
	delete myFormatTarget;
	delete parser;

	OpenFileWithNotepadPP(xmlFileNameOut);
}

void lhpOpModifyOpenSimModel::GetChildrenByTagName( DOMNode* inParentNode , string inTagName, DOMNode * &outputChild )
{
	DOMNodeList* children = inParentNode->getChildNodes();

	const  XMLSize_t nodeCount = children->getLength();

	// for all children ...
	for( XMLSize_t childrenId = 0; childrenId < nodeCount; ++childrenId )
	{
		DOMNode* currentNode = children->item(childrenId);
		if( currentNode->getNodeType() &&  // true is not NULL
			currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element
		{
			// Found node which is an Element. Re-cast node as element
			DOMElement* currentElement
				= dynamic_cast< xercesc::DOMElement* >( currentNode );

			string tagName = XMLString::transcode(currentElement->getTagName());

			if (inTagName == tagName)
			{
				// found
				outputChild = currentNode;
				return;
			}
		}
	}

	// not found
	outputChild = NULL;
}

void lhpOpModifyOpenSimModel::GetVMENodeById( xercesc::DOMDocument* inXMLDoc, int inVmeId, DOMElement * &outVmeNode )
{
	std::ostringstream stringStream;
	stringStream << inVmeId;
	string vmeIdString = stringStream.str().c_str();

	DOMElement *root = NULL;
	root = inXMLDoc->getDocumentElement();
	assert(root);

	DOMNode *current = NULL;

	// create an iterator to visit all nodes
	DOMNodeIterator *iterator = inXMLDoc->createNodeIterator(root, DOMNodeFilter::SHOW_ELEMENT, NULL, true);

	for ( current = iterator->nextNode(); current != 0; current = iterator->nextNode() )
	{
		DOMElement* currentElement
			= dynamic_cast< xercesc::DOMElement* >( current );

		// 		std::ostringstream stringStream;
		// 		stringStream << XMLString::transcode(currentElement->getTagName()) << std::endl;
		// 		mafLogMessage(stringStream.str().c_str());

		string tagName = XMLString::transcode(currentElement->getTagName());
		//			mafLogMessage(tagName.c_str());

		if (tagName == string("Node")) // is a vme
		{
			if (XMLString::transcode(currentElement->getAttribute(XMLString::transcode("Id"))) == vmeIdString)
			{
				// vme found
				outVmeNode = currentElement;
				return;
			}
		}
	}

	outVmeNode = NULL;
}

void lhpOpModifyOpenSimModel::OpenFileWithNotepadPP( wxString &absFileName )
{
	wxString ext, mime, command2execute;

	wxString applicationDirectory = lhpUtils::lhpGetApplicationDirectory();

	wxString notepadPPExeABSFileName = applicationDirectory + NOTEPADPP_LOCAL_FILENAME;

	assert(wxFileExists(notepadPPExeABSFileName.c_str()));

	assert(wxFileExists(absFileName.c_str()));

	command2execute = notepadPPExeABSFileName + " " + absFileName;

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );
	wxExecute(command2execute); //,FALSE
}

void lhpOpModifyOpenSimModel::OnOpenOutputModelDirectory()
{
	if (m_OutputModelDirectory != UNDEFINED_OUTPUT_MODEL_DIRECTORY.c_str())
	{
		wxString command = "explorer ";
		command << "\"";
		command << GetOutputModelDirectory();
		command << "\"";

		// open windows explorer in model directory
		wxExecute(command);
	}
}

void lhpOpModifyOpenSimModel::OnOpenMSFDirectory()
{
	mafEvent event;
	event.SetSender(this);
	event.SetId(ID_MSF_DATA_CACHE);
	mafEventMacro(event);

	wxString msfABSFileName;
	msfABSFileName.Append((*event.GetString()).GetCStr());

	wxString path, localFileNameWithoutExtension, ext;

	wxSplitPath(msfABSFileName, &path, &localFileNameWithoutExtension, &ext);

	wxUnix2DosFilename(const_cast<char*>((const char*)path.mb_str()));

	if (path != "")
	{
		wxString command = "explorer ";
		command << "\"";
		command << path.c_str();
		command << "\"";

		// open windows explorer in model directory
		wxExecute(command);
	}
}

void lhpOpModifyOpenSimModel::GetPythonInterpreters()
{
	// get python interpreters
	mafEvent eventGetPythonExe;
	eventGetPythonExe.SetSender(this);
	eventGetPythonExe.SetId(ID_REQUEST_PYTHON_EXE_INTERPRETER);
	mafEventMacro(eventGetPythonExe);

	if(eventGetPythonExe.GetString())
	{
		m_PythonwExeFullPath.Erase(0);
		m_PythonwExeFullPath = eventGetPythonExe.GetString()->GetCStr();
		m_PythonwExeFullPath.Append(" ");
	}

	mafEvent eventGetPythonwExe;
	eventGetPythonwExe.SetSender(this);
	eventGetPythonwExe.SetId(ID_REQUEST_PYTHONW_EXE_INTERPRETER);
	mafEventMacro(eventGetPythonwExe);

	if(eventGetPythonwExe.GetString())
	{
		m_PythonwExeFullPath.Erase(0);
		m_PythonwExeFullPath = eventGetPythonwExe.GetString()->GetCStr();
		m_PythonwExeFullPath.Append(" ");
	}
}

wxString lhpOpModifyOpenSimModel::GetOpenSimAPITemplatesDir()
{
	wxString applicationDirectory = lhpUtils::lhpGetApplicationDirectory();

	wxString createOpenSimComponentsDir = applicationDirectory + OPENSIM_COMPONENTS_LOCAL_DIR;

	assert(wxDirExists(createOpenSimComponentsDir));

	return createOpenSimComponentsDir;
}

wxString lhpOpModifyOpenSimModel::GetOutputModelDirectory()
{
	wxString applicationDirectory = lhpUtils::lhpGetApplicationDirectory();

	wxString generatedOpenSimModelABSDir = applicationDirectory + m_GeneratedOpenSimModelLocalDir;

	bool dirExists = wxDirExists(generatedOpenSimModelABSDir);

	if (dirExists)
	{
		// continue
	}
	else
	{
		wxMkDir(generatedOpenSimModelABSDir);
	}

	assert(wxDirExists(generatedOpenSimModelABSDir));

	return generatedOpenSimModelABSDir;
}

void lhpOpModifyOpenSimModel::ConfigureOSIMComponentDictionaryWithNMSBuilderData()
{
	GetPythonInterpreters();

	// dictionary to configure must exist
	wxString dictionaryToConfigureABSFileName = GetOpenSimAPITemplatesDir() + m_DictionaryToConfigureFileName;
	assert(wxFileExists(dictionaryToConfigureABSFileName));

	// along with data file written from NMSBuilder
	wxString dataFromNMSBuilderABSFileName = GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName;
	assert(wxFileExists(dataFromNMSBuilderABSFileName));

	wxString command2execute;

	// Configure dictionary :
	// OSIM_AddBlockToModel_Dictionary.txt.in
	// with NMSBuilder data:
	// OSIM_AddBlockToModel_DataFromNMSBuilder.txt
	// and generate:
	// OSIM_AddBlockToModel_Dictionary.txt

	mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

	wxString oldWorkingDir = wxGetCwd();

	wxSetWorkingDirectory(GetOpenSimAPITemplatesDir());

	mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

	assert(wxFileExists(m_PythonwExeFullPath.GetCStr()));

	command2execute = m_PythonwExeFullPath.GetCStr();
	command2execute.Append(" ");
	command2execute.Append(m_SinglePassMultipleReplaceFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_DictionaryToConfigureFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_DataFromNMSBuilderFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_ConfiguredDictionaryFileName.c_str());

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	wxArrayString output;
	wxArrayString errors;
	long pid = -1;

	if (m_Debug = 0)
	{
		pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute(command2execute, wxEXEC_SYNC);
	}

	wxString configuredDictionaryABSFileName = GetOpenSimAPITemplatesDir() + m_ConfiguredDictionaryFileName.c_str();
	assert(wxFileExists(configuredDictionaryABSFileName));

	assert(wxDirExists(oldWorkingDir));
	wxSetWorkingDirectory(oldWorkingDir);

	mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
}

wxString lhpOpModifyOpenSimModel::GetVTKFileNameAbsPath(mafVMESurface *vme)
{
	assert(vme);
	wxString url = vme->GetDataVector()->GetItem(0)->GetURL();

	mafEvent event;
	event.SetSender(this);
	event.SetId(ID_MSF_DATA_CACHE);
	mafEventMacro(event);

	wxString msfABSFileName;
	msfABSFileName.Append((*event.GetString()).GetCStr());

	assert(msfABSFileName != "");

	wxString path, fileName, ext;
	wxSplitPath(msfABSFileName, &path, &fileName, &ext);

	wxString surfaceVtkAbsPath = path + "/" + url;
	assert(wxFileExists(surfaceVtkAbsPath));

	assert(true);

	return surfaceVtkAbsPath;
}

void lhpOpModifyOpenSimModel::ConfigureOSIMComponentWithOSIMComponentDictionary()
{
	GetPythonInterpreters();

	wxString componentABSFileName = GetOpenSimAPITemplatesDir() + m_ComponentToConfigureFileName;

	assert(wxFileExists(componentABSFileName));

	wxString command2execute;

	mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

	wxString oldWorkingDir = wxGetCwd();

	wxSetWorkingDirectory(GetOpenSimAPITemplatesDir());

	mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

	assert(wxFileExists(m_PythonwExeFullPath.GetCStr()));

	command2execute = m_PythonwExeFullPath.GetCStr();
	command2execute.Append(" ");
	command2execute.Append(m_SinglePassMultipleReplaceFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_ComponentToConfigureFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_ConfiguredDictionaryFileName.c_str());
	command2execute.Append(" ");
	command2execute.Append(m_ConfiguredComponentFileName.c_str());

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	wxArrayString output;
	wxArrayString errors;
	long pid = -1;

	if (m_Debug = 0)
	{
		pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC);
	}
	else
	{
		pid = wxExecute(command2execute, wxEXEC_SYNC);
	}

	assert(wxFileExists(GetOpenSimAPITemplatesDir() + m_ConfiguredComponentFileName.c_str()));

	assert(true);

	assert(wxDirExists(oldWorkingDir));
	wxSetWorkingDirectory(oldWorkingDir);

	mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
}

void lhpOpModifyOpenSimModel::GenerateOpenSimComponentAPIAndOpenItInNotepadPP()
{
	GenerateOpenSimComponentAPI();
	OpenFileWithNotepadPP(GetOpenSimAPITemplatesDir() + m_ConfiguredComponentFileName);
}

void lhpOpModifyOpenSimModel::GenerateOpenSimComponentAPI()
{
	WriteNMSBuilderDataToFile();
	ConfigureOSIMComponentDictionaryWithNMSBuilderData();
	ConfigureOSIMComponentWithOSIMComponentDictionary();
}

wxString lhpOpModifyOpenSimModel::GetGeneratedComponentAbsFileName()
{
	wxString generatedComponentsABSFileName = GetOpenSimAPITemplatesDir() + m_ConfiguredComponentFileName;

	assert(wxFileExists(generatedComponentsABSFileName));

	return generatedComponentsABSFileName;
}

wxString lhpOpModifyOpenSimModel::GetOpenSimComponentText()
{
	wxString componentABSFileName = lhpOpModifyOpenSimModel::GetGeneratedComponentAbsFileName();

	assert(wxFileExists(componentABSFileName));

	std::ifstream file( componentABSFileName );

	std::stringstream buffer;

	if ( file )
	{
		buffer << file.rdbuf();

		file.close();
	}

	mafLogMessage(buffer.str().c_str());

	return buffer.str().c_str();
}

wxString lhpOpModifyOpenSimModel::FileToString( wxString absFileName )
{
	assert(wxFileExists(absFileName));

	std::ifstream file( absFileName );

	std::stringstream buffer;

	if ( file )
	{
		buffer << file.rdbuf();

		file.close();
	}

	mafLogMessage(buffer.str().c_str());

	wxString returnString = buffer.str().c_str();

	return returnString;
}

void lhpOpModifyOpenSimModel::StringToFile(wxString inText , wxString outAbsFileName )
{
	ofstream myfile;
	myfile.open (outAbsFileName.c_str());
	myfile << inText.c_str();
	myfile.close();
}

bool lhpOpModifyOpenSimModel::VMESurfaceAcceptForBodyGeneration(mafNode *node)
{
	bool hasInertialTensorComponents = node->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	bool hasMass = node->GetTagArray()->IsTagPresent("SURFACE_MASS");
	bool hasCenterOfMass = node->GetTagArray()->IsTagPresent("LOCAL_CENTER_OF_MASS_COMPONENTS");
	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;
	return hasInertialTensorComponents && hasMass && hasCenterOfMass && isVmeSurface;
}

bool lhpOpModifyOpenSimModel::VMEAcceptAsBody(mafNode *node)
{
	bool hasInertialTensorComponents = node->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	bool hasMass = node->GetTagArray()->IsTagPresent("SURFACE_MASS");
	bool hasCenterOfMass = node->GetTagArray()->IsTagPresent("LOCAL_CENTER_OF_MASS_COMPONENTS");
	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;
	bool isVmeGroup = mafVMEGroup::SafeDownCast(node) != NULL;

	return hasInertialTensorComponents && hasMass && hasCenterOfMass && (isVmeSurface || isVmeGroup);
}

bool lhpOpModifyOpenSimModel::VMEGroupAcceptForBodyGeneration(mafNode *node)
{
	mafVMEGroup *group = mafVMEGroup::SafeDownCast(node);
	if (group == NULL)
	{
		return  false;
	}

	bool hasInertialTensorComponents = group->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	if (hasInertialTensorComponents == false)
	{
		return false;
	}

	bool hasMass = group->GetTagArray()->IsTagPresent("SURFACE_MASS");
	if (hasMass == false)
	{
		return false;
	}

	bool hasCenterOfMass = group->GetTagArray()->IsTagPresent("LOCAL_CENTER_OF_MASS_COMPONENTS");
	if (hasCenterOfMass == false)
	{
		return false;
	}

	// all children must have SURFACE_MASS tag
	int numChildren = group->GetNumberOfChildren();

	for (int i=0;i<numChildren;i++)
	{
		mafNode *currentChild = group->GetChild(i);

		if (currentChild->IsMAFType(mafVMESurface))
		{
			bool childHasMass = currentChild->GetTagArray()->IsTagPresent("SURFACE_MASS");

			if (childHasMass == false)
			{
				return false;
			}
		}
	}

     return true;
}

wxString lhpOpModifyOpenSimModel::GetOpenSimComponentTextToBePasted()
{
	wxString componentABSFileName = GetGeneratedComponentAbsFileName();

	return FileToString(componentABSFileName);
}

bool lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept(mafNode *node)
{
	bool isVmeRefSys = mafVMERefSys::SafeDownCast(node) != NULL;

	if (isVmeRefSys)
	{
		return true;
	}

	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;

	if (isVmeSurface)
	{
		return true;
	}

	return false;
}

bool lhpOpModifyOpenSimModel::VMELandmarkCloudAcccept(mafNode *node)
{
	bool isVmeLandmarkCloud = mafVMELandmarkCloud::SafeDownCast(node) != NULL;
	return isVmeLandmarkCloud;
}

bool lhpOpModifyOpenSimModel::VMELandmarkAccept( mafNode *node )
{
	bool isVmeLandmark = mafVMELandmark::SafeDownCast(node) != NULL;
	return isVmeLandmark;
}

mafVME * lhpOpModifyOpenSimModel::ChooseRefSysVME()
{
	mafString title = mafString("VME Ref Sys:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());

	if (vme == NULL)
	{
		return NULL;
	}

	return vme;
}

bool lhpOpModifyOpenSimModel::VMEMeterAccept(mafNode *node)
{
	bool isMeter = medVMEComputeWrapping::SafeDownCast(node) != NULL;
	return isMeter;
}

void lhpOpModifyOpenSimModel::CreateBuildScriptOpenSim221_240_VisualStudio2008( std::ofstream &buildOsimCppBatchFile )
{
	buildOsimCppBatchFile << "REM Must be on C: drive" << std::endl;
	buildOsimCppBatchFile << "C:" << std::endl;
	buildOsimCppBatchFile << "REM change dir to OpenSim examples" << std::endl;
	buildOsimCppBatchFile << "cd \"" << m_OpenSimDir.GetCStr() << "\\sdk\\APIExamples\\ExampleMain\"" << std::endl;
	buildOsimCppBatchFile << "REM run cmake to generate visual studio 2008 solution" << std::endl;
	buildOsimCppBatchFile << "\"c:\\CMake 2.8\\bin\\cmake.exe\"  -G \"Visual Studio 9 2008\"" << std::endl;
	buildOsimCppBatchFile << "REM Load visual studio 2008 environment variables" << std::endl;
	buildOsimCppBatchFile << "CALL \"%PROGRAMFILES%\\Microsoft Visual Studio 9.0\\Common7\\Tools\\vsvars32.bat\"" << std::endl;

	buildOsimCppBatchFile << "REM build the visual studio 2008 solution" << std::endl;
	buildOsimCppBatchFile << "vcbuild  OpenSimTugOfWar.sln \"RelWithDebInfo|Win32\"" << std::endl;
}

void lhpOpModifyOpenSimModel::CreateBuildScriptOpenSim300_VisualStudio2010( std::ofstream &buildOsimCppBatchFile )
{
	buildOsimCppBatchFile << "REM Must be on C: drive" << std::endl;
	buildOsimCppBatchFile << "C:" << std::endl;
	buildOsimCppBatchFile << "REM change dir to OpenSim examples" << std::endl;
	buildOsimCppBatchFile << "cd \"" << m_OpenSimDir.GetCStr() << "\\sdk\\APIExamples\\ExampleMain\"" << std::endl;
	buildOsimCppBatchFile << "REM run cmake to generate visual studio 2010 solution" << std::endl;
	buildOsimCppBatchFile << "\"c:\\CMake 2.8\\bin\\cmake.exe\"  -G \"Visual Studio 10\"" << std::endl;
	buildOsimCppBatchFile << "REM Load visual studio 2010 environment variables" << std::endl;
	buildOsimCppBatchFile << "CALL \"%PROGRAMFILES%\\Microsoft Visual Studio 10.0\\Common7\\Tools\\vsvars32.bat\"" << std::endl;

	buildOsimCppBatchFile << "REM build the visual studio 2008 solution" << std::endl;
	buildOsimCppBatchFile << "msbuild  exampleMain.vcxproj /p:Configuration=RelWithDebInfo /p:Platform=Win32" << std::endl;
}

wxString lhpOpModifyOpenSimModel::ReplaceSpaceWithUnderscore( wxString in )
{
	wxString nameWithUnderscoreInsteadOfSpaces = in;
	nameWithUnderscoreInsteadOfSpaces.Replace(" ","_");
	return nameWithUnderscoreInsteadOfSpaces;
}