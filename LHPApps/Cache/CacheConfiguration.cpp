#include "CacheConfiguration.h"

namespace CacheSystem
{
	CacheConfiguration::CacheConfiguration(const CacheConfiguration & conf)
	{
		for (unsigned int i = 0; i < conf.paramsInfo.size(); i++)
			paramsInfo.push_back(conf.paramsInfo[i]->getCopy());
		returnInfo = conf.returnInfo == nullptr ? nullptr : conf.returnInfo->getCopy();
		minimumDataCreationTime = conf.minimumDataCreationTime;
		dependencyObject = conf.dependencyObject;
		maximumDataSize = conf.maximumDataSize;
	}
}