#ifndef _PARAMETER_INFO_H
#define _PARAMETER_INFO_H
#include "ParameterType.h"

namespace CacheSystem
{
	/**
	contains basic information about a parameter
	*/
	struct ParameterInfo
	{
		/**
		type of the parameter (input/output/ignore)
		*/
		ParameterType paramType;

		/**
		creates the object a sets the parameter type
		*/
		ParameterInfo(ParameterType paramType) : paramType(paramType) {}

		/**
		defined only for making the destructor virtual
		*/
		virtual ~ParameterInfo(){}

		/**
		creates a copy
		*/
		virtual ParameterInfo* getCopy() = 0;
	};
}

#endif