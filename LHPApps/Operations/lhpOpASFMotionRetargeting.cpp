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

#include "lhpOpASFMotionRetargeting.h"
#include "itkMatrix.h" //also includes vnl_matrix

#include <cmath>
#include <fstream>

#ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
#endif

/************************ Bone class functions **********************************/

int numBonesInSkel(Bone item)
{
	Bone * tmp = item.sibling;
	int i = 0;
	while (tmp != NULL) 
	{
		if (tmp->child != NULL)
			i+= numBonesInSkel(*(tmp->child));
		i++; tmp = tmp->sibling; 
	}
if (item.child != NULL)
	return i+1+numBonesInSkel(*item.child);
else
	return i+1;
}

int movBonesInSkel(Bone item)
{
	Bone * tmp = item.sibling;
	int i = 0;

	if (item.dof > 0) i++;
	
	while (tmp != NULL) 
	{
		if (tmp->child != NULL)
			i+= movBonesInSkel(*(tmp->child));
		if (tmp->dof > 0) i++; tmp = tmp->sibling; 
	}

if (item.child != NULL)
	return i+movBonesInSkel(*item.child);
else
	return i;
}

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

void matrix_mult(double a[][4], double b[][4], double c[][4])
{
  int i, j, k;
    for(i=0;i<4;i++)
       for(j=0;j<4;j++)
       {
	  c[i][j]=0;
	  for(k=0;k<4;k++)
	     c[i][j]+=a[i][k]*b[k][j];
       }
}

void matrix_translate(double a[][4], double x, double y, double z)
{
    double t[4][4] = { {1, 0, 0, x}, {0, 1, 0, y}, {0, 0, 1, z}, {0, 0, 0, 1} };
	double result[4][4];
	matrix_mult(a, t, result);
	memcpy(a[0], result[0], sizeof(double) * 16);
}

/* Transform the point (x,y,z) by the matrix m, which is
   assumed to be affine (last row 0 0 0 1) 
   this is just a matrix-vector multiply 
*/
void matrix_transform_affine(double m[4][4],
							 double x, double y, 
							 double z, float pt[3]) 
{
    pt[0] = float(m[0][0]*x + m[0][1]*y + m[0][2]*z + m[0][3]);
    pt[1] = float(m[1][0]*x + m[1][1]*y + m[1][2]*z + m[1][3]);
    pt[2] = float(m[2][0]*x + m[2][1]*y + m[2][2]*z + m[2][3]);
}

/*
	Rotate vector v by a, b, c in X,Y,Z order.
	v_out = Rz(c)*Ry(b)*Rx(a)*v_in
*/
void vector_rotationXYZ(float *v, float a, float b, float c)
{
    double Rx[4][4], Ry[4][4], Rz[4][4];

	//Rz is a rotation matrix about Z axis by angle c, same for Ry and Rx
    rotationZ(Rz, c);
    rotationY(Ry, b);
    rotationX(Rx, a);

	//Matrix vector multiplication to generate the output vector v.
    matrix_transform_affine(Rz, v[0], v[1], v[2], v);
    matrix_transform_affine(Ry, v[0], v[1], v[2], v);
    matrix_transform_affine(Rx, v[0], v[1], v[2], v);
}

// a = b
void matrix_copy(double a[][4], double b[][4])
{
    memcpy(a, b, sizeof(double) * 4 * 4);
}

// a = a * b
void matrix_mult(double a[][4], double b[][4])
{
    double c[4][4];
    int i, j, k;
    for(i=0;i<4;i++)
       for(j=0;j<4;j++) {
	        c[i][j]=0;
	        for(k=0;k<4;k++)
	            c[i][j]+=a[i][k]*b[k][j];
       }
    matrix_copy(a, c);
}

/* Compute transpose of a matrix
   Input: matrix  a
   Output: matrix b = Transpose(a)
*/
void matrix_transpose(double a[4][4], double b[4][4]) 
{
    int i, j;

    for (i=0; i<4; i++)
	for (j=0; j<4; j++)
	    b[i][j] = a[j][i];
}

// helper function to convert ASF part name into bone index
int Skeleton::name2idx(char *name)
{
int i=0;
	while(strcmp(m_pBoneList[i].name, name) != 0 && i++ < NUM_BONES_IN_ASF_FILE);
		return m_pBoneList[i].idx;
}

char * Skeleton::idx2name(int idx)
{
	int i=0;
	while(m_pBoneList[i].idx != idx && i++ < NUM_BONES_IN_ASF_FILE);
		return m_pBoneList[i].name;
}

void Skeleton::readASFfile(const char* asf_filename, float scale)
{
	//open file
    std::ifstream is(asf_filename, std::ios::in);
	if (is.fail()) return;

	//
	// ignore header information
	//
	char	str[2048], keyword[256];
	while (1)
	{
		is.getline(str, 2048);	
		sscanf(str, "%s", keyword);
		if (strcmp(keyword, ":bonedata") == 0)	break;
	}
	
	//
	// read bone information: global orientation and translation, DOF.
	//
	is.getline(str, 2048);
	char	part[256], *token;
	float length;

	bool done = false;
	for(int i = 1; !done && i < MAX_BONES_IN_ASF_FILE ; i++)
	{		
		m_pBoneList[i].dof=0;
		m_pBoneList[i].dofx=m_pBoneList[i].dofy=m_pBoneList[i].dofz=0;
		m_pBoneList[i].doftx=m_pBoneList[i].dofty=m_pBoneList[i].doftz=0;
		m_pBoneList[i].dofty=0;
		NUM_BONES_IN_ASF_FILE++;
		MOV_BONES_IN_ASF_FILE++;
		while(1)
		{
			is.getline(str, 2048);	sscanf(str, "%s", keyword);

			if(strcmp(keyword, "end") == 0) { break; }

			if(strcmp(keyword, ":hierarchy") == 0) { MOV_BONES_IN_ASF_FILE-=1; NUM_BONES_IN_ASF_FILE -= 1; done=true; break; }			

			//id of bone
			if(strcmp(keyword, "id") == 0)
			//	sscanf(str, "%s %d", keyword, &m_pBoneList[i].idx);
			{
				m_pBoneList[i].idx=NUM_BONES_IN_ASF_FILE-1;
			}
			//name of the bone
			if(strcmp(keyword, "name") == 0) {
				sscanf(str, "%s %s", keyword, part);
				sscanf(str, "%s %s", keyword, m_pBoneList[i].name);
			}
			
			//this line describes the bone's direction vector in global coordinate
			//it will later be converted to local coorinate system
			if(strcmp(keyword, "direction") == 0)  
				sscanf(str, "%s %f %f %f", keyword, &m_pBoneList[i].dir[0], &m_pBoneList[i].dir[1], &m_pBoneList[i].dir[2]);
			
			//length of the bone
			if(strcmp(keyword, "length") == 0)  
				sscanf(str, "%s %f", keyword, &length);

			//this line describes the orientation of bone's local coordinate 
			//system relative to the world coordinate system
			if(strcmp(keyword, "axis") == 0)      
				sscanf(str, "%s %f %f %f", keyword, &m_pBoneList[i].axis_x, &m_pBoneList[i].axis_y, &m_pBoneList[i].axis_z);

			// this line describes the bone's dof 
			if(strcmp(keyword, "dof") == 0)       
			{
				token=strtok(str, " "); 
				m_pBoneList[i].dof=0;
				while(token != NULL)      
				{
					int tdof = m_pBoneList[i].dof;

					if(strcmp(token, "rx") == 0) { m_pBoneList[i].dofx = 1; m_pBoneList[i].dofo[tdof] = 1; }
					else if(strcmp(token, "ry") == 0) { m_pBoneList[i].dofy = 1; m_pBoneList[i].dofo[tdof] = 2; }
					else if(strcmp(token, "rz") == 0) { m_pBoneList[i].dofz = 1; m_pBoneList[i].dofo[tdof] = 3; }
					else if(strcmp(token, "tx") == 0) { m_pBoneList[i].doftx = 1; m_pBoneList[i].dofo[tdof] = 4; }
					else if(strcmp(token, "ty") == 0) { m_pBoneList[i].dofty = 1; m_pBoneList[i].dofo[tdof] = 5; }
					else if(strcmp(token, "tz") == 0) { m_pBoneList[i].doftz = 1; m_pBoneList[i].dofo[tdof] = 6; }
					else if(strcmp(token, "l") == 0)  { m_pBoneList[i].doftl = 1; m_pBoneList[i].dofo[tdof] = 7; }
					else if(strcmp(token, "dof") == 0) { goto end; }
					else { printf("UNKNOWN %s\n",token); }

					m_pBoneList[i].dof++;
					m_pBoneList[i].dofo[m_pBoneList[i].dof] = 0;
end:
					token=strtok(NULL, " ");
				}
//				m_NumDOFs+=m_pBoneList[i].dof;
				printf("Bone %d DOF: ",i);
				for (int x = 0; (x < 7) && (m_pBoneList[i].dofo[x] != 0); x++) printf("%d ",m_pBoneList[i].dofo[x]);
				printf("\n");
			}


		}
		//store all the infro we read from the file into the data structure
//		m_pBoneList[i].idx = name2idx(part);
		if (!m_pBoneList[i].dofx && !m_pBoneList[i].dofx && !m_pBoneList[i].dofx) 
			MOV_BONES_IN_ASF_FILE-=1;
		m_pBoneList[i].length = length * scale;
		//init child/sibling to NULL, it will be assigned next (when hierarchy read)
		m_pBoneList[i].sibling = NULL; 
		m_pBoneList[i].child = NULL;
	}
		printf("READ %d\n",NUM_BONES_IN_ASF_FILE);
		
	//
	//read and build the hierarchy of the skeleton
	//
	char *part_name;
	int j, parent;
 
	//find "hierarchy" string in the ASF file
/*	while(1)
	{
		is.getline(str, 2048);	sscanf(str, "%s", keyword);
		if(strcmp(keyword, ":hierarchy") == 0)	
			break;
	} */
	
	//skip "begin" line
	is.getline(str, 2048);

	//Assign parent/child relationship to the bones
	while(1)
	{
		//read next line
		is.getline(str, 2048);	sscanf(str, "%s", keyword);

		//check if we are done
		if(strcmp(keyword, "end") == 0)   
			break;
		else
		{
			//parse this line, it contains parent followed by children
			part_name=strtok(str, " ");
			j=0;
			while(part_name != NULL)
			{
				if(j==0) 
					parent=name2idx(part_name);
				else 
					setChildrenAndSibling(parent, &m_pBoneList[name2idx(part_name)]);
				part_name=strtok(NULL, " ");
				j++;
			}
		}
	}

	is.close();
}


/*
   This recursive function traverces skeleton hierarchy 
   and returns a pointer to the bone with index - bIndex
   ptr should be a pointer to the root node 
   when this function first called
*/
Bone* Skeleton::getBone(Bone *ptr, int bIndex)
{
   static Bone *theptr;
   if(ptr==NULL) 
      return(NULL);
   else if(ptr->idx == bIndex)
   {
      theptr=ptr;
      return(theptr);
   }
   else
   { 
      getBone(ptr->child, bIndex);
      getBone(ptr->sibling, bIndex);
      return(theptr);
   }
}

/*
  This function sets sibling or child for parent bone
  If parent bone does not have a child, 
  then pChild is set as parent's child
  else pChild is set as a sibling of parents already existing child
*/
int Skeleton::setChildrenAndSibling(int parent, Bone *pChild)
{
	Bone *pParent;  
   
	//Get pointer to root bone
	pParent = getBone(m_pRootBone, parent);

	if(pParent==NULL)
	{
		printf("inbord bone is undefined\n"); 
		return(0);
	}
	else
	{
		//if pParent bone does not have a child
		//set pChild as parent bone child
		if(pParent->child == NULL)   
		{
			pParent->child = pChild;
		}
		else
		{ 
			//if pParent bone already has a child 
			//set pChils as pParent bone's child sibling
			pParent=pParent->child;              
			while(pParent->sibling != NULL) 
				pParent = pParent->sibling;            

			pParent->sibling = pChild;
		}
		return(1);
	}
}

/* 
	Return the pointer to the root bone
*/	
Bone* Skeleton::getRoot()
{
   return(m_pRootBone);
}

/*
  Get the bone by the index
 */
Bone* Skeleton::getBone(int index)
{
	return getBone(m_pRootBone, index);
}


/***************************************************************************************
  Compute relative orientation and translation between the 
  parent and child bones. That is, represent the orientation 
  matrix and translation vector in the local coordinate of parent body 
*****************************************************************************************/


/*
	This function sets rot_parent_current data member.
	Rotation from this bone local coordinate system 
	to the coordinate system of its parent
*/
void compute_rotation_parent_child(Bone *parent, Bone *child)
{
    double Rx[4][4], Ry[4][4], Rz[4][4], tmp[4][4], tmp1[4][4], tmp2[4][4];
    if(child != NULL)
    { 
     
        // The following openGL rotations are precalculated and saved in the orientation matrix. 
        //
        // glRotatef(-inboard->axis_x, 1., 0., 0.);
        // glRotatef(-inboard->axis_y, 0., 1,  0.);
        // glRotatef(-inboard->axis_z, 0., 0., 1.);
        // glRotatef(outboard->axis_z, 0., 0., 1.);
        // glRotatef(outboard->axis_y, 0., 1,  0.);
        // glRotatef(outboard->axis_x, 1., 0., 0.);
    
        rotationZ(Rz, -parent->axis_z);      
        rotationY(Ry, -parent->axis_y);  
        rotationX(Rx, -parent->axis_x);      
        matrix_mult(Rx, Ry, tmp);
        matrix_mult(tmp, Rz, tmp1);

        rotationZ(Rz, child->axis_z);
        rotationY(Ry, child->axis_y);
        rotationX(Rx, child->axis_x);
        matrix_mult(Rz, Ry, tmp);
        matrix_mult(tmp, Rx, tmp2);

        matrix_mult(tmp1, tmp2, tmp);
        matrix_transpose(tmp, child->rot_parent_current);    
    }
}


// loop through all bones to calculate local coordinate's direction vector and relative orientation  
void ComputeRotationToParentCoordSystem(Bone *bone)
{
	int i;
	double Rx[4][4], Ry[4][4], Rz[4][4], tmp[4][4], tmp2[4][4];

	//Compute rot_parent_current for the root 

	//Compute tmp2, a matrix containing root 
	//joint local coordinate system orientation
	rotationZ(Rz, bone[0].axis_z);
	rotationY(Ry, bone[0].axis_y);
	rotationX(Rx, bone[0].axis_x);
	matrix_mult(Rz, Ry, tmp);
	matrix_mult(tmp, Rx, tmp2);
	//set bone[root].rot_parent_current to transpose of tmp2
	matrix_transpose(tmp2, bone[0].rot_parent_current);    



	//Compute rot_parent_current for all other bones
	int numbones = numBonesInSkel(bone[0]);
	for(i=0; i<numbones; i++) 
	{
		if(bone[i].child != NULL)
		{
			compute_rotation_parent_child(&bone[i], bone[i].child);
		
			// compute parent child siblings...
			Bone * tmp = NULL;
			if (bone[i].child != NULL) tmp = (bone[i].child)->sibling;
			while (tmp != NULL)
			{
				compute_rotation_parent_child(&bone[i], tmp);
				tmp = tmp->sibling;
			}
		}
	}
}

/*
	Transform the direction vector (dir), 
	which is defined in character's global coordinate system in the ASF file, 
	to local coordinate
*/
void Skeleton::RotateBoneDirToLocalCoordSystem()
{
	int i;

	for(i=1; i<NUM_BONES_IN_ASF_FILE; i++) 
	{
		//Transform dir vector into local coordinate system
		vector_rotationXYZ(&m_pBoneList[i].dir[0], -m_pBoneList[i].axis_x, -m_pBoneList[i].axis_y, -m_pBoneList[i].axis_z);
	}

}

//Has the same hierarchy of the bone
bool Skeleton::hasSameHierarchy(Bone *pBoneA, Bone *pBoneB)
{
	if (pBoneA == NULL && pBoneB == NULL)
		return true;

	if (pBoneA == NULL || pBoneB == NULL)
		return false;

	if (pBoneA->dof != pBoneB->dof || pBoneA->dofx != pBoneB->dofx ||
		pBoneA->dofy != pBoneB->dofy || pBoneA->dofz != pBoneB->dofz)
		return false;

	if (pBoneA->axis_x != pBoneB->axis_x || pBoneA->axis_y != pBoneB->axis_y || pBoneA->axis_z != pBoneB->axis_z)
		return false;

	return hasSameHierarchy(pBoneA->sibling, pBoneB->sibling) && hasSameHierarchy(pBoneA->child, pBoneB->child);
}

/******************************************************************************
Interface functions to set the pose of the skeleton 
******************************************************************************/

//Initial posture Root at (0,0,0)
//All bone rotations are set to 0
void Skeleton::setBasePosture()
{
   int i;
   m_RootPos[0] = m_RootPos[1] = m_RootPos[2] = 0.0;

   for(i=0;i<NUM_BONES_IN_ASF_FILE;i++)
      m_pBoneList[i].drx = m_pBoneList[i].dry = m_pBoneList[i].drz = 0.0;
}

//Check whether the skeleton has the same hierarchy
bool Skeleton::hasSameHierarchy(Skeleton *pSkeleton)
{
	if (NUM_BONES_IN_ASF_FILE != pSkeleton->NUM_BONES_IN_ASF_FILE ||
		MOV_BONES_IN_ASF_FILE != pSkeleton->MOV_BONES_IN_ASF_FILE)
		return false;

	return hasSameHierarchy(m_pRootBone, pSkeleton->m_pRootBone);
}

// set the skeleton's pose based on the given posture
void Skeleton::setPosture(Posture posture) 
{
    m_RootPos[0] = (float)posture.root_pos[0];
    m_RootPos[1] = (float)posture.root_pos[1];
    m_RootPos[2] = (float)posture.root_pos[2];

    for(int j=0;j<NUM_BONES_IN_ASF_FILE;j++)
    {
		// if the bone has rotational degree of freedom in x direction
		if(m_pBoneList[j].dofx) 
		   m_pBoneList[j].drx = posture.bone_rotation[j][0];  

		if(m_pBoneList[j].doftx)
			m_pBoneList[j].tx = posture.bone_translation[j][0];

		// if the bone has rotational degree of freedom in y direction
		if(m_pBoneList[j].dofy) 
		   m_pBoneList[j].dry = posture.bone_rotation[j][1];    

		if(m_pBoneList[j].dofty)
			m_pBoneList[j].ty = posture.bone_translation[j][1];

		// if the bone has rotational degree of freedom in z direction
		if(m_pBoneList[j].dofz) 
		   m_pBoneList[j].drz = posture.bone_rotation[j][2];  

		if(m_pBoneList[j].doftz)
			m_pBoneList[j].tz= posture.bone_translation[j][2];
		
		if(m_pBoneList[j].doftl)
			m_pBoneList[j].tl = posture.bone_length[j][0];
    }
}

//Calculate the positions of end effectors in the world coordinate system using the forward kinematics
void calcEndEffectorPositionFK(Bone *bone, double matrix[4][4], std::vector<double>& endEffectorPos)
{
	if(!bone) return;
    double temp[4][4], back[4][4];
    matrix_copy(back, matrix);

	matrix_transpose(bone->rot_parent_current, temp);
    matrix_mult(matrix, temp);

	matrix_translate(matrix, bone->doftx ? bone->tx : 0, bone->dofty ? bone->ty : 0, bone->doftz ? bone->tz : 0);
	
    if(bone->dofz) {
        rotationZ(temp, bone->drz);
        matrix_mult(matrix, temp);
    }
    if(bone->dofy) {
        rotationY(temp, bone->dry);
        matrix_mult(matrix, temp);
    }
    if(bone->dofx) {
        rotationX(temp, bone->drx);
        matrix_mult(matrix, temp);
    }

	float tx = bone->dir[0] * bone->length;
	float ty = bone->dir[1] * bone->length;
	float tz = bone->dir[2] * bone->length;

	if (bone->child == NULL) {
		vnl_double_3 pos(matrix[0][3], matrix[1][3], matrix[2][3]);
		endEffectorPos.push_back(pos[0]);
        endEffectorPos.push_back(pos[1]);
        endEffectorPos.push_back(pos[2]);
	}

	matrix_translate(matrix, tx, ty, tz);
	calcEndEffectorPositionFK(bone->child, matrix, endEffectorPos);

	calcEndEffectorPositionFK(bone->sibling, back, endEffectorPos);
}

//Calculate the leg length in the rest standing pose
float Skeleton::calculateLegLength()
{
	float length = 0;
	Bone *pBone = m_pRootBone;

	while(pBone) {
		if (!strcmp(pBone->name, "lfemur"))
			length += pBone->length;

		if (!strcmp(pBone->name, "ltibia"))
			length += pBone->length;

		pBone = pBone->child;
	}

	return length;
}

//Calculate the positions of end effectors (tones) in the world coordinate system using the forward kinematic
void Skeleton::calcEndEffectorPositionFK()
{
    double matrix[4][4];
    matrix_indenty(matrix);
	endEffectorPos.clear();
	::calcEndEffectorPositionFK(m_pRootBone, matrix, endEffectorPos);
}

// convert a quaternion to a rotation matrix
void quaternion2matrix(double q0, double q1, double q2, double q3, double matrix[][4])
{
	double xx = q1 * q1;
	double xy = q1 * q2;
	double xz = q1 * q3;
	double xw = q1 * q0;

	double yy = q2 * q2;
	double yz = q2 * q3;
	double yw = q2 * q0;

	double zz = q3 * q3;
	double zw = q3 * q0;

	matrix[0][0]  = 1 - 2 * ( yy + zz );
	matrix[0][1]  =     2 * ( xy - zw );
	matrix[0][2]  =     2 * ( xz + yw );
	matrix[0][3]  = 0;

	matrix[1][0]  =     2 * ( xy + zw );
	matrix[1][1]  = 1 - 2 * ( xx + zz );
	matrix[1][2]  =     2 * ( yz - xw );
	matrix[1][3]  = 0;

	matrix[2][0]  =     2 * ( xz - yw );
	matrix[2][1]  =     2 * ( yz + xw );
	matrix[2][2]  = 1 - 2 * ( xx + yy );
	matrix[2][3]  = 0;

	matrix[3][0]  = 0;
	matrix[3][1]  = 0;
	matrix[3][2]  = 0;
	matrix[3][3]  = 1;
}

// Constructor 
Skeleton::Skeleton(const char *asf_filename, float scale)
{
	sscanf("root","%s",m_pBoneList[0].name);
	NUM_BONES_IN_ASF_FILE = 1;
	MOV_BONES_IN_ASF_FILE = 1;
    m_pBoneList[0].dofo[0] = 4;
	m_pBoneList[0].dofo[1] = 5;
	m_pBoneList[0].dofo[2] = 6;
    m_pBoneList[0].dofo[3] = 1;
	m_pBoneList[0].dofo[4] = 2;
	m_pBoneList[0].dofo[5] = 3;
	m_pBoneList[0].dofo[6] = 0;
	//Initializaton
	m_pBoneList[0].idx = 0;   // root of hierarchy
	m_pRootBone = &m_pBoneList[0];
	m_pBoneList[0].sibling = NULL;
	m_pBoneList[0].child = NULL; 
	m_pBoneList[0].dir[0] = 0; m_pBoneList[0].dir[1] = 0.; m_pBoneList[0].dir[2] = 0.;
	m_pBoneList[0].axis_x = 0; m_pBoneList[0].axis_y = 0.; m_pBoneList[0].axis_z = 0.;
	m_pBoneList[0].length = 0.05;
	m_pBoneList[0].dof = 6;
	m_pBoneList[0].dofx = m_pBoneList[0].dofy = m_pBoneList[0].dofz=1;
	m_RootPos[0] = m_RootPos[1]=m_RootPos[2]=0;
//	m_NumDOFs=6;
	tx = ty = tz = rx = ry = rz = 0;
	// build hierarchy and read in each bone's DOF information
	readASFfile(asf_filename, scale);  

	//transform the direction vector for each bone from the world coordinate system 
	//to it's local coordinate system
	RotateBoneDirToLocalCoordSystem();

	//Calculate rotation from each bone local coordinate system to the coordinate system of its parent
	//store it in rot_parent_current variable for each bone
	ComputeRotationToParentCoordSystem(m_pRootBone);
}

Skeleton::~Skeleton()
{
}

/************************ Motion class functions **********************************/
Motion::Motion(int nNumFrames)
{
//	m_NumDOFs = pActor.m_NumDOFs;
	
	m_NumFrames = nNumFrames;
	offset = 0;

	//allocate postures array
	m_pPostures = new Posture [m_NumFrames];

	//Set all postures to default posture
	SetPosturesToDefault();
}

Motion::Motion(const char *amc_filename, float scale,Skeleton * pActor2)
{
	pActor = pActor2;

//	m_NumDOFs = actor.m_NumDOFs;
	offset = 0;
	m_NumFrames = 0;
	m_pPostures = NULL;
	readAMCfile(amc_filename, scale);	
}

Motion::Motion(const char *amc_filename, float scale)
{
//	m_NumDOFs = actor.m_NumDOFs;
	offset = 0;
	m_NumFrames = 0;
	m_pPostures = NULL;
	readAMCfile(amc_filename, scale);
}


Motion::~Motion()
{
	if (m_pPostures != NULL)
		delete [] m_pPostures;
}


//Set all postures to default posture
void Motion::SetPosturesToDefault()
{
	//for each frame
	//int numbones = numBonesInSkel(bone[0]);
	for (int i = 0; i<MAX_BONES_IN_ASF_FILE; i++)
	{
		//set root position to (0,0,0)
		m_pPostures[i].root_pos[0] = 0;
        m_pPostures[i].root_pos[1] = 0;
        m_pPostures[i].root_pos[2] = 0;
		//set each bone orientation to (0,0,0)
        for (int j = 0; j < MAX_BONES_IN_ASF_FILE; j++) {
			m_pPostures[i].bone_rotation[j][0] = 0;
            m_pPostures[i].bone_rotation[j][1] = 0;
            m_pPostures[i].bone_rotation[j][2] = 0;
        }
	}
}

//Set posture at spesified frame
void Motion::SetPosture(int nFrameNum, Posture InPosture)
{
	m_pPostures[nFrameNum] = InPosture; 	
}

int Motion::GetPostureNum(int nFrameNum)
{
	nFrameNum += offset;

	if (nFrameNum < 0)
		return 0;
	else if (nFrameNum >= m_NumFrames)
		return m_NumFrames-1;
	else
		return nFrameNum;
	return 0;
}

void Motion::SetTimeOffset(int n_offset)
{
	offset = n_offset;
}

void Motion::SetBoneRotation(int nFrameNum, vnl_double_3 vRot, int nBone)
{
	m_pPostures[nFrameNum].bone_rotation[nBone] = vRot;
}

void Motion::SetRootPos(int nFrameNum, vnl_double_3 vPos)
{
	m_pPostures[nFrameNum].root_pos = vPos;
}


Posture* Motion::GetPosture(int nFrameNum)
{
	if (m_pPostures != NULL) 
		return &m_pPostures[nFrameNum]; 
	else 
		return NULL;
}


int Motion::readAMCfile(const char* name, float scale)
{
	Bone *hroot, *bone;
	bone = hroot= (*pActor).getRoot();

    std::ifstream file1( name, std::ios::in );
	if( file1.fail() ) return -1;

	int n=0;
	char str[2048];

	//count the number of lines
	while(!file1.eof())  
	{
		file1.getline(str, 2048);
		if(file1.eof()) break;
		//We do not want to count empty lines
		if (strcmp(str, "") != 0)
			n++;
	}
    file1.close();

    std::ifstream file( name, std::ios::in );
	//n = 16113;
	//file.close();

	//Compute number of frames. 
	//Subtract 3 to  ignore the header
	//There are (NUM_BONES_IN_ASF_FILE - 2) moving bones and 2 dummy bones (lhipjoint and rhipjoint)
	int numbones = numBonesInSkel(bone[0]);
	int movbones = movBonesInSkel(bone[0]);
	n = (n-3)/((movbones) + 1);   

	m_NumFrames = n;

	//Allocate memory for state vector
	m_pPostures = new Posture [m_NumFrames]; 

	//file.open(name);


	// skip the header

	while (1) 
	{
		file >> str;
		//int flag;
		//flag = strcmp(str, ":DEGREES");
		//if(flag == 0) break;
		if(strcmp(str, ":DEGREES") == 0) break;
	}

/*
	char strline[2048];
	char keyword[256];
	while (1) 
	{
		file.getline(strline,2048);
		sscanf(strline, "%s", keyword);
		if(strcmp(keyword, ":DEGREES") == 0) break;
	}
*/
	int frame_num;
	float x, y, z;
	int i, bone_idx;

	for(i=0; i<m_NumFrames; i++)
	{
		//read frame number
		file >> frame_num;
		x=y=z=0;

		//There are (NUM_BONES_IN_ASF_FILE - 2) moving bones and 2 dummy bones (lhipjoint and rhipjoint)
		for( int j=0; j<movbones; j++ )
		{
			//read bone name
			file >> str;
			
			//Convert to corresponding integer
			for( bone_idx = 0; bone_idx < numbones; bone_idx++ )
//				if( strcmp( str, AsfPartName[bone_idx] ) == 0 ) 
				if( strcmp( str, pActor->idx2name(bone_idx) ) == 0 ) 

					break;


			//init rotation angles for this bone to (0, 0, 0)
			m_pPostures[i].bone_rotation[bone_idx][0] = 0;
            m_pPostures[i].bone_rotation[bone_idx][1] = 0;
            m_pPostures[i].bone_rotation[bone_idx][2] = 0;

			for(int x = 0; x < bone[bone_idx].dof; x++)
			{
				float tmp;
				file >> tmp;
			//	printf("%d %f\n",bone[bone_idx].dofo[x],tmp);
				switch (bone[bone_idx].dofo[x]) 
				{
					case 0:
						printf("FATAL ERROR in bone %d not found %d\n",bone_idx,x);
						x = bone[bone_idx].dof;
						break;
					case 1:
						m_pPostures[i].bone_rotation[bone_idx][0] = tmp;
						break;
					case 2:
						m_pPostures[i].bone_rotation[bone_idx][1] = tmp;
						break;
					case 3:
						m_pPostures[i].bone_rotation[bone_idx][2] = tmp;
						break;
					case 4:
						m_pPostures[i].bone_translation[bone_idx][0] = tmp * scale;
						break;
					case 5:
						m_pPostures[i].bone_translation[bone_idx][1] = tmp * scale;
						break;
					case 6:
						m_pPostures[i].bone_translation[bone_idx][2] = tmp * scale;
						break;
					case 7:
						m_pPostures[i].bone_length[bone_idx][0] = tmp;// * scale;
						break;
				}
			}
			if( strcmp( str, "root" ) == 0 ) 
			{
				m_pPostures[i].root_pos[0] = m_pPostures[i].bone_translation[0][0];// * scale;
				m_pPostures[i].root_pos[1] = m_pPostures[i].bone_translation[0][1];// * scale;
				m_pPostures[i].root_pos[2] = m_pPostures[i].bone_translation[0][2];// * scale;
			}


			// read joint angles, including root orientation
			
		}
	}

	file.close();
	printf("%d samples in '%s' are read.\n", n, name);
	return n;
}

int Motion::writeAMCfile(const char *filename, float scale)
{
	int f, n, j;
	Bone *bone;
	bone=(*pActor).getRoot();

    std::ofstream os(filename);
	if(os.fail()) return -1;


	// header lines
    os << "#Unknow ASF file" << std::endl;
    os << ":FULLY-SPECIFIED" << std::endl;
    os << ":DEGREES" << std::endl;
	int numbones = numBonesInSkel(bone[0]);

	for(f=0; f < m_NumFrames; f++)
	{
        os << f+1 <<std::endl;
		os << "root " << m_pPostures[f].root_pos[0]/scale << " " 
			          << m_pPostures[f].root_pos[1]/scale << " " 
					  << m_pPostures[f].root_pos[2]/scale << " " 
					  << m_pPostures[f].bone_rotation[0][0] << " " 
					  << m_pPostures[f].bone_rotation[0][1] << " " 
					  << m_pPostures[f].bone_rotation[0][2] ;
		n=6;
		
		for(j = 2; j < numbones; j++) 
		{

			//output bone name
			if(bone[j].dof != 0)
//				os << endl << AsfPartName[j];
                os << std::endl << pActor->idx2name(j);

			//output bone rotation angles
			if(bone[j].dofx == 1) 
				os << " " << m_pPostures[f].bone_rotation[j][0];

			if(bone[j].dofy == 1) 
				os << " " << m_pPostures[f].bone_rotation[j][1];

			if(bone[j].dofz == 1) 
				os << " " << m_pPostures[f].bone_rotation[j][2];
		}
        os << std::endl;
	}

	os.close();
	printf("Write %d samples to '%s' \n", m_NumFrames, filename);
	return 0;
}

/**
A class that uses a Kalman filter to perform motion retargeting from a skeleton
representing an actor A (ASF file) with a corresponding motion (AMC file) to another
skeleton representing another actor B (ASF file) to generate another motion accoresponding
to the acotor B.

The original method takes into account four constraints: kinematic (pose), balance, torque 
limit and momentum. In our case, only knematic is considered as the other three are regarded 
to be physically correct as the data is obtained from motion capture and the output will reflect 
the same motion parameters (e.g. path, velocity, etc.). This may change depending on the retargeted 
motion such as jumping, sitting down, standing up, etc.
*/

//Based on Tak and Ko, A Physically-based motion retargeting filter, ACM Transactions
//on Graphics, 24(1), 98 - 117, 2005.

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpASFMotionRetargeting);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpASFMotionRetargeting::lhpOpASFMotionRetargeting(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
    m_OpType  = OPTYPE_OP;
    m_Canundo = true;
    MOCAP_SCALE = 0.06;
    m_ActorA  = NULL;
    m_ActorB  = NULL;
    m_MotionA = NULL;
    m_MotionB = NULL;
}

//----------------------------------------------------------------------------
lhpOpASFMotionRetargeting::~lhpOpASFMotionRetargeting()
//----------------------------------------------------------------------------
{
    delete m_ActorA;
    delete m_ActorB;
    delete m_MotionA;
    delete m_MotionB;
}

//----------------------------------------------------------------------------
void lhpOpASFMotionRetargeting::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
      case wxOK:
        OpStop(OP_RUN_OK);        
      break;
      case wxCANCEL:
        OpStop(OP_RUN_CANCEL);        
      break;
    }
  }
}

//----------------------------------------------------------------------------
bool lhpOpASFMotionRetargeting::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return true;
}

//----------------------------------------------------------------------------
mafOp *lhpOpASFMotionRetargeting::Copy()   
//----------------------------------------------------------------------------
{
  return (new lhpOpASFMotionRetargeting(m_Label));
}

//----------------------------------------------------------------------------
void lhpOpASFMotionRetargeting::OpRun()   
//----------------------------------------------------------------------------
{
    wxString asf_wildc = "ASF File (*.asf)|*.asf";
    wxString amc_wildc = "AMC File (*.amc)|*.amc";

    wxString actorA = mafGetOpenFile("", asf_wildc, "Open Source Actor File").c_str(); 
    if(!actorA.IsEmpty() && wxFileExists(actorA)) {
        m_ActorA = new Skeleton(actorA, MOCAP_SCALE);
    }
    
    wxString motionA = mafGetOpenFile("", amc_wildc, "Open Source Motion File").c_str(); 
    if(!motionA.IsEmpty() && wxFileExists(motionA) && m_ActorA) {
        m_MotionA = new Motion(motionA, MOCAP_SCALE, m_ActorA);
        
    }
    
    wxString actorB = mafGetOpenFile("", asf_wildc, "Open Target Actor File").c_str(); 
    if(!actorB.IsEmpty() && wxFileExists(actorB)) {
        m_ActorB = new Skeleton(actorB, MOCAP_SCALE);
        m_MotionB = new Motion(motionA, MOCAP_SCALE, m_ActorB);
    }

    m_MotionBFile = actorB.substr(0, actorB.length() - 3) + "amc"; 
    m_MotionBFile = mafGetSaveFile(m_MotionBFile, amc_wildc, "Save Target Motion File").c_str(); 
    
    mafEventMacro(mafEvent(this, (m_ActorA && m_ActorB && m_MotionA && m_MotionB && m_ActorA->hasSameHierarchy(m_ActorB) && !m_MotionBFile.IsEmpty()) ? OP_RUN_OK : OP_RUN_CANCEL));
}

//----------------------------------------------------------------------------
void lhpOpASFMotionRetargeting::OpDo()
//----------------------------------------------------------------------------
{
    KalmanBasedMotionRetargeting(m_ActorA, m_MotionA, m_ActorB, m_MotionB);

    m_MotionB->writeAMCfile(m_MotionBFile, MOCAP_SCALE);
}

void lhpOpASFMotionRetargeting::SimpleMotionRetargeting(Skeleton *pActorA, Motion *pMotionA, Skeleton *pActorB, Motion *pMotionB)
{
	int numFrames = pMotionA->m_NumFrames;
	
	// motion retargeting of the root position
	double legLengthA = pActorA->calculateLegLength();
	double legLengthB = pActorB->calculateLegLength();
	double scaleB2A   = legLengthB / legLengthA;
	// default standing direction y
	pMotionB->m_pPostures[0].root_pos[0] = pMotionA->m_pPostures[0].root_pos[0] + 1;    // add offset to distinguish the original actor
	pMotionB->m_pPostures[0].root_pos[1] = pMotionA->m_pPostures[0].root_pos[1] * scaleB2A;
	pMotionB->m_pPostures[0].root_pos[2] = pMotionA->m_pPostures[0].root_pos[2];
	pMotionB->m_pPostures[0].bone_translation[0] = pMotionB->m_pPostures[0].root_pos;
	for (int i = 1; i < numFrames; ++i) {
		vnl_double_3 lastPosA = pMotionA->m_pPostures[i - 1].root_pos;
		vnl_double_3 currPosA = pMotionA->m_pPostures[i].root_pos;
		vnl_double_3 lastPosB = pMotionB->m_pPostures[i - 1].root_pos;
		vnl_double_3 relativeMoveA = currPosA - lastPosA;
		pMotionB->m_pPostures[i].root_pos = lastPosB + relativeMoveA * scaleB2A;
		pMotionB->m_pPostures[i].bone_translation[0] = pMotionB->m_pPostures[i].root_pos;
	}

	// the bone's translation, rotation, and bone length
	// the actor B has the same rotation as the actor A
	// the translation (exception the root) and bone length should be all zero (no changes during the motion
	for (int i = 0; i < numFrames; ++i) {
		for (int j = 0; j < MAX_BONES_IN_ASF_FILE; ++j) {
			pMotionB->m_pPostures[i].bone_rotation[j] = pMotionA->m_pPostures[i].bone_rotation[j];
		}
	}
}

int motionParameterNum(Skeleton *pActor)
{
	int numParam = 3;
	// bone ratations, and we assume the bone rotation and length don't change during the motion
	for (int i = 0; i < pActor->NUM_BONES_IN_ASF_FILE; ++i) {
		Bone *pBone = pActor->getBone(i);
		// if the bone has rotational degree of freedom in x direction
		if(pBone->dofx) 
			numParam += 1;

		// if the bone has rotational degree of freedom in y direction
		if(pBone->dofy) 
		   numParam += 1;

		// if the bone has rotational degree of freedom in z direction
		if(pBone->dofz) 
		   numParam += 1;
    }
	return numParam;
}

int endEffectorNum(Bone *pBone)
{
	if (pBone == 0)
		return 0;
	if (pBone->child == NULL)
		return 1 + endEffectorNum(pBone->sibling);
	return endEffectorNum(pBone->child) + endEffectorNum(pBone->sibling);
}

void extractMotionParameters(Skeleton *pActor, Posture& posture, std::vector<double>& parameters)
{
	parameters.clear();

	// root position
	parameters.push_back(posture.root_pos[0]);
	parameters.push_back(posture.root_pos[1]);
	parameters.push_back(posture.root_pos[2]);

	// bone ratations, and we assume the bone rotation and length don't change during the motion
	for (int i = 0; i < pActor->NUM_BONES_IN_ASF_FILE; ++i) {
		Bone *pBone = pActor->getBone(i);
		// if the bone has rotational degree of freedom in x direction
		if(pBone->dofx) 
			parameters.push_back(posture.bone_rotation[i][0]);

		// if the bone has rotational degree of freedom in y direction
		if(pBone->dofy) 
		   parameters.push_back(posture.bone_rotation[i][1]);  

		// if the bone has rotational degree of freedom in z direction
		if(pBone->dofz) 
		   parameters.push_back(posture.bone_rotation[i][2]);
    }
}

void setMotionParameters(Skeleton *pActor, Posture& posture, std::vector<double>& parameters)
{
	int paramIndex = 0;
	// root position
	posture.root_pos[0] = parameters[paramIndex++];
	posture.root_pos[1] = parameters[paramIndex++];
	posture.root_pos[2] = parameters[paramIndex++];
	posture.bone_translation[0] = posture.root_pos;

	// bone ratations, and we assume the bone rotation and length don't change during the motion
	for (int i = 0; i < pActor->NUM_BONES_IN_ASF_FILE; ++i) {
		Bone *pBone = pActor->getBone(i);
		// if the bone has rotational degree of freedom in x direction
		if(pBone->dofx)
			posture.bone_rotation[i][0] = parameters[paramIndex++];

		// if the bone has rotational degree of freedom in y direction
		if(pBone->dofy) 
			posture.bone_rotation[i][1] = parameters[paramIndex++];

		// if the bone has rotational degree of freedom in z direction
		if(pBone->dofz) 
			posture.bone_rotation[i][2] = parameters[paramIndex++];
    }
}

void lhpOpASFMotionRetargeting::KalmanBasedMotionRetargeting(Skeleton *pActorA, Motion *pMotionA, Skeleton *pActorB, Motion *pMotionB)
{
	double Pk = 1e-8;
	double kappa = 3.0;	// lambda = alpha * alpha * (L - kappa) - L; kappa = 3 - L; alpha is in [1e-4, 1]
	int n = motionParameterNum(pActorA);
    int M = endEffectorNum(pActorA->getRoot()) * 3; // number of the end effectors * 3
    double delta = sqrt((n + 3) * Pk);		// k = 3?
    std::vector<double> sourceMotion(n);
	std::vector<double> weights(2 * n + 1);
    std::vector<double> endEffectorPos;
	std::vector< std::vector<double> > samples(2 * n + 1);
    vnl_vector<double> parameterDiff(n);
    vnl_vector<double> constraintDiff(M);

    // init samples' weights
    for (int i = 0; i < 2 * n + 1; ++i)
        weights[i] = 1 / (2 * n + 2 * kappa);
	weights[n] = kappa / (n + kappa);

	for (int k = 0; k < pMotionA->m_NumFrames; ++k) {
		// step 1: predict
		Posture posture = pMotionA->m_pPostures[k];
		extractMotionParameters(pActorA, posture, sourceMotion);
        // calculate constraints for end-effectors
        pActorA->setPosture(posture);
        pActorA->calcEndEffectorPositionFK();

		// step 2: construct (2n + 1) samples
        samples[n] = sourceMotion;
		for (int i = 0; i < n; ++i) {
            samples[i]             = sourceMotion;
			samples[i][i]         -= delta;
            samples[n + 1 + i]     = sourceMotion;
			samples[n + 1 + i][i] += delta;
		}

		// step 3: measurement using forward kinematic
		endEffectorPos.clear();
		for (int i = 0; i < 2 * n + 1; ++i) {
			setMotionParameters(pActorB, posture, samples[i]);
			pActorB->setPosture(posture);
			pActorB->calcEndEffectorPositionFK();
			for (int j = 0; j < M; ++j)
				endEffectorPos.push_back(pActorB->endEffectorPos[j]);
		}

		// step 4: sum
		// weighted predicated measurement
        std::vector<double> weightedEndPos(M, 0.0);
		for (int i = 0; i < 2 * n + 1; ++i)
			for (int j = 0; j < M; ++j)
				weightedEndPos[j] += endEffectorPos[i * M + j] * weights[i];

		// innovation covariance Pzz and cross-covariance Pxz
        vnl_matrix<double> innoCov(M, M, 0.0);  // Pzz -> innovation covariance
        vnl_matrix<double> crossCov(n, M, 0.0); // Pxz -> cross-covariance
        for (int i = 0; i < 2 * n + 1; ++i) {
            for (int j = 0; j < n; ++j)
                parameterDiff[j] = samples[i][j] - sourceMotion[j];
            for (int j = 0; j < M; ++j)
                constraintDiff[j] = endEffectorPos[i * M + j] - weightedEndPos[j];
            for (int r = 0; r < M; ++r)
                for (int c = 0; c < M; ++c)
                    innoCov[r][c] += weights[i] * constraintDiff[r] * constraintDiff[c];
            for (int r = 0; r < n; ++r)
                for (int c = 0; c < M; ++c)
                    crossCov[r][c] += weights[i] * parameterDiff[r] * constraintDiff[c];
        }

		// step 5: Kalman gain
        vnl_matrix<double> kalmanMat = crossCov * vnl_matrix_inverse<double>(innoCov);  // Pxz * (Pzz)^-1
        for (int i = 0; i < M; ++i)
            constraintDiff[i] = pActorA->endEffectorPos[i] - weightedEndPos[i];
        parameterDiff = kalmanMat * constraintDiff;
        for (int i = 0; i < n; ++i)
            sourceMotion[i] = sourceMotion[i] + parameterDiff[i];
        // save the estimated motion parameters
        setMotionParameters(pActorB, posture, sourceMotion);
        pMotionB->SetPosture(k, posture);
	}
}