/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpASFMotionRetargeting.h,v $
  Language:  C++
  Date:      $Date: 2012-07-28 09:32:51 $
  Version:   $Revision: 1.1.1.1 $
  Authors:   Yubo Tao
==========================================================================
  Copyright (c) 2012
  University of Bedfordshire

  The Bone, Posture, Skeleton and Motion Data Structures are from computer
  graphics course http://faculty.cs.tamu.edu/jchai/csce641/asf_amc_viewer.zip.
  Revision 1 - Steve Lin, Jan. 14, 2002
  Revision 2 - Alla and Kiran, Jan 18, 2002
  The motion retarget algorithm is based on Tak and Ko, A Physically-based 
  motion retargeting filter, ACM Transactions on Graphics, 24(1), 98-117, 2005.
=========================================================================*/

#ifndef __lhpOpASFMotionRetargeting_H__
#define __lhpOpASFMotionRetargeting_H__

#include "mafOp.h"
#include "vnl/vnl_double_3.h"
#include "lhpOperationsDefines.h"

#include <vector>

#define MAX_BONES_IN_ASF_FILE 256

// this structure defines the property of each bone segment, including its connection to other bones,
// DOF (degrees of freedom), relative orientation and distance to the outboard bone 
// http://www.scribd.com/doc/49641527/56/The-asf-File
struct Bone {
   
	struct Bone *sibling;		// Pointer to the sibling (branch bone) in the hierarchy tree 
	struct Bone *child;			// Pointer to the child (outboard bone) in the hierarchy tree 
   
	int idx;					// Bone index
	
	float dir[3];				// Unit vector describes the direction from local origin to 
								// the origin of the child bone 
								// Notice: stored in local coordinate system of the bone

	float length;				// Bone length  

	float axis_x, axis_y, axis_z;// orientation of each bone's local coordinate 
								 //system as specified in ASF file (axis field)

	int dof;					// number of bone's degrees of freedom 
	int dofx, dofy, dofz;		// degree of freedom mask in x, y, z axis (local)
	int doftx, dofty, doftz;
	int doftl;
								// dofx=1 if this bone has x degree of freedom, otherwise dofx=0.
	
    char name[256];
	// rotation matrix from the local coordinate of this bone to the local coordinate system of it's parent
	double rot_parent_current[4][4];
	
	//Rotation angles for this bone at a particular time frame (as read from AMC file) in local coordinate system, 
	//they are set in the setPosture function before dispay function is called
	float drx, dry, drz;
	float tx,ty,tz;
	float tl;
	int dofo[8];
};

//Root position and all bone rotation angles (including root) 
class Posture
{
//member variables
public:

	//Root position (x, y, z)		
	vnl_double_3 root_pos;								
		
	//Rotation (x, y, z) of all bones at a particular time frame in their local coordinate system.
	//If a particular bone does not have a certain degree of freedom, 
	//the corresponding rotation is set to 0.
	//The order of the bones in the array corresponds to their ids in .ASf file: root, lhipjoint, lfemur, ...
	vnl_double_3 bone_rotation[MAX_BONES_IN_ASF_FILE];
	vnl_double_3 bone_translation[MAX_BONES_IN_ASF_FILE];
	vnl_double_3 bone_length[MAX_BONES_IN_ASF_FILE];
};

class Skeleton {

//Member functions
public: 

	// The scale parameter adjusts the size of the skeleton. The default value is 0.06 (MOCAP_SCALE).
    // This creates a human skeleton of 1.7 m in height (approximately)
    Skeleton(const char *asf_filename, float scale);  
    ~Skeleton();                                

    //Get root node's address; for accessing bone data
    Bone* getRoot();

	//Get the bone by the index
	Bone* getBone(int index);

	//Set the skeleton's pose based on the given posture    
	void setPosture(Posture posture);        

	//Initial posture Root at (0,0,0)
	//All bone rotations are set to 0
    void setBasePosture();

	//Check whether the skeleton has the same hierarchy
	bool hasSameHierarchy(Skeleton *pSkeleton);

	//Calculate the leg length in the rest standing pose
	float calculateLegLength();

	//Calculate the positions of end effectors (tones) in the world coordinate system using the forward kinematic
	void calcEndEffectorPositionFK();

private:

	//parse the skeleton (.ASF) file	
    void readASFfile(const char* asf_filename, float scale);


	//This recursive function traverces skeleton hierarchy 
	//and returns a pointer to the bone with index - bIndex
	//ptr should be a pointer to the root node 
	//when this function first called
	Bone* getBone(Bone *ptr, int bIndex);
      

	//This function sets sibling or child for parent bone
	//If parent bone does not have a child, 
	//then pChild is set as parent's child
	//else pChild is set as a sibling of parents already existing child
	int setChildrenAndSibling(int parent, Bone *pChild);

	//Rotate all bone's direction vector (dir) from global to local coordinate system
	void RotateBoneDirToLocalCoordSystem();

	//Has the same hierarchy of the bone
	bool hasSameHierarchy(Bone *pBoneA, Bone *pBoneB);

//Member Variables
public:
	// root position in world coordinate system
    float m_RootPos[3];
	int tx,ty,tz;
	int rx,ry,rz;

	int name2idx(char *);
	char *idx2name(int);
	int NUM_BONES_IN_ASF_FILE;
	int MOV_BONES_IN_ASF_FILE;
	std::vector<double> endEffectorPos;

private:
	Bone *m_pRootBone;							// Pointer to the root bone, m_RootBone = &bone[0]
	Bone  m_pBoneList[MAX_BONES_IN_ASF_FILE];   // Array with all skeleton bones
};

class Motion 
{
//member functions 
public:
    //Include Actor (skeleton) ptr
    Motion(const char *amc_filename, float scale,Skeleton * pActor);
    //Use to creating motion from AMC file
    Motion(const char *amc_filename, float scale);
    //Use to create default motion with specified number of frames
    Motion(int nFrameNum);
	//delete motion
    ~Motion();

    // scale is a parameter to adjust the translational parameter
    // This value should be consistent with the scale parameter used in Skeleton()
    // The default value is 0.06
    int readAMCfile(const char* name, float scale);
    int writeAMCfile(const char* name, float scale);

	//Set all postures to default posture
	//Root position at (0,0,0), orientation of each bone to (0,0,0)
	void SetPosturesToDefault();

	//Set posture at spesified frame
	void SetPosture(int nFrameNum, Posture InPosture);
	int GetPostureNum(int nFrameNum);
	void SetTimeOffset(int n_offset);
	Posture* GetPosture(int nFrameNum);
	void SetBoneRotation(int nFrameNum, vnl_double_3 vRot, int nBone);
	void SetRootPos(int nFrameNum, vnl_double_3 vPos);

//data members
public:
    int m_NumFrames; //Number of frames in the motion 
	int offset;
	Skeleton * pActor;
	//Root position and all bone rotation angles for each frame (as read from AMC file)
    Posture* m_pPostures; 
};


//----------------------------------------------------------------------------
// lhpOpASFMotionRetargeting :
//----------------------------------------------------------------------------
/** 
Motion Retargeting Class using the ASF/AMC representation.
A class that uses a Kalman filter to perform motion retargeting from a skeleton
representing an actor A (ASF file) with a corresponding motion (AMC file) to another
skeleton representing another actor B (ASF file) to generate another motion accoresponding
to the acotor B.
*/

class LHP_OPERATIONS_EXPORT lhpOpASFMotionRetargeting: public mafOp
{
public:
	lhpOpASFMotionRetargeting(const wxString &label = "ASF Motion Retargeting");
	~lhpOpASFMotionRetargeting(); 
	
    /*virtual*/ void OnEvent(mafEventBase *maf_event);

    mafTypeMacro(lhpOpASFMotionRetargeting, mafOp);

    mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Builds operation's interface. */
	void OpRun();

    /** Execute the operation. */
    /*virtual*/ void OpDo();

protected:
    /** Scale the relative translation of the root node based on the leg length and copy the euler angles of bones */
    void SimpleMotionRetargeting(Skeleton *pActorA, Motion *pMotionA, Skeleton *pActorB, Motion *pMotionB);

    /** Kalman-filter based motion retargeting */
    void KalmanBasedMotionRetargeting(Skeleton *pActorA, Motion *pMotionA, Skeleton *pActorB, Motion *pMotionB);

    double    MOCAP_SCALE;  //< Scale of skeleton
    Skeleton *m_ActorA;     //< Actor A from a ASF file
    Skeleton *m_ActorB;     //< Actor B from another ASF file with the same local coordinate system and orientation with Actor A
    Motion   *m_MotionA;    //< Motion A for Actor A
    Motion   *m_MotionB;    //< Generated Motion B for Actor B
    wxString  m_MotionBFile;//< File to save the generated motion
};

#endif