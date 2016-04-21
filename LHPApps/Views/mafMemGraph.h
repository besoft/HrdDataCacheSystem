/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafMemGraph.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Vladik Aranov/Fedor Moiseev    
==========================================================================
  Copyright (c) 2002/2004
  ULB - Universite Libre de Bruxelles
=========================================================================*/

#ifndef __mafMemoryGraph_H__
#define __mafMemoryGraph_H__

#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

#include <vector>

#ifndef DIM
#define DIM(a)  (sizeof((a)) / sizeof(*(a)))
#endif
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#define _FitIn(val, from, to)   max(from, min(val, to))

//----------------------------------------------------------------------------
// forward references
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// type definitions
//----------------------------------------------------------------------------
/**
Main class for graph in-memory representation*/

template <class T, class I, class G> class mafMemoryGraphBase
{
public:
  /*Current MaxMin parameters for both parts of graph */
  struct mafMemGrTotalRange
  {
    T rXMin;
    T rXMax;
    T rYMin;
    T rYMax;
  };
  /**
  Just a handle */
  typedef mafMemGrTotalRange *mafPMemGrTotalRange;
  /**
  public constructor */

  mafMemoryGraphBase(unsigned int nDimension = 2,unsigned int nSize = 1000);
  //and  destructor
  virtual ~mafMemoryGraphBase(void);
  /**
  extract point based on logical index
  nPoint is logical index
  nCoord the actual coordinate index*/
  T& operator()(unsigned int nPoint,unsigned int nCoord);

  /**
  Actual constructor work is done here*/
  void   Init(unsigned int nDimension = 2, unsigned int nSize = 1000);
  /**
  Remove all elements from in-memory graph container*/
  void   Clean();
  
  /**
  Support warp break index: needed for visualization*/
  unsigned int BreakBegin(void)               {return m_BreakBegin;}
  unsigned int BreakEnd(void)                 {return m_BreakEnd;}

  /**
  Add point as vector to graph storage */
  void   AddPoint(const std::vector<T> &vrPoint);
  /**
  Add only one coordinate: other will be undefined!  */
  void   SetAddCoord(unsigned int nCoord, T rValue);
  
  /**
  Get number of free memory in storage in elements */
  int  GetFreeMemSpace(void);
  /**
  Get number of used memory in storage in elements */
  int  GetUsedMemSpace(void);

  /**
  Slow: use is to shrink excises when needed from time to time */
  void   RecalculateRanges();

  //
  inline unsigned int GetDim()   {    return this->nDim;  }

  inline unsigned int GetXDim()  {    return this->nXDim; }

  inline unsigned int GetYDim()  {    return this->nYDim;  }

  // Following functions works with indexes. Each value can have unique int32 index 
  // that allow user to identify what kind of value stored in certain index
  inline unsigned int GetXIndex(unsigned int nIndex)
  {
    wxASSERT(nIndex<this->nXDim);
    return this->vnXIndexes[nIndex];
  }
  inline unsigned int GetYIndex(unsigned int nIndex)
  {
    wxASSERT(nIndex<this->nYDim);
    return this->vnYIndexes[nIndex];
  }

  void   SetDim(unsigned int nDimension);

  inline void   SetXDim(unsigned int nDimX)
  {
    wxASSERT(nDimX<=this->nDim);
    this->nXDim = nDimX;
    return;
  }
  inline void   SetYDim(unsigned int nDimY)
  {
    wxASSERT(nDimY<=this->nDim);
    this->nYDim = nDimY;
    return;
  }

  inline void   SetXIndex(unsigned int nIndex,unsigned int nXIndex)
  {
    wxASSERT(nIndex<this->nXDim);
    wxASSERT(nXIndex<this->nDim);
    this->vnXIndexes[nIndex] = nXIndex;
    return;
  }
  inline void   SetYIndex(unsigned int nIndex,unsigned int nYIndex)
  {
    wxASSERT(nIndex<this->nYDim);
    wxASSERT(nYIndex<this->nDim);
    this->vnYIndexes[nIndex] = nYIndex;
    return;
  }
 
  inline const I& GetID(unsigned int nCoord)
  {
    return this->vnIDs[nCoord];
  }
  inline void SetID(unsigned int nCoord, const I& nID)
  {
    wxASSERT(nCoord<this->nDim);
    this->vnIDs[nCoord] = nID;
    return;
  }

  inline const I& GetXID(unsigned int nIndex) {return GetID(GetXIndex(nIndex));}
  inline const I& GetYID(unsigned int nIndex) {return GetID(GetYIndex(nIndex));}

  inline void   SetXID(unsigned int nIndex, const I& nID)
  {
    SetID(GetXIndex(nIndex), nID);
    return;
  }
  inline void SetYID(unsigned int nIndex, const I& nID)
  {
    SetID(GetYIndex(nIndex), nID);
    return;
  }

  inline const G&  GetGraphID(){return this->nGraphID;}
  inline void   SetGraphID(const G& nID)
  {
    this->nGraphID = nID;
  }

  bool   AddYVar(const I& nID);
  bool   RemYVar(const I& nID);
  bool   SetYVar(unsigned int nIndex, const I& nID);

  bool   AddXVar(const I& nID);
  bool   RemXVar(const I& nID);
  bool   SetXVar(unsigned int nIndex, const I& nID);

  mafMemGrTotalRange const *GetRange(void);
private:
  struct mafMemGrRange
  {
    T rMin, rMax;
  };
  void      RemovePoints(int nPoints);
  void      RemovePoint();
  void      UpdateBreakPosition(void);
  void      RemovePrevPassPoints(void);
  virtual T GetGarbageValue(void) = 0;

  bool                                m_Empty;
  /// user data is stored here!!! 
  std::vector<std::vector<T> >        m_Points;
  ///last added point sometimes stored here. No user access. For SetAddCoord method.
  std::vector<T>                      m_Coords;
  /// marks what fields in last added coords are filled. For SetAddCoord method.
  std::vector<bool>                   m_Filled;
  /// number of already added fields. For SetAddCoord method.
  unsigned int                        m_FilledPoints;

  ///native index of array beginning
  unsigned int                        nBegin;       
  ///native index of array end
  unsigned int                        nEnd;         

  //normalized index for break
  unsigned int                        m_BreakBegin; 
  //normalized index for break end
  unsigned int                        m_BreakEnd;    

  ///maximum possible number of points
  unsigned int                        nMaxSize;
  ///number of graphs drawn at once
  unsigned int                        nDim;
  ///currently always == 1
  unsigned int                        nXDim;
  /// can be almost any :)
  unsigned int                        nYDim;

  ///Identification index for user Can be managed by user
  G                                   nGraphID;
  ///Can be managed by user
  std::vector<I>                      vnIDs;
  std::vector<unsigned int>           vnXIndexes;
  std::vector<unsigned int>           vnYIndexes;

  ///automatically recalculated ranges for visualization
  std::vector<mafMemGrRange>          vgrRanges;
  mafMemGrTotalRange                  gtrTotalRange;

  ///internal usage
  T                                   rUnnoticedRange;
  ///Slider move support: TODO
  bool                                bSecondaryBreak;
  //memory graph tunings
  T                                   rRangeGrowStep;
  T                                   rPointNoticeCriterion;
  T                                   rSpaceInRange;

};

/**
 * detailed description
 *
 * @memo    
 * @return  None
 * @param   nSize
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
mafMemoryGraphBase<T, I, G>::mafMemoryGraphBase(unsigned int nDimension, unsigned int nSize)
{
  this->Init(nDimension, nSize);

  return;
} // end of mafMemoryGraphBase<T, I, G>::mafMemoryGraphBase

/**
 * detailed description
 *
 * @memo    Get number of allocated entries
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
int mafMemoryGraphBase<T, I, G>::GetUsedMemSpace(void)
{
  return this->nMaxSize - this->GetFreeMemSpace();
} // end of mafMemoryGraphBase<T, I, G>::GetUsedMemSpace

/**
 * detailed description
 *
 * @memo    obv.
 * @return  None
 * @param   nI
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
T& mafMemoryGraphBase<T, I, G>::operator()(unsigned int nPoint, unsigned int nCoord)
{
  int   nActualIdx;

//  wxASSERT(this->nMaxSize > nPoint); //no valid data otherwise

  nActualIdx = this->nBegin + nPoint;
  if(nActualIdx >= (int)this->nMaxSize)
  {
    //perform round warp
    nActualIdx -= this->nMaxSize;
    wxASSERT(0 <= nActualIdx && nActualIdx < (int)this->nMaxSize);
  }
  //realay actual data
  return m_Points[nActualIdx][nCoord];
} // end of mafMemoryGraphBase<T, I, G>::[]


/**
 * detailed description
 *
 * @memo    Remove N obsolete points
 * @return  None
 * @param   nPoints
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void   mafMemoryGraphBase<T, I, G>::RemovePoints(int nPoints)
{
  int nI;

  for(nI = 0; nI < nPoints; nI++)
  {
    this->RemovePoint();
  }
  return;
} // end of mafMemoryGraphBase<T, I, G>::RemovePoints

/**
 * detailed description
 *
 * @memo    remove one point obsolete for some reasons
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void   mafMemoryGraphBase<T, I, G>::RemovePoint(void)
{
  unsigned int nI;

  //access storage
  
  if(this->m_Empty)
  {
    wxASSERT(this->nBegin == 0 && this->nEnd == 0);
    //nothing to do here anymore
    return;
  }
  else if(this->nBegin == this->nEnd)
  {
    //one element case
    //invalitate it
    for(nI=0;nI<this->nDim;nI++)
    {
      m_Points[this->nBegin][nI] = GetGarbageValue();
    }
    //remove it
    this->nBegin = 0;
    this->nEnd   = 0;
    this->m_Empty = true;
  }
  else
  {
    //have more than two elements here to spare
    //invalidate it
    for(nI=0;nI<this->nDim;nI++)
    {
      m_Points[this->nBegin][nI] = GetGarbageValue();
    }
    this->nBegin++;
    //check warp
    if(this->nBegin == this->nMaxSize)
    {
      this->nBegin = 0;
    }
    if(this->nBegin == this->nEnd)
    {
      //move it
      for(nI=0;nI<this->nDim;nI++)
      {
        m_Points[0][nI] = m_Points[this->nBegin][nI];
      }
      //remove it
      this->nBegin = 0;
      this->nEnd   = 0;
    }
  }

  this->UpdateBreakPosition();

  return;
} // end of mafMemoryGraphBase<T, I, G>::RemovePoint
  
/**
 * detailed description
 *
 * @memo    Add new point into the class
 * @return  None
 * @param   gpPoint
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void mafMemoryGraphBase<T, I, G>::AddPoint(const std::vector<T> &vrPoint)
{
  T   rDiff;
  T   rNoticeableVal;
  T   rLength;
  T   rAddGrow;
  unsigned int  nI;

  if(this->nBegin == this->nEnd && this->m_Empty)
  {
    //list is empty: just add to end
    m_Points[this->nEnd] = vrPoint;
    this->m_Empty = false;
    for(nI=0;nI<this->nDim;nI++)
    {
      vgrRanges[nI].rMin = vrPoint[nI] - rSpaceInRange;
      vgrRanges[nI].rMax = vrPoint[nI] + rSpaceInRange;
      //init axises
    }
    if(this->nXDim>0)
    {
      this->gtrTotalRange.rXMin=vgrRanges[vnXIndexes[0]].rMin;
      this->gtrTotalRange.rXMax=vgrRanges[vnXIndexes[0]].rMax;
      for(nI=1;nI<this->nXDim;nI++)
      {
        if(vgrRanges[vnXIndexes[nI]].rMin<this->gtrTotalRange.rXMin)
        {
          this->gtrTotalRange.rXMin=vgrRanges[vnXIndexes[nI]].rMin;
        }
        if(vgrRanges[vnXIndexes[nI]].rMax>this->gtrTotalRange.rXMax)
        {
          this->gtrTotalRange.rXMax=vgrRanges[vnXIndexes[nI]].rMax;
        }
      }
    }
    if(this->nYDim>0)
    {
      this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[0]].rMin;
      this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[0]].rMax;
      for(nI=1;nI<this->nYDim;nI++)
      {
        if(vgrRanges[vnYIndexes[nI]].rMin<this->gtrTotalRange.rYMin)
        {
          this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[nI]].rMin;
        }
        if(vgrRanges[vnYIndexes[nI]].rMax>this->gtrTotalRange.rYMax)
        {
          this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[nI]].rMax;
        }
      }
    }
  }
  else //add to non empty container
  {
    rDiff = vrPoint[0] - (*this)(this->GetUsedMemSpace() - 1,0);
    if(rDiff > 0.0f) //do not apply to warped points
    {
      rNoticeableVal = rPointNoticeCriterion * (vgrRanges[0].rMax - vgrRanges[0].rMin);
      if((rDiff + this->rUnnoticedRange) < rNoticeableVal)
      {
        //this point is not worth to be noticed ==> ignore, bat mark
        this->rUnnoticedRange += rDiff;
        return;
      }
      else
      {
        //adjust marker
        this->rUnnoticedRange = (rDiff + this->rUnnoticedRange) - rNoticeableVal;
        //do not affect to much
        this->rUnnoticedRange = _FitIn(this->rUnnoticedRange, 0.0f, rNoticeableVal);
      }
    }
    //update points rect
    for(nI=0;nI<this->nDim;nI++)
    {
      if(vrPoint[nI] > vgrRanges[nI].rMax)
      {
        rLength            = vgrRanges[nI].rMax - vgrRanges[nI].rMin;
        rAddGrow           = rLength * rRangeGrowStep * 0.01f;
        vgrRanges[nI].rMax = vrPoint[nI] + rAddGrow;
      }
      if(vrPoint[nI] < vgrRanges[nI].rMin)
      {
        rLength   = vgrRanges[nI].rMax - vgrRanges[nI].rMin;
        rAddGrow  = rLength * rRangeGrowStep * 0.01f;
        vgrRanges[nI].rMin = vrPoint[nI] - rAddGrow;
      }
      //update graphic axixes
    }
    if(this->nXDim>0)
    {
      this->gtrTotalRange.rXMin=vgrRanges[vnXIndexes[0]].rMin;
      this->gtrTotalRange.rXMax=vgrRanges[vnXIndexes[0]].rMax;
      for(nI=1;nI<this->nXDim;nI++)
      {
        if(vgrRanges[vnXIndexes[nI]].rMin<this->gtrTotalRange.rXMin)
        {
          this->gtrTotalRange.rXMin=vgrRanges[vnXIndexes[nI]].rMin;
        }
        if(vgrRanges[vnXIndexes[nI]].rMax>this->gtrTotalRange.rXMax)
        {
          this->gtrTotalRange.rXMax=vgrRanges[vnXIndexes[nI]].rMax;
        }
      }
    }
    if(this->nYDim>0)
    {
      this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[0]].rMin;
      this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[0]].rMax;
      for(nI=1;nI<this->nYDim;nI++)
      {
        if(vgrRanges[vnYIndexes[nI]].rMin<this->gtrTotalRange.rYMin)
        {
          this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[nI]].rMin;
        }
        if(vgrRanges[vnYIndexes[nI]].rMax>this->gtrTotalRange.rYMax)
        {
          this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[nI]].rMax;
        }
      }
    }

    //not empty
    //TBD: Check is this point already exists in container SLOW!
    //check for available space
    if(this->GetFreeMemSpace() >= 0)
    {
      //have space
      //check end position
      if(this->nBegin <= this->nEnd && this->nEnd < this->nMaxSize - 1)
      {
        //non warped and not ready to warp
        this->nEnd++;
      }
      else if(this->nBegin <= this->nEnd && this->nEnd == this->nMaxSize - 1)
      {
        //non warped and ready to warp
        this->nEnd = 0;
        //shift begin as well if needed
        if(this->nBegin == this->nEnd)
        {
          this->nBegin = this->nEnd + 1;
        }
      }
      else if(this->nBegin > this->nEnd)
      {
        //warped
        this->nEnd++;
        //check for space
        if(this->nEnd == this->nBegin)
        {
          this->nBegin = this->nEnd + 1;
        }
        //check for warp
        if(this->nBegin == this->nMaxSize)
        {
          this->nBegin = 0;
        }
      }
      else
      {
        wxASSERT(false);
      }
      //store this damned data
      wxASSERT(this->nEnd >= 0 && this->nEnd <= this->nMaxSize - 1);
      m_Points[this->nEnd] = vrPoint;
    }
    else
    {
      wxASSERT(false);
    }    
  }
  this->UpdateBreakPosition();
  this->RemovePrevPassPoints();
  //wxASSERT(!this->bSecondaryBreak);
  return;
} // end of mafMemoryGraphBase<T, I, G>::AddPoint


template <class T, class I, class G>
void  mafMemoryGraphBase<T, I, G>::SetAddCoord(unsigned int nCoord, T rValue)
{
  unsigned int nI;
  wxASSERT(nCoord < this->nDim);
  wxASSERT(!(this->m_Filled[nCoord]));

  this->m_Filled[nCoord] = true;
  this->m_Coords[nCoord] = rValue;
  (this->m_FilledPoints)++;
  if(this->m_FilledPoints == this->nDim)
  {
    this->m_FilledPoints = 0;
    for(nI=0;nI<this->nDim;nI++)
    {
      this->m_Filled[nI] = false;
    }
    AddPoint(m_Coords);
  }

  return;
}


/**
 * detailed description
 *
 * @memo    Remove points from prev. pass
 *          should be called after each one point added
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void  mafMemoryGraphBase<T, I, G>::RemovePrevPassPoints(void)
{
  int   nPoints, nI;
  unsigned int   nStage, nToRem;

  //init
  nPoints = this->GetUsedMemSpace();

  //do not work until having less than 3 points
  if(nPoints <= 3) 
  {
    return;
  }
  nStage = 0;
  nToRem = 0;
  for(nI = nPoints - 2; nI >= 0; nI--)
  {
    if((*this)(nI,0) > (*this)(nI+1,0))
    {
      nStage++;
    }
    if(nStage >= 2 || ((*this)(nPoints - 1, 0) > (*this)(nI,0) && nStage >= 1))
    {
      nToRem++;
    }
  }
  this->RemovePoints(nToRem);
  if(nToRem > 0)
  {
    this->UpdateBreakPosition();
  }
  return;
} // end of mafMemoryGraphBase<T, I, G>::RemovePrevPassPoints

/**
 * detailed description
 *
 * @memo    Support state for break index
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void  mafMemoryGraphBase<T, I, G>::UpdateBreakPosition(void)
{
  int   nI, nUsed;
  bool    bFilled;

  nUsed = this->GetUsedMemSpace();
  if(this->m_Empty)// || nUsed <= 4)
  {
    //no elements
    this->m_BreakBegin = this->m_BreakEnd = this->nEnd;
    return;
  }
  else if(this->nBegin == this->nEnd)
  {
    //only one element
    this->m_BreakBegin = this->m_BreakEnd = this->nEnd;
    return;
  }
  else
  {
    bFilled = false;
    this->bSecondaryBreak = false;
    this->m_BreakBegin = this->m_BreakEnd = this->nEnd;
    for(nI=0;nI<nUsed-1;nI++)
    {
      if((*this)(nI,0)>(*this)(nI+1,0))
      {
        if(!bFilled)
        {
          bFilled = true;
          this->m_BreakBegin = nI;
          this->m_BreakEnd   = nI + 1;
        }
        else
        {
          //wxASSERT(!this->bSecondaryBreak);
          this->bSecondaryBreak = true;
        }
      }
    }
  }
  return;
} // end of mafMemoryGraphBase<T, I, G>::UpdateBreakPosition;

/**
 * detailed description
 *
 * @memo    Get number of free entries
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
int  mafMemoryGraphBase<T, I, G>::GetFreeMemSpace(void)
{
  int nFree;
  
  wxASSERT((this->nBegin == 0 && this->nEnd == 0) || (this->nBegin != this->nEnd));

  if(this->m_Empty)
  {
    return this->nMaxSize;
  }
  if(this->nBegin <= this->nEnd)
  {
    //native order
    nFree = this->nMaxSize - (this->nEnd - this->nBegin) - 1;
  }
  else
  {
    //warped order
    nFree = this->nBegin - this->nEnd - 1;
  }
  
  return nFree;
} // end of mafMemoryGraphBase<T, I, G>::GetFreeMemSpace;

template <class T, class I, class G>
typename mafMemoryGraphBase<T, I, G>::mafMemGrTotalRange const *mafMemoryGraphBase<T, I, G>::GetRange(void)
{
  return &gtrTotalRange;
}

/**
 * detailed description
 *
 * @memo    destructor
 * @return  None
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
mafMemoryGraphBase<T, I, G>::~mafMemoryGraphBase(void)
{
  return;
} // end of mafMemoryGraphBase<T, I, G>::~mafMemoryGraphBase

/**
 * detailed description
 *
 * @memo    
 * @return  None
 * @param   nSize
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void mafMemoryGraphBase<T, I, G>::Init(unsigned int nDimension, unsigned int nSize)
{
  unsigned int nI;

  wxASSERT(nDimension>=1);
  this->nDim = nDimension;

  this->nBegin               = 0;
  this->m_BreakBegin         = 0;
  this->nEnd                 = 0;
  this->m_BreakEnd           = 0;
  this->nMaxSize             = nSize;
  this->m_Empty              = true;
  this->bSecondaryBreak      = false;

  m_Points.resize(nSize);
  for(nI = 0; nI < nSize; nI++)
  {
    m_Points[nI].resize(nDimension);
  }
  vgrRanges.resize(nDimension);
  m_Coords.resize(nDimension);
  m_Filled.resize(nDimension);
  vnIDs.resize(nDimension);
  vnXIndexes.resize(nDimension);
  vnYIndexes.resize(nDimension);

  nXDim= 0;
  nYDim= 0;

  nGraphID.zero();
  this->m_FilledPoints = 0;
  for(nI=0;nI<this->nDim;nI++)
  {
    m_Filled[nI] = false;
  }
  //similar points skip system
  this->rUnnoticedRange = 0.0f;

  return;
} // end of mafMemoryGraphBase<T, I, G>::Init


template <class T, class I, class G>
void  mafMemoryGraphBase<T, I, G>::SetDim(unsigned int nDimension)
{
  unsigned int nI;
  unsigned int nSize;

  wxASSERT(nDimension>=1);
  this->nDim = nDimension;

  this->nBegin               = 0;
  this->m_BreakBegin         = 0;
  this->nEnd                 = 0;
  this->m_BreakEnd           = 0;
  this->m_Empty              = true;
  this->bSecondaryBreak      = false;

  nSize = m_Points.size();

  for(nI = 0; nI < nSize; nI++)
  {
    m_Points[nI].resize(nDimension);
  }
  vgrRanges.resize(nDimension);
  m_Coords.resize(nDimension);
  m_Filled.resize(nDimension);
  vnIDs.resize(nDimension);
  vnXIndexes.resize(nDimension);
  vnYIndexes.resize(nDimension);

  this->m_FilledPoints = 0;
  for(nI=0;nI<this->nDim;nI++)
  {
    m_Filled[nI] = false;
  }
  //similar points skip system
  this->rUnnoticedRange = 0.0f;

  rRangeGrowStep         = 5.0f;
  rPointNoticeCriterion  = 0.0025f;
  rSpaceInRange          = 0.000001f;

  return;
}

/**
 * detailed description
 *
 * @memo    
 * @return  None
 * @param   nSize
 * @author  Earnol
 * @see     Nothing
 */
template <class T, class I, class G>
void mafMemoryGraphBase<T, I, G>::Clean()
{
  unsigned int nI;

  this->nBegin              = 0;
  this->m_BreakBegin         = 0;
  this->nEnd                = 0;
  this->m_BreakEnd           = 0;
  this->m_Empty              = true;
  this->bSecondaryBreak     = false;

  this->m_FilledPoints = 0;
  for(nI=0;nI<this->nDim;nI++)
  {
    m_Filled[nI] = false;
  }
  //similar points skip system
  this->rUnnoticedRange = 0.0f;

  return;
} // end of mafMemoryGraphBase<T, I, G>::Clean




template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::AddYVar(const I& nID)
{
  unsigned int nIndex;
  unsigned int nI;

  for(nI = 0; nI < this->nYDim; nI++)//if this variable is already in Y list we have nothing to do, report about error
  {
    if(nID == this->GetYID(nI))
    {
      return false;
    }
  }

  this->Clean();//clean all stored data

  if(nID == this->GetID(0))//variable number 0 is a parameter for parametric graphs and is always present, special handling
  {
    nIndex = 0;
  }
  else
  {
    nIndex = this->nDim;
    for(nI = 0; nI < this->nXDim; nI++)//check if variable is already in list
    {
      if(nID == this->GetXID(nI))
      {
        nIndex = this->GetXIndex(nI);
        break;
      }
    }
  }

  if(nIndex == this->nDim)//variable not found
  {
    this->SetDim(this->nDim + 1);//increase only total dimension, Y dimension will be increased later 
    this->SetID(nIndex, nID);
  }

  this->SetYDim(this->nYDim + 1);
  this->SetYIndex(this->nYDim - 1, nIndex);
  return true;
}


template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::RemYVar(const I& nID)
{
  unsigned int nIndex, nCoord;
  unsigned int nI, nJ;

  if(this->nYDim == 0)
  {
    return false;
  }

  this->Clean();//clean all stored data

  nIndex = this->nDim;
  for(nI = 0; nI < this->nYDim; nI++)
  {
    if(nID == this->GetYID(nI))
    {
      nIndex = this->GetYIndex(nI);
      nCoord = nI;
      break;
    }
  }
  wxASSERT(nIndex < this->nDim);
  if(nIndex == 0)
  {
    //<clean only Y indexes>
    for(nJ = nCoord; nJ < this->nYDim - 1; nJ++)
    {
      this->SetYIndex(nJ, this->GetYIndex(nJ + 1));
    }
    this->SetYDim(this->nYDim - 1);
    return true;
  }
  for(nI = 0; nI < this->nXDim; nI++)
  {
    if(nIndex == this->GetXIndex(nI))
    {
      //<clean only Y indexes>
      for(nJ = nCoord; nJ < this->nYDim - 1; nJ++)
      {
        this->SetYIndex(nJ, this->GetYIndex(nJ + 1));
      }
      this->SetYDim(this->nYDim - 1);
      return true;
    }
  }
  //<clean both IDs and Y indexes>
  for(nI = nCoord; nI < this->nYDim - 1; nI++)//shift Y indexes
  {
    this->SetYIndex(nI, this->GetYIndex(nI + 1));
  }

  for(nI = nIndex; nI < this->nDim - 1; nI++)//shift IDs
  {
    this->SetID(nI, this->GetID(nI + 1));
  }

  for(nI = 0; nI < this->nXDim; nI++)//correct X indexes
  {
    if(nIndex < this->GetXIndex(nI))
    {
      this->SetXIndex(nI, this->GetXIndex(nI) - 1);
    }
  }

  for(nI = 0; nI < this->nYDim; nI++)//correct Y indexes
  {
    if(nIndex < this->GetYIndex(nI))
    {
      this->SetYIndex(nI, this->GetYIndex(nI) - 1);
    }
  }
  this->SetDim(this->nDim - 1);
  this->SetYDim(this->nYDim - 1);
  return true;
}

template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::SetYVar(unsigned int nIndex, const I& nID)
{
  unsigned int nNewIndex, nOldIndex;
  unsigned int nI;
  bool   bRemove,bAdd;
  if(nID == this->GetYID(nIndex))//if we don't want to change it return
  {
    return true;
  }
  for(nI = 0; nI < this->nYDim; nI++)//if we try to set this ID for second time return with error
  {
    if(nID == this->GetYID(nI))
    {
      return false;
    }
  }

  this->Clean();

  nOldIndex = this->GetYIndex(nIndex);//Index of old coordinate
  if(nOldIndex == 0)
  {
    bRemove = false;
  }
  else
  {
    bRemove   = true;
    for(nI = 0; nI < this->nXDim; nI++)
    {
      if(nOldIndex == this->GetXIndex(nI))
      {
        bRemove = false;
        break;
      }
    }
  }
  nNewIndex = this->nDim;
  for(nI = 0; nI< this->nDim; nI++)//find index for new coordinate
  {
    if(nID == this->GetID(nI))
    {
      nNewIndex = nI;
      break;
    }
  }
  bAdd = (nNewIndex == this->nDim);
  if(bAdd && bRemove)//add and remove variable, so change only ID
  {
    this->SetID(nOldIndex, nID);
  }
  else if(!bAdd && !bRemove)//neither add nor remove, so change only index
  {
    this->SetYIndex(nIndex,nNewIndex);
  }
  else if(bAdd && !bRemove)//add but not remove
  {
    this->SetDim(this->nDim + 1);
    this->SetID(nNewIndex, nID);
    this->SetYIndex(nIndex, nNewIndex);
  }
  else//remove but not add
  {
    this->SetYIndex(nIndex, nNewIndex);
    for(nI = nOldIndex; nI < this->nDim - 1; nI++)//shift IDs
    {
      this->SetID(nI, this->GetID(nI + 1));
    }

    for(nI = 0; nI < this->nXDim; nI++)//correct X indexes
    {
      if(nOldIndex < this->GetXIndex(nI))
      {
        this->SetXIndex(nI, this->GetXIndex(nI) - 1);
      }
    }

    for(nI = 0; nI < this->nYDim; nI++)//correct Y indexes
    {
      if(nOldIndex < this->GetYIndex(nI))
      {
        this->SetYIndex(nI, this->GetYIndex(nI) - 1);
      }
    }
    this->SetDim(this->nDim - 1);
  }
  return true;
}


template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::AddXVar(const I& nID)
{
  unsigned int nIndex;
  unsigned int nI;

  for(nI = 0; nI < this->nXDim; nI++)//if this variable is already in X list we have nothing to do, report about error
  {
    if(nID == this->GetXID(nI))
    {
      return false;
    }
  }

  this->Clean();//clean all stored data

  if(nID == this->GetID(0))//variable number 0 is a parameter for parametric graphsand is always present, special handling
  {
    nIndex = 0;
  }
  else
  {
    nIndex = this->nDim;
    for(nI = 0; nI < this->nYDim; nI++)//check if variable is already in list
    {
      if(nID == this->GetYID(nI))
      {
        nIndex = this->GetYIndex(nI);
        break;
      }
    }
  }

  if(nIndex == this->nDim)//variable not found
  {
    this->SetDim(this->nDim + 1);
    this->SetID(nIndex, nID);
  }

  this->SetXDim(this->nXDim + 1);
  this->SetXIndex(this->nXDim - 1, nIndex);
  return true;
}


template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::RemXVar(const I& nID)
{
  unsigned int nIndex, nCoord;
  unsigned int nI, nJ;

  if(this->nXDim == 1)
  {
    return false;
  }

  this->Clean();//clean all stored data

  nIndex = this->nDim;
  for(nI = 0; nI < this->nXDim; nI++)
  {
    if(nID == this->GetXID(nI))
    {
      nIndex = this->GetXIndex(nI);
      nCoord = nI;
      break;
    }
  }
  wxASSERT(nIndex < this->nDim);
  if(nIndex == 0)
  {
    //<clean only X indexes>
    for(nJ = nCoord; nJ < this->nXDim - 1; nJ++)
    {
      this->SetXIndex(nJ, this->GetXIndex(nJ + 1));
    }
    this->SetXDim(this->nXDim - 1);
    return true;
  }
  for(nI = 0; nI < this->nYDim; nI++)
  {
    if(nIndex == this->GetYIndex(nI))
    {
      //<clean only X indexes>
      for(nJ = nCoord; nJ < this->nXDim - 1; nJ++)
      {
        this->SetXIndex(nJ, this->GetXIndex(nJ + 1));
      }
      this->SetXDim(this->nXDim - 1);
      return true;
    }
  }
  //<clean both IDs and X indexes>
  for(nI = nCoord; nI < this->nXDim - 1; nI++)//shift X indexes
  {
    this->SetXIndex(nI, this->GetXIndex(nI + 1));
  }

  for(nI = nIndex; nI < this->nDim - 1; nI++)//shift IDs
  {
    this->SetID(nI, this->GetID(nI + 1));
  }

  for(nI = 0; nI < this->nYDim; nI++)//correct Y indexes
  {
    if(nIndex < this->GetYIndex(nI))
    {
      this->SetYIndex(nI, this->GetYIndex(nI) - 1);
    }
  }

  for(nI = 0; nI < this->nXDim; nI++)//correct X indexes
  {
    if(nIndex < this->GetXIndex(nI))
    {
      this->SetXIndex(nI, this->GetXIndex(nI) - 1);
    }
  }
  this->SetDim(this->nDim - 1);
  this->SetXDim(this->nXDim - 1);
  return true;
}

template <class T, class I, class G>
bool mafMemoryGraphBase<T, I, G>::SetXVar(unsigned int nIndex, const I& nID)
{
  unsigned int nNewIndex, nOldIndex;
  unsigned int nI;
  bool   bRemove,bAdd;

  if(nID == this->GetXID(nIndex))//if we don't want to change it return
  {
    return true;
  }
  for(nI = 0; nI < this->nXDim; nI++)//if we try to set this ID for second time return with error
  {
    if(nID == this->GetXID(nI))
    {
      return false;
    }
  }

  this->Clean();

  nOldIndex = this->GetXIndex(nIndex);//Index of old coordinate
  if(nOldIndex == 0)
  {
    bRemove = false;
  }
  else
  {
    bRemove   = true;
    for(nI = 0; nI < this->nYDim; nI++)
    {
      if(nOldIndex == this->GetYIndex(nI))
      {
        bRemove = false;
        break;
      }
    }
  }
  nNewIndex = this->nDim;
  for(nI = 0; nI< this->nDim; nI++)//find index for new coordinate
  {
    if(nID == this->GetID(nI))
    {
      nNewIndex = nI;
      break;
    }
  }
  bAdd = (nNewIndex == this->nDim);
  if(bAdd && bRemove)//add and remove variable, so change only ID
  {
    this->SetID(nOldIndex, nID);
  }
  else if(!bAdd && !bRemove)//neither add nor remove, so change only index
  {
    this->SetXIndex(nIndex,nNewIndex);
  }
  else if(bAdd && !bRemove)//add but not remove
  {
    this->SetDim(this->nDim + 1);//increase only total dimension, Y dimension will be increased later 
    this->SetID(nNewIndex, nID);
    this->SetXIndex(nIndex, nNewIndex);
  }
  else//remove but not add
  {
    this->SetXIndex(nIndex, nNewIndex);
    for(nI = nOldIndex; nI < this->nDim - 1; nI++)//shift IDs
    {
      this->SetID(nI, this->GetID(nI + 1));
    }

    for(nI = 0; nI < this->nYDim; nI++)//correct Y indexes
    {
      if(nOldIndex < this->GetYIndex(nI))
      {
        this->SetYIndex(nI, this->GetYIndex(nI) - 1);
      }
    }

    for(nI = 0; nI < this->nXDim; nI++)//correct X indexes
    {
      if(nOldIndex < this->GetXIndex(nI))
      {
        this->SetXIndex(nI, this->GetXIndex(nI) - 1);
      }
    }
    this->SetDim(this->nDim - 1);
  }
  return true;
}


template <class T, class I, class G>
void mafMemoryGraphBase<T, I, G>::RecalculateRanges()
{
  int nI,nJ;
  T rMin,rMax,rValue;
  int nSize;

  nSize = this->GetUsedMemSpace();
  if(nSize <= 0)
  {
    return;
  }
  for(nI = 0; nI < this->nDim; nI++)
  {
    rMin = (*this)(0, nI);
    rMax = (*this)(0, nI);
    for(nJ = 1; nJ < nSize; nJ++)
    {
      rValue = (*this)(nJ, nI);
      if(rValue > rMax)
      {
        rMax = rValue;
      }
      if(rValue < rMin)
      {
        rMin = rValue;
      }
    }
    this->vgrRanges[nI].rMin = rMin;
    this->vgrRanges[nI].rMax = rMax;
  }
  if(this->nXDim>0)
  {
    this->gtrTotalRange.rXMin = vgrRanges[vnXIndexes[0]].rMin;
    this->gtrTotalRange.rXMax = vgrRanges[vnXIndexes[0]].rMax;
    for(nI = 1; nI < this->nXDim; nI++)
    {
      if(vgrRanges[vnXIndexes[nI]].rMin < this->gtrTotalRange.rXMin)
      {
        this->gtrTotalRange.rXMin = vgrRanges[vnXIndexes[nI]].rMin;
      }
      if(vgrRanges[vnXIndexes[nI]].rMax > this->gtrTotalRange.rXMax)
      {
        this->gtrTotalRange.rXMax = vgrRanges[vnXIndexes[nI]].rMax;
      }
    }
  }
  if(this->nYDim > 0)
  {
    this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[0]].rMin;
    this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[0]].rMax;
    for(nI=1;nI<this->nYDim;nI++)
    {
      if(vgrRanges[vnYIndexes[nI]].rMin<this->gtrTotalRange.rYMin)
      {
        this->gtrTotalRange.rYMin=vgrRanges[vnYIndexes[nI]].rMin;
      }
      if(vgrRanges[vnYIndexes[nI]].rMax>this->gtrTotalRange.rYMax)
      {
        this->gtrTotalRange.rYMax=vgrRanges[vnYIndexes[nI]].rMax;
      }
    }
  }

  return;
}

template <class I, unsigned dimension = 1>
class mafGraphIDIndex:public std::vector<I>
{
public:
  mafGraphIDIndex():std::vector<I>(dimension){}
  void zero(){std::vector<I>::assign(size(), 0);}
  bool isZero()const{for(std::vector<I>::const_iterator iter = begin(); iter < end(); ++iter) if(*iter != I(0)) return false; return true;}
private:
  void resize(size_type _Newsize);
};

typedef mafGraphIDIndex<int, 2> IDType;
typedef mafGraphIDIndex<int, 1> GraphIDType;

/**
Memory Graph instantiation with float*/
class mafMemoryGraph: public mafMemoryGraphBase<double, IDType, GraphIDType>
{
public:
  mafMemoryGraph(double garbage, unsigned int nDimension = 2,unsigned int nSize = 1000): m_garbage(garbage),  mafMemoryGraphBase<double, IDType, GraphIDType>(nDimension, nSize){}
  double   GetGarbageValue(void) { return m_garbage;}
private:
  double m_garbage;
};


#ifdef _MSC_FULL_VER
#pragma warning (default: 4786)
#endif

/**   */
#endif




