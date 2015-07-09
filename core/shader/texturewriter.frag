#version 420 core

in float texCoord;
out vec4 color;
layout (binding = 0) uniform sampler1D tex;

void main()
{

//    if(v_texCoord.y > 0.4 && v_texCoord.y <= 0.6)
        //color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
        //color = texture( tex, vpos.x );
//    else
//        discard;
    color = texture( tex, texCoord );

}
