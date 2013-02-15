#include "display/memory_cache.hpp"

#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <GL/glew.h>
#include "drn/drn_reader.h"
#include "data_types.hpp"
#include "geom_types.hpp"

size_t loadInMemory(std::vector<Chunk>& memory, bool* loadedLeaf, Leaf l, double distance, uint16_t nbSub_lvl2, size_t freeMemory){
	drn_t cache;
	uint32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache <0){ throw std::runtime_error("unable to open data file"); }
	
	/* check if we can load the leaf - enough size in the memory */
	uint32_t lengthVoxelArray = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	uint32_t leafBytesSize = lengthVoxelArray*VOXELDATA_BYTES_SIZE + l.nbVertices_lvl2*3*sizeof(double);
	std::cout<<"//-> LeafBytesSize : "<<leafBytesSize<<std::endl;
	while(leafBytesSize > freeMemory && memory.size()>0){
		freeMemory += freeInMemory(memory, loadedLeaf);
	}
	
	/* load the voxel data */
	VoxelData* voxArray = NULL;
	voxArray = new VoxelData[lengthVoxelArray];
	test_cache = drn_read_chunk(&cache, 2*l.id + CONFIGCHUNK_OFFSET, voxArray);
	if(test_cache <0){ throw std::runtime_error("unable to read data file"); }
	
	/* load the mesh */
	Vertex* vertices = new Vertex[l.nbVertices_lvl2];
	test_cache = drn_read_chunk(&cache, 2*l.id+1 + CONFIGCHUNK_OFFSET, vertices);
	if(test_cache <0){ throw std::runtime_error("unable to read data file"); }
	
	GLuint meshVBO = 0;
	glGenBuffers(1, &meshVBO);
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
		glBufferData(GL_ARRAY_BUFFER, l.nbVertices_lvl2*sizeof(Vertex), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	delete[] vertices;
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	/* setting the mesh */
	GLuint meshVAO = 0;
	glGenVertexArrays(1, &meshVAO);
	glBindVertexArray(meshVAO);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glEnableVertexAttribArray(BENDING_LOCATION);
		glEnableVertexAttribArray(DRAIN_LOCATION);
		glEnableVertexAttribArray(GRADIENT_LOCATION);
		glEnableVertexAttribArray(SURFACE_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
			glVertexAttribPointer(POSITION_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(0));
			glVertexAttribPointer(NORMAL_LOCATION, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(3*sizeof(GLdouble)));
			glVertexAttribPointer(BENDING_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)));
			glVertexAttribPointer(DRAIN_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+sizeof(GLfloat)));
			glVertexAttribPointer(GRADIENT_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+2*sizeof(GLfloat)));
			glVertexAttribPointer(SURFACE_LOCATION, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(6*sizeof(GLdouble)+3*sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	//add the new chunk to the vector
	Chunk newChunk;
	newChunk.voxels = voxArray;
	newChunk.pos = l.pos;
	newChunk.idxLeaf = l.id;
	newChunk.d = distance;
	newChunk.vao = meshVAO;
	newChunk.vbo = meshVBO;
	newChunk.byteSize = leafBytesSize;
	memory.push_back(newChunk);
	
	std::cout<<"//-> Leaf "<<l.id<<" loaded."<<std::endl;
	
	freeMemory -= leafBytesSize;
	return freeMemory;
}

size_t freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf){
	Chunk lastElt = memory.back();
	/* delete voxelData */
	delete[] lastElt.voxels;
	lastElt.voxels = NULL;
	/* delete the VAO & VBO */
	glDeleteBuffers(1, &(lastElt.vbo));
	glDeleteVertexArrays(1, &(lastElt.vao));
	loadedLeaf[lastElt.idxLeaf] = false;
	std::cout<<"//-> Leaf "<<lastElt.idxLeaf<<" freed."<<std::endl;
	memory.pop_back();
	
	return lastElt.byteSize;
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
