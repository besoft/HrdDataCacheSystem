/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpOpImporterMeshFeBio.h,v $
Language:  C++
Date:      $Date: 2010-12-15 15:28:11 $
Version:   $Revision: 1.1.2.4 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpImporterMeshFeBio_H__
#define __lhpOpImporterMeshFeBio_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

class mafVMEMesh;

class lhpMeshFeBioReader;

class vtkCellArray;
class vtkPoints;
class vtkUnstructuredGrid;

//----------------------------------------------------------------------------
// lhpOpImporterMeshFeBio :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpImporterMeshFeBio : public mafOp
{
public:
  /** Class constructor */
  lhpOpImporterMeshFeBio(const wxString &label = "FEBio Importer");
  /** Class destructor */
  ~lhpOpImporterMeshFeBio(); 
  /**  Operation copy */
  mafOp* Copy();


  /** Return true for the acceptable vme type. */
  bool Accept(mafNode* node) {return true;};

  /** Builds operation's interface. */
  void OpRun();

  /** Read the file.
  File format:

  XML file:
  <Geometry>
     <Nodes></Nodes> 
     <Elements></Elements</>
  </Geometry>

  Nodes: points
  Elements: connectivity


  */
  /** Read the XML file*/
  void Read();

  /** Set the filename for the file to import */
  void SetFileName(const char *file_name){m_File = file_name;};

protected:

  /** Read nodes coordinates info*/
  void ReadNodes();
  /** Read geometry connectivity*/ 
  void ReadConnectivity();
  
  wxString m_FileDir;
  wxString m_File;

  mafVMEMesh *m_Mesh;
  vtkUnstructuredGrid *m_MeshData;
  vtkCellArray *m_Cells;
  vtkPoints *m_Points;

  lhpMeshFeBioReader *m_Reader;

};
#endif
