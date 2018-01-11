#version 430 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 MVP;
uniform mat3 NormalMatrix;
uniform vec4 outcolor;

out vec3 Normal;

void main()
{
    Normal = normalize(NormalMatrix * VertexPosition);
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
