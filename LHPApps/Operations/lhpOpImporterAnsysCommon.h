/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpImporterAnsysCommon.h,v $
Language:  C++
Date:      $Date: 2010-11-23 16:50:26 $
Version:   $Revision: 1.1.1.1.2.3 $
Authors:   Daniele Giunchi , Stefano Perticoni, Gianluigi Crimi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpImporterAnsysCommon_H__
#define __lhpOpImporterAnsysCommon_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;
class wxBusyInfo;

//----------------------------------------------------------------------------
// lhpOpImporterAnsysFile :
//----------------------------------------------------------------------------
/** 
Importer for Ansys Input files
*/
class LHP_OPERATIONS_EXPORT lhpOpImporterAnsysCommon : public mafOp
{
public:

	lhpOpImporterAnsysCommon(const wxString &label = "MeshImporter");
	~lhpOpImporterAnsysCommon(); 

	virtual void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Set the input Ansys file */
	void SetFileName(const char *ansysInputFileNameFullPath) {m_AnsysInputFileNameFullPath = ansysInputFileNameFullPath;};
	
  /** Get the input Ansys file */
  wxString GetFileName() {return m_AnsysInputFileNameFullPath.c_str();};

  /** Builds operation's interface. */
  void OpRun();

	/** Import the mesh*/
	int Import();

protected:
 
  virtual mafString GetWildcard() = 0;

  void InitProgressBar(wxString label);
  void CloseProgressBar();
  void UpdateProgressBar(long progress);

	virtual int ParseAnsysFile(mafString fileName) = 0;

  int ReadNBLOCK(FILE *outFile);
  int ReadEBLOCK(FILE *outFile);
  int ReadMPDATA(FILE *outFile);

	int GetLine(FILE *fp, char *buffer);
	int ReplaceInString(char *str, char from, char to);
  int ReadInit(mafString &fileName);
  void ReadFinalize();

  wxBusyInfo *m_BusyInfo;

	wxString m_FileDir;
	wxString m_AnsysInputFileNameFullPath;

	int m_ImporterType;
	mafVMEMesh *m_ImportedVmeMesh;

	mafString m_CacheDir;
	mafString m_DataDir;

  long m_OperationProgress;
	/** Nodes file name*/
	wxString m_NodesFileName;

	/** Elements file name*/
	wxString m_ElementsFileName;

	/** Materials file name*/
	wxString m_MaterialsFileName;
  
  FILE * m_FilePointer;
	char *m_Buffer;
	int m_BufferLeft;
  int m_BufferPointer;
  char m_Line[512];
  long m_FileSize;
	long m_BytesReaded;
  
	int m_CurrentMatId;  
};
#endif
