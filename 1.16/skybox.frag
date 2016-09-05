#version 150

in vec2 ex_TexCoord;

out vec4 out_Color;

uniform sampler2D tex8;


void main(void)
{
    out_Color = texture(tex8,ex_TexCoord);
}
