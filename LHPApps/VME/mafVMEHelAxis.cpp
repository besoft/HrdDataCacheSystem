/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEHelAxis.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "mafVMEHelAxis.h"

#include "mafGUI.h"
#include "mafPlotMath.h"
#include "mafIndent.h"
#include "mmaMaterial.h"
#include "mafTransform.h"

#include "mafTagArray.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMEAFRefSys.h"
#include "mafVMEOutputSurface.h"
#include "mafDataPipeCustom.h"
#include "mafStorageElement.h"

#include "vtkMAFSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkArrowSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkAppendPolyData.h"

#include "vtkPointData.h"
#include "vtkUnsignedCharArray.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafVMEHelAxis)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
static void GetGlobalMatrix(mafVME *vme, mafTimeStamp ts, DiMatrix *pMat)
//----------------------------------------------------------------------------
{
  mafMatrix matrix;

  ///This function can be called with zero this!
  vme->GetOutput()->GetMatrix(matrix, ts);
  mflMatrixToDi(matrix.GetVTKMatrix(), pMat);
  mafVMELandmarkCloud *lmc = mafVMELandmarkCloud::SafeDownCast(vme);
  mafVMEAFRefSys      *afs = NULL;
  if(lmc == NULL)
    return;

  for(int i = 0; i < lmc->GetNumberOfChildren(); i++)
  {
    mafNode *child = vme->GetChild(i);
    if(child->IsA("mafVMEAFRefSys"))
    {
      afs = mafVMEAFRefSys::SafeDownCast(child);
      break;
    }
  }

  if(afs == NULL)
    return;

  afs->GetOutput()->GetAbsMatrix(matrix, ts);
  mflMatrixToDi(matrix.GetVTKMatrix(), pMat);
}


//----------------------------------------------------------------------------
static void GetLocalMatrix(mafVME *vme, mafTimeStamp ts, DiMatrix *pMat)
//----------------------------------------------------------------------------
{
  DiMatrix pmatrix;
  DiMatrix cmatrix;
  DiMatrix pInv;

  DiMatrixIdentity(&pmatrix);
  if(vme->GetParent() != NULL)
    GetGlobalMatrix(vme->GetParent(), ts, &pmatrix);
  GetGlobalMatrix(vme, ts, &cmatrix);
  DiMatrixInvert(&pmatrix, &pInv);
  DiMatrixMultiply(&cmatrix, &pInv, pMat);
}

//----------------------------------------------------------------------------
void mafVMEHelAxis::InternalUpdate()
//----------------------------------------------------------------------------
{
  DiMatrix     mt;
  DiMatrix     mt1;
  DiMatrix     tmp;
  DiMatrix     mShowHelical;
  mafTransform *pTransf;
  vtkMatrix4x4 *mVTK             = NULL;

  double       helicalAxis[3]    = { 0, 0, 0 };
  double       point[3]          = { 0, 0, 0 };
  double       angle             = 0;
  double       translationAmount = 0;

  mafNEW(pTransf);

  GetLocalMatrix(GetParent(), GetTimeStamp(), &mt);
  GetLocalMatrix(GetParent(), GetTimeStamp() + 1, &mt1);
  DiMatrixInvert(&mt, &tmp);
  DiMatrixMultiply(&mt1, &tmp, &mt);
  vtkNEW(mVTK);
  DiMatrixToVTK(&mt, mVTK);
  pTransf->SetMatrix(mVTK);
  if(!DiMatrixTestIdentity(&mt))
  {
    pTransf->MatrixToHelicalAxis(pTransf->GetMatrix(), helicalAxis, point, angle, translationAmount, 2);
  }
  else
  {
    helicalAxis[0] = 1.0;
    helicalAxis[1] = 0.0;
    helicalAxis[2] = 0.0;
    angle          = 0.0;
  }
  if(angle < 0)
  {
    helicalAxis[0] = -helicalAxis[0];
    helicalAxis[1] = -helicalAxis[1];
    helicalAxis[2] = -helicalAxis[2];
    angle          = -angle;
  }
  while(angle > 360.0)
    angle -= 360.0;

  DiMatrixIdentity(&mShowHelical);
  mShowHelical.vAt.x = helicalAxis[0];
  mShowHelical.vAt.y = helicalAxis[1];
  mShowHelical.vAt.z = helicalAxis[2];
  DiMatrixOrtoNormalizeVectSys(&mShowHelical.vAt, &mShowHelical.vUp, &mShowHelical.vRight, TRUE);
  mShowHelical.vAt.w = 0.0f;
  mShowHelical.vUp.w = 0.0f;
  mShowHelical.vRight.w = 0.0f;

  DiMatrixToVTK(&mShowHelical, mVTK);
  m_AngleFactor = angle;
  SetScaleFactor(m_ScaleFactor);
  SetMatrix(mVTK);
  vtkDEL(mVTK);

  mafDEL(pTransf);
}

//----------------------------------------------------------------------------
void mafVMEHelAxis::SetMatrix(const mafMatrix &mat)
//----------------------------------------------------------------------------
{
  m_Transform->SetMatrix(mat);
  Modified();
}

//-------------------------------------------------------------------------
void mafVMEHelAxis::UpdateCS()
//-------------------------------------------------------------------------
{
  GetOutput()->Update();
  mafEvent *e  = new mafEvent(this,CAMERA_UPDATE);
  ForwardUpEvent(e);
  delete e;
}

//-------------------------------------------------------------------------
double mafVMEHelAxis::GetScaleFactor()
//-------------------------------------------------------------------------
{
  return m_ScaleFactor;
}
//-------------------------------------------------------------------------
void mafVMEHelAxis::SetScaleFactor(double scale)
//-------------------------------------------------------------------------
{
  m_ScaleFactor = scale;
  if (m_Gui)
  {
    m_Gui->Update();
  }
  m_ScaleAxisTransform->Identity();
  m_ScaleAxisTransform->Scale(m_AngleFactor * m_ScaleFactor,m_AngleFactor * m_ScaleFactor,m_AngleFactor * m_ScaleFactor);
  m_ScaleAxisTransform->Update();
  m_ScaleAxis->Update();
  Modified();
}
//-------------------------------------------------------------------------
char **mafVMEHelAxis::GetIcon()
//-------------------------------------------------------------------------
{
#include "mafVMESurface.xpm"
  return mafVMESurface_xpm;
}


//-------------------------------------------------------------------------
mafVMEHelAxis::mafVMEHelAxis() : mafVME()
//-------------------------------------------------------------------------
{
  mafNEW(m_Transform);
  mafVMEOutputSurface *output=mafVMEOutputSurface::New(); // an output with no data
  output->SetTransform(m_Transform); // force my transform in the output
  SetOutput(output);

  // attach a datapipe which creates a bridge between VTK and MAF
  mafDataPipeCustom *dpipe = mafDataPipeCustom::New();
  SetDataPipe(dpipe);

  DependsOnLinkedNodeOn();

  m_ScaleFactor = 1.0;
  m_AngleFactor = 1.0;

  vtkUnsignedCharArray *data;
  float scalar_red[3]   = {255,0,0};
  float scalar_green[3] = {0,255,0};
  float scalar_blu[3]   = {0,0,255};

  m_ZArrow = vtkArrowSource::New();
  m_ZArrow->SetShaftRadius(m_ZArrow->GetTipRadius() / 5);
  m_ZArrow->SetTipResolution(40);
  m_ZArrow->SetTipRadius(m_ZArrow->GetTipRadius() / 2);
  m_ZArrow->Update();

  m_ZAxisTransform = vtkTransform::New();
  m_ZAxisTransform->PostMultiply();
  m_ZAxisTransform->RotateY(-90);
  m_ZAxisTransform->Update();

  m_ZAxis  = vtkTransformPolyDataFilter::New();
  m_ZAxis->SetInput(m_ZArrow->GetOutput());
  m_ZAxis->SetTransform(m_ZAxisTransform);
  m_ZAxis->Update();

  int points = m_ZArrow->GetOutput()->GetNumberOfPoints();

  data = vtkUnsignedCharArray::New();
  data->SetName("AXES");
  data->SetNumberOfComponents(3);
  data->SetNumberOfTuples(points * 3);
  int i;
  for (i = 0; i < points; i++)
    data->SetTuple(i, scalar_red);

  // this filter do not copy the scalars also if all input 
  m_Axes = vtkAppendPolyData::New();    
  m_Axes->AddInput(m_ZAxis->GetOutput());

  m_Axes->Update();

  m_ScaleAxisTransform = vtkTransform::New();
  m_ScaleAxisTransform->Scale(m_ScaleFactor,m_ScaleFactor,m_ScaleFactor);
  m_ScaleAxisTransform->Update();

  vtkMAFSmartPointer<vtkPolyData> axes_surface;
  axes_surface = m_Axes->GetOutput();
  axes_surface->SetSource(NULL);
  axes_surface->GetPointData()->SetScalars(data);
  vtkDEL(data);

  m_ScaleAxis  = vtkTransformPolyDataFilter::New();
  m_ScaleAxis->SetInput(axes_surface.GetPointer());
  m_ScaleAxis->SetTransform(m_ScaleAxisTransform);
  m_ScaleAxis->Update();

  //SetData(m_ScaleAxis->GetOutput(), -1);

  dpipe->SetInput(m_ScaleAxis->GetOutput());
}
//-------------------------------------------------------------------------
int mafVMEHelAxis::InternalInitialize()
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
//-------------------------------------------------------------------------
mmaMaterial *mafVMEHelAxis::GetMaterial()
//-------------------------------------------------------------------------
{
  mmaMaterial *material = (mmaMaterial *)GetAttribute("MaterialAttributes");
  if (material == NULL)
  {
    material = mmaMaterial::New();
    SetAttribute("MaterialAttributes", material);
    if (m_Output)
    {
      ((mafVMEOutputSurface *)m_Output)->SetMaterial(material);
    }
  }
  return material;
}

//-------------------------------------------------------------------------
mafVMEHelAxis::~mafVMEHelAxis()
//-------------------------------------------------------------------------
{
  mafDEL(m_Transform);
  SetOutput(NULL);

  vtkDEL(m_ZArrow);
  vtkDEL(m_ZAxisTransform);
  m_ZAxis->SetTransform(NULL);
  vtkDEL(m_ZAxis);

  vtkDEL(m_Axes);
  vtkDEL(m_ScaleAxisTransform);
  m_ScaleAxis->SetTransform(NULL);
  vtkDEL(m_ScaleAxis);
}

//----------------------------------------------------------------------------
void mafVMEHelAxis::InternalPreUpdate()
//----------------------------------------------------------------------------
{

}

//-----------------------------------------------------------------------
int mafVMEHelAxis::InternalStore(mafStorageElement *parent)
//-----------------------------------------------------------------------
{  
  if (Superclass::InternalStore(parent)==MAF_OK)
  {
    parent->StoreMatrix("Transform",&m_Transform->GetMatrix());
    parent->StoreDouble("m_ScaleFactor", m_ScaleFactor);
    return MAF_OK;
  }
  return MAF_ERROR;
}

//-----------------------------------------------------------------------
int mafVMEHelAxis::InternalRestore(mafStorageElement *node)
//-----------------------------------------------------------------------
{
  if (Superclass::InternalRestore(node)==MAF_OK)
  {
    mafMatrix matrix;
    if (node->RestoreMatrix("Transform",&matrix)==MAF_OK)
    {
      m_Transform->SetMatrix(matrix);
      node->RestoreDouble("m_ScaleFactor", m_ScaleFactor);
      SetScaleFactor(m_ScaleFactor);
      return MAF_OK;
    }
  }
  return MAF_ERROR;
}


//----------------------------------------------------------------------------
void mafVMEHelAxis::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  switch (maf_event->GetId())
  {
  case ID_SCALE_FACTOR:
    {
      SetScaleFactor(m_ScaleFactor);
      mafEvent cam_event(this,CAMERA_UPDATE);
      this->ForwardUpEvent(cam_event);
      break;
    }
  default:
    {
      Superclass::OnEvent(maf_event);
      break; 
    }
  }
}

//----------------------------------------------------------------------------
mafGUI *mafVMEHelAxis::CreateGui()
//----------------------------------------------------------------------------
{
  wxString saAxisChoices[2] = {"Prefer Z Axis", "Prefer Y Axis"};

  m_Gui = Superclass::CreateGui();
  m_Gui->Show(false);
  m_Gui->Double(ID_SCALE_FACTOR,_("scale"),&m_ScaleFactor);
  m_Gui->Divider();
  m_Gui->Divider();
  m_Gui->Button(ID_PRINT, "print", "debug info" );
  m_Gui->Update();
  return m_Gui;
}

//----------------------------------------------------------------------------
void mafVMEHelAxis::Print(std::ostream& os, const int tabs)// const
//-----------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
  mafIndent indent(tabs);
  os<<indent<<"Scale: "<<indent<<m_ScaleFactor;
}
