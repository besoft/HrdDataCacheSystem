/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpOpScaleMMA.h,v $
Language:  C++
Date:      $Date: 2011-10-21 16:16:06 $
Version:   $Revision: 1.1.2.2 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpOpScaleMMA_H__
#define __lhpOpScaleMMA_H__

#include "mafop.h"
#include "lhpOperationsDefines.h"

#include "lhpOpCopyMusculoskeletalModel.h"
#include "mafVMESurface.h"
#include "mafVMELandmarkCloud.h"
#include <vector>
#include <set>

#include "medMSMGraph.h"
#include  <vtkMAFSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMAFSmartPointer.h>

class vtkPoints;

extern "C"
{
	#include"csparse.h"
}

typedef struct problem_struct
{
  cs *A;
  cs *C;
  int sym;
  double *x;
  double *b;
  double *r;
} problem;

//hpOpCopyMusculoskeletalModel 

class LHP_OPERATIONS_EXPORT lhpOpScaleMMA : public lhpOpCopyMusculoskeletalModel
{

public:
	lhpOpScaleMMA(wxString label = "ScaleMMA");
	~lhpOpScaleMMA(void);

	mafTypeMacro(lhpOpScaleMMA, mafOp);

	mafOp* Copy();

	/** Class for handle mafEvent*/
	virtual void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* vme);

	/** 
	Static copy of Accept(), required so that we can pass the function
	pointer to the VME_CHOOSE event */
	static bool AcceptStatic(mafNode* vme);

	//bool AcceptSurface(mafNode *node);

	bool lhpOpScaleMMA::AcceptLandmark(mafNode *node);

	//bool InputReady();

	/** Builds operation's GUI by calling CreateOpDialog() method. */
	void OpRun();

	/** Execute the operation. */
	virtual void OpDo();

	/** Makes the undo for the operation. */
	void OpUndo();

	/** remove the panel on operation exit */
	//void HideGui();

	enum BODY_DIR {
		LEFT, 
		RIGHT,
	};

protected:
	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);

	/** create op GUI panel */
	void CreateGUI();

	/** Returns true, if the VME represented by the given msm_node may be DeepCopied. 
	When overriden in inherited classes, at least one of CanBeDeepCopied and CanBeLinkCopied methods should
	return true, if filtering is not enabled (see EnableFiltering).
	N.B.: DeepCopy is always preferred to LinkCopy. */
	inline virtual bool CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node){
		if (msm_node->m_NodeDescriptor.m_Flags & medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::HullNode)
			return false;
		else
			return true;
	}

	struct LdMap {
		int  CloudId;
		int  LdId;
		int SurfId;
		int PtId;

		LdMap(int c, int l, int s, int p) : CloudId(c), LdId(l), SurfId(s), PtId(p) { };
	};

	//void OnChooseMuscle();
	//void OnChooseLandmarkMuscle();
	void OnChooseLandmarkPatient();

	/** if the landmark cloud set is close to the muscle surface */
	bool IsClose(mafVMELandmarkCloud * pCloud, vtkPolyData * pMuscle, double threshold);

	/** calculate the average distance of  the landmark cloud set is close to the muscle surface */
	double AverageDistance(mafVMELandmarkCloud * pCloudVme, vtkPolyData * pSurf);
	double PtSurfDistance(mafVMELandmarkCloud * pCloudVme, int ptIdx, vtkPolyData* pSurf, int & closestPtId);

	void CalcPatellaLandmark(BODY_DIR dir);

	void CalcAnkleLandmark(BODY_DIR dir);

	/** main body of OpDo */
	void Execute();

	template<class T>  void LoadSurfaceFromList(medMSMGraph::MSMGraphNodeList & list, const char * parentName, const char * nodeName,  std::vector<T *> & outList);
	
	void LoadMotionLsFromList(medMSMGraph::MSMGraphNodeList & list, std::vector<mafVMELandmarkCloud *> & outList);
	
	void PrefetchDataInfo();
	void PrepareData();
	void CleanSurfaceList(std::vector<mafVMESurface* > & surfList, double rate,
		std::vector<vtkMAFSmartPointer<vtkPolyData> > &, std::vector<vtkMAFSmartPointer<vtkPolyData> > &, int &, double weight);
	void CleanAllSurfaceList();

	typedef std::pair<int, double> tAttSurfDist;;

	void FindAttachmentMap();
	void FindAttachmentMap(std::vector< std::vector<tAttSurfDist>   >  & attachSurfMap, 
		std::vector<std::vector<int> > & surfAttachMap, std::vector<vtkMAFSmartPointer<vtkPolyData> >  & surfList, double threshold);

	void FindMotionLsMap( std::vector< LdMap>  & motionLsMap, 
		/* std::vector<std::vector<int> > & surfAttachMap, */ std::vector<vtkMAFSmartPointer<vtkPolyData> >  & surfList, double threshold);

	static const int m_LodLevel = medMSMGraph::LOD::Highest;
	//const double m_fLodReduction;

	typedef std::multimap<mafNode *, mafNode * >tGroupNodeMap;
	/** parent node to node map  */
	tGroupNodeMap m_GroupNodeMap;

	// for comparing of pairs
	struct  AttSurfDistComp {
		bool operator() (tAttSurfDist i,tAttSurfDist j) { return (i.second<j.second);}
	};



	typedef std::pair<int, int> tIndexPair;
	typedef std::vector<tIndexPair> tPointPairList;


	mafNode * m_InputCopy;

	/** musculoskeletal graph pointer*/
	const medMSMGraph * m_pGraph;

	/** number of muscles */
	int m_nMuscle;
	/** number of bones */
	int m_nBone;
	/** number of attachments */
	int m_nAttach;
	/** number of ligaments */
	int m_nLigaments;
	/** number of MotionLs */
	int m_nMotionLs;

	/** number of muscles to be processed, for testing purpose */
	int m_nMuscleRun;
	int m_nBoneRun;

	double m_LdScale;

	/** muscle to attachment set  mapping */
	std::vector<std::vector<int> > m_MuscleAttachMap;
	std::vector<std::vector<int> > m_BoneAttachMap;
	std::vector<std::vector<int> > m_LigamentAttachMap;
	
	std::vector< std::vector<tAttSurfDist>   > m_AttachMuscleMap;
	std::vector<  std::vector<tAttSurfDist>  > m_AttachBoneMap;
	std::vector<  std::vector<tAttSurfDist>  > m_AttachLigamentMap;

	std::vector< LdMap> m_MotionLsMap;

	// input data
	std::vector<mafVMESurface* > m_MuscleList; // source template surface list
	std::vector<mafVMESurface* > m_BoneList; // source template surface list
	std::vector<mafVMESurface * > m_LigamentList;
	/** attachment landmark set list, one muscle may have multiple attachments */
	std::vector<mafVMELandmarkCloud * > m_AttachList;
	std::vector<mafVMELandmarkCloud * > m_MotionLsList;

	/** low res */
	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_MuscleListClean;
	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_BoneListClean;
	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_LigamentListClean;

	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_HighResMuscleListClean;
	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_HighResBoneListClean;
	std::vector<vtkMAFSmartPointer<vtkPolyData> >  m_HighResLigamentListClean;

	int m_nMuscleClean;
	int m_nBoneClean;
	int m_nLigamentClean;


	std::vector<vtkMAFSmartPointer<vtkPolyData> > m_SurfList;
	std::vector<vtkMAFSmartPointer<vtkPolyData> > m_SurfListCleanHighRes;
	std::vector<mafVMESurface * > m_SurfVMEList;
	std::vector<double> m_WeightList;
	/** muscle name list */
	std::vector<std::string> m_srcNameList;
	/** muscle point number list */
	std::vector<int> m_srcSizeList; // source template vertex count list
	/**start number of points in the matrix ( need to 3x ) */
	std::vector<int> m_srcStartIndexList; 
	/** start index of attachments */
	std::vector<int> m_AttachSizeList;
	std::vector<int> m_AttachStartIndexList; 
	/** start index of motion landmarks */
	std::vector<int> m_MotionLsSizeList;
	std::vector<int> m_MotionLsStartIndexList; 


	/** number of points in attachment sets */
	std::vector<int> m_srcLdSizeList;
	std::vector<int> m_srcLdSizeListBone;
	
	//std::vector<mafVMELandmarkCloud * > m_AttachVMEList;
	// target landmark list, hasn't been put in use */
	std::vector<mafVMELandmarkCloud * > m_targetLandmarkList;
	
	/** landmark peer points in corresponding muscles*/
	//std::vector<vtkPoints * > m_LandmarkPeer;
	//typedef double POINT3[3];
	
	struct POINT3
	{
		double x, y, z;
		POINT3() {}
		POINT3 (double p[3]) : x(p[0]), y(p[1]), z(p[2])
		{
		}
	};
	

	std::vector<std::vector<POINT3 > >m_LandmarkPeer;

	/** point ids of landmark peer points */
	std::vector< tPointPairList  > m_LandmarkPeerId;

	/** inter-muscle constraint list */
	std::vector<tPointPairList * > m_interSurfConstraintList;

	// for landmark peers
	std::vector<bool> m_LdValid; // if Landmark pairs
	std::vector<bool> m_UseOuterLd;
	vtkMAFSmartPointer<vtkPoints>  m_LdSource;
	vtkMAFSmartPointer<vtkPoints> m_LdTarget;
	vtkMAFSmartPointer<vtkPoints>  m_LdPeer;
	std::vector<int> m_LdPeerSurfId;
	std::vector<int> m_LdPeerPointId;

	// three landmark postions * 2
	mafVMESurface * m_pFemur[2];
	mafVMESurface * m_pPatella[2];
	mafVMESurface * m_pAnkle[2];

	int m_FemurIdx[2];
	int m_PatellaIdx[2];
	int m_AnkleIdx[2];




	double m_Scale[3];

	/** csparse matrix */
	cs * m_T;

	/** csparse b vector */
	std::vector<double> m_bvec;

	/** csparse problem */
	problem * m_pProb;

	/** current row number in matrix building */
	int m_nRow;


	// debuging variables 
	int m_maxN;
	int m_minN;
	int m_isolatedCount;
	int m_dupCount;
	int m_degen;

	//double m_avgdist;
	int m_totalPoint;

	// matrix handling
	/* add matrix to input surface and landmark data */
	void MatSurfLdInput();;
	/* add matrix to input slandmark data */
	void MatLdInput(mafVMELandmarkCloud * pVme);
	/* add matrix to input surface data - not implemented  */
	void MatSurfInput(mafVMESurface * pVme);

	/* add matrix to input surface and landmark data */
	void MatSurfLdOutput();;
	/* add matrix to output slandmark data */
	void MatLdOutput(mafVMELandmarkCloud * pVme);
	/* add matrix to input surface data - not implemented  */
	//void MatSurfOutut(mafVMESurface * pVme);


	/**  locate positions corresponding to patient landmarks */
	void FindLandmarkBones();
	int FindVMEIdx(mafVMESurface * pSurf);

	vtkMAFSmartPointer<vtkPolyData> CleanPolyData(vtkMAFSmartPointer<vtkPolyData> pPoly);
	vtkMAFSmartPointer<vtkPolyData> ReducePolyData(vtkMAFSmartPointer<vtkPolyData> pPoly, double threshold);

	/** ouput deformed muscle list */
	std::vector<mafSmartPointer <mafVMESurface> > m_OutSurface;
	std::vector<vtkMAFSmartPointer<vtkPolyData> > m_OutSurfaceVtk;

	// final output surface in high resolution
	std::vector<mafSmartPointer < mafVMESurface> > m_OutSurfaceHighRes;
	std::vector<mafSmartPointer <mafVMELandmarkCloud> > m_OutAttach;

	/** calculate correspondence between attachment points and muscle points */
	void CalcAttachmentConstraint();
	void CalcAttachmentConstraint(std::vector<std::vector<int> > & attachSurfMap, std::vector<vtkMAFSmartPointer<vtkPolyData> > & surfList, int startIdxStart);

	/** calculate correspondence of close points between muscles */
	//void CalcInterSurfConstraint();
	void CalcInterSurfConstraint(const std::vector<vtkMAFSmartPointer<vtkPolyData>> & surfList);

	/** add muscles to the lapalacian matrix */
	void AddSourceSurface(int srcId);

	/** add attachment constraints to the lapalacian matrix */
	void AddAttachmentConstraint();

	 void AddMotionLsConstraint();

	/** add inter-muscle constraintsto the lapalacian matrix */
	void AddInterMuscleConstraint(int idx1, int idx2);

	void AddLandmarkConstraint();

	void CalcLandmarkConstraint();
	/* to locate the femur head of the femur bone */
	void LocateFemurHead(BODY_DIR);

	void AddRegionToGroupMap(int parentId, std::vector<int> & childList);

	/** find close points between two surfaces */
	tPointPairList * FindClosePoints(vtkPolyData * m1, vtkPolyData * m2, double d, int step);

	/** generate problem */
	problem * GenProblem (cs * T, std::vector<double> & bvec, double tol);

	/** solve Ax=b in least square optimization based on QR decompostion */
	int Solve (problem *Prob);

	/** free problem */
	problem * free_problem (problem *Prob);


	/** event ids */
	enum 
	{
		ID_CHOOSE_MUSCLE = MINID,	/* choose gadolinium left inner ventricle */

		ID_CHOOSE_LANDMARK_MUSCLE,			/* choose template left inner ventricle */

		ID_CHOOSE_LANDMARK_PATIENT,
		ID_NUM_MUSCLE,
		ID_NUM_BONE,
	};


	mafString m_NameLandmarkPatient;

	mafVMELandmarkCloud * m_LandmarkPatient;





private:

};



#endif