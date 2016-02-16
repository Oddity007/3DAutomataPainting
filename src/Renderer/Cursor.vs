#version 330

uniform mat4 ModelMatrix_client;
uniform mat4 ViewMatrix_client;
uniform mat4 ProjectionMatrix_client;
uniform vec3 Center_client;
uniform float Radius_client;

in vec3 VertexPosition_client;

out vec3 WorldPosition_vertex;

void main(void)
{
	vec3 vertexPosition = VertexPosition_client;

	vec4 worldPosition =
		ModelMatrix_client *
		vec4(vertexPosition * Radius_client, 1.0);

	WorldPosition_vertex = worldPosition.xyz;

	gl_Position =
		ProjectionMatrix_client *
		ViewMatrix_client *
		worldPosition;
}
