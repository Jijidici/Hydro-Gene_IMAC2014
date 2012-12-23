#ifndef __MEMORY_CACHE_HPP__
#define __MEMORY_CACHE_HPP__

#include <map>
#include <stdint.h>
#include "data_types.hpp"

void loadInMemory(std::map<uint32_t, VoxelData*>& memory, Leaf l, uint32_t l_id,  uint16_t nbSub_lvl2);

void freeInMemory(std::map<uint32_t, VoxelData*>& memory, uint32_t l_id);

#endif
