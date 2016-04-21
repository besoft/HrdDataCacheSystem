/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medVMEOutputJoint.h,v $
  Language:  C++
  Date:      $Date: 2012-04-03 13:44:36 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011-2012
  University of West Bohemia
=========================================================================*/
#ifndef __medVMEOutputJoint_h
#define __medVMEOutputJoint_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVMEOutput.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class vtkDataSet;


/**
medVMEOutputJoint produces the most important information about joints,
e.g., about the joint specified by the medVMEJoint.
*/
class LHP_VME_EXPORT medVMEOutputJoint : public mafVMEOutput
{
protected:
	vtkDataSet* m_JointOutput;

	double m_AbsPosition[3];				///<position of the joint (in coordinates of this space)

	int m_ValidRotateAxes;				///<Number of valid joint axis
	double m_RotateAxis[3][3];		///<Axis for rotation, these must be of unit length and should form orthogonal basis
	
	double m_ActorsAxis[2][3];		///<Principal axis for both actors
	double m_CurrentAngles[3];		///<Rotational angles (around Rotate Axis)
	double m_LimitAngles[3][2];		///<Limits for rotation (given in +/- degrees regarding both Centroids)

#pragma region GUI
	mafString m_szPosition;
	mafString m_szAxes;
	mafString m_szActors;
	mafString m_szAngles;
	mafString m_szLimits;
#pragma endregion

public:
	medVMEOutputJoint();
	virtual ~medVMEOutputJoint();

	mafTypeMacro(medVMEOutputJoint,mafVMEOutput);
	
	/**
    Returns a VTK dataset corresponding to the current time.  */
  /*virtual*/ vtkDataSet* GetVTKData();

	/** Update all the output data structures (data, bounds, matrix and abs matrix).*/
	/*virtual*/ void Update();

	/** Gets the coordinates of the joint position. 	*/
	inline const double* GetJointAbsPosition() const {
		return m_AbsPosition;
	}

	/** Gets the coordinates of the joint position. 	*/
	inline void GetJointAbsPosition(double* out) const {
		for (int i = 0; i < 3; i++) {
			out[i] = m_AbsPosition[i];
		}
	}

	/** Sets the coordinates of the joint position. 
	N.B. coordinates must be given in world coordinates. */
	inline void SetJointAbsPosition(const double* out) {
		for (int i = 0; i < 3; i++) {
			m_AbsPosition[i] = out[i];
		}
	}

	/** Gets the number of valid axes around which the actors (segments) may rotate. */
	inline int GetNumberOfRotationAxes() const {
		return m_ValidRotateAxes;
	}

	/** Sets the number of valid axes around which the actors (segments) may rotate. */
	inline void SetNumberOfRotationAxes(int axes) {
		m_ValidRotateAxes = axes;
	}

	/** Gets the given rotation axis. */
	inline const double* GetRotationAxis(int index) const {
		return m_RotateAxis[index];
	}

	/** Gets the given rotation axis. */
	inline void GetRotationAxis(int index, double* axis) const {
		for (int i = 0; i < 3; i++) {
			axis[i] = m_RotateAxis[index][i];
		}
	}

	/** Sets the given rotation axis. 
	N.B. index must be from interval 0 .. GetNumberOfRotationAxes()  - 1*/
	inline void SetRotationAxis(int index, double* axis) {
		for (int i = 0; i < 3; i++) {
			m_RotateAxis[index][i] = axis[i];
		}
	}

	/** Gets the principal axis of the main actor. 
	The method returns direction of the principal axis of the main (less movable) actor (segment)
	connected by the joint with the second (usually, more movable) actor (segment).
	From both axis, the angle between them can be calculated (use dot product).
	N.B. the principal axis goes through the AbsPosition.*/
	inline const double* GetMainActorPrincipalAxis() const {
		return m_ActorsAxis[0];
	}

	/** Gets the principal axis of the second actor. 
	The method returns direction of the principal axis of the second (usually, more movable) actor (segment)
	connected by the joint with the main (less movable) actor (segment).
	From both axis, the angle between them can be calculated (use dot product).
	N.B. the principal axis goes through the AbsPosition.*/
	inline const double* GetSecondActorPrincipalAxis() const {
		return m_ActorsAxis[1];
	}

	/** Gets the principal axis of the main actor. */
	inline void GetMainActorPrincipalAxis(double* out) const {
		for (int i = 0; i < 3; i++) {
			out[i] = m_ActorsAxis[0][i];
		}
	}

	/** Gets the principal axis of the second actor. */
	inline void GetSecondActorPrincipalAxis(double* out) const {
		for (int i = 0; i < 3; i++) {
			out[i] = m_ActorsAxis[1][i];
		}
	}

	/** Sets the principal axis of the main actor. 
	see GetMainActorPrincipalAxis() method*/
	inline void SetMainActorPrincipalAxis(const double* axis) {
		for (int i = 0; i < 3; i++) {
			m_ActorsAxis[0][i] = axis[i];
		}
	}

	/** Sets the principal axis of the second actor. 
	see GetMainActorPrincipalAxis() method */
	inline void SetSecondActorPrincipalAxis(const double* axis) {
		for (int i = 0; i < 3; i++) {
			m_ActorsAxis[1][i] = axis[i];
		}
	}

	/** Gets the current rotation angle (in degrees)  between both actors in the given axis.
	N.B. axis must be from interval 0..GetNumberOfRotationAxes() - 1. */
	inline double GetRotationAngle(int axis) const {
		return m_CurrentAngles[axis];
	}

	/** Gets all rotation angles (in degrees)  between both actors in each axis.*/
	inline const double* GetRotationAngles() const {
		return m_CurrentAngles;
	}

	/** Gets all rotation angles (in degrees)  between both actors in each axis.*/
	inline void GetRotationAngles(double* angles) const {
		for (int i = 0; i < 3; i++) {
			angles[i] = m_CurrentAngles[i];
		}
	}

	/** Sets all rotation angles (in degrees)  between both actors in each axis.*/
	inline void SetRotationAngles(const double* angles) {
		for (int i = 0; i < 3; i++) {
			m_CurrentAngles[i] = angles[i];
		}
	}

	/** Gets the limit angles (in degrees) for the given axis.
	N.B. axis must be from interval 0..GetNumberOfRotationAxes() - 1. */
	inline const double* GetRotationLimits(int axis) const {
		return m_LimitAngles[axis];
	}

	/** Gets the limit angles (in degrees) for the given axis.
	N.B. axis must be from interval 0..GetNumberOfRotationAxes() - 1. */
	inline void GetRotationLimits(int axis,  double* limits) const {
		limits[0] = m_LimitAngles[axis][0];
		limits[1] = m_LimitAngles[axis][1];
	}

	/** Sets all rotation angles (in degrees)  between both actors in each axis.*/
	inline void SetRotationLimits(int axis, const double* limits) {		
		m_LimitAngles[axis][0] = limits[0];
		m_LimitAngles[axis][1] = limits[1];		
	}

protected:
	 /** create and return the GUI for changing the node parameters */
	/*virtual*/ mafGUI *CreateGui();

private:
	medVMEOutputJoint(const medVMEOutputJoint&); // Not implemented
	void operator=(const medVMEOutputJoint&); // Not implemented
};

#endif //__medVMEOutputJoint_h 
