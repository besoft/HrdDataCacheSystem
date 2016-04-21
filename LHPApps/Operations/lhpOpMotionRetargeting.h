/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMotionRetargeting.h,v $
  Language:  C++
  Date:      $Date: 2011-07-28 09:32:51 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Gerardo Gonzalez
==========================================================================
  Copyright (c) 2011
  University of Bedfordshire
=========================================================================*/

#ifndef __lhpOpMotionRetargeting_H__
#define __lhpOpMotionRetargeting_H__


//----------------------------------------------------------------------------
// Includes :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEGroup;
class medVMEJoint;
class mafMatrix3x3;

//----------------------------------------------------------------------------
// lhpOpMotionRetargeting :
//----------------------------------------------------------------------------
/** 
Motion Retargeting Class.
A class that uses a Kalman filter to perform motion retargeting from a set of VMEs 
representing an actor with a corresponding template motion.
*/

class LHP_OPERATIONS_EXPORT lhpOpMotionRetargeting: public mafOp
{
//typedef struct {
//   double elem[7];
// } Pose3D;

public:
	lhpOpMotionRetargeting(const wxString &label = "Motion Retargeting");
	~lhpOpMotionRetargeting(); 
	
    /*virtual*/ void OnEvent(mafEventBase *maf_event);

    mafTypeMacro(lhpOpMotionRetargeting, mafOp);

    mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Builds operation's interface. */
	void OpRun();

    /** Execute the operation. */
    /*virtual*/ void OpDo();

protected:
  
    /** Callback for VME_CHOOSE that accepts VMEJoints only */
    static bool AcceptVME(mafNode *node);

    ///**Extract the rotation part from a Matrix4x4 to a Matrix3x3*/
    //void ExtractRotation(const mafMatrix &source, mafMatrix3x3 &target);

    ///**Transform sample (sigma) points based on kinematic constraints*/
    //void KinematicConstraints(std::vector<Pose3D> samplePoints, std::vector<Pose3D> &measuredPoints);

    /** Check the source and target joints have the same hierarchy */
    bool HasSameHierarchy(medVMEJoint* pJointA, medVMEJoint *pJointB);

    /** Copy the bone directin (direction between adjacent joints) from the target joint to the source joint recursively */
    void CopyJointDirection(medVMEJoint* pSourceJoint, medVMEJoint* pTargetJoint);

    /** Set the time stamp for the source joint, and update to get the root position and the valid euler angles of joints */
    void GetSourceMotionParameters(mafTimeStamp timeStamp, std::vector<double>& motion);

    /** Perform the motion retargeting using Kalman-based motion retargeting algorithm */
    void MotionRetargeting();

    medVMEJoint *m_SourceJointRoot;   //<Source joint root (with time-varying motion)
    medVMEJoint *m_TargetJointRoot;   //<Target joint root (static)

    //int m_Nx;   ///<N dimensional random variable
    //int m_NoSigmaPts; ///<Number of sample sigma points
};
#endif
