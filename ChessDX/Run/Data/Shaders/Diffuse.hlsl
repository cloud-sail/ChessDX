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

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition		: SV_Position;
	float4 color			: COLOR;
	float2 uv				: TEXCOORD;
	float3 worldPos			: WORLD_POSITION;
	float4 worldTangent		: WORLD_TANGENT;
	float4 worldBitangent	: WORLD_BITANGENT;
	float4 worldNormal		: WORLD_NORMAL;
	float4 modelTangent		: MODEL_TANGENT;
	float4 modelBitangent	: MODEL_BITANGENT;
	float4 modelNormal		: MODEL_NORMAL;
};

//-----------------------------------------------------------------------------------------------
struct Light
{
	float4 	c_color;					// Alpha(w) is the light intensity in [0,1]
	float3 	c_worldPosition;			// 
	float  	padding;					//
	float3 	c_spotForward;				// Forward Normal for spotlights (zero for omnidirectional point-lights)
	float  	c_ambience;					// Portion of indirect light this source gives to objects in its affected volume
	float	c_innerRadius;
	float	c_outerRadius;
	float	c_innerDotThreshold;		// if dot with forward is greater than inner threshold, full strength; -1 for point lights
	float	c_outerDotThreshold;		// if dot with forward is less than outer threshold, full strength; -2 for point lights
};

//-----------------------------------------------------------------------------------------------
cbuffer EngineConstants : register(b0)
{
	int		c_debugInt;
	float	c_debugFloat;
	float2  padding_01;
}

//-----------------------------------------------------------------------------------------------
cbuffer PerFrameConstants : register(b1)
{
	float3  c_resolution;
	float	c_time;

	float4	c_mouse;
}
// Shows how to use the mouse input (only left button supported):
//
//      mouse.xy  = mouse position during last button down
//  abs(mouse.zw) = mouse position during last button click
// sign(mouze.z)  = button is down
// sign(mouze.w)  = button is clicked

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

//------------------------------------------------------------------------------------------------
#define MAX_LIGHTS 8
cbuffer LightConstants : register(b4)
{
	float4 	c_sunColor; 				// Alpha (w) channel is intensity; parallel sunlight
	float3  c_sunNormal;				// Forward direction of parallel sunlight
	int		c_numLights;
	Light 	c_lightArray[MAX_LIGHTS];
};



//------------------------------------------------------------------------------------------------
Texture2D<float4> t_diffuseTexture : register(t0);
Texture2D<float4> t_normalTexture : register(t1);
Texture2D<float4> t_sgeTexture : register(t2);  // R=Specular, G=Glossy, B=Emissive
TextureCube t_cubeMap : register(t9);
//------------------------------------------------------------------------------------------------
SamplerState s_diffuseSampler : register(s0);
SamplerState s_normalSampler : register(s1);
SamplerState s_specGlossEmitSampler : register(s2);

//-----------------------------------------------------------------------------------------------
float3 EncodeXYZToRGB( float3 vec )
{
	return (vec + 1.0) * 0.5; 
}

float3 DecodeRGBToXYZ( float3 color )
{
	return (color * 2.0) - 1.0;
}

float3 GetCameraWorldPosition(float4x4 viewMatrix)
{
	float3x3 rotationMatrix = float3x3(
        viewMatrix[0].xyz,
        viewMatrix[1].xyz,
        viewMatrix[2].xyz 
    );
	
	float3 translation = viewMatrix._m03_m13_m23;
	
	float3 cameraPosition = mul(-translation, rotationMatrix);
	return cameraPosition;
}

float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	float fraction = (inValue - inStart) / (inEnd - inStart);
	float outValue = outStart + fraction * (outEnd - outStart);
	return outValue;
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	float fraction = saturate( (inValue - inStart) / (inEnd - inStart) );
	float outValue = outStart + fraction * (outEnd - outStart);
	return outValue;
}

float SmoothStep3(float t)
{
	return t * t * (3.0 - 2.0 * t);
}


//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 modelPosition = float4(input.modelPosition, 1);
	float4 worldPosition = mul(c_modelToWorldTransform, modelPosition);
	float4 cameraPosition = mul(c_worldToCameraTransform, worldPosition);
	float4 renderPosition = mul(c_cameraToRenderTransform, cameraPosition);
	float4 clipPosition = mul(c_renderToClipTransform, renderPosition);

	float4 worldTangent = mul(c_modelToWorldTransform, float4(input.modelTangent, 0.0f));
	float4 worldBitangent = mul(c_modelToWorldTransform, float4(input.modelBitangent, 0.0f));
	float4 worldNormal = mul(c_modelToWorldTransform, float4(input.modelNormal, 0.0f));

	v2p_t v2p;
	v2p.clipPosition = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldPos = worldPosition.xyz;
	v2p.worldTangent = worldTangent;
	v2p.worldBitangent = worldBitangent;
	v2p.worldNormal = worldNormal;
	v2p.modelTangent = float4(input.modelTangent, 0.0f);
	v2p.modelBitangent = float4(input.modelBitangent, 0.0f);
	v2p.modelNormal = float4(input.modelNormal, 0.0f);
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float2 uvCoords = input.uv;

	float4 diffuseTexel = t_diffuseTexture.Sample(s_diffuseSampler, uvCoords);
	//Texture2D<float4> albedoTexture = ResourceDescriptorHeap[0];
	//diffuseTexel = albedoTexture.Sample(s_diffuseSampler, uvCoords);
	float4 normalTexel	= t_normalTexture.Sample(s_normalSampler, uvCoords);
	float4 specGlossEmitTexel		= t_sgeTexture.Sample(s_specGlossEmitSampler, uvCoords);

	float4 surfaceColor = input.color;
	float4 modelColor = c_modelColor;

	float4 diffuseColor = diffuseTexel * surfaceColor * modelColor;
	clip(diffuseColor.a < 0.01f);

	float specularity	= specGlossEmitTexel.r;
	float glossiness	= specGlossEmitTexel.g;
	float emissive		= specGlossEmitTexel.b;

	// float3 cameraWorldPosition = GetCameraWorldPosition(c_worldToCameraTransform);
	float3 cameraWorldPosition = c_cameraWorldPosition;

	// Calculate worldNormal
	float3 pixelNormalTBNSpace = normalize(DecodeRGBToXYZ(normalTexel.rgb));



	//float3 surfaceNormalWorldSpace = normalize(input.worldNormal.xyz);
	//float3 surfaceTangentWorldSpace = normalize(input.worldTangent.xyz);
	//float3 surfaceBitangentWorldSpace = normalize(input.worldBitangent.xyz);

	float3 surfaceNormalWorldSpace = normalize(input.worldNormal.xyz);
	float3 surfaceTangentWorldSpace = normalize(input.worldTangent.xyz - dot(input.worldTangent.xyz, surfaceNormalWorldSpace) * surfaceNormalWorldSpace);
	float3 surfaceBitangentWorldSpace = cross(surfaceNormalWorldSpace, surfaceTangentWorldSpace); // reset the handness?

	float3x3 tbnToWorld = float3x3(surfaceTangentWorldSpace, surfaceBitangentWorldSpace, surfaceNormalWorldSpace);
	
	float3 pixelNormalWorldSpace = mul(pixelNormalTBNSpace, tbnToWorld);

	if (c_debugInt == 3)
	{
		pixelNormalWorldSpace = surfaceNormalWorldSpace; // Bypass normal map normals
	}

	float3 N = normalize(pixelNormalWorldSpace);
	float3 V = normalize(cameraWorldPosition - input.worldPos);
	float specularExponent = RangeMapClamped(glossiness, 0.f, 1.f, 1.f, 32.f);

	float3 totalDiffuseLight = float3(0.f, 0.f, 0.f);
	float3 totalSpecularLight = float3(0.f, 0.f, 0.f);

	//-----------------------------------------------------------------------------------------------------------
	// Sunlight
	//-----------------------------------------------------------------------------------------------------------
	{
		float sunAmbience = 0.2f;
		float3 L = -c_sunNormal;

		// diffuse lighting with progressive ambience
		float lightStrength = c_sunColor.a * saturate(RangeMap(dot(L, N), -sunAmbience, 1.f, 0.f, 1.f));
		totalDiffuseLight += (lightStrength * c_sunColor.rgb);

		// specular lighting
		float3 H = normalize(V + L);
		float NDotH = saturate(dot(N, H)); // SpecularDot
		float specularStength = glossiness * specularity * c_sunColor.a * pow(NDotH, specularExponent);
		totalSpecularLight += specularStength * c_sunColor.rgb;
	}

	//-----------------------------------------------------------------------------------------------------------
	// Point & Spot Lights
	//-----------------------------------------------------------------------------------------------------------
	for (int lightIndex = 0; lightIndex < c_numLights; ++lightIndex)
	{
		float3 lightPos 		= c_lightArray[lightIndex].c_worldPosition;
		float3 lightColor 		= c_lightArray[lightIndex].c_color.rgb;
		float  ambience			= c_lightArray[lightIndex].c_ambience;
		float  lightBrightness	= c_lightArray[lightIndex].c_color.a;
		float3 spotForward		= c_lightArray[lightIndex].c_spotForward;
		float  innerRadius		= c_lightArray[lightIndex].c_innerRadius;
		float  outerRadius		= c_lightArray[lightIndex].c_outerRadius;
		float  innerPenumbraDot	= c_lightArray[lightIndex].c_innerDotThreshold;
		float  outerPenumbraDot	= c_lightArray[lightIndex].c_outerDotThreshold;

		float dist = length(lightPos - input.worldPos);
		float3 L = normalize(lightPos - input.worldPos);

		float fallOff = saturate(RangeMap(dist, innerRadius, outerRadius, 1.f, 0.f));
		fallOff = SmoothStep3(fallOff);

		float penumbra = saturate(RangeMap(dot(-L, spotForward), innerPenumbraDot, outerPenumbraDot, 1.f, 0.f));
		penumbra = SmoothStep3(penumbra);

		float lightStrength = fallOff * penumbra * lightBrightness * saturate(RangeMap(dot(L, N), -ambience, 1.f, 0.f, 1.f));
		totalDiffuseLight += lightStrength * lightColor;

		float3 H = normalize(V + L);
		float NDotH = saturate(dot(N, H)); // SpecularDot
		float specularStength = glossiness * specularity * lightBrightness * fallOff * penumbra * pow(NDotH, specularExponent);
		totalSpecularLight += specularStength * lightColor;
	}

	//-----------------------------------------------------------------------------------------------------------
	// Emissive (glow)
	//-----------------------------------------------------------------------------------------------------------
	float3 emissiveLight = diffuseTexel.rgb * emissive;

	//-----------------------------------------------------------------------------------------------------------
	// Final lighting composite
	//-----------------------------------------------------------------------------------------------------------
	float3 finalRGB = saturate(totalDiffuseLight) * diffuseColor.rgb + totalSpecularLight + emissiveLight;

	// Add Specular reflection

	float3 R = reflect(-V, N);
	R = mul(c_cameraToRenderTransform, float4(R, 0.f)).xyz;

	if (c_debugInt == 9)
	{
		float cosIncidentAngle = saturate(dot(N, R));
		// float3 fresnelR0 = lerp(float3(0.04, 0.04, 0.04), diffuseColor.rgb, specularity);
		float3 fresnelR0 = float3(0.5f, 0.5f, 0.5f);
		float f0 = 1.f - cosIncidentAngle;
		float3 fresenelFactor = fresnelR0 + (1.f - fresnelR0) * f0 * f0 * f0 * f0 * f0;
		float3 envColor = t_cubeMap.Sample(s_specGlossEmitSampler, R).rgb;
		float reflectionStrength = specularity * glossiness;
		// float reflectionStrength = 0.5f;
		finalRGB = lerp(finalRGB, envColor, reflectionStrength * fresenelFactor);
	}
	else
	{
		float3 envColor = t_cubeMap.Sample(s_specGlossEmitSampler, R).rgb;
		float reflectionStrength = specularity * glossiness;
		finalRGB = lerp(finalRGB, envColor, reflectionStrength);
	}


	float4 finalColor = float4(finalRGB, diffuseColor.a);
	
	if (c_debugInt == 1)
	{
	    finalColor.rgba = diffuseTexel.rgba; 
	}
	else if (c_debugInt == 2 )
	{
		finalColor.rgb = EncodeXYZToRGB(N);
	}
	else if (c_debugInt == 3 )
	{
		finalColor.rgb = EncodeXYZToRGB(N);
	}
	else if (c_debugInt == 4 )
	{
		finalColor.rgb = totalSpecularLight;
	}
	else if (c_debugInt == 5 )
	{
		finalColor.rgb = emissiveLight;
	}
	else if (c_debugInt == 6 )
	{
		finalColor.rgb = saturate(totalDiffuseLight) * diffuseColor.rgb;
	}
	else if (c_debugInt == 7)
	{
		finalColor.rgb = EncodeXYZToRGB(input.worldTangent.xyz);
	}
	else if (c_debugInt == 8)
	{
		finalColor.rgb = EncodeXYZToRGB(input.worldBitangent.xyz);
	}

	// else if (c_debugInt == 3 )
	// {
	// 	finalColor.rgba = float4(input.uv, 0, 1);
	// }	
	// /*
	// else if (c_debugInt == 4 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(surfaceTangentWorldSpace) );
	// }
	// else if (c_debugInt == 5 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(surfaceBitangentWorldSpace) );
	// }
	// else if (c_debugInt == 6 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(surfaceNormalWorldSpace) );
	// } */
	// else if (c_debugInt == 4 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(input.modelTangent.xyz) );
	// }
	// else if (c_debugInt == 5 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(input.modelBitangent.xyz) );
	// }
	// else if (c_debugInt == 6 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB( normalize(input.modelNormal.xyz) );
	// }
	// else if (c_debugInt == 7 )
	// {
	// 	finalColor.rgba = normalTexel.rgba;
	// }
	// else if (c_debugInt == 8 )
	// {
	// 	finalColor.rgb = EncodeXYZToRGB(pixelNormalWorldSpace);
	// }
	// else if (c_debugInt == 9 )
	// {
	// 	finalColor.rgb = saturate(dot(normalize(pixelNormalWorldSpace), -SunDirection)).xxx;
	// }

	return finalColor;
}
