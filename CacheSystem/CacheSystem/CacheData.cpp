#include "CacheData.h"

namespace CacheSystem
{
	CacheData::~CacheData()
	{
		for (unsigned int i = 0; i < inputParameters.size(); i++)
			delete inputParameters[i];
		for (unsigned int i = 0; i < outputParameters.size(); i++)
			delete outputParameters[i];
		delete returnValue;
	}
}