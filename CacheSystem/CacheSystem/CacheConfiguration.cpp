#include "CacheConfiguration.h"

namespace CacheSystem
{
	CacheConfiguration::~CacheConfiguration()
	{
		for (unsigned int i = 0; i < paramsInfo.size(); i++)
			delete paramsInfo[i];
		delete returnInfo;
	}

	CacheConfiguration::CacheConfiguration(const CacheConfiguration & conf)
	{
		for (unsigned int i = 0; i < conf.paramsInfo.size(); i++)
			paramsInfo.push_back(conf.paramsInfo[i]->getCopy());
		returnInfo = conf.returnInfo == nullptr ? nullptr : conf.returnInfo->getCopy();
	}
}