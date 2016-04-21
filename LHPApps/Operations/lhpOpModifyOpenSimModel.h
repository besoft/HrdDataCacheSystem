/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModel.h,v $
  Language:  C++
  Date:      $Date: 2012-04-20 10:58:02 $
  Version:   $Revision: 1.1.2.11 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModel_H__
#define __lhpOpModifyOpenSimModel_H__

using namespace std;

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "xercesc\parsers\XercesDOMParser.hpp"
#include "xercesc\dom\DOMNode.hpp"

using namespace xercesc;

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafNode;
class mafVME;
class mafOp;
class mafEvent;
class mafVMESurface;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModel :
//----------------------------------------------------------------------------
/** 

MAF - OpenSIM API integration facilities

User documentation available at:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit?hl=en_US&pli=1

*/
class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModel: public mafOp
{
public:

	enum lhpOpModifyOpenSimModel_ID
	{
		ID_FIRST = MINID,
		ID_DEBUG_MODE,
		ID_EDIT_MODEL_SOURCE,
		ID_OPEN_MSF_DIRECTORY,
		ID_VME_REFERENCE_IN_MODEL_EDITOR,
		ID_OPEN_OUTPUT_MODEL_DIRECTORY,
		ID_GENERATE_MODEL,
		ID_VIEW_MODEL_IN_OPENSIM,
		ID_HELP,
		ID_LAST
	};

  lhpOpModifyOpenSimModel(const wxString &label = "lhpOpModifyOpenSimModel");
 ~lhpOpModifyOpenSimModel();

 mafTypeMacro(lhpOpModifyOpenSimModel, mafOp);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Generate the OpenSim component C++ code */
 void GenerateOpenSimComponentAPI();

 /** Get the generated OpenSim API component C++ code to be copy-pasted for model generation */
 wxString GetOpenSimComponentText();

 /** Return the text file into a wxString*/
 static wxString FileToString( wxString absFileName );

 /** Put text into file */
 static void StringToFile(wxString inText , wxString outAbsFileName );

 /** Get the directory containing OpenSim API templates */
 static wxString GetOpenSimAPITemplatesDir();

 /** Helper function to open a file with Notepad++*/
 static void OpenFileWithNotepadPP( wxString &absFileName);

 /** Get the C++ text to be pasted */
 wxString GetOpenSimComponentTextToBePasted();

 static bool VMERefSysOrVMESurfaceAccept(mafNode *node);

 static bool VMELandmarkCloudAcccept(mafNode *node);

 /** Perform validation on the vme Body chooser */
 static bool VMESurfaceAcceptForBodyGeneration(mafNode *node);

 static bool VMEAcceptAsBody(mafNode *node);

 /** Perform validation on the vme Body chooser */
 static bool VMEGroupAcceptForBodyGeneration(mafNode *node);

 static bool VMELandmarkAccept(mafNode *node);

 static bool VMEMeterAccept(mafNode *node);

 protected:

	/** Check the operation prerequisites: CMake, Visual Studio 2008 , ... and more as described in the linked doc
	https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit?hl=en_US&pli=1
	*/
	bool CheckPrerequisites();

	/** Edit the openSIM C++ model source in the model editor*/
	void EditOpenSIMFile();

	/** Open VME XML in model editor */
	void OpenVmeXMLInModelEditor();

	/** Build the OpenSIM C++ model source with Visual Studio 2008 Express*/
	void BuildOpenSIMFile();

	/** Return the "pid" of the wxExecute() */
	long GetPid();

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);
	
	/** Not implemented */
	void ViewModelInOpenSim();
	
	//--------------------------------------
	// xerces XML and DOM playground
	//--------------------------------------

	/** Get a vme node by its Id: output vme is set to null if the given id is not found */
	void GetVMENodeById( xercesc::DOMDocument* inXMLDoc, int inVmeId, DOMElement * &outVmeNode );

	/** Get a node children by tag name: output child is set to null if the given tag is
	not found in children nodes */
	void GetChildrenByTagName( DOMNode* inParentNode , string inTagName, DOMNode * &outputChild);
	
	void OnOpenOutputModelDirectory();
	void OnOpenMSFDirectory();

	/** Fill m_PythonExeFullPath ans m_PythonwExeFullPath ivars with the python interpreter */
	void GetPythonInterpreters();

	wxString m_NotepadPlusPlusOSIMStuffLocalDir;
	
	/** Get vtk absolute file name from surface vme */
	wxString GetVTKFileNameAbsPath(mafVMESurface *vme);

	/** Directory where the OpenSim model will be generated */
	wxString GetOutputModelDirectory();

	wxString GetOutputModelFileName() {return "outputModel.osim";};

	wxString m_GeneratedOpenSimModelLocalDir;

	wxString m_OutputModelDirectory;

	long m_Pid;
    int m_Debug;

	mafString m_PythonExeFullPath; //>python  executable
	mafString m_PythonwExeFullPath; //>pythonw  executable

	/** Configure the OSIMComponent with OSIMComponentDictionary (Step 2 of 2) */ 
	void ConfigureOSIMComponentWithOSIMComponentDictionary();

	/** Configure the OSIMComponentDictionary with data from NMSBuilder (Step 1 of 2) */ 
	void ConfigureOSIMComponentDictionaryWithNMSBuilderData();

	/** Python program used to configure files */
	wxString m_SinglePassMultipleReplaceFileName;

	//------------------------------------------------------
	// Configure the OSIMComponentDictionary with data from NMSBuilder (Step 1 of 2)
	//------------------------------------------------------

	/** OpenSim Component dictionary to configure (OSIM_AddBlockToModel_Dictionary.txt.in)  */
	wxString m_DictionaryToConfigureFileName;

	/** Data from NMS (OSIM_AddBlockToModel_DataFromNMSBuilder.txt) to configure (OSIM_AddBlockToModel_Dictionary.txt.in)  */
	wxString m_DataFromNMSBuilderFileName;

	/** OpenSim Component dictionary to configure (OSIM_AddBlockToModel_Dictionary.txt)  */
	wxString m_ConfiguredDictionaryFileName;

	//------------------------------------------------------
	// Configure the OSIMComponent with OSIMComponentDictionary (Step 2 of 2) 
	//------------------------------------------------------

	/** OpenSim Component to configure (OSIM_AddBlockToModel.cpp.in)  */
	wxString m_ComponentToConfigureFileName;

	/** OpenSim configured Component (OSIM_AddBlockToModel.cpp)  */
	wxString m_ConfiguredComponentFileName;

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	virtual void WriteNMSBuilderDataToFile() {};

	/** Create the OpenSim API into Notepad++ editor */
	void GenerateOpenSimComponentAPIAndOpenItInNotepadPP();

	/** Get generated OpenSim component abs file name */
	wxString GetGeneratedComponentAbsFileName();

	mafVME * ChooseRefSysVME();

	void CreateBuildScriptOpenSim221_240_VisualStudio2008( std::ofstream &buildOsimCppBatchFile );
	void CreateBuildScriptOpenSim300_VisualStudio2010( std::ofstream &buildOsimCppBatchFile );

	static wxString ReplaceSpaceWithUnderscore( wxString in );

	mafString m_OpenSimDir;
	mafString m_OpenSimVersion;
};
#endif
