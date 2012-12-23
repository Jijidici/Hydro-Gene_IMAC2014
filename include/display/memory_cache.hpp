#ifndef __MEMORY_CACHE_HPP__
#define __MEMORY_CACHE_HPP__

#include <vector>
#include <stdint.h>
#include "data_types.hpp"

void loadInMemory(std::vector<Chunk>& memory, Leaf l, uint16_t nbSub_lvl2);

void freeInMemory(std::vector<Chunk>& memory);

double computeDistanceLeafCamera(Chunk currentChunk, glm::mat4& view, double halfLeafSize);

#endif
