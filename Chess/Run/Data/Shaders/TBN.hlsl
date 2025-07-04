//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 modelPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 modelTangent : TANGENT;
	float3 modelBitangent : BITANGENT;
	float3 modelNormal : NORMAL;
};

struct v2g_t
{
    float3 worldPos     : WORLD_POSITION;
    float3 worldTangent : WORLD_TANGENT;
    float3 worldBitangent : WORLD_BITANGENT;
    float3 worldNormal  : WORLD_NORMAL;
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

struct GS_LINE_OUTPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};


//------------------------------------------------------------------------------------------------
v2g_t VertexMain(vs_input_t input)
{
	// float4 modelPosition = float4(input.modelPosition, 1);
	// float4 worldPosition = mul(c_modelToWorldTransform, modelPosition);
	// float4 cameraPosition = mul(c_worldToCameraTransform, worldPosition);
	// float4 renderPosition = mul(c_cameraToRenderTransform, cameraPosition);
	// float4 clipPosition = mul(c_renderToClipTransform, renderPosition);

	// float4 worldTangent = mul(c_modelToWorldTransform, float4(input.modelTangent, 0.0f));
	// float4 worldBitangent = mul(c_modelToWorldTransform, float4(input.modelBitangent, 0.0f));
	// float4 worldNormal = mul(c_modelToWorldTransform, float4(input.modelNormal, 0.0f));

	v2g_t output;
    float4 modelPosition = float4(input.modelPosition, 1);
    output.worldPos = mul(c_modelToWorldTransform, modelPosition).xyz;

    output.worldTangent   = mul(c_modelToWorldTransform, float4(input.modelTangent, 0.0f)).xyz;
    output.worldBitangent = mul(c_modelToWorldTransform, float4(input.modelBitangent, 0.0f)).xyz;
    output.worldNormal    = mul(c_modelToWorldTransform, float4(input.modelNormal, 0.0f)).xyz;
	return output;
}

[maxvertexcount(18)]
void GeometryMain(triangle v2g_t input[3], inout LineStream<GS_LINE_OUTPUT> OutputStream)
{
    float4x4 worldToClip = mul(c_renderToClipTransform, mul(c_cameraToRenderTransform, c_worldToCameraTransform));

    float lineLength = 0.01f;

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float3 pos      = input[i].worldPos;
        float3 tangent  = normalize(input[i].worldTangent);
        float3 bitangent= normalize(input[i].worldBitangent);
        float3 normal   = normalize(input[i].worldNormal);

        float3 t_end = pos + tangent   * lineLength;
        float3 b_end = pos + bitangent * lineLength;
        float3 n_end = pos + normal    * lineLength;

        float4 pos_clip = mul(worldToClip, float4(pos,    1.0f));
        float4 t_clip   = mul(worldToClip, float4(t_end,  1.0f));
        float4 b_clip   = mul(worldToClip, float4(b_end,  1.0f));
        float4 n_clip   = mul(worldToClip, float4(n_end,  1.0f));

        GS_LINE_OUTPUT v0, v1;
        // Tangent: Red
        v0.pos = pos_clip; v0.color = float4(1,0,0,1);
        v1.pos = t_clip;   v1.color = float4(1,0,0,1);
        OutputStream.Append(v0); OutputStream.Append(v1);
        OutputStream.RestartStrip();

        // Bitangent: Green
        v0.pos = pos_clip; v0.color = float4(0,1,0,1);
        v1.pos = b_clip;   v1.color = float4(0,1,0,1);
        OutputStream.Append(v0); OutputStream.Append(v1);
        OutputStream.RestartStrip();

        // Normal: Blue
        v0.pos = pos_clip; v0.color = float4(0,0,1,1);
        v1.pos = n_clip;   v1.color = float4(0,0,1,1);
        OutputStream.Append(v0); OutputStream.Append(v1);
        OutputStream.RestartStrip();
    }
}

float4 PixelMain(GS_LINE_OUTPUT input) : SV_Target
{
    return input.color;
}