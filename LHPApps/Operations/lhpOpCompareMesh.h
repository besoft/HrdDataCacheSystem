/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpCompareBonemat.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpCompareMesh_H__
#define __lhpOpCompareMesh_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class medVMEMuscleWrapper;
class mafVMEMesh;
class vtkDataArray;
class vtkFieldData;

//----------------------------------------------------------------------------
// lhpOpCompareBonemat :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpCompareMesh: public mafOp
{
public:
	lhpOpCompareMesh(const wxString &label = "Compare Bonemat");
	~lhpOpCompareMesh(); 

	mafTypeMacro(lhpOpCompareMesh, mafOp);

	mafOp* Copy();

	bool Accept(mafNode *node);
	void OpRun();


	mafVMEMesh* SelectMesh();

  void OnEvent(mafEventBase *maf_event);

	void OpStop(int result);

	void Execute();

	static bool AcceptMesh(mafNode *node);

private:

	void CompareArray(vtkDataArray *arrayA, vtkDataArray *arrayB, vtkDataArray *result);
	void CompareFieldData(vtkFieldData *cellDataA, vtkFieldData *cellDataB, vtkFieldData *cellDataC);
	int IsArrayToCopy(mafString arrayName);

	int IsNameInArrayList(int nArray, mafString arrayName, char ** arrayNamesToCopy);

	void CreateMeshCopy();
  void CreateGui();
	int IsArrayToExclude(mafString arrayName);


  int m_CompareMode;

	mafVMEMesh *m_MeshA;
	mafVMEMesh *m_MeshB;
	mafVMEMesh *m_OutputMesh;
};
#endif //__lhpOpCompareBonemat_H__
