/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeSurfacePlot.cpp,v $
  Language:  C++
  Date:      $Date: 2010-09-10 15:45:52 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time_Array error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

//#include <vnl/vnl_vector.h>

#include <math.h>

#include "lhpPipeSurfacePlot.h"
#include "mafDecl.h"
#include "mafSceneNode.h"
#include "mafGUI.h"
#include "mafGUICheckListBox.h"

#include "mafVMEScalarMatrix.h"
#include "mafTagArray.h"
#include "mafTagItem.h"
#include "mafVMEOutputScalar.h"
#include "mafVMEOutputScalarMatrix.h"
#include "mafEventSource.h"

#include "vtkTextProperty.h"
#include "vtkDoubleArray.h"
#include "vtkXYPlotActor.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkMAFSmartPointer.h"
#include "mafString.h"


#include <vtkStructuredGrid.h>
#include <vtkGeometryFilter.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAxes.h>
#include <vtkTubeFilter.h>
#include <vtkAxisActor2D.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkUnsignedCharArray.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper2D.h>


#include <list>
#include <sstream>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpPipeSurfacePlot);
//----------------------------------------------------------------------------

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

double lhpPipeSurfacePlot::m_scale[3] = { 5000, 300, 5 };
unsigned char lhpPipeSurfacePlot::colorList[6][3] = { 
		{255, 0, 0},
		{0, 255, 0},
		{0, 0, 255},
		{255, 255, 0},
		{255, 0, 255},
		{0, 255, 255}
	};

//----------------------------------------------------------------------------
lhpPipeSurfacePlot::lhpPipeSurfacePlot()
:mafPipe()
//----------------------------------------------------------------------------
{
	// ID
	m_SelectIDIK = 0;
	// Surface
	m_RenderMode = 0;

	m_InputPoly = NULL;
	m_InputFields = NULL;

	m_Surf = NULL;
	m_Lines = NULL;
	m_AxisX = NULL;
	m_AxisZ = NULL;

	m_SurfActor = NULL;
	m_LineActor = NULL;
	m_AxisXActor = NULL;
	m_AxisZActor = NULL;

	m_AxisXTitleActor = NULL;
}
//----------------------------------------------------------------------------
lhpPipeSurfacePlot::~lhpPipeSurfacePlot()
//----------------------------------------------------------------------------
{
	RemoveAll();
	m_Vme->GetEventSource()->RemoveObserver(this);

	m_VarNamesIDIK[0].clear();
	m_VarNamesIDIK[1].clear();

	mafEventMacro(mafEvent(this,PIPE_PLOT_DESTROY));
}

/*
void lhpPipeSurfacePlot::ReadTestFile(char * fname)
{
	
	std::ifstream inputfile;
	inputfile.open(fname, std::ifstream::in);
	if (!inputfile)
	{
		throw std::exception("Open file failed");
	}

	std::string line;
	std::string header;

	 while (!inputfile.eof())
	 {
		  std::getline(inputfile, line);

		  if (line.find("nRows=") != std::string::npos)
		  {
			  std::string sub = line.substr(6,line.length() - 6);
			  m_nRow = atoi(sub.c_str());
		  }
		  else if (line.find("nColumns=") != std::string::npos)
		  {
			  std::string sub = line.substr(9,line.length() - 9);
			  m_nCol = atoi(sub.c_str());
		  }
		  else if (line=="endheader")
		  {
			   std::getline(inputfile, header);
			   break;
		  }
	  }

	 mat = new double[m_nRow * m_nCol];
	 double d;
	 // read matrix
	 for (int i = 0; i < m_nRow; i++) 
	{
		 for (int j = 0; j < m_nCol; j++ )
		 {
			  inputfile >> d;
			  mat[ i * m_nCol + j ] = d;
		 }
		 //mafLogMessage("%d ", i);
	 }
	 //mafLogMessage("\n");

	inputfile.close();
	
}
*/
// read field data
void lhpPipeSurfacePlot::GenSurf()
{
	std::list<int> list = m_DataList[m_SelectIDIK];
	mafGUICheckListBox * pCheckList = m_CheckList[m_SelectIDIK];

	int nRows = 0; 
	int nCols = 0;
	int arraySize = 0;
	double * mat;
	double d;
	double minVal = 1e8;
	double maxVal = -1e8;

	vtkDoubleArray * pArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* list.begin()));	
	nRows = pArray->GetSize();
	//nCols = list.size();
	
	for (int i = 0; i < list.size() - 1; i++)
	{
		if (pCheckList->IsItemChecked(i))
			nCols++;
	}

	nCols ++;

	mat = new double[nCols * nRows];

	std::list<int>::iterator it;
	int j = 0;
	int varIdx = -1;
	for (it = list.begin(); it != list.end(); it++, j++) 
	{
		if ( ( j > 0) && (!pCheckList->IsItemChecked(j - 1)))
			continue;
		else 
			varIdx++;

		vtkDoubleArray * pArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* it));
		arraySize = pArray->GetSize();
		assert(arraySize == nRows);

		for (int i = 0; i < nRows; i++)
		{
			d = pArray->GetValue(i);
			mat[i * nCols + varIdx] = d;
			minVal = min(d, minVal);
			maxVal = max(d, maxVal);
		}
	}

	  // temp
	  //ReadTestFile("D:/proj/NMS/infovis/test/1_ID.sto");

	  int dims[3]={nRows,nCols - 1,1};
	  int nPoints = dims[0] * dims[1];
	  //grid
	  vtkMAFSmartPointer<vtkStructuredGrid> rect_grid; // = vtkStructuredGrid::New();
	  rect_grid->SetDimensions(dims);

	  vtkMAFSmartPointer<vtkPoints> points;
	  points->Allocate(nPoints);

	  int offset;
	  double v[3];

	  minVal = 1e8;
	  maxVal = -1e8;
	  //double scale[3] = { 2000, 100, 1 };

	  for (int j = 0; j < nCols - 1; j++)
		  for (int i = 0; i < nRows; i++)
		  {
			  v[0] = mat[i * nCols] * m_scale[0];
			  v[1] = j * m_scale[1];
			  v[2] = mat[i * nCols + j + 1] * m_scale[2];
			  minVal = min(v[2], minVal);
			  maxVal = max(v[2], maxVal);
			  //v[2] = 0;
		  
			  offset = j * nRows  + i;
			  points->InsertPoint(offset, v);
		  }

		range[0] = mat[0] * m_scale[0];
		range[1] = mat[(nRows-1) * nCols] * m_scale[0];
		range[2] = 0;
		range[3] = (nCols - 1 ) * m_scale[1];
		range[4] = minVal;
		range[5] = maxVal;
		rect_grid->SetPoints(points);

	  delete [] mat;

	  vtkMAFSmartPointer<vtkGeometryFilter> geometryFilter;  // = vtkGeometryFilter::New();

	  geometryFilter->SetInput(rect_grid);

	  geometryFilter->Update(); 
	  vtkNEW(m_Surf);
	  m_Surf->DeepCopy(vtkPolyData::SafeDownCast (geometryFilter->GetOutput() ) );

}


void lhpPipeSurfacePlot::ShowSurf()
{
	vtkMAFSmartPointer<vtkPolyDataMapper > m_SurfMapper;
	m_SurfMapper->SetInput(m_Surf);
	m_SurfActor = vtkActor::New();

	m_SurfActor->SetMapper(m_SurfMapper);
	m_SurfActor->GetProperty()->SetColor(1,1,0.8);
	m_SurfActor->GetProperty()->SetPointSize(10);
	//m_SurfActor->GetProperty()->SetRepresentationToWireframe ();
	m_RenFront->AddActor(m_SurfActor);

	if (m_RenderMode == 0)
		m_SurfActor->GetProperty()->SetRepresentationToSurface ();
	else if (m_RenderMode == 1)
		m_SurfActor->GetProperty()->SetRepresentationToWireframe ();
	else if (m_RenderMode == 2)
		m_SurfActor->GetProperty()->SetRepresentationToPoints ();
}

void lhpPipeSurfacePlot::RemoveSurf()
{
	m_RenFront->RemoveActor(m_SurfActor);
	vtkDEL(m_Surf);
	vtkDEL(m_SurfActor);
}

// read field data
void lhpPipeSurfacePlot::GenLines()
{
	std::list<int> list = m_DataList[m_SelectIDIK];
	mafGUICheckListBox * pCheckList = m_CheckList[m_SelectIDIK];

	int nRows = 0; 
	int nCols = 0;
	int arraySize = 0;
	//double * mat;
	double d;
	double minVal = 1e8;
	double maxVal = -1e8;
	
	// Setup two colors - one for each line
	vtkDoubleArray * pTimeArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* list.begin()));	
	nRows = pTimeArray->GetSize();
	//nCols = list.size();

	vtkMAFSmartPointer<vtkCellArray> lines; // = vtkCellArray::New();
	int nLines = lines->GetNumberOfCells();

	vtkMAFSmartPointer<vtkPoints> pts; // = vtkPoints::New();
	vtkMAFSmartPointer<vtkUnsignedCharArray> colors; // = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");

	double p[3];
	int idx = 0;
	vtkDoubleArray * pArray;

	std::list<int>::iterator it;
	// skip the time variable
	it = list.begin();
	it ++;
	int j = 0;
	int varIdx = -1;
	for (; it != list.end(); it++, j++) 
	{	
		if ( !pCheckList->IsItemChecked(j))
			continue;
		else 
			varIdx++;

		pArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* it));
		arraySize = pArray->GetSize();
		assert(arraySize == nRows);

		// Create the first line (between Origin and P0)
		vtkMAFSmartPointer<vtkLine> line; // = vtkLine::New();
		line->GetPointIds()->SetNumberOfIds(nRows);
		colors->InsertNextValue(colorList[(varIdx+1) % COLOR_NUM][0]);
		colors->InsertNextValue(colorList[(varIdx+1) % COLOR_NUM][1]);
		colors->InsertNextValue(colorList[(varIdx+1) % COLOR_NUM][2]);
		
		int nColor = colors->GetNumberOfTuples();
		
		p[1] = varIdx * m_scale[1];

		for (int i = 0; i < nRows; i++)
		{
			d = pArray->GetValue(i);
			p[0] = pTimeArray->GetValue(i) * m_scale[0];
			p[2] = d * m_scale[2];

			//int nPts = line->GetPointIds()->GetNumberOfIds();
			pts->InsertNextPoint(p);
			line->GetPointIds()->SetId(i, idx);
			idx++;

		}
		
		lines->InsertNextCell(line);
	}

	m_Lines = vtkPolyData::New();
	m_Lines->SetPoints(pts);
	m_Lines->SetLines(lines);
 
	m_Lines->GetCellData()->SetScalars(colors);

	//vtkDEL(pTimeArray);

}


void lhpPipeSurfacePlot::ShowLines()
{
	vtkMAFSmartPointer<vtkPolyDataMapper> m_LineMapper;
	m_LineMapper->SetInput(m_Lines);
	m_LineActor =  vtkActor::New();
	m_LineActor->SetMapper(m_LineMapper);
	m_LineActor->GetProperty()->SetLineWidth(3);

	m_RenFront->AddActor(m_LineActor);
}

void lhpPipeSurfacePlot::RemoveLines() 
{
	m_RenFront->RemoveActor(m_LineActor);
	vtkDEL(m_LineActor);
	vtkDEL(m_Lines);
}

void lhpPipeSurfacePlot::GenAxisY() 
{
	std::list<int> list = m_DataList[m_SelectIDIK];
	mafGUICheckListBox * pCheckList = m_CheckList[m_SelectIDIK];
	std::vector<std::string> varNameList = m_VarNamesIDIK[m_SelectIDIK];

	vtkDoubleArray * pTimeArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* list.begin()));	

	std::vector<std::string> varNames;
	
	for (int j = 0; j < varNameList.size(); j++) 
	{
		if ( pCheckList->IsItemChecked(j) )
			varNames.push_back(varNameList[j]);
	}

	// add variable texts
	std::string label;
	for (int i = 0; i < varNames.size(); i++) {
		vtkMAFSmartPointer<vtkVectorText> vLabel; // = vtkVectorText::New();

		vLabel->SetText(varNames[i].c_str());
		vtkMAFSmartPointer<vtkPolyDataMapper> textMapper; // = vtkPolyDataMapper::New();
		textMapper->SetInput( vLabel->GetOutput() );
	  
		vtkFollower * textActor = NULL; // = vtkFollower::New();
		vtkNEW(textActor);
		textActor->SetMapper(textMapper);
		textActor->SetScale(50, 50, 50);
		textActor->GetProperty()->SetColor( colorList[(i + 1) % COLOR_NUM][0], 
							colorList[(i + 1) % COLOR_NUM][1], 
							colorList[(i + 1) % COLOR_NUM][2]); 
		textActor->SetPosition(pTimeArray->GetValue(0) * m_scale[0] - 1500, i * m_scale[1], 0);
		textActor->RotateY(0.5 * PI);
		//textActor->SetCamera(m_RenFront->GetActiveCamera());
		m_AxisYLabelList.push_back(textActor);
		//m_RenFront->AddActor(textActor);
	}

	//vtkDEL(pTimeArray);
}

void lhpPipeSurfacePlot::ShowAxisY() 
{
	for (int i = 0; i < m_AxisYLabelList.size(); i ++)
	{
		m_RenFront->AddActor(m_AxisYLabelList[i]);
	}
}

void lhpPipeSurfacePlot::RemoveAxisY() 
{
	for (int i = 0; i < m_AxisYLabelList.size(); i ++)
	{
		m_RenFront->RemoveActor(m_AxisYLabelList[i]);
		vtkDEL(m_AxisYLabelList[i]);
	}
	m_AxisYLabelList.clear();
}

void lhpPipeSurfacePlot::GenAxisZ() 
{
	std::list<int> list = m_DataList[m_SelectIDIK];
	vtkDoubleArray * pTimeArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* list.begin()));	

	vtkMAFSmartPointer<vtkCellArray> lines; // = vtkCellArray::New();
	int nLines = lines->GetNumberOfCells();

	vtkMAFSmartPointer<vtkPoints> pts; // = vtkPoints::New();
	vtkMAFSmartPointer<vtkUnsignedCharArray> colors; // = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");
	
	double lenZ = (range[5] - range[4]) / m_scale[2];
	double minZ, maxZ;
	double tickLen = ( range[1] - range[0] ) / 50.0;

	int nTicks = 20;
	double step = lenZ / nTicks;
	int nDigit = log10(step);
	int iStep = step / pow(10.0, nDigit);

	if (iStep < 5)
		step = 5 * pow(10.0, nDigit);
	else 
		step = pow(10.0, nDigit + 1);

	minZ = floor( range[4]/ m_scale[2] / step )  * step;
	maxZ = ceil( range[5]/ m_scale[2] / step )  * step;

	nTicks = (maxZ - minZ ) / step;

	double p[3];
	int idx = 0;
	vtkMAFSmartPointer<vtkVectorText> vLabel;
	vtkMAFSmartPointer<vtkPolyDataMapper> textMapper;
	char labelString[100];
	double tickZ;
	for (int i = 0; i < nTicks; i++)
	{
		colors->InsertNextValue(255);
		colors->InsertNextValue(255);
		colors->InsertNextValue(255);

		vtkMAFSmartPointer<vtkLine> line; // = vtkLine::New();
		line->GetPointIds()->SetNumberOfIds(2);

		p[0] = pTimeArray->GetValue(0) * m_scale[0];
		p[1] = 0;
		tickZ = minZ + i * step;
		p[2] = tickZ * m_scale[2];

		//int nPts = line->GetPointIds()->GetNumberOfIds();
		pts->InsertNextPoint(p);
		line->GetPointIds()->SetId(0, idx);
		idx++;
		p[0] -= tickLen;
		pts->InsertNextPoint(p);
		line->GetPointIds()->SetId(1, idx);
		idx++;
		lines->InsertNextCell(line);

		sprintf(labelString, "%6.2f", tickZ);
		vtkMAFSmartPointer<vtkVectorText> vLabel;
		vLabel->SetText(labelString);
		vtkMAFSmartPointer<vtkPolyDataMapper> textMapper;
		textMapper->SetInput( vLabel->GetOutput() );
	  
		vtkFollower * labelActor; // = vtkFollower::New();
		vtkNEW(labelActor);
		labelActor->SetMapper(textMapper);
		labelActor->SetScale(50, 50, 50);
		labelActor->GetProperty()->SetColor(1, 1, 1); 
		labelActor->SetPosition( p[0] - 300, 0 , p[2]);
		labelActor->SetCamera(m_RenFront->GetActiveCamera());

		m_AxisZLabelList.push_back(labelActor);
	}
	
	colors->InsertNextValue(colorList[2 % COLOR_NUM][0]);
	colors->InsertNextValue(colorList[2 % COLOR_NUM][1]);
	colors->InsertNextValue(colorList[2 % COLOR_NUM][2]);
	// axis
	vtkMAFSmartPointer<vtkLine> line; // = vtkLine::New();
	line->GetPointIds()->SetNumberOfIds(2);
	line->GetPointIds()->SetId(0, 0);
	line->GetPointIds()->SetId(1, (nTicks -1 ) * 2);
	lines->InsertNextCell(line);
	
	m_AxisZ = vtkPolyData::New();
	m_AxisZ->SetPoints(pts);
	m_AxisZ->SetLines(lines);
	m_AxisZ->GetCellData()->SetScalars(colors);

	//vtkDEL(pTimeArray);
}

void lhpPipeSurfacePlot::ShowAxisZ() 
{
	vtkMAFSmartPointer<vtkPolyDataMapper > m_AxisZMapper;
	m_AxisZMapper->SetInput(m_AxisZ);
	vtkNEW(m_AxisZActor);
	m_AxisZActor->SetMapper(m_AxisZMapper);
	m_AxisZActor->GetProperty()->SetLineWidth(3);
	m_RenFront->AddActor(m_AxisZActor);

	for (int i = 0; i < m_AxisZLabelList.size(); i++ )
	{
		m_RenFront->AddActor(m_AxisZLabelList[i]);
	}
}

void lhpPipeSurfacePlot::RemoveAxisZ() 
{
	m_RenFront->RemoveActor(m_AxisZActor);
	vtkDEL(m_AxisZActor);
	vtkDEL(m_AxisZ);
	for (int i = 0; i < m_AxisZLabelList.size(); i++) 
	{
		m_RenFront->RemoveActor(m_AxisZLabelList[i]);
		vtkDEL(m_AxisZLabelList[i]);
	}
	m_AxisZLabelList.clear();
}

void lhpPipeSurfacePlot::GenAxisX() 
{
	std::list<int> list = m_DataList[m_SelectIDIK];
	vtkDoubleArray * pTimeArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(* list.begin()));

	// add X axis
	char labelString[100];

	//vtkMAFSmartPointer<vtkVectorText> vLabel = vtkVectorText::New();
	vtkMAFSmartPointer<vtkVectorText> vLabel;
	vLabel->SetText("Time");
	vtkMAFSmartPointer<vtkPolyDataMapper> textMapper;
	textMapper->SetInput( vLabel->GetOutput() );
	  
	//vtkMAFSmartPointer<vtkFollower> textActor = vtkFollower::New();
	m_AxisXTitleActor = vtkFollower::New();
	//vtkMAFSmartPointer<vtkFollower> textActor;
	m_AxisXTitleActor->SetMapper(textMapper);
	m_AxisXTitleActor->SetScale(100, 100, 100);
	m_AxisXTitleActor->GetProperty()->SetColor(1, 1, 1); 
	m_AxisXTitleActor->SetPosition( (range[0] + range[1])/2 , -m_scale[1] , 0);
	//textActor->SetCamera(m_RenFront->GetActiveCamera());
	//m_AxisXTitleActor = textActor;
	
	vtkMAFSmartPointer<vtkCellArray> lines; // = vtkCellArray::New();
	int nLines = lines->GetNumberOfCells();

	vtkMAFSmartPointer<vtkPoints> pts; // = vtkPoints::New();
	vtkMAFSmartPointer<vtkUnsignedCharArray> colors; // = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");
	
	
	double lenX = (range[1] - range[0]) / m_scale[0];
	
	int nVal = pTimeArray->GetSize();
	int nTicks = nVal;
	double tickLen = 0.2 * m_scale[1];
	double step = ( range[1] - range[0] ) / nTicks;

	double minVal, maxVal;
	minVal = pTimeArray->GetValue(0);
	maxVal = pTimeArray->GetValue(nVal - 1);

	double p[3];
	int idx = 0;
	//vtkMAFSmartPointer<vtkVectorText> vLabel;
	//vtkMAFSmartPointer<vtkPolyDataMapper> textMapper;
	//char labelString[100];
	double tick;
	for (int i = 0; i < nTicks; i++)
	{
		if (i % 10 != 0)
			continue;

		colors->InsertNextValue(255);
		colors->InsertNextValue(255);
		colors->InsertNextValue(255);

		vtkMAFSmartPointer<vtkLine> line; // = vtkLine::New();
		line->GetPointIds()->SetNumberOfIds(2);

		tick = pTimeArray->GetValue(i);
		p[0] = pTimeArray->GetValue(i) * m_scale[0];
		p[1] = 0;
		p[2] = 0;

		pts->InsertNextPoint(p);
		line->GetPointIds()->SetId(0, idx);
		idx++;
		p[1] -= tickLen;
		pts->InsertNextPoint(p);
		line->GetPointIds()->SetId(1, idx);
		idx++;
		lines->InsertNextCell(line);

		sprintf(labelString, "%6.3f", tick);
		vtkMAFSmartPointer<vtkVectorText> vLabel;
		vLabel->SetText(labelString);
		vtkMAFSmartPointer<vtkPolyDataMapper> textMapper;
		textMapper->SetInput( vLabel->GetOutput() );
	  
		vtkFollower * labelActor; // = vtkFollower::New();
		vtkNEW(labelActor);
		labelActor->SetMapper(textMapper);
		labelActor->SetScale(50, 50, 50);
		labelActor->GetProperty()->SetColor(1, 1, 1); 
		labelActor->SetPosition( p[0], -tickLen - 100 , p[2]);
		labelActor->SetCamera(m_RenFront->GetActiveCamera());

		m_AxisXLabelList.push_back(labelActor);
	}
	
	colors->InsertNextValue(colorList[0 % COLOR_NUM][0]);
	colors->InsertNextValue(colorList[0 % COLOR_NUM][1]);
	colors->InsertNextValue(colorList[0 % COLOR_NUM][2]);
	// axis
	vtkMAFSmartPointer<vtkLine> line; // = vtkLine::New();
	line->GetPointIds()->SetNumberOfIds(2);
	line->GetPointIds()->SetId(0, 0);
	line->GetPointIds()->SetId(1, idx - 2);
	lines->InsertNextCell(line);
	

	m_AxisX = vtkPolyData::New();
	m_AxisX->SetPoints(pts);
	m_AxisX->SetLines(lines);

	m_AxisX->GetCellData()->SetScalars(colors);
}

void lhpPipeSurfacePlot::ShowAxisX() 
{
	m_RenFront->AddActor(m_AxisXTitleActor);

	vtkMAFSmartPointer<vtkPolyDataMapper > m_AxisXMapper;
	m_AxisXMapper->SetInput(m_AxisX);
	vtkNEW(m_AxisXActor);
	m_AxisXActor->SetMapper(m_AxisXMapper);
	m_AxisXActor->GetProperty()->SetLineWidth(3);
	m_RenFront->AddActor(m_AxisXActor);

	for (int i = 0; i < m_AxisXLabelList.size(); i++ )
	{
		m_RenFront->AddActor(m_AxisXLabelList[i]);
	}
}

void lhpPipeSurfacePlot::RemoveAxisX()  
{
	m_RenFront->RemoveActor(m_AxisXTitleActor);
	m_RenFront->RemoveActor(m_AxisXActor);
	vtkDEL(m_AxisXTitleActor);
	vtkDEL(m_AxisXActor);
	vtkDEL(m_AxisX);

	for (int i = 0; i < m_AxisXLabelList.size(); i++ )
	{
		m_RenFront->RemoveActor(m_AxisXLabelList[i]);
		vtkDEL(m_AxisXLabelList[i]);
	}
	m_AxisXLabelList.clear();
}



void lhpPipeSurfacePlot::GenAll()
{
	GenSurf();
	GenLines();
	GenAxisX();
	GenAxisY();
	GenAxisZ();

	// for debugging
	int n1 = m_Surf-> GetNumberOfPoints();
	int n2 = m_Surf-> GetNumberOfPolys();
}

void lhpPipeSurfacePlot::ShowAll()
{
	ShowSurf();
	ShowLines();
	ShowAxisX();
	ShowAxisY();
	ShowAxisZ();

	m_RenFront->ResetCamera(range);
}


void lhpPipeSurfacePlot::RemoveAll()
{

	RemoveSurf();
	RemoveLines();

	RemoveAxisX();
	RemoveAxisY();
	RemoveAxisZ();
}

void lhpPipeSurfacePlot::OnChangeDataType(int select) { 
	//if (m_SelectIDIK != select)
	//{
		SetDataType(select); 
		UpdatePlot();
	//}
}

void lhpPipeSurfacePlot::OnChangeRenderMode(int renderMode)
{
	SetRenderMode( renderMode );
	if (m_RenderMode == 0)
		m_SurfActor->GetProperty()->SetRepresentationToSurface ();
	else if (m_RenderMode == 1)
		m_SurfActor->GetProperty()->SetRepresentationToWireframe ();
	else if (m_RenderMode == 2)
		m_SurfActor->GetProperty()->SetRepresentationToPoints ();

	//UpdatePlot();
}	

void lhpPipeSurfacePlot::OnChangeAxisScale(float scaleX, float scaleY, float scaleZ)
{
	SetAxisScale(scaleX, scaleY, scaleZ);
	UpdatePlot();
}

void lhpPipeSurfacePlot::UpdatePlot()
{
	/*
	if (m_DataList[m_SelectIDIK].empty() ) 
	{
		//wxMessageBox("ID or IK list empty");
		//return;
	}
	*/

	RemoveAll();

	int nVars = 0;
	std::list<int> list = m_DataList[m_SelectIDIK];
	mafGUICheckListBox * pCheckList = m_CheckList[m_SelectIDIK];

	for (int i = 0; i < list.size() - 1; i++)
	{
		if (pCheckList->IsItemChecked(i))
			nVars++;
	}

	if (nVars <= 0) {
		wxMessageBox("No selected variables");
		return;
	}


	if (! m_DataList[m_SelectIDIK].empty() ) 
	{
		GenAll();
		ShowAll();
	}
	else {
		if (m_SelectIDIK == 0)
			wxMessageBox("ID list empty");
		else if  (m_SelectIDIK == 1)
			wxMessageBox("IK list empty");
	}


	//mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}
//----------------------------------------------------------------------------
void lhpPipeSurfacePlot::Create(mafSceneNode *n)
//----------------------------------------------------------------------------
{
  Superclass::Create(n);

  if(m_Vme->GetEventSource() != NULL)
    m_Vme->GetEventSource()->AddObserver(this);

   m_InputPoly = vtkPolyData::SafeDownCast(m_Vme->GetOutput()->GetVTKData());
   m_InputFields = m_InputPoly->GetFieldData();
   int nFields = m_InputFields->GetNumberOfArrays();
   vtkDoubleArray * pArray;
	
	const char * name;
	std::string strname;
	std::string label;
	std::vector<std::string> varNamesID;
	std::vector<std::string> varNamesIK;
	int pos;
	for (int i = 0; i < nFields; i++)
	{
		pArray = vtkDoubleArray::SafeDownCast(m_InputFields->GetArray(i));
		name = pArray->GetName();
		if (strstr(name, "OpenSimIK") != NULL) {
				if (strstr(name, "time") != NULL)
					m_listIK.push_front(i);
				else {
					m_listIK.push_back(i);

					strname = std::string(pArray->GetName());
					pos = strname.find_first_of('_');
					label = strname.substr(pos + 1);

					varNamesIK.push_back(label);
				}
		}
		else if (strstr(name, "OpenSimID") != NULL) {
			if (strstr(name, "time") != NULL)
				m_listID.push_front(i);
			else {
				m_listID.push_back(i);

				strname = std::string(pArray->GetName());
				pos = strname.find_first_of('_');
				label = strname.substr(pos + 1);
				varNamesID.push_back(label);
			}
		}
	}

	m_DataList.push_back(m_listID);
	m_DataList.push_back(m_listIK);
	m_VarNamesIDIK.push_back(varNamesID);
	m_VarNamesIDIK.push_back(varNamesIK);

	//int n1 = m_listID.size();
    //int n2 = m_listIK.size();

	if ( (m_listID.size() > 1)  && (m_listIK.size() > 1) )
		m_SelectIDIK = 0;
	else if ( (m_listIK.size() <= 1)  && (m_listIK.size() > 1) )
		m_SelectIDIK = 1;

	CreateGui();


}



//----------------------------------------------------------------------------
mafGUI* lhpPipeSurfacePlot::CreateGui()
//----------------------------------------------------------------------------
{
	if(m_Gui == NULL) 
	m_Gui = new mafGUI(this);

	//radio button for selecting Inverse Dynamics or Inverse Kinetics
	wxString IDIK_Choices[] = { _("Dynamics"),  _("Kinetics")  };

	if ( (m_listID.size() > 1)  && (m_listIK.size() > 1) )
	{
		m_Gui->Radio(ID_SELECT_ID_IK,_(""), & m_SelectIDIK, 2, IDIK_Choices, 2, "Select ID or IK");
	}

	// add checklist of ID variables
	m_Gui->Divider(1);
	m_Gui->Label("Select Dynamics");
	m_CheckList[0] = m_Gui->CheckList(ID_SELECT_ID, "", 18 * m_listID.size(), "Select ID");

	if (!m_VarNamesIDIK[0].empty()) 
	{
		for (int j = 0; j < m_VarNamesIDIK[0].size(); j++) 
		{
			m_CheckList[0]->AddItem(-1, _(m_VarNamesIDIK[0][j].c_str()), true);
		}
		m_Gui->Button(ID_BUTTON_SELECT_ID,"Update","", "");
	}

	// add check list of IK variables
	m_Gui->Divider(1);
	m_Gui->Label("Select Kinetics");
	m_CheckList[1] = m_Gui->CheckList(ID_SELECT_IK, "", 18 * m_listIK.size(), "Select Kinetic Variables");
	
	if ( ! m_VarNamesIDIK[1].empty() ) {

		for (int j = 0; j < m_VarNamesIDIK[1].size(); j++) 
		{
			m_CheckList[1]->AddItem(-1, _(m_VarNamesIDIK[1][j].c_str()), true);
		}
		m_Gui->Button(ID_BUTTON_SELECT_IK,"Update","", "");
	}
	return m_Gui;
	
}

//----------------------------------------------------------------------------
void lhpPipeSurfacePlot::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
	 
    switch(e->GetId())
    {
	case ID_SELECT_ID_IK:
		OnChangeDataType(m_SelectIDIK);
		mafEventMacro(mafEvent(this,CAMERA_UPDATE));
		break;

		/*
	case ID_SELECT_ID:
		mafLogMessage("select ID event");
		break;

	case ID_SELECT_IK:
		mafLogMessage("select IK event");
		break;
		*/

	case ID_BUTTON_SELECT_ID:
		UpdatePlot();
		break;
	case ID_BUTTON_SELECT_IK:
		UpdatePlot();
		break;

	default:
		mafEventMacro(*e);
    }
	
	
    mafEventMacro(*e);
  }
 
}