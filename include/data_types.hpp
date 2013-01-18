#ifndef __DATA_TYPES__
#define __DATA_TYPES__

#include <stdint.h>
#include <GL/glew.h>
#include <drn/drn_types.h>
#include <glm/glm.hpp>

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
	uint32_t nbVertices;
	glm::dvec3 average;
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
