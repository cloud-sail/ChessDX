#include "Common/StaticSampler.hlsli"

struct vs_input_t
{
	float3 modelPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition : SV_POSITION;
    float3 sampleDir : POSITION; // for cubemap sampling
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 	c_worldToCameraTransform;	// View transform
	float4x4 	c_cameraToRenderTransform;	// Non-standard transform from game to DirectX conventions
	float4x4 	c_renderToClipTransform;		// Projection transform
	float3	 	c_cameraWorldPosition;       // Camera World Position
	float		padding_20;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 	c_modelToWorldTransform;		// Model transform
	float4 		c_modelColor;
};



TextureCube t_cubeMap : register(t9);
//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;

	float4 modelPosition = float4(input.modelPosition, 1);
    v2p.sampleDir = mul(c_cameraToRenderTransform, float4(input.modelPosition, 0.f)).xyz;

	// Should be identity M2W transform
    float4 worldPosition = mul(c_modelToWorldTransform, modelPosition);
    worldPosition.xyz += c_cameraWorldPosition; // follow camera

	float4 cameraPosition = mul(c_worldToCameraTransform, worldPosition);
	float4 renderPosition = mul(c_cameraToRenderTransform, cameraPosition);
	float4 clipPosition = mul(c_renderToClipTransform, renderPosition);

	v2p.clipPosition = clipPosition.xyww;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target
{
    return t_cubeMap.Sample(s_pointWrap, input.sampleDir);
}