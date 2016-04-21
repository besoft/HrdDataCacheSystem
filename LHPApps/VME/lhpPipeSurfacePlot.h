/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeSurfacePlot.h,v $
  Language:  C++
  Date:      $Date: 2009-11-03 12:58:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpPipeSurfacePlot_H__
#define __lhpPipeSurfacePlot_H__

#include "mafPipe.h"
#include "mafEvent.h"
#include "lhpVMEDefines.h"
#include "vtkMAFSmartPointer.h"
#include "mafSmartPointer.h"
#include <vtkSmartPointer.h>

#include <list>
//----------------------------------------------------------------------------
// forward refs :
//----------------------------------------------------------------------------
class mafVMEScalarMatrix;
class vtkActor;
class vtkFollower;
class mafGUICheckListBox;
class vtkLegendBoxActor;
class vtkXYPlotActor;
class vtkDoubleArray;
class vtkRectilinearGrid;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkFieldData;

#define PIPE_PLOT_DESTROY -10
#define COLOR_NUM  6
#define PI 3.1415926
//----------------------------------------------------------------------------
// lhpPipeSurfacePlot :
//----------------------------------------------------------------------------
/** 
Visual pipe to visualize graphs of analog signals. */
class LHP_VME_EXPORT lhpPipeSurfacePlot : public mafPipe
{
public:
	mafTypeMacro(lhpPipeSurfacePlot,mafPipe);

	lhpPipeSurfacePlot();
	virtual     ~lhpPipeSurfacePlot ();

	void lhpPipeSurfacePlot::OnEvent(mafEventBase *maf_event); 

	/*virtual*/ void Create(mafSceneNode *n);

  	//void ReadTestFile(char * fname);

	void SetDataType( int select) { m_SelectIDIK = select; };
	void SetRenderMode(int renderMode) { m_RenderMode = renderMode; };
	void SetAxisScale(float scaleX, float scaleY, float scaleZ)
		{ 	m_scale[0] = scaleX; m_scale[1] = scaleY; m_scale[2] = scaleZ; 	};

	void OnChangeDataType(int select);
	void OnChangeRenderMode(int renderMode);
	void OnChangeAxisScale(float scaleX, float scaleY, float scaleZ);

	void UpdatePlot();

	void GenAll();
	void ShowAll();
	void RemoveAll();

	void GenSurf();
	void ShowSurf();
	void RemoveSurf();

	void GenLines();
	void ShowLines();
	void RemoveLines();

	// Time Axis
	void GenAxisX();
	void ShowAxisX();
	void RemoveAxisX();
	// variables
	void GenAxisY();
	void ShowAxisY();
	void RemoveAxisY();
	// Value Axis
	void GenAxisZ();
	void ShowAxisZ();
	void RemoveAxisZ();



protected:

	mafGUI* CreateGui();

	enum PIPE_GRAPH_GUI_WIDGETS
	{
		ID_SELECT_ID_IK = Superclass::ID_LAST,
		ID_SELECT_ID, // Checklist ID
		ID_SELECT_IK, // Checklist IK
		ID_BUTTON_SELECT_ID, // Update ID
		ID_BUTTON_SELECT_IK, // Update IK
		ID_SCALE_X, // not in USE
		ID_SCALE_Y, // not in USE
		ID_SCALE_Z, // not in USE
		ID_LAST
	};

	/** colors used for line visualisation */
	static unsigned char colorList[COLOR_NUM][3];
	/** X, Y, Z scale for plotting */
	static double m_scale[3];
	/** box range of the data */
	double range[6];

	/** selection of the Inverse Dynamics or Inverse Kinetics */
	int m_SelectIDIK;
	/** render mode of surface, wireframe or points */
	int m_RenderMode;
	/** index lists of ID and IK */
	std::vector< std::list<int> > m_DataList;

	/** index list of Inverse Dynamics */
	std::list<int> m_listID;
	/** index list of Inverse Kinetics */
	std::list<int> m_listIK;

	std::vector< std::vector<std::string> > m_VarNamesIDIK;
	std::vector<std::string> m_VarNamesIK;

	/** checkbox list of ID and IK */
	mafGUICheckListBox *  m_CheckList[2];

	/** input polygon data */
	vtkPolyData *  m_InputPoly;
	/** field data in input polygon data */
	vtkFieldData * m_InputFields;

	/** data surface for visualisation */
	vtkPolyData * m_Surf;
	/** data lines for visualisation */
	vtkPolyData * m_Lines;
	/** axis X (time axis) for visualisation */
	vtkPolyData * m_AxisX;
	/** axis Z (value axis) for visualisation */
	vtkPolyData * m_AxisZ;

	/** surface actor for visualisation */
	vtkActor * m_SurfActor;
	/** line actor for visualisation */
	vtkActor * m_LineActor;
	/** axis X (time axis) actor for visualisation */
	vtkActor * m_AxisXActor;
	/** axis Z (value axis) actor for visualisation */
	vtkActor * m_AxisZActor;

	/** axis X (time axis) title actor ("time") for visualisation */
	vtkActor * m_AxisXTitleActor;
	/** label list of the X (time) axis */
	std::vector< vtkFollower * > m_AxisXLabelList;
	/** label list of the Y (variable) axis */
	std::vector< vtkFollower * > m_AxisYLabelList;
	/** label list of the Z (value) axis */
	std::vector< vtkFollower * > m_AxisZLabelList;

private:

};  
#endif // __lhpPipeSurfacePlot_H__
