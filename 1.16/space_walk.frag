#version 150

in vec3 ex_Normal;
in vec3 ex_Surface;
in vec4 texCoords;

out vec4 out_Color;

uniform sampler2D tex;

uniform vec3 camPos;
uniform vec3 lightPos;

uniform bool isDirectional[2];


void main(void)
{
    float diffuse, specular, shade;
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 light = vec3(0.0, 0.0, 0.0);
    out_Color = vec4(0.0, 0.0, 0.0, 0.0);

    //Transform to spherical coordiantes
    vec2 longitudeLatitude = vec2((atan(texCoords.y, texCoords.x) / 3.1415926 + 1.0) * 0.5,
      (asin(texCoords.z) / 3.1415926 + 0.5));

    // Runs both light sources
    for (int i=0; i<2; i++)
    {
      if (isDirectional[i])
        light = normalize(vec3(0.0,1.0,0.0));

    else
        light = normalize(lightPos - ex_Surface);
            diffuse = dot(normalize(ex_Normal), light);
            diffuse = max(0.0, diffuse);

            vec3 r = reflect(-light, normalize(ex_Normal));
            vec3 v = normalize(camPos - ex_Surface); 
            specular = dot(r, v);
            if (specular > 0.0)
                specular = 1.0 * pow(specular,3);
            specular = max(specular, 0.0);
            shade = 0.7*diffuse + 1.0*specular;

            color =  shade*vec3(1.0,1.0,1.0) + vec3(0.01,0.01,0.01);

            out_Color = out_Color + vec4(color, 1.0)* texture(tex,longitudeLatitude);
            out_Color[3] = 1.0;
    }
}
