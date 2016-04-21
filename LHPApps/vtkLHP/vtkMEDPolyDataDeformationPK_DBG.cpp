/*=========================================================================
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkMEDPolyDataDeformationPK_DBG.cpp,v $
  Language: C++
  Date: $Date: 2012-04-18 05:17:07 $
  Version: $Revision: 1.1.2.5 $
  Authors: Josef Kohout
  ==========================================================================
  Copyright (c) 2012 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details
  =========================================================================
*/

#include "vtkMEDPolyDataDeformationPK.h"
#include "mafDbg.h"

#include "vtkPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"

#pragma warning(disable: 4996)
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkTubeFilter.h"
#include "vtkMAFSmartPointer.h"
#include "vtkCellArray.h"
#include "vtkTextMapper.h"
#include "vtkActor2D.h"
#include "vtkProperty2D.h"
#include "vtkCamera.h"
#include "vtkCallbackCommand.h"
#include "vtkRendererCollection.h"
#include "vtkCellPicker.h"
#include "vtkSphereSource.h"

#include "vtkDataSetMapper.h"
#include "vtkLineSource.h"
#include "vtkTriangle.h"
#include "vtkPolyData.h"

vtkSmartPointer< vtkCamera> g_lastCamera;

int keyPressed = 0;

//////
/// Keypress callback function for vtkMEDPolyDataDeformationPK_DBG window
//////
/*static*/ void vtkMEDPolyDataDeformationPK::KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) )  
{  
	keyPressed = ((vtkRenderWindowInteractor*)caller)->GetKeyCode();

	if (((vtkRenderWindowInteractor*)caller)->GetKeyCode() == char(27)) {  // if ESC, close the window and continue in visualization
			((vtkRenderWindowInteractor*)caller)->ExitCallback();
	}
	
	if (((vtkRenderWindowInteractor*)caller)->GetKeyCode() == 'x') { //  if key X is pressed, visualization is stopped from the next step
		((vtkMEDPolyDataDeformationPK*)clientData)->debugMode = false;
		((vtkRenderWindowInteractor*)caller)->ExitCallback();
	}
}

//////
/// Mouse callback function for vtkMEDPolyDataDeformationPK_DBG window
//////
/*static*/ void vtkMEDPolyDataDeformationPK::MouseCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData))  
{  
	int x,y; // coordinates of mouse clicking

	vtkRenderWindow* win = ((vtkRenderWindowInteractor*)caller)->GetRenderWindow();  
	vtkRendererCollection* col = win->GetRenderers();  
	int nRenderers = col->GetNumberOfItems();  
	if (keyPressed == (int)'p' || keyPressed == (int)'c') { // reaction on keys P and C
		for (int i = 0; i < nRenderers; i++)  
		{  
			vtkRenderer* renderer = (vtkRenderer*)col->GetItemAsObject(i);

			if (renderer != NULL)
			{
				vtkMAFSmartPointer<vtkCellPicker> cellPicker;      // picker to be used for the picking
				cellPicker->SetTolerance(0.001);

				int *pick = ((vtkRenderWindowInteractor*)caller)->GetEventPosition();  
				cellPicker->Pick((double)pick[0], (double)pick[1], 0.0, renderer); // get coordinates of the clicking place
				x = pick[0];
				y = pick[1];

				if (cellPicker->Pick(x,y,0,renderer) != 0)
				{
					double pickedPos[3];    //place where the pick was done
					vtkDataSet* pickedObj;  //picked VTK object

					cellPicker->GetPickedPositions()->GetPoint(0, pickedPos);
					pickedObj = cellPicker->GetDataSet();

					if (pickedObj == NULL)
						return;

					if (keyPressed == (int)'p') {
						// get id of the point
						int id = pickedObj->FindPoint(pickedPos); // get object on which was clicked
						if (id < 0)
							return;

						const double* pcoord = pickedObj->GetPoint(id); // get coordinates of the object point
						char* text = (char*)malloc(1000*sizeof(char));
						sprintf(text, "#%d %.3f, %.3f, %.3f", id, pcoord[0], pcoord[1], pcoord[2]); // sumarize coordinates of the point to the string

						vtkMAFSmartPointer<vtkSphereSource> point; // create a sphere representing the point
						point->SetRadius(0.5);
						point->SetCenter(pcoord[0], pcoord[1], pcoord[2]);

						vtkMAFSmartPointer<vtkPolyDataMapper> pointMapper;
						pointMapper->SetInput(point->GetOutput());

						vtkMAFSmartPointer<vtkActor> pointActor;
						pointActor->PickableOff();
						pointActor->SetMapper(pointMapper.GetPointer());
						pointActor->GetProperty()->SetColor(0.0,0.0,1.0);

						renderer->AddActor(pointActor.GetPointer()); // draw the sphere

						#if defined(_MSC_VER) && _MSC_VER >= 1500
						  ::MessageBoxA(NULL, text, "title", MB_OK || MB_ICONINFORMATION);
						#endif
					}
					else if (keyPressed == (int)'c') {
						int id = cellPicker->GetCellId(); // cell picking
						
						vtkSmartPointer<vtkCell> cell = pickedObj->GetCell(id); // get the cell
						vtkSmartPointer<vtkPoints> cellPoints = cell->GetPoints(); // getting points of the cell
						
						vtkMAFSmartPointer<vtkPoints> points;
						points->InsertNextPoint(cellPoints->GetPoint(0));
						points->InsertNextPoint(cellPoints->GetPoint(1));
						points->InsertNextPoint(cellPoints->GetPoint(2));

						vtkMAFSmartPointer<vtkCellArray> triangles;
						vtkMAFSmartPointer<vtkTriangle> triangle;
						triangle->GetPointIds()->SetId(0, 0);
						triangle->GetPointIds()->SetId(1, 1);
						triangle->GetPointIds()->SetId(2, 2);
						triangles->InsertNextCell(triangle.GetPointer());

						vtkMAFSmartPointer<vtkPolyData> polydata;
						polydata->SetPoints(points.GetPointer());
						polydata->SetPolys(triangles.GetPointer());
  
					  vtkMAFSmartPointer<vtkPolyDataMapper> cellMapper;
						cellMapper->SetInput(polydata.GetPointer());

						vtkMAFSmartPointer<vtkActor> cellActor;
						cellActor->PickableOff();
						cellActor->GetProperty()->SetColor(1.0,1.0,0.0);
						cellActor->SetMapper(cellMapper.GetPointer());

						renderer->AddActor(cellActor.GetPointer()); // draw the cell
						 
						#if defined(_MSC_VER) && _MSC_VER >= 1500
							char* text = (char*)malloc(1000*sizeof(char));
							sprintf(text, "cell id:   %d", id);
							::MessageBoxA(NULL, text, "title", MB_OK || MB_ICONINFORMATION);
							free(text);
						#endif
					}
					keyPressed = 0;
				}
			}
		}
	}
}

//////
//This method visualizes the progress of the method for debugging purposes.
//	It is supposed to be called from FindEnergyMinimumGN method
void vtkMEDPolyDataDeformationPK::Debug_Visualize_Progress( int iterNum, double energy, PKSolutionState ** states, dirList* directions)
{	
	if (iterNum < 0) {
		g_lastCamera = NULL;	//remove previous camera
	}

#pragma region TEXT
	char szText[MAX_PATH];
	if (iterNum < 0)
		szText[0] = '\0';
	else
	{
	#pragma warning(suppress: 4996)
		sprintf(szText, "#%d - energy = %.2f", iterNum, energy);
	}	
	
	vtkMAFSmartPointer< vtkTextMapper > textMapper;  
	textMapper->SetInput(szText);

	vtkMAFSmartPointer < vtkActor2D > textActor;
	textActor->SetPosition(50,30); //20 because of coordinate symbols
	textActor->SetMapper(textMapper.GetPointer());
		
	textActor->GetProperty()->SetColor(0.7, 0.7, 0.7);    //dark grey
#pragma endregion TEXT

#pragma region OBSTACLES
	vtkMAFSmartPointer< vtkAppendPolyData > obstacles;
	vtkMAFSmartPointer< vtkPolyDataMapper > obstaclesMapper;
	vtkMAFSmartPointer< vtkActor > obstaclesActor;

	vtkMAFSmartPointer< vtkAppendPolyData > obstaclesHULL;
	vtkMAFSmartPointer< vtkPolyDataMapper > obstaclesHULLMapper;
	vtkMAFSmartPointer< vtkActor > obstaclesHULLActor;

	if (this->obstacleCount != 0)
	{
	for (int i = 0; i < this->obstacleCount; i++)
	{
		obstacles->AddInput(this->meshes[i + this->modelCount]->GetOriginal());
		obstaclesHULL->AddInput(this->meshes[i + this->modelCount]->GetCoarse());
	}

	obstaclesMapper->SetInput(obstacles->GetOutput());
	obstaclesHULLMapper->SetInput(obstaclesHULL->GetOutput());
	obstaclesActor->SetMapper(obstaclesMapper.GetPointer());
	obstaclesHULLActor->SetMapper(obstaclesHULLMapper.GetPointer());

	obstaclesActor->GetProperty()->SetColor(0.949999988079071, 0.9399999976158142, 0.8100000023841858);
	obstaclesHULLActor->GetProperty()->SetColor(1,1,1); //0.949999988079071, 0.9399999976158142, 0.8100000023841858);
	//obstaclesHULLActor->GetProperty()->SetOpacity(0.6);
	obstaclesHULLActor->GetProperty()->SetRepresentationToWireframe();

	obstaclesActor->PickableOff();
	obstaclesHULLActor->PickableOff();
	obstaclesActor->GetProperty()->SetOpacity(0.5);
	}
#pragma endregion

#pragma region MUSCLES
	vtkMAFSmartPointer< vtkAppendPolyData > models;
	vtkMAFSmartPointer< vtkPolyDataMapper > modelsMapper;
	vtkMAFSmartPointer< vtkActor > modelsActor;

	vtkMAFSmartPointer< vtkAppendPolyData > modelsHULL;
	vtkMAFSmartPointer< vtkPolyDataMapper > modelsHULLMapper;
	vtkMAFSmartPointer< vtkActor > modelsHULLActor;

	if (iterNum < 0)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			models->AddInput(this->meshes[i]->GetOriginal());
			modelsHULL->AddInput(this->meshes[i]->GetCoarse());
		}
	}
	else
	{
		//it will be a bit more complex
		for (int i = 0; i < this->modelCount; i++)
		{
			//we need to preserve the state
			vtkPolyData* pOri_Coarse = this->meshes[i]->GetCoarse()->NewInstance();
			pOri_Coarse->DeepCopy(this->meshes[i]->GetCoarse());

			vtkPolyData* pOri_Mesh = this->meshes[i]->GetOriginal()->NewInstance();
			pOri_Mesh->DeepCopy(this->meshes[i]->GetOriginal());
		
			this->meshes[i]->SetUpPointCoords(this->meshes[i]->GetCoarse(), states[i]->solutionK);	//update coarse, i.e., points of the coarse are moved
			this->meshes[i]->ApplyCoarseCoordsToOriginalMesh(states[i]->solutionK, states[i]->bufferHighResMatrix);									//update detail mesh

			//store results
			vtkPolyData* pNew_Coarse = this->meshes[i]->GetCoarse()->NewInstance();
			pNew_Coarse->ShallowCopy(this->meshes[i]->GetCoarse());

			vtkPolyData* pNew_Mesh = this->meshes[i]->GetOriginal()->NewInstance();
			pNew_Mesh->ShallowCopy(this->meshes[i]->GetOriginal());

			this->meshes[i]->GetCoarse()->ShallowCopy(pOri_Coarse);
			this->meshes[i]->GetOriginal()->ShallowCopy(pOri_Mesh);
			pOri_Coarse->Delete(); pOri_Mesh->Delete();

			models->AddInput(pNew_Mesh);
			modelsHULL->AddInput(pNew_Coarse);

			pNew_Mesh->Delete(); pNew_Coarse->Delete();
		}
	}

	modelsMapper->SetInput(models->GetOutput());
	modelsHULLMapper->SetInput(modelsHULL->GetOutput());
	modelsActor->SetMapper(modelsMapper.GetPointer());
	modelsHULLActor->SetMapper(modelsHULLMapper.GetPointer());

	modelsActor->GetProperty()->SetColor(0.9599999785423279, 0.2899999916553497, 0.2899999916553497);
	modelsHULLActor->GetProperty()->SetColor(1, 0, 0); //0.9599999785423279, 0.2899999916553497, 0.2899999916553497);
	//modelsHULLActor->GetProperty()->SetOpacity(0.6);
	modelsHULLActor->GetProperty()->SetRepresentationToWireframe();

	modelsActor->GetProperty()->SetOpacity(0.8);
	modelsHULLActor->PickableOff();
#pragma endregion


	//----------------------------------------------------------------------------
	// Render
	//----------------------------------------------------------------------------
	vtkRenderer *ren1= vtkRenderer::New();
	if (this->obstacleCount != 0)
	{
	ren1->AddActor( obstaclesHULLActor.GetPointer());
	ren1->AddActor( obstaclesActor.GetPointer());
	}
	ren1->AddActor( modelsHULLActor.GetPointer());
	ren1->AddActor( modelsActor.GetPointer());
	ren1->AddActor( textActor.GetPointer());

#pragma region WRAPPER
	boolean startColor = true;
	for(int i = 0; i < nWrappers; i++) {
		vtkMAFSmartPointer <vtkPolyData> w = wrapper[i];
		vtkMAFSmartPointer <vtkPoints> wrapperPoints = w->GetPoints();
		int nWrapperPoints = wrapperPoints->GetNumberOfPoints();

		double *point;
		for (int j = 0; j < nWrapperPoints; j++) 
		{      
			point = wrapperPoints->GetPoint(j);
			vtkMAFSmartPointer <vtkSphereSource> pointS; // original point
			pointS->SetRadius(1.5);
			pointS->SetCenter(point);

			vtkMAFSmartPointer <vtkPolyDataMapper> pointMapper;
			pointMapper->SetInput(pointS->GetOutput());

			vtkMAFSmartPointer <vtkActor> pointActor;
			pointActor->SetMapper(pointMapper);
			if (startColor) pointActor->GetProperty()->SetColor(0.0,0.1*j,0.0);
			else pointActor->GetProperty()->SetColor(0.0,0.0,0.1*j);
			ren1->AddActor(pointActor);
		}

		vtkMAFSmartPointer <vtkTubeFilter> tbsource;
		tbsource->SetInput(w);
		tbsource->SetRadius(1);
		tbsource->SetNumberOfSides(8);
		tbsource->SetCapping(1);
		tbsource->Update();

		vtkMAFSmartPointer< vtkPolyDataMapper > wrapperMapper;
		vtkMAFSmartPointer< vtkActor > wrapperActor;

		wrapperMapper->SetInput(tbsource->GetOutput());
		wrapperActor->SetMapper(wrapperMapper.GetPointer());

		if (startColor) wrapperActor->GetProperty()->SetColor(0, 0, 1);
		else wrapperActor->GetProperty()->SetColor(0, 1, 0);
		startColor = !startColor;
		ren1->AddActor(wrapperActor);
	}

#pragma endregion

#pragma region DIRECTIONS
	// draw all fixed points
	if (directions != NULL) {
		for (int i = 0; i < (int)directions->size(); i++) { // for each vertex data in the list
			DirDesc dd = directions->at(i);
			double* point = dd.point.data; // original position
			double* direction = dd.dir.data; // direction
			double size = dd.bestT; // distance of vertex motion
			
			double temp[3] = {0,0,0}; // new position of the point
			PKUtils::CopyVertex(direction, temp);
			PKUtils::MultiplyVertex(temp, size);
			PKUtils::AddVertex(point, temp, temp);

			vtkLineSource *line = vtkLineSource::New(); // line segment between both points
			line->SetPoint1(point);
			line->SetPoint2(temp);

			vtkPolyDataMapper *lineMapper = vtkPolyDataMapper::New();
			lineMapper->SetInput(line->GetOutput());

			vtkActor *lineActor = vtkActor::New();
			lineActor->SetMapper(lineMapper);
	
			ren1->AddActor(lineActor);
			
			
			vtkSphereSource *pointS = vtkSphereSource::New(); // original point
			pointS->SetRadius(0.3);
			pointS->SetCenter(point);

			vtkPolyDataMapper *pointMapper = vtkPolyDataMapper::New();
			pointMapper->SetInput(pointS->GetOutput());

			vtkActor *pointActor = vtkActor::New();
			pointActor->SetMapper(pointMapper);
			pointActor->GetProperty()->SetColor(1.0,1.0,1.0);

			ren1->AddActor(pointActor);

			vtkSphereSource *pointS2 = vtkSphereSource::New(); // point in a new position
			pointS2->SetRadius(0.3);
			pointS2->SetCenter(temp);

			vtkPolyDataMapper *pointMapper2 = vtkPolyDataMapper::New();
			pointMapper2->SetInput(pointS2->GetOutput());

			vtkActor *pointActor2 = vtkActor::New();
			pointActor2->SetMapper(pointMapper2);
			pointActor2->GetProperty()->SetColor(dd.color.data);

			ren1->AddActor(pointActor2);

			line->Delete();
			lineMapper->Delete();
			lineActor->Delete();
			pointS->Delete();
			pointMapper->Delete();
			pointActor->Delete();
			pointS2->Delete();
			pointMapper2->Delete();
			pointActor2->Delete();
		}
	}
	
#pragma endregion
	

	if (g_lastCamera.GetPointer() != NULL)
		ren1->SetActiveCamera(g_lastCamera.GetPointer());

	//ren1->AddActor(pActorAL);
	ren1->SetBackground( 0, 0, 0 );

	vtkRenderWindow *renWin = vtkRenderWindow::New();
	renWin->AddRenderer( ren1 );
	renWin->SetSize( 1280, 950 );
	

	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);


	vtkCallbackCommand *keypressCallback = vtkCallbackCommand::New();  
	keypressCallback->ClientData = (void*)this;
	keypressCallback->SetCallback(&vtkMEDPolyDataDeformationPK::KeypressCallbackFunction); 
	iren->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);   
	keypressCallback->Delete();

	vtkCallbackCommand *mouseCallback = vtkCallbackCommand::New();  
	mouseCallback->ClientData = (void*)this;
	mouseCallback->SetCallback(&vtkMEDPolyDataDeformationPK::MouseCallbackFunction); 
	iren->AddObserver(vtkCommand::LeftButtonPressEvent, mouseCallback);   
	mouseCallback->Delete();

	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	iren->Initialize();
	iren->Start();	

	g_lastCamera = ren1->GetActiveCamera();

	//Remove data
	iren->SetInteractorStyle(NULL);
	style->Delete();
	iren->Delete();
	renWin->Delete();
	
	ren1->RemoveAllProps();	//remove all actors
	ren1->Delete();
}

//////
//This method visualizes the progress of the method for debugging purposes.
//	It is supposed to be called from FindEnergyMinimumGPU method
void vtkMEDPolyDataDeformationPK::Debug_Visualize_ProgressGPU( int iterNum, double energy, PKSolutionStateGPU ** states, dirList* directions)
{	
	if (iterNum < 0) {
		g_lastCamera = NULL;	//remove previous camera
	}

#pragma region TEXT
	char szText[MAX_PATH];
	if (iterNum < 0)
		szText[0] = '\0';
	else
	{
	#pragma warning(suppress: 4996)
		sprintf(szText, "#%d - energy = %.2f", iterNum, energy);
	}	
	
	vtkMAFSmartPointer< vtkTextMapper > textMapper;  
	textMapper->SetInput(szText);

	vtkMAFSmartPointer < vtkActor2D > textActor;
	textActor->SetPosition(50,30); //20 because of coordinate symbols
	textActor->SetMapper(textMapper.GetPointer());
		
	textActor->GetProperty()->SetColor(0.7, 0.7, 0.7);    //dark grey
#pragma endregion TEXT

#pragma region OBSTACLES
	vtkMAFSmartPointer< vtkAppendPolyData > obstacles;
	vtkMAFSmartPointer< vtkPolyDataMapper > obstaclesMapper;
	vtkMAFSmartPointer< vtkActor > obstaclesActor;

	vtkMAFSmartPointer< vtkAppendPolyData > obstaclesHULL;
	vtkMAFSmartPointer< vtkPolyDataMapper > obstaclesHULLMapper;
	vtkMAFSmartPointer< vtkActor > obstaclesHULLActor;

	if (this->obstacleCount != 0)
	{
	for (int i = 0; i < this->obstacleCount; i++)
	{
		obstacles->AddInput(this->meshes[i + this->modelCount]->GetOriginal());
		obstaclesHULL->AddInput(this->meshes[i + this->modelCount]->GetCoarse());
	}

	obstaclesMapper->SetInput(obstacles->GetOutput());
	obstaclesHULLMapper->SetInput(obstaclesHULL->GetOutput());
	obstaclesActor->SetMapper(obstaclesMapper.GetPointer());
	obstaclesHULLActor->SetMapper(obstaclesHULLMapper.GetPointer());

	obstaclesActor->GetProperty()->SetColor(0.949999988079071, 0.9399999976158142, 0.8100000023841858);
	obstaclesHULLActor->GetProperty()->SetColor(1,1,1); //0.949999988079071, 0.9399999976158142, 0.8100000023841858);
	//obstaclesHULLActor->GetProperty()->SetOpacity(0.6);
	obstaclesHULLActor->GetProperty()->SetRepresentationToWireframe();

	obstaclesActor->PickableOff();
	obstaclesHULLActor->PickableOff();
	obstaclesActor->GetProperty()->SetOpacity(0.5);
	}
#pragma endregion

#pragma region MUSCLES
	vtkMAFSmartPointer< vtkAppendPolyData > models;
	vtkMAFSmartPointer< vtkPolyDataMapper > modelsMapper;
	vtkMAFSmartPointer< vtkActor > modelsActor;

	vtkMAFSmartPointer< vtkAppendPolyData > modelsHULL;
	vtkMAFSmartPointer< vtkPolyDataMapper > modelsHULLMapper;
	vtkMAFSmartPointer< vtkActor > modelsHULLActor;

	if (iterNum < 0)
	{
		for (int i = 0; i < this->modelCount; i++)
		{
			models->AddInput(this->meshes[i]->GetOriginal());
			modelsHULL->AddInput(this->meshes[i]->GetCoarse());
		}
	}
	else
	{
		//it will be a bit more complex
		for (int i = 0; i < this->modelCount; i++)
		{
			//we need to preserve the state
			vtkPolyData* pOri_Coarse = this->meshes[i]->GetCoarse()->NewInstance();
			pOri_Coarse->DeepCopy(this->meshes[i]->GetCoarse());

			vtkPolyData* pOri_Mesh = this->meshes[i]->GetOriginal()->NewInstance();
			pOri_Mesh->DeepCopy(this->meshes[i]->GetOriginal());
		
			this->meshes[i]->SetUpPointCoords(this->meshes[i]->GetCoarse(), states[i]->solutionK);	//update coarse, i.e., points of the coarse are moved
			this->meshes[i]->ApplyCoarseCoordsToOriginalMesh(states[i]->solutionK, states[i]->bufferHighResMatrix);									//update detail mesh

			//store results
			vtkPolyData* pNew_Coarse = this->meshes[i]->GetCoarse()->NewInstance();
			pNew_Coarse->ShallowCopy(this->meshes[i]->GetCoarse());

			vtkPolyData* pNew_Mesh = this->meshes[i]->GetOriginal()->NewInstance();
			pNew_Mesh->ShallowCopy(this->meshes[i]->GetOriginal());

			this->meshes[i]->GetCoarse()->ShallowCopy(pOri_Coarse);
			this->meshes[i]->GetOriginal()->ShallowCopy(pOri_Mesh);
			pOri_Coarse->Delete(); pOri_Mesh->Delete();

			models->AddInput(pNew_Mesh);
			modelsHULL->AddInput(pNew_Coarse);

			pNew_Mesh->Delete(); pNew_Coarse->Delete();
		}
	}

	modelsMapper->SetInput(models->GetOutput());
	modelsHULLMapper->SetInput(modelsHULL->GetOutput());
	modelsActor->SetMapper(modelsMapper.GetPointer());
	modelsHULLActor->SetMapper(modelsHULLMapper.GetPointer());

	modelsActor->GetProperty()->SetColor(0.9599999785423279, 0.2899999916553497, 0.2899999916553497);
	modelsHULLActor->GetProperty()->SetColor(1, 0, 0); //0.9599999785423279, 0.2899999916553497, 0.2899999916553497);
	//modelsHULLActor->GetProperty()->SetOpacity(0.6);
	modelsHULLActor->GetProperty()->SetRepresentationToWireframe();

	modelsActor->GetProperty()->SetOpacity(0.8);
	modelsHULLActor->PickableOff();
#pragma endregion


	//----------------------------------------------------------------------------
	// Render
	//----------------------------------------------------------------------------
	vtkRenderer *ren1= vtkRenderer::New();
	if (this->obstacleCount != 0)
	{
	ren1->AddActor( obstaclesHULLActor.GetPointer());
	ren1->AddActor( obstaclesActor.GetPointer());
	}
	ren1->AddActor( modelsHULLActor.GetPointer());
	ren1->AddActor( modelsActor.GetPointer());
	ren1->AddActor( textActor.GetPointer());

#pragma region WRAPPER
	boolean startColor = true;
	for(int i = 0; i < nWrappers; i++) {
		vtkMAFSmartPointer <vtkPolyData> w = wrapper[i];
		vtkMAFSmartPointer <vtkPoints> wrapperPoints = w->GetPoints();
		int nWrapperPoints = wrapperPoints->GetNumberOfPoints();

		double *point;
		for (int j = 0; j < nWrapperPoints; j++) 
		{      
			point = wrapperPoints->GetPoint(j);
			vtkMAFSmartPointer <vtkSphereSource> pointS; // original point
			pointS->SetRadius(1.5);
			pointS->SetCenter(point);

			vtkMAFSmartPointer <vtkPolyDataMapper> pointMapper;
			pointMapper->SetInput(pointS->GetOutput());

			vtkMAFSmartPointer <vtkActor> pointActor;
			pointActor->SetMapper(pointMapper);
			if (startColor) pointActor->GetProperty()->SetColor(0.0,0.1*j,0.0);
			else pointActor->GetProperty()->SetColor(0.0,0.0,0.1*j);
			ren1->AddActor(pointActor);
		}

		vtkMAFSmartPointer <vtkTubeFilter> tbsource;
		tbsource->SetInput(w);
		tbsource->SetRadius(1);
		tbsource->SetNumberOfSides(8);
		tbsource->SetCapping(1);
		tbsource->Update();

		vtkMAFSmartPointer< vtkPolyDataMapper > wrapperMapper;
		vtkMAFSmartPointer< vtkActor > wrapperActor;

		wrapperMapper->SetInput(tbsource->GetOutput());
		wrapperActor->SetMapper(wrapperMapper.GetPointer());

		if (startColor) wrapperActor->GetProperty()->SetColor(0, 0, 1);
		else wrapperActor->GetProperty()->SetColor(0, 1, 0);
		startColor = !startColor;
		ren1->AddActor(wrapperActor);
	}

#pragma endregion

#pragma region DIRECTIONS
	// draw all fixed points
	if (directions != NULL) {
		for (int i = 0; i < (int)directions->size(); i++) { // for each vertex data in the list
			DirDesc dd = directions->at(i);
			double* point = dd.point.data; // original position
			double* direction = dd.dir.data; // direction
			double size = dd.bestT; // distance of vertex motion
			
			double temp[3] = {0,0,0}; // new position of the point
			PKUtils::CopyVertex(direction, temp);
			PKUtils::MultiplyVertex(temp, size);
			PKUtils::AddVertex(point, temp, temp);

			vtkLineSource *line = vtkLineSource::New(); // line segment between both points
			line->SetPoint1(point);
			line->SetPoint2(temp);

			vtkPolyDataMapper *lineMapper = vtkPolyDataMapper::New();
			lineMapper->SetInput(line->GetOutput());

			vtkActor *lineActor = vtkActor::New();
			lineActor->SetMapper(lineMapper);
	
			ren1->AddActor(lineActor);
			
			
			vtkSphereSource *pointS = vtkSphereSource::New(); // original point
			pointS->SetRadius(0.3);
			pointS->SetCenter(point);

			vtkPolyDataMapper *pointMapper = vtkPolyDataMapper::New();
			pointMapper->SetInput(pointS->GetOutput());

			vtkActor *pointActor = vtkActor::New();
			pointActor->SetMapper(pointMapper);
			pointActor->GetProperty()->SetColor(1.0,1.0,1.0);

			ren1->AddActor(pointActor);

			vtkSphereSource *pointS2 = vtkSphereSource::New(); // point in a new position
			pointS2->SetRadius(0.3);
			pointS2->SetCenter(temp);

			vtkPolyDataMapper *pointMapper2 = vtkPolyDataMapper::New();
			pointMapper2->SetInput(pointS2->GetOutput());

			vtkActor *pointActor2 = vtkActor::New();
			pointActor2->SetMapper(pointMapper2);
			pointActor2->GetProperty()->SetColor(dd.color.data);

			ren1->AddActor(pointActor2);

			line->Delete();
			lineMapper->Delete();
			lineActor->Delete();
			pointS->Delete();
			pointMapper->Delete();
			pointActor->Delete();
			pointS2->Delete();
			pointMapper2->Delete();
			pointActor2->Delete();
		}
	}
	
#pragma endregion
	

	if (g_lastCamera.GetPointer() != NULL)
		ren1->SetActiveCamera(g_lastCamera.GetPointer());

	//ren1->AddActor(pActorAL);
	ren1->SetBackground( 0, 0, 0 );

	vtkRenderWindow *renWin = vtkRenderWindow::New();
	renWin->AddRenderer( ren1 );
	renWin->SetSize( 1280, 950 );
	

	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);


	vtkCallbackCommand *keypressCallback = vtkCallbackCommand::New();  
	keypressCallback->ClientData = (void*)this;
	keypressCallback->SetCallback(&vtkMEDPolyDataDeformationPK::KeypressCallbackFunction); 
	iren->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);   
	keypressCallback->Delete();

	vtkCallbackCommand *mouseCallback = vtkCallbackCommand::New();  
	mouseCallback->ClientData = (void*)this;
	mouseCallback->SetCallback(&vtkMEDPolyDataDeformationPK::MouseCallbackFunction); 
	iren->AddObserver(vtkCommand::LeftButtonPressEvent, mouseCallback);   
	mouseCallback->Delete();

	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	iren->Initialize();
	iren->Start();	

	g_lastCamera = ren1->GetActiveCamera();

	//Remove data
	iren->SetInteractorStyle(NULL);
	style->Delete();
	iren->Delete();
	renWin->Delete();
	
	ren1->RemoveAllProps();	//remove all actors
	ren1->Delete();
}
