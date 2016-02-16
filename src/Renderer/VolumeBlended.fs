#version 330

//layout(location = 0)
out vec4 Albedo_fragment;
//layout(location = 1) out vec4 SpecularSmoothness_fragment;
//layout(location = 2) out vec4 Normal_fragment;

uniform sampler3D BlendMap_client;
uniform vec3 BlendMapMin_client;
uniform vec3 BlendMapMax_client;
//uniform sampler2D BaseColorMaps_client[16];
//uniform sampler2D SpecularSmoothnessMap_client;
//uniform sampler2D NormalMap_client;

in mat3 TBNMatrix_vertex;
in vec2 Texcoord_vertex;
in vec3 WorldPosition_vertex;

void main(void)
{
	mat3 tbnm = mat3(normalize(TBNMatrix_vertex[0]), normalize(TBNMatrix_vertex[1]), normalize(TBNMatrix_vertex[2]));
	
	vec3 geonormal = normalize(TBNMatrix_vertex[2]);
	
	Albedo_fragment = vec4(0, 0, 0, 1);
	vec3 blendingWeights = geonormal * geonormal;
	
	//Albedo_fragment.rgb = texture(BaseColorMap_client, vec2(Texcoord_vertex.x, 1.0 - Texcoord_vertex.y)).rgb;
	
	vec3 blendControlSamplePosition = (WorldPosition_vertex - BlendMapMin_client) / (BlendMapMax_client - BlendMapMin_client);
	vec4 blendControl = texture(BlendMap_client, blendControlSamplePosition);
	
	/*for (int i = 0; i < 8; i++)
	{
		vec4 sample = texture(BaseColorMaps_client[i], );
	}*/
	
	Albedo_fragment.rgb *= max(0, dot(geonormal, normalize(vec3(1, 1, 1))));
}

