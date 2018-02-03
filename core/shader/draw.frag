#version 330

in vec3 Normal;
uniform vec4 outcolor;
layout( location = 0 ) out vec4 FragColor;

void main()
{
    FragColor = outcolor;
}

