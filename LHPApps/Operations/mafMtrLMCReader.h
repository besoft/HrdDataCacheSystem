/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafMtrLMCReader.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mafMTRLMCReader_h
#define __mafMTRLMCReader_h

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------
#include "mafVMELandmarkCloud.h"
#include <vector>


#define mafSetStringMacroNoDebug(name) \
virtual void Set##name (const char* _arg) \
  { \
  if ( this->m_##name == NULL && _arg == NULL) { return;} \
  if ( this->m_##name && _arg && (!strcmp(this->m_##name,_arg))) { return;} \
  if (this->m_##name) { delete [] this->m_##name; } \
  if (_arg) \
    { \
    this->m_##name = new char[strlen(_arg)+1]; \
    strcpy(this->m_##name,_arg); \
    } \
   else \
    { \
    this->m_##name = NULL; \
    } \
  } 



//----------------------------------------------------------------------------
// class mafMTRLMCReader
//----------------------------------------------------------------------------
class mafMTRLMCReader
{
public:
  static const int SetNotDefined = -1;
  //void PrintSelf(ostream& os, vtkIndent indent);

  // Construct object with merging set to true.
  static mafMTRLMCReader *New();

  // Specify file name of stereo lithography file.
  mafSetStringMacroNoDebug(FileName);

  void SetSet(int set) {m_Set = set;}
  void SetRadius(double radius){m_Radius = radius;}
  int  GetPointsRead() {return m_PointsRead;}
  const std::vector<std::pair<mafVMELandmarkCloud*, int> > &GetClouds() {return m_PointSet;}

  void Execute();

  ~mafMTRLMCReader();

protected:
  mafMTRLMCReader();

  char                               *m_FileName;
  int                                m_Set;
  int                                m_PointsRead;
  double                             m_PointShift;
  double                             m_Radius;
  std::vector<std::pair<mafVMELandmarkCloud*, int> >  m_PointSet;
  int ReadASCIIMTR(FILE *fp);
private:
  mafMTRLMCReader(const mafMTRLMCReader&);  // Not implemented.
  void operator=(const mafMTRLMCReader&);  // Not implemented.
};
#endif
