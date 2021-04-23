#version 410
#extension GL_ARB_explicit_uniform_location : enable

out vec4 color;
in vec2 fragTex;

uniform float on;

layout(location = 0) uniform sampler2D tex; // color
layout(location = 1) uniform sampler2D tex2; // other tex


void main()
{
	vec3 lp = vec3(20,20,100);

	vec3 texturecolor = texture(tex, fragTex).rgb;
	vec3 pixelpos = texture(tex2, fragTex).rgb;

	float dist = distance(lp, pixelpos);

	color.rgb = texturecolor;

	//if (on == 1)
	//{
		
		float gamma = 1.5;
		float attenuation = 1.0/(dist);
		//vec3 diffuseColor = pow(texturecolor, vec3(1/gamma));
		//color.rgb *= diffuseColor;
		if (color.rgb == vec3(0.1f, 0.1f, 0.1f))
			color.rgb = vec3(0.1f, 0.1f, 0.1f);
		else
			color.rgb = pow(color.rgb, vec3(1.0/gamma)) - attenuation;

	//}
	color.a =1;
}
