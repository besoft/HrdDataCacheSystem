/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: forarray.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef FOR_ARRAY_HEADER
#define FOR_ARRAY_HEADER

//----------------------------------------------------------------------------
//class ForArray
//Array for adaptation of Fortran codes
//It has 1-based indexation with operator(), has default value of elements equal to 0
//and offset system for selecting of subarray 
//----------------------------------------------------------------------------
template <class Type>
class ForArray
{
public:
  //default constructor
  ForArray():m_offset(0),m_vector(){}
  //constructor by size
  explicit ForArray(size_t _Count):m_offset(0),m_vector(_Count + 4, Type(0)){}
  //constructor by size and value
  ForArray(size_t _Count, const Type &val):m_offset(0),m_vector(_Count + 4, val){}
  //copy constructor
  ForArray(const ForArray& array):m_offset(array.m_offset),m_vector(array){}
  //assign operator
  ForArray &operator=(const ForArray& array){if(this == &array) return (*this);m_offset = array.m_offset; m_vector = array.m_vector; return (*this);}
  //Fortran like indexation
  Type &operator()(size_t pos){return m_vector[1 + pos + m_offset];}
  //Fortran like indexation
  const Type &operator()(size_t pos) const {return m_vector[1 + pos + m_offset];}
  //C-like indexation
  Type &operator[](size_t pos){return operator()(pos + 1);}
  //C-like indexation
  const Type &operator[](size_t pos) const {return operator()(pos + 1);}
  //reinitialization by size and value
  void assign(size_t _Newsize, const Type& _val= Type(0)){m_vector.assign(_Newsize + 4, _val);}
  //returns size of array
  size_t size() const {return m_vector.size() - 4;}
  //resize array 
  void resize(size_t _Newsize){m_vector.resize(_Newsize + 4, Type(0)); }
  //set offset of subarray definition
  int setOffset(int offset){int tmp = m_offset;m_offset = offset;return tmp;}
  //get offset of subarray definition
  int getOffset(int offset){return m_offset;}
  //increase offset of subarray definition
  int incOffset(int offset){int tmp = m_offset;m_offset += offset - 1;return tmp;}
  //decrease offset of subarray definition
  int decOffset(int offset){int tmp = m_offset;m_offset -= offset - 1;return tmp;}
protected:

private:
  std::vector<Type> m_vector;
  int               m_offset;
};

#endif