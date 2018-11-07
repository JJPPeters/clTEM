#version 150
                                                                                    
layout(points) in;
layout(triangle_strip) out;                                                         
layout(max_vertices = 4) out;

uniform mat4 Proj;

in vec4 ColourPass[];

out vec2 Vertex_UV;
out vec4 Vertex_Color;
                                                                                    
void main()                                                                         
{ 
    vec4 P = gl_in[0].gl_Position;

    float particle_size = 1.0f;

    Vertex_Color = ColourPass[0];

    vec2 va = P.xy + vec2(-0.5, -0.5) * particle_size;
    gl_Position = Proj * vec4(va, P.zw);
    Vertex_UV = vec2(0.0, 0.0);
    //Vertex_Color = ;
    EmitVertex();

    vec2 vb = P.xy + vec2(-0.5, 0.5) * particle_size;
    gl_Position = Proj * vec4(vb, P.zw);
    Vertex_UV = vec2(0.0, 1.0);
    //Vertex_Color = aColourpass[0];
    EmitVertex();

    vec2 vd = P.xy + vec2(0.5, -0.5) * particle_size;
    gl_Position = Proj * vec4(vd, P.zw);
    Vertex_UV = vec2(1.0, 0.0);
    //Vertex_Color = aColourpass[0];
    EmitVertex();

    vec2 vc = P.xy + vec2(0.5, 0.5) * particle_size;
    gl_Position = Proj * vec4(vc, P.zw);
    Vertex_UV = vec2(1.0, 1.0);
    //Vertex_Color = aColourpass[0];
    EmitVertex();

    EndPrimitive();
}
