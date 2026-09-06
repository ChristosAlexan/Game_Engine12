#pragma once
#include "DX12Includes.h"

namespace RootSlot
{
    enum class Raster : UINT
    {
        CameraVS = 0,  // b0 space0: VS global matrices
        LightPS = 1,  // b0 space0: PS global light simple properties
        MaterialTexPS = 2,  // t1 space1: PS material textures descriptor table
        SkinningInputVS = 3,  // b1 space0: VS animation/skinning parameters
        GbufferTexPS = 4,  // t0 space0: PS/All G-Buffer target views
        MaterialBuffer = 5,  // dynamic Instance SRV (t0 space9)
        CameraPS = 6,  // b2 space0: PS Camera parameters
        LightsStructuredBuffer = 7,  // t0 space2: Light array data table
        CubemapPS = 8,  // t0 space3: PS environment map
        GbufferCubePS = 9,  // t4 space0: PS reflection cubemap probe
        PbrParamsPS = 10, // b3 space0: PS PBR parameters
        PrefilterMap = 11, // t0 space4: IBL Prefiltered environment map
        IrradianceMap = 12, // t1 space4: IBL Irradiance map
        BrdfLUT = 13, // t2 space4: IBL BRDF look-up table
        GlobalLightData = 14, // b4 space0: PS global fallback light data
        TlasSRV = 15, // t0 space5: Dynamic DXR Acceleration Structure 
        RtUAVOutput = 16, // u0 space5: DXR compute output view 
        RtShadowsSRV = 17, // t0 space6: Raytraced shadow maps
        RtShadowsInput = 18, // t3 space4: Raytraced shadow lightpass input
        SkinningOutput = 19, // u1 space8: Dynamic skinning transform output table
        ShadowsData = 20, // t1 space2: Shadow map array layout
        RtReflections = 21, // t4 space4: Raytraced reflection input view
        RtAmbientOccl = 22, // t5 space4: Raytraced AO input view
        FxaaParamsPS = 23, // b5 space0: Post-processing parameters
        LightPassTexPS = 24, // t5 space0: Deferred shading compilation target
		InstanceDataBuffer = 25, // t0 space9: Dynamic array of Transform per instance
        BindlessTextures = 26,  // t0 space10
        MeshDataOffsets = 27,  // t3 space11
        Count = 28
    };

    enum class RayTracing : UINT
    {
        GbufferTex = 0,  // t0 space0
        TlasSRV = 1,  // t0 space5
        RtUAVOutput = 2,  // u0 space5
        LightsStructuredBuffer = 3,  // t0 space2
        GlobalLightData = 4,  // b4 space0
        CameraData = 5,  // b0 space0
        BindlessTextures = 6,  // t0 space10
        RtVertexData = 7,  // t1 space10
        RtIndexData = 8,  // t2 space10
        RtMeshDataOffsets = 9,  // t3 space10
        RtReflectionParams = 10, // b5 space0
        RtAoParams = 11, // b5 space1
        Count = 12
    };

    enum class Compute : UINT
    {
        SkinningInput = 0,  // t1 space8
        SkinningParams = 1,  // b0 space8
        SkinningOutput = 2,  // u1 space8
        Count = 3
    };
}