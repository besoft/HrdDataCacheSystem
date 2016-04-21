/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PriorityQueue.h,v $ 
  Language: C++ 
  Date: $Date: 2011-04-21 12:40:41 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Josef Kohout, David Cholt  
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#ifndef __priorityQueue_h
#define __priorityQueue_h

#include "vtkstd/vector"
#include "vtkstd/map"

//PriorityQueue is suitable for a larger number of elements with
//priorities that may alter whilst they are in the queue
template < typename T_PRIORITY, typename T_VALUE >
class CPriorityQueue
{
public:
    struct PQ_ITEM
    {
        T_PRIORITY priority;
        T_VALUE value;
    };

protected:
	vtkstd::vector<PQ_ITEM> m_Data;
	vtkstd::map<T_VALUE, int> m_MapTable;

public:
    //ctor
    CPriorityQueue()
    {
		PQ_ITEM head;
        m_Data.push_back(head);
    }

public:
    //puts a new element into the queue
    inline void Put(T_PRIORITY priority, T_VALUE value);

    //gets the element at the head
    void Get(T_PRIORITY& priority, T_VALUE& value);

    //gets the element at the head
    inline T_VALUE Get(T_PRIORITY& priority);

    //gets the element at the head
    inline T_VALUE Get();

    //gets the element at the head without removing it from the queue
    void Peek(T_PRIORITY priority, T_VALUE value);

    //gets the element at the head without removing it from the queue
    inline T_VALUE Peek(T_PRIORITY priority);

    //gets the element at the head without removing it from the queue
    inline T_VALUE Peek();

    //removes value from the queue, returns false if element not found
    bool Remove(T_VALUE value);

    //modifies the priority for the given value
    bool ModifyPriority(T_VALUE value, T_PRIORITY newpriority);

    //returns true if the queue is empty
    inline bool IsEmpty() {
		return m_Data.size() ==  1;
    }

    //returns number of elements in the queue
    inline int GetCount() {
        return m_Data.size() - 1;
    }

	void Clear() {
		m_Data.clear();
		m_MapTable.clear();
		PQ_ITEM head;
        m_Data.push_back(head);
	}


protected:
    //swaps two elements
    inline void SwapElements(int nIndex1, int nIndex2);

    //restore the heap in the direction from leaves to root
    //starting at index nIndex, ending at index 1
    void MoveUp(int nIndex);

    //restore the heap in the direction from root to leaves
    //starting at index nStartIndex, ending at index nEndIndex
    void MoveDown(int nStartIndex, int nEndIndex);

    //finds the index for val, returns -1 if not found
    inline int FindItem(T_VALUE& val);
};


//puts a new element into the queue
template < typename T_PRIORITY, typename T_VALUE >
inline void CPriorityQueue< T_PRIORITY, T_VALUE >::Put(T_PRIORITY priority, T_VALUE value)
{
    PQ_ITEM item;
    item.priority = priority;
    item.value = value;

	m_Data.push_back(item);
	int nIndex = m_Data.size() - 1;
	m_MapTable[value] = nIndex;
    MoveUp(nIndex);
}

//gets the element at the head
template < typename T_PRIORITY, typename T_VALUE >
void CPriorityQueue< T_PRIORITY, T_VALUE >::Get(T_PRIORITY &priority,
T_VALUE &value)
{
    int nIndex = m_Data.size() - 1;
	
    //BES_ASSERT(nIndex >= 1);
	if (nIndex < 0) return;

    SwapElements(1, nIndex);
    MoveDown(1, nIndex - 1);

    PQ_ITEM item = m_Data[nIndex];
    priority = item.priority;
    value = item.value;
    m_Data.erase(m_Data.end() - 1);
    m_MapTable.erase(value);
}

//gets the element at the head
template < typename T_PRIORITY, typename T_VALUE >
inline T_VALUE CPriorityQueue< T_PRIORITY, T_VALUE >::Get(T_PRIORITY& priority)
{
    T_VALUE ret;
    Get(priority, ret);
    return ret;
}

//gets the element at the head
template < typename T_PRIORITY, typename T_VALUE >
inline T_VALUE CPriorityQueue< T_PRIORITY, T_VALUE >::Get()
{
    T_PRIORITY dummy;
    return Get(dummy);
}

//gets the element at the head without removing it from the queue
template < typename T_PRIORITY, typename T_VALUE >
inline T_VALUE CPriorityQueue< T_PRIORITY, T_VALUE >::Peek()
{
    return m_Data[1].value;
}

//swaps two elements
template < typename T_PRIORITY, typename T_VALUE >
inline void CPriorityQueue< T_PRIORITY, T_VALUE >::SwapElements(int
nIndex1, int nIndex2)
{
    PQ_ITEM tmp = m_Data[nIndex1];
    m_Data[nIndex1] = m_Data[nIndex2];
    m_Data[nIndex2] = tmp;

    m_MapTable[m_Data[nIndex1].value] = nIndex1;
    m_MapTable[m_Data[nIndex2].value] = nIndex2;
}

//removes value from the queue, returns false if element not found
template < typename T_PRIORITY, typename T_VALUE >
bool CPriorityQueue< T_PRIORITY, T_VALUE >::Remove(T_VALUE value)
{
    int nIndex1 = FindItem(value);
    if (nIndex1 < 0)
        return false;

    T_PRIORITY pri1 = m_Data[nIndex1].priority;

	int nIndex2 = m_Data.size() - 1;
    T_PRIORITY pri2 = m_Data[nIndex2].priority;

    SwapElements(nIndex1, nIndex2);
    if (pri1 > pri2)
        MoveUp(nIndex1);
    else
        MoveDown(nIndex1, nIndex2-1);

    m_Data.erase(m_Data.end() - 1);
    m_MapTable.erase(value);
    return true;
}

//modifies the priority for the given value
template < typename T_PRIORITY, typename T_VALUE >
bool CPriorityQueue< T_PRIORITY, T_VALUE >::ModifyPriority(T_VALUE value, T_PRIORITY newpriority)
{
    int nIndex = FindItem(value);
    if (nIndex >= 0)
    {
        T_PRIORITY priOrig = m_Data[nIndex].priority;
        m_Data[nIndex].priority = newpriority;
        if (priOrig > newpriority)
            MoveUp(nIndex);
        else
            MoveDown(nIndex, m_Data.size() - 1);
		return true;
    }
	else return false;
}

//restore the heap in the direction from leaves to root
//starting at index nIndex, ending at index 1
template < typename T_PRIORITY, typename T_VALUE >
void CPriorityQueue< T_PRIORITY, T_VALUE >::MoveUp(int nIndex)
{
    while (nIndex > 1 && m_Data[nIndex / 2].priority > m_Data[nIndex].priority)
    {
        SwapElements(nIndex, nIndex / 2);
        nIndex /= 2;
    }
}

//restore the heap in the direction from root to leaves
//starting at index nStartIndex, ending at index nEndIndex
template < typename T_PRIORITY, typename T_VALUE >
void CPriorityQueue< T_PRIORITY, T_VALUE >::MoveDown(int nStartIndex,
int nEndIndex)
{
    int nNextIndex = 2*nStartIndex;
    while (nNextIndex <= nEndIndex)
    {
        //if both leaves exist, check their priorities
        if (nNextIndex < nEndIndex &&
            m_Data[nNextIndex].priority > m_Data[nNextIndex + 1].priority)
            nNextIndex++;

        if (m_Data[nStartIndex].priority <= m_Data[nNextIndex].priority)
            break;    //we're ready

        SwapElements(nStartIndex, nNextIndex);
        nStartIndex = nNextIndex;
        nNextIndex = 2*nStartIndex;
    }
}

//finds the index for val, returns -1 if not found
template < typename T_PRIORITY, typename T_VALUE >
inline int CPriorityQueue< T_PRIORITY, T_VALUE >::FindItem(T_VALUE& val)
{
	if (m_MapTable.find(val) != m_MapTable.end())
		return m_MapTable[val];
	else
		return -1;
}
#endif
