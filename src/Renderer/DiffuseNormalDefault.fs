#version 330

//layout(location = 0)
out vec4 Albedo_fragment;
//layout(location = 1) out vec4 SpecularSmoothness_fragment;
//layout(location = 2) out vec4 Normal_fragment;

uniform sampler2D BaseColorMap_client;
//uniform sampler2D SpecularSmoothnessMap_client;
uniform sampler2D NormalMap_client;

uniform sampler3D BlendMap_client;
uniform vec3 BlendMapMin_client;
uniform vec3 BlendMapMax_client;
uniform sampler2D EarthUpMap_client;
uniform sampler2D EarthSideMap_client;
uniform sampler2D EarthDownMap_client;
uniform sampler2D MetalMap_client;

in mat3 TBNMatrix_vertex;
in vec2 Texcoord_vertex;
in vec3 WorldPosition_vertex;

void main(void)
{
	mat3 tbnm = mat3(normalize(TBNMatrix_vertex[0]), normalize(TBNMatrix_vertex[1]), normalize(TBNMatrix_vertex[2]));
	vec4 normalMapSample = vec4(0.5, 0.5, 0, 0);//
	//normalMapSample.xyz = texture(NormalMap_client, Texcoord_vertex).xyz;
	
	vec3 normal = tbnm * (normalMapSample.xyz * 2.0 - 1.0);
	vec3 geonormal = normalize(TBNMatrix_vertex[2]);
	
	//Albedo_fragment = vec4(1, 1, 1, 1);
	//Albedo_fragment = vec4(normal * 0.5 + 0.5, 1);

	//Albedo_fragment = texture(BaseColorMap_client, Texcoord_vertex);
	Albedo_fragment = vec4(0, 0, 0, 1);
	vec3 blendingWeights = geonormal * geonormal;
	
	normal = (tbnm * (texture(NormalMap_client, Texcoord_vertex).rgb * 2.0 - 1.0));
	
	//Maps to the corners of a cube
	vec3 blendMapWeights = texture(BlendMap_client, (WorldPosition_vertex - BlendMapMin_client) / (BlendMapMax_client - BlendMapMin_client)).rgb;
	
	vec3 earthUpSample = texture(EarthUpMap_client, WorldPosition_vertex.xz).rgb;
	vec3 earthDownSample = texture(EarthDownMap_client, -WorldPosition_vertex.xz).rgb;
	vec3 earthSideSample1 = texture(EarthSideMap_client, WorldPosition_vertex.xy).rgb;
	vec3 earthSideSample2 = texture(EarthSideMap_client, WorldPosition_vertex.zy).rgb;
	vec3 earthCombinedSample = vec3(0, 0, 0);
	earthCombinedSample += blendingWeights.x * earthSideSample2;
	earthCombinedSample += blendingWeights.z * earthSideSample1;
	earthCombinedSample += blendingWeights.y * (geonormal.y > 0 ? earthUpSample : earthDownSample);
	
	
	vec3 metalUpSample = texture(MetalMap_client, WorldPosition_vertex.xz).rgb;
	vec3 metalSideSample1 = texture(MetalMap_client, WorldPosition_vertex.xy).rgb;
	vec3 metalSideSample2 = texture(MetalMap_client, WorldPosition_vertex.zy).rgb;
	vec3 metalCombinedSample = vec3(0, 0, 0);
	metalCombinedSample += blendingWeights.x * metalSideSample2;
	metalCombinedSample += blendingWeights.z * metalSideSample1;
	metalCombinedSample += blendingWeights.y * metalUpSample;
	
	vec3 baseColor = texture(BaseColorMap_client, Texcoord_vertex).rgb;
	
	vec3 samples[8];
	samples[0] = baseColor;
	samples[1] = baseColor;
	samples[2] = baseColor;
	samples[3] = baseColor;
	samples[4] = baseColor;
	samples[5] = baseColor;
	samples[6] = baseColor;
	samples[7] = baseColor;
	
	vec3 edges[4];
	for (int i = 0; i < 4; i++)
		edges[i] = mix(samples[i], samples[i + 4], blendMapWeights.x);
	
	vec3 faces[2];
	faces[0] = mix(edges[0], edges[1], blendMapWeights.y);
	faces[1] = mix(edges[2], edges[3], blendMapWeights.y);
	
	Albedo_fragment.rgb = mix(mix(mix(faces[0], faces[1], blendMapWeights.z), earthCombinedSample, blendMapWeights.y), metalCombinedSample, blendMapWeights.x);
	
	//Albedo_fragment.rgb = mix(mix(), , );
	
	//Albedo_fragment.rgb = blendMapWeights * 0.9 + 0.1;
	
	Albedo_fragment.rgb *= max(0.25, dot(normal, normalize(vec3(-1, 1, 0))));
	
	//SpecularSmoothness_fragment = texture(SpecularSmoothnessMap_client, Texcoord_vertex);
	//NormalMap_client.xyz = normal;
	//NormalMap_client.w = 1;
}

