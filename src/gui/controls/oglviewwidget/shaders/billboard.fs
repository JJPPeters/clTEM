#version 130

in vec4 ColourPass;
                                                           
out vec4 FragColor;
                                                                                    
void main()
{                   
	vec2 shifted = gl_PointCoord - vec2(0.5, 0.5);
	float rsq = dot(shifted, shifted);
	if(rsq >= 0.1) discard;

    if (rsq >= 0.09)
        FragColor = vec4(1.0,1.0,1.0,1.0);
    else
        FragColor = ColourPass;
}