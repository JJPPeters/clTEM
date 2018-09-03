#version 130

//uniform sampler2D SphereTexture;

//precision mediump float;

in vec4 ColourPass;
                                                           
out vec4 FragColor;
                                                                                    
void main()
{                   
	vec2 shifted = gl_PointCoord - vec2(0.5, 0.5);
	float rsq = dot(shifted, shifted);
	if(rsq >= 0.1) discard;

	//vec4 texValue = texture(SphereTexture, gl_PointCoord);
    FragColor = ColourPass;//texValue * ColourPass;
}