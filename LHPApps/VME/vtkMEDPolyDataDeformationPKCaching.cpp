#include "vtkMEDPolyDataDeformationPKCaching.h"

CacheSystem::CachedFunctionManager* vtkMEDPolyDataDeformationPKCaching::cacheManager = nullptr;
CacheSystem::CachedFunction<bool, vtkMEDPolyDataDeformationPKCaching*, vtkMEDPolyDataDeformationPKCaching*>* vtkMEDPolyDataDeformationPKCaching::cachedFunction = nullptr;

vtkStandardNewMacro(vtkMEDPolyDataDeformationPKCaching);

vtkMEDPolyDataDeformationPKCaching::vtkMEDPolyDataDeformationPKCaching() : vtkMEDPolyDataDeformationPK()
{
	if (cacheManager == nullptr)
	{
		cacheManager = Cache::getCacheManager();
		CacheSystem::CacheConfiguration conf;
		conf.setParamInfo(0, CacheSystem::TypedParameterInfo<vtkMEDPolyDataDeformationPKCaching*>(
			CacheSystem::ParameterType::InputParam,
			equalFunction, initInput, nullptr, destroyInput, inputHash, inputGetSize
			));
		conf.setParamInfo(1, CacheSystem::TypedParameterInfo<vtkMEDPolyDataDeformationPKCaching*>(
			CacheSystem::ParameterType::OutputParam,
			nullptr, initOutput, outputOutput, destroyOutput, nullptr, outputGetSize
			));
		conf.setReturnInfo(CacheSystem::TypedReturnInfo<bool>());  //the implicit constructor of TypedReturnInfo uses the standard functions, which are ok for type bool
		cachedFunction = cacheManager->createCachedFunction(conf, staticExectuteMultidata);
	}
}

void vtkMEDPolyDataDeformationPKCaching::log(std::string str)
{
#ifdef VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOGGING
	std::ofstream file("VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOG.txt", ios::out | ios::app);
	file << str << std::endl;
	file.close();
#endif
}

void vtkMEDPolyDataDeformationPKCaching::log(int num)
{
#ifdef VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOGGING
	std::ofstream file("VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOG.txt", ios::out | ios::app);
	file << num << std::endl;
	file.close();
#endif
}

void vtkMEDPolyDataDeformationPKCaching::log(std::string str, int num)
{
#ifdef VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOGGING
	std::ofstream file("VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOG.txt", ios::out | ios::app);
	file << str << num << std::endl;
	file.close();
#endif
}

bool vtkMEDPolyDataDeformationPKCaching::equalFunction(vtkMEDPolyDataDeformationPKCaching* const & obj1, vtkMEDPolyDataDeformationPKCaching* const & obj2, void*)
{
	log("equals begin");
	//compare number of meshes
	if (obj1->GetNumberOfMeshes() != obj2->GetNumberOfMeshes())
	{
		log("equal function false 1");
		return false;
	}
	//compare all the meshes and coarse meshes
	for (unsigned int i = 0; i < obj1->GetNumberOfMeshes(); i++)
	{
		vtkPolyData* inputPoly1 = obj1->GetInputMesh(i);
		vtkPolyData* inputPoly2 = obj2->GetInputMesh(i);
		//compare number of the meshes' points
		if (inputPoly1->GetNumberOfPoints() != inputPoly2->GetNumberOfPoints())
		{
			log("equal function false 2");
			return false;
		}
		//compare compare all the points of the meshes
		for (int k = 0; k < inputPoly1->GetNumberOfPoints(); k++)
		{
			if (abs(inputPoly1->GetPoint(k)[0] - inputPoly2->GetPoint(k)[0]) > 0.01)
			{
				log("equal function false 3");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[1] - inputPoly2->GetPoint(k)[1]) > 0.01)
			{
				log("equal function false 4");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[2] - inputPoly2->GetPoint(k)[2]) > 0.01)
			{
				log("equal function false 5");
				return false;
			}
		}
		inputPoly1 = obj1->GetCoarseMesh(i);
		inputPoly2 = obj2->GetCoarseMesh(i);
		//compare number of the coarse meshes' points
		if (inputPoly1->GetNumberOfPoints() != inputPoly2->GetNumberOfPoints())
		{
			log("equal function false 6");
			return false;
		}
		//compare all the points of the coarse meshes
		for (int k = 0; k < inputPoly1->GetNumberOfPoints(); k++)
		{
			if (abs(inputPoly1->GetPoint(k)[0] - inputPoly2->GetPoint(k)[0]) > 0.01)
			{
				log("equal function false 7");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[1] - inputPoly2->GetPoint(k)[1]) > 0.01)
			{
				log("equal function false 8");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[2] - inputPoly2->GetPoint(k)[2]) > 0.01)
			{
				log("equal function false 9");
				return false;
			}
		}
		//compare number of mesh skeletons
		if (obj1->GetNumberOfMeshSkeletons(i) != obj2->GetNumberOfMeshSkeletons(i))
		{
			log("equal function false 10");
			return false;
		}
		//compare all the mesh skeletons
		for (unsigned int j = 0; j < obj1->GetNumberOfMeshSkeletons(i); j++)
		{
			vtkPolyData* polyLines1 = obj1->multiM_Skeletons[i][j].pPolyLines[0];
			vtkPolyData* polyLines2 = obj2->multiM_Skeletons[i][j].pPolyLines[0];
			//not sure if this...
			if ((polyLines1 == nullptr && polyLines2 != nullptr) || (polyLines1 != nullptr && polyLines2 == nullptr))
			{
				log("equal function false 10.1");
				return false;
			}
			//...and this is necessary
			if (polyLines1 != nullptr)
			{
				//compare bumber of skeletons' first poly lines' points
				if (polyLines1->GetNumberOfPoints() != polyLines2->GetNumberOfPoints())
				{
					log("equal function false 11");
					return false;
				}
				//compare the skeletons' first poly lines' points
				for (int i = 0; i < polyLines1->GetNumberOfPoints(); i++)
				{
					if (abs(polyLines1->GetPoint(i)[0] - polyLines2->GetPoint(i)[0]) > 0.01)
					{
						log("equal function false 12");
						return false;
					}
					if (abs(polyLines1->GetPoint(i)[1] - polyLines2->GetPoint(i)[1]) > 0.01)
					{
						log("equal function false 13");
						return false;
					}
					if (abs(polyLines1->GetPoint(i)[2] - polyLines2->GetPoint(i)[2]) > 0.01)
					{
						log("equal function false 14");
						return false;
					}
				}
			}
			polyLines1 = obj1->multiM_Skeletons[i][j].pPolyLines[1];
			polyLines2 = obj2->multiM_Skeletons[i][j].pPolyLines[1];
			//not sure if this...
			if ((polyLines1 == nullptr && polyLines2 != nullptr) || (polyLines1 != nullptr && polyLines2 == nullptr))
			{
				log("equal function false 14.1");
				return false;
			}
			//...and this is necessary
			if (polyLines1 != nullptr)
			{
				//compare bumber of skeletons's second poly lines's points
				if (polyLines1->GetNumberOfPoints() != polyLines2->GetNumberOfPoints())
				{
					log("equal function false 15");
					return false;
				}
				//compare the skeletons's second poly lines's points
				for (int i = 0; i < polyLines1->GetNumberOfPoints(); i++)
				{
					if (abs(polyLines1->GetPoint(i)[0] - polyLines2->GetPoint(i)[0]) > 0.01)
					{
						log("equal function false 16");
						return false;
					}
					if (abs(polyLines1->GetPoint(i)[1] - polyLines2->GetPoint(i)[1]) > 0.01)
					{
						log("equal function false 17");
						return false;
					}
					if (abs(polyLines1->GetPoint(i)[2] - polyLines2->GetPoint(i)[2]) > 0.01)
					{
						log("equal function false 18");
						return false;
					}
				}
			}
			vtkIdList* list1 = obj1->multiM_Skeletons[i][j].pCCList;
			vtkIdList* list2 = obj2->multiM_Skeletons[i][j].pCCList;
			//compare number of the pCCLists' ids
			if (list1->GetNumberOfIds() != list2->GetNumberOfIds())
			{
				log("equal function false 19");
				return false;
			}
			//compare the pCCLists' ids
			for (int unsigned i = 0; i < list1->GetNumberOfIds(); i++)
			{
				if (list1->GetId(i) != list2->GetId(i))
				{
					log("equal function false 20");
					return false;
				}
			}
			//compare the RSOs
			if (abs(obj1->multiM_Skeletons[i][j].RSO[0][0] - obj2->multiM_Skeletons[i][j].RSO[0][0]) > 0.01)
			{
				log("equal function false 23");
				return false;
			}
			if (abs(obj1->multiM_Skeletons[i][j].RSO[0][1] - obj2->multiM_Skeletons[i][j].RSO[0][1]) > 0.01)
			{
				log("equal function false 24");
				return false;
			}
			if (abs(obj1->multiM_Skeletons[i][j].RSO[0][2] - obj2->multiM_Skeletons[i][j].RSO[0][2]) > 0.01)
			{
				log("equal function false 25");
				return false;
			}
			if (abs(obj1->multiM_Skeletons[i][j].RSO[1][0] - obj2->multiM_Skeletons[i][j].RSO[1][0]) > 0.01)
			{
				log("equal function false 26");
				return false;
			}
			if (abs(obj1->multiM_Skeletons[i][j].RSO[1][1] - obj2->multiM_Skeletons[i][j].RSO[1][1]) > 0.01)
			{
				log("equal function false 27");
				return false;
			}
			if (abs(obj1->multiM_Skeletons[i][j].RSO[1][2] - obj2->multiM_Skeletons[i][j].RSO[1][2]) > 0.01)
			{
				log("equal function false 28");
				return false;
			}
		}
	}
	//compare number of obstacles
	if (obj1->GetNumberOfObstacles() != obj2->GetNumberOfObstacles())
	{
		log("equal function false 29");
		return false;
	}
	//compare all the obstacles
	for (unsigned int i = 0; i < obj1->GetNumberOfObstacles(); i++)
	{
		vtkPolyData* inputPoly1 = obj1->GetObstacle(i);
		vtkPolyData* inputPoly2 = obj2->GetObstacle(i);
		//compare number of points of the obstacles
		if (inputPoly1->GetNumberOfPoints() != inputPoly2->GetNumberOfPoints())
		{
			log("equal function false 30");
			return false;
		}
		//compare the obstacles' points
		for (int k = 0; k < inputPoly1->GetNumberOfPoints(); k++)
		{
			if (abs(inputPoly1->GetPoint(k)[0] - inputPoly2->GetPoint(k)[0]) > 0.01)
			{
				log("equal function false 31");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[1] - inputPoly2->GetPoint(k)[1]) > 0.01)
			{
				log("equal function false 32");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[2] - inputPoly2->GetPoint(k)[2]) > 0.01)
			{
				log("equal function false 33");
				return false;
			}
		}
		inputPoly1 = obj1->GetCoarseObstacle(i);
		inputPoly2 = obj2->GetCoarseObstacle(i);
		//compare number of coarse obstacles
		if (inputPoly1->GetNumberOfPoints() != inputPoly2->GetNumberOfPoints())
		{
			log("equal function false 34");
			return false;
		}
		//compare the coarse obstacles' points
		for (int k = 0; k < inputPoly1->GetNumberOfPoints(); k++)
		{
			if (abs(inputPoly1->GetPoint(k)[0] - inputPoly2->GetPoint(k)[0]) > 0.01)
			{
				log("equal function false 35");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[1] - inputPoly2->GetPoint(k)[1]) > 0.01)
			{
				log("equal function false 36");
				return false;
			}
			if (abs(inputPoly1->GetPoint(k)[2] - inputPoly2->GetPoint(k)[2]) > 0.01)
			{
				log("equal function false 37");
				return false;
			}
		}
	}
	if (obj1->GetUseProgressiveHulls() != obj2->GetUseProgressiveHulls())
	{
		log("equal function false 38");
		return false;
	}
	log("equal function true");
	return true;
}

void vtkMEDPolyDataDeformationPKCaching::initInput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*)
{
	log("init input begin");
	(*destination) = vtkMEDPolyDataDeformationPKCaching::New();  //create new object
	(*destination)->SetNumberOfMeshes(source->GetNumberOfMeshes());
	//copy all the meshes
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		vtkPolyData* data = vtkPolyData::New();
		data->ShallowCopy(source->GetInputMesh(i));  //copy the mesh
		(*destination)->SetInputMesh(i, data);
		data->Delete();
		data = vtkPolyData::New();
		data->ShallowCopy(source->GetCoarseMesh(i));  //copy the coarse mesh
		(*destination)->SetCoarseMesh(i, data);
		data->Delete();
		(*destination)->SetNumberOfMeshSkeletons(i, source->GetNumberOfMeshSkeletons(i));
		//copy all the mesh skeletons
		for (unsigned int j = 0; j < source->GetNumberOfMeshSkeletons(i); j++)
		{
			vtkPolyData* polyLines1 = vtkPolyData::New();
			vtkPolyData* polyLines2 = vtkPolyData::New();
			polyLines1->ShallowCopy(source->multiM_Skeletons[i][j].pPolyLines[0]);  //copy first poly lines
			polyLines2->ShallowCopy(source->multiM_Skeletons[i][j].pPolyLines[1]);  //copy second poly lines
			vtkIdList* idList = vtkIdList::New();
			idList->DeepCopy(source->multiM_Skeletons[i][j].pCCList);  //copy d pCCList
			source->multiM_Skeletons[i][j].pCCList->Delete();
			(*destination)->SetMeshSkeleton(i, j, polyLines1, polyLines2, idList,  //create the skeleton from the copied data
				source->multiM_Skeletons[i][j].RSO[0], source->multiM_Skeletons[i][j].RSO[1]);
			polyLines1->Delete();
			polyLines2->Delete();
			idList->Delete();
		}
	}
	(*destination)->SetNumberOfObstacles(source->GetNumberOfObstacles());
	//copy all the obstacles
	for (unsigned int i = 0; i < source->GetNumberOfObstacles(); i++)
	{
		vtkPolyData* data = vtkPolyData::New();
		data->ShallowCopy(source->GetObstacle(i));  //copy the obstacle
		(*destination)->SetObstacle(i, data);
		data->Delete();
		data = vtkPolyData::New();
		data->ShallowCopy(source->GetCoarseObstacle(i));  //copy the coarse obstacle
		(*destination)->SetCoarseObstacle(i, data);
		data->Delete();
	}
	(*destination)->SetUseProgressiveHulls(source->GetUseProgressiveHulls());
	log("init input end");
}

void vtkMEDPolyDataDeformationPKCaching::destroyInput(vtkMEDPolyDataDeformationPKCaching* & data, void*)
{
	log("destroy input begin");
	data->Delete();  //delete object
	log("destroy input end");
}

uint32_t vtkMEDPolyDataDeformationPKCaching::inputHash(vtkMEDPolyDataDeformationPKCaching* const & source, void*)
{
	log("input hash begin");
	double sum = 0;
	//iterates through all meshes
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		vtkPolyData* inputPoly = source->GetInputMesh(i);
		//adds coordinates of the first, middle and last point of the input mesh to the total sum
		sum += inputPoly->GetPoint(0)[0] + inputPoly->GetPoint(0)[1] + inputPoly->GetPoint(0)[2] +
			inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[0] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[1] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[2] +
			inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[0] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[1] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[2];
		//iterates through all mesh skeletons
		for (unsigned int j = 0; j < source->GetNumberOfMeshSkeletons(i); j++)
		{
			//adds coordinates of the first, middle and last point of the skeleton's first polyLines to the total sum
			inputPoly = source->multiM_Skeletons[i][j].pPolyLines[0];
			sum += inputPoly->GetPoint(0)[0] + inputPoly->GetPoint(0)[1] + inputPoly->GetPoint(0)[2] +
				inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[0] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[1] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[2] +
				inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[0] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[1] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[2];
			inputPoly = source->multiM_Skeletons[i][j].pPolyLines[1];
			//adds coordinates of the first, middle and last point of the skeleton's second polyLines to the total sum
			sum += inputPoly->GetPoint(0)[0] + inputPoly->GetPoint(0)[1] + inputPoly->GetPoint(0)[2] +
				inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[0] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[1] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[2] +
				inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[0] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[1] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[2];
		}
	}
	//iterates through all obstacles
	for (unsigned int i = 0; i < source->GetNumberOfObstacles(); i++)
	{
		vtkPolyData* inputPoly = source->GetObstacle(i);
		//adds coordinates of the first, middle and last point of the obstacle to the total sum
		sum += inputPoly->GetPoint(0)[0] + inputPoly->GetPoint(0)[1] + inputPoly->GetPoint(0)[2] +
			inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[0] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[1] + inputPoly->GetPoint(inputPoly->GetNumberOfPoints() - 1)[2] +
			inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[0] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[1] + inputPoly->GetPoint((inputPoly->GetNumberOfPoints() - 1) / 2)[2];
	}
	sum *= 10000;  //multiplies the final sum by 10000
	log("input hash end: ", sum);
	return (uint32_t)sum;
}

uint64_t vtkMEDPolyDataDeformationPKCaching::inputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*)
{
	log("input get size begin");
	uint64_t sum = 0;
	//iterates through all meshes
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		//adds input mesh and coarse mesh size to the sum
		sum += source->GetInputMesh(i)->GetActualMemorySize();
		sum += source->GetCoarseMesh(i)->GetActualMemorySize();
		//iterates through all skeletons
		for (unsigned int j = 0; j < source->GetNumberOfMeshSkeletons(i); j++)
		{
			//adds the size of first polyLines, second polyLines and the ids to the sum
			sum += source->multiM_Skeletons[i][j].pPolyLines[0]->GetActualMemorySize();
			sum += source->multiM_Skeletons[i][j].pPolyLines[1]->GetActualMemorySize();
			sum += source->multiM_Skeletons[i][j].pCCList->GetNumberOfIds() * sizeof(vtkIdType);
		}
	}
	//iterates through all obstacles
	for (unsigned int i = 0; i < source->GetNumberOfObstacles(); i++)
		sum += source->GetObstacle(i)->GetActualMemorySize();  //add the size of the obstacle to the sum
	log("input get size end");
	//the sum is in kilobytes, so we have to multiply by 1024
	return sum * 1024;
}

void vtkMEDPolyDataDeformationPKCaching::initOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*)
{
	log("init output begin");
	(*destination) = vtkMEDPolyDataDeformationPKCaching::New();  //create new object
	(*destination)->SetNumberOfMeshes(source->GetNumberOfMeshes());
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		vtkPolyData* data = vtkPolyData::New();
		data->ShallowCopy(source->GetOutputMesh(i));
		(*destination)->SetOutputMesh(i, data);
		data->Delete();
		data = vtkPolyData::New();
		if (source->outputMeshesCoarse[i] == nullptr)
			(*destination)->SetOutputMeshCoarse(i, nullptr);
		else
		{
			data->ShallowCopy(source->outputMeshesCoarse[i]);
			(*destination)->SetOutputMeshCoarse(i, data);
		}
		data->Delete();
	}
	log("init output end");
}

void vtkMEDPolyDataDeformationPKCaching::outputOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching* & destination, void*)
{
	log("output output begin");
	destination->SetNumberOfMeshes(source->GetNumberOfMeshes());
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		vtkPolyData* data = vtkPolyData::New();
		data->DeepCopy(source->GetOutputMesh(i));
		if (destination->GetOutputMesh(i) != nullptr)
			destination->GetOutputMesh(i)->Delete();
		destination->SetOutputMesh(i, data);
		data = vtkPolyData::New();
		if (source->outputMeshesCoarse[i] == nullptr)
		{
			if (destination->outputMeshesCoarse[i] != nullptr)
				destination->outputMeshesCoarse[i]->Delete();
			destination->SetOutputMeshCoarse(i, nullptr);
		}
		else
		{
			data->DeepCopy(source->outputMeshesCoarse[i]);
			if (destination->outputMeshesCoarse[i] != nullptr)
				destination->outputMeshesCoarse[i]->Delete();
			destination->SetOutputMeshCoarse(i, data);
		}
		data->Delete();
	}
	log("output output end");
}

void vtkMEDPolyDataDeformationPKCaching::destroyOutput(vtkMEDPolyDataDeformationPKCaching* & data, void*)
{
	log("destroy output begin");
	data->Delete();
	log("destroy output begin");
}

uint64_t vtkMEDPolyDataDeformationPKCaching::outputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*)
{
	log("output get size begin");
	int sum = 0;
	for (unsigned int i = 0; i < source->GetNumberOfMeshes(); i++)
	{
		sum += source->GetOutputMesh(i)->GetActualMemorySize();
		if (source->outputMeshesCoarse[i] != nullptr)
			sum += source->outputMeshesCoarse[i]->GetActualMemorySize();
	}
	log("output get size end");
	return sum * 1024;
}

bool vtkMEDPolyDataDeformationPKCaching::staticExectuteMultidata(vtkMEDPolyDataDeformationPKCaching* inputFilter, vtkMEDPolyDataDeformationPKCaching* outputFilter)
{
	log("EXECUTING");
	bool ret = inputFilter->vtkMEDPolyDataDeformationPK::ExecuteMultiData();
	log("EXECUTING DONE");
	return ret;
}

bool vtkMEDPolyDataDeformationPKCaching::ExecuteMultiData()
{
	bool ret = cachedFunction->call(this, this);
	log("Space taken: ", cacheManager->getSpaceTaken());
	return ret;
}