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

static const size_t MAX_MEMORY_SIZE = 20000;
static const size_t CONFIGCHUNK_OFFSET = 2;

size_t loadInMemory(std::vector<Chunk>& memory, bool* loadedLeaf, Leaf l, double distance, uint16_t nbSub_lvl2, size_t freeMemory);

size_t freeInMemory(std::vector<Chunk>& memory, bool* loadedLeaf);

double computeDistanceLeafCamera(Leaf l, glm::mat4& view);

#endif
