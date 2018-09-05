#version 150

uniform mat4 ModelView;
                                                                                   
in vec3 PosBuf;
in vec3 ColBuf;

out vec4 ColourPass;

void main()                                                                         
{
    gl_Position = ModelView * vec4(PosBuf, 1.0);
    ColourPass = vec4(ColBuf, 1.0);
}                                                                                   
