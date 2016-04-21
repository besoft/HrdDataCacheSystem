/*=========================================================================
Program: Multimod Application Framework RELOADED
Module: $RCSfile: vtkMAFVisualDebugger.cpp,v $
Language: C++
Date: $Date: 2009-05-19 14:30:01 $
Version: $Revision: 1.0 $
Authors: Josef Kohout, Jana Hájková
==========================================================================
Copyright (c) 2012 University of University of West Bohemia
See the COPYINGS file for license details
=========================================================================
*/
#include "vtkMAFVisualDebugger.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkCubeSource.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkTubeFilter.h"
#include "vtkSmartPointer.h"
#include "vtkCellArray.h"
#include "vtkCamera.h"
#include "vtkCallbackCommand.h"
#include "vtkRendererCollection.h"
#include "vtkTextActor.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkProperty2D.h"
#include "vtkMAFSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkScalarBarActor.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#pragma warning(pop)

vtkStandardNewMacro(vtkMAFVisualDebugger);

#include "mafMemDbg.h"
#include "mafDbg.h"

//define secure versions of C functions if available
#if defined(_MSC_VER) && _MSC_VER >= 1500
#define strcpy	strcpy_s
#define sprintf sprintf_s
#define strcat	strcat_s
#endif

/** initalize a given LUT with the idx.th preset */
MAF_EXPORT extern void lutPreset(int idx, vtkLookupTable *lut);

vtkMAFVisualDebugger::vtkMAFVisualDebugger()
{
	m_NavigationState = vtkMAFVisualDebugger::NextStep;
	m_NumberOfIgnoredSteps = 5;
	m_nSkipIndex = -1;
	m_CurrentFrameId = -1;

	WindowLeft = WindowTop = 0;
	WindowWidth = 1024;
	WindowHeight = 768;
	BackgroundColor[0] = BackgroundColor[1] = BackgroundColor[2] = 0.0; //black

	m_Renderer = vtkRenderer::New();
	m_TotalResizeCoef = 1.0;
}

vtkMAFVisualDebugger::~vtkMAFVisualDebugger()
{
	RemoveAll();	//remove all objects,
	//N.B, if RemoveAll is overriden, the inherited class must call RemoveAll in its destructor because
	//otherwise it won't be called (naturally)

	if (m_Renderer != NULL) {
		m_Renderer->Delete(); m_Renderer = NULL;
	}
}

//------------------------------------------------------------------------
//Removes all objects from the scene.
/*virtual*/ void vtkMAFVisualDebugger::RemoveAll()
	//------------------------------------------------------------------------
{
	/*VisualObjectsMap::iterator it;
	for (it = m_VisualObjects.begin(); it != m_VisualObjects.end(); it++){		
		delete it->second; //free the memory
	}

	m_VisualObjects.clear();
	*/
	while (!m_VisualObjects.empty()){
		RemoveVisualObject(m_VisualObjects.begin()->first);
	}
}

//------------------------------------------------------------------------
//Adds the new visual object into the scene or updates it, if the object already exists in the scene.
//N.B. obj must be allocated on the heap and will be removed automatically when
//corresponding RemoveVisualObject or	RemoveAll is called
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateVisualObject(const char* Id, VISUAL_OBJECT* obj)
	//------------------------------------------------------------------------
{
	RemoveVisualObject(Id);	//make sure, the object does not exist

	for (int i = 0; i < obj->MAX_ACTORS; i++)
	{
		if (obj->pActors[i] != NULL) 
		{
			obj->pActors[i]->SetPickable(0);	//set it not pickable to speed-up rendering
			m_Renderer->AddActor(obj->pActors[i]);
		}
	}

	m_VisualObjects[Id] = obj;
}

//------------------------------------------------------------------------
//Removes the visual object from the scene.
/*virtual*/ void vtkMAFVisualDebugger::RemoveVisualObject(const char* Id)
//------------------------------------------------------------------------
{
	VisualObjectsMap::iterator it = m_VisualObjects.find(Id);
	if (it == m_VisualObjects.end())
		return;	//not found, so exit

	VISUAL_OBJECT* obj = it->second;
	for (int i = 0; i < obj->MAX_ACTORS; i++)
	{
		if (obj->pActors[i] != NULL) {
			m_Renderer->RemoveActor(obj->pActors[i]);
		}
	}

	delete obj;	//free memory

	m_VisualObjects.erase(it);
}

//------------------------------------------------------------------------
// Adds some label with the given Id from the scene.
//If an object with the same Id already exists, it is updated.
//The label is displayed using default font family and the given font size
//in (r, g, b) colour at position x and y. Position is relative to the specified
//horizontal and vertical alignment (parameters horiz_align and vert_align).
//Valid values of alignment are -1, 0, 1 denoting LEFT, CENTER, RIGHT
//or TOP, CENTER, BOTTOM alignment.
/*virtual*/  void vtkMAFVisualDebugger::AddOrUpdateLabel(const char* Id,
	const char* label, int fontsize,
	double r, double g, double b,
	double x, double y,
	int horiz_align, int vert_align)
{
	VISUAL_OBJECT* obj = new VISUAL_OBJECT();

	vtkMAFSmartPointer< vtkTextMapper > textMapper;
	textMapper->SetInput(label);

	vtkMAFSmartPointer < vtkTextActor > textActor;
	textActor->SetMapper(textMapper.GetPointer());

	//calculate position
	double delta_x = 0.0, delta_y = 0.0;
	if (horiz_align == 0)
		delta_x = 0.5;
	else if (horiz_align > 0)
		delta_x = 1.0;

	if (vert_align == 0)
		delta_y = 0.5;
	else if (vert_align > 0)
		delta_y = 1.0;

	textActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	textActor->GetPositionCoordinate()->SetValue(x + delta_x, y + delta_y);
	textActor->GetProperty()->SetColor(r, g, b);

	textActor->GetTextProperty()->SetFontSize(fontsize);

	obj->pActors[0] = textActor.GetPointer();
	AddOrUpdateVisualObject(Id, obj);
}

//------------------------------------------------------------------------
//Adds scalar field with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//Legend is displayed, if displaybar is true, in which case legend is shown
//at x, y position relative to the window size and values of horiz_align and
//vert_align - see also AddOrUpdateLabel method.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateScalarField(const char* Id,
	vtkPolyData* poly, bool displaybar, 
	double x, double y,	int horiz_align, int vert_align)
	//------------------------------------------------------------------------
{
	double sr[2];
	poly->GetPointData()->GetScalars()->GetRange(sr);           

	//build LUT
  vtkLookupTable* scalarFieldLUT = vtkLookupTable::New();	
  lutPreset(0, scalarFieldLUT); //initialize LUT to default one (it has index 0)  
	scalarFieldLUT->Build();

	scalarFieldLUT->SetTableRange(sr);

	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInput(poly);
  mapper->ImmediateModeRenderingOn();
  mapper->SetScalarModeToUsePointData();
  mapper->SetColorModeToMapScalars();
  mapper->SetLookupTable(scalarFieldLUT);
	mapper->SetScalarRange(sr);
	scalarFieldLUT->Delete();

	vtkMAFSmartPointer< vtkActor > scalarActor;  
  scalarActor->SetMapper(mapper);  
	
	
	VISUAL_OBJECT* obj = new VISUAL_OBJECT();
	obj->pActors[0] = scalarActor.GetPointer();

	vtkMAFSmartPointer< vtkScalarBarActor> mapActor;

	if (displaybar)
	{
		//we need to specify also legend
		mapActor = vtkScalarBarActor::New();
		mapActor->SetLookupTable(mapper->GetLookupTable());

		//calculate position
		double delta_x = 0.0, delta_y = 0.0;
		if (horiz_align == 0)
			delta_x = 0.5;
		else if (horiz_align > 0)
			delta_x = 1.0;

		if (vert_align == 0)
			delta_y = 0.5;
		else if (vert_align > 0)
			delta_y = 1.0;

		//specify position
		((vtkActor2D*)mapActor)->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
		((vtkActor2D*)mapActor)->GetPositionCoordinate()->SetValue(delta_x + x, delta_y + y);

		mapActor->SetOrientationToHorizontal();
		mapActor->SetWidth(0.8);
		mapActor->SetHeight(0.12);  
		mapActor->SetLabelFormat("%6.0f");
		mapActor->SetPickable(0);   //make it faster
		
		//if window is too light, make our label darker so it is well visible
		if ((this->BackgroundColor[0]*0.3 + this->BackgroundColor[1]*0.6 + this->BackgroundColor[0]*0.1) > 0.5)
			mapActor->GetProperty()->SetColor(0, 0, 0);
		obj->pActors[1] = mapActor.GetPointer();
	}

	mapper->Delete();

  AddOrUpdateVisualObject(Id, obj);
}

//------------------------------------------------------------------------
//Adds surface mesh with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//(R,G,B) is colour in which the mesh is to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateSurface(const char* Id,
	vtkPolyData* poly, 	double r, double g, double b,
	double opacity,	bool wireframe)
	//------------------------------------------------------------------------
{
	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
	mapper->SetInput(poly);
		
	vtkMAFSmartPointer< vtkActor > pointsActor;
	pointsActor->SetMapper(mapper);
	mapper->Delete();

	pointsActor->GetProperty()->SetColor(r, g, b);
	pointsActor->GetProperty()->SetOpacity(opacity);
	
	if (wireframe)
		pointsActor->GetProperty()->SetRepresentationToWireframe();

	VISUAL_OBJECT* obj = new VISUAL_OBJECT();
	obj->pActors[0] = pointsActor.GetPointer();
	AddOrUpdateVisualObject(Id, obj);
}

//------------------------------------------------------------------------
//Adds points with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//Radius = radius of spheres to be rendered for the given points,
//(R,G,B) is colour in which spheres are to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdatePoints(const char* Id, 
	vtkPolyData* points, double radius, 
	double r, double g, double b,
	double opacity,	bool wireframe, bool bcubeglyph)
	//------------------------------------------------------------------------
{	
	vtkGlyph3D* glfilter = vtkGlyph3D::New();
	vtkSphereSource* sphere = NULL;
	vtkCubeSource* cube = NULL;
	
	if (!bcubeglyph)
	{
		sphere = vtkSphereSource::New();
		sphere->SetRadius(radius*m_TotalResizeCoef);
		sphere->SetPhiResolution(6);
		sphere->SetThetaResolution(6);
		
		glfilter->SetSource(sphere->GetOutput());
	}
	else
	{
		cube = vtkCubeSource::New();
		cube->SetXLength(radius);
		cube->SetYLength(radius);
		cube->SetZLength(radius);

		glfilter->SetSource(cube->GetOutput());
	}
	
	glfilter->SetInput(points);

	AddOrUpdateSurface(Id, glfilter->GetOutput(), r, g, b, opacity, wireframe);
	glfilter->Delete();

	VISUAL_OBJECT* obj = m_VisualObjects[Id];
	obj->pResizableSource = sphere;

	if (sphere != NULL)
		sphere->Delete();		
	if (cube != NULL)
		cube->Delete();
}

//------------------------------------------------------------------------
//Adds points with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//Radius = radius of spheres to be rendered for the given points,
//(R,G,B) is colour in which spheres are to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdatePoints(const char* Id, 
	vtkPoints* points, double radius, 
	double r, double g, double b,
	double opacity,	bool wireframe, bool bCubeglyph)
	//------------------------------------------------------------------------
{
	vtkPolyData* pointToInsert = vtkPolyData::New();
	pointToInsert->SetPoints(points);
	
	AddOrUpdatePoints(Id, pointToInsert, radius, r, g, b, opacity, wireframe, bCubeglyph);
	
	pointToInsert->Delete();
}

//------------------------------------------------------------------------
//Adds points with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//Points = array of points in format (x1,y1,z1, x2, y2, z2, ...)
//Count = number of points in points array
//Radius = radius of spheres to be rendered for the given points,
//(R,G,B) is colour in which spheres are to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
//If bCubeglyph is true, small cubes are rendered instead of spheres.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdatePoints(const char* Id, 
	const double* points, int count, double radius, 
	double r, double g, double b,
	double opacity,	bool wireframe, bool bCubeglyph)
	//------------------------------------------------------------------------
{			
	vtkPoints* pp = vtkPoints::New(VTK_DOUBLE);
	pp->SetNumberOfPoints(count);
	for (int i = 0; i < count; i++) {
		pp->SetPoint(i, points + 3*i);
	}

	AddOrUpdatePoints(Id, pp, radius, r, g, b, opacity, wireframe, bCubeglyph);
	
	pp->Delete();
}

//------------------------------------------------------------------------
//Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
//only the points having their indices in validIndices array containing  validIndicesCount entries.
//Note: validIndices can be easily retrieved from vtkIdList.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateSelectedPoints(const char* Id, 
	vtkPolyData* points, const vtkIdType* validIndices,
	int validIndicesCount, double radius, 
	double r, double g, double b, double opacity,	
	bool wireframe, bool bCubeglyph)
{
	AddOrUpdateSelectedPoints(Id, points->GetPoints(),
		validIndices, validIndicesCount, radius, r, g, b, opacity, wireframe, bCubeglyph);
}

//------------------------------------------------------------------------
//Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
//only the points having their indices in validIndices array containing  validIndicesCount entries.
//Note: validIndices can be easily retrieved from vtkIdList. 
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateSelectedPoints(const char* Id, 
	vtkPoints* points,  const vtkIdType* validIndices,
	int validIndicesCount, double radius, 
	double r, double g, double b, double opacity,	bool wireframe, bool bCubeglyph)
	//------------------------------------------------------------------------
{
	vtkPoints* pp = vtkPoints::New();
	pp->SetNumberOfPoints(validIndicesCount);
	for (int i = 0; i < validIndicesCount; i++) {
		pp->SetPoint(i, points->GetPoint(validIndices[i]));
	}
	
	AddOrUpdatePoints(Id, pp, radius, r, g, b, opacity, wireframe, bCubeglyph);	
	pp->Delete();
}

//------------------------------------------------------------------------
//Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
//only the points having their indices in validIndices array containing  validIndicesCount entries.
//Note: validIndices can be easily retrieved from vtkIdList. 
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateSelectedPoints(const char* Id, 
	const double* points, int count,  const vtkIdType* validIndices,
	int validIndicesCount, double radius, 
	double r, double g, double b,
	double opacity,	bool wireframe, bool bCubeglyph)
	//------------------------------------------------------------------------
{	
	vtkPoints* pp = vtkPoints::New();
	pp->SetNumberOfPoints(validIndicesCount);
	for (int i = 0; i < validIndicesCount; i++) {
		pp->SetPoint(i, points + 3*validIndices[i]);
	}
	
	AddOrUpdatePoints(Id, pp, radius, r, g, b, opacity, wireframe, bCubeglyph);	
	pp->Delete();
}

//------------------------------------------------------------------------
// Adds lines with the given Id into the scene.
//If an object with the same Id already exists, it is updated.
//Radius = radius of tubes to be rendered for the given poly-lines,
//(R,G,B) is colour in which tubes are to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateLines(const char* Id,
	vtkPolyData* polylines, double radius,
	double r, double g, double b,
	double opacity,	bool wireframe)
	//------------------------------------------------------------------------
{	
	vtkTubeFilter*  tube = vtkTubeFilter::New();
	tube->SetInput(polylines);
	tube->SetRadius(radius*m_TotalResizeCoef);
	tube->SetNumberOfSides(6);

	AddOrUpdateSurface(Id, tube->GetOutput(), r, g, b, opacity, wireframe);

	VISUAL_OBJECT* obj = m_VisualObjects[Id];
	obj->pResizableSource = tube;

	tube->Delete();	
}

//------------------------------------------------------------------------
//Adds lines with the given Id into the scene. 
//If an object with the same Id already exists, it is updated.
//PolyLine contains the connectivity of the poly-line, polyLineCount is number of entries.
//Radius = radius of tubes to be rendered for the given poly-lines,
//(R,G,B) is colour in which tubes are to be rendered.
//Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateLines(const char* Id,
	vtkPoints* points, const vtkIdType* polyLine, int polyLineCount, 
	double radius, double r, double g, double b,	double opacity,	bool wireframe)
	//------------------------------------------------------------------------
{
	vtkPolyData* poly = vtkPolyData::New();
	poly->SetPoints(points);

	vtkCellArray* cells = vtkCellArray::New();
	cells->Allocate(polyLineCount - 1);
	for (int i = 1; i < polyLineCount; i++)
	{
		vtkIdType clId[2] = {polyLine[i - 1], polyLine[i] };
		cells->InsertNextCell(2, clId);
	}	

	poly->SetLines(cells);
	cells->Delete();

	AddOrUpdateLines(Id, poly, radius, r, g, b, opacity, wireframe);	
	poly->Delete();
}

//------------------------------------------------------------------------
/** Adds external actor with the given Id into the scene.
If an object with the same Id already exists, it is updated. */
/*virtual*/ void vtkMAFVisualDebugger::AddOrUpdateExternalObject(const char* Id, vtkProp* actor)
//------------------------------------------------------------------------
{
	VISUAL_OBJECT* obj = new VISUAL_OBJECT();
	obj->pActors[0] = actor;

	AddOrUpdateVisualObject(Id, obj);
}

//------------------------------------------------------------------------
//Opens the rendering window displaying everything and interacting with the user.
//The method ends when the user expresses his/her wishes to continue with the method being debugged.
/*virtual*/ void vtkMAFVisualDebugger::DebugStep()
	//------------------------------------------------------------------------
{
	if (m_NavigationState == vtkMAFVisualDebugger::StopDebugging)
		return;	//there is nothing for us to do

	++m_CurrentFrameId;	//increase the number of frames

	if (m_NavigationState != vtkMAFVisualDebugger::NextStep) {
		m_NavigationState = (FrameNavigation)(m_NavigationState - 1); return;	//decrease the number of
	}

	vtkRenderWindow *renWin = vtkRenderWindow::New();
	m_Renderer->SetBackground(BackgroundColor);

	renWin->AddRenderer(m_Renderer);
	renWin->SetSize(WindowWidth, WindowHeight);
	renWin->SetPosition(WindowLeft, WindowTop);	
	
	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);

	vtkCallbackCommand *keypressCallback = vtkCallbackCommand::New();
	keypressCallback->ClientData = (void*)this;
	keypressCallback->SetCallback(&vtkMAFVisualDebugger::KeypressCallback);
	iren->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	iren->Initialize();

	char szFrameId[20];
	sprintf(szFrameId, "Frame #%d", m_CurrentFrameId);
	renWin->SetWindowName(szFrameId);

	iren->Start();

	//Keep changed staff
	WindowLeft = renWin->GetPosition()[0];
	WindowTop = renWin->GetPosition()[1];

	WindowWidth = renWin->GetSize()[0];
	WindowHeight = renWin->GetSize()[1];

	//Remove data
	iren->SetInteractorStyle(NULL);
	style->Delete();
	iren->Delete();
	renWin->Delete();
}

//------------------------------------------------------------------------
/*static*/ void vtkMAFVisualDebugger::KeypressCallback(vtkObject* caller,
		long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) )
//------------------------------------------------------------------------
{
	vtkMAFVisualDebugger* pThis = (vtkMAFVisualDebugger*)clientData;

	char key = ((vtkRenderWindowInteractor*)caller)->GetKeyCode();

	if (key == char(27)) {  // if ESC, close the window and continue in visualization
			((vtkRenderWindowInteractor*)caller)->ExitCallback();
	}
	else if (key== 'x') { //  if key X is pressed, visualization is stopped from the next step
		pThis->m_NavigationState = vtkMAFVisualDebugger::StopDebugging;
		((vtkRenderWindowInteractor*)caller)->ExitCallback();
	}
	else if (key == '+' || key == '-') { // change sizes of spheres, etc.
		double dblCoef = key == '+' ? 2.0 : 0.5;

		pThis->m_TotalResizeCoef *= dblCoef;
		bool bHasChange = false;

		VisualObjectsMap::const_iterator it;
		for (it =	pThis->m_VisualObjects.begin(); it != pThis->m_VisualObjects.end(); it++)
		{
			vtkSphereSource* sphere = vtkSphereSource::SafeDownCast(it->second->pResizableSource);
			if (sphere != NULL) {
				sphere->SetRadius(sphere->GetRadius()*dblCoef); bHasChange = true;
			}

			vtkTubeFilter* tube = vtkTubeFilter::SafeDownCast(it->second->pResizableSource);
			if (tube != NULL) {
				tube->SetRadius(tube->GetRadius()*dblCoef); bHasChange = true;
			}
		}

		if (bHasChange)
		{
			vtkRenderWindow* win = ((vtkRenderWindowInteractor*)caller)->GetRenderWindow();

			win->Modified();
			win->Render();
		}
	}
	else if (key == 's' || (key >= '0' && key <= '9') || key == char(13)) {	//skip some few steps
		if (pThis->m_nSkipIndex < 0 && key != 's')
			return;	//ignore these keys because they come from invalid context

		if (key == 's')	//we have start of entry
			pThis->m_nSkipIndex = 0;
		else if (key != (char)13)
		{
			//some character => let us get the next character, but avoid buffer overrun
			if (pThis->m_nSkipIndex < (sizeof(pThis->m_szSkip)/sizeof(char)-1))
			{
				pThis->m_szSkip[pThis->m_nSkipIndex] = key;
				pThis->m_szSkip[++pThis->m_nSkipIndex] = '\0';
			}
		}
		else
		{
			//enter
			if (pThis->m_nSkipIndex != 0){
				pThis->m_NumberOfIgnoredSteps = atoi(pThis->m_szSkip);
			}

			pThis->m_nSkipIndex = -1;	//close entering
			pThis->m_NavigationState = (FrameNavigation)pThis->m_NumberOfIgnoredSteps;

			pThis->RemoveLabel("__SKIPFRAMES");
			((vtkRenderWindowInteractor*)caller)->ExitCallback();
			return;	//we are finish here
		}

		//OK, so display updated info for the user
		char szMessage[256];
		strcpy(szMessage, "Current number of frames to be skipped is ");

		char szVal[16];
		if (pThis->m_nSkipIndex == 0)
			sprintf(szVal, "%d", pThis->m_NumberOfIgnoredSteps);
		else
			strcpy(szVal, pThis->m_szSkip);

		strcat(szMessage, szVal);
		strcat(szMessage, ".\nPress <ENTER> to confirm or insert a new value.");

		pThis->AddOrUpdateLabel("__SKIPFRAMES", szMessage, 24,
			1.0, 0, 0,	//RED
			0, 0,				//Center position
			0, 0);			//centered in x and y

		vtkRenderWindow* win = ((vtkRenderWindowInteractor*)caller)->GetRenderWindow();

		win->Modified();
		win->Render();
	}
}