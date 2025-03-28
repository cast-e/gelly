#include "SplattingStructs.hlsli"
#include "FluidRenderCBuffer.hlsli"

[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream) {
    GS_OUTPUT output = (GS_OUTPUT)0;
	float4x4 invQuadric = input[0].InvQuadric;
	float3 absorption = input[0].Absorption;
	float acceleration = input[0].Acceleration;
	float variance = input[0].Variance;
	float2 ndcPos = input[0].NDCPos;

    // if we're outside of the NDC cube, we might as well discard
	[branch]
	if (ndcPos.x < -1.f || ndcPos.x > 1.f || ndcPos.y < -1.f || ndcPos.y > 1.f) {
		return;
	}

    float4 bounds = input[0].Bounds;

    float xmin = bounds.x;
    float xmax = bounds.y;
    float ymin = bounds.z;
    float ymax = bounds.w;

    output.InvQuadric = invQuadric;
	output.Absorption = absorption;
	output.Acceleration = acceleration;

    output.Pos = float4(xmin, ymax, 0.5f, 1.0f);
    triStream.Append(output);

    output.Pos = float4(xmin, ymin, 0.5f, 1.0f);
    triStream.Append(output);

    output.Pos = float4(xmax, ymax, 0.5f, 1.0f);
    triStream.Append(output);

    output.Pos = float4(xmax, ymin, 0.5f, 1.0f);
    triStream.Append(output);
}