// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

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
