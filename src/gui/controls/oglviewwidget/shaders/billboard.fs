#version 150

in vec2 Vertex_UV;
in vec4 Vertex_Color;
                                                           
out vec4 FragColor;
                                                                                    
void main()
{
	vec2 uv = Vertex_UV.xy - vec2(0.5, 0.5);
	float rsq = dot(uv, uv);

	float outside = 0.25f;
	float ring = 0.23f;

	if(rsq >= outside)
	    discard;
	else if (rsq >= ring)
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    else
        FragColor = Vertex_Color;
}