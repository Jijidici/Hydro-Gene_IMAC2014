#include "display/memory_cache.hpp"

#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "drn/drn_reader.h"
#include "data_types.hpp"
#include "geom_types.hpp"

size_t loadInMemory(std::vector<Chunk>& memory, bool* loadedLeaf, Leaf l, double distance, uint16_t nbSub_lvl2, size_t freeMemory){
	drn_t cache;
	
	/* check if we can load the leaf - enough size in the memory */
	uint32_t lengthVoxelArray = nbSub_lvl2*nbSub_lvl2*nbSub_lvl2;
	uint32_t leafBytesSize = lengthVoxelArray*VOXELDATA_BYTES_SIZE + l.nbVertices_lvl2*3*sizeof(double);
	//~ std::cout<<"//-> LeafBytesSize : "<<leafBytesSize<<std::endl;
	while(leafBytesSize > freeMemory && memory.size()>0){
		freeMemory += freeInMemory(memory, loadedLeaf);
	}
	
	/* load the voxel data */
	uint32_t test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_NOLOAD);
	if(test_cache <0){ throw std::runtime_error("unable to open data file"); }
	
	VoxelData* voxArray = NULL;
	voxArray = new VoxelData[lengthVoxelArray];
	test_cache = drn_read_chunk(&cache, 2*l.id + CONFIGCHUNK_OFFSET, voxArray);
	if(test_cache <0){ throw std::runtime_error("unable to read data file"); }
	
	test_cache = drn_close(&cache);
	if(test_cache <0){ throw std::runtime_error("unable to close data file"); }
	
	/* load the mesh */
	test_cache = drn_open(&cache, "./voxels_data/voxel_intersec_1.data", DRN_READ_MMAP);
	if(test_cache <0){ throw std::runtime_error("unable to open data file in MMAP mode"); }
	
	const void* pMesh = drn_get_chunk(&cache, 2*l.id + 1 + CONFIGCHUNK_OFFSET);
	if(NULL == pMesh){ throw std::runtime_error("unable to get a chunk pointer"); }
	
	GLuint meshVBO = 0;
	glGenBuffers(1, &meshVBO);
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
		glBufferData(GL_ARRAY_BUFFER, l.nbVertices_lvl2*sizeof(Vertex), pMesh, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
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
	
	//~ std::cout<<"//-> Leaf "<<l.id<<" loaded."<<std::endl;
	loadedLeaf[l.id] = true;
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
	//~ std::cout<<"//-> Leaf "<<lastElt.idxLeaf<<" freed."<<std::endl;
	memory.pop_back();
	
	return lastElt.byteSize;
}

double computeDistanceLeafCamera(Leaf& l, glm::vec3 camPosition, float terrainScale){
	float scaledLeafSize = l.size*terrainScale;
	
	/* get the relative position of the camera in comparison of leaf origin */
	glm::vec3 rel_camPosition = camPosition - glm::vec3(l.pos.x*terrainScale, l.pos.y*terrainScale, l.pos.z*terrainScale);
	
	/* Declare closest point on leaf cube */
	glm::vec3 closestPoint(0.f,0.f,0.f);
	
	/* Get the closest x coordinate*/
	if(rel_camPosition.x < 0.f){
		closestPoint.x = 0.f;
	}else if(rel_camPosition.x > scaledLeafSize){
		closestPoint.x = scaledLeafSize;	
	}else{
		closestPoint.x = rel_camPosition.x;	
	}
	
	/* Get the closest y coordinate*/
	if(rel_camPosition.y < 0.f){
		closestPoint.y = 0.f;
	}else if(rel_camPosition.y > scaledLeafSize){
		closestPoint.y = scaledLeafSize;	
	}else{
		closestPoint.y = rel_camPosition.y;	
	}
	
	/* Get the closest z coordinate*/
	if(rel_camPosition.z < 0.f){
		closestPoint.z = 0.f;
	}else if(rel_camPosition.z > scaledLeafSize){
		closestPoint.z = scaledLeafSize;	
	}else{
		closestPoint.z = rel_camPosition.z;	
	}
	
	/* case where the camera is inside the cube */
	if(closestPoint == rel_camPosition){
		return 0;
	}
	return glm::distance(closestPoint, rel_camPosition);
}  
