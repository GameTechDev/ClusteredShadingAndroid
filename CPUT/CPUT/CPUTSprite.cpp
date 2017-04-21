/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUT.h"
#include "CPUTBuffer.h"
#ifdef CPUT_FOR_DX11
#include "CPUTSpriteDX11.h"
#else    
	#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
	#include "CPUTSpriteOGL.h"
	#else
		#error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
	#endif
#endif

CPUTSprite* CPUTSprite::CreateSprite(float   spriteX, float          spriteY,float          spriteWidth, float          spriteHeight, const cString &materialName )
{
	// TODO: accept DX11/OGL param to control which platform we generate.
	// TODO: be sure to support the case where we want to support only one of them
#ifdef CPUT_FOR_DX11
	return CPUTSpriteDX11::CreateSprite( spriteX,spriteY,spriteWidth,spriteHeight,materialName );
#else    
	#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
	return CPUTSpriteOGL::CreateSprite( spriteX,spriteY,spriteWidth,spriteHeight,materialName );
	#else    
	#error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
	return NULL;
	#endif
#endif
    
}