#include "display/memory_cache.hpp"

#include <map>
#include <stdint.h>
#include <stdexcept>
#include "drn/drn_reader.h"
#include "data_types.hpp"

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t nbSub_lvl2){
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
	memory.push_back(newChunk);
}

void freeInMemory(std::vector<Chunk>& memory, uint32_t l_id){
	/*delete[] memory[l_id];
	memory[l_id] = NULL;
	memory.erase(l_id);*/
}

double computeDistanceLeafCamera(Chunk currentChunk, glm::mat4& view, double halfLeafSize){
	glm::vec4 homogenPos = glm::vec4(currentChunk.pos.x + halfLeafSize, currentChunk.pos.y + halfLeafSize, currentChunk.pos.z + halfLeafSize, 1.);
	return glm::length(view*homogenPos);
}  
