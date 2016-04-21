/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateParticle.cpp,v $
  Language:  C++
  Date:      $Date: 2012-04-04 16:02:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Yubo Tao
==========================================================================
 Copyright (c) 2012
 University of Bedfordshire
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <sstream>
#include "medOpCreateParticle.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVMEOutputSurface.h"
#include "mafVMELandmarkCloud.h"
#include "../VME/medVMEMuscleWrapper.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(medOpCreateParticle);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
medOpCreateParticle::medOpCreateParticle(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;  
}

//----------------------------------------------------------------------------
medOpCreateParticle::~medOpCreateParticle( ) 
//----------------------------------------------------------------------------
{
  mafDEL(m_Output);
}

//----------------------------------------------------------------------------
mafOp* medOpCreateParticle::Copy()   
//----------------------------------------------------------------------------
{
	return new medOpCreateParticle(m_Label);
}

//----------------------------------------------------------------------------
bool medOpCreateParticle::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
   return node && node->IsMAFType(medVMEMuscleWrapper);
}

//----------------------------------------------------------------------------
void medOpCreateParticle::OpRun()   
//----------------------------------------------------------------------------
{
    mafVMELandmarkCloud *cloud = 0;
    mafNEW(cloud);
    cloud->Close();

    medVMEMuscleWrapper *wrapper = dynamic_cast<medVMEMuscleWrapper*>(m_Input);
    
    // set landmark cloud name
    std::stringstream ss;
    ss<<"P_"<<wrapper->GetName();
    cloud->SetName(ss.str().c_str());
    
    for(size_t i = 0; i < wrapper->m_Particles.size(); ++i) {
        ss.clear();
        ss.str("");
        ss<<i;
        medVMEMuscleWrapper::CParticle &particle = wrapper->m_Particles[i];
        cloud->AppendLandmark(particle.position[0], particle.position[1], particle.position[2], ss.str().c_str());
    }
    
    m_Output = cloud;
    mafEventMacro(mafEvent(this, OP_RUN_OK));
}

//----------------------------------------------------------------------------
void medOpCreateParticle::OpDo()
//----------------------------------------------------------------------------
{	
	if (m_Output != NULL)
		m_Output->ReparentTo(m_Input);
}

//----------------------------------------------------------------------------
void medOpCreateParticle::OpUndo()
//----------------------------------------------------------------------------
{
	if (m_Output != NULL)
		m_Output->ReparentTo(NULL);	
}
