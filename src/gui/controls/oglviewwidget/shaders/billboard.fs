#version 150

in vec2 Vertex_UV;
in vec4 Vertex_Color;
                                                           
out vec4 FragColor;
                                                                                    
void main()
{
	vec2 uv = Vertex_UV.xy - vec2(0.5, 0.5);
	float rsq = dot(uv, uv);
	if(rsq >= 0.25)
	    discard;
	else if (rsq >= 0.22)
        FragColor = vec4(1.0,1.0,1.0,1.0);
    else
        FragColor = Vertex_Color;
}