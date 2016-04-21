
/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpSmoothMuscle.cpp,v $
  Language:  C++
  Date:      $Date: 2011-06-30 12:57:24 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011
  University of West Bohemia
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpSmoothMuscle.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafGUIDialog.h"
#include "mafGUIValidator.h"

#include "vtkMAFSmartPointer.h"
#if 0
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkCleanPolyData.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkFeatureEdges.h"
#include "vtkPoints.h"
#include "vtkGenericCell.h"
#include "vtkMAFRemoveCellsFilter.h"
#include "vtkMEDFillingHole.h"
#endif

#include "lhpUtils.h"
#include "vtkPolyData.h"
#include "vtkSTLWriter.h"
#include "vtkPLYReader.h"
#include "mafDbg.h"

#include <queue>
#include <set>
#include <map>

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpSmoothMuscle);
//----------------------------------------------------------------------------
#if 0
#define DEFAULT_TAUBIN_STEPS								500
#define DEFAULT_TAUBIN_SHAPE_PRESERVATION		0
#define DEFAULT_QUALITY_ANGLE								60
#define DEFAULT_REMOVE_STEPS								1//00
#define DEFAULT_REMOVE_RINGSIZE							0
#endif

#define DEFAULT_POLYMENDER_SMFACTOR					7

//----------------------------------------------------------------------------
lhpOpSmoothMuscle::lhpOpSmoothMuscle(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = true;  

#if 0
	m_TaubinSmoothSteps = DEFAULT_TAUBIN_STEPS;
	m_TaubinSmoothPreserveShape = DEFAULT_TAUBIN_SHAPE_PRESERVATION;
	m_QualityDihedralAngle = DEFAULT_QUALITY_ANGLE;
	m_RemoveMaxSteps = DEFAULT_REMOVE_STEPS;
	m_RemoveRingSize = DEFAULT_REMOVE_RINGSIZE;
#endif

	m_PolyMenderSmoothFactor = DEFAULT_POLYMENDER_SMFACTOR;

	m_OriginalPolydata = NULL;
	m_ResultPolydata = NULL;
}

//----------------------------------------------------------------------------
lhpOpSmoothMuscle::~lhpOpSmoothMuscle( ) 
//----------------------------------------------------------------------------
{
	vtkDEL(m_OriginalPolydata);
	vtkDEL(m_ResultPolydata);
}
//----------------------------------------------------------------------------
mafOp* lhpOpSmoothMuscle::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpSmoothMuscle(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpSmoothMuscle::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(mafVMESurface));
}
//----------------------------------------------------------------------------
void lhpOpSmoothMuscle::OpRun()   
//----------------------------------------------------------------------------
{	
	//just a simple gui to ask for m_PolyMenderSmoothFactor
  mafGUIDialog* dlg = new mafGUIDialog(m_Label, mafCLOSEWINDOW | mafRESIZABLE);  

	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer2->Add( new wxStaticText( dlg, wxID_ANY, wxT("Polymender Smooth Factor:"), wxDefaultPosition, wxSize( 120,-1 ), 0 ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );	
	wxTextCtrl* vmePolymenderCtrl = new wxTextCtrl( dlg, ID_POLYMENDER_SMFACTOR );
	vmePolymenderCtrl->SetToolTip( wxT("Sets the polymender smooth factor (octree depth).") );	
	bSizer2->Add( vmePolymenderCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	//wxButton* bttnOK = new wxButton( dlg, ID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	//bSizer2->Add( bttnOK, 0, wxALL|wxALIGN_BOTTOM, 5 );	

	dlg->Add(bSizer2, 1, wxEXPAND);
	vmePolymenderCtrl->SetValidator(mafGUIValidator(this, ID_POLYMENDER_SMFACTOR, vmePolymenderCtrl, &m_PolyMenderSmoothFactor, 1, 10));
//	bttnOK->SetValidator(mafGUIValidator(this, ID_OK, bttnOK));

	dlg->ShowModal();

	Execute();	//no need to build GUI, so proceed

	mafEventMacro(mafEvent(this,  OP_RUN_OK)); //: OP_RUN_CANCEL));
}

//----------------------------------------------------------------------------
void lhpOpSmoothMuscle::OpDo()
//----------------------------------------------------------------------------
{
	((mafVMESurface *)m_Input)->SetData(m_ResultPolydata,((mafVME *)m_Input)->GetTimeStamp());
	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}

//----------------------------------------------------------------------------
void lhpOpSmoothMuscle::OpUndo()
//----------------------------------------------------------------------------
{		
	((mafVMESurface *)m_Input)->SetData(m_OriginalPolydata,((mafVME *)m_Input)->GetTimeStamp());
	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}



//----------------------------------------------------------------------------
//Runs the transformation - the core
//Creates m_OriginalPolydata and m_OriginalPolydata.
//OpDo / OpUndo sets m_Input, called from OpRun 
/*virtual*/ void lhpOpSmoothMuscle::Execute()
	//----------------------------------------------------------------------------
{
	//copy input		
	vtkNEW(m_OriginalPolydata);
	m_OriginalPolydata->DeepCopy((vtkPolyData*)((mafVME *)m_Input)->GetOutput()->GetVTKData());

	wxString oldDir = ::wxGetWorkingDirectory();

	//change directory to binary
	::wxSetWorkingDirectory( lhpUtils::lhpGetApplicationDirectory() + "\\PolyMender");	

	//export muscle to STL format for polymender
	vtkMAFSmartPointer< vtkSTLWriter > stlw;
	stlw->SetInput(m_OriginalPolydata);
	stlw->SetFileName("temp.stl");
	stlw->SetFileTypeToBinary();
	stlw->Update();

	stlw->SetInput(NULL);

	//get the smooth

	//call polymender 
	::wxExecute(wxString::Format("PolyMender-clean.exe temp.stl %d 1.0 temp.ply 1.0", m_PolyMenderSmoothFactor), wxEXEC_SYNC);
	
	//read the data
	vtkNEW(m_ResultPolydata);
	vtkMAFSmartPointer< vtkPLYReader > plyr;
	plyr->SetOutput(m_ResultPolydata);
	plyr->SetFileName("temp.ply");	
	plyr->Update();

	plyr->SetOutput(NULL);
#if 0
	//surface reconstruction - it is not used as it makes it even worse
	//vtkMAFSmartPointer<vtkMEDFixTopology> fixtopology;
	//fixtopology->SetInput(vtkPolyData::SafeDownCast(
	//	mafVMESurface::SafeDownCast(m_Input)->GetOutput()->GetVTKData()));

	//extract the largest connected part 	
	vtkMAFSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter;
	connectivityFilter->SetInput(vtkPolyData::SafeDownCast(
		mafVMESurface::SafeDownCast(m_Input)->GetOutput()->GetVTKData()));
		//fixtopology->GetOutput());
	connectivityFilter->SetExtractionModeToLargestRegion();
		
	vtkMAFSmartPointer<vtkCleanPolyData> clean;
	clean->SetInput(connectivityFilter->GetOutput());	
		
	//taubin smoothing
	vtkMAFSmartPointer<vtkWindowedSincPolyDataFilter> smoothFilter;
	smoothFilter->SetInput(clean->GetOutput());
	smoothFilter->SetFeatureAngle(m_QualityDihedralAngle);
	smoothFilter->SetBoundarySmoothing(1);
	smoothFilter->SetNonManifoldSmoothing(1);
	smoothFilter->SetFeatureEdgeSmoothing(1);
	smoothFilter->SetNumberOfIterations(m_TaubinSmoothSteps);
	smoothFilter->SetPassBand(0.1);

	smoothFilter->SetOutput(m_ResultPolydata);
	smoothFilter->Update();
	
	smoothFilter->SetOutput(NULL);	//detach output
	RemoveSpikes();
#endif
}

#if 0
//----------------------------------------------------------------------------
//Removes spikes from m_ResultPolydata.
//N.B. called from Execute
/*virtual*/ void lhpOpSmoothMuscle::RemoveSpikes()
	//----------------------------------------------------------------------------
{
	vtkMAFSmartPointer< vtkGenericCell> cell;
	vtkMAFSmartPointer<vtkFeatureEdges> featurFilter;
	featurFilter->FeatureEdgesOn();
	featurFilter->NonManifoldEdgesOn();
	featurFilter->BoundaryEdgesOff();
	featurFilter->ManifoldEdgesOff();
	featurFilter->SetFeatureAngle(m_QualityDihedralAngle);	

	//delete wrong parts
	for (int steps = 0; steps < m_RemoveMaxSteps; steps++)
	{
		m_ResultPolydata->BuildCells();		//make sure we have topology known
		m_ResultPolydata->BuildLinks();	
				      
    featurFilter->SetInput(m_ResultPolydata);    
    featurFilter->Update();

		featurFilter->SetInput(NULL);	//detach its input

		//get points of selected points
		vtkPoints* ptsIn = featurFilter->GetOutput()->GetPoints();
		int nPoints = ptsIn->GetNumberOfPoints();
		if (nPoints == 0)
			break;	//we have already a good mesh here			

		//process all points (one by one)
		int index = 0;
		for (int i = 0; i < nPoints; i++)
		{
			m_ResultPolydata->BuildCells();		//make sure we have topology known
			m_ResultPolydata->BuildLinks();	

			vtkMAFSmartPointer<vtkMAFRemoveCellsFilter> removeCells;
			removeCells->SetInput(m_ResultPolydata);
			removeCells->Update();	//this is here because of BUG in vtkMAFRemoveCellsFilter preventing us to call MarkCell without initialization

			std::queue< vtkIdType > queue;					//queue of visited points
			std::set< vtkIdType > map_cells;				//keep track where we already were

			//get the point and find it in the source mesh
			double* coords = ptsIn->GetPoint(index);
			vtkIdType ptId = m_ResultPolydata->FindPoint(coords);

			_VERIFY_CMD(ptId >= 0, continue);

			//process the ring			
			std::map< vtkIdType, int > map_pts;			//keep track of visited points
			map_pts[ptId] = 0;
			queue.push(ptId);

			while (!queue.empty())
			{
				ptId = queue.front();
				int ring = map_pts[ptId];
				if (ring <= m_RemoveRingSize)
				{
					ring++;	//increase ring size

					//the point is in a remove ring size
					unsigned short ncells;
					vtkIdType* cells;
					m_ResultPolydata->GetPointCells(ptId, ncells, cells);					

					//for all cells around ptId
					for (int j = 0; j < ncells; j++)
					{
						//if this cell has not been already processed
						if (map_cells.find(cells[j]) == map_cells.end())
						{
							//mark the cell to be removed
							removeCells->MarkCell(cells[j]);
							map_cells.insert(cells[j]);

							//get the cell and its vertices
							m_ResultPolydata->GetCell(cells[j], cell);
							vtkIdList* idlist = cell->GetPointIds();
							int nverts = idlist->GetNumberOfIds();
							vtkIdType* ids = idlist->GetPointer(0);

							for (int k = 0; k < nverts; k++)
							{
								//if not alerady processed
								if (map_pts.find(ids[k]) == map_pts.end())
								{
									queue.push(ids[k]);
									map_pts[ids[k]] = ring;
								}
							}
						}
					}		
				} //end if in the ring

				queue.pop();
			} //end while

			//remove cells around the vertex
			removeCells->RemoveMarkedCells();
			removeCells->Update();			

			vtkMAFSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter;
			connectivityFilter->SetInput(removeCells->GetOutput());
			connectivityFilter->SetExtractionModeToLargestRegion();

			removeCells->SetInput(NULL);	//detach m_ResultPolydata
			removeCells->SetOutput(NULL); //detach output

			vtkMAFSmartPointer<vtkCleanPolyData> clean;
			clean->SetInput(connectivityFilter->GetOutput());	
			
			//fill the hole, it created
			vtkMAFSmartPointer<vtkMEDFillingHole> holefillFilter;
			holefillFilter->SetInput(clean->GetOutput());			
			holefillFilter->SetSmoothFill(true);
			holefillFilter->SetFillAllHole();
			holefillFilter->SetOutput(m_ResultPolydata);
			holefillFilter->Update();	
			
			holefillFilter->SetOutput(NULL);	//detach m_ResultPolydata

			//recalculate the mesh quality			
			featurFilter->SetInput(m_ResultPolydata);    
			featurFilter->Update();

			featurFilter->SetInput(NULL);	//detach its input

			ptsIn = featurFilter->GetOutput()->GetPoints();
			int nNewPoints = ptsIn->GetNumberOfPoints();

			if (nNewPoints >= nPoints)
				index++;	//we have still the same number of "spike" vertices, i.e., it had no effect => try another point
			else if (index >= nNewPoints)
				index = 0;	//effect reached

			nPoints = nNewPoints;
		} //end for each point in the feature edges						
	}

		
	//remove isolated vertices 
	/*vtkMAFSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter;
	connectivityFilter->SetInput(removeCells->GetOutput());
	connectivityFilter->SetExtractionModeToLargestRegion();
	connectivityFilter->Update();	*/
		
	/*
	vtkMAFSmartPointer<vtkCleanPolyData> clean;
	clean->SetInput(m_ResultPolydata);	
	clean->Update();
	clean->SetInput(NULL);

	m_ResultPolydata->DeepCopy(clean->GetOutput());
	*/
}
#endif
#endif