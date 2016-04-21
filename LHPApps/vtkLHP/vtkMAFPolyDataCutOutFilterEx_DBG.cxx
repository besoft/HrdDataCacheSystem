/*========================================================================= 
Program: Multimod Application Framework RELOADED 
Module: $RCSfile: vtkMAFPolyDataCutOutFilterEx_DBG.cxx,v $ 
Language: C++ 
Date: $Date: 2012-03-19 12:45:22 $ 
Version: $Revision: 1.1.2.1 $ 
Authors: Josef Kohout
========================================================================== 
Copyright (c) 2008 University of Bedfordshire (www.beds.ac.uk)
Copyright (c) 2011University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details 
=========================================================================
*/

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------

#include "vtkMAFPolyDataCutOutFilterEx.h"
#include "vtkMAFVisualDebugger.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkMath.h"
#include "vtkMAFSmartPointer.h"
#include <float.h>
#include <queue>
#include <stack>
#include <map>

#include "mafMemDbg.h"
#include "mafDbg.h"

//Visualize the mesh with highlighted points to be inserted (pInsertedPts)
//and already connected points (pCountourPts). Called only, if Debug != 0 
void vtkMAFPolyDataCutOutFilterEx::Debug_Visualize_Progress(vtkMAFVisualDebugger* vd,
	const vtkIdList* pInsertedPts, const vtkIdList* pContourPts, int ptIdFrom, int ptIdTo)
{
	//mesh
	vtkMAFSmartPointer< vtkPolyData > mesh;
	DoneMesh(mesh, false);

	vd->AddOrUpdateSurface("Surface", mesh, 0.9599999785423279, 0.2899999916553497, 0.2899999916553497);	//solid
	vd->AddOrUpdateSurface("SurfaceWF", mesh, 0.5, 0.5, 0.5, 1.0, true);	//wireframe	
	
	//mesh points to be connected
	vd->AddOrUpdateSelectedPoints("KeyPoints", mesh,	
		const_cast<vtkIdList*>(pInsertedPts)->GetPointer(0), 
		const_cast<vtkIdList*>(pInsertedPts)->GetNumberOfIds(), 
		0.25, 1.0, 0.0, 0.0); //red
	
	//current points	
	vtkIdType ptIds[2] = {ptIdFrom, ptIdTo};
	vd->AddOrUpdateSelectedPoints("KeyPointsCur", mesh,	ptIds, 2,
		0.3, 0.0, 1.0, 0.0);	//green
	
		
	//contour
	vtkMAFSmartPointer< vtkPolyData > contour;
	DoneCuttingPolyline(contour, const_cast<vtkIdList*>(pContourPts));
#ifdef DIJKSTRA_CUT
	//change the last item
	vtkIdType idPt = contour->GetPoints()->InsertNextPoint(m_Vertices[ptIdFrom].dCoord);
	int nEdges = contour->GetLines()->GetNumberOfCells();
	if (nEdges > 0) {
		contour->GetLines()->GetPointer()[2] = idPt;
	}
#endif
	vd->AddOrUpdateLines("Contour", contour, 0.125, 0.0, 1.0, 0.0);	//green

	vd->DebugStep();
}

//Debug visualization of InsertCutPoints.
//Visualize the mesh with highlighted points already processed (pProcessed, nProcessed)
//and input poly-line contour sorted as given in pInputContourPts. iCurId is the point of
//the input contour to be inserted in the current iteration.	Called only, if Debug != 0
void vtkMAFPolyDataCutOutFilterEx::Debug_Visualize_Progress2(vtkMAFVisualDebugger* vd,
	const vtkIdList* pInputContourPts, int iCurId, 
	const vtkIdType* pProcessed, int nProcessed)
{
	//mesh
	vtkMAFSmartPointer< vtkPolyData > mesh;
	DoneMesh(mesh, false);

	vd->AddOrUpdateSurface("Surface", mesh, 0.9599999785423279, 0.2899999916553497, 0.2899999916553497);	//solid
	vd->AddOrUpdateSurface("SurfaceWF", mesh, 0.5, 0.5, 0.5, 1.0, true);	//wireframe

	//points on the mesh, already processed
	vd->AddOrUpdateSelectedPoints("KeyPoints", mesh, pProcessed, nProcessed,		
		0.25, 0.0, 1.0, 0.0); //green
	
	//input poly-line but with different cells
	vtkPolyData* incontour = vtkPolyData::New();
	incontour->ShallowCopy(this->CuttingPolyline);

	vtkCellArray* cells = vtkCellArray::New();
	cells->InsertNextCell(const_cast<vtkIdList*>(pInputContourPts));
	incontour->SetLines(cells);
	cells->Delete();

	vd->AddOrUpdateLines("Contour", incontour, 0.125, 1.0, 0.0, 0.0); //red
	vd->AddOrUpdatePoints("ContourPoints", incontour, 0.25, 1.0, 0.0, 0.0); //red
		
	incontour->Delete();

	//current point	
	if (iCurId >= 0) {
		vd->AddOrUpdatePoints("CurPoint", this->CuttingPolyline->GetPoint(
			const_cast<vtkIdList*>(pInputContourPts)->GetId(iCurId)), 1, 0.3,
			1.0, 1.0, 0.0); //yellow		
	}		

	vd->DebugStep();
}

// Debug visualization of correcting the contour.
//Visualize the mesh with highlighted intersecting contour pInputContourPts 
//and the  current non-intersecting contour  pProcessed. iCurId is the point of pInputContourPts
//where the contour is intersecting (or touching) itself.	Called only, if Debug != 0 
void vtkMAFPolyDataCutOutFilterEx::Debug_Visualize_Progress3(vtkMAFVisualDebugger* vd,
	const vtkIdList* pInputContourPts, int iCurId, const vtkIdList* pProcessed)
{
	//mesh
	vtkMAFSmartPointer< vtkPolyData > mesh;
	DoneMesh(mesh, false);

	vd->AddOrUpdateSurface("Surface", mesh, 0.9599999785423279, 0.2899999916553497, 0.2899999916553497);	//solid
	vd->AddOrUpdateSurface("SurfaceWF", mesh, 0.5, 0.5, 0.5, 1.0, true);	//wireframe
		
	//current points	
	vtkIdType ptIds = const_cast<vtkIdList*>(pInputContourPts)->GetId(iCurId);
	vd->AddOrUpdateSelectedPoints("CurPoint", mesh, &ptIds, 1, 0.2, 1.0, 0.0, 0.0); //red	
		
	//contour1
	vtkMAFSmartPointer< vtkPolyData > contour1;
	DoneCuttingPolyline(contour1, const_cast<vtkIdList*>(pInputContourPts));
	vd->AddOrUpdateLines("Contour1", contour1, 0.120, 1.0, 0.0, 0.0); //red

	//contour2
	vtkMAFSmartPointer< vtkPolyData > contour2;
	DoneCuttingPolyline(contour1, const_cast<vtkIdList*>(pProcessed));
	vd->AddOrUpdateLines("Contour2", contour1, 0.125, 0.0, 1.0, 0.0); //green

	vd->DebugStep();
}