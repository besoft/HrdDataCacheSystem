/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVMEPoseGroup.h,v $
  Language:  C++
  Date:      $Date: 2010-03-18 15:48:02 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2002/2007
  SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/
 
#ifndef __lhpVMEPoseGroup_h
#define __lhpVMEPoseGroup_h

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafVMEGeneric.h"
#include "mafVMEOutput.h"
#include "mafVMEOutputNULL.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// class forwarding:
//----------------------------------------------------------------------------

/**
class lhpVMEPoseGroup: dummy class that implements a time-varying group
*/
class LHP_VME_EXPORT lhpVMEPoseGroup : public mafVMEGenericAbstract
{
public:
  mafTypeMacro(lhpVMEPoseGroup,mafVMEGenericAbstract);

  /** return an xpm-icon that can be used to represent this node */
  static char ** GetIcon();

  /** Return the right type of output.*/  
  mafVMEOutputNULL *GetVTKOutput() {return (mafVMEOutputNULL *)GetOutput();};

  /** Return the output. This create the output object on demand. */  
  virtual mafVMEOutput *GetOutput();

  /** override superclass */
  void Print(std::ostream& os, const int tabs);

protected:
    /** default ctor */
  lhpVMEPoseGroup();

    /** default dtor */
  virtual ~lhpVMEPoseGroup();

  /** Internally used to create a new instance of the GUI.*/
  virtual mafGUI *CreateGui();

private:
  lhpVMEPoseGroup(const lhpVMEPoseGroup&); // Not implemented
  void operator=(const lhpVMEPoseGroup&); // Not implemented
};

#endif
