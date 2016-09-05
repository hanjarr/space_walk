#version 150

in  vec3 in_Position;
in  vec2 in_TexCoord;

out vec2 ex_TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 model2world;
uniform mat4 world2view;

void main(void)
{
	ex_TexCoord = in_TexCoord;
	gl_Position = projectionMatrix*world2view*model2world*vec4(in_Position, 1.0);
}
