/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterC3D.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Daniele  Giunchi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpImporterC3D_H__
#define __lhpOpImporterC3D_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include <map>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;
class mafVMELandmarkCloud;
class mafVMESurface;
class mafVMEVector;
class medVMEAnalog;
class mafVMEGroup;

//----------------------------------------------------------------------------
// lhpOpImporterC3D :
//----------------------------------------------------------------------------
/** 
  This operation is a wrapper of C3D Reader SDK of Aurion. Need C3D_Reader.lib, C3D_Reader.dll,
  LicenseAurion.dll. The license can be obtained by Aurion following messagebox created by dll.
*/
class lhpOpImporterC3D : public mafOp
{
public:
	lhpOpImporterC3D(const wxString &label = "C3D Importer");
	~lhpOpImporterC3D(); 
	
  mafTypeMacro(lhpOpImporterC3D, mafOp);

  virtual void OnEvent(mafEventBase *maf_event);

  mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

  /** Builds operation's interface. */
	void OpRun();

  /** Execute the operation. */
  void OpDo();

  /** Makes the undo for the operation. */
  void OpUndo();

  /** Import the c3d*/
  bool Import();

  //methods useful for test

  /* Set/Get Full pathname of c3d File*/
  void SetC3DFileName(const char *filec3d){Clear(); m_C3DInputFileNameFullPaths.resize(1);m_C3DInputFileNameFullPaths[0] = filec3d;}
  const char * GetC3DFileName() {if(m_C3DInputFileNameFullPaths.empty()) return NULL; return m_C3DInputFileNameFullPaths[0];}

  /* Set/Get Full pathname of dictionary File*/
  void SetDictionaryFileName(const char *fileDict){m_DictionaryFileName = fileDict;DictionaryUpdate();}
  const char * GetDictionaryFileName() {return m_DictionaryFileName;}

  /* Get Group representing result of import */
  mafVMEGroup *GetGroup(){if(m_intData.empty()) return NULL; return m_intData[0].m_VmeGroup;}

  /* Get Landmark Cloud */
  mafVMELandmarkCloud *GetLandmarkCloudVME(){if(m_intData.empty()) return NULL; if(m_intData[0].m_Clouds.empty()) return NULL; return m_intData[0].m_Clouds.begin()->second;}

  /* Get Analog VME */
  medVMEAnalog *GetAnalogVME(){return m_intData.empty() ? NULL : m_intData[0].m_VmeAnalog;}
  
  /* Get Platform vmes from a std list*/
  mafVMESurface *GetPlatformVME(int index=0){if(m_intData.empty()) return NULL; return(index>=0 && index<m_intData[0].m_PlatformList.size())?m_intData[0].m_PlatformList[index]:NULL;}

  /* Get force vmes from a std list*/
  mafVMEVector *GetForceVME(int index=0){if(m_intData.empty()) return NULL; return(index>=0 && index<m_intData[0].m_ForceList.size())?m_intData[0].m_ForceList[index]:NULL;}

  /* Get moment vmes from a std list*/
  mafVMEVector *GetMomentVME(int index=0){if(m_intData.empty()) return NULL; return(index>=0 && index<m_intData[0].m_MomentList.size())?m_intData[0].m_MomentList[index]:NULL;}

  /* Set/Get Trajectory import flag*/
  void SetImportTrajectories(int flag){m_ImportTrajectoriesFlag = flag;}
  int  GetImportTrajectories(){return m_ImportTrajectoriesFlag;}

  /* Set/Get Analog import flag*/
  void SetImportAnalog(int flag){m_ImportAnalogFlag = flag;}
  int  GetImportAnalog(){return m_ImportAnalogFlag;}

  /* Set/Get Platform import flag*/
  void SetImportPlatform(int flag){m_ImportPlatformFlag = flag;}
  int  GetImportPlatform(){return m_ImportPlatformFlag;}

  /* Set/Get Event import flag*/
  void SetImportEvent(int flag){m_ImportEventFlag = flag;}
  int  GetImportEvent(){return m_ImportEventFlag;}

  

protected:
  struct _InternalC3DData;
  /** Create the dialog interface for the importer. */
  virtual void CreateGui();

	/** Initialize all structures needed for import data*/
  void Initialize(const mafString &fullFileName, _InternalC3DData &intData);

	/** Open C3D File*/
	int OpenC3D(const mafString &fullFileName);

	/** Read C3D File Header*/
	int ReadHeaderC3D(_InternalC3DData &intData);

	/** Read C3D Data*/
	int ReadDataC3D();

	/** Close C3D File, it should destroy objects*/
	int CloseC3D();

	/** Import the c3d Landmark coordinates*/
	void ImportTrajectories(_InternalC3DData &intData);
	
	/** Import the c3d analog data like EMG*/
	void ImportAnalog(_InternalC3DData &intData);

  /** Import the c3d platform data*/
  void ImportPlatform(_InternalC3DData &intData);

	/** Import the c3d events*/
	void ImportEvent(_InternalC3DData &intData);

	/** Cleans allocated memory*/
	void Clear();

  /** Import the c3d events*/
  mafVMEGroup* ImportSingleFile(const mafString &fullFileName, _InternalC3DData &intData);

  /** Do actions necessary on dictionary name changing*/
  void lhpOpImporterC3D::DictionaryUpdate();

  /** Load dictionary from specified file*/
  bool lhpOpImporterC3D::LoadDictionary();

  /** Destroy dictionary*/
  void lhpOpImporterC3D::DestroyDictionary();

  std::vector<mafString>         m_C3DInputFileNameFullPaths;
  mafString                      m_FileDir;
  mafString                      m_DictionaryFileName;
  std::map<mafString, mafString> m_dictionaryStruct;
	int m_ImportTrajectoriesFlag;
  int m_ImportAnalogFlag;
  int m_ImportPlatformFlag;
  int m_ImportEventFlag;

  //vmes
  struct _InternalC3DData
  {
    _InternalC3DData();

    mafString m_FileName;

    mafVMEGroup                               *m_VmeGroup;
    std::map<mafString, mafVMELandmarkCloud*> m_Clouds;

    medVMEAnalog *m_VmeAnalog;

    std::vector<mafVMESurface *> m_PlatformList;

    std::vector<mafVMEVector *> m_ForceList;
    std::vector<mafVMEVector *> m_MomentList;

	  //data filled by Aurion importer
	  //int m_Errcode;
	  float m_AnalogRate, m_VideoRate;
	  long m_LengthMs;

    //derived member
    double m_TrajectorySamplePeriod;
    double m_AnalogSamplePeriod;
    double m_VectogramSamplePeriod;

	  int m_NumTotTrajectories; 
	  int m_NumTrajectories; 
	  int m_NumAngles; 
	  int m_NumMoments; 
	  int m_NumPowers; 
	  int m_NumFrames; 
	  int m_NumChannels;
	  int m_NumSamples;
	  int m_NumEvents;
	  int m_NumPlatforms;

	  char *m_TrajectoryName;
	  char *m_ChannelName;
	  char *m_AngleName; 
	  char *m_MomentName;
	  char *m_PowerName;
	  char *m_EventContext;

	  char *m_TrajectoryUnit;
	  char *m_ChannelUnit;
	  char *m_AngleUnit;
	  char *m_MomentUnit;
	  char *m_PowerUnit;
  	
	  double m_X;
	  double m_Y;
	  double m_Z;

	  double m_AnalogValue;
    double m_EventValue;
  	
	  double m_CopX;
	  double m_CopY;
  	
	  double m_ForceX;
	  double m_ForceY; 
	  double m_ForceZ;
  	
	  double m_MomentX;
	  double m_MomentY;
	  double m_MomentZ;
  	
	  double m_CenterX;
	  double m_CenterY;
  };

  std::vector<_InternalC3DData> m_intData;
	
};
#endif
