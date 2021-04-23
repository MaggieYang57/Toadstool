#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec3 camoff;
uniform vec3 campos;

void main()
{
	vec2 texcoords = vertex_tex;
	float t = 1./100;
	texcoords -= vec2(camoff.x, camoff.z) * t;
	//vec3 heightcolor = texture(tex, texcoords).rgb;

	vec4 heightcolor = texture(tex,vertex_tex);
	heightcolor.r = 0.4 + heightcolor.r*0.9;
	color.rgb = texture(tex2,texcoords *50).rgb * heightcolor.r;
	color.a = 1;

	//faded background
	float len = length(vertex_pos.xz + campos.xz);
	len -= 45;
	len /= 4.;
	len = clamp(len, 0, 1);
	color.a = 1 - len;

}
