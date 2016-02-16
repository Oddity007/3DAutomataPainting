#ifndef Renderer_Stage_hpp
#define Renderer_Stage_hpp

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <map>
#include <memory>

#include "Renderer.hpp"
#include "SPState.hpp"

namespace GX
{

struct Stage
{
	Stage(const char* workingDirectory);
	~Stage(void);

	Renderer renderer;
	SPState painter;
	
	/*struct Message
	{
		enum class Destination
		{
			enum class Type
			{
	 
			};
			
			Type type;
		};
		
		Destination destination;
	};*/

	/*struct LoadedSector
	{
		int64_t position[3];
		std::vector<size_t> loadedBundleIndex;
	};*/
	
	struct LoadedBundle
	{
		uint64_t permanentBundleID;
		size_t referenceCount;
		
		std::vector<uint16_t>
			vertexBufferIndices,
			indexBufferIndices,
			jointBufferIndices,
			//instanceBufferIndices,
			clusterIndices,
			materialIndices,
			imageIndices,
			bundleIndices;
		
		LoadedBundle(void)
		{
			permanentBundleID = 0;
			referenceCount = 0;
		}
	};
	
	//std::vector<LoadedSector> loadedSectors;
	std::vector<LoadedBundle> loadedBundles;
	
	uint16_t acquireBundle(uint64_t permanentBundleID);
	void releaseBundle(uint16_t bundleIndex);
	
	void loadBundle(uint16_t bundleIndex);
	
	//const uint32_t sectorExtents[3];
	
	std::string workingDirectory;
};

}

#endif