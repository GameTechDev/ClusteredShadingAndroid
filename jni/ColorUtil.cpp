/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "ColorUtil.h"

float3 HueToRGB(float hue)
{
    float intPart;
    float fracPart = modf(hue * 6.0f, &intPart);
    int region = static_cast<int>(intPart);
    
    switch (region) {
    case 0: return float3(1.0f, fracPart, 0.0f);
    case 1: return float3(1.0f - fracPart, 1.0f, 0.0f);
    case 2: return float3(0.0f, 1.0f, fracPart);
    case 3: return float3(0.0f, 1.0f - fracPart, 1.0f);
    case 4: return float3(fracPart, 0.0f, 1.0f);
    case 5: return float3(1.0f, 0.0f, 1.0f - fracPart);
    };

    return float3(0.0f, 0.0f, 0.0f);
}
