/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: vtkMAFVisualDebugger.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.0 $
Authors:   Josef Kohout,  Jana Hájková
==========================================================================
Copyright (c) 2012 University of University of West Bohemia
See the COPYINGS file for license details 
=========================================================================
Simple helper class for Visual debugging of methods.
=========================================================================*/

#ifndef __vtkMAFVisualDebugger_h
#define __vtkMAFVisualDebugger_h

#include "vtkObject.h"
#include "vtkLHPConfigure.h"

#include <map>

//forward declarations
class vtkPolyData;
class vtkPoints;
class vtkRenderer;
class vtkProp;

/** Use this class to display an interactive rendering window that allows you 
to show the current data using various visualization techniques without 
necessity to implement complex visual pipes (these pipes are created by this class)*/
class VTK_vtkLHP_EXPORT vtkMAFVisualDebugger : public vtkObject
{
public:
  vtkTypeMacro(vtkMAFVisualDebugger, vtkObject);

protected:
	typedef enum FrameNavigation {
		StopDebugging = -1,	//debugger is to be disabled, further calls of DebugStep are ignored		
		NextStep = 0,				//each step is displayed		
		///positive values 1, ... denote how may calls (of the same part) should be ignored
	};

	FrameNavigation m_NavigationState;		///<Current  navigation state
	int m_NumberOfIgnoredSteps;						///<number of steps to be ignored
	int m_CurrentFrameId;									///<Id of the current frame

	int WindowLeft;											///<position of window
	int WindowTop;											///<	position of window
	int WindowWidth;										///<	size of window
	int WindowHeight;										///<	size of window

	double BackgroundColor[3];					///<Background color

	vtkRenderer* m_Renderer;							///<renderer

	typedef struct VISUAL_OBJECT
	{
		const static int MAX_ACTORS = 4;

		vtkObject* pResizableSource;				///<item having resizable values 
		vtkProp* pActors[MAX_ACTORS];				///<list of actors

		VISUAL_OBJECT() {
			pResizableSource = NULL;
			for (int i = 0; i < MAX_ACTORS; i++) {
				pActors[i] = NULL;
			}
		}
	};

	typedef std::map< const char*, VISUAL_OBJECT*  > VisualObjectsMap;
	VisualObjectsMap m_VisualObjects;	///<list of objects to be visualized
	
	double m_TotalResizeCoef;				///<coefficient for resizing

	///<variables from GUI to skip some frames
	int m_nSkipIndex;					///<next index to m_szSkip where character should be placed -1, if m_NumberOfIgnoredSteps
	char m_szSkip[16];				///<entered number of frames to be skipped

protected:
	/** Constructor */
	vtkMAFVisualDebugger();

	/** Destructor */
	virtual ~vtkMAFVisualDebugger();
	
public:
	/** Creates an instance of the object */
	static vtkMAFVisualDebugger *New();

		/** Gets the current position of the renderer window*/
	vtkGetMacro(WindowLeft, int);

	/** Gets the current position of the renderer window*/
	vtkGetMacro(WindowTop, int);

	/** Gets the current size of the renderer window*/
	vtkGetMacro(WindowWidth, int);

	/** Gets the current size of the renderer window*/
	vtkGetMacro(WindowHeight, int);

	/** Sets the new position of the renderer window*/
	vtkSetMacro(WindowLeft, int);

	/** Sets the new position of the renderer window*/
	vtkSetMacro(WindowTop, int);

	/** Sets the new size of the renderer window*/
	vtkSetMacro(WindowWidth, int);

	/** Sets the new size of the renderer window*/
	vtkSetMacro(WindowHeight, int);

	/** Gets the current background color*/
	vtkGetVector3Macro(BackgroundColor, double);

	/** Sets the new background color*/
	vtkSetVector3Macro(BackgroundColor, double);

	/** Opens the rendering window displaying everything and interacting with the user.
	The method ends when the user expresses his/her wishes to continue with the method being debugged.	*/
	virtual void DebugStep();

	/** Adds some label with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	The label is displayed using default font family and the given font size
	in (r, g, b) colour at position x and y. Position is relative to the specified
	horizontal and vertical alignment (parameters horiz_align and vert_align).
	Valid values of alignment are -1, 0, 1 denoting LEFT, CENTER, RIGHT
	or TOP, CENTER, BOTTOM alignment. */
	virtual void AddOrUpdateLabel(const char* Id,
		const char* label, int fontsize, 
		double r, double g, double b,
		double x = 0.01, double y = 0.01,
		int horiz_align = -1, int vert_align = 1);

	/** Adds scalar field with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	Legend is displayed, if displaybar is true, in which case legend is shown
	at x, y position relative to the window size and values of horiz_align and
	vert_align - see also AddOrUpdateLabel method.*/
	virtual void AddOrUpdateScalarField(const char* Id,
		vtkPolyData* poly, bool displaybar = false, 
		double x = 0.1, double y = 0.01,	
		int horiz_align = -1, int vert_align = -1);

	/** Adds surface mesh with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	(R,G,B) is colour in which the mesh is to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.*/
	virtual void AddOrUpdateSurface(const char* Id,
		vtkPolyData* poly, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false);

	/** Adds points with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	Radius = radius of spheres to be rendered for the given points,
	(R,G,B) is colour in which spheres are to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
	If bCubeglyph is true, small cubes are rendered instead of spheres. */
	virtual void AddOrUpdatePoints(const char* Id, 
		vtkPolyData* points, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);
	
	/** Adds points with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	Radius = radius of spheres to be rendered for the given points,
	(R,G,B) is colour in which spheres are to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
	If bCubeglyph is true, small cubes are rendered instead of spheres.*/
	virtual void AddOrUpdatePoints(const char* Id, 
		vtkPoints* points, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);

	/** Adds points with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	Points = array of points in format (x1,y1,z1, x2, y2, z2, ...)
	Count = number of points in points array
	Radius = radius of spheres to be rendered for the given points,
	(R,G,B) is colour in which spheres are to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.
	If bCubeglyph is true, small cubes are rendered instead of spheres.*/
	virtual void AddOrUpdatePoints(const char* Id, 
		const double* points, int count, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);

	/** Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
	only the points having their indices in validIndices array containing  validIndicesCount entries.
	Note: validIndices can be easily retrieved from vtkIdList. */
	virtual void AddOrUpdateSelectedPoints(const char* Id, 
		vtkPolyData* points, const vtkIdType* validIndices,
		int validIndicesCount, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);
	
	/** Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
	only the points having their indices in validIndices array containing  validIndicesCount entries.
	Note: validIndices can be easily retrieved from vtkIdList. */
	virtual void AddOrUpdateSelectedPoints(const char* Id, 
		vtkPoints* points,  const vtkIdType* validIndices,
		int validIndicesCount, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);

	/** Equivalent to AddOrUpdatePoints method with a small difference: this method visualize
	only the points having their indices in validIndices array containing  validIndicesCount entries.
	Note: validIndices can be easily retrieved from vtkIdList. */
	virtual void AddOrUpdateSelectedPoints(const char* Id, 
		const double* points, int count,  const vtkIdType* validIndices,
		int validIndicesCount, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false, bool bcubeglyph = false);

	/** Adds lines with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	Radius = radius of tubes to be rendered for the given poly-lines,
	(R,G,B) is colour in which tubes are to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.*/
	virtual void AddOrUpdateLines(const char* Id,
		vtkPolyData* polylines, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false);

	/** Adds lines with the given Id into the scene. 
	If an object with the same Id already exists, it is updated.
	PolyLine contains the connectivity of the poly-line, polyLineCount is number of entries.
	Radius = radius of tubes to be rendered for the given poly-lines,
	(R,G,B) is colour in which tubes are to be rendered.
	Opacity is allows transparency, wireframe set to true enables rendering of wire frame model.*/
	virtual void AddOrUpdateLines(const char* Id,
		vtkPoints* points, const vtkIdType* polyLine,
		int polyLineCount, double radius, 
		double r, double g, double b,
		double opacity = 1.0,	bool wireframe = false);

	/** Adds external actor with the given Id into the scene. 
	If an object with the same Id already exists, it is updated. */
	virtual void AddOrUpdateExternalObject(const char* Id, vtkProp* actor);
	

	/** Removes label with the given Id from the scene. */
	inline void RemoveLabel(const char* Id) {
		RemoveVisualObject(Id);
	}

	
	/** Removes the scalar field with the given Id from the scene. */
	inline void RemoveScalarField(const char* Id) {
		RemoveVisualObject(Id);
	}

	/** Removes surface with the given Id from the scene. */
	inline void RemoveSurface(const char* Id) {
		RemoveVisualObject(Id);
	}

	/** Removes points with the given Id from the scene. */
	inline void RemovePoints(const char* Id) {
		RemoveVisualObject(Id);
	}

	/** Removes points with the given Id from the scene. */
	inline void RemoveSelectedPoints(const char* Id) {
		RemoveVisualObject(Id);
	}


	/** Removes lines with the given Id from the scene. */
	inline void RemoveLines(const char* Id) {
		RemoveVisualObject(Id);
	}

	/** Removes the external object with the given Id from the scene. */
	inline void RemoveExternalObject(const char* Id) {
		RemoveVisualObject(Id);
	}

	/** Removes all objects from the scene. */
	virtual void RemoveAll();

protected:
	/** Adds the new visual object into the scene or updates it, if the object already exists in the scene. 
	N.B. obj must be allocated on the heap and will be removed automatically when 
	corresponding RemoveVisualObject or	RemoveAll is called. */
	virtual void AddOrUpdateVisualObject(const char* Id, VISUAL_OBJECT* obj);

	/** Removes the visual object from the scene. */
	virtual void RemoveVisualObject(const char* Id);

private:
		/** Callback for visualization */
	static void KeypressCallback(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) );

private:
  vtkMAFVisualDebugger(const vtkMAFVisualDebugger&);  // Not implemented.
  void operator = (const vtkMAFVisualDebugger&);  // Not implemented.  
};


#endif //__vtkMAFVisualDebugger_h