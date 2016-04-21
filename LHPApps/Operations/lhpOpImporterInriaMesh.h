/*=========================================================================
Program:   vph2
Module:    $RCSfile: lhpOpImporterInriaMesh.h,v $
Language:  C++
Date:      $Date: 2009-06-03 13:26:56 $
Version:   $Revision: 1.1.2.1 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2009
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpImporterInriaMesh_H__
#define __lhpOpImporterInriaMesh_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class vtkUnstructuredGrid;

//----------------------------------------------------------------------------
// lhpOpImporterInriaMesh :
//----------------------------------------------------------------------------
/*
Importer born to import Inria type mesh, Tetrahedri.
*/
class lhpOpImporterInriaMesh: public mafOp
{
public:
  lhpOpImporterInriaMesh(const wxString &label = "Mesh Importer");
  ~lhpOpImporterInriaMesh(); 

  mafTypeMacro(lhpOpImporterInriaMesh, mafOp);

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node){return true;};

  /*virtual*/ mafOp* Copy();

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Import mesh data. */
  virtual int ImportMesh();

  /** Set the vtk filename to be imported. 
  This is used when the operation is executed not using user interface. */
  void SetFileName(const char *name) {m_File = name;};

  /** Return File Name*/
  const char *GetFileName(){return m_File.GetCStr();};

private:
  /** Import specific type of mesh*/
  int ImportInriaTypeTetrahedriMesh();

  bool RemoveComment(const char *str, std::ifstream &file);

  mafString     m_File;
  mafString     m_FileDir;
  vtkUnstructuredGrid   *m_UnstructuredGrid;
  
};
#endif
