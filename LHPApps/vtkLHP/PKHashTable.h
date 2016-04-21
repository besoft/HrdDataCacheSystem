/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKHashTable.h,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:01:00 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#ifndef PK_HASH_TABLE_H_
#define PK_HASH_TABLE_H_

#pragma once

#include <cstring>

#include "MutexLocker.h"

// My code

#define PK_HASH_TABLE_DEF_SIZE 10

using namespace std;

template<typename T, typename U>
struct PKHashTableRecord {
	T key;
	U value;
	PKHashTableRecord<T, U>* other;
};

template<typename T, typename U>
class PKHashTable
{
public:

	PKHashTable(const int startSize = PK_HASH_TABLE_DEF_SIZE);
	~PKHashTable(void);

	void Add(T key, U value);
	void Union(const PKHashTable<T, U>* other);

	U Get(const T& key) const;
	bool Get(const T& key, U& value) const;
	bool Get(const T& key, U* value) const;

	bool Has(const T& key) const;

	const PKHashTableRecord<T, U>* GetValuesRef() const;

	int GetSize() const;
	void SetSize(const int size);

	int GetCount() const;

	void Clear();

	static inline unsigned int GetHash(const T& key);

private:
	void Dispose();

	void _Add(T key, U value);
	PKHashTableRecord<T, U>* _Get(const T& key) const;
	void _SetSize(const int size);
	void _Clear();

	inline int _GetIndex(const T& key) const;



private:
	bool* used;

	PKHashTableRecord<T, U>* table;
	PKHashTableRecord<T, U>* flat;

	int size;
	int count;

	vtkMutexLock* writeMutex;

};

#endif