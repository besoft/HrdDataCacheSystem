/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpConvertMotionData.h,v $
  Language:  C++
  Date:      $Date: 2011-04-11 07:18:22 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpConvertMotionData_H__
#define __lhpOpConvertMotionData_H__

#include "lhpOpCopyMusculoskeletalModel.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"

#include "optimization.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMESurface;
class mafVMELandmarkCloud;
class medVMEMusculoSkeletalModel;

//----------------------------------------------------------------------------
// lhpOpConvertMotionData :
//----------------------------------------------------------------------------
/** This operation converts motion data given as time-variant landmark cloud (where
each landmark corresponds to one body marker) into a set of time-variant joints
positions (and rotations). It calculates the estimated joints positions using the method
described in AG. Kirk, JF. O'Brian, DA. Forsyth: Skeletal Parameter Estimation from Optical Motion Capture Data,
In: Proceedings of IEEE Conference on Computer Vision and Pattern Recognition, 2005, 782-788
and uses ICP registration (with similarity) to find the orientation of joints taking joints
and various landmark specified on bones (in the atlas) as inputs.*/
class LHP_OPERATIONS_EXPORT lhpOpConvertMotionData: public lhpOpCopyMusculoskeletalModel
{
protected:	
	mafVME* m_MotionDataVME;								///< Pointer to the VME with the motion data
	mafVME* m_MusculoskeletalModelVME;			///< Pointer to the VME with the musculoskeletal model
	bool  m_bEstimateJoints;								///<true, if joints should be estimated
	bool	m_bPreserveEstimatedJoints;				///<true, if estimated joints are to be preserved	
	double m_JointsWeight;									///<weight for the joints matching
	int m_RegistrationMode;									///<Registration mode (RIGID, SIMILARITY, AFFINE) for medOpRegisterClusters

public:
  lhpOpConvertMotionData(const wxString &label = "Convert Motion Data");
  ~lhpOpConvertMotionData(); 

  mafTypeMacro(lhpOpConvertMotionData, lhpOpCopyMusculoskeletalModel);	

  /*virtual*/ mafOp* Copy();

	/** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

	/*Sets a new musculoskeletal mode VME
	NB. you need to call OpDo to confirm the changes. */
	inline virtual void SetMusculoskeletalModelVME(mafVME* vme) {
		m_MusculoskeletalModelVME = vme;
	}

	/*Sets a new MotionData VME
	NB. you need to call OpDo to confirm the changes. */
	virtual void SetMotionDataVME(mafVME* vme);

	/** Sets new ICP registration mode. 
	Valid options are RIGID, SIMILARITY, AFFINE - see medOpRegisterCulsters::ID_REGISTER_CLUSTERS enum*/
	inline void SetRegistrationMode(int mode) {
		m_RegistrationMode = mode;
	}

	/** Gets the current ICP registration mode. */
	inline int GetRegistrationMode() {
		return m_RegistrationMode;
	}
	
	/** Enables/Disables estimation of joints from the input motion data
	and their use in the registration process. */
	inline void EstimateJoints(bool enable) {
		m_bEstimateJoints = enable;
	}

	/** Gets whether estimation of joints from the input motion data is enabled. */
	inline bool IsJointsEstimationEnabled() {
		return m_bEstimateJoints;
	}

	/** Enables/Disables storing of estimated joints in the motion data. */
	inline void StoreEstimatedJoints(bool enable) {
		m_bPreserveEstimatedJoints = enable;
	}

	/** Gets whether estimated joints from the input motion data should be stored. */
	inline bool IsStoringOfEstimatedJointsEnabled() {
		return m_bPreserveEstimatedJoints;
	}
	
	/** Sets the weight of joints in ICP registration. */
	inline void GetJointWeight(double weight) {
		m_JointsWeight = weight;
	}

	/** Gets the weight of joints in ICP registration. */
	inline double GetJointWeight() {
		return m_JointsWeight;
	}

protected:
	/** Fuses the specified atlas with the given motion. */
	virtual void FuseMotion(medVMEMusculoSkeletalModel* vmeModel2, mafVMELandmarkCloud* vmeMotion);

	/** Fuses the specified region with the motion data */
	virtual void FuseMotion(mafVMELandmarkCloud* vmeSource, mafVMELandmarkCloud* vmeMotion, mafVMESurface* vmeTarget);

	/** Callback for VME_CHOOSE that accepts Musculoskeletal atlas only */
  static bool AcceptMusculoskeletalAtlas(mafNode *node);	
	
	/** Callback for VME_CHOOSE that accepts landmarkclouds only */
  static bool AcceptLandmarkClouds(mafNode *node);

#pragma region Joint Centres Estimation
//See: AG. Kirk, JF. O'Brian, DA. Forsyth: Skeletal Parameter Estimation from Optical Motion Capture Data,
//In: Proceedings of IEEE Conference on Computer Vision and Pattern Recognition, 2005, 782-788
	
protected:
	/** Joint descriptor. */
	typedef struct JNT_INFO
	{
		int JointType;							///<one of medMSMGraph::JointEnum values
		const char* JointTypeName;	///<name of JointType

		const char** Landmarks;			///<names of landmarks defining this joint
	} JNT_INFO;

	/** Gets the array of joints ended by {-1, NULL} entry. */
	static const JNT_INFO* GetJointsDesc();

	/** Estimates joint positions (at each frame) for the given landmark cloud and adds these positions into the cloud.
	Call RemoveEstimatedJoints method to remove these positions. */
	virtual void AddEstimatedJoints(mafVMELandmarkCloud* target);

	/** Remove landmarks created by AddEstimatedJoints method. */
	virtual void RemoveEstimatedJoints(mafVMELandmarkCloud* target);

	/** Adds centres of all joints (from listJoint) connecting the given region with others as landmarks into source landmark cloud.
	Call RemoveJointsForRegion method to remove these landmarks. */
	virtual void AddJointsForRegion(int region, medMSMGraph::MSMGraphNodeList& listJoints, mafVMELandmarkCloud* source);
	
	/** Removes landmarks created by AddJointsForRegion method. */
	virtual void RemoveJointsForRegion(int region, mafVMELandmarkCloud* source);

private:

	

	typedef double VCoord[3];
	typedef struct OPT_INFO
	{
		VCoord* solution;	//solution for each frame

		VCoord* markers;	//all markers in all frames, structure: [Frame 1 = {Point 1, Point 2, ...}], [Frame 2]	
		bool* validity;		//validity of each marker in each frame	 (marker is invalid in a frame, if it is occluded)
		int* validframes;	//number of frames in which each marker is present (i.e., valid)

		int numMarkers;	//number of markers
		int numFrames;	//number of Frames	
	} OPT_INFO;

	/** Calculates the average distance (through all time frames) between a marker with ID m and a centroid.
	Coordinates of the centroid in every frame are given in c, the coordinates of marker are in pInfo. */
	static double CalcDCM(OPT_INFO* pInfo, const VCoord* c, int m);

	/** Calculates the variance in distance calculated by CalcDCM method.
	Coordinates of the centroid in every frame are given in c, the coordinates of marker are in pInfo. 
	Calculated average distance is passes in d_cm parameter.*/
	static double CalcSigmaCM(OPT_INFO* pInfo, const VCoord* c, int m, double d_cm);

	/** Calculates the partial derivative of CalcDCM function for c[i][j] variable of the centroid.
	Parameters i and j denote frame number and coordinate dimension (0-2), respectively. */
	static double CalcDCM_PD(OPT_INFO* pInfo, const VCoord* c, int m, int i, int j);

	/** Calculates the partial derivative of CalcSigmaCM function for c[i][j] variable of the centroid.
	Parameters i and j denote frame number and coordinate dimension (0-2), respectively. 
	Partial derivative of CalcDCM function for c[i][j] is given in d_cm_PD. */
	static double CalcSigmaCM_PD(OPT_INFO* pInfo, const VCoord* c, int m, double d_cm, double d_cm_PD, int i, int j);
	
	/** Calculates joint cost function and its gradient.
	This is a callback function called from alglib non-linear optimization solver. */
	static void JointCostFunction(const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad, void *ptr);
#pragma endregion

protected:	
	/** Returns true, if the VME represented by the given msm_node should be DeepCopied*/
	/*virtual*/ bool CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node);	
};

#endif //__lhpOpConvertMotionData_H__