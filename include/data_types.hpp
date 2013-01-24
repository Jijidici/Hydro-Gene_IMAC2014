#ifndef __DATA_TYPES__
#define __DATA_TYPES__

#include <stdint.h>
#include <GL/glew.h>
#include <drn/drn_types.h>
#include <glm/glm.hpp>

#include "geom_types.hpp"

static const size_t VOXELDATA_BYTES_SIZE = 56;

typedef struct s_voxelData{ //voxels dans tabVoxel
	glm::dvec3 sumNormal;
	double sumBending;
	double sumGradient;
	double sumSurface;
	int sumDrain;
	uint32_t nbFaces;
}VoxelData;

typedef struct s_leaf{
	drn_chunk_id_t id;
	glm::dvec3 pos;
	double size;
	uint32_t nbIntersection;
	uint32_t nbVertices_lvl1;
	uint32_t nbVertices_lvl2;
	Vertex optimal;

	s_leaf() : id(0), pos(glm::dvec3(-1.f, -1.f, -1.f)), size(-1), nbIntersection(0), nbVertices_lvl1(0), nbVertices_lvl2(0){ }
	
	bool operator()(const s_leaf l1, const s_leaf l2){
		return l1.id < l2.id;
	}
}Leaf;

typedef struct s_chunk{
	VoxelData* voxels;
	glm::dvec3 pos;
	uint16_t idxLeaf;
	double d;
	GLuint vao;
	GLuint vbo;
	
	bool operator()(const s_chunk chk1, const s_chunk chk2){
		return chk1.d < chk2.d;
	}
}Chunk;

#endif
