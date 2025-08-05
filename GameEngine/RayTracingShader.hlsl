typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct [raypayload] RayPayload
{
    float4 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct Attributes
{
    float2 barycentric;
};

RWTexture2D<float4> gOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space5);

[shader("raygeneration")]
void RayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    gOutput[launchIndex] = float4(0, 1, 0, 1); // red for testing
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}
