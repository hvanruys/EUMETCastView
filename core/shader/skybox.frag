#version 430 core

in vec3 TexCoords;
out vec4 color;

layout(binding=4) uniform samplerCube skybox;

void main()
{
    color = texture(skybox, TexCoords);
}

