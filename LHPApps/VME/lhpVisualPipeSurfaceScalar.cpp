/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVisualPipeSurfaceScalar.cpp,v $
  Language:  C++
  Date:      $Date: 2011-12-07 15:46:40 $
  Version:   $Revision: 1.1.1.1.2.2 $
  Authors:   Paolo Quadrani
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpVisualPipeSurfaceScalar.h"
#include "mafSceneNode.h"
#include "mafGUI.h"

#include "mmaMaterial.h"

#include "mafEventSource.h"
#include "mafVME.h"
#include "mafVMEOutputSurface.h"
#include "lhpVMESurfaceScalarVarying.h"

#include "vtkMAFAssembly.h"
#include "vtkMAFSmartPointer.h"
#include "vtkLookupTable.h"
#include "vtkRenderer.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpVisualPipeSurfaceScalar);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpVisualPipeSurfaceScalar::lhpVisualPipeSurfaceScalar()
:mafPipe()
//----------------------------------------------------------------------------
{
  m_Actor           = NULL;
  m_OutlineActor    = NULL;
}
//----------------------------------------------------------------------------
void lhpVisualPipeSurfaceScalar::Create(mafSceneNode *n)
//----------------------------------------------------------------------------
{
  Superclass::Create(n);
  
  m_Selected = false;
  m_Actor           = NULL;
  m_OutlineActor    = NULL;

  mafVMEOutputSurface *output_surface = mafVMEOutputSurface::SafeDownCast(m_Vme->GetOutput());
  assert(output_surface);
  output_surface->Update();

  m_Vme->GetEventSource()->AddObserver(this);

  vtkPolyData *data = output_surface->GetSurfaceData();
  data->Update();

  double sr[2];
  vtkDataArray *scalar = data->GetPointData()->GetScalars();
  //scalar->GetRange(sr);

  m_Material = output_surface->GetMaterial();
  m_Material->m_MaterialType = mmaMaterial::USE_LOOKUPTABLE;
  m_Material->m_ColorLut->GetTableRange(sr);

  int immediate = m_Vme->IsAnimated() ? 1 : 0;
	vtkNEW(m_Mapper);
  m_Mapper->SetInput(data);
	m_Mapper->SetImmediateModeRendering(immediate);
  m_Mapper->ScalarVisibilityOn();
  m_Mapper->SetScalarModeToUsePointData();  
  m_Mapper->SetLookupTable((vtkScalarsToColors *)m_Material->m_ColorLut);
  m_Mapper->UseLookupTableScalarRangeOn();
  //m_Mapper->SetColorModeToMapScalars();
  //m_Mapper->SetScalarRange(sr);

  m_Actor = vtkActor::New();
	m_Actor->SetMapper(m_Mapper);

  m_AssemblyFront->AddPart(m_Actor);

  // selection highlight
  vtkMAFSmartPointer<vtkOutlineCornerFilter> corner;
	corner->SetInput(data);  

  vtkMAFSmartPointer<vtkPolyDataMapper> corner_mapper;
	corner_mapper->SetInput(corner->GetOutput());

  vtkMAFSmartPointer<vtkProperty> corner_props;
	corner_props->SetColor(1,1,1);
	corner_props->SetAmbient(1);
	corner_props->SetRepresentationToWireframe();
	corner_props->SetInterpolationToFlat();

	m_OutlineActor = vtkActor::New();
	m_OutlineActor->SetMapper(corner_mapper);
	m_OutlineActor->VisibilityOff();
	m_OutlineActor->PickableOff();
	m_OutlineActor->SetProperty(corner_props);

  m_AssemblyFront->AddPart(m_OutlineActor);
}
//----------------------------------------------------------------------------
lhpVisualPipeSurfaceScalar::~lhpVisualPipeSurfaceScalar()
//----------------------------------------------------------------------------
{
  m_Vme->GetEventSource()->RemoveObserver(this);

  m_AssemblyFront->RemovePart(m_Actor);
  m_AssemblyFront->RemovePart(m_OutlineActor);

  vtkDEL(m_Mapper);
  vtkDEL(m_Actor);
  vtkDEL(m_OutlineActor);
}
//----------------------------------------------------------------------------
void lhpVisualPipeSurfaceScalar::Select(bool sel)
//----------------------------------------------------------------------------
{
	m_Selected = sel;
	if(m_Actor->GetVisibility()) 
	{
    m_OutlineActor->SetVisibility(sel);
	}
}
//-------------------------------------------------------------------------
mafGUI* lhpVisualPipeSurfaceScalar::CreateGui()
//-------------------------------------------------------------------------
{
  assert(m_Gui == NULL);
  m_Gui = mafPipe::CreateGui();
  m_Gui->Lut(ID_LUT, "lut", m_Material->m_ColorLut);
  return m_Gui;
}
//-------------------------------------------------------------------------
void lhpVisualPipeSurfaceScalar::OnEvent(mafEventBase *maf_event)
//-------------------------------------------------------------------------
{
  mafEvent *e = mafEvent::SafeDownCast(maf_event);
  if (e != NULL)
  {
    switch(e->GetId())
    {
      case ID_LUT:
        m_Material->UpdateFromLut();
      break;
      default:
        Superclass::OnEvent(maf_event);
    }
  }
  else if (maf_event->GetSender() == m_Vme)
  {
    if(maf_event->GetId() == VME_OUTPUT_DATA_UPDATE)
    {
      UpdateProperty();
    }
  }
}
//----------------------------------------------------------------------------
void lhpVisualPipeSurfaceScalar::UpdateProperty(bool fromTag)
//----------------------------------------------------------------------------
{
  lhpVMESurfaceScalarVarying *vme = lhpVMESurfaceScalarVarying::SafeDownCast(m_Vme);
  mafVMEOutputSurface *output_surface = mafVMEOutputSurface::SafeDownCast(m_Vme->GetOutput());
  output_surface->Update();
  vtkPolyData *data = output_surface->GetSurfaceData();
  data->Update();

  if (vme->GetNumberOfScalarData() == 1)
  {
    double rgb[3], v;
    int scalar_index;
    scalar_index = vme->GetSurfaceScalarIndexes(0)->GetId(0);
    mafLogMessage(">>> scalar index is %d", scalar_index);
    vtkDoubleArray *scalars = (vtkDoubleArray *)data->GetPointData()->GetScalars();
    v = scalars->GetValue(scalar_index);
    m_Mapper->ScalarVisibilityOff();
    m_Mapper->UseLookupTableScalarRangeOff();
    m_Material->m_ColorLut->GetColor(v,rgb);
    m_Actor->GetProperty()->SetColor(rgb);
  }
  else
  {
    m_Mapper->ScalarVisibilityOn();
    m_Mapper->UseLookupTableScalarRangeOn();
  }
  /*double sr[2];
  vtkDataArray *scalar = data->GetPointData()->GetScalars();
  scalar->GetRange(sr);
  m_Mapper->SetScalarRange(sr);*/
  m_Mapper->Update();

  //mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}
