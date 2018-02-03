#version 330

//layout(location=0) in vec2 VertexPosition;
//layout(location=1) in float VertexTexCoord;

in vec2 VertexPosition;
in float VertexTexCoord;

out float texCoord;

void main()
{
    gl_Position = vec4(VertexPosition, 0.0, 1.0);
    texCoord = VertexTexCoord;
}
