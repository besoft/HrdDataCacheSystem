/*=========================================================================
Program:   lhp
Module:    $RCSfile: lhpOpMeshGeneratorExecutable.cpp,v $
Language:  C++
Date:      $Date: 2010-03-30 14:55:16 $
Version:   $Revision: 1.1.2.4 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/


#include "lhpDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpMeshGeneratorExecutable.h"
#include "lhpUtils.h"

#include "wx/busyinfo.h"

#include "mafGUI.h"
#include "mafGUIRollout.h"
#include "mafNode.h"
#include "mafVME.h"
#include "mafTagArray.h"
#include "mafVMEMesh.h"
#include "mafVMESurface.h"
#include "mafTagItem.h"

#include "vtkMAFSmartPointer.h"

#include "vtkDataSet.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkCellArray.h"

#include "vtkTriangleFilter.h"
#include "vtkDataSetReader.h"
#include "vtkDataSetWriter.h"
#include "vtkUnstructuredGrid.h"

#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"

#include "vtkSTLWriter.h"
#include "vtkTriangleFilter.h"
#include "lhpOpImporterInriaMesh.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMeshGeneratorExecutable);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMeshGeneratorExecutable::lhpOpMeshGeneratorExecutable(wxString label) : mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = true;
  m_InputPreserving = true;

  m_AdvancedGUIRollOut = NULL;

  this->m_PLC = FALSE;                // -p switch, 0
  this->m_Refine = FALSE;             // -r switch, 0
  this->m_Coarsen = FALSE;             // -R switch, 0
  this->m_Quality = FALSE;            // -q switch, 0
  this->m_NoBoundarySplit = FALSE;    // -Y switch, 0
  this->m_MinRatio = 2.0;         //    number after -q, 2.0
  this->m_VarVolume = FALSE;          // -a switch without number, 0
  this->m_FixedVolume = FALSE;        // -a switch with number, 0
  this->m_MaxVolume = -1.0;       //    number after -a, -1.0
  this->m_RemoveSliver = FALSE;       // -s switch, 0
  this->m_MaxDihedral = 0.0;      //    number after -s, 0.0
  this->m_RegionAttrib = FALSE;       // -A switch, 0
  this->m_Epsilon = 1.0e-8;       // number after -T switch, 1.0e-8
  this->m_NoMerge = FALSE;            // -M switch, 0
  this->m_DetectInter = FALSE;        // -d switch, 0
  this->m_Order = TRUE;              // number after -o switch, 1 (e.g. -o2 for quadratic elements)
  this->m_DoCheck = FALSE;            // -C switch, 0
  this->m_UseSizingFunction = FALSE;            // -C switch, 0

  this->m_Verbose = FALSE;

  this->m_CellEntityIdsArrayName = "";
  this->m_TetrahedronVolumeArrayName = "";
  this->m_SizingFunctionArrayName = "";

  this->m_OutputSurfaceElements = FALSE; // was TRUE
  this->m_OutputVolumeElements = TRUE;

  m_TetGenCommandLineParametersString = "";
}
//----------------------------------------------------------------------------
lhpOpMeshGeneratorExecutable::~lhpOpMeshGeneratorExecutable( ) 
//----------------------------------------------------------------------------
{
  
 
}
//----------------------------------------------------------------------------
bool lhpOpMeshGeneratorExecutable::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node->IsA("mafVMESurface"));
}
//----------------------------------------------------------------------------
mafOp* lhpOpMeshGeneratorExecutable::Copy()   
//----------------------------------------------------------------------------
{
  return new lhpOpMeshGeneratorExecutable(m_Label);
}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui(); 
}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::CreateGui()   
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);

  mafGUI *advancedGUI = new mafGUI(this);
  advancedGUI->Bool(ID_PLC,_("PLC"),&m_PLC, TRUE);
  advancedGUI->Bool(ID_REFINE,_("Refine"),&m_Refine, TRUE);
  advancedGUI->Bool(ID_COARSEN,_("Coarsen"),&m_Coarsen, TRUE);
  advancedGUI->Bool(ID_NO_BOUNDARY_SPLIT,_("No Boundary Split"),&m_NoBoundarySplit, TRUE);
  advancedGUI->Bool(ID_QUALITY,_("Quality"),&m_Quality, TRUE);
  
  advancedGUI->Divider(1);
  advancedGUI->Label("Min Ratio:", true);
  advancedGUI->Double(ID_MIN_RATIO,_(""),&m_MinRatio,0);
  advancedGUI->Divider(1);

  advancedGUI->Bool(ID_VAR_VOLUME,_("Var. Volume"),&m_VarVolume, TRUE);
  advancedGUI->Bool(ID_FIXED_VOLUME,_("Fixed Volume"),&m_FixedVolume, TRUE);

  advancedGUI->Divider(1);
  advancedGUI->Label("Max Volume:", true);
  advancedGUI->Double(ID_MAX_VOLUME,_(""),&m_MaxVolume,0);
  advancedGUI->Divider(1);

  advancedGUI->Bool(ID_REMOVE_SLIVER,_("Remove Sliver"),&m_RemoveSliver, TRUE);

  advancedGUI->Divider(1);
  advancedGUI->Label("Max Dihedral:", true);
  advancedGUI->Double(ID_MAX_DIHEDRAL,_(""),&m_MaxDihedral,0);
  advancedGUI->Divider(1);

  advancedGUI->Bool(ID_REGION_ATTRIB,_("Region. Attrib."),&m_RegionAttrib, TRUE);

  advancedGUI->Divider(1);
  advancedGUI->Label("Epsilon:", true);
  advancedGUI->Double(ID_EPSILON,_(""),&m_Epsilon,0);
  advancedGUI->Divider(1);

  advancedGUI->Bool(ID_NO_MERGE,_("No Merge"),&m_NoMerge, TRUE);

  advancedGUI->Bool(ID_DETECT_INTER,_("Detect.Inter."),&m_DetectInter, TRUE);
  advancedGUI->Integer(ID_ORDER,_("Order"),&m_Order);
  advancedGUI->Bool(ID_DO_CHECK,_("Do Check"),&m_DoCheck, TRUE);
  advancedGUI->Bool(ID_VERBOSE,_("Verbose"),&m_Verbose, TRUE);
  //advancedGUI->Bool(ID_USE_SIZING_FUNCTION,_("Use Sizing Function"),&m_UseSizingFunction, TRUE);
  
  /*advancedGUI->Divider(1);
  advancedGUI->Label("Cell Entity IDs:", true);
  advancedGUI->String(ID_CELL_ENTITY_IDS_ARRAY_NAME,_(""), &m_CellEntityIdsArrayName);
  advancedGUI->Label("Tetrahedron Volume:", true);
  advancedGUI->String(ID_TETRAHEDRON_VOLUME_ARRAY_NAME,_(""), &m_TetrahedronVolumeArrayName);
  advancedGUI->Label("Sizing Function:", true);
  advancedGUI->String(ID_SIZING_FUNCTION_ARRAY_NAME,_(""), &m_SizingFunctionArrayName);
  advancedGUI->Divider(1);*/

  advancedGUI->Bool(ID_OUTPUT_SURFACE_ELEMENTS,_("Surface Elements"),&m_OutputSurfaceElements, TRUE);
  advancedGUI->Bool(ID_OUTPUT_VOLUME_ELEMENTS,_("Volume Elements"),&m_OutputVolumeElements, TRUE);


  advancedGUI->Divider(1);

  m_AdvancedGUIRollOut = m_Gui->RollOut(ID_ADVANCED_GUI_ROLLOUT,_("Mesh Generator"),advancedGUI, true);
  
  m_Gui->OkCancel();
  ShowGui();
}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::OpStop(int result)   
//----------------------------------------------------------------------------
{
	HideGui();
  mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::Execute()   
//----------------------------------------------------------------------------
{
  wxString newDir = (lhpUtils::lhpGetApplicationDirectory() + "/tetgen/").c_str();
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(newDir);
  mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  mafVMEOutputSurface *out_surface = mafVMEOutputSurface::SafeDownCast(((mafVME *)m_Input)->GetOutput());
  out_surface->Update();

  vtkMAFSmartPointer<vtkTriangleFilter>triangles;
  triangles->SetInput(out_surface->GetSurfaceData());
  triangles->Update();

  //export STL
  vtkMAFSmartPointer<vtkSTLWriter> writer;
  mafEventMacro(mafEvent(this,BIND_TO_PROGRESSBAR,writer));
  mafString tetGenInputSTLFile = newDir;
  tetGenInputSTLFile << "TempSTL.stl";
  writer->SetFileName(tetGenInputSTLFile.GetCStr());
  writer->SetInput(triangles->GetOutput());
  writer->SetFileTypeToASCII();
  writer->Update();

  //launch tetgen with correct parameters saving in Medit .mesh
  wxString command = "tetgen142.exe ";
  Fill_m_TetGenCommandLineParametersString_FromIVars();
  command.Append(m_TetGenCommandLineParametersString.GetCStr());
  command.Append(" \"");
  command.Append(tetGenInputSTLFile.GetCStr());
  command.Append("\" ");

  mafLogMessage(command.c_str());
  wxExecute(command,wxEXEC_SYNC);

  //reload mesh with import medit
  lhpOpImporterInriaMesh *op = new lhpOpImporterInriaMesh("");
  mafString MeditInputFile = newDir;
  MeditInputFile << "TempSTL.1.mesh";
  op->SetFileName(MeditInputFile.GetCStr());
  op->SetInput(m_Input);
  op->ImportMesh();

  wxSetWorkingDirectory(oldDir);

  delete op;

}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::Fill_m_TetGenCommandLineParametersString_FromIVars()
//----------------------------------------------------------------------------
{
  char buffer[64];
  m_TetGenCommandLineParametersString = "";
  //m_TetGenCommandLineParametersString << "-gBNEF";
  m_TetGenCommandLineParametersString << "-g";

  if (this->m_PLC)
  {
    m_TetGenCommandLineParametersString << " -p";  
  }

  if (this->m_Refine)
  {
    m_TetGenCommandLineParametersString << " -r";  
  }

  if (this->m_Coarsen)
  {
    m_TetGenCommandLineParametersString << " -R";
  }

  if (this->m_Quality)
  {
    m_TetGenCommandLineParametersString << " -q";  
    sprintf(buffer,"%f",this->m_MinRatio);
    m_TetGenCommandLineParametersString << buffer;
  }

  if (this->m_NoBoundarySplit)
  {
    m_TetGenCommandLineParametersString << " -Y";  
  }

  if (this->m_RemoveSliver)
  {
    m_TetGenCommandLineParametersString << " -s";  
    sprintf(buffer,"%f",this->m_MaxDihedral);
    m_TetGenCommandLineParametersString << buffer;
  }

  if (this->m_Order == 2)
  {
    m_TetGenCommandLineParametersString << " -o2";  
  }

  m_TetGenCommandLineParametersString << " -T";
  sprintf(buffer,"%e",this->m_Epsilon);
  m_TetGenCommandLineParametersString << buffer;

  if (this->m_VarVolume)
  {
    m_TetGenCommandLineParametersString << " -a";
  }

  if (this->m_FixedVolume)
  {
    m_TetGenCommandLineParametersString << " -a";
    sprintf(buffer,"%f",this->m_MaxVolume);
    m_TetGenCommandLineParametersString << buffer;
  }

  if (this->m_RegionAttrib)
  {
    m_TetGenCommandLineParametersString << " -A";
  }

  if (this->m_NoMerge)
  {
    m_TetGenCommandLineParametersString << " -M";
  }

  if (this->m_DetectInter)
  {
    m_TetGenCommandLineParametersString << " -d";
  }

  if (this->m_DoCheck)
  {
    m_TetGenCommandLineParametersString << " -C";
  }

  m_TetGenCommandLineParametersString << " -z";  
  // m_TetGenCommandLineParametersString << " -Y";  

  if (this->m_Verbose)
  {
    m_TetGenCommandLineParametersString << " -V";  
  }
  else
  {
    m_TetGenCommandLineParametersString << " -Q";  
  }

}
//----------------------------------------------------------------------------
void lhpOpMeshGeneratorExecutable::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{
      /*case ID_PLC:
        break;
      case ID_REFINE:
        break;
      case ID_COARSEN:
        break;
      case ID_NO_BOUNDARY_SPLIT:
        break;
      case ID_QUALITY:
        break;
      case ID_MIN_RATIO:
        break;
      case ID_VAR_VOLUME:
        break;
      case ID_FIXED_VOLUME:
        break;
      case ID_MAX_VOLUME:
        break;
      case ID_REMOVE_SLIVER:
        break;
      case ID_MAX_DIHEDRAL:
        break;
      case ID_REGION_ATTRIB:
        break;
      case ID_EPSILON:
        break;
      case ID_NO_MERGE:
        break;
      case ID_DETECT_INTER:
        break;
      case ID_ORDER:
        break;
      case ID_DO_CHECK:
        break;
      case ID_VERBOSE:
        break;
      case ID_USE_SIZING_FUNCTION:
        break;
      case ID_CELL_ENTITY_IDS_ARRAY_NAME:
        break;
      case ID_TETRAHEDRON_VOLUME_ARRAY_NAME:
        break;
      case ID_SIZING_FUNCTION_ARRAY_NAME:
        break;
      case ID_OUTPUT_SURFACE_ELEMENTS:
        break;
      case ID_OUTPUT_VOLUME_ELEMENTS:
        break;*/
		case wxOK:
			{
				Execute();
				OpStop(OP_RUN_OK);
			}
			break;
		case wxCANCEL:
			{
				OpStop(OP_RUN_CANCEL);
			}
			break;
		default:
			mafEventMacro(*e);
			break;
		}
	}
}