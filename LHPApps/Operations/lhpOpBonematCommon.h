/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonematCommon.h,v $
  Language:  C++
  Date:      $Date: 2014-10-07 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.4 $
  Authors:   Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpBonematCommon_H__
#define __lhpOpBonematCommon_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "mafNode.h"
#include "mafVMEVolumeGray.h"
#include "mafGUIRollOut.h"
#include "lhpOperationsDefines.h"
#include <iostream>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class vtkTransform;
class wxBusyInfo;

enum RhoSelection
{
  USE_RHO_QCT,
  USE_RHO_ASH,
};

enum DensitySelection
{
  USE_MEAN_DENSISTY,
  USE_MAXIMUM_DENSITY,
};

typedef struct calibrationStruct
{
	//---------------------RhoQCTFromHU-----------------
	/*rho = a + b * HU*/
	double rhoIntercept;
	double rhoSlope;

	//---------------------RhoQCTFromHU-----------------
	//rho calibration
	int rhoCalibrationCorrectionIsActive;
	int rhoCalibrationCorrectionType;
	//rho interval
	double rhoQCT1, rhoQCT2;
	//single interval rho calibration
	double a_CalibrationCorrection, b_CalibrationCorrection;
	//three intervals rho calibration
	double a_RhoQCTLessThanRhoQCT1, b_RhoQCTLessThanRhoQCT1;
	double a_RhoQCTBetweenRhoQCT1AndRhoQCT2, b_RhoQCTBetweenRhoQCT1AndRhoQCT2;
	double a_RhoQCTBiggerThanRhoQCT2, b_RhoQCTBiggerThanRhoQCT2;

	//---------------YoungModuleFromRho-----------------
	int densityIntervalsNumber;
	double rhoAsh1, rhoAsh2;
	//single interval rho calibration
	double a_OneInterval,b_OneInterval,c_OneInterval;
	//three intervals rho calibration
	double a_RhoAshLessThanRhoAsh1,b_RhoAshLessThanRhoAsh1,c_RhoAshLessThanRhoAsh1;
	double a_RhoAshBetweenRhoAsh1andRhoAsh2,b_RhoAshBetweenRhoAsh1andRhoAsh2,c_RhoAshBetweenRhoAsh1andRhoAsh2;
	double a_RhoAshBiggerThanRhoAsh2,b_RhoAshBiggerThanRhoAsh2,c_RhoAshBiggerThanRhoAsh2;

	double minElasticity;
} Calibration;

//----------------------------------------------------------------------------
// lhpOpBonematCommon :
//----------------------------------------------------------------------------
/** Operation to map CT Volume properties on the operation input finite element mesh*/
class LHP_OPERATIONS_EXPORT lhpOpBonematCommon: public mafOp
{
public:

  /** Set the source volume for mapping*/
  void SetSourceVolume(mafVMEVolumeGray *volume) {m_InputVolume = volume;};
  mafVMEVolumeGray *GetVolume() {return m_InputVolume;};

  /** Fill vars from configuration file */
  int LoadConfigurationFile(const char *configurationFileName);
  /** Fill vars from configuration file.txt */
  int LoadConfigurationTxtFile(const char *configurationFileName);
  /** Fill vars from configuration file.xml */
  int LoadConfigurationXmlFile(const char *configurationFileName);

  /** Set the Output Frequency file name */
  void SetFrequencyFileName(const char* name);  

  /** Get the Output Frequency file name */
  const char* GetFrequencyFileName();

  /** Set the execution modality, Default is HU integration */
  void SetYoungModuleCalculationModalityToHUIntegration() {m_YoungModuleCalculationModality = HU_INTEGRATION;};
  /** Set the execution modality, Default is E integration */
  void SetYoungModuleCalculationModalityToYoungModuleIntegration() {m_YoungModuleCalculationModality = YOUNG_MODULE_INTEGRATION;};
  
  /** Execute the procedure that maps TAC values on the finite element mesh; 
  Beware: input mafVMEMesh data is changed in place*/
  virtual int Execute() = 0;

  /** Create the operation graphical user interface*/
  void CreateGui();

  static bool VolumeAccept(mafNode* node) {return(node != NULL && node->IsMAFType(mafVMEVolumeGray));};

  /** Overridden to accept mafVMEMesh only */
  bool Accept(mafNode *node);

  /** Builds operation's interface. */
  void OpRun();

  lhpOpBonematCommon(wxString label);

  ~lhpOpBonematCommon(); 

  void OnEvent(mafEventBase *maf_event);

	mafNode *GetOutput();;

protected:

  /** Execute the procedure that maps TAC values on the finite element mesh:
  DENSITY: FROM_DATASET == old version
  YOUNG MODULE: FROM_DENSITY == old version
  */
  int Integration();

  /** Execute the procedure that maps TAC values on the finite element mesh:
  DENSITY: FROM_DATASET == old version
  YOUNG MODULE: FROM_DATASET  
  */
  int YoungModuleIntegration();

  int SaveConfigurationFile(const char *configurationFileName);

  /** Set the configuration file name */
  void SetConfigurationFileName(const char* name);  

  /** Get the configuration file name */
  const char* GetConfigurationFileName();


  /** Read configuration file and fill in member variables */
  int OpenConfigurationFile();

  /** Write the configuration file */
  int SaveConfigurationFileAs();

  bool CheckNodeElement(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *elementName);
  mafString GetElementAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *attributeName);
  double lhpOpBonematCommon::GetDoubleElementAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *attributeName);

  void OnOpenConfigurationFileButton();
  void OnSaveConfigurationFileButton();
  void OnSaveConfigurationFileAsButton();
  void OnOpenInputTacButton();
  void OnOutputFrequencyFileName();
  void OnExecute();

  void UpdateGuiConfiguration();

  void VolumeSelection();

  void CreateMeshCopy();

  enum
  {
    POINTS_COORDINATES = 0,
    INTERCEPT_SLOPE = 1,
  };

  enum
  {
    SINGLE_INTERVAL = 0,
    THREE_INTERVALS = 1,
    INTERVALS_NUMBER,
  };

  enum 
  {
    HU_INTEGRATION = 0,
    YOUNG_MODULE_INTEGRATION = 1,
    INTEGRATION_MODALITIES_NUMBER,
  };



  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);

  void InitProgressBar(wxString label);
  void CloseProgressBar();
  void UpdateProgressBar(long progress);

  mafString m_ConfigurationFileName;
  mafString m_InputVolumeName;
  mafString m_FrequencyFileName;
 
  Calibration m_Calibration;

  int m_StepsNumber;
  double m_Egap;

  int m_YoungModuleCalculationModality;

  // Advanced Configuration
  int m_RhoSelection;
  int m_DensitySelection;
  double m_PoissonRatio;
  long m_OperationProgress;

  // my inclusions SUBSTITUTION
  mafVMEVolumeGray *m_InputVolume;
  mafVMEMesh *m_OutputMesh;
  mafVME   *m_Vme; 

  // GUI elements
  wxBusyInfo *m_BusyInfo;
  mafGUIRollOut *m_GuiRollOutCalibrationOneInterval;
  mafGUIRollOut *m_GuiRollOutCalibrationThreeIntervals;
  mafGUIRollOut *m_GuiRollOutDensityOneInterval;
  mafGUIRollOut *m_GuiRollOutDensityThreeIntervals;
  mafGUIRollOut *m_GuiRollOutAdvancedConfig;

  mafGUI *m_GuiASCalibrationOneInterval;
  mafGUI *m_GuiASCalibrationThreeIntervals;
  mafGUI *m_GuiASDensityOneInterval;
  mafGUI *m_GuiASDensityThreeIntervals;
  mafGUI *m_GuiASAdvancedConfig;

  // friend test
  friend class lhpOpBonematCommonTest;
};
#endif
