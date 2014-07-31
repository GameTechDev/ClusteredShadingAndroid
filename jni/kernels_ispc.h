//
// C:/Users/eayusov/Perforce/eayusov_djbookou-desk5/projects/samples/ClusteredShadingAndroid/jni/kernels_ispc.h
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#ifndef ISPC_C__USERS_EAYUSOV_PERFORCE_EAYUSOV_DJBOOKOU_DESK5_PROJECTS_SAMPLES_CLUSTEREDSHADINGANDROID_JNI_KERNELS_ISPC_H
#define ISPC_C__USERS_EAYUSOV_PERFORCE_EAYUSOV_DJBOOKOU_DESK5_PROJECTS_SAMPLES_CLUSTEREDSHADINGANDROID_JNI_KERNELS_ISPC_H

#include <stdint.h>



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus
#ifndef __ISPC_STRUCT_PointLight__
#define __ISPC_STRUCT_PointLight__
struct PointLight {
    float positionView[3];
    float attenuationBegin;
    float color[3];
    float attenuationEnd;
};
#endif

#ifndef __ISPC_STRUCT_LightBounds__
#define __ISPC_STRUCT_LightBounds__
struct LightBounds {
    int32_t p1[3];
    int32_t p2[3];
};
#endif

#ifndef __ISPC_STRUCT_Camera__
#define __ISPC_STRUCT_Camera__
struct Camera {
    float proj11;
    float proj22;
    float near;
    float far;
};
#endif

#ifndef __ISPC_STRUCT_LightGridDimensions__
#define __ISPC_STRUCT_LightGridDimensions__
struct LightGridDimensions {
    int32_t width;
    int32_t height;
    int32_t depth;
};
#endif

#ifndef __ISPC_STRUCT_Fragment__
#define __ISPC_STRUCT_Fragment__
struct Fragment {
    int32_t cellIndex;
    int32_t lightIndex;
    uint64_t coverage;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
extern "C" {
#endif // __cplusplus
    extern void CoarseRasterizeLights(struct PointLight * lights, struct LightBounds * bounds, int32_t lightCount, struct Camera * camera, struct LightGridDimensions * dim);
    extern void FineRasterizeLights(struct PointLight * lights, struct Fragment * fragments, int32_t fragmentCount, struct Camera * camera, struct LightGridDimensions * dim);
    extern void RasterizeLights(struct PointLight * lights, uint32_t opaque, int32_t lightCount, struct Camera * camera, struct LightGridDimensions * dim);
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus

#endif // ISPC_C__USERS_EAYUSOV_PERFORCE_EAYUSOV_DJBOOKOU_DESK5_PROJECTS_SAMPLES_CLUSTEREDSHADINGANDROID_JNI_KERNELS_ISPC_H