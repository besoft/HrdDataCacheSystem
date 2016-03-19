#include "DefaultCachePolicy.h"
#include "CachedFunctionManager.h"

namespace CacheSystem
{
	namespace CachePolicies
	{
		void DefaultCachePolicy::dataCreationEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			PolicyData* policyData = new PolicyData;
			policyData->lastPriorityCalculation = 0;
			policyData->timeContribution = pow((dataCreationTime + 10), LOG_10_2) - 2;
			double partOfMemory = (double)dataSize / data->getCachedFunction()->getManager()->getCacheCapacity();
			policyData->sizeContribution = 6 * (1 - pow(partOfMemory, 1 / 3.0));
			data->setUserData(policyData);
			dataCount++;
		}

		void DefaultCachePolicy::cacheHitEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			PolicyData* policyData = (PolicyData*)data->getUserData();
			if (policyData->lastPriorityCalculation == 0)
				policyData->lastPriorityCalculation = timeStamp;
			policyData->referenceCount++;
			timeStamp++;
		}

		void DefaultCachePolicy::cacheMissEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			PolicyData* policyData = (PolicyData*)data->getUserData();
			policyData->referenceCount -= (1.0 / dataCount);
			policyData->lastPriorityCalculation = timeStamp;
		}

		double DefaultCachePolicy::getPriority(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			PolicyData* policyData = (PolicyData*)data->getUserData();
			double subValue = (double)(timeStamp - policyData->lastPriorityCalculation) * (1.0 / dataCount);
			policyData->lastPriorityCalculation = timeStamp;
			policyData->referenceCount -= subValue;
			return policyData->referenceCount * referenceContributionConstant + policyData->sizeContribution * sizeContributionConstant +
				policyData->timeContribution * timeContributionConstant;
		}

		void DefaultCachePolicy::dataEvictionEvent(uint64_t dataId)
		{
			CacheData* data = (CacheData*)dataId;
			delete (PolicyData*)data->getUserData();
			dataCount--;
		}

		void DefaultCachePolicy::setTimeContributionConstant(double c)
		{
			if (c > 1 || c < 0)
				throw std::invalid_argument("The constant must be from <0, 1>");
			timeContributionConstant = c;
			double rest = 1 - c;
			double sum = sizeContributionConstant + referenceContributionConstant;
			sizeContributionConstant /= sum;
			referenceContributionConstant /= sum;
			sizeContributionConstant *= rest;
			referenceContributionConstant *= rest;
		}

		void DefaultCachePolicy::setSizeContributionConstant(double c)
		{
			if (c > 1 || c < 0)
				throw std::invalid_argument("The constant must be from <0, 1>");
			sizeContributionConstant = c;
			double rest = 1 - c;
			double sum = timeContributionConstant + referenceContributionConstant;
			timeContributionConstant /= sum;
			referenceContributionConstant /= sum;
			timeContributionConstant *= rest;
			referenceContributionConstant *= rest;
		}

		void DefaultCachePolicy::setReferenceContributionConstant(double c)
		{
			if (c > 1 || c < 0)
				throw std::invalid_argument("The constant must be from <0, 1>");
			referenceContributionConstant = c;
			double rest = 1 - c;
			double sum = timeContributionConstant + sizeContributionConstant;
			timeContributionConstant /= sum;
			sizeContributionConstant /= sum;
			timeContributionConstant *= rest;
			sizeContributionConstant *= rest;
		}

		void DefaultCachePolicy::setReferenceAndTimeContributionConstant(double refC, double timeC)
		{
			if (refC < 0 || timeC < 0 || refC > 1 || timeC > 1 || refC + timeC > 1)
				throw std::invalid_argument("Both constants and their sum must be from <0, 1>");
			referenceContributionConstant = refC;
			timeContributionConstant = timeC;
			sizeContributionConstant = 1 - (timeC + refC);
		}

		void DefaultCachePolicy::setReferenceAndSizeContributionConstant(double refC, double sizeC)
		{
			if (refC < 0 || sizeC < 0 || refC > 1 || sizeC > 1 || refC + sizeC > 1)
				throw std::invalid_argument("Both constants and their sum must be from <0, 1>");
			referenceContributionConstant = refC;
			sizeContributionConstant = sizeC;
			timeContributionConstant = 1 - (sizeC + refC);
		}

		void DefaultCachePolicy::setTimeAndSizeContributionConstant(double timeC, double sizeC)
		{
			if (timeC < 0 || sizeC < 0 || timeC > 1 || sizeC > 1 || timeC + sizeC > 1)
				throw std::invalid_argument("Both constants and their sum must be from <0, 1>");
			timeContributionConstant = timeC;
			sizeContributionConstant = sizeC;
			referenceContributionConstant = 1 - (sizeC + timeC);
		}
	}
}