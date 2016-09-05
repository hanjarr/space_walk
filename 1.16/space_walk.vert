#version 150

in  vec3 in_Position;
in  vec3 in_Normal;

out vec3 ex_Normal;
out vec3 ex_Surface;

out vec4 texCoords;

uniform mat4 projectionMatrix;
uniform mat4 model2world;
uniform mat4 world2view;

void main(void)
{
	ex_Normal = mat3(model2world)*in_Normal;
	ex_Surface = vec3(model2world*vec4(in_Position, 1.0));
	gl_Position = projectionMatrix*world2view*model2world*vec4(in_Position, 1.0);
    
    texCoords = vec4(in_Position, 1.0);
}
