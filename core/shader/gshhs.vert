#version 330

layout(location=0) in vec3 VertexPosition;

uniform mat4 MVP;
uniform mat3 NormalMatrix;

out vec3 Normal;
out float angle;

void main()
{
    Normal = normalize(NormalMatrix * VertexPosition);
    angle = 1 / MVP[3].w;
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
