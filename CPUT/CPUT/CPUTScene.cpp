/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or imlied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUTScene.h"
#include "CPUTAssetLibrary.h"

//-----------------------------------------------------------------------------
CPUTResult CPUTScene::LoadScene(const std::string &sceneFileName)
{
    CPUTResult result = CPUT_SUCCESS;

    result = mSceneFile.LoadFile(sceneFileName);
    if (CPUTFAILED(result)) {
        DEBUG_PRINT(_L("Failed to load scene: %s"), sceneFileName.data());
        return result;
    }

    CPUTConfigBlockA *pAssetsBlock = mSceneFile.GetBlockByName("Assets");
    if (pAssetsBlock == NULL) {
        DEBUG_PRINT(_L("Failed to load Assets"));
        return CPUT_WARNING_NO_ASSETS_LOADED;
    }

    int numAssets = pAssetsBlock->ValueCount();
    if (numAssets <= 0) {
        DEBUG_PRINT(_L("Failed to load Assets"));
        return CPUT_WARNING_NO_ASSETS_LOADED;
    }

    CPUTAssetSet *pAssetSet = NULL;
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    for (int i = 0; i < numAssets; ++i) {
         CPUTConfigEntryA *pEntry = pAssetsBlock->GetValue(i);
         if (pEntry == NULL) {
             continue;
         }

         std::string resolvedAssetNameAndPath;
         char * mediaDirectory = cs2s(pAssetLibrary->GetMediaDirectoryName());
         CPUTFileSystem::ResolveAbsolutePathAndFilename(mediaDirectory + pEntry->NameAsString(), &resolvedAssetNameAndPath);
         delete mediaDirectory;

         //
         // Extract the set file name off the end of the path
         //
         const char *delimiters = "\\/";
         size_t pos = resolvedAssetNameAndPath.find_last_of(delimiters);
         if (pos == std::string::npos) {
             // then there are no directories in the path provided. There should always be at least /asset/
         }
         std::string assetFileName = resolvedAssetNameAndPath.substr(pos + 1); // +1 to skip the '/' or '\' character

         //
         // the assetname will always end in /asset/name.set
         //
         pos = resolvedAssetNameAndPath.rfind("asset");
         if (pos == std::string::npos) {
             pos = resolvedAssetNameAndPath.rfind("Asset");
         }
         if (pos == std::string::npos) {
             // then the set file is not in the correct folder
         }
         std::string assetFilePath = resolvedAssetNameAndPath.substr(0, pos);

         pAssetLibrary->SetAssetDirectoryName(s2cs(assetFilePath));

         pAssetSet  = pAssetLibrary->GetAssetSet(s2ws(resolvedAssetNameAndPath.c_str()), true); // need to state that this is the fully qualified path name so CPUT will not append a .set extension
        if (!pAssetSet)
            DEBUG_PRINT(_L("Failed to load AssetSet"));// %p", pAssetSet);
         ASSERTA( pAssetSet, "Failed loading" + assetFilePath);
         mpAssetSetList[mNumAssetSets] = pAssetSet;
         mNumAssetSets++;

         ASSERTA(mNumAssetSets <= MAX_NUM_ASSETS, "Number of Assets in scene file exceeds MAX_NUM_ASSETS");
    }

    CalculateBoundingBox();

    return result;
}

//-----------------------------------------------------------------------------
void CPUTScene::CalculateBoundingBox()
{
    mMinExtent.x = mMinExtent.y = mMinExtent.z =  FLT_MAX;
    mMaxExtent.x = mMaxExtent.y = mMaxExtent.z = -FLT_MAX;

    for (UINT i = 0; i < mNumAssetSets; ++i) {
        float3 lookAtPoint(0.0f, 0.0f, 0.0f);
        float3 half(1.0f, 1.0f, 1.0f);
        mpAssetSetList[i]->GetBoundingBox( &lookAtPoint, &half );

        mMinExtent = Min( (lookAtPoint - half), mMinExtent );
        mMaxExtent = Max( (lookAtPoint + half), mMaxExtent );
    }

    mSceneBoundingBoxCenter  = (mMaxExtent + mMinExtent) * 0.5f;
    mSceneBoundingBoxHalf    = (mMaxExtent - mMinExtent) * 0.5f;
}

//-----------------------------------------------------------------------------
void CPUTScene::Render(CPUTRenderParameters &renderParameters, int materialIndex)
{
    for (UINT i = 0; i < mNumAssetSets; ++i)
    {
        mpAssetSetList[i]->RenderRecursive(renderParameters, materialIndex);
    }
}
void CPUTScene::Update( float dt )
{
	for (UINT i = 0; i < mNumAssetSets; ++i)
	{
		mpAssetSetList[i]->UpdateRecursive(dt);
	}
}
