#include "NDCQuadStages.hlsli"

// FYI: The other shaders are used in SHADERed, but not in Gelly.
// This is the Gelly shader used for any technique that is screen-space.

VS_OUTPUT main(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;

	// No transformation required since the geometry is just covering the entire
	// screen in NDC space.
	output.Position = input.Position;
	output.Texcoord = input.Texcoord;

	return output;
}