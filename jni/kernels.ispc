// Copyright 2014 Intel Corporation
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

#ifndef ISPC
    // make VS2013 highlighting happier
    #define uniform
    #define export
    #define int64
    #define int32
    #define min
    #define max
    #define sqrt
#endif

typedef unsigned int64 uint64_t;
typedef unsigned int32 uint32_t;

#ifndef ISPC
typedef float float4;
typedef float float3;
#else
typedef float<4> float4;
typedef float<3> float3;
#endif

struct PointLight
{
    float positionView[3];
    float attenuationBegin;
    float color[3];
    float attenuationEnd;
};

struct LightBounds
{
    int p1[3], p2[3];
};

struct Camera
{
    float proj11;
    float proj22;
    float near;
    float far;
};

struct LightGridDimensions
{
    int width;
    int height;
    int depth;
};

// Bounds computation utilities, similar to GPUQuad.hlsl
void UpdateClipRegionRoot(float nc,          // Tangent plane x/y normal coordinate (view space)
    float lc,          // Light x/y coordinate (view space)
    float lz,          // Light z coordinate (view space)
    float lightRadius,
    float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
    float& clipMin,
    float& clipMax)
{
    float nz = (lightRadius - nc * lc) / lz;
    float pz = (lc * lc + lz * lz - lightRadius * lightRadius) / (lz - (nz / nc) * lc);

    if (pz > 0.0f) {
        float c = -nz * cameraScale / nc;
        if (nc > 0.0f)
        {                      // Left side boundary
            clipMin = max(clipMin, c);
        }
        else
        {                       // Right side boundary
            clipMax = min(clipMax, c);
        }
    }
}

void UpdateClipRegion(float lc,          // Light x/y coordinate (view space)
    float lz,          // Light z coordinate (view space)
    float lightRadius,
    float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
    float& clipMin,
    float& clipMax)
{
    float rSq = lightRadius * lightRadius;
    float lcSqPluslzSq = lc * lc + lz * lz;
    float d = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

    if (d > 0)
    {
        float a = lightRadius * lc;
        float b = sqrt(d);
        float nx0 = (a + b) / lcSqPluslzSq;
        float nx1 = (a - b) / lcSqPluslzSq;

        UpdateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
        UpdateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
    }
}

// Returns bounding box [min.xy, max.xy] in clip [-1, 1] space.
float4 ComputeClipRegion(float3 lightPosView, float lightRadius, uniform Camera camera[])
{
    // Early out with empty rectangle if the light is too far behind the view frustum
    float4 clipRegion = { 1, 1, 0, 0 };
    if (lightPosView.z + lightRadius >= camera->near) {
        float<2> clipMin = { -1.0f, -1.0f };
        float<2> clipMax = { 1.0f, 1.0f };

        UpdateClipRegion(lightPosView.x, lightPosView.z, lightRadius, camera->proj11, clipMin.x, clipMax.x);
        UpdateClipRegion(lightPosView.y, lightPosView.z, lightRadius, camera->proj22, clipMin.y, clipMax.y);

        float4 v = { clipMin.x, clipMin.y, clipMax.x, clipMax.y };
        clipRegion = v;
    }

    return clipRegion;
}

float3 to_float3(float v[3])
{
    float3 r = { v[0], v[1], v[2] };
    return r;
}

void GenerateLightBounds(PointLight light, LightBounds& box, uniform Camera camera[], uniform LightGridDimensions dim[])
{

    // compute view space quad
    float4 clipRegion = ComputeClipRegion(to_float3(light.positionView), light.attenuationEnd, camera);
    //clipRegion = D3DXVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f);

    //float4 clipRegion;
    clipRegion = (clipRegion + 1.0f) / 2; // map coordinates to [0..1]

    // meh, this is upside-down
    clipRegion.y = 1 - clipRegion.y;
    clipRegion.w = 1 - clipRegion.w;
    clipRegion = clipRegion.xwzy;

    int intClipRegion[4];
    intClipRegion[0] = (int)(clipRegion[0] * dim->width);
    intClipRegion[1] = (int)(clipRegion[1] * dim->height);
    intClipRegion[2] = (int)(clipRegion[2] * dim->width);
    intClipRegion[3] = (int)(clipRegion[3] * dim->height);

    if (intClipRegion[0] < 0) intClipRegion[0] = 0;
    if (intClipRegion[1] < 0) intClipRegion[1] = 0;
    if (intClipRegion[2] >= dim->width) intClipRegion[2] = dim->width - 1;
    if (intClipRegion[3] >= dim->height) intClipRegion[3] = dim->height - 1;

    float center_z = (light.positionView[2] - camera->near) / (camera->far - camera->near);
    float dist_z = light.attenuationEnd / (camera->far - camera->near);

    //dist_z = center_z = 0.5f;

    int intZBounds[2];
    intZBounds[0] = (int)((center_z - dist_z)* dim->depth);
    intZBounds[1] = (int)((center_z + dist_z)* dim->depth);

    if (intZBounds[0] < 0) intZBounds[0] = 0;
    if (intZBounds[1] >= dim->depth) intZBounds[1] = dim->depth - 1;

    box.p1[0] = intClipRegion[0];
    box.p2[0] = intClipRegion[2];
    box.p1[1] = intClipRegion[1];
    box.p2[1] = intClipRegion[3];

    box.p1[2] = intZBounds[0];
    box.p2[2] = intZBounds[1];
}

float dot(float3 a, float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float length(float3 v)
{
    return sqrt(dot(v, v));
}

inline bool separationtTest(int x, int y, int z, float3 lightPosition, float lightSize, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    //D3DXVECTOR4 mCameraNearFar = D3DXVECTOR4(viewerCamera->GetFarClip(), viewerCamera->GetNearClip(), 0.0f, 0.0f);
    //D3DXMATRIX mCameraProj = *viewerCamera->GetProjMatrix();

    int grid[3] = { dim->width, dim->height, dim->depth };

    // sub-frustrum bounds in view space        
    float minZ = (z - 0) * 1.0f / grid[2] * (camera->far - camera->near) + camera->near;
    float maxZ = (z + 1) * 1.0f / grid[2] * (camera->far - camera->near) + camera->near;

    float minZminX = -(1 - 2.0f / grid[0] * (x - 0))*minZ / camera->proj11;
    float minZmaxX = -(1 - 2.0f / grid[0] * (x + 1))*minZ / camera->proj11;
    float minZminY = (1 - 2.0f / grid[1] * (y - 0))*minZ / camera->proj22;
    float minZmaxY = (1 - 2.0f / grid[1] * (y + 1))*minZ / camera->proj22;

    float maxZminX = -(1 - 2.0f / grid[0] * (x - 0))*maxZ / camera->proj11;
    float maxZmaxX = -(1 - 2.0f / grid[0] * (x + 1))*maxZ / camera->proj11;
    float maxZminY = (1 - 2.0f / grid[1] * (y - 0))*maxZ / camera->proj22;
    float maxZmaxY = (1 - 2.0f / grid[1] * (y + 1))*maxZ / camera->proj22;

    // heuristic plane separation test - works pretty well in practice
    float3 minZcenter = { (minZminX + minZmaxX) / 2, (minZminY + minZmaxY) / 2, minZ };
    float3 maxZcenter = { (maxZminX + maxZmaxX) / 2, (maxZminY + maxZmaxY) / 2, maxZ };
    float3 center = (minZcenter + maxZcenter) / 2;
    float3 normal = center - lightPosition;
    normal /= length(normal);

    // compute distance of all corners to the tangent plane, with a few shortcuts (saves 14 muls)
    float min_d1 = -dot(normal, lightPosition);
    float min_d2 = min_d1;
    min_d1 += min(normal.x * minZminX, normal.x * minZmaxX);
    min_d1 += min(normal.y * minZminY, normal.y * minZmaxY);
    min_d1 += normal.z * minZ;
    min_d2 += min(normal.x * maxZminX, normal.x * maxZmaxX);
    min_d2 += min(normal.y * maxZminY, normal.y * maxZmaxY);
    min_d2 += normal.z * maxZ;
    float min_d = min(min_d1, min_d2);
    bool separated = min_d > lightSize;

    // exact frustrum-sphere test
    // gain depends on effectiveness of the plane heuristic:
    // with very fine-grained clusters, only ~0.5%
    // with badly balanced (64x32x32) clusters, 5-15%
    if (false)
    if (!separated)
    {
#if 0
        // corners of the convex hull
        D3DXVECTOR3 corners[8];
        corners[0] = D3DXVECTOR3(minZminX, minZminY, minZ);
        corners[1] = D3DXVECTOR3(minZmaxX, minZminY, minZ);
        corners[2] = D3DXVECTOR3(minZminX, minZmaxY, minZ);
        corners[3] = D3DXVECTOR3(minZmaxX, minZmaxY, minZ);
        corners[4] = D3DXVECTOR3(maxZminX, maxZminY, maxZ);
        corners[5] = D3DXVECTOR3(maxZmaxX, maxZminY, maxZ);
        corners[6] = D3DXVECTOR3(maxZminX, maxZmaxY, maxZ);
        corners[7] = D3DXVECTOR3(maxZmaxX, maxZmaxY, maxZ);

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
        D3DXVECTOR3 nearest_corners[4];
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

            D3DXVECTOR3 pp;
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
#endif
    }

    return separated;
}


extern "C" void pushFragment_callback(uniform size_t opaque, uniform int cellIndex, uniform int lightIndex, uniform uint64_t fragment);

void pushFragment(uniform size_t opaque, int cellIndex, int lightIndex, uint64_t fragment)
{
    foreach_active(instance)
    {
        pushFragment_callback(opaque, extract(cellIndex, instance), extract(lightIndex, instance), extract(fragment, instance));
    }
}

void GenerateLightBounds2(PointLight light, int lightIndex, uniform size_t opaque, uniform Camera camera[], uniform LightGridDimensions dim[])
{

    // compute view space quad
    float4 clipRegion = ComputeClipRegion(to_float3(light.positionView), light.attenuationEnd, camera);
    //clipRegion = D3DXVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f);

    //float4 clipRegion;
    clipRegion = (clipRegion + 1.0f) / 2; // map coordinates to [0..1]

    // meh, this is upside-down
    clipRegion.y = 1 - clipRegion.y;
    clipRegion.w = 1 - clipRegion.w;
    clipRegion = clipRegion.xwzy;

    int intClipRegion[4];
    intClipRegion[0] = (int)(clipRegion[0] * dim->width);
    intClipRegion[1] = (int)(clipRegion[1] * dim->height);
    intClipRegion[2] = (int)(clipRegion[2] * dim->width);
    intClipRegion[3] = (int)(clipRegion[3] * dim->height);

    if (intClipRegion[0] < 0) intClipRegion[0] = 0;
    if (intClipRegion[1] < 0) intClipRegion[1] = 0;
    if (intClipRegion[2] >= dim->width) intClipRegion[2] = dim->width - 1;
    if (intClipRegion[3] >= dim->height) intClipRegion[3] = dim->height - 1;

    float center_z = (light.positionView[2] - camera->near) / (camera->far - camera->near);
    float dist_z = light.attenuationEnd / (camera->far - camera->near);

    //dist_z = center_z = 0.5f;

    int intZBounds[2];
    intZBounds[0] = (int)((center_z - dist_z)* dim->depth);
    intZBounds[1] = (int)((center_z + dist_z)* dim->depth);

    if (intZBounds[0] < 0) intZBounds[0] = 0;
    if (intZBounds[1] >= dim->depth) intZBounds[1] = dim->depth - 1;

    LightBounds box;
    box.p1[0] = intClipRegion[0];
    box.p2[0] = intClipRegion[2];
    box.p1[1] = intClipRegion[1];
    box.p2[1] = intClipRegion[3];

    box.p1[2] = intZBounds[0];
    box.p2[2] = intZBounds[1];

    LightBounds region = box;

    for (int y = region.p1[1] / 4; y <= region.p2[1] / 4; y++)
    for (int x = region.p1[0] / 4; x <= region.p2[0] / 4; x++)
    for (int z = region.p1[2] / 4; z <= region.p2[2] / 4; z++)
    {
        int x1 = clamp(region.p1[0] - x * 4, 0, 3);
        int x2 = clamp(region.p2[0] - x * 4, 0, 3);
        int y1 = clamp(region.p1[1] - y * 4, 0, 3);
        int y2 = clamp(region.p2[1] - y * 4, 0, 3);
        int z1 = clamp(region.p1[2] - z * 4, 0, 3);
        int z2 = clamp(region.p2[2] - z * 4, 0, 3);

        uint64_t coverage = 0;
        for (int zz = z1; zz <= z2; zz++)
        for (int yy = y1; yy <= y2; yy++)
        for (int xx = x1; xx <= x2; xx++)
        {
            int fineIndex = (yy / 2 % 2) * 32 + (xx / 2 % 2) * 16 + (yy % 2) * 8 + (xx % 2) * 4 + (zz % 4);

            bool separated = false;

            //int grid[3] = { dim->width, dim->height, dim->depth };
            separated = separationtTest(x * 4 + xx, y * 4 + yy, z * 4 + zz, to_float3(light.positionView), light.attenuationEnd, camera, dim);

            uint64_t one = 1;

            if (!separated)
                coverage |= one << fineIndex;
        }

        int cellIndex = (y*dim->width / 4 + x)*dim->depth / 4 + z;

        pushFragment(opaque, cellIndex, lightIndex, coverage);
        //builder->pushFragment(dim.cellIndex(x, y, z), lightIndex, coverage);
    }
}

export void CoarseRasterizeLights(uniform PointLight lights[], uniform LightBounds bounds[], uniform int lightCount, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    foreach(idx = 0 ... lightCount)
    {
        //bounds[idx] = GenerateLightBounds(lights[idx], camera);
        LightBounds box;
        GenerateLightBounds(lights[idx], box, camera, dim);
        bounds[idx] = box;
    }
}

export void RasterizeLights(uniform PointLight lights[], uniform size_t opaque, uniform int lightCount, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    foreach(idx = 0 ... lightCount)
    {
        GenerateLightBounds2(lights[idx], idx, opaque, camera, dim);
    }
}

struct Fragment
{
    int cellIndex;
    int lightIndex;
    uint64_t coverage;
};

uint64_t ComputeCoverage(int cellIndex, float3 lightPosition, float lightSize, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    //int cellIndex = (y*dim.width / 4 + x)*dim.depth / 4 + z;
    int z = cellIndex % (dim->depth / 4);
    int x = (cellIndex / (dim->depth / 4)) % (dim->width / 4);
    int y = cellIndex / (dim->depth / 4 * dim->width / 4);

    uint64_t coverage = 0;
    for (uniform int zz = 0; zz <= 3; zz++)
    for (uniform int yy = 0; yy <= 3; yy++)
    for (uniform int xx = 0; xx <= 3; xx++)
    {
        uniform int fineIndex = (yy / 2 % 2) * 32 + (xx / 2 % 2) * 16 + (yy % 2) * 8 + (xx % 2) * 4 + (zz % 4);

        int fx = x * 4 + xx;
        int fy = y * 4 + yy;
        int fz = z * 4 + zz;

        bool separated = false;
        separated = separationtTest(x * 4 + xx, y * 4 + yy, z * 4 + zz, lightPosition, lightSize, camera, dim);

        uniform uint64_t one = 1;
        if (!separated)
            coverage |= one << fineIndex;
    }

    return coverage;
}

extern "C" void debugBreak();

uint64_t ComputeCoverage2(int cellIndex, float3 lightPosition, float lightSize, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    //int cellIndex = (y*dim.width / 4 + x)*dim.depth / 4 + z;
     int cz = cellIndex % (dim->depth / 4);
     int cx = (cellIndex / (dim->depth / 4)) % (dim->width / 4);
     int cy = cellIndex / (dim->depth / 4 * dim->width / 4);

    uint64_t coverage = 0;
    for (uniform  int zz = 0; zz <= 3; zz++)
    {
        int z = cz * 4 + zz;
        float minZ = (z - 0) * 1.0f / dim->depth * (camera->far - camera->near) + camera->near;
        float maxZ = (z + 1) * 1.0f / dim->depth * (camera->far - camera->near) + camera->near;

        for (uniform  int yy = 0; yy <= 3; yy++)
        {
            int y = cy * 4 + yy;
            float minZminY = (1 - 2.0f / dim->height * (y - 0))*minZ / camera->proj22;
            float minZmaxY = (1 - 2.0f / dim->height * (y + 1))*minZ / camera->proj22;
            float maxZminY = (1 - 2.0f / dim->height * (y - 0))*maxZ / camera->proj22;
            float maxZmaxY = (1 - 2.0f / dim->height * (y + 1))*maxZ / camera->proj22;

            float minZmulX = 2.0f / dim->width *minZ / camera->proj11;
            float minZaddX = -minZ / camera->proj11;
            float maxZmulX = 2.0f / dim->width *maxZ / camera->proj11;
            float maxZaddX = -maxZ / camera->proj11;

            for (uniform  int xx = 0; xx <= 3; xx++)
            {
                //debugBreak();
                uniform int fineIndex = (yy / 2 % 2) * 32 + (xx / 2 % 2) * 16 + (yy % 2) * 8 + (xx % 2) * 4 + (zz % 4);
                
                int x = cx * 4 + xx;
                //float minZminX = -(1 - 2.0f / dim->width * (x - 0))*minZ / camera->proj11;
                //float minZmaxX = -(1 - 2.0f / dim->width * (x + 1))*minZ / camera->proj11;
                //float maxZminX = -(1 - 2.0f / dim->width * (x - 0))*maxZ / camera->proj11;
                //float maxZmaxX = -(1 - 2.0f / dim->width * (x + 1))*maxZ / camera->proj11;
                float minZminX = (x - 0) * minZmulX + minZaddX;
                float minZmaxX = (x + 1) * minZmulX + minZaddX;
                float maxZminX = (x - 0) * maxZmulX + maxZaddX;
                float maxZmaxX = (x + 1) * maxZmulX + maxZaddX;

                // heuristic plane separation test - works pretty well in practice
                float3 minZcenter2 = { minZminX + minZmaxX, minZminY + minZmaxY, minZ * 2 };
                float3 maxZcenter2 = { maxZminX + maxZmaxX, maxZminY + maxZmaxY, maxZ * 2 };
                float3 center = (minZcenter2 + maxZcenter2) * 0.25;
                float3 normal = center - lightPosition;
                //normal /= length(normal);
                normal *= rsqrt(dot(normal, normal));

                // compute distance of all corners to the tangent plane, with a few shortcuts (saves 14 muls)
                float min_d1 = -dot(normal, lightPosition);
                float min_d2 = min_d1;
                min_d1 += min(normal.x * minZminX, normal.x * minZmaxX);
                min_d1 += min(normal.y * minZminY, normal.y * minZmaxY);
                min_d1 += normal.z * minZ;
                min_d2 += min(normal.x * maxZminX, normal.x * maxZmaxX);
                min_d2 += min(normal.y * maxZminY, normal.y * maxZmaxY);
                min_d2 += normal.z * maxZ;
                float min_d = min(min_d1, min_d2);
                bool separated = min_d > lightSize;

                uniform uint64_t one = 1;
                if (!separated)
                    coverage |= one << fineIndex;
            }
        }
    }

    return coverage;
}

float sq(float v)
{
    return v*v;
}

inline uniform int getFineIndex(uniform int xx, uniform int yy)
{
    const uniform int fineIndexTable[][4] =
    {
        {  0,  1,  4,  5 },
        {  2,  3,  6,  7 },
        {  8,  9, 12, 13 },
        { 10, 11, 14, 15 },
    };
    return fineIndexTable[yy][xx];
}

uint64_t ComputeCoverage3(int cellIndex, float3 lightPosition, float lightSize, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    //int cellIndex = (y*dim.width / 4 + x)*dim.depth / 4 + z;
    int cz = cellIndex % (dim->depth / 4);
    int cx = (cellIndex / (dim->depth / 4)) % (dim->width / 4);
    int cy = cellIndex / (dim->depth / 4 * dim->width / 4);

    uint64_t coverage = 0;
    for (uniform int zz = 0; zz <= 3; zz++)
    {
        int z = cz * 4 + zz;
        float minZ = (z + 0) * 1.0f / dim->depth * (camera->far - camera->near) + camera->near;
        float maxZ = (z + 1) * 1.0f / dim->depth * (camera->far - camera->near) + camera->near;

        float minZsqRadius = sq(lightSize) - sq(minZ - lightPosition.z);
        float maxZsqRadius = sq(lightSize) - sq(maxZ - lightPosition.z);
        float maxSqRadius = max(minZsqRadius, maxZsqRadius);

        if (minZ < lightPosition.z && lightPosition.z < maxZ)
        {
            maxSqRadius = sq(lightSize);
        }

        float minZmulX = 2.0f / dim->width *minZ / camera->proj11;
        float minZaddX = -minZ / camera->proj11;

        float minZmulY = -2.0f / dim->height *minZ / camera->proj22;
        float minZaddY = minZ / camera->proj22;

        int yypos = floor((lightPosition.y - minZaddY) / minZmulY) - cy * 4;
        int xxpos = floor((lightPosition.x - minZaddX) / minZmulX) - cx * 4;

        float sqDistX[5];
        for (uniform int xx = 0; xx <= 4; xx++)
        {
            int x = cx * 4 + xx;
            float posX = x * minZmulX + minZaddX;
            sqDistX[xx] = sq(posX - lightPosition.x);
        }

        float sqDistY[5];
        for (uniform int yy = 0; yy <= 4; yy++)
        {
            int y = cy * 4 + yy;
            float posY = y * minZmulY + minZaddY;
            sqDistY[yy] = sq(posY - lightPosition.y);
        }

        float minSqDistX[4];
        for (uniform int xx = 0; xx <= 3; xx++)
        {
            float sqDist = min(sqDistX[xx], sqDistX[xx + 1]);
            minSqDistX[xx] = select(xx != xxpos, sqDist, 0);
        }

        float minSqDistY[4];
        for (uniform int yy = 0; yy <= 3; yy++)
        {
            float sqDist = min(sqDistY[yy], sqDistY[yy + 1]);
            minSqDistY[yy] = select(yy != yypos, sqDist, 0);
        }

        uint32_t coverage_l = 0;
        uint32_t coverage_h = 0;

        for (uniform int yy = 0; yy <= 3; yy++)
        for (uniform int xx = 0; xx <= 3; xx++)
        {
            //debugBreak();
            uniform unsigned int fineIndex = getFineIndex(xx, yy) * 4 + zz;

            float totalDist = minSqDistX[xx] + minSqDistY[yy];
            bool separated = totalDist > maxSqRadius;

            if (!separated)
            {
                uniform uint32_t mask_l = select(fineIndex < 32, 1 << fineIndex, 0);
                uniform uint32_t mask_h = select(fineIndex < 32, 0, 1 << (fineIndex - 32));
                coverage_h |= mask_h;
                coverage_l |= mask_l;
            }
        }

        uint64_t zero = 0;
        coverage |= (zero+coverage_h) << 32 | coverage_l;
    }

    return coverage;
}

export void FineRasterizeLights(uniform PointLight lights[], uniform Fragment fragments[], uniform int fragmentCount, uniform Camera camera[], uniform LightGridDimensions dim[])
{
    foreach(idx = 0 ... fragmentCount)
    {
        PointLight light = lights[fragments[idx].lightIndex];
        fragments[idx].coverage = ComputeCoverage2(fragments[idx].cellIndex, to_float3(light.positionView), light.attenuationEnd, camera, dim);
        //fragments[idx].coverage = 0xFFFFFFFFFFFFFFFF;
    }
}
