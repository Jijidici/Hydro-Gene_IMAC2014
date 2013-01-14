#include "display/memory_cache.hpp"

#include <vector>
#include <stdint.h>
#include <stdexcept>
#include "drn/drn_reader.h"
#include "data_types.hpp"

size_t initMemory(std::vector<Chunk>& memory, Leaf* leafArray, bool* loadedLeaf, uint16_t nbSub_lvl2, size_t chunkBytesSize, glm::mat4 V, double halfLeafSize){
	size_t currentMemSize = 0;
	uint32_t currentLeaf = 0;
	/* TEST si pas de chunk rentré car memcache trop petite -> erreur et quitter le programme  ET  gérer le cas où tous les chunks sont chargés */
	while(currentMemSize + chunkBytesSize < MAX_MEMORY_SIZE){
		loadInMemory(memory, leafArray[currentLeaf], currentLeaf, computeDistanceLeafCamera(leafArray[currentLeaf], V, halfLeafSize), nbSub_lvl2);
		loadedLeaf[currentLeaf] = true;
		currentLeaf++;
		currentMemSize+= chunkBytesSize; 
	}
	return currentMemSize;
}

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t l_idx, double distance, uint16_t nbSub_lvl2){
	drn_t cache;
	uint32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache <0){ throw std::runtime_error("unable to open data file"); }
	
	/* load the voxel data */
	VoxelData* voxArray = NULL;
	voxArray = new VoxelData[nbSub_lvl2*nbSub_lvl2*nbSub_lvl2];
	test_cache = drn_read_chunk(&cache, l.id, voxArray);
	if(test_cache <0){ throw std::runtime_error("unable to read data file"); }
	
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	//add to map
	Chunk newChunk;
	newChunk.voxels = voxArray;
	newChunk.pos = l.pos;
	newChunk.idxLeaf = l_idx;
	newChunk.d = distance;
	memory.push_back(newChunk);
}

void freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf){
	Chunk lastElt = memory.back();
	loadedLeaf[lastElt.idxLeaf] = false;
	delete[] lastElt.voxels;
	memory.pop_back();
}

double computeDistanceLeafCamera(Leaf currentLeaf, glm::mat4& view, double halfLeafSize){
	glm::vec4 homogenPos = glm::vec4(currentLeaf.pos.x + halfLeafSize, currentLeaf.pos.y + halfLeafSize, currentLeaf.pos.z + halfLeafSize, 1.);
	return glm::length(view*homogenPos) -1;
}  
