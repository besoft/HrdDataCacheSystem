/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKHashTable.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-08-23 14:01:00 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "PKHashTable.h"

#define RESIZE_COEF 4
#define MAX_FILL_COEF 0.5f

template<typename T, typename U>
PKHashTable<T, U>::PKHashTable(const int startSize) {
	if (startSize <= 0) {
		throw "Invalid startSize in PKHashTable<T, U>::PKHashTable(int startSize).";
	}

	this->used = new bool[startSize];
	this->table = new PKHashTableRecord<T, U>[startSize];
	this->flat = new PKHashTableRecord<T, U>[startSize];

	this->count = 0;
	this->size = startSize;
	this->writeMutex = vtkMutexLock::New();

	this->Clear();
}

template<typename T, typename U>
PKHashTable<T, U>::~PKHashTable(void) {
	this->Clear();
	this->Dispose();
}


template<typename T, typename U>
void PKHashTable<T, U>::Add(T key, U value) {
	MutexLocker locker = MutexLocker(this->writeMutex);

	this->_Add(key, value);
}

template<typename T, typename U>
void PKHashTable<T, U>::Union(const PKHashTable<T, U>* other) {
	MutexLocker locker = MutexLocker(this->writeMutex);

	PKHashTableRecord<T, U>* otherFlat = other->flat;

	int count = other->count;

	for (int i = 0; i < other->count; i++, otherFlat++) {
		this->_Add(otherFlat->key, otherFlat->value);
	}
}

template<typename T, typename U>
U PKHashTable<T, U>::Get(const T& key) const {	
	PKHashTableRecord<T, U>* res = this->_Get(key);

	if (res == NULL) {
		throw "Key not found in U PKHashTable<T, U>::Get(const T& key) const.";
	}

	return res->value;
}

template<typename T, typename U>
bool PKHashTable<T, U>::Get(const T& key, U& value) const {
	return this->Get(key, &value);
}

template<typename T, typename U>
bool PKHashTable<T, U>::Get(const T& key, U* value) const {
	PKHashTableRecord<T, U>* res = this->_Get(key);

	if (res == NULL) {
		return false;
	}

	*value = res->value;
	return true;
}

template<typename T, typename U>
bool PKHashTable<T, U>::Has(const T& key) const {
	PKHashTableRecord<T, U>* res = this->_Get(key);

	if (res == NULL) {
		return false;
	}

	return true;
}

template<typename T, typename U>
const PKHashTableRecord<T, U>* PKHashTable<T, U>::GetValuesRef() const {
	return this->flat;
}


template<typename T, typename U>
int PKHashTable<T, U>::GetSize() const {
	return this->size;
}

template<typename T, typename U>
void PKHashTable<T, U>::SetSize(const int size) {
	MutexLocker locker = MutexLocker(this->writeMutex);

	if (size < this->count) {
		throw "At least GetCount() size expected at void PKHashTable<T, U>::SetSize(const int size) const";
	}

	this->_SetSize(size);
}

template<typename T, typename U>
int PKHashTable<T, U>::GetCount() const {
	return this->count;
}

template<typename T, typename U>
void PKHashTable<T, U>::Clear() {
	MutexLocker locker = MutexLocker(this->writeMutex);
	this->_Clear();
}


#pragma region Private Zone
/* private zone */

template<typename T, typename U>
void PKHashTable<T, U>::Dispose() {

	this->_SetSize(0);
	
	if (this->writeMutex != NULL) {
		//delete this->writeMutex;
		this->writeMutex->Delete();
		this->writeMutex = NULL;
	}
}

template<typename T, typename U>
void PKHashTable<T, U>::_Add(T key, U value) {
	float ratio = (this->count + 1) / (float)this->size;

	if (ratio > MAX_FILL_COEF) {
		this->_SetSize(this->size * RESIZE_COEF);
	}

	int index = this->_GetIndex(key);

	bool update = false;
	
	while (this->used[index]) {

		if (this->table[index].key == key) {
			update = true;
			break;
		}

		index++;
		index %= this->size;
	}

	this->used[index] = true;
	
	PKHashTableRecord<T, U>* tableRecord = this->table + index;
	PKHashTableRecord<T, U>* flatRecord = this->flat + this->count;

	if (update) {
		flatRecord = tableRecord->other;
	} else {
		flatRecord->other = tableRecord;
		tableRecord->other = flatRecord;
		this->count++;
	}

	// set key & value
	tableRecord->key = key;
	tableRecord->value = value;

	flatRecord->key = key;
	flatRecord->value = value;
}

template<typename T, typename U>
PKHashTableRecord<T, U>* PKHashTable<T, U>::_Get(const T& key) const {
	int index = this->_GetIndex(key);
	
	while (this->used[index]) {

		if (this->table[index].key == key) {
			return this->table + index;
		}

		index++;
		index %= this->size;
	}

	return NULL;
}

template<typename T, typename U>
void PKHashTable<T, U>::_SetSize(const int size) {
	if (size < 0) {
		throw "Table size must be >= 0 in void PKHashTable<T, U>::_SetSize(const int size)";
	}

	/* backup old */
	PKHashTableRecord<T, U>* oldFlat = this->flat;
	int oldCount = this->count;

	/* delete all */
	if (this->table != NULL) {
		delete[] this->table;
		this->table = NULL;
	}

	if (this->used != NULL) {
		delete[] this->used;
		this->used = NULL;
	}

	this->flat = NULL;


	/* allocate new */
	this->size = size;
	this->count = 0;
	if (size > 0) {
		this->used = new bool[size];
		this->table = new PKHashTableRecord<T, U>[size];
		this->flat = new PKHashTableRecord<T, U>[size];
		this->_Clear();
	}

	/* reinsert */
	int newCount = oldCount < size ? oldCount : size;
	for (int i = 0; i < newCount; i++) {
		this->_Add(oldFlat[i].key, oldFlat[i].value);
	}

	/* delete back up */
	if (oldFlat != NULL) {
		delete[] oldFlat;
		oldFlat = NULL;
	}
}

template<typename T, typename U>
void PKHashTable<T, U>::_Clear() {
	this->count = 0;
	memset(this->used, 0, this->size * sizeof(bool)); 
}


template<typename T, typename U>
inline int PKHashTable<T, U>::_GetIndex(const T& key) const {
	unsigned int hash = PKHashTable<T, U>::GetHash(key);
	return hash % this->size;
}

/**
* Robert Jenkins' 32 bit integer hash function
* http://www.concentric.net/~ttwang/tech/inthash.htm
*/
template<typename T, typename U>
inline unsigned int PKHashTable<T, U>::GetHash(const T& key) {
	unsigned int a = (int)key;
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

/* int */
template
class PKHashTable<int, int>;

/* int */
template
struct PKHashTableRecord<int, int>;
#pragma endregion