/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVMESurfaceScalarVarying.cpp,v $
  Language:  C++
  Date:      $Date: 2011-12-09 08:28:49 $
  Version:   $Revision: 1.1.1.1.2.3 $
  Authors:   Paolo Quadrani
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpVMESurfaceScalarVarying.h"
#include "mafGUI.h"
#include "mafGUICheckListBox.h"

#include "mafInteractorPicker.h"

#include "mmaMaterial.h"

#include "mafIndent.h"
#include "mafTagArray.h"
#include "mafTagItem.h"

#include "mafEventSource.h"
#include "mafTransform.h"
#include "mafTransformFrame.h"
#include "mafStorageElement.h"
#include "mafDataPipeCustom.h"
#include "mafVMEOutputSurface.h"
#include "mafTagItem.h"
#include "mafTagArray.h"
#include "mafMatrix.h"
#include "mafAbsMatrixPipe.h"
#include "mafVMEOutputScalarMatrix.h"

#include "vtkMAFSmartPointer.h"
#include "vtkDataSet.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkLookupTable.h"
#include "vtkIdList.h"
#include "vtkPointLocator.h"
#include "vtkTransform.h"

#include <vnl/vnl_matrix.h>

//-------------------------------------------------------------------------
mafCxxTypeMacro(lhpVMESurfaceScalarVarying)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
lhpVMESurfaceScalarVarying::lhpVMESurfaceScalarVarying()
//-------------------------------------------------------------------------
{
  m_SuggestUser = true;
  m_PickedPoint = NULL;
  m_OldBehavior = NULL;
  mafNEW(m_PickScalar);
  m_PickScalar->SetListener(this);

  m_ScalarTimeStamps.clear();

  mafNEW(m_Transform);
  mafVMEOutputSurface *output = mafVMEOutputSurface::New(); // an output with no data
  output->SetTransform(m_Transform); // force my transform in the output
  SetOutput(output);

  DependsOnLinkedNodeOn();

  m_Locator = NULL;

  vtkNEW(m_PolyData);
  m_SurfaceScalars = NULL;

  m_ScalarRange[0] = m_ScalarRange[1] = 0.0;

  m_EditMode = 0;
  m_CurrentTimeIndex = 0;
  m_Radius = 10;
  m_ScalarMin = 0.0;

  // attach a data pipe which creates a bridge between VTK and MAF
  mafDataPipeCustom *dpipe = mafDataPipeCustom::New();
  dpipe->SetDependOnAbsPose(true);
  SetDataPipe(dpipe);
  dpipe->SetInput(m_PolyData);
}

//-------------------------------------------------------------------------
lhpVMESurfaceScalarVarying::~lhpVMESurfaceScalarVarying()
//-------------------------------------------------------------------------
{
  mafDEL(m_PickScalar);
  vtkDEL(m_PolyData);
  vtkDEL(m_Locator);

  // these links are children, thus it's not our responsibility to
  // destroy them, it's part of the vtkTree one's
  mafDEL(m_Transform);
  SetOutput(NULL);
}

//-------------------------------------------------------------------------
mmaMaterial *lhpVMESurfaceScalarVarying::GetMaterial()
//-------------------------------------------------------------------------
{
  mmaMaterial *material = (mmaMaterial *)GetAttribute("MaterialAttributes");
  if (material == NULL)
  {
    material = mmaMaterial::New();
    SetAttribute("MaterialAttributes", material);
  }
  return material;
}

//-------------------------------------------------------------------------
int lhpVMESurfaceScalarVarying::InternalInitialize()
//-------------------------------------------------------------------------
{
  if (Superclass::InternalInitialize()==MAF_OK)
  {
    // force material allocation
    GetMaterial();
    return MAF_OK;
  }
  return MAF_ERROR;
}
//-----------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::InternalPreUpdate()
//-----------------------------------------------------------------------
{
  mafVME *surf = mafVME::SafeDownCast(GetSurfaceLink());
  medVMEAnalog *scalar = medVMEAnalog::SafeDownCast(GetScalarLink());
  
  if (surf != NULL && scalar != NULL)
  {
    if (m_ScalarTimeStamps.size() == 0)
    {
      scalar->GetLocalTimeStamps(m_ScalarTimeStamps);
      UpdateTimeIndex(GetTimeStamp());
    }
    UpdateSurface();
  }
}
//-------------------------------------------------------------------------
std::vector<mafTimeStamp>::iterator lhpVMESurfaceScalarVarying::FindNearestItem(mafTimeStamp t)
//-------------------------------------------------------------------------
{
  std::pair<std::vector<mafTimeStamp>::iterator, std::vector<mafTimeStamp>::iterator> range = std::equal_range(m_ScalarTimeStamps.begin(), m_ScalarTimeStamps.end(), t);
  if (range.first != m_ScalarTimeStamps.end())
  {
    if (range.second != m_ScalarTimeStamps.end())
    {
      if (fabs(*range.first - t)>fabs(*range.second - t))
        return range.second;
    }    
    return range.first;
  }
  else if (range.second != m_ScalarTimeStamps.end())
  {
    return range.second;
  }

  return --range.second;
}
//-----------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::InternalUpdate()
//-----------------------------------------------------------------------
{
  vtkDataArray *scalars = NULL;
  scalars = m_PolyData->GetPointData()->GetScalars();
  vtkPolyData *p = vtkPolyData::SafeDownCast(GetOutput()->GetVTKData());
  p->Update();
  if (scalars)
  {
    p->GetPointData()->SetScalars(scalars);
  }
  m_EventSource->InvokeEvent(this, VME_OUTPUT_DATA_UPDATE);
}
//-------------------------------------------------------------------------
int lhpVMESurfaceScalarVarying::DeepCopy(mafNode *a)
//-------------------------------------------------------------------------
{ 
  if (Superclass::DeepCopy(a) == MAF_OK)
  {
    lhpVMESurfaceScalarVarying *scalarvarying = lhpVMESurfaceScalarVarying::SafeDownCast(a);
    mafVME *surface_linked_node = mafVME::SafeDownCast(scalarvarying->GetSurfaceLink());
    if (surface_linked_node)
    {
      SetSurfaceLink(surface_linked_node);
    }
    mafVME *scalar_linked_node = mafVME::SafeDownCast(scalarvarying->GetScalarLink());
    if (scalar_linked_node)
    {
      SetScalarLink(scalar_linked_node);
    }
    m_Transform->SetMatrix(scalarvarying->m_Transform->GetMatrix());
    m_SurfaceName = scalarvarying->m_SurfaceName;
    m_Radius = scalarvarying->m_Radius;
    m_SurfaceScalars = vtkDoubleArray::SafeDownCast(m_PolyData->GetPointData()->GetScalars());
    for (int s = 0; s < scalarvarying->GetNumberOfScalarData(); s++)
    {
      SetScalarIDs(scalarvarying->GetScalarVMEIndex(s), scalarvarying->GetSurfaceScalarIndexes(s));
    }
    return MAF_OK;
  }  
  return MAF_ERROR;
}
//-------------------------------------------------------------------------
bool lhpVMESurfaceScalarVarying::Equals(mafVME *vme)
//-------------------------------------------------------------------------
{
  bool ret = false;
  if (Superclass::Equals(vme))
  {
    ret = m_Transform->GetMatrix() == ((lhpVMESurfaceScalarVarying *)vme)->m_Transform->GetMatrix() && \
          m_Radius == ((lhpVMESurfaceScalarVarying *)vme)->m_Radius && \
          m_SurfaceName == ((lhpVMESurfaceScalarVarying *)vme)->m_SurfaceName;
  }
  return ret;
}

//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::SetMatrix(const mafMatrix &mat)
//-------------------------------------------------------------------------
{
  m_Transform->SetMatrix(mat);
  Modified();
}

//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes)
//-------------------------------------------------------------------------
{
  kframes.clear(); // one timestamp
  kframes.push_back(m_Transform->GetTimeStamp());
}

//-------------------------------------------------------------------------
bool lhpVMESurfaceScalarVarying::IsDataAvailable()
//-------------------------------------------------------------------------
{
  mafVME *surf = mafVME::SafeDownCast(GetSurfaceLink());
  return (surf && surf->IsDataAvailable());
}
//-----------------------------------------------------------------------
mafNode *lhpVMESurfaceScalarVarying::GetScalarLink()
//-----------------------------------------------------------------------
{
  return GetLink("Scalar");
}
//-----------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::SetScalarLink(mafNode *scalar)
//-----------------------------------------------------------------------
{
  medVMEAnalog *scalar_link = medVMEAnalog::SafeDownCast(scalar);

  if (scalar_link != NULL && scalar_link != GetScalarLink())
  {
    m_ScalarRegionMap.clear();
    vnl_matrix<double> mat = scalar_link->GetScalarOutput()->GetScalarData();
    m_ScalarMin = mat.min_value();
    m_ScalarName = scalar_link ? scalar_link->GetName() : _("none");
    scalar_link->GetLocalTimeStamps(m_ScalarTimeStamps);
    SetLink("Scalar", scalar);
    Modified();
  }

  FillScalarsName();
}
//-----------------------------------------------------------------------
mafNode *lhpVMESurfaceScalarVarying::GetSurfaceLink()
//-----------------------------------------------------------------------
{
  return GetLink("Surface");
}
//-----------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::SetSurfaceLink(mafNode *surface)
//-----------------------------------------------------------------------
{
  mafVME *surf_link = mafVME::SafeDownCast(surface);
  
  if (surf_link != NULL && surf_link != GetSurfaceLink())
  {
    SetLink("Surface", surface);
    m_ScalarRegionMap.clear();
    FillScalarsName();

    vtkPolyData *polydata = vtkPolyData::SafeDownCast(surf_link->GetOutput()->GetVTKData());
    if (polydata != NULL)
    {
      polydata->Update();
      m_PolyData->DeepCopy(polydata);
      m_SurfaceName = surf_link ? surf_link->GetName() : _("none");
      if (m_Gui)
      {
        m_Gui->Update();
      }
      //m_PolyData->Update();
      vtkPointData *pd = m_PolyData->GetPointData();
      vtkDataArray *link_scalars = pd->GetScalars();
      m_SurfaceScalars = vtkDoubleArray::SafeDownCast(link_scalars);
      if (m_SurfaceScalars == NULL)
      {
        InitScalars();
      }
      mafDataPipeCustom *dpipe = mafDataPipeCustom::SafeDownCast(GetDataPipe());
      if (dpipe)
      {
        dpipe->SetInput(m_PolyData);
      }
      Modified();
    }
  }
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::InitScalars()
//-------------------------------------------------------------------------
{
  mafVME *surf_link = mafVME::SafeDownCast(GetSurfaceLink());
  medVMEAnalog *scalar = medVMEAnalog::SafeDownCast(GetScalarLink());
  if (scalar != NULL)
  {
    mafVMEOutputScalarMatrix *output = scalar->GetScalarOutput();
    vnl_matrix<double> mat = output->GetScalarData();
    m_ScalarMin = mat.min_value();
  }
  vtkPolyData *polydata = vtkPolyData::SafeDownCast(surf_link->GetOutput()->GetVTKData());
  polydata->Update();
  m_PolyData->DeepCopy(polydata);

  m_Locator = vtkPointLocator::New();
  m_Locator->SetDataSet(m_PolyData);
  m_Locator->BuildLocator();

  if (m_Gui != NULL)
  {
    m_Gui->Enable(ID_RADIUS, m_Locator != NULL);
  }

  m_SurfaceScalars = vtkDoubleArray::New();
  m_SurfaceScalars->SetNumberOfComponents(1);
  m_SurfaceScalars->SetNumberOfValues(m_PolyData->GetNumberOfPoints());
  m_SurfaceScalars->FillComponent(0, m_ScalarMin);
  m_PolyData->GetPointData()->SetScalars(m_SurfaceScalars);
  m_PolyData->Modified();
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::UpdateSurface()
//-------------------------------------------------------------------------
{
  // Update scalar values at current timestamp into the vtkPolyData according to the linked Scalar VMEs
  medVMEAnalog *scalar = medVMEAnalog::SafeDownCast(GetScalarLink());
  if (m_CurrentTimeIndex != -1 && scalar != NULL)
  {
    vtkDataArray *sca = m_PolyData->GetPointData()->GetScalars();
    if (m_SurfaceScalars == NULL)
    {
      InitScalars();
    }
    mafVMEOutputScalarMatrix *output = scalar->GetScalarOutput();
    vnl_matrix<double> mat = output->GetScalarData();

    double val;
    SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
    for (; it != m_ScalarRegionMap.end(); it++)
    {
      val = mat.get(it->first, m_CurrentTimeIndex);
      for (int s = 0; s < it->second->GetNumberOfIds(); s++)
      {
        m_SurfaceScalars->SetValue(it->second->GetId(s), (val + m_SurfaceScalars->GetValue(it->second->GetId(s)))/2); //modified the result scalar in order to calculate a simple mean between 2 signals
      }
    }
    double sr[2];
    m_SurfaceScalars->Modified();
    m_SurfaceScalars->GetRange(sr);
    m_PolyData->Update();
    m_PolyData->GetScalarRange(sr);
  }
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::MarkRegion(int vertex_id, int radius, vtkIdList &Ids)
//-------------------------------------------------------------------------
{
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::SetTimeStamp(mafTimeStamp t)
//-------------------------------------------------------------------------
{
  Superclass::SetTimeStamp(t);
  UpdateTimeIndex(t);
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::UpdateTimeIndex(mafTimeStamp t)
//-------------------------------------------------------------------------
{
  if (m_ScalarTimeStamps.size() > 0)
  {
    std::vector<mafTimeStamp>::iterator it = FindNearestItem(t);
    m_CurrentTimeIndex = it != m_ScalarTimeStamps.end() ? std::distance(m_ScalarTimeStamps.begin(), it) : -1;
  }
  else
  {
    m_CurrentTimeIndex = 0;
  }
}
//-----------------------------------------------------------------------
int lhpVMESurfaceScalarVarying::InternalStore(mafStorageElement *parent)
//-----------------------------------------------------------------------
{  
  if (Superclass::InternalStore(parent) == MAF_OK)
  {
    if (parent->StoreMatrix("Transform",&m_Transform->GetMatrix()) == MAF_OK &&
        parent->StoreInteger("Radius", m_Radius) == MAF_OK &&
        parent->StoreInteger("NumOfScalarVMEIndexes", m_ScalarRegionMap.size()) == MAF_OK)
    {
      SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
      int *indexIds, num_indexes;
      mafString indexesName, numIndexesName;
      for (int n = 0; it != m_ScalarRegionMap.end(); it++, n++)
      {
        numIndexesName = "NumOfScalarVMEIndexes";
        indexesName = "ScalarVMEIndexes";
        indexesName << n;
        numIndexesName << n;
        num_indexes = it->second->GetNumberOfIds() + 1;
        indexIds = new int[num_indexes];
        indexIds[0] = it->first;
        for (int i = 1; i < num_indexes; i++)
        {
          indexIds[i] = it->second->GetId(i-1);
        }
        if (parent->StoreInteger(numIndexesName.GetCStr(), num_indexes)      == MAF_ERROR ||
            parent->StoreVectorN(indexesName.GetCStr(),indexIds,num_indexes) == MAF_ERROR)
        {
          delete indexIds;
          return MAF_ERROR;
        }
        delete indexIds;
      }
      return MAF_OK;
    }
  }
  return MAF_ERROR;
}
//-----------------------------------------------------------------------
int lhpVMESurfaceScalarVarying::InternalRestore(mafStorageElement *node)
//-----------------------------------------------------------------------
{
  if (Superclass::InternalRestore(node)==MAF_OK)
  {
    mafMatrix matrix;
    if (node->RestoreMatrix("Transform",&matrix) == MAF_OK)
    {
      m_Transform->SetMatrix(matrix);
      if (node->RestoreInteger("Radius", m_Radius) == MAF_OK)
      {
        int num = 0;
        if (node->RestoreInteger("NumOfScalarVMEIndexes", num) == MAF_OK)
        {
          int *indexIds, num_indexes;
          mafString indexesName, numIndexesName;
          for (int n = 0; n < num; n++)
          {
            numIndexesName = "NumOfScalarVMEIndexes";
            indexesName = "ScalarVMEIndexes";
            indexesName << n;
            numIndexesName << n;
            if(node->RestoreInteger(numIndexesName.GetCStr(), num_indexes) == MAF_OK)
            {
              indexIds = new int[num_indexes];
              if (node->RestoreVectorN(indexesName.GetCStr(),indexIds,num_indexes) == MAF_OK)
              {
                m_ScalarRegionMap[indexIds[0]] = vtkIdList::New();
                for (int i = 1; i < num_indexes; i++)
                {
                  m_ScalarRegionMap[indexIds[0]]->InsertNextId(indexIds[i]);
                }
              }
              delete indexIds;
            }
          }
          return MAF_OK;
        }
      }
    }
  }
  return MAF_ERROR;
}
//-------------------------------------------------------------------------
int lhpVMESurfaceScalarVarying::GetScalarVMEIndex(int idx)
//-------------------------------------------------------------------------
{
  SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
  std::advance(it, idx);
  return it->first;
}
//-------------------------------------------------------------------------
vtkIdList *lhpVMESurfaceScalarVarying::GetSurfaceScalarIndexes(int idx)
//-------------------------------------------------------------------------
{
  SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
  std::advance(it, idx);
  return it->second;
}
//-------------------------------------------------------------------------
mafGUI* lhpVMESurfaceScalarVarying::CreateGui()
//-------------------------------------------------------------------------
{
  m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
  m_Gui->SetListener(this);
  m_Gui->Divider();
  mafVME *surf = mafVME::SafeDownCast(GetSurfaceLink());
  m_SurfaceName = surf ? surf->GetName() : _("none");
  mafVME *scalar = mafVME::SafeDownCast(GetScalarLink());
  m_ScalarName = scalar ? scalar->GetName() : _("none");
  m_Gui->Button(ID_SURFACE_LINK,&m_SurfaceName,_("surface"), _("Select the surface to be colored by scalar values."));
  m_Gui->Button(ID_SCALAR_LINK,&m_ScalarName,_("scalar"), _("Select the Analog VME to be used to color the surface."));
  // update the linked scalars if there are some.
  m_Gui->Bool(ID_EDIT_SCALAR_POSITION,_("Edit scalar"),&m_EditMode, 1);
  m_Gui->Label("available scalars:", true);
  m_ScalarsAvailableList = m_Gui->CheckList(ID_LIST_SCALARS_AVAILABLES, "", 100);
  m_Gui->Integer(ID_RADIUS, _("radius"), &m_Radius, 1);
  m_Gui->Divider();
  FillScalarsName(false);
  SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
  for (; it != m_ScalarRegionMap.end(); it++)
  {
    m_ScalarsAvailableList->CheckItem(it->first - 1, true);
  }
  m_Gui->Enable(ID_LIST_SCALARS_AVAILABLES, m_EditMode != 0);
  m_Gui->Enable(ID_RADIUS, m_Locator != NULL);
  m_Gui->Update();
  return m_Gui;
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::OnEvent(mafEventBase *maf_event)
//-------------------------------------------------------------------------
{
  // events to be sent up or down in the tree are simply forwarded
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case ID_SURFACE_LINK:
      {
        mafString title = _("Choose surface");
        e->SetId(VME_CHOOSE);
        e->SetArg((long)&lhpVMESurfaceScalarVarying::OutputSurfaceAccept);
        e->SetString(&title);
        ForwardUpEvent(e);
        mafVME *n = mafVME::SafeDownCast(e->GetVme());
        if (n != NULL)
        {
          SetSurfaceLink(n);
          m_Gui->Update();
        }
        e->SetArg(0);
        e->SetId(CAMERA_UPDATE);
        ForwardUpEvent(e);
      }
      break;
      case ID_SCALAR_LINK:
        {
          mafString title = _("Choose analog");
          e->SetId(VME_CHOOSE);
          e->SetArg((long)&lhpVMESurfaceScalarVarying::VmeScalarAccept);
          e->SetString(&title);
          ForwardUpEvent(e);
          mafVME *n = mafVME::SafeDownCast(e->GetVme());
          if (n != NULL)
          {
            SetScalarLink(n);
          }
        }
      break;
      case ID_RADIUS:
      break;
      case VME_PICKED:
        {
          // Added by Losi to avoid crash picking on the surface without selected scalars
          int number_of_selected_scalars = 0;
          for(int s = 0; s < m_ScalarsAvailableList->GetNumberOfItems(); s++)
          {
            if(m_ScalarsAvailableList->IsItemChecked(s))
            {
              number_of_selected_scalars++;
            }
          }
          if(number_of_selected_scalars > 0)
          {
            m_PickedPoint = vtkPoints::SafeDownCast(e->GetVtkObj());
            int idx = e->GetArg();
            if (m_PickedPoint)
            {
              double pos[3];
              m_PickedPoint->GetPoint(0,pos);            
              vtkIdList *idList = vtkIdList::New();

              // added by Losi to fix bug #1812
              // must transform the located point with the inverse transform represented by the vme absolute matrix

              vtkTransform *inverse_abs_pose = vtkTransform::New();
              inverse_abs_pose->SetMatrix(this->GetOutput()->GetAbsMatrix()->GetVTKMatrix());
              inverse_abs_pose->Inverse();
              double *new_pos = inverse_abs_pose->TransformDoublePoint(pos);
              pos[0] = new_pos[0];
              pos[1] = new_pos[1];
              pos[2] = new_pos[2];
              inverse_abs_pose->Delete();
              m_Locator->FindPointsWithinRadius(m_Radius, pos, idList);
              SetScalarIDs(m_ActiveScalarVMEIndex, idList);
              vtkDEL(idList);
            }
            e->SetId(CAMERA_UPDATE);
            ForwardUpEvent(e);
          }
          else
          {
            mafMessage(_("Check the signal from the listbox then pick on the linked surface to position the scalar."));
          }
        }
      break;
      case ID_EDIT_SCALAR_POSITION:
        m_Gui->Enable(ID_LIST_SCALARS_AVAILABLES,m_EditMode != 0);
        m_Gui->Update();
        if (m_EditMode != 0)
        {
          e->SetId(OP_RUN_STARTING);
          ForwardUpEvent(e);
          m_OldBehavior = GetBehavior();
          SetBehavior(m_PickScalar);
          if (m_SuggestUser)
          {
            mafMessage(_("Check the signal from the listbox then pick on the linked surface to position the scalar."));
            m_SuggestUser = false;
          }
        }
        else
        {
          e->SetId(OP_RUN_TERMINATED);
          ForwardUpEvent(e);
          SetBehavior(m_OldBehavior);
        }
      break;
      case ID_LIST_SCALARS_AVAILABLES:
        if (m_ScalarsAvailableList->IsCheckEvent())
        {
          m_ActiveScalarVMEIndex = e->GetArg();
          if (!e->GetBool())
          {
            int row_index = m_ActiveScalarVMEIndex + 1; // skip the first row that is referred to the time,
            SurfaceScalarRegionMap::iterator it = m_ScalarRegionMap.begin();
            for (; it != m_ScalarRegionMap.end(); it++)
            {
              if (it->first == row_index)
              {
                break;
              }
            }
            if (it != m_ScalarRegionMap.end())
            {
              for (int s=0; s < it->second->GetNumberOfIds(); s++)
              {
                m_SurfaceScalars->SetValue(it->second->GetId(s), m_ScalarMin);
              }

              m_ScalarRegionMap[it->first]->Delete();
              m_ScalarRegionMap.erase(it);
              UpdateSurface();
            }
          }
        }
      break;
      default:
        mafNode::OnEvent(maf_event);
    }
  }
  else
  {
    Superclass::OnEvent(maf_event);
  }
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::SetScalarIDs(int analog_scalar_index, vtkIdList *surface_scalar_idx)
//-------------------------------------------------------------------------
{
  m_ScalarRegionMap[analog_scalar_index + 1] = vtkIdList::New();
  m_ScalarRegionMap[analog_scalar_index + 1]->DeepCopy(surface_scalar_idx);
  Modified();
}
//-------------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::FillScalarsName(bool new_scalars)
//-------------------------------------------------------------------------
{
  medVMEAnalog *scalar = medVMEAnalog::SafeDownCast(GetScalarLink());
  if (scalar != NULL && scalar->GetTagArray()->IsTagPresent("SIGNALS_NAME"))
  {
    m_ScalarsAvailableList->Clear();
    mafTagItem *tag_Signals = scalar->GetTagArray()->GetTag("SIGNALS_NAME");
    for (int s = 0; s < tag_Signals->GetNumberOfComponents(); s++)
    {
      m_ScalarsAvailableList->AddItem(s,tag_Signals->GetValue(s),false);
    }
    if (new_scalars)
    {
      mafVMEOutputScalarMatrix *output = scalar->GetScalarOutput();
      vnl_matrix<double> mat = output->GetScalarData();
      vnl_matrix<double> mat_scalars = mat.get_n_rows(1, mat.rows()-1);

      m_ScalarRange[0] = mat_scalars.min_value();
      m_ScalarRange[1] = mat_scalars.max_value();
      mmaMaterial *material = GetMaterial();
      material->m_ColorLut->SetTableRange(m_ScalarRange);
      material->m_ColorLut->Build();
    }
  }
  if (m_Gui != NULL)
  {
    m_Gui->Update();
  }
}

//-------------------------------------------------------------------------
char** lhpVMESurfaceScalarVarying::GetIcon() 
//-------------------------------------------------------------------------
{
#include "mafVMESurface.xpm"
  return mafVMESurface_xpm;
}
//-----------------------------------------------------------------------
void lhpVMESurfaceScalarVarying::Print(std::ostream& os, const int tabs)
//-----------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
  mafIndent indent(tabs);

  mafMatrix m = m_Transform->GetMatrix();
  m.Print(os,indent.GetNextIndent());

  os << "\n";
  os << indent << "Scalar Range: [" << m_ScalarRange[0] << ", " << m_ScalarRange[1] << "]" << "\n";
  os << indent << "Linked scalars: \n";
  for (int s = 0; s < m_ScalarsAvailableList->GetNumberOfItems(); s++)
  {
    if (m_ScalarsAvailableList->IsItemChecked(s))
    {
      os << indent << m_ScalarsAvailableList->GetItemLabel(s) << "\n";
    }
  }
  os << std::endl;
}
