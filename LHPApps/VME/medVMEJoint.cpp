/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medVMEJoint.cpp,v $
  Language:  C++
  Date:      $Date: 2012-03-20 15:32:05 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011 University of West Bohemia
  See the COPYINGS file for license details 
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "medVMEJoint.h"
#include "mafVMEOutputSurface.h"
#include "medVMEOutputJoint.h"
#include "mafStorageElement.h"
#include "mafTransform.h"
#include "mafGUI.h"
#include "mafGUIValidator.h"
#include "mafEventSource.h"
#include "mafAbsMatrixPipe.h"
#include "mafGizmoTranslate.h"
#include "mafGizmoRotate.h"
#include "mafVMESurfaceParametric.h"
#include "mafVMESurface.h"
#include "mafDataPipeCustom.h"
#include "mafDbg.h"

#include "vtkMath.h"
#include "vtkDataSet.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkArrowSource.h"
#include "vtkMAFSmartPointer.h"
#include "vtkTransformPolyDataFilter.h"

namespace jointmatrix {
void matrix_indenty(double a[][4])
{
	a[0][0]=1; a[0][1]=0; a[0][2]=0; a[0][3]=0;
	a[1][0]=0; a[1][1]=1; a[1][2]=0; a[1][3]=0;
	a[2][0]=0; a[2][1]=0; a[2][2]=1; a[2][3]=0;
	a[3][0]=0; a[3][1]=0; a[3][2]=0; a[3][3]=1;
}

void rotationZ(double r[][4], double a)
{
	a=a*M_PI/180.;
	r[0][0]=cos(a); r[0][1]=-sin(a); r[0][2]=0; r[0][3]=0;
	r[1][0]=sin(a); r[1][1]=cos(a);  r[1][2]=0; r[1][3]=0;
	r[2][0]=0;      r[2][1]=0;       r[2][2]=1; r[2][3]=0;
	r[3][0]=0;      r[3][1]=0;       r[3][2]=0; r[3][3]=1;
}

void rotationY(double r[][4], double a)
{
	a=a*M_PI/180.;
	r[0][0]=cos(a);  r[0][1]=0;       r[0][2]=sin(a); r[0][3]=0;
	r[1][0]=0;       r[1][1]=1;       r[1][2]=0;      r[1][3]=0;
	r[2][0]=-sin(a); r[2][1]=0;       r[2][2]=cos(a); r[2][3]=0;
	r[3][0]=0;       r[3][1]=0;       r[3][2]=0;      r[3][3]=1;
}

void rotationX(double r[][4], double a)
{
	a=a*M_PI/180.;
	r[0][0]=1;       r[0][1]=0;       r[0][2]=0;       r[0][3]=0;
	r[1][0]=0;       r[1][1]=cos(a);  r[1][2]=-sin(a); r[1][3]=0;
	r[2][0]=0;       r[2][1]=sin(a);  r[2][2]=cos(a);  r[2][3]=0;
	r[3][0]=0;       r[3][1]=0;       r[3][2]=0;       r[3][3]=1;
}

// c = a * b
void matrix_mult(double a[][4], double b[][4], double c[][4])
{
	int i, j, k;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++) {
			c[i][j]=0;
			for(k=0;k<4;k++)
				c[i][j]+=a[i][k]*b[k][j];
		}
}

// a = a * b
void matrix_mult(double a[][4], double b[][4])
{
	double c[4][4];
	matrix_mult(a, b, c);
	memcpy(a, c, sizeof(double) * 4 * 4);
}

void matrix_translate(double a[][4], double x, double y, double z)
{
	double t[4][4] = { {1, 0, 0, x}, {0, 1, 0, y}, {0, 0, 1, z}, {0, 0, 0, 1} };
	double result[4][4];
	matrix_mult(a, t, result);
	memcpy(a[0], result[0], sizeof(double) * 16);
}

}

//-------------------------------------------------------------------------
mafCxxTypeMacro(medVMEJoint)
//-------------------------------------------------------------------------

	const /*static*/ char* medVMEJoint::JOINT_LINK_NAMES[] = {
		"MainActorVME", "SecondActorVME",
};


//-------------------------------------------------------------------------
medVMEJoint::medVMEJoint()
//-------------------------------------------------------------------------
{	
	InitJoint();

	m_LastUpdateTime = 0;

	m_GizmoTranslate = NULL;
	m_GizmoRotate = NULL;

	mafNEW(m_Transform);
	medVMEOutputJoint *output = medVMEOutputJoint::New(); // an output with no data  
	output->SetTransform(m_Transform);										// force my transform in the output
	SetOutput(output);  

	DependsOnLinkedNodeOn();		
}

//-------------------------------------------------------------------------
medVMEJoint::~medVMEJoint()
//-------------------------------------------------------------------------
{
  // data pipe destroyed in mafVME
  // data vector destroyed in mafVMEGeneric
	mafDEL(m_Transform);  
	SetOutput(NULL);  
}

//-------------------------------------------------------------------------
//Initializes the joint to default values
/*virtual*/ void medVMEJoint::InitJoint()
	//-------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		this->m_MainActorCentroid[i] = 0.0;
		this->m_SecondActorCentroid[i] = 0.0;

		this->m_AnchorPosition[i] = 0.0;		
		
		this->m_ValidAnchorRotateAxis[i] = 1;		//enabled rotation
		this->m_AnchorRotateAxis[i][0] = 0.0;
		this->m_AnchorRotateAxis[i][1] = 0.0;
		this->m_AnchorRotateAxis[i][2] = 0.0;
		this->m_AnchorRotateAxis[i][i] = 1.0;		//axis (diagonal)
		this->m_AnchorRotateDefaults[i] = 0.0;
		this->m_AnchorRotateLimits[i][0] = -360;
		this->m_AnchorRotateLimits[i][1] = 360;
	}	
	this->m_JointDirection[0] = 0.0;
	this->m_JointDirection[1] = 0.0;
	this->m_JointDirection[2] = 1.0;
}

//-------------------------------------------------------------------------
int medVMEJoint::DeepCopy(mafNode *a)
	//-------------------------------------------------------------------------
{ 	
	int ret = Superclass::DeepCopy(a);	//this copies links
	if (ret != MAF_OK)
		return ret;
	
	medVMEJoint* source = medVMEJoint::SafeDownCast(a);
	_VERIFY_RETVAL(source != NULL, MAF_ERROR);

	m_Transform->SetMatrix(source->m_Transform->GetMatrix());	

	for (int i = 0; i < 3; i++)
	{
		this->m_MainActorCentroid[i] = source->m_MainActorCentroid[i];
		this->m_SecondActorCentroid[i] = source->m_SecondActorCentroid[i];

		this->m_AnchorPosition[i] = source->m_AnchorPosition[i];		
		
		this->m_ValidAnchorRotateAxis[i] = source->m_ValidAnchorRotateAxis[i];
		this->m_AnchorRotateAxis[i][0] = source->m_AnchorRotateAxis[i][0];		
		this->m_AnchorRotateAxis[i][1] = source->m_AnchorRotateAxis[i][1];		
		this->m_AnchorRotateAxis[i][2] = source->m_AnchorRotateAxis[i][2];	

		this->m_AnchorRotateDefaults[i] = source->m_AnchorRotateDefaults[i];
		this->m_AnchorRotateLimits[i][0] = source->m_AnchorRotateLimits[i][0];	
		this->m_AnchorRotateLimits[i][1] = source->m_AnchorRotateLimits[i][1];	
	}

	return MAF_OK;
}

//-------------------------------------------------------------------------
bool medVMEJoint::Equals(mafVME *vme)
	//-------------------------------------------------------------------------
{
	if (!Superclass::Equals(vme))   //checks also Links
		return false;

	medVMEJoint* jnt = medVMEJoint::SafeDownCast(vme);
	if (jnt == NULL ||		
			!(this->m_Transform->GetMatrix() == jnt->m_Transform->GetMatrix()) ||
			this->GetMainActorVME() != jnt->GetMainActorVME() ||
			this->GetSecondActorVME() != jnt->GetSecondActorVME()
		)
		return false;	

	for (int i = 0; i < 3; i++)
	{
		if (
			this->m_MainActorCentroid[i] != jnt->m_MainActorCentroid[i] ||
			this->m_SecondActorCentroid[i] != jnt->m_SecondActorCentroid[i] ||

			this->m_AnchorPosition[i] != jnt->m_AnchorPosition[i] ||		

			this->m_ValidAnchorRotateAxis[i] != jnt->m_ValidAnchorRotateAxis[i] ||
			this->m_AnchorRotateAxis[i][0] != jnt->m_AnchorRotateAxis[i][0] ||		
			this->m_AnchorRotateAxis[i][1] != jnt->m_AnchorRotateAxis[i][1] ||		
			this->m_AnchorRotateAxis[i][2] != jnt->m_AnchorRotateAxis[i][2] ||	

			this->m_AnchorRotateDefaults[i] != jnt->m_AnchorRotateDefaults[i] ||
			this->m_AnchorRotateLimits[i][0] != jnt->m_AnchorRotateLimits[i][0] ||	
			this->m_AnchorRotateLimits[i][1] != jnt->m_AnchorRotateLimits[i][1]
		)
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------
void medVMEJoint::SetMatrix(const mafMatrix &mat)
	//-------------------------------------------------------------------------
{    
	m_Transform->SetMatrix(mat);
	Modified();
}

//-------------------------------------------------------------------------
void medVMEJoint::GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes)
	//-------------------------------------------------------------------------
{
	kframes.clear(); // no timestamps
}

//------------------------------------------------------------------------
/*virtual*/ int medVMEJoint::InternalStore(mafStorageElement *parent)
	//------------------------------------------------------------------------
{	
	//stores links to m_MainActorVME and m_SecondActorVME
	int ret = Superclass::InternalStore(parent);
	if (ret != MAF_OK)
		return ret;

	parent->StoreVectorN("MainActorCentroid", this->m_MainActorCentroid, 3);
	parent->StoreVectorN("SecondActorCentroid", this->m_SecondActorCentroid, 3);
	parent->StoreVectorN("AnchorPosition", this->m_AnchorPosition, 3);
	parent->StoreVectorN("ValidAnchorRotateAxis", this->m_ValidAnchorRotateAxis, 3);
	parent->StoreVectorN("AnchorRotateAxis1", this->m_AnchorRotateAxis[0], 3);
	parent->StoreVectorN("AnchorRotateAxis2", this->m_AnchorRotateAxis[1], 3);
	parent->StoreVectorN("AnchorRotateAxis3", this->m_AnchorRotateAxis[2], 3);
	parent->StoreVectorN("AnchorRotateDefaults", this->m_AnchorRotateDefaults, 3);
	parent->StoreVectorN("AnchorRotateLimits1", this->m_AnchorRotateLimits[0], 2);
	parent->StoreVectorN("AnchorRotateLimits2", this->m_AnchorRotateLimits[1], 2);
	parent->StoreVectorN("AnchorRotateLimits3", this->m_AnchorRotateLimits[2], 2);

	parent->StoreMatrix("Transform",&m_Transform->GetMatrix());
	return MAF_OK;
}

//------------------------------------------------------------------------
/*virtual*/ int medVMEJoint::InternalRestore(mafStorageElement *node)
//------------------------------------------------------------------------
{
	InitJoint();	//reset everything to initial (default) values

	if (Superclass::InternalRestore(node) == MAF_OK)
	{
		//links may not be restored fully now, they must be restored later

		node->RestoreVectorN("MainActorCentroid", this->m_MainActorCentroid, 3);
		node->RestoreVectorN("SecondActorCentroid", this->m_SecondActorCentroid, 3);
		node->RestoreVectorN("AnchorPosition", this->m_AnchorPosition, 3);		
		node->RestoreVectorN("ValidAnchorRotateAxis", this->m_ValidAnchorRotateAxis, 3);
		node->RestoreVectorN("AnchorRotateAxis1", this->m_AnchorRotateAxis[0], 3);
		node->RestoreVectorN("AnchorRotateAxis2", this->m_AnchorRotateAxis[1], 3);
		node->RestoreVectorN("AnchorRotateAxis3", this->m_AnchorRotateAxis[2], 3);
		node->RestoreVectorN("AnchorRotateDefaults", this->m_AnchorRotateDefaults, 3);
		node->RestoreVectorN("AnchorRotateLimits1", this->m_AnchorRotateLimits[0], 2);
		node->RestoreVectorN("AnchorRotateLimits2", this->m_AnchorRotateLimits[1], 2);
		node->RestoreVectorN("AnchorRotateLimits3", this->m_AnchorRotateLimits[2], 2);

		mafMatrix matrix;
		if (node->RestoreMatrix("Transform",&matrix)==MAF_OK) {    
			m_Transform->SetMatrix(matrix);
		}
	}

	return MAF_OK;
}

//-----------------------------------------------------------------------
void medVMEJoint::InternalUpdate()
//-----------------------------------------------------------------------
{
	//check, if anything has changed (including time stamp)
	unsigned long tm = this->GetMTime();
	if (tm <= m_LastUpdateTime)
		return;	//there is no change => exit

	m_LastUpdateTime = tm;

	//Update the Output
	medVMEOutputJoint* jntout = medVMEOutputJoint::SafeDownCast(m_Output);
	_VERIFY_RET(jntout != NULL);
	
	//1) set the position
	double pos_jnt[3];
	if (this->GetJointAbsPosition(pos_jnt))
		jntout->SetJointAbsPosition(pos_jnt);

	//2) set axes
	int nValidAxes = 0;
	for (int i = 0; i < 3; i++) {
		if (m_ValidAnchorRotateAxis[i] != 0)
			nValidAxes++;
	}

	jntout->SetNumberOfRotationAxes(nValidAxes);
	if (nValidAxes != 0)
	{
		for (int i = 0; i < 3; i++) {
			jntout->SetRotationAxis(i, m_AnchorRotateAxis[i]);			
			jntout->SetRotationLimits(i, m_AnchorRotateLimits[i]);
		}
	}

	//3) get the current principal axis
	double pos_ma[3], pos_sa[3];
	if (this->GetMainActorAbsCentroid(pos_ma) && this->GetSecondActorAbsCentroid(pos_sa))
	{
		//NOTE: if we are here, pos_jnt must be also valid since GetJointAbsPosition depends on ma
		double u[3], v[3];
		for (int i = 0; i < 3; i++)
		{
			u[i] = pos_ma[i] - pos_jnt[i];
			v[i] = pos_sa[i] - pos_jnt[i];
		}

		jntout->SetMainActorPrincipalAxis(u);
		jntout->SetSecondActorPrincipalAxis(v);

		//4) and finally calculate angles between u and v around rotational axis
		double angles[3];
		for (int i = 0; i < 3; i++)
		{
			if (m_ValidAnchorRotateAxis[i] == 0) {
				angles[i] = 0.0; 
			}
			else {
				angles[i] = ComputeAngleBetweenVectors(u, v, m_AnchorRotateAxis[i]) - m_AnchorRotateDefaults[i];	//make it relative
			}
		}

		jntout->SetRotationAngles(angles);
	}
	
}

//------------------------------------------------------------------------
// Computes the angle (given in degrees) that is between vectors u and v in projection to the direction w. 
//In other words: it returns angle needed to be used in transform matrices for rotation around vector w to transform u to v
double medVMEJoint::ComputeAngleBetweenVectors(const double* u, const double* v, const double* w) const
{
	//so let us project principal axis onto the plane defined
	//by the joint position and the current axis
	//
	//according to Schneider et al. Geometric Tools for Computer Graphics, page 665,
	//the projection of vector u onto a plane P*n + d = 0 is simply u - (u*n)*n	providing
	//that the normal n is normalized, otherwise it is: u - ((u*n)/|n|^2)*n
	//
	double norm_1 = 1.0 / vtkMath::Norm(w);
	double u_cnst = vtkMath::Dot(u, w) * norm_1;
	double v_cnst = vtkMath::Dot(v, w) * norm_1;

	double u_prj[3], v_prj[3];			
	for (int k = 0; k < 3; k++) 
	{				
		u_prj[k] = u[k] - u_cnst*w[k];
		v_prj[k] = v[k] - v_cnst*w[k];
	}

	//calculate angle between u_prj and v_prj => first normalize these vectors
	vtkMath::Normalize(u_prj);
	vtkMath::Normalize(v_prj);

	double nn[3];
	vtkMath::Cross(u_prj, v_prj, nn);

	double sin_theta = vtkMath::Norm(nn);
	double cos_theta = vtkMath::Dot(u_prj, v_prj);

	return ComputeAngle(cos_theta, sin_theta);
}

//------------------------------------------------------------------------
//Gets the abs matrix for the given VME that can be used to
//transform points from Local Coordinates to World and vice versa
const mafMatrix* medVMEJoint::GetVMEAbsMatrix(mafVME* vme)
{
	vme->GetAbsMatrixPipe()->Update();
	return vme->GetAbsMatrixPipe()->GetMatrixPointer();
}

//-----------------------------------------------------------------------
//This internal method gets the validated coordinates for the given actorVME.	
//	General coordinates, e.g., of centroid, given in  in_coord, which are supposed to be relative to
//	the origin of the reference system of actorVME, are, if bGetAbsCoords is true,
//	transformed into the world coordinates and then stored into out_coord point buffer.
//	If bGetAbsCoords is false, the local coordinates from in_coord are simply copied
//	into out_coord. The returned coordinates are guaranteed to be valid only, if the 
//	actorVME is not NULL. In this case the method returns true (false otherwise). 
bool medVMEJoint::GetActorCoords(mafVME* actorVME, const double* in_coord, 
	double* out_coord, bool bGetAbsCoords)
//-----------------------------------------------------------------------
{
	if (actorVME == NULL){
		return false;
	}

	if (!bGetAbsCoords) 
	{
		for (int i = 0; i < 3; i++) {
			out_coord[i] = in_coord[i];
		}
	}
	else
	{
		//we are going to transform the point
		mafTransform transform;
		transform.SetMatrix(*GetVMEAbsMatrix(actorVME));
		transform.TransformPoint(in_coord, out_coord);
	}

	return true;
}


//-----------------------------------------------------------------------
//This internal method sets the coordinates, e.g., of centroid, for the given actorVME.	
//The input coordinates are passed in in_coord and are either local (relative to the	
//the origin of the reference system of actorVME) - in this case bInIsAbsCoords must be false,
//or global (world coordinates) - in this case bInIsAbsCoords must be true. If the 
//coordinates are global, they are transformed into local space of actorVME.
//The final coordinates are stored into out_coord + Modified method is called. 
//If the transformation fails (e.g., because actorVME is NULL), the method
//returns false, otherwise true. 
bool medVMEJoint::SetActorCoords(mafVME* actorVME, const double* in_coord, 
	double* out_coord, bool bInIsAbsCoords)
//-----------------------------------------------------------------------
{		
	const double* out_tmp;
	double tmp[3];

	if (!bInIsAbsCoords){
		out_tmp = in_coord;
	}
	else
	{
		if (actorVME == NULL)
			return false;

		//we are going to transform the point
		TransformPoint(in_coord, tmp, actorVME, false);		
		out_tmp = tmp;
	}

	if (
		out_coord[0] != out_tmp[0] ||
		out_coord[1] != out_tmp[1] ||
		out_coord[2] != out_tmp[2]
	)
	{	
		for (int i = 0; i < 3; i++) {
			out_coord[i] = out_tmp[i];					
		}

		this->Modified();
	}

	return true;
}

//-----------------------------------------------------------------------
//Sets the axis vector for the given axis (0,1 or 2). 
//	N.B. vector may be NULL, if the axis should be invalidated (NOT used at all).
//	Returns false, if the axis is invalid (not invalidated).
bool medVMEJoint::SetJointRotationAxis(int axis, const double* vector)
//-----------------------------------------------------------------------
{
	if (axis < 0 || axis > 2)
		return false;

	int validity = (vector == NULL) ? 0 : 1;
	if (m_ValidAnchorRotateAxis[axis] != validity)
	{
		m_ValidAnchorRotateAxis[axis] = validity;
		this->Modified();
	}

	if (m_ValidAnchorRotateAxis[axis] != 0 && (
		vector[0] != m_AnchorRotateAxis[axis][0] ||
		vector[1] != m_AnchorRotateAxis[axis][1] ||
		vector[2] != m_AnchorRotateAxis[axis][2])
	)
	{
		for (int i = 0; i < 3; i++) {
			m_AnchorRotateAxis[axis][i] = vector[i];					
		}

		this->Modified();
	}

	return true;
}


//-----------------------------------------------------------------------
//Computes the centroid of the given actor.
//	The coordinates are local (relative to the origin of reference system of actorVME).
//	The method returns false, if actorVME is NULL.
bool medVMEJoint::ComputeActorCentroid(mafVME* actorVME, double* out_centroid)
//-----------------------------------------------------------------------
{
	 if (actorVME == NULL || actorVME->GetOutput() == NULL)
	return false;

  vtkDataSet* ds = actorVME->GetOutput()->GetVTKData();
  if (ds == NULL)
	return false;

  ds->Update();
  ds->GetCenter(out_centroid);

	return true;
}

//-----------------------------------------------------------------------
//Computes the position of the joint from the specified actors. 
//Joint is supposed to be between both segments. 
//Return false, if the joint position could not be computed, e.g.,
//because one of actors is invalid.
/*virtual*/ bool medVMEJoint::ComputeJointPosition()
	//-----------------------------------------------------------------------
{
	mafVME* mainActor = GetMainActorVME();
	mafVME* secondActor = GetSecondActorVME();

	if (mainActor == NULL || secondActor == NULL)
		return false;

	if (mainActor == secondActor)
		return false;

	//get centres of bounding boxes of these actor VMEs
	//in their world (global) coordinates
	double mainActorBBC[6][3], secondActorBBC[6][3];
	GetBBoxAbsSideCentres(mainActor, mainActorBBC);
	GetBBoxAbsSideCentres(secondActor, secondActorBBC);

	//find the closest pair of points
	int ICP_MA = -1, ICP_SA = -1;
	double dblMinDist = DBL_MAX;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			double dblDist = vtkMath::Distance2BetweenPoints(mainActorBBC[i], secondActorBBC[j]);
			if (dblDist < dblMinDist)
			{
				dblMinDist = dblDist;
				ICP_MA = i; ICP_SA = j;
			}
		}
	} //end for

	//ICP_MA and ICP_SA are indices of two closest points
	double mainActorCentroid[3], secondActorCentroid[3];
	GetActorCoords(mainActor, this->m_MainActorCentroid, mainActorCentroid, true);
	GetActorCoords(secondActor, this->m_SecondActorCentroid, secondActorCentroid, true);

	/* The following commented algorithm is great for epipolar geometry but in this case it does not provide the correct results, 
	if two segments are almost colinear (in which case the joint position is calculated to be inside one of these segment.
	Hence, the simple mid-point is calculated instead.

	//mainActorBBC[ICP_MA] and mainActorCentroid defines line p
	//secondActorBBC[ICP_SA] and secondActorCentroid defines line q
	//the joint should be placed at the intersection of these lines
	//these lines, however, do not intersect in general => find point P on p and Q on q that
	//are closest to each other. The joint position is in the middle of edge PQ
	//SEE: http://objectmix.com/graphics/133793-coordinates-closest-points-pair-skew-lines-3.html
	double u[3], v[3], BA[3];
	for (int i = 0; i < 3; i++) 
	{
		u[i] = mainActorBBC[ICP_MA][i] - mainActorCentroid[i];
		v[i] = secondActorBBC[ICP_SA][i] - secondActorCentroid[i];
		BA[i] = secondActorCentroid[i] - mainActorCentroid[i];
	}

	//Let denote A = mainActorCentroid and B = secondActorCentroid
	//then P = A + t*u and Q = B + s*v.
	//Since |Q-P| must bve minimal, (Q-P) must be perpendicular to both u and v vectors
	double n[3];	//vector perpendicular to u and v
	vtkMath::Cross(u, v, n);

	//Hence, Q-P = const*n => B - A + s*v - t*u = const*n; B-A = BA
	//Here comes a trick: get two vectors U1 perpendicular to u and n 
	//and U2 perpendicular to v and n and construct two equations
	//by multiplying the equation with these vectors (dot product), i.e.,
	//we get 1) U1*BA + s*U1*v - t*U1*u = const*U1*n
	//and 2) U2*BA + s*U2*v - t*U2*u = const*U2*n
	//Naturally, Ux*n = 0 (both are perpendicular)
	//Similarly, U1*u = 0 and U2*v = 0 => both equations are simplified to:
	//1) U1*BA + s*U1*v = 0 and 2) U2*BA -t*U2*u = 0 =>
	//1) (u x n)*BA + s*(u x n)*v = 0 and 2) (v x n)*BA - t*(v x n)*u = 0
	//We apply a rule  (see http://en.wikipedia.org/wiki/Triple_product) that 
	// u * (v x w) = v* ( w x u) = w * (u x v) = - u * (w x v) = -v * (u x w) = -w * (v x u)
	//Hence, the equations can be modified as follows:
	//1) -BA*(n x u) + s*v*(u x n) = 0 => -u*(BA x n) + s*n*(v x u) = 0
	//2) -BA*(n x v) -t*u*(v x n) = 0 => -v*(BA x n) -t*n*(u x v) = 0
	//and consequently to the following:
	//1) -u*(BA x n) - s*n*(u x v) = 0 => -u(BA x n) = s*n*n
	//2) -v*(BA x n) = t*n*n
	double BAn[3];	//BA x n
	vtkMath::Cross(BA, n, BAn);
	
	double nn = vtkMath::Dot(n, n);
	double s = -vtkMath::Dot(u, BAn) / nn;
	double t = -vtkMath::Dot(v, BAn) / nn;

	//and get the position at (P + Q) / 2
	double abspos[3];
	for (int i = 0; i < 3; i++){
		abspos[i] = 0.5 * (mainActorCentroid[i] + secondActorCentroid[i] + t*u[i] + s*v[i]);
	}
	*/

	//and get the position at (P + Q) / 2
	double abspos[3];
	for (int i = 0; i < 3; i++){
		abspos[i] = 0.5 * (mainActorBBC[ICP_MA][i] + secondActorBBC[ICP_SA][i]);
	}

	//and finally transform the point into local coordinates (relative to main actor)
	TransformPoint(abspos, this->m_AnchorPosition, mainActor, false);
	return true;
}

//-----------------------------------------------------------------------
//Transforms the coordinates in_coord into output coordinates as defined by the absolute matrix transformation of vme.
//Parameter bLocal2Abs determines, if the transformation is from local to abs (world) coordinate or vice versa
void medVMEJoint::TransformPoint(const double in_coord[3], double out_coord[3], mafVME* vme, bool bLocal2Abs)
//-----------------------------------------------------------------------
{
	mafTransform transform;
	transform.SetMatrix(*GetVMEAbsMatrix(vme));

	if (!bLocal2Abs) {
		transform.Invert();
	}

	transform.TransformPoint(in_coord, out_coord);
}

//-----------------------------------------------------------------------
//Gets the world coordinates of centres of sides of the bounding box of the given vme. 
//N.B. vme may not by NULL 
void medVMEJoint::GetBBoxAbsSideCentres(mafVME* vme, double out[6][3])
//-----------------------------------------------------------------------
{
	double bounds[6];	//x_min, x_max, y_min, y_max, z_min, z_max
	vme->GetOutput()->GetVMELocalBounds(bounds);

	//calculate side centres
	double centres[3];
	for (int i = 0; i < 3; i++) {
		centres[i] = (bounds[2*i] + bounds[2*i + 1]) / 2;
	}

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 3; j++) {
			out[i][j] = centres[j];		//set it to volume centre
		}

		out[i][i / 2] = bounds[i];
	}

	//transform points into the world coords			
	mafTransform transform;
	transform.SetMatrix(*GetVMEAbsMatrix(vme));
	
	for (int i = 0; i < 6; i++) {
		transform.TransformPoint(out[i], out[i]);
	}
}

#pragma region GUI part

//-------------------------------------------------------------------------
/*virtual*/ mafGUI* medVMEJoint::CreateGui()
//-------------------------------------------------------------------------
{
	m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
	m_Gui->SetListener(this);

#pragma region wxFormsBuilder
	wxBoxSizer* bSizer13 = new wxBoxSizer( wxVERTICAL );

#pragma region MA
	wxStaticBoxSizer* sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Main Actor (MA)") ), wxVERTICAL );	
	wxBoxSizer* bSizer14 = new wxBoxSizer( wxHORIZONTAL );
		 
	bSizer14->Add(new wxStaticText( m_Gui, wxID_ANY, wxT("MA VME:")) , 0, wxALL, 5 );	

	m_txtMAVME = new wxTextCtrl( m_Gui, ID_MA_VME_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_txtMAVME->SetToolTip( wxT("Name of the main actor VME.") );
	bSizer14->Add(m_txtMAVME, 1, wxALL, 0 );
	
	m_bttnMAUpdate = new wxButton( m_Gui, ID_SELECT_MAIN_ACTOR, wxT("Update"));
	m_bttnMAUpdate->SetToolTip( wxT("Select the main actor VME: it should be the less movable segment connected with m_Gui joint, e.g., pelvis.") );
	
	bSizer14->Add( m_bttnMAUpdate, 0, wxALL, 0 );	
	sbSizer2->Add( bSizer14, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141 = new wxBoxSizer( wxHORIZONTAL );		
	bSizer141->Add(new wxStaticText( m_Gui, wxID_ANY, wxT("MA Mass Centre:")), 0, wxALL, 5 );
	
	wxTextCtrl* txtMA_CX = new wxTextCtrl( m_Gui, ID_MA_CENTROID_X);
	txtMA_CX->SetToolTip( wxT("Position of mass centre in x-coordinate (in the reference system of the main actor VME).") );	
	bSizer141->Add( txtMA_CX, 1, wxALL, 0 );
	
	wxTextCtrl* txtMA_CY = new wxTextCtrl( m_Gui, ID_MA_CENTROID_Y);
	txtMA_CY->SetToolTip( wxT("Position of mass centre in y-coordinate (in the reference system of the main actor VME).") );	
	bSizer141->Add( txtMA_CY, 1, wxALL, 0 );
	
	wxTextCtrl* txtMA_CZ = new wxTextCtrl( m_Gui, ID_MA_CENTROID_Z);
	txtMA_CZ->SetToolTip( wxT("Position of mass centre in z-coordinate (in the reference system of the main actor VME).") );	
	bSizer141->Add( txtMA_CZ, 1, wxALL, 0 );
	
	m_bttnMAAdjust = new wxButton( m_Gui, ID_MA_CENTROID_UPDATE, wxT("Adjust"));
	m_bttnMAAdjust->SetToolTip( wxT("Allows transformations of mass centre via gizmo.") );	
	bSizer141->Add( m_bttnMAAdjust, 0, wxALL, 0 );
	
	m_bttnMAAdjustOK = new wxButton( m_Gui, ID_MA_CENTROID_COMMIT, wxT("OK"));
	m_bttnMAAdjustOK->Hide();
	m_bttnMAAdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );	
	bSizer141->Add( m_bttnMAAdjustOK, 0, wxALL, 0 );
	
	m_bttnMAAdjustReset = new wxButton( m_Gui, ID_MA_CENTROID_RESET, wxT("Reset"));
	m_bttnMAAdjustReset->Hide();
	m_bttnMAAdjustReset->SetToolTip( wxT("Resets the mass centre to its original value.") );	
	bSizer141->Add( m_bttnMAAdjustReset, 0, wxALL, 0 );
	
	sbSizer2->Add( bSizer141, 0, wxEXPAND, 5 );	
	bSizer13->Add( sbSizer2, 0, wxEXPAND, 5 );
#pragma endregion MA

#pragma region SA	
	wxStaticBoxSizer* sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Second Actor (SA)") ), wxVERTICAL );	
	wxBoxSizer* bSizer142 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer142->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("SA VME:")), 0, wxALL, 5 );	

	m_txtSAVME = new wxTextCtrl( m_Gui, ID_SA_VME_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_txtSAVME->SetToolTip( wxT("Name of the second actor VME.") );	
	bSizer142->Add(m_txtSAVME, 1, wxALL, 0 );
	
	m_bttnSAUpdate = new wxButton( m_Gui, ID_SELECT_SECOND_ACTOR, wxT("Update"));
	m_bttnSAUpdate->SetToolTip( wxT("Select the second actor VME: it should be the movable segment connected with m_Gui joint, e.g. femur.") );	
	bSizer142->Add( m_bttnSAUpdate, 0, wxALL, 0 );	
	sbSizer21->Add( bSizer142, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1411 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer1411->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("SA Mass Centre:")), 0, wxALL, 5 );
	
	wxTextCtrl* txtSA_CX = new wxTextCtrl( m_Gui, ID_SA_CENTROID_X);
	txtSA_CX->SetToolTip( wxT("Position of mass centre in x-coordinate (in the reference system of the second actor VME).") );	
	bSizer1411->Add( txtSA_CX, 1, wxALL, 0 );
	
	wxTextCtrl* txtSA_CY = new wxTextCtrl( m_Gui, ID_SA_CENTROID_Y);
	txtSA_CY->SetToolTip( wxT("Position of mass centre in y-coordinate (in the reference system of the second actor VME).") );	
	bSizer1411->Add( txtSA_CY, 1, wxALL, 0 );
	
	wxTextCtrl* txtSA_CZ = new wxTextCtrl( m_Gui, ID_SA_CENTROID_Z);
	txtSA_CZ->SetToolTip( wxT("Position of mass centre in z-coordinate (in the reference system of the second actor VME).") );	
	bSizer1411->Add( txtSA_CZ, 1, wxALL, 0 );
	
	m_bttnSAAdjust = new wxButton( m_Gui, ID_SA_CENTROID_UPDATE, wxT("Adjust"));
	m_bttnSAAdjust->SetToolTip( wxT("Allows transformations of mass centre via gizmo.") );	
	bSizer1411->Add( m_bttnSAAdjust, 0, wxALL, 0 );
	
	m_bttnSAAdjustOK = new wxButton( m_Gui, ID_SA_CENTROID_COMMIT, wxT("OK"));
	m_bttnSAAdjustOK->Hide();
	m_bttnSAAdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );	
	bSizer1411->Add( m_bttnSAAdjustOK, 0, wxALL, 0 );
	
	m_bttnSAAdjustReset = new wxButton( m_Gui, ID_SA_CENTROID_RESET, wxT("Reset"));
	m_bttnSAAdjustReset->Hide();
	m_bttnSAAdjustReset->SetToolTip( wxT("Resets the mass centre to its original value.") );	
	bSizer1411->Add( m_bttnSAAdjustReset, 0, wxALL, 0 );	
	sbSizer21->Add( bSizer1411, 0, wxEXPAND, 5 );	
	bSizer13->Add( sbSizer21, 0, wxEXPAND, 5 );
#pragma endregion SA

#pragma region JointSpec	
	wxStaticBoxSizer* sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Joint Specification") ), wxVERTICAL );	
	wxBoxSizer* bSizer1412 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer1412->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Position:")), 0, wxALL, 5 );
	
	wxTextCtrl* txtAnchor_X = new wxTextCtrl( m_Gui, ID_ANCHOR_X);
	txtAnchor_X->SetToolTip( wxT("Joint position in x-coordinate (in the reference system of the main actor VME).") );	
	bSizer1412->Add( txtAnchor_X, 1, wxALL, 0 );
	
	wxTextCtrl* txtAnchor_Y = new wxTextCtrl( m_Gui, ID_ANCHOR_Y);
	txtAnchor_Y->SetToolTip( wxT("Joint position in y-coordinate (in the reference system of the main actor VME).") );	
	bSizer1412->Add( txtAnchor_Y, 1, wxALL, 0 );
	
	wxTextCtrl* txtAnchor_Z = new wxTextCtrl( m_Gui, ID_ANCHOR_Z);
	txtAnchor_Z->SetToolTip( wxT("Joint position in z-coordinate (in the reference system of the main actor VME).") );	
	bSizer1412->Add( txtAnchor_Z, 1, wxALL, 0 );
	
	m_bttnAnchorAdjust = new wxButton( m_Gui, ID_ANCHOR_UPDATE, wxT("Adjust"));
	m_bttnAnchorAdjust->SetToolTip( wxT("Allows transformations of joint position via gizmo.") );	
	bSizer1412->Add( m_bttnAnchorAdjust, 0, wxALL, 0 );
	
	m_bttnAnchorAdjustOK = new wxButton( m_Gui, ID_ANCHOR_COMMIT, wxT("OK"));
	m_bttnAnchorAdjustOK->Hide();
	m_bttnAnchorAdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );	
	bSizer1412->Add( m_bttnAnchorAdjustOK, 0, wxALL, 0 );
	
	m_bttnAnchorAdjustReset = new wxButton( m_Gui, ID_ANCHOR_RESET, wxT("Reset"));
	m_bttnAnchorAdjustReset->Hide();
	m_bttnAnchorAdjustReset->SetToolTip( wxT("Resets the joint position to its original value.") );	
	bSizer1412->Add( m_bttnAnchorAdjustReset, 0, wxALL, 0 );	
	sbSizer22->Add( bSizer1412, 0, wxEXPAND, 5 );
	
#pragma region Axis1
	wxStaticBoxSizer* sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Axis1") ), wxVERTICAL );	
	wxCheckBox* chckAxis1 = new wxCheckBox( m_Gui, ID_AXIS1_ENABLE, wxT("Enable"));	
	chckAxis1->SetToolTip( wxT("If checked, the Axis1 (and rotation around Axis1) is enabled.") );	
	sbSizer9->Add( chckAxis1, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer14121 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer14121->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Vector:")), 0, wxALL, 5 );
	wxTextCtrl* txtAxis1_X = new wxTextCtrl( m_Gui, ID_AXIS1_X);
	bSizer14121->Add( txtAxis1_X, 1, wxALL, 0 );
	wxTextCtrl* txtAxis1_Y = new wxTextCtrl( m_Gui, ID_AXIS1_Y);
	bSizer14121->Add( txtAxis1_Y, 1, wxALL, 0 );
	wxTextCtrl* txtAxis1_Z = new wxTextCtrl( m_Gui, ID_AXIS1_Z);
	bSizer14121->Add( txtAxis1_Z, 1, wxALL, 0 );
	m_bttnAxis1Adjust = new wxButton( m_Gui, ID_AXIS1_UPDATE, wxT("Adjust"));
	m_bttnAxis1Adjust->SetToolTip( wxT("Allows specification of axis1 via gizmo.") );
	bSizer14121->Add( m_bttnAxis1Adjust, 0, wxALL, 0 );
	m_bttnAxis1AdjustOK = new wxButton( m_Gui, ID_AXIS1_COMMIT, wxT("OK"));
	m_bttnAxis1AdjustOK->Hide();
	m_bttnAxis1AdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );
	bSizer14121->Add( m_bttnAxis1AdjustOK, 0, wxALL, 0 );
	m_bttnAxis1AdjustReset = new wxButton( m_Gui, ID_AXIS1_RESET, wxT("Reset"));
	m_bttnAxis1AdjustReset->Hide();
	m_bttnAxis1AdjustReset->SetToolTip( wxT("Resets the vector to its original value.") );
	bSizer14121->Add( m_bttnAxis1AdjustReset, 0, wxALL, 0 );	
	sbSizer9->Add( bSizer14121, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141211 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer141211->Add(new wxStaticText( m_Gui, wxID_ANY, wxT("Zero Angle:")), 0, wxALL, 5 );
	wxTextCtrl* txtAxis1_Zero = new wxTextCtrl( m_Gui, ID_AXIS1_ZERO);
	txtAxis1_Zero->SetToolTip( wxT("Specifies the default (zero) angle that is between both segments (given by their MA and SA mass centres) on the axis1 axis in the rest-pose position.") );	
	bSizer141211->Add( txtAxis1_Zero, 1, wxALL, 0 );		
	m_bttnAutoZeroAngle1 = new wxButton( m_Gui, ID_AXIS1_AUTO_ZERO, wxT("Auto"));	
	m_bttnAutoZeroAngle1->SetToolTip( wxT("Automatically set the zero angle to match the current position.") );	
	bSizer141211->Add( m_bttnAutoZeroAngle1, 0, wxALL, 0 );
	bSizer141211->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Limit Angles:")), 0, wxALL, 5 );	
	wxTextCtrl* txtAxis1_LimitMin = new wxTextCtrl( m_Gui, ID_AXIS1_LIMIT_MIN);
	txtAxis1_LimitMin->SetToolTip( wxT("Maximal allowed CCW angle (relative to the zero angle), e.g., -90. ") );	
	bSizer141211->Add( txtAxis1_LimitMin, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis1_LimitMax = new wxTextCtrl( m_Gui, ID_AXIS1_LIMIT_MAX);
	txtAxis1_LimitMax->SetToolTip( wxT("Maximal allowed CW angle (relative to the zero angle), e.g., 90.") );	
	bSizer141211->Add( txtAxis1_LimitMax, 1, wxALL, 0 );	
	sbSizer9->Add( bSizer141211, 0, wxEXPAND, 5 );	
	sbSizer22->Add( sbSizer9, 1, wxEXPAND, 5 );
#pragma endregion Axis1

#pragma region Axis2
	wxStaticBoxSizer* sbSizer91 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Axis2") ), wxVERTICAL );	
	wxCheckBox* chckAxis2 = new wxCheckBox( m_Gui, ID_AXIS2_ENABLE, wxT("Enable"));
	chckAxis2->SetToolTip( wxT("If checked, the Axis2 (and rotation around Axis2) is enabled.") );	
	sbSizer91->Add( chckAxis2, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141212 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer141212->Add(new wxStaticText( m_Gui, wxID_ANY, wxT("Vector:")), 0, wxALL, 5 );	
	wxTextCtrl* txtAxis2_X = new wxTextCtrl( m_Gui, ID_AXIS2_X);
	bSizer141212->Add( txtAxis2_X, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis2_Y = new wxTextCtrl( m_Gui, ID_AXIS2_Y);
	bSizer141212->Add( txtAxis2_Y, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis2_Z = new wxTextCtrl( m_Gui, ID_AXIS2_Z);
	bSizer141212->Add( txtAxis2_Z, 1, wxALL, 0 );	
	m_bttnAxis2Adjust = new wxButton( m_Gui, ID_AXIS2_UPDATE, wxT("Adjust"));
	m_bttnAxis2Adjust->SetToolTip( wxT("Allows specification of axis2 via gizmo.") );	
	bSizer141212->Add( m_bttnAxis2Adjust, 0, wxALL, 0 );	
	m_bttnAxis2AdjustOK = new wxButton( m_Gui, ID_AXIS2_COMMIT, wxT("OK"));
	m_bttnAxis2AdjustOK->Hide();
	m_bttnAxis2AdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );	
	bSizer141212->Add( m_bttnAxis2AdjustOK, 0, wxALL, 0 );	
	m_bttnAxis2AdjustReset = new wxButton( m_Gui, ID_AXIS2_RESET, wxT("Reset"));
	m_bttnAxis2AdjustReset->Hide();
	m_bttnAxis2AdjustReset->SetToolTip( wxT("Resets the vector to its original value.") );	
	bSizer141212->Add( m_bttnAxis2AdjustReset, 0, wxALL, 0 );	
	sbSizer91->Add( bSizer141212, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1412111 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer1412111->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Zero Angle:")), 0, wxALL, 5 );	
	wxTextCtrl* txtAxis2_Zero = new wxTextCtrl( m_Gui, ID_AXIS2_ZERO);
	txtAxis2_Zero->SetToolTip( wxT("Specifies the default (zero) angle that is between both segments (given by their MA and SA mass centres) on the axis1 axis in the rest-pose position.") );	
	bSizer1412111->Add( txtAxis2_Zero, 1, wxALL, 0 );		
	m_bttnAutoZeroAngle2 = new wxButton( m_Gui, ID_AXIS2_AUTO_ZERO, wxT("Auto"));	
	m_bttnAutoZeroAngle2->SetToolTip( wxT("Automatically set the zero angle to match the current position.") );	
	bSizer1412111->Add( m_bttnAutoZeroAngle2, 0, wxALL, 0 );
	bSizer1412111->Add(new wxStaticText( m_Gui, wxID_ANY, wxT("Limit Angles:")), 0, wxALL, 5 );	
	wxTextCtrl* txtAxis2_LimitMin = new wxTextCtrl( m_Gui, ID_AXIS2_LIMIT_MIN);
	txtAxis2_LimitMin->SetToolTip( wxT("Maximal allowed CCW angle (relative to the zero angle), e.g., -90. ") );	
	bSizer1412111->Add( txtAxis2_LimitMin, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis2_LimitMax = new wxTextCtrl( m_Gui, ID_AXIS2_LIMIT_MAX);
	txtAxis2_LimitMax->SetToolTip( wxT("Maximal allowed CW angle (relative to the zero angle), e.g., 90.") );	
	bSizer1412111->Add( txtAxis2_LimitMax, 1, wxALL, 0 );	
	sbSizer91->Add( bSizer1412111, 0, wxEXPAND, 5 );	
	sbSizer22->Add( sbSizer91, 1, wxEXPAND, 5 );
#pragma endregion Axis2

#pragma region Axis3
	wxStaticBoxSizer* sbSizer92 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Axis3") ), wxVERTICAL );	
	wxCheckBox* chckAxis3 = new wxCheckBox( m_Gui, ID_AXIS3_ENABLE, wxT("Enable"));	
	chckAxis3->SetToolTip( wxT("If checked, the Axis3 (and rotation around Axis3) is enabled.") );	
	sbSizer92->Add( chckAxis3, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141213 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer141213->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Vector:")), 0, wxALL, 5 );
	
	wxTextCtrl* txtAxis3_X = new wxTextCtrl( m_Gui, ID_AXIS3_X);
	bSizer141213->Add( txtAxis3_X, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis3_Y = new wxTextCtrl( m_Gui, ID_AXIS3_Y);
	bSizer141213->Add( txtAxis3_Y, 1, wxALL, 0 );	
	wxTextCtrl* txtAxis3_Z = new wxTextCtrl( m_Gui, ID_AXIS3_Z);
	bSizer141213->Add( txtAxis3_Z, 1, wxALL, 0 );
	
	m_bttnAxis3Adjust = new wxButton( m_Gui, ID_AXIS3_UPDATE, wxT("Adjust"));
	m_bttnAxis3Adjust->SetToolTip( wxT("Allows specification of axis3 via gizmo.") );	
	bSizer141213->Add( m_bttnAxis3Adjust, 0, wxALL, 0 );
	
	m_bttnAxis3AdjustOK = new wxButton( m_Gui, ID_AXIS3_COMMIT, wxT("OK"));
	m_bttnAxis3AdjustOK->Hide();
	m_bttnAxis3AdjustOK->SetToolTip( wxT("Commit changes in Adjustment.") );	
	bSizer141213->Add( m_bttnAxis3AdjustOK, 0, wxALL, 0 );
	
	m_bttnAxis3AdjustReset = new wxButton( m_Gui, ID_AXIS3_RESET, wxT("Reset"));
	m_bttnAxis3AdjustReset->Hide();
	m_bttnAxis3AdjustReset->SetToolTip( wxT("Resets the vector to its original value.") );	
	bSizer141213->Add( m_bttnAxis3AdjustReset, 0, wxALL, 0 );	
	sbSizer92->Add( bSizer141213, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1412112 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer1412112->Add(  new wxStaticText( m_Gui, wxID_ANY, wxT("Zero Angle:")), 0, wxALL, 5 );	
	wxTextCtrl* txtAxis3_Zero = new wxTextCtrl( m_Gui, ID_AXIS3_ZERO);
	txtAxis3_Zero->SetToolTip( wxT("Specifies the default (zero) angle that is between both segments (given by their MA and SA mass centres) on the axis1 axis in the rest-pose position.") );	
	bSizer1412112->Add( txtAxis3_Zero, 1, wxALL, 0 );		
	m_bttnAutoZeroAngle3 = new wxButton( m_Gui, ID_AXIS3_AUTO_ZERO, wxT("Auto"));	
	m_bttnAutoZeroAngle3->SetToolTip( wxT("Automatically set the zero angle to match the current position.") );	
	bSizer1412112->Add( m_bttnAutoZeroAngle3, 0, wxALL, 0 );
	bSizer1412112->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Limit Angles:")), 0, wxALL, 5 );
	
	wxTextCtrl* txtAxis3_LimitMin = new wxTextCtrl( m_Gui, ID_AXIS3_LIMIT_MIN);
	txtAxis3_LimitMin->SetToolTip( wxT("Maximal allowed CCW angle (relative to the zero angle), e.g., -90. ") );	
	bSizer1412112->Add( txtAxis3_LimitMin, 1, wxALL, 0 );
	
	wxTextCtrl* txtAxis3_LimitMax = new wxTextCtrl( m_Gui, ID_AXIS3_LIMIT_MAX);
	txtAxis3_LimitMax->SetToolTip( wxT("Maximal allowed CW angle (relative to the zero angle), e.g., 90.") );	
	bSizer1412112->Add( txtAxis3_LimitMax, 1, wxALL, 0 );	
	sbSizer92->Add( bSizer1412112, 0, wxEXPAND, 5 );	
	sbSizer22->Add( sbSizer92, 1, wxEXPAND, 5 );
#pragma endregion Axis3
	
	bSizer13->Add( sbSizer22, 1, wxEXPAND, 5 );
#pragma endregion JointSpec
#pragma endregion wxFormsBuilder
	m_Gui->Add(bSizer13);
  m_Gui->FitGui();

	//update MA/SA VME names
	UpdateActorNameCtrl(m_txtMAVME, GetMainActorVME());
	UpdateActorNameCtrl(m_txtSAVME, GetSecondActorVME());

	//validators
	txtMA_CX->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_X, txtMA_CX, &this->m_MainActorCentroid[0]));
	txtMA_CY->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_Y, txtMA_CY, &this->m_MainActorCentroid[1]));
	txtMA_CZ->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_Z, txtMA_CZ, &this->m_MainActorCentroid[2]));
	
	txtSA_CX->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_X, txtSA_CX, &this->m_SecondActorCentroid[0]));
	txtSA_CY->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_Y, txtSA_CY, &this->m_SecondActorCentroid[1]));
	txtSA_CZ->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_Z, txtSA_CZ, &this->m_SecondActorCentroid[2]));

	txtAnchor_X->SetValidator(mafGUIValidator(this, ID_ANCHOR_X, txtAnchor_X, &this->m_AnchorPosition[0]));
	txtAnchor_Y->SetValidator(mafGUIValidator(this, ID_ANCHOR_Y, txtAnchor_Y, &this->m_AnchorPosition[1]));
	txtAnchor_Z->SetValidator(mafGUIValidator(this, ID_ANCHOR_Z, txtAnchor_Z, &this->m_AnchorPosition[2]));

	chckAxis1->SetValidator(mafGUIValidator(this, ID_AXIS1_ENABLE, chckAxis1, &this->m_ValidAnchorRotateAxis[0]));
	chckAxis2->SetValidator(mafGUIValidator(this, ID_AXIS2_ENABLE, chckAxis2, &this->m_ValidAnchorRotateAxis[1]));
	chckAxis3->SetValidator(mafGUIValidator(this, ID_AXIS3_ENABLE, chckAxis3, &this->m_ValidAnchorRotateAxis[2]));

	txtAxis1_X->SetValidator(mafGUIValidator(this, ID_AXIS1_X, txtAxis1_X, &this->m_AnchorRotateAxis[0][0]));
	txtAxis1_Y->SetValidator(mafGUIValidator(this, ID_AXIS1_Y, txtAxis1_Y, &this->m_AnchorRotateAxis[0][1]));
	txtAxis1_Z->SetValidator(mafGUIValidator(this, ID_AXIS1_Z, txtAxis1_Z, &this->m_AnchorRotateAxis[0][2]));

	txtAxis2_X->SetValidator(mafGUIValidator(this, ID_AXIS2_X, txtAxis2_X, &this->m_AnchorRotateAxis[1][0]));
	txtAxis2_Y->SetValidator(mafGUIValidator(this, ID_AXIS2_Y, txtAxis2_Y, &this->m_AnchorRotateAxis[1][1]));
	txtAxis2_Z->SetValidator(mafGUIValidator(this, ID_AXIS2_Z, txtAxis2_Z, &this->m_AnchorRotateAxis[1][2]));

	txtAxis3_X->SetValidator(mafGUIValidator(this, ID_AXIS3_X, txtAxis3_X, &this->m_AnchorRotateAxis[2][0]));
	txtAxis3_Y->SetValidator(mafGUIValidator(this, ID_AXIS3_Y, txtAxis3_Y, &this->m_AnchorRotateAxis[2][1]));
	txtAxis3_Z->SetValidator(mafGUIValidator(this, ID_AXIS3_Z, txtAxis3_Z, &this->m_AnchorRotateAxis[2][2]));

	txtAxis1_Zero->SetValidator(mafGUIValidator(this, ID_AXIS1_ZERO, txtAxis1_Zero, &this->m_AnchorRotateDefaults[0]));
	txtAxis2_Zero->SetValidator(mafGUIValidator(this, ID_AXIS2_ZERO, txtAxis2_Zero, &this->m_AnchorRotateDefaults[1]));
	txtAxis3_Zero->SetValidator(mafGUIValidator(this, ID_AXIS3_ZERO, txtAxis3_Zero, &this->m_AnchorRotateDefaults[2]));
	
	txtAxis1_LimitMin->SetValidator(mafGUIValidator(this, ID_AXIS1_LIMIT_MIN, txtAxis1_LimitMin, &this->m_AnchorRotateLimits[0][0]));
	txtAxis2_LimitMin->SetValidator(mafGUIValidator(this, ID_AXIS2_LIMIT_MIN, txtAxis2_LimitMin, &this->m_AnchorRotateLimits[1][0]));
	txtAxis3_LimitMin->SetValidator(mafGUIValidator(this, ID_AXIS3_LIMIT_MIN, txtAxis3_LimitMin, &this->m_AnchorRotateLimits[2][0]));

	txtAxis1_LimitMax->SetValidator(mafGUIValidator(this, ID_AXIS1_LIMIT_MAX, txtAxis1_LimitMax, &this->m_AnchorRotateLimits[0][1]));
	txtAxis2_LimitMax->SetValidator(mafGUIValidator(this, ID_AXIS2_LIMIT_MAX, txtAxis2_LimitMax, &this->m_AnchorRotateLimits[1][1]));
	txtAxis3_LimitMax->SetValidator(mafGUIValidator(this, ID_AXIS3_LIMIT_MAX, txtAxis3_LimitMax, &this->m_AnchorRotateLimits[2][1]));

	//button validators
	m_bttnMAUpdate->SetValidator(mafGUIValidator(this, ID_SELECT_MAIN_ACTOR, m_bttnMAUpdate));
	m_bttnSAUpdate->SetValidator(mafGUIValidator(this, ID_SELECT_SECOND_ACTOR, m_bttnSAUpdate));

	m_bttnMAAdjust->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_UPDATE, m_bttnMAAdjust));
	m_bttnMAAdjustOK->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_COMMIT, m_bttnMAAdjustOK));
	m_bttnMAAdjustReset->SetValidator(mafGUIValidator(this, ID_MA_CENTROID_RESET, m_bttnMAAdjustReset));

	m_bttnSAAdjust->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_UPDATE, m_bttnSAAdjust));
	m_bttnSAAdjustOK->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_COMMIT, m_bttnSAAdjustOK));
	m_bttnSAAdjustReset->SetValidator(mafGUIValidator(this, ID_SA_CENTROID_RESET, m_bttnSAAdjustReset));

	m_bttnAnchorAdjust->SetValidator(mafGUIValidator(this, ID_ANCHOR_UPDATE, m_bttnAnchorAdjust));
	m_bttnAnchorAdjustOK->SetValidator(mafGUIValidator(this, ID_ANCHOR_COMMIT, m_bttnAnchorAdjustOK));
	m_bttnAnchorAdjustReset->SetValidator(mafGUIValidator(this, ID_ANCHOR_RESET, m_bttnAnchorAdjustReset));

	m_bttnAxis1Adjust->SetValidator(mafGUIValidator(this, ID_AXIS1_UPDATE, m_bttnAxis1Adjust));
	m_bttnAxis1AdjustOK->SetValidator(mafGUIValidator(this, ID_AXIS1_COMMIT, m_bttnAxis1AdjustOK));
	m_bttnAxis1AdjustReset->SetValidator(mafGUIValidator(this, ID_AXIS1_RESET, m_bttnAxis1AdjustReset));

	m_bttnAxis2Adjust->SetValidator(mafGUIValidator(this, ID_AXIS2_UPDATE, m_bttnAxis2Adjust));
	m_bttnAxis2AdjustOK->SetValidator(mafGUIValidator(this, ID_AXIS2_COMMIT, m_bttnAxis2AdjustOK));
	m_bttnAxis2AdjustReset->SetValidator(mafGUIValidator(this, ID_AXIS2_RESET, m_bttnAxis2AdjustReset));

	m_bttnAxis3Adjust->SetValidator(mafGUIValidator(this, ID_AXIS3_UPDATE, m_bttnAxis3Adjust));
	m_bttnAxis3AdjustOK->SetValidator(mafGUIValidator(this, ID_AXIS3_COMMIT, m_bttnAxis3AdjustOK));
	m_bttnAxis3AdjustReset->SetValidator(mafGUIValidator(this, ID_AXIS3_RESET, m_bttnAxis3AdjustReset));

	m_bttnAutoZeroAngle1->SetValidator(mafGUIValidator(this, ID_AXIS1_AUTO_ZERO, m_bttnAutoZeroAngle1));
	m_bttnAutoZeroAngle2->SetValidator(mafGUIValidator(this, ID_AXIS2_AUTO_ZERO, m_bttnAutoZeroAngle2));
	m_bttnAutoZeroAngle3->SetValidator(mafGUIValidator(this, ID_AXIS3_AUTO_ZERO, m_bttnAutoZeroAngle3));

	return m_Gui;
}

//----------------------------------------------------------------------------
//Updates the name of actor VME shown in the given ctrl. 
//Returns true, if the actor VME is valid, otherwise false. 
/*virtual*/ bool medVMEJoint::UpdateActorNameCtrl(wxTextCtrl* ctrl, mafVME* vme)
//----------------------------------------------------------------------------
{
	if (vme == NULL)
	{
		ctrl->SetValue("<null>");
		return false;
	}
	else
	{
		ctrl->SetValue(wxString::Format("%s (#%d)", vme->GetName(), vme->GetId()));
		return true;
	}
}

//----------------------------------------------------------------------------
//Handles the SELECT_ACTOR event. Shows a dialog for selecting a new actor VME by the user.
//bMainActor is true, if the main actor is to be changed, false otherwise
/*virtual*/ void medVMEJoint::OnUpdateActor(bool bMainActor)
//----------------------------------------------------------------------------
{
	mafString title = "Select the actor VME (segment connected by this joint)";

	mafEvent ev(this, VME_CHOOSE, (long)&medVMEJoint::AcceptActorVME);
  ev.SetString(&title);
  ForwardUpEvent(ev);

  mafVME* vme = mafVME::SafeDownCast(ev.GetVme());
  if (vme != NULL)
	{
		//here is a valid actor => update the actor name ctrl 
		//and optionally, updates automatically mass centre
		bool bCalcCentroid = (wxMessageBox("Do you want to recalculate automatically the mass centre of selected actor?",
			wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES);

		if (bMainActor)
		{
			SetMainActorVME(vme);							//modify the actor
			UpdateActorNameCtrl(m_txtMAVME, vme);
			
			if (bCalcCentroid) 
			{
				ComputeMainActorCentroid();				//calculate main actor mass centre							
				m_Gui->TransferDataToWindow();
			}
		}
		else
		{
			SetSecondActorVME(vme);	//modify the actor
			UpdateActorNameCtrl(m_txtSAVME, vme);
			
			if (bCalcCentroid)
			{
				ComputeSecondActorCentroid();			//calculate second actor mass centre			
				m_Gui->TransferDataToWindow();
			}
		}		

		if (GetMainActorVME() != NULL && GetSecondActorVME() != NULL)
		{
			if (wxMessageBox("Do you want to recalculate automatically the position of the joint?",
					wxMessageBoxCaptionStr, wxYES_NO | wxICON_QUESTION) == wxYES)
			{
				ComputeJointPosition();	//compute the joint position
				m_Gui->TransferDataToWindow();
			}
		}
	}
}

//----------------------------------------------------------------------------
//Updates the visibility of Adjust, Commit and Reset buttons for the anchor
void medVMEJoint::UpdateAnchorButtons(bool bEnableCommitReset)
//----------------------------------------------------------------------------
{
	m_bttnAnchorAdjust->Show(!bEnableCommitReset);
	m_bttnAnchorAdjustOK->Show(bEnableCommitReset);
	m_bttnAnchorAdjustReset->Show(bEnableCommitReset);

	//disable all other operations until the update of anchor is finished
	EnableAdjustButtons(!bEnableCommitReset);	
	m_Gui->Layout();
}

//----------------------------------------------------------------------------
//Handles ID_ANCHOR_UPDATE event. Shows translate gizmo for the anchor.
/*virtual*/ void medVMEJoint::OnUpdateAnchor()
//----------------------------------------------------------------------------
{	
	double abspos[3];
	this->GetJointAbsPosition(abspos);
	BeginGizmoTranslate(abspos);

	UpdateAnchorButtons(true);
}

//----------------------------------------------------------------------------
//Handles ID_ANCHOR_COMMIT event. Hides translate gizmo for the anchor + stores changes. 
/*virtual*/ void medVMEJoint::OnCommitAnchor()
//----------------------------------------------------------------------------
{
	double abspos[3];
	EndGizmoTranslate(abspos);
	this->SetJointAbsPosition(abspos);
	
	UpdateAnchorButtons(false);
	m_Gui->TransferDataToWindow();	//make sure changes are apparent	
}

//----------------------------------------------------------------------------
// Handles ID_ANCHOR_RESET event. Hides translate gizmo for the anchor. 
/*virtual*/ void medVMEJoint::OnResetAnchor()
//----------------------------------------------------------------------------
{
	EndGizmoTranslate(NULL);
	UpdateAnchorButtons(false);
}

//----------------------------------------------------------------------------
//Updates the visibility of Adjust, Commit and Reset buttons for the centroids
void medVMEJoint::UpdateCentroidButtons(bool bMainActor, bool bEnableCommitReset)
//----------------------------------------------------------------------------
{
	if (bMainActor)
	{		
		m_bttnMAAdjust->Show(!bEnableCommitReset);
		m_bttnMAAdjustOK->Show(bEnableCommitReset);
		m_bttnMAAdjustReset->Show(bEnableCommitReset);
	}
	else
	{		
		m_bttnSAAdjust->Show(!bEnableCommitReset);
		m_bttnSAAdjustOK->Show(bEnableCommitReset);
		m_bttnSAAdjustReset->Show(bEnableCommitReset);
	}

	//disable all other operations until the update of anchor is finished
	EnableAdjustButtons(!bEnableCommitReset);
	m_Gui->Layout();
}

//----------------------------------------------------------------------------
//Handles ID_XA_CENTROID_UPDATE event. Shows translate gizmo for the centroid of actor. 
/*virtual*/  void medVMEJoint::OnUpdateActorCentroid(bool bMainActor)
//----------------------------------------------------------------------------
{
	double abspos[3];
	if (bMainActor)	
		this->GetMainActorAbsCentroid(abspos);
	else
		this->GetSecondActorAbsCentroid(abspos);
	
	BeginGizmoTranslate(abspos);
	UpdateCentroidButtons(bMainActor, true);
}

//----------------------------------------------------------------------------
//Handles ID_XA_CENTROID_COMMIT event. Hides translate gizmo for the centroid + stores changes.
/*virtual*/  void medVMEJoint::OnCommitActorCentroid(bool bMainActor)
//----------------------------------------------------------------------------
{
	double abspos[3];
	EndGizmoTranslate(abspos);
	
	if (bMainActor)	
		this->SetMainActorAbsCentroid(abspos);
	else
		this->SetSecondActorAbsCentroid(abspos);

	UpdateCentroidButtons(bMainActor, false);		

	m_Gui->TransferDataToWindow();	//make sure changes are apparent
}


//----------------------------------------------------------------------------
//Handles ID_XA_CENTROID_RESET event. Hides translate gizmo for the centroid.
/*virtual*/  void medVMEJoint::OnResetActorCentroid(bool bMainActor)
//----------------------------------------------------------------------------
{
	EndGizmoTranslate(NULL);
	UpdateCentroidButtons(bMainActor, false);
}

//----------------------------------------------------------------------------
//Initializes translation GIZMO. After that ID_TRANSFORM is sent from gizmo to update the position. 
//The original absolute position of the point to translate is given as the first parameter.
/*virtual*/ void medVMEJoint::BeginGizmoTranslate(const double* abs_startpos)
//----------------------------------------------------------------------------
{
	//create helper ball to be placed
	mafVMESurfaceParametric* surf;
	mafNEW(surf);

	surf->SetGeometryType(mafVMESurfaceParametric::PARAMETRIC_SPHERE);

	mafVME* ma = GetMainActorVME();
	if (ma == NULL)
		surf->SetSphereRadius(10);
	else 
		surf->SetSphereRadius(ma->GetOutput()->GetVTKData()->GetLength() * 0.0625);

	double rxyz[3] = {0, 0, 0};
	surf->SetAbsPose(const_cast<double*>(abs_startpos), rxyz);
	surf->SetName("DUMMY");	//some dummy name
	this->AddChild(surf);

	this->ForwardUpEvent(mafEvent(this, VME_SHOW, (mafNode*)surf, true));

	m_GizmoTranslate = new mafGizmoTranslate(surf, this, false);
	m_GizmoTranslate->SetInput(surf);
	m_GizmoTranslate->SetRefSys(surf);
	m_GizmoTranslate->Show(true);	
}

//----------------------------------------------------------------------------
//Terminates translation GIZMO storing the final position into abs_end_pos. 
//N.B. abs_endpos may by NULL, if this information is not required.
/*virtual*/ void medVMEJoint::EndGizmoTranslate(double* abs_endpos)
//----------------------------------------------------------------------------
{
	mafVME* surf = m_GizmoTranslate->GetInput();

	if (abs_endpos != NULL)
	{	
		vtkTransform* tr = vtkTransform::New();
		tr->SetMatrix(
			surf->GetOutput()->GetAbsMatrix()->GetVTKMatrix()
			);
		tr->GetPosition(abs_endpos);
		tr->Delete();		
	}


	//m_GizmoTranslate->SetInput(NULL);
	//m_GizmoTranslate->SetRefSys(NULL);
	m_GizmoTranslate->Show(false);
	cppDEL(m_GizmoTranslate);

	surf->ReparentTo(NULL);
	mafDEL(surf);	
}

//----------------------------------------------------------------------------
//Processes events from the gizmo (send in the source) 
/*virtual*/ void medVMEJoint::ProcessGizmo(mafEventBase *maf_event)
	//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{		
		if (e->GetId() == ID_TRANSFORM)
		{
			mafGizmoInterface* gizmo = (mafGizmoInterface*)e->GetSender();
			mafVME* dummySurf =  gizmo->GetInput();

			vtkTransform *tr = vtkTransform::New();
			tr->PostMultiply();
			tr->SetMatrix(dummySurf->GetOutput()						
				->GetAbsMatrix()->GetVTKMatrix());
			tr->Concatenate(e->GetMatrix()->GetVTKMatrix());
			tr->Update();

			mafMatrix absPose;
			absPose.DeepCopy(tr->GetMatrix());
			absPose.SetTimeStamp(0.0);

			// move vme
			dummySurf->SetAbsMatrix(absPose);
			tr->Delete();			
		}
	}
}

//----------------------------------------------------------------------------
//Initializes rotation GIZMO. After that ID_TRANSFORM is sent from gizmo to update the position. 
//	The original absolute position of the origin is given as the first parameter. Vector to be rotated is
//	given as vector (the second parameter). 
/*virtual*/  void medVMEJoint::BeginGizmoRotate(const double* abs_startpos, const double* vector)
//----------------------------------------------------------------------------
{
	const double EPS = 1e-5;

	//create helper ball to be placed
	mafVMESurface* surf;
	mafNEW(surf);

	//the arrow is from (0,0,0) to (1,0,0), i.e., it lies on x-axis
	vtkMAFSmartPointer< vtkArrowSource > arrow;

	mafVME* ma = GetMainActorVME();	
	double length = ma == NULL ? 5 : ma->GetOutput()->GetVTKData()->GetLength() * 0.25;

	vtkMAFSmartPointer< vtkTransform > trans;	
	trans->Scale(length, length, length);

	vtkMAFSmartPointer< vtkTransformPolyDataFilter > PDF;   
	PDF->SetInput(arrow->GetOutput());
	PDF->SetTransform(trans);  

	surf->SetDataByDetaching(PDF->GetOutput(), 0.0);			
		
	double rxyz[3];
	rxyz[0] = 0.0;	//no rotation around x axis

	//first, rotate it around y axis
	//project the vector dir onto the plane XZ
	double rproj[3] = {vector[0], 0.0, vector[2] };	
	double r = vtkMath::Norm(rproj);
	if (r >= EPS)	//vector collinear with y-axis does not have projection
		rxyz[1] = - (ComputeAngle(vector[0] / r, vector[2] / r));
	else
		rxyz[1] = 0.0;

	//now, rotate it around z axis
	//project the vector dir onto the plane XY
	rproj[1] = vector[1];
	rproj[2] = 0.0;			//x is the same
	r = vtkMath::Norm(rproj);
	if (r >= EPS)	//vector collinear with z-axis does not have projection
		rxyz[2] = (ComputeAngle(vector[0] / r, vector[1] / r));
	else
		rxyz[2] = 0.0;

	surf->SetAbsPose(const_cast<double*>(abs_startpos), rxyz);
	surf->SetName("DUMMY");	//some dummy name
	this->AddChild(surf);

	this->ForwardUpEvent(mafEvent(this, VME_SHOW, (mafNode*)surf, true));

	m_GizmoRotate = new mafGizmoRotate(surf, this, false);
	m_GizmoRotate->SetInput(surf);
	m_GizmoRotate->SetRefSys(surf);
	m_GizmoRotate->Show(true);
}

//----------------------------------------------------------------------------
//Terminates rotation GIZMO storing the final vector into vector. 
//N.B. vector may by NULL, if this information is not required.
/*virtual*/ void medVMEJoint::EndGizmoRotate(double* vector)
//----------------------------------------------------------------------------
{	
	mafVME* surf = m_GizmoRotate->GetInput();

	if (vector != NULL)
	{	
		vtkTransform* tr = vtkTransform::New();
		tr->SetMatrix(
			surf->GetOutput()->GetAbsMatrix()->GetVTKMatrix()
			);

		double in_vec[3] = {1, 0, 0};
		tr->TransformVector(in_vec, vector);
		tr->Delete();		
	}
	
	m_GizmoRotate->Show(false);
	cppDEL(m_GizmoRotate);

	surf->ReparentTo(NULL);
	mafDEL(surf);	
}

//----------------------------------------------------------------------------
//Updates the visibility of Adjust, Commit and Reset buttons for the given axis
void medVMEJoint::UpdateAxisButtons(int axis, bool bEnableCommitReset)
//----------------------------------------------------------------------------
{
	switch (axis)
	{
	case 0:
		m_bttnAxis1Adjust->Show(!bEnableCommitReset);
		m_bttnAxis1AdjustOK->Show(bEnableCommitReset);
		m_bttnAxis1AdjustReset->Show(bEnableCommitReset);
		break;
	case 1:
		m_bttnAxis2Adjust->Show(!bEnableCommitReset);
		m_bttnAxis2AdjustOK->Show(bEnableCommitReset);
		m_bttnAxis2AdjustReset->Show(bEnableCommitReset);
		break;
	case 2:
		m_bttnAxis3Adjust->Show(!bEnableCommitReset);
		m_bttnAxis3AdjustOK->Show(bEnableCommitReset);
		m_bttnAxis3AdjustReset->Show(bEnableCommitReset);
		break;
	}

	//disable all other operations until the update of anchor is finished
	EnableAdjustButtons(!bEnableCommitReset);
	m_Gui->Layout();
}

//----------------------------------------------------------------------------
//Enables/disables adjust buttons
void medVMEJoint::EnableAdjustButtons(bool Enable)
{
	m_bttnAnchorAdjust->Enable(Enable);
	m_bttnMAAdjust->Enable(Enable);
	m_bttnSAAdjust->Enable(Enable);
	m_bttnAxis1Adjust->Enable(Enable);
	m_bttnAxis2Adjust->Enable(Enable);
	m_bttnAxis3Adjust->Enable(Enable);
}

//----------------------------------------------------------------------------
//Handles ID_AXISx_UPDATE event. Shows rotate gizmo for the given axis. 
/*virtual*/ void medVMEJoint::OnUpdateAxis(int axis)
//----------------------------------------------------------------------------
{
	double abspos[3];
	this->GetJointAbsPosition(abspos);
	BeginGizmoRotate(abspos, m_AnchorRotateAxis[axis]);

	UpdateAxisButtons(axis, true);
}

//----------------------------------------------------------------------------
/** Handles ID_AXISx_COMMIT event. Hides rotate gizmo for the given axis and stores the changes.  */
/*virtual*/ void medVMEJoint::OnCommitAxis(int axis)
//----------------------------------------------------------------------------
{
	EndGizmoRotate(m_AnchorRotateAxis[axis]);
	UpdateAxisButtons(axis, false);

	m_Gui->TransferDataToWindow();	//make sure changes are apparent
}

//----------------------------------------------------------------------------
/** Handles ID_AXISx_RESET event. Hides rotate gizmo for the given axis. */
/*virtual*/ void medVMEJoint::OnResetAxis(int axis)
//----------------------------------------------------------------------------
{
	EndGizmoRotate(NULL);
	UpdateAxisButtons(axis, false);
}

//----------------------------------------------------------------------------
//Handles ID_AXISx_ENABLE event. Enables or disables GUI controls for the given axis.
/*virtual */void medVMEJoint::OnEnableAxis(int axis)
//----------------------------------------------------------------------------
{
	//TODO:
}

//----------------------------------------------------------------------------
//Handles ID_AXISx_AUTO_ZERO event.
/*virtual*/ void medVMEJoint::OnAutoZeroAngle(int axis)
{
	double pos_ma[3], pos_sa[3], pos_jnt[3];
	if (this->GetMainActorAbsCentroid(pos_ma) && 
		this->GetSecondActorAbsCentroid(pos_sa) &&
		this->GetJointAbsPosition(pos_jnt))
	{		
		double u[3], v[3];
		for (int i = 0; i < 3; i++)
		{
			u[i] = pos_ma[i] - pos_jnt[i];
			v[i] = pos_sa[i] - pos_jnt[i];
		}
		
		m_AnchorRotateDefaults[axis] = ComputeAngleBetweenVectors(u, v, m_AnchorRotateAxis[axis]);			
		this->Modified();

		m_Gui->TransferDataToWindow();	//make sure changes are apparent
	}	
}

//----------------------------------------------------------------------------
void medVMEJoint::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{	
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		wxWindow* win = static_cast< wxWindow* >(e->GetSender());
		if (win == NULL || win->GetParent() != this->m_Gui)
		{
			//the event comes from gizmos or core
			if (e->GetId() == ID_TRANSFORM)
			{
				//if it is from gizmo, process the transformation
				ProcessGizmo(e);
				
				//and force redrawing of modified stuff
				this->ForwardUpEvent(mafEvent(this, CAMERA_UPDATE));
			}
		}
		else
		{
			//the event comes from our GUI
			unsigned long oldtime = this->GetMTime();

			switch (e->GetId())
			{ 
			case medVMEJoint::ID_SELECT_MAIN_ACTOR:
			case medVMEJoint::ID_SELECT_SECOND_ACTOR:
				OnUpdateActor(e->GetId() == medVMEJoint::ID_SELECT_MAIN_ACTOR);
				break;

			case medVMEJoint::ID_ANCHOR_UPDATE:
				OnUpdateAnchor();
				break;

			case medVMEJoint::ID_ANCHOR_COMMIT:
				OnCommitAnchor();
				break;

			case medVMEJoint::ID_ANCHOR_RESET:
				OnResetAnchor();
				break;

			case medVMEJoint::ID_MA_CENTROID_UPDATE:
			case medVMEJoint::ID_SA_CENTROID_UPDATE:
				OnUpdateActorCentroid(e->GetId() == medVMEJoint::ID_MA_CENTROID_UPDATE);
				break;

			case medVMEJoint::ID_MA_CENTROID_COMMIT:
			case medVMEJoint::ID_SA_CENTROID_COMMIT:
				OnCommitActorCentroid(e->GetId() == medVMEJoint::ID_MA_CENTROID_COMMIT);
				break;

			case medVMEJoint::ID_MA_CENTROID_RESET:
			case medVMEJoint::ID_SA_CENTROID_RESET:
				OnResetActorCentroid(e->GetId() == medVMEJoint::ID_MA_CENTROID_COMMIT);
				break;

			case medVMEJoint::ID_AXIS1_ENABLE:
				OnEnableAxis(0);
				break;

			case medVMEJoint::ID_AXIS2_ENABLE:
				OnEnableAxis(1);
				break;

			case medVMEJoint::ID_AXIS3_ENABLE:
				OnEnableAxis(3);
				break;

			case medVMEJoint::ID_AXIS1_UPDATE:
				OnUpdateAxis(0);
				break;

			case medVMEJoint::ID_AXIS2_UPDATE:
				OnUpdateAxis(1);
				break;

			case medVMEJoint::ID_AXIS3_UPDATE:
				OnUpdateAxis(2);
				break;

			case medVMEJoint::ID_AXIS1_COMMIT:
				OnCommitAxis(0);
				break;

			case medVMEJoint::ID_AXIS2_COMMIT:
				OnCommitAxis(1);
				break;

			case medVMEJoint::ID_AXIS3_COMMIT:
				OnCommitAxis(2);
				break;

			case medVMEJoint::ID_AXIS1_RESET:
				OnResetAxis(0);
				break;

			case medVMEJoint::ID_AXIS2_RESET:
				OnResetAxis(1);
				break;

			case medVMEJoint::ID_AXIS3_RESET:
				OnResetAxis(2);
				break;

			case medVMEJoint::ID_AXIS1_AUTO_ZERO:
				OnAutoZeroAngle(0);
				break;

			case medVMEJoint::ID_AXIS2_AUTO_ZERO:
				OnAutoZeroAngle(1);
				break;

			case medVMEJoint::ID_AXIS3_AUTO_ZERO:
				OnAutoZeroAngle(2);
				break;

			default:	//change of values, Enabling of axis, ... => our settings are modified
				this->Modified();
			}

			if (this->GetMTime() != oldtime)
			{
				//someone called this->Modified() => we have something new, something that our pipe should take
				GetOutput()->Update();	//update output, first [in case that we do not have pipe]
				GetEventSource()->InvokeEvent(this, VME_OUTPUT_DATA_CHANGED);			
			}
			
			//force redrawing of modified stuff
			this->ForwardUpEvent(mafEvent(this, CAMERA_UPDATE));
			return;
		}		
	}
	
	Superclass::OnEvent(maf_event); 	
}

//------------------------------------------------------------------------
void medVMEJoint::SetTimeStamp(mafTimeStamp t)
//------------------------------------------------------------------------
{
	Superclass::SetTimeStamp(t);
	mafVME* mainActor = GetMainActorVME();
	mafVME* secondActor = GetSecondActorVME();
	if(mainActor)
		mainActor->SetTimeStamp(t);
	if(secondActor)
		secondActor->SetTimeStamp(t);
}

//------------------------------------------------------------------------
void medVMEJoint::SetTimeStampRecursive(mafTimeStamp t)
//------------------------------------------------------------------------
{
	SetTimeStamp(t);
	Update();

	// set time stamp recuisively
	int numOfChild = GetNumberOfChildren();
	for (int i = 0; i < numOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild)
			pChild->SetTimeStampRecursive(t);
	}
}

//------------------------------------------------------------------------
void medVMEJoint::InitJointDirection()
//------------------------------------------------------------------------
{
	double parentPos[3], childPos[3];
	Update();
	medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(m_Output);
	output->GetJointAbsPosition(parentPos);
	
	int numOfChild = GetNumberOfChildren();
	for (int i = 0; i < numOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild) {
			pChild->Update();
			output = medVMEOutputJoint::SafeDownCast(pChild->GetOutput());
			output->GetJointAbsPosition(childPos);
			childPos[0] -= parentPos[0];
			childPos[1] -= parentPos[1];
			childPos[2] -= parentPos[2];
			pChild->m_JointDirection[0] = vtkMath::Dot(m_AnchorRotateAxis[0], childPos);
			pChild->m_JointDirection[1] = vtkMath::Dot(m_AnchorRotateAxis[1], childPos);
			pChild->m_JointDirection[2] = vtkMath::Dot(m_AnchorRotateAxis[2], childPos);
			pChild->InitJointDirection();
		}
	}
}

//------------------------------------------------------------------------
int medVMEJoint::GetNumberOfValidAxis()
//------------------------------------------------------------------------
{
	int numberOfValidAxis = 0;
	for (int i = 0; i < 3; ++i) {
		if (m_ValidAnchorRotateAxis[i])
			numberOfValidAxis += 1;
	}

	int numberOfChild = GetNumberOfChildren();
	for (int i = 0; i < numberOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild)
			numberOfValidAxis += pChild->GetNumberOfValidAxis();
	}
	return numberOfValidAxis;
}

//------------------------------------------------------------------------
int medVMEJoint::GetNumberOfEndEffectors()
//------------------------------------------------------------------------
{
	int numberOfChild = GetNumberOfChildren();
	if (numberOfChild == 0)
		return 1;

	int numberOfEndEffectors = 0;
	for (int i = 0; i < numberOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild)
			numberOfEndEffectors += pChild->GetNumberOfEndEffectors();
	}
	return numberOfEndEffectors;
}

//------------------------------------------------------------------------
void medVMEJoint::GetEndEffectorPosition(std::vector<double>& endEffectorPos)
//------------------------------------------------------------------------
{
	int numberOfChild = GetNumberOfChildren();
	if (numberOfChild == 0) {
		double positions[3];
		medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(m_Output);
		output->GetJointAbsPosition(positions);
		endEffectorPos.push_back(positions[0]);
		endEffectorPos.push_back(positions[1]);
		endEffectorPos.push_back(positions[2]);
	}
	else {
		for (int i = 0; i < numberOfChild; ++i) {
			medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
			if (pChild)
				pChild->GetEndEffectorPosition(endEffectorPos);
		}
	}    
}

//------------------------------------------------------------------------
void medVMEJoint::CalcEndEffectorPosition(std::vector<double>& endEffectorPos)
//------------------------------------------------------------------------
{
	double angles[3];
	double parentPos[3], childPos[3];
	medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(m_Output);
	output->GetJointAbsPosition(parentPos);

	int numOfChild = GetNumberOfChildren();
	if (numOfChild == 0) {
		endEffectorPos.push_back(parentPos[0]);
		endEffectorPos.push_back(parentPos[1]);
		endEffectorPos.push_back(parentPos[2]);
	}

	for (int i = 0; i < numOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild) {
			output = medVMEOutputJoint::SafeDownCast(pChild->GetOutput());
			output->GetRotationAngles(angles);

			double temp[4][4];
			double matrix[4][4];
			jointmatrix::matrix_indenty(matrix);
			if (pChild->m_ValidAnchorRotateAxis[2]) {
				jointmatrix::rotationZ(temp, angles[2]);
				jointmatrix::matrix_mult(matrix, temp);
			}
			if (pChild->m_ValidAnchorRotateAxis[1]) {
				jointmatrix::rotationY(temp, angles[1]);
				jointmatrix::matrix_mult(matrix, temp);
			}
			if (pChild->m_ValidAnchorRotateAxis[0]) {
				jointmatrix::rotationX(temp, angles[0]);
				jointmatrix::matrix_mult(matrix, temp);
			}

			jointmatrix::matrix_translate(matrix, pChild->m_JointDirection[0], pChild->m_JointDirection[1], pChild->m_JointDirection[2]);
			childPos[0] = parentPos[0] + m_AnchorRotateAxis[0][0] * matrix[0][4] + m_AnchorRotateAxis[1][0] * matrix[1][4] + m_AnchorRotateAxis[2][0] * matrix[2][4];
			childPos[1] = parentPos[1] + m_AnchorRotateAxis[0][1] * matrix[0][4] + m_AnchorRotateAxis[1][1] * matrix[1][4] + m_AnchorRotateAxis[2][1] * matrix[2][4];
			childPos[2] = parentPos[2] + m_AnchorRotateAxis[0][2] * matrix[0][4] + m_AnchorRotateAxis[1][2] * matrix[1][4] + m_AnchorRotateAxis[2][2] * matrix[2][4];
			
			output->SetJointAbsPosition(childPos);
			pChild->CalcEndEffectorPosition(endEffectorPos);
		}
	}
}

//------------------------------------------------------------------------
void medVMEJoint::GetRotationParameters(std::vector<double>& angles)
//------------------------------------------------------------------------
{
	double dirOri[3], dirCur[3];
	double parentPos[3], childPos[3];
	medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(m_Output);
	output->GetJointAbsPosition(parentPos);

	int numOfChild = GetNumberOfChildren();
	for (int i = 0; i < numOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild) {
			output = medVMEOutputJoint::SafeDownCast(pChild->GetOutput());
			output->GetJointAbsPosition(childPos);
			childPos[0] -= parentPos[0];
			childPos[1] -= parentPos[1];
			childPos[2] -= parentPos[2];
			dirCur[0] = vtkMath::Dot(m_AnchorRotateAxis[0], childPos);
			dirCur[1] = vtkMath::Dot(m_AnchorRotateAxis[1], childPos);
			dirCur[2] = vtkMath::Dot(m_AnchorRotateAxis[2], childPos);
			dirOri[0] = pChild->m_JointDirection[0];
			dirOri[1] = pChild->m_JointDirection[1];
			dirOri[2] = pChild->m_JointDirection[2];

			// rotating axis and angle
			double axis[3];
			vtkMath::Cross(dirOri, dirCur, axis);
			vtkMath::Normalize(axis);
			double angle = vtkMath::Dot(dirOri, dirCur) / (vtkMath::Norm(dirOri) * vtkMath::Norm(dirCur));
			angle = acos(angle);

			// quaternion 
			double sina = sin(angle * 0.5);
			double cosa = cos(angle * 0.5);

			double q0 = cosa;
			double q1 = axis[0] * sina;
			double q2 = axis[1] * sina;
			double q3 = axis[2] * sina;

			// convert quanternion to euler angles
			double x = atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2)) * 180 / M_PI;
			double y = asin(2 * (q0 * q2 - q3 * q1)) * 180 / M_PI;
			double z = atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3)) * 180 / M_PI;

			if (pChild->m_ValidAnchorRotateAxis[0])
				angles.push_back(x);
			if (pChild->m_ValidAnchorRotateAxis[1])
				angles.push_back(y);
			if (pChild->m_ValidAnchorRotateAxis[2])
				angles.push_back(z);

			pChild->GetRotationParameters(angles);
		}
	}
}

//------------------------------------------------------------------------
void medVMEJoint::SetMotionParameters(std::vector<double>& paramters)
//------------------------------------------------------------------------
{
	medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(m_Output);
	output->SetJointAbsPosition(&paramters[0]);
	SetRotationParameters(paramters, 3);
}

//------------------------------------------------------------------------
void medVMEJoint::SetRotationParameters(std::vector<double>& paramters, int index)
//------------------------------------------------------------------------
{
	double angles[3];
	int numOfChild = GetNumberOfChildren();
	for (int i = 0; i < numOfChild; ++i) {
		medVMEJoint *pChild = medVMEJoint::SafeDownCast(GetChild(i));
		if (pChild) {
			medVMEOutputJoint* output = medVMEOutputJoint::SafeDownCast(pChild->GetOutput());
			for (int i = 0; i < 3; ++i) {
				if (pChild->m_ValidAnchorRotateAxis[i])
					angles[i] = paramters[index++];
				else
					angles[i] = 0.0;
			}
			output->SetRotationAngles(angles);

			pChild->SetRotationParameters(paramters, index);
		}
	}
}

//------------------------------------------------------------------------
/*static*/ bool medVMEJoint::AcceptActorVME(mafNode *node) 
//------------------------------------------------------------------------
{  
	//accept any surface VME 
	mafVME* vme = mafVME::SafeDownCast(node);
	return (vme != NULL && mafVMEOutputSurface::SafeDownCast(vme->GetOutput()) != NULL);		
}


//-------------------------------------------------------------------------
char** medVMEJoint::GetIcon() 
//-------------------------------------------------------------------------
{
  #include "medVMEJoint.xpm"
  return medVMEJoint_xpm;
}

#pragma endregion GUI part

//----------------------------------------------------------------------------
//Given cos(theta) and sin(theta) values, it returns the angle theta in degrees. 
/*static*/ double medVMEJoint::ComputeAngle(double cos_theta, double sin_theta)
//----------------------------------------------------------------------------
{
	//get theta angle
	double theta = asin(sin_theta)*vtkMath::RadiansToDegrees();
	if (theta < 0.0)
		theta += 360.0;	//go to the correct place

	if (theta <= 90.0)
	{
		//I. quadrant
		if (cos_theta < 0.0)
			theta = 180.0 - theta;
	} 
	else if (theta <= 180.0)
	{
		//II. quadrant
		if (cos_theta > 0.0)
			theta = 180.0 - theta;
	} 
	else if (theta <= 270.0)
	{
		//III. quadrant
		if (cos_theta > 0.0)
			theta = 540.0 - theta;
	}
	else
	{
		//IV. quadrant
		if (cos_theta < 0.0)
			theta = 540.0 - theta;
	}

	return theta;
}
