#version 330

in vec3 TexCoords;
out vec4 color;

//layout(binding=4) uniform samplerCube skybox;
uniform samplerCube skybox;

void main()
{
    color = texture(skybox, TexCoords);
}

