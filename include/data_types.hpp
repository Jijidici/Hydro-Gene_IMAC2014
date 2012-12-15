#ifndef __DATA_TYPES__
#define __DATA_TYPES__

typedef struct s_voxelData{ //voxels dans tabVoxel
	glm::dvec3 sumNormal;
	double sumBending;
	double sumGradient;
	double sumSurface;
	int sumDrain;
	uint32_t nbFaces;
}VoxelData;

#endif
