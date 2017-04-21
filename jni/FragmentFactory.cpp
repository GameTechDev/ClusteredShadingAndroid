/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUT.h"
#include <cassert>
#include "FragmentFactory.h"
#include <algorithm>


#undef near
#undef far
#include "kernels_ispc.h"


class FragmentFactory
{
    vector<uint64_t> masks;

public:
    FragmentFactory();
    uint64_t coverage(int x1, int x2, int y1, int y2, int z1, int z2) const;
};

FragmentFactory::FragmentFactory()
{
    masks.resize(48);

    for (int k = 0; k < 16; k++)
    {
        int b = k % 4;
        int a = k / 4;

        uint64_t one = 1;
        uint64_t x_segment = 0;
        uint64_t y_segment = 0;
        uint64_t z_segment = 0;

        for (int l = a; l <= b; l++)
        for (int m = 0; m < 16; m++)
        {
            x_segment |= one << ((l / 2 % 2 + m / 8 % 2 * 2) * 16 + (l % 2 + m / 4 % 2 * 2) * 4 + (m % 4));
            y_segment |= one << ((l / 2 % 2 * 2 + m / 8 % 2) * 16 + (l % 2 * 2 + m / 4 % 2) * 4 + (m % 4));
            //x_segment |= one * 15 << (m * 4);
            //y_segment |= one * 15 << (m * 4);
            z_segment |= one << (m * 4 + l);
        }

        //x_segment = 0xFFFFFFFFFFFFFFFFull;
        //y_segment = 0xFFFFFFFFFFFFFFFFull;
        //z_segment = 0xFFFFFFFFFFFFFFFFull;

        masks[0 + k] = x_segment;
        masks[16 + k] = y_segment;
        masks[32 + k] = z_segment;
    }
}

uint64_t FragmentFactory::coverage(int x1, int x2, int y1, int y2, int z1, int z2) const
{
    uint64_t x_segment = masks[0 + x1 * 4 + x2];
    uint64_t y_segment = masks[16 + y1 * 4 + y2];
    uint64_t z_segment = masks[32 + z1 * 4 + z2];
    uint64_t coverage = x_segment & y_segment & z_segment;
    return coverage;
}

template<typename T>
T clamp(T v, T lb, T ub)
{
    return std::min(std::max(v, lb), ub);
}

// Bounds computation utilities, similar to GPUQuad.hlsl
void UpdateClipRegionRoot(float nc,          // Tangent plane x/y normal coordinate (view space)
    float lc,          // Light x/y coordinate (view space)
    float lz,          // Light z coordinate (view space)
    float lightRadius,
    float cameraScale, // Project scale for coordinate (r0.x or r1.y for x/y respectively)
    float& clipMin,
    float& clipMax)
{
    float nz = (lightRadius - nc * lc) / lz;
    float pz = (lc * lc + lz * lz - lightRadius * lightRadius) / (lz - (nz / nc) * lc);

    if (pz > 0.0f) {
        float c = -nz * cameraScale / nc;
        if (nc > 0.0f)
        {                      // Left side boundary
            clipMin = std::max(clipMin, c);
        }
        else
        {                       // Right side boundary
            clipMax = std::min(clipMax, c);
        }
    }
}

void UpdateClipRegion(float lc,          // Light x/y coordinate (view space)
    float lz,          // Light z coordinate (view space)
    float lightRadius,
    float cameraScale, // Project scale for coordinate (r0.x or r1.y for x/y respectively)
    float& clipMin,
    float& clipMax)
{
    float rSq = lightRadius * lightRadius;
    float lcSqPluslzSq = lc * lc + lz * lz;
    float d = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

    if (d > 0)
    {
        float a = lightRadius * lc;
        float b = sqrtf(d);
        float nx0 = (a + b) / lcSqPluslzSq;
        float nx1 = (a - b) / lcSqPluslzSq;

        UpdateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
        UpdateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
    }
}

// Returns bounding box [min.xy, max.xy] in clip [-1, 1] space.
float4 ComputeClipRegion(const float3 &lightPosView, float lightRadius,
    const float4x4& mCameraProj, const float4& mCameraNearFar)
{
    // Early out with empty rectangle if the light is too far behind the view frustum
    float4 clipRegion = float4(1, 1, 0, 0);
    if (lightPosView.z + lightRadius >= mCameraNearFar.x) {
        float2 clipMin = float2(-1.0f, -1.0f);
        float2 clipMax = float2(1.0f, 1.0f);

        UpdateClipRegion(lightPosView.x, lightPosView.z, lightRadius, mCameraProj.r0.x, clipMin.x, clipMax.x);
        UpdateClipRegion(lightPosView.y, lightPosView.z, lightRadius, mCameraProj.r1.y, clipMin.y, clipMax.y);

        clipRegion = float4(clipMin.x, clipMin.y, clipMax.x, clipMax.y);
    }

    return clipRegion;
}

struct
{
    int lights;
    int clusters_rasterized;
    int clusters_unseparated;
    int clusters_lit;
} rasterization_stats;


float length(const float3 &vec)
{
    return vec.length();
}

float length_sq(const float3 &vec)
{
    return dot3(vec, vec);
}

float dot(const float3 &a, const float3 &b)
{
    return dot3(a, b);
}

void GetNearFarFromProjMatr(const float4x4 &mProj, float &fNear, float &fFar)
{
    fNear = -mProj.r3.z / mProj.r2.z;
    fFar = mProj.r2.z / (mProj.r2.z-1) * fNear;
}

bool separationtTest(int x, int y, int z, int grid[3], const float4x4 &mCameraProj, PointLight* light)
{
    float4 mCameraNearFar;// = float4(viewerCamera->GetFarClip(), viewerCamera->GetNearClip(), 0.0f, 0.0f);
    GetNearFarFromProjMatr(mCameraProj, mCameraNearFar.y, mCameraNearFar.x);
    //D3DXMATRIX mCameraProj = *viewerCamera->GetProjMatrix();

    // sub-frustrum bounds in view space        
    float minZ = (z - 0) * 1.0f / grid[2] * (mCameraNearFar.y - mCameraNearFar.x) + mCameraNearFar.x;
    float maxZ = (z + 1) * 1.0f / grid[2] * (mCameraNearFar.y - mCameraNearFar.x) + mCameraNearFar.x;

    float minZminX = -(1 - 2.0f / grid[0] * (x - 0))*minZ / mCameraProj.r0.x;
    float minZmaxX = -(1 - 2.0f / grid[0] * (x + 1))*minZ / mCameraProj.r0.x;
    float minZminY = (1 - 2.0f / grid[1] * (y - 0))*minZ / mCameraProj.r1.y;
    float minZmaxY = (1 - 2.0f / grid[1] * (y + 1))*minZ / mCameraProj.r1.y;

    float maxZminX = -(1 - 2.0f / grid[0] * (x - 0))*maxZ / mCameraProj.r0.x;
    float maxZmaxX = -(1 - 2.0f / grid[0] * (x + 1))*maxZ / mCameraProj.r0.x;
    float maxZminY = (1 - 2.0f / grid[1] * (y - 0))*maxZ / mCameraProj.r1.y;
    float maxZmaxY = (1 - 2.0f / grid[1] * (y + 1))*maxZ / mCameraProj.r1.y;

    // heuristic plane separation test - works pretty well in practice
    float3 minZcenter = float3((minZminX + minZmaxX) / 2, (minZminY + minZmaxY) / 2, minZ);
    float3 maxZcenter = float3((maxZminX + maxZmaxX) / 2, (maxZminY + maxZmaxY) / 2, maxZ);
    float3 center = (minZcenter + maxZcenter) / 2;
    float3 normal = center - light->positionView;
    normal /= normal.length();

    // compute distance of all corners to the tangent plane, with a few shortcuts (saves 14 muls)
    float min_d1 = -dot(normal, light->positionView);
    float min_d2 = min_d1;
    min_d1 += std::min(normal.x * minZminX, normal.x * minZmaxX);
    min_d1 += std::min(normal.y * minZminY, normal.y * minZmaxY);
    min_d1 += normal.z * minZ;
    min_d2 += std::min(normal.x * maxZminX, normal.x * maxZmaxX);
    min_d2 += std::min(normal.y * maxZminY, normal.y * maxZmaxY);
    min_d2 += normal.z * maxZ;
    float min_d = std::min(min_d1, min_d2);
    bool separated = min_d > light->attenuationEnd;
    
    // exact frustrum-sphere test
    // gain depends on effectiveness of the plane heuristic:
    // with very fine-grained clusters, only ~0.5%
    // with badly balanced (64x32x32) clusters, 5-15%
    if (false)
    if (!separated)
    {
        // corners of the convex hull
        float3 corners[8];
        corners[0] = float3(minZminX, minZminY, minZ);
        corners[1] = float3(minZmaxX, minZminY, minZ);
        corners[2] = float3(minZminX, minZmaxY, minZ);
        corners[3] = float3(minZmaxX, minZmaxY, minZ);
        corners[4] = float3(maxZminX, maxZminY, maxZ);
        corners[5] = float3(maxZmaxX, maxZminY, maxZ);
        corners[6] = float3(maxZminX, maxZmaxY, maxZ);
        corners[7] = float3(maxZmaxX, maxZmaxY, maxZ);

        int min_k = 0;
        float min_dist = length(corners[0] - light->positionView);
        for (int k = 1; k < 8; k++)
        {
            if (length(corners[k] - light->positionView) < min_dist)
            {
                min_dist = length(corners[k] - light->positionView);
                min_k = k;
            }
        }

        // closest edges 
        float3 nearest_corners[4];
        nearest_corners[0] = corners[min_k ^ 0];
        nearest_corners[1] = corners[min_k ^ 1];
        nearest_corners[2] = corners[min_k ^ 2];
        nearest_corners[3] = corners[min_k ^ 4];

        float dot_a = dot(light->positionView - nearest_corners[0], nearest_corners[1] - nearest_corners[0]);
        float dot_b = dot(light->positionView - nearest_corners[0], nearest_corners[2] - nearest_corners[0]);
        float dot_c = dot(light->positionView - nearest_corners[0], nearest_corners[3] - nearest_corners[0]);

        if (dot_a < 0 && dot_b < 0 && dot_c < 0)
        {
            // point-sphere
        }
        else if (dot_a < 0 && dot_b < 0)
        {
            // edge-sphere
            float dot = dot_c;
            float norm = length(nearest_corners[3] - nearest_corners[0]);
            min_dist = sqrtf(min_dist*min_dist - dot*dot / (norm*norm));
        }
        else if (dot_a < 0 && dot_c < 0)
        {
            // edge-sphere
            float dot = dot_b;
            float norm = length(nearest_corners[2] - nearest_corners[0]);
            min_dist = sqrtf(min_dist*min_dist - dot*dot / (norm*norm));
        }
        else if (dot_b < 0 && dot_c < 0)
        {
            // edge-sphere
            float dot = dot_a;
            float norm = length(nearest_corners[1] - nearest_corners[0]);
            min_dist = sqrtf(min_dist*min_dist - dot*dot / (norm*norm));
        }
        else if (dot_a < 0 || dot_b < 0 || dot_c < 0)
        {
            // plane-sphere
            float dot_xp;
            float dot_yp;
            float dot_xx;
            float dot_yy;
            float dot_xy;

            if (dot_c < 0)
            {
                dot_xp = dot_a;
                dot_yp = dot_b;
                dot_xx = length_sq(nearest_corners[1] - nearest_corners[0]);
                dot_yy = length_sq(nearest_corners[2] - nearest_corners[0]);
                dot_xy = dot(nearest_corners[1] - nearest_corners[0], nearest_corners[2] - nearest_corners[0]);
            }
            else if (dot_b < 0)
            {
                dot_xp = dot_a;
                dot_yp = dot_c;
                dot_xx = length_sq(nearest_corners[1] - nearest_corners[0]);
                dot_yy = length_sq(nearest_corners[3] - nearest_corners[0]);
                dot_xy = dot(nearest_corners[1] - nearest_corners[0], nearest_corners[3] - nearest_corners[0]);
            }
            else if (dot_a < 0)
            {
                dot_xp = dot_b;
                dot_yp = dot_c;
                dot_xx = length_sq(nearest_corners[2] - nearest_corners[0]);
                dot_yy = length_sq(nearest_corners[3] - nearest_corners[0]);
                dot_xy = dot(nearest_corners[2] - nearest_corners[0], nearest_corners[3] - nearest_corners[0]);
            }

            // 2x2 matrix solve => get coordinates in plane
            float inv_det = 1 / (dot_xx*dot_yy - dot_xy*dot_xy);
            float mu_x = inv_det*(dot_yy*dot_xp - dot_xy*dot_yp);
            float mu_y = inv_det*(-dot_xy*dot_xp + dot_xx*dot_yp);

            float3 pp;
            if (dot_c < 0) pp = mu_x*(nearest_corners[1] - nearest_corners[0]) + mu_y*(nearest_corners[2] - nearest_corners[0]);
            if (dot_b < 0) pp = mu_x*(nearest_corners[1] - nearest_corners[0]) + mu_y*(nearest_corners[3] - nearest_corners[0]);
            if (dot_a < 0) pp = mu_x*(nearest_corners[2] - nearest_corners[0]) + mu_y*(nearest_corners[3] - nearest_corners[0]);

            float dot_xpp = 0, dot_ypp = 0;

            if (dot_c < 0) dot_xpp = dot(pp, nearest_corners[1] - nearest_corners[0]);
            if (dot_c < 0) dot_ypp = dot(pp, nearest_corners[2] - nearest_corners[0]);
            if (dot_b < 0) dot_xpp = dot(pp, nearest_corners[1] - nearest_corners[0]);
            if (dot_b < 0) dot_ypp = dot(pp, nearest_corners[3] - nearest_corners[0]);
            if (dot_a < 0) dot_xpp = dot(pp, nearest_corners[2] - nearest_corners[0]);
            if (dot_a < 0) dot_ypp = dot(pp, nearest_corners[3] - nearest_corners[0]);

            assert(abs(dot_xpp - dot_xp) < 0.001f);
            assert(abs(dot_ypp - dot_yp) < 0.001f);

            float dot_pp = mu_x*mu_x*dot_xx + mu_y*mu_y*dot_yy + 2 * mu_x*mu_y*dot_xy;

            min_dist = sqrtf(min_dist*min_dist - dot_pp);
        }
        else
        {
            // inside
            min_dist = 0;
        }

        if (min_dist > light->attenuationEnd)
            separated = true;
    }

    return separated;
}

void GenerateLightFragments(const FragmentFactory& fragmentFactory, LightGridBuilder* builder,
                            const float4x4 &mCameraProj, PointLight* light, int lightIndex)
{
    LightGridDimensions dim = builder->dimensions();
    float4 mCameraNearFar;// = float4(viewerCamera->GetFarClip(), viewerCamera->GetNearClip(), 0.0f, 0.0f);
    GetNearFarFromProjMatr(mCameraProj, mCameraNearFar.y, mCameraNearFar.x);
    //D3DXMATRIX mCameraProj = *viewerCamera->GetProjMatrix();

    // compute view space quad
    float4 clipRegion = ComputeClipRegion(light->positionView, light->attenuationEnd, mCameraProj, mCameraNearFar);

    //clipRegion = float4(-1.0f, -1.0f, 1.0f, 1.0f);
    clipRegion = (clipRegion + float4(1.0f, 1.0f, 1.0f, 1.0f)) / 2; // map coordinates to [0..1]

    // meh, this is upside-down
    clipRegion.y = 1 - clipRegion.y;
    clipRegion.w = 1 - clipRegion.w;
    std::swap(clipRegion.y, clipRegion.w);

    int intClipRegion[4];
    intClipRegion[0] = (int)(clipRegion.x * dim.width);
    intClipRegion[1] = (int)(clipRegion.y * dim.height);
    intClipRegion[2] = (int)(clipRegion.z * dim.width);
    intClipRegion[3] = (int)(clipRegion.w * dim.height);

    if (intClipRegion[0] < 0) intClipRegion[0] = 0;
    if (intClipRegion[1] < 0) intClipRegion[1] = 0;
    if (intClipRegion[2] >= dim.width) intClipRegion[2] = dim.width - 1;
    if (intClipRegion[3] >= dim.height) intClipRegion[3] = dim.height - 1;

    float center_z = (light->positionView.z - mCameraNearFar.x) / (mCameraNearFar.y - mCameraNearFar.x);
    float dist_z = light->attenuationEnd / (mCameraNearFar.y - mCameraNearFar.x);

    //dist_z = center_z = 0.5f;

    int intZBounds[2];
    intZBounds[0] = (int)((center_z - dist_z)* dim.depth);
    intZBounds[1] = (int)((center_z + dist_z)* dim.depth);

    if (intZBounds[0] < 0) intZBounds[0] = 0;
    if (intZBounds[1] >= dim.depth) intZBounds[1] = dim.depth - 1;


    for (int y = intClipRegion[1] / 4; y <= intClipRegion[3] / 4; y++)
    for (int x = intClipRegion[0] / 4; x <= intClipRegion[2] / 4; x++)
    for (int z = intZBounds[0] / 4; z <= intZBounds[1] / 4; z++)
    {
        int x1 = clamp(intClipRegion[0] - x * 4, 0, 3);
        int x2 = clamp(intClipRegion[2] - x * 4, 0, 3);
        int y1 = clamp(intClipRegion[1] - y * 4, 0, 3);
        int y2 = clamp(intClipRegion[3] - y * 4, 0, 3);
        int z1 = clamp(intZBounds[0] - z * 4, 0, 3);
        int z2 = clamp(intZBounds[1] - z * 4, 0, 3);

        uint64_t coverage = 0; 
        coverage = fragmentFactory.coverage(x1, x2, y1, y2, z1, z2);

        if (false)
        for (int zz = z1; zz <= z2; zz++)
        for (int yy = y1; yy <= y2; yy++)
        for (int xx = x1; xx <= x2; xx++)
        {
            int fineIndex = (yy / 2 % 2) * 32 + (xx / 2 % 2) * 16 + (yy % 2) * 8 + (xx % 2) * 4 + (zz % 4);

            bool separated = false;

            int grid[3];
            grid[0] = dim.width;
            grid[1] = dim.height;
            grid[2] = dim.depth;
            //separated = separationtTest(x * 4 + xx, y * 4 + yy, z * 4 + zz, grid, viewerCamera, light);

            if (!separated)
                coverage |= uint64_t(1) << fineIndex;
        }

        builder->pushFragment(dim.cellIndex(x, y, z), lightIndex, coverage);
    }
}

extern "C" void pushFragment_callback(size_t opaque, int cellindex, int lightIndex, uint64_t fragment);

void pushFragment_callback(size_t opaque, int cellindex, int lightIndex, uint64_t fragment)
{
    LightGridBuilder* builder = (LightGridBuilder*)opaque;
    builder->pushFragment(cellindex, lightIndex, fragment);
}

vector<ispc::Fragment> gFragments;

void RasterizeLights(LightGridBuilder* builder, const float4x4& mCameraProj, PointLight lights[], int lightCount)
{
    ispc::Camera camera;
    FragmentFactory fragmentFactory;
    //D3DXMATRIX mCameraProj = *viewerCamera->GetProjMatrix();

    // z is flipped...
    GetNearFarFromProjMatr(mCameraProj, camera.far, camera.near);
    //camera.far = viewerCamera->GetNearClip();
    //camera.near = viewerCamera->GetFarClip();
    camera.proj11 = mCameraProj.r0.x;
    camera.proj22 = mCameraProj.r1.y;

#define USE_SIMD 0
#if !USE_SIMD
    for (int lightIndex = 0; lightIndex < lightCount; lightIndex++)
    {
        GenerateLightFragments(fragmentFactory, builder, mCameraProj, &lights[lightIndex], lightIndex);
    }
#else
    bool twopass = true;
    if (twopass)
    {
        vector<ispc::LightBounds> bounds;
        bounds.resize(lightCount);
        LightGridDimensions dim = builder->dimensions();

        //vector<ispc::Fragment> fragments;
        //fragments.reserve(8 * lightCount);
        vector<ispc::Fragment>& fragments = gFragments;
        gFragments.resize(0);

        ispc::CoarseRasterizeLights((ispc::PointLight*)lights, &bounds[0], lightCount, &camera, (ispc::LightGridDimensions*)&dim);

        for (int lightIndex = 0; lightIndex < lightCount; lightIndex++)
        {
            ispc::LightBounds region = bounds[lightIndex];

            for (int y = region.p1[1] / 4; y <= region.p2[1] / 4; y++)
            for (int x = region.p1[0] / 4; x <= region.p2[0] / 4; x++)
            for (int z = region.p1[2] / 4; z <= region.p2[2] / 4; z++)
            {
                ispc::Fragment fragment;
                fragment.cellIndex = dim.cellIndex(x, y, z);
                fragment.lightIndex = lightIndex;
                fragments.push_back(fragment);
            }
        }

        int fragCount = fragments.size();
        ispc::FineRasterizeLights((ispc::PointLight*)lights, &fragments[0], fragCount, &camera, (ispc::LightGridDimensions*)&dim);

        for (int fragIndex = 0; fragIndex < fragCount; fragIndex++)
        {
            ispc::Fragment fragment = fragments[fragIndex];

            builder->pushFragment(fragment.cellIndex, fragment.lightIndex, fragment.coverage);
        }
    }
    else
    {
        LightGridDimensions dim = builder->dimensions();
        ispc::RasterizeLights((ispc::PointLight*)lights, (size_t)builder, lightCount, &camera, (ispc::LightGridDimensions*)&dim);
    }
#endif
}
