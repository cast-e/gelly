#include "FluidRenderCBuffer.hlsli"
#include "SplattingStructs.hlsli"

PS_OUTPUT main(GS_OUTPUT input) {
    PS_OUTPUT output = (PS_OUTPUT)0;
    
    float3 normal;
    normal.xy = input.Tex * float2(2.0, -2.0) + float2(-1.0, 1.0);
    float magnitude = dot(normal.xy, normal.xy);

    if (magnitude > 1.0) {
        discard;
    }

    normal.z = sqrt(1.0 - magnitude);

    output.DepthOut = float4(normal * 0.5 + 0.5, 1.0); // for debugging
    return output;
}