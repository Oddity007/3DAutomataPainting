#version 330

//layout(location = 0)
out vec4 Albedo_fragment;
//layout(location = 1) out vec4 SpecularSmoothness_fragment;
//layout(location = 2) out vec4 Normal_fragment;

in vec3 WorldPosition_vertex;
uniform vec3 Center_client;
uniform float Radius_client;

void main(void)
{
	vec3 difference = WorldPosition_vertex - Center_client;
	//if (dot(difference, difference) > Radius_client * Radius_client)
	//	discard;
	Albedo_fragment = vec4(1, 0.9, 0.9, 1);
}

