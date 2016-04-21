/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMotionRetargeting.cpp,v $
  Language:  C++
  Date:      $Date: 2011-07-28 09:32:51 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Gerardo Gonzalez
==========================================================================
  Copyright (c) 2011
  University of Bedfordshire
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpMotionRetargeting.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "mafGui.h"
#include "medVMEJoint.h"
#include "medVMEOutputJoint.h"
#include "mafVMEGroup.h"
#include "mafMatrix3x3.h"
#include "mafMatrix.h"

#include "itkMatrix.h" //also includes vnl_matrix

/**
A class that uses a Kalman filter to perform motion retargeting from a set of VMEs (medVMEJoint)
representing an actor with a corresponding template motion. It produces a new motion that will 
be applied to a scaled version of the original actor.
The original method takes into account four constraints: kinematic (pose), balance, torque 
limit and momentum. In our case, only knematic is considered as the other three are regarded 
to be physically correct as the data is obtained from motion capture and the output will reflect 
the same motion parameters (e.g. path, velocity, etc.). This may change depending on the retargeted 
motion such as jumping, sitting down, standing up, etc.

*/

//Based on Tak and Ko, A Physically-based motion retargeting filter, ACM Transactions
//on Graphics, 24(1), 98 - 117, 2005.

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMotionRetargeting);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMotionRetargeting::lhpOpMotionRetargeting(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo	= true;

	m_InputPreserving = true;

    //m_Nx = 7 ;   //Number of dimensions (3 for position, 4 for rotation [quaternions])
    m_SourceJointRoot = NULL;
    m_TargetJointRoot = NULL;
  
}

//----------------------------------------------------------------------------
lhpOpMotionRetargeting::~lhpOpMotionRetargeting()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
bool lhpOpMotionRetargeting::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return node != NULL;
}

//----------------------------------------------------------------------------
/*static*/ bool lhpOpMotionRetargeting::AcceptVME(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(medVMEJoint/*mafVMEGroup*/)); 
}

//----------------------------------------------------------------------------
mafOp *lhpOpMotionRetargeting::Copy()   
//----------------------------------------------------------------------------
{
  return (new lhpOpMotionRetargeting(m_Label));
}

//----------------------------------------------------------------------------
// Constants:
//----------------------------------------------------------------------------
enum OP_MOTION_RETARGETING_ID
{
	ID_MY_OP = MINID,
  // ToDO: add your custom IDs...
};

//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::OpRun()   
//----------------------------------------------------------------------------
{  
    // source actor and motion
    mafString strSource = "Select Source Time-Varying Joint Root";
    mafEvent Source(this, VME_CHOOSE);
	Source.SetString(&strSource);
	Source.SetArg((long)&lhpOpMotionRetargeting::AcceptVME);
	mafEventMacro(Source);
	m_SourceJointRoot = medVMEJoint::SafeDownCast(Source.GetVme());

    // target actor
    mafString strTarget = "Select Target Static Joint Root";
    mafEvent eTarget(this, VME_CHOOSE);
	eTarget.SetString(&strTarget);
	eTarget.SetArg((long)&lhpOpMotionRetargeting::AcceptVME);
	mafEventMacro(eTarget);
	m_TargetJointRoot = medVMEJoint::SafeDownCast(eTarget.GetVme());

    mafEventMacro(mafEvent(this, (m_SourceJointRoot != NULL && m_TargetJointRoot != NULL) ? OP_RUN_OK : OP_RUN_CANCEL));
}
//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::OpDo()
//----------------------------------------------------------------------------
{
    std::stringstream ss;

    // checking source joint root and target joint root
    if (m_SourceJointRoot == NULL) {
        ss << "Invalid source joint root" << std::endl;
        mafLogMessage(ss.str().c_str());
        return;
    }
    if (m_TargetJointRoot == NULL) {
        ss << "Invalid target joint root" << std::endl;
        mafLogMessage(ss.str().c_str());
        return;
    }
    // checking source joint root and target joint root has the same hierarchy
    if (!HasSameHierarchy(m_SourceJointRoot, m_TargetJointRoot)) {
        ss << "Joint roots " << m_SourceJointRoot->GetName() << " and " << m_TargetJointRoot->GetName() << "do not have the same hierarchy" << std::endl;
        mafLogMessage(ss.str().c_str());
        return;
    }

    // this should be initialized when the joints are created
    m_SourceJointRoot->InitJointDirection();
    m_TargetJointRoot->InitJointDirection();
    // this line should be removed
    CopyJointDirection(m_SourceJointRoot, m_TargetJointRoot);

    MotionRetargeting();

  /************************** Gerardo's Codes ***************************
  //--TODO: 
  //    For each frame
  //for(int k=0, ...


  //-- Step1: Motion prediction 
  //outline:
  //for i: each children in medVMEJoint group
  //
  //position = medVMEJoint->getMatrix(Positon only)
  //rotation = medVMEJoint->getMatrix(Rotation only)
  //convert rotation to quaternion
  //arrayPose[i] = (position, quaterinon) //vector of 7 entries (3 position, 4 quaterions)  


  double noiseCov[7];   //noiseCovariance: Pk (only positional values)
  double noise = 1E-9;  //recommended value by Tak and Ko
  noise = sqrt(noise);

  for(int i=0; i<m_Nx; i++)
    noiseCov[i] = noise; 

  std::vector<Pose3D> origMotion; //original motion (xk)
  std::vector<double*> origCovMat; //represents the diagonal covariance matrix from xk

  const mafNode::mafChildrenVector* children = m_JointGroup->GetChildren();
  for (mafNode::mafChildrenVector::const_iterator it = children->begin();
			it != children->end(); it++)
	{
    medVMEJoint* vmeJoint = medVMEJoint::SafeDownCast((*it));
    if (vmeJoint != NULL)
    {
      double jointPos[3]; 
      if(vmeJoint->GetJointAbsPosition(jointPos) == true)   //gets the coordinates of the absolute joint position
      {
        //TODO (in medVMEJoint):
        //Rotation currently gets rotation matrix of each joint as an identity matrix because joint is a VME landmark,
        //each medVMEJoint needs to calculate its rotation with respect to its parent.
        mafMatrix jointRotMat4x4;
        mafMatrix3x3 jointRotMat3x3;
        double jointQuat[4];

        vmeJoint->GetOutput()->GetMatrix(jointRotMat4x4);  
        ExtractRotation(jointRotMat4x4, jointRotMat3x3);    //extract 3x3 rotation matrix from a 4x4 matrix
        jointRotMat3x3.MatrixTommuQuaternion(jointQuat);    //convert rotation to quaternion

        //origMotion.push_back(jointPos);
        //origMotion.push_back(jointQuat);
        //double jointPose[7];    
        //memset(jointPose, 0, sizeof(double)*7);

        Pose3D jointPose;     
        //merge joint position and rotation (quaternion) into a single array
        for(int i=0; i<3; i++)
          jointPose.elem[i] = jointPos[i];

        for(int i=0; i<4; i++)
          jointPose.elem[3+i] = jointQuat[i];

        origMotion.push_back(jointPose);
        origCovMat.push_back(noiseCov);

  
        //-- Step2: construct sample (sigma) points

        m_NoSigmaPts = 2*m_Nx + 1;    //Number of sample (sigma) points

        int i=0;
        double scale, weight;
        //arbitrary constants
        double alpha = 1;//1E-4;     //a small positive value (1 <= alpha <= 1E-4)
        int kappa = 3 - m_Nx; //kappa is usually set to 0 or 3-NumDimensions
        
        scale = alpha*alpha * (m_Nx + kappa) - m_Nx;
        std::vector<Pose3D> samplePts;  //Xi   
        std::vector<double> weightPts;   //Wi
        std::vector<Pose3D> measuredPts;  //Zi
        
        //samplePts.assign(origMotion.begin(), origMotion.end());    //i=0 -> copy original motion
        samplePts.push_back(jointPose);

        weight = scale / (m_Nx + scale);
        weightPts.push_back(weight);

        //double sample[7];
        Pose3D sample;

        double perturbation = sqrt(m_Nx + scale); //sqrt(n+s)

        int counter = 0;
        //for each jointPose element, add a perturbation component to create a sample point
        for(i = 1; i<=m_Nx; i++)            //i = 1,...,n
        {                    
          for(int j=0; j<m_Nx; j++) 
          {        
            double pointPerturbation = 0.0;

            if(counter == j)  //add the elements in the "diagonal" matrix
              pointPerturbation = perturbation * noiseCov[counter]; //( sqrt((n+s)*Pki) )

            sample.elem[j] = jointPose.elem[j] + pointPerturbation;
          }

          samplePts.push_back(sample);

          weight = 1 / (2*(m_Nx+scale));
          weightPts.push_back(weight);
          counter++;
        }

       counter = 0;
       for(i = m_Nx+1; i <= 2*m_Nx; i++)    //i = n+1,...,2n
        {
          for(int j=0; j<m_Nx; j++) 
          {         
            double pointPerturbation = 0.0;

            if(counter == j)
              pointPerturbation = perturbation * noiseCov[counter]; //( sqrt((n+s)*Pki) )

            sample.elem[j] = jointPose.elem[j] - pointPerturbation;
          }

          samplePts.push_back(sample);

          weight = 1 / (2*(m_Nx+scale));
          weightPts.push_back(weight);
          counter++;
        }


         //-- Step3: Transform sample points through a measurement model (using forward kinematics)

        KinematicConstraints(samplePts, measuredPts);


        //-- Step4: Measurement update using transformed sample points and weight values
                
        
        Pose3D sumMeasuredPts;  //Zk -> weighted sample mean        
        double measuredDiff[7]; //(Zi - Zk)
        double sampleDiff[7]; //(Xi - Xk)
        const int m = m_Nx;//2*m_Nx;
        
        vnl_matrix<double> innoCov; //Pzz -> innovation covariance     
        innoCov.set_size(m,m);
        vnl_matrix<double> crossCov; //Pxz -> cross-covariance
        crossCov.set_size(m,m);
  
      //  itk::Matrix<double> *innoCov;    
      //  itk::VariableSizeMatrix<double> *crossCov; //Pxz -> cross-covariance
      //  crossCov = new itk::VariableSizeMatrix<double>(2*m_Nx, 2*m_Nx);

        vnl_matrix<double> tempMat; //temporary matrix
        tempMat.set_size(m,m);

        innoCov.fill(0); //initialise to zero
        crossCov.fill(0);
        tempMat.fill(0);

        for(int j = 0; j < m_Nx; j++)
        {         
          sumMeasuredPts.elem[j] = 0.0; //initialise summation      
        }

        for(i = 0; i <= 2*m_Nx; i++) //for each transformed sample point
        {          
          for(int j=0; j<m_Nx; j++)  //for each element in the transformed sample point
          {
            sumMeasuredPts.elem[j] += measuredPts[i].elem[j] * weightPts[i];   //summation
          }          
        } 

        //create innovation covariance
        for(i = 0; i <= 2*m_Nx; i++)
        {
          for(int j=0; j<m_Nx; j++)
          {
            measuredDiff[j] = measuredPts[i].elem[j] - sumMeasuredPts.elem[j];  //(Zi - Zk)
          }

          double weightTmp = weightPts[i];

          for(int j=0; j<m_Nx; j++)
          {
            for(int k=0; k<m_Nx; k++)
            {
              double temp =  measuredDiff[j]*measuredDiff[k] * weightTmp; 
              tempMat.put(j,k,temp);              
            }
          }
          
          innoCov += tempMat; //Pzz
        }
      
        tempMat.fill(0);  //reset tempMat

        //create cross-covariance
        for(i = 0; i <= 2*m_Nx; i++)
        {
          for(int j=0; j<m_Nx; j++)
          {
            measuredDiff[j] = measuredPts[i].elem[j] - sumMeasuredPts.elem[j];  //(Zi - Zk)
            sampleDiff[j] = samplePts[i].elem[j] - jointPose.elem[j]; //(Xi - Xk)
          }

          double weightTmp = weightPts[i];

          for(int j=0; j<m_Nx; j++)
          {
            for(int k=0; k<m_Nx; k++)
            {
              double temp =  sampleDiff[j]*measuredDiff[k] * weightTmp; 
              tempMat.put(j,k,temp);              
            }
          }

          crossCov += tempMat; //Pxz
        }


        //-- Step5: Kalman gain and final state update

        vnl_matrix<double> innoCovInv; //(Pzz)^-1 : inverse of innovation covariance
        //innoCovInv.set_size(m,m);
        vnl_matrix<double> kalmanMat;
        //kalmanMat.set_size(m,m);
        
        innoCovInv = vnl_matrix_inverse<double>(innoCov)// crossCov; 
        //innoCovInv = vnl_svd_inverse(innoCov); //  --- ???
        kalmanMat = crossCov * innoCovInv;

        vnl_vector<double> constraintDiff;
        constraintDiff.set_size(m_Nx);
        vnl_vector<double> tempDiff;
        tempDiff.set_size(m_Nx);
        vnl_vector<double> jointPoseOriginal;
        jointPoseOriginal.set_size(m_Nx);
        vnl_vector<double> jointPoseFinal;
        jointPoseFinal.set_size(m_Nx);

        for(int j=0; j<m_Nx; j++)
        {
          double temp = jointPose.elem[j] - sumMeasuredPts.elem[j];  //TODO: jointPose should be end-effectors constraints
          constraintDiff.put(j, temp);

          jointPoseOriginal.put(j, jointPose.elem[j]);
        }

        tempDiff = kalmanMat * constraintDiff;
        jointPoseFinal = jointPoseOriginal + tempDiff;

      }//end if (vmeJoint != NULL)

    } //end if 
  } //end for each child


  //--TODO: 
    //   End For each frame
    ********************************************************************/
}

////----------------------------------------------------------------------------
//void lhpOpMotionRetargeting::KinematicConstraints(std::vector<Pose3D> samplePoints, std::vector<Pose3D> &measuredPoints)
//{
//  //TODO: apply kinematic constraints to sample points using forward kinematics
//
//  //for now just copy input motion into output
//  measuredPoints.assign(samplePoints.begin(), samplePoints.end());
//
//}

//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
      case ID_MY_OP:
        // ToDo: add something here...
      break;
      case wxOK:
        OpStop(OP_RUN_OK);        
      break;
      case wxCANCEL:
        OpStop(OP_RUN_CANCEL);        
      break;
    }
  }
}

////----------------------------------------------------------------------------
//void lhpOpMotionRetargeting::ExtractRotation(const mafMatrix &source, mafMatrix3x3 &target)
////----------------------------------------------------------------------------
//{
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//		  target.SetElement(i,j, source.GetElement(i,j));
//		}
//	}
//  target.Modified();
//}

//----------------------------------------------------------------------------
bool lhpOpMotionRetargeting::HasSameHierarchy(medVMEJoint* pJointA, medVMEJoint *pJointB)
{
    if (pJointA == NULL && pJointB == NULL)
        return true;

    if (pJointA == NULL || pJointB == NULL)
        return false;

    double epsion = 1e-10;
    double axisA[3], axisB[3];
    for (int i = 0; i < 3; ++i) {
        bool validAxisA = pJointA->GetJointRotationAxis(i, axisA);
        bool validAxisB = pJointB->GetJointRotationAxis(i, axisB);
        if (validAxisA != validAxisB          ||
            abs(axisA[0] - axisB[0]) > epsion || 
            abs(axisA[1] - axisB[1]) > epsion || 
            abs(axisA[2] - axisB[2]) > epsion)
            return false;
    }

    if (pJointA->GetNumberOfChildren() != pJointB->GetNumberOfChildren())
        return false;

    bool sameHierarchy = true;
    for (int i = 0; i < pJointA->GetNumberOfChildren(); ++i) {
        medVMEJoint *pChildA = medVMEJoint::SafeDownCast(pJointA->GetChild(i));
        medVMEJoint *pChildB = medVMEJoint::SafeDownCast(pJointB->GetChild(i));
        sameHierarchy &= HasSameHierarchy(pChildA, pChildB);
    }
    return sameHierarchy;
}

//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::CopyJointDirection(medVMEJoint* pSourceJoint, medVMEJoint* pTargetJoint)
{
    double dir[3];
    pTargetJoint->GetJointDirection(dir);
    pSourceJoint->SetJointDirection(dir);

    int numOfChild = pTargetJoint->GetNumberOfChildren();
    for (int i = 0; i < numOfChild; ++i) {
        medVMEJoint *pTargetChild = medVMEJoint::SafeDownCast(pTargetJoint->GetChild(i));
        medVMEJoint *pSourceChild = medVMEJoint::SafeDownCast(pSourceJoint->GetChild(i));
        CopyJointDirection(pSourceChild, pTargetChild);
    }
}

//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::GetSourceMotionParameters(mafTimeStamp timeStamp, std::vector<double>& motion)
{
    m_SourceJointRoot->SetTimeStampRecursive(timeStamp);

    // root position
    double position[3];
    medVMEOutputJoint *output = medVMEOutputJoint::SafeDownCast(m_SourceJointRoot->GetOutput());
    output->GetJointAbsPosition(position);
    motion.clear();
    motion.push_back(position[0]);
    motion.push_back(position[1]);
    motion.push_back(position[2]);

    // joint angles
    m_SourceJointRoot->GetRotationParameters(motion);
}

//----------------------------------------------------------------------------
void lhpOpMotionRetargeting::MotionRetargeting()
{
    double Pk = 1e-8;
    double kappa = 3.0;	// lambda = alpha * alpha * (L - kappa) - L; kappa = 3 - L; alpha is in [1e-4, 1]
    int m = m_SourceJointRoot->GetNumberOfEndEffectors();
    int n = m_SourceJointRoot->GetNumberOfValidAxis() + 3;  // plus root position (3 freedom)
    int M = m * 3;  // number of the end effectors * 3
    double delta = sqrt((n + 3) * Pk);		// k = 3?
    std::vector<double> motionParam(n);
    std::vector<double> weights(2 * n + 1);
    std::vector<double> sourceEndEffectorPos;
    std::vector<double> targetEndEffectorPos;
    std::vector< std::vector<double> > samples(2 * n + 1);
    vnl_vector<double> parameterDiff(n);
    vnl_vector<double> constraintDiff(M);

    // init samples' weights
    for (int i = 0; i < 2 * n + 1; ++i)
        weights[i] = 1 / (2 * n + 2 * kappa);
    weights[n] = kappa / (n + kappa);

    // time stamps
    std::vector<mafTimeStamp> timeStamps;
    m_SourceJointRoot->GetMainActorVME()->GetTimeStamps(timeStamps);
    for (size_t k = 0; k < timeStamps.size(); ++k) {
        // step 1: predict
        sourceEndEffectorPos.clear();
        GetSourceMotionParameters(timeStamps[k], motionParam);
        m_SourceJointRoot->GetEndEffectorPosition(sourceEndEffectorPos);

        // step 2: construct (2n + 1) samples
        samples[n] = motionParam;
        for (int i = 0; i < n; ++i) {
            samples[i]             = motionParam;
            samples[i][i]         -= delta;
            samples[n + 1 + i]     = motionParam;
            samples[n + 1 + i][i] += delta;
        }

        // step 3: measurement using forward kinematic
        targetEndEffectorPos.clear();
        for (int i = 0; i < 2 * n + 1; ++i) {
            m_TargetJointRoot->SetMotionParameters(motionParam);    // 0-2 is the root position, 3-n is the angles 
            m_TargetJointRoot->CalcEndEffectorPosition(targetEndEffectorPos);
        }

        // step 4: sum
        // weighted predicated measurement
        std::vector<double> weightedEndPos(M, 0.0);
        for (int i = 0; i < 2 * n + 1; ++i)
            for (int j = 0; j < M; ++j)
                weightedEndPos[j] += targetEndEffectorPos[i * M + j] * weights[i];

		// innovation covariance Pzz and cross-covariance Pxz
        vnl_matrix<double> innoCov(M, M, 0.0);  // Pzz -> innovation covariance
        vnl_matrix<double> crossCov(n, M, 0.0); // Pxz -> cross-covariance
        for (int i = 0; i < 2 * n + 1; ++i) {
            for (int j = 0; j < n; ++j)
                parameterDiff[j] = samples[i][j] - motionParam[j];
            for (int j = 0; j < M; ++j)
                constraintDiff[j] = targetEndEffectorPos[i * M + j] - weightedEndPos[j];
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
            constraintDiff[i] = sourceEndEffectorPos[i] - weightedEndPos[i];
        parameterDiff = kalmanMat * constraintDiff;
        for (int i = 0; i < n; ++i)
            motionParam[i] = motionParam[i] + parameterDiff[i];
        // save the estimated motion parameters
        // ??
    }
}
