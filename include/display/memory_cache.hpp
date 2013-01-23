#ifndef __MEMORY_CACHE_HPP__
#define __MEMORY_CACHE_HPP__

#include <vector>
#include <stdint.h>
#include "data_types.hpp"

#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define BENDING_LOCATION 3
#define DRAIN_LOCATION 4
#define GRADIENT_LOCATION 5
#define SURFACE_LOCATION 6

static const size_t MAX_MEMORY_SIZE = 524288000;
static const size_t CONFIGCHUNK_OFFSET = 2;

size_t initMemory(std::vector<Chunk>& memory, Leaf* leafArray, bool* loadedLeaf, uint32_t nbLeaves, uint16_t nbSub_lvl2, size_t chunkBytesSize, glm::mat4 V, double halfLeafSize);

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t l_idx, double distance, uint16_t nbSub_lvl2, GLuint idxVao, GLuint idxVbo);

Chunk freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf);

double computeDistanceLeafCamera(Leaf currentLeaf, glm::mat4& view, double halfLeafSize);

#endif
