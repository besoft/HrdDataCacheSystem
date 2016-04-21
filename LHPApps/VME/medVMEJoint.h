/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEJoint.h,v $
Language:  C++
Date:      $Date: 2012-03-20 15:32:05 $
Version:   $Revision: 1.1.2.4 $
Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011 University of West Bohemia
  See the COPYINGS file for license details 
=========================================================================*/
#ifndef __medVMEJoint_h
#define __medVMEJoint_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVME.h"
#include "lhpVMEDefines.h"

class mafGizmoTranslate;
class mafGizmoRotate;

/** medVMEJoint - a VME used to represent a joint in musculoskeletal model. 
It defines two segments it connects, where it is placed, type of connection, DOF, ... */
class LHP_VME_EXPORT medVMEJoint : public mafVME
{
public:	
	mafTypeMacro(medVMEJoint, mafVME);

protected:

#pragma region Joint Definition
	enum JOINT_LINK_IDS
	{
		LNK_MAIN_ACTOR = 0,    
		LNK_SECOND_ACTOR,    		

		LNK_LAST,
	};

	const static char* JOINT_LINK_NAMES[];    

	//mafVME* m_MainActorVME;						///<the input VME containing the main segment connected with this joint --- cannot be cached
	double m_MainActorCentroid[3];			///<relative position of mass centroid of the main actor

	//mafVME* m_SecondActorVME;						///<the input VME containing the second segment connected with this joint
	double m_SecondActorCentroid[3];		///<relative position of mass centroid of the second actor

	double m_AnchorPosition[3];					///<position of the joint relative to the main actor (or invalid, if any of actors is missing)
	int m_ValidAnchorRotateAxis[3];			///<Specifies the degree of freedom in this joint, which of the following axis are valid for rotation
																			///Valid values are: 0 = disable rotation, 1 = enable rotation. N.B., values 1, 0,  0 defines revolute joint( hinge). 
	double m_AnchorRotateAxis[3][3];		///<Axis for rotation, these must be of unit length and should form orthogonal basis	
	double m_AnchorRotateDefaults[3];		///<Zero angles for each rotate axis. The angle between vectors u1 = m_MainActorCentroid-m_AnchorPosition and 
																			///u2 = m_SecondActorCentroid - m_AnchorPosition is decomposed into three components,
																			///into flexion, adduction and rotation, using AnchorRotateAxis basis. In m_AnchorRotateDefaults 
																			///are stored values of these components that are considered to be in the rest-pose (zero angles).
																			///N.B. if both actors are already in their rest-position (and their centroids are, thus, also in their rest-pose
																			///position, the decomposed values matches these defaults.

	double m_AnchorRotateLimits[3][2];	///<Limits for rotation (given in +/- degrees regarding both Centroids), these are relative to m_AnchorRotateDefaults
																			///e.g., if Defaults[0] = 15 and Limits[0] = {-90,90}, then flexion is limited to -75,105 degrees

    double m_JointDirection[3];         ///<Direction to the parent joint to the child joint (this joint) in the parent local coordinate system 

	//double m_AnchorTranslateAxis[3][3];	///<Axis for translations --- these are not used at present
#pragma endregion //Joint Definition

	mafTransform *m_Transform;					///< pose matrix for the output (taken from input muscle)

	unsigned long m_LastUpdateTime;			///<time of the last update

#pragma region GUI
	enum JOINT_WIDGET_ID
	{		
		ID_SELECT_MAIN_ACTOR = Superclass::ID_LAST,
		ID_MA_VME_NAME,
		ID_MA_CENTROID_X,
		ID_MA_CENTROID_Y,
		ID_MA_CENTROID_Z,
		ID_MA_CENTROID_UPDATE,
		ID_MA_CENTROID_COMMIT,
		ID_MA_CENTROID_RESET,

		ID_SELECT_SECOND_ACTOR,
		ID_SA_VME_NAME,
		ID_SA_CENTROID_X,
		ID_SA_CENTROID_Y,
		ID_SA_CENTROID_Z,
		ID_SA_CENTROID_UPDATE,
		ID_SA_CENTROID_COMMIT,
		ID_SA_CENTROID_RESET,

		ID_ANCHOR_X,
		ID_ANCHOR_Y,
		ID_ANCHOR_Z,
		ID_ANCHOR_UPDATE,
		ID_ANCHOR_COMMIT,
		ID_ANCHOR_RESET,

		ID_AXIS1_ENABLE,
		ID_AXIS1_X,
		ID_AXIS1_Y,
		ID_AXIS1_Z,
		ID_AXIS1_UPDATE,
		ID_AXIS1_COMMIT,
		ID_AXIS1_RESET,
		ID_AXIS1_ZERO,
		ID_AXIS1_AUTO_ZERO, 
		ID_AXIS1_LIMIT_MIN,
		ID_AXIS1_LIMIT_MAX,
		
		ID_AXIS2_ENABLE,
		ID_AXIS2_X,
		ID_AXIS2_Y,
		ID_AXIS2_Z,
		ID_AXIS2_UPDATE,
		ID_AXIS2_COMMIT,
		ID_AXIS2_RESET,
		ID_AXIS2_ZERO,
		ID_AXIS2_AUTO_ZERO, 
		ID_AXIS2_LIMIT_MIN,
		ID_AXIS2_LIMIT_MAX,

		ID_AXIS3_ENABLE,
		ID_AXIS3_X,
		ID_AXIS3_Y,
		ID_AXIS3_Z,
		ID_AXIS3_UPDATE,
		ID_AXIS3_COMMIT,
		ID_AXIS3_RESET,
		ID_AXIS3_ZERO,
		ID_AXIS3_AUTO_ZERO,
		ID_AXIS3_LIMIT_MIN,
		ID_AXIS3_LIMIT_MAX,

		ID_LAST,
	};	
			
	wxButton* m_bttnMAUpdate;
	wxButton* m_bttnSAUpdate;
	wxButton* m_bttnMAAdjust;
	wxButton* m_bttnMAAdjustOK;
	wxButton* m_bttnMAAdjustReset;
	wxButton* m_bttnSAAdjust;
	wxButton* m_bttnSAAdjustOK;
	wxButton* m_bttnSAAdjustReset;
	wxButton* m_bttnAnchorAdjust;
	wxButton* m_bttnAnchorAdjustOK;
	wxButton* m_bttnAnchorAdjustReset;
	wxButton* m_bttnAxis1Adjust;
	wxButton* m_bttnAxis1AdjustOK;
	wxButton* m_bttnAxis1AdjustReset;	
	wxButton* m_bttnAxis2Adjust;
	wxButton* m_bttnAxis2AdjustOK;
	wxButton* m_bttnAxis2AdjustReset;
	wxButton* m_bttnAxis3Adjust;
	wxButton* m_bttnAxis3AdjustOK;
	wxButton* m_bttnAxis3AdjustReset;		
	wxButton *m_bttnAutoZeroAngle1;
	wxButton *m_bttnAutoZeroAngle2;
	wxButton *m_bttnAutoZeroAngle3;
	wxTextCtrl* m_txtMAVME;
	wxTextCtrl* m_txtSAVME;

	
	mafGizmoTranslate*	m_GizmoTranslate;
	mafGizmoRotate*			m_GizmoRotate;
#pragma endregion GUI


protected:
	/** Shows dialog (with the message in title) where the user selects vme.
	The VME that can be selected are defined by accept_callback.
	If no VME is selected, the routine returns false, otherwise it returns
	reference to the VME, its name and updates GUI */  
	bool SelectVme(mafString title, long accept_callback,
		mafVME*& pOutVME, mafString& szOutVmeName);

public:
	/** This method updates the output data structure bypassing 
	the datapipe approach inherited from mafVME*/
  inline /*virtual*/ void Update() 
	{
		//there is no VTK data pipe, so call updating routines directly
		InternalPreUpdate();
		InternalUpdate();
	}

protected:	
	/** Initializes the joint to default values */
	virtual void InitJoint();

	/** Create GUI for the VME */
	/*virtual*/ mafGUI *CreateGui();

	/** Stores this object into the storage element passed as argument. */
	/*virtual*/ int InternalStore(mafStorageElement *parent);

	/** Restores this object from the storage element passed as argument. */
	/*virtual*/ int InternalRestore(mafStorageElement *node);	  

	/** update the output data structure */
	/*virtual*/ void InternalUpdate();
	
	/** Return the list of timestamps for this VME. Timestamps list is 
	obtained merging timestamps for matrices and VME items*/
	/*virtual*/ void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes);

public:

	/** Given cos(theta) and sin(theta) values, it returns the angle theta in degrees. */
	static double ComputeAngle(double cos_theta, double sin_theta);

	/** Gets the main actor VME containing the main segment connected with this joint*/
	inline virtual mafVME* GetMainActorVME() {
		return mafVME::SafeDownCast(GetLink(JOINT_LINK_NAMES[LNK_MAIN_ACTOR]));
	}

	/** Gets the second actor VME containing the main segment connected with this joint*/
	inline virtual mafVME* GetSecondActorVME() {
		return mafVME::SafeDownCast(GetLink(JOINT_LINK_NAMES[LNK_SECOND_ACTOR]));
	}

	/** Sets the main actor VME containing the main segment connected with this joint*/
	inline virtual void SetMainActorVME(mafVME* vme) 
	{
		if (vme == NULL)
			RemoveLink(JOINT_LINK_NAMES[LNK_MAIN_ACTOR]);
		else
			SetLink(JOINT_LINK_NAMES[LNK_MAIN_ACTOR], vme);
	}

	/** Sets the second actor VME containing the main segment connected with this joint*/
	inline virtual void SetSecondActorVME(mafVME* vme) 
	{
		if (vme == NULL)
			RemoveLink(JOINT_LINK_NAMES[LNK_SECOND_ACTOR]);
		else
			SetLink(JOINT_LINK_NAMES[LNK_SECOND_ACTOR], vme);
	}

	/** Computes automatically the centroid  (mass centre) of the main actor. */
	inline void ComputeMainActorCentroid(){
		ComputeActorCentroid(GetMainActorVME(), m_MainActorCentroid);
	}

	/** Computes automatically the centroid  (mass centre) of the second actor. */
	inline void ComputeSecondActorCentroid(){
		ComputeActorCentroid(GetSecondActorVME(), m_SecondActorCentroid);
	}

	/** Gets the current main actor centroid.
	The returned coordinates are relative to the origin of the reference system of  the main actor. 
	and they are valid guaranteed to be only, if the main actor is specified. 
	The method returns true, if the main actor is specified (false otherwise). */
	inline bool GetMainActorCentroid(double* out) {
		return GetActorCoords(GetMainActorVME(), m_MainActorCentroid, out, false);	
	}

	/** Gets the current second actor centroid.
	The returned coordinates are relative to the origin of the reference system of the second actor 
	and they are guaranteed to be valid only, if the second actor is specified.
	The method returns true, if the second actor is specified (false otherwise). */
	inline bool GetSecondActorCentroid(double* out) {
		return GetActorCoords(GetSecondActorVME(), m_SecondActorCentroid, out, false);
	}

	/** Gets the current main actor centroid.
	The returned coordinates are given in the world space, i.e., abs space. 
	and they are guaranteed to be  valid only, if the main actor is specified.
	The method returns true, if the main actor is specified (false otherwise).  */
	inline bool GetMainActorAbsCentroid(double* out) {
		return GetActorCoords(GetMainActorVME(), m_MainActorCentroid, out, true);
	}

	/** Gets the current second actor centroid.
	The returned coordinates are given in the world space, i.e., abs space.   
	and they are guaranteed to be valid only, if the second actor is specified.
	The method returns true, if the second actor is specified (false otherwise). */
	inline bool GetSecondActorAbsCentroid(double* out) {
		return GetActorCoords(GetSecondActorVME(), m_SecondActorCentroid, out, true);
	}

	/** Sets the main actor centroid.
	The  coordinates must be relative to the origin of the reference system of  the main actor. 	
	The method returns true, if the coordinates were successfully set, false otherwise. */
	inline bool SetMainActorCentroid(const double* inc) {
		return SetActorCoords(GetMainActorVME(), inc, m_MainActorCentroid, false);	
	}

	/** Sets the second actor centroid.
	The  coordinates must be relative relative to the origin of the reference system of the second actor 	
	The method returns true, if the coordinates were successfully set, false otherwise. */
	inline bool SetSecondActorCentroid(const double* inc) {
		return SetActorCoords(GetSecondActorVME(), inc, m_SecondActorCentroid, false);
	}

	/** Sets the main actor centroid.
	The coordinates must be given in the world space, i.e., abs space. 	
	The method returns false, if the main actor is not specified and, thus, coordinates cannot be set.  */
	inline bool SetMainActorAbsCentroid(const double* inc) {
		return SetActorCoords(GetMainActorVME(), inc, m_MainActorCentroid, true);
	}

	/** Sets the second actor centroid.
	The coordinates must be given in the world space, i.e., abs space. 	
	The method returns false, if the second actor is not specified and, thus, coordinates cannot be set.  */
	inline bool SetSecondActorAbsCentroid(const double* inc) {
		return SetActorCoords(GetSecondActorVME(), inc, m_SecondActorCentroid, true);
	}

	/** Gets the coordinates of the joint position. 
	The returned coordinates are local and relative to the main actor VME. 
	The method returns false, if the coordinates could not be retrieved.*/
	inline bool GetJointPosition(double* out) {
		return GetActorCoords(GetMainActorVME(), m_AnchorPosition, out, false);
	}

	/** Gets the coordinates of the joint position. 
	The returned coordinates are global (these are world coordinates). 
	The method returns false, if the coordinates could not be retrieved.*/
	inline bool GetJointAbsPosition(double* out) {
		return GetActorCoords(GetMainActorVME(), m_AnchorPosition, out, true);
	}

	/** Sets the coordinates of the joint position. 
	The input coordinates are local and must be relative to the main actor VME. 
	The method returns false, if the coordinates could not be set.*/
	inline bool SetJointPosition(const double* inc) {
		return SetActorCoords(GetMainActorVME(), inc, m_AnchorPosition, false);
	}

	/** Sets the coordinates of the joint position. 
	The input coordinates are global (these are world coordinates). 
	The method returns false, if the coordinates could not be set.*/
	inline bool SetJointAbsPosition(const double* inc) {
		return SetActorCoords(GetMainActorVME(), inc, m_AnchorPosition, true);
	}

	/** Computes the position of the joint from the specified actors. 
	Joint is supposed to be between both segments. 
	Return false, if the joint position could not be computed, e.g.,
	because one of actors is invalid. */
	virtual bool ComputeJointPosition();

	/** Gets the vector set for the given axis (0,1 or 2). 
	Returns false, if the axis is invalid. */
	inline bool GetJointRotationAxis(int axis, double* vector) 
	{
		if (axis < 0 || axis > 2 || m_ValidAnchorRotateAxis[axis] == 0)
			return false;

		for (int i = 0; i < 3; i++) {
			vector[i] = m_AnchorRotateAxis[axis][i];
		}
		return true;
	}

	/** Sets the axis vector for the given axis (0,1 or 2). 
	N.B. vector may be NULL, if the axis should be invalidated (NOT used at all).
	Returns false, if the axis is invalid (not invalidated). */
	bool SetJointRotationAxis(int axis, const double* vector); 

	/** Gets the default (rest-pose) position of the joint in the given axis
	Returns false, if the axis is invalid. */
	inline bool GetJointRotationAxisZero(int axis, double* zero) 
	{
		if (axis < 0 || axis > 2 || m_ValidAnchorRotateAxis[axis] == 0)
			return false;

		*zero = m_AnchorRotateDefaults[axis];
		return true;
	}

	/** Sets the default (rest-pose) position of the joint in the given axis
	Returns false, if the axis is invalid. */
	inline bool SetJointRotationAxisZero(int axis, double zero) 
	{
		if (axis < 0 || axis > 2 || m_ValidAnchorRotateAxis[axis] == 0)
			return false;

		if (zero != m_AnchorRotateDefaults[axis]) 
		{
			m_AnchorRotateDefaults[axis] = zero;
			this->Modified();
		}
		return true;
	}

	/** Gets the limit angles of the joint in the given axis
	Returns false, if the axis is invalid. */
	inline bool GetJointRotationAxisLimits(int axis, double* limits) 
	{
		if (axis < 0 || axis > 2 || m_ValidAnchorRotateAxis[axis] == 0)
			return false;
		
		limits[0] = m_AnchorRotateLimits[axis][0];		
		limits[1] = m_AnchorRotateLimits[axis][1];		
		return true;
	}

	/** Sets the limit angles  of the joint in the given axis
	Returns false, if the axis is invalid. */
	inline bool SetJointRotationAxisLimits(int axis, const double* limits) 
	{
		if (axis < 0 || axis > 2 || m_ValidAnchorRotateAxis[axis] == 0)
			return false;

		if (limits[0] != m_AnchorRotateLimits[axis][0] ||
			limits[1] != m_AnchorRotateLimits[axis][2]) 
		{
			m_AnchorRotateLimits[axis][0] = limits[0];		
		  m_AnchorRotateLimits[axis][1] = limits[1];	
			this->Modified();
		}
		return true;
	}
	

	/** Copy the contents of another VME-Joint into this one. */
	/*virtual*/ int DeepCopy(mafNode *a);

	/** Compare with another VME-Joint. */
	/*virtual*/ bool Equals(mafVME *vme);

	/** Return the suggested pipe-typename for the visualization of this vme */
	/*virtual*/ mafString GetVisualPipe() {
		return mafString("medPipeJoint");
	};

	/**
	Set the Pose matrix of the VME. This function modifies the MatrixVector. You can
	set or get the Pose for a specified time. When setting, if the time does not exist
	the MatrixVector creates a new KeyMatrix on the fly. When getting, the matrix vector
	interpolates on the fly according to the matrix interpolator.*/
	/*virtual*/ void SetMatrix(const mafMatrix &mat);

	/** Process events coming from other objects */ 
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

    /** Update the scalar values on the polydata.*/
    void SetTimeStamp(mafTimeStamp t);

    /** Update the scalar values on the polydata and its children joints.*/
    void SetTimeStampRecursive(mafTimeStamp t);

    /** Init the bone directions for children joints */
    void InitJointDirection();

    /** Get joint direction */
    void GetJointDirection(double dir[3]) {
        dir[0] = m_JointDirection[0];
        dir[1] = m_JointDirection[1];
        dir[2] = m_JointDirection[2];
    }

    /** Set joint direction */
    void SetJointDirection(double dir[3]) {
        m_JointDirection[0] = dir[0];
        m_JointDirection[1] = dir[1];
        m_JointDirection[2] = dir[2];
    }

    /** Get the number of valid axis recursively */
    int GetNumberOfValidAxis();

    /** Get the number of end-effectors recursively */
    int GetNumberOfEndEffectors();

    /** Get all pre-computed positions of end effectors recursively */
    void GetEndEffectorPosition(std::vector<double>& endEffectorPos);

    /** Calculate all positions of end effectors using the output's euler angles recursively */
    void CalcEndEffectorPosition(std::vector<double>& endEffectorPos);

    /** Get the angle parameters recursively */
    void GetRotationParameters(std::vector<double>& angles);

    /** Set the position of the root and the angle parameters recursively  */
    void SetMotionParameters(std::vector<double>& positions);

    /** Set the angle parameters recursively */
    void SetRotationParameters(std::vector<double>& angles, int index); 

protected:
	/** This internal method gets the validated coordinates for the given actorVME.	
	General coordinates, given in  in_coord, e.g., centroid, which are supposed to be relative to
	the origin of the reference system of actorVME, are, if bGetAbsCoords is true,
	transformed into the world coordinates and then stored into out_coord point buffer.
	If bGetAbsCoords is false, the local coordinates from in_coord are simply copied
	into out_coord. The returned coordinates are guaranteed to be valid only, if the 
	actorVME is not NULL. In this case the method returns true (false otherwise). */
	bool GetActorCoords(mafVME* actorVME, const double* in_coord, 
		double* out_coord, bool bGetAbsCoords);	

	/** This internal method sets the coordinates, e.g., centroid, for the given actorVME.	
	The input coordinates are passed in in_coord and are either local (relative to the	
	the origin of the reference system of actorVME) - in this case bInIsAbsCoords must be false,
	or global (world coordinates) - in this case bInIsAbsCoords must be true. If the 
	coordinates are global, they are transformed into local space of actorVME.
	The final coordinates are stored into out_coord + Modified method is called. 
	If the transformation fails (e.g., because actorVME is NULL), the method
	returns false, otherwise true. */
	bool SetActorCoords(mafVME* actorVME, const double* in_coord, 
		double* out_coord, bool bInIsAbsCoords);	

	/** Computes the centroid of the given actor.
	The coordinates are local (relative to the origin of reference system of actorVME).
	The method returns false, if actorVME is NULL.*/
	virtual bool ComputeActorCentroid(mafVME* actorVME, double* out_centroid); 

	/** Gets the abs matrix for the given VME that can be used to
	transform points from Local Coordinates to World and vice versa */
	const mafMatrix* GetVMEAbsMatrix(mafVME* vme);

	/** Gets the world coordinates of centres of sides of the bounding box of the given vme. 
	N.B. vme may not by NULL */
	void GetBBoxAbsSideCentres(mafVME* vme, double out[6][3]);

	/** Transforms the coordinates in_coord into output coordinates as defined by the absolute matrix transformation of vme.
	Parameter bLocal2Abs determines, if the transformation is from local to abs (world) coordinate or vice versa*/
	void TransformPoint(const double in_coord[3], double out_coord[3], mafVME* vme, bool bLocal2Abs);

	/** Computes the angle (given in degrees) that is between vectors u and v in projection to the direction w. 
	In other words: it returns angle needed to be used in transform matrices for rotation around vector w to transform u to v*/
	double ComputeAngleBetweenVectors(const double* u, const double* v, const double* w) const;

#pragma region GUI
	/** Updates the name of actor VME shown in the given ctrl. 
	Returns true, if the actor VME is valid, otherwise false. */
	virtual bool UpdateActorNameCtrl(wxTextCtrl* ctrl, mafVME* vme);

	/** Initializes translation GIZMO. After that ID_TRANSFORM is sent from gizmo to update the position. 
	The original absolute position of the point to translate is given as the first parameter.*/
	virtual void BeginGizmoTranslate(const double* abs_startpos);
	
	/** Processes events from the gizmo (send in the source) */
	virtual void ProcessGizmo(mafEventBase *maf_event);

	/** Terminates translation GIZMO storing the final position into abs_end_pos. 
	N.B. abs_endpos may by NULL, if this information is not required.*/
	virtual void EndGizmoTranslate(double* abs_endpos);

	/** Initializes rotation GIZMO. After that ID_TRANSFORM is sent from gizmo to update the position. 
	The original absolute position of the origin is given as the first parameter. Vector to be rotated is
	given as vector (the second parameter). */
	virtual void BeginGizmoRotate(const double* abs_startpos, const double* vector);

	/** Terminates rotation GIZMO storing the final vector into vector. 
	N.B. vector may by NULL, if this information is not required.*/
	virtual void EndGizmoRotate(double* vector);

	/** Handles the SELECT_ACTOR event. Shows a dialog for selecting a new actor VME by the user.
	bMainActor is true, if the main actor is to be changed, false otherwise*/
	virtual void OnUpdateActor(bool bMainActor);
	
	/** Handles ID_ANCHOR_UPDATE event. Shows translate gizmo for the anchor. */
	virtual void OnUpdateAnchor();

	/** Handles ID_ANCHOR_COMMIT event. Hides translate gizmo for the anchor + stores changes. */
	virtual void OnCommitAnchor();

	/** Handles ID_ANCHOR_RESET event. Hides translate gizmo for the anchor. */
	virtual void OnResetAnchor();

	/** Handles ID_XA_CENTROID_UPDATE event. Shows translate gizmo for the centroid of actor. */
	virtual void OnUpdateActorCentroid(bool bMainActor);

	/** Handles ID_XA_CENTROID_COMMIT event. Hides translate gizmo for the centroid + stores changes. */
	virtual void OnCommitActorCentroid(bool bMainActor);

	/** Handles ID_XA_CENTROID_RESET event. Hides translate gizmo for the centroid. */
	virtual void OnResetActorCentroid(bool bMainActor);

	/** Handles ID_AXISx_UPDATE event. Shows rotate gizmo for the given axis. */
	virtual void OnUpdateAxis(int axis);

	/** Handles ID_AXISx_COMMIT event. Hides rotate gizmo for the given axis and stores the changes.  */
	virtual void OnCommitAxis(int axis);

	/** Handles ID_AXISx_RESET event. Hides rotate gizmo for the given axis. */
	virtual void OnResetAxis(int axis);

	/** Handles ID_AXISx_ENABLE event. Enables or disables GUI controls for the given axis. */
	virtual void OnEnableAxis(int axis);

	/** Handles ID_AXISx_AUTO_ZERO event. */
	virtual void OnAutoZeroAngle(int axis);

	/** Updates the visibility of Adjust, Commit and Reset buttons for the anchor*/
	void UpdateAnchorButtons(bool bEnableCommitReset);

	/** Updates the visibility of Adjust, Commit and Reset buttons for the centroids*/
	void UpdateCentroidButtons(bool bMainActor, bool bEnableCommitReset);

	/** Updates the visibility of Adjust, Commit and Reset buttons for the given axis*/
	void UpdateAxisButtons(int axis, bool bEnableCommitReset);

	/** Enables/disables adjust buttons*/
	void EnableAdjustButtons(bool Enable);
#pragma endregion GUI

public:
	/** return icon */
	static char** GetIcon();

protected:
	/** Accepts target VME for segments*/
	static bool AcceptActorVME(mafNode *node);	

protected:
	medVMEJoint();
	virtual ~medVMEJoint();	

private:
	medVMEJoint(const medVMEJoint&); // Not implemented
	void operator=(const medVMEJoint&); // Not implemented		
};


#endif //__medVMEJoint_h
