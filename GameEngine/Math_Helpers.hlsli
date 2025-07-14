// To calculate quaternion rotations
float3 RotateVectorByQuaternion(float3 v, float4 q)
{
    float3 u = q.xyz;
    float s = q.w;

    return 2.0f * dot(u, v) * u +
           (s * s - dot(u, u)) * v +
           2.0f * s * cross(u, v);
}