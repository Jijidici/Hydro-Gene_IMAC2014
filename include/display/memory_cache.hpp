#ifndef __MEMORY_CACHE_HPP__
#define __MEMORY_CACHE_HPP__

#include <vector>
#include <stdint.h>
#include "data_types.hpp"

static const size_t MAX_MEMORY_SIZE = 524288000;
static const double THRESHOLD_DISTANCE = 1.;
static const size_t CONFIGCHUNK_OFFSET = 1;

size_t initMemory(std::vector<Chunk>& memory, Leaf* leafArray, bool* loadedLeaf, uint32_t nbLeaves, uint16_t nbSub_lvl2, size_t chunkBytesSize, glm::mat4 V, double halfLeafSize);

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t l_idx, double distance, uint16_t nbSub_lvl2, GLuint idxVao, GLuint idxVbo);

Chunk freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf);

double computeDistanceLeafCamera(Leaf currentLeaf, glm::mat4& view, double halfLeafSize);

#endif
