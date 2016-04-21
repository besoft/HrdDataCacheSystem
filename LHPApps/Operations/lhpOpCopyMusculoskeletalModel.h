/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCopyMusculoskeletalModel.h,v $
  Language:  C++
  Date:      $Date: 2011-04-11 07:18:22 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011-2012
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpCopyMusculoskeletalModel_H__
#define __lhpOpCopyMusculoskeletalModel_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "medMSMGraph.h"
#include <map>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;

//----------------------------------------------------------------------------
// lhpOpCopyMusculoskeletalModel :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpCopyMusculoskeletalModel: public mafOp
{
 protected:
	typedef std::map< mafID, mafID > lhpVMEIds;	///<lookup table used to Update Links
	bool m_bFilteringEnabled;			///<true, if VMEs that cannot be fused with the motion data should be excluded from the copy

public:
  lhpOpCopyMusculoskeletalModel(const wxString &label = "Clone Musculoskeletal Model");
  ~lhpOpCopyMusculoskeletalModel(); 

  mafTypeMacro(lhpOpCopyMusculoskeletalModel, mafOp);	

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

	/** Enables or disables filtering of VMEs that are irrelevant to be copied (e.g. for motion data fusion
	presence of muscles, muscle geometry, etc. in Current-Pose atlas is meaningless).*/
	virtual void EnableFiltering(bool bEnabled);
	
protected:
	/**  Creates a copy of the whole subtree identified by node and places it as a child of parent.
	Returns the pointer to the created subtree.*/
	virtual mafNode* CopyTree(mafNode* node, mafNode* parent);
		
	/**  Update links of VMEs present in subtree identified by node in such a manner that
	1) if a linked VME was also copied, the link is updated to refer to the copy of linked VME
	2) if the VME is medVMEMuscleWrapper, current pose links are processed as 1), rest pose untouched. 
	Original IDs and IDs of their copies are given in hash. */
	virtual void UpdateLinks(mafNode* node, const lhpVMEIds& hash);

protected:
	typedef medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor MSMGraphNodeDescriptor;

	/** Recursive method called from CopyTree that creates a copy of VMEs described by the subtree msm_node
	and places the node into parent and returns the instance of created subtree or NULL, if some constraints
	disallow its construction. In mapIds are returned Ids of VMEs so the links could be updated after CopyTree.
	BEWARE: if a node (e.g., with ID=2198) is linked copied (to a shortcut VME with ID= 2789 referring to ID 2198),
	mapIDs will contain a pair (2198, 2789) and, if links are update without taking care
	we will end up with a shortcut referring to itself*/
	mafNode* CopyTreeR(const medMSMGraph::MSMGraphNode* msm_node, mafNode* parent, lhpVMEIds& mapIds);

	/** Returns true, if the VME represented by the given msm_node may be DeepCopied. 
	When overriden in inherited classes, at least one of CanBeDeepCopied and CanBeLinkCopied methods should
	return true, if filtering is not enabled (see EnableFiltering).
	N.B.: DeepCopy is always preferred to LinkCopy. */
	inline virtual bool CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node){
		return true;
	}

	/** Returns true, if for the VME represented by the given msm_node, a shortcut may be created. 
	When overriden in inherited classes, at least one of CanBeDeepCopied and CanBeLinkCopied methods should
	return true, if filtering is not enabled (see EnableFiltering).
	N.B.: DeepCopy is always preferred to LinkCopy. */
	inline virtual bool CanBeLinkCopied(const medMSMGraph::MSMGraphNode* msm_node) {
		return false;	//by default nothing is to be LinkCopied since mafVMEShortcut does not work acceptably
	}
};

#endif //__lhpOpCopyMusculoskeletalModel_H__