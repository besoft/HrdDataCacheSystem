#ifndef _RETURN_INFO_H
#define _RETURN_INFO_H
#include "ReturnType.h"

namespace CacheSystem
{
	/**
	contains basic information about the return value
	*/
	struct ReturnInfo
	{
		/**
		type of the return value (used/ignored)
		*/
		ReturnType returnType;

		/**
		creates the object and sets the return type
		*/
		ReturnInfo(ReturnType returnType) : returnType(returnType) {}

		/**
		only defined to make the destructor virtual
		*/
		virtual ~ReturnInfo(){}

		/**
		creates a copy
		*/
		virtual ReturnInfo* getCopy() = 0;
	};
}

#endif