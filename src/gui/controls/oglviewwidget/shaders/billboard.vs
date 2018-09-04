#version 130

uniform mat4 ModelView;
uniform mat4 Proj;
                                                                                   
attribute vec3 PosBuf;  
attribute vec3 ColBuf;                                            

out vec4 ColourPass;

uniform vec2 ScreenSize;

void main()                                                                         
{
    ColourPass = vec4(ColBuf, 1.0);

    vec4 EyePos = ModelView * vec4(PosBuf, 1);

    vec4 ProjVoxel = Proj * vec4(0.5, 0.5, EyePos.z, EyePos.w);

    vec2 ProjSize = ScreenSize * ProjVoxel.xy / ProjVoxel.w;

    gl_PointSize = 0.8 * (ProjSize.x + ProjSize.y);

    gl_Position = 1.5 * Proj * EyePos;
}                                                                                   
