/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SHADER_DEFINES_H
#define SHADER_DEFINES_H

#define MAX_LIGHTS_POWER 12
#define MAX_LIGHTS (1<<MAX_LIGHTS_POWER)

// reduce maximum light list size per tile for better occupancy (more realistic performance)
#define MAX_SMEM_LIGHTS 512

// This determines the tile size for light binning and associated tradeoffs
#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

#define LIGHT_GRID_TEXTURE_WIDTH 1024
#define LIGHT_GRID_TEXTURE_HEIGHT 1024

#endif
