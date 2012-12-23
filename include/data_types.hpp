#ifndef __DATA_TYPES__
#define __DATA_TYPES__

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
}Leaf;

typedef struct s_chunk{
	VoxelData* voxels;
	glm::dvec3 pos;
	uint16_t idxLeaf;
}Chunk;

#endif
