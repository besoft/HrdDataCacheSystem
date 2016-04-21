/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterC3DBTK.cpp,v $
  Language:  C++
  Date:      $Date: 2010-02-23 13:35:49 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Matteo Giacomoni
==========================================================================
  Copyright (c) 2009
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "medDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#ifdef MED_USE_BTK

#include "lhpOpImporterC3DBTK.h"

#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafGUI.h"

#include "mafSmartPointer.h"
#include "mafVME.h"
#include "vtkMAFSmartPointer.h"
#include "mafVMEGroup.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafVMESurface.h"
#include "mafVMEVector.h"
#include "medVMEAnalog.h"
#include "mafTagArray.h"

#include <vtkCubeSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include "vtkMAFSmartPointer.h"
#include "vtkCellArray.h"
#include <vtkPoints.h>
#include <vtkPolyData.h>

//#include "C3D_Reader.h"

#include <vcl_fstream.h>
#include <vcl_string.h>
#include <vnl\vnl_matrix.h>

#include <iostream>
#include <fstream>

#define PLATFORM_THICKNESS 5.0
#define mafMax(a, b) (((a) >= (b)) ? (a) : (b))
#define mafMin(a, b) (((a) <= (b)) ? (a) : (b))

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpImporterC3DBTK);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpImporterC3DBTK::_InternalC3DData::_InternalC3DData()
//----------------------------------------------------------------------------
{
  //vmes
  m_VmeGroup = NULL;
  //m_VmeCloud = NULL;
  m_VmeAnalog = NULL;

  //c3d filename
  m_FileName = "";

  //Aurion
  //m_Errcode = 0;
  m_AnalogRate = 0;
  m_VideoRate = 0;
  m_LengthMs = 0;

  //derived
  m_TrajectorySamplePeriod = 0;
  m_AnalogSamplePeriod = 0;
  m_VectogramSamplePeriod = 0;

  m_NumTotTrajectories = 0; 
  m_NumTrajectories = 0; 
  m_NumAngles = 0; 
  m_NumMoments = 0; 
  m_NumPowers = 0; 
  m_NumFrames = 0; 
  m_NumChannels = 0;
  m_NumSamples = 0;
  m_NumEvents = 0;
  m_NumPlatforms = 0;

  m_X = 0;
  m_Y = 0;
  m_Z = 0;

  m_AnalogValue = 0;
  m_EventValue  = 0;

  m_CopX = 0;
  m_CopY = 0;

  m_ForceX = 0;
  m_ForceY = 0; 
  m_ForceZ = 0;

  m_MomentX = 0;
  m_MomentY = 0;
  m_MomentZ = 0;

  m_CenterX = 0;
  m_CenterY = 0;

  m_OriginX = 0;
  m_OriginY = 0;
  m_OriginZ = 0;
}

//----------------------------------------------------------------------------
lhpOpImporterC3DBTK::lhpOpImporterC3DBTK(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_IMPORTER;
  m_Canundo = true;

  m_FileDir = (mafGetApplicationDirectory() + "/Data/External/").c_str();
  m_DictionaryFileName = "";

  //gui
  m_ImportTrajectoriesFlag = TRUE;
  m_ImportAnalogFlag = TRUE;
  m_ImportPlatformFlag = TRUE;
  m_ImportEventFlag = FALSE;
}

//----------------------------------------------------------------------------
lhpOpImporterC3DBTK::~lhpOpImporterC3DBTK()
//----------------------------------------------------------------------------
{
  Clear();
}

//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::Clear()
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_intData.size(); i++)
  {
    mafDEL(m_intData[i].m_VmeGroup);
    for(std::map<mafString, mafVMELandmarkCloud*>::iterator it = m_intData[i].m_Clouds.begin(); it != m_intData[i].m_Clouds.end(); ++it)
    {
      mafDEL(it->second);
    }
    m_intData[i].m_Clouds.clear();
    //mafDEL(m_intData[i].m_VmeCloud);
    mafDEL(m_intData[i].m_VmeAnalog);
    for(int currentPlatForm=0; currentPlatForm< m_intData[i].m_PlatformList.size();currentPlatForm++)
    {
      mafDEL(m_intData[i].m_PlatformList[currentPlatForm]);
      mafDEL(m_intData[i].m_ForceList[currentPlatForm]);
      mafDEL(m_intData[i].m_MomentList[currentPlatForm]);
    }
    m_intData[i].m_PlatformList.clear();
    m_intData[i].m_ForceList.clear();
    m_intData[i].m_MomentList.clear();
  }
  m_intData.clear();
}
//----------------------------------------------------------------------------
bool lhpOpImporterC3DBTK::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return true;
}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterC3DBTK::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpImporterC3DBTK *cp = new lhpOpImporterC3DBTK(m_Label);
  cp->m_Canundo = m_Canundo;
  cp->m_OpType = m_OpType;
  cp->m_Listener = m_Listener;
  //cp->m_intData.m_VmeCloud = m_intData.m_VmeCloud;
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
  ShowGui();
}
//----------------------------------------------------------------------------
int lhpOpImporterC3DBTK::OpenC3D(const mafString &fullFileName)
//----------------------------------------------------------------------------
{
  mafLogMessage("C3D_Open");

	btk::AcquisitionFileReader::Pointer reader = btk::AcquisitionFileReader::New();
  reader->SetFilename(fullFileName.GetCStr());
  reader->Update();
  m_Acq = reader->GetOutput();

  m_Pfe = btk::ForcePlatformsExtractor::New();
  m_Pfe->SetInput(reader->GetOutput());
  m_Pfc = m_Pfe->GetOutput();
  m_Pfe->Update();

  m_Grws = btk::GroundReactionWrenchFilter::New();
  m_Grws ->SetInput(m_Pfe->GetOutput());
  m_Grws->Update();

  return 0;
}
//----------------------------------------------------------------------------
bool lhpOpImporterC3DBTK::LoadDictionary()
//----------------------------------------------------------------------------
{
  vcl_string landmarkName, segmentName;
  vcl_ifstream dictionaryInputStream(m_DictionaryFileName, std::ios::in);

  if(dictionaryInputStream.is_open() == 0)
    return false;
  while(dictionaryInputStream >> landmarkName)	
  {
    dictionaryInputStream >> segmentName;			
    std::map<mafString, mafString>::iterator it = m_dictionaryStruct.find(landmarkName.c_str());
    if(it != m_dictionaryStruct.end())
    {
      m_dictionaryStruct.clear();
      return false;
    }
    m_dictionaryStruct[landmarkName.c_str()] = segmentName.c_str();
  }
  return true;
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::DestroyDictionary()
//----------------------------------------------------------------------------
{
  m_dictionaryStruct.clear();
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::Initialize(const mafString &fullFileName, lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
	//initialize class members with read data 
  //Trajectories
  intData.m_NumTotTrajectories = m_Acq->GetPointNumber();		//number of total trajectories(with angles, moments, powers)
  intData.m_NumFrames = m_Acq->GetPointFrameNumber();		  //number of frames

  //Analog
  intData.m_NumChannels = m_Acq->GetAnalogs()->GetItemNumber();    //channels number
  intData.m_NumSamples  = m_Acq->GetAnalogFrameNumber(); //samples number

  //Platforms
  intData.m_NumPlatforms = m_Pfc->GetItemNumber(); // platforms number

  //Events
  intData.m_NumEvents = m_Acq->GetEventNumber();              //events number

  //derived
  intData.m_AnalogRate = m_Acq->GetAnalogFrequency();
  intData.m_VideoRate = m_Acq->GetPointFrequency();
  intData.m_LengthMs = (long)((intData.m_NumFrames/intData.m_VideoRate)*1000); //is necessary because we need of msec and not of sec
  
  intData.m_TrajectorySamplePeriod = ((double)intData.m_LengthMs/(double)intData.m_NumFrames) / 1000.0;
  intData.m_AnalogSamplePeriod = ((double)intData.m_LengthMs/(double)intData.m_NumSamples) / 1000.0;
  intData.m_VectogramSamplePeriod = intData.m_AnalogSamplePeriod;

  wxString fileName = fullFileName.GetCStr();
  fileName = fileName.AfterLast('\\').BeforeLast('.');

  intData.m_FileName = fileName;
}
//----------------------------------------------------------------------------
mafVMEGroup *lhpOpImporterC3DBTK::ImportSingleFile(const mafString &fullFileName, lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
	if(OpenC3D(fullFileName)==NOERROR)
	{
		//c3d read data
    Initialize(fullFileName, intData);

		//fill data structures
    if(m_ImportTrajectoriesFlag || m_ImportAnalogFlag || m_ImportPlatformFlag || m_ImportEventFlag) 
    {
      mafNEW(intData.m_VmeGroup);
      mafString resultName;
      resultName.Append(intData.m_FileName);
      resultName.Append("_C3D");
      intData.m_VmeGroup->SetName(resultName);
    }

    if(m_ImportTrajectoriesFlag) 
    {
      ImportTrajectories(intData);
      for(std::map<mafString, mafVMELandmarkCloud*>::iterator it = intData.m_Clouds.begin(); it != intData.m_Clouds.end(); ++it)
      {
        it->second->ReparentTo(intData.m_VmeGroup);
      }
      //intData.m_VmeCloud->ReparentTo(intData.m_VmeGroup);
    }
    
		if(m_ImportAnalogFlag) 
    {
      ImportAnalog(intData);	
      intData.m_VmeAnalog->ReparentTo(intData.m_VmeGroup);
    }
    
		if(m_ImportPlatformFlag) 
    {
      ImportPlatform(intData);
      for(int currentPlatform = 0; currentPlatform<intData.m_PlatformList.size(); currentPlatform++)
      {
        intData.m_PlatformList[currentPlatform]->ReparentTo(intData.m_VmeGroup);
        intData.m_ForceList[currentPlatform]->ReparentTo(intData.m_PlatformList[currentPlatform]);
        intData.m_MomentList[currentPlatform]->ReparentTo(intData.m_PlatformList[currentPlatform]);
      }
    }
    
    if(m_ImportEventFlag) 
    {
      ImportEvent(intData);
    }
	}
  return intData.m_VmeGroup;
}

//----------------------------------------------------------------------------
bool lhpOpImporterC3DBTK::Import()
//----------------------------------------------------------------------------
{
  bool result = false;
  Clear();
  for(unsigned fileIndex = 0; fileIndex < m_C3DInputFileNameFullPaths.size(); fileIndex++)
  {
    _InternalC3DData intData;
    mafVMEGroup *imported = ImportSingleFile(m_C3DInputFileNameFullPaths[fileIndex], intData);
    if(imported != NULL)
    {
      result = true;
      //m_VmeGroups.push_back(imported);
      m_intData.push_back(intData);
    }
  }
  return result;
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::ImportTrajectories(lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
  wxBusyInfo *wait;
  if(!m_TestMode)
  {
    wait = new wxBusyInfo("Please wait, import trajectories");
  }

  intData.m_Clouds.clear();

  bool usingDictionary = (m_DictionaryFileName != "");
  mafVMELandmarkCloud *specCloud = NULL;//the only cloud if read without dictionary and NOT_IN_DICTIONARY with
  mafString specCloudName;//name of specCloud

  specCloudName.Append(intData.m_FileName);
  if(!usingDictionary)//without dictionary create cloud and set its name
  {
    mafNEW(specCloud);
    if(specCloud == NULL)
      return;
    specCloudName.Append("_TRAJECTORIES");
    specCloud->SetName(specCloudName);
  }
  else//with dictionary just prepare name, creation only if needed
    specCloudName.Append("_NOT_IN_DICTIONARY");

  mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  
  long progress = 0;

  for(int currentFrame = 0; currentFrame < intData.m_NumFrames; currentFrame++)
  {
    intData.m_NumTrajectories=0;
    intData.m_NumAngles=0;
    intData.m_NumMoments=0;
    intData.m_NumPowers=0;
    //For every trajectory

    for(int currentTrajectory=0; currentTrajectory<intData.m_NumTotTrajectories; currentTrajectory++)
    {

      int v=m_Acq->GetPoint(currentTrajectory)->GetType();

      switch(m_Acq->GetPoint(currentTrajectory)->GetType())
      {
      case btk::Point::Force:
      case btk::Point::Marker:
        {
          strcpy (intData.m_TrajectoryName, m_Acq->GetPoint(currentTrajectory)->GetLabel().c_str());

          //control if m_Trajectory is not a phantom landmark(camera reflexes)
          if(intData.m_TrajectoryName[0] != '*')
          {
            mafVMELandmarkCloud *addTo = specCloud;//by default add to this specific cloud
            if(usingDictionary)
            {
              //find current trajectory name in dictionary
              std::map<mafString, mafString>::iterator nmIt = m_dictionaryStruct.find(intData.m_TrajectoryName);
              //trajectory name found
              if(nmIt != m_dictionaryStruct.end())
              {
                //find corresponding cloud if already exists
                std::map<mafString, mafVMELandmarkCloud*>::iterator clIt = intData.m_Clouds.find(nmIt->second);
                //not created yet
                if(clIt == intData.m_Clouds.end() || clIt->second == NULL)
                {
                  mafVMELandmarkCloud *cld = NULL;
                  mafString cldName;
                  //create cloud
                  mafNEW(cld);
                  //if created successfully use it
                  if(cld != NULL)
                  {
                    cldName.Append(intData.m_FileName);
                    cldName.Append("_");
                    cldName.Append(nmIt->second);
                    cld->SetName(cldName);
                    intData.m_Clouds[nmIt->second] = cld;
                    addTo = cld;
                  }
                  //clear and exit in case of problems in cloud creation, as we have not correct one, we cannot create new
                  if(addTo == NULL)
                  {
                    for(std::map<mafString, mafVMELandmarkCloud*>::iterator it = intData.m_Clouds.begin(); it != intData.m_Clouds.end(); ++it)
                    {
                      mafDEL(it->second);
                    }
                    intData.m_Clouds.clear();
                    return;
                  }
                }
                //cloud is already created, just select it to use
                else
                {
                  addTo = clIt->second;
                }
              }
              //trajectory name not in dictionary
              else
              {
                //if NOT_IN_DICTIONARY is not created yet, create it and use. in case of problems exit
                if(specCloud == NULL)
                {
                  mafNEW(specCloud);
                  //clear and exit in case of problems in cloud creation, as we have not correct one, we cannot create new
                  if(specCloud == NULL)
                  {
                    for(std::map<mafString, mafVMELandmarkCloud*>::iterator it = intData.m_Clouds.begin(); it != intData.m_Clouds.end(); ++it)
                    {
                      mafDEL(it->second);
                    }
                    intData.m_Clouds.clear();
                    return;
                  }
                  //select this cloud for using
                  specCloud->SetName(specCloudName);
                  addTo = specCloud;
                }
              }
            }

            strcpy (intData.m_TrajectoryUnit, m_Acq->GetPointUnit().c_str());

            bool visibility = m_Acq->GetPoint(currentTrajectory)->GetResiduals().coeffRef(currentFrame) != -1;
            if(visibility)
            {
              intData.m_X = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,0);					//component x of the trajectory
              intData.m_Y = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,1);					//component y of the trajectory
              intData.m_Z = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,2);					//component z of the trajectory
            }
            else
            {
              intData.m_X = intData.m_Y = intData.m_Z = 0.0;
            }


            if(currentFrame == 0)
            {
              addTo->AppendLandmark(intData.m_X,intData.m_Y,intData.m_Z,intData.m_TrajectoryName);
            }
            else
            {
              addTo->SetLandmark(intData.m_TrajectoryName,intData.m_X,intData.m_Y,intData.m_Z,currentFrame * intData.m_TrajectorySamplePeriod);
            }

            addTo->SetLandmarkVisibility(intData.m_TrajectoryName,visibility,currentFrame * intData.m_TrajectorySamplePeriod);

            intData.m_NumTrajectories++;
          }
        }
        break;
      case btk::Point::Angle:

        strcpy (intData.m_AngleName, m_Acq->GetPoint(currentTrajectory)->GetLabel().c_str());
        
        strcpy (intData.m_AngleUnit, m_Acq->GetPointUnit().c_str());
        
        if(m_Acq->GetPoint(currentTrajectory)->GetResiduals().coeffRef(currentFrame) != -1)
        {
          intData.m_X = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,0);					//component x of angle
          intData.m_Y = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,1);					//component y of angle
          intData.m_Z = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,2);					//component z of angle
        }
        intData.m_NumAngles++;
        break;
      case btk::Point::Moment:

        strcpy (intData.m_MomentName, m_Acq->GetPoint(currentTrajectory)->GetLabel().c_str());

        strcpy (intData.m_MomentUnit, m_Acq->GetPointUnit().c_str());
        
        if(m_Acq->GetPoint(currentTrajectory)->GetResiduals().coeffRef(currentFrame) != -1)
        {
          intData.m_X = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,0);					//component x of the moment
          intData.m_Y = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,1);					//component y of the moment
          intData.m_Z = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,2);					//component z of the moment
        }
        intData.m_NumMoments++;
        break;
      case btk::Point::Power:

        strcpy (intData.m_PowerName, m_Acq->GetPoint(currentTrajectory)->GetLabel().c_str());
        
        strcpy (intData.m_PowerUnit, m_Acq->GetPointUnit().c_str());
        
        if(m_Acq->GetPoint(currentTrajectory)->GetResiduals().coeffRef(currentFrame) != -1)
        {
          intData.m_X = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,0);					//component x of power
          intData.m_Y = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,1);					//component y of power
          intData.m_Z = m_Acq->GetPoint(currentTrajectory)->GetValues()(currentFrame,2);					//component z of power
        }
        intData.m_NumPowers++;
        break;
      default:
        {
          //wxMessageBox("ok");
          break;
        }
      }
    }

    progress = (currentFrame + 1) * 100 / intData.m_NumFrames;
    mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
  }

  //if specCloud exists add it to all clouds
  if(specCloud != NULL)
    intData.m_Clouds[specCloudName] = specCloud;


  for(std::map<mafString, mafVMELandmarkCloud*>::iterator it = intData.m_Clouds.begin(); it != intData.m_Clouds.end(); ++it)
  {
    it->second->Modified();
    it->second->Update();
  }

  mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));

  if(!m_TestMode)
  {
    delete wait;
  }
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::ImportAnalog(lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
  wxBusyInfo *wait;
  if(!m_TestMode)
  {
    wait = new wxBusyInfo("Please wait, import analog data");
  }
  
  long progress = 0;
  mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));

  //name analog vme
  mafNEW(intData.m_VmeAnalog);
  mafString analogVmeName;
  analogVmeName.Append(intData.m_FileName);
  analogVmeName.Append("_ANALOG");
  intData.m_VmeAnalog->SetName(analogVmeName);

  vnl_matrix<double> analogMatrix;
  analogMatrix.set_size(intData.m_NumChannels+1 , intData.m_NumSamples); //set dimensions

  std::vector<mafString> channelsNameList; //string array for channel name

  //For every Sample
  for(int currentSample=0; currentSample < intData.m_NumSamples; currentSample++)
  {
    mafTimeStamp currentTime = currentSample * intData.m_AnalogSamplePeriod;
    
    analogMatrix.put(0,currentSample, currentTime); //fill first row with timeframe, every column is a time

    //For every channel
    for(int currentChannel=0; currentChannel < intData.m_NumChannels; currentChannel++)
    {

      strcpy (intData.m_ChannelName, m_Acq->GetAnalog(currentChannel)->GetLabel().c_str());
      intData.m_AnalogValue = m_Acq->GetAnalog(currentChannel)->GetValues()(currentSample, 0);
      strcpy (intData.m_ChannelUnit, m_Acq->GetAnalog(currentChannel)->GetUnit().c_str());

      if(currentSample == 0) channelsNameList.push_back(intData.m_ChannelName);

      analogMatrix.put(currentChannel+1,currentSample, intData.m_AnalogValue); //fill following rows with values, every channel is a row
    }

    progress = (currentSample +1 ) * 100 / (intData.m_NumSamples);
    mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
  }

  mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));

  intData.m_VmeAnalog->SetData(analogMatrix, 0);

  mafTagItem tag_Sig;
  tag_Sig.SetName("SIGNALS_NAME");
  tag_Sig.SetNumberOfComponents(intData.m_NumSamples);
  intData.m_VmeAnalog->GetTagArray()->SetTag(tag_Sig);

  mafTagItem *tag_Signals = intData.m_VmeAnalog->GetTagArray()->GetTag("SIGNALS_NAME");
  for (int n = 0; n < channelsNameList.size(); n++)
  {
    tag_Signals->SetValue(channelsNameList[n], n);
  }

  if(!m_TestMode)
  {
    delete wait;
  }
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::ImportPlatform(lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
  wxBusyInfo *wait;
  if(!m_TestMode)
  {
    wait = new wxBusyInfo("Please wait, import Force Plate Data");
  }

  long progress = 0;
  mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));

  //For every platform
  for(int currentPlatform=0; currentPlatform<intData.m_NumPlatforms; currentPlatform++)
  {
    btk::ForcePlatform::Pointer pf1 = m_Pfc->GetItem(currentPlatform);

    intData.m_OriginX = pf1->GetOrigin().x();

    //corner coordinate  1
    double platformCorner1[2];
    intData.m_X = pf1->GetCorner(0)(0);
    intData.m_Y = pf1->GetCorner(0)(1);
    platformCorner1[0] = intData.m_X;
    platformCorner1[1] = intData.m_Y;

    //corner coordinate  2
    double platformCorner2[2];
    intData.m_X = pf1->GetCorner(1)(0);
    intData.m_Y = pf1->GetCorner(1)(1);
    platformCorner2[0] = intData.m_X;
    platformCorner2[1] = intData.m_Y;
    
    //corner coordinate  3
    double platformCorner3[2];
    intData.m_X = pf1->GetCorner(2)(0);
    intData.m_Y = pf1->GetCorner(2)(1);
    platformCorner3[0] = intData.m_X;
    platformCorner3[1] = intData.m_Y;
   
    //corner coordinate  4
    double platformCorner4[2];
    intData.m_X = pf1->GetCorner(3)(0);
    intData.m_Y = pf1->GetCorner(3)(1);
    platformCorner4[0] = intData.m_X;
    platformCorner4[1] = intData.m_Y;

    intData.m_CenterX = (platformCorner1[0]+platformCorner2[0]+platformCorner3[0]+platformCorner4[0])/4;
    intData.m_CenterY = (platformCorner1[1]+platformCorner2[1]+platformCorner3[1]+platformCorner4[1])/4;

    double minX,maxX;
    double minY,maxY;
    minX = VTK_DOUBLE_MAX;
    maxX = VTK_DOUBLE_MIN;
    minY = VTK_DOUBLE_MAX;
    maxY = VTK_DOUBLE_MIN;

    minX = mafMin(mafMin(mafMin(mafMin(minX,platformCorner1[0]),platformCorner2[0]),platformCorner3[0]),platformCorner4[0]);
    maxX = mafMax(mafMax(mafMax(mafMax(maxX,platformCorner1[0]),platformCorner2[0]),platformCorner3[0]),platformCorner4[0]);
    minY = mafMin(mafMin(mafMin(mafMin(minY,platformCorner1[1]),platformCorner2[1]),platformCorner3[1]),platformCorner4[1]);
    maxY = mafMax(mafMax(mafMax(mafMax(maxY,platformCorner1[1]),platformCorner2[1]),platformCorner3[1]),platformCorner4[1]);

    vtkMAFSmartPointer<vtkCubeSource> cube;

    mafVMESurface *platform;
    mafNEW(platform);
    intData.m_PlatformList.push_back(platform);
    mafString platformNumber;
    platformNumber << (currentPlatform + 1) ;
    mafString platformName;
    platformName.Append(intData.m_FileName);
    platformName.Append("_FORCE_PLATFORM_");
    platformName.Append(platformNumber);
    intData.m_PlatformList[currentPlatform]->SetName(platformName);

    double z = 0;
    double thickness = z - PLATFORM_THICKNESS;

    cube->SetBounds(minX,maxX,minY,maxY,thickness,z);
    
    //Create the mafVMESurface for the platforms
    intData.m_PlatformList[currentPlatform]->SetData(cube->GetOutput(), 0);

    //force vector
    mafVMEVector *force;
    mafNEW(force);
    intData.m_ForceList.push_back(force);
    mafString forceName;
    forceName.Append(intData.m_FileName);
    forceName.Append("_GRF_");
    forceName.Append(platformNumber);
    intData.m_ForceList[currentPlatform]->SetName(forceName);

    //moment vector
    mafVMEVector *moment;
    mafNEW(moment);
    intData.m_MomentList.push_back(moment);
    mafString momentName;
    momentName.Append(intData.m_FileName);
    momentName.Append("_MOMENT_");
    momentName.Append(platformNumber);
    intData.m_MomentList[currentPlatform]->SetName(momentName);

    vtkMAFSmartPointer<vtkPolyData> vectorForce;
    vtkMAFSmartPointer<vtkPoints> pointsForce;
    vtkMAFSmartPointer<vtkCellArray> cellArrayForce;
    int pointIdForce[2] = {0,1};
    vectorForce->SetPoints(pointsForce);
    vectorForce->SetLines(cellArrayForce);

    vtkMAFSmartPointer<vtkPolyData> vectorMoment;
    vtkMAFSmartPointer<vtkPoints> pointsMoment;
    vtkMAFSmartPointer<vtkCellArray> cellArrayMoment;
    int pointIdMoment[2] = {0,1};
    vectorMoment->SetPoints(pointsMoment);
    vectorMoment->SetLines(cellArrayMoment);

    //For every sample
    mafTimeStamp currentTime = 0;
    m_Grws->SetThresholdValue(5.0);
    m_Grws->SetThresholdState(true);
    m_Grws->Update();
    btk::WrenchCollection::Pointer grwc = m_Grws->GetOutput();
    grwc->Update();
    btk::Wrench::Pointer grw = grwc->GetItem(currentPlatform);
    
    btk::ForcePlatformWrenchFilter::Pointer fpwf = btk::ForcePlatformWrenchFilter::New();
    fpwf->SetInput(m_Pfe->GetOutput());
    fpwf->Update();

    btk::WrenchCollection::Pointer fpwc = fpwf->GetOutput();
    fpwc->Update();
    
    for(int currentSample=0; currentSample<intData.m_NumSamples; currentSample++)
    {
      int nEl = fpwc->GetItemNumber();
      intData.m_CopX = grw->GetPosition()->GetValues()(currentSample,0);
      intData.m_CopY = grw->GetPosition()->GetValues()(currentSample,1);

      double Fx = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(0)(currentSample);
      double Fy = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(1)(currentSample);
      double Fz = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(2)(currentSample);
      double Mx = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(0)(currentSample);
      double My = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(1)(currentSample);
      double Mz = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(2)(currentSample);
      
      intData.m_ForceX = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(0)(currentSample);
      intData.m_ForceY = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(1)(currentSample);
      intData.m_ForceZ = fpwc->GetItem(currentPlatform)->GetForce()->GetValues().col(2)(currentSample);

      intData.m_MomentX = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(0)(currentSample);
      intData.m_MomentY = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(1)(currentSample);
      intData.m_MomentZ = fpwc->GetItem(currentPlatform)->GetMoment()->GetValues().col(2)(currentSample);

      currentTime = currentSample * intData.m_VectogramSamplePeriod;

      //force      
      pointsForce->Reset();
      pointsForce->InsertPoint(0, 0, 0, 0);
      pointsForce->InsertPoint(1, intData.m_ForceX, intData.m_ForceY, intData.m_ForceZ);
      cellArrayForce->Reset();
      cellArrayForce->InsertNextCell(2, pointIdForce);
      vectorForce->Update();

      vtkMAFSmartPointer<vtkTransformPolyDataFilter> transfVecForce;
      vtkMAFSmartPointer<vtkTransform> transfForce;

      transfForce->Translate(intData.m_CopX, intData.m_CopY, z); //z = 0
      transfVecForce->SetTransform(transfForce);
      transfVecForce->SetInput(vectorForce);
      transfVecForce->Update();

      
      intData.m_ForceList[currentPlatform]->SetData(transfVecForce->GetOutput(), currentTime); //look here times

      intData.m_ForceList[currentPlatform]->Modified();
      intData.m_ForceList[currentPlatform]->Update();
      intData.m_ForceList[currentPlatform]->GetOutput()->GetVTKData()->Update();

      //moment
      
      
      pointsMoment->Reset();
      pointsMoment->InsertPoint(0, 0, 0, 0);
      pointsMoment->InsertPoint(1, intData.m_MomentX, intData.m_MomentY, intData.m_MomentZ);

      cellArrayMoment->Reset();
      cellArrayMoment->InsertNextCell(2, pointIdMoment);  
      vectorMoment->Update();

      vtkMAFSmartPointer<vtkTransformPolyDataFilter> transfVecMoment;
      vtkMAFSmartPointer<vtkTransform> transfMoment;

      transfMoment->Translate(intData.m_CopX, intData.m_CopY, z); //z = 0
      transfVecMoment->SetTransform(transfMoment);
      transfVecMoment->SetInput(vectorMoment);
      transfVecMoment->Update();


      intData.m_MomentList[currentPlatform]->SetData(transfVecMoment->GetOutput(), currentTime); //look here times

      intData.m_MomentList[currentPlatform]->Modified();
      intData.m_MomentList[currentPlatform]->Update();
      intData.m_MomentList[currentPlatform]->GetOutput()->GetVTKData()->Update();

      progress = (currentSample + 1 + (currentPlatform * intData.m_NumSamples )) * 100 / (intData.m_NumSamples * intData.m_NumPlatforms);
      mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
    }
  }
  mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));

  if(!m_TestMode)
  {
    delete wait;
  }
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::ImportEvent(lhpOpImporterC3DBTK::_InternalC3DData &intData)
//----------------------------------------------------------------------------
{
  //For every event
  for(int currentEvent=0; currentEvent<intData.m_NumEvents; currentEvent++)
  {
    //event context
    strcpy (intData.m_EventContext, m_Acq->GetEvent(currentEvent)->GetContext().c_str());
    //event value in seconds
    intData.m_EventValue = m_Acq->GetEvent(currentEvent)->GetTime();
  }
}
//----------------------------------------------------------------------------
// Operation constants
//----------------------------------------------------------------------------
enum C3D_IMPORTER_ID
{
  ID_FIRST = MINID,
  ID_IMPORT_TRAJECTORIES,
  ID_IMPORT_ANALOG,
  ID_IMPORT_PLATFORM,
  ID_IMPORT_EVENT,
  ID_LOAD_DICT,
  ID_CLEAR_DICT,
  ID_OK,
  ID_CANCEL,
};
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::CreateGui()
//----------------------------------------------------------------------------
{
  mafString wildcard = "c3d files (*.c3d)|*.c3d";
  std::vector<std::string> files;
  mafString f;

  m_C3DInputFileNameFullPaths.clear();
  {
    mafGetOpenMultiFiles(m_FileDir,wildcard, files);
    for(unsigned i = 0; i < files.size(); i++)
    {
      f = files[i].c_str();
      m_C3DInputFileNameFullPaths.push_back(f);
    }
  }

  //mafEventMacro(mafEvent(this,result));
	m_Gui = new mafGUI(this);
	m_Gui->Label("Select:", true);

  m_Gui->Bool(ID_IMPORT_TRAJECTORIES,_("Trajectories"),&m_ImportTrajectoriesFlag,1);
  m_Gui->Bool(ID_IMPORT_ANALOG,_("Analog Data"),&m_ImportAnalogFlag,1);
  m_Gui->Bool(ID_IMPORT_PLATFORM,_("Force Plate Data"),&m_ImportPlatformFlag,1);
  //m_Gui->Bool(ID_IMPORT_EVENT,_("Auto Crop"),&m_ImportEventFlag,1);
  m_Gui->Label("");
  m_Gui->FileOpen(ID_LOAD_DICT, "Dictionary",  &m_DictionaryFileName, "*.txt");
  m_Gui->Button(ID_CLEAR_DICT, "Clean", "", "Press to cancel using dictionary" );  

  m_Gui->Enable(ID_CLEAR_DICT, (m_DictionaryFileName != ""));

	m_Gui->OkCancel();
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::DictionaryUpdate() 
//----------------------------------------------------------------------------
{
  bool emptyName = (m_DictionaryFileName == "");
  DestroyDictionary();
  if(!emptyName)
  {
    if(!LoadDictionary())
    {
      wxLogMessage("Error reading dictionary.");
      m_DictionaryFileName = "";
    }
  }
  if(m_Gui)
  {
    m_Gui->Enable(ID_CLEAR_DICT, !emptyName);
    m_Gui->Update();
  }
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case wxOK:
      {
				if(Import())
				{
					this->OpStop(OP_RUN_OK);
				}
				else
				{
					this->OpStop(OP_RUN_CANCEL);
				}
        
      }
      break;
      case wxCANCEL:
      {
        this->OpStop(OP_RUN_CANCEL);
      }
      break;
      case ID_CLEAR_DICT:
        {
          m_DictionaryFileName = "";
        }//WARNING! NO break operator here, execution will continue in ID_LOAD_DICT
      case ID_LOAD_DICT:
        {
          DictionaryUpdate();
          break;
        }
      case ID_IMPORT_TRAJECTORIES:
        {
          if(m_Gui)
          {
            m_Gui->Enable(ID_LOAD_DICT, m_ImportTrajectoriesFlag != 0);
            m_Gui->Enable(ID_CLEAR_DICT, m_ImportTrajectoriesFlag != 0 && m_DictionaryFileName != "");
          }
          break;
        }
      default:
        mafEventMacro(*e);
      break;
    }	
  }
}

//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::OpDo()
//----------------------------------------------------------------------------
{
  wxBusyInfo wait("Please wait, create all VMEs in tree");

  for(unsigned i = 0; i < m_intData.size(); i++)
  {
    m_intData[i].m_VmeGroup->ReparentTo(m_Input);
    mafEventMacro(mafEvent(this, VME_ADD, m_intData[i].m_VmeGroup));
  }
}
//----------------------------------------------------------------------------
void lhpOpImporterC3DBTK::OpUndo()
//----------------------------------------------------------------------------
{   
  for(unsigned i = 0; i < m_intData.size(); i++)
  {
      mafEventMacro(mafEvent(this, VME_REMOVE, m_intData[i].m_VmeGroup));
  }
}

#endif