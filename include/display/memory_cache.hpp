#ifndef __MEMORY_CACHE_HPP__
#define __MEMORY_CACHE_HPP__

#include <vector>
#include <stdint.h>
#include "data_types.hpp"

static const size_t MAX_MEMORY_SIZE = 2097152;
static const double THRESHOLD_DISTANCE = 0.2;

size_t initMemory(std::vector<Chunk>& memory, Leaf* leafArray, bool* loadedLeaf, uint16_t nbSub_lvl2, size_t chunkBytesSize);

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t l_idx, uint16_t nbSub_lvl2);

void freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf);

double computeDistanceLeafCamera(Leaf currentLeaf, glm::mat4& view, double halfLeafSize);

#endif
