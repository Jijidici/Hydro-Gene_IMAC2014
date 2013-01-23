#include "display/memory_cache.hpp"

#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <GL/glew.h>
#include "drn/drn_reader.h"
#include "data_types.hpp"
#include "geom_types.hpp"

size_t initMemory(std::vector<Chunk>& memory, Leaf* leafArray, bool* loadedLeaf, uint32_t nbLeaves, uint16_t nbSub_lvl2, size_t chunkBytesSize, glm::mat4 V, double halfLeafSize){
	//if the memcache is to tiny for the chunks
	if(chunkBytesSize >= MAX_MEMORY_SIZE){
		throw std::logic_error("the memcache is to little compare with the chunk size");
	}
	
	drn_t cache;
	uint32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache <0){ throw std::runtime_error("unable to open data file"); }
	
	/* SEND TRIANGLES TO THE GPU */
	GLuint* vaos = new GLuint[nbLeaves];
	GLuint* vbos = new GLuint[nbLeaves];
	
	for(uint32_t idx=0;idx<nbLeaves;++idx){
		glGenBuffers(1, &(vbos[idx]));
		glGenVertexArrays(1, &(vaos[idx]));
		
		/* load the triangles and send them to the GPU */
		Vertex* vertices = NULL;
		vertices = new Vertex[leafArray[idx].nbVertices_lvl2];
		test_cache = drn_read_chunk(&cache,  2*leafArray[idx].id+1 + CONFIGCHUNK_OFFSET, vertices);

		glBindBuffer(GL_ARRAY_BUFFER, vbos[idx]);
			glBufferData(GL_ARRAY_BUFFER, leafArray[idx].nbVertices_lvl2*sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		delete[] vertices;
		
		glBindVertexArray(vaos[idx]);
			glEnableVertexAttribArray(POSITION_LOCATION);
			glEnableVertexAttribArray(NORMAL_LOCATION);
			glEnableVertexAttribArray(BENDING_LOCATION);
			glEnableVertexAttribArray(DRAIN_LOCATION);
			glEnableVertexAttribArray(GRADIENT_LOCATION);
			glEnableVertexAttribArray(SURFACE_LOCATION);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[idx]);
				glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(0));
				glVertexAttribPointer(NORMAL_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
				glVertexAttribPointer(BENDING_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)));
				glVertexAttribPointer(DRAIN_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+sizeof(GLfloat)));
				glVertexAttribPointer(GRADIENT_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+2*sizeof(GLfloat)));
				glVertexAttribPointer(SURFACE_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+3*sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		std::cout<<"//-> Triangle from leaf "<<idx<<" is loaded"<<std::endl;
	}
	
	/* Load the leaf */
	size_t currentMemSize = 0;
	uint32_t currentLeaf = 0;
	while(currentMemSize + chunkBytesSize < MAX_MEMORY_SIZE && currentLeaf < nbLeaves){		
		loadInMemory(memory, leafArray[currentLeaf], currentLeaf, computeDistanceLeafCamera(leafArray[currentLeaf], V, halfLeafSize), nbSub_lvl2, vaos[leafArray[currentLeaf].id], vbos[leafArray[currentLeaf].id]);
		loadedLeaf[currentLeaf] = true;
		currentLeaf++;
		currentMemSize+= chunkBytesSize; 
	}
	
	//free ressources
	delete[] vaos;
	delete[] vbos;
	
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	return currentMemSize;
}

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t l_idx, double distance, uint16_t nbSub_lvl2, GLuint idxVao, GLuint idxVbo){
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
	newChunk.idxLeaf = l_idx;
	newChunk.d = distance;
	newChunk.vao = idxVao;
	newChunk.vbo = idxVbo;
	memory.push_back(newChunk);
	
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	std::cout<<"//-> Leaf chunks "<<l.id*2+2<<" & "<<l.id*2+3<<" loaded."<<std::endl;
}

Chunk freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf){
	Chunk lastElt = memory.back();
	loadedLeaf[lastElt.idxLeaf] = false;
	delete[] lastElt.voxels;
	lastElt.voxels = NULL;
	memory.pop_back();
	
	return lastElt;
}

double computeDistanceLeafCamera(Leaf currentLeaf, glm::mat4& view, double halfLeafSize){
	glm::vec4 homogenPos = glm::vec4(currentLeaf.pos.x + halfLeafSize, currentLeaf.pos.y + halfLeafSize, currentLeaf.pos.z + halfLeafSize, 1.);
	return glm::length(view*homogenPos) -1;
}  
