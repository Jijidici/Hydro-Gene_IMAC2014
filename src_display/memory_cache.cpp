#include "display/memory_cache.hpp"

#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <GL/glew.h>
#include "drn/drn_reader.h"
#include "data_types.hpp"
#include "geom_types.hpp"

void loadInMemory(std::vector<Chunk>& memory, Leaf l, double distance, uint16_t nbSub_lvl2, GLuint idxVao, GLuint idxVbo){
	drn_t cache;
	uint32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache <0){ throw std::runtime_error("unable to open data file"); }
	
	/* load the voxel data */
	VoxelData* voxArray = NULL;
	voxArray = new VoxelData[nbSub_lvl2*nbSub_lvl2*nbSub_lvl2];
	test_cache = drn_read_chunk(&cache, 2*l.id + CONFIGCHUNK_OFFSET, voxArray);
	if(test_cache <0){ throw std::runtime_error("unable to read data file"); }
	
	//add to map
	Chunk newChunk;
	newChunk.voxels = voxArray;
	newChunk.pos = l.pos;
	newChunk.idxLeaf = l.id;
	newChunk.d = distance;
	newChunk.vao = idxVao;
	newChunk.vbo = idxVbo;
	memory.push_back(newChunk);
	
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	//~ std::cout<<"//-> Leaf chunks "<<l.id*2+2<<" & "<<l.id*2+3<<" loaded."<<std::endl;
}

Chunk freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf){
	Chunk lastElt = memory.back();
	loadedLeaf[lastElt.idxLeaf] = false;
	delete[] lastElt.voxels;
	lastElt.voxels = NULL;
	memory.pop_back();
	
	return lastElt;
}

double computeDistanceLeafCamera(Leaf l, glm::mat4& view){
	glm::vec4 l_vertices[8];
	l_vertices[0] = glm::vec4(l.pos.x, l.pos.y, l.pos.z, 1.f);
	l_vertices[1] = glm::vec4(l.pos.x+l.size, l.pos.y, l.pos.z, 1.f);
	l_vertices[2] = glm::vec4(l.pos.x, l.pos.y, l.pos.z+l.size, 1.f);
	l_vertices[3] = glm::vec4(l.pos.x+l.size, l.pos.y, l.pos.z+l.size, 1.f);
	l_vertices[4] = glm::vec4(l.pos.x, l.pos.y+l.size, l.pos.z, 1.f);
	l_vertices[5] = glm::vec4(l.pos.x+l.size, l.pos.y+l.size, l.pos.z, 1.f);
	l_vertices[6] = glm::vec4(l.pos.x, l.pos.y+l.size, l.pos.z+l.size, 1.f);
	l_vertices[7] = glm::vec4(l.pos.x+l.size, l.pos.y+l.size, l.pos.z+l.size, 1.f);
	
	double distance = glm::length(view*l_vertices[0]);
	for(uint16_t idx=1;idx<8;++idx){
		double tmp_distance = glm::length(view*l_vertices[idx]);
		if(tmp_distance<distance){ distance = tmp_distance; }
	}	
	return distance-1;
}  
