/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper.h,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.1.1.2.20 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2001/2005
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __medVMEMuscleWrapper_h
#define __medVMEMuscleWrapper_h
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVME.h"
#include "medVMEMusculoSkeletalModel.h"
#include "vtkMAFMuscleDecomposition.h"
#include "vtkUnstructuredGrid.h"
#include "lhpDefines.h"
#include "lhpVMEDefines.h"
#include "vtkMEDPolyDataDeformationWrapperJH.h"

#if defined(VPHOP_WP10)
#define _DEBUG_VIS_
//#define VTK2OBJ

#if !defined(_DEBUG)
#define _PROFILING_
#define _FIBRES_LENGTS_OUTPUT_
#endif
#endif

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class vtkPoints;
class vtkPolyData;
class vtkTubeFilter;
class vtkMAFPolyDataDeformation;
class vtkMSSDataSet;
class vtkMassSpringMuscle;
class vtkMassSpringBone;
class mafVMESurface;

class wxListCtrl;
class mmaMaterial;

/** medVMEMuscleWrapper - */
class LHP_VME_EXPORT medVMEMuscleWrapper : public mafVME
{
public:
	mafTypeMacro(medVMEMuscleWrapper,mafVME);

	enum MUSCLEWRAPPER_WIDGET_ID
	{
		ID_INPUTMODE = Superclass::ID_LAST,
		ID_USE_FASTCHECKS,

		ID_SELECT_RP,
		ID_SELECT_CP,
		ID_SELECT_RP_REFSYS_LINK,
		ID_SELECT_CP_REFSYS_LINK,
		ID_ADDWRAPPER,
		ID_LIST_WRAPPERS,
		ID_REMOVEWRAPPER,

		ID_RESTPOSE_MUSCLE_LINK,
		ID_RESTPOSE_WRAPPER_LINK,
		ID_CURRENTPOSE_WRAPPER_LINK,
		ID_RESTPOSE_REFSYS_LINK,
		ID_CURRENTPOSE_REFSYS_LINK,
		ID_FIBERS_ORIGIN_LINK,
		ID_FIBERS_INSERTION_LINK,

		ID_GENERATE_FIBERS,
		ID_DECOMPOSITION_METHOD, //method for decomposition
		ID_FIBERS_TEMPLATE,     //template type
		ID_FIBERS_RESOLUTION,   //resolution
		ID_FIBERS_NUMFIB,       //number of fibers
		ID_FIBERS_THICKNESS,    //how thick a fiber should be

		ID_FIBERS_SMOOTH,					//smooth computed fibres
		ID_FIBERS_SMOOTH_STEPS,   //number of smooth steps
		ID_FIBERS_SMOOTH_WEIGHT,   //smoothing weight

		ID_FIBLER_UNIFORM_SAMPLING,         //unfirm sampling for particles
		ID_PARTICLE_CONSTRAINT_THRESHOLD,   //the threshold distance for the judgement of contrainted particles
		ID_PARTICLE_BONE_DISTANCE,          //the distance for the judgement of the second nearest bone of particles
		ID_SPRING_LAYOUT_TYPE,              //layout model of the spring used in the mass spring method. 0: 6modelCube, 1:26modelCube, 2:Delaunay, 3:N-nearest
		ID_CLOSEST_PARTICLE_NUMBER,         //number of closest particles to be used when layout type 3 is chosen

#ifdef _DEBUG_VIS_
		ID_DEFORM_DEBUG_SHOWALL,					//show input for deformation
		ID_DEFORM_DEBUG_SHOWPROGRESS,			//show steps of the deformation
		ID_FIBERS_DEBUG_SHOWTEMPLATE,			//show template but not projection
		ID_FIBERS_DEBUG_SHOWFITTING,			//show fitting process
		ID_FIBERS_DEBUG_SHOWFITTINGRES,		//show fitting process
		ID_FIBERS_DEBUG_SHOWSLICING,			//show slicing process
		ID_FIBERS_DEBUG_SHOWSLICINGRES,		//show slicing process
		ID_FIBERS_DEBUG_SHOW_CONSTRAINT_ONLY,    // show the constrainted particles only

		ID_FIBERS_DEBUG_SHOWOICONSTRUCT,			//show construction of attachment areas process
		ID_FIBERS_DEBUG_SHOWHARMFUNC,		//show slicing process
#endif

		ID_LAST,
	};

	enum MUSCLEWRAPPER_LINK_IDS
	{
		LNK_RESTPOSE_MUSCLE = 0,
		LNK_RESTPOSE_WRAPPERx,
		LNK_DYNPOSE_WRAPPERx,
		LNK_RESTPOSE_REFSYSx,
		LNK_DYNPOSE_REFSYSx,
		LNK_FIBERS_ORIGIN,
		LNK_FIBERS_INSERTION,

		LNK_LAST,
	};

	//this must match combobox values sets in CreateOpDialog
	enum FIBER_TEMPLATES
	{
		FT_PARALLEL = 0,
		FT_PENNATE,
		FT_CURVED,
		FT_FANNED,
		FT_RECTUS,
	};

	const static char* MUSCLEWRAPPER_LINK_NAMES[];

	//These are flags that are used to identify what has changed
	//since the last time the muscle wrapper produced its output.
	//
	//Checks are done in two modes:
	//1) Fast checks do not consider changes within input muscle VME or wrappers,
	//i.e., it is considered that once muscle is taken, its geometry do not change
	//and transformation matrix is the same. All other changes are detected.
	//2) Slow checks perform full checks to detect if geometry, matrices,
	//wrappers, etc. has not changed. It is slow but accurate.
	//Switching from one mode to the other one always cause the whole
	//"update" process.
	enum MODIFIED_FLAGS
	{
		NOTHING_MODIFIED = 0,							//no change at all, just skip the processing
		OUTPUT_SPACE_MODIFIED = 1,				//SetMatrix has been called => transformation + deformation + decomposition + poly output needed
		INPUT_MUSCLE_MODIFIED = 2,				//input muscle has changed => transformation + deformation + decomposition + poly output needed
		INPUT_WRAPPER_MODIFIED = 4,				//wrappers have changed => transformation + deformation + decomposition + poly output needed
		DEFORMATION_OPTIONS_MODIFIED = 8,	//deformation is needed =>  decomposition (if fibres are to be displayed) + poly output needed
		FIBERS_OPTIONS_MODIFIED = 16,			//decomposition is needed => poly output needed	 (if fibres are to be displayed)
		POLYDATA_MUST_UPDATE = 32,				//set, when poly output m_PolyData still must be updated		
		EVERYTHING_MODIFIED = 63,					//typically during serialization
	};

	//what kind of muscle decomposition will be used
	enum GEN_FIBER_OPTION
	{
		GF_SLICE,					//LHDL original slice method
		GF_SLICE_ADV,			//VPHOP slice method
		GF_MMSS,					//VPHOP mass spring method
		GF_UFP,           //VPHOP updated from particle method
#ifdef ADV_KUKACKA
		GF_KUKACKA,				//VPHOP Kukacka method
#endif
	};

public:
#pragma region Particle
	class CParticle
	{
	public:
		bool   fixed;                       // indicate whether the particle is attached on the bone and transformed with the bone
		bool   boundary;										// true if the particle is on a boundary fibre
		int    boneID[2];                   // the two nearest bone ID, which the particle is first transformed with
		int    fiberID;                     // the fiber ID in this muscle wrapper, used to reconstruct the fiber from particles
		int    prevParticle, nextParticle;  // the particle ID in the same fiber
		double boneWeight;                  // the weight of the nearest bone, used for the transformation: T1 * w + T2 * (1 - w)
		double position[3];                 // the position of the particle
		std::vector<int> neighbors;         // neighbor particle ID

		CParticle(int fiberIndex, double pos[3]) {
			fixed   = false;
			boundary= false;
			boneID[0] = -1;
			boneID[1] = -1;
			fiberID = fiberIndex;
			prevParticle = -1;
			nextParticle = -1;
			boneWeight = 1.0;  // only use the nearest bone's transformation matrix
			position[0] = pos[0]; position[1] = pos[1]; position[2] = pos[2];
		}

		void AddNeighbor(int neighborID) {
			for(size_t i = 0; i < neighbors.size(); ++i)
				if(neighbors[i] == neighborID)
					return;
			neighbors.push_back(neighborID);
		}
	};
	
	//m_Particles and m_ParticleMap stores the state of particle system in the current time of this wrapper
	//and they are used 1) to initialize the particle system and 2) to generate muscle fibres by UpdatedParticle decomposition method
	std::vector<CParticle> m_Particles;     // the particles generated from fibers
	std::vector<int> m_ParticleMap;         // the mapping from the sampled points of fibers to the index of particles (m_Particles)

	/** This class encapsulates all information about the particle system.
	There is only one instance being held by the MASTER wrapper in the current-pose. */
	class CParticleSystem
	{
	public:
		//here we will store all bone groups detected to be processed (in both RP and CP poses)
		medMSMGraph::mafVMENodeList m_BoneGroupsList_RP;	///<an array of bones (preferably bone groups) in their rest-pose
		medMSMGraph::mafVMENodeList m_BoneGroupsList_CP;	///<an arry of corresponding bone (preferably bone groups) in their current-pose		
		std::vector< vtkMassSpringBone* > m_MSSbones;			///<mass-spring systems for bones (obstacles)

		std::vector< medVMEMuscleWrapper* > m_MuscleWrappers_RP;		///<an array of muscle wrappers in their rest-pose for which the system was constructed
		std::vector< medVMEMuscleWrapper* > m_MuscleWrappers_CP;		///<an array of muscle wrappers in their current-pose (corresponding to m_MuscleWrappers_RP)
		std::vector< std::vector < int > >  m_MuscleBoneAssociation;///<contains an array of indices into m_BoneGroupsList_RP for each muscle wrapper

		std::vector< vtkMassSpringMuscle* > m_MSSmuscles;					///<mass-spring systems for muscles		
	public:
		~CParticleSystem();
	};

	CParticleSystem* m_pParticleSystem;

#pragma endregion

protected:
	mafVME* m_MuscleVme;							//<the input VME with muscle geometry in the rest pose
	unsigned long m_nMuscleChecksum;	//<the checksum of the input VME

	vtkPolyData* m_pTransformedMuscle;	//<cached transformed muscle
	vtkPolyData* m_pDeformedMuscle;			//<cached deformed muscle
	vtkPolyData* m_pDecomposedMuscle;		//<cached deformed muscle
	vtkTubeFilter* m_pTubeFilter;				//<tube filter for the visualization of fibers

	class CWrapperItem
	{
	public:
		enum MODIFIED_WRAPPERITEM_FLAGS
		{
			NOTHING_MODIFIED = 0,
			RP_WRAPPER_MODIFIED = 1,	//N.B. must be 1
			CP_WRAPPER_MODIFIED = 2,
			RP_REFSYS_MODIFIED = 4,
			CP_REFSYS_MODIFIED = 8,

			EVERYTHING_MODIFIED = 15,	//typically during serialization
		};

	public:
		mafVME* pVmeRP_CP[2];           //<VME with poly-line representing action lines of the muscle in the rest/current pose
		mafVME* pVmeRefSys_RP_CP[2];    //<VME with reference system

		/// cached data
		unsigned long nWrapperCheckSums[2];  //<checksums of these VMEs to prevent recalculation of everything
		unsigned long nRefSysChecksSums[2];	 //<checksums of Ref sys to prevent recalculation of everything

		vtkPolyData* pCurves[2];        //<the refined curves (valid as long as VMEChecksums of pVmeRP_CP are correct)
		double RefSysOrigin[2][3];      //<and origins of their associated referenced systems (ignored, if pVmeRefSys_RP_CP is NULL)

		int Modified;	//<flags what has been modified and must be updated - see enum MODIFIED_WRAPPERITEM_FLAGS

	public:
		CWrapperItem();
		CWrapperItem(const CWrapperItem& src);
		~CWrapperItem();
	};

	typedef std::vector< CWrapperItem* > CWrapperItemCollection;
	typedef CWrapperItemCollection::iterator CWrapperItemCollectionIterator;
	CWrapperItemCollection m_Wrappers;					//<the input wrappers
	bool m_bLinksRestored;											//<true, if m_Wrappers has been restored and the changes can be saved

	mafTransform *m_Transform;							///< pose matrix for the output (taken from input muscle)
	unsigned long m_nOutputSpaceChecksum;		///< checksum of m_Transform matrix

	vtkPolyData* m_PolyData;        //<the output polydata with either deformed surface or fibers
	int m_Modified;									//<flags what has changed since the last InternalPreUpdate
	//valid values are those from enum MODIFIED_FLAGS and their bitwise combinations

	medVMEMusculoSkeletalModel* m_pVMEMusculoSkeletalModel;						///<cached instance to medVMEMusculoSkeletalModel, created in InternalPreUpdate
	medVMEMusculoSkeletalModel::mafVMENodeList m_VMEMuscleWrappers;		///<cached instances to all muscle wrappers participating in the current deformation
	medVMEMuscleWrapper* m_MasterVMEMuscleWrapper;										///<this refers to the master muscle wrapper that will perform the deformation

	int  m_DeformationMethod;				///<cached from m_pVMEMusculoSkeletalModel
	int m_InterpolateStepCount;				///<cached from m_pVMEMusculoSkeletalModel
	bool m_UseProgressiveHulls;				///<cached from m_pVMEMusculoSkeletalModel
	bool m_UsePenetrationPrevention;	    ///<cached from m_pVMEMusculoSkeletalModel
	double  m_IterationStep;                   ///<cached from m_pVMEMusculoSkeletalModel
	int  m_IterationNum;                    ///<cached from m_pVMEMusculoSkeletalModel
	bool  m_UseFixedTimestep;                    ///<cached from m_pVMEMusculoSkeletalModel	

#pragma region Settings
	int m_VisMode;      //<non-zero, if the output are fibers instead of deformed mesh
	int m_UseRefSys;    //<non-zero, if the reference systems should be used whenever applicable
	int m_UseFastChecks;//<non-zero, if only fast detection of changes should be performed
	double m_LastUpdateTimeStamp;	//<last timestamp of the output - used when m_UseFastChecks = 1
	int m_DecompositionMethod;	///<decomposition method used for fibers
	int m_FbTemplate;   //<fiber template geometry
	int m_FbNumFib;     //<number of fibers
	int m_FbResolution; //<resolution
	double m_FbThickness; //<thickness of fibre
	int m_FbSmooth;		  //<non-zero, if fibres should be smoothed
	int m_FbSmoothSteps;  //<number of smoothing steps (higher means more smoothed)
	double m_FbSmoothWeight; //<smoothing weight (lower means more smoothed)
	int m_FbUniformSampling; //<0 = sampling using Slicing Method (by J. Kohout), 1 = uniform sampling for the particles
	double m_PConstraintThreshold;  //<the threshold distance for the judgement of constraints
	double m_PBoneDistance;    //<the distance threshold for the judgement of the second nearest bone of particles
	int m_PSpringLayoutType; //<layout model of the spring used in the mass spring method. 0: 6modelCube, 1:26modelCube, 2:Delaunay, 3:N-nearest
	int m_PClosestParticleNum; //<number of closest particles to be used when layout type 3 is chosen			

#ifdef _DEBUG_VIS_
	int m_DefDebugShow;						//<non-zero, if visualization of deformation input is to be done
	int m_DefDebugShowSteps;			//<non-zero, if progressive steps for the deformation should be displayed
	int m_FbDebugShowTemplate;		//<non-zero, if template fibres should be displayed, but not mapped ones
	int m_FbDebugShowFitting;			//<non-zero, if fitting process should be visualized (debug)
	int m_FbDebugShowFittingRes;	//<non-zero, if the result of fitting process should be visualized (debug)
	int m_FbDebugShowSlicing;			//<non-zero, if slicing process should be visualized (debug)
	int m_FbDebugShowSlicingRes;	//<non-zero, if the result of slicing process should be visualized (debug)
	int m_FbDebugFiberID;             //<the fiber ID to show its links (debug)
	int m_FbDebugShowConstraints;    //<non-zero, if only contraint particles should be displayed (debug)
	int m_FbDebugShowOIConstruction;	///<non-zero, if the construction of surface attachment areas should be displayed
#ifdef ADV_KUKACKA
	int m_FbDebugShowHarmFunc;				///<non-zero, if the harmonic scalar function should be displayed
#endif
#endif

	int m_LastMMConfirmTimeStamp;	//<last timestamp of the musculoskeletal model
#pragma endregion

#pragma region GUI
	mafString m_MuscleVmeName;      //<and its name to be shown in GUI
	mafVME* m_WrappersVme[2];        //<wrapper VMEs for AddWrapper command
	mafString m_WrappersVmeName[2];  //<and their names
	mafVME* m_RefSysVme[2];         //<VMEs with the reference system
	mafString m_RefSysVmeName[2];   //<and their name to be shown in GUI
	mafVME* m_OIVME[2];             //<VMEs with landmark(s) denoting origin and insertion areas
	mafString m_OIVMEName[2];       //<and their names

	wxStaticText* m_SmLabel1;
	wxTextCtrl* m_SmStepsCtrl;
	wxStaticText* m_SmLabel2;
	wxTextCtrl* m_SmWeightCtrl;
	wxStaticText* m_LabelRP;
	wxTextCtrl* m_RPNameCtrl;
	wxButton* m_BttnSelRP;
	wxButton* m_BttnAddWrapper;
	wxListCtrl* m_WrappersCtrl;
	wxButton* m_BttnRemoveWrapper;
	wxStaticText* m_LabelRP_RS;
	wxTextCtrl* m_RPRefSysVmeCtrl;
	wxButton* m_BttnSelectRPRefSys;
#pragma endregion GUI
public:
	static bool VMEAcceptMuscle(mafNode *node);
	static bool VMEAcceptWrapper(mafNode *node);
	static bool VMEAcceptOIAreas(mafNode *node);
	static bool VMEAcceptRefSys(mafNode *node);

	/** Precess events coming from other objects */
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	/** Copy the contents of another VME-Meter into this one. */
	/*virtual*/ int DeepCopy(mafNode *a);

	/** Compare with another VME-Meter. */
	/*virtual*/ bool Equals(mafVME *vme);

	/** Return the suggested pipe-typename for the visualization of this vme */
	/*virtual*/ mafString GetVisualPipe() {
		return mafString("mafPipeSurface");
	};

	/**
	Set the Pose matrix of the VME. This function modifies the MatrixVector. You can
	set or get the Pose for a specified time. When setting, if the time does not exist
	the MatrixVector creates a new KeyMatrix on the fly. When getting, the matrix vector
	interpolates on the fly according to the matrix interpolator.*/
	/*virtual*/ void SetMatrix(const mafMatrix &mat);

	/** Return the list of timestamps for this VME. Timestamps list is
	obtained merging timestamps for matrices and VME items*/
	/*virtual*/ void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes);

	/** return always false since (currently) the slicer is not an animated VME (position
	is the same for all timestamps). */
	/*virtual*/ bool IsAnimated();

	/** Gets muscle VME in its rest pose */
	mafVME* GetMuscleVME_RP();

	/** Gets the nIndex action line VME in its rest pose */
	mafVME* GetWrapperVME_RP(int nIndex);

	/** Gets the nIndex action line VME in its current pose */
	mafVME* GetWrapperVME(int nIndex);

	/** Gets the reference system VME for the nIndex action line in its rest pose. */
	mafVME* GetWrapperRefSysVME_RP(int nIndex);

	/** Gets the reference system VME for the nIndex action line in its current pose. */
	mafVME* GetWrapperRefSysVME(int nIndex);

	/** Gets the origin area VME in its current pose */
	mafVME* GetFibersOriginVME();

	/** Gets the insertion area VME in its current pose */
	mafVME* GetFibersInsertionVME();

	/** Return pointer to material attribute. */
	mmaMaterial *GetMaterial();

	/** Stores all meter links into Links list.
	N.B. This method is supposed to be call from InternalStore*/
	void StoreMeterLinks();

	/** Restore all meter links from Links list.
	N.B. this method is supposed to be called only after all VMEs were restored,
	i.e, it cannot be called from InternalRestore */
	void RestoreMeterLinks();

	/** Returns the instance of medVMEMusculoSkeletalModel under which this VME belongs.
	May return NULL, if there is no ancestor of medVMEMusculoSkeletalModel kind. */
	medVMEMusculoSkeletalModel* GetMusculoSkeletalModel();

	/** Returns the instance of medVMEMusculoSkeletalModel in the rest-pose under which this VME belongs.
	May return NULL, if there is no ancestor of medVMEMusculoSkeletalModel kind or RP is currently unavailable. */
	medVMEMusculoSkeletalModel* GetMusculoSkeletalModel_RP();

	/** Returns the instance of lower resolution of the given VME.
	Returns NULL, if there is no lower resolution for the given VME. */
	mafVME* GetLowerResolutionVME(mafVME* vme);

	/** Returns the lowest resolution of the given VME.
	Returns vme, if the given VME does not have any lower resolution. */
	inline mafVME* GetLowestResolutionVME(mafVME* vme)
	{
		mafVME* lower;
		while (NULL != (lower = GetLowerResolutionVME(vme))) {
			vme = lower;
		}

		return vme;
	}

	/** Returns the instance of er resolution of the given VME.
	Returns NULL, if there is no lower resolution for the given VME. */
	mafVME* GetHullVME(mafVME* vme);

	/** Creates a new surface VME to store hull of the given vme.
	Hull are automatically transformed from this coordinate system
	into the local coordinate system of the target VME. */
	void StoreHull(mafVME* vme, vtkPolyData* hull);

	/** Returns the object containing particles, springs and other data needed for processing of the
	mass spring system associated with the given VME.
	Returns NULL, if there is none. */
	mafVME* GetMSSParticlesVME(mafVME* vme);

	/** Creates a new VME containing particles, springs and other data needed for processing of the
	mass spring system  of the given vme (muscle). */
	void StoreMSSParticles(mafVME* vme, vtkPolyData* data);

	/** Returns the object containing bone as a set of balls.
	Returns NULL, if there is none. */
	mafVME* GetMSSBoneVME(mafVME* vme);

	/** Creates a new VME containing a bone as a set of balls. */
	void StoreMSSBone(mafVME* vme, vtkPolyData* data);

protected:
	medVMEMuscleWrapper();
	virtual ~medVMEMuscleWrapper();

	/** Stores this object into the storage element passed as argument. */
	/*virtual*/ int InternalStore(mafStorageElement *parent);

	/** Restores this object from the storage element passed as argument. */
	/*virtual*/ int InternalRestore(mafStorageElement *node);

	/** this creates the Material attribute at the right time... */
	/*virtual*/ int InternalInitialize();

	/** called to prepare the update of output
	it ensures that we have transformed input muscle and wrappers, so
	InternalUpdate may deform x deform + create fibers as needed.
	This method also changes m_Modified status. */
	/*virtual*/ void InternalPreUpdate() ;

	/** update the output data structure */
	/*virtual*/ void InternalUpdate();

	/** Sets m_Modified attribute according to what should be updated since it has changed
	Called from InternalPreUpdate. */
	virtual void MustUpdateCheck();

	/** It ensures that we have the transformed muscle surface and wrappers
	on the input, so the deformation may proceed. m_Modified flags are
	set in this method to allow determination, if input has changed.
	N.B. Must be called after MustUpdateCheck() + Called from InternalPreUpdate */
	virtual void PrepareInput();

	/** It ensures that we have the transformed wrappers on the input.
	N.B. Must be called after MustUpdateCheck() + called from PrepareInput */
	virtual void PrepareInputWrappers();

	/** It ensures that we have the transformed muscle surface on the input.
	N.B. Must be called after MustUpdateCheck() + called from PrepareInput */
	virtual void PrepareInputMuscle();

	/** Internally used to create a new instance of the GUI.*/
	/*virtual*/ mafGUI *CreateGui();

	/** Shows dialog (with the message in title) where the user selects vme.
	The VME that can be selected are defined by accept_callback.
	If no VME is selected, the routine returns false, otherwise it returns
	reference to the VME, its name and updates GUI */
	bool SelectVme(mafString title, long accept_callback,
		mafVME*& pOutVME, mafString& szOutVmeName);

	/** Updates the visibility (etc) of GUI controls */
	void UpdateControls();

	/** Creates a new polydata without duplicate vertices and edges that might be in the input. */
	void FixPolyline(vtkPolyData* input, vtkPolyData* output);

	/** Gets the origin of the specified vme.
	Returns false, if the vme is invalid. */
	bool GetRefSysVMEOrigin(mafVME* vme, double* origin);

	/** Deforms the transformed muscle according to existing wrappers.
	m_pTransformedMuscle -> m_pDeformedMuscle. */
	void DeformMuscle();
	
	/** Deforms the transformed muscle using Mass-spring system
	m_pTransformedMuscle -> m_pDeformedMuscle.*/
	void DeformMuscleMMSS();

	/** Generates fibers for the given muscle.
	m_pDeformedMuscle -> m_pDecomposedMuscle*/
	void GenerateFibers();

	/** Creates polydata from surface vme, and transforms its vertices into the common coordinate space
	N.B. the caller is responsible for deleting the returned object. */
	vtkPolyData* CreateTransformedPolyDataFromVME(mafVME* vme
#ifdef _DEBUG_VIS_
		, const char* dbgtext = NULL
#endif
		);

	/** Creates points from landmark cloud vme, landmark, etc. and transforms them into the common coordinate space
	N.B. the caller is responsible for deleting the returned object. */
	vtkPoints* CreateTransformedPointsFromVME(mafVME* vme);

	/** Creates points from the first (bStartPoints == true) or the last (bStartPoints == true) vertices of wrappers  and transforms them into the common coordinate space
	If bUseCP is true (default) the current pose wrapper is used, otherwise, the rest pose wrapper is used.
	N.B. the caller is responsible for deleting the returned object. */
	vtkPoints* CreateTransformedPointsFromWrappers(CWrapperItemCollection& wrappers, bool bStartPoints, bool bUseCP = true);

	/** Compute checksum for VTK polydata. */
	unsigned long ComputeCheckSum(vtkPolyData* pPoly);

	/** Adds a new wrapper into the list of wrappers and GUI list.
	pxP_RS denotes reference systems used for corresponding wrappers (optional). */
	void AddWrapper(mafVME* pRP, mafVME* pCP, mafVME* pRP_RS = NULL, mafVME* pCP_RS = NULL);

	/** Adds a new wrapper into the GUI list of wrappers */
	void AddWrapper(CWrapperItem* pItem);

	/** Remove wrapper from the GUI and releases its memory */
	void RemoveWrapper(int nIndex);

	/** Removes all wrappers from the list (NOT FROM GUI!) */
	void DeleteAllWrappers();

	/** Set a new link for the given vme.
	The link have name with prefix from the link table at index nLinkNameId (see LNK_ enums)
	and suffix nPosId. If nPosId is -1, no suffix is specified  */
	void StoreMeterLink(mafVME* vme, int nLinkNameId, int nPosId = -1);

	/** Restore vme with the specified link.
	Returns NULL, if there is no VME. IMPORTANT: removes vme from the Links list */
	mafVME* RestoreMeterLink(int nLinkNameId, int nPosId = -1);

	/** Transform the given inPoints having inTransform matrix into
	outPoints that have outTransform matrix (i.e., transforms coordinates
	from one reference system into another one. */
	void TransformPoints(vtkPoints* inPoints, vtkPoints* outPoints,
		const mafMatrix* inTransform, const mafMatrix* outTransform);

	/** Transform the coordinates of the inoutPoints that are given in the
	reference system described by inTransform into the coordinates in the
	output reference system (i.e., output of this VME) */
	inline void TransformPoints(vtkPoints* inoutPoints, const mafMatrix* inTransform){
		TransformPoints(inoutPoints, inoutPoints, inTransform,
			&GetOutput()->GetAbsTransform()->GetMatrix());
	}

	/** Transforms the coordinates of particles inParticles having inTransform matrix 
	into outParticles that  have outTransform matrix (i.e., transforms coordinates
	from one reference system into another one. */
	void TransformParticles(const std::vector< CParticle >& inParticles, std::vector< CParticle >& outParticles, 
		const mafMatrix* inTransform, const mafMatrix* outTransform);

	/** Transforms the coordinates of particles that are given in
	the reference system described by inTransform into the coordinates in the
	output reference system (i.e., output of this VME)  */
	inline void TransformParticles(std::vector< CParticle >& particles, const mafMatrix* inTransform){
		TransformParticles(particles, particles, inTransform,
			&GetOutput()->GetAbsTransform()->GetMatrix()); 
	}

	/** Gets the abs matrix for the given VME that can be used to
	transform points from Local Coordinates to World and vice versa */
	const mafMatrix* GetVMEAbsMatrix(mafVME* vme);

	/**
	Gets the transformation that transforms rest-pose pelvis onto current-pose pelvis and
	that can be used to minimize the differences between both poses and improve the stability of method.
	Returns false, if the matrix could not be calculated (e.g., because pelvis was not found in atlas)
	*/
	bool GetInitialTransform( mafTransform* trasform);

	/**
	Creates a new polydata with points transformed by the given transformation.
	N.B., the caller is responsible for deleting the returned object.*/
	vtkPolyData* TransformPolyData(vtkPolyData* poly, const mafTransform* transform);

	/** Calculates distance between two particles */
	double CalculateDistance(double posA[3], double posB[3]);

	/** Adds particles and remove duplicated particles */
	int AddParticle(int fiberID, double position[3], bool checkDuplicated = false);

	/** Finds all vertices connected with a given (seed) vertex of a unstructured mesh*/
	void GetConnectedVertices(vtkUnstructuredGrid * mesh, int seed, vtkIdList *connectedVertices);

	/** Creates neighbourhood between individual particles (to be springs) as edges of delaunay tetrahedronization of the particles. */
	void GenerateNeighbourParticlesDelaunay();

	/** Creates neighbourhood between individual particles (to be springs) by connecting each particle with m_NumClosestParticles closest other particles.*/
	void GenerateNeighbourParticlesClosest();

	/** Creates neighbourhood between individual particles (to be springs).
	Assumes the particls form a cubical grid. The neighbours are set as edges (and diagonals) of the grid.
	Setting of m_SpringLayoutType decides how many edges/diagonals are used.*/
	void GenerateNeighbourParticlesCubicLattice();

	/** Generates particles from the current fibers 
	N.B. this method is supposed to be called from InitializeParticleSystem_RP method only. */
	void GenerateParticles();

	/** Generates neighbors of particles 
	N.B. this method is supposed to be called from InitializeParticleSystem_RP method only. */
	void GenerateParticleNeighbors();

	/** Generates constraints for particles.
	Parameters: boneGroupsList_RP contains an array of all bones in the model, idxBones 
	contains indices (zero-based) into boneGroupsList_RP determining the bones being affecte by the muscle being wrapped.
	N.B. this method is supposed to be called from InitializeParticleSystem_RP method only. */
	void GenerateParticleConstraints(const medMSMGraph::mafVMENodeList& boneGroupsList_RP, const std::vector< int >& idxBones);

	/** Get the muscle wrapper VME at the rest position */
	medVMEMuscleWrapper* GetMuscleWrapper_RP();

	/** Gets the absolute transform matrices of all bones.	
	The method returns the transformation that would transform the rest-pose into the current-pose (in absolute coordinates).
	N.B. the caller is responsible for calling Delete method upon every returned item*/
	void GetAllBoneCurrentTransforms(const medMSMGraph::mafVMENodeList& bonesCP, std::vector<mafTransform*>& transforms);

	/** Get the bounding box of particles */
	void GetBoundingBoxOfParticles(double bounds[6]);

	/** Regenerate fibers from the sampled particles */
	void RegenerateFiberFromParticle();

	/** Updates the decomposed fibers using the deformed particles */
	void UpdateFiberFromParticle();

	/** Updates the showed particles in the children node */
	void UpdateChildrenParticleNode();

	/** Returns true, if the given transformation is identity, i.e., it does not change position of vertices being transformed by this transformation. */
	bool IsIdentityTransform(const mafTransform* transform);

	/** Initializes the particle system.
	Creates or updates m_pParticleSystem according to the current state.
	N.B. This method can be called from MASTER or SINGLE current-pose wrapper only. */
	void InitializeParticleSystem();

	/** Initializes the particle system for the rest-pose according to the current settings of the wrapper. 
	The system is created for the muscle being wrapped by this wrapper using the passed
	parameters: boneGroupsList_RP contains an array of all bones in the model, idxBones 
	contains indices (zero-based) into boneGroupsList_RP determining the bones being affected
	by the muscle being wrapped.
	N.B. This method is supposed to be called from a rest-pose wrapper only! */
	void InitializeParticleSystem_RP(const medMSMGraph::mafVMENodeList& boneGroupsList_RP, const std::vector< int >& idxBones);

	/** Returns indices to bones (given in the array 'bones') affected by the given muscle (in the rest-pose).
	N.B. bones array may contain a bone VME or a bone group VME in any resolution in the rest-pose musculoskeletal model.	*/
	void GetMuscleBoneAssociation(const mafVME* muscle, const medMSMGraph::mafVMENodeList& bones, std::vector< int >& out_idx); 

	/** Returns index of the bone VME (given in the array 'bones') that contains the model of bone identified by 'bonetype' tag (specified as BoneType enum).
	N.B.  bones array may contain a bone VME or a bone group VME in any resolution in the rest-pose musculoskeletal model.		
	If the bone could not be found (in any of its resolutions and bone groups) in 'bones' array, the method returns -1.*/
	int FindBone(const medMSMGraph::mafVMENodeList& bones, int bonetype);

private:
	medVMEMuscleWrapper(const medVMEMuscleWrapper&); // Not implemented
	void operator=(const medVMEMuscleWrapper&); // Not implemented

protected:
#ifndef _DEBUG_VIS_
#define Debug_Write	__noop
#define Debug_Visualize_Data __noop
#else
	void Debug_Write(const char* strText, vtkPolyData* pData, int limit = INT_MAX);

	/** Visualizes the result of deformation. */
	void Debug_Visualize_DeformationData();
#endif
};
#endif
